/**
 * @file      bsp_sdhc.h
 * @brief     This module is responsible for initializing and de-initializing the SDHC bus used by the sytem.
 *
 * This module requires:
 * - Shared use of SDHC
 * - Shared use of SDHC pins P1.0, P1.1, P1.2, P1.3, P1.4, and P1.5 (WP/P1.6 and CD/P1.7 are not used)
 */

#ifndef BSP_SDHC_H__
#define BSP_SDHC_H__

/* Public function declarations --------------------------------------------------------------------------------------*/

/**
 * @brief `bsp_sdhc_init()` initializes and starts SDHC bus.
 *
 * @post the SDHC interface is initialized and ready to use. The GPIO pins associated with the bus are configured.
 *
 * @retval Success/Fail, see MXC_Error_Codes for a list of return codes.
 */
int bsp_sdhc_init(void);

/**
 * @brief `bsp_sdhc_deinit()` de-initializes the SDHC bus and sets the associated pins to high-Z.
 *
 * @post the SDHC bus is de-initialized. The GPIO pins associated with the bus are high-Z.
 *
 * @retval Success/Fail, see MXC_Error_Codes for a list of return codes.
 */
int bsp_sdhc_deinit(void);

#endif
