
/* Private includes --------------------------------------------------------------------------------------------------*/

#include "board.h"
#include "bsp_i2c.h"
#include "bsp_pins.h"
#include "bsp_pushbutton.h"
#include "bsp_sdhc.h"
#include "bsp_spi.h"
#include "bsp_spixf.h"
#include "bsp_status_led.h"
#include "bsp_uart.h"

#include "mxc_errors.h"
#include "uart.h"

/* Public function definitions ---------------------------------------------------------------------------------------*/

int Board_Init(void)
{
    // deinit all peripherals at startup
    bsp_1V8_i2c_deinit();
    bsp_3v3_i2c_deinit();

    bsp_adc_config_spi_deinit();
    bsp_adc_ch0_data_spi_deinit();
    bsp_adc_ch1_data_spi_deinit();

    bsp_console_uart_deinit();
    bsp_gnss_uart_deinit();
    bsp_ble_uart_deinit();

    bsp_sdhc_deinit();

    bsp_spixf_deinit();

    // Configure RTC interrupt pin so it's on 3V3 domain.
    MXC_GPIO_Config(&bsp_pins_ldo_en_cfg);

    // power down the LDOs
    MXC_GPIO_Config(&bsp_pins_rtc_int_cfg);
    bsp_power_off_LDOs();

    // initialize the status LEDs
    status_led_init();

    // initialize the pushbuttons
    pushbuttons_init();

    return E_NO_ERROR;
}

void bsp_power_on_LDOs()
{
    gpio_write_pin(&bsp_pins_ldo_en_cfg, true);
}

void bsp_power_off_LDOs()
{
    gpio_write_pin(&bsp_pins_ldo_en_cfg, false);
}

void gpio_write_pin(const mxc_gpio_cfg_t *pin, bool state)
{
    state ? MXC_GPIO_OutSet(pin->port, pin->mask) : MXC_GPIO_OutClr(pin->port, pin->mask);
}

void gpio_toggle_pin(const mxc_gpio_cfg_t *pin)
{
    MXC_GPIO_OutToggle(pin->port, pin->mask);
}

bool gpio_read_pin(const mxc_gpio_cfg_t *pin)
{
    return (bool)MXC_GPIO_InGet(pin->port, pin->mask);
}
