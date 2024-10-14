/**
 * @file    gnss_module.h
 * @brief   A software interface for interacting with the optional GNSS module is represented here.
 * @details An optional GNSS module may be connected to the main Magpie PCB via a 5-pin connector. The main
 * microcontroller communicates with the GNSS via UART. The GNSS is mainly used to get an accurate timestamp to use to
 * sync the Real Time Clock to GNSS time.
 *
 * This module requires:
 * - Exclusive use of UART2
 * - Exclusive use of pins P0.24, P0.24, P0.28, and P0.29
 */

#ifndef GNSS_MODULE_H__
#define GNSS_MODULE_H__

/* Includes ----------------------------------------------------------------------------------------------------------*/

#include "uart.h"

/* Public enumerations -----------------------------------------------------------------------------------------------*/

/**
 * @brief enumerated GNSS module errors are represented here.
 */
typedef enum
{
    GNSS_MODULE_ERROR_ALL_OK,
    GNSS_MODULE_UART_ERROR,
    GNSS_MODULE_TIMEOUT_ERROR,
    GNSS_MODULE_DATETIME_ERROR,
} GNSS_Module_Error_t;

/* Public function declarations --------------------------------------------------------------------------------------*/

/**
 * @brief `gnss_module_init()` initializes and powers on the GNSS module, this must be called before using the GNSS.
 *
 * @post the GNSS module is initialized, powered on, and ready to use.
 *
 * @retval `GNSS_MODULE_ERROR_ALL_OK` if initialization is successful, else and enumerated error.
 */
GNSS_Module_Error_t gnss_module_init();

/**
 * @brief `gnss_module_deinit()` de-initializes and powers down the GNSS module. The GNSS module must then be
 * re-initialized before using it again.
 *
 * @post the GNSS module is de-initialized and powered down, and all related GPIO pin are set to high-Z.
 *
 * @retval `GNSS_MODULE_ERROR_ALL_OK` if de-initialization is successful, else and enumerated error.
 */
GNSS_Module_Error_t gnss_module_deinit();

/**
 * @brief `gnss_module_sync_RTC_to_GNSS_time(t)` syncs the Real Time Clock module to the time reported by the GNSS
 * module. If it takes longer than `t` seconds the function is aborted.
 *
 * @pre The GNSS and RTC modules are both successfuly initialized, the GNSS module is enabled, and TODO add aprecondition relating to a mandatory GPS fix (need a function to report the fix quality too)
 *
 * @param timeout_sec the timeout period in seconds, if it takes at least this long to get a GPS fix and sync the RTC,
 * the function aborts and returns an error code.
 *
 * @post the RTC module is syncronized to the current UTC time as reported by the GNSS module. Calling this function
 * will mutate the time kept by the RTC module if the sync is successful (otherwise the RTC time is left unchanged).
 *
 * @retval `GNSS_MODULE_ERROR_ALL_OK` if initialization is successful, else and enumerated error.
 */
GNSS_Module_Error_t gnss_module_sync_RTC_to_GNSS_time(int timeout_sec);

#endif
