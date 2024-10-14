
/* Private includes --------------------------------------------------------------------------------------------------*/

#include "mxc_device.h"

#include "bsp_pins.h"
#include "board.h"
#include "bsp_status_led.h"

/* Public function definitions ---------------------------------------------------------------------------------------*/

void status_led_init()
{
    MXC_GPIO_Config(&bsp_pins_red_led_cfg);
    status_led_set(STATUS_LED_COLOR_RED, false);

    MXC_GPIO_Config(&bsp_pins_green_led_cfg);
    status_led_set(STATUS_LED_COLOR_GREEN, false);

    MXC_GPIO_Config(&bsp_pins_blue_led_cfg);
    status_led_set(STATUS_LED_COLOR_BLUE, false);
}

void status_led_set(Status_LED_Color_t color, bool state)
{
    switch (color)
    {
    case STATUS_LED_COLOR_RED:
        gpio_write_pin(&bsp_pins_red_led_cfg, !state);
        break;
    case STATUS_LED_COLOR_GREEN:
        gpio_write_pin(&bsp_pins_green_led_cfg, !state);
        break;
    case STATUS_LED_COLOR_BLUE:
        gpio_write_pin(&bsp_pins_blue_led_cfg, !state);
        break;
    default:
        break;
    }
}

void status_led_toggle(Status_LED_Color_t color)
{
    switch (color)
    {
    case STATUS_LED_COLOR_RED:
        gpio_toggle_pin(&bsp_pins_red_led_cfg);
        break;
    case STATUS_LED_COLOR_GREEN:
        gpio_toggle_pin(&bsp_pins_green_led_cfg);
        break;
    case STATUS_LED_COLOR_BLUE:
        gpio_toggle_pin(&bsp_pins_blue_led_cfg);
        break;
    default:
        break;
    }
}

void status_led_all_off()
{
    status_led_set(STATUS_LED_COLOR_RED, false);
    status_led_set(STATUS_LED_COLOR_GREEN, false);
    status_led_set(STATUS_LED_COLOR_BLUE, false);
}
