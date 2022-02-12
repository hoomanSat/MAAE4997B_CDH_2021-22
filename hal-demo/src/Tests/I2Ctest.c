/*
 * I2Ctest.c
 *
 *  Created on: 23-Jan-2013
 *      Author: Akhil Piplani
 */
#include <Tests/I2Ctest.h>

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

void taskQueuedI2Ctest3() {
	int retValInt = 0;
	unsigned int i;
	I2Ctransfer i2cTx;
	unsigned char readData[64] = {0}, writeData[64] = {0};
	TRACE_DEBUG_WP("\n\r taskQueuedI2Ctest3: Starting. \n\r");


	writeData[0] = 0x33;
	for(i=1; i<sizeof(writeData); i++) {
		writeData[i] = (unsigned char)(i*2);
	}


	unsigned char writeOut[13] = "Hello World!\0";
	i2cTx.readData = readData;
	i2cTx.readSize = 20;
	i2cTx.writeData = writeOut;
	i2cTx.writeSize = 13;
	i2cTx.writeReadDelay = 2;
	i2cTx.slaveAddress = 0x41;


		TRACE_DEBUG_WP(" taskQueuedI2Ctest3 \n\r");

		retValInt = I2C_writeRead(&i2cTx); // Use I2C_writeRead instead of our own implementation.
		if(retValInt != 0) {
			TRACE_WARNING("\n\r taskQueuedI2Ctest3: I2C_writeRead returned: %d! \n\r", retValInt);
			while(1);
		}

		TRACE_DEBUG_WP(" taskQueuedI2Ctest3: received back: \n\r");
		printf("%c", readData[0]);
		for(i=1; i<i2cTx.readSize; i++) {
			printf("%c", readData[i]);
		}
		printf("\n");
		printf("%02X", readData[0]);
		for(i=1; i<i2cTx.readSize; i++) {
			printf("%02X", readData[i]);
		}
		printf("\n\r");
		TRACE_DEBUG_WP(" \n\r\n\r");
		//vTaskDelay(5);

}

Boolean I2Ctest() {
	int retValInt = 0;
	//xTaskHandle taskQueuedI2Ctest3Handle;//, taskQueuedI2Ctest2Handle, taskQueuedI2Ctest3Handle;

	//Our I2c can do 400 khz max. FAST MODE
	retValInt = I2C_start(400000, 5000);//2nd param can be 'portMAX_DELAY' for debug step through to prevent timeout.
	if(retValInt != 0) {
		TRACE_FATAL("\n\r I2Ctest: I2C_start returned: %d! \n\r", retValInt);
	}


	//xTaskGenericCreate(taskQueuedI2Ctest1, (const signed char*)"taskQueuedI2Ctest1", 1024, NULL, configMAX_PRIORITIES-2, &taskQueuedI2Ctest1Handle, NULL, NULL);
	//xTaskGenericCreate(taskQueuedI2Ctest2, (const signed char*)"taskQueuedI2Ctest2", 1024, NULL, 2, &taskQueuedI2Ctest2Handle, NULL, NULL);
	//xTaskGenericCreate(taskQueuedI2Ctest3, (const signed char*)"taskQueuedI2Ctest3", 1024, NULL, 2, &taskQueuedI2Ctest3Handle, NULL, NULL);
	taskQueuedI2Ctest3();
	return TRUE;
}
