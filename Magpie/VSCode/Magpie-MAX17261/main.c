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

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "led.h"
#include "pb.h"
#include "mxc_delay.h"
#include "gcr_regs.h"
#include "mxc_sys.h"
#include "board.h"
#include "spi.h"
#include <stdarg.h>
#include "nvic_table.h"
#include "gpio.h"
#include "mxc_device.h"
#include "dev_gpio_config.h" //SwiftV2 specific GPIO configuration
#include "tmr.h"
#include "MAX17261.h"


/*****************************************************************************
MAX17261 Globals
******************************************************************************/
uint8_t counter = 0;
volatile unsigned long timer_count = 0;
int error, tempdata1;
int retvalue = 0;
char log_string[256] = {0}; // Used to print the paramter values on terminal
// MAX17261 specific globals
uint8_t max17261_regs1[256]; // represents all the max17261 registers.
unsigned long timer_count_value;
char *OutputString;

/**
 * @brief    Fuel_gauge_data_collect. This function reads the fuel gauge registers and outputs to OutputString Variable
 * @param[in] None
 * @param[out]  char * OutputString.  This is the output string of fuel gauge readings.
 *
 ****************************************************************************/
// void Fuel_gauge_data_collect()
// {
// 	// local variables
// 	uint16_t tempdata = 0;
// 	int16_t stempdata = 0;
// 	char tempstring[20] = {0};
// 	double timerh = 0; // timerh + timer = time since last por
// 	double timer = 0;

// 	// Read repsoc
// 	tempdata = 0;
// 	memset(tempstring, 0, 20);
// 	tempdata = max17261_read_repsoc();
// 	printf("State Of Charge = %i\r\n", tempdata);

// 	// Read VCell
// 	tempdata = 0;
// 	memset(tempstring, 0, 20);
// 	max17261_read_reg(MAX17261_I2C_ADDR, VCell_addr, &max17261_regs1[0x00], 2); // Read register
// 	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
// 	tempdata = (double)((1.25 * tempdata / 16) / 1000);
// 	printf("voltage per cell of the batterypack = %lf\r\n", tempdata);

// 	// Read AvgVCell
// 	tempdata = 0;
// 	memset(tempstring, 0, 20);
// 	max17261_read_reg(MAX17261_I2C_ADDR, AvgVCell_addr, &max17261_regs1[0x00], 2); // Read register
// 	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
// 	printf("AVerage of VCell register readings = %lf\r\n", (double)((1.25 * tempdata / 16) / 1000));

// 	tempdata = 0;
// 	memset(tempstring, 0, 20);
// 	max17261_read_reg(MAX17261_I2C_ADDR, Temp_addr, &max17261_regs1[0x00], 2); // Read register
// 	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];				   // put register into unsigned int
// 	stempdata = (int16_t)tempdata;											   // this is going to be signed number, so convert to 16-bit int
// 	// to calculate temperature, see Table 2 on page 16 of max17261 data sheet
// 	printf("Temp register = %f\r\n", (double)stempdata / 256);

// 	// Read AvgTA (temperature in degrees C)
// 	tempdata = 0;
// 	memset(tempstring, 0, 20);
// 	max17261_read_reg(MAX17261_I2C_ADDR, AvgTA_addr, &max17261_regs1[0x00], 2); // Read register
// 	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
// 	stempdata = (int16_t)tempdata; // this is going to be signed number, so convert to 16-bit int
// 	printf("AvgTA register = %f\r\n", (double)stempdata / 256);

// 	// Read Current (in mA)
// 	tempdata = 0;
// 	memset(tempstring, 0, 20);
// 	max17261_read_reg(MAX17261_I2C_ADDR, Current_addr, &max17261_regs1[0x00], 2); // Read register
// 	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
// 	stempdata = (int16_t)tempdata; // this is going to be signed number, so convert to 16-bit int
// 	printf("Current register = %f\r\n", (double)stempdata * (double)78.125 / 1000);
// 	// printf("Current register = %f\r\n", (double)stempdata * (double)156.25 / 1000);

// 	// Read AvgCurrent
// 	tempdata = 0;
// 	memset(tempstring, 0, 20);
// 	max17261_read_reg(MAX17261_I2C_ADDR, AvgCurrent_addr, &max17261_regs1[0x00], 2); // Read register
// 	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
// 	stempdata = (int16_t)tempdata; // this is going to be signed number, so convert to 16-bit int
// 	//  printf("AvgCurrent register = %f\r\n", (double)stempdata * (double)156.25 / 1000);
// 	printf("AvgCurrent register = %f\r\n", (double)stempdata * (double)78.125 / 1000);

// 	// Read TTF
// 	tempdata = 0;
// 	memset(tempstring, 0, 20);
// 	max17261_read_reg(MAX17261_I2C_ADDR, TTF_addr, &max17261_regs1[0x00], 2); // Read register
// 	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
// 	stempdata = (int16_t)tempdata; // this is going to be signed number, so convert to 16-bit int
// 	printf("TTF register = %i\r\n", stempdata);

// 	// Read RepCap
// 	tempdata = 0;
// 	memset(tempstring, 0, 20);
// 	max17261_read_reg(MAX17261_I2C_ADDR, RepCap_addr, &max17261_regs1[0x00], 2); // Read register
// 	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
// 	// printf("RepCap register = 0x%04x\r\n",tempdata);
// 	stempdata = (int16_t)tempdata; // this is going to be signed number, so convert to 16-bit int
// 	// printf("RepCap register = %f\r\n",(double)stempdata/2);

// 	// Read VFRemCap
// 	tempdata = 0;
// 	memset(tempstring, 0, 20);
// 	max17261_read_reg(MAX17261_I2C_ADDR, VFRemCap_addr, &max17261_regs1[0x00], 2); // Read register
// 	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
// 	stempdata = (int16_t)tempdata; // this is going to be signed number, so convert to 16-bit int
// 	// printf("VFRemCap register = %f\r\n",(double)stempdata/2);

// 	// Read MixCap
// 	tempdata = 0;
// 	memset(tempstring, 0, 20);
// 	max17261_read_reg(MAX17261_I2C_ADDR, MixCap_addr, &max17261_regs1[0x00], 2); // Read register
// 	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
// 	stempdata = (int16_t)tempdata; // this is going to be signed number, so convert to 16-bit int
// 	//printf("MixCap register = %f\r\n",(double)stempdata*(double)0.5);

// 	// Read FullCapRep
// 	tempdata = 0;
// 	memset(tempstring, 0, 20);
// 	max17261_read_reg(MAX17261_I2C_ADDR, FullCapRep_addr, &max17261_regs1[0x00], 2); // Read register
// 	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
// 	stempdata = (int16_t)tempdata; // this is going to be signed number, so convert to 16-bit int
// 	// printf("FullCapRep register = %f\r\n",(double)stempdata*(double)0.5);

// 	// Read FullCapNom
// 	tempdata = 0;
// 	memset(tempstring, 0, 20);
// 	max17261_read_reg(MAX17261_I2C_ADDR, FullCapNom_addr, &max17261_regs1[0x00], 2); // Read register
// 	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
// 	stempdata = (int16_t)tempdata; // this is going to be signed number, so convert to 16-bit int
// 	//printf("FullCapNom register = %f\r\n",(double)stempdata*(double)0.5);

// 	// Read QResidual
// 	tempdata = 0;
// 	memset(tempstring, 0, 20);
// 	max17261_read_reg(MAX17261_I2C_ADDR, QResidual_addr, &max17261_regs1[0x00], 2); // Read register
// 	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
// 	stempdata = (int16_t)tempdata; // this is going to be signed number, so convert to 16-bit int
// 	printf("QResidual register = %f\r\n", (double)stempdata * (double)0.5);
// 	// printf("QResidual register = %f\r\n", (double)stempdata * (double)0.5);

// 	// Read REPSOC
// 	tempdata = 0;
// 	memset(tempstring, 0, 20);
// 	max17261_read_reg(MAX17261_I2C_ADDR, REPSOC_addr, &max17261_regs1[0x00], 2); // Read register
// 	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
// 	// printf("REPSOC register = %f\r\n",(double)tempdata/256);

// 	// Read AvSOC
// 	tempdata = 0;
// 	memset(tempstring, 0, 20);
// 	max17261_read_reg(MAX17261_I2C_ADDR, AvSOC_addr, &max17261_regs1[0x00], 2); // Read register
// 	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
// 	// printf("AvSOC register = %f\r\n",(double)tempdata/256);

// 	// Read Timer
// 	tempdata = 0;
// 	memset(tempstring, 0, 20);
// 	max17261_read_reg(MAX17261_I2C_ADDR, Timer_addr, &max17261_regs1[0x00], 2); // Read register
// 	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
// 	timer = (double)tempdata * (double)175.8 / 3600000; // see User Guide page 34 for Timer register
// 	// printf("Timer register (hours) = %f\r\n",timer);

// 	// Read TimerH
// 	tempdata = 0;
// 	memset(tempstring, 0, 20);
// 	max17261_read_reg(MAX17261_I2C_ADDR, TimerH_addr, &max17261_regs1[0x00], 2); // Read register
// 	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
// 	timerh = (double)tempdata * (double)3.2; // see User Guide page 34 for TimerH register calculation in hours
// 	// printf("TimerH register (hours) = %f\r\n",timerh); // see page User Guide page 34

// 	// Read QH
// 	tempdata = 0;
// 	memset(tempstring, 0, 20);
// 	max17261_read_reg(MAX17261_I2C_ADDR, QH_addr, &max17261_regs1[0x00], 2); // Read register
// 	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
// 	printf("QH register(coulomb count) = %f\r\n", (double)tempdata * (double)0.25); // We have 20mOhm sense resistor so the LSB is 0.25
// 	// printf("QH register(coulomb count) = %f\r\n", (double)tempdata * (double)0.25 / 3600000); // We have 20mOhm sense resistor so the LSB is 0.25

// 	// Read AvCap
// 	tempdata = 0;
// 	memset(tempstring, 0, 20);
// 	max17261_read_reg(MAX17261_I2C_ADDR, AvCap_addr, &max17261_regs1[0x00], 2); // Read register
// 	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
// 	//	printf("AvCap register = %f\r\n",(double)tempdata * (double)0.5);

// 	// Read MixSOC
// 	tempdata = 0;
// 	memset(tempstring, 0, 20);
// 	max17261_read_reg(MAX17261_I2C_ADDR, MixSOC_addr, &max17261_regs1[0x00], 2); // Read register
// 	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
// 	//	printf("MixSOC register = %f\r\n",(double)tempdata/256);
// }

/******************************************************************************
Your main code starts here
******************************************************************************/
int main(void)
{

	LDO_startup();
	//*****************************************************************************/
	// MAX17261 Code
	//*****************************************************************************/
	// check if slave is responding on I2C
	max17261_i2c_test();
	MXC_Delay(2000);

	max17261_soft_reset(); // perform soft reset
	MXC_Delay(250);
	if (max17261_por_detected())
	{
		// load max17261 configuration
		printf("POR detected. Load fuel gauge config\r\n");
		max17261_wait_dnr();
		max17261_config_ez();
		max17261_clear_por_bit();
	}
	else
	{
		printf("POR not detected\r\n");
	}
	// debugPrint("MAX17261 driver \r\n");
	printf("MAX17261 driver \r\n");

	// main spin loop.  Please note that the Learn and Save Parameters section (step 3.5 and
	// 3.6 of the max17261 Software Implementation Guide) are not implemented in this solution.
	// If many power-on-resets are expected occur, it is a good idea to implement.
	int count = 0;
	while (1)
	{

		printf("/================================/\r\n");
		printf("Data poll count is %d\r\n", count);
		printf("/================================/\r\n");
		Fuel_gauge_data_collect();
		printf("/================================/\r\n");
		MXC_Delay(500000);
		count = count + 1;
	}
}
