/*
 * BME688.c
 *
 *  Created on: Jun 17, 2024
 *      Author: VKarra
 */

#include "BME688.h"
#include "i2c.h"
#include <stddef.h> // for NULL

/***** Definitions *****/


#define BME688_TX_BUFF_LEN (1u)
uint8_t tx_buff[];

#define BME688_RX_BUFF_LEN (1u)
uint8_t rx_buff[BME688_RX_BUFF_LEN];

/**********************************/
//Register Definitions
/*********************************/
//Writes to BME688
int8_t BME688_Write(uint8_t reg_addr, uint8_t tx_data, uint8_t tx_len)
{
	// the format for reading is [addr, dummy data, data_read]
	tx_buff[0] = reg_addr;
	tx_buff[1] = tx_data;


	mxc_i2c_req_t req;
	req.i2c =  MXC_I2C0_BUS0;
	req.addr = I2C_ADDR;
	req.tx_buf = tx_buff;
	req.tx_len = tx_len;
	req.rx_buf = NULL;
	req.rx_len = 0;
	req.restart = 0;
	req.callback = NULL;

	MXC_I2C_MasterTransaction(&req);
	return 0;
}

//Reads from the BME688
int8_t BME688_Read(uint8_t reg_addr, uint8_t *data, uint8_t rx_len)
{
	// the format for reading is [addr, dummy data, data_read]
	tx_buff[0] = reg_addr;


	mxc_i2c_req_t req;
	req.i2c =  MXC_I2C0_BUS0;
	req.addr = I2C_ADDR;
	req.tx_buf = tx_buff;
	req.tx_len = BME688_TX_BUFF_LEN;
	req.rx_buf = data;
	req.rx_len = rx_len;
	req.restart = 0;
	req.callback = NULL;

	MXC_I2C_MasterTransaction(&req);
	return BME68X_OK;
}




//Looks for all the available I2C devices
int I2C_device_scan()
{
	int error;
	uint8_t counter = 0;


	printf("-->Scanning started\n");

	mxc_i2c_req_t reqMaster;
	reqMaster.i2c = MXC_I2C0_BUS0;
	reqMaster.addr = 0;
	reqMaster.tx_buf = NULL;
	reqMaster.tx_len = 0;
	reqMaster.rx_buf = NULL;
	reqMaster.rx_len = 0;
	reqMaster.restart = 0;
	reqMaster.callback = NULL;

	for (uint8_t address = 8; address < 120; address++)
	{
		reqMaster.addr = address;
		printf(".");

		if ((MXC_I2C_MasterTransaction(&reqMaster)) == 0)
		{
			printf("\nFound slave ID %03d; 0x%02X\n", address, address);
			counter++;
		}
		MXC_Delay(MXC_DELAY_MSEC(200));
	}

	printf("\n-->Scan finished. %d devices found\n", counter);
	return E_NO_ERROR;
}


//Writing OxE0 to 0XB6 register soft resets the devices which is same as power on reset
int BME688_soft_reset()
{
	int result;
	uint8_t reg_addr = BME68X_REG_SOFT_RESET;

	/* 0xb6 is the soft reset command */
	uint8_t soft_rst_cmd = BME68X_SOFT_RESET_CMD;

	result = BME688_Write(reg_addr, soft_rst_cmd, 2);
	MXC_Delay(MXC_DELAY_MSEC(5));
	return result;

}

static int read_variant_id()
{
	int8_t rslt;
	uint8_t reg_data = 0;

	/* Read variant ID information register */
	rslt = BME688_Read(BME68X_REG_VARIANT_ID, &reg_data, 1);

	if (rslt != BME68X_OK)
	{
		printf("Error reading variant ID\r\n");
	}

	return BME68X_OK;
}

int8_t bme68x_init()
{
	int result;
	uint8_t chip_id;
	(void) BME688_soft_reset(); //Perform soft reset

	//Now we read the chip ID
	result = BME688_Read(BME68X_REG_CHIP_ID, &chip_id, 1);

	if(result == BME68X_OK )
	{
		if(chip_id == BME68X_CHIP_ID)
		{
			result = read_variant_id();
			if(result == BME68X_OK)
			{

			}
		}
	}





}
