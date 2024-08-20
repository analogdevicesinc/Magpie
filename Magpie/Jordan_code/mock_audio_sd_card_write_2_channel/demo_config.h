/**
 * @file    demo_config.h
 * @brief   This module simply provides some definitions for configuring the demo application.
 */

#ifndef DEMO_CONFIG_H_
#define DEMO_CONFIG_H_

/* Includes ----------------------------------------------------------------------------------------------------------*/

#include "wav_header.h"

/* Public defines ----------------------------------------------------------------------------------------------------*/

// The sample rate to use for the audio on the SD card, a positive integer
#define DEMO_CONFIG_SAMPLE_RATE_HZ (WAVE_HEADER_SAMPLE_RATE_48kHz)

// the sine frequency to write to the SD card, a positive integer below sample-rate / 2
#define DEMO_CONFIG_SINE_FREQUENCY_HZ (1000)

// the length of the WAVE file to write to the SD card, a positive integer, long file durations will tale a long time to write
#define DEMO_CONFIG_AUDIO_FILE_LENGTH_MINUTES (1)

// the length of the audio buffer in samples, not in bytes, a positive integer
#define DEMO_CONFIG_AUDIO_BUFF_LEN_IN_SAMPS (8256)

// the bit depth for the audio data, typically 16, a positive integer
#define DEMO_CONFIG_BITS_PER_SAMPLE (WAVE_HEADER_16_BITS_PER_SAMPLE)

#endif /* DEMO_CONFIG_H_ */
