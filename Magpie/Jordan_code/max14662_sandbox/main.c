
/**
 * @file        main.c
 * @brief     Magpie example code for driving the MAX14662 in charge of AFE gain
 * @details     This example uses the I2C Master to talk to the MAX14662. It sets the gain to a few enumerated AFE gain
 * values and reads back the value from the MAX14662. Gains are cycled through in a forever loop once per second.
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

#include "afe_gain_ctl.h"

/***** Definitions *****/
#define I2C_MASTER MXC_I2C0_BUS0 // SCL P0_6; SDA P0_7
#define I2C_FREQ MXC_I2C_STD_MODE

#define DELAY_uSec (1000000)

/**
 * Helper function to cycle through the enumerated AFE gain settings.
 *
 * Given a gain setting, yields the next gain setting to use in the demo.
 *
 * This is not intended as a useful function for the end application,
 * it's only for demo purposes.
 *
 * If the enumerated AFE gains are changed to reflect the actual available
 * gains defined by the resistors around the gain MUX, this must change as well.
 */
AFE_Gain_Setting_t next_gain(AFE_Gain_Setting_t g)
{
    switch (g)
    {
    case AFE_GAIN_SETTING_15dB:
        return AFE_GAIN_SETTING_26dB;
    case AFE_GAIN_SETTING_26dB:
        return AFE_GAIN_SETTING_32dB;
    case AFE_GAIN_SETTING_32dB:
    default:
        return AFE_GAIN_SETTING_15dB;
    }
}

int main()
{
    printf("\n******** I2C AFE GAIN MUX EXAMPLE *********\n");
    printf("\nThis example rotates through the various gain setting of the MAX14662 AFE gain MUX");
    printf("\nUses I2C0 (SCL - P0.6, SDA - P0.7).");

    MXC_Delay(DELAY_uSec);

    if (MXC_I2C_Init(I2C_MASTER, 1, 0) != E_NO_ERROR)
    {
        printf("-->I2C initialization FAILED\n");
        return -1;
    }
    if (MXC_I2C_SetFrequency(I2C_MASTER, I2C_FREQ) != I2C_FREQ)
    {
        printf("-->I2C frequency set FAILED\n");
        return -1;
    }
    printf("\n-->I2C Master Initialization Complete\n");

    MXC_Delay(DELAY_uSec);

    AFE_Gain_Setting_t intended_gain = AFE_GAIN_SETTING_15dB;

    while (1)
    {
        printf("\nSetting gain to:   0x%02x", intended_gain);
        // vars marked volatile because otherwise the compiler was optimizing them out during debugging
        volatile const int res = afe_gain_ctl_set_gain(intended_gain);
        if (res != AFE_GAIN_CTL_ERR_OK)
        {
            printf("\n*** Error writing gain to MAX14662 ***\n");
        }

        volatile const AFE_Gain_Setting_t readback_gain = afe_gain_ctl_get_gain();
        printf("\nReading gain back: 0x%02x\n", readback_gain);

        if (intended_gain != readback_gain)
        {
            printf("\n*** INTENDED GAIN AND GAIN READ BACK DO NOT MATCH ***\n");
        }

        intended_gain = next_gain(intended_gain);

        MXC_Delay(DELAY_uSec);
    }

    return 0;
}
