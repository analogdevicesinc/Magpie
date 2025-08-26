
/* Private includes --------------------------------------------------------------------------------------------------*/

#include <stdio.h>`
#include <stdint.h>
#include <string.h>

#include "mxc_delay.h"

#include "ad4630.h"
#include "afe_control.h"
#include "audio_dma.h"
#include "audio_enums.h"
#include "board.h"
#include "bsp_i2c.h"
#include "bsp_pins.h"
#include "bsp_status_led.h"
#include "data_converters.h"
#include "decimation_filter.h"
#include "sd_card.h"
#include "sd_card_bank_ctl.h"
#include "wav_header.h"
#include "system_config.h"

#include "bsp_pushbutton.h"
#include "gpio.h"
#include "DS3231_driver.h"
#include <time.h>

#include "icc.h"

// #include "mxc_device.h"
// #include "mxc_sys.h"
// #include "nvic_table.h"
// #include "tmr.h"

#ifdef TERMINAL_IO_USE_SEGGER_RTT
#include "SEGGER_RTT.h"
#endif

// // Parameters for One-shot timer
// #define INTERVAL_TIME_OST 5 // (ms)
// #define OST_TIMER MXC_TMR5 // Can be MXC_TMR0 through MXC_TMR5
// #define OST_TIMER_IRQn TMR5_IRQn

/* Private function declarations -------------------------------------------------------------------------------------*/

/**
 * @brief `write_wav_file(a, l)` writes a wav file with attributes `a`, and length in seconds `l`, with a name
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
static void write_wav_file(Wave_Header_Attributes_t *wav_attr, uint32_t file_len_secs);

// the error handler simply rapidly blinks the given LED color forever
static void error_handler(Status_LED_Color_t c);

//system control functions
static void initialize_system(void);
static void setup_realtimeclock(void);
static void power_on_audio_chain(void);
static void power_off_audio_chain(void);

static void LED_cascade_right(void);
static void LED_cascade_left(void);

static void start_recording(uint8_t number_of_channel, Audio_Sample_Rate_t rate, Audio_Bits_Per_Sample_t bit, SD_Card_Bank_Card_Slot_t sd_slot, int32_t duration_s);
static void user_pushbutton_interrupt_callback(void *cbdata);
static void setup_user_pushbutton_interrupt(void);

//void OneshotTimerHandler(void);
//void OneshotTimer(void);


// #define FIRST_SET_RTC 1    //uncomment this to set the clock time in the setup_realtimeclock()

#define DEFAULT_FILENAME "Magpie00_19000101_000000"
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

static bool isContinuousRecording = false; //Flag to indicate if the recording is continuous or not

/* Public function definitions ---------------------------------------------------------------------------------------*/
int main(void)
{
#ifdef TERMINAL_IO_USE_CONSOLE_UART
    bsp_console_uart_init();
#endif

    MXC_ICC_Enable();

    initialize_system();

    setup_user_pushbutton_interrupt();

    setup_realtimeclock();

    printf("\n\n==================================\n");
    printf("   Cornell Lab of Ornithology     \n");
    printf("       K. Lisa Yang Center       \n");
    printf("   For Conservation Bioacoustics    \n");
    printf("==================================\n");
    printf(" __  __                   _      \n");
    printf("|  \\/  | __ _  __ _ _ __ (_) ___ \n");
    printf("| |\\/| |/ _` |/ _` | \'_ \\| |/ _ \\\n");
    printf("| |  | | (_| | (_| | |_) | |  __/\n");
    printf("|_|  |_|\\__,_|\\__, | .__/|_|\\___|\n");
    printf("              |___/|_|           \n\n");
    printf("==================================\n\n\n");

    //Get Temperature from RTC
    if (E_NO_ERROR != DS3231_RTC.read_temperature(&ds3231_temperature)) {
        printf("\nDS3231 read temperature error\n");
    } else {
        sprintf((uint8_t*)output_msgBuffer, "\n-->Temperature (C): %.2f\r\n", ds3231_temperature);
        printf(output_msgBuffer);
    }
    
    //Get Time Stamp from RTC
    if (E_NO_ERROR != DS3231_RTC.read_datetime(&ds3231_datetime, ds3231_datetime_str)) {
        printf("\nDS3231 read datetime error\n");
    } else {
        strftime((uint8_t*)output_msgBuffer, OUTPUT_MSG_BUFFER_SIZE, "\n-->DateTime: %F %TZ\r\n", &ds3231_datetime);
        printf(output_msgBuffer);

        strftime((uint8_t*)output_msgBuffer, OUTPUT_MSG_BUFFER_SIZE, "\n-->FileStampTime: %Y%m%d_%H%M%SZ\r\n", &ds3231_datetime);
        //printf(output_msgBuffer);

        //printf(ds3231_datetime_str);		
    }

    printf("Build configuration check:\n");
#ifdef NATIVE_SDHC
    printf("NATIVE_SDHC is defined as: %d\n", NATIVE_SDHC);
#else
    printf("NATIVE_SDHC is NOT defined\n");
#endif
   
    printf("Standing by and waiting for a push from user button ...\n");

    // MXC_NVIC_SetVector(OST_TIMER_IRQn, OneshotTimerHandler);
    // NVIC_EnableIRQ(OST_TIMER_IRQn);
    // OneshotTimer();

    const u_int32_t idle_blink_interval = 20;  //Blink blue every 2 seconds, 100ms per loop
    uint32_t loopCount = 0;

    while (1)
    {
        //Idle blinking loop
         if(loopCount == idle_blink_interval)
        {
            loopCount = 0;
            status_led_set(STATUS_LED_COLOR_BLUE, TRUE);
            MXC_Delay(MXC_DELAY_MSEC(50));
            status_led_set(STATUS_LED_COLOR_BLUE, FALSE);
        }
        MXC_Delay(MXC_DELAY_MSEC(100));
        loopCount++;

        

        uint8_t button_state = get_user_pushbutton_state();

        if((BUTTON_STATE_JUST_PRESSED == button_state)||(BUTTON_STATE_PRESSED == button_state)||isContinuousRecording)
        //if (BUTTON_STATE_PRESSED == button_state)
        {
            // The first time user pushed the button, we set it to contiounous recording mode
            isContinuousRecording = true;   //this will get set to false if user interrupts the recording

            LED_cascade_left();
            printf("Start recording ...\n");

            // Remove callback for push button
            // MXC_GPIO_RegisterCallback(&bsp_pins_user_pushbutton_cfg, NULL, NULL);
            // MXC_GPIO_DisableInt(bsp_pins_user_pushbutton_cfg.port, bsp_pins_user_pushbutton_cfg.mask);
            // NVIC_DisableIRQ(GPIO0_IRQn);

            status_led_set(STATUS_LED_COLOR_GREEN, TRUE);
            MXC_Delay(MXC_DELAY_MSEC(20));
            status_led_set(STATUS_LED_COLOR_GREEN, FALSE);
                               

            //Get Date Time from RTC 
            if(E_NO_ERROR != DS3231_RTC.read_datetime(&ds3231_datetime, ds3231_datetime_str))
            {
                sprintf(savedFileName,"%s",DEFAULT_FILENAME);
            } else {
                
                sprintf(savedFileName,"%s%s","Magpie00_",ds3231_datetime_str);
                printf("File to be saved: %s \n", savedFileName);
            }

            //re-enable button (this is for checking a stop in the middle of recording)
            pushbuttons_init();
            setup_user_pushbutton_interrupt();

            start_recording(SYS_CONFIG_NUM_CHANNEL, SYS_CONFIG_SAMPLE_RATE,
                            SYS_CONFIG_NUM_BIT_DEPTH,
                            SYS_CONFIG_SD_CARD_SLOT_TO_USE,
                            SYS_CONFIG_AUDIO_FILE_LEN_IN_SECONDS);
            
            
            printf("Recording Done ...\n");    
            
            //re-enable button (this is for the next recording)
            pushbuttons_init();
            setup_user_pushbutton_interrupt();

            printf("Standing by and waiting for a push from user button or a Continue Signal ...\n\n");
        }
    }
}

/* Private function definitions --------------------------------------------------------------------------------------*/

void write_wav_file(Wave_Header_Attributes_t *wav_attr, uint32_t file_len_secs)
{
    printf(
        "\n[STARTING]--> %dk %d-bit %d-second %d-channel recording...\n",
        wav_attr->sample_rate / 1000,
        wav_attr->bits_per_sample,
        file_len_secs,
        wav_attr->num_channels);

    // a variable to store the number of bytes written to the SD card, can be checked against the intended amount
    uint32_t bytes_written;

    // a string buffer to write file names into
    char file_name_buff[64];

    // derive the file name from the input parameters
    sprintf(
        file_name_buff,
        "%s_%dkHz_%d_bit_%d_channel.wav",
        savedFileName,        
        wav_attr->sample_rate / 1000,
        wav_attr->bits_per_sample,
        wav_attr->num_channels);

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

    // there will be some integer truncation here, good enough for this early demo, but improve file-len code eventually
    const uint32_t file_len_in_microsecs = file_len_secs * 1000000;
    const uint32_t num_dma_blocks_in_the_file = file_len_in_microsecs / AUDIO_DMA_CHUNK_READY_PERIOD_IN_MICROSECS;

    decimation_filter_set_sample_rate(wav_attr->sample_rate);

// expanding to q31 means 4 bytes per sample, 2 channels
#define MAX_WORKSPACE_NEEDED_FOR_ENDIAN_SWAP_AND_Q31_EXPANSION (AUDIO_DMA_BUFF_LEN_IN_SAMPS * DATA_CONVERTERS_Q31_SIZE_IN_BYTES * 2)

// tradeoff between memory and current consumption, bigger buffers reduces current by doing fewer, larger, SD writes
#define FULL_BYTE_POOL_SIZE (MAX_WORKSPACE_NEEDED_FOR_ENDIAN_SWAP_AND_Q31_EXPANSION * 3)

    /*
     This is a single pool of memory which we use to do all the processing, we reserve a small potion at the start of
     the array for decimation for the non-384kHz rates. The rest of the pool is used to cram as many processed DMA
     blocks in as we can, and then write them all to the SD card once this pool is as full as it can get.

     For the 384k case, we just use the whole pool for chunk-packing with no reserved portion at the start, since we
     don't need to do any decimation filtering for 384k.
     */
    static uint8_t workspace_byte_pool[FULL_BYTE_POOL_SIZE];

    // 384kHz is treated differently to all the other sample rates, since it does not go through the decimation filter
    const bool its_the_special_case_of_384kHz = (wav_attr->sample_rate == AUDIO_SAMPLE_RATE_384kHz);

    // decimate by 2 for 192k, 4 for 96k, etc
    const uint32_t decimation_factor = AUDIO_SAMPLE_RATE_384kHz / wav_attr->sample_rate;

    // this represents the final sample width that is written to the SD card, not during intermediate steps
    const uint32_t bytes_per_processed_sample = (wav_attr->bits_per_sample / 8);

    // a single SD card write event will consist of one or more processed blocks, most of the time it will be several blocks
    const uint32_t num_bytes_in_one_processed_DMA_block = (AUDIO_DMA_BUFF_LEN_IN_SAMPS / decimation_factor) * bytes_per_processed_sample * wav_attr->num_channels;

    // we don't reserve any bytes in the 384k case, but we reserve a potion for the decimated rates to do the filtering
    const uint32_t num_bytes_reserved_for_decimation = its_the_special_case_of_384kHz ? 0 : (AUDIO_DMA_BUFF_LEN_IN_SAMPS * 4 * wav_attr->num_channels);

    // this portion represents everything except the bytes at the array start reserved for decimation, will be the whole pool for the 384k case
    const uint32_t num_bytes_reserved_for_chunk_packing = FULL_BYTE_POOL_SIZE - num_bytes_reserved_for_decimation;

    // this will have some integer truncation, so there may be some portion of the memory pool wasted for some combos
    const uint32_t num_processed_chunks_we_can_fit_in_the_workspace_buff = num_bytes_reserved_for_chunk_packing / num_bytes_in_one_processed_DMA_block;

    // we start the chunk-packing portion of the memory pool immediately after the portion reserved for decimation (if any, none for 384k)
    uint8_t *start_of_chunk_packing_sector = workspace_byte_pool + num_bytes_reserved_for_decimation;

    uint32_t chunk_packing_idx = 0;
    uint32_t total_num_bytes_in_the_buff = 0;

    printf("[Info]--> Start ADC clock and DMA. Recording for %d sec(s) ... \n", file_len_secs);

    status_led_set(STATUS_LED_COLOR_GREEN, true); // green led on


    ad4630_384kHz_fs_clk_and_cs_start();
    audio_dma_start();

    //status_led_set(STATUS_LED_COLOR_GREEN, true); // green led on while recording



    uint32_t num_dma_blocks_consumed = 0;

    while (num_dma_blocks_consumed < num_dma_blocks_in_the_file)
    {        
        // check if the user has pressed the button to stop recording
        uint8_t button_state = get_user_pushbutton_state();       
    

        if (audio_dma_overrun_occured(AUDIO_CHANNEL_0) || audio_dma_overrun_occured(AUDIO_CHANNEL_1))
        {
            printf("[ERROR]--> Audio DMA overrrun\n");
            //error_handler(STATUS_LED_COLOR_BLUE);
              //error_handler(STATUS_LED_COLOR_BLUE);
            num_dma_blocks_consumed = num_dma_blocks_in_the_file;  // stop recording this run and exit the loop
            audio_dma_clear_overrun(AUDIO_CHANNEL_0);
            audio_dma_clear_overrun(AUDIO_CHANNEL_1);
            //we'll start over with the net
            break;
        }

        if (audio_dma_num_buffers_available(AUDIO_CHANNEL_0) > 0 && audio_dma_num_buffers_available(AUDIO_CHANNEL_1) > 0)
        {
            uint8_t *next_chunk_to_write_to = start_of_chunk_packing_sector + (num_bytes_in_one_processed_DMA_block * chunk_packing_idx);
            chunk_packing_idx++;

            if (wav_attr->num_channels == WAVE_HEADER_STEREO)
            {
                if (its_the_special_case_of_384kHz)
                {
                    if (wav_attr->bits_per_sample == AUDIO_BIT_DEPTH_24_BITS_PER_SAMPLE)
                    {
                        data_converters_interleave_2_i24_and_swap_endianness(
                            audio_dma_consume_buffer(AUDIO_CHANNEL_0),
                            audio_dma_consume_buffer(AUDIO_CHANNEL_1),
                            next_chunk_to_write_to,
                            AUDIO_DMA_BUFF_LEN_IN_SAMPS);
                    }
                    else // it's 384k 16 bits
                    {
                        data_converters_interleave_2_i24_to_q15_and_swap_endianness(
                            audio_dma_consume_buffer(AUDIO_CHANNEL_0),
                            audio_dma_consume_buffer(AUDIO_CHANNEL_1),
                            (q15_t *)next_chunk_to_write_to,
                            AUDIO_DMA_BUFF_LEN_IN_SAMPS);
                    }
                }
                else // it's not the special case of 384kHz, all other sample rates are filtered
                {
                    // we use some space at the start of the big workspace array for filtering
                    // since decimation works with q31's, we need to move the ch1 start up by nsamps * 4 bytes
                    const uint8_t *workspace_for_ch0 = workspace_byte_pool;
                    const uint8_t *workspace_for_ch1 = workspace_byte_pool + (AUDIO_DMA_BUFF_LEN_IN_SAMPS * 4);

                    data_converters_i24_to_q31_with_endian_swap(
                        audio_dma_consume_buffer(AUDIO_CHANNEL_0),
                        (q31_t *)workspace_for_ch0,
                        AUDIO_DMA_BUFF_LEN_IN_SAMPS);

                    data_converters_i24_to_q31_with_endian_swap(
                        audio_dma_consume_buffer(AUDIO_CHANNEL_1),
                        (q31_t *)workspace_for_ch1,
                        AUDIO_DMA_BUFF_LEN_IN_SAMPS);

                    const uint32_t decimated_len_in_samps = decimation_filter_downsample(
                        (q31_t *)workspace_for_ch0,
                        (q31_t *)workspace_for_ch0, // we're decimating "in place", this is ok because the decimated output will always be less than the original input
                        AUDIO_DMA_BUFF_LEN_IN_SAMPS,
                        AUDIO_CHANNEL_0);

                    decimation_filter_downsample( // the len will be the same, we already grabbed it from ch0 above
                        (q31_t *)workspace_for_ch1,
                        (q31_t *)workspace_for_ch1, //  decimating in-place
                        AUDIO_DMA_BUFF_LEN_IN_SAMPS,
                        AUDIO_CHANNEL_1);

                    if (wav_attr->bits_per_sample == AUDIO_BIT_DEPTH_24_BITS_PER_SAMPLE)
                    {
                        data_converters_interleave_2_q31_to_i24(
                            (q31_t *)workspace_for_ch0,
                            (q31_t *)workspace_for_ch1,
                            next_chunk_to_write_to,
                            decimated_len_in_samps);
                    }
                    else // it's one of the decimated sample rates at 16 bits
                    {
                        data_converters_interleave_2_q31_to_q15(
                            (q31_t *)workspace_for_ch0,
                            (q31_t *)workspace_for_ch1,
                            (q15_t *)next_chunk_to_write_to,
                            decimated_len_in_samps);
                    }
                }
            }
            else // num-channels must be MONO
            {
                // hack- consume the buffer we're not using so we don't get overruns, TODO change this
                audio_dma_consume_buffer(SYS_CONFIG_CHANNEL_TO_USE_FOR_MONO_MODE ? AUDIO_CHANNEL_0 : AUDIO_CHANNEL_1);

                if (its_the_special_case_of_384kHz)
                {
                    if (wav_attr->bits_per_sample == AUDIO_BIT_DEPTH_24_BITS_PER_SAMPLE)
                    {
                        data_converters_i24_swap_endianness(
                            audio_dma_consume_buffer(SYS_CONFIG_CHANNEL_TO_USE_FOR_MONO_MODE),
                            next_chunk_to_write_to,
                            AUDIO_DMA_BUFF_LEN_IN_SAMPS);
                    }
                    else // it's 384k 16 bits
                    {
                        data_converters_i24_to_q15_with_endian_swap(
                            audio_dma_consume_buffer(SYS_CONFIG_CHANNEL_TO_USE_FOR_MONO_MODE),
                            (q15_t *)next_chunk_to_write_to,
                            AUDIO_DMA_BUFF_LEN_IN_SAMPS);
                    }
                }
                else // it's not the special case of 384kHz, all other sample rates are filtered
                {
                    // all sample rates other than 384k are filtered, so we need to swap endianness and also expand to 32 bit words as expected by the filters
                    data_converters_i24_to_q31_with_endian_swap(
                        audio_dma_consume_buffer(SYS_CONFIG_CHANNEL_TO_USE_FOR_MONO_MODE),
                        (q31_t *)workspace_byte_pool,
                        AUDIO_DMA_BUFF_LEN_IN_SAMPS);

                    // workspace_buff starting at index zero now has the endian-swapped and q31-expanded data for this DMA chunk

                    const uint32_t decimated_len_in_samps = decimation_filter_downsample(
                        (q31_t *)workspace_byte_pool,
                        (q31_t *)workspace_byte_pool, // decimating in-place
                        AUDIO_DMA_BUFF_LEN_IN_SAMPS,
                        SYS_CONFIG_CHANNEL_TO_USE_FOR_MONO_MODE);

                    // workspace_buff starting at index zero now has the decimated version of this DMA chunk

                    if (wav_attr->bits_per_sample == AUDIO_BIT_DEPTH_24_BITS_PER_SAMPLE)
                    {
                        data_converters_q31_to_i24((q31_t *)workspace_byte_pool, next_chunk_to_write_to, decimated_len_in_samps);
                    }
                    else // it's one of the decimated sample rates at 16 bits
                    {
                        data_converters_q31_to_q15((q31_t *)workspace_byte_pool, (q15_t *)next_chunk_to_write_to, decimated_len_in_samps);
                    }
                }
            }

            total_num_bytes_in_the_buff += num_bytes_in_one_processed_DMA_block;

            if (chunk_packing_idx >= num_processed_chunks_we_can_fit_in_the_workspace_buff) // we filled up the chunk-packing sector as much as we can
            {
                if (sd_card_fwrite(start_of_chunk_packing_sector, total_num_bytes_in_the_buff, &bytes_written) != E_NO_ERROR)
                {
                    error_handler(STATUS_LED_COLOR_RED);
                }

                chunk_packing_idx = 0;
                total_num_bytes_in_the_buff = 0;
            }

            num_dma_blocks_consumed += 1;

            //Blink Green LED while recording
            if(num_dma_blocks_consumed % 120 == 0)
            {                
                status_led_set(STATUS_LED_COLOR_GREEN, TRUE); // turn the green LED back on
                MXC_Delay(MXC_DELAY_MSEC(25));
                status_led_set(STATUS_LED_COLOR_GREEN, FALSE);
            }
        }
        
        if((BUTTON_STATE_JUST_PRESSED == button_state)||(BUTTON_STATE_PRESSED == button_state))
        {
            num_dma_blocks_consumed = num_dma_blocks_in_the_file; // stop recording if the button is pressed
            isContinuousRecording = false; // set the flag to false so that we don't continue recording next time
            
            printf("[INFO]--> User interrupted recording.  End recording ....\n");
            status_led_set(STATUS_LED_COLOR_BLUE, TRUE); // turn the green LED back on
            // MXC_Delay(MXC_DELAY_MSEC(250));
            // status_led_set(STATUS_LED_COLOR_BLUE, FALSE);

        }
    }

    // there may be some data left in the chunk-packing buffer, if we don't write this out the file will be a little shorter than expected
    if (total_num_bytes_in_the_buff > 0)
    {
        if (sd_card_fwrite(start_of_chunk_packing_sector, total_num_bytes_in_the_buff, &bytes_written) != E_NO_ERROR)
        {
            error_handler(STATUS_LED_COLOR_RED);
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

        //Generate time stamp
	    set_file_timestamp(file_name_buff, ds3231_datetime.tm_year + 1900, 
        ds3231_datetime.tm_mon + 1, 
        ds3231_datetime.tm_mday, 
        ds3231_datetime.tm_hour,
        ds3231_datetime.tm_min, 
        ds3231_datetime.tm_sec);
    }

    printf("[SUCCESS]--> Wrote file %s\n", file_name_buff);

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


static void setup_user_pushbutton_interrupt(void)
{

    // Configure interrupt
    MXC_GPIO_RegisterCallback(&bsp_pins_user_pushbutton_cfg, user_pushbutton_interrupt_callback, NULL);

    // Configure for falling edge detection (trigger on button press)
    MXC_GPIO_IntConfig(&bsp_pins_user_pushbutton_cfg, MXC_GPIO_INT_FALLING);

     // Enable interrupt
    MXC_GPIO_EnableInt(bsp_pins_user_pushbutton_cfg.port, bsp_pins_user_pushbutton_cfg.mask);
    // Enable global interrupts
    //NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(bsp_pins_user_pushbutton_cfg.mask)));
    NVIC_EnableIRQ(GPIO0_IRQn);
}


static void LED_cascade_right(void) //R->G->B
{
    status_led_all_off();
    for (size_t i = 0; i < 3; i++)
    {
        //Turn each color LED on
        status_led_set(i,TRUE);
        MXC_Delay(MXC_DELAY_MSEC(20));
    }
    MXC_Delay(MXC_DELAY_MSEC(20));
    status_led_all_off();
}

static void LED_cascade_left(void) //R<-G<-B
{
    status_led_all_off();
    for (size_t i = 3; i > 0; i--)
    {
        //Turn each color LED on
        status_led_set(i-1,TRUE);
        MXC_Delay(MXC_DELAY_MSEC(20));
    }
    MXC_Delay(MXC_DELAY_MSEC(20));
    status_led_all_off();
}

static void initialize_system(void)
{
     printf("\n*********** Initializing Magpie Core Systems ***********\n\n");
    status_led_all_off();    
    
    //initialize push button
    pushbuttons_init();

     bsp_power_on_LDOs();

    if (ad4630_init() != E_NO_ERROR)
    {
        printf("[ERROR]--> AD4630 init\n");
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> AD4630 init\n");
    }

    if (audio_dma_init() != E_NO_ERROR)
    {
        printf("[ERROR]--> DMA init\n");
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> DMA init\n");
    }

    if (bsp_3v3_i2c_init() != E_NO_ERROR)   //This allows I2C connecting to DS3231, SD, SD_CTL
    {
        printf("[ERROR]--> I2C init\n");
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> I2C init\n");
    }

    if (bsp_1v8_i2c_init() != E_NO_ERROR)  //This allows I2C connecting to Bosch Sensor, Fuel Gauge, LDO, AFE
    {
        printf("[ERROR]--> 1V8 I2C init\n");
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> 1V8 I2C init\n");
    }  

    ////////////////////  AUDIO INIT ////////////////////

    if (afe_control_init() != E_NO_ERROR)
    {
        printf("[ERROR]--> AFE Control init\n");
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> AFE Control init\n");
    }
    if (afe_control_enable(AUDIO_CHANNEL_0) != E_NO_ERROR)
    {
        printf("[ERROR]--> AFE Control CH0 EN\n");
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> AFE Control CH0 EN\n");
    }
    if (afe_control_enable(AUDIO_CHANNEL_1) != E_NO_ERROR)
    {
        printf("[ERROR]--> AFE Control CH1 EN\n");
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> AFE Control CH1 EN\n");
    }
    if (afe_control_set_gain(AUDIO_CHANNEL_0, SYS_CONFIG_AUDIO0_GAIN) != E_NO_ERROR)
    {
        printf("[ERROR]--> AFE Control CH0 gain set to %ddB\n", SYS_CONFIG_AUDIO0_GAIN);
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> AFE Control CH0 gain set to %ddB\n", SYS_CONFIG_AUDIO0_GAIN);
    }
    if (afe_control_set_gain(AUDIO_CHANNEL_1, SYS_CONFIG_AUDIO1_GAIN) != E_NO_ERROR)
    {
        printf("[ERROR]--> AFE Control CH1 gain set to %ddB\n", SYS_CONFIG_AUDIO1_GAIN);
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> AFE Control CH1 gain set to %ddB\n", SYS_CONFIG_AUDIO1_GAIN);
    }

    Audio_Gain_t readback_gain = afe_control_get_gain(AUDIO_CHANNEL_0);
    if (readback_gain != SYS_CONFIG_AUDIO0_GAIN)
    {
        printf("[ERROR]--> AFE CH0 set (%ddB) and get (%ddB) gain don't match\n", SYS_CONFIG_AUDIO0_GAIN, readback_gain);
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> AFE CH0 get-gain matches AFE set-gain\n");
    }
    readback_gain = afe_control_get_gain(AUDIO_CHANNEL_1);
    if (readback_gain != SYS_CONFIG_AUDIO1_GAIN)
    {
        printf("[ERROR]--> AFE CH1 set (%ddB) and get (%ddB) gain don't match\n", SYS_CONFIG_AUDIO1_GAIN, readback_gain);
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> AFE CH1 get-gain matches AFE set-gain\n");
    }

    //////////////// SD CARD Bank Control INIT /////////////////////

    if (sd_card_bank_ctl_init() != E_NO_ERROR)
    {
        printf("[ERROR]--> SD card bank ctl init\n");
        error_handler(STATUS_LED_COLOR_GREEN);
    }
    else
    {
        printf("[SUCCESS]--> SD card bank ctl init\n");
    }
   
}

static void setup_realtimeclock()
{
    DS3231_RTC = DS3231_Open();

    if(E_NO_ERROR != DS3231_RTC.init(bsp_i2c_3v3_i2c_handle))
    {
      printf("[ERROR] --> Unable to initialize RTC driver.\n");
      error_handler(STATUS_LED_COLOR_RED);
    }

    //only do this if the clock has never been set before
    #ifdef FIRST_SET_RTC
	// //Set Date Time to something
	//Year is always Year - 1900
	//Month is 0-11 so subtract 1 from the month you want to set
	//Time is in UTC so set appropriately
	// hour is 0-23
	// min is 0-59
	// sec is 0-59
	struct tm newTime = {
		.tm_year = 2025 - 1900U,
		.tm_mon =  8 - 1U,
		.tm_mday = 26,
		.tm_hour = 10,
		.tm_min = 17,
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
}

static void power_on_audio_chain(void)
{
    bsp_power_on_LDOs();            
}

static void power_off_audio_chain(void)
{

    // if (afe_control_disable(AUDIO_CHANNEL_0) != E_NO_ERROR)
    // {
    //     printf("[ERROR]--> AFE Control CH0 DIS\n");
    //     error_handler(STATUS_LED_COLOR_RED);
    // }
    // else
    // {
    //     printf("[SUCCESS]--> AFE Control CH0 DIS\n");
    // }

    // if (afe_control_disable(AUDIO_CHANNEL_1) != E_NO_ERROR)
    // {
    //     printf("[ERROR]--> AFE Control CH1 DIS\n");
    //     error_handler(STATUS_LED_COLOR_RED);
    // }
    // else
    // {
    //     printf("[SUCCESS]--> AFE Control CH0 DIS\n");
    // }

    bsp_power_off_LDOs();
}

static void start_recording(uint8_t number_of_channel, 
    Audio_Sample_Rate_t rate, Audio_Bits_Per_Sample_t bit, 
                            SD_Card_Bank_Card_Slot_t sd_slot, int32_t duration_s)
{
    
    //power_on_audio_chain();

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
    MXC_Delay(100000);  // 100ms delay
    //MXC_DELAY_MSEC(200);

    int sd_card_mount_result = sd_card_mount();
    printf("[INFO]--> SD card mount result: %d\n", sd_card_mount_result);

    if (sd_card_mount() != E_NO_ERROR)
    {
       
        printf("[ERROR]--> SD card mount failed.\n");

        error_handler(STATUS_LED_COLOR_RED);            
    }
    else
    {
        printf("[SUCCESS]--> SD card mounted\n");
        QWORD disk_size =  sd_card_disk_size_bytes();
        QWORD disk_free = sd_card_free_space_bytes();
        printf("[INFO]--> SD card Total Space: %llu Bytes\n", disk_size);
        printf("[INFO]--> SD card Free Space: %llu Bytes\n", disk_free);
        
    }

    

    Wave_Header_Attributes_t wav_attr = {
        .num_channels =  number_of_channel,
        .sample_rate = rate,
        .bits_per_sample = bit,
    };
                         
    write_wav_file(&wav_attr, duration_s);
    //MXC_Delay(500000);  //Delay 500 ms 


    if (sd_card_unmount() != E_NO_ERROR)
    {
        printf("[ERROR]--> SD card unmount\n");
        error_handler(STATUS_LED_COLOR_RED);
    }
    else
    {
        printf("[SUCCESS]--> SD card unmounted\n");
    }

    printf("[SUCCESS]--> All files recorded, shutting down\n");

    //power_off_audio_chain();
}

///////////////////////////ISP CALL BACKS ////////////////////////////////////

static void user_pushbutton_interrupt_callback(void *cbdata)
{
    // Get and clear interrupt flags
    uint32_t status = MXC_GPIO_GetFlags(bsp_pins_user_pushbutton_cfg.port);
    MXC_GPIO_ClearFlags(bsp_pins_user_pushbutton_cfg.port, status);

    // Disable interrupt temporarily
    MXC_GPIO_DisableInt(bsp_pins_user_pushbutton_cfg.port, bsp_pins_user_pushbutton_cfg.mask);

    NVIC_DisableIRQ(GPIO0_IRQn);

    // Start the debounce timer which should produce "button_pressed" after some time after
    // checking the GPIO pin
    start_user_btn_debounceTimer();  //re-activate one shot timer
    
}

// void OneshotTimerHandler()
// {
//     // Clear interrupt
//     MXC_TMR_ClearFlags(OST_TIMER);
//     printf("One Shot Time fired \n");
// }

// void OneshotTimer()
// {
//     // Declare variables
//     mxc_tmr_cfg_t tmr;
//     uint32_t periodTicks = PeripheralClock / (128*1000) * INTERVAL_TIME_OST;
//     /*
//     Steps for configuring a timer for PWM mode:
//     1. Disable the timer
//     2. Set the prescale value
//     3  Configure the timer for continuous mode
//     4. Set polarity, timer parameters
//     5. Enable Timer
//     */

//     MXC_TMR_Shutdown(OST_TIMER);

//     tmr.pres = TMR_PRES_128;
//     tmr.mode = TMR_MODE_ONESHOT;
//     tmr.cmp_cnt = periodTicks;
//     tmr.pol = 0;

//     MXC_SYS_Reset_Periph(MXC_SYS_RESET_TIMER5);
//     while (MXC_GCR->rstr0 & MXC_F_GCR_RSTR0_TIMER5) {}
//     MXC_SYS_ClockEnable(MXC_SYS_PERIPH_CLOCK_T5);
//     MXC_GPIO_Config(&gpio_cfg_tmr5);

//     MXC_TMR_Init(OST_TIMER, &tmr);

//     MXC_TMR_Start(OST_TIMER);

//     printf("PeriodTicks = %d \n", periodTicks);
//     printf("Oneshot timer started.\n\n");
// }