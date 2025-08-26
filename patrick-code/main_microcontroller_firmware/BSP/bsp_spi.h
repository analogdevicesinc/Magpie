/**
 * @file      bsp_spi.h
 * @brief     This module is responsible for initializing and de-initializing the SPI busses used by the sytem.
 *
 * This module requires:
 * - Shared use of QSPI0, QSPI1, and QSPI2
 * - Shared use of SPI pins P0.16, P0.17, P0.19, P0.25, P0.26, P0.27, P1.8, P1.9, and P1.11
 */

#ifndef BSP_SPI_H__
#define BSP_SPI_H__

/* Includes ----------------------------------------------------------------------------------------------------------*/

#include "mxc_device.h"
#include "mxc_sys.h"

#include "spi.h"

/* Public variables --------------------------------------------------------------------------------------------------*/

/**
 * @brief pointer to the SPI handle responsible for sending config data to the ADC
 */
extern mxc_spi_regs_t *bsp_spi_adc_cfg_spi_handle;

/**
 * @brief pointer to the SPI handle responsible for reading audio data from the ADC channel 0
 */
extern mxc_spi_regs_t *bsp_spi_adc_ch0_data_spi_handle;

/**
 * @brief the integer code for the DMA request for the audio channel 0 SPI bus
 */
extern const mxc_dma_reqsel_t bsp_spi_adc_ch0_data_spi_dma_req;

/**
 * @brief pointer to the SPI handle responsible for reading audio data from the ADC channel 1
 */
extern mxc_spi_regs_t *bsp_spi_adc_ch1_data_spi_handle;

/**
 * @brief the integer code for the DMA request for the audio channel 1 SPI bus
 */
extern const mxc_dma_reqsel_t bsp_spi_adc_ch1_data_spi_dma_req;

/* Public function declarations --------------------------------------------------------------------------------------*/

/**
 * @brief `bsp_adc_config_spi_init()` enables and initializes the GPIO pins for the SPI bus responsible for configuring
 * the ADC.
 *
 * @pre the LDOs are powered on via bsp_power_on_LDOs().
 *
 * @post The ADC config SPI is initialized and ready to send config data to the ADC.
 *
 * @retval Success/Fail, see MXC_Error_Codes for a list of return codes.
 */
int bsp_adc_config_spi_init(void);

/**
 * @brief `bsp_adc_config_spi_deinit()` de-initializes the config SPI and sets the associated pins to high-Z.
 *
 * @post The ADC config SPI is de-initialized and the GPIO pins associated with the bus are high-Z.
 *
 * @retval Success/Fail, see MXC_Error_Codes for a list of return codes.
 */
int bsp_adc_config_spi_deinit(void);

/**
 * @brief `bsp_adc_ch0_data_spi_init()` enables and initializes the SPI bus responsible for reading audio data from the
 * ADC channel 0.
 *
 * @pre the LDOs are powered on via bsp_power_on_LDOs().
 *
 * @post The ADC channel-0 date SPI bus is initialized and ready to read data from the ADC.
 *
 * @retval Success/Fail, see MXC_Error_Codes for a list of return codes.
 */
int bsp_adc_ch0_data_spi_init(void);

/**
 * @brief `bsp_adc_ch0_data_spi_deinit()` de-initializes the ch0 data SPI and sets the associated pins to high-Z.
 *
 * @post The ADC channel-0 date SPI bus is de-initialized and the GPIO pins associated with the bus are high-Z.
 *
 * @retval Success/Fail, see MXC_Error_Codes for a list of return codes.
 */
int bsp_adc_ch0_data_spi_deinit(void);

/**
 * @brief `bsp_adc_ch1_data_spi_init()` enables and initializes the SPI bus responsible for reading audio data from the
 * ADC channel 1.
 *
 * @pre the LDOs are powered on via bsp_power_on_LDOs().
 *
 * @post The ADC channel 1 data SPI bus is initialized and ready to read data from the ADC.
 *
 * @retval Success/Fail, see MXC_Error_Codes for a list of return codes.
 */
int bsp_adc_ch1_data_spi_init(void);

/**
 * @brief `bsp_adc_ch1_data_spi_deinit()` de-initializes the ch1 data SPI and sets the associated pins to high-Z.
 *
 * @post The ADC channel 1 data SPI bus is de-initialized and the GPIO pins associated with the bus are high-Z.
 *
 * @retval Success/Fail, see MXC_Error_Codes for a list of return codes.
 */
int bsp_adc_ch1_data_spi_deinit(void);

#endif
