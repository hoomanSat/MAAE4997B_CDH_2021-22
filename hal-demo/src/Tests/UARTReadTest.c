/*
 * UARTReadTest.c
 *
 *  Created on: Mar. 24, 2022
 *      Author: Sam Dunthorne
 */


#include <at91/boards/ISIS_OBC_G20/board.h>
#include <at91/utility/trace.h>
#include <at91/commons.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>
#include <freertos/projdefs.h>

#include <hal/Drivers/UART.h>
#include <hal/interruptPriorities.h>
#include <hal/boolean.h>
#include <hal/Utility/util.h>

#include <string.h>
#include <stdio.h>

void taskUARTReadTest(void *arguments) {
	int UART_Response_Code = 0;
	unsigned int readSize = 5, i;
	unsigned char readData[5] = {0};
	UARTbus bus = *((UARTbus*)arguments);

	while(1) {
		UART_Response_Code = UART_read(bus, readData, readSize);
		if(UART_Response_Code != 0) {
			TRACE_WARNING("\n\r taskUARTtest: UART_read returned: %d for bus %d \n\r", UART_Response_Code, bus);
		}else if(UART_Response_Code == 0){
			printf("Successful Read on bus: %d \n\r", bus);
		}

		printf("Read: %s \n\r", readData);

		vTaskDelay(0); // Must have no delay otherwise you may lose data
	}
}

Boolean UARTReadTest() {
	int UART_Response_Code = 0;
	xTaskHandle UART0testHandle;
	xTaskHandle UART2testHandle;
	static UARTbus UARTtestBus[2] = {bus0_uart, bus2_uart};

	// The iOBC has two different UART inputs on J7, UART0 is pins 10 and 11 (Rx0 & Tx0), and UART2 is pins 13 & 16 (Rx2 & Tx2)

	UARTconfig configBus0 = {.mode = AT91C_US_USMODE_NORMAL | AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS | AT91C_US_PAR_NONE | AT91C_US_OVER_16 | AT91C_US_NBSTOP_1_BIT,
								.baudrate = 115200, .timeGuard = 1, .busType = rs232_uart, .rxtimeout = 0x2580};


	UARTconfig configBus2 = {.mode = AT91C_US_USMODE_HWHSH  | AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS | AT91C_US_PAR_NONE | AT91C_US_OVER_16 | AT91C_US_NBSTOP_1_BIT,
								.baudrate = 9600, .timeGuard = 1, .busType = rs232_uart, .rxtimeout = 0x2580};


	// Both UART peripherals must be started separately as they can use different configurations.

	UART_Response_Code = UART_start(bus0_uart, configBus0);
	if(UART_Response_Code != 0) {
		TRACE_WARNING("\n\r UARTtest: UART_start returned %d! \n\r", UART_Response_Code);
		while(1);
	}

	UART_Response_Code = UART_start(bus2_uart, configBus2);
    if(UART_Response_Code != 0) {
        TRACE_WARNING("\n\r UARTtest: UART_start returned %d! \n\r", UART_Response_Code);
        while(1);
    }

	// Instantiate two separate versions of UARTtest and pass different bus-id's as a parameter.
	xTaskGenericCreate(taskUARTReadTest, (const signed char*)"UARTtest-0", 1024, (void*)&UARTtestBus[0], 2, &UART0testHandle, NULL, NULL);
	xTaskGenericCreate(taskUARTReadTest, (const signed char*)"UARTtest-2", 1024, (void*)&UARTtestBus[1], 2, &UART2testHandle, NULL, NULL);

	printf("\n\n\r");

	vTaskDelay(0); // Must have no delay otherwise you may lose data

	return FALSE;
}
