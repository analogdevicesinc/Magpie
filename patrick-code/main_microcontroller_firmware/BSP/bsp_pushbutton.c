
/* Private includes --------------------------------------------------------------------------------------------------*/

#include "mxc_device.h"

#include "board.h"
#include "bsp_pins.h"
#include "bsp_pushbutton.h"
#include "mxc_delay.h"
#include "tmr.h"
#include "nvic_table.h"

#include <stdio.h>

/* Public function definitions ---------------------------------------------------------------------------------------*/
static void usr_btn_debounce_timer_handler(void);

static bool last_user_button_state = false;
static bool last_ble_en_button_state = false;
static bool user_button_pressed = false;

// Timer configuration
mxc_tmr_cfg_t usr_tmr_cfg;
mxc_tmr_cfg_t ble_tmr_cfg;


void pushbuttons_init()
{
    MXC_GPIO_Config(&bsp_pins_user_pushbutton_cfg);
    MXC_GPIO_Config(&bsp_pins_ble_en_pushbutton_cfg);   


}

Button_State_t get_user_pushbutton_state()  //This will set the oneshot timer in motion
{
    bool this_button_state = user_button_pressed;
    
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
    else  //false in both last stae and current state
    {
        retval = BUTTON_STATE_NOT_PRESSED;
    }

    last_user_button_state = this_button_state;

    //reset button state
    user_button_pressed = false;

    return retval;
}

void start_user_btn_debounceTimer(void)
{
    MXC_NVIC_SetVector(USRBTN_TIMER_IRQn, usr_btn_debounce_timer_handler);
    NVIC_EnableIRQ(USRBTN_TIMER_IRQn);

    uint32_t debounceTicks = PeripheralClock / (128 * 1000) * DEBOUNCE_DELAY_MS; // Calculate timer ticks

    MXC_TMR_Shutdown(USRBTN_TIMER_ID);

    usr_tmr_cfg.pres = TMR_PRES_128;
    usr_tmr_cfg.mode = TMR_MODE_ONESHOT;
    usr_tmr_cfg.cmp_cnt = debounceTicks;
    usr_tmr_cfg.pol = 0;

    MXC_SYS_Reset_Periph(MXC_SYS_RESET_TIMER5);
    while (MXC_GCR->rstr0 & MXC_F_GCR_RSTR0_TIMER5) {}
    MXC_SYS_ClockEnable(MXC_SYS_PERIPH_CLOCK_T5);
    MXC_GPIO_Config(&gpio_cfg_tmr5);

    MXC_TMR_Init(USRBTN_TIMER_ID, &usr_tmr_cfg);
    MXC_TMR_Start(USRBTN_TIMER_ID);

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

/*************************** Timer Handler ************************************ */

//this happens after some ms of the debounce time specified
static void usr_btn_debounce_timer_handler()  
{    
    // Stop timer
    MXC_TMR_ClearFlags(USRBTN_TIMER_ID);   //clear timer

    if (!gpio_read_pin(&bsp_pins_user_pushbutton_cfg))  //read if button is pulled low (i.e. pushed)
    {
        user_button_pressed = true;        
    }
    else
    {
        user_button_pressed = false;
    }   
    printf("Timer handler fired\n");     
}

/******************************************************************************/
/** 
 * NOTE: This weak definition is included to support Push Button interrupts in
 *       case the user does not define this interrupt handler in their application.
 **/
__weak void GPIO0_IRQHandler(void)
{
    MXC_GPIO_Handler(MXC_GPIO_GET_IDX(MXC_GPIO0));
}

// __weak void TMR5_IRQHandler(void)
// {
//     NVIC_DisableIRQ(TMR5_IRQn);
//     NVIC_ClearPendingIRQ(TMR5_IRQn);
// }

