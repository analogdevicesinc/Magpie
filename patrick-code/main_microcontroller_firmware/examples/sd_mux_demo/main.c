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
#include "bsp_uart.h"
#include "sd_card.h"
#include "sd_card_bank_ctl.h"

#ifdef TERMINAL_IO_USE_SEGGER_RTT
#include "SEGGER_RTT.h"
#endif

/* Variables ---------------------------------------------------------------------------------------------------------*/

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
#ifdef TERMINAL_IO_USE_CONSOLE_UART
    bsp_console_uart_init();
#endif

    printf("\n******************* SD Card Bank Example ******************* \n");

    if (bsp_3v3_i2c_init() != E_NO_ERROR)
    {
        printf("[ERROR]--> I2C init\n");
    }
    else
    {
        printf("[SUCCESS]--> I2C init\n");
    }

    if (sd_card_bank_ctl_init() != E_NO_ERROR)
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
        if (sd_card_init() != E_NO_ERROR)
        {
            printf("[ERROR]--> Card %d init failed\n", slot);

            num_cards_with_mount_errors += 1;
            sd_card_bank_ctl_disable_all();
            blink_n_times(STATUS_LED_COLOR_RED, 10);
            continue;
        }

        // without a brief delay between card init and mount, there are often mount errors
        MXC_Delay(100000);

        if (sd_card_mount() != E_NO_ERROR)
        {
            printf("[ERROR]--> Card %d mount failed\n", slot);

            num_cards_with_mount_errors += 1;
            sd_card_bank_ctl_disable_all();
            blink_n_times(STATUS_LED_COLOR_RED, 10);
            continue;
        }

        // open a file
        sprintf(str_buff, "sd_card_%i.txt", slot);
        if (sd_card_fopen(str_buff, POSIX_FILE_MODE_WRITE) != E_NO_ERROR)
        {
            printf("[ERROR]--> Card %d fopen failed\n", slot);

            num_cards_with_file_errors += 1;
            sd_card_bank_ctl_disable_all();
            blink_n_times(STATUS_LED_COLOR_RED, 10);
            continue;
        }

        // write a simple message
        sprintf(str_buff, "Hello from SD card %i", slot);
        uint32_t bytes_written;
        const int res = sd_card_fwrite(str_buff, strlen((char *)str_buff), (void *)&bytes_written);
        if (bytes_written != strlen(str_buff) || res != E_NO_ERROR)
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
