
/* Private includes --------------------------------------------------------------------------------------------------*/

#include "mxc_delay.h"
#include "sd_card.h"
#include "sdhc_lib.h"
#include "sdhc_regs.h"
#include <stddef.h> // for NULL

#include "mxc_device.h"

/* Private defines ---------------------------------------------------------------------------------------------------*/

#define SD_CARD_INIT_NUM_RETRIES (100)

#define SDHC_CONFIG_BLOCK_GAP (0)
#define SDHC_CONFIG_CLK_DIV (0x0b0)

/* Private variables -------------------------------------------------------------------------------------------------*/

static FATFS *fs; // FFat Filesystem Object
static FATFS fs_obj;
static FIL SD_file; // FFat File Object
static bool is_mounted;

static char volume = '0';

/* Public function definitions ---------------------------------------------------------------------------------------*/

SD_Card_Error_t sd_card_init()
{
    // needed for FTHR2
    const mxc_gpio_cfg_t sd_card_en_pin = {
        .port = MXC_GPIO1,
        .mask = MXC_GPIO_PIN_6,
        .pad = MXC_GPIO_PAD_NONE,
        .func = MXC_GPIO_FUNC_OUT,
        .vssel = MXC_GPIO_VSSEL_VDDIOH,
    };
    MXC_GPIO_Config(&sd_card_en_pin);
    MXC_GPIO_OutClr(sd_card_en_pin.port, sd_card_en_pin.mask);

    const mxc_sdhc_cfg_t sdhc_cfg = {
        .bus_voltage = MXC_SDHC_Bus_Voltage_3_3,
        .block_gap = SDHC_CONFIG_BLOCK_GAP,
        .clk_div = SDHC_CONFIG_CLK_DIV,
    };

    if (MXC_SDHC_Init(&sdhc_cfg) != E_NO_ERROR)
    {
        return SD_CARD_INIT_ERROR;
    }

    // needed for FTHR2
    MXC_GPIO_Config(&sd_card_en_pin);
    MXC_GPIO_OutClr(sd_card_en_pin.port, sd_card_en_pin.mask);

    // without a delay here the next function was consistently returning an error
    MXC_Delay(10000);

    if (MXC_SDHC_Lib_InitCard(SD_CARD_INIT_NUM_RETRIES) != E_NO_ERROR)
    {
        return SD_CARD_INIT_ERROR;
    }

    return SD_CARD_ERROR_ALL_OK;
}

SD_Card_Error_t sd_card_mount()
{
    fs = &fs_obj;

    if (f_mount(fs, "", 1) != FR_OK)
    {
        return SD_CARD_MOUNT_ERROR;
    }

    is_mounted = true;
    return SD_CARD_ERROR_ALL_OK;
}

SD_Card_Error_t sd_card_unmount()
{
    is_mounted = false;
    return f_mount(0, "", 0);
}

bool sd_card_is_mounted()
{
    return is_mounted;
}

uint32_t sd_card_disk_size_bytes()
{
    if (!sd_card_is_mounted())
    {
        return 0;
    }

    // from elm-chan: http://elm-chan.org/fsw/ff/doc/getfree.html
    const uint32_t total_sectors = (fs->n_fatent - 2) * fs->csize;
    return total_sectors / 2;
}

uint32_t sd_card_free_space_bytes()
{
    if (!sd_card_is_mounted())
    {
        return 0;
    }

    // from elm-chan: http://elm-chan.org/fsw/ff/doc/getfree.html
    uint32_t free_clusters;
    if (f_getfree(&volume, &free_clusters, &fs) != FR_OK)
    {
        return 0;
    }

    const uint32_t free_sectors = free_clusters * fs->csize;
    return free_sectors / 2;
}

SD_Card_Error_t sd_card_mkdir(const char *path)
{
    return f_mkdir(path) == FR_OK ? SD_CARD_ERROR_ALL_OK : SD_CARD_DIRECTORY_ERROR;
}

SD_Card_Error_t sd_card_cd(const char *path)
{
    return f_chdir(path) == FR_OK ? SD_CARD_ERROR_ALL_OK : SD_CARD_DIRECTORY_ERROR;
}

SD_Card_Error_t sd_card_fopen(const char *file_name, POSIX_FileMode_t mode)
{
    return f_open(&SD_file, file_name, mode) == FR_OK ? SD_CARD_ERROR_ALL_OK : SD_CARD_FILE_IO_ERROR;
}

SD_Card_Error_t sd_card_fclose()
{
    return f_close(&SD_file) == FR_OK ? SD_CARD_ERROR_ALL_OK : SD_CARD_FILE_IO_ERROR;
}

SD_Card_Error_t sd_card_fwrite(const void *buff, uint32_t size, uint32_t *written)
{
    return f_write(&SD_file, buff, size, (UINT *)written) == FR_OK ? SD_CARD_ERROR_ALL_OK : SD_CARD_FILE_IO_ERROR;
}

SD_Card_Error_t sd_card_lseek(uint32_t offset)
{
    return f_lseek(&SD_file, offset) == FR_OK ? SD_CARD_ERROR_ALL_OK : SD_CARD_FILE_IO_ERROR;
}

uint32_t sd_card_fsize()
{
    return f_size(&SD_file);
}
