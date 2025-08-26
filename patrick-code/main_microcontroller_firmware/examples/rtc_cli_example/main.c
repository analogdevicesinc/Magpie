
/* Private includes --------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "cli.h"
#include "mxc_delay.h"

#include "board.h"
#include "bsp_i2c.h"
#include "bsp_pins.h"
#include "bsp_status_led.h"
#include "bsp_uart.h"
#include "real_time_clock.h"

#include <string.h>

/* Private defines ---------------------------------------------------------------------------------------------------*/

#define CUSTOM_COMMANDS_ARRAY_SIZE (2)

/* Private function declarations -------------------------------------------------------------------------------------*/

int cli_set_rtc(int argc, char *argv[]);

int cli_show_rtc(int argc, char *argv[]);

// the error handler simply rapidly blinks the given LED color forever
static void error_handler(Status_LED_Color_t c);

/* Private variables -------------------------------------------------------------------------------------------------*/

const command_t cli_commands[CUSTOM_COMMANDS_ARRAY_SIZE] = {
    {
        "set_rtc",
        "[set_rtc] [year] [month] [day] [hour] [min] [sec]",
        "Sets Datetime to the given UTC time, hour is 0-23. min is 0-59,sec is 0-59",
        cli_set_rtc,
    },
    {
        "show_rtc",
        "show_rtc",
        "Prints the current RTC time to the terminal",
        cli_show_rtc,
    },
};

/* Public function definitions ---------------------------------------------------------------------------------------*/

int main(void)
{
    bsp_console_uart_init();

    // simple LED pattern for a visual indication of a reset
    status_led_set(STATUS_LED_COLOR_RED, true);
    MXC_Delay(500000);
    status_led_set(STATUS_LED_COLOR_GREEN, true);
    MXC_Delay(500000);
    status_led_set(STATUS_LED_COLOR_BLUE, true);
    MXC_Delay(1000000);
    status_led_all_off();

    printf("\n*********** RTC CLI example ***********\n\n");

    if (bsp_3v3_i2c_init() != E_NO_ERROR)
    {
        printf("[ERROR]--> I2C init\n");
        error_handler(STATUS_LED_COLOR_BLUE);
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

    printf("\nRTC time at startup: ");
    cli_show_rtc(0, NULL); // manually call the function which is normally called automatically by the CLI
    printf("\n");

    MXC_CLI_Init(bsp_uart_console_uart_handle, cli_commands, CUSTOM_COMMANDS_ARRAY_SIZE);

    while (1)
    {
        // everything is handled by the CLI
        MXC_Delay(1000000);
    }
}

/* Private function definitions --------------------------------------------------------------------------------------*/

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

int cli_set_rtc(int argc, char *argv[])
{
    // fail if wrong number of args
    if (argc != 7)
    {
        return -1;
    }

    struct tm newTime = {
        .tm_year = atoi(argv[1]) - 1900U,
        .tm_mon = atoi(argv[2]) - 1U,
        .tm_mday = atoi(argv[3]),
        .tm_hour = atoi(argv[4]),
        .tm_min = atoi(argv[5]),
        .tm_sec = atoi(argv[6])};

    if (real_time_clock_set_datetime(&newTime) != E_NO_ERROR)
    {
        return -1;
    }

    return 0;
}

int cli_show_rtc(int argc, char *argv[])
{
    char str_buff[100];

    tm_t t0;

    if (real_time_clock_get_datetime(&t0) != E_NO_ERROR)
    {
        return -1;
    }

    time_helpers_tm_to_string(t0, str_buff);

    printf("%s\n", str_buff);

    return 0;
}
