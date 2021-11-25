/*
 * cspTest.cpp
 *
 *  Created on: Nov. 25, 2021
 *      Author: calac
 */

#include "cspTest.h"
#include <csp/drivers/i2c.h>

#define I2C_SPEED 			400000
#define I2C_TIMEOUT 		1000

/* Server port, the port the server listens on for incoming connections from the client. */
#define MY_SERVER_PORT		10

/* Commandline options */
static uint8_t server_address = 255;

/* test mode, used for verifying that host & client can exchange packets over the loopback interface */
static bool test_mode = false;
static unsigned int server_received = 0;

Boolean cspTest() {

	//Startup CSP Test
	uint8_t address = 1;
	csp_debug_level_t debug_level = CSP_INFO;

	//Need to set the rTABLE! -ex. "0/0 AX100, 0/2 I2C, 6/5 CAN"
	const char * rtable = NULL;

	/* enable/disable debug levels */
	for (csp_debug_level_t i = 0; i <= CSP_LOCK; ++i) {
		csp_debug_set_level(i, (i <= debug_level) ? true : false);
	}

	csp_log_info("Initializing CSP");

	/* Init CSP with address and default settings */
	csp_conf_t csp_conf;
	csp_conf_get_defaults(&csp_conf);
	csp_conf.address = address;
	int error = csp_init(&csp_conf);
	if (error != CSP_ERR_NONE) {
		csp_log_error("csp_init() failed, error: %d", error);
		exit(1);
	}

	/* Start router task with 500 bytes of stack (priority is only supported on FreeRTOS) */
	csp_route_start_task(500, configMAX_PRIORITIES-2);

	/* Add interface(s) */
	csp_iface_t * default_iface = NULL;
	error = csp_I2C_start_and_add_I2C_interface(I2C_SPEED, I2C_TIMEOUT, CSP_IF_I2C_DEFAULT_NAME,  &default_iface);
	if (error != CSP_ERR_NONE) {
		csp_log_error("failed to add I2C interface, error: %d", error);
		exit(1);
	}


	if (rtable) {
		error = csp_rtable_load(rtable);
		if (error < 1) {
			csp_log_error("csp_rtable_load(%s) failed, error: %d", rtable, error);
			exit(1);
		}
	} else if (default_iface) {
		csp_rtable_set(CSP_DEFAULT_ROUTE, 0, default_iface, CSP_NO_VIA_ADDRESS);
	} else {
		/* no interfaces configured - run server and client in process, using loopback interface */
		server_address = address;
	}

	printf("Connection table\r\n");
	csp_conn_print_table();

	printf("Interfaces\r\n");
	csp_route_print_interfaces();

	printf("Route table\r\n");
	csp_route_print_table();

	/* Start server thread */
	if ((server_address == 255) ||  /* no server address specified, I must be server */
		(default_iface == NULL)) {  /* no interfaces specified -> run server & client via loopback */
		//csp_thread_create(task_server, "SERVER", 1000, NULL, 0, NULL);
	}

	/* Start client thread */
	if ((server_address != 255) ||  /* server address specified, I must be client */
		(default_iface == NULL)) {  /* no interfaces specified -> run server & client via loopback */
		//csp_thread_create(task_client, "CLIENT", 1000, NULL, 0, NULL);
	}

	/* Wait for execution to end (ctrl+c) */
	while(1) {
		csp_sleep_ms(3000);

		if (test_mode) {
			/* Test mode is intended for checking that host & client can exchange packets over loopback */
			if (server_received < 5) {
				csp_log_error("Server received %u packets", server_received);
				exit(1);
			}
			csp_log_info("Server received %u packets", server_received);
			exit(0);
		}
	}

	return 0;


    return TRUE;
}


