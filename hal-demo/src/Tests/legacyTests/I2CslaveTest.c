/*
 * I2CslaveTest.c
 *
 *  Created on: 11-Mar-2014
 *      Author: apip
 */
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include <at91/commons.h>
#include <at91/utility/trace.h>

#include <hal/Drivers/LED.h>
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

// A task to read commands from the driver and responds back to them if the command is supposed to carry a response.
void taskI2CslaveTest() {
	int bytesRead, bytesWritten, bytesToWrite;
	unsigned int i, commandListSize = sizeof(CommandList) / sizeof(CommandList[0]);
	unsigned int commandCount = 0;
	unsigned char WriteBuffer[I2C_SLAVE_RECEIVE_BUFFER_SIZE + sizeof(unsigned int)];
	printf("SlaveTest Task starts \n\r");
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
			printf("taskI2CslaveTest: Ignored an invalid command sent by I2C master. \n");
		}

	}

}

Boolean I2CslaveTest() {
	int retValInt = 0;
	xTaskHandle taskI2CslaveTestHandle;
	unsigned int commandListSize = sizeof(CommandList) / sizeof(CommandList[0]);

	retValInt = I2Cslave_start(0x5D, CommandList, commandListSize);
	if(retValInt != 0) {
		TRACE_FATAL("\n\r I2CslaveTest: I2Cslave_start returned: %d! \n\r", retValInt);
	}
	printf("\n\r I2C Slave Test Make Task \n \r");
	xTaskGenericCreate(taskI2CslaveTest, (const signed char*)"taskI2CslaveTest", 1024, NULL, configMAX_PRIORITIES-2, &taskI2CslaveTestHandle, NULL, NULL);
	//taskI2CslaveTest();

	return FALSE;
}
