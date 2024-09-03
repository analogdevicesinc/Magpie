/**
 * @file      TPS2294.h
 * @brief     A software module for the quad channel load switch TPS22994 is presented here.
 * @details   The HF and LF mic can be enabled using this I2C load switch. This module requires shared use of I2C0 and uses 7-bit address 0x71 (E2/E3>>1)
 *
 * NOTE: 
 */

#ifndef TPS22994_H_
#define TPS22994_H_

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include "mxc_device.h"
#include "mxc_sys.h"
#include "nvic_table.h"
#include "i2c.h"
#include "i2c_regs.h"
#include "mxc_delay.h"
#include "tmr.h"
#include "dev_i2c.h"




//Device tps22994 I2C Address
#define TPS22994_I2C_ADDR           0x71u  //(E2/E3>>1)
#define TPS22994_REG_CONTROL        0x05u  //The control register (address 05h) can be configured for GPIO or I2C enable on a per channel basis.
#define TPS22994_REG_CONTROL_DATA   0xF0u  //Writing F0h to 05h enables I2C control

/* Public enumerations -----------------------------------------------------------------------------------------------*/

/**
 * @brief TPS22994 errors are represented here
 */
typedef enum
{
    TPS22994_ERROR_ALL_OK,
    TPS22994_ERROR_LS_ERROR,

} TPS22994_Error_t;




/***** Function Prototypes *****/

/**
 * @brief   tps22994_write_reg. Generic function to read MAX17261 registers.
 * @param[out]  dev_addr. The 1-byte slave address of MAX17261.
 * @param[out]  reg_addr. The 1-byte address of the register on the I2C slave to start writing to.
 * @param[in]   reg_data. Array of uint8_t data to write to the I2C slave.
 * @param[out]  len. Number of uint16_t registers to write.
* @retval         TPS22994_ERROR_ALL_OK if operation is suceeded, else an error code
 ****************************************************************************/

TPS22994_Error_t TPS22994_write_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);


/**
 * @brief   tps_read_reg. Generic function to read max17261 registers.
 * @param[out]  dev_addr. The 1-byte slave address of energy harvester.
 * @param[out]  reg_addr. The 1-byte address of the register on the I2C slave to start reading from.
 * @param[in]   reg_data. Array of uint8_t data to read from the I2C slave.
 * @param[out]  len. Number of uint16_t registers to read.
 * @retval        TPS22994_ERROR_ALL_OK if operation is suceeded, else an error code
 ****************************************************************************/
TPS22994_Error_t TPS22994_read_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);

/**
 * @brief   TPS22994_write_verify_reg. Generic function to read max17261 registers.
 * @param[out]  dev_addr. The 1-byte slave address of energy harvester.
 * @param[out]  reg_addr. The 1-byte address of the register on the I2C slave to start reading from.
 * @param[in]  *reg_data. Array of uint8_t data to read from the I2C slave.
 * @param[out]  num_of_byts. Number of bytes to read.
 * @return      true on success, false on failure
 ****************************************************************************/
bool TPS22994_write_verify_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t num_of_bytes);


/**
 * @brief    TPS22994_i2c_test(). Function to check that TPS22994 is present and
 * responding on I2C bus.  This finction also initializes the I2C and then checks for MAX17261
 * @retval         TPS22994_ERROR_ALL_OK if operation is suceeded, else an error code
 ****************************************************************************/
TPS22994_Error_t TPS22994_i2c_test(void);

/**
 * @brief    TPS22994_start(). This function initializes the TPS22994 and we make it is I2C onctrolled instead of GPIO  
 * @retval   TPS22994_ERROR_ALL_OK if operation is suceeded, else an error code
 ****************************************************************************/
 TPS22994_Error_t TPS22994_start(uint8_t dev_addr, uint8_t reg_addr, uint8_t reg_data, uint16_t num_of_bytes);


 /**
 * @brief    TPS22994_Channel_Off. This function initializes the TPS22994 and we make it is I2C onctrolled instead of GPIO  
 * @param[in] Channel_num.  Number of the channel to be tunred OFF  
 * @retval   TPS22994_ERROR_ALL_OK if operation is suceeded, else an error code
 ****************************************************************************/
 TPS22994_Error_t TPS22994_Channel_Off(uint8_t dev_addr, uint8_t Channel_num);

 /**
 * @brief    TPS22994_Channel_On. This function turns ON specific channel of the device.
 * @param[in] Channel_num.  Number of the channel to be tunred ON  
 * @retval   TPS22994_ERROR_ALL_OK if operation is suceeded, else an error code
 ****************************************************************************/
 TPS22994_Error_t TPS22994_Channel_On(uint8_t dev_addr, uint8_t Channel_num);






#endif /* MAX17261_H_ */
