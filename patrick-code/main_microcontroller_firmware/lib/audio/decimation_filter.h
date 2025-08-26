/**
 * @file      decimation_filter.h
 * @brief     A software interface for decimation filters is represented here.
 * @details   This module is used to down-sample the raw 384kHz 24-bit audio data from the ADC/DMA modules to other
 *            sample rates.
 */

#ifndef DECIMATION_FILTER_H_
#define DECIMATION_FILTER_H_

/* Includes ----------------------------------------------------------------------------------------------------------*/

#include <stdint.h>
#include "arm_math.h"
#include "audio_enums.h"

/* Public function declarations --------------------------------------------------------------------------------------*/

/**
 * @brief `decimation_filter_set_sample_rate(sr)` sets the sample rate for the decimation filter to `sr`. This must not be
 * called in the middle of writing a WAV file. Only call this between SD card file writes when you want to change
 * sample rates.
 *
 * @param sample_rate the enumerated sample rate to use for following calls to `decimation_filter_downsample()`. Can be
 * any enumerated sample rate EXCEPT for 384kHz, do not call with this sample rate.
 *
 * The sample rate is shared across both channels, two channels can not have two different sample rates at the same time.
 *
 * @post Future calls to `decimation_filter_downsample()` will use the sample rate set here until changed by calling
 * this function again with a new sample rate.
 */
void decimation_filter_set_sample_rate(Audio_Sample_Rate_t sample_rate);

/**
 * @brief `decimation_filter_downsample(s, d, n, c)` downsamples `n` samples from source buffer `s` using the existing
 * state for channel `c` and stores the result in destination buffer `d` with the sample rate previously set by
 * `decimation_filter_set_sample_rate(sr)`
 *
 * @pre `decimation_filter_set_sample_rate(sr)` has been called with the desired sample rate `sr`
 *
 * @param src_384kHz the source buffer to downsample. It is expected that this is the 384kHz buffer with little-endian
 * q31 samples. Must be at least as long as `num_samps_to_filter`
 *
 * @param dest the destination buffer for the downsampled data. Must be long enough to hold the downsampled data. The
 * size needed for the destination buffer is given by `num_samps_to_filter * (sr / 384e3)` where `sr` is the previously
 * set sample rate.
 *
 * @param num_samps_to_filter the number of samples from `src` to decimate and store in `dest`. This must be a multiple
 * of 16.
 *
 * @param channel the audio channel to use, internal state for each channel is maintained across function calls.
 *
 * @post the source buffer is downsampled and stored in the destination buffer.
 *
 * @retval the length of the downsampled destination buffer in samples, this will be an integer multiple of
 * `num_samps_to_filter` given by `num_samps_to_filter * (sr / 384e3)` if all preconditions are met.
 */
uint32_t decimation_filter_downsample(
    q31_t *src_384kHz,
    q31_t *dest,
    uint32_t num_samps_to_filter,
    Audio_Channel_t channel);

#endif /* DECIMATION_FILTER_H_ */
