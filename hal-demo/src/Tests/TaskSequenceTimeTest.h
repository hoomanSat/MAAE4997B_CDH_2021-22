/*
 * TaskSequenceTimeTest.h
 *
 *  	Created: 2022-03-18
 *      Author: Dante Corsi
 */

#ifndef TIMETEST_H_
#define TIMETEST_H_

#include <at91/commons.h>
#include <at91/utility/trace.h>

#include <freertos/FreeRTOS.h>
#include <freertos/FreeRTOSConfig.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/timers.h>
#include <freertos/projdefs.h>
#include <freertos/portable/GCC/ARM9_AT91SAM9G20/portmacro.h>

#include <hal/boolean.h>
#include <hal/errors.h>
#include <hal/supervisor.h>
#include <hal/Drivers/SPI.h>
#include <hal/Drivers/LED.h>
#include <hal/Drivers/I2C.h>
#include <hal/Timing/Time.h>
#include <hal/Timing/RTC.h>

#include <stdlib.h>
#include <string.h>

Boolean TaskSequenceTimeTest();

#endif /* TIMETEST_H_ */
