/*
 * I2CmultiMasterTest.c
 *
 */
#include <Tests/I2CmultiMasterTest.h>

#define ARDUINO_PACKET_MAXSIZE 31

static AT91_REG status_register;

/*!
 * expected packet size - Must be less than I2C_SLAVE_RECEIVE_BUFFER_SIZE
 */
#define I2C_SLAVE_PACKET_SIZE 255
static int I2CInputIndex = 0; //same as bytesRead but used to index
static int packetReady = 1; //if 1, packet is zeroed and ready to receive new data
static unsigned char I2CInput[I2C_SLAVE_PACKET_SIZE] = {0};
static int bytesRead = 0; //tracks bytes read into a packet and can be error codes

/*
 * The way the slave driver works is that is is initialized and started with the commandList table below. This causes the driver to automatically scan each first byte and
 * treat it as a command code, this governs behavior, how many bytes it reads after that command byte. We need to use the table in a way that lets us capture all
 * possible incoming packets since the handling of the packet is for CSP to do.
 *
 */
//HAL SLAVE Testing Code with a 32 byte long packet buffer limit
//The Idea is we want to accept any command and read up to a certain length, currently +31 which equals 32 bytes. The arduinos we use for test
//are limited in I2C packet size so this is why the max is 32.
/*
 * To 'hack' this driver into being compatible with CSP it needs a command list that accepts every possible first byte that could arrive,
 * CSP encodes its first 2 data bytes as 0x0000 to 0x00FF to denote data length. If we cap it at 256 length (likely we will choose a little lower)
 * Then the first byte shall always be 0x00, and we shall always seek the next 255 bytes.
 * and then reads the next PACKET_SIZE-1 bytes. It shall never need to respond as we anticipate CSP handles this by switching to master mode if it needs
 * to send a response. assuming you figure out how to integrate these old drivers with CSP.
 */
static I2CslaveCommandList MMCommandList[] = {	{.command = 0x54, .commandParameterSize = 31, .hasResponse = FALSE},
												{.command = 0x48, .commandParameterSize = 31, .hasResponse = FALSE},
												{.command = 0xAC, .commandParameterSize = 31, .hasResponse = FALSE},
												{.command = 0xAD, .commandParameterSize = 31, .hasResponse = FALSE},
												{.command = 0x00, .commandParameterSize = 255, .hasResponse = FALSE},
											};//Last line is an example of a line that should capture any CSP packet capped at 256 byte length, no other lines required - untested

static int bytesRead;
static unsigned char I2CMMcommandBuffer[32] = {0}; //Used to capture the incoming data packet in a data structure

#define TEST_SLAVE_ADR 0x5D //our slave address

//Some mode flagging constants
#define MASTER_MODE 1
#define SLAVE_MODE 0

//Flag variables used to keep track of the mode we are in and changing to
unsigned int I2CModeCurrent = SLAVE_MODE; //0 Slave, 1 Master
unsigned int I2CModeRequested = SLAVE_MODE; //0 Slave, 1 Master

xTaskHandle halSlaveHandle;


/*
 * Copied over from I2CslaveTest and adapted for use here, used for acting as the slave, we want to read the incoming data
 * Imagine a message from earth has been relayed via the AX100, it is in the CSP format so we need to capture it and provide it to
 * CSP layer.
 * BLOCKING
 */
void halSlaveTest() {
	int bytesWritten, bytesToWrite;
	unsigned int i, commandListSize = sizeof(MMCommandList) / sizeof(MMCommandList[0]);
	unsigned int commandCount = 0;
	unsigned char WriteBuffer[I2C_SLAVE_RECEIVE_BUFFER_SIZE + sizeof(unsigned int)];
	//printf("\nSlaveTest Task starts \n\r");

		// Call I2Cslave_read which will block (make this task sleep until I2C master sends a command).
		bytesRead = I2Cslave_read(I2CMMcommandBuffer);
		if(bytesRead < 0) {
			// An error occurred!
			TRACE_ERROR_WP("taskI2CslaveTest: I2Cslave_read returned %d. \n", bytesRead);
			return;
			//continue;
		}

		commandCount++;

		// Print out the received command
		//TRACE_DEBUG_WP("taskI2CslaveTest: received command and data: \n\r");
		//UTIL_DbguDumpArrayBytes(I2CMMcommandBuffer, bytesRead);
		//printf("\nAttempt to read input as string: %s\n", I2CMMcommandBuffer);

		// Check which command was received
		for(i=0; i<commandListSize; i++) {

			// Check if the command is present in the list.
			if(MMCommandList[i].command == I2CMMcommandBuffer[0]) {

				// Check if the command is supposed to have a response.
				if(MMCommandList[i].hasResponse != FALSE) {
					// The command has a response, this task does nothing special with commands
					// It takes the original command and echoes it back along with a count of the number of commands received.
					memcpy(WriteBuffer, I2CMMcommandBuffer, bytesRead);
					memcpy(WriteBuffer + bytesRead, &commandCount, sizeof(commandCount));

					bytesToWrite = bytesRead + sizeof(commandCount);

					// Call I2Cslave_write to send back the response. The function will block (make this task sleep) until the master retrieves the data.
					bytesWritten = I2Cslave_write(WriteBuffer, bytesToWrite, portMAX_DELAY);

					// Check if the I2C master retrieved all the bytes we wanted to send.
					if(bytesWritten != bytesToWrite) {
						printf("taskI2CslaveTest: I2Cslave_write returned %d. \n", bytesWritten);
					}
				}

				break;
			}
		}

		// The above loop reached commandListSize. This means the command sent by master was not in the list.
		if(i == commandListSize) {
			printf("taskI2CslaveTest: Ignored an invalid command sent by I2C master. \n");
		}
}

void initHalSlave(){

	I2C_stop();
	unsigned int commandListSize = sizeof(MMCommandList) / sizeof(MMCommandList[0]);
	int retValInt = I2Cslave_start(0x5D, MMCommandList, commandListSize);
	if(retValInt != 0) {
		TRACE_FATAL("\n\r I2CslaveTest: I2Cslave_start returned: %d! \n\r", retValInt);
	}

}


void initHalMaster(){
	TRACE_DEBUG_WP("Init Master Mode \n\r");

	I2Cslave_stop();

	int retValInt = I2C_start(100000, 1000);//2nd param can be 'portMAX_DELAY' for debug step through to prevent timeout.
	if(retValInt != 0) {
		csp_log_error("\n\r I2Ctest: I2C_start returned: %d! \n\r", retValInt);
		csp_log_info("\n\r Try to switch to slave \n\r");
		I2CModeRequested = SLAVE_MODE;
	}
	I2CModeCurrent = MASTER_MODE;
}

/*!
 * Blocking Call - we use this to do a master mode read-write to a slave for testing
 */
int mastermodeWriteReadProcess() {
	int retValInt = 0;
	unsigned int i;
	I2Ctransfer i2cTx;
	unsigned char readData[64] = {0}, writeData[64] = {0};
	//TRACE_DEBUG_WP("\n\r Master mode call Starting. \n\r");

	writeData[0] = 0x33;
	for(i=1; i<sizeof(writeData); i++) {
		writeData[i] = (unsigned char)(i*2);
	}

	unsigned char writeOut[19] = "Send me your data!\0";
	i2cTx.readData = readData;
	i2cTx.readSize = 20;
	i2cTx.writeData = writeOut;
	i2cTx.writeSize = 19;
	i2cTx.writeReadDelay = 1;
	i2cTx.slaveAddress = 0x41;// <--------------SLAVE TARGET


		TRACE_DEBUG_WP("MASTER OUPUT To 41 Sending I2CRequest to Slave via writeRead() \n\r");

		retValInt = I2C_writeRead(&i2cTx); // Use I2C_writeRead instead of our own implementation.
		if(retValInt != 0) {
			TRACE_WARNING_WP("\n\r I2C_writeRead returned with code: %d! \n\r", retValInt);
			return retValInt;
		}

		TRACE_DEBUG_WP("\n\rMessage received back: \n\r");
		TRACE_DEBUG_WP("%c", readData[0]);
		for(i=1; i<i2cTx.readSize; i++) {
			TRACE_DEBUG_WP("%c", readData[i]);
		}
		TRACE_DEBUG_WP("\n");
		TRACE_DEBUG_WP("%02X", readData[0]);
		for(i=1; i<i2cTx.readSize; i++) {
			TRACE_DEBUG_WP("%02X", readData[i]);
		}

		// Print out the last recieved slave command and data - prevents spam of debug console
		TRACE_DEBUG_WP("\n\r\n\rhalSlaveTest: LAST INPUT RECIEVED: received command and data: \n\r");
		UTIL_DbguDumpArrayBytes(I2CMMcommandBuffer, bytesRead);
		printf("\n\rAttempt to read input as string: %s \n\r", I2CMMcommandBuffer);

		TRACE_DEBUG_WP("\n\r");
		TRACE_DEBUG_WP(" \n\r\n\r");
		return retValInt;
}

/*
 * Was supposed to implement the Multi-master logical flow from the SAM9G20 manual. unfortunately this can't work with HAL drivers.
 * The Hal drivers use the status bits we need to read and so we can't reliably also use them to control behavior and switch modes.
 * This logical flow is only useful if we make our own drivers.
 *
 * Presented below is the very simplified version that uses HAL drivers, this should demonstrate mode switching
 */
void multimasterDirector(){

	while(1){

		//Check for mode switch
		if (I2CModeRequested != I2CModeCurrent){
			if(I2CModeRequested == SLAVE_MODE){
				//TRACE_DEBUG_WP("Mode Switch to slave \n\r")
				initHalSlave();
				I2CModeCurrent = SLAVE_MODE;
			}else if (I2CModeRequested == MASTER_MODE){
				//TRACE_DEBUG_WP("Mode Switch to master \n\r")
				initHalMaster();
				I2CModeCurrent = MASTER_MODE;
			}else{
				TRACE_ERROR("I2CModeRequested was set to unexpected value \n\r");
				exit(1);
			}
			taskYIELD();//May not be required, but letting background system tasks go just in-case required for the inits to take hold?, try without?
		}

		if (I2CModeCurrent == SLAVE_MODE){
			halSlaveTest();
		}else if (I2CModeCurrent == MASTER_MODE){
			mastermodeWriteReadProcess();
			I2CModeRequested = SLAVE_MODE; //Switch back to default slave mode once master message complete
		}else{
			TRACE_ERROR("I2CModeCurrent was set to unexpected value \n\r");
			exit(1);
		}

		taskYIELD();
		//csp_sleep_ms(100);

	}//Task Loop Top
}

/*
 * Currently randomly triggers a mode switch to MASTER MODE every half second we check with a 25% chance to switch
 */
void localSchedule(){
	while(1){
		if((rand() % 100) < 25){
			//TRACE_DEBUG_WP("Requesting Master-Mode \n\r");
			I2CModeRequested = MASTER_MODE;
			csp_sleep_ms(2000); //2 second break before random chance kicks back in
		}else{
			//do nothing
		}
		csp_sleep_ms(500); //every half second, 25% to do a Master Task
	}
}

/*
 * This Test may not work out of the box, I cleaned it up before adding to the repo, it may be broken. It should demo
 * Mode switching. We stay in slave mode until a separate task wishes to go to master mode.
 * localSchedule is the stand in for some other task
 * multimasterDirector acts as the mode manager and inits the appropriate driver.
 *
 */
Boolean I2CmultiMasterTest() {
	int retValInt = 0;
	xTaskHandle multimasterDirectorHandle;
	xTaskHandle localScheduleHandle;

	initHalSlave();

	//initSlave(); //<--Custom Driver init - does not work

	TRACE_DEBUG_WP("\n\r Starting MM Director \n\r");

	xTaskGenericCreate(multimasterDirector, (const signed char*)"multimasterDirector", 1024, NULL, configMAX_PRIORITIES-2, &multimasterDirectorHandle, NULL, NULL);

	TRACE_DEBUG_WP("Starting localSchedule");
	xTaskGenericCreate(localSchedule, (const signed char*)"localSchedule", 1024, NULL, configMAX_PRIORITIES-2, &localScheduleHandle, NULL, NULL);

	//Terminate (fall out) on the mainmenu task, only the director remains and the poll scheduler
	return FALSE; //No other commands may execute after this test, restart required.
}
