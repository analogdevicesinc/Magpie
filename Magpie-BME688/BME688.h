/*
 * BME688.h
 *
 *  Created on: Jun 17, 2024
 *      Author: VKarra
 */

#ifndef BME688_H_
#define BME688_H_


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "mxc_device.h"
#include "mxc_delay.h"
#include "nvic_table.h"
#include "BME688_defs.h"





//I2C defines

int I2C_device_scan();//  Looks for all the devices on the I2C line

int8_t BME688_Write(uint8_t reg_addr, uint8_t data, uint8_t len); //Writes to BME688

int8_t BME688_Read(uint8_t reg_addr, uint8_t *data, uint8_t rx_len); //Reads from BME688

int BME688_soft_reset(); //Writing OxE0 to 0XB6 register soft resets the devices which is same as power on reset

int8_t bme68x_init();
static int read_variant_id(); //This function reads variant ID of the device





#endif /* BME688_H_ */
