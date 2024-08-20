/***** Includes *****/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "mxc_device.h"



#include "mxc_sys.h"
#include "mxc_delay.h"
#include "mxc_errors.h"
#include "sdhc_regs.h"
#include "led.h"
#include "tmr.h"
#include "uart.h"
#include "gpio.h"
#include "board.h"




#include "nvic_table.h"
#include "spi.h"
#include "i2c.h"
#include "sdhc_regs.h"
#include "sdhc_lib.h"
#include "ff.h"
#include "utils.h"
#include "dma.h"
#include "arm_math.h"

#include "DS3231_driver.h"   //Access to the RTC DS3231 peripheral
#include <time.h>
#include "rtc.h"

#include "./data_converters.h"
#include "./periphDirectAccess.txt"


// make sure the following is set BEFORE including the decimation filter code
//#define UNALIGNED_SUPPORT_DISABLE

//#include "arm_fir_decimate_fast_q15_bob.h"
#include "arm_fir_decimate_fast_q31_bob.h"
#include "arm_fir_decimate_fast_q31_HB.h"

/***** #defines  *****/

/********************************************************************************************/

//#define FIRST_SET_RTC true   //undefine this if you don't want to set the time on RTC
							 //Change the values of struct tm newTime to the date you want to set

/********************************************************************************************/
// RECORDING TIME IN DMA Blocks (each DMA block is 21.33 ms)

#define RECORDING_TIME_DMABLOCKS  1000//10000


//Pin 0.20 for ADC clock enable or disable

#define MXC_SPI2_SSIDX 0

//****************************************************************
//SPI defines for SPI1 slave
//****************************************************************

#define SPI_MASTER_IRQ SPI1_IRQn


// defines for DMA
//#define DMA_bufflen 8192 // 24-bit words (not bytes)
// magpie_new; change dma length to be divisible by all possible decimation factors
#define DMA_buffLen 8256 // 24-bit words (not bytes), note this is divisible by 2,4,8,12 and 24 for all sample-rates
#define DMA_buffLen_bytes 3*DMA_buffLen //in  bytes, = 8K 24-bit words;after decimation, = 1K 48KHz words).

// buffer lengths for various decimation factors (all integers)
#define buffLen_deci2x DMA_buffLen/2 // after 1 stage (dec 2)
#define buffLen_deci3x DMA_buffLen/3 // after 1 stage (dec 3)
#define buffLen_deci4x DMA_buffLen/4 // after 2 stages (2 - 2)
#define buffLen_deci6x DMA_buffLen/6 // after 2 stages (3 - 2)
#define buffLen_deci8x DMA_buffLen/8 // after 3 stages ( 2 - 2 - 2)
#define buffLen_deci12x DMA_buffLen/12 // after 3 stages (3 - 2 - 2)
#define buffLen_deci16x DMA_buffLen/16 // after 4 stages (2 - 2 - 2 - 2)
#define buffLen_deci24x DMA_buffLen/24 // after 4 stages (3 - 2 - 2 - 2)

// magpie_new ; number of coefficients, and length of the state vector for all sample-rates
// 16 k

// decimation order: 3,2,2,2 All 2's are halfband
#define deci_16k_numcoeffs_0 6
#define deci_16k_numcoeffs_1 7
#define deci_16k_numcoeffs_2 7
#define deci_16k_numcoeffs_3 27

// note filter state registers are shared between all sample-rates, so set to the laongest needed

// 24 k
// decimation order: 2,2,2,2 , the last 3 2's are halfband 2's are halfband

#define deci_24k_numcoeffs_0 5
#define deci_24k_numcoeffs_1 7
#define deci_24k_numcoeffs_2 7
#define deci_24k_numcoeffs_3 23


// 32 k
// note decimation factors are 3,2,2, the 2's are halfband

#define deci_32k_numcoeffs_0 6
#define deci_32k_numcoeffs_1 7
#define deci_32k_numcoeffs_2 23


// 48 k
// decimation factors 2,2,2, the last 2 2's are hafband
#define deci_48k_numcoeffs_0 5
#define deci_48k_numcoeffs_1 7
#define deci_48k_numcoeffs_2 23


// 96 k
// decimation factors 2,2, the last section is halfband
#define deci_96k_numcoeffs_0 6
#define deci_96k_numcoeffs_1 23



// 192 k 30 dB
// decimation is 2, not halfband. I could use halfband but then I can't scale the input down by -3dB.
// If this could be done before the filter, then I would switch to halfband and save mips,time

#define deci_192k_numcoeffs_0 11
// #define deci_state_len_192k_0 DMA_buffLen + deci_192k_numcoeffs_0 -1


// try universal filter state memory to minimize memory useage (actual requirement usually lower)
#define deci_stage0_state_len DMA_buffLen + 6 - 1
#define deci_stage1_state_len buffLen_deci2x + 23 -1
#define deci_stage2_state_len buffLen_deci4x + 23 -1
#define deci_stage3_state_len buffLen_deci8x + 23 -1



// I2C defines
#define I2C_MASTER MXC_I2C0_BUS0
#define I2C_SLAVE_ADDR (0x98) // for a write, read is 99
#define I2C_BYTES 2
#define I2C_FREQ 100000


// I2C config for connecting to DS3231
#define MAX32666_I2C_BUS_3V3_PULLUPS (MXC_I2C2_BUS0)    //***Change the I2C Bus here
#define MAX32666_I2C_CFG_MASTER_MODE (1)
#define MAX32666_I2C_CLK_SPEED (MXC_I2C_STD_MODE)


// ************************ ADC/SPI Defines *******************


#define AD463X_REG_EXIT_CFG_MODE	0x14
#define AD463X_REG_MODES		0x20
#define AD463X_REG_OSCILATOR		0x21
#define AD463X_EXIT_CFG_MODE		NO_OS_BIT(0)
#define AD463X_REG_READ_DUMMY		0x00
#define AD463X_REG_READ		    	NO_OS_BIT(7)

// Error codes used in the Secure CRT functions

#define _SECURECRT_ERRCODE_VALUES_DEFINED
#define EINVAL          22
#define ERANGE          34
#define EILSEQ          42
#define STRUNCATE       80


//****************************************************************
//GPIO defines for ADC
//****************************************************************
//For the CS pin while using the SPI2 on the FTHR
#define MXC_GPIO_PORT_OUT0 MXC_GPIO0

// bit-bang slave-sel during ADC init
#define MXC_GPIO_PIN_OUT16 MXC_GPIO_PIN_16

//Pin 0.20 used for enabling and disabling CNV start signal through 74LV4060
#define MXC_GPIO_PIN_OUT20 MXC_GPIO_PIN_20

//Pin 0.21 use for hardware reset
#define MXC_GPIO_PIN_OUT21 MXC_GPIO_PIN_21

#define MXC_GPIO_PIN_OUT5 MXC_GPIO_PIN_5  // gpio5, pin 6 on feather, green LED on motherboard

#define MXC_GPIO_PIN_OUT3 MXC_GPIO_PIN_3  // gpio3, pin 5 on feather, blue led on motherboard

#define MXC_GPIO_PIN_OUT4 MXC_GPIO_PIN_4  // gpio4, pin 7 on feather

#define MXC_GPIO_PIN_OUT12 MXC_GPIO_PIN_12  // gpio12, pin 4 on feather connected to pin 6 of p8 header, used for timing code

#define MXC_GPIO_PIN_OUT30  MXC_GPIO_PIN_30 //Blue LED on FTHR board (not motherboard!)


// Parameters for PWM output
#define PORT_PWM MXC_GPIO0 //port
#define PIN_PWM MXC_GPIO_PIN_12 //pin
#define FREQ 200000 // (Hz)
#define DUTY_CYCLE 75 // (%)
#define PWM_TIMER MXC_TMR0 // must change PWM_PORT and PWM_PIN if changed

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define MAXLEN 256


#define WAV_HEADER_SIZE 44
#define WAV_BLOCK_SIZE 512

#define testWriteLen 32768


/****************** TypeDef ******************/
/**
 * @brief Enumerated DS3231 I2C error mode.
 */
typedef enum{
    DS3231_I2C_NO_ERROR,
	MCU_MASTER_I2C_INIT_ERROR,
    DS3231_I2C_INIT_ERROR,
    DS3231_I2C_FREQSET_ERROR
} DS3231_i2cError;


/************************ Globals ******************************/
#define DEFAULT_FILENAME "0:Magpie00_19000101_000000.wav"
char savedFileName[31] = DEFAULT_FILENAME;

//==============DS3231 Related=========================
#define OUTPUT_MSG_BUFFER_SIZE       128U

ds3231_driver_t DS3231_RTC;
static struct tm ds3231_datetime;
static char ds3231_datetime_str[17];
static float ds3231_temperature;
static uint8_t output_msgBuffer[OUTPUT_MSG_BUFFER_SIZE];
const struct tm ds3231_dateTimeDefault = {
	.tm_year = 118U,
	.tm_mon = 00U,
	.tm_mday = 1U,
	.tm_hour = 0U,
	.tm_min = 0U,
	.tm_sec = 0U
};

FRESULT set_timestamp (
    char *obj,     /* Pointer to the file name */
    int year,
    int month,
    int mday,
    int hour,
    int min,
    int sec
);

// structs for spi,gpio


mxc_gpio_cfg_t gpio_out16;
mxc_gpio_cfg_t gpio_out20; // adc clock enable
mxc_gpio_cfg_t gpio_out5; //pin 6 on feather, green LED on motherboard
mxc_gpio_cfg_t gpio_in3; // pin 5 on feather, blue led on motherboard

mxc_gpio_cfg_t gpio_in30; // the blue LED on feather (not on motherboard!)
mxc_gpio_cfg_t gpio_out12; // pin 4 on feather, used for timing etsts

// structs for spi port
mxc_spi_req_t SPI2_req_master_ctrl_write; // use this struct when using spi2 to write to the adc control port
mxc_spi_req_t SPI1_req; // struct for SPI int


volatile uint32_t count_dma_irq = 0;
volatile uint32_t dataBlocksDmaCount=0,dataBlocksConsumedCount=0;
volatile uint8_t dataBlockWriteComplete=1; // init to 1 so the first dma interupt will work
volatile uint32_t writeNotComplete = 0;
static uint8_t SD_write_buff[4*DMA_buffLen_bytes] = {0}; // up to 4 sd card dma interval stalls at 384k

FATFS *fs; //FFat Filesystem Object
FATFS fs_obj;
FIL file; //FFat  File Object
FRESULT err; //FFat Result (Struct)
FILINFO fno; //FFat File Information Object
DIR dir; //FFat Directory Object
TCHAR message[32768];
WORD message1[32768];
TCHAR directory[MAXLEN], cwd[MAXLEN], filename[MAXLEN], volume_label[24],
volume = '0';
TCHAR *FF_ERRORS[20];
uint32_t clusters_free = 0, sectors_free = 0, sectors_total = 0, volume_sn = 0;
UINT bytes_written = 0, bytes_read = 0, mounted = 0;
BYTE work[4096];




// global DMA var
static uint8_t dmaDestBuff[DMA_buffLen_bytes] = {0}; // bytes transferred here from the dma
//static uint8_t dmaDestBuffCopy[DMA_buffLen_bytes] = {0}; // debug

#ifndef TEST_DECIMATE
static q31_t dmaDestBuff_32bit[DMA_buffLen] = {0}; // same data but assembled back into 32-bit words
#endif
#ifdef TEST_DECIMATE
// fill dmaDestBuff with values from Matlab-generated test file
//#include "./decimate_test_16k.txt" // yes
//#include "./decimate_test_24k.txt" // yes
//#include "./decimate_test_32k.txt" // yes
//#include "./decimate_test_48k.txt" // yes
//#include "./decimate_test_96k.txt" // yes
//#include "./decimate_test_192k_30dB.txt"
#endif

int mychannel = -1;
// magpie_new ; enumeration list for all sample-rates
typedef enum {
		fs_384k,
		fs_192k,
		fs_96k,
		fs_48k,
		fs_32k,
		fs_24k,
		fs_16k
} FS_enum; // this is a variable 'type'

static FS_enum magpie_FS; // use for switch statements
static uint32_t FS = 48000; // use for calculations. This will be set to agree with the magpie_fs variable in main()


static uint32_t numBytesSDwrite = 0u; // number of bytes to write in each f_write function, varies with sample-rate
static uint8_t magpie_bitdepth = 0u; // 1 = 24 bits, 0 = 16 bits
static uint32_t block_ptr_modulo_mask = 0x00000003;
// apply to roll over the block index, for the sd-card slow-write buffer scheme.
// This varies with sample-rate. Low sample-rates have small buffer write sizes,
// so you can fit lots of blocks in the same SD buffer memory

// decimated output buffers for various stages of the multi-rate filters
static q31_t deci_stage0_out[buffLen_deci2x] = {0}; // 1st decimator output , dec 2 or 3
static q31_t deci_stage1_out[buffLen_deci4x] = {0}; // 2nd decimator out, dec 4 or 6
static q31_t deci_stage2_out[buffLen_deci8x] = {0}; // 3rd decimator out, dec 8 or 12
static q31_t deci_stage3_out[buffLen_deci16x] = {0}; // 4th decimator out, dec 16 or 24

// magpie_new, all the coefficients from the Matlab program
// 16 k

static q31_t firCoeffs_16k_0[deci_16k_numcoeffs_0] = {
66940622, 234663147, 399187040, 399187040, 234663147, 66940622};

static q31_t firCoeffs_16k_1[deci_16k_numcoeffs_1] = {
-72761205, 0, 609333412, 1073741824, 609333412, 0, -72761205};

static q31_t firCoeffs_16k_2[deci_16k_numcoeffs_2] = {
-93393363, 0, 624899797, 1073741824, 624899797, 0, -93393363};

static q31_t firCoeffs_16k_3[deci_16k_numcoeffs_3] = {
14350378, 0, -22432788, 0, 39696342, 0, -66842481, 0, 113185102, 0, -213099907, 0, 678529663, 1073741824, 678529663, 0, -213099907, 0, 113185102, 0, -66842481, 0, 39696342, 0, -22432788, 0, 14350378};

// 24k

static q31_t firCoeffs_24k_0[deci_24k_numcoeffs_0] = {
87026071, 382177371, 589816446, 382177371, 87026071};

static q31_t firCoeffs_24k_1[deci_24k_numcoeffs_1] = {
-72761205, 0, 609333412, 1073741824, 609333412, 0, -72761205};

static q31_t firCoeffs_24k_2[deci_24k_numcoeffs_2] = {
-93393363, 0, 624899797, 1073741824, 624899797, 0, -93393363};

static q31_t firCoeffs_24k_3[deci_24k_numcoeffs_3] = {
-24688186, 0, 36136863, 0, -63560487, 0, 110521559, 0, -211345515, 0, 677934784, 1073741824, 677934784, 0, -211345515, 0, 110521559, 0, -63560487, 0, 36136863, 0, -24688186};

// 32 k


static q31_t firCoeffs_32k_0[deci_32k_numcoeffs_0] = {
70889607, 230467457, 380341257, 380341257, 230467457, 70889607};

static q31_t firCoeffs_32k_1[deci_32k_numcoeffs_1] = {
-93393363, 0, 624899797, 1073741824, 624899797, 0, -93393363};

static q31_t firCoeffs_32k_2[deci_32k_numcoeffs_2] = {
-24688186, 0, 36136863, 0, -63560487, 0, 110521559, 0, -211345515, 0, 677934784, 1073741824, 677934784, 0, -211345515, 0, 110521559, 0, -63560487, 0, 36136863, 0, -24688186};


// 48 k halfband


static q31_t firCoeffs_48k_0[deci_48k_numcoeffs_0] = {
90612570, 389972970, 596749769, 389972970, 90612570};

static q31_t firCoeffs_48k_1[deci_48k_numcoeffs_1] = {
-93393363, 0, 624899797, 1073741824, 624899797, 0, -93393363};

static q31_t firCoeffs_48k_2[deci_48k_numcoeffs_2] = {
-24688186, 0, 36136863, 0, -63560487, 0, 110521559, 0, -211345515, 0, 677934784, 1073741824, 677934784, 0, -211345515, 0, 110521559, 0, -63560487, 0, 36136863, 0, -24688186};

// 96 k


static q31_t firCoeffs_96k_0[deci_96k_numcoeffs_0] = {
-10807389, 213981735, 614165432, 614165432, 213981735, -10807389};

static q31_t firCoeffs_96k_1[deci_96k_numcoeffs_1] = {
-24688186, 0, 36136863, 0, -63560487, 0, 110521559, 0, -211345515, 0, 677934784, 1073741824, 677934784, 0, -211345515, 0, 110521559, 0, -63560487, 0, 36136863, 0, -24688186};


// 192 k

static q31_t firCoeffs_192k_0[deci_192k_numcoeffs_0] = {
70537572, 3275316, -138383646, 1734727, 471760716, 762876425, 471760716, 1734727, -138383646, 3275316, 70537572};


// magpie_new , filter state variables as required by the cmsis functions, for all sample-rates.

// universal state filter arrays , set to largest needed across sample-rates
static q31_t firState_stage0[deci_stage0_state_len] = {0};
static q31_t firState_stage1[deci_stage1_state_len] = {0};
static q31_t firState_stage2[deci_stage2_state_len] = {0};
static q31_t firState_stage3[deci_stage3_state_len] = {0};


int location = 0;
static uint32_t numSlowWrites = 0u; // keeps track of how many slow sd writes
static uint32_t numSDwriteErrors = 0u;// keeps track of how many slow sd writes could not be fixed
static uint32_t delta = 0; // diff between the dma block count and the "consumed" count

// magpie_new
static uint32_t blockPtrModuloSDbuff=0; // the block pointer into SD memory for the slow-sd buffer scheme
static uint32_t offsetSDbuff = 0; // offset in bytes into the SD write memory, for slow-sd scheme
static uint32_t blockPtrModuloDMA=0;// the block pointer for reading from the DMA buff for the slow-sd scheme
static uint32_t offsetDMA = 0; // offset in bytes for writing the dma memory, for the slow-sd scheme

uint8_t SPI1_rx_3byte[3];

//static q15_t spi_data_q15; // use for 16-bit

// I2C variables
static uint8_t i2cErr=0;
static int I2C_FLAG=0;

static uint8_t i2c_txdata[I2C_BYTES] = {0x1};
static uint8_t i2c_rxdata[I2C_BYTES] = {0x1};


// debug variables
// use volatile so compiler does not optimize these variables away when using -o2
volatile uint32_t debug0=0,debug1=0,debug2=0,debug3 = 0;
//static uint32_t temp0=0,temp1=0,temp2=0,temp3=0;
volatile uint32_t temp0=0,temp1=0,temp2=0,temp3=0;
volatile uint32_t errCount=0;


// function prototypes

// magpie_new - all the filter instances for every sample-rate
// CMSIS instances. Note that the "fast" version
// uses the same structure as the regular version

// 16 k
arm_fir_decimate_instance_q31 Sdeci_16k_0;
arm_fir_decimate_instance_q31 Sdeci_16k_1;
arm_fir_decimate_instance_q31 Sdeci_16k_2;
arm_fir_decimate_instance_q31 Sdeci_16k_3;

// 24 k
arm_fir_decimate_instance_q31 Sdeci_24k_0;
arm_fir_decimate_instance_q31 Sdeci_24k_1;
arm_fir_decimate_instance_q31 Sdeci_24k_2;
arm_fir_decimate_instance_q31 Sdeci_24k_3;

// 32 k

arm_fir_decimate_instance_q31 Sdeci_32k_0;
arm_fir_decimate_instance_q31 Sdeci_32k_1;
arm_fir_decimate_instance_q31 Sdeci_32k_2;

// 48 k
arm_fir_decimate_instance_q31 Sdeci_48k_0;
arm_fir_decimate_instance_q31 Sdeci_48k_1;
arm_fir_decimate_instance_q31 Sdeci_48k_2;


// 48 k 2-ch test
arm_fir_decimate_instance_q31 Sdeci_2ch_48k_0;
arm_fir_decimate_instance_q31 Sdeci_2ch_48k_1;
arm_fir_decimate_instance_q31 Sdeci_2ch_48k_2;


// 96 k
arm_fir_decimate_instance_q31 Sdeci_96k_0;
arm_fir_decimate_instance_q31 Sdeci_96k_1;

// 192 k
arm_fir_decimate_instance_q31 Sdeci_192k_0;


//arm_biquad_casd_df1_inst_q31 Siir;



// *********** start functions *******************



int mount()
{
	fs = &fs_obj;
	if ((err = f_mount(fs, "", 1)) != FR_OK) { //Mount the default drive to fs now
		debug1=20;
		//printf("Error opening SD card: %s\n", FF_ERRORS[err]);
		f_mount(NULL, "", 0);
	} else {
		debug1=22;
		//printf("SD card mounted.\n");
		mounted = 1;
	}

	f_getcwd(cwd, sizeof(cwd)); //Set the Current working directory

	return err;
}

int umount()
{
	if ((err = f_mount(NULL, "", 0)) != FR_OK) { //Unmount the default drive from its mount point
		printf("Error unmounting volume: %s\n", FF_ERRORS[err]);
	} else {
		printf("SD card unmounted.\n");
		mounted = 0;
	}

	return err;
}


void read_SPI1_regs() // for debug only
{
	// note, these appear in the same order as in the user guide
	SPI1_DATA0_direct_probe =  SPI1_DATA0_direct;
	SPI1_CTRL0_direct_probe =  SPI1_CTRL0_direct;
	SPI1_CTRL1_direct_probe =  SPI1_CTRL1_direct;
	SPI1_CTRL2_direct_probe =  SPI1_CTRL2_direct;
	SPI1_SS_TIME_direct_probe =  SPI1_SS_TIME_direct;
	SPI1_CLK_CFG_direct_probe =  SPI1_CLK_CFG_direct;
	SPI1_DMA_direct_probe =  SPI1_DMA_direct;
	SPI1_INT_FL_direct_probe =  SPI1_INT_FL_direct;
	SPI1_INT_EN_direct_probe =  SPI1_INT_EN_direct;
	SPI1_WAKE_FL_direct_probe =  SPI1_WAKE_FL_direct;
	SPI1_WAKE_EN_direct_probe =  SPI1_WAKE_EN_direct;
	SPI1_STAT_direct_probe =  SPI1_STAT_direct;
}

void read_SPI2_regs() // for debug only
{
	// note, these appear in the same order as in the user guide
	SPI2_DATA0_direct_probe =  SPI2_DATA0_direct;
	SPI2_CTRL0_direct_probe =  SPI2_CTRL0_direct;
	SPI2_CTRL1_direct_probe =  SPI2_CTRL1_direct;
	SPI2_CTRL2_direct_probe =  SPI2_CTRL2_direct;
	SPI2_SS_TIME_direct_probe =  SPI2_SS_TIME_direct;
	SPI2_CLK_CFG_direct_probe =  SPI2_CLK_CFG_direct;
	SPI2_DMA_direct_probe =  SPI2_DMA_direct;
	SPI2_INT_FL_direct_probe =  SPI2_INT_FL_direct;
	SPI2_INT_EN_direct_probe =  SPI2_INT_EN_direct;
	SPI2_WAKE_FL_direct_probe =  SPI2_WAKE_FL_direct;
	SPI2_WAKE_EN_direct_probe =  SPI2_WAKE_EN_direct;
	SPI2_STAT_direct_probe =  SPI2_STAT_direct;

}


void read_DMA0_regs() // for debug only
{
	// note, these appear in the same order as in the user guide

	DMA0_CTRL_direct_probe = DMA0_CTRL_direct;
	DMA0_INTR_direct_probe  = DMA0_INTR_direct;
	DMA0_CH0_CFG_direct_probe = DMA0_CH0_CFG_direct;
	DMA0_CH0_ST_direct_probe = DMA0_CH0_ST_direct;
	DMA0_CH0_SRC_direct_probe = DMA0_CH0_SRC_direct;
	DMA0_CH0_DST_direct_probe = DMA0_CH0_DST_direct;
	DMA0_CH0_CNT_direct_probe = DMA0_CH0_CNT_direct;
	DMA0_CH0_SRC_RLD_direct_probe = DMA0_CH0_SRC_RLD_direct;
	DMA0_CH0_DST_RLD_direct_probe = DMA0_CH0_DST_RLD_direct;
	DMA0_CH0_CNT_RLD_direct_probe = DMA0_CH0_CNT_RLD_direct;
}

void reset_adc()
{
	mxc_gpio_cfg_t gpio_out21;
	gpio_out21.port =MXC_GPIO_PORT_OUT0;
	gpio_out21.mask= MXC_GPIO_PIN_OUT21;
	gpio_out21.pad = MXC_GPIO_PAD_NONE;
	gpio_out21.func = MXC_GPIO_FUNC_OUT;
	gpio_out21.vssel = MXC_GPIO_VSSEL_VDDIO;
	MXC_GPIO_Config(&gpio_out21);
	MXC_GPIO_OutClr(gpio_out21.port,gpio_out21.mask); // start with ADC disabled
	MXC_Delay(100000);
	MXC_GPIO_OutSet(gpio_out21.port,gpio_out21.mask); // enable ADC
}




// set the ADC into host clock mode; this outputs a 20MHz sclk on the busy pin
// that can be used to drive SPI 1 in slave mode
// so we don't need to do this ourselves externally
// must connect the busy pin to spi 1 sclk (requires board hack)
void set_adc_host_clock_mode()
{
	int ret;
	uint16_t reg_addr = 0;
	// over-rides the GPIO control of SS, so do all the ADC register init writes first

	uint8_t SPI2_tx_3byte[3];
	uint8_t SPI2_rx_3byte[3];

	// configure gpio bit-bang AFTER spi2 init
	gpio_out16.port = MXC_GPIO_PORT_OUT0;
	gpio_out16.mask = MXC_GPIO_PIN_OUT16;
	gpio_out16.pad = MXC_GPIO_PAD_NONE;
	gpio_out16.func = MXC_GPIO_FUNC_OUT;
	gpio_out16.vssel = MXC_GPIO_VSSEL_VDDIO;
	gpio_out16.drvstr = MXC_GPIO_DRVSTR_3;
	ret = MXC_GPIO_Config(&gpio_out16);
	MXC_GPIO_OutSet(gpio_out16.port, gpio_out16.mask); // SS high

	ret = MXC_SPI_Init(MXC_SPI2, 1, 0, 1, 0, 5000000, MAP_A);
	if (ret != E_NO_ERROR) {
		   debug1 = 2;
		}

	//Setting the data size
	ret = MXC_SPI_SetDataSize(MXC_SPI2, 8);// 8 bits/char
	if (ret != E_NO_ERROR) {
		   debug1 = 2;
		}
	//Setting width of the SPI in this case 4 wire for spi2 master
	ret = MXC_SPI_SetWidth(MXC_SPI2, SPI_WIDTH_STANDARD);
	if (ret != E_NO_ERROR) {
		   debug1 = 2;
		}
	//Setting the SPI mode
	ret = MXC_SPI_SetMode(MXC_SPI2, SPI_MODE_0);
	if (ret != E_NO_ERROR) {
		   debug1 = 2;
		}

	// Master 2 register write parameters
	SPI2_req_master_ctrl_write.spi = MXC_SPI2;
	SPI2_req_master_ctrl_write.txData = (uint8_t *)SPI2_tx_3byte;
	SPI2_req_master_ctrl_write.rxData = (uint8_t *)SPI2_rx_3byte;
	SPI2_req_master_ctrl_write.txLen = 3;
	SPI2_req_master_ctrl_write.rxLen = 1;
	SPI2_req_master_ctrl_write.ssIdx = MXC_SPI2_SSIDX;
	SPI2_req_master_ctrl_write.ssDeassert = 1;
	SPI2_req_master_ctrl_write.txCnt = 0;
	SPI2_req_master_ctrl_write.rxCnt = 0;
	SPI2_req_master_ctrl_write.completeCB = NULL;

// put in host clk mode
	// Separating first byte of 16 bit address and making sure the first bit is 1 for read operation
	reg_addr = 0x3fff;
	SPI2_tx_3byte[0] = AD463X_REG_READ |((reg_addr >> 8) & 0x7F); // Separating first byte of 16 bit address
	SPI2_tx_3byte[1] = (uint8_t)reg_addr;  //Separating second byte of address
	SPI2_tx_3byte[2] = AD463X_REG_READ_DUMMY;
	//	**** Perform Transaction ****
	MXC_GPIO_OutClr(gpio_out16.port, gpio_out16.mask);
	ret = MXC_SPI_MasterTransaction(&SPI2_req_master_ctrl_write);
	if (ret != E_NO_ERROR) {
		   debug1 = 2;
		}
	MXC_GPIO_OutSet(gpio_out16.port, gpio_out16.mask);
	MXC_Delay(4);
// ** now write clock divider for host clock mode
	reg_addr = AD463X_REG_OSCILATOR; // 0x21
	SPI2_tx_3byte[0] = ((reg_addr >> 8) & 0x7F); // Separating first byte of 16 bit address
	SPI2_tx_3byte[1] = (uint8_t)reg_addr;  //Separating second byte of address
	SPI2_tx_3byte[2] = 0x02;
	//	**** Perform Transaction ****
	MXC_GPIO_OutClr(gpio_out16.port, gpio_out16.mask);
	MXC_Delay(4);
	MXC_SPI_MasterTransaction(&SPI2_req_master_ctrl_write);
	MXC_GPIO_OutSet(gpio_out16.port, gpio_out16.mask);
	MXC_Delay(4);
	// ** now write host clock mode
	// ** note, once this is set, an sclk burst will appear on BUSY, but only when Slave Sel falls
	reg_addr = AD463X_REG_MODES; // 0x20
	SPI2_tx_3byte[0] = ((reg_addr >> 8) & 0x7F); // Separating first byte of 16 bit address
	SPI2_tx_3byte[1] = (uint8_t)reg_addr;  //Separating second byte of address
	SPI2_tx_3byte[2] = 0x20;
	//SPI2_tx_3byte[2] = 0x00;
	//	**** Perform Transaction ****
	MXC_GPIO_OutClr(gpio_out16.port, gpio_out16.mask);
	MXC_Delay(4);
	MXC_SPI_MasterTransaction(&SPI2_req_master_ctrl_write);
	MXC_GPIO_OutSet(gpio_out16.port, gpio_out16.mask);
	MXC_Delay(4);


	// now exit host mode
	reg_addr = AD463X_REG_EXIT_CFG_MODE; // 0x14
	SPI2_tx_3byte[0] = ((reg_addr >> 8) & 0x7F); // Separating first byte of 16 bit address
	SPI2_tx_3byte[1] = (uint8_t)reg_addr;  //Separating second byte of address
	SPI2_tx_3byte[2] = AD463X_EXIT_CFG_MODE; // 0x01
	//	**** Perform Transaction ****
	MXC_GPIO_OutClr(gpio_out16.port, gpio_out16.mask);
	MXC_Delay(4);

	MXC_SPI_MasterTransaction(&SPI2_req_master_ctrl_write);
	MXC_Delay(4);
	MXC_GPIO_OutSet(gpio_out16.port, gpio_out16.mask);
	MXC_Delay(100);
	// now shutdown SPI2 and the bit-bang GPIO SS control
	MXC_SPI_Shutdown(MXC_SPI2);
	// make gpio tri-state so ADC SS cab be driven by ext FS soure
	gpio_out16.func = MXC_GPIO_FUNC_IN;
	MXC_GPIO_Config(&gpio_out16);
	MXC_GPIO_Shutdown(gpio_out16.mask); // let the ADC clock series resistor drive the SPI1_Slave SS pin

}




// spi 1 receives the stream adc input and is connected to the dma, so it must be a slave.
// warning, board hacks are required to make spi1 work as a slave.
// its complicated because spi2 is set up to write to the adc control registers
// using gpio bit-bang for the slave select, but in adc-streaming mode, the
// slave select must be driven externally. So we connect the adc convert signal
// (a square wave at fs) to the slave-select pin using a resistor, When the spi2 control
// writes are done, the gpio is tri-stated and the pin then is driven through the resistor.
// This works but is pretty hacky, and a better solution should be found using
// a modified timing generator using external logic, with a tri-state output that
// can be enabled sing a gpio, so the slave select pin can either drive the output or be driven
// from the timing generator

void spi1_init_slave()
{
	// over-rides the GPIO control of SS, so do all the ADC register init writes first
	SPI1_req.spi = MXC_SPI1;
	SPI1_req.txData = NULL;
	SPI1_req.rxData = (uint8_t *)SPI1_rx_3byte;
	SPI1_req.txLen = 0;
	SPI1_req.rxLen = 3; // chars
	SPI1_req.ssIdx = 0;
	SPI1_req.ssDeassert = 1;
	SPI1_req.txCnt = 0;
	SPI1_req.rxCnt = 0;
	SPI1_req.completeCB = NULL;

	MXC_SPI_Init(MXC_SPI1, 0, 0, 0, 0, 0, MAP_A);
	//Setting the data size
	MXC_SPI_SetDataSize(MXC_SPI1, 8);// bits/char, each spi trans takes in 3 chars = 24 bits
	//Setting width of the SPI in this case 3- wire SPI for SPI1 master
	MXC_SPI_SetWidth(MXC_SPI1, SPI_WIDTH_3WIRE);
	//Setting the SPI mode
	MXC_SPI_SetMode(MXC_SPI1, SPI_MODE_1);

	MXC_SPI_SlaveTransactionAsync(&SPI1_req); // complete the init; don't use the data!

	MXC_SPI_SetRXThreshold(MXC_SPI1, 24); // threshold of 24 bytes ( 8 samples of 3 bytes each) to trigger dma
	SPI1_CTRL0_direct &= 0xfffffffe; // disable the port
	MXC_SPI_ClearRXFIFO(MXC_SPI1); // clear the fifo, start only on pos edge of Slave-sel-B

}



//I2C callback function
void I2C_Callback(mxc_i2c_req_t *req, int error)
{
    I2C_FLAG = error;
}


void DMA_CALLBACK_func(int a, int b)
// this gets called by the DMA 1st, and when this returns, it goes directly to the DMA0_IRQHandler()
{

}



void init_dma_MXC()
{
	MXC_DMA_Init(MXC_DMA0);
	mychannel = MXC_DMA_AcquireChannel(MXC_DMA0);

	mxc_dma_srcdst_t dma_transfer;
	dma_transfer.ch = mychannel;
	dma_transfer.source = NULL;
	dma_transfer.dest = &dmaDestBuff[0];
	dma_transfer.len = DMA_buffLen_bytes; // 3 X 8K bytes

	mxc_dma_config_t dma_config;
	dma_config.ch = mychannel;
	dma_config.reqsel = MXC_DMA_REQUEST_SPI1RX;
	dma_config.srcwd = MXC_DMA_WIDTH_BYTE;
	dma_config.dstwd = MXC_DMA_WIDTH_BYTE;
	dma_config.srcinc_en = 0; // this is ignored??
	dma_config.dstinc_en = 1;

	mxc_dma_adv_config_t advConfig;
	advConfig.ch = mychannel;
	advConfig.prio = MXC_DMA_PRIO_HIGH;
	advConfig.reqwait_en = 0;
	advConfig.tosel = MXC_DMA_TIMEOUT_4_CLK;
	advConfig.pssel = MXC_DMA_PRESCALE_DISABLE;
	advConfig.burst_size = 24;


	MXC_DMA_ConfigChannel(dma_config, dma_transfer);
	MXC_DMA_AdvConfigChannel(advConfig);
	MXC_DMA_SetSrcDst(dma_transfer); // is this redundant??
	MXC_DMA_SetSrcReload(dma_transfer);// is this redundant??
	MXC_DMA_SetChannelInterruptEn(mychannel, false, true); // ctz
	MXC_DMA_SetCallback(mychannel, DMA_CALLBACK_func);
	MXC_DMA_EnableInt(mychannel);

}



void write_wav_header(FIL *file) { // from chapGPT
	// 44 byte header with zero-fill
    uint8_t header[WAV_BLOCK_SIZE];
    //zero-fill oit to 512 bytes so that all block SD card writes align to sector size
    // note, the zero-filling occurs after the 44-byte wav header, 512 -44 = 468,
    //which is a multiple of both 3 (for 24-bit) and 2 (for 16-bit); required to not mess up
    // the byte alignment when the real data starts

    // Initialize header with default values
    memset(header, 0, WAV_BLOCK_SIZE);
    // RIFF header
    memcpy(header, "RIFF", 4); // Chunk ID
    // Chunk Size (placeholder, to be updated later)
    // Format
    memcpy(header + 8, "WAVE", 4);

    // fmt subchunk
    memcpy(header + 12, "fmt ", 4); // Subchunk1 ID
    *(uint32_t *)(header + 16) = 16; // Subchunk1 Size (16 for PCM)
    *(WORD *)(header + 20) = 1; // Audio Format (1 for PCM)
    *(WORD *)(header + 22) = 1; // Num Channels (1 for mono)
    // magpie_new write header according to fs and bit depth
    if(magpie_bitdepth == 0) { // 16 bits

			*(uint32_t *)(header + 24) = FS; // Sample Rate (44.1 kHz)
			*(uint32_t *)(header + 28) = FS * 1 * 2; // Byte Rate (Sample Rate * Num Channels * BitsPerSample/8)
			*(WORD *)(header + 32) = 1 * 2; // Block Align (Num Channels * BitsPerSample/8)
			*(WORD *)(header + 34) = 16; // Bits Per Sample (16 bits)
    }
    if(magpie_bitdepth == 1) { // 24 nits
			*(uint32_t *)(header + 24) = FS; // Sample Rate (44.1 kHz)
			  *(uint32_t *)(header + 28) = FS * 1 * 3; // Byte Rate (Sample Rate * Num Channels * BitsPerSample/8)
			  *(WORD *)(header + 32) = 1 * 3; // Block Align (Num Channels * BitsPerSample/8)
			  *(WORD *)(header + 34) = 24; // Bits Per Sample (16 bits)

    }

    // data subchunk
    memcpy(header + 36, "data", 4); // Subchunk2 ID
    // Subchunk2 Size, 4 bytes (40-43) (placeholder, to be updated later)


    // note the rest of the 512 bytes are already set to 0, so just write the whole block at once

    // Write header to file
    u_int32_t bw;
    f_write(file, header, WAV_BLOCK_SIZE, &bw);
    debug2 = (uint32_t)bw;
    debug1 = 10;
}


// after wav file is closed, go back and find the size, and fill in the missing data in the header
void update_wav_header(FIL *file) { // from CHATgpt
    uint32_t fileSize = f_size(file);
    uint32_t chunkSize = fileSize - 8;
    uint32_t subchunk2Size = fileSize - WAV_HEADER_SIZE;

    // Seek to Chunk Size position and update
    f_lseek(file, 4);
    UINT bw;
    f_write(file, &chunkSize, 4, &bw);

    // Seek to Subchunk2 Size position and update
    f_lseek(file, 40);
    f_write(file, &subchunk2Size, 4, &bw);
}

// function to write a metadata chunk at the end oft the audio file (LIST chunk with INFO sub-chunnk)
void  writeListChunk(FIL *file,const char *chunkId,const char *metadata) { // kinda from CHATgpt

	UINT bw;
	uint32_t chunkSize=4+strlen(chunkId)+1+strlen(metadata)+1;
	uint32_t fileSize = f_size(file);
	f_lseek(file, fileSize);
	f_write(file,"LIST",4,&bw);
	f_write(file,&chunkSize,4,&bw);
	f_write(file,"INFO",4,&bw);
	f_write(file,chunkId,4,&bw);
	uint32_t metadataSize=strlen(metadata)+1;
	f_write(file,&metadataSize,4,&bw);
	f_write(file,metadata,metadataSize,&bw);
	if(metadataSize % 2 != 0) {
		f_write(file,(uint8_t)0,1,&bw);
	}
}


void copy_dec_by_2(const q31_t *A, q31_t *B,uint32_t size) {
	uint32_t i=0;
	uint32_t j = size/2;
	do {
		B[i] = A[2*i];
		B[i+1] = A[2*i+2];
		B[i+2] = A[2*i+4];
		B[i+3] = A[2*i+6];
		i+=4;

	} while(--j);
}

// ************************* This is where all the work gets done.
// *** 8k input sample buffer appears here and persists for 20ms ***
void DMA0_IRQHandler()
{

	uint32_t k,i,j;
	uint8_t dmaByte2,dmaByte1,dmaByte0;
    int flags;
	//MXC_GPIO_OutSet(gpio_out5.port,gpio_out5.mask); // timing test
    //memcpy(dmaDestBuffCopy,dmaDestBuff,DMA_buffLen_bytes);

    MXC_DMA_Handler(MXC_DMA0);
    flags = MXC_DMA_ChannelGetFlags(mychannel); // clears the cfg enable bit
    MXC_DMA_ChannelClearFlags(mychannel, flags);


	k = DMA_buffLen; // loop counter
	i = 0; // byte pointer
	j=0; // word pointer
	//MXC_GPIO_OutSet(gpio_out5.port,gpio_out5.mask); // timing test

	// DMA byte-to-signed 32 bit word assembly process
	// Note that if the DMA streaming is turned off to run other processes, you
	// will need to use the "stall" mechanism to turn it back on (see main()) in order to insure
	// that you don't start off with partially-written words (1 or 2 bytes) in the SPI fifo

	// do the fast fir filtering; must be compiled with 02 effort
	//I use a multi-rate filter for fastest speed,2:1 or 3:1 for each stage


	// magpie_new - everything in the isr
	// don't over-write dmaDestBuff_32bit in test mode (it gets it's value from an include file)
//#ifndef TEST_DECIMATE

	while(k > 0) { // all rates other than 384k, convert to q31
		// it's good to read the memory from the bottom up, because the low memory
		// will be the first to be over-written with new samples
		dmaByte2 = dmaDestBuff[i++]; //ms byte
		dmaByte1 = dmaDestBuff[i++]; //mid byte
		dmaByte0 = dmaDestBuff[i++]; //ls byte
		dmaDestBuff_32bit[j++]  = (q31_t)((dmaByte2 << 24) | (dmaByte1 << 16) | (dmaByte0 << 8)); // use for 24-bit case
		k--;

	}



//#endif

	switch(magpie_FS) { // do the correct filter for each sample-rate

		case fs_16k: // timing test, 5ms
			MXC_GPIO_OutSet(gpio_out5.port,gpio_out5.mask); // timing test
			arm_fir_decimate_fast_q31_bob(&Sdeci_16k_0,dmaDestBuff_32bit,deci_stage0_out,DMA_buffLen);// use 2x buffer to save mem (dont need a 3x buffer)
			arm_fir_decimate_fast_q31_HB(&Sdeci_16k_1,deci_stage0_out,deci_stage1_out,buffLen_deci3x);
			arm_fir_decimate_fast_q31_HB(&Sdeci_16k_2,deci_stage1_out,deci_stage2_out,buffLen_deci6x);
			arm_fir_decimate_fast_q31_HB(&Sdeci_16k_3,deci_stage2_out,deci_stage3_out,buffLen_deci12x);
			data_converters_q31_to_i16_24(deci_stage3_out,SD_write_buff+offsetDMA,buffLen_deci24x,magpie_bitdepth);
			MXC_GPIO_OutClr(gpio_out5.port,gpio_out5.mask); // timing test
			break;
		case fs_24k: // timing test 6.5 ms
			MXC_GPIO_OutSet(gpio_out5.port,gpio_out5.mask); // timing test
			arm_fir_decimate_fast_q31_bob(&Sdeci_24k_0,dmaDestBuff_32bit,deci_stage0_out,DMA_buffLen);
			arm_fir_decimate_fast_q31_HB(&Sdeci_24k_1,deci_stage0_out,deci_stage1_out,buffLen_deci2x);
			arm_fir_decimate_fast_q31_HB(&Sdeci_24k_2,deci_stage1_out,deci_stage2_out,buffLen_deci4x);
			arm_fir_decimate_fast_q31_HB(&Sdeci_24k_3,deci_stage2_out,deci_stage3_out,buffLen_deci8x);
			data_converters_q31_to_i16_24(deci_stage3_out,SD_write_buff+offsetDMA,buffLen_deci16x,magpie_bitdepth);
			MXC_GPIO_OutClr(gpio_out5.port,gpio_out5.mask); // timing test

			break;
		case fs_32k: // timing test 4.8 ms
			MXC_GPIO_OutSet(gpio_out5.port,gpio_out5.mask); // timing test
			arm_fir_decimate_fast_q31_bob(&Sdeci_32k_0,dmaDestBuff_32bit,deci_stage0_out,DMA_buffLen); // use 2x buffer to save mem (dont need a 3x buffer)
			arm_fir_decimate_fast_q31_HB(&Sdeci_32k_1,deci_stage0_out,deci_stage1_out,buffLen_deci3x);
			arm_fir_decimate_fast_q31_HB(&Sdeci_32k_2,deci_stage1_out,deci_stage2_out,buffLen_deci6x);
			data_converters_q31_to_i16_24(deci_stage2_out,SD_write_buff+offsetDMA,buffLen_deci12x,magpie_bitdepth);
			MXC_GPIO_OutClr(gpio_out5.port,gpio_out5.mask); // timing test

			break;
		case fs_48k:
			// timing test, 03 or Ofast, 8 ms w/o halfband, 6ms with halfband, conversion takes only 0.1ms
			// timing test, 02, 8 ms w/o halfband, 6.5ms with halfband, conversion takes only 0.1ms

			MXC_GPIO_OutSet(gpio_out5.port,gpio_out5.mask); // timing test

			//data_converters_i24_to_q31(dmaDestBuff, dmaDestBuff_32bit, DMA_buffLen_bytes);
			// test the halfband, length 7
			arm_fir_decimate_fast_q31_bob(&Sdeci_48k_0,dmaDestBuff_32bit,deci_stage0_out,DMA_buffLen);
			arm_fir_decimate_fast_q31_HB(&Sdeci_48k_1,deci_stage0_out,deci_stage1_out,buffLen_deci2x);
			arm_fir_decimate_fast_q31_HB(&Sdeci_48k_2,deci_stage1_out,deci_stage2_out,buffLen_deci4x);

			data_converters_q31_to_i16_24(deci_stage2_out,SD_write_buff+offsetDMA,buffLen_deci8x,magpie_bitdepth);
			MXC_GPIO_OutClr(gpio_out5.port,gpio_out5.mask); // timing test

			break;
		case fs_96k: // timing test 6.7ms
			MXC_GPIO_OutSet(gpio_out5.port,gpio_out5.mask); // timing test

			//data_converters_i24_to_q31(dmaDestBuff, dmaDestBuff_32bit, DMA_buffLen_bytes);
			arm_fir_decimate_fast_q31_bob(&Sdeci_96k_0,dmaDestBuff_32bit,deci_stage0_out,DMA_buffLen);
			arm_fir_decimate_fast_q31_HB(&Sdeci_96k_1,deci_stage0_out,deci_stage1_out,buffLen_deci2x);
			data_converters_q31_to_i16_24(deci_stage1_out,SD_write_buff+offsetDMA,buffLen_deci4x,magpie_bitdepth);
			MXC_GPIO_OutClr(gpio_out5.port,gpio_out5.mask); // timing test

			break;
		case fs_192k: // timing test 5.6 ms
			MXC_GPIO_OutSet(gpio_out5.port,gpio_out5.mask); // timing test
			//data_converters_i24_to_q31(dmaDestBuff, dmaDestBuff_32bit, DMA_buffLen_bytes);
			arm_fir_decimate_fast_q31_bob(&Sdeci_192k_0,dmaDestBuff_32bit,deci_stage0_out,DMA_buffLen);
			data_converters_q31_to_i16_24(deci_stage0_out,SD_write_buff+offsetDMA,buffLen_deci2x,magpie_bitdepth);
			MXC_GPIO_OutClr(gpio_out5.port,gpio_out5.mask); // timing test

			break;
		case fs_384k: // no filtering, just copy the bytes
			k = DMA_buffLen; // loop counter
			i = 0; // byte pointer
			j=0; // word pointer
			while(k > 0) {
				SD_write_buff[i+2 + offsetDMA] = (dmaDestBuff_32bit[j] >> 24) & 0x000000ff; //ms byte
				SD_write_buff[i+1 + offsetDMA] =  (dmaDestBuff_32bit[j] >> 16) & 0x000000ff;//mid byte
				SD_write_buff[i + offsetDMA] = (dmaDestBuff_32bit[j] >> 8) & 0x000000ff; //ls byte
				i+=3;
				k--;
				j++;

			}

			break;
		default:
			break;
	}



	//	MXC_GPIO_OutClr(gpio_out5.port,gpio_out5.mask); // timing test

	// timing test results; the 3 filters above take about 9ms, and the dma byte-to-signed-data conversion
	// takes about 2ms. since we have 21.3 ms per DMA frame, we have about 10ms left.
	// this should be plenty of time to blast out data to the the SD cards (I hope!)
	// Note, this result is obtained with -o2 or -o3 compiler effort.
	// With -o1 effort, the times above are worse by almost 2x (leaving very little time for anything else)
	// With standard compiler effort, the dma processing does not finish at all!
//

	blockPtrModuloDMA = (blockPtrModuloDMA+1) & block_ptr_modulo_mask; // wraps to fit in spi write mem
	offsetDMA = blockPtrModuloDMA*numBytesSDwrite;
	dataBlocksDmaCount+= 1;

    count_dma_irq++;

	//MXC_GPIO_OutClr(gpio_out5.port,gpio_out5.mask); // timing test


    // get ready for next dma transfer
    DMA0_CH0_CFG_direct |= 0x3; // enable dma and reload bits
    DMA0_CH0_CNT_RLD_direct |= 0x80000000; // redundant, do I have to do it again here?



}



/*********************************************************************************************
*
* @name     DS3231_I2C_init
*
* @brief Function to initialize the USCI B1 peripheral. Setting this up
*           as a I2C bus.
*
*           P1_15 - SCL
*           P1_16 - SDA
*
* @return DS3231_i2cError enum type:   DS3231_I2C_NO_ERROR, 
*                                      DS3231_I2C_INIT_ERROR,
*                                      DS3231_I2C_FREQSET_ERROR
*
**********************************************************************************************/
DS3231_i2cError DS3231_I2C_init()
{
  if (MXC_I2C_Init(MAX32666_I2C_BUS_3V3_PULLUPS, MAX32666_I2C_CFG_MASTER_MODE, 0) != E_NO_ERROR)
  {
	printf("Unable to initialize I2C.\n");
    return MCU_MASTER_I2C_INIT_ERROR;
  }

  // I2C pins default to VDDIO for the logical high voltage, we want VDDIOH for 3.3v pullups
  const mxc_gpio_cfg_t i2c_2_pins = {
      .port = MXC_GPIO1,
      .mask = (MXC_GPIO_PIN_14 | MXC_GPIO_PIN_15),
      .pad = MXC_GPIO_PAD_NONE,
      .func = MXC_GPIO_FUNC_ALT1,
      .vssel = MXC_GPIO_VSSEL_VDDIOH,
      .drvstr = MXC_GPIO_DRVSTR_0,
  };
  MXC_GPIO_Config(&i2c_2_pins);

  if (MXC_I2C_SetFrequency(MAX32666_I2C_BUS_3V3_PULLUPS, MAX32666_I2C_CLK_SPEED) != MAX32666_I2C_CLK_SPEED)
  {
	printf("Unable to set I2C frequency.\n");
    return DS3231_I2C_FREQSET_ERROR;
  }

  DS3231_RTC = DS3231_Open();

  if(E_NO_ERROR != DS3231_RTC.init(MAX32666_I2C_BUS_3V3_PULLUPS))
  {
	printf("Unable to initialize RTC driver.\n");
	return DS3231_I2C_INIT_ERROR;
  }
 
  return DS3231_I2C_NO_ERROR;
}

FRESULT set_timestamp (
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

void MB_LED(u_int8_t state)
{
	if(0 == state){

	// gpio 5 is pin 6 on the feather header, connected to motherboard green LED
		gpio_out5.port =MXC_GPIO_PORT_OUT0;
		gpio_out5.mask= MXC_GPIO_PIN_OUT5;
		gpio_out5.pad = MXC_GPIO_PAD_NONE;
		gpio_out5.func = MXC_GPIO_FUNC_OUT;
		gpio_out5.vssel = MXC_GPIO_VSSEL_VDDIO;
		MXC_GPIO_Config(&gpio_out5);
		MXC_GPIO_OutSet(gpio_out5.port,gpio_out5.mask); // LED OFF
	} else {
		gpio_out5.port =MXC_GPIO_PORT_OUT0;
		gpio_out5.mask= MXC_GPIO_PIN_OUT5;
		gpio_out5.pad = MXC_GPIO_PAD_NONE;
		gpio_out5.func = MXC_GPIO_FUNC_OUT;
		gpio_out5.vssel = MXC_GPIO_VSSEL_VDDIO;
		MXC_GPIO_Config(&gpio_out5);
		MXC_GPIO_OutClr(gpio_out5.port,gpio_out5.mask); // LED ON
	}
	
}

/*=================================================================================
*]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
*
*								MAIN MODULE
*
*]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
===================================================================================*/

int main(void)
{
	char metaBuffer[32] = {0};
	MXC_Delay(MXC_DELAY_SEC(1));
	mxc_sdhc_cfg_t cfg;
	FF_ERRORS[0] = "FR_OK";
	FF_ERRORS[1] = "FR_DISK_ERR";
	FF_ERRORS[2] = "FR_INT_ERR";
	FF_ERRORS[3] = "FR_NOT_READY";
	FF_ERRORS[4] = "FR_NO_FILE";
	FF_ERRORS[5] = "FR_NO_PATH";
	FF_ERRORS[6] = "FR_INVLAID_NAME";
	FF_ERRORS[7] = "FR_DENIED";
	FF_ERRORS[8] = "FR_EXIST";
	FF_ERRORS[9] = "FR_INVALID_OBJECT";
	FF_ERRORS[10] = "FR_WRITE_PROTECTED";
	FF_ERRORS[11] = "FR_INVALID_DRIVE";
	FF_ERRORS[12] = "FR_NOT_ENABLED";
	FF_ERRORS[13] = "FR_NO_FILESYSTEM";
	FF_ERRORS[14] = "FR_MKFS_ABORTED";
	FF_ERRORS[15] = "FR_TIMEOUT";
	FF_ERRORS[16] = "FR_LOCKED";
	FF_ERRORS[17] = "FR_NOT_ENOUGH_CORE";
	FF_ERRORS[18] = "FR_TOO_MANY_OPEN_FILES";
	FF_ERRORS[19] = "FR_INVALID_PARAMETER";
	srand(12347439);
	int run = 1, input = -1;

	int ret=0;

	static uint8_t stall;
	static uint32_t ktrace = 0;

    printf("Initializing .....\n");
  
	//Init DS3231 RTC peripheral
	if(DS3231_I2C_NO_ERROR != DS3231_I2C_init())
	{
		printf("Unable to initialize DS3231 driver.\n");
		LED_On(LED_RED);
		return 1;
	}

	#ifdef FIRST_SET_RTC
	// //Set Date Time to something
	//Year is always Year - 1900
	//Month is 0-11 so subtract 1 from the month you want to set
	//Time is in UTC so set appropriately
	// hour is 0-23
	// min is 0-59
	// sec is 0-59
	struct tm newTime = {
		.tm_year = 2024 - 1900U,
		.tm_mon = 7 - 1U,
		.tm_mday = 19,
		.tm_hour = 19,
		.tm_min = 36,
		.tm_sec = 0
	};

	
	//Set Date Time on RTC. 
	
	if (E_NO_ERROR != DS3231_RTC.set_datetime(&newTime)) {
		printf("\nDS3231 set time error\n");
	} else {
		strftime((char*)output_msgBuffer, OUTPUT_MSG_BUFFER_SIZE, "\n-->Set DateTime: %F %TZ\r\n", &newTime);
		printf(output_msgBuffer);
	}
	#endif

	//Get Date Time from RTC
	
	if (E_NO_ERROR != DS3231_RTC.read_datetime(&ds3231_datetime, ds3231_datetime_str)) {
		printf("\nDS3231 read datetime error\n");
	} else {
		strftime((char*)output_msgBuffer, OUTPUT_MSG_BUFFER_SIZE, "\n-->DateTime: %F %TZ\r\n", &ds3231_datetime);
		printf(output_msgBuffer);

		strftime((char*)output_msgBuffer, OUTPUT_MSG_BUFFER_SIZE, "\n-->FileStampTime: %Y%m%d_%H%M%SZ\r\n", &ds3231_datetime);
		printf(output_msgBuffer);

		printf(ds3231_datetime_str);		

	}
	//time_t currentTime = mktime(ds3231_datetime);

	//Get Temperature from RTC	
	if (E_NO_ERROR != DS3231_RTC.read_temperature(&ds3231_temperature)) {
		printf("\nDS3231 read temperature error\n");
	} else {
		sprintf((char*)output_msgBuffer, "\n-->Temperature: %.2f\r\n", ds3231_temperature);
		printf(output_msgBuffer);
	}
		
	
// magpie_new - init all the decimation filters. This allows you to change fs without re-compiling
	arm_fir_decimate_init_q31(&Sdeci_16k_0,deci_16k_numcoeffs_0,3, &firCoeffs_16k_0[0],&firState_stage0[0],DMA_buffLen);
	arm_fir_decimate_init_q31(&Sdeci_16k_1,deci_16k_numcoeffs_1,2, &firCoeffs_16k_1[0],&firState_stage1[0],buffLen_deci2x);
	arm_fir_decimate_init_q31(&Sdeci_16k_2,deci_16k_numcoeffs_2,2, &firCoeffs_16k_2[0],&firState_stage2[0],buffLen_deci4x);
	arm_fir_decimate_init_q31(&Sdeci_16k_3,deci_16k_numcoeffs_3,2, &firCoeffs_16k_3[0],&firState_stage3[0],buffLen_deci8x);

	arm_fir_decimate_init_q31(&Sdeci_24k_0,deci_24k_numcoeffs_0,2, &firCoeffs_24k_0[0],&firState_stage0[0],DMA_buffLen);
	arm_fir_decimate_init_q31(&Sdeci_24k_1,deci_24k_numcoeffs_1,2, &firCoeffs_24k_1[0],&firState_stage1[0],buffLen_deci2x);
	arm_fir_decimate_init_q31(&Sdeci_24k_2,deci_24k_numcoeffs_2,2, &firCoeffs_24k_2[0],&firState_stage2[0],buffLen_deci4x);
	arm_fir_decimate_init_q31(&Sdeci_24k_3,deci_24k_numcoeffs_3,2, &firCoeffs_24k_3[0],&firState_stage3[0],buffLen_deci8x);

	arm_fir_decimate_init_q31(&Sdeci_32k_0,deci_32k_numcoeffs_0,3, &firCoeffs_32k_0[0],&firState_stage0[0],DMA_buffLen);
	arm_fir_decimate_init_q31(&Sdeci_32k_1,deci_32k_numcoeffs_1,2, &firCoeffs_32k_1[0],&firState_stage1[0],buffLen_deci2x);
	arm_fir_decimate_init_q31(&Sdeci_32k_2,deci_32k_numcoeffs_2,2, &firCoeffs_32k_2[0],&firState_stage2[0],buffLen_deci4x);

	arm_fir_decimate_init_q31(&Sdeci_48k_0,deci_48k_numcoeffs_0,2, &firCoeffs_48k_0[0],&firState_stage0[0],DMA_buffLen);
	arm_fir_decimate_init_q31(&Sdeci_48k_1,deci_48k_numcoeffs_1,2, &firCoeffs_48k_1[0],&firState_stage1[0],buffLen_deci2x);
	arm_fir_decimate_init_q31(&Sdeci_48k_2,deci_48k_numcoeffs_2,2, &firCoeffs_48k_2[0],&firState_stage2[0],buffLen_deci4x);

	arm_fir_decimate_init_q31(&Sdeci_96k_0,deci_96k_numcoeffs_0,2, &firCoeffs_96k_0[0],&firState_stage0[0],DMA_buffLen);
	arm_fir_decimate_init_q31(&Sdeci_96k_1,deci_96k_numcoeffs_1,2, &firCoeffs_96k_1[0],&firState_stage1[0],buffLen_deci2x);

	arm_fir_decimate_init_q31(&Sdeci_192k_0,deci_192k_numcoeffs_0,2, &firCoeffs_192k_0[0],&firState_stage0[0],DMA_buffLen);

	// magpie_new - set sample-rate and bit depth
	//******************* set sample rate ************************
	magpie_FS =fs_384k; // use this to set sample rate; the variable FS is also set, for writing the wav header file
	//*************************************************************

	//******************* set bit depth, 1=24 bits, 0=16 bits ************************
	magpie_bitdepth = 1;
	// *******************************************************************************

	// check for invalid condition (384k, 16 bits)
	if(magpie_FS == fs_384k && !magpie_bitdepth) {
		printf("Error: Invalid Decimation Conditions.\n");
		LED_On(LED_RED);
		while(1) {}
	}

	// magpie_new - set the number of bytes to write according to the sample-rate; also set the modulo for the spi slow-write scheme
	switch(magpie_FS) {

		case fs_16k:
			FS = 16000;
			if(magpie_bitdepth) numBytesSDwrite = 3*buffLen_deci24x; else numBytesSDwrite = 2*buffLen_deci24x;
			block_ptr_modulo_mask = 0x00000007;// lower fs means smaller writes, which allows more blocks to be stored in the SD_write buffer
			break;
		case fs_24k:
			FS = 24000;
			if(magpie_bitdepth) numBytesSDwrite = 3*buffLen_deci16x; else numBytesSDwrite = 2*buffLen_deci16x;
			block_ptr_modulo_mask = 0x00000007;// lower fs means smaller writes, which allows more blocks to be stored in the SD_write buffer
			break;
		case fs_32k:
			FS = 32000;
			if(magpie_bitdepth) numBytesSDwrite = 3*buffLen_deci12x; else numBytesSDwrite = 2*buffLen_deci12x;
			block_ptr_modulo_mask = 0x000000007;// lower fs means smaller writes, which allows more blocks to be stored in the SD_write buffer
			break;
		case fs_48k:
			FS = 48000;
			if(magpie_bitdepth) numBytesSDwrite = 3*buffLen_deci8x; else numBytesSDwrite = 2*buffLen_deci8x;
			block_ptr_modulo_mask = 0x000000007;// lower fs means smaller writes, which allows more blocks to be stored in the SD_write buffer
			break;
		case fs_96k:
			FS = 96000;
			if(magpie_bitdepth) numBytesSDwrite = 3*buffLen_deci4x; else numBytesSDwrite = 2*buffLen_deci4x;
			block_ptr_modulo_mask = 0x00000007;// lower fs means smaller writes, which allows more blocks to be stored in the SD_write buffer
			break;
		case fs_192k:
			FS = 192000;
			if(magpie_bitdepth) numBytesSDwrite = 3*buffLen_deci2x; else numBytesSDwrite = 2*buffLen_deci2x;
			block_ptr_modulo_mask = 0x0000003;// lower fs means smaller writes, which allows more blocks to be stored in the SD_write buffer
			break;
		case fs_384k:
			FS = 384000;
			numBytesSDwrite = DMA_buffLen_bytes; // note, 384k and 16 bits not supported (yet)
			block_ptr_modulo_mask = 0x00000003;// this case needs the most sdwrite memory, up to 4*(dma_length in bytes)
			break;

		default:

			break;
	}


	//*******************************************



	//CARD writes, cluster size (== allocation size) may be 128Kb, whereas sector size is 512 bytes
	cfg.bus_voltage = MXC_SDHC_Bus_Voltage_3_3;
	cfg.block_gap = 0;
	cfg.clk_div = 0x0b0; // Maximum divide ratio, frequency must be <= 400 kHz during Card Identification phase
	// 7us clock period = 130khz, OK
	if (MXC_SDHC_Init(&cfg) != E_NO_ERROR)
	{
		printf("Unable to initialize SDHC driver.\n");
		LED_On(LED_RED);
		return 1;
	}

	
	// wait for card to be inserted
		while (!MXC_SDHC_Card_Inserted()) {
			LED_On(LED_RED);
			MXC_Delay(500000);
			LED_Off(LED_RED);
			MXC_Delay(500000);
		}
	printf("Card inserted.\n");
	LED_On(LED_GREEN);

	//////// FTHR2 SDCARD Enabling ////

    const mxc_gpio_cfg_t sd_card_en_pin = {
    .port = MXC_GPIO1,
    .mask = MXC_GPIO_PIN_6,
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_OUT,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
    .drvstr = MXC_GPIO_DRVSTR_0,
	};

	MXC_GPIO_Config(&sd_card_en_pin);
	MXC_GPIO_OutSet(sd_card_en_pin.port, sd_card_en_pin.mask);

	//////////////////////////////////

	// set up card to get it ready for a transaction
	if (MXC_SDHC_Lib_InitCard(10) == E_NO_ERROR) {
		printf("Card Initialized.\n");
		debug1 = 10;
	} else {
		printf("No card response! Remove card, reset MCU, and try again.\n");
		debug1 = 11;
		LED_Off(LED_GREEN);
		LED_On(LED_RED);
		return -1;
	}

	if (MXC_SDHC_Lib_Get_Card_Type() == CARD_SDHC) {
		printf("Card type: SDHC\n");
		debug1 = 12;
	} else {
		printf("Card type: MMC/eMMC\n");
		debug1 = 13;
	}

	/* Configure for fastest possible clock, must not exceed 52 MHz for eMMC */


	// skip formatting
	// for now, format the card every time, so we just make a single file per pass
//	MKFS_PARM format_options = { .fmt = FM_EXFAT };
//
//	if ((err = f_mkfs("", &format_options, work, sizeof(work))) != FR_OK) { //Format the default drive to FAT32
//		printf("Error formatting SD card: %s\n", FF_ERRORS[err]);
//		debug1 = 10;
//	} else {
//		printf("Drive formatted.\n");
//		debug1 = 10;
//	}

	//Turn ON LED to indicate we are starting the recording.
	LED_Off(LED_RED);
	LED_Off(LED_BLUE);
	LED_Off(LED_GREEN);
	for (size_t i = 0; i < 5; i++)
	{
		LED_On(LED_BLUE);
		MXC_Delay(500000);
		LED_Off(LED_BLUE);
		MXC_Delay(500000);
	}
	printf("Start recording.\n");

	LED_On(LED_GREEN);     
	MB_LED(1);

	// mount the card
	mount();
	// create a file

	//Get Date Time from RTC 
	if(E_NO_ERROR != DS3231_RTC.read_datetime(&ds3231_datetime, ds3231_datetime_str))
	{
		sprintf(savedFileName,"%s",DEFAULT_FILENAME);
	} else {
		
		sprintf(savedFileName,"%s%s%s","0:Magpie00_",ds3231_datetime_str,".wav");
	}
	

	if ((err = f_open(&file, savedFileName, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK)
	{
		printf("Error opening file: %s\n", FF_ERRORS[err]);
		f_mount(NULL, "", 0);
		return err;
	}
	debug1=11;

	//Writing  Audio header file

	write_wav_header(&file);
	


	debug1= 16;

	// blue led pin 5 on feather
	// gpio_in30.port =MXC_GPIO_PORT_OUT0;
	// gpio_in30.mask= MXC_GPIO_PIN_OUT30;
	// gpio_in30.pad = MXC_GPIO_PAD_NONE;
	// gpio_in30.func = MXC_GPIO_FUNC_OUT;
	// gpio_in30.vssel = MXC_GPIO_VSSEL_VDDIO;
	// MXC_GPIO_Config(&gpio_in30);
	// MXC_GPIO_OutClr(gpio_in30.port,gpio_in30.mask); // set LOW (Led on)


// adc fs clock enable. Note after the spi_init_slave, this will be over-written by the spi definition and will go high
	gpio_out20.port =MXC_GPIO_PORT_OUT0;
	gpio_out20.mask= MXC_GPIO_PIN_OUT20;
	gpio_out20.pad = MXC_GPIO_PAD_NONE;
	gpio_out20.func = MXC_GPIO_FUNC_OUT;
	gpio_out20.vssel = MXC_GPIO_VSSEL_VDDIO;
	MXC_GPIO_Config(&gpio_out20);
	MXC_GPIO_OutClr(gpio_out20.port,gpio_out20.mask); // start with adc clock disabled



	// gpio 3 is pin 5 on the feather header and blue LED on motherboard
	// we will disconnect the blue LED and use this gpio as in input from spi1 slave select
	// so we can syncronize starting the spi port with rising slave-sel-B
	gpio_in3.port =MXC_GPIO_PORT_OUT0;
	gpio_in3.mask= MXC_GPIO_PIN_OUT3;
	gpio_in3.pad = MXC_GPIO_PAD_NONE;
	gpio_in3.func = MXC_GPIO_FUNC_IN;
	gpio_in3.vssel = MXC_GPIO_VSSEL_VDDIO;
	MXC_GPIO_Config(&gpio_in3);

	// gpio 5, pin6 on feather, used for code timing tests
	// gpio_out5.port = MXC_GPIO_PORT_OUT0;
	// gpio_out5.mask = MXC_GPIO_PIN_OUT5;
	// gpio_out5.pad = MXC_GPIO_PAD_NONE;
	// gpio_out5.func = MXC_GPIO_FUNC_OUT;
	// gpio_out5.vssel = MXC_GPIO_VSSEL_VDDIO;
	// gpio_out5.drvstr = MXC_GPIO_DRVSTR_2;
	// ret = MXC_GPIO_Config(&gpio_out5);



	// ***  set up I2C, write address 0x98, read addr 0x99

	// i2cErr = MXC_I2C_Init(I2C_MASTER, 1, 0);
	// mxc_i2c_req_t reqMaster;
	// reqMaster.i2c = I2C_MASTER;
	// reqMaster.addr = I2C_SLAVE_ADDR;
	// reqMaster.tx_buf = i2c_txdata;
	// reqMaster.tx_len = I2C_BYTES;
	// reqMaster.rx_buf = i2c_rxdata;
	// reqMaster.rx_len = 0;
	// reqMaster.restart = 0;
	// reqMaster.callback = NULL;
	// reqMaster.callback = I2C_Callback;
	// I2C_FLAG = 1;
	// MXC_I2C_SetFrequency(I2C_MASTER, I2C_FREQ);
	// i2cErr = MXC_I2C_MasterTransaction(&reqMaster);
	// MXC_Delay(10000);

	//reset_adc();
	// MXC_Delay(200000);

	set_adc_host_clock_mode();
	MXC_Delay(100000);
	MXC_GPIO_OutSet(gpio_out20.port,gpio_out20.mask); // turn on adc clock

	spi1_init_slave();
	// over-rides the GPIO control of SS, so do all the ADC register init writes first
	// note this sets the adc clock enable gpio high accidentally, but its already ON
	debug1 = 11;
	// enable the DMA interupts
	//__enable_irq();
	NVIC_EnableIRQ(DMA0_IRQn);

	init_dma_MXC();

//	PWMTimer();
//	read_TMR0_regs();
	// note, the following could probably be done with MXC functions
	SPI1_DMA_direct = 0b00000000010110000000000000000000; // 24 bytes, enable rx fifo
	SPI1_DMA_direct |= 0x80000000;// receive dma enable


	stall = 1;
	while(stall) { // stall until a rising edge on slave-sel-B. This is to insure we have no partial writes (1 or 2 bytes) that mess up the dma
		temp1  = MXC_GPIO_InGet(gpio_in3.port,gpio_in3.mask); // L
		temp2 =  MXC_GPIO_InGet(gpio_in3.port,gpio_in3.mask); // H
                 		stall = (!temp2 || temp1);

		printf("Stalling temp1: %d, temp2: %d.\n",temp1,temp2);
	}
	printf("Partial Writes cleared ...\n\n");

	SPI1_CTRL0_direct |= 0x00000001; // start the port (fifo was previously cleared)
	MXC_DMA_Start(mychannel); // sets bits 0 and 1 of control reg and bit 31 of count reload reg

	// note, the DMA enable and reload bits need to be set every time
	// in the IRQ handler routine, otherwise it only does a single block transfer

	// magpie_new; write 1 or more blocks, using the slow-sd card recovery scheme
	u_int32_t bw;
	while(count_dma_irq < RECORDING_TIME_DMABLOCKS) 
	{ // interupts happen here, count_dma_irq increments at fs/dmaBlockSize
		while((dataBlocksDmaCount - dataBlocksConsumedCount) > 0) 
		{ // there is normally a difference of 1, unless the SD card has stalled and the block writes have fallen behind
			delta = dataBlocksDmaCount - dataBlocksConsumedCount;
		}
		//MXC_GPIO_OutSet(gpio_out5.port,gpio_out5.mask); // timing test
		f_write(&file, SD_write_buff + offsetSDbuff, numBytesSDwrite, &bw); // # bytes = 3X word length of buffer, 24 bits
		//MXC_GPIO_OutClr(gpio_out5.port,gpio_out5.mask); // timing test

		dataBlocksConsumedCount+=1;
		blockPtrModuloSDbuff = (blockPtrModuloSDbuff+1) & block_ptr_modulo_mask; // wraps before end of sd_write_buff
		offsetSDbuff = blockPtrModuloSDbuff*numBytesSDwrite;
	}


	SPI1_CTRL0_direct &= 0xfffffffe; // stop the port
	MXC_DMA_Stop(mychannel);


	if ((err = f_close(&file)) != FR_OK)
	{
		debug1=13;
		f_mount(NULL, "", 0);
		return err;
	}

	//writing is done
	LED_Off(LED_GREEN);

	//Completing the data size update to SD Card
	LED_On(LED_RED);

	// re-open to finish the job
	f_open(&file, savedFileName, FA_READ | FA_WRITE);
	update_wav_header(&file); // go back and fill in the file size (2 different places)
	// close again
	f_close(&file);

	// append metadata to end of file , print the number of slow writes and the number of errors
	sprintf(metaBuffer,"Slow writes: %d, block-write Errors: %d",numSlowWrites,numSDwriteErrors );
	f_open(&file, savedFileName, FA_READ | FA_WRITE);
	writeListChunk(&file,"FILE INFO ",metaBuffer); // go back and fill in the file size (2 different places)
	// close again
	f_close(&file);

	set_timestamp(savedFileName, ds3231_datetime.tm_year + 1900, 
						ds3231_datetime.tm_mon + 1, 
						ds3231_datetime.tm_mday, 
						ds3231_datetime.tm_hour,
						ds3231_datetime.tm_min, 
						ds3231_datetime.tm_sec);
    

	// unmount SD Card

	if ((err = f_mount(NULL, "", 0)) != FR_OK)
	{
		debug1 = 15;
		return err;
	}
	else
	{
		mounted = 0;
	}

	debug1 = count_dma_irq;


	//writing is done
	LED_Off(LED_RED);
	MB_LED(0);


	printf("Recording Completed.\n");
	printf("Reset the FTHR2 board to start another recording.\n");

	LED_On(LED_BLUE);

	for(;;){}

} // end of main



