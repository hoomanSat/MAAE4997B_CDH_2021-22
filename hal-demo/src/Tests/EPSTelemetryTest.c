/*
 * EPSTelemetryTest.c
 *
 *  Created on: 2022-02-07
 *      Author: Dante Corsi
 */

#include <Tests/EPSTelemetryTest.h>

void taskEPS_I2C_Test() {
	int retValInt = 0;
	unsigned int i;
	I2Ctransfer i2cTx;
	unsigned char readData[64] = {0}, writeData[64] = {0};
	TRACE_DEBUG("\n\r taskEPS_I2C_Test: Starting. \n\r");

	writeData[0] = 0x33;
	for(i=1; i<sizeof(writeData); i++) {
		writeData[i] = (unsigned char)(i*2);
	}

	unsigned char writeOut[13] = "Hello World!\0";
	i2cTx.readData = readData;
	i2cTx.readSize = 13;
	i2cTx.writeData = writeOut;
	i2cTx.writeSize = 13;
	i2cTx.writeReadDelay = 2;
	i2cTx.slaveAddress = 0x41;

		TRACE_DEBUG(" taskEPS_I2C_Test \n\r");

		retValInt = I2C_writeRead(&i2cTx); // Use I2C_writeRead instead of our own implementation.
		if(retValInt != 0) {
			TRACE_WARNING("\n\r taskEPS_I2C_Test: I2C_writeRead returned: %d! \n\r", retValInt);
			while(1);
		}

		TRACE_DEBUG(" taskEPS_I2C_Test: received back: \n\r");
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
		TRACE_DEBUG(" \n\r\n\r");
		//vTaskDelay(5);

}

Boolean EPSTelemetryTest() {
	int retValInt = 0;
	xTaskHandle taskEPS_I2C_Test_Handle;

	//Our I2c can do 400 khz max. FAST MODE
	retValInt = I2C_start(400000, 5000);//2nd param can be 'portMAX_DELAY' for debug step through to prevent timeout.
	if(retValInt != 0) {
		TRACE_FATAL("\n\r taskEPS_I2C_Test: I2C_start returned: %d! \n\r", retValInt);
	}

	//create the EPS Telemetry task
	xTaskGenericCreate(taskEPS_I2C_Test, (const signed char*)"taskEPS_I2C_Test", 1024, NULL, 2, &taskEPS_I2C_Test_Handle, NULL, NULL);

	//Run the task
	taskEPS_I2C_Test();
	return TRUE;
}
