
#include "mxc_delay.h"
#include "bsp_status_led.h"

#include "data_converters.h"

int main(void)
{
    status_led_init();

    while (1)
    {
        status_led_toggle(STATUS_LED_COLOR_GREEN);
        MXC_Delay(500000);
    }
}
