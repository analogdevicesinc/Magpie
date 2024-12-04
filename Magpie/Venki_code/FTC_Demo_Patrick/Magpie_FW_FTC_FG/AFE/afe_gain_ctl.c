#include "afe_gain_ctl.h"
#include "i2c.h"
#include <stddef.h> // for NULL

#define MAX14662_CH0_7_BIT_I2C_ADDRESS (0x4Fu)
#define MAX14662_CH1_7_BIT_I2C_ADDRESS (0x4Eu)

//This one is on the I2C1 line
#define TPS22994_7_BIT_I2C_ADDRESS (0x71u)
#define TPS22994_CTL_REGISTER_ADDRESS (0x05u)
// mask to select I2C control for channels 0 and 1 instead of GPIO control
#define TPS22994_I2C_CTL_MASK (0x30)

#define MAX14662_TX_BUFF_LEN (2u)
static unsigned char tx_buff[MAX14662_TX_BUFF_LEN];
#define MAX14662_DUMMY_REGISTER (0x00u)

#define MAX14662_RX_BUFF_LEN (1u)
static unsigned char rx_buff[MAX14662_RX_BUFF_LEN];

// buffers for I2C transactions
static unsigned char tx_buff[MAX14662_TX_BUFF_LEN];
static unsigned char rx_buff[MAX14662_RX_BUFF_LEN];

static bool is_channel_0_enabled = false;
static bool is_channel_1_enabled = false;

// the I2C handle to use
mxc_i2c_regs_t *max32666_MAX14662_hi2c_;
mxc_i2c_regs_t *max32666_TPS22994_hi2c_;

//GPIO control pin for enabling op amp power on AFE
mxc_gpio_cfg_t gpio_out11;   //Op Amp power enable via load switch 0  (CH1_EN, P0.11)
mxc_gpio_cfg_t gpio_out12;   //Op Amp power enable via load switch 1  (CH2_EN, P0.12)

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
static AFE_Gain_Ctl_Error_t tps22994_configure_ch0_and_ch1_for_i2c_control();

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
static AFE_Gain_Ctl_Error_t tps22994_set_channels(bool ch0, bool ch1);



static AFE_Gain_Ctl_Error_t tps22994_configure_ch0_and_ch1_for_i2c_control()
{
    tx_buff[0] = TPS22994_CTL_REGISTER_ADDRESS;
    tx_buff[1] = TPS22994_I2C_CTL_MASK;

    mxc_i2c_req_t req = {
        .i2c = max32666_TPS22994_hi2c_,
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
        return AFE_GAIN_CTL_ERR_I2C_ERROR;
    }

    return AFE_GAIN_CTL_ERR_OK;
}

static AFE_Gain_Ctl_Error_t tps22994_set_channels(bool ch0, bool ch1)
{
    // turn on any enabled channels
    tx_buff[0] = TPS22994_CTL_REGISTER_ADDRESS;
    tx_buff[1] = TPS22994_I2C_CTL_MASK | (is_channel_0_enabled << 1) | (is_channel_1_enabled);

    mxc_i2c_req_t req = {
        .i2c = max32666_TPS22994_hi2c_,
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
        return AFE_GAIN_CTL_ERR_I2C_ERROR;
    }

    return AFE_GAIN_CTL_ERR_OK;
}

/****************************************************************************************************** */

AFE_Gain_Ctl_Error_t afe_gain_ctl_init(mxc_i2c_regs_t *hi2c, mxc_i2c_regs_t *hi2c1)
{
    max32666_MAX14662_hi2c_ = hi2c;
    max32666_TPS22994_hi2c_ = hi2c1;

    if (tps22994_configure_ch0_and_ch1_for_i2c_control() != AFE_GAIN_CTL_ERR_OK)
    {
        return AFE_GAIN_CTL_ERR_I2C_ERROR;
    }

    afe_control_disable(AFE_CONTROL_CHANNEL_0);
    afe_control_disable(AFE_CONTROL_CHANNEL_1);

    return AFE_GAIN_CTL_ERR_OK;
}

bool afe_control_channel_is_enabled(AFE_Control_Channel_t channel)
{
    if (channel == AFE_CONTROL_CHANNEL_0)
    {
        return is_channel_0_enabled;
    }
    else if (channel == AFE_CONTROL_CHANNEL_1)
    {
        return is_channel_1_enabled;
    }

    return false;
}

AFE_Gain_Ctl_Error_t afe_control_enable(AFE_Control_Channel_t channel, bool mic_only)
{
    if(!mic_only)
    {
        // first power on the opamps and common mode ref via load switches controlled by GPIO pins
        if (channel == AFE_CONTROL_CHANNEL_0)
        {
            is_channel_0_enabled = true;
            //Enable AFE CH0 OP AMP power. Set to HI  (P0.11) 
            gpio_out11.port = MXC_GPIO0;
            gpio_out11.mask= MXC_GPIO_PIN_11;
            gpio_out11.pad = MXC_GPIO_PAD_NONE;
            gpio_out11.func = MXC_GPIO_FUNC_OUT;
            gpio_out11.vssel = MXC_GPIO_VSSEL_VDDIOH;
            MXC_GPIO_Config(&gpio_out11);
            MXC_GPIO_OutSet(gpio_out11.port,gpio_out11.mask); // set HI  
        }
        else if (channel == AFE_CONTROL_CHANNEL_1)
        {
            is_channel_1_enabled = true;
            //Enable AFE CH0 OP AMP power. Set to HI  (P0.12) 
            gpio_out12.port = MXC_GPIO0;
            gpio_out12.mask= MXC_GPIO_PIN_12;
            gpio_out12.pad = MXC_GPIO_PAD_NONE;
            gpio_out12.func = MXC_GPIO_FUNC_OUT;
            gpio_out12.vssel = MXC_GPIO_VSSEL_VDDIOH;
            MXC_GPIO_Config(&gpio_out12);
            MXC_GPIO_OutSet(gpio_out12.port,gpio_out12.mask); // set HI  
        }
    }else{
        if (channel == AFE_CONTROL_CHANNEL_0)
        {
            is_channel_0_enabled = true;
            //Disable AFE CH0 OP AMP power. Set to LO  (P0.11) 
            gpio_out11.port = MXC_GPIO0;
            gpio_out11.mask= MXC_GPIO_PIN_11;
            gpio_out11.pad = MXC_GPIO_PAD_NONE;
            gpio_out11.func = MXC_GPIO_FUNC_OUT;
            gpio_out11.vssel = MXC_GPIO_VSSEL_VDDIOH;
            MXC_GPIO_Config(&gpio_out11);
            MXC_GPIO_OutClr(gpio_out11.port,gpio_out11.mask); // set Lo  
        }
        else if (channel == AFE_CONTROL_CHANNEL_1)
        {
            is_channel_1_enabled = true;
            //Disable AFE CH1 OP AMP power. Set to LO  (P0.12) 
            gpio_out12.port = MXC_GPIO0;
            gpio_out12.mask= MXC_GPIO_PIN_12;
            gpio_out12.pad = MXC_GPIO_PAD_NONE;
            gpio_out12.func = MXC_GPIO_FUNC_OUT;
            gpio_out12.vssel = MXC_GPIO_VSSEL_VDDIOH;
            MXC_GPIO_Config(&gpio_out12);
            MXC_GPIO_OutClr(gpio_out12.port,gpio_out12.mask); // set Lo  
        }
    }
    // then power on the MEMs microphones via the I2C controlled load switches
    return tps22994_set_channels(is_channel_0_enabled, is_channel_1_enabled);
}

AFE_Gain_Ctl_Error_t afe_control_disable(AFE_Control_Channel_t channel)
{
    if (channel == AFE_CONTROL_CHANNEL_0)
    {
        is_channel_0_enabled = false;
    }
    else if (channel == AFE_CONTROL_CHANNEL_1)
    {
        is_channel_1_enabled = false;
    }

    // first power off the MEMs microphones via the I2C controlled load switches
    if (tps22994_set_channels(is_channel_0_enabled, is_channel_1_enabled) != 0)
    {
        return AFE_GAIN_CTL_ERR_I2C_ERROR;
    }

    // then power off the opamps and common mode ref via load switches controlled by GPIO pins
    if (channel == AFE_CONTROL_CHANNEL_0)
    {
        //Disable AFE CH0 OP AMP power. Set to LO  (P0.11) 
        gpio_out11.port = MXC_GPIO0;
        gpio_out11.mask= MXC_GPIO_PIN_11;
        gpio_out11.pad = MXC_GPIO_PAD_NONE;
        gpio_out11.func = MXC_GPIO_FUNC_OUT;
        gpio_out11.vssel = MXC_GPIO_VSSEL_VDDIOH;
        MXC_GPIO_Config(&gpio_out11);
        MXC_GPIO_OutClr(gpio_out11.port,gpio_out11.mask); // set Lo  
    }
    else if (channel == AFE_CONTROL_CHANNEL_1)
    {
        //Disable AFE CH1 OP AMP power. Set to LO  (P0.12) 
        gpio_out12.port = MXC_GPIO0;
        gpio_out12.mask= MXC_GPIO_PIN_12;
        gpio_out12.pad = MXC_GPIO_PAD_NONE;
        gpio_out12.func = MXC_GPIO_FUNC_OUT;
        gpio_out12.vssel = MXC_GPIO_VSSEL_VDDIOH;
        MXC_GPIO_Config(&gpio_out12);
        MXC_GPIO_OutClr(gpio_out12.port,gpio_out12.mask); // set Lo  
    }

    return AFE_GAIN_CTL_ERR_OK;
}

AFE_Gain_Ctl_Error_t afe_gain_ctl_set_gain(AFE_Control_Channel_t channel, AFE_Gain_Setting_t gain)
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
        .i2c = max32666_MAX14662_hi2c_,
        .addr = address,
        .tx_buf = tx_buff,
        .tx_len = MAX14662_TX_BUFF_LEN,
        .rx_buf = NULL,
        .rx_len = 0,
        .restart = 0,
        .callback = NULL,
    };

    const int res = MXC_I2C_MasterTransaction(&req);

    return res == 0 ? AFE_GAIN_CTL_ERR_OK : AFE_GAIN_CTL_ERR_I2C_ERROR;
    
}

AFE_Gain_Setting_t afe_gain_ctl_get_gain(AFE_Control_Channel_t channel)
{
    if (!afe_control_channel_is_enabled(channel))
    {
        return AFE_GAIN_SETTING_UNDEFINED;
    }

const uint8_t address = channel == AFE_CONTROL_CHANNEL_0 ? MAX14662_CH0_7_BIT_I2C_ADDRESS : MAX14662_CH1_7_BIT_I2C_ADDRESS;

    mxc_i2c_req_t req;
    req.i2c = max32666_MAX14662_hi2c_;
    req.addr = address;
    req.tx_buf = NULL;
    req.tx_len = 0;
    req.rx_buf = rx_buff;
    req.rx_len = MAX14662_RX_BUFF_LEN;
    req.restart = 0;
    req.callback = NULL;

    if (MXC_I2C_MasterTransaction(&req) != 0)
    {
        return AFE_GAIN_SETTING_UNDEFINED;
    }

    const AFE_Gain_Setting_t gain = rx_buff[0];

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
        return AFE_GAIN_SETTING_UNDEFINED;
    }
}

