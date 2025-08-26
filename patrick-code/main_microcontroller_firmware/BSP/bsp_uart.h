/**
 * @file      bsp_uart.h
 * @brief     This module is responsible for initializing and de-initializing the UART busses used by the sytem.
 *
 * This module requires:
 * - Shared use of UART0, UART1, and UART2
 * - Shared use of UART pins P0.9, P0.10, P0.28, P0.29, P1,12, and P1.13
 */

#ifndef BSP_UART_H_INCLUDED__
#define BSP_UART_H_INCLUDED__

/* Includes ----------------------------------------------------------------------------------------------------------*/

#include "uart.h"

/* Public definitions ------------------------------------------------------------------------------------------------*/

#define BSP_BLE_UART_BAUD (115200)

#define BSP_CONSOLE_UART_BAUD (115200)

#define BSP_GNSS_UART_BAUD (115200)

/* Public variables --------------------------------------------------------------------------------------------------*/

extern mxc_uart_regs_t *bsp_uart_ble_uart_handle;

extern mxc_uart_regs_t *bsp_uart_console_uart_handle;

extern mxc_uart_regs_t *bsp_uart_gps_uart_handle;

/* Public function declarations --------------------------------------------------------------------------------------*/

/**
 * @brief `bsp_ble_uart_init()` initializes and starts Nordic UART.
 *
 * @post UART0 is initialized and ready to use. The GPIO pins associated with the bus are configured for UART operation.
 *
 * @retval Success/Fail, see MXC_Error_Codes for a list of return codes.
 */
int bsp_ble_uart_init(void);

/**
 * @brief `bsp_ble_uart_deinit()` de-initializes the Nordic UART and sets the associated pins to high-Z.
 *
 * @post the Nordic UART is de-initialized. The GPIO pins associated with the bus are hig-Z.
 *
 * @retval Success/Fail, see MXC_Error_Codes for a list of return codes.
 */
int bsp_ble_uart_deinit(void);

/**
 * @brief `bsp_console_uart_init()` initializes and starts Console UART.
 *
 * @post UART1 is initialized and ready to use. The GPIO pins associated with the bus are configured for UART operation.
 *
 * @retval Success/Fail, see MXC_Error_Codes for a list of return codes.
 */
int bsp_console_uart_init(void);

/**
 * @brief `bsp_console_uart_deinit()` de-initializes the Console UART and sets the associated pins to high-Z.
 *
 * @post the Console UART is de-initialized. The GPIO pins associated with the bus are hig-Z.
 *
 * @retval Success/Fail, see MXC_Error_Codes for a list of return codes.
 */
int bsp_console_uart_deinit(void);

/**
 * @brief `bsp_gnss_uart_init()` initializes and starts GNSS module UART.
 *
 * @post UART2 is initialized and ready to use. The GPIO pins associated with the bus are configured for UART operation.
 *
 * @retval Success/Fail, see MXC_Error_Codes for a list of return codes.
 */
int bsp_gnss_uart_init(void);

/**
 * @brief `bsp_gnss_uart_deinit()` de-initializes the GNSS module UART and sets the associated pins to high-Z.
 *
 * @post the GNSS UART is de-initialized. The GPIO pins associated with the bus are hig-Z.
 *
 * @retval Success/Fail, see MXC_Error_Codes for a list of return codes.
 */
int bsp_gnss_uart_deinit(void);

#endif
