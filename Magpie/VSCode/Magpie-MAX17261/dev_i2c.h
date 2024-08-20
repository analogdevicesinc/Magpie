/*
 * dev_i2C.h
 *
 *  Created on: Jul 2, 2024
 *      Author: VKarra
 */

#ifndef DEV_I2C_H_
#define DEV_I2C_H_


#include <stdio.h>
#include <stdint.h>

//====================================
//I2C defines
//====================================
#define I2C_MASTER MXC_I2C0_BUS0
#define I2C_FREQ 100000 // 100kHZ
#define I2C_CONFIG      1           //0: I2C Slave Configuration,   1: *I2C Master Configuration


/**
* @brief        Initialize I2C object
*
* @return       0 (Success), -1 (Failure)
*/
int8_t MAX32665_I2C_Init();

/**
* @brief        Write to a specified register of I2C Slave
* @param[in]    dev_addr - I2C device address
* @param[in]    reg_addr - I2C slave register address to write
* @param[out]	data - byte value to write to address
*
* @return       0 (Success), -1 (Failure)
*/
int8_t MAX32665_I2C_Write(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);

/**
* @brief        Read from a specified register of I2C Slave
* @param[in]    dev_addr - I2C slave device address
* @param[in]    reg_addr - I2C Slave register address to read
* @param[out]	data - pointer to location where data should be stored
*
* @return       0 (Success), -1 (Failure)
*/
int8_t MAX32665_I2C_Read(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);



/**
* @brief        Looks for all the I2C devices on the I2C line
* @return       0 (Success), -1 (Failure)
*/
int I2C_device_scan();//  Looks for all the devices on the I2C line



#endif /* DEV_I2C_H_ */
