/*
 * EPSTelemetryTest.c
 *
 *  Created on: 2022-02-07
 *  Updated on: 2022-04-05
 *      Author: Dante Corsi
 *      Modified by: Sam Dunthorne
 */

#include <Tests/EPSTelemetryTest.h>

#define EPS_ADDRESS 0x2A // EPS i2c address
#define READ_SIZE 2 // EPS Telemetry return size

void taskEPS_I2C_Test() {

	Boolean continueTest = TRUE;
	unsigned int choice;
	unsigned int selection = 0;


	while (continueTest){

	int retValInt = 0;
	 // command byte
	unsigned char COMMAND[3] = {0x10,0xE2,0x80};
    unsigned char DATA1 = 0x00; // data byte 2
	//unsigned char DATA2 = 0x00; // data byte
	unsigned char EPS_TELEMETRY[2] = {0}; // returned telemetry from EPS
	unsigned int conversionValue = 0.008993; // This value is specifically for test 1, in the future this value can be variable and use a lookup table for each test

	I2Ctransfer i2cTx;
	i2cTx.readData = EPS_TELEMETRY;
	i2cTx.readSize = 2;
	i2cTx.slaveAddress = EPS_ADDRESS;
	i2cTx.writeSize = 3;
	i2cTx.writeReadDelay = 5;

	printf( "\n\r Select an EPS test to perform: \n\r");
	printf("\t 1) Check Battery Output Voltage \n\r");
	printf("\t 2) Check EPS Motherboard Temperature \n\r");


	while(UTIL_DbguGetIntegerMinMax(&selection, 1, 15) == 0);
	switch(selection) {
		case 1:
			COMMAND[3] = (0x10,0xE2,0x80);
			i2cTx.writeData = COMMAND;
			break;
		case 2:
			COMMAND[3] = (0x10,0xE3,0x08);
			i2cTx.writeData = COMMAND;
			break;

	}

	retValInt = I2C_writeRead(&i2cTx);
	if (retValInt != 0) {
		TRACE_FATAL("\n\r taskEPS_I2C_Test: I2C_write_read #1 returned: %d \n\r", retValInt); // returns the error code for debugging
	}


	UTIL_DbguDumpArrayBytes(EPS_TELEMETRY, 2);

	char hexA = EPS_TELEMETRY[0];
	char hexB = EPS_TELEMETRY[1];

	double x1 = (hexA << 8);
	double x2 = hexB;

	double sum = x1 + x2;
	float convert = sum * conversionValue;

	printf("Read Decimal: %f \n\r", sum);
	printf("Converted Value: %f Volts \n\r", convert);


	if(continueTest != FALSE) {
	printf("Continue EPS Test? (1=Yes, 0=No): \n\r");
	while(UTIL_DbguGetIntegerMinMax(&choice, 0, 1) == 0);
	if(choice == 0) {
		continueTest = FALSE;
		}
	}
	else {
		break;
		}
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
