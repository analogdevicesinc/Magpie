
/**
 * @file        main.c
 * @brief     Magpie example code for driving the MAX14662 in charge of AFE gain
 * @details     This example uses the I2C Master to talk to the MAX14662. It sets the gain to a few enumerated AFE gain
 * values and reads back the value from the MAX14662. Gains are cycled through in a forever loop once per second.
 *
 * TODO: update the main demo application to exercise both channels once the FTC spin boards arrive
 */

/* Includes ----------------------------------------------------------------------------------------------------------*/

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "board.h"
#include "i2c.h"
#include "mxc_device.h"
#include "mxc_delay.h"
#include "nvic_table.h"

#include "afe_control.h"

/* Defines -----------------------------------------------------------------------------------------------------------*/

// brief delay for terminal printing legibility
#define DELAY_uSec (1000000)

/* Helper function declarations --------------------------------------------------------------------------------------*/

/**
 * Helper function to cycle through the enumerated AFE gain settings. Given a gain setting, yields the next gain setting
 * to use in the demo.
 *
 * This is not intended as a useful function for the end application, it's only for demo purposes.
 *
 * If the enumerated AFE gains are changed to reflect the actual available gains defined by the resistors around the
 * gain MUX, this must change as well.
 */
AFE_Control_Gain_t next_gain(AFE_Control_Gain_t g);

/* Function definitions ----------------------------------------------------------------------------------------------*/

int main()
{
    printf("\n******** I2C AFE GAIN MUX EXAMPLE *********\n");
    printf("\nThis example rotates through the various gain setting of the MAX14662 AFE gain MUX");
    printf("\nUses I2C0 (SCL - P0.6, SDA - P0.7).");

    MXC_Delay(DELAY_uSec);

    if (MXC_I2C_Init(MXC_I2C0_BUS0, 1, 0) != E_NO_ERROR)
    {
        printf("-->I2C initialization FAILED\n");
        return -1;
    }
    if (MXC_I2C_SetFrequency(MXC_I2C0_BUS0, MXC_I2C_STD_MODE) != MXC_I2C_STD_MODE)
    {
        printf("-->I2C frequency set FAILED\n");
        return -1;
    }
    printf("\n-->I2C Master Initialization Complete\n");

    afe_control_init(MXC_I2C0_BUS0);

    MXC_Delay(DELAY_uSec);

    AFE_Control_Gain_t intended_gain = AFE_CONTROL_GAIN_5dB;

    while (1)
    {
        printf("\nSetting gain to:   0x%02x", intended_gain);

        const int res = afe_control_set_gain(AFE_CONTROL_CHANNEL_0, intended_gain);
        if (res != AFE_CONTROL_ERROR_ALL_OK)
        {
            printf("\n*** Error writing gain to MAX14662 ***\n");
        }

        volatile const AFE_Control_Gain_t readback_gain = afe_control_get_gain(AFE_CONTROL_CHANNEL_0);
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

AFE_Control_Gain_t next_gain(AFE_Control_Gain_t g)
{
    switch (g)
    {
    case AFE_CONTROL_GAIN_5dB:
        return AFE_CONTROL_GAIN_10dB;
    case AFE_CONTROL_GAIN_10dB:
        return AFE_CONTROL_GAIN_15dB;
    case AFE_CONTROL_GAIN_15dB:
        return AFE_CONTROL_GAIN_20dB;
    case AFE_CONTROL_GAIN_20dB:
        return AFE_CONTROL_GAIN_25dB;
    case AFE_CONTROL_GAIN_25dB:
        return AFE_CONTROL_GAIN_30dB;
    case AFE_CONTROL_GAIN_30dB:
        return AFE_CONTROL_GAIN_35dB;
    case AFE_CONTROL_GAIN_35dB:
        return AFE_CONTROL_GAIN_40dB;
    case AFE_CONTROL_GAIN_40dB:
    default:
        return AFE_CONTROL_GAIN_5dB;
    }
}