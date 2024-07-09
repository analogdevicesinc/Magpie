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
 * @brief       Example code for talking to MAX17261 using I2C master
 * @details     This example uses the I2C Master to found addresses of the I2C Slave devices 
 *               and will talk to MAX17261 for configuring and reading it
 *
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
#include "i2c_regs.h"
#include <stdbool.h>
#include <math.h>
#include "tmr.h"
#include "MAX17261.h"
#include "pb.h"
#include "gpio.h"



/***** Globals *****/
uint8_t counter = 0;
volatile unsigned long timer_count = 0;



// *****************************************************************************
int main()
{
	printf("\n******** MAX17261 fuel gauge code*********\n");

	int error;


	int retvalue = 0;
	char log_string[256] = {0};
	timer_count = 0;
	MAX32665_I2C_Init();

	// perform soft reset
	max17261_soft_reset();
	MXC_TMR_Delay(MXC_TMR1,100); //Delay 100ms

	// check if slave is responding on I2C
	retvalue = max17261_i2c_test(); //This scan should find the fuel gauge at the address 0x36
	printf("max17261 I2C Test = %i\n", retvalue);

	while (retvalue < E_NO_ERROR)
	{
		// print out error -- usually -9 for "E_COMM_ERR" when I2C is connected incorrectly
		printf("max17261 is not responding on I2C bus. Check I2C connections.\n");
		// wait 2 seconds
		//  TMR_Delay(MXC_TMR0, MSEC(2000), &sys_tmr_cfg); // delay 1 second
		MXC_TMR_Delay(MXC_TMR0,2000);
		max17261_soft_reset();  // perform soft reset
		// TMR_Delay(MXC_TMR0, MSEC(250), &sys_tmr_cfg); // delay 250 ms
		MXC_TMR_Delay(MXC_TMR0,250);
	}

	if (max17261_por_detected())
	{
		// load max17261 configuration
		printf("POR detected. Load fuel gauge config\n");
		max17261_wait_dnr();
		max17261_config_ez();
		max17261_clear_por_bit();
	}


	// main spin loop.  Please note that the Learn and Save Parameters section (step 3.5 and
	// 3.6 of the max17261 Software Implementation Guide) are not implemented in this solution.
	// If many power-on-resets are expected occur, it is a good idea to implement.
	while (1)
	{

		if ((timer_count == 0))
		{
			// Check on fuel gauge to see if power-on reset occurred.  If so, configure it.
			if (max17261_por_detected())
			{
				// load max17261 configuration
				printf("POR detected. Load fuel gauge config\n");
				max17261_wait_dnr();
				max17261_config_ez();
				max17261_clear_por_bit();
			}
			else
			{
				printf("POR not detected\n");
			}
			Fuel_gauge_data_collect(timer_count,&log_string[0]);
			printf("log_string = %s", &log_string[0]); // print log string to debug console
			printf("\n");
			timer_count = timer_count + 1; // increment the elapsed timer count value (once per while loop iteration)

		}
		retvalue = max17261_i2c_test();
		if (retvalue < E_NO_ERROR)
		{
			printf("max17261 is not responding on I2C bus. Error: %i\n", retvalue);
			max17261_soft_reset();  // perform soft reset
		}
		//TMR_Delay(MXC_TMR0, MSEC(1000), &sys_tmr_cfg); // delay 1 second
		MXC_TMR_Delay(MXC_TMR0,1000);




	}
	return 0;
}



