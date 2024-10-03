
/* Private includes --------------------------------------------------------------------------------------------------*/

#include "mxc_device.h"

#include "board.h"
#include "bsp_pins.h"
#include "bsp_pushbutton.h"

/* Public function definitions ---------------------------------------------------------------------------------------*/

void pushbuttons_init()
{
    MXC_GPIO_Config(&bsp_pins_user_pushbutton_cfg);
    MXC_GPIO_Config(&bsp_pins_ble_en_pushbutton_cfg);
}

Button_State_t user_pushbutton_state()
{
    // TODO, code to debounce the switch and get the other states

    // note: the button has a pullup which is pulled to ground, so the action is inverting
    return gpio_read_pin(&bsp_pins_user_pushbutton_cfg) ? BUTTON_STATE_NOT_PRESSED : BUTTON_STATE_PRESSED;
}

Button_State_t ble_enable_pushbutton_state()
{
    // TODO, code to debounce the switch and get the other states

    // note: the button has a pullup which is pulled to ground, so the action is inverting
    return gpio_read_pin(&bsp_pins_ble_en_pushbutton_cfg) ? BUTTON_STATE_NOT_PRESSED : BUTTON_STATE_PRESSED;
}
