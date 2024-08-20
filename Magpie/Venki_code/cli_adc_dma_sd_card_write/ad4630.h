/**
 * @file      ad4630.h
 * @brief     A software interface for configuring the AD4630 ADC is represented here.
 * @details   This module is repsonsible for initializing the ADC and starting/stopping conversions.
 *            Access to the audio data converted by the ADC is handled by the audio_dma module.
 */

#ifndef AD4630_H_
#define AD4630_H_

/* Public enumerations -----------------------------------------------------------------------------------------------*/

/**
 * @brief AD4630 errors are represented here
 */
typedef enum
{
    AD4630_ERROR_ALL_OK,
    AD4630_ERROR_CONFIG_ERROR,
} AD4630_Error_t;

/* Public function declarations --------------------------------------------------------------------------------------*/

/***
 * @brief `ad4630_init()` initializes the AD4630 ADC
 *
 * @post the ADC is initialized and ready to use
 *
 * @retval `AD4630_ERROR_ALL_OK` if the operation succeeded, else an error code
 */
AD4630_Error_t ad4630_init();

/**
 * `ad4630_cont_conversions_start()` enables the ADC conversion clock and starts the ADC continuous conversions
 *
 * @pre ADC initialization is complete
 *
 * @post the ADC conversion clock is enabled and the ADC continuously converts samples at the rate of the ADC conversion clock
 */
void ad4630_cont_conversions_start();

/**
 * `ad4630_cont_conversions_stop()` disables the ADC conversion clock and stops the ADC continuous conversions
 *
 * @pre ADC initialization is complete
 *
 * @post the ADC conversion clock is disabled and conversions stop
 */
void ad4630_cont_conversions_stop();

#endif /* AD4630_H_ */
