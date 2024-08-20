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

/**
 * @brief  gain settings are represented here.
 *
 * The values are based on the PCB routing, and should not be changed unless there is a new PCB revision.
 */
typedef enum
{
    AFE_GAIN_SETTING_15dB = (1u << 6u),
    AFE_GAIN_SETTING_26dB = (1u << 0u),
    AFE_GAIN_SETTING_32dB = (1u << 3u),
    AFE_GAIN_SETTING_UNDEFINED,
    // TODO: add up to 8 gain settings with the correct dB value, the values here are just placeholders
    // the enum here depends on the final board layout and resistors chosen
} AFE_Gain_Setting_t;

/**
 * @brief control errors are represented here.
 */
typedef enum
{
    AFE_GAIN_CTL_ERR_OK,
    AFE_GAIN_CTL_ERR_I2C_ERROR
} AFE_Gain_Ctl_Error_t;

/**
 * @brief `gain_ctl_set_gain(g)` sets the gain of the AFE to enumerated gain setting `g`
 *
 * @param gain the enumerated gain setting to use.
 *
 * @pre I2C0 is initialized and configured as master.
 *
 * @post The gain of the AFE is changed to enumerated gain setting `g`.
 *
 * @return error code: `AFE_GAIN_CTL_E_OK` if all I2C transactions were successful, else an enumerated error.
 */
AFE_Gain_Ctl_Error_t afe_gain_ctl_set_gain(AFE_Gain_Setting_t gain);

/**
 * @brief `gain_ctl_get_gain()` is the current AFE gain setting
 *
 * @pre I2C0 is initialized and configured as master
 *
 * @return the current gain setting: or `AFE_GAIN_SETTING_UNDEFINED` if there was an error reading the gain
 */
AFE_Gain_Setting_t afe_gain_ctl_get_gain();

#endif /* AFE_GAIN_CTL_H_ */
