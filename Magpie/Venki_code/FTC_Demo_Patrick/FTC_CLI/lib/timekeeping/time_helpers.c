
/* Private includes --------------------------------------------------------------------------------------------------*/

#include "time_helpers.h"

/* Public function definitions ---------------------------------------------------------------------------------------*/

tm_t time_helpers_get_default_time()
{
    const tm_t default_time = {
        .tm_hour = 0,
        .tm_isdst = -1,
        .tm_mday = 1,
        .tm_min = 0,
        .tm_mon = 0,
        .tm_sec = 0,
        .tm_wday = 0,
        .tm_yday = 0,
        .tm_year = 2000 - 1900, // use 2000 as the default year, because that's where the DS3231 starts
    };

    return default_time;
}

Clock_Time_Comparison_t time_helpers_compare_time(tm_t lhs, tm_t rhs)
{
    // convert to seconds since the unix epoch so we can do simple lt/gt/eq tests
    time_t lhs_ = mktime(&lhs);
    time_t rhs_ = mktime(&rhs);

    if (lhs_ < rhs_)
    {
        return CLOCK_TIME_COMPARISON_LHS_IS_EARLIER;
    }
    else if (lhs_ > rhs_)
    {
        return CLOCK_TIME_COMPARISON_LHS_IS_LATER;
    }
    else
    {
        return CLOCK_TIME_COMPARISON_TIMES_ARE_EQUAL;
    }
}

bool time_helpers_tm_is_strictly_between_lhs_and_rhs(tm_t lhs, tm_t tm, tm_t rhs)
{
    // convert to seconds since the unix epoch so we can do simple lt/gt/eq tests
    const time_t lhs_ = mktime(&lhs);
    const time_t tm_ = mktime(&tm);
    const time_t rhs_ = mktime(&rhs);

    return (lhs_ < tm_) && (tm_ < rhs_);
}

tm_t time_helpers_add_time(tm_t t0, int days, int hours, int minutes, int seconds)
{
    // convert to UTC seconds so we can do simple arithmetic
    time_t t = mktime(&t0) + (days * 86400) + (hours * 3600) + (minutes * 60) + seconds;

    if (t == -1) // sentinal value from mktime to indicate an error
    {
        return time_helpers_get_default_time();
    }

    // and then back to a time struct
    tm_t *t_ = localtime(&t);
    tm_t retval = *t_;
    return retval;
}

size_t time_helpers_tm_to_string(tm_t time, char *str_buff)
{
    return strftime(str_buff, 16, "%Y%m%d_%H%M%S", &time);
}

int time_helpers_bcd8_byte_to_decimal(uint8_t bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0x0Fu);
}

uint8_t time_helpers_decimal_0_99_to_bcd8(int decimal)
{
    return ((decimal / 10) << 4) | (decimal % 10);
}
