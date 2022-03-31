/*
 * EPSTelemetryTest.c
 *
 *  Created on: 2022-02-07
 *      Author: Dante Corsi
 */

#include <Tests/EPSTelemetryTest.h>

#define EPS_ADDRESS 0x2A // EPS i2c address
#define READ_SIZE 2 // EPS Telemetry return size

void taskEPS_I2C_Test() {
	int retValInt = 0;
	unsigned char COMMAND[3] = {0x10,0xE2,0x80}; // command byte
    unsigned char DATA1 = 0x00; // data byte 2
	//unsigned char DATA2 = 0x00; // data byte
	unsigned char EPS_TELEMETRY[2] = {0}; // returned telemetry from EPS

	I2Ctransfer i2cTx;
	i2cTx.readData = EPS_TELEMETRY;
	i2cTx.readSize = 2;
	i2cTx.slaveAddress = EPS_ADDRESS;
	i2cTx.writeData = COMMAND;
	i2cTx.writeSize = 3;
	i2cTx.writeReadDelay = 5;

	retValInt = I2C_writeRead(&i2cTx);
	if (retValInt != 0) {
		TRACE_FATAL("\n\r taskEPS_I2C_Test: I2C_write_read #1 returned: %d \n\r", retValInt); // returns the error code for debugging
	}
	else {
		printf("\n\r taskEPS_I2C_Test: I2C_write_read #1 successful!"); // print this on success
	}

	printf("\n\r EPS Telemetry Read: \n\r");
	UTIL_DbguDumpArrayBytes(EPS_TELEMETRY, 2);

	double x = (double) ((EPS_TELEMETRY[1] << 8) & EPS_TELEMETRY[0]);
	printf("\n\r Telemetry Data: %f \n\r",x*0.008993);

	return;

	// all this is ignored due to return above
    // send commands
	retValInt = I2C_write(EPS_ADDRESS, &COMMAND, 1); // write the first i2c command
	if (retValInt != 0) {
		TRACE_FATAL("\n\r taskEPS_I2C_Test: I2C_write #1 returned: %d \n\r", retValInt); // returns the error code for debugging
	}
	else {
		printf("\n\r taskEPS_I2C_Test: I2C_write #1 successful!"); // print this on success
	}

	retValInt = I2C_write(EPS_ADDRESS, &DATA1, 1); // write the second i2c command
	if (retValInt != 0) {
		TRACE_FATAL("\n\r taskEPS_I2C_Test: I2C_write #2 returned: %d \n\r", retValInt); // returns the error code for debugging
	}
	else {
		printf("\n\r taskEPS_I2C_Test: I2C_write #2 successful!"); // print this on success
	}

/*	retValInt = I2C_write(EPS_ADDRESS, &DATA2, 1); // write the second i2c command
	if (retValInt != 0) {
		TRACE_FATAL("\n\r taskEPS_I2C_Test: I2C_write #3 returned: %d \n\r", retValInt); // returns the error code for debugging
	}
	else {
		printf("\n\r taskEPS_I2C_Test: I2C_write #3 successful!"); // print this on success
	}*/

	// wait 1000 ms
	vTaskDelay(1000);

	// read from EPS
	retValInt = I2C_read(EPS_ADDRESS, &EPS_TELEMETRY, READ_SIZE); // write the first i2c command
	if (retValInt != 0) {
		TRACE_FATAL("\n\r taskEPS_I2C_Test: I2C_read #1 returned: %d \n\r", retValInt); // returns the error code for debugging
	}
	else {
		printf("\n\r taskEPS_I2C_Test: I2C_read #1 successful!"); // print this on success
	}




}

Boolean EPSTelemetryTest() {
	int retValInt = 0;


	// for EPS communication, i2c bus runs at 100 kHz speed
	retValInt = I2C_start(100000, 10000); // start the i2c bus
	if(retValInt != 0) {
		TRACE_FATAL("\n\r taskEPS_I2C_Test: I2C_start returned: %d \n\r", retValInt);
	}
	else {
		printf("\n\r taskEPS_I2C_Test: I2C_start successful!");
	}

	//create the EPS Telemetry task
	//xTaskHandle taskEPS_I2C_Test_Handle;
	//xTaskGenericCreate(taskEPS_I2C_Test, (const signed char*)"taskEPS_I2C_Test", 1024, NULL, 2, &taskEPS_I2C_Test_Handle, NULL, NULL);

	//Run the task
	taskEPS_I2C_Test();

	I2C_stop(); // stops the i2c bus once communication is done

	return TRUE;
}
