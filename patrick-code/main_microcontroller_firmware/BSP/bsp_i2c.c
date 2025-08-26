/**
 * NOTE: On the FTC spin the I2C0 pullups are tied to VREGO_A which is an intrnally generated 1.8V source. VREGO_A is
 * always enabled, but the downstream devices can be powered on and off via LDO enable pins and load switches. This
 * means that the downstream devices will see 1.8V through the pullup resistors even when they are powered off.
 *
 * To avoid violating the voltage levels of the downstream devices when they are powered down, we drive the two pins
 * for I2C0 low when the devices are powered down. This wastes a small amount of power in the resistors. In the next
 * spin we can avoid this issue by tying the pullups to the normal 1.8V rail.
 */

/* Private includes --------------------------------------------------------------------------------------------------*/

#include "board.h"
#include "bsp_i2c.h"
#include "bsp_pins.h"
#include "mxc_errors.h"

/* Public variables --------------------------------------------------------------------------------------------------*/

mxc_i2c_regs_t *bsp_i2c_1v8_i2c_handle = MXC_I2C0_BUS0;

mxc_i2c_regs_t *bsp_i2c_3v3_i2c_handle = MXC_I2C1_BUS0;

/* Private variables -------------------------------------------------------------------------------------------------*/

static const mxc_sys_periph_clock_t bsp_i2c_1v8_i2c_clock_enum = MXC_SYS_PERIPH_CLOCK_I2C0;

static const mxc_sys_periph_clock_t bsp_i2c_3v3_i2c_clock_enum = MXC_SYS_PERIPH_CLOCK_I2C1;

/* Public function definitions ---------------------------------------------------------------------------------------*/

int bsp_1v8_i2c_init()
{
    MXC_GPIO_Config(&bsp_pins_1v8_i2c_active_cfg);

    MXC_SYS_ClockEnable(bsp_i2c_1v8_i2c_clock_enum);

    int res = E_NO_ERROR;

    if ((res = MXC_I2C_Init(bsp_i2c_1v8_i2c_handle, 1, 0)) != E_NO_ERROR)
    {
        return res;
    }

    if ((res = MXC_I2C_SetFrequency(bsp_i2c_1v8_i2c_handle, BSP_I2C_1V8_BUS_SPEED)) != BSP_I2C_1V8_BUS_SPEED)
    {
        return res;
    }

    return E_NO_ERROR;
}

int bsp_1V8_i2c_deinit()
{
    MXC_GPIO_Config(&bsp_pins_1v8_i2c_driven_low_cfg);
    // drive the pins low to avoid violating downstream devices, we can remove this when we move to the next spin
    gpio_write_pin(&bsp_pins_1v8_i2c_driven_low_cfg, false);

    MXC_SYS_ClockDisable(bsp_i2c_1v8_i2c_clock_enum);

    return MXC_I2C_Shutdown(bsp_i2c_1v8_i2c_handle);
}

int bsp_3v3_i2c_init()
{
    MXC_GPIO_Config(&bsp_pins_3v3_i2c_active_cfg);

    MXC_SYS_ClockEnable(bsp_i2c_3v3_i2c_clock_enum);

    int res = E_NO_ERROR;

    if ((res = MXC_I2C_Init(bsp_i2c_3v3_i2c_handle, 1, 0)) != E_NO_ERROR)
    {
        return res;
    }

    if ((res = MXC_I2C_SetFrequency(bsp_i2c_3v3_i2c_handle, BSP_I2C_3V3_BUS_SPEED)) != BSP_I2C_3V3_BUS_SPEED)
    {
        return res;
    }

    return E_NO_ERROR;
}

int bsp_3v3_i2c_deinit()
{
    MXC_GPIO_Config(&bsp_pins_3v3_i2c_high_z_cfg);

    MXC_SYS_ClockDisable(bsp_i2c_3v3_i2c_clock_enum);

    return MXC_I2C_Shutdown(bsp_i2c_3v3_i2c_handle);
}
