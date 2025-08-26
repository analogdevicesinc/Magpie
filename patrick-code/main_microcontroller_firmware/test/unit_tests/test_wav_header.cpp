/**
 * This test suite is a very glass-box view of the WAV header. While viewing or
 * modifying this file it may help to have a WAV header refernece open, for
 * example: https://docs.fileformat.com/audio/wav/
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test_helpers.hpp"

extern "C"
{
#include "wav_header.h"
}

/**
 * @brief Enumerated starting positions for the various sections of a wav header are represented here.
 */
enum Wave_Header_Position_t
{
    POS_START_OF_RIFF = 0,
    POS_START_OF_FILE_LEN_MINUS_8 = 4,
    POS_START_OF_WAVE = 8,
    POS_START_OF_FMT_ = 12,
    POS_START_OF_FMT_CHUNK_SIZE = 16,
    POS_START_OF_FMT_TAG = 20,
    POS_START_OF_NUM_CHANNELS = 22,
    POS_START_OF_SAMPLE_RATE = 24,
    POS_START_OF_BYTES_PER_SEC = 28,
    POS_START_OF_BYTES_PER_BLOCK = 32,
    POS_START_OF_BITS_PER_SAMPLE = 34,
    POS_START_OF_DATA_STR = 36,
    POS_START_OF_DATA_LEN = 40,
};

// a handle to the wav header so we can use a slightly shorter name in the code
static char *wav_header = wav_header_get_header();

TEST(WavHeaderTest, header_length_is_44_bytes)
{
    ASSERT_EQ(wav_header_get_header_length(), 44);
}

TEST(WavHeaderTest, string_RIFF_is_at_the_start)
{
    ASSERT_EQ(wav_header[POS_START_OF_RIFF + 0], 'R');
    ASSERT_EQ(wav_header[POS_START_OF_RIFF + 1], 'I');
    ASSERT_EQ(wav_header[POS_START_OF_RIFF + 2], 'F');
    ASSERT_EQ(wav_header[POS_START_OF_RIFF + 3], 'F');
}

TEST(WavHeaderTest, eight_is_subtracted_from_file_len)
{
    // just an arbitrary constant where it's easy to see if 8 has been subtracted
    Wave_Header_Attributes_t attr = {.file_length = 12345678};
    wav_header_set_attributes(&attr);

    const uint32_t file_len_minus_8 = arr_slice_to_u32(wav_header, POS_START_OF_FILE_LEN_MINUS_8);

    ASSERT_EQ(file_len_minus_8, 12345670); // should be 8 smaller than the one we originally set
}

TEST(WavHeaderTest, string_WAVE_is_in_the_right_spot)
{
    ASSERT_EQ(wav_header[POS_START_OF_WAVE + 0], 'W');
    ASSERT_EQ(wav_header[POS_START_OF_WAVE + 1], 'A');
    ASSERT_EQ(wav_header[POS_START_OF_WAVE + 2], 'V');
    ASSERT_EQ(wav_header[POS_START_OF_WAVE + 3], 'E');
}

TEST(WavHeaderTest, string_fmt_is_in_the_right_spot)
{
    ASSERT_EQ(wav_header[POS_START_OF_FMT_ + 0], 'f');
    ASSERT_EQ(wav_header[POS_START_OF_FMT_ + 1], 'm');
    ASSERT_EQ(wav_header[POS_START_OF_FMT_ + 2], 't');
    ASSERT_EQ(wav_header[POS_START_OF_FMT_ + 3], ' ');
}

TEST(WavHeaderTest, fmt_chunk_size_is_always_16)
{
    const uint32_t fmt_chunk_size = arr_slice_to_u32(wav_header, POS_START_OF_FMT_CHUNK_SIZE);
    ASSERT_EQ(fmt_chunk_size, 16);
}

TEST(WavHeaderTest, fmt_tag_is_always_PCM)
{
    const uint16_t fmt_tag = arr_slice_to_u16(wav_header, POS_START_OF_FMT_TAG);
    ASSERT_EQ(fmt_tag, 1); // 1 means PCM, other integer values represent other formats which we don't use
}

TEST(WavHeaderTest, num_channels_can_be_set)
{
    Wave_Header_Attributes_t attr = {.num_channels = WAVE_HEADER_MONO};
    wav_header_set_attributes(&attr);

    uint16_t num_channels = arr_slice_to_u16(wav_header, POS_START_OF_NUM_CHANNELS);
    ASSERT_EQ(num_channels, 1);

    attr.num_channels = WAVE_HEADER_STEREO;
    wav_header_set_attributes(&attr);

    num_channels = arr_slice_to_u16(wav_header, POS_START_OF_NUM_CHANNELS);
    ASSERT_EQ(num_channels, 2);
}

TEST(WavHeaderTest, sample_rate_can_be_set)
{
    Wave_Header_Attributes_t attr = {.sample_rate = AUDIO_SAMPLE_RATE_24kHz};

    wav_header_set_attributes(&attr);

    uint32_t sample_rate = arr_slice_to_u32(wav_header, POS_START_OF_SAMPLE_RATE);
    ASSERT_EQ(sample_rate, 24000);

    attr.sample_rate = AUDIO_SAMPLE_RATE_48kHz;
    wav_header_set_attributes(&attr);

    sample_rate = arr_slice_to_u32(wav_header, POS_START_OF_SAMPLE_RATE);
    ASSERT_EQ(sample_rate, 48000);

    attr.sample_rate = AUDIO_SAMPLE_RATE_384kHz;
    wav_header_set_attributes(&attr);

    sample_rate = arr_slice_to_u32(wav_header, POS_START_OF_SAMPLE_RATE);
    ASSERT_EQ(sample_rate, 384000);
}

TEST(WavHeaderTest, bytes_per_sec_is_calculated_and_set)
{
    // bytes per second depends on the num channels, the bit depth, and sample rate
    Wave_Header_Attributes_t attr = {
        .num_channels = WAVE_HEADER_STEREO,
        .bits_per_sample = AUDIO_BIT_DEPTH_16_BITS_PER_SAMPLE,
        .sample_rate = AUDIO_SAMPLE_RATE_24kHz};
    wav_header_set_attributes(&attr);

    uint32_t bytes_per_sec = arr_slice_to_u32(wav_header, POS_START_OF_BYTES_PER_SEC);
    ASSERT_EQ(bytes_per_sec, (2 * 16 * 24000) / 8);

    attr.num_channels = WAVE_HEADER_MONO;
    attr.bits_per_sample = AUDIO_BIT_DEPTH_24_BITS_PER_SAMPLE;
    attr.sample_rate = AUDIO_SAMPLE_RATE_384kHz;
    wav_header_set_attributes(&attr);

    bytes_per_sec = arr_slice_to_u32(wav_header, POS_START_OF_BYTES_PER_SEC);
    ASSERT_EQ(bytes_per_sec, (1 * 24 * 384000) / 8);
}

TEST(WavHeaderTest, bytes_per_block_is_calculated_and_set)
{
    // bytes per block relies on num channels and bit depth
    Wave_Header_Attributes_t attr = {
        .num_channels = WAVE_HEADER_STEREO,
        .bits_per_sample = AUDIO_BIT_DEPTH_16_BITS_PER_SAMPLE};
    wav_header_set_attributes(&attr);

    uint32_t bytes_per_block = arr_slice_to_u16(wav_header, POS_START_OF_BYTES_PER_BLOCK);
    ASSERT_EQ(bytes_per_block, 2 * 2);

    attr.bits_per_sample = AUDIO_BIT_DEPTH_24_BITS_PER_SAMPLE;
    wav_header_set_attributes(&attr);

    bytes_per_block = arr_slice_to_u16(wav_header, POS_START_OF_BYTES_PER_BLOCK);
    ASSERT_EQ(bytes_per_block, 2 * 3);

    attr.num_channels = WAVE_HEADER_MONO;
    attr.bits_per_sample = AUDIO_BIT_DEPTH_24_BITS_PER_SAMPLE;
    wav_header_set_attributes(&attr);

    bytes_per_block = arr_slice_to_u16(wav_header, POS_START_OF_BYTES_PER_BLOCK);
    ASSERT_EQ(bytes_per_block, 1 * 3);
}

TEST(WavHeaderTest, bits_per_sample_is_set)
{
    Wave_Header_Attributes_t attr = {.bits_per_sample = AUDIO_BIT_DEPTH_16_BITS_PER_SAMPLE};
    wav_header_set_attributes(&attr);

    uint32_t bits_per_samp = arr_slice_to_u16(wav_header, POS_START_OF_BITS_PER_SAMPLE);
    ASSERT_EQ(bits_per_samp, 16);

    attr.bits_per_sample = AUDIO_BIT_DEPTH_24_BITS_PER_SAMPLE;
    wav_header_set_attributes(&attr);

    bits_per_samp = arr_slice_to_u16(wav_header, POS_START_OF_BITS_PER_SAMPLE);
    ASSERT_EQ(bits_per_samp, 24);
}

TEST(WavHeaderTest, string_data_is_in_the_right_spot)
{
    ASSERT_EQ(wav_header[POS_START_OF_DATA_STR + 0], 'd');
    ASSERT_EQ(wav_header[POS_START_OF_DATA_STR + 1], 'a');
    ASSERT_EQ(wav_header[POS_START_OF_DATA_STR + 2], 't');
    ASSERT_EQ(wav_header[POS_START_OF_DATA_STR + 3], 'a');
}

TEST(WavHeaderTest, data_size_does_not_include_header_len)
{
    /**
     * When we set the file length field, this is the length of the whole file including the header itself. This is
     * because it's easier to ask the FatFS library to give you the length of the whole file before updating the wav
     * header. Hopwever, the internal field is the length of the data section only, not including the length of the
     * header itself. The calculation is handled internally by the wav header module under test. This test verifies
     * that when we set the file length, the data length is calculated correctly.
     */
    const uint32_t arbitrary_file_len = 100044; // should come out to 100k after the 44 bytes of the header are subtracted

    Wave_Header_Attributes_t attr = {.file_length = arbitrary_file_len};
    wav_header_set_attributes(&attr);

    const uint32_t data_section_len = arr_slice_to_u32(wav_header, POS_START_OF_DATA_LEN);
    ASSERT_EQ(data_section_len, arbitrary_file_len - wav_header_get_header_length());
}