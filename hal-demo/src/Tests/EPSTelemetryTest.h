/*
 * EPSTelemetryTest.h
 *
 *  Created on: 2022-02-07
 *      Author: Dante Corsi
 */

#ifndef EPSTEST_H_
#define EPSTEST_H_

#include <at91/commons.h>
#include <at91/utility/trace.h>

#include <hal/boolean.h>
#include <hal/Drivers/LED.h>
#include <hal/Drivers/I2C.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

Boolean EPSTelemetryTest(void);

#endif /* EPSTEST_H_ */
