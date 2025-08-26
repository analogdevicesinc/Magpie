/**
 * @file    bsp_status_led.h
 * @brief   A software module for controlling the status LEDs is represented here.
 *
 * This module requires:
 * - Exclusive use of pins P0.31, P1.15, and P1.15
 */

#ifndef BSP_STATUS_LED_H__
#define BSP_STATUS_LED_H__

/* Includes ----------------------------------------------------------------------------------------------------------*/

#include <stdbool.h>

/* Public enumerations -----------------------------------------------------------------------------------------------*/

/**
 * @brief enumerated status LED colors are represented here
 */
typedef enum
{
    STATUS_LED_COLOR_RED,
    STATUS_LED_COLOR_GREEN,
    STATUS_LED_COLOR_BLUE,
} Status_LED_Color_t;

/* Public function declarations --------------------------------------------------------------------------------------*/

/**
 * @brief `status_led_init()` initializes the status LEDs, this must be called before using the LEDs
 *
 * @post the status LEDs are initialized and ready to use, all LEDs are turned OFF.
 */
void status_led_init(void);

/**
 * @brief `status_led_set(c, s)` sets status LED color `c` to state `s`, true for ON and false for OFF
 *
 * @pre the status LEDs are initialized.
 *
 * @param color the color to set
 *
 * @param state the state to set the LED to
 *
 * @post the given LED is turned on or off according to the given state
 */
void status_led_set(Status_LED_Color_t color, bool state);

/**
 * @brief `status_led_toggle(c)` toggles LED color `c`, if it was OFF it turns ON, and vice versa
 *
 * @pre the status LEDs are initialized
 *
 * @param color the color to toggle
 *
 * @post the given LED is toggled
 */
void status_led_toggle(Status_LED_Color_t color);

/**
 * @brief `status_led_all_off()` turns all the status LEDs OFF
 *
 * @pre the status LEDs are initialized
 *
 * @post all the LEDs are turned OFF
 */
void status_led_all_off(void);

#endif
