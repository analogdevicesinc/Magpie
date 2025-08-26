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
 * `configure_adc_to_output_host_clock()` configures the AD4630 to output a 20MHz clock on its busy pin while the AD4630
 * chip select pin is pulled low.
 *
 * @pre all preconditions for `ad4630_init()` are met.
 *
 * @post the AD4630 outputs a 20MHz SPI clock signal on its busy pin when the CS signal is enabled via
 * `ad4630_cont_conversions_start()`
 *
 * @retval `E_NO_ERROR` if successful, else a negative error code
 */
static int configure_adc_to_output_host_clock(void);

/**
 * `configure_data_spi_bus_to_receive_data_from_adc(hspi)` configures the SPI bus `hspi` responsible for reading the
 * AD4630 channel 0/1 data.
 *
 * @pre all preconditions for `ad4630_init()` are met, `configure_adc_to_output_host_clock()` has been called, and
 * the 384kHz chip select signal is turned on via `ad4630_cont_conversions_start()`
 *
 * @param hspi the SPI bus to use, must be either the SPI bus dedicated to ADC channel 0 or ADC channel 1
 *
 * @post the given SPI bus is configured as a SPI slave to the AD4630
 *
 * @retval `E_NO_ERROR` if successful, else a negative error code
 */
static int configure_data_spi_bus_to_receive_data_from_adc(mxc_spi_regs_t *hspi);

/**
 * @brief `ad4630_read_reg(r, o)` reads register `r` and stores the value in out pointer `o`
 *
 * @param reg the enumerated register to read
 *
 * @param out [out] pointer to the byte to read into
 *
 * @pre the AD4630 has been placed in register access mode
 *
 * @retval `E_NO_ERROR` if successful, else a negative error code
 */
static int ad4630_read_reg(AD4630_Register_t reg, uint8_t *out);

/**
 * @brief `ad4630_write_reg(r, v)` writes value `v` to register `r`
 *
 * @param reg the enumerated register to write to
 *
 * @param val the value to write to the register
 *
 * @pre the AD4630 has been placed in register access mode
 *
 * @post the value is written to the given register
 *
 * @retval `E_NO_ERROR` if successful, else a negative error code
 */
static int ad4630_write_reg(AD4630_Register_t reg, uint8_t val);

/* Public function definitions ---------------------------------------------------------------------------------------*/

int ad4630_init()
{
    int res = E_NO_ERROR;

    // these 3 pins control the 384k ADC clock circuit
    MXC_GPIO_Config(&bsp_pins_adc_clk_en_cfg);
    MXC_GPIO_Config(&bsp_pins_adc_clk_master_reset_cfg);

    //Set power supply in burst mode for low power
    MXC_GPIO_Config(&bsp_pins_bb_mode_active_cfg);
    //MXC_GPIO_Config(&bsp_pins_adc_cs_disable_cfg);

    // start with the 384k clock disabled and U2 output set to high-z
    ad4630_384kHz_fs_clk_and_cs_stop();

    // must be high for ADC to run
    MXC_GPIO_Config(&bsp_pins_adc_n_reset_cfg);

    // used to check for partial samples, do the config right away to make sure
    // it's high-Z, otherwise there will be contention with the SPI CS
    MXC_GPIO_Config(&bsp_pins_adc_cs_check_pin_cfg);

    // reset the ADC
    gpio_write_pin(&bsp_pins_adc_n_reset_cfg, false);
    MXC_Delay(100000);
    gpio_write_pin(&bsp_pins_adc_n_reset_cfg, true);

    // after this call the AD4630 will output the 20MHz SPI clock on the BUSY pin
    if ((res = configure_adc_to_output_host_clock()) != E_NO_ERROR)
    {
        return res;
    }

    MXC_Delay(100000);

    // after this SPI is ready to receive samples from the AD4630
    ad4630_384kHz_fs_clk_and_cs_start();
    if ((res = configure_data_spi_bus_to_receive_data_from_adc(bsp_spi_adc_ch0_data_spi_handle)) != E_NO_ERROR)
    {
        return res;
    }
    if ((res = configure_data_spi_bus_to_receive_data_from_adc(bsp_spi_adc_ch1_data_spi_handle)) != E_NO_ERROR)
    {
        return res;
    }
    ad4630_384kHz_fs_clk_and_cs_stop();

    return E_NO_ERROR;
}

void ad4630_384kHz_fs_clk_and_cs_start()
{
    gpio_write_pin(&bsp_pins_adc_clk_master_reset_cfg, false);
    //gpio_write_pin(&bsp_pins_adc_cs_disable_cfg, false);
    gpio_write_pin(&bsp_pins_adc_clk_en_cfg, true);
}

void ad4630_384kHz_fs_clk_and_cs_stop()
{
    gpio_write_pin(&bsp_pins_adc_clk_en_cfg, false);
    //gpio_write_pin(&bsp_pins_adc_cs_disable_cfg, true);
    gpio_write_pin(&bsp_pins_adc_clk_master_reset_cfg, true);
}

/* Private function definitions --------------------------------------------------------------------------------------*/

int configure_adc_to_output_host_clock()
{
    int res = E_NO_ERROR;

    // enable the SPI port and configure the GPIO pins associated with the SPI bus responsible for ADC configuration
    if ((res = bsp_adc_config_spi_init()) != E_NO_ERROR)
    {
        return res;
    }

    // freq doesn't matter too much, we only init rarely and write to a few registers
    const uint32_t CFG_SPI_CLK_FREQ_Hz = 5000000;

    if ((res = MXC_SPI_Init(
             bsp_spi_adc_cfg_spi_handle,
             1, // 1 -> master mode
             0, // 0 -> quad mode not used, single bit SPI
             0, // num slaves
             0, // CS polarity (0 for active low)
             CFG_SPI_CLK_FREQ_Hz,
             MAP_A // note that the MAP is irrelevant because we define MSDK_NO_GPIO_CLK_INIT
             )) != E_NO_ERROR)
    {
        return res;
    }
    if ((res = MXC_SPI_SetDataSize(bsp_spi_adc_cfg_spi_handle, 8)) != E_NO_ERROR)
    {
        return res;
    }
    if ((res = MXC_SPI_SetWidth(bsp_spi_adc_cfg_spi_handle, SPI_WIDTH_STANDARD)) != E_NO_ERROR)
    {
        return res;
    }
    if ((res = MXC_SPI_SetMode(bsp_spi_adc_cfg_spi_handle, SPI_MODE_0)) != E_NO_ERROR)
    {
        return res;
    }

    // begin register access mode
    uint8_t dummy = 0;
    if ((res = ad4630_read_reg(AD4630_REG_CONFIG_MODE_SPECIAL_CONSTANT, &dummy)) != E_NO_ERROR)
    {
        return res;
    }
    if ((res = ad4630_write_reg(AD4630_REG_OSCILLATOR, AD4630_OSCILLATOR_FLAG_OSC_DIV_BY_4)) != E_NO_ERROR)
    {
        return res;
    }
    if ((res = ad4630_write_reg(AD4630_REG_MODES, AD4630_MODES_FLAG_CLK_MD_HOST_CLK_MODE)) != E_NO_ERROR)
    {
        return res;
    }
    // end register access mode
    if ((res = ad4630_write_reg(AD4630_REG_EXIT_CFG_MD, AD4630_EXIT_CONFIG_MODE_FLAG_EXIT)) != E_NO_ERROR)
    {
        return res;
    }

    // shutdown the config SPI, we're done with it
    return bsp_adc_config_spi_deinit();
}

int configure_data_spi_bus_to_receive_data_from_adc(mxc_spi_regs_t *hspi)
{
    int res = E_NO_ERROR;

    // enable the SPI port and configure the GPIO pins associated with the SPI bus responsible reading ADC channel 0
    if (hspi == bsp_spi_adc_ch0_data_spi_handle)
    {
        if ((res = bsp_adc_ch0_data_spi_init()) != E_NO_ERROR)
        {
            return res;
        }
    }
    else if (hspi == bsp_spi_adc_ch1_data_spi_handle)
    {
        if ((res = bsp_adc_ch1_data_spi_init()) != E_NO_ERROR)
        {
            return res;
        }
    }
    else // it's not one of the two SPI busses that reads data from the ADC
    {
        return E_BAD_PARAM;
    }

    if ((res = MXC_SPI_Init(
             hspi,
             0,    // 0 -> slave mode
             0,    // 0 -> quad mode not used, single bit SPI
             0,    // num slaves, none
             0,    // CS polarity (0 for active low)
             0,    // freq is defined by the driving clock
             MAP_A // note that the MAP is irrelevant because we define MSDK_NO_GPIO_CLK_INIT
             )) != E_NO_ERROR)
    {
        return res;
    }
    if ((res = MXC_SPI_SetWidth(hspi, SPI_WIDTH_3WIRE)) != E_NO_ERROR)
    {
        return res;
    }
    if ((res = MXC_SPI_SetMode(hspi, SPI_MODE_1)) != E_NO_ERROR)
    {
        return res;
    }

    // complete the init; don't use the data!
    mxc_spi_req_t data_spi_req = {
        .spi = hspi,
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

    if ((res = MXC_SPI_SlaveTransactionAsync(&data_spi_req)) != E_NO_ERROR)
    {
        return res;
    }

    // threshold of 24 bytes ( 8 samples of 3 bytes each) to trigger dma
    if ((res = MXC_SPI_SetRXThreshold(hspi, 24)) != E_NO_ERROR)
    {
        return res;
    }

    // disable the port
    hspi->ctrl0 &= ~(MXC_F_SPI_CTRL0_EN);

    MXC_SPI_ClearRXFIFO(hspi);

    return E_NO_ERROR;
}

int ad4630_read_reg(AD4630_Register_t reg, uint8_t *out)
{
    cfg_spi_tx_buff[0] = (1 << 7) | (reg >> 8);
    cfg_spi_tx_buff[1] = (uint8_t)reg;
    cfg_spi_tx_buff[2] = AD4630_REG_READ_DUMMY; // the output data will end up here

    gpio_write_pin(&bsp_pins_adc_cfg_spi_cs_out_cfg, false);
    MXC_Delay(4); // TODO are these delays necessary?

    mxc_spi_req_t cfg_spi_req = {
        .spi = bsp_spi_adc_cfg_spi_handle,
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

    int res = E_NO_ERROR;
    if ((res = MXC_SPI_MasterTransaction(&cfg_spi_req)) != E_NO_ERROR)
    {
        return res;
    }

    gpio_write_pin(&bsp_pins_adc_cfg_spi_cs_out_cfg, true);
    MXC_Delay(4); // TODO are these delays necessary?

    *out = cfg_spi_tx_buff[2];

    return E_NO_ERROR;
}

int ad4630_write_reg(AD4630_Register_t reg, uint8_t val)
{
    cfg_spi_tx_buff[0] = 0; // all registers we can write to are 8 bits, so this first byte is always zero for writes
    cfg_spi_tx_buff[1] = (uint8_t)reg;
    cfg_spi_tx_buff[2] = val;

    gpio_write_pin(&bsp_pins_adc_cfg_spi_cs_out_cfg, false);
    MXC_Delay(4); // TODO are these delays necessary?

    mxc_spi_req_t cfg_spi_req = {
        .spi = bsp_spi_adc_cfg_spi_handle,
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

    int res = E_NO_ERROR;
    if ((res = MXC_SPI_MasterTransaction(&cfg_spi_req)) != E_NO_ERROR)
    {
        return res;
    }

    gpio_write_pin(&bsp_pins_adc_cfg_spi_cs_out_cfg, true);
    MXC_Delay(4); // TODO are these delays necessary?

    return E_NO_ERROR;
}
