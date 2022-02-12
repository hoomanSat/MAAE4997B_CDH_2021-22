/*
 * I2CmultiMasterTest.c
 *
 */
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include <at91/commons.h>
#include <at91/utility/trace.h>
#include <at91/boards/ISIS_OBC_G20/at91sam9g20/AT91SAM9G20.H>

#include <hal/Drivers/LED.h>
#include <hal/Drivers/I2C.h>
#include <hal/boolean.h>
#include <hal/Drivers/I2Cslave.h>
#include <hal/Utility/util.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// A list of commands supported by this slave.
static I2CslaveCommandList CommandList[] = {	{.command = 0xAA, .commandParameterSize = 8, .hasResponse = TRUE},
												{.command = 0xAD, .commandParameterSize = 8, .hasResponse = FALSE},
												{.command = 0xA0, .commandParameterSize = 1, .hasResponse = TRUE},
												{.command = 0x0A, .commandParameterSize = 1, .hasResponse = FALSE},
											};

static unsigned char I2CcommandBuffer[I2C_SLAVE_RECEIVE_BUFFER_SIZE] = {0};

#define MASTER_MODE 1
#define SLAVE_MODE 0

static unsigned int I2CModeCurrent = SLAVE_MODE; //0 Slave, 1 Master
static unsigned int I2CModeRequested = SLAVE_MODE; //0 Slave, 1 Master

// A task to read commands from the driver and responds back to them if the command is supposed to carry a response.
void slaveTestProcess() {
	int bytesRead, bytesWritten, bytesToWrite;
	unsigned int i, commandListSize = sizeof(CommandList) / sizeof(CommandList[0]);
	unsigned int commandCount = 0;
	unsigned char WriteBuffer[I2C_SLAVE_RECEIVE_BUFFER_SIZE + sizeof(unsigned int)];

	while(1) {
		// Call I2Cslave_read which will block (make this task sleep until I2C master sends a command).
		bytesRead = I2Cslave_read(I2CcommandBuffer);
		if(bytesRead < 0) {
			// An error occurred!
			TRACE_ERROR_WP("taskI2CslaveTest: I2Cslave_read returned %d. \n", bytesRead);
			continue;
		}

		commandCount++;

		// Print out the received command
		TRACE_DEBUG_WP("taskI2CslaveTest: received command: \n\r");
		UTIL_DbguDumpArrayBytes(I2CcommandBuffer, bytesRead);

		// Check which command was received
		for(i=0; i<commandListSize; i++) {

			// Check if the command is present in the list.
			if(CommandList[i].command == I2CcommandBuffer[0]) {

				// Check if the command is supposed to have a response.
				if(CommandList[i].hasResponse != FALSE) {
					// The command has a response, this task does nothing special with commands
					// It takes the original command and echoes it back along with a count of the number of commands received.
					memcpy(WriteBuffer, I2CcommandBuffer, bytesRead);
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
			csp_log_warn("taskI2CslaveTest: Ignored an invalid command sent by I2C master. \n");
		}
	}
}

void mastermodeWriteReadProcess() {
	int retValInt = 0;
	unsigned int i;
	I2Ctransfer i2cTx;
	unsigned char readData[64] = {0}, writeData[64] = {0};
	TRACE_DEBUG_WP("\n\r taskQueuedI2Ctest3: Starting. \n\r");

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


		TRACE_DEBUG_WP("To 41 Sending I2CTransfer via writeRead() \n\r");

		retValInt = I2C_writeRead(&i2cTx); // Use I2C_writeRead instead of our own implementation.
		if(retValInt != 0) {
			TRACE_WARNING_WP("\n\r taskQueuedI2Ctest3: I2C_writeRead returned with code: %d! \n\r", retValInt);
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
	I2C_stop();
	int retValInt = I2Cslave_start(0x00, CommandList, commandListSize);
	if(retValInt != 0) {
		csp_log_error("\n\r I2CslaveTest: I2Cslave_start returned: %d! \n\r", retValInt);
	}
	I2CModeCurrent = SLAVE_MODE;
}

void initMaster(){
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
				initSlave();
			}else if (I2CModeRequested == MASTER_MODE){
				initMaster();
			}else{
				csp_log_error("I2CModeRequested was set to unexpected value");
				exit(1);
			}
		}
		//Follow Flowchart - REFERENCE SAM8g20.pdf (pg. 423)
		if (I2CModeCurrent == SLAVE_MODE){
			while(1){ //THE SLAVE FLOW LOOP
				//READ STATUS REGISTER
				status_register = AT91C_BASE_TWI->TWI_SR;
				//CHECK SVACC
				if ((status_register & AT91C_TWI_SVACC) == 1){
					//Check GACC
					if ((status_register & AT91C_TWI_GACC) == 1){
						csp_log_error("GENERAL CALL NOT SUPPORTED");
						exit(1);
					}else{//GACC == 0
						if ((status_register & AT91C_TWI_SVREAD) == 0){
							if ((status_register & AT91C_TWI_RXRDY) == 0){
								//READ TWI_RHR - Let the hal driver do this
								slaveTestProcess();
							}else{

							}
						}else{

						}
					}
				}else{//SVACC == 0, or NO on flow
					//CHECK EOSACC
					if((status_register & AT91C_TWI_EOSACC) == 1){
						//CHECK TXCOMP
						if((status_register & AT91C_TWI_TXCOMP) == 1){
							//Another task sets up the I2CModeRequested to = MASTER_MODE to indicate a master communication is requested - TODO Data is moved via task queue or global?
							break; //Return to outer loop and check for mode switch
						}
						//continue
					}
					//continue
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
					mastermodeWriteReadTest();
					break;
				}

			}
		}else{
			csp_log_error("I2CModeCurrent was set to unexpected value");
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
	}
	csp_sleep_ms(5);
}


Boolean I2CmultiMasterTest() {
	int retValInt = 0;
	xTaskHandle multimasterDirector;
	xTaskHandle localSchedule;

	unsigned int commandListSize = sizeof(CommandList) / sizeof(CommandList[0]);

	retValInt = I2Cslave_start(0x5D, CommandList, commandListSize);
	if(retValInt != 0) {
		TRACE_FATAL("\n\r I2CslaveTest: I2Cslave_start returned: %d! \n\r", retValInt);
	}

	xTaskGenericCreate(multimasterDirector, (const signed char*)"multimasterDirector", 1024, NULL, configMAX_PRIORITIES-2, &multimasterDirector, NULL, NULL);
	xTaskGenericCreate(localSchedule, (const signed char*)"localSchedule", 1024, NULL, configMAX_PRIORITIES-2, &localSchedule, NULL, NULL);

	//Terminate (fall out) on the mainmenu task, only the director remains and the poll scheduler
	return FALSE; //No other commands may execute after this test, reboot required.
}
