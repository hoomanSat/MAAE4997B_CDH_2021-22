/*
 * I2Ctest.c
 *
 *  Created on: 23-Jan-2013
 *      Author: Akhil Piplani
 */
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include <at91/commons.h>
#include <at91/utility/trace.h>

#include <hal/boolean.h>
#include <hal/Drivers/LED.h>
#include <hal/Drivers/I2C.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void I2Ccallback(SystemContext context, xSemaphoreHandle semaphore) {
	signed portBASE_TYPE flag = pdFALSE;

	if(context == task_context) {
		xSemaphoreGive(semaphore);
	}
	else {
		xSemaphoreGiveFromISR(semaphore, &flag);
	}
	//TRACE_DEBUG_WP(" I2C_callbackTask1: Transfer complete. \n\r");
}

int doBlockingI2CTransfer(xSemaphoreHandle semaphore, I2CgenericTransfer *tx) {
	int retVal;

	if(xSemaphoreTake(semaphore, (portTickType)10) != pdTRUE) { // This should never happen if we coded things correctly.
		TRACE_WARNING(" doBlockingI2CTransfer: xSemaphoreTake failed! \n\r");
		while(1);
	}

	retVal = I2C_queueTransfer(tx);
	if(retVal != 0) { // This would never happen unless there are >64 transfers queued into the driver.
		return retVal;
	}

	// Block on the semaphore waiting for the transfer to finish.
	// This will swap the task out of context and allow other tasks to post their transfers.
	if(xSemaphoreTake(semaphore, (portTickType)portMAX_DELAY) != pdTRUE) { // This should never happen either!
		TRACE_WARNING("\n\r doBlockingI2CTransfer: xSemaphoreTake failed! \n\r");
		while(1);
	}
	xSemaphoreGive(semaphore);
	return 0;
}


/*
 * Here the use of I2C_queueTransfer function is demonstrated by implementing our own I2Ccallback and
 * doBlockingI2CTransfer functions to achieve a blocking transfer.
 * Sometimes, the result of a transfer may not matter to the system, or the task may not want to block
 * on each transfer. For example:
 *
  	if(xSemaphoreTake(semaphore, (portTickType)10) != pdTRUE) { // This should never happen if we coded things correctly.
		TRACE_WARNING(" doBlockingI2CTransfer: xSemaphoreTake failed! \n\r");
		while(1);
	}
  	retVal = I2C_queueTransfer(tx);
	if(retVal != 0) { // This would never happen unless there are >64 transfers queued into the driver.
		printf("taskName: I2C_queueTransfer returned: %d \n\r", retVal);
	}

	// Do something useful here instead of giving up the processor to other tasks like
	// make ADCS calculations; create the data for the next packet
	// and queue that transfer too (using another semaphore and data buffers).
	somethingSmartWhileTransferHappens();

	//  Now try and take the semaphore again, if the callback released the semaphore already, this should return immediately.
	if(xSemaphoreTake(semaphore, (portTickType)portMAX_DELAY) != pdTRUE) { // This should never happen either!
		TRACE_WARNING("\n\r doBlockingI2CTransfer: xSemaphoreTake failed! \n\r");
		while(1);
	}
	xSemaphoreGive(semaphore);
 *
 * However, if blocking transfers is all we need, simply call int I2C_writeRead(I2CwriteReadTransfer *tx, I2CtransferStatus *result);
 * The I2C driver implements the same functions internally.
 */
/*
 * Here the use of I2C_queueTransfer function is demonstrated by implementing our own I2Ccallback and
 * doBlockingI2CTransfer functions to achieve a blocking transfer.
 * Sometimes, the result of a transfer may not matter to the system, or the task may not want to block
 * on each transfer. For example:
 *
  	if(xSemaphoreTake(semaphore, (portTickType)10) != pdTRUE) { // This should never happen if we coded things correctly.
		TRACE_WARNING(" doBlockingI2CTransfer: xSemaphoreTake failed! \n\r");
		while(1);
	}
  	retVal = I2C_queueTransfer(tx);
	if(retVal != 0) { // This would never happen unless there are >64 transfers queued into the driver.
		printf("taskName: I2C_queueTransfer returned: %d \n\r", retVal);
	}

	// Do something useful here instead of giving up the processor to other tasks like
	// make ADCS calculations; create the data for the next packet
	// and queue that transfer too (using another semaphore and data buffers).
	somethingSmartWhileTransferHappens();

	//  Now try and take the semaphore again, if the callback released the semaphore already, this should return immediately.
	if(xSemaphoreTake(semaphore, (portTickType)portMAX_DELAY) != pdTRUE) { // This should never happen either!
		TRACE_WARNING("\n\r doBlockingI2CTransfer: xSemaphoreTake failed! \n\r");
		while(1);
	}
	xSemaphoreGive(semaphore);
 *
 * However, if blocking transfers is all we need, simply call int I2C_writeRead(I2CwriteReadTransfer *tx, I2CtransferStatus *result);
 * The I2C driver implements the same functions internally.
 */


void taskQueuedI2Ctest1() {
	int retValInt = 0;
	unsigned int i;
	I2CgenericTransfer i2cTx;
	I2CtransferStatus txResult;
	xSemaphoreHandle txSemaphore = NULL;
	unsigned char readData[64] = {0}, writeData[64] = {0};
	TRACE_DEBUG("\n\r taskQueuedI2Ctest1: Starting. \n\r");

	writeData[0] = 0xEF;
	for(i=1; i<sizeof(writeData); i++) {
		writeData[i] = (unsigned char)(i*2);
	}

	vSemaphoreCreateBinary(txSemaphore);
	if(txSemaphore == NULL) {
		TRACE_WARNING("\n\r taskQueuedI2Ctest1: vSemaphoreCreateBinary failed! \n\r");
		while(1);
	}

	i2cTx.callback = I2Ccallback;
	i2cTx.direction = writeRead_i2cDir;
	i2cTx.readData = readData;
	i2cTx.readSize = 2;
	i2cTx.writeData = writeData;
	i2cTx.writeSize = 3;
	i2cTx.writeReadDelay = 0;
	i2cTx.slaveAddress = 0x41;
	i2cTx.result = &txResult;
	i2cTx.semaphore = txSemaphore;

	while(1) {
		//TRACE_DEBUG(" taskQueuedI2Ctest1 \n\r");

		retValInt = doBlockingI2CTransfer(txSemaphore, &i2cTx);
		if(retValInt != 0) {
			TRACE_WARNING("\n\r taskQueuedI2Ctest1: I2C_queueTransfer returned: %d! \n\r", retValInt);
			while(1);
		}
		else {
			if(txResult==error_i2c || txResult==readError_i2c || txResult==writeError_i2c) {
				TRACE_WARNING("\n\r taskQueuedI2Ctest1: transfer error! \n\r");
			}
		}

		//TRACE_DEBUG(" taskQueuedI2Ctest1: received back: \n\r");
		//TRACE_DEBUG("0x%X ", readData[0]);
		for(i=1; i<i2cTx.readSize; i++) {
			//TRACE_DEBUG("0x%X ", readData[i]);
			writeData[i]++;
		}
		writeData[i]++;

		//TRACE_DEBUG(" \n\r\n\r");
		vTaskDelay(5);
	}
}

void taskQueuedI2Ctest2() {
	int retValInt = 0;
	unsigned int i;
	I2CgenericTransfer i2cTx;
	I2CtransferStatus txResult;
	xSemaphoreHandle txSemaphore = NULL;
	unsigned char readData[64] = {0}, writeData[64] = {0};
	TRACE_DEBUG("\n\r taskQueuedI2Ctest2: Starting. \n\r");

	writeData[0] = 0xEF;
	for(i=1; i<sizeof(writeData); i++) {
		writeData[i] = (unsigned char)(i*2);
	}

	vSemaphoreCreateBinary(txSemaphore);
	if(txSemaphore == NULL) {
		TRACE_WARNING("\n\r taskQueuedI2Ctest2: vSemaphoreCreateBinary failed! \n\r");
		while(1);
	}

	i2cTx.callback = I2Ccallback;
	i2cTx.direction = writeRead_i2cDir;
	i2cTx.readData = readData;
	i2cTx.readSize = 3;
	i2cTx.writeData = writeData;
	i2cTx.writeSize = 4;
	i2cTx.writeReadDelay = 1;
	i2cTx.slaveAddress = 0x41;
	i2cTx.result = &txResult;
	i2cTx.semaphore = txSemaphore;

	while(1) {
		//TRACE_DEBUG(" taskQueuedI2Ctest2 \n\r");

		retValInt = doBlockingI2CTransfer(txSemaphore, &i2cTx);
		if(retValInt != 0) {
			TRACE_WARNING("\n\r taskQueuedI2Ctest2: I2C_queueTransfer returned: %d! \n\r", retValInt);
			while(1);
		}
		else {
			if(txResult==error_i2c || txResult==readError_i2c || txResult==writeError_i2c) {
				TRACE_WARNING("\n\r taskQueuedI2Ctest2: transfer error! \n\r");
			}
		}

		//TRACE_DEBUG(" taskQueuedI2Ctest2: received back: \n\r");
		//TRACE_DEBUG("0x%X ", readData[0]);
		for(i=1; i<i2cTx.readSize; i++) {
			//TRACE_DEBUG("0x%X ", readData[i]);
			writeData[i]++;
		}
		writeData[i]++;

		//TRACE_DEBUG(" \n\r\n\r");
		vTaskDelay(5);
	}
}

void taskQueuedI2Ctest3() {
	int retValInt = 0;
	unsigned int i;
	I2Ctransfer i2cTx;
	unsigned char readData[64] = {0}, writeData[64] = {0};
	TRACE_DEBUG("\n\r taskQueuedI2Ctest3: Starting. \n\r");

	writeData[0] = 0xEF;
	for(i=1; i<sizeof(writeData); i++) {
		writeData[i] = (unsigned char)(i*2);
	}

	i2cTx.readData = readData;
	i2cTx.readSize = 4;
	i2cTx.writeData = writeData;
	i2cTx.writeSize = 5;
	i2cTx.writeReadDelay = 2;
	i2cTx.slaveAddress = 0x41;

	while(1) {
		//TRACE_DEBUG(" taskQueuedI2Ctest3 \n\r");

		retValInt = I2C_writeRead(&i2cTx); // Use I2C_writeRead instead of our own implementation.
		if(retValInt != 0) {
			TRACE_WARNING("\n\r taskQueuedI2Ctest3: I2C_writeRead returned: %d! \n\r", retValInt);
			while(1);
		}

		//TRACE_DEBUG(" taskQueuedI2Ctest3: received back: \n\r");
		//TRACE_DEBUG("0x%X ", readData[0]);
		for(i=1; i<i2cTx.readSize; i++) {
			//TRACE_DEBUG("0x%X ", readData[i]);
			writeData[i]++;
		}
		writeData[i]++;

		//TRACE_DEBUG(" \n\r\n\r");
		vTaskDelay(5);
	}
}

void taskQueuedI2CtestCustom() {
	TRACE_DEBUG_WP("\n\r taskQueuedI2CtestCustom: Starting. \n\r"); //(PSA) _WP removes the prepended text in the debug output. i.e. "-D-" will be removed

	int retValInt = 0;
	unsigned int i;
	I2Ctransfer i2cTx;
	unsigned char readData[64] = {0};
	unsigned char writeOut[13] = "Hello World!\0";

	i2cTx.readData = readData; //Datastructure to hold inbound data from slave
	i2cTx.readSize = 13; //Size we want to read from the inbound data, not duplicate last character if inbound data is longer then we choose to read
	i2cTx.writeData = writeOut; //data structure for outbound data
	i2cTx.writeSize = 13; //length to send out
	i2cTx.writeReadDelay = 1; //add a delay in ticks, divide by portTICK_RATE_MS to get ms, 1 tick = 1 ms based on configuration so divide not required
	i2cTx.slaveAddress = 0x41; //<-- SLAVE ADDRESS, you should use a define for this hex address, our custom C files use defines

	//while (1){ <-- Insert infinite loop here if you want to change this into a task style function

		TRACE_DEBUG_WP("To x41 Sending I2CTransfer via writeRead() \n\r");

		retValInt = I2C_writeRead(&i2cTx); // Use I2C_writeRead to send data and expect data in response, see other tests on how to do blocking queue transfers
		if(retValInt != 0) {
			TRACE_WARNING_WP("\n\r taskQueuedI2CtestCustom: I2C_writeRead returned with code: %d! \n\r", retValInt);
			return;
		}

		TRACE_DEBUG_WP(" Message received back: \n\r");
		TRACE_DEBUG_WP("%c", readData[0]);
		for(i=1; i<i2cTx.readSize; i++) {
			TRACE_DEBUG_WP("%c", readData[i]);
		}
		TRACE_DEBUG_WP("\n\r Message received back in HEX: \n\r");
		TRACE_DEBUG_WP("%02X", readData[0]);
		for(i=1; i<i2cTx.readSize; i++) {
			TRACE_DEBUG_WP("%02X", readData[i]);
		}
		TRACE_DEBUG_WP(" \n\r\n\r");

		csp_sleep_ms(5); //<-- Just like vTaskDelay but the parameter is ms not ticks. (vTaskDelay() turns out to be 1 tick = 1 ms but the csp call does the division to ensure it)

	//<--}
}

/**
 * We tend to run only one of the test tasks to isolate what we want to look at
 * If we opt to not use a task and just use the function call we can return true to go back to menu and then we can try test again as long as we wish
 * If you opt to use task style tests make sure to return FALSE or you will not see debug output as you get returned to main menu that will hold the output stream open for menu selection
 * @return
 */
Boolean I2Ctest() {
	int retValInt = 0;
	//xTaskHandle taskQueuedI2CtestCustomHandle, taskQueuedI2Ctest3Handle, taskQueuedI2Ctest2Handle, taskQueuedI2Ctest1Handle;

	//Our I2c can do 400 khz max. FAST MODE but we need to run in 100 kHz for slower slaves
	retValInt = I2C_start(100000, 2500);//2nd param can be 'portMAX_DELAY' for debug step through to prevent timeout.
	if(retValInt != 0) {
		TRACE_FATAL("\n\r I2Ctest: I2C_start returned: %d! \n\r", retValInt);
	}

	//xTaskGenericCreate(taskQueuedI2Ctest1, (const signed char*)"taskQueuedI2Ctest1", 1024, NULL, 2, &taskQueuedI2Ctest1Handle, NULL, NULL);
	//xTaskGenericCreate(taskQueuedI2Ctest2, (const signed char*)"taskQueuedI2Ctest2", 1024, NULL, 2, &taskQueuedI2Ctest2Handle, NULL, NULL);
	//xTaskGenericCreate(taskQueuedI2Ctest3, (const signed char*)"taskQueuedI2Ctest3", 1024, NULL, 2, &taskQueuedI2Ctest3Handle, NULL, NULL);

	//Our Custom I2C Task
	//xTaskGenericCreate(taskQueuedI2CtestCustom, (const signed char*)"taskQueuedI2CtestCustom", 1024, NULL, configMAX_PRIORITIES-2, &taskQueuedI2CtestCustomHandle, NULL, NULL);

	taskQueuedI2CtestCustom(); //We are setup to run the test function once and return to menu, you may then initiate it again.
	return TRUE;
}
