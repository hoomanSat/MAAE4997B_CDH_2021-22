/*
 * SAM_UART_Write_Test.c
 *
 *  Created on: Mar. 13th, 2022
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

void taskSAM_UART_Write_Test(void *arguments) {
	int UART_Response_Code = 0;
	unsigned int readSize = 10, i; // Originally expects 4 bytes, but this presents an issues with the Arduinos, thus I want to test if a longer readSize helps
	unsigned char readData[8] = {0};
	unsigned char writeData[8] = {0};
	UARTbus bus = *((UARTbus*)arguments);
	char* output = "abcdefgh\n\r";


	while(1) {

		/*
		UART_Response_Code = UART_read(bus, readData, readSize);
		if(UART_Response_Code != 0) {
			TRACE_WARNING("\n\r taskUARTtest: UART_read returned: %d for bus %d \n\r", UART_Response_Code, bus); //This statement prints on every unsuccessful read
		}else if(UART_Response_Code == 0){
			printf("Successful Read on bus: %d", bus);
		}

		for(i=0; i<readSize; i++) { // Loops for every character read
			printf("Read: %X", readData[i]);
			//writeData[i] = readData[i]; // Echo
		}

		*/

		//writeData[i]   = '\n';
		//writeData[i+1] = '\r';


		UART_Response_Code = UART_write(bus, output, readSize); // Write 2 bytes more than we received for \n\r

		if(UART_Response_Code != 0) {
			TRACE_WARNING("\n\r taskUARTtest: UART_write returned: %d for bus %d \n\r", UART_Response_Code, bus); // Runs on unsuccessful transmission
		}else{
			printf("Successful Transmission on bus: %d \n\r", bus);
		}



		vTaskDelay(1000);
	}
}

Boolean SAM_UART_Write_Test() {
	int UART_Response_Code = 0;
	//unsigned int bus2type = 0;
	xTaskHandle taskUART0testHandle; //, taskUART2testHandle;
	static UARTbus UARTtestBus[2] = {bus0_uart, bus2_uart};

	// The iOBC has two different UART inputs on J7, UART0 is pins 10 and 11 (Rx0 & Tx0), and UART2 is pins 13 & 16 (Rx2 & Tx2)
	// For testing, the baudrate needed to be lowered to 9600, higher speeds produced corrupted data
	// The timeout is 0x2580 which equals 1 second: time in seconds = HEX(speed/baudrate)

	UARTconfig configBus0 = {.mode = AT91C_US_USMODE_NORMAL | AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS | AT91C_US_PAR_NONE | AT91C_US_OVER_16 | AT91C_US_NBSTOP_1_BIT,
								.baudrate = 9600, .timeGuard = 1, .busType = rs232_uart, .rxtimeout = 0x2580};

	/*
	UARTconfig configBus2 = {.mode = AT91C_US_USMODE_HWHSH  | AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS | AT91C_US_PAR_NONE | AT91C_US_OVER_16 | AT91C_US_NBSTOP_1_BIT,
								.baudrate = 9600, .timeGuard = 1, .busType = rs232_uart, .rxtimeout = 0x2580};



	//printf("\n This test will receive 4 characters over UART, capitalize them and send them back. \n");
	//printf(" If you send \"12ab\", you will receive back \"12AB\" on the same bus. \n");
	//printf("\n Please select a configuration for UART2 (0=RS232 1=RS422): \n");
	//UTIL_DbguGetIntegerMinMax(&bus2type, 0, 1);
	if(bus2type != 0) {
		configBus2.mode = AT91C_US_USMODE_NORMAL | AT91C_US_CLKS_CLOCK | AT91C_US_CHRL_8_BITS | AT91C_US_PAR_NONE | AT91C_US_OVER_16 | AT91C_US_NBSTOP_1_BIT;
		configBus2.busType = rs422_noTermination_uart;
	}
	*/
	// Both UART peripherals must be started separately as they can use different configurations.
	UART_Response_Code = UART_start(bus0_uart, configBus0);
	if(UART_Response_Code != 0) {
		TRACE_WARNING("\n\r UARTtest: UART_start returned %d! \n\r", UART_Response_Code);
		while(1);
	}

	printf("Started \n");

	/*
	UART_Response_Code = UART_start(bus2_uart, configBus2);
	if(UART_Response_Code != 0) {
		TRACE_WARNING("\n\r UARTtest: UART_start returned %d! \n\r", UART_Response_Code);
		while(1);
	}

	*/

	// Instantiate two separate versions of taskUARTtest and pass different bus-id's as a parameter.
	xTaskGenericCreate(taskSAM_UART_Write_Test, (const signed char*)"taskUARTtest-0", 1024, (void*)&UARTtestBus[0], 2, &taskUART0testHandle, NULL, NULL);
	//xTaskGenericCreate(taskSAM_UART_Test, (const signed char*)"taskUARTtest-2", 1024, (void*)&UARTtestBus[1], 2, &taskUART2testHandle, NULL, NULL);


	printf("\n\n\r");


	/*
	UART_Response_Code = UART_stop(bus2_uart);
	if(UART_Response_Code != 0) {
		TRACE_WARNING("\n\r UARTtest: UART_start returned %d! \n\r", UART_Response_Code); // Return value of UART Bus 2
		while(1);
	}
	*/

	vTaskDelay(1000);

	return FALSE;
}
