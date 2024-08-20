#include "afe_gain_ctl.h"
#include "i2c.h"
#include <stddef.h> // for NULL

// this address assumes pins 23 and 28 are tied high
#define MAX14662_7_BIT_ADDR (0x4Fu)

#define MAX14662_TX_BUFF_LEN (2u)
static unsigned char tx_buff[MAX14662_TX_BUFF_LEN];
#define MAX14662_DUMMY_REGISTER (0x00u)

#define MAX14662_RX_BUFF_LEN (1u)
static unsigned char rx_buff[MAX14662_RX_BUFF_LEN];

AFE_Gain_Ctl_Error_t afe_gain_ctl_set_gain(AFE_Gain_Setting_t gain)
{
    // the format for writing is [addr, dummy, data]
    tx_buff[0] = MAX14662_DUMMY_REGISTER;
    tx_buff[1] = gain;

    mxc_i2c_req_t req;
    req.i2c = MXC_I2C0_BUS0;
    req.addr = MAX14662_7_BIT_ADDR;
    req.tx_buf = tx_buff;
    req.tx_len = MAX14662_TX_BUFF_LEN;
    req.rx_buf = NULL;
    req.rx_len = 0;
    req.restart = 0;
    req.callback = NULL;

    const int res = MXC_I2C_MasterTransaction(&req);

    return res == 0 ? AFE_GAIN_CTL_ERR_OK : AFE_GAIN_CTL_ERR_I2C_ERROR;
}

AFE_Gain_Setting_t afe_gain_ctl_get_gain()
{
    mxc_i2c_req_t req;
    req.i2c = MXC_I2C0_BUS0;
    req.addr = MAX14662_7_BIT_ADDR;
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
    // TODO: this will need to change when we decide on the 8 actual gain settings
    case AFE_GAIN_SETTING_15dB:
    case AFE_GAIN_SETTING_26dB:
    case AFE_GAIN_SETTING_32dB:
        return gain;
    default:
        return AFE_GAIN_SETTING_UNDEFINED;
    }
}
