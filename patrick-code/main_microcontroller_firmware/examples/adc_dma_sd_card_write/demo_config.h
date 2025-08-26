/**
 * @file    demo_config.h
 * @brief   This module simply provides some definitions for configuring the demo application.
 */

#ifndef DEMO_CONFIG_H_
#define DEMO_CONFIG_H_

/* Includes ----------------------------------------------------------------------------------------------------------*/

#include "audio_enums.h"
#include "sd_card_bank_ctl.h"
#include "wav_header.h"

/* Public defines ----------------------------------------------------------------------------------------------------*/

// the length of the WAVE file to write to the SD card, a positive integer, long file durations will tale a long time to write
// max value 4k seconds, about 70 minutes (we will remove this limitation in the final code, limited for the demo for simplicity).
#define DEMO_CONFIG_AUDIO_FILE_LEN_IN_SECONDS (5)

// comment or uncomment sample rates to add them to the test
const Audio_Sample_Rate_t demo_sample_rates_to_test[] = {
    // AUDIO_SAMPLE_RATE_24kHz,
    // AUDIO_SAMPLE_RATE_48kHz,
    // AUDIO_SAMPLE_RATE_96kHz,
    // AUDIO_SAMPLE_RATE_192kHz,
    AUDIO_SAMPLE_RATE_384kHz,
};

const uint32_t DEMO_CONFIG_NUM_SAMPLE_RATES_TO_TEST = sizeof(demo_sample_rates_to_test) / sizeof(demo_sample_rates_to_test[0]);

// comment or uncomment bit depths to add them to the test
const Audio_Bits_Per_Sample_t demo_bit_depths_to_test[] = {
    AUDIO_BIT_DEPTH_16_BITS_PER_SAMPLE,
    AUDIO_BIT_DEPTH_24_BITS_PER_SAMPLE,
};

const uint32_t DEMO_CONFIG_NUM_BIT_DEPTHS_TO_TEST = sizeof(demo_bit_depths_to_test) / sizeof(demo_bit_depths_to_test[0]);

// comment of uncomment mono/stereo variations to add them to the test
const Wave_Header_Num_Channels_t demo_num_channel_variations_to_test[] = {
    WAVE_HEADER_MONO,
    // WAVE_HEADER_STEREO,
};

const uint32_t DEMO_CONFIG_NUM_CHANNEL_VARIATIONS_TO_TEST = sizeof(demo_num_channel_variations_to_test) / sizeof(demo_num_channel_variations_to_test)[0];

// in mono mode, we only record one channel, choose it here
const Audio_Channel_t DEMO_CONFIG_CHANNEL_TO_USE_FOR_MONO_MODE = AUDIO_CHANNEL_0;

// gain is shared by both mics
const Audio_Gain_t DEMO_CONFIG_AUDIO_GAIN = AUDIO_GAIN_40dB;

// choose one SD card slot to use
const SD_Card_Bank_Card_Slot_t DEMO_CONFIG_SD_CARD_SLOT_TO_USE = SD_CARD_BANK_CARD_SLOT_0;

#endif /* DEMO_CONFIG_H_ */
