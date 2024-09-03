#include <stdio.h>
#include <stdint.h>
#include "mxc_device.h"
#include "mxc_sys.h"
#include "nvic_table.h"
#include "i2c.h"
#include "i2c_regs.h"
#include "mxc_delay.h"
#include "tmr.h"
#include "dev_i2c.h"
// #include "dev_i2c.c"
#include "TPS22994.h"
#include "stdbool.h"
#include "math.h"
#include "string.h"

/***** Globals *****/

volatile unsigned long mode_tick_count = 0;

typedef enum
{
	FAILED,
	PASSED
} test_t;

// TPS22994 specific globals
uint8_t TPS22994_regs[256]; // represents all the max17261 registers.

// General firmware globals
int elapsed_time_multiplier = 1;

/***** Functions *****/

/**
 * I2C functions used in this code
 */

/***** Function definitions *****/

/********************************************************************************************************/
TPS22994_Error_t TPS22994_write_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
	uint16_t i;
	uint8_t i2c_data[256];

	for (i = 0; i < len; i++)
	{
		i2c_data[i] = *(reg_data + i);
	}

	const int res = (MAX32665_I2C_Write(dev_addr, reg_addr, i2c_data, len));
    return res == E_NO_ERROR ? TPS22994_ERROR_ALL_OK : TPS22994_ERROR_LS_ERROR;
}
/********************************************************************************************************/

/********************************************************************************************************/
TPS22994_Error_t TPS22994_read_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{

	const int res = (MAX32665_I2C_Read(dev_addr, reg_addr, reg_data, len));
    return res == E_SUCCESS ? TPS22994_ERROR_ALL_OK : TPS22994_ERROR_LS_ERROR;
}
/********************************************************************************************************/

/********************************************************************************************************/
bool TPS22994_write_verify_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t num_of_bytes)
{
	bool is_verified = false;
	int i = 0;
	uint8_t read_data[256];
	while (!is_verified)
	{
		TPS22994_write_reg(dev_addr, reg_addr, reg_data, num_of_bytes);
		// delay 3ms with timer 1
		MXC_Delay(3000);
		TPS22994_read_reg(dev_addr, reg_addr, &read_data[0], num_of_bytes);
		printf("write_and_verify reg_data = ");
		for (i = 0; i < num_of_bytes; i++)
		{
			printf("%02X", *(reg_data + i));
			if (read_data[i] != *(reg_data + i))
			{
				is_verified = false;
				return is_verified;
			}
		}
		printf("\n");
		is_verified = true;
	}
	return is_verified;
}
/********************************************************************************************************/

/********************************************************************************************************/
TPS22994_Error_t TPS22994_i2c_test(void)
{
	uint8_t address;
	printf("\n******** This example finds if MAX17261 is connected to I2C line *********\r\n");
	int error;

	// Setup the I2CM
	error = MXC_I2C_Init(I2C_MASTER, 1, 0);
	if (error != TPS22994_ERROR_ALL_OK)
	{
		printf("-->Failed master\r\n");
		return TPS22994_ERROR_LS_ERROR;
	}
	else
	{
		printf("\n-->I2C Master Initialization Complete\r\n");
	}

	printf("-->Scanning started\r\n");
	MXC_I2C_SetFrequency(I2C_MASTER, I2C_FREQ);

	mxc_i2c_req_t reqMaster;
	reqMaster.i2c = I2C_MASTER;
	reqMaster.addr = TPS22994_I2C_ADDR;
	reqMaster.tx_buf = NULL;
	reqMaster.tx_len = 0;
	reqMaster.rx_buf = NULL;
	reqMaster.rx_len = 0;
	reqMaster.restart = 0;
	reqMaster.callback = NULL;

	if ((MXC_I2C_MasterTransaction(&reqMaster)) == TPS22994_ERROR_ALL_OK)
	{
		address = TPS22994_I2C_ADDR;
		printf("\nFound slave ID %03d; 0x%02X\r\n", address, address);
	}
	else
	{
		printf("-->No slave device found\r\n");
		return TPS22994_ERROR_LS_ERROR;
	}
	MXC_Delay(MXC_DELAY_MSEC(200));
	return TPS22994_ERROR_ALL_OK;
}
/********************************************************************************************************/

/********************************************************************************************************/
TPS22994_Error_t TPS22994_start(uint8_t dev_addr, uint8_t reg_addr, uint8_t reg_data, uint16_t num_of_bytes)
{
	int error;
	// Setup the I2CM
	error = MXC_I2C_Init(I2C_MASTER, 1, 0);
	if (error != TPS22994_ERROR_ALL_OK)
	{
		printf("-->Failed master\r\n");
		return TPS22994_ERROR_LS_ERROR;
	}
	else
	{
		printf("\n-->I2C Master Initialization Complete\r\n");
	}

	const int res = (TPS22994_write_reg(dev_addr, reg_addr, &reg_data, num_of_bytes));
    
    if(res != TPS22994_ERROR_ALL_OK)
	{
		printf("Could not write to control register\r\n"); 
    }
	return res == 0 ? TPS22994_ERROR_ALL_OK : TPS22994_ERROR_LS_ERROR;
	
}

/**********************************************************************************************************/


/**********************************************************************************************************/
TPS22994_Error_t TPS22994_Channel_Off(uint8_t dev_addr, uint8_t Channel_num)
{
	int error;	
	error = TPS22994_read_reg(dev_addr, TPS22994_REG_CONTROL, &TPS22994_regs[0], 1);
	if(error != TPS22994_ERROR_LS_ERROR)
	{
		TPS22994_regs[0] &= ~(1 << (Channel_num & 0xF) );
	}

	error = TPS22994_write_reg(dev_addr, TPS22994_REG_CONTROL, &TPS22994_regs[0], 1);
	if(error != TPS22994_ERROR_LS_ERROR )
	{
		printf("Channel number %d is tunred OFF", Channel_num);
	}
	return TPS22994_ERROR_ALL_OK;

}
/**********************************************************************************************************/

/**********************************************************************************************************/
TPS22994_Error_t TPS22994_Channel_On(uint8_t dev_addr, uint8_t Channel_num)
{
	int error;	
	error = TPS22994_read_reg(dev_addr, TPS22994_REG_CONTROL, &TPS22994_regs[0], 1);
	if(error != TPS22994_ERROR_LS_ERROR)
	{
		TPS22994_regs[0] |= ~(1 << (Channel_num & 0xF) );
	}

	error = TPS22994_write_reg(dev_addr, TPS22994_REG_CONTROL, &TPS22994_regs[0], 1);
	if(error != TPS22994_ERROR_LS_ERROR )
	{
		printf("Channel number %d is tunred OFF", Channel_num);
	}
	return TPS22994_ERROR_ALL_OK;

}