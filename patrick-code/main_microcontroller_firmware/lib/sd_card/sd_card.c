
/* Private includes --------------------------------------------------------------------------------------------------*/

#include <stddef.h> // for NULL

#include "mxc_delay.h"
#include "mxc_device.h"
#include "sdhc_lib.h"
#include "sdhc_regs.h"

#include "bsp_sdhc.h"
#include "sd_card.h"

/* Private defines ---------------------------------------------------------------------------------------------------*/

#define SD_CARD_INIT_NUM_RETRIES (20)

#define SDHC_CONFIG_BLOCK_GAP (0)
#define SDHC_CONFIG_CLK_DIV (0x0b0)

/* Private variables -------------------------------------------------------------------------------------------------*/

static FATFS *fs; // FFat Filesystem Object
static FATFS fs_obj;
static FIL SD_file; // FFat File Object
static bool is_mounted = false;

static char volume = '0';

/* Public function definitions ---------------------------------------------------------------------------------------*/

int sd_card_init()
{
    int res = E_NO_ERROR;

    if ((res = bsp_sdhc_init()) != E_NO_ERROR)
    {
        return res;
    }

    // without a delay here the next function was consistently returning an error
    MXC_Delay(100000);

    if ((res = MXC_SDHC_Lib_InitCard(SD_CARD_INIT_NUM_RETRIES)) != E_NO_ERROR)
    {
        return res;
    }

    return E_NO_ERROR;
}

int sd_card_mount()
{
    fs = &fs_obj;

    if (f_mount(fs, "", 1) != FR_OK)
    {
        return E_COMM_ERR;
    }

    is_mounted = true;
    return E_NO_ERROR;
}

int sd_card_unmount()
{
    is_mounted = false;
    return f_mount(0, "", 0) == FR_OK ? E_NO_ERROR : E_COMM_ERR;
}

bool sd_card_is_mounted()
{
    return is_mounted;
}

QWORD sd_card_disk_size_bytes()
{
    if (!sd_card_is_mounted())
    {
        return 0;
    }

    // from elm-chan: http://elm-chan.org/fsw/ff/doc/getfree.html
    DWORD total_sectors = (fs->n_fatent - 2) * fs->csize;
    
    return ((QWORD)(total_sectors / 2) * (QWORD)(1024)); // for cards over 3GB, we need QWORD to hold size
}

QWORD sd_card_free_space_bytes()
{
    if (!sd_card_is_mounted())
    {
        return 0;
    }

    // from e]lm-chan: http://elm-chan.org/fsw/ff/doc/getfree.html
    QWORD free_clusters;
    if (f_getfree(&volume, &free_clusters, &fs) != FR_OK)
    {
        return 0;
    }

    DWORD free_sectors = free_clusters * fs->csize;
    return ((QWORD)(free_sectors / 2) * (QWORD)(1024));
}

int sd_card_mkdir(const char *path)
{
    return f_mkdir(path) == FR_OK ? E_NO_ERROR : E_COMM_ERR;
}

int sd_card_cd(const char *path)
{
    return f_chdir(path) == FR_OK ? E_NO_ERROR : E_COMM_ERR;
}

int sd_card_fopen(const char *file_name, POSIX_FileMode_t mode)
{
    return f_open(&SD_file, file_name, mode) == FR_OK ? E_NO_ERROR : E_COMM_ERR;
}

int sd_card_fclose()
{
    return f_close(&SD_file) == FR_OK ? E_NO_ERROR : E_COMM_ERR;
}

int sd_card_fwrite(const void *buff, uint32_t size, uint32_t *written)
{
    return f_write(&SD_file, buff, size, (UINT *)written) == FR_OK ? E_NO_ERROR : E_COMM_ERR;
}

int sd_card_lseek(uint32_t offset)
{
    return f_lseek(&SD_file, offset) == FR_OK ? E_NO_ERROR : E_COMM_ERR;
}

uint32_t sd_card_fsize()
{
    return f_size(&SD_file);
}

FRESULT set_file_timestamp (
    char *obj,     /* Pointer to the file name */
    int year,
    int month,
    int mday,
    int hour,
    int min,
    int sec
)
{
    FILINFO fno;

    fno.fdate = (WORD)(((year - 1980) * 512U) | month * 32U | mday);
    fno.ftime = (WORD)(hour * 2048U | min * 32U | sec / 2U);

    return f_utime(obj, &fno);
}
