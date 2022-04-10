/*
 * RTCTest.c
 *
 *  Created on: Mar. 24, 2022
 *      Author: Sam Dunthorne
 */

// Uses the internal SPI Bus

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

Boolean RTCTest() {
	int retValInt = 0;
	xTaskHandle taskRTCtestHandle;

	retValInt = SPI_start(bus0_spi, slave2_spi); // Turns on the internal SPI bus if it is not already running
	if(retValInt != 0) {
		TRACE_WARNING("\n\r SPITest: SPI_start returned %d! \n\r", retValInt);
		while(1);
	}

	xTaskGenericCreate(RTCtest, (const signed char*)"RTCtest", 10240, NULL, 2, &taskRTCtestHandle, NULL, NULL);

	return FALSE;
}
