/*
 * EPSTelemetryTest.c
 *
 *  Created on: 2022-02-07
 *      Author: Dante Corsi
 */

#include <Tests/EPSTelemetryTest.h>

#define EPS_ADDRESS 0x2B // EPS i2c address

void taskEPS_I2C_Test() {
	int retValInt = 0;
	unsigned char firstCommand = 0x00;
	unsigned char secondCommand = 0x00;

	retValInt = I2C_write(EPS_ADDRESS, firstCommand, 1); // write the first i2c command
	if (retValInt != 0) {
		TRACE_FATAL("\n\r taskEPS_I2C_Test: I2C_write #1 returned: %d \n\r", retValInt);
	}
	else {
		printf("\n\r taskEPS_I2C_Test: I2C_write #1 successful!");
	}

	retValInt = I2C_write(EPS_ADDRESS, secondCommand, 1); // write the second i2c command
	if (retValInt != 0) {
		TRACE_FATAL("\n\r taskEPS_I2C_Test: I2C_write #2 returned: %d \n\r", retValInt);
	}
	else {
		printf("\n\r taskEPS_I2C_Test: I2C_write #2 successful!");
	}

}

Boolean EPSTelemetryTest() {
	int retValInt = 0;
	xTaskHandle taskEPS_I2C_Test_Handle;

	// for EPS communication, i2c bus runs at 100 kHz speed
	retValInt = I2C_start(100000, 5000); // start the i2c bus
	if(retValInt != 0) {
		TRACE_FATAL("\n\r taskEPS_I2C_Test: I2C_start returned: %d \n\r", retValInt);
	}
	else {
		printf("\n\r taskEPS_I2C_Test: I2C_start successful!");
	}

	//create the EPS Telemetry task
	xTaskGenericCreate(taskEPS_I2C_Test, (const signed char*)"taskEPS_I2C_Test", 1024, NULL, 2, &taskEPS_I2C_Test_Handle, NULL, NULL);

	//Run the task
	taskEPS_I2C_Test();
	return TRUE;
}
