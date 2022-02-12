/*
 * I2Ctest.h
 *
 *  Created on: 23-Jan-2013
 *      Author: Akhil Piplani
 */

#ifndef I2CTEST_H_
#define I2CTEST_H_


#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include <at91/commons.h>
#include <at91/utility/trace.h>

#include <hal/boolean.h>
#include <hal/Drivers/LED.h>
#include <hal/Drivers/I2C.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


Boolean I2Ctest();

/*
void I2Ccallback(SystemContext context, xSemaphoreHandle semaphore);
int doBlockingI2CTransfer(xSemaphoreHandle semaphore, I2CgenericTransfer *tx);
void taskQueuedI2Ctest3();
*/

#endif /* I2CTEST_H_ */
