
/**
 * @file        main.c
 * @brief     Magpie example code for driving the TPS22994
 * @details     This example uses the I2C Master to talk to the TPS22994. It enables/disables channel1 and channel 2 od the load switch
 * to control the mics.
 */

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "mxc_device.h"
#include "mxc_delay.h"
#include "nvic_table.h"
#include "i2c.h"
#include "board.h"
#include "TPS22994.h"

/***** Definitions *****/
// #define I2C_MASTER MXC_I2C0_BUS0 // SCL P0_6; SDA P0_7
// #define I2C_FREQ MXC_I2C_STD_MODE
#define DELAY_uSec (1000000)



int main()
{
    printf("\n******** I2C TPS22994 load switch control *********\n");
    printf("\nThis example initiates the TPS22994 and provides an option to turn ON and OFF different channels of the device");
    printf("\nUses I2C0 (SCL - P0.6, SDA - P0.7).");
    MXC_Delay(DELAY_uSec);
    TPS22994_i2c_test();
    TPS22994_start(TPS22994_I2C_ADDR,TPS22994_REG_CONTROL, TPS22994_REG_CONTROL_DATA, 1);
    while (1)
    {
        TPS22994_Channel_On(TPS22994_I2C_ADDR, 1);
        MXC_Delay(DELAY_uSec);
        TPS22994_Channel_Off(TPS22994_I2C_ADDR, 1);
        MXC_Delay(DELAY_uSec);
        TPS22994_Channel_On(TPS22994_I2C_ADDR, 2);
        MXC_Delay(DELAY_uSec);
        TPS22994_Channel_Off(TPS22994_I2C_ADDR, 2);

    }

    return 0;
}
