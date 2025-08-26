
/* Private includes --------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "mxc_delay.h"

#include "ad4630.h"
#include "afe_control.h"
#include "audio_dma.h"
#include "board.h"
#include "bsp_i2c.h"
#include "bsp_pins.h"
#include "bsp_pushbutton.h"
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

/* Private variables */
static uint32_t file_count = 0; // Keep track of file count since turn on

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

// LED light pattern
static void LED_cascade_right(void);
static void LED_cascade_left(void);

// system control functions
static void initialize(void);
static void power_on_audio_chain(void);
static void power_off_audio_chain(void);

static void start_recording(Wave_Header_Sample_Rate_t rate, Wave_Header_Bits_Per_Sample_t bit, SD_Card_Bank_Card_Slot_t sd_slot, int32_t duration_s);

/* Public function definitions ---------------------------------------------------------------------------------------*/

int main(void)
{
#ifdef TERMINAL_IO_USE_CONSOLE_UART
    bsp_console_uart_init();
#endif

    LED_cascade_right();

    initialize();

    bool isButtonPushed = false;

    // do a slow green blink to indicate success
    const uint32_t start_blink_rate = MXC_DELAY_MSEC(50);
    const u_int32_t idle_blink_interval = 20; // Blink blue every 2 seconds, 100ms per loop
    uint32_t loopCount = 0;

    while (1)
    {

        if (loopCount == idle_blink_interval)
        {
            loopCount = 0;
            status_led_set(STATUS_LED_COLOR_BLUE, TRUE);
            MXC_Delay(MXC_DELAY_MSEC(50));
            status_led_set(STATUS_LED_COLOR_BLUE, FALSE);
        }
        // check button push every 100ms
        MXC_Delay(MXC_DELAY_MSEC(100));
        loopCount++;
        if (user_pushbutton_state() == (BUTTON_STATE_JUST_PRESSED || BUTTON_STATE_PRESSED) && !isButtonPushed)
        {
            isButtonPushed = true; // blocking anymore button push during processing. Doesn't matter here since there's no parallel task.

            status_led_set(STATUS_LED_COLOR_GREEN, TRUE);
            MXC_Delay(MXC_DELAY_MSEC(1000));
            status_led_set(STATUS_LED_COLOR_GREEN, FALSE);

            // No longer uses the demo_config.h values except for the Gain
            start_recording(WAVE_HEADER_SAMPLE_RATE_384kHz,
                            WAVE_HEADER_24_BITS_PER_SAMPLE,
                            SD_CARD_BANK_CARD_SLOT_0,
                            5);

            isButtonPushed = false;
        }
    }
}

static void LED_cascade_right(void) // R->G->B
{
    status_led_all_off();
    for (size_t i = 0; i < 3; i++)
    {
        // Turn each color LED on
        status_led_set(i, TRUE);
        MXC_Delay(MXC_DELAY_MSEC(250));
    }
    MXC_Delay(MXC_DELAY_SEC(1));
    status_led_all_off();
}

static void LED_cascade_left(void) // R<-G<-B
{
    status_led_all_off();
    for (size_t i = 3; i > 0; i--)
    {
        // Turn each color LED on
        status_led_set(i - 1, TRUE);
        MXC_Delay(MXC_DELAY_MSEC(250));
    }
    MXC_Delay(MXC_DELAY_SEC(1));
    status_led_all_off();
}

static void initialize(void)
{
    // initialize push button
    pushbuttons_init();

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

    printf("\n*********** ADC/DMA Wave File Write Example ***********\n\n");
}

static void power_on_audio_chain(void)
{
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

    if (afe_control_init() != E_NO_ERROR)
    {
        printf("[ERROR]--> AFE Control init\n");
        error_handler(STATUS_LED_COLOR_GREEN);
    }
    else
    {
        printf("[SUCCESS]--> AFE Control init\n");
    }

    if (afe_control_enable(AFE_CONTROL_CHANNEL_0) != E_NO_ERROR)
    {
        printf("[ERROR]--> AFE Control CH0 EN\n");
        error_handler(STATUS_LED_COLOR_GREEN);
    }
    else
    {
        printf("[SUCCESS]--> AFE Control CH0 EN\n");
    }

    if (afe_control_set_gain(AFE_CONTROL_CHANNEL_0, DEMO_CONFIG_AUDIO_GAIN) != E_NO_ERROR)
    {
        printf("[ERROR]--> AFE Control CH0 gain set to %ddB\n", DEMO_CONFIG_AUDIO_GAIN);
        error_handler(STATUS_LED_COLOR_GREEN);
    }
    else
    {
        printf("[SUCCESS]--> AFE Control CH0 gain set to %ddB\n", DEMO_CONFIG_AUDIO_GAIN);
    }

    const AFE_Control_Gain_t readback_gain = afe_control_get_gain(AFE_CONTROL_CHANNEL_0);
    if (readback_gain != DEMO_CONFIG_AUDIO_GAIN)
    {
        printf("[ERROR]--> AFE set (%ddB) and get (%ddB) gain don't match\n", DEMO_CONFIG_AUDIO_GAIN, readback_gain);
        error_handler(STATUS_LED_COLOR_GREEN);
    }
    else
    {
        printf("[SUCCESS]--> AFE get-gain matches AFE set-gain\n");
    }
}

static void power_off_audio_chain(void)
{

    if (afe_control_disable(AFE_CONTROL_CHANNEL_0) != E_NO_ERROR)
    {
        printf("[ERROR]--> AFE Control CH0 DIS\n");
        error_handler(STATUS_LED_COLOR_GREEN);
    }
    else
    {
        printf("[SUCCESS]--> AFE Control CH0 DIS\n");
    }

    bsp_power_off_LDOs();
}

static void start_recording(Wave_Header_Sample_Rate_t rate, Wave_Header_Bits_Per_Sample_t bit, SD_Card_Bank_Card_Slot_t sd_slot, int32_t duration_s)
{
    power_on_audio_chain();

    if (sd_card_bank_ctl_init() != E_NO_ERROR)
    {
        printf("[ERROR]--> SD card bank ctl init\n");
        error_handler(STATUS_LED_COLOR_GREEN);
    }
    else
    {
        printf("[SUCCESS]--> SD card bank ctl init\n");
    }

    sd_card_bank_ctl_enable_slot(sd_slot);

    sd_card_bank_ctl_read_and_cache_detect_pins();

    if (!sd_card_bank_ctl_active_card_is_inserted())
    {
        printf("[ERROR]--> Card at slot %d not inserted\n", sd_slot);
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> SD card inserted in slot %d\n", sd_slot);
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

    Wave_Header_Attributes_t wav_attr = {
        .num_channels = WAVE_HEADER_MONO, // only mono is supported for now, 2 channel might be added later
    };

    wav_attr.sample_rate = rate;
    wav_attr.bits_per_sample = bit;
    write_demo_wav_file(&wav_attr, duration_s);

    MXC_Delay(500000);

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

    power_off_audio_chain();
}

/* Private function definitions --------------------------------------------------------------------------------------*/

void write_demo_wav_file(Wave_Header_Attributes_t *wav_attr, uint32_t file_len_secs)
{
    printf("\n[STARTING]--> %dk %d-bit %d second recording...\n", wav_attr->sample_rate / 1000, wav_attr->bits_per_sample, file_len_secs);

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
    sprintf(file_name_buff, "Magpie_%dkHz_%d_bit_%03d.wav", wav_attr->sample_rate / 1000, wav_attr->bits_per_sample, file_count);

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

    decimation_filter_set_sample_rate(wav_attr->sample_rate);

    ad4630_384kHz_fs_clk_and_cs_start();
    audio_dma_start();

    status_led_set(STATUS_LED_COLOR_GREEN, true); // green led on while recording

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
                if (wav_attr->bits_per_sample == WAVE_HEADER_24_BITS_PER_SAMPLE)
                {
                    const uint32_t len = data_converters_i24_swap_endianness(audio_dma_consume_buffer(), audio_buff_0, AUDIO_DMA_BUFF_LEN_IN_SAMPS);

                    if (sd_card_fwrite(audio_buff_0, len, &bytes_written) != E_NO_ERROR)
                    {
                        printf("[ERROR]--> SD card fwrite\n");
                        error_handler(STATUS_LED_COLOR_RED);
                    }
                }
                else // it must be 16 bits
                {
                    const uint32_t len = data_converters_i24_to_q15_with_endian_swap(audio_dma_consume_buffer(), (q15_t *)audio_buff_0, AUDIO_DMA_BUFF_LEN_IN_SAMPS);

                    if (sd_card_fwrite(audio_buff_0, len, &bytes_written) != E_NO_ERROR)
                    {
                        printf("[ERROR]--> SD card fwrite\n");
                        error_handler(STATUS_LED_COLOR_RED);
                    }
                }
            }
            else // it's not the special case of 384kHz, all other sample rates are filtered
            {
                // all sample rates other than 384k are filtered, so we need to swap endianness and also expand to 32 bit words as expected by the filters
                data_converters_i24_to_q31_with_endian_swap(audio_dma_consume_buffer(), (q31_t *)audio_buff_0, AUDIO_DMA_BUFF_LEN_IN_SAMPS);

                const uint32_t len_in_samps = decimation_filter_downsample(
                    (q31_t *)audio_buff_0,
                    (q31_t *)audio_buff_1,
                    AUDIO_DMA_BUFF_LEN_IN_SAMPS); // we want num samples, not num bytes

                // note that the data conversion functions for truncating down to 16 and 24 bits can work in-place
                uint32_t len_in_bytes;
                if (wav_attr->bits_per_sample == WAVE_HEADER_24_BITS_PER_SAMPLE)
                {
                    len_in_bytes = data_converters_q31_to_i24((q31_t *)audio_buff_1, audio_buff_1, len_in_samps);
                }
                else // it's 16 bits
                {
                    len_in_bytes = data_converters_q31_to_q15((q31_t *)audio_buff_1, (q15_t *)audio_buff_1, len_in_samps);
                }

                if (sd_card_fwrite(audio_buff_1, len_in_bytes, &bytes_written) != E_NO_ERROR)
                {
                    error_handler(STATUS_LED_COLOR_RED);
                }
            }

            if (num_dma_blocks_written % 30 == 0)
            {
                status_led_toggle(STATUS_LED_COLOR_GREEN);
            }

            num_dma_blocks_written += 1;
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

    file_count++; // increment file count for the next write

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