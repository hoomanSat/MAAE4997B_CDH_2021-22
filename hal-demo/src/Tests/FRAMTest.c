/*
 * FRAMTest.c
 *
 *  Created on: Mar. 24, 2022
 *      Author: Sam Dunthorne
 */

// Tests the FRAM on the internal SPI Bus

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


#define FRAM_TEST_TRANSACTION_SIZE	1024

static unsigned char FRAMreadData[FRAM_TEST_TRANSACTION_SIZE] = {0};
static unsigned char FRAMwriteData[FRAM_TEST_TRANSACTION_SIZE] = {0};
static unsigned char FRAMwriteVerifyData[FRAM_TEST_TRANSACTION_SIZE] = {0};
static Boolean FRAMtestOnce = FALSE;

void FRAMTestTask() {
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


Boolean FRAMTest() {
	int retValInt = 0;

	xTaskHandle FRAMTestHandle;

	retValInt = SPI_start(bus0_spi, slave2_spi); // Turns on the internal SPI bus if not already initialized
	if(retValInt != 0) {
		TRACE_WARNING("\n\r SPITest: SPI_start returned %d! \n\r", retValInt);
		while(1);
	}

	xTaskGenericCreate(FRAMTestTask, (const signed char*)"FRAMTestTask", 10240, NULL, 2, &FRAMTestHandle, NULL, NULL);

	return FALSE;
}
