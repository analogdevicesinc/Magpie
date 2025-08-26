/**
 * @file      bsp_pushbutton.h
 * @brief     A software module for reading the physical pushbuttons is represented here.
 *
 * This module requires:
 * - Exclusive use of pins P0.5 and P0.8
 */

#ifndef BSP_PUSHBUTTON_H_INCLUDED__
#define BSP_PUSHBUTTON_H_INCLUDED__

/* Defines --------------------------------------------------------------------------------------- */
#define USRBTN_TIMER_ID MXC_TMR5  // Timer 5
#define USRBTN_TIMER_IRQn TMR5_IRQn

#define DEBOUNCE_DELAY_MS 2  // 2 ms debounce delay

/* Public enumerations -----------------------------------------------------------------------------------------------*/

typedef enum
{
    BSP_PUSHBUTTON_USER_BUTTON,
    BSP_PUSHBUTTON_BLE_ENABLE,
} BSP_Pushbutton_t;

/**
 * @brief enumerated button states are represented here.
 */
typedef enum
{
    BUTTON_STATE_NOT_PRESSED,
    BUTTON_STATE_JUST_PRESSED,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_JUST_RELEASED,
} Button_State_t;

/* Public function declarations --------------------------------------------------------------------------------------*/

/**
 * @brief `pushbuttons_init()` initializes the pushbuttons.
 *
 * @post the pushbuttons are ready to be read.
 */
void pushbuttons_init(void);

/**
 * @brief `get_user_pushbutton_state()` is the enumerated state of the user pushbutton.
 *
 * @retval get the enumerated state of the pushbutton.
 */
Button_State_t get_user_pushbutton_state(void);

/**
 * @brief `ble_enable_pushbutton_state()` is the enumerated state of the BLE enable pushbutton.
 *
 * @retval the enumerated state of the pushbutton.
 */
Button_State_t ble_enable_pushbutton_state(void);

/**
 * @brief `start_user_btn_debounceTimer()` is to start a timer after the user button is pushed.
 *
 */
void start_user_btn_debounceTimer(void);

#endif
