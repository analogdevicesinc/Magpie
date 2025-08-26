/**
 * @file    main.c
 * @brief   CLI Hello World example
 * @details This example uses the CLI via UART to toggle the status LEDs.
 */

/* Private includes --------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "bsp_pins.h"
#include "bsp_status_led.h"
#include "bsp_uart.h"

#include "cli.h"
#include "mxc_delay.h"

/* Private defines ---------------------------------------------------------------------------------------------------*/

#define CUSTOM_COMMANDS_ARRAY_SIZE (1)

/* Private function declarations -------------------------------------------------------------------------------------*/

int cli_led_toogle(int argc, char *argv[]);

/* Private variables -------------------------------------------------------------------------------------------------*/

const command_t cli_commands[CUSTOM_COMMANDS_ARRAY_SIZE] = {
    {
        "led_toggle",
        "[led_toggle r/g/b] on command line",
        "`led_toggle c` toggles LED color `c`",
        cli_led_toogle,
    },
};

/* Public function definitions ---------------------------------------------------------------------------------------*/

int main(void)
{
    bsp_console_uart_init();

    printf("***********Hello CLI!***********\n\n");

    MXC_CLI_Init(bsp_uart_console_uart_handle, cli_commands, CUSTOM_COMMANDS_ARRAY_SIZE);

    while (1)
    {
        // everything is handled by the CLI
        MXC_Delay(10000);
    }
}

/* Private function definitions --------------------------------------------------------------------------------------*/

int cli_led_toogle(int argc, char *argv[])
{
    // fail is wrong number of args
    if (argc != 2)
    {
        return -1;
    }

    // fail if the color given is more than 1 char, it should be 'r', 'g', or 'b' only
    if (strlen(argv[1]) != 1)
    {
        return -1;
    }

    const char color = argv[1][0];

    if (color == 'r')
    {
        status_led_toggle(STATUS_LED_COLOR_RED);
    }
    else if (color == 'g')
    {
        status_led_toggle(STATUS_LED_COLOR_GREEN);
    }
    else if (color == 'b')
    {
        status_led_toggle(STATUS_LED_COLOR_BLUE);
    }
    else // the given arg was something other than 'r', 'g', or 'b'
    {
        return -1;
    }

    return 0; // success
}
