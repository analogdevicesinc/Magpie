/**
 * @file      bsp_pins.h
 * @brief     This file contains constant pin configurations for all of the pins used by the sytem.
 * @details   Peripherals which are enabled and disabled include an "active" and "inactive" version of the pin config
 * structs. The inactive variant is typically high-Z, but in some cases it may be driven low. Pins and peripherals that
 * do not change configuration over runtime have a single config struct.
 */

#ifndef BSP_PINS_H__
#define BSP_PINS_H__

/* Includes ----------------------------------------------------------------------------------------------------------*/

#include "mxc_device.h"
#include "mxc_sys.h"

#include "i2c.h"
#include "spi.h"
#include "uart.h"

/* SPIX_F pins -------------------------------------------------------------------------------------------------------*/

// SPIXF reads and writes to the SPI flash memory
extern const mxc_gpio_cfg_t bsp_pins_spixf_active_cfg;
extern const mxc_gpio_cfg_t bsp_pins_spixf_high_z_cfg;

/* NOR Flash pins -------------------------------------------------------------------------------------------------------*/

// NOR Flash reads and writes to the SPI flash memory
extern const mxc_gpio_cfg_t bsp_pins_nor_active_cfg;
extern const mxc_gpio_cfg_t bsp_pins_nor_high_z_cfg;

/* I2C pins ----------------------------------------------------------------------------------------------------------*/

// the 1V8 I2C bus communicates with the environmental sensor, the fuel gauge, and the AFE gain MUX
extern const mxc_gpio_cfg_t bsp_pins_1v8_i2c_active_cfg;
extern const mxc_gpio_cfg_t bsp_pins_1v8_i2c_driven_low_cfg;

// the 3V3 I2C bus communicates with the RTC, AFE load switch, SD card MUX, and the BLE module
extern const mxc_gpio_cfg_t bsp_pins_3v3_i2c_active_cfg;
extern const mxc_gpio_cfg_t bsp_pins_3v3_i2c_high_z_cfg;

/* SDHC pins ---------------------------------------------------------------------------------------------------------*/

// the SDHC reads and writes to the SD card
extern const mxc_gpio_cfg_t bsp_pins_sdhc_active_cfg;
extern const mxc_gpio_cfg_t bsp_pins_sdhc_high_z_cfg;

/* ADC pins ----------------------------------------------------------------------------------------------------------*/

// the ADC config SPI writes configuration data to the ADC
extern const mxc_gpio_cfg_t bsp_pins_adc_cfg_spi_active_cfg;
extern const mxc_gpio_cfg_t bsp_pins_adc_cfg_spi_high_z_cfg;

// the ADC config SPI cs pin acts as a manual chip select for the ADC during configuration, and then is set to high-Z
extern const mxc_gpio_cfg_t bsp_pins_adc_cfg_spi_cs_out_cfg;
extern const mxc_gpio_cfg_t bsp_pins_adc_cfg_spi_cs_high_z_cfg;

// the ADC channel 0 data SPI reads data from the ADC channel 0
extern const mxc_gpio_cfg_t bsp_pins_adc_ch0_data_spi_active_cfg;
extern const mxc_gpio_cfg_t bsp_pins_adc_ch0_data_spi_high_z_cfg;

// the ADC channel 1 data SPI reads data from the ADC channel 1
extern const mxc_gpio_cfg_t bsp_pins_adc_ch1_data_spi_active_cfg;
extern const mxc_gpio_cfg_t bsp_pins_adc_ch1_data_spi_high_z_cfg;

// // the ADC chip select disable pin disables the chip select signal from the ADC clocking circuitry when driven high
// extern const mxc_gpio_cfg_t bsp_pins_adc_cs_disable_cfg;

// set the power supply into the BuckBoost Mode - High for fixed PWM mode, Low for power saving burst mode (default)
extern const mxc_gpio_cfg_t bsp_pins_bb_mode_active_cfg;

// the ADC chip select check pin reads the state of the ADC chip select signal, used to avoid partial DMA writes
extern const mxc_gpio_cfg_t bsp_pins_adc_cs_check_pin_cfg;

// the ADC clock enable pin enables the clock signal from the ADC clocking circuitry when driven high
extern const mxc_gpio_cfg_t bsp_pins_adc_clk_en_cfg;

// the ADC clock master reset pin holds the ADC clock in reset while driven high
extern const mxc_gpio_cfg_t bsp_pins_adc_clk_master_reset_cfg;

// the ADC reset pin holds the ADC in reset when driven low
extern const mxc_gpio_cfg_t bsp_pins_adc_n_reset_cfg;

/* AFE enable pins ---------------------------------------------------------------------------------------------------*/

// the AFE enable pins enable and disable the AFE circuitry along with an I2C controlled load switch
extern const mxc_gpio_cfg_t bsp_pins_afe_ch0_en_cfg;
extern const mxc_gpio_cfg_t bsp_pins_afe_ch1_en_cfg;

/* GNSS pins ---------------------------------------------------------------------------------------------------------*/

// the GNSS enable pin enables and disables power to the GNSS circuitry
extern const mxc_gpio_cfg_t bsp_pins_gps_en_cfg;

// the GNSS PPS pin is used to synchronize the microcontroller with GNSS time
extern const mxc_gpio_cfg_t bsp_pins_gps_pps_cfg;

// the GNSS UART pin is used to communicate with the external GNSS module
extern const mxc_gpio_cfg_t bsp_pins_gps_uart_active_cfg;
extern const mxc_gpio_cfg_t bsp_pins_gps_uart_high_z_cfg;

/* RTC pins ----------------------------------------------------------------------------------------------------------*/

// the RTC interrupt pin is used to detect RTC alarms
extern const mxc_gpio_cfg_t bsp_pins_rtc_int_cfg;

/* BLE module pins ---------------------------------------------------------------------------------------------------*/

// the BLE enable pushbuton is used to detect user input to enable the BLE module
extern const mxc_gpio_cfg_t bsp_pins_ble_en_pushbutton_cfg;

// the BLE UART pin is used to communicate with the BLE module
extern const mxc_gpio_cfg_t bsp_pins_ble_uart_active_cfg;
extern const mxc_gpio_cfg_t bsp_pins_ble_uart_high_z_cfg;

/* LDO enable pin ----------------------------------------------------------------------------------------------------*/

// the LDO enable pin enables and disables power to the LDO circuitry
extern const mxc_gpio_cfg_t bsp_pins_ldo_en_cfg;

/* User pushbutton pin -----------------------------------------------------------------------------------------------*/

// the user pushbutton pin is used to detect user input
extern const mxc_gpio_cfg_t bsp_pins_user_pushbutton_cfg;

/* LED pins ----------------------------------------------------------------------------------------------------------*/

// the LEDs are used to indicate the state of the system
extern const mxc_gpio_cfg_t bsp_pins_red_led_cfg;
extern const mxc_gpio_cfg_t bsp_pins_green_led_cfg;
extern const mxc_gpio_cfg_t bsp_pins_blue_led_cfg;

/* Console UART pins -------------------------------------------------------------------------------------------------*/

// the console UART pin is used to communicate with the host PC
extern const mxc_gpio_cfg_t bsp_pins_console_uart_active_cfg;
extern const mxc_gpio_cfg_t bsp_pins_console_uart_high_z_cfg;

#endif
