/*
 * cspTest.h
 *
 *  Created on: Nov. 25, 2021
 *      Author: calac
 */

#ifndef TESTS_CSPTEST_H_
#define TESTS_CSPTEST_H_

#include <hal/boolean.h>
#include <csp/csp.h>
#include <csp/csp_debug.h>
#include <csp/csp_interface.h>
#include <csp/arch/csp_thread.h>
#include <csp/arch/csp_clock.h>
#include <csp/arch/csp_time.h>
#include <csp/arch/csp_malloc.h>
#include <csp/arch/csp_queue.h>
#include <csp/arch/csp_semaphore.h>
#include <stdlib.h>
#include <csp/drivers/I2C.h>

Boolean cspTest();

#endif /* TESTS_CSPTEST_H_ */
