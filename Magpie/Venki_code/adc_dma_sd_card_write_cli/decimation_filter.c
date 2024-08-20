
/* Private includes --------------------------------------------------------------------------------------------------*/

#include "arm_fir_decimate_fast_q31_bob.h"
#include "audio_dma.h"
#include "data_converters.h"
#include "decimation_filter.h"

/* Private defines ---------------------------------------------------------------------------------------------------*/

#define buffLen_deci2x (AUDIO_DMA_BUFF_LEN_IN_SAMPS / 2)
#define buffLen_deci3x (AUDIO_DMA_BUFF_LEN_IN_SAMPS / 3)
#define buffLen_deci4x (AUDIO_DMA_BUFF_LEN_IN_SAMPS / 4)
#define buffLen_deci6x (AUDIO_DMA_BUFF_LEN_IN_SAMPS / 6)
#define buffLen_deci8x (AUDIO_DMA_BUFF_LEN_IN_SAMPS / 8)
#define buffLen_deci12x (AUDIO_DMA_BUFF_LEN_IN_SAMPS / 12)
#define buffLen_deci16x (AUDIO_DMA_BUFF_LEN_IN_SAMPS / 16)
#define buffLen_deci24x (AUDIO_DMA_BUFF_LEN_IN_SAMPS / 24)

// 16 k
#define deci_16k_numcoeffs_0 5
#define deci_16k_numcoeffs_1 7
#define deci_16k_numcoeffs_2 9
#define deci_16k_numcoeffs_3 47
#define deci_state_len_16k_0 (AUDIO_DMA_BUFF_LEN_IN_SAMPS + deci_16k_numcoeffs_0 - 1)
#define deci_state_len_16k_1 (buffLen_deci2x + deci_16k_numcoeffs_1 - 1)
#define deci_state_len_16k_2 (buffLen_deci4x + deci_16k_numcoeffs_2 - 1)
#define deci_state_len_16k_3 (buffLen_deci8x + deci_16k_numcoeffs_3 - 1)

// 24 k
#define deci_24k_numcoeffs_0 5
#define deci_24k_numcoeffs_1 7
#define deci_24k_numcoeffs_2 9
#define deci_24k_numcoeffs_3 33
#define deci_state_len_24k_0 (AUDIO_DMA_BUFF_LEN_IN_SAMPS + deci_24k_numcoeffs_0 - 1)
#define deci_state_len_24k_1 (buffLen_deci2x + deci_24k_numcoeffs_1 - 1)
#define deci_state_len_24k_2 (buffLen_deci4x + deci_24k_numcoeffs_2 - 1)
#define deci_state_len_24k_3 (buffLen_deci8x + deci_24k_numcoeffs_3 - 1)

// 32 k
#define deci_32k_numcoeffs_0 7
#define deci_32k_numcoeffs_1 9
#define deci_32k_numcoeffs_2 47
#define deci_state_len_32k_0 (AUDIO_DMA_BUFF_LEN_IN_SAMPS + deci_32k_numcoeffs_0 - 1)
#define deci_state_len_32k_1 (buffLen_deci2x + deci_32k_numcoeffs_1 - 1)
#define deci_state_len_32k_2 (buffLen_deci4x + deci_32k_numcoeffs_2 - 1)

// 48 k
#define deci_48k_numcoeffs_0 7
#define deci_48k_numcoeffs_1 9
#define deci_48k_numcoeffs_2 33
#define deci_state_len_48k_0 (AUDIO_DMA_BUFF_LEN_IN_SAMPS + deci_48k_numcoeffs_0 - 1)
#define deci_state_len_48k_1 (buffLen_deci2x + deci_48k_numcoeffs_1 - 1)
#define deci_state_len_48k_2 (buffLen_deci4x + deci_48k_numcoeffs_2 - 1)

// 96 k
#define deci_96k_numcoeffs_0 9
#define deci_96k_numcoeffs_1 33
#define deci_state_len_96k_0 (AUDIO_DMA_BUFF_LEN_IN_SAMPS + deci_96k_numcoeffs_0 - 1)
#define deci_state_len_96k_1 (buffLen_deci2x + deci_96k_numcoeffs_1 - 1)

// 192 k
#define deci_192k_numcoeffs_0 15
#define deci_state_len_192k_0 (AUDIO_DMA_BUFF_LEN_IN_SAMPS + deci_192k_numcoeffs_0 - 1)

/* Private variables -------------------------------------------------------------------------------------------------*/

// 16 k
static q31_t firCoeffs_16k_0[deci_16k_numcoeffs_0] = {
    86385383, 380767973, 588547483, 380767973, 86385383};
static q31_t firCoeffs_16k_1[deci_16k_numcoeffs_1] = {
    -63623254, 11777617, 601023485, 1051356664, 601023485, 11777617, -63623254};
static q31_t firCoeffs_16k_2[deci_16k_numcoeffs_2] = {
    -27728029, -77658950, 90916825, 613537738, 945568961, 613537738, 90916825, -77658950, -27728029};
static q31_t firCoeffs_16k_3[deci_16k_numcoeffs_3] = {
    544679, 5591621, 10519170, 11396576, 3740919, -9199321, -16459240, -8186184, 12423382, 27105429, 17140829, -15659956, -43184586, -32667249, 18552286, 68866053, 61256147, -20863109, -119392334, -129148756, 22338651, 303309288, 578589578, 692986716, 578589578, 303309288, 22338651, -129148756, -119392334, -20863109, 61256147, 68866053, 18552286, -32667249, -43184586, -15659956, 17140829, 27105429, 12423382, -8186184, -16459240, -9199321, 3740919, 11396576, 10519170, 5591621, 544679};

// 24k
static q31_t firCoeffs_24k_0[deci_24k_numcoeffs_0] = {
    87026071, 382177371, 589816446, 382177371, 87026071};
static q31_t firCoeffs_24k_1[deci_24k_numcoeffs_1] = {
    -59682168, 25489114, 599055019, 1028294198, 599055019, 25489114, -59682168};
static q31_t firCoeffs_24k_2[deci_24k_numcoeffs_2] = {
    -35829136, -93392547, 90204797, 624894336, 955274946, 624894336, 90204797, -93392547, -35829136};
static q31_t firCoeffs_24k_3[deci_24k_numcoeffs_3] = {
    -2823963, 804105, 13756249, 13832557, -12099816, -21810016, 17681236, 39284877, -22118934, -66381589, 26258540, 112809109, -29540929, -212849373, 31655722, 678451831, 1041361918, 678451831, 31655722, -212849373, -29540929, 112809109, 26258540, -66381589, -22118934, 39284877, 17681236, -21810016, -12099816, 13832557, 13756249, 804105, -2823963};

// 32 k
static q31_t firCoeffs_32k_0[deci_32k_numcoeffs_0] = {
    -44988434, 8328033, 424987782, 743421426, 424987782, 8328033, -44988434};
static q31_t firCoeffs_32k_1[deci_32k_numcoeffs_1] = {
    -27728029, -77658950, 90916825, 613537738, 945568961, 613537738, 90916825, -77658950, -27728029};
static q31_t firCoeffs_32k_2[deci_32k_numcoeffs_2] = {
    544679, 5591621, 10519170, 11396576, 3740919, -9199321, -16459240, -8186184, 12423382, 27105429, 17140829, -15659956, -43184586, -32667249, 18552286, 68866053, 61256147, -20863109, -119392334, -129148756, 22338651, 303309288, 578589578, 692986716, 578589578, 303309288, 22338651, -129148756, -119392334, -20863109, 61256147, 68866053, 18552286, -32667249, -43184586, -15659956, 17140829, 27105429, 12423382, -8186184, -16459240, -9199321, 3740919, 11396576, 10519170, 5591621, 544679};

// 48 k
static q31_t firCoeffs_48k_0[deci_48k_numcoeffs_0] = {
    -42201666, 18023525, 423595866, 727113801, 423595866, 18023525, -42201666};
static q31_t firCoeffs_48k_1[deci_48k_numcoeffs_1] = {
    -35829136, -93392547, 90204797, 624894336, 955274946, 624894336, 90204797, -93392547, -35829136};
static q31_t firCoeffs_48k_2[deci_48k_numcoeffs_2] = {
    -2823963, 804105, 13756249, 13832557, -12099816, -21810016, 17681236, 39284877, -22118934, -66381589, 26258540, 112809109, -29540929, -212849373, 31655722, 678451831, 1041361918, 678451831, 31655722, -212849373, -29540929, 112809109, 26258540, -66381589, -22118934, 39284877, 17681236, -21810016, -12099816, 13832557, 13756249, 804105, -2823963};

// 96 k
static q31_t firCoeffs_96k_0[deci_96k_numcoeffs_0] = {
    -20749647, -66609278, 51582801, 442242045, 691682165, 442242045, 51582801, -66609278, -20749647};
static q31_t firCoeffs_96k_1[deci_96k_numcoeffs_1] = {
    -3229201, 1658598, 16721610, 16330065, -12855261, -23661054, 18450717, 41258441, -22628821, -68277309, 26456118, 114386501, -29451143, -213892037, 31368109, 678814008, 1041713732, 678814008, 31368109, -213892037, -29451143, 114386501, 26456118, -68277309, -22628821, 41258441, 18450717, -23661054, -12855261, 16330065, 16721610, 1658598, -3229201};

// 192k, 40dB version
static q31_t firCoeffs_192k_0[deci_192k_numcoeffs_0] = {
    -23215316, 15943290, 69341217, -4194368, -136036336, 11320770, 476387342, 749653914, 476387342, 11320770, -136036336, -4194368, 69341217, 15943290, -23215316};

// decimated buffers for various stages of the multi-rate filters, reused in a ping-pong fashion to save SRAM
static q31_t rx_ping_pong_buff_0[buffLen_deci2x] = {0}; // guaranteed to be big enough for the first round of any sample rate
static q31_t rx_ping_pong_buff_1[buffLen_deci4x] = {0}; // guaranteed to be big enough for the second round of any sample rate

// FIR states are reused across the filters to save SRMS, they must be zerod out and reinitialized when changing sample rates
static q31_t fir_state_0[deci_state_len_192k_0] = {0}; // guaranteed to be big enough for the first round of any sample rate
static q31_t fir_state_1[deci_state_len_96k_1] = {0};  // guaranteed to be big enough for the second round of any sample rate
static q31_t fir_state_2[deci_state_len_32k_2] = {0};  // guaranteed to be big enough for the third round of any sample rate
static q31_t fir_state_3[deci_state_len_16k_3] = {0};  // guaranteed to be big enough for the fourth round of any sample rate

// 16 k
static arm_fir_decimate_instance_q31 Sdeci_16k_0;
static arm_fir_decimate_instance_q31 Sdeci_16k_1;
static arm_fir_decimate_instance_q31 Sdeci_16k_2;
static arm_fir_decimate_instance_q31 Sdeci_16k_3;

// 24 k
static arm_fir_decimate_instance_q31 Sdeci_24k_0;
static arm_fir_decimate_instance_q31 Sdeci_24k_1;
static arm_fir_decimate_instance_q31 Sdeci_24k_2;
static arm_fir_decimate_instance_q31 Sdeci_24k_3;

// 32 k
static arm_fir_decimate_instance_q31 Sdeci_32k_0;
static arm_fir_decimate_instance_q31 Sdeci_32k_1;
static arm_fir_decimate_instance_q31 Sdeci_32k_2;

// 48 k
static arm_fir_decimate_instance_q31 Sdeci_48k_0;
static arm_fir_decimate_instance_q31 Sdeci_48k_1;
static arm_fir_decimate_instance_q31 Sdeci_48k_2;

// 96 k
static arm_fir_decimate_instance_q31 Sdeci_96k_0;
static arm_fir_decimate_instance_q31 Sdeci_96k_1;

// 192 k
static arm_fir_decimate_instance_q31 Sdeci_192k_0;

// the current sample rate to use when decimating
static Wave_Header_Sample_Rate_t current_sample_rate = WAVE_HEADER_SAMPLE_RATE_384kHz;

/* Public function definitions ---------------------------------------------------------------------------------------*/

void decimation_filter_set_sample_rate(Wave_Header_Sample_Rate_t sample_rate)
{
    current_sample_rate = sample_rate;

    // reset the FIR state buffers
    // memset(fir_state_0, 0, deci_state_len_192k_0);
    // memset(fir_state_1, 0, deci_state_len_96k_1);
    // memset(fir_state_2, 0, deci_state_len_32k_2);
    // memset(fir_state_3, 0, deci_state_len_16k_3);

    switch (sample_rate)
    {
    case WAVE_HEADER_SAMPLE_RATE_384kHz:
        // don't need to do anything for the special case 384k, it does no filtering and should not be called here
        break;

    case WAVE_HEADER_SAMPLE_RATE_192kHz:
        arm_fir_decimate_init_q31(&Sdeci_192k_0, deci_192k_numcoeffs_0, 2, firCoeffs_192k_0, fir_state_0, AUDIO_DMA_BUFF_LEN_IN_SAMPS);
        break;

    case WAVE_HEADER_SAMPLE_RATE_96kHz:
        arm_fir_decimate_init_q31(&Sdeci_96k_0, deci_96k_numcoeffs_0, 2, firCoeffs_96k_0, fir_state_0, AUDIO_DMA_BUFF_LEN_IN_SAMPS);
        arm_fir_decimate_init_q31(&Sdeci_96k_1, deci_96k_numcoeffs_1, 2, firCoeffs_96k_1, fir_state_1, buffLen_deci2x);
        break;

    case WAVE_HEADER_SAMPLE_RATE_48kHz:
        arm_fir_decimate_init_q31(&Sdeci_48k_0, deci_48k_numcoeffs_0, 2, firCoeffs_48k_0, fir_state_0, AUDIO_DMA_BUFF_LEN_IN_SAMPS);
        arm_fir_decimate_init_q31(&Sdeci_48k_1, deci_48k_numcoeffs_1, 2, firCoeffs_48k_1, fir_state_1, buffLen_deci2x);
        arm_fir_decimate_init_q31(&Sdeci_48k_2, deci_48k_numcoeffs_2, 2, firCoeffs_48k_2, fir_state_2, buffLen_deci4x);
        break;

    case WAVE_HEADER_SAMPLE_RATE_32kHz:
        arm_fir_decimate_init_q31(&Sdeci_32k_0, deci_32k_numcoeffs_0, 3, firCoeffs_32k_0, fir_state_0, AUDIO_DMA_BUFF_LEN_IN_SAMPS);
        arm_fir_decimate_init_q31(&Sdeci_32k_1, deci_32k_numcoeffs_1, 2, firCoeffs_32k_1, fir_state_1, buffLen_deci3x);
        arm_fir_decimate_init_q31(&Sdeci_32k_2, deci_32k_numcoeffs_2, 2, firCoeffs_32k_2, fir_state_2, buffLen_deci6x);
        break;

    case WAVE_HEADER_SAMPLE_RATE_24kHz:
        arm_fir_decimate_init_q31(&Sdeci_24k_0, deci_24k_numcoeffs_0, 2, firCoeffs_24k_0, fir_state_0, AUDIO_DMA_BUFF_LEN_IN_SAMPS);
        arm_fir_decimate_init_q31(&Sdeci_24k_1, deci_24k_numcoeffs_1, 2, firCoeffs_24k_1, fir_state_1, buffLen_deci2x);
        arm_fir_decimate_init_q31(&Sdeci_24k_2, deci_24k_numcoeffs_2, 2, firCoeffs_24k_2, fir_state_2, buffLen_deci4x);
        arm_fir_decimate_init_q31(&Sdeci_24k_3, deci_24k_numcoeffs_3, 2, firCoeffs_24k_3, fir_state_3, buffLen_deci8x);
        break;

    case WAVE_HEADER_SAMPLE_RATE_16kHz:
        arm_fir_decimate_init_q31(&Sdeci_16k_0, deci_16k_numcoeffs_0, 3, firCoeffs_16k_0, fir_state_0, AUDIO_DMA_BUFF_LEN_IN_SAMPS);
        arm_fir_decimate_init_q31(&Sdeci_16k_1, deci_16k_numcoeffs_1, 2, firCoeffs_16k_1, fir_state_1, buffLen_deci3x);
        arm_fir_decimate_init_q31(&Sdeci_16k_2, deci_16k_numcoeffs_2, 2, firCoeffs_16k_2, fir_state_2, buffLen_deci6x);
        arm_fir_decimate_init_q31(&Sdeci_16k_3, deci_16k_numcoeffs_3, 2, firCoeffs_16k_3, fir_state_3, buffLen_deci12x);
        break;
    }
}

uint32_t decimation_filter_downsample(
    q31_t *src_384kHz,
    q31_t *dest,
    uint32_t num_samps_to_filter)
{
    switch (current_sample_rate)
    {
    case WAVE_HEADER_SAMPLE_RATE_384kHz:
        // 384k is a special case which should not be called
        return 0;

    case WAVE_HEADER_SAMPLE_RATE_192kHz:
        arm_fir_decimate_fast_q31_bob(&Sdeci_192k_0, src_384kHz, dest, num_samps_to_filter);

        return num_samps_to_filter / 2;

    case WAVE_HEADER_SAMPLE_RATE_96kHz:
        arm_fir_decimate_fast_q31_bob(&Sdeci_96k_0, src_384kHz, rx_ping_pong_buff_0, num_samps_to_filter);
        arm_fir_decimate_fast_q31_bob(&Sdeci_96k_1, rx_ping_pong_buff_0, dest, num_samps_to_filter / 2);

        return num_samps_to_filter / 4;

    case WAVE_HEADER_SAMPLE_RATE_48kHz:
        arm_fir_decimate_fast_q31_bob(&Sdeci_48k_0, src_384kHz, rx_ping_pong_buff_0, num_samps_to_filter);
        arm_fir_decimate_fast_q31_bob(&Sdeci_48k_1, rx_ping_pong_buff_0, rx_ping_pong_buff_1, num_samps_to_filter / 2);
        arm_fir_decimate_fast_q31_bob(&Sdeci_48k_2, rx_ping_pong_buff_1, dest, num_samps_to_filter / 4);

        return num_samps_to_filter / 8;

    case WAVE_HEADER_SAMPLE_RATE_32kHz:
        arm_fir_decimate_fast_q31_bob(&Sdeci_32k_0, src_384kHz, rx_ping_pong_buff_0, num_samps_to_filter);
        arm_fir_decimate_fast_q31_bob(&Sdeci_32k_1, rx_ping_pong_buff_0, rx_ping_pong_buff_1, num_samps_to_filter / 3);
        arm_fir_decimate_fast_q31_bob(&Sdeci_32k_2, rx_ping_pong_buff_1, dest, num_samps_to_filter / 6);

        return num_samps_to_filter / 12;

    case WAVE_HEADER_SAMPLE_RATE_24kHz:
        arm_fir_decimate_fast_q31_bob(&Sdeci_24k_0, src_384kHz, rx_ping_pong_buff_0, num_samps_to_filter);
        arm_fir_decimate_fast_q31_bob(&Sdeci_24k_1, rx_ping_pong_buff_0, rx_ping_pong_buff_1, num_samps_to_filter / 2);
        arm_fir_decimate_fast_q31_bob(&Sdeci_24k_2, rx_ping_pong_buff_1, rx_ping_pong_buff_0, num_samps_to_filter / 4);
        arm_fir_decimate_fast_q31_bob(&Sdeci_24k_3, rx_ping_pong_buff_0, dest, num_samps_to_filter / 8);

        return num_samps_to_filter / 16;

    case WAVE_HEADER_SAMPLE_RATE_16kHz:
        arm_fir_decimate_fast_q31_bob(&Sdeci_16k_0, src_384kHz, rx_ping_pong_buff_0, num_samps_to_filter);
        arm_fir_decimate_fast_q31_bob(&Sdeci_16k_1, rx_ping_pong_buff_0, rx_ping_pong_buff_1, num_samps_to_filter / 3);
        arm_fir_decimate_fast_q31_bob(&Sdeci_16k_2, rx_ping_pong_buff_1, rx_ping_pong_buff_0, num_samps_to_filter / 6);
        arm_fir_decimate_fast_q31_bob(&Sdeci_16k_3, rx_ping_pong_buff_0, dest, num_samps_to_filter / 12);

        return num_samps_to_filter / 24;
    }

    // never reached if all preconditions are met
    return 0;
}
