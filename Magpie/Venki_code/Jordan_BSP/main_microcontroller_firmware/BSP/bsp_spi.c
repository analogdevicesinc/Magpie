
/* Private includes --------------------------------------------------------------------------------------------------*/

#include "spi.h"

#include "board.h"
#include "bsp_pins.h"
#include "bsp_spi.h"
#include "mxc_errors.h"

/* Public function definitions ---------------------------------------------------------------------------------------*/

int bsp_adc_config_spi_init()
{
    MXC_GPIO_Config(&bsp_pins_adc_cfg_spi_cs_out_cfg);

    const mxc_gpio_cfg_t config_chip_sel_pin = {
        .port = MXC_GPIO0,
        .mask = MXC_GPIO_PIN_4,
        .pad = MXC_GPIO_PAD_NONE,
        .func = MXC_GPIO_FUNC_OUT,
        .vssel = MXC_GPIO_VSSEL_VDDIO,
        .drvstr = MXC_GPIO_DRVSTR_3,
    };
    MXC_GPIO_Config(&config_chip_sel_pin);

    gpio_write_pin(&config_chip_sel_pin, true);

    MXC_SYS_ClockEnable(bsp_pins_adc_cfg_spi_clock_enum);

    int res = E_NO_ERROR;

    // freq doesn't matter too much, we only init rarely and write to a few registers
    const uint32_t CFG_SPI_CLK_FREQ_Hz = 5000000;

    if ((res = MXC_SPI_Init(
             bsp_pins_adc_cfg_spi_handle,
             1, // 1 -> master mode
             0, // 0 -> quad mode not used, single bit SPI
             1, // num slaves
             0, // CS polarity (0 for active low)
             CFG_SPI_CLK_FREQ_Hz,
             MAP_A)) != E_NO_ERROR)
    {
        return res;
    }
    if ((res = MXC_SPI_SetDataSize(bsp_pins_adc_cfg_spi_handle, 8)) != E_NO_ERROR)
    {
        return res;
    }
    if ((res = MXC_SPI_SetWidth(bsp_pins_adc_cfg_spi_handle, SPI_WIDTH_STANDARD)) != E_NO_ERROR)
    {
        return res;
    }
    if ((res = MXC_SPI_SetMode(bsp_pins_adc_cfg_spi_handle, SPI_MODE_0)) != E_NO_ERROR)
    {
        return res;
    }

    return E_NO_ERROR;
}

int bsp_adc_config_spi_deinit()
{
    MXC_GPIO_Config(&bsp_pins_adc_cfg_spi_high_z_cfg);

    const mxc_gpio_cfg_t config_chip_sel_pin = {
        .port = MXC_GPIO0,
        .mask = MXC_GPIO_PIN_4,
        .pad = MXC_GPIO_PAD_NONE,
        .func = MXC_GPIO_FUNC_IN,
        .vssel = MXC_GPIO_VSSEL_VDDIO,
        .drvstr = MXC_GPIO_DRVSTR_0,
    };
    gpio_write_pin(&config_chip_sel_pin, false);
    MXC_GPIO_Config(&config_chip_sel_pin);

    MXC_SYS_ClockDisable(bsp_pins_adc_cfg_spi_clock_enum);

    return MXC_SPI_Shutdown(bsp_pins_adc_cfg_spi_handle);
}

int bsp_adc_ch0_data_spi_init()
{
    MXC_GPIO_Config(&bsp_pins_adc_ch0_data_spi_active_cfg);

    MXC_SYS_ClockEnable(bsp_pins_adc_ch0_data_spi_clock_enum);

    int res = E_NO_ERROR;

    if ((res = MXC_SPI_Init(
             bsp_pins_adc_ch0_data_spi_handle,
             0, // 0 -> slave mode
             0, // 0 -> quad mode not used, single bit SPI
             0, // num slaves, none
             0, // CS polarity (0 for active low)
             0, // freq is defined by the driving clock
             MAP_A)) != E_NO_ERROR)
    {
        return res;
    }

    if ((res = MXC_SPI_SetWidth(bsp_pins_adc_ch0_data_spi_handle, SPI_WIDTH_3WIRE)) != E_NO_ERROR)
    {
        return res;
    }
    if ((res = MXC_SPI_SetMode(bsp_pins_adc_ch0_data_spi_handle, SPI_MODE_1)) != E_NO_ERROR)
    {
        return res;
    }

    return E_NO_ERROR;
}

int bsp_adc_ch0_data_spi_deinit()
{
    MXC_GPIO_Config(&bsp_pins_adc_ch0_data_spi_high_z_cfg);

    MXC_SYS_ClockDisable(bsp_pins_adc_ch0_data_spi_clock_enum);

    return MXC_SPI_Shutdown(bsp_pins_adc_ch0_data_spi_handle);
}

int bsp_adc_ch1_data_spi_init()
{
    // TODO
    return -1;
}

int bsp_adc_ch1_data_spi_deinit()
{
    // TODO
    return -1;
}
