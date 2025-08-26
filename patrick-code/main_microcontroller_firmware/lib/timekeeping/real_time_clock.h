/**
 * @file    real_time_clock.h
 * @brief   A software interface for interacting with the Real Time Clock (RTC) is represented here.
 * @details The RTC is used to tell time, for scheduling recordings and events.
 *
 * This module requires:
 * - Shared use of an I2C bus on the 3V3 domain using 7-bit address 0x68
 * - Exclusive use of pin P0.13
 */

#ifndef REALTIME_CLOCK_H_
#define REALTIME_CLOCK_H_

/* Includes ----------------------------------------------------------------------------------------------------------*/

#include "time_helpers.h"

/* Public function declarations --------------------------------------------------------------------------------------*/

/**
 * @brief `real_time_clock_init()` initializes the real time clock. the RTC is composed of both the external DS3231
 * chip and the onboard MAX32666 RTC.
 *
 * @pre the I2C bus on the 3.3V domain is configured as an I2C master and has pullup resistors to 3.3V.
 *
 * @post the system RTC is initialized and ready to use, the 32kHz clock from the DS3231 is enabled.
 *
 * @retval `E_NO_ERROR` if successful, else a negative error code.
 */
int real_time_clock_init(void);

/**
 * @brief `real_time_clock_set_datetime(t)` sets the real time clock to time `t`
 *
 * @pre the real time clock has been initialized.
 *
 * @param new_time the time structure to set as the new time, must be on or after year 2000.
 *
 * @post the real time clock is set to time `t` and immediately starts ticking forward in time.
 *
 * @retval `E_NO_ERROR` if successful, else a negative error code.
 */
int real_time_clock_set_datetime(const tm_t *new_time);

/**
 * @brief `real_time_clock_get_datetime(t)` stores the current real time clock time in time struct `t`.
 *
 * @pre the real time clock has been initialized.
 *
 * @param out_time a pointer to the time structure to fill with the current time, will be mutated.
 *
 * @post the current time is stored in `t`
 *
 * @retval `E_NO_ERROR` if successful, else a negative error code.
 */
int real_time_clock_get_datetime(tm_t *out_time);

/**
 * @brief `real_time_clock_get_milliseconds(ms)` stores the current millisecond value in `ms`, this is always in [0..999].
 *
 * @pre the real time clock has been initialized.
 *
 * @param out_msec a pointer to an integer to store the millisecond value in, this will be mutated.
 *
 * @post the current millisecond value in [0..999] is stored in `out_msec`.
 *
 * @retval `E_NO_ERROR` if successful, else a negative error code.
 */
int real_time_clock_get_milliseconds(int *out_msec);

/**
 * @brief `real_time_clock_set_alarm(t)` sets an alarm to trigger an interrupt at time `t`
 *
 * @pre the real time clock has been initialized, and `t` is in the future.
 *
 * @param alarm_time the time structure to set the alarm.
 *
 * @post an alarm is set to trigger the DS3231 interrupt at time `t`.
 *
 * @retval `E_NO_ERROR` if successful, else a negative error code.
 *
 * TODO: should we allow setting both alarm A and alarm B? maybe make an enum to keep track? There is also the internal
 * MAX32666 RTC alarms we could potentially use? Depends on what we need to do with it, don't overcomplicate it.
 */
int real_time_clock_set_alarm(const tm_t *alarm_time);

/**
 * TODO: create a function to register a callback for RTC interrupts, we also need to create a funciton pointer type
 * for the callbacks, what should the signature be?
 */
// int real_time_clock_set_callback(rtc_callback_t callback);

#endif
