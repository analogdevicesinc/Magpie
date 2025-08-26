/**
 * @file    time_helpers.h
 * @brief   A software module for comparing and manipulating time structures is represented here.
 */

#ifndef TIME_HELPERS_H_
#define TIME_HELPERS_H_

/* Includes ----------------------------------------------------------------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

/* Public types ------------------------------------------------------------------------------------------------------*/

// typedef so we don't need to write "struct" each time
typedef struct tm tm_t;

/**
 * @brief enumerated clock time comparison results are represented here. Two time can be equal, or the left-hand-side
 * of the comparison can be earlier or later than the right-hand-side of the comparison.
 */
typedef enum
{
    CLOCK_TIME_COMPARISON_TIMES_ARE_EQUAL = 0,
    CLOCK_TIME_COMPARISON_LHS_IS_EARLIER,
    CLOCK_TIME_COMPARISON_LHS_IS_LATER,
} Clock_Time_Comparison_t;

/* Public function declarations --------------------------------------------------------------------------------------*/

/**
 * @brief `time_helpers_get_default_time()` is a default time struct with year 2000 and all values set to their minimums.
 *
 * @retval a time struct with a default datetime.
 */
tm_t time_helpers_get_default_time(void);

/**
 * @brief `time_helpers_compare_time(lhs, rhs)` is the enumerated comparison between times `lhs` and `rhs`.
 *
 * @pre `lhs` and `rhs` are normalized, meaning no 42nd of September or 25th hour of the day are allowed.
 *
 * @param lhs the left-hand-side of the comparison.
 *
 * @param rhs the right-hand-side of the comparison.
 *
 * @retval the enumerated Clock_Time_Comparison_t resulting from the comparison of `lhs` and `rhs`.
 */
Clock_Time_Comparison_t time_helpers_compare_time(tm_t lhs, tm_t rhs);

/**
 * @brief `time_helpers_tm_is_strictly_between_lhs_and_rhs(lhs, tm, rhs)` is true iff time `tm` falls strictly between
 * `lhs` and `rhs`.
 *
 * @pre `lhs`, `tm`, and `rhs` are all normalized valid time structs.
 *
 * @param lhs the left-hand-side of the comparison.
 *
 * @param tm the time in the middle to check if it falls after `lhs` and before `rhs`.
 *
 * @param rhs the right-hand-side of the comparison.
 *
 * @retval true iff the times are ordered `lhs` < `tm` < `rhs`.
 */
bool time_helpers_tm_is_strictly_between_lhs_and_rhs(tm_t lhs, tm_t tm, tm_t rhs);

/**
 * @brief `time_helpers_add_time(t0, d, h, m, s)` is the new time `t1` resulting from adding `d` days, `h` hours, `m` minutes,
 * and `s` seconds to initial time `t0`. Positive values for `d`, `h`, `m`, and `s` will move `t0` into the future while
 * negative values will result in moving `t0` towards the past.
 *
 * @pre `t0` is normalized, meaning no 42nd of September or 25th hour of the day are allowed.
 *
 * @param t0 the initial time to add to.
 *
 * @param days the number of days to add to `t0`.
 *
 * @param hours the number of hours to add to `t0`.
 *
 * @param minutes the number of minutes to add to `t0`.
 *
 * @param seconds the number of seconds to add to `t0`.
 *
 * @retval a new time `t1` given by `t0` with the given days, hours, minutes, and seconds added to it. If the given t0
 * and addends result in an invalid datatime, a default time is returned.
 */
tm_t time_helpers_add_time(tm_t t0, int days, int hours, int minutes, int seconds);

/**
 * @brief `time_helpers_tm_to_string(t, sb)` convertes the time-struct `t` to a string representation of the time and
 * stores the result in string buffer `sb` in the form "YYYYmmdd_HHMMSS".
 *
 * @param time the time struct to convert to a string.
 *
 * @param str_buff a c-style string buffer to store the time string into, must be at least 16 chars long.
 *
 * @post `str_buff` is mutated with the chars representing the time given in `time`.
 *
 * @retval the number of chars written into `str_buff`.
 */
size_t time_helpers_tm_to_string(tm_t time, char *str_buff);

/**
 * @brief `time_helpers_bcd8_byte_to_decimal(bcd)` is the decimal value of the 8-bit Binary Coded Decimal value `bcd`.
 *
 * @param bcd the BCD value to convert to decimal.
 *
 * @retval the decimal value of `bcd`.
 *
 * Uses the naive algorithm `res = ((bcd >> 4) * 10) + (bcd & 0x0Fu)`. This gives correct results when fed with valid
 * BCD values, but it will also convert invalid BCD codes such as 0xFA to nonsense values.
 */
int time_helpers_bcd8_byte_to_decimal(uint8_t bcd);

/**
 * @brief `time_helpers_decimal_0_99_to_bcd8(d)` is the 8-bit BCD value of `d` which is restricted to 0..99
 *
 * @param decimal the integer to convert to BCD, must be in [0..99],
 *
 * @retval the BCD value of `decimal`,
 */
uint8_t time_helpers_decimal_0_99_to_bcd8(int decimal);

#endif
