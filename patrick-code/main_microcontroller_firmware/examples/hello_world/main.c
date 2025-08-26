/**
 * @file    main.c
 * @brief   Hello World!
 * @details This example uses RTT to print to a terminal and flashes the LEDs.
 */

#include <stdio.h>

#include "mxc_delay.h"

#include "bsp_uart.h"
#include "bsp_status_led.h"

#ifdef TERMINAL_IO_USE_SEGGER_RTT
#include "SEGGER_RTT.h"
#endif

Status_LED_Color_t next_color(Status_LED_Color_t this_color)
{
    switch (this_color)
    {
    case STATUS_LED_COLOR_RED:
        return STATUS_LED_COLOR_GREEN;
    case STATUS_LED_COLOR_GREEN:
        return STATUS_LED_COLOR_BLUE;
    case STATUS_LED_COLOR_BLUE:
    default:
        return STATUS_LED_COLOR_RED;
    }
}

int main(void)
{
#ifdef TERMINAL_IO_USE_CONSOLE_UART
    bsp_console_uart_init();
#endif

    int count = 0;

    printf("***********Hello Magpie!***********\n");

    Status_LED_Color_t this_color = STATUS_LED_COLOR_RED;

    while (1)
    {
        status_led_toggle(this_color);
        this_color = next_color(this_color);

        MXC_Delay(500000);

        printf("count = %d\n", count++);
    }
}
