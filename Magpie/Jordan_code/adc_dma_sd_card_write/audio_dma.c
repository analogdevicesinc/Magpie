/* Private includes --------------------------------------------------------------------------------------------------*/

#include "arm_math.h"
#include "audio_dma.h"
#include "dma.h"
#include "dma_regs.h"
#include "gpio_helpers.h"
#include "spi.h"
#include "spi_regs.h"

#include <stdbool.h>
#include <stddef.h> // for NULL

/* Private defines ---------------------------------------------------------------------------------------------------*/

// the number of stalls we can tolerate when the SD card takes longer to write than usual, MUST be a power of 2
#define DMA_NUM_STALLS_ALLOWED (4)

// the length of the big DMA buffer with spare room for tolerating SD card write stalls
#define AUDIO_DMA_BIG_DMA_BUFF_LEN_IN_BYTES (AUDIO_DMA_BUFF_LEN_IN_BYTES * DMA_NUM_STALLS_ALLOWED)

// least common multiple for 3-byte samples crammed into 4-byte words
#define I24_AND_I32_LCM (3 * 4)

// halt compilation if the buffer lengths do not conform to the necessary multiplicity, this is so an even number of
// samples can fit in the buffers for all sample rates
#define AUDIO_DMA_MAIN_BUFFER_LEN_MANDATORY_LCM (16)
#if (AUDIO_DMA_BUFF_LEN_IN_SAMPS % AUDIO_DMA_MAIN_BUFFER_LEN_MANDATORY_LCM)
#error "Main audio DMA buffer length must be divisible by 2, 4, 8, and 16"
#endif
// this check ensures that we can fit an even number of 3-byte samples into the DMA buffer
#if (AUDIO_DMA_BIG_DMA_BUFF_LEN_IN_BYTES % AUDIO_DMA_MAIN_BUFFER_LEN_MANDATORY_LCM)
#error "Big audio DMA buffer length must be divisible by 2, 4, 8, and 16"
#endif

// the threshold for triggering a DMA request
#define DMA_SPI_RX_THRESHOLD (3 * 8)

// the SPI bus to use to read audio samples from the ADC
#define DATA_SPI_BUS (MXC_SPI1)

/* Private variables -------------------------------------------------------------------------------------------------*/

// the DMA channel to use, will be updated to a valid DMA channel during initialization
static int dma_channel = E_BAD_STATE;

// audio samples from the ADC are dumped here in a modulo fashion, this can tolerate iterations with slow SD write speed
static uint8_t bigDMAbuff[AUDIO_DMA_BIG_DMA_BUFF_LEN_IN_BYTES] = {0};

// the number of DMA_BUFF_LEN_IN_BYTES length buffers available to read, should usually just be 1, but can be up to
// DMA_NUM_STALLS_ALLOWED without issues. If it exceeds DMA_NUM_STALLS_ALLOWED then this indicates an overrun
static uint32_t num_buffer_chunks_with_data_to_be_consumed = 0;

// true if we write more than DMA_NUM_STALLS_ALLOWED into the big DMA buffer
static bool overrun_occured = false;

/**
 * The ADC busy pin, goes high when the ADC starts a conversion and goes low when the conversion finishes.
 */
static const mxc_gpio_cfg_t adc_busy_pin = {
    .port = MXC_GPIO0,
    .mask = MXC_GPIO_PIN_3,
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_IN,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
};

/* Private function declarations -------------------------------------------------------------------------------------*/

/**
 * this gets called by the DMA 1st, and when this returns, it goes directly to the DMA0_IRQHandler()
 */
static void DMA_CALLBACK_func(int a, int b)
{
    /* do nothing, immediately transitions to DMA0_IRQHandler() where all the real work happens */
}

/**
 * @brief In the DMA interrupt handler we correct the endian-ness of the audio data and write it to the big DMA buffer
 */
void DMA0_IRQHandler();

/* Public function definitions ---------------------------------------------------------------------------------------*/

Audio_DMA_Error_t audio_dma_init()
{
    MXC_GPIO_Config(&adc_busy_pin);

    NVIC_EnableIRQ(DMA0_IRQn);

    if (MXC_DMA_Init(MXC_DMA0) != E_NO_ERROR)
    {
        return AUDIO_DMA_ERROR_DMA_ERROR;
    }

    dma_channel = MXC_DMA_AcquireChannel(MXC_DMA0);

    if (dma_channel == E_NONE_AVAIL || dma_channel == E_BAD_STATE || dma_channel == E_BUSY)
    {
        return AUDIO_DMA_ERROR_DMA_ERROR;
    }

    mxc_dma_srcdst_t dma_transfer = {
        .ch = dma_channel,
        .source = NULL,
        .dest = bigDMAbuff,
        .len = AUDIO_DMA_BUFF_LEN_IN_BYTES,
    };

    mxc_dma_config_t dma_config = {
        .ch = dma_channel,
        .reqsel = MXC_DMA_REQUEST_SPI1RX,
        .srcwd = MXC_DMA_WIDTH_BYTE,
        .dstwd = MXC_DMA_WIDTH_BYTE,
        .srcinc_en = 0, // this is ignored??
        .dstinc_en = 1,
    };

    mxc_dma_adv_config_t advConfig = {
        .ch = dma_channel,
        .prio = MXC_DMA_PRIO_HIGH,
        .reqwait_en = 0,
        .tosel = MXC_DMA_TIMEOUT_4_CLK,
        .pssel = MXC_DMA_PRESCALE_DISABLE,
        .burst_size = 24,
    };

    if (MXC_DMA_ConfigChannel(dma_config, dma_transfer) != E_NO_ERROR)
    {
        return AUDIO_DMA_ERROR_DMA_ERROR;
    }

    if (MXC_DMA_AdvConfigChannel(advConfig) != E_NO_ERROR)
    {
        return AUDIO_DMA_ERROR_DMA_ERROR;
    }

    if (MXC_DMA_SetSrcDst(dma_transfer) != E_NO_ERROR) // is this redundant??
    {
        return AUDIO_DMA_ERROR_DMA_ERROR;
    }

    if (MXC_DMA_SetSrcReload(dma_transfer) != E_NO_ERROR) // is this redundant??
    {
        return AUDIO_DMA_ERROR_DMA_ERROR;
    }

    const bool ch_complete_int = false;
    const bool count_to_zero_int = true;
    if (MXC_DMA_SetChannelInterruptEn(dma_channel, ch_complete_int, count_to_zero_int) != E_NO_ERROR)
    {
        return AUDIO_DMA_ERROR_DMA_ERROR;
    }

    if (MXC_DMA_SetCallback(dma_channel, DMA_CALLBACK_func) != E_NO_ERROR)
    {
        return AUDIO_DMA_ERROR_DMA_ERROR;
    }

    return AUDIO_DMA_ERROR_ALL_OK;
}

Audio_DMA_Error_t audio_dma_start()
{
    MXC_SPI_ClearRXFIFO(DATA_SPI_BUS);

    if (MXC_DMA_EnableInt(dma_channel) != E_NO_ERROR)
    {
        return AUDIO_DMA_ERROR_DMA_ERROR;
    }

    DATA_SPI_BUS->dma |= MXC_F_SPI_DMA_RX_FIFO_EN;
    if (MXC_SPI_SetRXThreshold(DATA_SPI_BUS, DMA_SPI_RX_THRESHOLD) != E_NO_ERROR)
    {
        return AUDIO_DMA_ERROR_DMA_ERROR;
    }
    DATA_SPI_BUS->dma |= MXC_F_SPI_DMA_RX_DMA_EN;

    // stall until a rising edge on slave-sel-B. This is to insure we have no partial writes (1 or 2 bytes) that mess up the dma
    bool stall = true;
    bool first_read;
    bool second_read;
    while (stall)
    {
        first_read = gpio_read_pin(&adc_busy_pin);  // L
        second_read = gpio_read_pin(&adc_busy_pin); // H
        stall = (!second_read || first_read);
        // TODO: there should be a timeout here in case we get stuck for any reason
    }

    DATA_SPI_BUS->ctrl0 |= MXC_F_SPI_CTRL0_EN;

    if (MXC_DMA_Start(dma_channel) != E_NO_ERROR) // sets bits 0 and 1 of control reg and bit 31 of count reload reg
    {
        return AUDIO_DMA_ERROR_DMA_ERROR;
    }

    return AUDIO_DMA_ERROR_ALL_OK;
}

Audio_DMA_Error_t audio_dma_stop()
{
    DATA_SPI_BUS->ctrl0 &= ~MXC_F_SPI_CTRL0_EN; // stop the port

    if (MXC_DMA_Stop(dma_channel) != E_NO_ERROR)
    {
        return AUDIO_DMA_ERROR_DMA_ERROR;
    }

    return AUDIO_DMA_ERROR_ALL_OK;
}

uint32_t audio_dma_num_buffers_available()
{
    return num_buffer_chunks_with_data_to_be_consumed;
}

uint8_t *audio_dma_consume_buffer()
{
    static uint32_t blockPtrModulo = 0;
    static uint32_t offset = 0;

    uint8_t *retval = bigDMAbuff + offset;

    blockPtrModulo = (blockPtrModulo + 1) & (DMA_NUM_STALLS_ALLOWED - 1);
    offset = blockPtrModulo * AUDIO_DMA_BUFF_LEN_IN_BYTES;

    num_buffer_chunks_with_data_to_be_consumed -= 1;

    return retval;
}

bool audio_dma_overrun_occured()
{
    return overrun_occured;
}

void audio_dma_clear_overrun()
{
    overrun_occured = false;
}

/* Private function definitions --------------------------------------------------------------------------------------*/

void DMA0_IRQHandler()
{
    // note, the ADC data is sadly in big-endian format (location 0 is an msb) but the wave file is little-endian, so
    // we eventually need to swap the MSByte and the LSbyte (the middle byte can stay the same). In this IRQ we just
    // set the pointer for the next DMA chunk, we don't worry about the endian swap yet. This is because we have a very
    // short time from the start of this function to the moment the first few bytes are overwritten. If we take too
    // long messing about in this function we will get invalid data for the first few samples in the buffer. The time
    // we have is under 1/384kHz = 2.6 microseconds.

    static uint32_t blockPtrModuloDMA = 0;
    static uint32_t offsetDMA = 0;

    MXC_DMA_Handler(MXC_DMA0);
    int flags = MXC_DMA_ChannelGetFlags(dma_channel); // clears the cfg enable bit
    MXC_DMA_ChannelClearFlags(dma_channel, flags);

    blockPtrModuloDMA = (blockPtrModuloDMA + 1) & (DMA_NUM_STALLS_ALLOWED - 1);
    offsetDMA = blockPtrModuloDMA * AUDIO_DMA_BUFF_LEN_IN_BYTES;

    const uint32_t next_chunk = bigDMAbuff + offsetDMA;

    MXC_DMA0->ch[dma_channel].dst = next_chunk;
    MXC_DMA0->ch[dma_channel].dst_rld = next_chunk;
    MXC_DMA0->ch[dma_channel].cnt = AUDIO_DMA_BUFF_LEN_IN_BYTES;
    MXC_DMA0->ch[dma_channel].cnt_rld |= MXC_F_DMA_CNT_RLD_RLDEN;

    num_buffer_chunks_with_data_to_be_consumed += 1;

    if (num_buffer_chunks_with_data_to_be_consumed > DMA_NUM_STALLS_ALLOWED)
    {
        overrun_occured = true;
    }
}
