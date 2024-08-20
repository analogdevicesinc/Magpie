/* Private includes --------------------------------------------------------------------------------------------------*/

#include "ad4630.h"
#include "dma.h"
#include "dma_regs.h"
#include "gpio_helpers.h"
#include "mxc_delay.h"
#include "mxc_device.h"
#include "nvic_table.h"
#include "spi.h"
#include "spi_regs.h"

#include <stddef.h> // for NULL

/* Private defines ---------------------------------------------------------------------------------------------------*/

// we use two SPI busses for the ADC, one to configure the ADC, and one to read the audio data from the ADC
#define CONFIG_SPI_BUS (MXC_SPI2)
#define DATA_SPI_BUS (MXC_SPI1)

// some operations require a dummy read
#define AD4630_REG_READ_DUMMY (0x00)

// exit config mode register flags
#define AD4630_EXIT_CONFIG_MODE_FLAG_EXIT (1)

// modes register flags
#define AD4630_MODES_REG_OUT_DATA_POS (0)
#define AD4630_MODES_FLAG_OUT_DATA_MD_24_BIT_DIFF (0b000 << AD4630_MODES_REG_OUT_DATA_POS)
#define AD4630_MODES_FLAG_OUT_DATA_MD_16_BIT_DIFF_PLUS_8_CMN (0b001 << AD4630_MODES_REG_OUT_DATA_POS)
#define AD4630_MODES_FLAG_OUT_DATA_MD_24_BIT_DIFF_PLUS_8_CMN (0b010 << AD4630_MODES_REG_OUT_DATA_POS)
#define AD4630_MODES_FLAG_OUT_DATA_MD_30_BIT_AVG (0b011 << AD4630_MODES_REG_OUT_DATA_POS)
#define AD4630_MODES_FLAG_OUT_DATA_MD_32_BIT_TEST_PATTERN (0b100 << AD4630_MODES_REG_OUT_DATA_POS)

#define AD4630_MODES_REG_DDR_MD_POS (3)
#define AD4630_MODES_FLAG_DDR_MD (1 << AD4630_MODES_REG_DDR_MD_POS)

#define AD4630_MODES_REG_CLK_MD_POS (4)
#define AD4630_MODES_FLAG_CLK_MD_SPI_MODE (0b00 << AD4630_MODES_REG_CLK_MD_POS)
#define AD4630_MODES_FLAG_CLK_MD_ECHO_MODE (0b01 << AD4630_MODES_REG_CLK_MD_POS)
#define AD4630_MODES_FLAG_CLK_MD_HOST_CLK_MODE (0b10 << AD4630_MODES_REG_CLK_MD_POS)
#define AD4630_MODES_FLAG_CLK_MD_INVALID_SETTING (0b11 << AD4630_MODES_REG_CLK_MD_POS)

#define AD4630_MODES_REG_LANE_MD_POS (6)
#define AD4630_MODES_FLAG_LANE_MD_ONE_PER_CHAN (0b00 << AD4630_MODES_REG_LANE_MD_POS)
#define AD4630_MODES_FLAG_LANE_MD_TWO_PER_CHAN (0b01 << AD4630_MODES_REG_LANE_MD_POS)
#define AD4630_MODES_FLAG_LANE_MD_FOUR_PER_CHAN (0b10 << AD4630_MODES_REG_LANE_MD_POS)
#define AD4630_MODES_FLAG_LANE_MD_INTERLEAVED_ON_SDO0 (0b11 << AD4630_MODES_REG_LANE_MD_POS)

// oscillator register flags
#define AD4630_OSCILLATOR_REG_OSC_DIV_POS (0)
#define AD4630_OSCILLATOR_FLAG_OSC_DIV_BY_1 (0b00 << AD4630_OSCILLATOR_REG_OSC_DIV_POS)
#define AD4630_OSCILLATOR_FLAG_OSC_DIV_BY_2 (0b01 << AD4630_OSCILLATOR_REG_OSC_DIV_POS)
#define AD4630_OSCILLATOR_FLAG_OSC_DIV_BY_4 (0b10 << AD4630_OSCILLATOR_REG_OSC_DIV_POS)
#define AD4630_OSCILLATOR_FLAG_INVALID_SETTING (0b11 << AD4630_OSCILLATOR_REG_OSC_DIV_POS)

// size of the various buffers used for SPI transactions
#define CONFIG_SPI_TX_BUFF_LEN_IN_BYTES (3)
#define CONFIG_SPI_RX_BUFF_LEN_IN_BYTES (3)
#define DATA_SPI_RX_BUFF_LEN_IN_BYTES (3)

/* Private types -----------------------------------------------------------------------------------------------------*/

/**
 * @brief Enumerated AD4630 registers are represented here. Only the subset of registers that we actually use is
 * represented.
 */
typedef enum
{
    AD4630_REG_EXIT_CFG_MD = 0x14,
    AD4630_REG_MODES = 0x20,
    AD4630_REG_OSCILLATOR = 0x21,
    AD4630_REG_CONFIG_MODE_SPECIAL_CONSTANT = 0x3fff,
} AD4630_Register_t;

/* Private variables -------------------------------------------------------------------------------------------------*/

/**
 * Chip select pin for config SPI, shorted to the ADC busy pin. Used as the chip select for the config SPI, then set to hi-Z
 */
static mxc_gpio_cfg_t config_spi_cs_pin = {
    .port = MXC_GPIO0,
    .mask = MXC_GPIO_PIN_16,
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_OUT,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
    .drvstr = MXC_GPIO_DRVSTR_3,
};

/**
 * Pin to enable and disable the ADC clock generation, HIGH to enable, LOW to disable
 */
static const mxc_gpio_cfg_t adc_clk_en_pin = {
    .port = MXC_GPIO0,
    .mask = MXC_GPIO_PIN_20,
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_OUT,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

/**
 * Pin to force a reset of the ADC. To reset, pull LOW, then HIGH. Must be HIGH when ADC is in use.
 */
static const mxc_gpio_cfg_t adc_n_reset_pin = {
    .port = MXC_GPIO0,
    .mask = MXC_GPIO_PIN_21,
    .pad = MXC_GPIO_PAD_NONE,
    .func = MXC_GPIO_FUNC_OUT,
    .vssel = MXC_GPIO_VSSEL_VDDIO,
    .drvstr = MXC_GPIO_DRVSTR_0,
};

/**
 * Buffers for writing and reading from the two SPI busses
 */
static uint8_t cfg_spi_tx_buff[CONFIG_SPI_TX_BUFF_LEN_IN_BYTES];
static uint8_t cfg_spi_rx_buff[CONFIG_SPI_RX_BUFF_LEN_IN_BYTES];
static uint8_t data_spi_rx_buff[DATA_SPI_RX_BUFF_LEN_IN_BYTES];

/**
 * SPI request structure for the configuration SPI, used to initialize and set up the ADC
 */
static mxc_spi_req_t cfg_spi_req = {
    .spi = CONFIG_SPI_BUS,
    .txData = cfg_spi_tx_buff,
    .rxData = cfg_spi_rx_buff,
    .txLen = CONFIG_SPI_TX_BUFF_LEN_IN_BYTES,
    .rxLen = 1,
    .ssIdx = 0,
    .ssDeassert = 1,
    .txCnt = 0,
    .rxCnt = 0,
    .completeCB = NULL,
};

/* Private function declarations -------------------------------------------------------------------------------------*/

/**
 * `ad4630_begin_register_access_mode()` puts the AD4360 into register access mode
 *
 * @pre ADC initialization is complete
 *
 * @post the AD4360 is ready for register reads and writes via the Config SPI bus
 *
 * @retval `AD4630_ERROR_ALL_OK` if successful, else an error code
 */
static AD4630_Error_t ad4630_begin_register_access_mode();

/**
 * @brief `ad4630_end_register_access_mode()` ends register access mode
 *
 * @pre the AD4630 has been placed in register access mode by calling `ad4630_begin_register_access_mode()`
 *
 * @post the AD4630 exits register access mode
 *
 * @retval `AD4630_ERROR_ALL_OK` if successful, else an error code
 */
static AD4630_Error_t ad4630_end_register_access_mode();

/**
 * @brief `ad4630_read_reg(r, o)` reads register `r` and stores the value in out pointer `o`
 *
 * @param reg the enumerated register to read
 *
 * @param out [out] pointer to the byte to read into
 *
 * @pre the AD4630 has been placed in register access mode by calling `ad4630_begin_register_access_mode()`
 *
 * @retval `AD4630_ERROR_ALL_OK` if successful, else an error code
 */
static AD4630_Error_t ad4630_read_reg(AD4630_Register_t reg, uint8_t *out);

/**
 * @brief `ad4630_write_reg(r, v)` writes value `v` to register `r`
 *
 * @param reg the enumerated register to write to
 *
 * @param val the value to write to the register
 *
 * @pre the AD4630 has been placed in register access mode by calling `ad4630_begin_register_access_mode()`
 *
 * @post the value is written to the given register
 *
 * @retval `AD4630_ERROR_ALL_OK` if successful, else an error code
 */
static AD4630_Error_t ad4630_write_reg(AD4630_Register_t reg, uint8_t val);

/**
 * `initialize_ad4630_with_config_spi()` initializes the configuration SPI bus and writes to AD4630 registers to
 * configure the ADC
 *
 * @pre ADC initialization is complete
 *
 * @post the ADC is configured by the config SPI bus, but the data SPI bus is not yet set to slave mode
 *
 * @retval `AD4630_ERROR_ALL_OK` if successful, else an error code
 */
static AD4630_Error_t initialize_ad4630_with_config_spi();

/**
 * `set_data_spi_to_slave_mode()` sets the audio data SPI bus to slave mode, getting it ready to receive samples
 *
 * @pre `initialize_ad4630_with_config_spi()` has been called
 *
 * @post the SPI bus responsible for recieving audio samples is set to slave mode and ready to receive data
 *
 * @retval `AD4630_ERROR_ALL_OK` if successful, else an error code
 */
static AD4630_Error_t set_data_spi_to_slave_mode();

/* Public function definitions ---------------------------------------------------------------------------------------*/

AD4630_Error_t ad4630_init()
{
    MXC_GPIO_Config(&adc_clk_en_pin);
    MXC_GPIO_Config(&config_spi_cs_pin);

    // the reset pin must be high or the ADC will be stuck in reset
    MXC_GPIO_Config(&adc_n_reset_pin);
    gpio_write_pin(&adc_n_reset_pin, true);

    ad4630_cont_conversions_stop();

    if (initialize_ad4630_with_config_spi() != AD4630_ERROR_ALL_OK)
    {
        return AD4630_ERROR_CONFIG_ERROR;
    }

    // turn on the ADC clock so that we can config the SPI which need this clock
    ad4630_cont_conversions_start();

    if (set_data_spi_to_slave_mode() != AD4630_ERROR_ALL_OK)
    {
        return AD4630_ERROR_CONFIG_ERROR;
    }

    // we need to re-initialize the clock enable pin, because the config is overwritten when we init SPI1
    MXC_GPIO_Config(&adc_clk_en_pin);

    ad4630_cont_conversions_stop();

    return AD4630_ERROR_ALL_OK;
}

void ad4630_cont_conversions_start()
{
    gpio_write_pin(&adc_clk_en_pin, true);
}

void ad4630_cont_conversions_stop()
{
    gpio_write_pin(&adc_clk_en_pin, false);
}

/* Private function definitions --------------------------------------------------------------------------------------*/

AD4630_Error_t ad4630_read_reg(AD4630_Register_t reg, uint8_t *out)
{
    cfg_spi_tx_buff[0] = (1 << 7) | (reg >> 8);
    cfg_spi_tx_buff[1] = (uint8_t)reg;
    cfg_spi_tx_buff[2] = AD4630_REG_READ_DUMMY; // the output data will end up here

    gpio_write_pin(&config_spi_cs_pin, false);
    MXC_Delay(4); // TODO are these delays necessary?

    if (MXC_SPI_MasterTransaction(&cfg_spi_req) != E_NO_ERROR)
    {
        return AD4630_ERROR_CONFIG_ERROR;
    }

    gpio_write_pin(&config_spi_cs_pin, true);
    MXC_Delay(4); // TODO are these delays necessary?

    *out = cfg_spi_tx_buff[2];

    return AD4630_ERROR_ALL_OK;
}

AD4630_Error_t ad4630_write_reg(AD4630_Register_t reg, uint8_t val)
{
    cfg_spi_tx_buff[0] = 0; // all registers we can write to are 8 bits, so this first byte is always zero for writes
    cfg_spi_tx_buff[1] = (uint8_t)reg;
    cfg_spi_tx_buff[2] = val;

    gpio_write_pin(&config_spi_cs_pin, false);
    MXC_Delay(4); // TODO are these delays necessary?

    if (MXC_SPI_MasterTransaction(&cfg_spi_req) != E_NO_ERROR)
    {
        return AD4630_ERROR_CONFIG_ERROR;
    }

    gpio_write_pin(&config_spi_cs_pin, true);
    MXC_Delay(4); // TODO are these delays necessary?

    return AD4630_ERROR_ALL_OK;
}

AD4630_Error_t ad4630_begin_register_access_mode()
{
    uint8_t dummy = 0;
    return ad4630_read_reg(AD4630_REG_CONFIG_MODE_SPECIAL_CONSTANT, &dummy);
}

AD4630_Error_t ad4630_end_register_access_mode()
{
    return ad4630_write_reg(AD4630_REG_EXIT_CFG_MD, AD4630_EXIT_CONFIG_MODE_FLAG_EXIT);
}

AD4630_Error_t initialize_ad4630_with_config_spi()
{
    // chip select high to start
    gpio_write_pin(&config_spi_cs_pin, true);

    // set up SPI2 to perform ADC initialization
    const uint32_t CFG_SPI_CLK_FREQ_Hz = 5000000;
    if (MXC_SPI_Init(
            CONFIG_SPI_BUS,
            1, // 1 -> master mode
            0, // 0 -> quad mode not used, single bit SPI
            1, // num slaves
            0, // CS polarity (0 for active low)
            CFG_SPI_CLK_FREQ_Hz,
            MAP_A) != E_NO_ERROR)
    {
        return AD4630_ERROR_CONFIG_ERROR;
    }
    if (MXC_SPI_SetDataSize(CONFIG_SPI_BUS, 8) != E_NO_ERROR)
    {
        return AD4630_ERROR_CONFIG_ERROR;
    }
    if (MXC_SPI_SetWidth(CONFIG_SPI_BUS, SPI_WIDTH_STANDARD) != E_NO_ERROR)
    {
        return AD4630_ERROR_CONFIG_ERROR;
    }
    if (MXC_SPI_SetMode(CONFIG_SPI_BUS, SPI_MODE_0) != E_NO_ERROR)
    {
        return AD4630_ERROR_CONFIG_ERROR;
    }

    // use SPI2 to initialize the ADC
    if (ad4630_begin_register_access_mode() != AD4630_ERROR_ALL_OK)
    {
        return AD4630_ERROR_CONFIG_ERROR;
    }
    if (ad4630_write_reg(AD4630_REG_OSCILLATOR, AD4630_OSCILLATOR_FLAG_OSC_DIV_BY_4) != AD4630_ERROR_ALL_OK)
    {
        return AD4630_ERROR_CONFIG_ERROR;
    }
    if (ad4630_write_reg(AD4630_REG_MODES, AD4630_MODES_FLAG_CLK_MD_HOST_CLK_MODE) != AD4630_ERROR_ALL_OK)
    {
        return AD4630_ERROR_CONFIG_ERROR;
    }
    if (ad4630_end_register_access_mode() != AD4630_ERROR_ALL_OK)
    {
        return AD4630_ERROR_CONFIG_ERROR;
    }

    // we no longer need SPI2 for the ADC, set the chip sel line as high-Z
    MXC_SPI_Shutdown(CONFIG_SPI_BUS);
    config_spi_cs_pin.func = MXC_GPIO_FUNC_IN;
    MXC_GPIO_Config(&config_spi_cs_pin);

    return AD4630_ERROR_ALL_OK;
}

AD4630_Error_t set_data_spi_to_slave_mode()
{
    if (MXC_SPI_Init(
            DATA_SPI_BUS,
            0, // 0 -> slave mode
            0, // 0 -> quad mode not used, single bit SPI
            0, // num slaves, none
            0, // CS polarity (0 for active low)
            0, // freq is defined by the driving clock
            MAP_A) != E_NO_ERROR)
    {
        return AD4630_ERROR_CONFIG_ERROR;
    }

    // TODO: this call returns an error. It does in Bob's code too, but it is not handled there.
    // Do we even need to set the data size to 8 bits?
    if (MXC_SPI_SetDataSize(DATA_SPI_BUS, 8) != E_NO_ERROR)
    {
        //        return AD4630_ERROR_CONFIG_ERROR;
    }

    if (MXC_SPI_SetWidth(DATA_SPI_BUS, SPI_WIDTH_3WIRE) != E_NO_ERROR)
    {
        return AD4630_ERROR_CONFIG_ERROR;
    }
    if (MXC_SPI_SetMode(DATA_SPI_BUS, SPI_MODE_1) != E_NO_ERROR)
    {
        return AD4630_ERROR_CONFIG_ERROR;
    }

    // complete the init; don't use the data!
    mxc_spi_req_t data_spi_req = {
        .spi = DATA_SPI_BUS,
        .txData = NULL,
        .rxData = data_spi_rx_buff,
        .txLen = 0,
        .rxLen = DATA_SPI_RX_BUFF_LEN_IN_BYTES,
        .ssIdx = 0,
        .ssDeassert = 1,
        .txCnt = 0,
        .rxCnt = 0,
        .completeCB = NULL,
    };
    if (MXC_SPI_SlaveTransactionAsync(&data_spi_req) != E_NO_ERROR)
    {
        return AD4630_ERROR_CONFIG_ERROR;
    }

    // disable the port
    DATA_SPI_BUS->ctrl0 &= ~(MXC_F_SPI_CTRL0_EN);

    // clear the fifo, start only on pos edge of Slave-sel-B
    MXC_SPI_ClearRXFIFO(DATA_SPI_BUS);

    return AD4630_ERROR_ALL_OK;
}
