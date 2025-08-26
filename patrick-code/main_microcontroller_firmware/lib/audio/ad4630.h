/**
 * @file      ad4630.h
 * @brief     A software interface for configuring the AD4630 ADC is represented here.
 * @details   This module is repsonsible for initializing the ADC and starting/stopping conversions.
 *            Access to the audio data converted by the ADC is handled by the audio_dma module.
 */

#ifndef AD4630_H_
#define AD4630_H_

/* Public function declarations --------------------------------------------------------------------------------------*/

/**
 * @brief `ad4630_init()` initializes the AD4630 ADC
 *
 * @pre the LDOs are powered on via bsp_power_on_LDOs().
 *
 * @post the ADC is initialized and ready to use
 *
 * @retval `E_NO_ERROR` if successful, else a negative error code.
 */
int ad4630_init(void);

/**
 * @brief `ad4630_384kHz_fs_clk_and_cs_start()` enables the 384kHz ADC conversion clock and chip select signal. The chip select
 * signal from U2 is set as an output which echos the 384kHz conversion clock to be used as a chip select for the SPI
 * busses which read data from the ADC.
 *
 * @pre ADC initialization is complete, the LDOs are powered on via bsp_power_on_LDOs().
 *
 * @post the ADC conversion clock is enabled and the ADC continuously converts samples at 384k samples per second. The
 * chip selct line from the ADC clocking circuit is configured as an output.
 */
void ad4630_384kHz_fs_clk_and_cs_start(void);

/**
 * @brief `ad4630_384kHz_fs_clk_and_cs_stop()` disables the 384kHz ADC conversion clock and chip select signal, the chip select
 * line from U2 is set to high-Z.
 *
 * @pre ADC initialization is complete, the LDOs are powered on via bsp_power_on_LDOs().
 *
 * @post the ADC conversion clock is disabled and conversions stop, the chip select signal from the ADC clocking circuit
 * is left in a high-Z state.
 */
void ad4630_384kHz_fs_clk_and_cs_stop(void);

#endif /* AD4630_H_ */
