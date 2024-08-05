/* Private includes --------------------------------------------------------------------------------------------------*/

#include "gpio_helpers.h"

/* Private variables -------------------------------------------------------------------------------------------------*/

static const mxc_gpio_cfg_t profiling_pin = {
    .port = MXC_GPIO1,
    .mask = MXC_GPIO_PIN_8 | MXC_GPIO_PIN_9 | MXC_GPIO_PIN_10 | MXC_GPIO_PIN_11,
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_OUT,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

/* Public function definitions ---------------------------------------------------------------------------------------*/

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

void gpio_profiling_pin_init()
{
    MXC_GPIO_Config(&profiling_pin);
}

void gpio_profiling_pin_write(GPIO_Profiling_Pin_t pin, bool state)
{
    state ? MXC_GPIO_OutSet(MXC_GPIO1, pin) : MXC_GPIO_OutClr(MXC_GPIO1, pin);
}
