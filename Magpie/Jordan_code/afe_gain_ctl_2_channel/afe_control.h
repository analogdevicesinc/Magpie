/**
 * @file      afe_gain_ctl.h
 * @brief     A software module for controlling the Analog Front End (AFE) is represented here.
 * @details   The gain is set by switching resistors in a differential opamp circuit. Eight discrete gain settings are
 * available. AFE channels are enabled and disabled via load switches controlled by GPIO pins.
 *
 * This module requires:
 * - Exclusive use of P0.11 and P0.12
 * - Shared use of I2C0 using 7-bit addresses 0x4E and 0x4F
 */

#ifndef AFE_GAIN_CTL_H_
#define AFE_GAIN_CTL_H_

/* Includes ----------------------------------------------------------------------------------------------------------*/

#include "i2c.h"

/* Public enumerations -----------------------------------------------------------------------------------------------*/

/**
 * @brief enumerated AFE gain control channels are represented here.
 */
typedef enum
{
    AFE_CONTROL_CHANNEL_0 = 0x4Fu, // the values here represent the gain MUX I2C addresses
    AFE_CONTROL_CHANNEL_1 = 0x4Eu,
} AFE_Control_Channel_t;

/**
 * @brief  gain settings are represented here.
 *
 * The values are based on the PCB routing, and should not be changed unless there is a new PCB revision.
 */
typedef enum
{
    AFE_CONTROL_GAIN_5dB = (1u << 7u),
    AFE_CONTROL_GAIN_10dB = (1u << 6u),
    AFE_CONTROL_GAIN_15dB = (1u << 5u),
    AFE_CONTROL_GAIN_20dB = (1u << 4u),
    AFE_CONTROL_GAIN_25dB = (1u << 3u),
    AFE_CONTROL_GAIN_30dB = (1u << 2u),
    AFE_CONTROL_GAIN_35dB = (1u << 1u),
    AFE_CONTROL_GAIN_40dB = (1u << 0u),
    AFE_CONTROL_GAIN_UNDEFINED,
} AFE_Control_Gain_t;

/**
 * @brief control errors are represented here.
 */
typedef enum
{
    AFE_CONTROL_ERROR_ALL_OK = 0,
    AFE_CONTROL_ERROR_I2C_ERROR,
    AFE_CONTROL_ERROR_CHANNEL_NOT_ENABLED_ERROR,
} AFE_Control_Error_t;

/* Public function declarations --------------------------------------------------------------------------------------*/

/**
 * @brief `afe_control_init(hi2c)` initializes the AFE gain control and sets the internal I2C handle to `hi2c`.
 *
 * @pre `hi2c` is configured as an I2C master and has pullup resistors to 1.8V.
 *
 * @param hi2c the I2C handle to use for all AFE gain control I2C communication.
 *
 * @post the AFE gain control is initialized and ready to use.
 */
void afe_control_init(mxc_i2c_regs_t *hi2c);

/**
 * @brief `afe_control_enable(c)` enables AFE channel `c`, powering it on.
 *
 * @pre `afe_gain_ctl_init(hi2c)` has been called with all its preconditions met.
 *
 * @param channel the enumerated AFE channel to enable.
 *
 * @post the given channel is powered on.
 */
void afe_control_enable(AFE_Control_Channel_t channel);

/**
 * @brief `afe_control_disable(c)` disables AFE channel `c`, powering it off.
 *
 * @pre `afe_gain_ctl_init(hi2c)` has been called with all its preconditions met.
 *
 * @param channel the enumerated AFE channel to disable.
 *
 * @post the given channel is powered off.
 */
void afe_control_disable(AFE_Control_Channel_t channel);

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
AFE_Control_Error_t afe_control_set_gain(AFE_Control_Channel_t channel, AFE_Control_Gain_t gain);

/**
 * @brief `afe_gain_ctl_get_gain(c)` is the current AFE gain setting of AFE channel `c`.
 *
 * @param channel the enumerated channel to read the gain of.
 *
 * @pre `afe_gain_ctl_init(hi2c)` has been called with all its preconditions met and channel `c` is enabled.
 *
 * @return the current gain setting, or `AFE_GAIN_SETTING_UNDEFINED` if there was an error reading the gain.
 */
AFE_Control_Gain_t afe_control_get_gain(AFE_Control_Channel_t channel);

#endif /* AFE_GAIN_CTL_H_ */
