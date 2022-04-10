
#ifndef CSP_DRIVERS_I2C_H
#define CSP_DRIVERS_I2C_H

#include <csp/interfaces/csp_if_i2c.h>

#ifdef __cplusplus
extern "C" {
#endif

int csp_I2C_start_and_add_I2C_interface(unsigned int i2cBusSpeed_Hz,unsigned int i2cTransferTimeout, const char * ifname, csp_iface_t ** return_iface);

#ifdef __cplusplus
}
#endif
#endif
