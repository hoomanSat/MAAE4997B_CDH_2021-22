/*
 * main.c

 *      Author: Akhil
 *      Alteration by C&DH Team
 */

#include "Demo/demo_sd.h"

#include "Tests/legacyTests/ADCtest.h"
#include "Tests/legacyTests/boardTest.h"
#include "Tests/legacyTests/checksumTest.h"
#include "Tests/legacyTests/FloatingPointTest.h"
#include "Tests/legacyTests/I2CslaveTest.h"
#include "Tests/legacyTests/I2Ctest.h"
#include "Tests/legacyTests/LEDtest.h"
#include "Tests/legacyTests/PinTest.h"
#include "Tests/legacyTests/PWMtest.h"
#include "Tests/legacyTests/SDCardTest.h"
#include "Tests/legacyTests/SPI_FRAM_RTCtest.h"
#include "Tests/legacyTests/SupervisorTest.h"
#include "Tests/legacyTests/TimeTest.h"
#include "Tests/legacyTests/UARTtest.h"
#include "Tests/legacyTests/USBdeviceTest.h"

#include "Tests/FreeRTOSTest.h"
#include "Tests/cspTest.h"
#include "Tests/I2CmultiMasterTest.h"
#include "Tests/TaskSequenceTimeTest.h"
#include "Tests/FRAMTest.h"
#include "Tests/SPITest.h"
#include "Tests/UARTReadTest.h"
#include "Tests/UARTReadWriteTest.h"
#include "Tests/UARTWriteTest.h"
#include "Tests/RTCTest.h"
#include "Tests/EPSTelemetryTest.h"

#include <at91/utility/exithandler.h>
#include <at91/commons.h>
#include <at91/utility/trace.h>
#include <at91/peripherals/cp15/cp15.h>

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
	printf("\t 1) I2C \n\r");
	printf("\t 2) SD-Card File System \n\r");
	printf("\t 3) SPI + FRAM + RTC \n\r");
	printf("\t 4) UART \n\r");
	printf("\t 5) ADC Single Shot \n\r");
	printf("\t 6) ADC Continuous Mode \n\r");
	printf("\t 7) Aux Pins \n\r");
	printf("\t 8) LED \n\r");
	printf("\t 9) PWM \n\r");
	printf("\t 10) USB Device \n\r");
	printf("\t 11) Supervisor Controller Test - SPI interface \n\r");
	printf("\t 12) Supervisor Controller Test - I2C interface \n\r");
	printf("\t 13) Board Test \n\r");
	printf("\t 14) Time Test \n\r");
	printf("\t 15) Checksum Test \n\r");
	printf("----- CuSAT Additions 2021_2022 -----\n\r");
	printf("\t 16) I2C Slave Test \n\r");
	printf("\t 17) FreeRTOS Test \n\r");
	printf("\t 18) CSP-AX100 Test \n\r");
	printf("\t 19) MultiMaster Test \n\r");
	printf("\t 20) SPI Test \n\r");
	printf("\t 21) UART Read \n\r");
	printf("\t 22) UART Write \n\r");
	printf("\t 23) UART Read And Write \n\r");
	printf("\t 24) RTC Test \n\r");
	printf("\t 25) FRAM Test \n\r");
	printf("\t 26) EPSCmd \n\r");
	printf("\t 27) Task Sequence Time Test \n\r");

	while(UTIL_DbguGetIntegerMinMax(&selection, 1, 27) == 0); //Update the range if you add functions

	//Verify you are importing the header file for any new tests you add that are in a new file
	switch(selection) {
	case 1:
		offerMoreTests = I2Ctest();
		break;
	case 2:
		offerMoreTests = SDCardTest();
		break;
	case 3:
		offerMoreTests = SPI_FRAM_RTCtest();
		break;
	case 4:
		offerMoreTests = UARTtest();
		break;
	case 5:
		offerMoreTests = ADCtestSingleShot();
		break;
	case 6:
		offerMoreTests = ADCtest();
		break;
	case 7:
		offerMoreTests = PinTest();
		break;
	case 8:
		offerMoreTests = LEDtest();
		break;
	case 9:
		offerMoreTests = PWMtest();
		break;
	case 10:
		offerMoreTests = USBdeviceTest();
		break;
	case 11:
		offerMoreTests = SupervisorTest(FALSE);
		break;
	case 12:
		offerMoreTests = SupervisorTest(TRUE);
		break;
	case 13:
		offerMoreTests = boardTest();
		break;
	case 14:
		offerMoreTests = timeTest();
		break;
	case 15:
		offerMoreTests = checksumTest();
		break;
		//Our Tests Begin
	case 16:
		offerMoreTests = I2CslaveTest();
		break;
	case 17:
		offerMoreTests = freeRTOSTest();
		break;
	case 18:
		offerMoreTests = cspTest();
		break;
	case 19:
		offerMoreTests = I2CmultiMasterTest();
		break;
	case 20:
		offerMoreTests = SPITest();
		break;
	case 21:
		offerMoreTests = UARTReadTest();
		break;
	case 22:
		offerMoreTests = UARTWriteTest();
		break;
	case 23:
		offerMoreTests = UARTReadWriteTest();
		break;
	case 24:
		offerMoreTests = RTCTest();
		break;
	case 25:
		offerMoreTests = FRAMTest();
		break;
	case 26:
		offerMoreTests = EPSTelemetryTest();
		break;
	case 27:
		offerMoreTests = TaskSequenceTimeTest();
		break;
	default:
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

		/*
		 * This section works by going out to a test function, this function will typically generate test tasks.
		 * If the test function returns false, we will fall out of the main menu and yield to the next task, allowing them to go.
		 * If the test function returns true you enter the block below and may run more tests, in effect you can stack them up and let them all go at once
		 * as soon as you answer NO for perform more tests, we fall out of the menu loop and yield to the other tasks
		 *
		 * In some cases we opt to not use tasks and instead run the test inline with the menu task and when we return to menu when the test is complete.
		 * In this way we can run a test over and over, good for I2C testing for example when we want to send many times without reboot.
		 *
		 */

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
		//We added the LED wave to indicate the main menu tasks is complete and yielding to other tasks
		LED_wave(1);
		LED_waveReverse(1);
		LED_wave(1);
		LED_waveReverse(1);
		vTaskDelay(500);
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
