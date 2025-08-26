
/* Private includes --------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "mxc_delay.h"
#include "tmr.h"

#include "board.h"
#include "bsp_i2c.h"
#include "bsp_status_led.h"
#include "gnss_module.h"
#include "real_time_clock.h"

#ifdef TERMINAL_IO_USE_SEGGER_RTT
#include "SEGGER_RTT.h"
#endif

/* Private function declarations -------------------------------------------------------------------------------------*/

/**
 * print out the time held by the RTC module
 */
static void print_rtc_time(void);

// the error handler simply rapidly blinks the given LED color forever
static void error_handler(Status_LED_Color_t c);

/* Public function definitions ---------------------------------------------------------------------------------------*/

int main(void)
{
#ifdef TERMINAL_IO_USE_CONSOLE_UART
    bsp_console_uart_init();
#endif

    // simple LED pattern for a visual indication of a reset
    status_led_set(STATUS_LED_COLOR_RED, true);
    MXC_Delay(500000);
    status_led_set(STATUS_LED_COLOR_GREEN, true);
    MXC_Delay(500000);
    status_led_set(STATUS_LED_COLOR_BLUE, true);
    MXC_Delay(1000000);
    status_led_all_off();

    printf("\n*** GNSS -> RTC Clock sync example ***\n");

    if (gnss_module_init() != E_NO_ERROR)
    {
        printf("[ERROR]--> GNSS init\n");
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> GNSS init\n");
    }

    if (bsp_3v3_i2c_init() != E_NO_ERROR)
    {
        printf("[ERROR]--> I2C init\n");
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> I2C init\n");
    }

    if (real_time_clock_init() != E_NO_ERROR)
    {
        printf("[ERROR]--> RTC init\n");
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> RTC init\n");
    }

    // force the RTC to a default time, this way it will be obvious when we sync it back to the correct time with GPS
    tm_t t0 = time_helpers_get_default_time();
    if (real_time_clock_set_datetime(&t0) != E_NO_ERROR)
    {
        printf("[ERROR]--> RTC set time\n");
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> RTC set time\n");
    }

    printf("\nRTC default time before syncing to GPS:\n");
    print_rtc_time();

    printf("\n... Attempting to sync RTC to GPS, this can take some time ...\n");

    while (1)
    {
        const uint32_t gps_sync_timeout_secs = 20;
        const int res = gnss_module_sync_RTC_to_GNSS_time(gps_sync_timeout_secs);
        if (res == E_NO_ERROR)
        {
            printf("[SUCCESS]--> GNSS-RTC time sync\n");

            print_rtc_time();
        }
        else
        {
            printf("[ERROR]--> RTC time sync failure [%d]\n", res);
        }

        MXC_Delay(10);
    }
}

/* Private function definitions --------------------------------------------------------------------------------------*/

void print_rtc_time()
{
    char str_buff[100];

    tm_t t0;

    if (real_time_clock_get_datetime(&t0) != E_NO_ERROR)
    {
        printf("[ERROR]--> RTC get time\n");
        error_handler(STATUS_LED_COLOR_RED);
    }

    time_helpers_tm_to_string(t0, str_buff);

    printf("%s\n", str_buff);
}

void error_handler(Status_LED_Color_t color)
{
    status_led_all_off();

    const uint32_t fast_blink = 100000;
    while (true)
    {
        status_led_toggle(color);
        MXC_Delay(fast_blink);
    }
}
