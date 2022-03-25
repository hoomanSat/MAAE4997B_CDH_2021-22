/*
 * SPITest.c
 *
 *  Created on: Mar. 24, 2022
 *      Author: Sam Dunthorne
 */

// SPI Pinout is on J5 pin GND = 1, CS0 = 12, CS1 = 13, CS2 = 14,
//MOSI = 17, MISO = 18, CLK = 16

#include <at91/commons.h>
#include <at91/utility/trace.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>
#include <freertos/projdefs.h>

#include <hal/interruptPriorities.h>
#include <hal/boolean.h>
#include <hal/Drivers/SPI.h>
#include <hal/Storage/FRAM.h>
#include <hal/Timing/RTC.h>
#include <hal/Utility/util.h>

#include <string.h>
#include <stdio.h>


void SPI_Callback(SystemContext context, xSemaphoreHandle semaphore) {
	printf("Callback Received - Data Should Have Transferred \n\r");
	signed portBASE_TYPE flag = pdFALSE;

	if(context == task_context) {
		xSemaphoreGive(semaphore);
	}
	else {
		xSemaphoreGiveFromISR(semaphore, &flag);
	}
}

void SPItest1() {
	int retValInt = 0;
	unsigned int i, j=0;
	SPIslaveParameters slaveParams;
	SPItransfer spiTransfer;
	xSemaphoreHandle txSemaphore = NULL;
	unsigned char readData[32] = {0}, writeData[32] = {0}, writeData2[32] = {0};
	TRACE_DEBUG("\n\r SPItest1: Starting. \n\r");

	writeData[0] = 0xEF;
	for(i=1; i<sizeof(writeData); i++) {
		writeData[i] = (unsigned char)(i*2);
		writeData2[i] = (unsigned char)(i*4);
	}

	vSemaphoreCreateBinary(txSemaphore);
	if(txSemaphore == NULL) {
		TRACE_WARNING("\n\r taskSPItest1: vSemaphoreCreateBinary failed! \n\r");
		while(1);
	}

	slaveParams.bus    = bus1_spi;
	slaveParams.mode   = mode0_spi;
	slaveParams.slave  = slave0_spi;
	slaveParams.dlybs  = 0;
	slaveParams.dlybct = 0;
	slaveParams.busSpeed_Hz = 600000;
	slaveParams.postTransferDelay = 4;

	spiTransfer.slaveParams = &slaveParams;
	spiTransfer.callback  = SPI_Callback;
	spiTransfer.readData  = readData;
	spiTransfer.writeData = writeData;
	spiTransfer.transferSize = 10;
	spiTransfer.semaphore  = txSemaphore;

	//while(1)
	{
		if(j%2 == 0) {
			spiTransfer.writeData = writeData;
		}
		else {
			spiTransfer.writeData = writeData2;
		}
		j++;

		if(xSemaphoreTake(txSemaphore, (portTickType)10) != pdTRUE) {
			TRACE_WARNING("\n\r taskSPItest1: xSemaphoreTake failed! \n\r");
			while(1);
		}
		retValInt = SPI_queueTransfer(&spiTransfer);
		if(retValInt != 0) {
			TRACE_WARNING("\n\r taskSPItest1: SPI_queueTransfer returned: %d! \n\r", retValInt);
			while(1);
		}

		else {

			// Make use of the transfer-time: Prepare the other writeBuffer while the transfer is in progress.
			if(j%2 == 0) {
				for(i=1; i<sizeof(writeData2); i++) {
					writeData2[i]++;
				}
			}
			else {
				for(i=1; i<sizeof(writeData); i++) {
					writeData[i]++;
				}
			}

			// Block on the semaphore waiting for the transfer to finish
			if(xSemaphoreTake(txSemaphore, (portTickType)portMAX_DELAY) != pdTRUE) {
				TRACE_WARNING("\n\r taskSPItest1: xSemaphoreTake failed! \n\r");
				while(1);
			}
			xSemaphoreGive(txSemaphore);
		}

		//TRACE_DEBUG(" SPItest1: received back: \n\r");
		//TRACE_DEBUG("0x%X ", readData[0]);
		for(i=1; i<spiTransfer.transferSize; i++) {
			//TRACE_DEBUG("0x%X ", readData[i]);
		}

		//TRACE_DEBUG(" \n\r\n\r");
		vTaskDelay(5);
	}
}

void SPItest2() {
	int retValInt = 0;
	unsigned int i;
	SPIslaveParameters slaveParams;
	SPItransfer spiTransfer;
	unsigned char readData[64] = {0}, writeData[64] = {0};
	TRACE_DEBUG("\n\r taskSPItest2: Starting. \n\r");

	writeData[0] = 0xEF;
	for(i=1; i<sizeof(writeData); i++) {
		writeData[i] = (unsigned char)(i);
		readData[i] = 0xEF;
	}

	slaveParams.bus    = bus1_spi;
	slaveParams.mode   = mode0_spi;
	slaveParams.slave  = slave0_spi;
	slaveParams.dlybs  = 1;
	slaveParams.dlybct = 1;
	slaveParams.busSpeed_Hz = 600000;
	slaveParams.postTransferDelay = 0;

	spiTransfer.slaveParams = &slaveParams;
	spiTransfer.callback  = SPI_Callback;
	spiTransfer.readData  = readData;
	spiTransfer.writeData = writeData;
	spiTransfer.transferSize = 10;

	while(1) {

		retValInt = SPI_writeRead(&spiTransfer);
		if(retValInt != 0) {
			TRACE_WARNING("\n\r taskSPItest2: SPI_queueTransfer returned: %d! \n\r", retValInt);
			while(1);
		}

		//TRACE_DEBUG(" taskSPItest2: received back: \n\r");
		//TRACE_DEBUG("0x%X ", readData[0]);
		for(i=1; i<spiTransfer.transferSize; i++) {
			//TRACE_DEBUG("0x%X ", readData[i]);
			writeData[i]++;
			readData[i] = 0xEF;
		}
		writeData[i]++;

		//TRACE_DEBUG(" \n\r\n\r");
		vTaskDelay(5);
	}
}

Boolean SPITest() {
	int retValInt = 0;

	xTaskHandle SPItest1Handle;

	retValInt = SPI_start(both_spi, slave7_spi); // Turns on both SPI buses if not already, and initializes all possible slaves
	if(retValInt != 0) {
		TRACE_WARNING("\n\r SPITest: SPI_start returned %d! \n\r", retValInt);
		while(1);
	}

	xTaskGenericCreate(SPItest1, (const signed char*)"SPItest1", 10240, NULL, 2, &SPItest1Handle, NULL, NULL);
	//xTaskGenericCreate(SPItest2, (const signed char*)"SPItest2", 10240, NULL, 2, &SPItest2Handle, NULL, NULL);


	return FALSE;
}
