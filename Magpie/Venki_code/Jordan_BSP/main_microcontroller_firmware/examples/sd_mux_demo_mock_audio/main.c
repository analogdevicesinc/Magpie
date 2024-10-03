/**
 * @file    main.c
 * @brief   SD card bank control demo.
 * @details This example writes files to each of the 6 SD cards in the SD card bank.
 */


/* Private includes --------------------------------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "mxc_device.h"
#include "mxc_delay.h"

#include "bsp_i2c.h"
#include "bsp_status_led.h"
#include "sd_card.h"
#include "sd_card_bank_ctl.h"
#include "SEGGER_RTT.h"
#include "demo_config.h"
#include "mock_audio.h"

#include "wav_header.h"

/* Defines -----------------------------------------------------------------------------------------------------------*/
// This is to add mock audio file
#define BYTES_PER_AUDIO_BUFF_SAMPLE (DEMO_CONFIG_BITS_PER_SAMPLE / 8)

// there will be some integer truncation in the following calculations, aiming for "good enough for rock and roll" here.
#define AUDIO_BUFF_LEN_IN_BYTES (DEMO_CONFIG_AUDIO_BUFF_LEN_IN_SAMPS * BYTES_PER_AUDIO_BUFF_SAMPLE)
#define AUDIO_BUFF_DURATION_MILLISECONDS ((DEMO_CONFIG_AUDIO_BUFF_LEN_IN_SAMPS * 1000) / DEMO_CONFIG_SAMPLE_RATE_HZ)

#define AUDIO_FILE_LENGTH_MILLISECONDS (DEMO_CONFIG_AUDIO_FILE_LENGTH_MINUTES * 60 * 1000)

#define NUM_FULL_BUFFERS_IN_WHOLE_FILE (AUDIO_FILE_LENGTH_MILLISECONDS / AUDIO_BUFF_DURATION_MILLISECONDS)

/* Types -------------------------------------------------------------------------------------------------------------*/
// For mock audio

typedef enum
{
    LED_COLOR_RED = 0,
    LED_COLOR_BLUE = 1,
    LED_COLOR_GREEN = 2

} LED_Color_t;

/* Variables ---------------------------------------------------------------------------------------------------------*/

// the buffer to write mock audio data into
int16_t audio_buff[DEMO_CONFIG_AUDIO_BUFF_LEN_IN_SAMPS]; // For mock audio file
// variable to store the number of bytes written to the SD card during write operations
uint32_t bytes_written;

/* Variables for the break out board---------------------------------------------------------------------------------------------------------*/
// a buffer for writing strings into
static char str_buff[128];

// keep track of how many successes and errors there were so we can blink a pattern at the end showing this information.
static uint8_t num_successful_cards = 0;
static uint8_t num_missing_cards = 0;
static uint8_t num_cards_with_mount_errors = 0;
static uint8_t num_cards_with_file_errors = 0;

/* Private function declarations -------------------------------------------------------------------------------------*/

/**
 * `blink_n_times(c, n)` blinks the LED color `c` on and off `n` times with a short pause between blinks.
 *
 * The given LED color is always forced OFF at the end of this function.
 */
static void blink_n_times(Status_LED_Color_t c, uint8_t n);

/* Public function definitions ---------------------------------------------------------------------------------------*/

int main(void)
{
    printf("\n******************* SD Card Bank Example ******************* \n");

  //  =====================================================================================================
 //   This block of code conflicts with the actual bsp ADC pins so do not use in final version
  //  set up the pins for code timing tasks
    const mxc_gpio_cfg_t file_write_timing_pin = {
        .port = MXC_GPIO1,
        .mask = MXC_GPIO_PIN_6,
        .pad = MXC_GPIO_PAD_NONE,
        .func = MXC_GPIO_FUNC_OUT,
        .vssel = MXC_GPIO_VSSEL_VDDIO,
        .drvstr = MXC_GPIO_DRVSTR_2,
    };
    MXC_GPIO_Config(&file_write_timing_pin);

    const mxc_gpio_cfg_t buff_write_timing_pin = {
        .port = MXC_GPIO1,
        .mask = MXC_GPIO_PIN_8,
        .pad = MXC_GPIO_PAD_NONE,
        .func = MXC_GPIO_FUNC_OUT,
        .vssel = MXC_GPIO_VSSEL_VDDIO,
        .drvstr = MXC_GPIO_DRVSTR_2,
    };
    MXC_GPIO_Config(&buff_write_timing_pin);
   // ==================================================================================================

    // set up the mock sine wave
    mock_audio_sine_init();
    mock_audio_set_sample_rate(DEMO_CONFIG_SAMPLE_RATE_HZ);
    mock_audio_set_sine_freq(DEMO_CONFIG_SINE_FREQUENCY_HZ);
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

    for (int slot = 0; slot < SD_CARD_BANK_CTL_NUM_CARDS; slot++)
    {
        printf("[CHECKING]--> SD card %d\n", slot);

        // this delay is just to slow it down so a human can see what's happening
        MXC_Delay(1000000);

        sd_card_bank_ctl_enable_slot(slot);

        sd_card_bank_ctl_read_and_cache_detect_pins();

        if (!sd_card_bank_ctl_active_card_is_inserted())
        {
            printf("[ERROR]--> Card %d not inserted\n", slot);

            num_missing_cards += 1;
            sd_card_bank_ctl_disable_all();
            continue;
        }

        // initialize and mount the card
        if (sd_card_init() != SD_CARD_ERROR_ALL_OK)
        {
            printf("[ERROR]--> Card %d init failed\n", slot);

            num_cards_with_mount_errors += 1;
            sd_card_bank_ctl_disable_all();
            blink_n_times(STATUS_LED_COLOR_RED, 10);
            continue;
        }

        // without a brief delay between card init and mount, there are often mount errors
        MXC_Delay(100000);
        printf("error error\r\n");

        if (sd_card_mount() != SD_CARD_ERROR_ALL_OK)
        {
            printf("[ERROR]--> Card %d mount failed\n", slot);

            num_cards_with_mount_errors += 1;
            sd_card_bank_ctl_disable_all();
            blink_n_times(STATUS_LED_COLOR_RED, 10);
            continue;
        }

        // open fake wav file
        if (sd_card_fopen("fake_wav.wav", POSIX_FILE_MODE_WRITE) != SD_CARD_ERROR_ALL_OK)
        {
            printf("[ERROR]--> Card %d fopen failed\n", slot);
            num_cards_with_file_errors += 1;
            sd_card_bank_ctl_disable_all();
            blink_n_times(STATUS_LED_COLOR_RED, 10);
            continue;
        }

        // // open a file
        // sprintf(str_buff, "sd_card_%i.txt", slot);
        // if (sd_card_fopen(str_buff, POSIX_FILE_MODE_WRITE) != SD_CARD_ERROR_ALL_OK)
        // {
        //     printf("[ERROR]--> Card %d fopen failed\n", slot);

        //     num_cards_with_file_errors += 1;
        //     sd_card_bank_ctl_disable_all();
        //     blink_n_times(STATUS_LED_COLOR_RED, 10);
        //     continue;
        // }

        if (sd_card_lseek(wav_header_get_header_length()) != SD_CARD_ERROR_ALL_OK)
        {
            printf("[ERROR]--> Card %d SD card lsleek failed\n", slot);
            blink_n_times(STATUS_LED_COLOR_BLUE, 10);
            continue;
        }
        // write all the audio data
        MXC_GPIO_OutSet(file_write_timing_pin.port, file_write_timing_pin.mask); // whole file timing
        for (uint32_t buff = 0; buff < NUM_FULL_BUFFERS_IN_WHOLE_FILE; buff++)
        {
            for (uint32_t samp = 0; samp < DEMO_CONFIG_AUDIO_BUFF_LEN_IN_SAMPS; samp++)
            {
                audio_buff[samp] = mock_audio_sine_tick();
            }

            MXC_GPIO_OutSet(buff_write_timing_pin.port, buff_write_timing_pin.mask); // single buffer timing
            if (sd_card_fwrite(audio_buff, AUDIO_BUFF_LEN_IN_BYTES, &bytes_written) != SD_CARD_ERROR_ALL_OK)
            {
                printf("[ERROR]--> Card %d write failed\n", slot);

                num_cards_with_file_errors += 1;
                sd_card_bank_ctl_disable_all();
                blink_n_times(STATUS_LED_COLOR_BLUE, 10);
                continue;
            }
            else
            {
                printf("[SUCCESS]--> Wrote test file to card %d\n", slot);
            }
            MXC_GPIO_OutClr(buff_write_timing_pin.port, buff_write_timing_pin.mask); // single buffer timing
        }

        // // write a simple message
        // sprintf(str_buff, "Hello from SD card %i", slot);
        // uint32_t bytes_written;
        // const SD_Card_Error_t res = sd_card_fwrite(str_buff, strlen((char *)str_buff), (void *)&bytes_written);
        // if (bytes_written != strlen(str_buff) || res != SD_CARD_ERROR_ALL_OK)
        // {
        //     printf("[ERROR]--> Card %d write failed\n", slot);

        //     num_cards_with_file_errors += 1;
        //     sd_card_bank_ctl_disable_all();
        //     blink_n_times(STATUS_LED_COLOR_BLUE, 10);
        //     continue;
        // }
        // else
        // {
        //     printf("[SUCCESS]--> Wrote test file to card %d\n", slot);
        // }
        // calculate the total size of the file and write the header
        const uint32_t file_size = sd_card_fsize();

        Wave_Header_Attributes_t wav_attr = {
            .num_channels = WAVE_HEADER_MONO,
            .bits_per_sample = DEMO_CONFIG_BITS_PER_SAMPLE,
            .sample_rate = DEMO_CONFIG_SAMPLE_RATE_HZ,
            .file_length = file_size,
        };
        wav_header_set_attributes(&wav_attr);

        if (sd_card_lseek(0) != SD_CARD_ERROR_ALL_OK)
        {
            blink_n_times(STATUS_LED_COLOR_BLUE, 10);
        }
        if (sd_card_lseek(0) != SD_CARD_ERROR_ALL_OK)
        {
           blink_n_times(STATUS_LED_COLOR_BLUE, 10);
        }

        if (sd_card_fwrite(wav_header_get_header(), wav_header_get_header_length(), &bytes_written) != SD_CARD_ERROR_ALL_OK)
        {
            blink_n_times(STATUS_LED_COLOR_GREEN, 10);
        }

        MXC_GPIO_OutClr(file_write_timing_pin.port, file_write_timing_pin.mask); // whole file timing


        // close the file, unmount, and unlink the drive in preparation for the next card
        sd_card_fclose();
        sd_card_unmount();

        num_successful_cards += 1;
        sd_card_bank_ctl_disable_all();
        blink_n_times(STATUS_LED_COLOR_GREEN, 10);
    }

    printf("\n******************* SUMMARY ******************************** \n");
    printf("Cards detected:               %d\n", SD_CARD_BANK_CTL_NUM_CARDS - num_missing_cards);
    printf("Cards with init/mount errors: %d\n", num_cards_with_mount_errors);
    printf("Cards with file errors:       %d\n", num_cards_with_file_errors);
    printf("Cards all successfull:        %d\n", num_successful_cards);

    while (1)
    {
        blink_n_times(STATUS_LED_COLOR_GREEN, num_successful_cards);
        MXC_Delay(1000000);

        blink_n_times(STATUS_LED_COLOR_BLUE, num_missing_cards);
        MXC_Delay(1000000);

        blink_n_times(STATUS_LED_COLOR_RED, num_cards_with_mount_errors + num_cards_with_file_errors);
        MXC_Delay(1000000);
    }
}

/* Private function definitions --------------------------------------------------------------------------------------*/

void blink_n_times(Status_LED_Color_t c, uint8_t n)
{
    status_led_all_off();

    for (int i = 0; i < n * 2; i++)
    {
        status_led_toggle(c);
        MXC_Delay(100000);
    }
}





