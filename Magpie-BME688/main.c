/******************************************************************************
 *
 * Copyright (C) 2022-2023 Maxim Integrated Products, Inc. All Rights Reserved.
 * (now owned by Analog Devices, Inc.),
 * Copyright (C) 2023 Analog Devices, Inc. All Rights Reserved. This software
 * is proprietary to Analog Devices, Inc. and its licensors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************/

/**
 * @file        main.c
 * @brief     Example code for scanning the available addresses on an I2C bus
 * @details     This example uses the I2C Master to found addresses of the I2C Slave devices 
 *              connected to the bus. You must connect the pull-up jumpers (JP21 and JP22) 
 *              to the proper I/O voltage.
 */

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "mxc_device.h"
#include "mxc_delay.h"
#include "nvic_table.h"
#include "i2c.h"
#include "board.h"
#include "BME688.h"


#define I2C_FREQ 100000 // 100kHZ
typedef enum { FAILED, PASSED } test_t;

/***** Globals *****/

// *****************************************************************************
int main()
{
	printf("\n******** I2C example to read BME688 *********\n");
	printf("\nsame bus as I2C0 (SCL - P0.6, SDA - P0.7).");

	int error;
	uint8_t Chip_ID, Variant_ID;




	//Setup the I2CM
	error = MXC_I2C_Init(MXC_I2C0_BUS0, 1, 0);
	if (error != E_NO_ERROR)
	{
		printf("-->Failed master\n");
		return FAILED;
	}
	else
	{
		printf("\n-->I2C Master Initialization Complete\n");
	}


	MXC_I2C_SetFrequency(MXC_I2C0_BUS0, I2C_FREQ);

	I2C_device_scan();

	printf("Make sure the scanned address matches the BME688 device address which is 0x76\r\n");

	while(1)
	{
		BME688_soft_reset();
		//MXC_Delay(MXC_DELAY_MSEC(500));
		//BME688_Read(BME68X_REG_CHIP_ID, &Chip_ID);
		BME688_Read(BME68X_REG_VARIANT_ID, &Variant_ID,1);
		printf("Variant ID: 0x%x  \r\n",Variant_ID);
		MXC_Delay(MXC_DELAY_MSEC(500));
	}


	return E_NO_ERROR;
}
