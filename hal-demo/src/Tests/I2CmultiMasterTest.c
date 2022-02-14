/*
 * I2CmultiMasterTest.c
 *
 */
#include <Tests/I2CmultiMasterTest.h>

// A list of commands supported by this slave. - Required to init slave mode
static I2CslaveCommandList CommandList[] = {	{.command = 0xAA, .commandParameterSize = 8, .hasResponse = TRUE},
												{.command = 0xAD, .commandParameterSize = 8, .hasResponse = FALSE},
												{.command = 0xA0, .commandParameterSize = 1, .hasResponse = TRUE},
												{.command = 0x0A, .commandParameterSize = 1, .hasResponse = FALSE},
											};


static unsigned char I2CInput[I2C_SLAVE_RECEIVE_BUFFER_SIZE] = {0};

#define TEST_SLAVE_ADR 0x5D

#define MASTER_MODE 1
#define SLAVE_MODE 0

unsigned int I2CModeCurrent = SLAVE_MODE; //0 Slave, 1 Master
unsigned int I2CModeRequested = SLAVE_MODE; //0 Slave, 1 Master

// A task to read commands from the driver and responds back to them if the command is supposed to carry a response.
void slaveTestProcess() {
	int bytesRead;
	TRACE_DEBUG_WP("I2C Slave_read runs \n\r");
	// Call I2Cslave_read which will block (make this task sleep until I2C master sends a command).
	bytesRead = I2Cslave_read(I2CInput);
	if(bytesRead < 0) {
		// An error occurred!
		TRACE_ERROR_WP("taskI2CslaveTest: I2Cslave_read returned %d. \n", bytesRead);
	}

	TRACE_DEBUG_WP("I2C Slave_read : bytesRead %d \n\r", bytesRead);
	TRACE_DEBUG_WP("Message from AX100 received: \n\r");
	UTIL_DbguDumpArrayBytes(I2CInput, bytesRead);

	TRACE_DEBUG_WP("%c", I2CInput[0]);
	for(int i = 1; i < bytesRead; i++) {
		TRACE_DEBUG_WP("%c", I2CInput[i]);
	}
	TRACE_DEBUG_WP("\n");
	TRACE_DEBUG_WP("%02X", I2CInput[0]);
	for(int i = 1; i < bytesRead; i++) {
		TRACE_DEBUG_WP("%02X", I2CInput[i]);
	}
	TRACE_DEBUG_WP("\n\r");
	TRACE_DEBUG_WP(" \n\r\n\r");

}

void mastermodeWriteReadProcess() {
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
			return;
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

		csp_sleep_ms(5);
}

void initSlave(){
	unsigned int commandListSize = sizeof(CommandList) / sizeof(CommandList[0]);
	TRACE_DEBUG_WP("Init SlaveMode \n\r");
	I2C_stop();
	int retValInt = I2Cslave_start(TEST_SLAVE_ADR, CommandList, commandListSize);
	if(retValInt != 0) {
		csp_log_error("\n\r I2CslaveTest: I2Cslave_start returned: %d! \n\r", retValInt);
	}
	I2CModeCurrent = SLAVE_MODE;
}

void initMaster(){
	TRACE_DEBUG_WP("Init Master Mode \n\r");
	I2Cslave_stop();
	int retValInt = I2C_start(400000, 1000);//2nd param can be 'portMAX_DELAY' for debug step through to prevent timeout.
	if(retValInt != 0) {
		csp_log_error("\n\r I2Ctest: I2C_start returned: %d! \n\r", retValInt);
	}
	I2CModeCurrent = MASTER_MODE;
}

void multimasterDirector(){
	AT91_REG status_register;

	while(1){

		//Check for mode switch
		if (I2CModeRequested != I2CModeCurrent){
			if(I2CModeRequested == SLAVE_MODE){
				TRACE_DEBUG_WP("Mode Switch to slave \n\r")
				initSlave();
			}else if (I2CModeRequested == MASTER_MODE){
				TRACE_DEBUG_WP("Mode Switch to slave master \n\r")
				initMaster();
			}else{
				TRACE_ERROR("I2CModeRequested was set to unexpected value \n\r");
				exit(1);
			}
		}

		//Follow Flowchart - REFERENCE SAM8g20.pdf (pg. 423)
		if (I2CModeCurrent == SLAVE_MODE){
			while(1){ //THE SLAVE FLOW LOOP
				//slaveTestProcess();

				//READ STATUS REGISTER
				status_register = AT91C_BASE_TWI->TWI_SR;
				//TRACE_DEBUG_WP("Status Reg %X \n\r", status_register);
				//CHECK SVACC
				//TRACE_DEBUG_WP("SVACC %X \n\r", (status_register & AT91C_TWI_SVACC));
				if ((status_register & AT91C_TWI_SVACC) == AT91C_TWI_SVACC){
					//Check GACC
					//TRACE_DEBUG_WP("GACC %X \n\r", (status_register & AT91C_TWI_GACC));
					if ((status_register & AT91C_TWI_GACC) == AT91C_TWI_GACC){
						TRACE_ERROR("GENERAL CALL NOT SUPPORTED \n\r");
						exit(1);
					}else{//GACC == 0
						if ((status_register & AT91C_TWI_SVREAD) == 0){
							if ((status_register & AT91C_TWI_RXRDY) == 0){
								//READ TWI_RHR - Let the hal driver do this
								slaveTestProcess();
							}else{
								//loop out
							}
						}else{
							//loop out
						}
					}
				}else{//SVACC == 0, or NO on flow
					//CHECK EOSACC
					//TRACE_DEBUG_WP("EOSACC %X \n\r", (status_register & AT91C_TWI_EOSACC));
					if((status_register & AT91C_TWI_EOSACC) == AT91C_TWI_EOSACC){
						//CHECK TXCOMP
						//TRACE_DEBUG_WP("TXCOMP %X \n\r", (status_register & AT91C_TWI_TXCOMP));
						if((status_register & AT91C_TWI_TXCOMP) == AT91C_TWI_TXCOMP){
							//Another task sets up the I2CModeRequested to = MASTER_MODE to indicate a master communication is requested - TODO Data is moved via task queue or global?
							break; //Return to outer loop and check for mode switch
						}
						//continue
					}
					//continue
				}

				if (I2CModeRequested == MASTER_MODE){
					break;
				}
				//GOTO READ STATUS REGISTER
				csp_sleep_ms(5);
			}
		}else if (I2CModeCurrent == MASTER_MODE){
			while(1){//THE MASTER MODE FLOW LOOP - loop required if multiple MASTER tasks are enqueued, shall bypass the mode switch aspect
				status_register = AT91C_BASE_TWI->TWI_SR;
				//CHECK ARBLST
				if ((status_register & AT91C_TWI_ARBLST_MULTI_MASTER) == 1){
					I2CModeRequested = SLAVE_MODE; //GO DIRECTLY TO SLAVE MODE - Arbitration was lost, a MASTER is attempting to call, need to listen!
					break; //Return to outer loop and check for mode switch
				}else{//ARBLST == 0, or NO in the flow
					//DO MASTER MODE TEST-READWRITE
					mastermodeWriteReadProcess();
					I2CModeRequested = SLAVE_MODE;
					break;
				}

			}
		}else{
			TRACE_ERROR("I2CModeCurrent was set to unexpected value \n\r");
			exit(1);
		}

		csp_sleep_ms(5);
	}
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
		csp_sleep_ms(5);
	}

}


Boolean I2CmultiMasterTest() {
	int retValInt = 0;
	xTaskHandle multimasterDirectorHandle;
	xTaskHandle localScheduleHandle;

	unsigned int commandListSize = sizeof(CommandList) / sizeof(CommandList[0]);

	retValInt = I2Cslave_start(TEST_SLAVE_ADR, CommandList, commandListSize);
	if(retValInt != 0) {
		TRACE_FATAL("\n\r I2CslaveTest: I2Cslave_start returned: %d! \n\r", retValInt);
	}
	TRACE_DEBUG_WP("\n\r Starting MM Director \n\r");

	xTaskGenericCreate(multimasterDirector, (const signed char*)"multimasterDirector", 1024, NULL, configMAX_PRIORITIES-2, &multimasterDirectorHandle, NULL, NULL);

	TRACE_DEBUG_WP("Starting localSchedule");
	xTaskGenericCreate(localSchedule, (const signed char*)"localSchedule", 1024, NULL, configMAX_PRIORITIES-2, &localScheduleHandle, NULL, NULL);

	//Terminate (fall out) on the mainmenu task, only the director remains and the poll scheduler
	return FALSE; //No other commands may execute after this test, reboot required.
}
