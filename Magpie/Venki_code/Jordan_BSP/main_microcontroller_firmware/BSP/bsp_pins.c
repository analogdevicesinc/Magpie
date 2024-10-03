
/* Private includes --------------------------------------------------------------------------------------------------*/

#include "bsp_pins.h"

/* SPIX_F pins -------------------------------------------------------------------------------------------------------*/

const mxc_gpio_cfg_t bsp_pins_spixf_active_cfg = {
    .port = MXC_GPIO0,
    .mask = (MXC_GPIO_PIN_0 | MXC_GPIO_PIN_1 | MXC_GPIO_PIN_2 | MXC_GPIO_PIN_3),
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_ALT1,
    .vssel = MXC_GPIO_VSSEL_VDDIOH,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

const mxc_gpio_cfg_t bsp_pins_spixf_high_z_cfg = {
    .port = MXC_GPIO0,
    .mask = (MXC_GPIO_PIN_0 | MXC_GPIO_PIN_1 | MXC_GPIO_PIN_2 | MXC_GPIO_PIN_3),
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_IN,
    .vssel = MXC_GPIO_VSSEL_VDDIOH,
};

/* I2C pins ----------------------------------------------------------------------------------------------------------*/

const mxc_gpio_cfg_t bsp_pins_1v8_i2c_active_cfg = {
    .port = MXC_GPIO0,
    .mask = (MXC_GPIO_PIN_6 | MXC_GPIO_PIN_7),
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_ALT1,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

const mxc_gpio_cfg_t bsp_pins_1v8_i2c_driven_low_cfg = {
    .port = MXC_GPIO0,
    .mask = (MXC_GPIO_PIN_6 | MXC_GPIO_PIN_7),
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_OUT,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

mxc_i2c_regs_t *bsp_pins_1v8_i2c_handle = MXC_I2C0_BUS0;

const mxc_sys_periph_clock_t bsp_pins_1v8_i2c_clock_enum = MXC_SYS_PERIPH_CLOCK_I2C0;

const mxc_gpio_cfg_t bsp_pins_3v3_i2c_active_cfg = {
    .port = MXC_GPIO0,
    .mask = (MXC_GPIO_PIN_14 | MXC_GPIO_PIN_15),
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_ALT1,
    .vssel = MXC_GPIO_VSSEL_VDDIOH,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

const mxc_gpio_cfg_t bsp_pins_3v3_i2c_high_z_cfg = {
    .port = MXC_GPIO0,
    .mask = (MXC_GPIO_PIN_14 | MXC_GPIO_PIN_15),
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_IN,
    .vssel = MXC_GPIO_VSSEL_VDDIOH,
};

mxc_i2c_regs_t *bsp_pins_3v3_i2c_handle = MXC_I2C1_BUS0;

const mxc_sys_periph_clock_t bsp_pins_3v3_i2c_clock_enum = MXC_SYS_PERIPH_CLOCK_I2C1;

/* SDHC pins ---------------------------------------------------------------------------------------------------------*/

const mxc_gpio_cfg_t bsp_pins_sdhc_active_cfg = {
    .port = MXC_GPIO1,
    .mask = (MXC_GPIO_PIN_0 | MXC_GPIO_PIN_1 | MXC_GPIO_PIN_2 | MXC_GPIO_PIN_3 | MXC_GPIO_PIN_4 | MXC_GPIO_PIN_5),
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_ALT1,
    .vssel = MXC_GPIO_VSSEL_VDDIOH,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

const mxc_gpio_cfg_t bsp_pins_sdhc_high_z_cfg = {
    .port = MXC_GPIO1,
    .mask = (MXC_GPIO_PIN_0 | MXC_GPIO_PIN_1 | MXC_GPIO_PIN_2 | MXC_GPIO_PIN_3 | MXC_GPIO_PIN_4 | MXC_GPIO_PIN_5),
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_IN,
    .vssel = MXC_GPIO_VSSEL_VDDIOH,
};

/* ADC pins ----------------------------------------------------------------------------------------------------------*/

const mxc_gpio_cfg_t bsp_pins_adc_cfg_spi_active_cfg = {
    .port = MXC_GPIO0,
    .mask = (MXC_GPIO_PIN_25 | MXC_GPIO_PIN_26 | MXC_GPIO_PIN_27),
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_ALT2,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

const mxc_gpio_cfg_t bsp_pins_adc_cfg_spi_high_z_cfg = {
    .port = MXC_GPIO0,
    .mask = (MXC_GPIO_PIN_25 | MXC_GPIO_PIN_26 | MXC_GPIO_PIN_27),
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_IN,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
};

mxc_spi_regs_t *bsp_pins_adc_cfg_spi_handle = MXC_SPI2;

const mxc_sys_periph_clock_t bsp_pins_adc_cfg_spi_clock_enum = MXC_SYS_PERIPH_CLOCK_SPI2;

const mxc_gpio_cfg_t bsp_pins_adc_cfg_spi_cs_out_cfg = {
    .port = MXC_GPIO0,
    .mask = MXC_GPIO_PIN_16,
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_OUT,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
    .drvstr = MXC_GPIO_DRVSTR_3,
};

const mxc_gpio_cfg_t bsp_pins_adc_cfg_spi_cs_high_z_cfg = {
    .port = MXC_GPIO0,
    .mask = MXC_GPIO_PIN_16,
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_IN,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
};

const mxc_gpio_cfg_t bsp_pins_adc_ch0_data_spi_active_cfg = {
    .port = MXC_GPIO0,
    .mask = (MXC_GPIO_PIN_16 | MXC_GPIO_PIN_17 | MXC_GPIO_PIN_19),
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_ALT2,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

const mxc_gpio_cfg_t bsp_pins_adc_ch0_data_spi_high_z_cfg = {
    .port = MXC_GPIO0,
    .mask = (MXC_GPIO_PIN_16 | MXC_GPIO_PIN_17 | MXC_GPIO_PIN_19),
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_IN,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
};

mxc_spi_regs_t *bsp_pins_adc_ch0_data_spi_handle = MXC_SPI1;

const mxc_sys_periph_clock_t bsp_pins_adc_ch0_data_spi_clock_enum = MXC_SYS_PERIPH_CLOCK_SPI1;

const mxc_gpio_cfg_t bsp_pins_adc_ch1_data_spi_active_cfg = {
    .port = MXC_GPIO1,
    .mask = (MXC_GPIO_PIN_8 | MXC_GPIO_PIN_9 | MXC_GPIO_PIN_11),
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_ALT1,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

const mxc_gpio_cfg_t bsp_pins_adc_ch1_data_spi_high_z_cfg = {
    .port = MXC_GPIO1,
    .mask = (MXC_GPIO_PIN_8 | MXC_GPIO_PIN_9 | MXC_GPIO_PIN_11),
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_IN,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
};

mxc_spi_regs_t *bsp_pins_adc_ch1_data_spi_handle = MXC_SPI0;

const mxc_sys_periph_clock_t bsp_pins_adc_ch1_data_spi_clock_enum = MXC_SYS_PERIPH_CLOCK_SPI0;

const mxc_gpio_cfg_t bsp_pins_adc_cs_en_cfg = {
    .port = MXC_GPIO1,
    .mask = MXC_GPIO_PIN_6,
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_OUT,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

const mxc_gpio_cfg_t bsp_pins_adc_cs_check_pin_cfg = {
    .port = MXC_GPIO0,
    .mask = MXC_GPIO_PIN_4,
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_IN,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
};

const mxc_gpio_cfg_t bsp_pins_adc_clk_en_cfg = {
    .port = MXC_GPIO0,
    .mask = MXC_GPIO_PIN_20,
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_OUT,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

const mxc_gpio_cfg_t bsp_pins_adc_clk_master_reset_cfg = {
    .port = MXC_GPIO0,
    .mask = MXC_GPIO_PIN_22,
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_OUT,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

const mxc_gpio_cfg_t bsp_pins_adc_n_reset_cfg = {
    .port = MXC_GPIO0,
    .mask = MXC_GPIO_PIN_21,
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_OUT,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

/* AFE enable pins ---------------------------------------------------------------------------------------------------*/

const mxc_gpio_cfg_t bsp_pins_afe_ch0_en_cfg = {
    .port = MXC_GPIO0,
    .mask = MXC_GPIO_PIN_11,
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_OUT,
    .vssel = MXC_GPIO_VSSEL_VDDIOH,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

const mxc_gpio_cfg_t bsp_pins_afe_ch1_en_cfg = {
    .port = MXC_GPIO0,
    .mask = MXC_GPIO_PIN_12,
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_OUT,
    .vssel = MXC_GPIO_VSSEL_VDDIOH,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

/* GNSS pins ---------------------------------------------------------------------------------------------------------*/

const mxc_gpio_cfg_t bsp_pins_gps_en_cfg = {
    .port = MXC_GPIO0,
    .mask = MXC_GPIO_PIN_23,
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_OUT,
    .vssel = MXC_GPIO_VSSEL_VDDIOH,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

const mxc_gpio_cfg_t bsp_pins_gps_pps_cfg = {
    .port = MXC_GPIO0,
    .mask = MXC_GPIO_PIN_24,
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_IN,
    .vssel = MXC_GPIO_VSSEL_VDDIOH,
};

const mxc_gpio_cfg_t bsp_pins_gps_uart_active_cfg = {
    .port = MXC_GPIO0,
    .mask = (MXC_GPIO_PIN_28 | MXC_GPIO_PIN_29),
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_ALT3,
    .vssel = MXC_GPIO_VSSEL_VDDIOH,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

const mxc_gpio_cfg_t bsp_pins_gps_uart_high_z_cfg = {
    .port = MXC_GPIO0,
    .mask = (MXC_GPIO_PIN_28 | MXC_GPIO_PIN_29),
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_IN,
    .vssel = MXC_GPIO_VSSEL_VDDIOH,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

mxc_uart_regs_t *bsp_pins_gps_uart_handle = MXC_UART2;

const sys_map_t bsp_pins_gps_uart_map = MAP_B;

const mxc_sys_periph_clock_t bsp_pins_gps_uart_clock_enum = MXC_SYS_PERIPH_CLOCK_UART2;

/* RTC pins ----------------------------------------------------------------------------------------------------------*/

const mxc_gpio_cfg_t bsp_pins_rtc_int_cfg = {
    .port = MXC_GPIO0,
    .mask = MXC_GPIO_PIN_13,
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_IN,
    .vssel = MXC_GPIO_VSSEL_VDDIOH,
};

/* BLE module pins ---------------------------------------------------------------------------------------------------*/

const mxc_gpio_cfg_t bsp_pins_ble_en_pushbutton_cfg = {
    .port = MXC_GPIO0,
    .mask = MXC_GPIO_PIN_8,
    .pad = MXC_GPIO_PAD_PULL_UP,
    .func = MXC_GPIO_FUNC_IN,
    .vssel = MXC_GPIO_VSSEL_VDDIOH,
};

const mxc_gpio_cfg_t bsp_pins_ble_uart_active_cfg = {
    .port = MXC_GPIO0,
    .mask = (MXC_GPIO_PIN_9 | MXC_GPIO_PIN_10),
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_ALT3,
    .vssel = MXC_GPIO_VSSEL_VDDIOH,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

const mxc_gpio_cfg_t bsp_pins_ble_uart_high_z_cfg = {
    .port = MXC_GPIO0,
    .mask = (MXC_GPIO_PIN_9 | MXC_GPIO_PIN_10),
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_IN,
    .vssel = MXC_GPIO_VSSEL_VDDIOH,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

mxc_uart_regs_t *bsp_pins_ble_uart_handle = MXC_UART0;

const sys_map_t bsp_pins_ble_uart_map = MAP_A;

const mxc_sys_periph_clock_t bsp_pins_ble_uart_clock_enum = MXC_SYS_PERIPH_CLOCK_UART0;

/* LDO enable pin ----------------------------------------------------------------------------------------------------*/

const mxc_gpio_cfg_t bsp_pins_ldo_en_cfg = {
    .port = MXC_GPIO0,
    .mask = MXC_GPIO_PIN_30,
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_OUT,
    .vssel = MXC_GPIO_VSSEL_VDDIOH,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

/* User pushbutton pin -----------------------------------------------------------------------------------------------*/

const mxc_gpio_cfg_t bsp_pins_user_pushbutton_cfg = {
    .port = MXC_GPIO0,
    .mask = MXC_GPIO_PIN_5,
    .pad = MXC_GPIO_PAD_PULL_UP,
    .func = MXC_GPIO_FUNC_IN,
    .vssel = MXC_GPIO_VSSEL_VDDIOH,
};

/* LED pins ----------------------------------------------------------------------------------------------------------*/

const mxc_gpio_cfg_t bsp_pins_red_led_cfg = {
    .port = MXC_GPIO0,
    .mask = MXC_GPIO_PIN_31,
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_OUT,
    .vssel = MXC_GPIO_VSSEL_VDDIOH,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

const mxc_gpio_cfg_t bsp_pins_green_led_cfg = {
    .port = MXC_GPIO1,
    .mask = MXC_GPIO_PIN_14,
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_OUT,
    .vssel = MXC_GPIO_VSSEL_VDDIOH,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

const mxc_gpio_cfg_t bsp_pins_blue_led_cfg = {
    .port = MXC_GPIO1,
    .mask = MXC_GPIO_PIN_15,
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_OUT,
    .vssel = MXC_GPIO_VSSEL_VDDIOH,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

/* Console UART pins -------------------------------------------------------------------------------------------------*/

const mxc_gpio_cfg_t bsp_pins_console_uart_active_cfg = {
    .port = MXC_GPIO1,
    .mask = (MXC_GPIO_PIN_12 | MXC_GPIO_PIN_13),
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_ALT3,
    .vssel = MXC_GPIO_VSSEL_VDDIOH,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

const mxc_gpio_cfg_t bsp_pins_console_uart_high_z_cfg = {
    .port = MXC_GPIO1,
    .mask = (MXC_GPIO_PIN_12 | MXC_GPIO_PIN_13),
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_IN,
    .vssel = MXC_GPIO_VSSEL_VDDIOH,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

mxc_uart_regs_t *bsp_pins_console_uart_handle = MXC_UART1;

const sys_map_t bsp_pins_console_uart_map = MAP_B;

const mxc_sys_periph_clock_t bsp_pins_console_uart_clock_enum = MXC_SYS_PERIPH_CLOCK_UART1;
