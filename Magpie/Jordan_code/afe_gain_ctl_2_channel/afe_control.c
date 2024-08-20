/* Private includes --------------------------------------------------------------------------------------------------*/

#include <stddef.h> // for NULL

#include "mxc_device.h"

#include "afe_control.h"

/* Private defines ---------------------------------------------------------------------------------------------------*/

#define MAX14662_TX_BUFF_LEN (2u)
#define MAX14662_RX_BUFF_LEN (1u)

#define MAX14662_DUMMY_REGISTER (0x00u)

/* Private variables -------------------------------------------------------------------------------------------------*/

// the I2C handle to use
static mxc_i2c_regs_t *hi2c_;

// buffers for I2C transactions
static unsigned char tx_buff[MAX14662_TX_BUFF_LEN];
static unsigned char rx_buff[MAX14662_RX_BUFF_LEN];

const mxc_gpio_cfg_t afe_ch0_enable_pin = {
    .port = MXC_GPIO0,
    .mask = MXC_GPIO_PIN_11,
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_OUT,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

const mxc_gpio_cfg_t afe_ch1_enable_pin = {
    .port = MXC_GPIO0,
    .mask = MXC_GPIO_PIN_12,
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_OUT,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

/* Public function definitions ---------------------------------------------------------------------------------------*/

void afe_control_init(mxc_i2c_regs_t *hi2c)
{
    hi2c_ = hi2c;

    MXC_GPIO_Config(&afe_ch0_enable_pin);
    MXC_GPIO_Config(&afe_ch1_enable_pin);
}

void afe_control_enable(AFE_Control_Channel_t channel)
{
    switch (channel)
    {
    case AFE_CONTROL_CHANNEL_0:
        MXC_GPIO_OutSet(afe_ch0_enable_pin.port, afe_ch0_enable_pin.mask);
        break;
    case AFE_CONTROL_CHANNEL_1:
        MXC_GPIO_OutSet(afe_ch1_enable_pin.port, afe_ch1_enable_pin.mask);
        break;
    default:
        break;
    }
}

void afe_control_disable(AFE_Control_Channel_t channel)
{
    switch (channel)
    {
    case AFE_CONTROL_CHANNEL_0:
        MXC_GPIO_OutClr(afe_ch0_enable_pin.port, afe_ch0_enable_pin.mask);
        break;
    case AFE_CONTROL_CHANNEL_1:
        MXC_GPIO_OutClr(afe_ch1_enable_pin.port, afe_ch1_enable_pin.mask);
        break;
    default:
        break;
    }
}

bool afe_control_channel_is_enabled(AFE_Control_Channel_t channel)
{
    switch (channel)
    {
    case AFE_CONTROL_CHANNEL_0:
        return MXC_GPIO_InGet(afe_ch0_enable_pin.port, afe_ch0_enable_pin.mask);
    case AFE_CONTROL_CHANNEL_1:
        return MXC_GPIO_InGet(afe_ch1_enable_pin.port, afe_ch1_enable_pin.mask);
    default:
        return false;
    }
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

    mxc_i2c_req_t req = {
        .i2c = hi2c_,
        .addr = channel,
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

    mxc_i2c_req_t req = {
        .i2c = hi2c_,
        .addr = channel,
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
