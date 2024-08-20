/**
 * @file    main.c
 * @brief   Mock audio SD card writes
 * @details This example writes mock audio to the SD card.
 */

/* Includes ----------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdint.h>
#include "mxc_device.h"
#include "led.h"
#include "board.h"
#include "mxc_delay.h"

#include "demo_config.h"
#include "mock_audio.h"
#include "sd_card.h"
#include "wav_header.h"

/* Defines -----------------------------------------------------------------------------------------------------------*/

#define BYTES_PER_AUDIO_BUFF_SAMPLE (DEMO_CONFIG_BITS_PER_SAMPLE / 8)

// there will be some integer truncation in the following calculations, aiming for "good enough for rock and roll" here.
#define AUDIO_BUFF_LEN_IN_BYTES (DEMO_CONFIG_AUDIO_BUFF_LEN_IN_SAMPS * BYTES_PER_AUDIO_BUFF_SAMPLE)
#define AUDIO_BUFF_DURATION_MILLISECONDS ((DEMO_CONFIG_AUDIO_BUFF_LEN_IN_SAMPS * 1000) / DEMO_CONFIG_SAMPLE_RATE_HZ)

#define AUDIO_FILE_LENGTH_MILLISECONDS (DEMO_CONFIG_AUDIO_FILE_LENGTH_MINUTES * 60 * 1000)

// x2 is because this demo writes 2 channels
#define NUM_FULL_BUFFERS_IN_WHOLE_FILE ((AUDIO_FILE_LENGTH_MILLISECONDS / AUDIO_BUFF_DURATION_MILLISECONDS) * 2)

/* Types -------------------------------------------------------------------------------------------------------------*/

typedef enum
{
    LED_COLOR_RED = 0,
    LED_COLOR_BLUE = 1,
    LED_COLOR_GREEN = 2
} LED_Color_t;

/* Variables ---------------------------------------------------------------------------------------------------------*/

// the buffer to write mock audio data into, doubled for 2 channels
int16_t audio_buff[DEMO_CONFIG_AUDIO_BUFF_LEN_IN_SAMPS * 2];

// variable to store the number of bytes written to the SD card during write operations
uint32_t bytes_written;

/* Helper function declarations --------------------------------------------------------------------------------------*/

// the error handler simply rapidly blinks the given LED color forever
void error_handler(LED_Color_t color);

/* Function definitions ----------------------------------------------------------------------------------------------*/

int main(void)
{
    // set up the pins for code timing tasks
    const mxc_gpio_cfg_t file_write_timing_pin = {
        .port = MXC_GPIO0,
        .mask = MXC_GPIO_PIN_5,
        .pad = MXC_GPIO_PAD_NONE,
        .func = MXC_GPIO_FUNC_OUT,
        .vssel = MXC_GPIO_VSSEL_VDDIO,
        .drvstr = MXC_GPIO_DRVSTR_2,
    };
    MXC_GPIO_Config(&file_write_timing_pin);

    const mxc_gpio_cfg_t buff_write_timing_pin = {
        .port = MXC_GPIO0,
        .mask = MXC_GPIO_PIN_12,
        .pad = MXC_GPIO_PAD_NONE,
        .func = MXC_GPIO_FUNC_OUT,
        .vssel = MXC_GPIO_VSSEL_VDDIO,
        .drvstr = MXC_GPIO_DRVSTR_2,
    };
    MXC_GPIO_Config(&buff_write_timing_pin);

    // set up the mock sine wave
    mock_audio_sine_init();
    mock_audio_set_sample_rate(DEMO_CONFIG_SAMPLE_RATE_HZ);

    mock_audio_set_sine_freq_A(DEMO_CONFIG_SINE_FREQUENCY_HZ);
    mock_audio_set_sine_freq_B(DEMO_CONFIG_SINE_FREQUENCY_HZ * 2); // make the 2nd channel freq different so we can tell them apart in the wav file

    // get the SD card ready to write
    if (sd_card_init() != SD_CARD_ERROR_ALL_OK)
    {
        error_handler(LED_COLOR_RED);
    }

    if (sd_card_mount() != SD_CARD_ERROR_ALL_OK)
    {
        error_handler(LED_COLOR_RED);
    }

    if (sd_card_fopen("fake_wav.wav", POSIX_FILE_MODE_WRITE) != SD_CARD_ERROR_ALL_OK)
    {
        error_handler(LED_COLOR_BLUE);
    }

    if (sd_card_lseek(wav_header_get_header_length()) != SD_CARD_ERROR_ALL_OK)
    {
        error_handler(LED_COLOR_BLUE);
    }

    // write all the audio data
    MXC_GPIO_OutSet(file_write_timing_pin.port, file_write_timing_pin.mask); // whole file timing
    for (uint32_t buff = 0; buff < NUM_FULL_BUFFERS_IN_WHOLE_FILE; buff++)
    {
        for (uint32_t samp = 0; samp < DEMO_CONFIG_AUDIO_BUFF_LEN_IN_SAMPS; samp += 2)
        {
            const int16_t left_sample = mock_audio_sine_tick_A();
            const int16_t right_sample = mock_audio_sine_tick_B();

            audio_buff[samp] = left_sample;
            audio_buff[samp + 1] = right_sample;
        }

        MXC_GPIO_OutSet(buff_write_timing_pin.port, buff_write_timing_pin.mask); // single buffer timing
        if (sd_card_fwrite(audio_buff, AUDIO_BUFF_LEN_IN_BYTES, &bytes_written) != SD_CARD_ERROR_ALL_OK)
        {
            error_handler(LED_COLOR_GREEN);
        }
        MXC_GPIO_OutClr(buff_write_timing_pin.port, buff_write_timing_pin.mask); // single buffer timing
    }

    // calculate the total size of the file and write the header
    const uint32_t file_size = sd_card_fsize();

    Wave_Header_Attributes_t wav_attr = {
        .num_channels = WAVE_HEADER_STEREO,
        .bits_per_sample = DEMO_CONFIG_BITS_PER_SAMPLE,
        .sample_rate = DEMO_CONFIG_SAMPLE_RATE_HZ,
        .file_length = file_size,
    };
    wav_header_set_attributes(&wav_attr);

    if (sd_card_lseek(0) != SD_CARD_ERROR_ALL_OK)
    {
        error_handler(LED_COLOR_BLUE);
    }

    if (sd_card_fwrite(wav_header_get_header(), wav_header_get_header_length(), &bytes_written) != SD_CARD_ERROR_ALL_OK)
    {
        error_handler(LED_COLOR_GREEN);
    }

    // shut down the SD card
    if (sd_card_fclose() != SD_CARD_ERROR_ALL_OK)
    {
        error_handler(LED_COLOR_BLUE);
    }

    MXC_GPIO_OutClr(file_write_timing_pin.port, file_write_timing_pin.mask); // whole file timing

    if (sd_card_unmount() != SD_CARD_ERROR_ALL_OK)
    {
        error_handler(LED_COLOR_RED);
    }

    // if we get here, then the file write was successful
    const uint32_t slow_blink = 1000000;
    while (1)
    {
        LED_On(LED_COLOR_GREEN);
        MXC_Delay(slow_blink);
        LED_Off(LED_COLOR_GREEN);
        MXC_Delay(slow_blink);
    }
}

void error_handler(LED_Color_t color)
{
    const uint32_t fast_blink = 100000;
    while (true)
    {
        LED_On(color);
        MXC_Delay(fast_blink);
        LED_Off(color);
        MXC_Delay(fast_blink);
    }
}
