

#include <csp/drivers/i2c.h>

#include <stdio.h>

#include <csp/csp.h>
#include <hal/Drivers/I2C.h>
#include <csp/arch/csp_malloc.h>
#include <csp/interfaces/csp_if_i2c.h>
#include <csp/csp_debug.h>

typedef struct {
	char name[CSP_IFLIST_NAME_MAX + 1];
	csp_iface_t iface;
	csp_i2c_interface_data_t ifdata;
	unsigned int slaveAddress;

} i2c_context_t;

/*
 * typedef struct _I2CwriteReadTransfer {
	unsigned int slaveAddress; //!< Address of the slave where to make the transfer.
	unsigned int writeSize; //!< Number of bytes to be written to the I2C slave.
	unsigned int readSize; //!< Number of bytes to be read from the I2C slave.
	unsigned char *writeData; //!< Memory location of the data to be written to the I2C slave.
	volatile unsigned char *readData; //!< Memory location to store the data read from the I2C slave.
	portTickType writeReadDelay; //!< A delay inserted between writing to an I2C slave and reading back from it.
} I2Ctransfer;

 */

//WIP
/**
 * FROM INTERFACE
 * ifroute->iface->driver_data (SELF REFERENCE - NOT USED), frame
 *
 * TO HAL DRIVER
 * unsigned int slaveAddress, unsigned char *data, unsigned int size
 *
 * @param driver_data
 * @param data
 * @param data_length
 * @return
 */
static int i2c_driver_tx(void *driver_data, csp_i2c_frame_t * frame) {

	i2c_context_t * ctx = driver_data;
	I2CtransferStatus txResult = I2C_write(frame->dest, frame->data, frame->len);
	if (txResult == 0) {
		csp_buffer_free(frame);
		return CSP_ERR_NONE;
	}
	csp_log_error("I2C Driver write error code: %d", txResult);
	return CSP_ERR_TX;
}

//TODO
static void i2c_driver_rx(void * user_data, uint8_t * data, size_t data_size, void * pxTaskWoken) {

	//kiss_context_t * ctx = user_data;
	//csp_i2c_rx(&ctx->iface, data, data_size, NULL);
}

int csp_I2C_start_and_add_I2C_interface(unsigned int i2cBusSpeed_Hz,unsigned int i2cTransferTimeout, const char * ifname, csp_iface_t ** return_iface) {

	if (ifname == NULL) {
		ifname = CSP_IF_I2C_DEFAULT_NAME;
	}

	csp_log_info("INIT %s: Speed: %d hz, Timeout: %d ",
			ifname, i2cBusSpeed_Hz, i2cTransferTimeout);

	i2c_context_t * ctx = csp_calloc(1, sizeof(*ctx));
	if (ctx == NULL) {
		return CSP_ERR_NOMEM;
	}

	strncpy(ctx->name, ifname, sizeof(ctx->name) - 1);
	ctx->iface.name = ctx->name;
	ctx->iface.driver_data = ctx;
	ctx->iface.interface_data = &ctx->ifdata;
	ctx->ifdata.tx_func = i2c_driver_tx;

	int res = csp_i2c_add_interface(&ctx->iface);
	if (res == CSP_ERR_NONE) {
		res = I2C_start(i2cBusSpeed_Hz, i2cTransferTimeout);
		if(res != 0) {
			TRACE_FATAL("\n\r I2Ctest: I2C_start returned: %d! \n\r", res);
		}
		xTaskHandle taskQueuedI2Ctest3Handle;
		csp_thread_create(csp_i2c_rx, "i2c_rx", 1024, NULL, configMAX_PRIORITIES-2, &taskQueuedI2Ctest3Handle);
		int retValInt = 0;
	}

	if (return_iface) {
		*return_iface = &ctx->iface;
	}

	return res;
}
