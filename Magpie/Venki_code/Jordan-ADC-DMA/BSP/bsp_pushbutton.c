
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
    static bool last_user_button_state = false;

    // TODO, code to debounce the switch

    // note: the button has a pullup and pressing the switch shorts it to ground, so the action is inverting
    const bool this_button_state = !gpio_read_pin(&bsp_pins_user_pushbutton_cfg);

    Button_State_t retval;

    if (!last_user_button_state && this_button_state)
    {
        retval = BUTTON_STATE_JUST_PRESSED;
    }
    else if (last_user_button_state && this_button_state)
    {
        retval = BUTTON_STATE_PRESSED;
    }
    else if (last_user_button_state && !this_button_state)
    {
        retval = BUTTON_STATE_JUST_RELEASED;
    }
    else
    {
        retval = BUTTON_STATE_NOT_PRESSED;
    }

    last_user_button_state = this_button_state;

    return retval;
}

Button_State_t ble_enable_pushbutton_state()
{
    static bool last_ble_button_state = false;

    // TODO, code to debounce the switch

    // note: the button has a pullup and pressing the switch shorts it to ground, so the action is inverting
    const bool this_button_state = !gpio_read_pin(&bsp_pins_ble_en_pushbutton_cfg);

    Button_State_t retval;

    if (!last_ble_button_state && this_button_state)
    {
        retval = BUTTON_STATE_JUST_PRESSED;
    }
    else if (last_ble_button_state && this_button_state)
    {
        retval = BUTTON_STATE_PRESSED;
    }
    else if (last_ble_button_state && !this_button_state)
    {
        retval = BUTTON_STATE_JUST_RELEASED;
    }
    else
    {
        retval = BUTTON_STATE_NOT_PRESSED;
    }

    last_ble_button_state = this_button_state;

    return retval;
}
