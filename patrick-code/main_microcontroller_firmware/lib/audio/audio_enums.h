/**
 * @file      audio_enums.h
 * @brief     Some common enumerations used by the audio system are represented here
 */

#ifndef AUDIO_ENUMS_H__
#define AUDIO_ENUMS_H__

/**
 * @brief  Enumerated Audio channels are represented here.
 * The Magpie has two independent analog audio channels.
 */
typedef enum
{
    AUDIO_CHANNEL_0 = 0,
    AUDIO_CHANNEL_1 = 1,
} Audio_Channel_t;

/**
 * @brief Enumerted Audio sample rates are represented here.
 * Each WAV file has exactly one sample-rate which is shared by all channels in the WAV file.
 */
typedef enum
{
    AUDIO_SAMPLE_RATE_24kHz = 24000,
    AUDIO_SAMPLE_RATE_48kHz = 48000,
    AUDIO_SAMPLE_RATE_96kHz = 96000,
    AUDIO_SAMPLE_RATE_192kHz = 192000,
    AUDIO_SAMPLE_RATE_384kHz = 384000,
} Audio_Sample_Rate_t;

/**
 * @brief Enumerated Audio bit-depths are represented here.
 * Each WAV file has exactly one bit-depth which is shared by all channels in the WAV file.
 */
typedef enum
{
    AUDIO_BIT_DEPTH_16_BITS_PER_SAMPLE = 16,
    AUDIO_BIT_DEPTH_24_BITS_PER_SAMPLE = 24,
} Audio_Bits_Per_Sample_t;

/**
 * @brief Enumerated Audio gain settings are represented here.
 * The gain for each channel is independent, the two channels can have different gain.
 */
typedef enum
{
    AUDIO_GAIN_5dB = 5,
    AUDIO_GAIN_10dB = 10,
    AUDIO_GAIN_15dB = 15,
    AUDIO_GAIN_20dB = 20,
    AUDIO_GAIN_25dB = 25,
    AUDIO_GAIN_30dB = 30,
    AUDIO_GAIN_35dB = 35,
    AUDIO_GAIN_40dB = 40,
    AUDIO_GAIN_UNDEFINED = 255, // sentinal value for undefined or invalid gain settings
} Audio_Gain_t;

#endif
