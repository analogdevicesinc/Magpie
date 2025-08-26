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

// mask to select I2C control for channels 1,2,3,4 instead of GPIO control
#define TPS22994_I2C_CTL_MASK (0xF0)  // 1111 0000  sets to I2C control for all 4 channels

//      Bit :       7       |        6       |       5       |       4      |     3    |     2    |     1    |    0
//  Function:  GPIO/I2C CH4 |  GPIO/I2C CH3  | GPIO/I2C CH2  | GPIO/I2C CH1 | CH4 Enb  |  CH3 Enb |  CH2 Enb |  CH1 Enb
//   Default:      0 (GPIO) |     0 (GPIO)   |   0 (GPIO)    |   0 (GPIO)   |  0 (off) |  0 (off) |  0 (off) |  0 (off)        
//

/* Private variables -------------------------------------------------------------------------------------------------*/

// buffers for I2C transactions
static unsigned char tx_buff[MAX14662_TX_BUFF_LEN];
static unsigned char rx_buff[MAX14662_RX_BUFF_LEN];

static bool channel_0_is_enabled = false;
static bool channel_1_is_enabled = false;
static bool preamp_0_is_enabled = false;
static bool preamp_1_is_enabled = false;

/* Private function declarations -------------------------------------------------------------------------------------*/

/**
 * @brief `afe_control_gain_enum_to_bit_pos(ge)` is the bit position for a MAX14662 analog switch which results in
 * the given gain in dB.
 *
 * The bit positions are a function of the analog switches chosen and the PCB layout. The bit position represents a
 * single analog switch in the closed position in a MAX14662 analog switch. Refer to the schematic for clarification.
 *
 * This and `afe_control_bit_pos_to_gain_enum(bp)` are inverse functions.
 *
 * @param gain_enum the enumerated gain to convert to a bit-position.
 *
 * @retval the bit position for the analog switches U4 & U6 that results in the given gain.
 */
uint8_t afe_control_gain_enum_to_bit_pos(Audio_Gain_t gain_enum);

/**
 * @brief `afe_control_bit_pos_to_gain_enum(bp)` is the Audio_Gain_t enumeration resulting from the analog switch
 * bit position `bp`.
 *
 * The bit positions are a function of the analog switches chosen and the PCB layout. The bit position represents a
 * single analog switch in the closed position in a MAX14662 analog switch. Refer to the schematic for clarification.
 *
 * This and `afe_control_gain_enum_to_bit_pos(ge)` are inverse functions.
 *
 * @param bit_pos the bit position to convert to a gain enumeration
 *
 * @retval the gain enumeration which maps to the bit position `bit_pos`.
 */
Audio_Gain_t afe_control_bit_pos_to_gain_enum(uint8_t bit_pos);

/**
 * @brief Configures the TPS22994 to control the AFE channels over I2C.
 *
 * @pre the I2C bus on the 3.3V domain is configured as an I2C master and has pullup resistors to 3.3V.
 *
 * @post the TPS22994 is configured to control the AFE channels over I2C instead of via GPIO pins.
 *
 * @retval `E_NO_ERROR` if successful, else a negative error code.
 */
static int tps22994_configure_ch0_and_ch1_for_i2c_control(void);

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
 * @retval `E_NO_ERROR` if successful, else a negative error code.
 */
static int tps22994_set_channels(bool ch0, bool ch1, bool preamp0, bool preamp1);

/* Public function definitions ---------------------------------------------------------------------------------------*/

int afe_control_init()
{
    int res = E_NO_ERROR;

    MXC_GPIO_Config(&bsp_pins_afe_ch0_en_cfg);
    MXC_GPIO_Config(&bsp_pins_afe_ch1_en_cfg);

    if ((res = tps22994_configure_ch0_and_ch1_for_i2c_control()) != E_NO_ERROR)
    {
        return res;
    }

    if ((res = afe_control_disable(AUDIO_CHANNEL_0)) != E_NO_ERROR)
    {
        return res;
    }
    return afe_control_disable(AUDIO_CHANNEL_1);
}

int afe_control_enable(Audio_Channel_t channel)
{
    // first power on the opamps and common mode ref via load switches controlled by GPIO pins
    if (channel == AUDIO_CHANNEL_0)
    {
        channel_0_is_enabled = true;
        preamp_0_is_enabled = true;
        gpio_write_pin(&bsp_pins_afe_ch0_en_cfg, true);
    }
    else if (channel == AUDIO_CHANNEL_1)
    {
        channel_1_is_enabled = true;
        preamp_1_is_enabled = true;
        gpio_write_pin(&bsp_pins_afe_ch1_en_cfg, true);
    }
    else
    {
        return E_BAD_PARAM;
    }

    // then power on the MEMs microphones and preamp via the I2C controlled load switches
    return tps22994_set_channels(channel_0_is_enabled, channel_1_is_enabled, preamp_0_is_enabled, preamp_1_is_enabled);
}

int afe_control_disable(Audio_Channel_t channel)
{
    int res = E_NO_ERROR;

    if (channel == AUDIO_CHANNEL_0)
    {
        channel_0_is_enabled = false;
        preamp_0_is_enabled = false;
    }
    else if (channel == AUDIO_CHANNEL_1)
    {
        channel_1_is_enabled = false;
        preamp_1_is_enabled = false;
    }
    else
    {
        return E_BAD_PARAM;
    }

    // first power off the MEMs microphones via the I2C controlled load switches
    if ((res = tps22994_set_channels(channel_0_is_enabled, channel_1_is_enabled, preamp_0_is_enabled, preamp_1_is_enabled)) != E_NO_ERROR)
    {
        return res;
    }

    // then power off the opamps and common mode ref via load switches controlled by GPIO pins
    if (channel == AUDIO_CHANNEL_0)
    {
        gpio_write_pin(&bsp_pins_afe_ch0_en_cfg, false);
    }
    else if (channel == AUDIO_CHANNEL_1)
    {
        gpio_write_pin(&bsp_pins_afe_ch1_en_cfg, false);
    }

    return E_NO_ERROR;
}

bool afe_control_channel_is_enabled(Audio_Channel_t channel)
{
    if (channel == AUDIO_CHANNEL_0)
    {
        return channel_0_is_enabled;
    }
    else if (channel == AUDIO_CHANNEL_1)
    {
        return channel_1_is_enabled;
    }

    return false;
}

int afe_control_set_gain(Audio_Channel_t channel, Audio_Gain_t gain)
{
    if (!afe_control_channel_is_enabled(channel))
    {
        return E_UNINITIALIZED;
    }

    // the format for writing is [addr, dummy, data]
    tx_buff[0] = MAX14662_DUMMY_REGISTER;
    tx_buff[1] = afe_control_gain_enum_to_bit_pos(gain);

    const uint8_t address = channel == AUDIO_CHANNEL_0 ? MAX14662_CH0_7_BIT_I2C_ADDRESS : MAX14662_CH1_7_BIT_I2C_ADDRESS;

    mxc_i2c_req_t req = {
        .i2c = bsp_i2c_1v8_i2c_handle,
        .addr = address,
        .tx_buf = tx_buff,
        .tx_len = MAX14662_TX_BUFF_LEN,
        .rx_buf = NULL,
        .rx_len = 0,
        .restart = 0,
        .callback = NULL,
    };

    return MXC_I2C_MasterTransaction(&req);
}

Audio_Gain_t afe_control_get_gain(Audio_Channel_t channel)
{
    if (!afe_control_channel_is_enabled(channel))
    {
        return AUDIO_GAIN_UNDEFINED;
    }

    const uint8_t address = channel == AUDIO_CHANNEL_0 ? MAX14662_CH0_7_BIT_I2C_ADDRESS : MAX14662_CH1_7_BIT_I2C_ADDRESS;

    mxc_i2c_req_t req = {
        .i2c = bsp_i2c_1v8_i2c_handle,
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
        return AUDIO_GAIN_UNDEFINED;
    }

    const uint8_t bit_pos = rx_buff[0];

    return afe_control_bit_pos_to_gain_enum(bit_pos);
}

/* Private function definitions --------------------------------------------------------------------------------------*/

uint8_t afe_control_gain_enum_to_bit_pos(Audio_Gain_t gain_enum)
{
    // refer to the schematic to understand the bit positions, these are based on the PCB layout
    switch (gain_enum)
    {
    case AUDIO_GAIN_5dB:
        return (1u << 7u);
    case AUDIO_GAIN_10dB:
        return (1u << 6u);
    case AUDIO_GAIN_15dB:
        return (1u << 5u);
    case AUDIO_GAIN_20dB:
        return (1u << 4u);
    case AUDIO_GAIN_25dB:
        return (1u << 3u);
    case AUDIO_GAIN_30dB:
        return (1u << 2u);
    case AUDIO_GAIN_35dB:
        return (1u << 1u);
    case AUDIO_GAIN_40dB:
        return (1u << 0u);
    case AUDIO_GAIN_UNDEFINED: // fall through for undefined case or any other invalid value
    default:
        return 0;
    }
}

Audio_Gain_t afe_control_bit_pos_to_gain_enum(uint8_t bit_pos)
{
    // refer to the schematic to understand the bit positions, these are based on the PCB layout
    switch (bit_pos)
    {
    case (1u << 7u):
        return AUDIO_GAIN_5dB;
    case (1u << 6u):
        return AUDIO_GAIN_10dB;
    case (1u << 5u):
        return AUDIO_GAIN_15dB;
    case (1u << 4u):
        return AUDIO_GAIN_20dB;
    case (1u << 3u):
        return AUDIO_GAIN_25dB;
    case (1u << 2u):
        return AUDIO_GAIN_30dB;
    case (1u << 1u):
        return AUDIO_GAIN_35dB;
    case (1u << 0u):
        return AUDIO_GAIN_40dB;
    default:
        return AUDIO_GAIN_UNDEFINED;
    }
}

int tps22994_configure_ch0_and_ch1_for_i2c_control()
{
    tx_buff[0] = TPS22994_CTL_REGISTER_ADDRESS;
    tx_buff[1] = TPS22994_I2C_CTL_MASK;

    mxc_i2c_req_t req = {
        .i2c = bsp_i2c_3v3_i2c_handle,
        .addr = TPS22994_7_BIT_I2C_ADDRESS,
        .tx_buf = tx_buff,
        .tx_len = 2,
        .rx_buf = NULL,
        .rx_len = 0,
        .restart = 0,
        .callback = NULL,
    };

    return MXC_I2C_MasterTransaction(&req);
}

int tps22994_set_channels(bool ch0, bool ch1, bool preamp0, bool preamp1)
{
    // turn on any enabled channels
    tx_buff[0] = TPS22994_CTL_REGISTER_ADDRESS;
    tx_buff[1] = TPS22994_I2C_CTL_MASK | (uint8_t)ch0 | 
                                        ((uint8_t)ch1 << 1)  |
                                        ((uint8_t)preamp0 << 2)  | 
                                        ((uint8_t)preamp1 << 3) ;

    mxc_i2c_req_t req = {
        .i2c = bsp_i2c_3v3_i2c_handle,
        .addr = TPS22994_7_BIT_I2C_ADDRESS,
        .tx_buf = tx_buff,
        .tx_len = 2,
        .rx_buf = NULL,
        .rx_len = 0,
        .restart = 0,
        .callback = NULL,
    };

    return MXC_I2C_MasterTransaction(&req);
}
