/*
 * dbgu_test.h
 *
 *  Created on: 11 aug. 2015
 *      Author: malv
 */

#ifndef DBGU_TEST_H_
#define DBGU_TEST_H_

#include <stdio.h>

/**
 *  @brief       Initializes the DBGU on interrupt based reception.
 *  @param[in]   mode Operating mode to configure.
 *  @param[in]   baudrate Desired baudrate (e.g. 115200).
 *  @param[in]   mck Frequency of the system master clock in Hz.
 */
void DBGU_Init( unsigned int mode, unsigned int baudrate, unsigned int mck );

/**
 *  @brief    Initializes the DBGU on interrupt based reception.
 *  @return   value 1 if a character can be read in DBGU.
 */
unsigned int DBGU_IntIsRxReady(void);

/**
 *  @brief    Reads and returns a character from the DBGU.
 *  @note     This function is synchronous (i.e. uses polling).
 *  @return   Character received.
 */
unsigned char DBGU_IntGetChar(void);

#endif /* DBGU_TEST_H_ */
