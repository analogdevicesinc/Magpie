#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "test_helpers.hpp"

extern "C"
{
#include "audio_dma.h"
#include "decimation_filter.h"
}

using namespace testing;

TEST(DecimationFilterTest, all_sample_rates_yield_correct_buff_lengths_as_output)
{
    const auto sample_rates = std::array<Wave_Header_Sample_Rate_t, 6>{
        // WAVE_HEADER_SAMPLE_RATE_384kHz, // don't do 384k, this is a special case that should not be filtered
        WAVE_HEADER_SAMPLE_RATE_192kHz,
        WAVE_HEADER_SAMPLE_RATE_96kHz,
        WAVE_HEADER_SAMPLE_RATE_48kHz,
        WAVE_HEADER_SAMPLE_RATE_32kHz,
        WAVE_HEADER_SAMPLE_RATE_24kHz,
        WAVE_HEADER_SAMPLE_RATE_16kHz,
    };

    q31_t src[AUDIO_DMA_BUFF_LEN_IN_SAMPS];
    q31_t dest[AUDIO_DMA_BUFF_LEN_IN_SAMPS];

    for (const auto sr : sample_rates)
    {

        decimation_filter_set_sample_rate(sr);
        const uint32_t actual_len = decimation_filter_downsample(src, dest, AUDIO_DMA_BUFF_LEN_IN_SAMPS);

        const uint32_t decimation_factor = WAVE_HEADER_SAMPLE_RATE_384kHz / sr;

        const uint32_t expected_len = AUDIO_DMA_BUFF_LEN_IN_SAMPS / decimation_factor;

        ASSERT_EQ(actual_len, expected_len);
    }
}

TEST(DecimationFilterTest, changing_sample_rates_does_not_influence_future_calls)
{
    q31_t src[AUDIO_DMA_BUFF_LEN_IN_SAMPS] = {0};

    q31_t dest_0[AUDIO_DMA_BUFF_LEN_IN_SAMPS] = {0};
    q31_t dest_1[AUDIO_DMA_BUFF_LEN_IN_SAMPS] = {0};

    // fill src with arbitrary non-zero data
    for (q31_t i = 0; i < AUDIO_DMA_BUFF_LEN_IN_SAMPS; i++)
    {
        src[i] = i % 1000;
    }

    decimation_filter_set_sample_rate(WAVE_HEADER_SAMPLE_RATE_192kHz);

    // store filtered samples in dest_0, we'll compare this against dest_1 later
    decimation_filter_downsample(src, dest_0, AUDIO_DMA_BUFF_LEN_IN_SAMPS);

    decimation_filter_set_sample_rate(WAVE_HEADER_SAMPLE_RATE_16kHz);

    // store different filtered samples in dest_1, we use a different sample rate and bit depth here
    decimation_filter_downsample(src, dest_1, AUDIO_DMA_BUFF_LEN_IN_SAMPS);

    // setting the sample rate back to 192k resets any impact that the 16k call above had
    decimation_filter_set_sample_rate(WAVE_HEADER_SAMPLE_RATE_192kHz);

    // after this dest_1 should be identical to dest_0, which shows that the middle call to decimate at 16k did not influence the final call to 192k
    decimation_filter_downsample(src, dest_1, AUDIO_DMA_BUFF_LEN_IN_SAMPS);

    ASSERT_THAT(dest_0, ContainerEq(dest_1));
}
