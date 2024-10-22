
/* Private includes --------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdint.h>

#include "mxc_delay.h"
#include "tmr.h"

#include "ad4630.h"
#include "audio_dma.h"
#include "board.h"
#include "bsp_i2c.h"
#include "bsp_spi.h"
#include "bsp_status_led.h"
#include "data_converters.h"
#include "decimation_filter.h"
#include "demo_config.h"
#include "sd_card.h"
#include "sd_card_bank_ctl.h"
#include "wav_header.h"

#include <string.h>
#include "SEGGER_RTT.h"

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
    
    
    printf("*********** CLI ADC write Example ***********\n\n");
    

    bsp_power_on_LDOs();

    if (ad4630_init() != AD4630_ERROR_ALL_OK)
    {
        printf("[ERROR]--> AD4630 init\n");
        error_handler(STATUS_LED_COLOR_BLUE);
    }
    else
    {
        printf("[SUCCESS]--> AD4630 init\n");
    }

    if (audio_dma_init() != AUDIO_DMA_ERROR_ALL_OK)
    {
        printf("[ERROR]--> DMA init\n");
        error_handler(STATUS_LED_COLOR_BLUE);
    }
    else
    {
        printf("[SUCCESS]--> DMA init\n");
    }

    
    printf("[Success]-->Decimation\n");
    ad4630_cont_conversions_start();
    printf("[Success]-->ADC converison\n");    
    audio_dma_start();

    if (bsp_3v3_i2c_init() != E_NO_ERROR)
    {
        printf("[ERROR]--> I2C init\n");
    }
    else
    {
        printf("[SUCCESS]--> I2C init\n");
    }

    if (sd_card_bank_ctl_init() != SD_CARD_BANK_CTL_ERROR_ALL_OK)
    {
        printf("[ERROR]--> SD card bank ctl init\n");
    }
    else
    {
        printf("[SUCCESS]--> SD card bank ctl init\n");
    }

    sd_card_bank_ctl_enable_slot(0);

    sd_card_bank_ctl_read_and_cache_detect_pins();

    if (!sd_card_bank_ctl_active_card_is_inserted())
    {
        printf("[ERROR]--> Card at slot 0 not inserted\n");
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> SD card inserted in slot 0\n");
    }

    if (sd_card_init() != SD_CARD_ERROR_ALL_OK)
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

    if (sd_card_mount() != SD_CARD_ERROR_ALL_OK)
    {
        printf("[ERROR]--> SD card mount\n");
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> SD card mounted\n");
    }

    Wave_Header_Attributes_t wav_attr = {
        .num_channels = WAVE_HEADER_MONO, // only mono is supported for now, 2 channel might be added later
    };

    for (uint32_t sr = 0; sr < DEMO_CONFIG_NUM_SAMPLE_RATES_TO_TEST; sr++)
    {
        for (uint32_t bd = 0; bd < DEMO_CONFIG_NUM_BIT_DEPTHS_TO_TEST; bd++)
        {
            wav_attr.sample_rate = demo_sample_rates_to_test[sr];
            wav_attr.bits_per_sample = demo_bit_depths_to_test[bd];
            write_demo_wav_file(&wav_attr, DEMO_CONFIG_AUDIO_FILE_LEN_IN_SECONDS);

            MXC_Delay(500000);
        }
    }

    if (sd_card_unmount() != SD_CARD_ERROR_ALL_OK)
    {
        printf("[ERROR]--> SD card unmount\n");
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> SD card unmounted\n");
    }

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
    printf("[STARTING]--> %dk %d-bit recording\n", wav_attr->sample_rate / 1000, wav_attr->bits_per_sample);

    // a buffers for processing the audio data, big enough to fit one full buffers worth of samples as q31s
    static uint8_t audio_buff_0[AUDIO_DMA_BUFF_LEN_IN_SAMPS * 4];
    static uint8_t audio_buff_1[AUDIO_DMA_BUFF_LEN_IN_SAMPS * 4];

    // a variable to store the number of bytes written to the SD card, can be checked against the intended amount
    static uint32_t bytes_written;

    // a string buffer to write file names into
    static char file_name_buff[64];

    // there will be some integer truncation here, good enough for this early demo, but improve file-len code eventually
    const uint32_t file_len_in_microsecs = file_len_secs * 1000000;
    const uint32_t num_dma_blocks_in_the_file = file_len_in_microsecs / AUDIO_DMA_CHUNK_READY_PERIOD_IN_MICROSECS;

    // derive the file name from the input parameters
    sprintf(file_name_buff, "demo_%dkHz_%d_bit.wav", wav_attr->sample_rate / 1000, wav_attr->bits_per_sample);

    if (sd_card_fopen(file_name_buff, POSIX_FILE_MODE_WRITE) != SD_CARD_ERROR_ALL_OK)
    {
        printf("[ERROR]--> SD card fopen\n");
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> SD card fopen\n");
    }

    // seek past the wave header, we'll fill it in later after recording the audio, we'll know the file length then
    if (sd_card_lseek(wav_header_get_header_length()) != SD_CARD_ERROR_ALL_OK)
    {
        printf("[ERROR]--> SD card lseek\n");
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> SD card lseek\n");
    }

  

    decimation_filter_set_sample_rate(wav_attr->sample_rate);
    printf("[Success]-->Decimation\n");
    ad4630_cont_conversions_start();
    printf("[Success]-->ADC converison\n");
    
    audio_dma_start();
    printf("[Success]-->DMA\n");

    for (uint32_t num_dma_blocks_written = 0; num_dma_blocks_written < num_dma_blocks_in_the_file;)
    {
        if (audio_dma_overrun_occured())
        {
            printf("[ERROR]--> Audio DMA overrrun\n");
            error_handler(STATUS_LED_COLOR_BLUE);
        }

        while (audio_dma_num_buffers_available() > 0)
        {
            if (wav_attr->sample_rate == WAVE_HEADER_SAMPLE_RATE_384kHz)
            {
                // for 384kHz data, we just need to swap the endianness of the sample to little-endian format needed for WAV
                data_converters_i24_swap_endianness(audio_dma_consume_buffer(), audio_buff_0, AUDIO_DMA_BUFF_LEN_IN_BYTES);

                // if (wav_attr->bits_per_sample == WAVE_HEADER_24_BITS_PER_SAMPLE)
                // {
                //     // if (sd_card_fwrite(audio_buff_0, AUDIO_DMA_BUFF_LEN_IN_BYTES, &bytes_written) != SD_CARD_ERROR_ALL_OK)
                //     // {
                //     //     printf("[ERROR]--> SD card fwrite\n");
                //     //     error_handler(STATUS_LED_COLOR_RED);
                //     // }
                // }
                // else // it must be 16 bits
                // {
                //     const uint32_t len = data_converters_i24_to_q15(audio_buff_0, (q15_t *)audio_buff_0, AUDIO_DMA_BUFF_LEN_IN_BYTES);

                //     if (sd_card_fwrite(audio_buff_0, len, &bytes_written) != SD_CARD_ERROR_ALL_OK)
                //     {
                //         printf("[ERROR]--> SD card fwrite\n");
                //         error_handler(STATUS_LED_COLOR_RED);
                //     }
                // }
            }
            else // it's not the special case of 384kHz, all other sample rates are filtered
            {
                // all sample rates other than 384k are filtered, so we need to swap endianness and also expand to 32 bit words as expected by the filters
                data_converters_i24_to_q31_with_endian_swap(audio_dma_consume_buffer(), (q31_t *)audio_buff_0, AUDIO_DMA_BUFF_LEN_IN_BYTES);

                const uint32_t len_in_samps = decimation_filter_downsample(
                    (q31_t *)audio_buff_0,
                    (q31_t *)audio_buff_1,
                    AUDIO_DMA_BUFF_LEN_IN_SAMPS); // we want num samples, not num bytes

                // note that the data conversion functions for truncating down to 16 and 24 bits can work in-place
                // uint32_t len_in_bytes;
                // if (wav_attr->bits_per_sample == WAVE_HEADER_24_BITS_PER_SAMPLE)
                // {
                //     len_in_bytes = data_converters_q31_to_i24((q31_t *)audio_buff_1, audio_buff_1, len_in_samps);
                // }
                // else // it's 16 bits
                // {
                //     len_in_bytes = data_converters_q31_to_q15((q31_t *)audio_buff_1, (q15_t *)audio_buff_1, len_in_samps);
                // }

                // if (sd_card_fwrite(audio_buff_1, len_in_bytes, &bytes_written) != SD_CARD_ERROR_ALL_OK)
                // {
                //     error_handler(STATUS_LED_COLOR_RED);
                // }
            }

        //     num_dma_blocks_written += 1;
        // }

        //  while (audio_dma_num_buffers_available() < num_dma_blocks_in_the_file)
        // {
        //     if (wav_attr->sample_rate == WAVE_HEADER_SAMPLE_RATE_384kHz)
        //     {
        //         // for 384kHz data, we just need to swap the endianness of the sample to little-endian format needed for WAV
        //         data_converters_i24_swap_endianness(audio_dma_consume_buffer(), audio_buff_0, AUDIO_DMA_BUFF_LEN_IN_BYTES);

        //         if (wav_attr->bits_per_sample == WAVE_HEADER_24_BITS_PER_SAMPLE)
        //         {
        //             if (sd_card_fwrite(audio_buff_0, AUDIO_DMA_BUFF_LEN_IN_BYTES, &bytes_written) != SD_CARD_ERROR_ALL_OK)
        //             {
        //                 printf("[ERROR]--> SD card fwrite\n");
        //                 error_handler(STATUS_LED_COLOR_RED);
        //             }
        //         }
        //         else // it must be 16 bits
        //         {
        //             const uint32_t len = data_converters_i24_to_q15(audio_buff_0, (q15_t *)audio_buff_0, AUDIO_DMA_BUFF_LEN_IN_BYTES);

        //             if (sd_card_fwrite(audio_buff_0, len, &bytes_written) != SD_CARD_ERROR_ALL_OK)
        //             {
        //                 printf("[ERROR]--> SD card fwrite\n");
        //                 error_handler(STATUS_LED_COLOR_RED);
        //             }
        //         }
        //     }
        //     else // it's not the special case of 384kHz, all other sample rates are filtered
        //     {
        //         // all sample rates other than 384k are filtered, so we need to swap endianness and also expand to 32 bit words as expected by the filters
        //         data_converters_i24_to_q31_with_endian_swap(audio_dma_consume_buffer(), (q31_t *)audio_buff_0, AUDIO_DMA_BUFF_LEN_IN_BYTES);

        //         const uint32_t len_in_samps = decimation_filter_downsample(
        //             (q31_t *)audio_buff_0,
        //             (q31_t *)audio_buff_1,
        //             AUDIO_DMA_BUFF_LEN_IN_SAMPS); // we want num samples, not num bytes

        //         // note that the data conversion functions for truncating down to 16 and 24 bits can work in-place
        //         uint32_t len_in_bytes;
        //         if (wav_attr->bits_per_sample == WAVE_HEADER_24_BITS_PER_SAMPLE)
        //         {
        //             len_in_bytes = data_converters_q31_to_i24((q31_t *)audio_buff_1, audio_buff_1, len_in_samps);
        //         }
        //         else // it's 16 bits
        //         {
        //             len_in_bytes = data_converters_q31_to_q15((q31_t *)audio_buff_1, (q15_t *)audio_buff_1, len_in_samps);
        //         }

        //         if (sd_card_fwrite(audio_buff_1, len_in_bytes, &bytes_written) != SD_CARD_ERROR_ALL_OK)
        //         {
        //             error_handler(STATUS_LED_COLOR_RED);
        //         }
        //     }

        //     num_dma_blocks_written += 1;
         }
    }

    printf("[SUCCESS]--> Wrote file %s\n", file_name_buff);

    ad4630_cont_conversions_stop();
    audio_dma_stop();

    // back to the top of the file so we can write the wav header now that we can determine the size of the file
    if (sd_card_lseek(0) != SD_CARD_ERROR_ALL_OK)
    {
        error_handler(STATUS_LED_COLOR_RED);
    }

    wav_attr->file_length = sd_card_fsize();
    wav_header_set_attributes(wav_attr);

    if (sd_card_fwrite(wav_header_get_header(), wav_header_get_header_length(), &bytes_written) != SD_CARD_ERROR_ALL_OK)
    {
        error_handler(STATUS_LED_COLOR_RED);
    }

    if (sd_card_fclose() != SD_CARD_ERROR_ALL_OK)
    {
        error_handler(STATUS_LED_COLOR_RED);
    }
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
