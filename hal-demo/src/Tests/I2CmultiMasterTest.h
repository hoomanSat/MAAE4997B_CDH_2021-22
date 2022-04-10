/*
 * I2CmultiMasterTest.h
 *
 */

#ifndef I2CMULTIMASTER_H_
#define I2CMULTIMASTER_H_


#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include <at91/commons.h>
#include <at91/utility/trace.h>
#include <at91/boards/ISIS_OBC_G20/at91sam9g20/AT91SAM9G20.H>

#include <hal/Drivers/LED.h>
#include <hal/Drivers/I2C.h>
#include <hal/boolean.h>
#include <hal/Drivers/I2Cslave.h>
#include <hal/Utility/util.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <csp/csp_debug.h>
#include <csp/arch/csp_thread.h>
/*
void slaveTestProcess();
void mastermodeWriteReadProcess();
void initSlave();
void initMaster();
void multimasterDirector();
void localSchedule();
*/
Boolean I2CmultiMasterTest();


#endif /* I2CMULTIMASTER_H_ */
