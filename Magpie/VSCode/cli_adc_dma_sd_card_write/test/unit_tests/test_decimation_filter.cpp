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
    const auto sample_rates = std::array<Wave_Header_Sample_Rate_t, 4>{
        // WAVE_HEADER_SAMPLE_RATE_384kHz, // don't do 384k, this is a special case that should not be filtered
        WAVE_HEADER_SAMPLE_RATE_192kHz,
        WAVE_HEADER_SAMPLE_RATE_96kHz,
        WAVE_HEADER_SAMPLE_RATE_48kHz,
        WAVE_HEADER_SAMPLE_RATE_24kHz,
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
