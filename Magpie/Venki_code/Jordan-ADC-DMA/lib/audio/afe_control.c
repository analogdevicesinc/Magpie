/* Private includes --------------------------------------------------------------------------------------------------*/

#include <stddef.h> // for NULL
#include <stdbool.h>

#include "mxc_device.h"

#include "afe_control.h"
#include "board.h"
#include "bsp_i2c.h"
#include "bsp_pins.h"

/* Private defines ---------------------------------------------------------------------------------------------------*/

#define MAX14662_CH0_7_BIT_I2C_ADDRESS (0x4Fu)
#define MAX14662_CH1_7_BIT_I2C_ADDRESS (0x4Eu)

#define MAX14662_TX_BUFF_LEN (2u)
#define MAX14662_RX_BUFF_LEN (1u)

#define MAX14662_DUMMY_REGISTER (0x00u)

#define TPS22994_7_BIT_I2C_ADDRESS (0x71u)

#define TPS22994_CTL_REGISTER_ADDRESS (0x05u)

// mask to select I2C control for channels 0 and 1 instead of GPIO control
#define TPS22994_I2C_CTL_MASK (0x30)

/* Private variables -------------------------------------------------------------------------------------------------*/

// buffers for I2C transactions
static unsigned char tx_buff[MAX14662_TX_BUFF_LEN];
static unsigned char rx_buff[MAX14662_RX_BUFF_LEN];

static bool channel_0_is_enabled = false;
static bool channel_1_is_enabled = false;

/* Private function declarations -------------------------------------------------------------------------------------*/

/**
 * @brief Configures the TPS22994 to control the AFE channels over I2C.
 *
 * @pre the I2C bus on the 3.3V domain is configured as an I2C master and has pullup resistors to 3.3V.
 *
 * @post the TPS22994 is configured to control the AFE channels over I2C instead of via GPIO pins.
 *
 * @return error code: `AFE_GAIN_CTL_ERR_ALL_OK` if all I2C transactions were successful, else an enumerated error.
 */
static AFE_Control_Error_t tps22994_configure_ch0_and_ch1_for_i2c_control();

/**
 * @brief Sets the channels to be enabled on the TPS22994.
 *
 * @pre `tps22994_configure_ch0_and_ch1_for_i2c_control()` has been called with all its preconditions met.
 *
 * @param ch0 whether to enable channel 0
 *
 * @param ch1 whether to enable channel 1
 *
 * @post channels 0 and/or 1 are enabled or disabled according to the inputs.
 *
 * @return error code: `AFE_GAIN_CTL_ERR_ALL_OK` if all I2C transactions were successful, else an enumerated error.
 */
static AFE_Control_Error_t tps22994_set_channels(bool ch0, bool ch1);

/* Public function definitions ---------------------------------------------------------------------------------------*/

AFE_Control_Error_t afe_control_init()
{
    MXC_GPIO_Config(&bsp_pins_afe_ch0_en_cfg);
    MXC_GPIO_Config(&bsp_pins_afe_ch1_en_cfg);

    if (tps22994_configure_ch0_and_ch1_for_i2c_control() != AFE_CONTROL_ERROR_ALL_OK)
    {
        return AFE_CONTROL_ERROR_I2C_ERROR;
    }

    afe_control_disable(AFE_CONTROL_CHANNEL_0);
    afe_control_disable(AFE_CONTROL_CHANNEL_1);

    return AFE_CONTROL_ERROR_ALL_OK;
}

AFE_Control_Error_t afe_control_enable(AFE_Control_Channel_t channel)
{
    // first power on the opamps and common mode ref via load switches controlled by GPIO pins
    if (channel == AFE_CONTROL_CHANNEL_0)
    {
        channel_0_is_enabled = true;
        gpio_write_pin(&bsp_pins_afe_ch0_en_cfg, true);
    }
    else if (channel == AFE_CONTROL_CHANNEL_1)
    {
        channel_1_is_enabled = true;
        gpio_write_pin(&bsp_pins_afe_ch1_en_cfg, true);
    }

    // then power on the MEMs microphones via the I2C controlled load switches
    return tps22994_set_channels(channel_0_is_enabled, channel_1_is_enabled);
}

AFE_Control_Error_t afe_control_disable(AFE_Control_Channel_t channel)
{
    if (channel == AFE_CONTROL_CHANNEL_0)
    {
        channel_0_is_enabled = false;
    }
    else if (channel == AFE_CONTROL_CHANNEL_1)
    {
        channel_1_is_enabled = false;
    }

    // first power off the MEMs microphones via the I2C controlled load switches
    if (tps22994_set_channels(channel_0_is_enabled, channel_1_is_enabled) != 0)
    {
        return AFE_CONTROL_ERROR_I2C_ERROR;
    }

    // then power off the opamps and common mode ref via load switches controlled by GPIO pins
    if (channel == AFE_CONTROL_CHANNEL_0)
    {
        gpio_write_pin(&bsp_pins_afe_ch0_en_cfg, false);
    }
    else if (channel == AFE_CONTROL_CHANNEL_1)
    {
        gpio_write_pin(&bsp_pins_afe_ch1_en_cfg, false);
    }

    return AFE_CONTROL_ERROR_ALL_OK;
}

bool afe_control_channel_is_enabled(AFE_Control_Channel_t channel)
{
    if (channel == AFE_CONTROL_CHANNEL_0)
    {
        return channel_0_is_enabled;
    }
    else if (channel == AFE_CONTROL_CHANNEL_1)
    {
        return channel_1_is_enabled;
    }

    return false;
}

AFE_Control_Error_t afe_control_set_gain(AFE_Control_Channel_t channel, AFE_Control_Gain_t gain)
{
    if (!afe_control_channel_is_enabled(channel))
    {
        return AFE_CONTROL_ERROR_CHANNEL_NOT_ENABLED_ERROR;
    }

    // the format for writing is [addr, dummy, data]
    tx_buff[0] = MAX14662_DUMMY_REGISTER;
    tx_buff[1] = gain;

    const uint8_t address = channel == AFE_CONTROL_CHANNEL_0 ? MAX14662_CH0_7_BIT_I2C_ADDRESS : MAX14662_CH1_7_BIT_I2C_ADDRESS;

    mxc_i2c_req_t req = {
        .i2c = bsp_pins_1v8_i2c_handle,
        .addr = address,
        .tx_buf = tx_buff,
        .tx_len = MAX14662_TX_BUFF_LEN,
        .rx_buf = NULL,
        .rx_len = 0,
        .restart = 0,
        .callback = NULL,
    };

    const int res = MXC_I2C_MasterTransaction(&req);

    return res == 0 ? AFE_CONTROL_ERROR_ALL_OK : AFE_CONTROL_ERROR_I2C_ERROR;
}

AFE_Control_Gain_t afe_control_get_gain(AFE_Control_Channel_t channel)
{
    if (!afe_control_channel_is_enabled(channel))
    {
        return AFE_CONTROL_GAIN_UNDEFINED;
    }

    const uint8_t address = channel == AFE_CONTROL_CHANNEL_0 ? MAX14662_CH0_7_BIT_I2C_ADDRESS : MAX14662_CH1_7_BIT_I2C_ADDRESS;

    mxc_i2c_req_t req = {
        .i2c = bsp_pins_1v8_i2c_handle,
        .addr = address,
        .tx_buf = NULL,
        .tx_len = 0,
        .rx_buf = rx_buff,
        .rx_len = MAX14662_RX_BUFF_LEN,
        .restart = 0,
        .callback = NULL,
    };

    if (MXC_I2C_MasterTransaction(&req) != 0)
    {
        return AFE_CONTROL_GAIN_UNDEFINED;
    }

    const AFE_Control_Gain_t gain = rx_buff[0];

    switch (gain)
    {
    case AFE_CONTROL_GAIN_5dB:
    case AFE_CONTROL_GAIN_10dB:
    case AFE_CONTROL_GAIN_15dB:
    case AFE_CONTROL_GAIN_20dB:
    case AFE_CONTROL_GAIN_25dB:
    case AFE_CONTROL_GAIN_30dB:
    case AFE_CONTROL_GAIN_35dB:
    case AFE_CONTROL_GAIN_40dB:
        return gain;
    default:
        return AFE_CONTROL_GAIN_UNDEFINED;
    }
}

/* Private function definitions --------------------------------------------------------------------------------------*/

AFE_Control_Error_t tps22994_configure_ch0_and_ch1_for_i2c_control()
{
    tx_buff[0] = TPS22994_CTL_REGISTER_ADDRESS;
    tx_buff[1] = TPS22994_I2C_CTL_MASK;

    mxc_i2c_req_t req = {
        .i2c = bsp_pins_3v3_i2c_handle,
        .addr = TPS22994_7_BIT_I2C_ADDRESS,
        .tx_buf = tx_buff,
        .tx_len = 2,
        .rx_buf = NULL,
        .rx_len = 0,
        .restart = 0,
        .callback = NULL,
    };

    if (MXC_I2C_MasterTransaction(&req) != 0)
    {
        return AFE_CONTROL_ERROR_I2C_ERROR;
    }

    return AFE_CONTROL_ERROR_ALL_OK;
}

AFE_Control_Error_t tps22994_set_channels(bool ch0, bool ch1)
{
    // turn on any enabled channels
    tx_buff[0] = TPS22994_CTL_REGISTER_ADDRESS;
    tx_buff[1] = TPS22994_I2C_CTL_MASK | (channel_0_is_enabled << 1) | (channel_1_is_enabled);

    mxc_i2c_req_t req = {
        .i2c = bsp_pins_3v3_i2c_handle,
        .addr = TPS22994_7_BIT_I2C_ADDRESS,
        .tx_buf = tx_buff,
        .tx_len = 2,
        .rx_buf = NULL,
        .rx_len = 0,
        .restart = 0,
        .callback = NULL,
    };

    if (MXC_I2C_MasterTransaction(&req) != 0)
    {
        return AFE_CONTROL_ERROR_I2C_ERROR;
    }

    return AFE_CONTROL_ERROR_ALL_OK;
}
