/*
 * SPITest.c
 *
 *  Created on: Mar. 17, 2022
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

#define TEST_RTC	0

#define FRAM_TEST_TRANSACTION_SIZE	1024

static unsigned char FRAMreadData[FRAM_TEST_TRANSACTION_SIZE] = {0};
static unsigned char FRAMwriteData[FRAM_TEST_TRANSACTION_SIZE] = {0};
static unsigned char FRAMwriteVerifyData[FRAM_TEST_TRANSACTION_SIZE] = {0};
static Boolean FRAMtestOnce = FALSE;

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

void FRAMtest() {
	unsigned int i = 0, j = 0;
	FRAMblockProtect blocks;
	int retVal;
	unsigned char deviceID[9] = {0};
	unsigned int size = FRAM_TEST_TRANSACTION_SIZE;
	unsigned int address = 0x10000;

	TRACE_DEBUG(" Starting FRAM test \n\r");
	retVal = FRAM_start();
	if(retVal != 0) {
		TRACE_WARNING(" Error during FRAM_start: %d \n\r", retVal);
		while(1);
	}


	{
		retVal = FRAM_getDeviceID(deviceID);
		if(retVal != 0) {
			TRACE_WARNING(" Error during FRAM_protectBlocks: %d \n\r", retVal);
			while(1);

		}
		TRACE_DEBUG_WP("Device ID: ");
		for(i=0; i<sizeof(deviceID); i++) {
			TRACE_DEBUG_WP("0x%02X ", deviceID[i]);
		}
		TRACE_DEBUG_WP("\n\r");
		vTaskDelay(500);
	}

	// Unprotect all blocks
	blocks.fields.blockProtect = 0;
	retVal = FRAM_protectBlocks(blocks);
	if(retVal != 0) {
		TRACE_WARNING(" Error during FRAM_protectBlocks: %d \n\r", retVal);
		while(1);
	}

	// TODO: Test block-protection!  <-- WUT?

	while(1) {
		retVal = FRAM_read(FRAMreadData, address, size);
		if(retVal != 0) {
			TRACE_WARNING(" Error during FRAM_read: %d \n\r", retVal);
			while(1);
		}

		TRACE_DEBUG_WP("\n\r FRAM readPacket contents: \n\r");
		for(i=0; i<size; i++) {
			TRACE_DEBUG_WP("0x%02X, ", FRAMreadData[i]);
			FRAMwriteData[i] = i*2 + j;
			FRAMwriteVerifyData[i] = FRAMwriteData[i] + 1;
			FRAMreadData[i] = 0;
		}
		TRACE_DEBUG_WP(" \n\r");

		retVal = FRAM_write(FRAMwriteData, address, size);
		if(retVal != 0) {
			TRACE_WARNING(" Error during FRAM_write: %d \n\r", retVal);
			while(1);
		}

		retVal = FRAM_writeAndVerify(FRAMwriteVerifyData, address, size);
		if(retVal != 0) {
			TRACE_WARNING(" Error during FRAM_writeAndVerify: %d \n\r", retVal);
		}

		j++;

		if(FRAMtestOnce == TRUE) {
			break;
		}

		vTaskDelay(1000);
	}

}

void RTCtest() {

	// Tests the 'Real Time Clock' (RTC)
	// The entire SPI bus must be inactive before this test, otherwise the clock may misbehave

	int retVal;
	unsigned int i;
	float temperature;
	Time time;

	TRACE_DEBUG(" Starting RTC test \n\r");

	RTC_start();

	while(1) {
		retVal = RTC_testGetSet(); // This will set the seconds value to 7, amongst others  <-- Useless comment
		if(retVal != 0) {
			TRACE_WARNING("RTC_testGetSet returned: %d \n\r", retVal); // Only if SPI does not initialize
		}

		for(i=0; i<5; i++) { // Should print 7-11 seconds
			retVal = RTC_getTime(&time); // Checks the time, if fail print error, otherwise prints the time in seconds
			if(retVal != 0) {
				TRACE_WARNING("RTC_getTime returned: %d \n\r", retVal);
			}
			else {
				TRACE_DEBUG_WP("seconds: %d \n\r", time.seconds); // As the time is initially set to 7, it should repeat 7, 8, 9, 10 ,11 seconds
			}
			vTaskDelay(800);
		}

		retVal = RTC_getTemperature(&temperature); // 0 on success, -1 on SPI Fail
		if(retVal != 0) {
			TRACE_WARNING("RTC_getTemp returned: %d \n\r", retVal);
		}
		else {
			TRACE_DEBUG_WP("RTC temperature: %d.", (int)temperature);  // output Clock temp
			if(temperature < 0) {
				temperature = temperature * (-1); // Bug Catching?  -ve temp may very well happen on orbit so we should check this
			}
			temperature = temperature - (unsigned int)temperature; // Pardon? Why?
			TRACE_DEBUG_WP("%d \n\r", (unsigned int)(temperature*100)); // Prints temp again?
		}


	}
}

Boolean SPITest() {
	int retValInt = 0;

	#if TEST_RTC
		xTaskHandle taskRTCtestHandle;
	#else
		//xTaskHandle FRAMtestHandle;
		xTaskHandle SPItest1Handle;
	#endif

	retValInt = SPI_start(bus1_spi, slave7_spi); //Returns 0 on success -1 on fail, buses 0, 1, 2 initialized by default
	if(retValInt != 0) {
		TRACE_WARNING("\n\r SPITest: SPI_start returned %d! \n\r", retValInt);
		while(1);
	}


		//Below, if TEST_RTC is 1, it performs an FRAM test, and RTC Test
		//Otherwise, run SPItest1, SPItest2, FRAMtest


	#if TEST_RTC
		// Perform a basic FRAM test and then test the RTC
		xTaskGenericCreate(RTCtest, (const signed char*)"RTCtest", 10240, NULL, 2, &taskRTCtestHandle, NULL, NULL);
	#else
		xTaskGenericCreate(SPItest1, (const signed char*)"SPItest1", 10240, NULL, 2, &SPItest1Handle, NULL, NULL);
		//xTaskGenericCreate(SPItest2, (const signed char*)"SPItest2", 10240, NULL, 2, &SPItest2Handle, NULL, NULL);
		//xTaskGenericCreate(FRAMtest, (const signed char*)"FRAMtest", 10240, NULL, 2, &FRAMtestHandle, NULL, NULL);
	#endif

	return FALSE;
}


