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
#include "led.h"
#include "board.h"
#include "mxc_delay.h"

#include "i2c.h"
#include "sd_card.h"
#include "sd_card_bank_ctl.h"

/* Private definitions -----------------------------------------------------------------------------------------------*/

#define I2C_BUS_3V3_PULLUPS (MXC_I2C2_BUS0)

#define I2C_CFG_MASTER_MODE (1)

#define I2C_CLK_SPEED (MXC_I2C_STD_MODE)

/* Private enumerations ----------------------------------------------------------------------------------------------*/

/**
 * Onboard LED colors for the FTHR2 board are represented here
 */
typedef enum
{
  LED_COLOR_RED = 0,
  LED_COLOR_GREEN,
  LED_COLOR_BLUE,
  LED_COLOR_YELLOW
} LED_Colot_t;

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
static void blink_n_times(LED_Colot_t c, uint8_t n);

/* Public function definitions ---------------------------------------------------------------------------------------*/

int main(void)
{
  if (MXC_I2C_Init(I2C_BUS_3V3_PULLUPS, I2C_CFG_MASTER_MODE, 0) != E_NO_ERROR)
  {
    printf("-->I2C initialization FAILED\n");
    return -1;
  }

  // I2C pins default to VDDIO for the logical high voltage, we want VDDIOH for 3.3v pullups
  const mxc_gpio_cfg_t i2c2_pins = {
      .port = MXC_GPIO1,
      .mask = (MXC_GPIO_PIN_14 | MXC_GPIO_PIN_15),
      .pad = MXC_GPIO_PAD_NONE,
      .func = MXC_GPIO_FUNC_ALT1,
      .vssel = MXC_GPIO_VSSEL_VDDIOH,
      .drvstr = MXC_GPIO_DRVSTR_0,
  };
  MXC_GPIO_Config(&i2c2_pins);

  if (MXC_I2C_SetFrequency(I2C_BUS_3V3_PULLUPS, I2C_CLK_SPEED) != I2C_CLK_SPEED)
  {
    printf("-->I2C frequency set FAILED\n");
    return -1;
  }

  if (sd_card_bank_ctl_init(I2C_BUS_3V3_PULLUPS) != SD_CARD_BANK_CTL_ERROR_ALL_OK)
  {
    printf("-->SD card bank ctl init FAILED\n");
    return -1;
  }

  for (int slot = 0; slot < SD_CARD_BANK_CTL_NUM_CARDS; slot++)
  {
    // this delay is just to slow it down so a human can see what's happening
    MXC_Delay(1000000);

    sd_card_bank_ctl_enable_slot(slot);

    sd_card_bank_ctl_read_and_cache_detect_pins();

    if (!sd_card_bank_ctl_active_card_is_inserted())
    {
      num_missing_cards += 1;
      sd_card_bank_ctl_disable_all();
      blink_n_times(LED_COLOR_BLUE, 10);
      continue;
    }

    // initialize and mount the card
    if (sd_card_init() != SD_CARD_ERROR_ALL_OK)
    {
      num_cards_with_mount_errors += 1;
      sd_card_bank_ctl_disable_all();
      blink_n_times(LED_COLOR_RED, 10);
      continue;
    }

    // without a brief delay between card init and mount, there are often mount errors
    MXC_Delay(100000);

    if (sd_card_mount() != SD_CARD_ERROR_ALL_OK)
    {
      num_cards_with_mount_errors += 1;
      sd_card_bank_ctl_disable_all();
      blink_n_times(LED_COLOR_RED, 10);
      continue;
    }

    // open a file
    sprintf(str_buff, "sd_card_%i.txt", slot);
    if (sd_card_fopen(str_buff, POSIX_FILE_MODE_WRITE) != SD_CARD_ERROR_ALL_OK)
    {
      num_cards_with_file_errors += 1;
      sd_card_bank_ctl_disable_all();
      blink_n_times(LED_COLOR_RED, 10);
      continue;
    }

    // write a simple message
    sprintf(str_buff, "Hello from SD card %i", slot);
    uint32_t bytes_written;
    const SD_Card_Error_t res = sd_card_fwrite(str_buff, strlen((char *)str_buff), (void *)&bytes_written);
    if (bytes_written != strlen(str_buff) || res != SD_CARD_ERROR_ALL_OK)
    {
      num_cards_with_file_errors += 1;
      sd_card_bank_ctl_disable_all();
      blink_n_times(LED_COLOR_YELLOW, 10);
      continue;
    }

    // close the file, unmount, and unlink the drive in preparation for the next card
    sd_card_fclose();
    sd_card_unmount();

    // if we get here, then all steps were successful
    num_successful_cards += 1;
    sd_card_bank_ctl_disable_all();
    blink_n_times(LED_COLOR_GREEN, 10);
  }

  while (1)
  {
    blink_n_times(LED_COLOR_GREEN, num_successful_cards);
    MXC_Delay(1000000);

    blink_n_times(LED_COLOR_BLUE, num_missing_cards);
    MXC_Delay(1000000);

    blink_n_times(LED_COLOR_RED, num_cards_with_mount_errors);
    MXC_Delay(1000000);

    blink_n_times(LED_COLOR_YELLOW, num_cards_with_file_errors);
    MXC_Delay(1000000);
  }
}

/* Private function definitions --------------------------------------------------------------------------------------*/

void blink_n_times(LED_Colot_t c, uint8_t n)
{
  for (int i = 0; i < n; i++)
  {
    if (c == LED_COLOR_YELLOW)
    {
      LED_On(LED_COLOR_RED);
      LED_On(LED_COLOR_GREEN);
      MXC_Delay(100000);
      LED_Off(LED_COLOR_RED);
      LED_Off(LED_COLOR_GREEN);
      MXC_Delay(100000);
    }
    else
    {
      LED_On(c);
      MXC_Delay(100000);
      LED_Off(c);
      MXC_Delay(100000);
    }
  }
}
