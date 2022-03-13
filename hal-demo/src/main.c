/*
 * main.c
 *      Author: Akhil
 */

#include "Demo/demo_sd.h"

#include "Tests/I2Ctest.h"
#include "Tests/I2CslaveTest.h"
#include "Tests/SPI_FRAM_RTCtest.h"
#include "Tests/FloatingPointTest.h"
#include "Tests/ADCtest.h"
#include "Tests/UARTtest.h"
#include "Tests/PinTest.h"
#include "Tests/LEDtest.h"
#include "Tests/PWMtest.h"
#include "Tests/TimeTest.h"
#include "Tests/USBdeviceTest.h"
#include "Tests/SupervisorTest.h"
#include "Tests/boardTest.h"
#include "Tests/checksumTest.h"
#include "Tests/SDCardTest.h"
#include <at91/utility/exithandler.h>
#include <at91/commons.h>
#include <at91/utility/trace.h>
#include <at91/peripherals/cp15/cp15.h>
#include <Tests/SAM_UART_Write_Test.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include <hal/Timing/WatchDogTimer.h>
#include <hal/Drivers/LED.h>
#include <hal/Utility/util.h>

#include <hal/boolean.h>
#include <hal/version/version.h>

#include <hcc/api_fat_test.h>
#include <hcc/api_hcc_mem.h>
#include <hcc/api_fat.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define ENABLE_MAIN_TRACES 1
#if ENABLE_MAIN_TRACES
	#define MAIN_TRACE_INFO			TRACE_INFO
	#define MAIN_TRACE_DEBUG		TRACE_DEBUG
	#define MAIN_TRACE_WARNING		TRACE_WARNING
	#define MAIN_TRACE_ERROR		TRACE_ERROR
	#define MAIN_TRACE_FATAL		TRACE_FATAL
#else
	#define MAIN_TRACE_INFO(...)	{ }
	#define MAIN_TRACE_DEBUG(...)	{ }
	#define MAIN_TRACE_WARNING(...)	{ }
	#define MAIN_TRACE_ERROR		TRACE_ERROR
	#define MAIN_TRACE_FATAL		TRACE_FATAL
#endif

Boolean selectAndExecuteTest() {
	unsigned int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf( "\n\r Select a test to perform: \n\r");
	printf("\t 1) Sam-UART Write Test \n\r");
	printf("\t 2) Sam-UART Read Test \n\r");
	printf("\t 3) SAM-UART Read and Write Test \n\r");


	while(UTIL_DbguGetIntegerMinMax(&selection, 1, 15) == 0);

	switch(selection) {
	case 1:
		offerMoreTests = SAM_UART_Write_Test();
		break;
	case 2:
		offerMoreTests = SAM_UART_Read_Test();
		break;
	case 3:
		offerMoreTests = SAM_UART_Read_And_Write_Test();
		break;
	}

	return offerMoreTests;
}

void taskMain() {
	unsigned int choice;
	Boolean offerMoreTests = FALSE;

	WDT_startWatchdogKickTask(10 / portTICK_RATE_MS, FALSE);

	while(1) {
		LED_toggle(led_1);

		offerMoreTests = selectAndExecuteTest();

		if(offerMoreTests != FALSE) {
			// Not all tests will actually exit, so we may not reach here.
			// Even when we do, its good to be careful not to simultaneously run too many tests.
			// Instead, reboot between the tests.
			// In some cases, you must select 0 here to allow the test-tasks to do their work.
			printf("Perform more tests? (1=Yes, 0=No): \n\r");
			// No WatchDog resets here: The DBGU driver resets the WatchDog while it waits for user to enter characters.
			while(UTIL_DbguGetIntegerMinMax(&choice, 0, 1) == 0);
			if(choice == 0) {
				break;
			}
		}
		else {
			break;
		}
	}

	// Suspend itself.
	//vTaskSuspend(NULL);

	while(1) {
		LED_wave(1);
		//LED_waveReverse(1);
		LED_wave(1);
		//LED_waveReverse(1);
		vTaskDelay(100);
	}

}

int main() {
	unsigned int i = 0;
	xTaskHandle taskMainHandle;

	TRACE_CONFIGURE_ISP(DBGU_STANDARD, 2000000, BOARD_MCK);
	// Enable the Instruction cache of the ARM9 core. Keep the MMU and Data Cache disabled.
	CP15_Enable_I_Cache();

	printf("\n\r -- iobcBase Program Booted --\n\r");

	LED_start();

	// The actual watchdog is already started, this only initializes the watchdog-kick interface.
	WDT_start();

	printf("\nDemo applications for ISIS OBC Hardware Abstraction Layer Library built on %s at %s\n", __DATE__, __TIME__);
	printf("\nDemo applications use:\n");
	printf("* HAL lib version %s.%s.%s built on %s at %s\n", HalVersionMajor, HalVersionMinor, HalVersionRevision,
				HalCompileDate, HalCompileTime);

	LED_wave(1);
	LED_waveReverse(1);
	LED_wave(1);
	LED_waveReverse(1);

	MAIN_TRACE_DEBUG("\t main: Starting main task.. \n\r");
	xTaskGenericCreate(taskMain, (const signed char*)"taskMain", 4096, NULL, configMAX_PRIORITIES-2, &taskMainHandle, NULL, NULL);

	MAIN_TRACE_DEBUG("\t main: Starting scheduler.. \n\r");
	vTaskStartScheduler();

	// This part should never be reached.
	MAIN_TRACE_DEBUG("\t main: Waiting in an infinite loop. \n\r");
	while(1) {
		LED_wave(1);
		MAIN_TRACE_DEBUG("MAIN: STILL ALIVE %d\n\r", i);
		i++;
	}

	return 0;
}
