/* Private includes --------------------------------------------------------------------------------------------------*/

#include "audio_dma.h"
#include "board.h"
#include "bsp_pins.h"
#include "bsp_spi.h"

#include "arm_math.h"

#include "dma.h"
#include "dma_regs.h"
#include "mxc_sys.h"
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

/* Private variables -------------------------------------------------------------------------------------------------*/

// the DMA channel index to use, will be updated to a valid DMA channel during initialization. This variable only holds
// an integer which maps to the DMA channel. Audio channel 0 may map to DMA channel 0, or it may map to a different DMA
// channel, this depends on which DMA channels have been allocated previously. When requesting a DMA channel, it gives
// you the first available index that is not already allocated. Initialized to bad state, will be updated during init.
static int dma_channel_0_idx = E_BAD_STATE;
static int dma_channel_1_idx = E_BAD_STATE;

// audio samples from the ADC are dumped here in a modulo fashion, this can tolerate iterations with slow SD write speed
static uint8_t big_dma_buff_ch0[AUDIO_DMA_BIG_DMA_BUFF_LEN_IN_BYTES] = {0};
static uint8_t big_dma_buff_ch1[AUDIO_DMA_BIG_DMA_BUFF_LEN_IN_BYTES] = {0};

// the number of DMA_BUFF_LEN_IN_BYTES length buffers available to read, should usually just be 1, but can be up to
// DMA_NUM_STALLS_ALLOWED without issues. If it exceeds DMA_NUM_STALLS_ALLOWED then this indicates an overrun
static int num_buffer_chunks_with_data_to_be_consumed_ch0 = 0;
static int num_buffer_chunks_with_data_to_be_consumed_ch1 = 0;

// true if we write more than DMA_NUM_STALLS_ALLOWED into the big DMA buffer
static bool overrun_occured_ch0 = false;
static bool overrun_occured_ch1 = false;

// Direct register addresses for the DMA channel registers we need to access, saving them like this allows for faster
// access as opposed to accessing them via MXC_DMAn->ch[channel_of_interest].register_of_interest.
// We don't need the source or source-reload registers, since the sources are peripherals (SPI in our case).
static volatile uint32_t *ch0_config_reg;
static volatile uint32_t *ch0_status_reg;
static volatile uint32_t *ch0_dest_reg;
static volatile uint32_t *ch0_dest_reload_reg;
static volatile uint32_t *ch0_count_reg;
static volatile uint32_t *ch0_count_reload_reg;

static volatile uint32_t *ch1_config_reg;
static volatile uint32_t *ch1_status_reg;
static volatile uint32_t *ch1_dest_reg;
static volatile uint32_t *ch1_dest_reload_reg;
static volatile uint32_t *ch1_count_reg;
static volatile uint32_t *ch1_count_reload_reg;

/* Private function declarations -------------------------------------------------------------------------------------*/

/**
 * @brief In the DMA interrupt handler we only update the starting address of the next buffer chunk and increment the
 * number of available buffers. We do as little work as possible in this function, because there is very little time
 * between a DMA IRQ firing and the first few bytes of the buffer being overwritten.
 */
void dma_ch0_callback(int ch, int reason);
void dma_ch1_callback(int ch, int reason);

void DMA0_IRQHandler(void)
{
    // the MXC handler will call the appropriate dma_chn_callback() function based on the DMA register flags
    MXC_DMA_Handler(MXC_DMA0);
}

/* Public function definitions ---------------------------------------------------------------------------------------*/

int audio_dma_init()
{
    int res = E_NO_ERROR;

    MXC_GPIO_Config(&bsp_pins_adc_cs_check_pin_cfg);

    // enable the peripheral clock, required when MSDK_NO_GPIO_CLK_INIT is defined
    if (!MXC_SYS_IsClockEnabled(MXC_SYS_PERIPH_CLOCK_DMA))
    {
        MXC_SYS_ClockEnable(MXC_SYS_PERIPH_CLOCK_DMA);
        MXC_SYS_Reset_Periph(MXC_SYS_RESET_DMA0);
    }

    if ((res = MXC_DMA_Init(MXC_DMA0)) != E_NO_ERROR)
    {
        return res;
    }

    dma_channel_0_idx = MXC_DMA_AcquireChannel(MXC_DMA0);
    dma_channel_1_idx = MXC_DMA_AcquireChannel(MXC_DMA0);

    if (dma_channel_0_idx < 0) // negative result indicates an error
    {
        return dma_channel_0_idx;
    }

    if (dma_channel_1_idx < 0) // negative result indicates an error
    {
        return dma_channel_1_idx;
    }

    MXC_DMA_SetCallback(dma_channel_0_idx, dma_ch0_callback);
    MXC_DMA_SetCallback(dma_channel_1_idx, dma_ch1_callback);

    // save register addresses so we don't need to do a costly lookup for each access
    ch0_config_reg = &MXC_DMA0->ch[dma_channel_0_idx].cfg;
    ch0_status_reg = &MXC_DMA0->ch[dma_channel_0_idx].st;
    ch0_dest_reg = &MXC_DMA0->ch[dma_channel_0_idx].dst;
    ch0_dest_reload_reg = &MXC_DMA0->ch[dma_channel_0_idx].dst_rld;
    ch0_count_reg = &MXC_DMA0->ch[dma_channel_0_idx].cnt;
    ch0_count_reload_reg = &MXC_DMA0->ch[dma_channel_0_idx].cnt_rld;

    ch1_config_reg = &MXC_DMA0->ch[dma_channel_1_idx].cfg;
    ch1_status_reg = &MXC_DMA0->ch[dma_channel_1_idx].st;
    ch1_dest_reg = &MXC_DMA0->ch[dma_channel_1_idx].dst;
    ch1_dest_reload_reg = &MXC_DMA0->ch[dma_channel_1_idx].dst_rld;
    ch1_count_reg = &MXC_DMA0->ch[dma_channel_1_idx].cnt;
    ch1_count_reload_reg = &MXC_DMA0->ch[dma_channel_1_idx].cnt_rld;

    // STEPS FROM THE REFERENCE MANUAL - DMA USAGE

    // (1) clear out the config register and wait until status count-to-zero flag is clear
    *ch0_config_reg = 0;
    while (*ch0_status_reg & MXC_F_DMA_ST_CTZ_ST)
    {
        // TODO: there should be a timeout here in case we get stuck for any reason
    }

    *ch1_config_reg = 0;
    while (*ch1_status_reg & MXC_F_DMA_ST_CTZ_ST)
    {
        // TODO: there should be a timeout here in case we get stuck for any reason
    }

    // (2) set data destination to the start of the big DMA buffer
    *ch0_dest_reg = (uint32_t)big_dma_buff_ch0;
    *ch1_dest_reg = (uint32_t)big_dma_buff_ch1;

    // (3) we don't need to do anything with the source register

    // (4) count and count reload get the size of the buffer in bytes
    *ch0_count_reg = AUDIO_DMA_BUFF_LEN_IN_BYTES;
    *ch1_count_reg = AUDIO_DMA_BUFF_LEN_IN_BYTES;

    // (5) write the whole config register word, minus the channel-enable bit, which we'll set later
    *ch0_config_reg =
        MXC_F_DMA_CFG_CTZIEN |             // ctzien, count to zero interrupt
        (0 << MXC_F_DMA_CFG_CHDIEN_POS) |  // chdien bit is zero, channel disable interrupt not set
        (23 << MXC_F_DMA_CFG_BRST_POS) |   // brst, burst size is 24 bytes, note the -1, 0 = burst size of 1
        MXC_F_DMA_CFG_DISTINC |            // distinc, destination auto increment
        MXC_S_DMA_CFG_DSTWD_BYTE |         // distwd, dest width
        (0 << MXC_F_DMA_CFG_SRINC_POS) |   // srcinc bit is zero, source does not increment
        MXC_S_DMA_CFG_SRCWD_BYTE |         // srcwd, source width
        MXC_S_DMA_CFG_PSSEL_DIS |          // pssel bits are zero, no clock pre-scale divider
        MXC_S_DMA_CFG_TOSEL_TO4 |          // tosel bits are zero, 3-4 periods for timeout
        (0 << MXC_F_DMA_CFG_REQWAIT_POS) | // reqwait bit is zero, start timeout timer immediately
        bsp_spi_adc_ch0_data_spi_dma_req | // reqsel, SPI request
        MXC_DMA_PRIO_HIGH |                // pri bits, smaller integers are higher priority, zero is highest
        (0 << MXC_F_DMA_CFG_RLDEN_POS) |   // rlden, reload enable
        (0 << MXC_F_DMA_CFG_CHEN_POS);     // chen is zero for now, we'll set this later when it's time to start the DMA

    *ch1_config_reg =
        MXC_F_DMA_CFG_CTZIEN |             // ctzien, count to zero interrupt
        (0 << MXC_F_DMA_CFG_CHDIEN_POS) |  // chdien bit is zero, channel disable interrupt not set
        (23 << MXC_F_DMA_CFG_BRST_POS) |   // brst, burst size is 24 bytes, note the -1, 0 = burst size of 1
        MXC_F_DMA_CFG_DISTINC |            // distinc, destination auto increment
        MXC_S_DMA_CFG_DSTWD_BYTE |         // distwd, dest width
        (0 << MXC_F_DMA_CFG_SRINC_POS) |   // srcinc bit is zero, source does not increment
        MXC_S_DMA_CFG_SRCWD_BYTE |         // srcwd, source width
        MXC_S_DMA_CFG_PSSEL_DIS |          // pssel bits are zero, no clock pre-scale divider
        MXC_S_DMA_CFG_TOSEL_TO4 |          // tosel bits are zero, 3-4 periods for timeout
        (0 << MXC_F_DMA_CFG_REQWAIT_POS) | // reqwait bit is zero, start timeout timer immediately
        bsp_spi_adc_ch1_data_spi_dma_req | // reqsel, SPI request
        MXC_DMA_PRIO_HIGH |                // pri bits, smaller integers are higher priority, zero is highest
        (0 << MXC_F_DMA_CFG_RLDEN_POS) |   // rlden, reload enable
        (0 << MXC_F_DMA_CFG_CHEN_POS);     // chen is zero for now, we'll set this later when it's time to start the DMA

    // (6) set the reload and count-reload, the initial reload value is the 2nd chunk (1st chunk is done by the time we reload)
    *ch0_dest_reload_reg = (uint32_t)big_dma_buff_ch0 + AUDIO_DMA_BUFF_LEN_IN_BYTES;
    *ch0_count_reload_reg = AUDIO_DMA_BUFF_LEN_IN_BYTES;

    *ch1_dest_reload_reg = (uint32_t)big_dma_buff_ch1 + AUDIO_DMA_BUFF_LEN_IN_BYTES;
    *ch1_count_reload_reg = AUDIO_DMA_BUFF_LEN_IN_BYTES;

    // (7) setting reload enable happens in audio_dma_start()

    // (8) enabling the channel happens in audio_dma_start()

    // (9) we handle the completed DMA transfer in DMA0_IRQHandler()

    return E_NO_ERROR;
}

int audio_dma_start()
{
    if (dma_channel_0_idx < 0) // negative result indicates an error
    {
        return dma_channel_0_idx;
    }

    if (dma_channel_1_idx < 0) // negative result indicates an error
    {
        return dma_channel_1_idx;
    }

    // enable DMA interrupts
    NVIC_EnableIRQ(DMA0_IRQn);
    MXC_DMA0->cn |= (1 << dma_channel_0_idx) | (1 << dma_channel_1_idx);

    // flush the FIFO, then make sure FIFO is enabled
    bsp_spi_adc_ch0_data_spi_handle->dma |= MXC_F_SPI_DMA_RX_FIFO_CLEAR;
    bsp_spi_adc_ch0_data_spi_handle->dma |= MXC_F_SPI_DMA_RX_FIFO_EN;

    bsp_spi_adc_ch1_data_spi_handle->dma |= MXC_F_SPI_DMA_RX_FIFO_CLEAR;
    bsp_spi_adc_ch1_data_spi_handle->dma |= MXC_F_SPI_DMA_RX_FIFO_EN;

    // clear and then reset the FIFO threshold
    bsp_spi_adc_ch0_data_spi_handle->dma &= ~(0xF << MXC_F_SPI_DMA_RX_FIFO_LEVEL_POS);
    bsp_spi_adc_ch0_data_spi_handle->dma |= (DMA_SPI_RX_THRESHOLD << MXC_F_SPI_DMA_RX_FIFO_LEVEL_POS);
    bsp_spi_adc_ch1_data_spi_handle->dma &= ~(0xF << MXC_F_SPI_DMA_RX_FIFO_LEVEL_POS);
    bsp_spi_adc_ch1_data_spi_handle->dma |= (DMA_SPI_RX_THRESHOLD << MXC_F_SPI_DMA_RX_FIFO_LEVEL_POS);

    // allow SPI ports to receive DMA data
    bsp_spi_adc_ch0_data_spi_handle->dma |= MXC_F_SPI_DMA_RX_DMA_EN;
    bsp_spi_adc_ch1_data_spi_handle->dma |= MXC_F_SPI_DMA_RX_DMA_EN;

    // calculate pre-computed control words so we can do write-only operations during the time-sensitive period
    const uint32_t ch0_spi_ctrl0_control_word = (bsp_spi_adc_ch0_data_spi_handle->ctrl0 | MXC_F_SPI_CTRL0_EN);
    const uint32_t ch1_spi_ctrl0_control_word = (bsp_spi_adc_ch1_data_spi_handle->ctrl0 | MXC_F_SPI_CTRL0_EN);
    const uint32_t ch0_dma_config_reg_control_word = (*ch0_config_reg | MXC_F_DMA_CFG_RLDEN | MXC_F_DMA_CFG_CHEN);
    const uint32_t ch1_dma_config_reg_control_word = (*ch1_config_reg | MXC_F_DMA_CFG_RLDEN | MXC_F_DMA_CFG_CHEN);

    /**
     * Wait until we see a rising SPI Chip Select line, then immediately start the DMA for the 1st channel.
     * This ensures that we don't start the DMA somewhere in the middle of a 3-byte sample.
     */
    bool stall = true;
    bool first_read, second_read;
    while (stall)
    {
        first_read = gpio_read_pin(&bsp_pins_adc_cs_check_pin_cfg);
        second_read = gpio_read_pin(&bsp_pins_adc_cs_check_pin_cfg);
        stall = (!second_read || first_read);
    }

    bsp_spi_adc_ch0_data_spi_handle->ctrl0 = ch0_spi_ctrl0_control_word;
    *ch0_config_reg = ch0_dma_config_reg_control_word;

    /**
     * Now that the 1st channel is started, we want to wait a few frame clocks before starting the 2nd channel.
     * Doing this ensures that the interrupts for the 2 channels don't fire at the same time - we get nasty glitches
     * in the audio if this happens.
     *
     * Waiting for one full FIFO worth (8 samples) plus another 1/2 FIFO (4 samples) makes sure that the DMA channels
     * trigger at different times, with enough time between them that the 1st one is done before the 2nd one fires.
     */
    uint32_t stall_count = 0;
    const uint32_t one_and_a_half_spi_fifo_frames = 8 + 4;
    stall = true;
    while (stall_count < one_and_a_half_spi_fifo_frames)
    {
        first_read = gpio_read_pin(&bsp_pins_adc_cs_check_pin_cfg);
        second_read = gpio_read_pin(&bsp_pins_adc_cs_check_pin_cfg);
        stall = (!second_read || first_read);
        if (!stall)
        {
            stall_count++;
        }
    }

    bsp_spi_adc_ch1_data_spi_handle->ctrl0 = ch1_spi_ctrl0_control_word;
    *ch1_config_reg = ch1_dma_config_reg_control_word;

    // once you are here the CTZ counter is decrementing by 24 every 24 bytes (= 8 24-bit samples)

    return E_NO_ERROR;
}

int audio_dma_stop()
{
    if (dma_channel_0_idx < 0) // negative result indicates an error
    {
        return dma_channel_0_idx;
    }

    if (dma_channel_1_idx < 0) // negative result indicates an error
    {
        return dma_channel_1_idx;
    }

    MXC_DMA0->cn &= ~((1 << dma_channel_0_idx) | (1 << dma_channel_1_idx));
    NVIC_DisableIRQ(DMA0_IRQn);

    // disable the DMA channel
    *ch0_config_reg &= ~(MXC_F_DMA_CFG_RLDEN | MXC_F_DMA_CFG_CHEN);
    *ch1_config_reg &= ~(MXC_F_DMA_CFG_RLDEN | MXC_F_DMA_CFG_CHEN);

    // disable SPI
    bsp_spi_adc_ch0_data_spi_handle->ctrl0 &= ~MXC_F_SPI_CTRL0_EN;
    bsp_spi_adc_ch1_data_spi_handle->ctrl0 &= ~MXC_F_SPI_CTRL0_EN;

    return E_NO_ERROR;
}

uint32_t audio_dma_num_buffers_available(Audio_Channel_t channel)
{
    return channel ? num_buffer_chunks_with_data_to_be_consumed_ch1 : num_buffer_chunks_with_data_to_be_consumed_ch0;
}

uint8_t *audio_dma_consume_buffer(Audio_Channel_t channel)
{
    static uint32_t blockPtrModulo_ch0 = 0;
    static uint32_t blockPtrModulo_ch1 = 0;
    static uint32_t offset_ch0 = 0;
    static uint32_t offset_ch1 = 0;

    uint8_t *retval;

    if (channel == AUDIO_CHANNEL_0)
    {
        retval = big_dma_buff_ch0 + offset_ch0;
        blockPtrModulo_ch0 = (blockPtrModulo_ch0 + 1) & (DMA_NUM_STALLS_ALLOWED - 1);
        offset_ch0 = blockPtrModulo_ch0 * AUDIO_DMA_BUFF_LEN_IN_BYTES;
        num_buffer_chunks_with_data_to_be_consumed_ch0 -= 1;
    }
    else
    {
        retval = big_dma_buff_ch1 + offset_ch1;
        blockPtrModulo_ch1 = (blockPtrModulo_ch1 + 1) & (DMA_NUM_STALLS_ALLOWED - 1);
        offset_ch1 = blockPtrModulo_ch1 * AUDIO_DMA_BUFF_LEN_IN_BYTES;
        num_buffer_chunks_with_data_to_be_consumed_ch1 -= 1;
    }

    return retval;
}

bool audio_dma_overrun_occured(Audio_Channel_t channel)
{
    return channel ? overrun_occured_ch1 : overrun_occured_ch0;
}

void audio_dma_clear_overrun(Audio_Channel_t channel)
{
    if (channel == AUDIO_CHANNEL_0)
    {
        overrun_occured_ch0 = false;
    }
    else
    {
        overrun_occured_ch1 = false;
    }
    // TODO there should probably be some more housekeeping in here, we need to think about how we can recover
}

/* Private function definitions --------------------------------------------------------------------------------------*/

void dma_ch0_callback(int ch, int reason)
{
    (void)ch;
    (void)reason;

    /*
     *  note, the ADC data is sadly in big-endian format (location 0 is an msb) but the wave file is little-endian, so
     * we eventually need to swap the MSByte and the LSbyte (the middle byte can stay the same). In this IRQ we just
     * set the pointer for the next DMA chunk, we don't worry about the endian swap yet. This is because we have a very
     * short time from the start of this function to the moment the first few bytes are overwritten. If we take too
     * long messing about in this function we will get invalid data for the first few samples in the buffer. The time
     * we have is under 1/384kHz = 2.6 microseconds.
     */

    /* This modulo index counts [0, 1, 2, 3, 0, 1, 2, 3, ...], this represents the chunk number in the big DMA buffer.
     * It is used to calculate the chunk address in the big DMA buffer.
     *
     * Initialized to 1 because when we first enter this function, we're already reloading past the very first chunk.
     */
    static uint32_t block_idx_modulo = 1;

    *ch0_config_reg |= (MXC_F_DMA_CFG_RLDEN);

    block_idx_modulo = (block_idx_modulo + 1) & (DMA_NUM_STALLS_ALLOWED - 1);
    const uint32_t offset = block_idx_modulo * AUDIO_DMA_BUFF_LEN_IN_BYTES;
    const uint32_t next_chunk = (uint32_t)big_dma_buff_ch0 + offset;

    *ch0_dest_reload_reg = next_chunk;

    num_buffer_chunks_with_data_to_be_consumed_ch0 += 1;
    overrun_occured_ch0 = num_buffer_chunks_with_data_to_be_consumed_ch0 > DMA_NUM_STALLS_ALLOWED;
}

void dma_ch1_callback(int ch, int reason)
{
    (void)ch;
    (void)reason;

    // see the comments in the ch0 callback, same applies

    static uint32_t block_idx_modulo = 1;

    *ch1_config_reg |= (MXC_F_DMA_CFG_RLDEN);

    block_idx_modulo = (block_idx_modulo + 1) & (DMA_NUM_STALLS_ALLOWED - 1);
    const uint32_t offset = block_idx_modulo * AUDIO_DMA_BUFF_LEN_IN_BYTES;
    const uint32_t next_chunk = (uint32_t)big_dma_buff_ch1 + offset;

    *ch1_dest_reload_reg = next_chunk;

    num_buffer_chunks_with_data_to_be_consumed_ch1 += 1;
    overrun_occured_ch1 = num_buffer_chunks_with_data_to_be_consumed_ch1 > DMA_NUM_STALLS_ALLOWED;
}
