/**
 * @file    main.c
 * @brief   CLI Hello World example
 * @details This example uses the CLI via UART to toggle the status LEDs.
 */

/* Private includes --------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "afe_control.h"
#include "board.h"
#include "bsp_i2c.h"
#include "bsp_pins.h"
#include "bsp_status_led.h"
#include "bsp_uart.h"

#include "cli.h"
#include "mxc_delay.h"
//#include"SEGGER_RTT.h"

/* Private defines ---------------------------------------------------------------------------------------------------*/

#define CUSTOM_COMMANDS_ARRAY_SIZE (2)

/* Private function declarations -------------------------------------------------------------------------------------*/

/**
 * Examples:
 * afe_pwr 0 0 -> channel 0 off
 * afe_pwr 0 1 -> channel 0 on
 * afe_pwr 1 0 -> channel 1 off
 * afe_pwr 1 1 -> channel 1 on
 */
int cli_afe_pwr(int argc, char *argv[]);

/**
 * Examples:
 * afe_gain 0 5 -> channel 0 gain to 5dB
 * afe_gain 1 25 -> channel 1 gain to 25dB
 *
 * NOTE you must first power on a channel before you can set the gain
 */
int cli_afe_gain(int argc, char *argv[]);

/* Private variables -------------------------------------------------------------------------------------------------*/

const command_t cli_commands[CUSTOM_COMMANDS_ARRAY_SIZE] = {
    {
        "afe_pwr",
        "afe_pwr [0/1] [0/1]",
        "afe_pwr [channel] [state] sets the given channel to the given state",
        cli_afe_pwr,
    },
    {
        "afe_gain",
        "afe_gain [0/1] [5, 10, 15, ... 40]",
        "afe_gain [channel] [gain] sets the gain of the channel, the channel must be powered on first",
        cli_afe_gain,
    },
};

/* Public function definitions ---------------------------------------------------------------------------------------*/

int main(void)
{
    printf("*********** CLI AFE Control Example ***********\n\n");

    if (bsp_3v3_i2c_init() != E_NO_ERROR)
    {
        printf("[ERROR]--> 3.3V I2C init\n");
    }
    else
    {
        printf("[SUCCESS]--> 3.3V I2C init\n");
    }

    bsp_power_on_LDOs();

    if (bsp_1v8_i2c_init() != E_NO_ERROR)
    {
        printf("[ERROR]--> 1.8V I2C init\n");
    }
    else
    {
        printf("[SUCCESS]--> 1.8V I2C init\n");
    }

    if (afe_control_init() != AFE_CONTROL_ERROR_ALL_OK)
    {
        printf("[ERROR]--> AFE control init\n");
    }
    else
    {
        printf("[SUCCESS]--> AFE control init\n");
    }

    MXC_CLI_Init(bsp_pins_console_uart_handle, cli_commands, CUSTOM_COMMANDS_ARRAY_SIZE);

    while (1)
    {
        // everything is handled by the CLI
        MXC_Delay(10000);
    }
}

/* Private function definitions --------------------------------------------------------------------------------------*/

int cli_afe_pwr(int argc, char *argv[])
{
    if (argc != 3)
    {
        return -1;
    }

    if (strlen(argv[1]) != 1 || strlen(argv[2]) != 1)
    {
        return -1;
    }

    const int channel = argv[1][0] - '0'; // integer 0 or 1 if the user entered correct input

    if (channel != 0 && channel != 1)
    {
        return -1;
    }

    const int state = argv[2][0] - '0'; // integer 0 or 1 assuming correct input

    if (state != 0 && state != 1)
    {
        return -1;
    }

    if (state == 0)
    {
        if (afe_control_disable(channel) != AFE_CONTROL_ERROR_ALL_OK)
        {
            printf("[ERROR]--> AFE channel %d power disable\n", channel);
        }
        else
        {
            printf("[SUCCESS]--> AFE channel %d power disable\n", channel);
        }
    }
    else // it must be 1
    {
        if (afe_control_enable(channel) != AFE_CONTROL_ERROR_ALL_OK)
        {
            printf("[ERROR]--> AFE channel %d power enable\n", channel);
        }
        else
        {
            printf("[SUCCESS]--> AFE channel %d power enable\n", channel);
        }
    }

    return 0;
}

int cli_afe_gain(int argc, char *argv[])
{
    if (argc != 3)
    {
        return -1;
    }

    if (strlen(argv[1]) != 1 || strlen(argv[2]) > 2)
    {
        return -1;
    }

    const int channel = argv[1][0] - '0'; // integer 0 or 1 if the user entered correct input

    if (channel != 0 && channel != 1)
    {
        return -1;
    }

    int gain;

    if (sscanf(argv[2], "%d", &gain) != 1)
    {
        return -1;
    }

    // gain must be in 5, 10, 15, ... 40
    if ((gain < 5) || (gain > 40) || ((gain % 5) != 0))
    {
        return -1;
    }

    const AFE_Control_Error_t result = afe_control_set_gain(channel, gain);

    if (result == AFE_CONTROL_ERROR_ALL_OK)
    {
        printf("[SUCCESS]--> AFE ch %d set gain to %d\n", channel, gain);
    }
    else if (result == AFE_CONTROL_ERROR_CHANNEL_NOT_ENABLED_ERROR)
    {
        printf("[ERROR]--> AFE ch %d must be enabled\n", channel);
    }
    else
    {
        printf("[ERROR]--> AFE ch %d set gain to %d\n", channel, gain);
    }

    return 0;
}
