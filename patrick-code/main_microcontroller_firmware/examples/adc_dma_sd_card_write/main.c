
/* Private includes --------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "mxc_delay.h"

#include "ad4630.h"
#include "afe_control.h"
#include "audio_dma.h"
#include "audio_enums.h"
#include "board.h"
#include "bsp_i2c.h"
#include "bsp_pins.h"
#include "bsp_status_led.h"
#include "data_converters.h"
#include "decimation_filter.h"
#include "demo_config.h"
#include "sd_card.h"
#include "sd_card_bank_ctl.h"
#include "wav_header.h"

#ifdef TERMINAL_IO_USE_SEGGER_RTT
#include "SEGGER_RTT.h"
#endif

/* Private function declarations -------------------------------------------------------------------------------------*/

/**
 * @brief `write_demo_wav_file(a, l)` writes a wav file with attributes `a`, and length in seconds `l`, with a name
 * derived from the attributes. Calling this function starts the ADC/DMA and continuously records audio in blocking
 * fashion until the time is up.
 *
 * @pre initialization is complete for the ADC, DMA, decimation filters, and SD card, the SD card must be mounted
 *
 * @param wav_attr pointer to the wav header attributes structure holding information about sample rate, bit depth, etc
 *
 * @param file_len_secs the length of the audio file to write, in seconds
 *
 * @post this function consumes buffers from the ADC/DMA until the duration of the file length has elapsed and writes
 * the audio data out to a .wav file on the SD card. The wav header for the file is also written in this function.
 */
static void write_demo_wav_file(Wave_Header_Attributes_t *wav_attr, uint32_t file_len_secs);

// the error handler simply rapidly blinks the given LED color forever
static void error_handler(Status_LED_Color_t c);

/* Public function definitions ---------------------------------------------------------------------------------------*/

int main(void)
{
#ifdef TERMINAL_IO_USE_CONSOLE_UART
    bsp_console_uart_init();
#endif

    printf("\n*********** ADC/DMA Wave File Write Example ***********\n\n");

    // simple LED pattern for a visual indication of a reset
    status_led_set(STATUS_LED_COLOR_RED, true);
    MXC_Delay(500000);
    status_led_set(STATUS_LED_COLOR_GREEN, true);
    MXC_Delay(500000);
    status_led_set(STATUS_LED_COLOR_BLUE, true);
    MXC_Delay(1000000);
    status_led_all_off();

    bsp_power_on_LDOs();

    if (ad4630_init() != E_NO_ERROR)
    {
        printf("[ERROR]--> AD4630 init\n");
        error_handler(STATUS_LED_COLOR_BLUE);
    }
    else
    {
        printf("[SUCCESS]--> AD4630 init\n");
    }

    if (audio_dma_init() != E_NO_ERROR)
    {
        printf("[ERROR]--> DMA init\n");
        error_handler(STATUS_LED_COLOR_BLUE);
    }
    else
    {
        printf("[SUCCESS]--> DMA init\n");
    }

    if (bsp_3v3_i2c_init() != E_NO_ERROR)
    {
        printf("[ERROR]--> I2C init\n");
        error_handler(STATUS_LED_COLOR_GREEN);
    }
    else
    {
        printf("[SUCCESS]--> I2C init\n");
    }

    if (bsp_1v8_i2c_init() != E_NO_ERROR)
    {
        printf("[ERROR]--> 1V8 I2C init\n");
        error_handler(STATUS_LED_COLOR_GREEN);
    }
    else
    {
        printf("[SUCCESS]--> 1V8 I2C init\n");
    }

    if (afe_control_init() != E_NO_ERROR)
    {
        printf("[ERROR]--> AFE Control init\n");
        error_handler(STATUS_LED_COLOR_GREEN);
    }
    else
    {
        printf("[SUCCESS]--> AFE Control init\n");
    }

    if (afe_control_enable(AUDIO_CHANNEL_0) != E_NO_ERROR)
    {
        printf("[ERROR]--> AFE Control CH0 EN\n");
        error_handler(STATUS_LED_COLOR_GREEN);
    }
    else
    {
        printf("[SUCCESS]--> AFE Control CH0 EN\n");
    }
    if (afe_control_enable(AUDIO_CHANNEL_1) != E_NO_ERROR)
    {
        printf("[ERROR]--> AFE Control CH1 EN\n");
        error_handler(STATUS_LED_COLOR_GREEN);
    }
    else
    {
        printf("[SUCCESS]--> AFE Control CH1 EN\n");
    }

    if (afe_control_set_gain(AUDIO_CHANNEL_0, DEMO_CONFIG_AUDIO_GAIN) != E_NO_ERROR)
    {
        printf("[ERROR]--> AFE Control CH0 gain set to %ddB\n", DEMO_CONFIG_AUDIO_GAIN);
        error_handler(STATUS_LED_COLOR_GREEN);
    }
    else
    {
        printf("[SUCCESS]--> AFE Control CH0 gain set to %ddB\n", DEMO_CONFIG_AUDIO_GAIN);
    }
    if (afe_control_set_gain(AUDIO_CHANNEL_1, DEMO_CONFIG_AUDIO_GAIN) != E_NO_ERROR)
    {
        printf("[ERROR]--> AFE Control CH1 gain set to %ddB\n", DEMO_CONFIG_AUDIO_GAIN);
        error_handler(STATUS_LED_COLOR_GREEN);
    }
    else
    {
        printf("[SUCCESS]--> AFE Control CH1 gain set to %ddB\n", DEMO_CONFIG_AUDIO_GAIN);
    }

    Audio_Gain_t readback_gain = afe_control_get_gain(AUDIO_CHANNEL_0);
    if (readback_gain != DEMO_CONFIG_AUDIO_GAIN)
    {
        printf("[ERROR]--> AFE CH0 set (%ddB) and get (%ddB) gain don't match\n", DEMO_CONFIG_AUDIO_GAIN, readback_gain);
        error_handler(STATUS_LED_COLOR_GREEN);
    }
    else
    {
        printf("[SUCCESS]--> AFE CH0 get-gain matches AFE set-gain\n");
    }
    readback_gain = afe_control_get_gain(AUDIO_CHANNEL_1);
    if (readback_gain != DEMO_CONFIG_AUDIO_GAIN)
    {
        printf("[ERROR]--> AFE CH1 set (%ddB) and get (%ddB) gain don't match\n", DEMO_CONFIG_AUDIO_GAIN, readback_gain);
        error_handler(STATUS_LED_COLOR_GREEN);
    }
    else
    {
        printf("[SUCCESS]--> AFE CH1 get-gain matches AFE set-gain\n");
    }
    if (sd_card_bank_ctl_init() != E_NO_ERROR)
    {
        printf("[ERROR]--> SD card bank ctl init\n");
        error_handler(STATUS_LED_COLOR_GREEN);
    }
    else
    {
        printf("[SUCCESS]--> SD card bank ctl init\n");
    }

    sd_card_bank_ctl_enable_slot(DEMO_CONFIG_SD_CARD_SLOT_TO_USE);

    sd_card_bank_ctl_read_and_cache_detect_pins();

    if (!sd_card_bank_ctl_active_card_is_inserted())
    {
        printf("[ERROR]--> Card at slot %d not inserted\n", DEMO_CONFIG_SD_CARD_SLOT_TO_USE);
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> SD card inserted in slot %d\n", DEMO_CONFIG_SD_CARD_SLOT_TO_USE);
    }

    if (sd_card_init() != E_NO_ERROR)
    {
        printf("[ERROR]--> SD card init\n");
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> SD card init\n");
    }

    // without a brief delay between card init and mount, there are often mount errors
    MXC_Delay(100000);

    if (sd_card_mount() != E_NO_ERROR)
    {
        printf("[ERROR]--> SD card mount\n");
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> SD card mounted\n");
    }

    Wave_Header_Attributes_t wav_attr;

    for (uint32_t sr = 0; sr < DEMO_CONFIG_NUM_SAMPLE_RATES_TO_TEST; sr++)
    {
        for (uint32_t bd = 0; bd < DEMO_CONFIG_NUM_BIT_DEPTHS_TO_TEST; bd++)
        {
            for (uint32_t nc = 0; nc < DEMO_CONFIG_NUM_CHANNEL_VARIATIONS_TO_TEST; nc++)
            {
                wav_attr.sample_rate = demo_sample_rates_to_test[sr];
                wav_attr.bits_per_sample = demo_bit_depths_to_test[bd];
                wav_attr.num_channels = demo_num_channel_variations_to_test[nc];
                write_demo_wav_file(&wav_attr, DEMO_CONFIG_AUDIO_FILE_LEN_IN_SECONDS);

                MXC_Delay(500000);
            }
        }
    }

    if (sd_card_unmount() != E_NO_ERROR)
    {
        printf("[ERROR]--> SD card unmount\n");
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> SD card unmounted\n");
    }

    printf("\n[SUCCESS]--> All files recorded, shutting down\n");

    bsp_power_off_LDOs();

    // do a slow green blink to indicate success
    const uint32_t slow_blink = 1000000;
    while (1)
    {
        status_led_toggle(STATUS_LED_COLOR_GREEN);
        MXC_Delay(slow_blink);
    }
}

/* Private function definitions --------------------------------------------------------------------------------------*/

void write_demo_wav_file(Wave_Header_Attributes_t *wav_attr, uint32_t file_len_secs)
{
    printf(
        "\n[STARTING]--> %dk %d-bit %d-second %d-channel recording...\n",
        wav_attr->sample_rate / 1000,
        wav_attr->bits_per_sample,
        file_len_secs,
        wav_attr->num_channels);

    // a variable to store the number of bytes written to the SD card, can be checked against the intended amount
    uint32_t bytes_written;

    // a string buffer to write file names into
    char file_name_buff[64];

    // derive the file name from the input parameters
    sprintf(
        file_name_buff,
        "demo_%dkHz_%d_bit_%d_channel.wav",
        wav_attr->sample_rate / 1000,
        wav_attr->bits_per_sample,
        wav_attr->num_channels);

    if (sd_card_fopen(file_name_buff, POSIX_FILE_MODE_WRITE) != E_NO_ERROR)
    {
        printf("[ERROR]--> SD card fopen\n");
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> SD card fopen\n");
    }

    // seek past the wave header, we'll fill it in later after recording the audio, we'll know the file length then
    if (sd_card_lseek(wav_header_get_header_length()) != E_NO_ERROR)
    {
        printf("[ERROR]--> SD card lseek past wav header\n");
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> SD card lseek past wav header\n");
    }

    // there will be some integer truncation here, good enough for this early demo, but improve file-len code eventually
    const uint32_t file_len_in_microsecs = file_len_secs * 1000000;
    const uint32_t num_dma_blocks_in_the_file = file_len_in_microsecs / AUDIO_DMA_CHUNK_READY_PERIOD_IN_MICROSECS;

    decimation_filter_set_sample_rate(wav_attr->sample_rate);

// expanding to q31 means 4 bytes per sample, 2 channels
#define MAX_WORKSPACE_NEEDED_FOR_ENDIAN_SWAP_AND_Q31_EXPANSION (AUDIO_DMA_BUFF_LEN_IN_SAMPS * DATA_CONVERTERS_Q31_SIZE_IN_BYTES * 2)

// tradeoff between memory and current consumption, bigger buffers reduces current by doing fewer, larger, SD writes
#define FULL_BYTE_POOL_SIZE (MAX_WORKSPACE_NEEDED_FOR_ENDIAN_SWAP_AND_Q31_EXPANSION * 3)

    /*
     This is a single pool of memory which we use to do all the processing, we reserve a small potion at the start of
     the array for decimation for the non-384kHz rates. The rest of the pool is used to cram as many processed DMA
     blocks in as we can, and then write them all to the SD card once this pool is as full as it can get.

     For the 384k case, we just use the whole pool for chunk-packing with no reserved portion at the start, since we
     don't need to do any decimation filtering for 384k.
     */
    static uint8_t workspace_byte_pool[FULL_BYTE_POOL_SIZE];

    // 384kHz is treated differently to all the other sample rates, since it does not go through the decimation filter
    const bool its_the_special_case_of_384kHz = (wav_attr->sample_rate == AUDIO_SAMPLE_RATE_384kHz);

    // decimate by 2 for 192k, 4 for 96k, etc
    const uint32_t decimation_factor = AUDIO_SAMPLE_RATE_384kHz / wav_attr->sample_rate;

    // this represents the final sample width that is written to the SD card, not during intermediate steps
    const uint32_t bytes_per_processed_sample = (wav_attr->bits_per_sample / 8);

    // a single SD card write event will consist of one or more processed blocks, most of the time it will be several blocks
    const uint32_t num_bytes_in_one_processed_DMA_block = (AUDIO_DMA_BUFF_LEN_IN_SAMPS / decimation_factor) * bytes_per_processed_sample * wav_attr->num_channels;

    // we don't reserve any bytes in the 384k case, but we reserve a potion for the decimated rates to do the filtering
    const uint32_t num_bytes_reserved_for_decimation = its_the_special_case_of_384kHz ? 0 : (AUDIO_DMA_BUFF_LEN_IN_SAMPS * 4 * wav_attr->num_channels);

    // this portion represents everything except the bytes at the array start reserved for decimation, will be the whole pool for the 384k case
    const uint32_t num_bytes_reserved_for_chunk_packing = FULL_BYTE_POOL_SIZE - num_bytes_reserved_for_decimation;

    // this will have some integer truncation, so there may be some portion of the memory pool wasted for some combos
    const uint32_t num_processed_chunks_we_can_fit_in_the_workspace_buff = num_bytes_reserved_for_chunk_packing / num_bytes_in_one_processed_DMA_block;

    // we start the chunk-packing portion of the memory pool immediately after the portion reserved for decimation (if any, none for 384k)
    uint8_t *start_of_chunk_packing_sector = workspace_byte_pool + num_bytes_reserved_for_decimation;

    uint32_t chunk_packing_idx = 0;
    uint32_t total_num_bytes_in_the_buff = 0;

    ad4630_384kHz_fs_clk_and_cs_start();
    audio_dma_start();

    status_led_set(STATUS_LED_COLOR_GREEN, true); // green led on while recording

    uint32_t num_dma_blocks_consumed = 0;

    while (num_dma_blocks_consumed < num_dma_blocks_in_the_file)
    {
        if (audio_dma_overrun_occured(AUDIO_CHANNEL_0) || audio_dma_overrun_occured(AUDIO_CHANNEL_1))
        {
            printf("[ERROR]--> Audio DMA overrrun\n");
            error_handler(STATUS_LED_COLOR_BLUE);
        }

        if (audio_dma_num_buffers_available(AUDIO_CHANNEL_0) > 0 && audio_dma_num_buffers_available(AUDIO_CHANNEL_1) > 0)
        {
            uint8_t *next_chunk_to_write_to = start_of_chunk_packing_sector + (num_bytes_in_one_processed_DMA_block * chunk_packing_idx);
            chunk_packing_idx++;

            if (wav_attr->num_channels == WAVE_HEADER_STEREO)
            {
                if (its_the_special_case_of_384kHz)
                {
                    if (wav_attr->bits_per_sample == AUDIO_BIT_DEPTH_24_BITS_PER_SAMPLE)
                    {
                        data_converters_interleave_2_i24_and_swap_endianness(
                            audio_dma_consume_buffer(AUDIO_CHANNEL_0),
                            audio_dma_consume_buffer(AUDIO_CHANNEL_1),
                            next_chunk_to_write_to,
                            AUDIO_DMA_BUFF_LEN_IN_SAMPS);
                    }
                    else // it's 384k 16 bits
                    {
                        data_converters_interleave_2_i24_to_q15_and_swap_endianness(
                            audio_dma_consume_buffer(AUDIO_CHANNEL_0),
                            audio_dma_consume_buffer(AUDIO_CHANNEL_1),
                            (q15_t *)next_chunk_to_write_to,
                            AUDIO_DMA_BUFF_LEN_IN_SAMPS);
                    }
                }
                else // it's not the special case of 384kHz, all other sample rates are filtered
                {
                    // we use some space at the start of the big workspace array for filtering
                    // since decimation works with q31's, we need to move the ch1 start up by nsamps * 4 bytes
                    const uint8_t *workspace_for_ch0 = workspace_byte_pool;
                    const uint8_t *workspace_for_ch1 = workspace_byte_pool + (AUDIO_DMA_BUFF_LEN_IN_SAMPS * 4);

                    data_converters_i24_to_q31_with_endian_swap(
                        audio_dma_consume_buffer(AUDIO_CHANNEL_0),
                        (q31_t *)workspace_for_ch0,
                        AUDIO_DMA_BUFF_LEN_IN_SAMPS);

                    data_converters_i24_to_q31_with_endian_swap(
                        audio_dma_consume_buffer(AUDIO_CHANNEL_1),
                        (q31_t *)workspace_for_ch1,
                        AUDIO_DMA_BUFF_LEN_IN_SAMPS);

                    const uint32_t decimated_len_in_samps = decimation_filter_downsample(
                        (q31_t *)workspace_for_ch0,
                        (q31_t *)workspace_for_ch0, // we're decimating "in place", this is ok because the decimated output will always be less than the original input
                        AUDIO_DMA_BUFF_LEN_IN_SAMPS,
                        AUDIO_CHANNEL_0);

                    decimation_filter_downsample( // the len will be the same, we already grabbed it from ch0 above
                        (q31_t *)workspace_for_ch1,
                        (q31_t *)workspace_for_ch1, //  decimating in-place
                        AUDIO_DMA_BUFF_LEN_IN_SAMPS,
                        AUDIO_CHANNEL_1);

                    if (wav_attr->bits_per_sample == AUDIO_BIT_DEPTH_24_BITS_PER_SAMPLE)
                    {
                        data_converters_interleave_2_q31_to_i24(
                            (q31_t *)workspace_for_ch0,
                            (q31_t *)workspace_for_ch1,
                            next_chunk_to_write_to,
                            decimated_len_in_samps);
                    }
                    else // it's one of the decimated sample rates at 16 bits
                    {
                        data_converters_interleave_2_q31_to_q15(
                            (q31_t *)workspace_for_ch0,
                            (q31_t *)workspace_for_ch1,
                            (q15_t *)next_chunk_to_write_to,
                            decimated_len_in_samps);
                    }
                }
            }
            else // num-channels must be MONO
            {
                // hack- consume the buffer we're not using so we don't get overruns, TODO change this
                audio_dma_consume_buffer(DEMO_CONFIG_CHANNEL_TO_USE_FOR_MONO_MODE ? AUDIO_CHANNEL_0 : AUDIO_CHANNEL_1);

                if (its_the_special_case_of_384kHz)
                {
                    if (wav_attr->bits_per_sample == AUDIO_BIT_DEPTH_24_BITS_PER_SAMPLE)
                    {
                        data_converters_i24_swap_endianness(
                            audio_dma_consume_buffer(DEMO_CONFIG_CHANNEL_TO_USE_FOR_MONO_MODE),
                            next_chunk_to_write_to,
                            AUDIO_DMA_BUFF_LEN_IN_SAMPS);
                    }
                    else // it's 384k 16 bits
                    {
                        data_converters_i24_to_q15_with_endian_swap(
                            audio_dma_consume_buffer(DEMO_CONFIG_CHANNEL_TO_USE_FOR_MONO_MODE),
                            (q15_t *)next_chunk_to_write_to,
                            AUDIO_DMA_BUFF_LEN_IN_SAMPS);
                    }
                }
                else // it's not the special case of 384kHz, all other sample rates are filtered
                {
                    // all sample rates other than 384k are filtered, so we need to swap endianness and also expand to 32 bit words as expected by the filters
                    data_converters_i24_to_q31_with_endian_swap(
                        audio_dma_consume_buffer(DEMO_CONFIG_CHANNEL_TO_USE_FOR_MONO_MODE),
                        (q31_t *)workspace_byte_pool,
                        AUDIO_DMA_BUFF_LEN_IN_SAMPS);

                    // workspace_buff starting at index zero now has the endian-swapped and q31-expanded data for this DMA chunk

                    const uint32_t decimated_len_in_samps = decimation_filter_downsample(
                        (q31_t *)workspace_byte_pool,
                        (q31_t *)workspace_byte_pool, // decimating in-place
                        AUDIO_DMA_BUFF_LEN_IN_SAMPS,
                        DEMO_CONFIG_CHANNEL_TO_USE_FOR_MONO_MODE);

                    // workspace_buff starting at index zero now has the decimated version of this DMA chunk

                    if (wav_attr->bits_per_sample == AUDIO_BIT_DEPTH_24_BITS_PER_SAMPLE)
                    {
                        data_converters_q31_to_i24((q31_t *)workspace_byte_pool, next_chunk_to_write_to, decimated_len_in_samps);
                    }
                    else // it's one of the decimated sample rates at 16 bits
                    {
                        data_converters_q31_to_q15((q31_t *)workspace_byte_pool, (q15_t *)next_chunk_to_write_to, decimated_len_in_samps);
                    }
                }
            }

            total_num_bytes_in_the_buff += num_bytes_in_one_processed_DMA_block;

            if (chunk_packing_idx >= num_processed_chunks_we_can_fit_in_the_workspace_buff) // we filled up the chunk-packing sector as much as we can
            {
                if (sd_card_fwrite(start_of_chunk_packing_sector, total_num_bytes_in_the_buff, &bytes_written) != E_NO_ERROR)
                {
                    error_handler(STATUS_LED_COLOR_RED);
                }

                chunk_packing_idx = 0;
                total_num_bytes_in_the_buff = 0;
            }

            num_dma_blocks_consumed += 1;
        }
    }

    // there may be some data left in the chunk-packing buffer, if we don't write this out the file will be a little shorter than expected
    if (total_num_bytes_in_the_buff > 0)
    {
        if (sd_card_fwrite(start_of_chunk_packing_sector, total_num_bytes_in_the_buff, &bytes_written) != E_NO_ERROR)
        {
            error_handler(STATUS_LED_COLOR_RED);
        }
    }

    ad4630_384kHz_fs_clk_and_cs_stop();
    audio_dma_stop();

    // back to the top of the file so we can write the wav header now that we can determine the size of the file
    if (sd_card_lseek(0) != E_NO_ERROR)
    {
        printf("[ERROR]--> SD card lseek to top of file\n");
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> SD card lseek to top of file\n");
    }

    wav_attr->file_length = sd_card_fsize();
    wav_header_set_attributes(wav_attr);

    if (sd_card_fwrite(wav_header_get_header(), wav_header_get_header_length(), &bytes_written) != E_NO_ERROR)
    {
        printf("[ERROR]--> SD card WAV header fwrite\n");
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> SD card WAV header fwrite\n");
    }

    if (sd_card_fclose() != E_NO_ERROR)
    {
        printf("[ERROR]--> SD card fclose\n");
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> SD card fclose\n");
    }

    printf("[SUCCESS]--> Wrote file %s\n", file_name_buff);

    status_led_set(STATUS_LED_COLOR_GREEN, false); // green led off after recording is complete
}

void error_handler(Status_LED_Color_t color)
{
    status_led_all_off();

    const uint32_t fast_blink = 100000;
    while (true)
    {
        status_led_toggle(color);
        MXC_Delay(fast_blink);
    }
}
