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
	I2Ctransfer i2c;
	unsigned char readData[64] = {0}, writeData[64] = {0};
	TRACE_DEBUG_WP("\n\r taskEPS_I2C_Test: Starting. \n\r");

	/*writeData[0] = 0x33;
	for(i=1; i<sizeof(writeData); i++) {
		writeData[i] = (unsigned char)(i*2);
	}*/

	// this sequence of bytes prompts the EPS to return the current EPS board status
	unsigned char writeOut[2];
	writeOut[0] = 0x01; // command
	writeOut[1] = 0x00; // data parameter

	i2c.writeData = writeOut;
	i2c.writeSize = 2;
	i2c.readData = readData;
	i2c.readSize = 2;
	i2c.writeReadDelay = 2;
	i2c.slaveAddress = 0x2B; // EPS slave address as listed in the EPS technical manual

		TRACE_DEBUG_WP(" taskEPS_I2C_Test \n\r");

		retValInt = I2C_writeRead(&i2c); // Use I2C_writeRead instead of our own implementation.
		if(retValInt != 0) {
			TRACE_WARNING("\n\r taskEPS_I2C_Test: I2C_writeRead returned: %d! \n\r", retValInt);
			while(1);
		}

		TRACE_DEBUG_WP(" taskEPS_I2C_Test: received back: \n\r");
		printf("%c", readData[0]);
		for(i=1; i<i2c.readSize; i++) {
			printf("%c", readData[i]);
		}
		printf("\n");
		printf("%02X", readData[0]);
		for(i=1; i<i2c.readSize; i++) {
			printf("%02X", readData[i]);
		}
		printf("\n\r");
		TRACE_DEBUG_WP(" \n\r\n\r");
		//vTaskDelay(5);

}

Boolean EPSTelemetryTest() {
	int retValInt = 0;
	xTaskHandle taskEPS_I2C_Test_Handle;

	// For EPS communication, i2c bus runs at 100 kHz speed
	retValInt = I2C_start(100000, 5000);
	if(retValInt != 0) {
		TRACE_FATAL("\n\r taskEPS_I2C_Test: I2C_start returned: %d! \n\r", retValInt);
	}

	//create the EPS Telemetry task
	xTaskGenericCreate(taskEPS_I2C_Test, (const signed char*)"taskEPS_I2C_Test", 1024, NULL, 2, &taskEPS_I2C_Test_Handle, NULL, NULL);

	//Run the task
	taskEPS_I2C_Test();
	return TRUE;
}
