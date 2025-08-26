
/* Private includes --------------------------------------------------------------------------------------------------*/

#include "bsp_pins.h"
#include "bsp_uart.h"
#include "mxc_errors.h"

/* Public variables --------------------------------------------------------------------------------------------------*/

mxc_uart_regs_t *bsp_uart_ble_uart_handle = MXC_UART0;

mxc_uart_regs_t *bsp_uart_console_uart_handle = MXC_UART1;

mxc_uart_regs_t *bsp_uart_gps_uart_handle = MXC_UART2;

/* Private variables -------------------------------------------------------------------------------------------------*/

static const sys_map_t bsp_uart_ble_uart_map = MAP_A;
static const mxc_sys_periph_clock_t bsp_uart_ble_uart_clock_enum = MXC_SYS_PERIPH_CLOCK_UART0;

static const sys_map_t bsp_uart_console_uart_map = MAP_B;
static const mxc_sys_periph_clock_t bsp_uart_console_uart_clock_enum = MXC_SYS_PERIPH_CLOCK_UART1;

static const sys_map_t bsp_uart_gps_uart_map = MAP_B;
static const mxc_sys_periph_clock_t bsp_uart_gps_uart_clock_enum = MXC_SYS_PERIPH_CLOCK_UART2;

/* Public function definitions ---------------------------------------------------------------------------------------*/

int bsp_ble_uart_init()
{
    MXC_GPIO_Config(&bsp_pins_ble_uart_active_cfg);

    MXC_SYS_ClockEnable(bsp_uart_ble_uart_clock_enum);

    return MXC_UART_Init(bsp_uart_ble_uart_handle, BSP_BLE_UART_BAUD, bsp_uart_ble_uart_map);
}

int bsp_ble_uart_deinit()
{
    MXC_GPIO_Config(&bsp_pins_ble_uart_high_z_cfg);

    MXC_SYS_ClockDisable(bsp_uart_ble_uart_clock_enum);

    return MXC_UART_Shutdown(bsp_uart_ble_uart_handle);
}

int bsp_console_uart_init()
{
    MXC_GPIO_Config(&bsp_pins_console_uart_active_cfg);

    MXC_SYS_ClockEnable(bsp_uart_console_uart_clock_enum);

    return MXC_UART_Init(bsp_uart_console_uart_handle, BSP_CONSOLE_UART_BAUD, bsp_uart_console_uart_map);
}

int bsp_console_uart_deinit()
{
    MXC_GPIO_Config(&bsp_pins_console_uart_high_z_cfg);

    MXC_SYS_ClockDisable(bsp_uart_console_uart_clock_enum);

    return MXC_UART_Shutdown(bsp_uart_console_uart_handle);
}

int bsp_gnss_uart_init()
{
    MXC_GPIO_Config(&bsp_pins_gps_uart_active_cfg);

    MXC_SYS_ClockEnable(bsp_uart_gps_uart_clock_enum);

    return MXC_UART_Init(bsp_uart_gps_uart_handle, BSP_GNSS_UART_BAUD, bsp_uart_gps_uart_map);
}

int bsp_gnss_uart_deinit()
{
    MXC_GPIO_Config(&bsp_pins_gps_uart_high_z_cfg);

    MXC_SYS_ClockDisable(bsp_uart_gps_uart_clock_enum);

    return MXC_UART_Shutdown(bsp_uart_gps_uart_handle);
}
