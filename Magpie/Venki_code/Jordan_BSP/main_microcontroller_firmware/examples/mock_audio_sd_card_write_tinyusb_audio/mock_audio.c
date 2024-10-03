/**
 * A sine wave is generated using a basic form of Direct Digital Synthesis (DDS). A phase accumulator creates a ramp
 * wave at the given frequency and sample rate. The upper bits of the ramp index into a Look Up Table (LUT) to find
 * the current value of the sine wave stored in the LUT.
 */

/* Private includes --------------------------------------------------------------------------------------------------*/

#include <math.h>
#include <stdint.h>

/* Private defines ---------------------------------------------------------------------------------------------------*/

#define PHASE_ACCUM_NUM_BITS (24)
#define PHASE_ACCUM_2_TO_THE_N (1 << PHASE_ACCUM_NUM_BITS)

#define SINE_LUT_NUM_INDEX_BITS (10)
#define SINE_LUT_SIZE (1 << SINE_LUT_NUM_INDEX_BITS)

// turn down a bit from fullscale so we don't accidentally blast our speakers when testing
#define SINE_LUT_MAX_VAL (INT16_MAX / 2)

/* Private variables -------------------------------------------------------------------------------------------------*/

static int16_t sine_lut[SINE_LUT_SIZE];

static uint32_t phase_accumulator;

static uint32_t sample_rate;
static uint32_t accum_increment;

/* Public function definitions ---------------------------------------------------------------------------------------*/

void mock_audio_sine_init()
{
    for (uint32_t i = 0; i < SINE_LUT_SIZE; i++)
    {
        const float fl_sin = sin(2.0f * M_PI * ((float)i / (float)SINE_LUT_SIZE));
        const int16_t int_sin = (int16_t)(fl_sin * SINE_LUT_MAX_VAL);
        sine_lut[i] = int_sin;
    }
}

void mock_audio_set_sample_rate(uint32_t sr_Hz)
{
    sample_rate = sr_Hz;
}

void mock_audio_set_sine_freq(uint32_t freq_Hz)
{
    // note that depending on how we order these ops, we might overflow a u32, that's why the divide happens first
    accum_increment = (PHASE_ACCUM_2_TO_THE_N / sample_rate) * freq_Hz;
}

int16_t mock_audio_sine_tick()
{
    phase_accumulator += accum_increment;
    phase_accumulator &= (PHASE_ACCUM_2_TO_THE_N - 1);

    const uint32_t lut_idx = phase_accumulator >> (PHASE_ACCUM_NUM_BITS - SINE_LUT_NUM_INDEX_BITS);
    return sine_lut[lut_idx];
}
