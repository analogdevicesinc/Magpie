
/**
 * @file      afe_gain_ctl.h
 * @brief     A software module for controlling the analog gain of the AFE via a MAX14662 MUX chip is represented here.
 * @details   The gain is set by switching resistors in the differential opamp circuit. Eight discrete gain settings are
 * available. This module requires shared use of I2C0 and uses 7-bit address 0x4F
 *
 * NOTE: the prototype boards available at the time of this writing only have three resistors fitted in the gain
 * control circuit.
 */

#ifndef AFE_GAIN_CTL_H_
#define AFE_GAIN_CTL_H_

#include <stdbool.h>
#include "i2c.h"
#include "max32665.h"

/* Public enumerations -----------------------------------------------------------------------------------------------*/

/**
 * @brief enumerated AFE gain control channels are represented here.
 */
typedef enum
{
    AFE_CONTROL_CHANNEL_0 = 0, // the values here represent the gain MUX I2C addresses
    AFE_CONTROL_CHANNEL_1 = 1,
} AFE_Control_Channel_t;

/**
 * @brief  gain settings are represented here.
 *
 * The values are based on the PCB routing, and should not be changed unless there is a new PCB revision.
 */
typedef enum
{   AFE_CONTROL_GAIN_0dB = 0x00,
    AFE_CONTROL_GAIN_5dB = (1u << 7u),   //128
    AFE_CONTROL_GAIN_10dB = (1u << 6u),  //64
    AFE_CONTROL_GAIN_15dB = (1u << 5u),  //32
    AFE_CONTROL_GAIN_20dB = (1u << 4u),  //16
    AFE_CONTROL_GAIN_25dB = (1u << 3u),  //8
    AFE_CONTROL_GAIN_30dB = (1u << 2u),  //4
    AFE_CONTROL_GAIN_35dB = (1u << 1u),  //2
    AFE_CONTROL_GAIN_40dB = (1u << 0u),  //1
    AFE_GAIN_SETTING_UNDEFINED,
} AFE_Gain_Setting_t;

/**
 * @brief control errors are represented here.
 */
typedef enum
{
    AFE_GAIN_CTL_ERR_OK = 0,
    AFE_GAIN_CTL_ERR_I2C_ERROR,
    AFE_CONTROL_ERROR_CHANNEL_NOT_ENABLED_ERROR,
} AFE_Gain_Ctl_Error_t;



/**
 * @brief `afe_gain_ctl_init(hi2c, hi2c1)` initialize gain control by passing I2c handles `hi2c` and `hi2c1`.
 *
 * @pre I2C from the master MCU has already been configured
 *
 * @param *hi2c handle for the I2C
 *
 */
AFE_Gain_Ctl_Error_t afe_gain_ctl_init(mxc_i2c_regs_t *hi2c, mxc_i2c_regs_t *hi2c1);

/**
 * @brief `afe_control_enable(c)` enables AFE channel `c`, powering it on.
 *
 * @pre `afe_gain_ctl_init(hi2c)` has been called with all its preconditions met.
 *
 * @param channel the enumerated AFE channel to enable.
 *
 * @post the given channel is powered on.
 */
AFE_Gain_Ctl_Error_t afe_control_enable(AFE_Control_Channel_t channel, bool mic_only);

/**
 * @brief `afe_control_disable(c)` disables AFE channel `c`, powering it off.
 *
 * @pre `afe_gain_ctl_init(hi2c)` has been called with all its preconditions met.
 *
 * @param channel the enumerated AFE channel to disable.
 *
 * @post the given channel is powered off.
 */
AFE_Gain_Ctl_Error_t afe_control_disable(AFE_Control_Channel_t channel);

/**
 * @brief `afe_control_channel_is_enabled(c)` is true iff AFE channel `c` is enabled.
 *
 * @param channel the channel to check
 *
 * @retval true if the given channel is enabled, else false
 */
bool afe_control_channel_is_enabled(AFE_Control_Channel_t channel);

/**
 * @brief `afe_control_set_gain(c, g)` sets the gain of the AFE channel `c` to enumerated gain setting `g`
 *
 * @param channel the enumerated channel to set the gain of.
 *
 * @param gain the enumerated gain setting to use.
 *
 * @pre `afe_gain_ctl_init(hi2c)` has been called with all its preconditions met and channel `c` is enabled.
 *
 * @post The gain of AFE channel `c` is changed to enumerated gain setting `g`.
 *
 * @return error code: `AFE_GAIN_CTL_ERR_ALL_OK` if all I2C transactions were successful, else an enumerated error.
 */
AFE_Gain_Ctl_Error_t afe_gain_ctl_set_gain(AFE_Control_Channel_t channel, AFE_Gain_Setting_t gain);

/**
 * @brief `afe_gain_ctl_get_gain(c)` is the current AFE gain setting of AFE channel `c`.
 *
 * @param channel the enumerated channel to read the gain of.
 *
 * @pre `afe_gain_ctl_init(hi2c)` has been called with all its preconditions met and channel `c` is enabled.
 *
 * @return the current gain setting, or `AFE_GAIN_SETTING_UNDEFINED` if there was an error reading the gain.
 */
AFE_Gain_Setting_t afe_gain_ctl_get_gain(AFE_Control_Channel_t channel);

#endif /* AFE_GAIN_CTL_H_ */
