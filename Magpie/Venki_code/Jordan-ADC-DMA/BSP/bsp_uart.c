
/* Private includes --------------------------------------------------------------------------------------------------*/

#include "bsp_pins.h"
#include "bsp_uart.h"
#include "mxc_errors.h"

/* Public function definitions ---------------------------------------------------------------------------------------*/

int bsp_ble_uart_init()
{
    MXC_GPIO_Config(&bsp_pins_ble_uart_active_cfg);

    MXC_SYS_ClockEnable(bsp_pins_ble_uart_clock_enum);

    return MXC_UART_Init(bsp_pins_ble_uart_handle, BSP_NORDIC_UART_BAUD, bsp_pins_ble_uart_map);
}

int bsp_ble_uart_deinit()
{
    MXC_GPIO_Config(&bsp_pins_ble_uart_high_z_cfg);

    MXC_SYS_ClockDisable(bsp_pins_ble_uart_clock_enum);

    return MXC_UART_Shutdown(bsp_pins_ble_uart_handle);
}

int bsp_console_uart_init()
{
    MXC_GPIO_Config(&bsp_pins_console_uart_active_cfg);

    MXC_SYS_ClockEnable(bsp_pins_console_uart_clock_enum);

    return MXC_UART_Init(bsp_pins_console_uart_handle, BSP_CONSOLE_UART_BAUD, bsp_pins_console_uart_map);
}

int bsp_console_uart_deinit()
{
    MXC_GPIO_Config(&bsp_pins_console_uart_high_z_cfg);

    MXC_SYS_ClockDisable(bsp_pins_console_uart_clock_enum);

    return MXC_UART_Shutdown(bsp_pins_console_uart_handle);
}

int bsp_gnss_uart_init()
{
    MXC_GPIO_Config(&bsp_pins_gps_uart_active_cfg);

    MXC_SYS_ClockEnable(bsp_pins_gps_uart_clock_enum);

    return MXC_UART_Init(bsp_pins_gps_uart_handle, BSP_GNSS_UART_BAUD, bsp_pins_gps_uart_map);
}

int bsp_gnss_uart_deinit()
{
    MXC_GPIO_Config(&bsp_pins_gps_uart_high_z_cfg);

    MXC_SYS_ClockDisable(bsp_pins_gps_uart_clock_enum);

    return MXC_UART_Shutdown(bsp_pins_gps_uart_handle);
}
