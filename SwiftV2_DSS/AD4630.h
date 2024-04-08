/*
 * AD4630.h
 *
 *  Created on: Feb 2, 2024
 *      Author: VKarra
 */

/*
| Number  |  JUMPER | SPI connections Configuration mode|
|---------|---------|-----------------------------------|
| 1       |    P16_8| AD4630_SCK                        |
| 2       |    P16_9| AD4630_SDI(SPI2-MOSI)             |
| 3       |   P16_10| AD4630_SDI(SPI2-MISO)             |
| 4       |    P16_2| AD_4630_CSn                       |

*/

#ifndef AD4630_H_
#define AD4630_H_

#include <stdio.h>
#include <string.h>
#include "mxc_device.h"
#include "gpio.h"
#include "board.h"
#include "utils.h"
#include "spi.h"
#include "stdint.h"

/******************************************************************************/
/********************** Macros and Types Declarations *************************/
/******************************************************************************/



/* Register addresses */
//Look at page43 of the DS for the register address
// https://www.analog.com/media/en/technical-documentation/data-sheets/ad4630-24_ad4632-24.pdf

#define AD463X_REG_INTERFACE_CONFIG_A	0x00
#define AD463X_REG_INTERFACE_CONFIG_B	0x01
#define AD463X_REG_DEVICE_CONFIG	0x02
#define AD463X_REG_CHIP_TYPE		0x03
#define AD463X_REG_PRODUCT_ID_L		0x04
#define AD463X_REG_PRODUCT_ID_H		0x05
#define AD463X_REG_CHIP_GRADE		0x06
#define AD463X_REG_SCRATCH_PAD		0x0A
#define AD463X_REG_SPI_REVISION		0x0B
#define AD463X_REG_VENDOR_L		0x0C
#define AD463X_REG_VENDOR_H		0x0D
#define AD463X_REG_STREAM_MODE		0x0E
#define AD463X_REG_EXIT_CFG_MODE	0x14
#define AD463X_REG_AVG			0x15
#define AD463X_REG_OFFSET_BASE		0x16
#define AD463X_REG_OFFSET_X0_0		0x16
#define AD463X_REG_OFFSET_X0_1		0x17
#define AD463X_REG_OFFSET_X0_2		0x18
#define AD463X_REG_OFFSET_X1_0		0x19
#define AD463X_REG_OFFSET_X1_1		0x1A
#define AD463X_REG_OFFSET_X1_2		0x1B
#define AD463X_REG_GAIN_BASE		0x1C
#define AD463X_REG_GAIN_X0_LSB		0x1C
#define AD463X_REG_GAIN_X0_MSB		0x1D
#define AD463X_REG_GAIN_X1_LSB		0x1E
#define AD463X_REG_GAIN_X1_MSB		0x1F
#define AD463X_REG_MODES		0x20
#define AD463X_REG_OSCILATOR		0x21
#define AD463X_REG_IO			0x22
#define AD463X_REG_PAT0			0x23
#define AD463X_REG_PAT1			0x24
#define AD463X_REG_PAT2			0x25
#define AD463X_REG_PAT3			0x26
#define AD463X_REG_DIG_DIAG		0x34
#define AD463X_REG_DIG_ERR		0x35
/* INTERFACE_CONFIG_A */
#define AD463X_CFG_SW_RESET		(NO_OS_BIT(7) | NO_OS_BIT(0))
#define AD463X_CFG_SDO_ENABLE		NO_OS_BIT(4)
/* MODES */
#define AD463X_SW_RESET_MSK		(NO_OS_BIT(7) | NO_OS_BIT(0))
#define AD463X_LANE_MODE_MSK		(NO_OS_BIT(7) | NO_OS_BIT(6))
#define AD463X_CLK_MODE_MSK		(NO_OS_BIT(5) | NO_OS_BIT(4))
#define AD463X_DDR_MODE_MSK		NO_OS_BIT(3)
#define AD463X_SDR_MODE			0x00
#define AD463X_DDR_MODE			NO_OS_BIT(3)
#define AD463X_OUT_DATA_MODE_MSK	(NO_OS_BIT(2) | NO_OS_BIT(1) | NO_OS_BIT(0))
#define AD463X_24_DIFF			0x00
#define AD463X_16_DIFF_8_COM		0x01
#define AD463X_24_DIFF_8_COM		0x02
#define AD463X_30_AVERAGED_DIFF		0x03
#define AD463X_32_PATTERN		0x04
/* EXIT_CFG_MD */
#define AD463X_EXIT_CFG_MODE		NO_OS_BIT(0)
/* CHANNEL */
#define AD463X_CHANNEL_0		0x00
#define AD463X_CHANNEL_1		0x01
/* OFFSET */
#define AD463X_OFFSET_0			0x00
#define AD463X_OFFSET_1			0x01
#define AD463X_OFFSET_2			0x02
/* GAIN */
#define AD463X_GAIN_LSB			0x00
#define AD463X_GAIN_MSB			0x01
/* LANE MODE */
#define AD463X_ONE_LANE_PER_CH		0x00
#define AD463X_TWO_LANES_PER_CH		NO_OS_BIT(6)
#define AD463X_FOUR_LANES_PER_CH	NO_OS_BIT(7)
#define AD463X_SHARED_TWO_CH		(NO_OS_BIT(6) | NO_OS_BIT(7))
/* CLK MODE */
#define AD463X_SPI_COMPATIBLE_MODE	0x00
#define AD463X_ECHO_CLOCK_MODE		NO_OS_BIT(4)
#define AD463X_CLOCK_MASTER_MODE	NO_OS_BIT(5)
/* POWER MODE */
#define AD463X_NORMAL_MODE 		0x00
#define AD463X_LOW_POWER_MODE		(NO_OS_BIT(1) | NO_OS_BIT(0))
/* AVG */
#define AD463X_AVG_FILTER_RESET		NO_OS_BIT(7)
#define AD463X_CONFIG_TIMING		0x2000
#define AD463X_REG_READ_DUMMY		0x00
#define AD463X_REG_WRITE        	0x00
#define AD463X_REG_READ		    	NO_OS_BIT(7)
#define AD463X_REG_CHAN_OFFSET(ch, pos)	(AD463X_REG_OFFSET_BASE + (3 * ch) + pos)
#define AD463X_REG_CHAN_GAIN(ch, pos)	(AD463X_REG_GAIN_BASE + (2 * ch) + pos)
/* IO */
#define AD463X_DRIVER_STRENGTH_MASK	NO_OS_BIT(0)
#define AD463X_NORMAL_OUTPUT_STRENGTH	0x00
#define AD463X_DOUBLE_OUTPUT_STRENGTH	NO_OS_BIT(1)
/* OUT_DATA_PAT */
#define AD463X_OUT_DATA_PAT		0x5A5A0F0F

#define AD463X_TRIGGER_PULSE_WIDTH_NS	0x0A

#define AD463X_GAIN_MAX_VAL_SCALED	19997
// Error codes used in the Secure CRT functions

#define _SECURECRT_ERRCODE_VALUES_DEFINED
#define EINVAL          22
#define ERANGE          34
#define EILSEQ          42
#define STRUNCATE       80





//****************************************************************
//GPIO defines
//****************************************************************
//For the CS pin
#define MXC_GPIO_PORT_OUT0 MXC_GPIO0
#define MXC_GPIO_PIN_OUT16 MXC_GPIO_PIN_16

//Pin 0.20
#define MXC_GPIO_PORT_OUT0 MXC_GPIO0
#define MXC_GPIO_PIN_OUT20 MXC_GPIO_PIN_20

//Pin 0.21
#define MXC_GPIO_PORT_OUT0 MXC_GPIO0
#define MXC_GPIO_PIN_OUT21 MXC_GPIO_PIN_21

//Pin 0.10, used for input Busy signal
#define MXC_GPIO_PORT_IN0 MXC_GPIO0
#define MXC_GPIO_PIN_IN10 MXC_GPIO_PIN_10

//Pin 0.18, used for input Busy signal call back
#define MXC_GPIO_PORT_OUT0 MXC_GPIO0
#define MXC_GPIO_PIN_OUT30 MXC_GPIO_PIN_30


//****************************************************************
//SPI defines for SPI2 master
//****************************************************************
/***** Definitions *****/
#define DATA_LEN 3 // Words
#define SPI_SPEED 100000 // Bit Rate
#define DATA_SIZE 8  //Data size

#define SPI_MASTER2 MXC_SPI2 //Using SPI2 as Master to configure ADC
#define SPI_MASTER2_SSIDX 0

//****************************************************************
//SPI defines for SPI1 master
//****************************************************************
#define DATA_LEN_SPI1 3 // Words
#define SPI_SPEED_SPI1 16000000// Bit Rate
#define DATA_SIZE_SPI1 8  //Data size
#define SPI_MASTER1 MXC_SPI1 //Using SPI2 as Master to configure ADC
#define SPI_MASTER1_SSIDX 0
#define SPI1_IRQ SPI1_IRQn

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

//Hardware reset
void hardware_reset();

//Enable CNV signal

void enable_cnv_signal();

//Disable CNV signal

void disable_cnv_signal();

//ADC configuration
void ADC_Config();

int32_t ad4630_conversion();

/** Read device register. */
int32_t ad4630_spi_reg_read_4_wire(uint16_t reg_addr,  uint8_t reg_data);

/** Write device register. */
int32_t ad4630_spi_reg_write_4_wire(uint16_t reg_addr,
			    uint8_t reg_data);

/** Read device register masked. */
int32_t ad4630_spi_reg_read_4_wire_masked(uint16_t reg_addr, uint8_t mask, uint8_t data);

/** Write device register masked. */
int32_t ad4630_spi_reg_write_4_wire_masked(uint16_t reg_addr, uint8_t mask, uint8_t data);


/**
 * @Sending dummy byte to enter into the configuration mode
 */
int32_t ad4630_enter_config_mode(uint16_t addr, uint8_t data);



/**
 * @Write and read from scratch pad test data
 */
int32_t ad4630_scratchpad(uint16_t addr, uint8_t data);


/**
 * @brief Set power mode.
 */
int32_t ad4630_set_pwr_mode(uint8_t mode);


/**
 * @brief Set drive strength.
 * @param mode - The register data.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad4630_set_drive_strength(uint8_t mode);

/**
 * @brief Exit register configuration mode.
 */
int32_t ad4630_exit_reg_cfg_mode(void);


/**
 * @brief Set channel gain
 * @param ch_idx - The channel index.
 * @param gain_lb - lower byte gain in this case 0x00.
 * @param gain_hb - lower byte gain in this case 0x80.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad4630_set_ch_gain(uint8_t ch_idx, uint8_t gain_lb, uint8_t gain_hb );


/**
 * @brief Set channel offset
 * @param ch_idx - The channel index.
 * @param offset - The channel offset.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad4630_set_ch_offset(uint8_t ch_idx,uint32_t offset);


/******************************************/
/* Setting device frame length*/
/******************************************/

/**
 * @brief Set average frame length.
 * @param mode - Average filter frame length mode.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad4630_set_avg_frame_len(uint8_t mode);

/**
 * @brief Set channel offset
 * @param dev - The device structure.
 * @param ch_idx - The channel index.
 * @param offset - The channel offset.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad4630_set_ch_offset(uint8_t ch_idx,
			     uint32_t offset);


#endif /* AD4630_H_ */
