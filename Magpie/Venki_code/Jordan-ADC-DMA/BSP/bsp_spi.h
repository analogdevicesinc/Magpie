/**
 * @file      bsp_sdpi.h
 * @brief     This module is responsible for initializing and de-initializing the SPI busses used by the sytem.
 *
 * This module requires:
 * - Shared use of QSPI0, QSPI1, and QSPI2
 * - Shared use of SPI pins P0.16, P0.17, P0.19, P0.25, P0.26, P0.27, P1.8, P1.9, and P1.11
 */

#ifndef BSP_SPI_H__
#define BSP_SPI_H__

/* Public function declarations --------------------------------------------------------------------------------------*/

/**
 * @brief `bsp_adc_config_spi_init()` initializes and starts SPI2 bus as a master.
 *
 * @pre the LDOs are powered on via bsp_power_on_LDOs().
 *
 * @post QSPI2 is initialized and ready to send config data to the ADC.
 *
 * @retval Success/Fail, see MXC_Error_Codes for a list of return codes.
 */
int bsp_adc_config_spi_init();

/**
 * @brief `bsp_adc_config_spi_deinit()` de-initializes the config SPI and sets the associated pins to high-Z.
 *
 * @post QSPI2 is de-initialized and the GPIO pins associated with the bus are high-Z.
 *
 * @retval Success/Fail, see MXC_Error_Codes for a list of return codes.
 */
int bsp_adc_config_spi_deinit();

/**
 * @brief `bsp_adc_ch0_data_spi_init()` initializes and starts SPI1 bus as a slave which received data from the ADC.
 *
 * @pre the LDOs are powered on via bsp_power_on_LDOs().
 *
 * @post QSPI1 is initialized and ready to read data from the ADC.
 *
 * @retval Success/Fail, see MXC_Error_Codes for a list of return codes.
 */
int bsp_adc_ch0_data_spi_init();

/**
 * @brief `bsp_adc_ch0_data_spi_deinit()` de-initializes the ch0 data SPI and sets the associated pins to high-Z.
 *
 * @post QSPI1 is de-initialized and the GPIO pins associated with the bus are high-Z.
 *
 * @retval Success/Fail, see MXC_Error_Codes for a list of return codes.
 */
int bsp_adc_ch0_data_spi_deinit();

/**
 * @brief `bsp_adc_ch1_data_spi_init()` initializes and starts SPI0 bus as a slave which received data from the ADC.
 *
 * @pre the LDOs are powered on via bsp_power_on_LDOs().
 *
 * @post QSPI0 is initialized and ready to read data from the ADC.
 *
 * @retval Success/Fail, see MXC_Error_Codes for a list of return codes.
 */
int bsp_adc_ch1_data_spi_init();

/**
 * @brief `bsp_adc_ch1_data_spi_deinit()` de-initializes the ch1 data SPI and sets the associated pins to high-Z.
 *
 * @post QSPI0 is de-initialized and the GPIO pins associated with the bus are high-Z.
 *
 * @retval Success/Fail, see MXC_Error_Codes for a list of return codes.
 */
int bsp_adc_ch1_data_spi_deinit();

#endif
