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


#define TEST_SLAVE_ADR 0x5D

#define MASTER_MODE 1
#define SLAVE_MODE 0

unsigned int I2CModeCurrent = SLAVE_MODE; //0 Slave, 1 Master
unsigned int I2CModeRequested = SLAVE_MODE; //0 Slave, 1 Master

// A task to read commands from the driver and responds back to them if the command is supposed to carry a response.
void readNextByte() {
	//TRACE_DEBUG_WP("readNextByte runs \n\r");
	packetReady = 0;

	AT91_REG rhr = AT91C_BASE_TWI->TWI_RHR; //1 byte of incoming data
	if(I2CInputIndex < I2C_SLAVE_PACKET_SIZE){
		I2CInput[I2CInputIndex] = rhr; //record the next byte
		bytesRead += 1; //reset when TXCOMP trips to 1
		I2CInputIndex += 1; //reset when TXCOMP trips to 1
	}else{
		TRACE_ERROR("I2CInput OVERFLOW Detected \n\r");
		bytesRead = -1;
	}
}

void readPacket(){
	TRACE_DEBUG_WP(" Reading Packet of size %d: \n\r", bytesRead);
	if(bytesRead == 0){
		TRACE_DEBUG_WP("No Data! \n\r", bytesRead);
		return;
	}
	if(bytesRead < 0) {
		// An error occurred!
		TRACE_ERROR_WP("An error occurred reading the packet, bytesRead code = %d. \n\r Dumping entire packet... \n\r", bytesRead);
		bytesRead = I2C_SLAVE_PACKET_SIZE; //Print the Packet to max limit
	}

	TRACE_DEBUG_WP("I2C Slave_read : bytesRead %d \n\r", bytesRead);
	TRACE_DEBUG_WP("Message from AX100 (DumpArray Bytes): \n\r");
	UTIL_DbguDumpArrayBytes(I2CInput, bytesRead);

	TRACE_DEBUG_WP("Character Output: %c", I2CInput[0]);
	for(int i = 1; i < bytesRead; i++) {
		TRACE_DEBUG_WP("%c", I2CInput[i]);
	}
	TRACE_DEBUG_WP("\n\r");
	TRACE_DEBUG_WP("Hex Output: %02X", I2CInput[0]);
	for(int i = 1; i < bytesRead; i++) {
		TRACE_DEBUG_WP(" %02X", I2CInput[i]);
	}
	TRACE_DEBUG_WP("\n\r READ PACKET ENDS \n\r");
	TRACE_DEBUG_WP(" \n\r\n\r");
}

void resetPacket(){
	if(packetReady){
		return;
	}
	bytesRead = 0;
	I2CInputIndex = 0; //Reset the counter for next incoming I2CInput packet array
	memset(I2CInput, 0, sizeof(I2C_SLAVE_PACKET_SIZE)); //ZERO THE PACKET BUFFER
	packetReady = 1;
}

/*
 * Basic use, no tasks used or interrupt capabilities, multimasterDirector is considered the task.
 */
void initSlave(){
	TRACE_DEBUG_WP("Init Slave Mode \n\r");

	//Halt Master Mode via HAL Driver
	I2C_stop();

	//Program Slave Address into SMR.SADR register at bits [22,16]
	//Must be done prior to activation of slave mode or write is prohibited
	AT91_REG smr = AT91C_BASE_TWI->TWI_SMR;
	smr &= ~(AT91C_TWI_SADR); //CLEAR SADR
	smr |= TEST_SLAVE_ADR << 16; //Set SADR to the slave address value

	//Deactivate mastermode MSDIS = 1 (redundant, since HAL driver should do this)
	//Enable SVEN = 1 to turn on slave mode
	AT91_REG cr = AT91C_BASE_TWI->TWI_CR;
	cr |= AT91C_TWI_MSDIS;
	cr |= AT91C_TWI_SVEN;

	//Set global flag
	I2CModeCurrent = SLAVE_MODE;
}

void initMaster(){
	TRACE_DEBUG_WP("Init Master Mode \n\r");

	//I2Cslave_stop();
	AT91_REG cr = AT91C_BASE_TWI->TWI_CR;
	cr |= AT91C_TWI_SVDIS;	//disable slave mode bit

	int retValInt = I2C_start(400000, 1000);//2nd param can be 'portMAX_DELAY' for debug step through to prevent timeout.
	if(retValInt != 0) {
		csp_log_error("\n\r I2Ctest: I2C_start returned: %d! \n\r", retValInt);
	}
	I2CModeCurrent = MASTER_MODE;
}

/*!
 * Blocking Call
 */
int mastermodeWriteReadProcess() {
	int retValInt = 0;
	unsigned int i;
	I2Ctransfer i2cTx;
	unsigned char readData[64] = {0}, writeData[64] = {0};
	TRACE_DEBUG_WP("\n\r Master mode call Starting. \n\r");

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
	i2cTx.slaveAddress = TEST_SLAVE_ADR;// <--------------SLAVE TARGET


		TRACE_DEBUG_WP("To 41 Sending I2CRequest to Slave via writeRead() \n\r");

		retValInt = I2C_writeRead(&i2cTx); // Use I2C_writeRead instead of our own implementation.
		if(retValInt != 0) {
			TRACE_WARNING_WP("\n\r I2C_writeRead returned with code: %d! \n\r", retValInt);
			return retValInt;
		}

		TRACE_DEBUG_WP(" Message received back: \n\r");
		TRACE_DEBUG_WP("%c", readData[0]);
		for(i=1; i<i2cTx.readSize; i++) {
			TRACE_DEBUG_WP("%c", readData[i]);
		}
		TRACE_DEBUG_WP("\n");
		TRACE_DEBUG_WP("%02X", readData[0]);
		for(i=1; i<i2cTx.readSize; i++) {
			TRACE_DEBUG_WP("%02X", readData[i]);
		}
		TRACE_DEBUG_WP("\n\r");
		TRACE_DEBUG_WP(" \n\r\n\r");
		return retValInt;
}

void multimasterDirector(){

	while(1){

		//Check for mode switch
		if (I2CModeRequested != I2CModeCurrent){
			if(I2CModeRequested == SLAVE_MODE){
				TRACE_DEBUG_WP("Mode Switch to slave \n\r")
				initSlave();
			}else if (I2CModeRequested == MASTER_MODE){
				TRACE_DEBUG_WP("Mode Switch to master \n\r")
				initMaster();
			}else{
				TRACE_ERROR("I2CModeRequested was set to unexpected value \n\r");
				exit(1);
			}
			taskYIELD();//May not be required, but letting system tasks go just in-case required for the inits to take?, try without?
		}

		//Follow Flowchart - REFERENCE SAM8g20.pdf (pg. 423)
		if (I2CModeCurrent == SLAVE_MODE){

			while(1){ //THE SLAVE FLOW LOOP
				//READ STATUS REGISTER
				status_register = AT91C_BASE_TWI->TWI_SR;
				//CHECK SVACC
				if (status_register & AT91C_TWI_SVACC){
					if(bytesRead == 0){
						TRACE_DEBUG_WP("SVACC Received: Status Register: %X \n\r", status_register);
					}
					//Check GACC
					if (status_register & AT91C_TWI_GACC){
						TRACE_ERROR("GENERAL CALL NOT SUPPORTED \n\r");
						exit(1);
					}else{//GACC == 0
						if ((status_register & AT91C_TWI_SVREAD) == 0){
							if ((status_register & AT91C_TWI_RXRDY) == 0){
								//READ TWI_RHR byte by byte
								readNextByte(); //Loop until NACK from master and TXCOMP goes to 1
							}else{
								//loop out
							}
						}else{
							//loop out
						}
					}
				}else{//SVACC == 0, or NO on flow
					//CHECK EOSACC - End of Slave Access bit
					//TRACE_DEBUG_WP("EOSACC %X \n\r", (status_register & AT91C_TWI_EOSACC));
					if(status_register & AT91C_TWI_EOSACC){
						//CHECK TXCOMP - verify transmission is complete before allow a mode switch
						//TRACE_DEBUG_WP("TXCOMP %X \n\r", (status_register & AT91C_TWI_TXCOMP));
						if(status_register & AT91C_TWI_TXCOMP){
							readPacket();
							resetPacket();

							//Another task sets up the I2CModeRequested to = MASTER_MODE to indicate a master communication is requested - TODO Data is moved via task queue or global struct?
							break; //Return to outer loop and check for mode switch - we continuously loop from outer to here if nothing to do //EXIT SLAVE LOOP
						}
						//continue
					}
					//continue
				}

				//GOTO READ STATUS REGISTER - SLAVE LOOP ENDS
				taskYIELD();
				//csp_sleep_ms(1);
			}//SLAVE LOOP
		}else if (I2CModeCurrent == MASTER_MODE){
			while(1){//THE MASTER MODE FLOW LOOP - loop not required as we are using the HAL driver
				status_register = AT91C_BASE_TWI->TWI_SR;
				//CHECK ARBLST
				if (status_register & AT91C_TWI_ARBLST_MULTI_MASTER){
					TRACE_DEBUG_WP("ARBLST Seen: Status Register: %X \n\r", status_register);
					I2CModeRequested = SLAVE_MODE; //GO DIRECTLY TO SLAVE MODE - Arbitration was lost, a MASTER is attempting to call, need to listen!
					break; //Return to outer loop and check for mode switch //EXIT MASTER LOOP
				}else{//ARBLST == 0, or NO in the flow
					//DO MASTER MODE TEST-READWRITE
					int rtnVal = mastermodeWriteReadProcess();
					if(rtnVal){
						//TODO Blocking Call - MAY NOT CONSIDER THE ARBITRATION LOST BIT! Master driver may be required to consider the arbitration bit
						//TODO Currently Anticipate a code 7 if arbitration is lost during a HAL Master command, this is ok since we will go to slave mode regardless

						TRACE_ERROR("Master Read-Write returns code: %d \n\r", rtnVal);

						//Check the I2C Transfer Status
						switch (I2C_getCurrentTransferStatus()) {
							case writeError_i2c:
								TRACE_ERROR("I2C Status shows: writeError_i2c \n\r");
								break;
							case readError_i2c:
								TRACE_ERROR("I2C Status shows: readError_i2c \n\r");
								break;
							case timeoutError_i2c:
								TRACE_ERROR("I2C Status shows: timeoutError_i2c \n\r");
								break;
							case error_i2c:
								TRACE_ERROR("I2C Status shows: General Error 7! \n\r");
								break;
							default:
								//nominal
								break;
						}
					}
					I2CModeRequested = SLAVE_MODE;
					break; //EXIT MASTER LOOP
				}
				//CANT REACH THIS AREA
			}//MASTER LOOP
		}else{
			TRACE_ERROR("I2CModeCurrent was set to unexpected value \n\r");
			exit(1);
		}

		taskYIELD();
		//csp_sleep_ms(1);

	}//Task Loop Top
}

void localSchedule(){
	//Design this task to change mode to MASTER (I2CModeRequested == MASTER_MODE) when it wants to talk to the slave
	//How it passes data is a Dante problem
	while(1){
		//TODO
		if((rand() % 100) < 25){
			TRACE_DEBUG_WP("Requesting Master-Mode \n\r");
			I2CModeRequested = MASTER_MODE;
			csp_sleep_ms(1000);
		}else{
			//do nothing
		}
		csp_sleep_ms(500); //every half second, 25% to do a Master Task
	}

}

Boolean I2CmultiMasterTest() {
	int retValInt = 0;
	xTaskHandle multimasterDirectorHandle;
	xTaskHandle localScheduleHandle;

	initSlave();

	TRACE_DEBUG_WP("\n\r Starting MM Director \n\r");

	xTaskGenericCreate(multimasterDirector, (const signed char*)"multimasterDirector", 1024, NULL, configMAX_PRIORITIES-2, &multimasterDirectorHandle, NULL, NULL);

	//TRACE_DEBUG_WP("Starting localSchedule");
	//xTaskGenericCreate(localSchedule, (const signed char*)"localSchedule", 1024, NULL, configMAX_PRIORITIES-2, &localScheduleHandle, NULL, NULL);

	//Terminate (fall out) on the mainmenu task, only the director remains and the poll scheduler
	return FALSE; //No other commands may execute after this test, reboot required.
}
