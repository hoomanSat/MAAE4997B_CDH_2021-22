
#ifndef CSP_DRIVERS_I2C_H
#define CSP_DRIVERS_I2C_H

#include <csp/interfaces/csp_if_i2c.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
   I2C configuration.
   @see I2C_start()
*/
typedef struct csp_I2C_conf {
	uint32_t i2cBusSpeed_Hz;
	uint32_t i2cTransferTimeout;
} csp_I2C_conf_t;


#ifdef __cplusplus
}
#endif
#endif
