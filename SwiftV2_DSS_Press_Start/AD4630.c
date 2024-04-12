/*
 * AD4630.c
 *
 *  Created on: Feb 2, 2024
 *      Author: VKarra
 */
#include <stdio.h>
#include <string.h>
#include "mxc_device.h"
#include "board.h"
#include "gpio.h"
#include"mxc_delay.h"
#include "AD4630.h"
#include "stdint.h"

/******************************************************************************/
/********************** Globals**********************/
/******************************************************************************/

uint8_t master_rx[DATA_LEN];
uint8_t master_tx[DATA_LEN];



/******************************************************************************/
/************************* Functions Definitions ******************************/
/******************************************************************************/


void hardware_reset()
{
	mxc_gpio_cfg_t gpio_out21;
	gpio_out21.port = MXC_GPIO_PORT_OUT0;
	gpio_out21.mask = MXC_GPIO_PIN_OUT21;
	gpio_out21.pad = MXC_GPIO_PAD_NONE;
	gpio_out21.func = MXC_GPIO_FUNC_OUT;
	gpio_out21.vssel = MXC_GPIO_VSSEL_VDDIO;
	gpio_out21.drvstr = MXC_GPIO_DRVSTR_2;
	MXC_GPIO_Config(&gpio_out21);
	MXC_GPIO_OutClr(gpio_out21.port, gpio_out21.mask);
	MXC_Delay(100000);  //100ms
	MXC_GPIO_OutSet(gpio_out21.port, gpio_out21.mask);
	MXC_Delay(100000); //100ms

}

void enable_cnv_signal()
{
	mxc_gpio_cfg_t gpio_out20;
	gpio_out20.port = MXC_GPIO_PORT_OUT0;
	gpio_out20.mask = MXC_GPIO_PIN_OUT20;
	gpio_out20.pad = MXC_GPIO_PAD_NONE;
	gpio_out20.func = MXC_GPIO_FUNC_OUT;
	gpio_out20.vssel = MXC_GPIO_VSSEL_VDDIO;
	gpio_out20.drvstr = MXC_GPIO_DRVSTR_2;
	MXC_GPIO_Config(&gpio_out20);
	MXC_GPIO_OutClr(gpio_out20.port, gpio_out20.mask);
	MXC_Delay(100000);  //100ms
	MXC_GPIO_OutSet(gpio_out20.port, gpio_out20.mask);

}

void disable_cnv_signal()
{
	mxc_gpio_cfg_t gpio_out20;
	gpio_out20.port = MXC_GPIO_PORT_OUT0;
	gpio_out20.mask = MXC_GPIO_PIN_OUT20;
	gpio_out20.pad = MXC_GPIO_PAD_NONE;
	gpio_out20.func = MXC_GPIO_FUNC_OUT;
	gpio_out20.vssel = MXC_GPIO_VSSEL_VDDIO;
	gpio_out20.drvstr = MXC_GPIO_DRVSTR_2;
	MXC_GPIO_Config(&gpio_out20);
	MXC_GPIO_OutClr(gpio_out20.port, gpio_out20.mask);

}


//************************************************************************
//The following functions are used specifically for ADC Configuration purposes

/***********************************************************************/
// @brief Read device register.
// @param dev - The device structure.
// @param reg_addr - The register address.
// @param reg_data - The data read from the register.
// @return 0 in case of success, negative error code otherwise.
/**********************************************************************/

int32_t ad4630_spi_reg_read_4_wire(uint16_t reg_addr, uint8_t reg_data)
{



	int ret = E_NO_ERROR;
	mxc_spi_req_t master_req;
	//Using the GPIO 16
	mxc_gpio_cfg_t gpio_out16;

	gpio_out16.port = MXC_GPIO_PORT_OUT0;
	gpio_out16.mask = MXC_GPIO_PIN_OUT16;
	gpio_out16.pad = MXC_GPIO_PAD_NONE;
	gpio_out16.func = MXC_GPIO_FUNC_OUT;
	gpio_out16.vssel = MXC_GPIO_VSSEL_VDDIO;
	gpio_out16.drvstr = MXC_GPIO_DRVSTR_2;
	ret = MXC_GPIO_Config(&gpio_out16);
	if(ret != E_NO_ERROR)
	{
		printf("\nGPIO INITIALIZATION ERROR\n");
		return E_FAIL;
	}

	MXC_GPIO_OutSet(gpio_out16.port, gpio_out16.mask);
	//Initializing the SPI2 as Master

	ret = MXC_SPI_Init(SPI_MASTER2, 1, 0, 1, 0, SPI_SPEED, MAP_A);
	if (ret != E_NO_ERROR)
	{
		printf("\nSPI MASTER INITIALIZATION ERROR\n");
		return E_FAIL;
	}

	//Setting the data size
	ret = MXC_SPI_SetDataSize(SPI_MASTER2, DATA_SIZE);
	if (ret != E_NO_ERROR)
	{
		printf("\nSPI MASTER INITIALIZATION ERROR\n");
		return E_FAIL;
	}

	//Setting width of the SPI in this case 4- wire SPI for QSPI2 master
	ret = MXC_SPI_SetWidth(SPI_MASTER2, SPI_WIDTH_STANDARD);
	if (ret != E_NO_ERROR)
	{
		printf("\nSPI MASTER INITIALIZATION ERROR\n");
		return E_FAIL;
	}

	ret = MXC_SPI_SetMode(SPI_MASTER2, SPI_MODE_0);
	if (ret != E_NO_ERROR)
	{
		printf("\nSPI MASTER INITIALIZATION ERROR\n");
		return E_FAIL;
	}
	master_req.spi = SPI_MASTER2;
	master_req.txData = (uint8_t *)master_tx;
	master_req.rxData = (uint8_t *)master_rx;
	master_req.txLen = 3;
	master_req.rxLen = 1;
	master_req.ssIdx = SPI_MASTER2_SSIDX;
	master_req.ssDeassert = 1;
	master_req.txCnt = 0;
	master_req.rxCnt = 0;
	master_req.completeCB = NULL;

	//************************************************
	//    SPI Read for ADC procedure
	//     * Declare the address as uint16_t
	//     * Declare read data as uint8_t
	//     * Number of data bytes must be 3 to make 24 clock cycles of SCK
	//*************************************************


	// Separating first byte of 16 bit address and making sure the first bit is 1 for read operation
	master_tx[0] = AD463X_REG_READ| ((reg_addr >> 8) & 0x7F); // Separating first byte of 16 bit address
	master_tx[1] = (uint8_t)reg_addr;  //Separating second byte of address
	master_tx[2] = AD463X_REG_READ_DUMMY;

	//	**** Perform Transaction ****
	MXC_GPIO_OutClr(gpio_out16.port, gpio_out16.mask);
	MXC_SPI_MasterTransaction(&master_req);
	MXC_GPIO_OutSet(gpio_out16.port, gpio_out16.mask);
	reg_data = master_tx[2];

	MXC_SPI_Shutdown(SPI_MASTER2);

	return ret;

}



/***********************************************************************/
// @brief Read device register.
// @param reg_addr - The register address.
// @param reg_data - The data read from the register.
// @return 0 in case of success, negative error code otherwise.
/**********************************************************************/

int32_t ad4630_spi_reg_write_4_wire(uint16_t reg_addr, uint8_t reg_data)
{

	mxc_spi_req_t master_req;
	int ret = E_NO_ERROR;
	mxc_gpio_cfg_t gpio_out16;

	gpio_out16.port = MXC_GPIO_PORT_OUT0;
	gpio_out16.mask = MXC_GPIO_PIN_OUT16;
	gpio_out16.pad = MXC_GPIO_PAD_NONE;
	gpio_out16.func = MXC_GPIO_FUNC_OUT;
	gpio_out16.vssel = MXC_GPIO_VSSEL_VDDIO;
	gpio_out16.drvstr = MXC_GPIO_DRVSTR_2;
	//Using the GPIO 16

	ret = MXC_GPIO_Config(&gpio_out16);
	if(ret != E_NO_ERROR)
	{
		printf("\nGPIO INITIALIZATION ERROR\n");
		return E_FAIL;
	}

	MXC_GPIO_OutSet(gpio_out16.port, gpio_out16.mask);
	//Initializing the SPI2 as Master

	ret = MXC_SPI_Init(SPI_MASTER2, 1, 0, 1, 0, SPI_SPEED, MAP_A);
	if (ret != E_NO_ERROR)
	{
		printf("\nSPI MASTER INITIALIZATION ERROR\n");
		return E_FAIL;
	}

	//Setting the data size
	ret = MXC_SPI_SetDataSize(SPI_MASTER2, DATA_SIZE);
	if (ret != E_NO_ERROR)
	{
		printf("\nSPI MASTER INITIALIZATION ERROR\n");
		return E_FAIL;
	}

	//Setting width of the SPI in this case 4- wire SPI for QSPI2 master
	ret = MXC_SPI_SetWidth(SPI_MASTER2, SPI_WIDTH_STANDARD);
	if (ret != E_NO_ERROR)
	{
		printf("\nSPI MASTER INITIALIZATION ERROR\n");
		return E_FAIL;
	}

	ret = MXC_SPI_SetMode(SPI_MASTER2, SPI_MODE_0);
	if (ret != E_NO_ERROR)
	{
		printf("\nSPI MASTER INITIALIZATION ERROR\n");
		return E_FAIL;
	}

	// **** Initialize Transaction Parameters ****
	master_req.spi = SPI_MASTER2;
	master_req.txData = (uint8_t *)master_tx;
	master_req.rxData = (uint8_t *)master_rx;
	master_req.txLen = 3;
	master_req.rxLen = 1;
	master_req.ssIdx = SPI_MASTER2_SSIDX;
	master_req.ssDeassert = 1;
	master_req.txCnt = 0;
	master_req.rxCnt = 0;
	master_req.completeCB = NULL;


	// Separating first byte of 16 bit address and making sure the first bit is 0 for write operation
	master_tx[0] = ((reg_addr >> 8) & 0x7F);
	master_tx[1] = (uint8_t)reg_addr;  //Separating second byte of address
	master_tx[2] = reg_data;

	//	**** Perform Transaction ****
	MXC_GPIO_OutClr(gpio_out16.port, gpio_out16.mask);
	MXC_SPI_MasterTransaction(&master_req);
	MXC_GPIO_OutSet(gpio_out16.port, gpio_out16.mask);

	MXC_SPI_Shutdown(SPI_MASTER2);
	return ret;
}


/**
 * @brief SPI read device register using a mask.
 * @param reg_addr - The register address.
 * @param mask - The mask.
 * @param data - The data read from the register.
 * @return 0 in case of success, negative error code otherwise.
 */

int32_t ad4630_spi_reg_read_4_wire_masked(uint16_t reg_addr, uint8_t mask, uint8_t data)
{


	uint8_t reg_data = 0x00;
	int ret = E_NO_ERROR;

	ret = ad4630_spi_reg_read_4_wire(reg_addr, reg_data);
	if(ret != E_NO_ERROR)
	{
		return ret;
	}

	data = (reg_data & mask) >> no_os_find_first_set_bit(mask);

	return E_NO_ERROR;
}

/**
 * @brief SPI write device register using a mask.
 * @param reg_addr - The register address.
 * @param mask - The mask.
 * @param data - The register data.
 * @return 0 in case of success, negative error code otherwise.
 */

int32_t ad4630_spi_reg_write_4_wire_masked(uint16_t reg_addr, uint8_t mask, uint8_t data)
{

	int32_t ret;
	uint8_t reg_data = 0;
	ret = ad4630_spi_reg_read_4_wire(reg_addr, reg_data);
	if(ret != 0)
	{
		return ret;

	}

	reg_data &= ~mask;
	reg_data |= data;
	return ad4630_spi_reg_write_4_wire(reg_addr, reg_data);

}

/**
 * @brief Read dummy register to enter configuration mode as per the DS
 * @param reg_addr - The register address.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad4630_enter_config_mode(uint16_t addr, uint8_t data)
{
	uint32_t ret;
	ret = ad4630_spi_reg_read_4_wire(addr,data);
	if(ret != E_NO_ERROR)
	{
		return ret;
	}
}

/**
 * @brief Write and read from scratch pad test data
 * @param reg_addr - The register address.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad4630_scratchpad(uint16_t addr, uint8_t data)
{
	uint8_t read_data = 0x00;
	ad4630_spi_reg_write_4_wire(addr,data);
	ad4630_spi_reg_read_4_wire(addr,read_data);
	return data;
}


/**
 * @brief Set power mode.
 * @param dev - The device structure.
 * @param mode - The power mode.
 * @return 0 in case of success, negative error code otherwise.
 * This is related to register 0X02 in the DS page 43
 */
int32_t ad4630_set_pwr_mode(uint8_t mode)  //mode should be 0x00 for normal mode or 0x01  for low power mode
{
	if ((mode != AD463X_NORMAL_MODE) && (mode != AD463X_LOW_POWER_MODE))
		return -1;
	return ad4630_spi_reg_write_4_wire(AD463X_REG_DEVICE_CONFIG,mode);
}


/**
 * @brief Set drive strength.
 * @param mode - The register data.
 * @return 0 in case of success, negative error code otherwise.
 * This is related to register 0X22 in the DS page 53
 */
int32_t ad4630_set_drive_strength(uint8_t mode)  //mode should be 0x00 for normal mode or 0x01  for double output driver strength
{
	if((mode != AD463X_NORMAL_OUTPUT_STRENGTH)&& (mode != AD463X_DOUBLE_OUTPUT_STRENGTH))
		return -EINVAL;

	return ad4630_spi_reg_write_4_wire_masked(AD463X_REG_IO, AD463X_DRIVER_STRENGTH_MASK, mode);
}


/**
 * @brief Set channel gain
 * @param ch_idx - Channel index is 0 or 1, 0 covers registers 0x1C and 0x1D, 1 covers 0x1E, 0x1F
 * @param gain - The gain value scaled by 10000.
 * 			Example: to set gain 1.5, use 150000
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad4630_set_ch_gain(uint8_t ch_idx, uint8_t gain_lb, uint8_t gain_hb )
{
	int32_t ret;

	ret = ad4630_spi_reg_write_4_wire(AD463X_REG_CHAN_GAIN(ch_idx, AD463X_GAIN_LSB),gain_lb);
	if (ret != 0)
	{
		return ret;
	}

	return ad4630_spi_reg_write_4_wire(AD463X_REG_CHAN_GAIN(ch_idx, AD463X_GAIN_MSB),gain_hb);


}


/**
 * @brief Set channel offset
 * @param ch_idx - The channel index.
 * @param offset - The channel offset.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad4630_set_ch_offset(uint8_t ch_idx,uint32_t offset)
{
	int32_t ret;

	ret = ad4630_spi_reg_write_4_wire(AD463X_REG_CHAN_OFFSET(ch_idx, AD463X_OFFSET_0), offset);
	if (ret < 0)
		return ret;

	ret = ad4630_spi_reg_write_4_wire(AD463X_REG_CHAN_OFFSET(ch_idx, AD463X_OFFSET_1),offset >> 8);
	if (ret < 0)
		return ret;

	return ad4630_spi_reg_write_4_wire(AD463X_REG_CHAN_OFFSET(ch_idx, AD463X_OFFSET_2), offset >> 16);
}


/******************************************/
/* Setting device frame length*/
/******************************************/

/**
 * @brief Set average frame length.
 * @param mode - Average filter frame length mode.
 * @return 0 in case of success, negative error code otherwise.
 * Refer to page 49 of DS https://www.analog.com/media/en/technical-documentation/data-sheets/ad4630-24_ad4632-24.pdf
 *
 */
int32_t ad4630_set_avg_frame_len(uint8_t mode)
{
	int32_t ret = E_NO_ERROR;
	if(mode == AD463X_NORMAL_MODE)
	{
		ad4630_spi_reg_write_4_wire(AD463X_REG_MODES, AD463X_24_DIFF);
	}

	return ret;
}

/**
 * @brief Exit register configuration mode.
 */
int32_t ad4630_exit_reg_cfg_mode()
{
	int32_t ret= E_NO_ERROR;
	ret = ad4630_spi_reg_write_4_wire(AD463X_REG_EXIT_CFG_MODE, AD463X_EXIT_CFG_MODE);
	return ret;
}




