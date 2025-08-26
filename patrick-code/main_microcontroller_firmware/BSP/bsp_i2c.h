/**
 * @file      bsp_i2c.h
 * @brief     This module is responsible for initializing and de-initializing the I2C busses used by the sytem.
 *
 * This module requires:
 * - Shared use of I2C0 and I2C1
 * - Shared use of I2C pins P0.6, P0.7, P0.14, and P0.15
 */

#ifndef BSP_I2C_H_INCLUDED__
#define BSP_I2C_H_INCLUDED__

/* Includes ----------------------------------------------------------------------------------------------------------*/

#include "i2c.h"

/* Public definitions ------------------------------------------------------------------------------------------------*/

#define BSP_I2C_1V8_BUS_SPEED (MXC_I2C_STD_MODE)

#define BSP_I2C_3V3_BUS_SPEED (MXC_I2C_STD_MODE)

/* Public variables --------------------------------------------------------------------------------------------------*/

/**
 * @brief pointer to the I2C handle in the 1.8V domain
 */
extern mxc_i2c_regs_t *bsp_i2c_1v8_i2c_handle;

/**
 * @brief pointer to the I2C handle in the 3.3V domain
 */
extern mxc_i2c_regs_t *bsp_i2c_3v3_i2c_handle;

/* Public function declarations --------------------------------------------------------------------------------------*/

/**
 * @brief `bsp_1v8_i2c_init()` initializes and starts I2C0 bus as an I2C master in the 1.8V domain.
 *
 * @pre the LDOs are powered on via bsp_power_on_LDOs().
 *
 * @post the I2C0 bus is initialized and ready to use. The GPIO pins associated with the bus are pulled up to 1.8V.
 *
 * @retval Success/Fail, see MXC_Error_Codes for a list of return codes.
 */
int bsp_1v8_i2c_init(void);

/**
 * @brief `bsp_1V8_i2c_deinit()` de-initializes the I2C0 bus and sets the associated pins to high-Z.
 *
 * @post the I2C0 bus is de-initialized. The GPIO pins associated with the bus are high-Z.
 *
 * @retval Success/Fail, see MXC_Error_Codes for a list of return codes.
 */
int bsp_1V8_i2c_deinit(void);

/**
 * @brief `bsp_3v3_i2c_init()` initializes and starts I2C1 bus as an I2C master in the 3.3V domain.
 *
 * @post the I2C1 bus is initialized and ready to use. The GPIO pins associated with the bus are pulled up to 3.3V.
 *
 * @retval Success/Fail, see MXC_Error_Codes for a list of return codes.
 */
int bsp_3v3_i2c_init(void);

/**
 * @brief `bsp_3v3_i2c_deinit()` de-initializes the I2C1 bus and sets the associated pins to high-Z.
 *
 * @post the I2C1 bus is de-initialized. The GPIO pins associated with the bus are high-Z.
 *
 * @retval Success/Fail, see MXC_Error_Codes for a list of return codes.
 */
int bsp_3v3_i2c_deinit(void);

#endif
