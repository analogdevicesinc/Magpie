#include <gtest/gtest.h>
#include <gmock/gmock.h>

extern "C"
{
#include "time_helpers.h"
}

// a buffer to store strings into
static char str_buff[100];

TEST(TimeHelpersTest, time_to_string_yields_correct_string)
{
    tm_t t1 = {
        .tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 1, .tm_mon = 0, .tm_year = 0, .tm_yday = 0, .tm_isdst = -1};

    const auto len = time_helpers_tm_to_string(t1, str_buff);
    ASSERT_EQ(len, 15);
    ASSERT_STREQ(str_buff, "19000101_000000");

    t1.tm_sec = 56;
    t1.tm_min = 34;
    t1.tm_hour = 12;
    t1.tm_mday = 27;
    t1.tm_mon = 8; // September, remember the -1 in the date
    t1.tm_year = 2024 - 1900;

    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "20240927_123456");
}

TEST(TimeHelpersTest, compare_time_orders_correctly_for_basic_cases)
{
    tm_t t1 = {
        .tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 1, .tm_mon = 0, .tm_year = 0, .tm_yday = 0, .tm_isdst = -1};

    tm_t t2 = {
        .tm_sec = 1, .tm_min = 0, .tm_hour = 0, .tm_mday = 1, .tm_mon = 0, .tm_year = 0, .tm_yday = 0, .tm_isdst = -1};

    ASSERT_EQ(time_helpers_compare_time(t1, t2), CLOCK_TIME_COMPARISON_LHS_IS_EARLIER);

    t1.tm_sec = 2;
    ASSERT_EQ(time_helpers_compare_time(t1, t2), CLOCK_TIME_COMPARISON_LHS_IS_LATER);

    t2.tm_sec = 2;
    ASSERT_EQ(time_helpers_compare_time(t1, t2), CLOCK_TIME_COMPARISON_TIMES_ARE_EQUAL);

    t1.tm_min = 1;
    ASSERT_EQ(time_helpers_compare_time(t1, t2), CLOCK_TIME_COMPARISON_LHS_IS_LATER);

    t2.tm_hour = 1;
    ASSERT_EQ(time_helpers_compare_time(t1, t2), CLOCK_TIME_COMPARISON_LHS_IS_EARLIER);

    t1.tm_yday = 5;
    t2.tm_yday = 5;
    ASSERT_EQ(time_helpers_compare_time(t1, t2), CLOCK_TIME_COMPARISON_LHS_IS_EARLIER);

    t1.tm_hour = 23;
    ASSERT_EQ(time_helpers_compare_time(t1, t2), CLOCK_TIME_COMPARISON_LHS_IS_LATER);
}

TEST(TimeHelpersTest, compare_time_orders_correctly_for_complex_cases)
{
    tm_t t1, t2;

    // Test year difference
    t1 = {
        .tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 1, .tm_mon = 0, .tm_year = 120, .tm_yday = 0, .tm_isdst = -1};
    t2 = {
        .tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 1, .tm_mon = 0, .tm_year = 121, .tm_yday = 0, .tm_isdst = -1};

    ASSERT_EQ(time_helpers_compare_time(t1, t2), CLOCK_TIME_COMPARISON_LHS_IS_EARLIER);
    ASSERT_EQ(time_helpers_compare_time(t2, t1), CLOCK_TIME_COMPARISON_LHS_IS_LATER);

    // Test month difference
    t1 = {
        .tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 31, .tm_mon = 0, .tm_year = 120, .tm_wday = 0, .tm_yday = 0, .tm_isdst = -1}; // 2020-01-31
    t2 = {
        .tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 1, .tm_mon = 1, .tm_year = 120, .tm_wday = 0, .tm_yday = 0, .tm_isdst = -1}; // 2020-02-01
    ASSERT_EQ(time_helpers_compare_time(t1, t2), CLOCK_TIME_COMPARISON_LHS_IS_EARLIER);

    // Test day difference at year boundary
    t1 = {
        .tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 31, .tm_mon = 11, .tm_year = 120, .tm_wday = 0, .tm_yday = 0, .tm_isdst = -1}; // 2020-12-31
    t2 = {
        .tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 1, .tm_mon = 0, .tm_year = 121, .tm_wday = 0, .tm_yday = 0, .tm_isdst = -1}; // 2021-01-01
    ASSERT_EQ(time_helpers_compare_time(t1, t2), CLOCK_TIME_COMPARISON_LHS_IS_EARLIER);

    // Test hour difference at day boundary
    t1 = {
        .tm_sec = 59, .tm_min = 59, .tm_hour = 23, .tm_mday = 1, .tm_mon = 0, .tm_year = 120, .tm_wday = 0, .tm_yday = 0, .tm_isdst = -1};
    t2 = {
        .tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 2, .tm_mon = 0, .tm_year = 120, .tm_wday = 0, .tm_yday = 0, .tm_isdst = -1};
    ASSERT_EQ(time_helpers_compare_time(t1, t2), CLOCK_TIME_COMPARISON_LHS_IS_EARLIER);

    // Test minute and second differences
    t1 = {
        .tm_sec = 0, .tm_min = 30, .tm_hour = 12, .tm_mday = 1, .tm_mon = 0, .tm_year = 120, .tm_wday = 0, .tm_yday = 0, .tm_isdst = -1};
    t2 = {
        .tm_sec = 1, .tm_min = 30, .tm_hour = 12, .tm_mday = 1, .tm_mon = 0, .tm_year = 120, .tm_wday = 0, .tm_yday = 0, .tm_isdst = -1};
    ASSERT_EQ(time_helpers_compare_time(t1, t2), CLOCK_TIME_COMPARISON_LHS_IS_EARLIER);

    t1 = {
        .tm_sec = 59, .tm_min = 30, .tm_hour = 12, .tm_mday = 1, .tm_mon = 0, .tm_year = 120, .tm_wday = 0, .tm_yday = 0, .tm_isdst = -1};
    t2 = {
        .tm_sec = 0, .tm_min = 31, .tm_hour = 12, .tm_mday = 1, .tm_mon = 0, .tm_year = 120, .tm_wday = 0, .tm_yday = 0, .tm_isdst = -1};
    ASSERT_EQ(time_helpers_compare_time(t1, t2), CLOCK_TIME_COMPARISON_LHS_IS_EARLIER);

    // Test equality
    t1 = {
        .tm_sec = 45, .tm_min = 30, .tm_hour = 10, .tm_mday = 15, .tm_mon = 5, .tm_year = 120, .tm_wday = 0, .tm_yday = 0, .tm_isdst = -1};
    t2 = t1;
    ASSERT_EQ(time_helpers_compare_time(t1, t2), CLOCK_TIME_COMPARISON_TIMES_ARE_EQUAL);
}

TEST(TimeHelpersTest, test_for_strict_betweenness_is_correct_for_basic_cases)
{
    tm_t t1 = {
        .tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 1, .tm_mon = 0, .tm_year = 120, .tm_wday = 0, .tm_yday = 0, .tm_isdst = -1};
    tm_t t2 = {
        .tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 1, .tm_mon = 0, .tm_year = 121, .tm_wday = 0, .tm_yday = 0, .tm_isdst = -1};
    tm_t t3 = {
        .tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 1, .tm_mon = 0, .tm_year = 122, .tm_wday = 0, .tm_yday = 0, .tm_isdst = -1};

    ASSERT_EQ(time_helpers_tm_is_strictly_between_lhs_and_rhs(t1, t2, t3), true);
}

TEST(TimeHelpersTest, test_for_strict_betweenness_is_correct_for_complex_cases)
{
    tm_t t1, t2, t3;

    // Test year boundary
    t1 = {
        .tm_sec = 59, .tm_min = 59, .tm_hour = 23, .tm_mday = 31, .tm_mon = 11, .tm_year = 120, .tm_wday = 0, .tm_yday = 365, .tm_isdst = -1}; // 2020-12-31 23:59:59
    t2 = {
        .tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 1, .tm_mon = 0, .tm_year = 121, .tm_wday = 0, .tm_yday = 0, .tm_isdst = -1}; // 2021-01-01 00:00:00
    t3 = {
        .tm_sec = 1, .tm_min = 0, .tm_hour = 0, .tm_mday = 1, .tm_mon = 0, .tm_year = 121, .tm_wday = 0, .tm_yday = 0, .tm_isdst = -1}; // 2021-01-01 00:00:01
    ASSERT_TRUE(time_helpers_tm_is_strictly_between_lhs_and_rhs(t1, t2, t3));
    ASSERT_FALSE(time_helpers_tm_is_strictly_between_lhs_and_rhs(t1, t1, t3));
    ASSERT_FALSE(time_helpers_tm_is_strictly_between_lhs_and_rhs(t1, t3, t3));

    // Test month boundary
    t1 = {
        .tm_sec = 59, .tm_min = 59, .tm_hour = 23, .tm_mday = 31, .tm_mon = 0, .tm_year = 121, .tm_wday = 0, .tm_yday = 30, .tm_isdst = -1}; // 2021-01-31 23:59:59
    t2 = {
        .tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 1, .tm_mon = 1, .tm_year = 121, .tm_wday = 0, .tm_yday = 31, .tm_isdst = -1}; // 2021-02-01 00:00:00
    t3 = {
        .tm_sec = 1, .tm_min = 0, .tm_hour = 0, .tm_mday = 1, .tm_mon = 1, .tm_year = 121, .tm_wday = 0, .tm_yday = 31, .tm_isdst = -1}; // 2021-02-01 00:00:01
    ASSERT_TRUE(time_helpers_tm_is_strictly_between_lhs_and_rhs(t1, t2, t3));

    // Test day boundary
    t1 = {
        .tm_sec = 59, .tm_min = 59, .tm_hour = 23, .tm_mday = 1, .tm_mon = 2, .tm_year = 121, .tm_wday = 0, .tm_yday = 59, .tm_isdst = -1}; // 2021-03-01 23:59:59
    t2 = {
        .tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 2, .tm_mon = 2, .tm_year = 121, .tm_wday = 0, .tm_yday = 60, .tm_isdst = -1}; // 2021-03-02 00:00:00
    t3 = {
        .tm_sec = 1, .tm_min = 0, .tm_hour = 0, .tm_mday = 2, .tm_mon = 2, .tm_year = 121, .tm_wday = 0, .tm_yday = 60, .tm_isdst = -1}; // 2021-03-02 00:00:01
    ASSERT_TRUE(time_helpers_tm_is_strictly_between_lhs_and_rhs(t1, t2, t3));

    // Test hour boundary
    t1 = {
        .tm_sec = 59, .tm_min = 59, .tm_hour = 11, .tm_mday = 15, .tm_mon = 5, .tm_year = 121, .tm_wday = 0, .tm_yday = 165, .tm_isdst = -1}; // 2021-06-15 11:59:59
    t2 = {
        .tm_sec = 0, .tm_min = 0, .tm_hour = 12, .tm_mday = 15, .tm_mon = 5, .tm_year = 121, .tm_wday = 0, .tm_yday = 165, .tm_isdst = -1}; // 2021-06-15 12:00:00
    t3 = {
        .tm_sec = 1, .tm_min = 0, .tm_hour = 12, .tm_mday = 15, .tm_mon = 5, .tm_year = 121, .tm_wday = 0, .tm_yday = 165, .tm_isdst = -1}; // 2021-06-15 12:00:01
    ASSERT_TRUE(time_helpers_tm_is_strictly_between_lhs_and_rhs(t1, t2, t3));

    // Test minute and second boundaries
    t1 = {
        .tm_sec = 59, .tm_min = 58, .tm_hour = 10, .tm_mday = 1, .tm_mon = 7, .tm_year = 121, .tm_wday = 0, .tm_yday = 212, .tm_isdst = -1}; // 2021-08-01 10:58:59
    t2 = {
        .tm_sec = 0, .tm_min = 59, .tm_hour = 10, .tm_mday = 1, .tm_mon = 7, .tm_year = 121, .tm_wday = 0, .tm_yday = 212, .tm_isdst = -1}; // 2021-08-01 10:59:00
    t3 = {
        .tm_sec = 1, .tm_min = 59, .tm_hour = 10, .tm_mday = 1, .tm_mon = 7, .tm_year = 121, .tm_wday = 0, .tm_yday = 212, .tm_isdst = -1}; // 2021-08-01 10:59:01
    ASSERT_TRUE(time_helpers_tm_is_strictly_between_lhs_and_rhs(t1, t2, t3));
}

TEST(TimeHelpersTest, add_times_adding_zero_leaves_t0_unchanged)
{
    tm_t t0 = {
        .tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 1, .tm_mon = 0, .tm_year = 0, .tm_yday = 0, .tm_isdst = -1};

    time_helpers_add_time(t0, 0, 0, 0, 0);

    time_helpers_tm_to_string(t0, str_buff);
    ASSERT_STREQ(str_buff, "19000101_000000");

    t0 = {
        .tm_sec = 42, .tm_min = 13, .tm_hour = 21, .tm_mday = 30, .tm_mon = 7, .tm_year = 2024 - 1900, .tm_yday = 243, .tm_isdst = -1};

    time_helpers_add_time(t0, 0, 0, 0, 0);

    time_helpers_tm_to_string(t0, str_buff);
    ASSERT_STREQ(str_buff, "20240830_211342");
}

TEST(TimeHelpersTest, add_times_with_no_rollover)
{
    tm_t t0 = {
        .tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 1, .tm_mon = 0, .tm_year = 0, .tm_yday = 0, .tm_isdst = -1};

    int days = 1;
    int hours = 1;
    int minutes = 1;
    int seconds = 1;

    tm_t t1 = time_helpers_add_time(t0, days, hours, minutes, seconds);

    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "19000102_010101");

    tm_t t2 = {
        .tm_sec = 42, .tm_min = 13, .tm_hour = 3, .tm_mday = 30, .tm_mon = 7, .tm_year = 2024 - 1900, .tm_wday = 5, .tm_yday = 243, .tm_isdst = -1};

    tm_t t3 = time_helpers_add_time(t2, days, hours, minutes, seconds);

    time_helpers_tm_to_string(t3, str_buff);
    ASSERT_STREQ(str_buff, "20240831_041443");
}

TEST(TimeHelpersTest, add_seconds_with_rollover)
{
    tm_t t0 = {
        .tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 1, .tm_mon = 0, .tm_year = 0, .tm_yday = 0, .tm_isdst = -1};

    int days = 0;
    int hours = 0;
    int minutes = 0;
    int seconds = 60;

    tm_t t1 = time_helpers_add_time(t0, days, hours, minutes, seconds);
    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "19000101_000100");

    seconds = (20 * 60) + 34;
    t1 = time_helpers_add_time(t0, days, hours, minutes, seconds);
    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "19000101_002034");

    seconds = (3 * 3600) + (14 * 60) + 33;
    t1 = time_helpers_add_time(t0, days, hours, minutes, seconds);
    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "19000101_031433");

    tm_t t2 = {
        .tm_sec = 42, .tm_min = 13, .tm_hour = 3, .tm_mday = 30, .tm_mon = 7, .tm_year = 2024 - 1900, .tm_wday = 5, .tm_yday = 243, .tm_isdst = -1};

    seconds = 60;
    tm_t t3 = time_helpers_add_time(t2, days, hours, minutes, seconds);
    time_helpers_tm_to_string(t3, str_buff);
    ASSERT_STREQ(str_buff, "20240830_031442");
}

TEST(TimeHelpersTest, add_minutes_with_rollover)
{
    tm_t t0, t1;

    // Test simple minute addition
    t0 = {
        .tm_sec = 30, .tm_min = 45, .tm_hour = 12, .tm_mday = 15, .tm_mon = 5, .tm_year = 121, .tm_wday = 2, .tm_yday = 165, .tm_isdst = -1}; // 2021-06-15 12:45:30
    t1 = time_helpers_add_time(t0, 0, 0, 10, 0);
    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "20210615_125530");

    // Test minute rollover to next hour
    t0 = {
        .tm_sec = 30, .tm_min = 55, .tm_hour = 23, .tm_mday = 15, .tm_mon = 5, .tm_year = 121, .tm_wday = 2, .tm_yday = 165, .tm_isdst = -1}; // 2021-06-15 23:55:30
    t1 = time_helpers_add_time(t0, 0, 0, 10, 0);
    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "20210616_000530");

    // Test minute rollover to next day
    t0 = {
        .tm_sec = 30, .tm_min = 55, .tm_hour = 23, .tm_mday = 30, .tm_mon = 5, .tm_year = 121, .tm_wday = 3, .tm_yday = 180, .tm_isdst = -1}; // 2021-06-30 23:55:30
    t1 = time_helpers_add_time(t0, 0, 0, 10, 0);
    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "20210701_000530");

    // Test minute rollover to next month
    t0 = {
        .tm_sec = 30, .tm_min = 55, .tm_hour = 23, .tm_mday = 31, .tm_mon = 11, .tm_year = 121, .tm_wday = 5, .tm_yday = 364, .tm_isdst = -1}; // 2021-12-31 23:55:30
    t1 = time_helpers_add_time(t0, 0, 0, 10, 0);
    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "20220101_000530");

    // Test large minute addition
    t0 = {
        .tm_sec = 30, .tm_min = 0, .tm_hour = 0, .tm_mday = 1, .tm_mon = 0, .tm_year = 121, .tm_wday = 5, .tm_yday = 0, .tm_isdst = -1}; // 2021-01-01 00:00:30
    t1 = time_helpers_add_time(t0, 0, 0, 1440, 0);                                                                                       // Add 24 hours in minutes
    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "20210102_000030");

    // Test negative minute addition
    t0 = {
        .tm_sec = 30, .tm_min = 0, .tm_hour = 0, .tm_mday = 2, .tm_mon = 0, .tm_year = 121, .tm_wday = 6, .tm_yday = 1, .tm_isdst = -1}; // 2021-01-02 00:00:30
    t1 = time_helpers_add_time(t0, 0, 0, -10, 0);
    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "20210101_235030");
}

TEST(TimeHelpersTest, add_hours_with_rollover)
{
    tm_t t0, t1;

    // Test simple hour addition
    t0 = {
        .tm_sec = 30, .tm_min = 45, .tm_hour = 12, .tm_mday = 15, .tm_mon = 5, .tm_year = 121, .tm_wday = 2, .tm_yday = 165, .tm_isdst = -1}; // 2021-06-15 12:45:30
    t1 = time_helpers_add_time(t0, 0, 5, 0, 0);
    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "20210615_174530");

    // Test hour rollover to next day
    t0 = {
        .tm_sec = 30, .tm_min = 45, .tm_hour = 22, .tm_mday = 15, .tm_mon = 5, .tm_year = 121, .tm_wday = 2, .tm_yday = 165, .tm_isdst = -1}; // 2021-06-15 22:45:30
    t1 = time_helpers_add_time(t0, 0, 3, 0, 0);
    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "20210616_014530");

    // Test hour rollover to next month
    t0 = {
        .tm_sec = 30, .tm_min = 45, .tm_hour = 22, .tm_mday = 30, .tm_mon = 5, .tm_year = 121, .tm_wday = 3, .tm_yday = 180, .tm_isdst = -1}; // 2021-06-30 22:45:30
    t1 = time_helpers_add_time(t0, 0, 3, 0, 0);
    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "20210701_014530");

    // Test hour rollover to next year
    t0 = {
        .tm_sec = 30, .tm_min = 45, .tm_hour = 22, .tm_mday = 31, .tm_mon = 11, .tm_year = 121, .tm_wday = 5, .tm_yday = 364, .tm_isdst = -1}; // 2021-12-31 22:45:30
    t1 = time_helpers_add_time(t0, 0, 3, 0, 0);
    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "20220101_014530");

    // Test large hour addition
    t0 = {
        .tm_sec = 30, .tm_min = 45, .tm_hour = 0, .tm_mday = 1, .tm_mon = 0, .tm_year = 121, .tm_wday = 5, .tm_yday = 0, .tm_isdst = -1}; // 2021-01-01 00:45:30
    t1 = time_helpers_add_time(t0, 0, 48, 0, 0);                                                                                          // Add 48 hours
    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "20210103_004530");

    // Test negative hour addition
    t0 = {
        .tm_sec = 30, .tm_min = 45, .tm_hour = 2, .tm_mday = 1, .tm_mon = 0, .tm_year = 121, .tm_wday = 5, .tm_yday = 0, .tm_isdst = -1}; // 2021-01-01 02:45:30
    t1 = time_helpers_add_time(t0, 0, -3, 0, 0);
    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "20201231_234530");
}

TEST(TimeHelpersTest, add_days_with_rollover)
{
    tm_t t0, t1;

    // Test simple day addition
    t0 = {
        .tm_sec = 30, .tm_min = 45, .tm_hour = 12, .tm_mday = 15, .tm_mon = 5, .tm_year = 121, .tm_wday = 2, .tm_yday = 165, .tm_isdst = -1}; // 2021-06-15 12:45:30
    t1 = time_helpers_add_time(t0, 5, 0, 0, 0);
    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "20210620_124530");

    // Test day rollover to next month
    t0 = {
        .tm_sec = 30, .tm_min = 45, .tm_hour = 12, .tm_mday = 28, .tm_mon = 1, .tm_year = 121, .tm_wday = 0, .tm_yday = 58, .tm_isdst = -1}; // 2021-02-28 12:45:30
    t1 = time_helpers_add_time(t0, 3, 0, 0, 0);
    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "20210303_124530");

    // Test day rollover in a leap year
    t0 = {
        .tm_sec = 30, .tm_min = 45, .tm_hour = 12, .tm_mday = 28, .tm_mon = 1, .tm_year = 120, .tm_wday = 5, .tm_yday = 58, .tm_isdst = -1}; // 2020-02-28 12:45:30 (leap year)
    t1 = time_helpers_add_time(t0, 3, 0, 0, 0);
    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "20200302_124530");

    // Test day rollover to next year
    t0 = {
        .tm_sec = 30, .tm_min = 45, .tm_hour = 12, .tm_mday = 30, .tm_mon = 11, .tm_year = 121, .tm_wday = 4, .tm_yday = 363, .tm_isdst = -1}; // 2021-12-30 12:45:30
    t1 = time_helpers_add_time(t0, 3, 0, 0, 0);
    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "20220102_124530");

    // Test large day addition
    t0 = {
        .tm_sec = 30, .tm_min = 45, .tm_hour = 12, .tm_mday = 1, .tm_mon = 0, .tm_year = 121, .tm_wday = 5, .tm_yday = 0, .tm_isdst = -1}; // 2021-01-01 12:45:30
    t1 = time_helpers_add_time(t0, 365, 0, 0, 0);                                                                                          // Add 365 days
    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "20220101_124530");

    // Test negative day addition
    t0 = {
        .tm_sec = 30, .tm_min = 45, .tm_hour = 12, .tm_mday = 3, .tm_mon = 0, .tm_year = 121, .tm_wday = 0, .tm_yday = 2, .tm_isdst = -1}; // 2021-01-03 12:45:30
    t1 = time_helpers_add_time(t0, -5, 0, 0, 0);
    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "20201229_124530");
}

TEST(TimeHelpersTest, add_times_with_all_args_non_zero)
{
    tm_t t0, t1;

    // Test 1: Simple addition within same month
    t0 = {
        .tm_sec = 30, .tm_min = 45, .tm_hour = 12, .tm_mday = 15, .tm_mon = 5, .tm_year = 121, .tm_wday = 2, .tm_yday = 165, .tm_isdst = -1}; // 2021-06-15 12:45:30
    t1 = time_helpers_add_time(t0, 3, 4, 30, 45);
    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "20210618_171615");

    // Test 2: Rollover to next month
    t0 = {
        .tm_sec = 59, .tm_min = 59, .tm_hour = 23, .tm_mday = 28, .tm_mon = 1, .tm_year = 121, .tm_wday = 0, .tm_yday = 58, .tm_isdst = -1}; // 2021-02-28 23:59:59
    t1 = time_helpers_add_time(t0, 1, 0, 0, 1);
    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "20210302_000000");

    // Test 3: Rollover to next year
    t0 = {
        .tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 31, .tm_mon = 11, .tm_year = 121, .tm_wday = 5, .tm_yday = 364, .tm_isdst = -1}; // 2021-12-31 00:00:00
    t1 = time_helpers_add_time(t0, 0, 24, 0, 0);
    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "20220101_000000");

    // Test 4: Leap year (2020)
    t0 = {
        .tm_sec = 59, .tm_min = 59, .tm_hour = 23, .tm_mday = 28, .tm_mon = 1, .tm_year = 120, .tm_wday = 5, .tm_yday = 58, .tm_isdst = -1}; // 2020-02-28 23:59:59
    t1 = time_helpers_add_time(t0, 1, 0, 0, 1);
    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "20200301_000000");

    // Test 5: Large addition
    t0 = {
        .tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 1, .tm_mon = 0, .tm_year = 121, .tm_wday = 5, .tm_yday = 0, .tm_isdst = -1}; // 2021-01-01 00:00:00
    t1 = time_helpers_add_time(t0, 400, 25, 61, 61);
    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "20220206_020201");

    // Test 6: Negative addition
    t0 = {
        .tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 1, .tm_mon = 2, .tm_year = 121, .tm_wday = 1, .tm_yday = 59, .tm_isdst = -1}; // 2021-03-01 00:00:00
    t1 = time_helpers_add_time(t0, -1, -1, -1, -1);
    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "20210227_225859");

    // Test 7: Extreme addition
    t0 = {
        .tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 1, .tm_mon = 0, .tm_year = 120, .tm_wday = 3, .tm_yday = 0, .tm_isdst = -1}; // 2020-01-01 00:00:00
    t1 = time_helpers_add_time(t0, 366, 48, 120, 3661);
    time_helpers_tm_to_string(t1, str_buff);
    ASSERT_STREQ(str_buff, "20210103_030101");
}

TEST(TimeHelpersTest, check_bcd8_to_decimal)
{
    ASSERT_EQ(time_helpers_bcd8_byte_to_decimal(0x00), 0);
    ASSERT_EQ(time_helpers_bcd8_byte_to_decimal(0x01), 1);
    ASSERT_EQ(time_helpers_bcd8_byte_to_decimal(0x07), 7);

    ASSERT_EQ(time_helpers_bcd8_byte_to_decimal(0x10), 10);

    ASSERT_EQ(time_helpers_bcd8_byte_to_decimal(0x37), 37);
    ASSERT_EQ(time_helpers_bcd8_byte_to_decimal(0x99), 99);
}

TEST(TimeHelpersTest, check_decimal_to_bcd8)
{
    ASSERT_EQ(time_helpers_decimal_0_99_to_bcd8(0), 0x00);
    ASSERT_EQ(time_helpers_decimal_0_99_to_bcd8(1), 0x01);
    ASSERT_EQ(time_helpers_decimal_0_99_to_bcd8(07), 0x07);

    ASSERT_EQ(time_helpers_decimal_0_99_to_bcd8(10), 0x10);

    ASSERT_EQ(time_helpers_decimal_0_99_to_bcd8(37), 0x37);
    ASSERT_EQ(time_helpers_decimal_0_99_to_bcd8(99), 0x99);
}

TEST(TimeHelpersTest, bcd_to_decimal_and_vice_versa_are_inverse_funcs_0_to_99)
{
    // check that dec-to-bcd is the inverse of bcd-to-dec
    for (int i = 0; i <= 99; i++)
    {
        ASSERT_EQ(time_helpers_bcd8_byte_to_decimal(time_helpers_decimal_0_99_to_bcd8(i)), i);
    }

    // and also the other way around
    for (int tens = 0; tens <= 9; tens++)
    {
        for (int ones = 0; ones <= 9; ones++)
        {
            const uint8_t bcd = (tens << 4) | ones;
            ASSERT_EQ(time_helpers_decimal_0_99_to_bcd8(time_helpers_bcd8_byte_to_decimal(bcd)), bcd);
        }
    }
}
