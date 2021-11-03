/*!
 * @file	FRAM.h
 * @brief	Driver for reading and writing data from/to the FRAM.
 * @note	Please don't call any of those functions from an interrupt context
 */

#ifndef FRAM_H_
#define FRAM_H_

#define FRAM_MAX_ADDRESS	0x0003FFFF //!< Highest memory address available in FRAM memory

/*!
 * Used by the FRAM_protectBlocks function to protect certain blocks of the FRAM from write operations.
 * blockProtect Values: 0 = None,
 * 						1 = upper 1/4 protected (0x30000 to 0x3FFFF),
 * 						2 = upper 1/2 protected (0x20000 to 0x3FFFF),
 * 						3 = Entire FRAM protected (0x00000 to 0x3FFFF)
 */
typedef union __attribute__((__packed__)) _FRAMblockProtect {
	unsigned char rawValue;
	struct __attribute__((__packed__)) {
		unsigned char	reserved1 : 2,
						blockProtect: 2,
						reserved2 : 4;
	} fields;
} FRAMblockProtect;

/*!
 * Initializes the FRAM driver and the SPI driver if its not already initialized.
 * @return -2 if initializing the SPI driver fails,
 * -1 If creating semaphores to control access to the FRAM fails.
 * 0 on success.
 */
int FRAM_start(void);

/*!
 * De-initializes the FRAM driver.
 */
void FRAM_stop(void);

/*!
 * Writes data to the FRAM.
 * @param data Address where data to be written is stored.
 * @param address Location in the FRAM where data should be written.
 * @param size Number of bytes to write.
 * @return -2 if the specified address and size are out of range,
 * -1 if obtaining lock for FRAM access fails,
 * 0 on success.
 */
int FRAM_write(unsigned char *data, unsigned int address, unsigned int size);

/*!
 * Reads data from the FRAM.
 * @param data Address where read data will be stored, this location must be able to accommodate size bytes.
 * @param address Location in the FRAM from which the data should be read.
 * @param size Number of bytes to read.
 * @return -2 if the specified address and size are out of range of the FRAM space.
 * -1 if obtaining lock for FRAM access fails,
 * 0 on success.
 */
int FRAM_read(unsigned char *data, unsigned int address, unsigned int size);


/*!
 * Writes data to the FRAM and reads it back to verify that it was written correctly.
 * @param data Address where data to be written is stored.
 * @param address Location in the FRAM where data should be written.
 * @param size Number of bytes to write.
 * @return -3 written data didn't match the data read back from the FRAM,
 * -2 if the specified address and size are out of range,
 * -1 if obtaining lock for FRAM access fails,
 * 0 on success.
 */
int FRAM_writeAndVerify(unsigned char *data, unsigned int address, unsigned int size);

/*!
 * Write protects or un-protects blocks of the FRAM.
 * @param blocks FRAMblockProtect structure specifying the blocks to protect.
 * @return -3 if the effective FRAM block protect configuration differs from the requested value
 * -2 if obtaining lock for FRAM access fails,
 * -1 if the SPI transfer fails,
 * 0 on success.
 */
int FRAM_protectBlocks(FRAMblockProtect blocks);

/*!
 * Retrieves the Device ID of the FRAM chip.
 * @param deviceID Pointer to where the retrieved device ID should be stored
 * @return -1 if obtaining lock for FRAM access fails,
 * 0 on success.
 *
 * @note This function will copy 9 bytes to the memory location indicated by deviceID
 */
int FRAM_getDeviceID(unsigned char *deviceID);

#endif /* FRAM_H_ */
