/* Private includes --------------------------------------------------------------------------------------------------*/

#include <stddef.h> // for NULL

#include "dma.h"
#include "dma_regs.h"
#include "mxc_delay.h"
#include "mxc_device.h"
#include "nvic_table.h"
#include "spi.h"
#include "spi_regs.h"

#include "ad4630.h"
#include "board.h"
#include "bsp_pins.h"
#include "bsp_spi.h"

/* Private defines ---------------------------------------------------------------------------------------------------*/

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
 * Buffers for writing and reading from the two SPI busses
 */
static uint8_t cfg_spi_tx_buff[CONFIG_SPI_TX_BUFF_LEN_IN_BYTES];
static uint8_t cfg_spi_rx_buff[CONFIG_SPI_RX_BUFF_LEN_IN_BYTES];
static uint8_t data_spi_rx_buff[DATA_SPI_RX_BUFF_LEN_IN_BYTES];

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

/* Public function definitions ---------------------------------------------------------------------------------------*/

AD4630_Error_t ad4630_init()
{
    // configure all the GPIO pins needed for the ADC
    MXC_GPIO_Config(&bsp_pins_adc_cfg_spi_cs_out_cfg);
    MXC_GPIO_Config(&bsp_pins_adc_clk_en_cfg);
    MXC_GPIO_Config(&bsp_pins_adc_n_reset_cfg);
    MXC_GPIO_Config(&bsp_pins_adc_clk_master_reset_cfg);
    MXC_GPIO_Config(&bsp_pins_adc_cs_disable_cfg);

    // the reset pin must be high or the ADC will be stuck in reset
    gpio_write_pin(&bsp_pins_adc_n_reset_cfg, true);

    ad4630_cont_conversions_stop();

    // use the config SPI to initialize the ADC
    if (bsp_adc_config_spi_init() != E_NO_ERROR)
    {
        return AD4630_ERROR_CONFIG_ERROR;
    }

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

    // set the config CS pin back to high-Z so that it doesn't interfere with the data CS pin
    MXC_GPIO_Config(&bsp_pins_adc_cfg_spi_cs_high_z_cfg);

    if (bsp_adc_config_spi_deinit() != E_NO_ERROR) // we don't need the config SPI anymore
    {
        return AD4630_ERROR_CONFIG_ERROR;
    }

    // turn on the ADC clock so that we can config the data SPI which need this clock
    ad4630_cont_conversions_start();

    if (bsp_adc_ch0_data_spi_init() != E_NO_ERROR)
    {
        return AD4630_ERROR_CONFIG_ERROR;
    }

    // complete the init; don't use the data!
    mxc_spi_req_t data_spi_req = {
        .spi = bsp_pins_adc_ch0_data_spi_handle,
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
    bsp_pins_adc_ch0_data_spi_handle->ctrl0 &= ~(MXC_F_SPI_CTRL0_EN);

    // clear the fifo, start only on pos edge of Slave-sel-B
    MXC_SPI_ClearRXFIFO(bsp_pins_adc_ch0_data_spi_handle);

    ad4630_cont_conversions_stop();

    return AD4630_ERROR_ALL_OK;
}

void ad4630_cont_conversions_start()
{
    gpio_write_pin(&bsp_pins_adc_clk_en_cfg, true);
    gpio_write_pin(&bsp_pins_adc_clk_master_reset_cfg, false);
    gpio_write_pin(&bsp_pins_adc_cs_disable_cfg, false);
}

void ad4630_cont_conversions_stop()
{
    gpio_write_pin(&bsp_pins_adc_clk_en_cfg, false);
    gpio_write_pin(&bsp_pins_adc_clk_master_reset_cfg, true);
    gpio_write_pin(&bsp_pins_adc_cs_disable_cfg, true);
}

/* Private function definitions --------------------------------------------------------------------------------------*/

AD4630_Error_t ad4630_read_reg(AD4630_Register_t reg, uint8_t *out)
{
    cfg_spi_tx_buff[0] = (1 << 7) | (reg >> 8);
    cfg_spi_tx_buff[1] = (uint8_t)reg;
    cfg_spi_tx_buff[2] = AD4630_REG_READ_DUMMY; // the output data will end up here

    gpio_write_pin(&bsp_pins_adc_cfg_spi_cs_out_cfg, false);
    MXC_Delay(4); // TODO are these delays necessary?

    mxc_spi_req_t cfg_spi_req = {
        .spi = bsp_pins_adc_cfg_spi_handle,
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

    if (MXC_SPI_MasterTransaction(&cfg_spi_req) != E_NO_ERROR)
    {
        return AD4630_ERROR_CONFIG_ERROR;
    }

    gpio_write_pin(&bsp_pins_adc_cfg_spi_cs_out_cfg, true);
    MXC_Delay(4); // TODO are these delays necessary?

    *out = cfg_spi_tx_buff[2];

    return AD4630_ERROR_ALL_OK;
}

AD4630_Error_t ad4630_write_reg(AD4630_Register_t reg, uint8_t val)
{
    cfg_spi_tx_buff[0] = 0; // all registers we can write to are 8 bits, so this first byte is always zero for writes
    cfg_spi_tx_buff[1] = (uint8_t)reg;
    cfg_spi_tx_buff[2] = val;

    gpio_write_pin(&bsp_pins_adc_cfg_spi_cs_out_cfg, false);
    MXC_Delay(4); // TODO are these delays necessary?

    mxc_spi_req_t cfg_spi_req = {
        .spi = bsp_pins_adc_cfg_spi_handle,
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

    if (MXC_SPI_MasterTransaction(&cfg_spi_req) != E_NO_ERROR)
    {
        return AD4630_ERROR_CONFIG_ERROR;
    }

    gpio_write_pin(&bsp_pins_adc_cfg_spi_cs_out_cfg, true);
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
