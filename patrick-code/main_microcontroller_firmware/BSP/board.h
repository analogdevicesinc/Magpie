/**
 * @file      board.h
 * @brief     Custom Board Support Package (BSP) for Magpie main board
 *
 * This module requires:
 * - Exclusive use of UART1, mapped to P1.12 and P1.13
 * - Exclusive use of pins P0.5, P0.8, P0.31, P1.14., P1.15, and P0.29
 */

#ifndef MAGPIE_BOARD_H_INCLUDED__
#define MAGPIE_BOARD_H_INCLUDED__

/* Includes ----------------------------------------------------------------------------------------------------------*/

#include <stdbool.h>

#include "bsp_uart.h"
#include "mxc_device.h"

/* Public defines ----------------------------------------------------------------------------------------------------*/

#define BOARD_CUSTOM (1)

#define CONSOLE_UART (1)
#define CONSOLE_BAUD (BSP_CONSOLE_UART_BAUD)

/* Public function declarations --------------------------------------------------------------------------------------*/

/**
 * @brief `Board_Init()` initializes the low level Board Support Package (BSP) peripherals. It is automatically called
 * by the MSDK provided `SystemInit()` function, which is called by the assembly startup routing. These startup
 * functions are called before execution jumps to `main()`.
 *
 * @retval `E_NO_ERROR` if initialization is successful, else an error code.
 */
int Board_Init(void);

/**
 * @brief `bsp_power_on_LDOs()` powers on all LDOs and regulators on the board. This function only controls the LDOs,
 * individual peripherals may still be powered off. Peripherals are power on and off by their respective drivers.
 *
 * @post the LDOs are powered on.
 */
void bsp_power_on_LDOs(void);

/**
 * @brief `bsp_power_off_LDOs()` powers off all LDOs and regulators on the board. Power to the main microcontroller
 * remains on.
 *
 * @post the LDOs are powered off.
 */
void bsp_power_off_LDOs(void);

/**
 * @brief `gpio_write_pin(p, s)` writes GPIO pin `p` to state `s`, true for pin HIGH and false for pin LOW.
 *
 * @pre pin `p` is configured as a GPIO output.
 *
 * @param pin the GPIO pin to write.
 *
 * @param state the state to write to the GPIO pin.
 *
 * @post the given pin is set to the given state.
 */
void gpio_write_pin(const mxc_gpio_cfg_t *pin, bool state);

/**
 * @brief `gpio_toggle_pin(p)` toggles GPIO pin `p`, if it was LOW, set it HIGH, and vice versa.
 *
 * @pre pin `p` is configured as a GPIO output.
 *
 * @param pin the GPIO pin to toggle.
 *
 * @post the pin is toggled to the opposite state.
 */
void gpio_toggle_pin(const mxc_gpio_cfg_t *pin);

/**
 * @brief `gpio_read_pin(p)` is the current state of GPIO pin `p`.
 *
 * @param pin the pin to read
 *
 * @retval true if the given pin is HIGH, false if it's LOW
 */
bool gpio_read_pin(const mxc_gpio_cfg_t *pin);

#endif
