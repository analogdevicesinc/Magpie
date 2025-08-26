/**
 * @file      bsp_spixf.h
 * @brief     This module is responsible for initializing and de-initializing the SPIXF interface used by the sytem.
 *
 * This module requires:
 * - Shared use of SPIXF
 * - Shared use of SPIXF pins P0.0, P0.1, P0.2, and P0.3
 */

#ifndef BSP_SPIXF_H__
#define BSP_SPIXF_H__

/* Public function declarations --------------------------------------------------------------------------------------*/

/**
 * @brief `bsp_spixf_init()` initializes and starts the SPIXF interface in 4-wire SPI mode.
 *
 * @post SPIXF is initialized and ready to use. The GPIO pins associated with the bus are configured for SPIXF operation.
 *
 * @retval Success/Fail, see MXC_Error_Codes for a list of return codes.
 */
int bsp_spixf_init(void);

/**
 * @brief `bsp_spixf_deinit()` deinitializes SPIXF and sets the associated pins to high-Z.
 *
 * @post SPIXF is deinitialized. The GPIO pins associated with the bus are hig-Z.
 *
 * @retval Success/Fail, see MXC_Error_Codes for a list of return codes.
 */
int bsp_spixf_deinit(void);

#endif
