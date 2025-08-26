/**
 * Description:
 * This file represents a software module for controlling the power and communication line routing of the SD card bank.
 * A MAX7312 16-IO I2C GPIO port expander is used for all the IO pins. There are 3 main groupings of pins:
 *
 * 1) 4 pins to control the MUX which routes the SD card communication lines. The MUX is comprised of 3x MAX4999 chips.
 * These are 2-pole 8-throw switches meant for routing USB signals, but they work well in this application as well.
 * With 3 of these chips, we have 6 poles and 8 throws. We use only 6 of the 8 throws available, because we have only 6
 * SD cards. This module would accommodate up to 8 SD cards if they were installed on the PCB. 6 cards was chosen because
 * it is enough storage for our needs without wasting parts. The control lines of the MAX4999 are all wired in parallel.
 * There is one ENABLE line and three SELECT lines for a total of 4 pins to control the MUX.
 *
 * 2) 6 enable pins to power on the active SD card slot. In typical use a single slot can be active at a time.
 *
 * 3) 6 detect pins for the SD card detect pins. An SD card is detected by physically closing a switch to ground when
 * a card is inserted in the slot. There is a pullup resistor so the logic is inverting- high means "no card inserted
 * and low means "yes a card is inserted". We invert the logic again using the MAX7312 input invert register, so when
 * we read a "1" from the appropriate MAX7312 input register it means that a card is indeed inserted. This extra
 * inversion simplifies the reading and gives us an intuitive logical true for "yes there is a card there".
 *
 * Much of the code for interacting with the MAX7312 depends on how the PCB is routed, pin functions were chosen to
 * make PCB routing clean. If the PCB changes for any reason, much of this code will need to be tweaked.
 */

/* Private includes --------------------------------------------------------------------------------------------------*/

#include "bsp_i2c.h"
#include "bsp_pins.h"
#include "sd_card_bank_ctl.h"
#include <stddef.h> // for NULL

/* Private defines ---------------------------------------------------------------------------------------------------*/

// this address assumes that A0..A2 are tied low on the PCB
#define MAX7312_7_BIT_I2C_ADDR (0x20u)

// mux control pins are in port 0, from bit 0..3
#define MAX7312_MUX_EN_PORT0_POS (1u << 0u)
#define MAX7312_MUX_C0_PORT0_POS (1u << 1u)
#define MAX7312_MUX_C1_PORT0_POS (1u << 2u)
#define MAX7312_MUX_C2_PORT0_POS (1u << 3u)

#define MAX7312_MUX_CTL_PINS_IN_PORT0_MASK (uint8_t)( \
    MAX7312_MUX_EN_PORT0_POS |                        \
    MAX7312_MUX_C0_PORT0_POS |                        \
    MAX7312_MUX_C1_PORT0_POS |                        \
    MAX7312_MUX_C2_PORT0_POS)

// the first chunk of enable pins are in port 0, from bits 4..7
#define MAX7312_SD_EN_0_PORT0_POS (1u << 4u)
#define MAX7312_SD_EN_1_PORT0_POS (1u << 5u)
#define MAX7312_SD_EN_2_PORT0_POS (1u << 6u)
#define MAX7312_SD_EN_3_PORT0_POS (1u << 7u)

#define MAX7312_SD_EN_PINS_IN_PORT0_MASK (uint8_t)( \
    MAX7312_SD_EN_0_PORT0_POS |                     \
    MAX7312_SD_EN_1_PORT0_POS |                     \
    MAX7312_SD_EN_2_PORT0_POS |                     \
    MAX7312_SD_EN_3_PORT0_POS)

// the last two enable pins are in port 1, bits 0..1
#define MAX7312_SD_EN_4_PORT1_POS (1u << 0u)
#define MAX7312_SD_EN_5_PORT1_POS (1u << 1u)

#define MAX7312_SD_EN_PINS_IN_PORT1_MASK (uint8_t)( \
    MAX7312_SD_EN_4_PORT1_POS |                     \
    MAX7312_SD_EN_5_PORT1_POS)

// SD detect pins are in port 1 starting at bit 2, bits 2..7
#define MAX7312_SD_DETECT_0_PORT1_POS (1u << 2u)
#define MAX7312_SD_DETECT_1_PORT1_POS (1u << 3u)
#define MAX7312_SD_DETECT_2_PORT1_POS (1u << 4u)
#define MAX7312_SD_DETECT_3_PORT1_POS (1u << 5u)
#define MAX7312_SD_DETECT_4_PORT1_POS (1u << 6u)
#define MAX7312_SD_DETECT_5_PORT1_POS (1u << 7u)

#define MAX7312_SD_DETECT_PINS_IN_PORT1_MASK (uint8_t)( \
    MAX7312_SD_DETECT_0_PORT1_POS |                     \
    MAX7312_SD_DETECT_1_PORT1_POS |                     \
    MAX7312_SD_DETECT_2_PORT1_POS |                     \
    MAX7312_SD_DETECT_3_PORT1_POS |                     \
    MAX7312_SD_DETECT_4_PORT1_POS |                     \
    MAX7312_SD_DETECT_5_PORT1_POS)

/* Private enumerations ----------------------------------------------------------------------------------------------*/

/**
 * Enumerated MAX7312 register addresses are represented here, taken from the datasheet
 */
typedef enum uint8_t
{
    MAX7312_REG_INPUT_PORT_0 = (0x00u),
    MAX7312_REG_INPUT_PORT_1 = (0x01u),
    MAX7312_REG_OUTPUT_PORT_0 = (0x02u),
    MAX7312_REG_OUTPUT_PORT_1 = (0x03u),
    MAX7312_REG_POLARITY_INV_0 = (0x04u),
    MAX7312_REG_POLARITY_INV_1 = (0x05u),
    MAX7312_REG_CONFIG_0 = (0x06u),
    MAX7312_REG_CONFIG_1 = (0x07u),
    MAX7312_REG_TIMEOUT = (0x08u),
    MAX7312_REG_RESERVED = (0xFFu),
} MAX7312_Register_Addr_t;

/* Private variables -------------------------------------------------------------------------------------------------*/

// a buffer for MAX7312 reads/writes, we need a maximum of two bytes
#define MAX7312_I2C_BUFF_LEN (2u)
static uint8_t max_7312_i2c_buff[MAX7312_I2C_BUFF_LEN];

// there is at most one active card at a time, or all cards can be disabled
static SD_Card_Bank_Card_Slot_t active_card_slot = SD_CARD_BANK_ALL_SLOTS_DISABLED;

// Detect pins are stored as a bitfield with a set bit indicating that a card was detected during the last check.
// The lower 6 bits of the u8 are used, with SD card 0 in the LSB.
// Since we typically only power on one card at a time, we expect at most one bit to be set during normal operation.
static uint8_t detect_pins_bitfield;

/* Private function declarations -------------------------------------------------------------------------------------*/

/**
 * @brief `max7312_reg_write(r, v)` writes value `v` to max7312 register `r`.
 *
 * @param reg: the register to write to, must be a valid MAX7312 reg as defined in the datasheet.
 *
 * @param val: the value to write, note that pins configured as inputs are are read only and writes are ignored.
 *
 * @pre I2C is configured as a master and has pullups to +3.3V.
 *
 * @post the data byte is written to the given register of the MAX7312.
 *
 * @retval `E_NO_ERROR` if successful, else a negative error code
 */
static int max7312_reg_write(MAX7312_Register_Addr_t reg, uint8_t val);

/**
 * @brief `max7312_reg_read(r, b)` reads the 8-bit value of max7312 register `r` and stores the result in pointer `b`
 *
 * @param reg: the register to read, must be a valid MAX7312 reg as defined in the datasheet.
 *
 * @param read_byte [out]: pointer to the byte to store the result in.
 *
 * @pre I2C is configured as a master and has pullups to +3.3V.
 *
 * @post the 8-bit value of the register to be read will be stored in read_byte.
 *
 * @retval `E_NO_ERROR` if successful, else a negative error code
 */
static int max7312_reg_read(MAX7312_Register_Addr_t reg, uint8_t *read_byte);

/* Public function definitions ---------------------------------------------------------------------------------------*/

int sd_card_bank_ctl_init()
{
    int res = E_NO_ERROR;

    // Reset all ports to zero, they initialize to 1 on power up.
    if ((res = sd_card_bank_ctl_disable_all()) != E_NO_ERROR)
    {
        return res;
    }

    // The following steps set the appropriate pins to input/output modes, only the SD detect pins are inputs.

    // The the mux control pins and enable pins for cards 0..3 are in port0. Clearing a bit sets the pin as an output.
    const uint8_t port_0_output_pins = (MAX7312_MUX_CTL_PINS_IN_PORT0_MASK | MAX7312_SD_EN_PINS_IN_PORT0_MASK);
    if ((res = max7312_reg_write(MAX7312_REG_CONFIG_0, (uint8_t)~port_0_output_pins)) != E_NO_ERROR)
    {
        return res;
    }

    // The enable pins for sd cards 4 and 5 are in port1. Clearing a bit sets the pin as an output.
    const uint8_t port_1_output_pins = MAX7312_SD_EN_PINS_IN_PORT1_MASK;
    if ((res = max7312_reg_write(MAX7312_REG_CONFIG_1, ~port_1_output_pins)) != E_NO_ERROR)
    {
        return res;
    }

    // The card detect pins have a pullup which is shorted to gnd when a card is inserted.
    // We invert the pins so that when a card is detected the port reads back the value 1.
    // All six card detect input pins are in port1.
    const uint8_t port_1_detect_pins = MAX7312_SD_DETECT_PINS_IN_PORT1_MASK;
    return max7312_reg_write(MAX7312_REG_POLARITY_INV_1, port_1_detect_pins); // setting a bit inverts the input polarity
}

int sd_card_bank_ctl_disable_all()
{
    int res = E_NO_ERROR;

    active_card_slot = SD_CARD_BANK_ALL_SLOTS_DISABLED;

    if ((res = max7312_reg_write(MAX7312_REG_OUTPUT_PORT_0, 0x00u)) != E_NO_ERROR)
    {
        return res;
    }

    return max7312_reg_write(MAX7312_REG_OUTPUT_PORT_1, 0x00u);
}

int sd_card_bank_ctl_enable_slot(SD_Card_Bank_Card_Slot_t slot)
{
    int res = E_NO_ERROR;

    // early return on bad inputs
    if (slot >= SD_CARD_BANK_CTL_NUM_CARDS)
    {
        return E_BAD_PARAM;
    }

    // turn all cards off so we don't have any overlapping cards powered on at the same time
    if ((res = sd_card_bank_ctl_disable_all()) != E_NO_ERROR)
    {
        return res;
    }

    // return early if we want to disable all slots, there is nothing else to do
    if (slot == SD_CARD_BANK_ALL_SLOTS_DISABLED)
    {
        return E_NO_ERROR;
    }

    uint8_t port_0_data = 0x00u;
    uint8_t port_1_data = 0x00u;

    // We want to turn on only one single card, since the enable pins span across port0
    // and port1, we need to shift things around to write the correct bits to the ports.
    // The magic number "4" depends on how the pins are split across port0 and port1.
    if (slot < 4u)
    {
        port_0_data = 1u << (slot + 4u);
    }
    else
    {
        port_1_data = 1u << (slot - 4u);
    }

    // Now we need to set the MUX control pins to steer the analog switches. All MUX control pins are in port 0,
    // The MUX EN pin is in position 0, and the C0..C2 pins are in bits 1..3
    // The magic numbers here depend on how the pins are routed.
    port_0_data |= (1u | (slot << 1u));

    if ((res = max7312_reg_write(MAX7312_REG_OUTPUT_PORT_0, port_0_data)) != E_NO_ERROR)
    {
        return res;
    }

    if ((res = max7312_reg_write(MAX7312_REG_OUTPUT_PORT_1, port_1_data)) != E_NO_ERROR)
    {
        return res;
    }

    active_card_slot = slot;

    return E_NO_ERROR;
}

SD_Card_Bank_Card_Slot_t sd_card_bank_ctl_get_active_slot()
{
    return active_card_slot;
}

int sd_card_bank_ctl_read_and_cache_detect_pins()
{
    int res = E_NO_ERROR;

    uint8_t read_byte;

    if ((res = max7312_reg_read(MAX7312_REG_INPUT_PORT_1, &read_byte)) != E_NO_ERROR)
    {
        return res;
    }

    // The detect pins are in bits 2..7 of port 1, so we shift the bitfield representing the card detect pins
    // such that they are represented by a bitfield encompassing bits 0..5 of a uint8.
    // Detect pins are pulled low when a card is inserted, but the input pins are set as inverting in the init routine.
    // The magic numbers here depend on how the pins are routed.
    detect_pins_bitfield = read_byte >> 2u;

    return E_NO_ERROR;
}

bool sd_card_bank_ctl_active_card_is_inserted()
{
    if (active_card_slot == SD_CARD_BANK_ALL_SLOTS_DISABLED)
    {
        return false;
    }

    // in order for this to be valid, sd_card_bank_read_detect_pins() must have been called recently
    const uint8_t pin_pos = 1u << active_card_slot;
    return (pin_pos & detect_pins_bitfield) != 0u;
}

/* Private function definitions --------------------------------------------------------------------------------------*/

int max7312_reg_write(MAX7312_Register_Addr_t reg, uint8_t val)
{
    // don't try to write to registers that are forbidden or invalid to write to
    if (!((MAX7312_REG_OUTPUT_PORT_0 <= reg) && (reg <= MAX7312_REG_TIMEOUT)))
    {
        return E_BAD_PARAM;
    }

    const uint16_t num_bytes_to_write = 2u;
    max_7312_i2c_buff[0u] = reg;
    max_7312_i2c_buff[1u] = val;

    mxc_i2c_req_t req = {
        .i2c = bsp_i2c_3v3_i2c_handle,
        .addr = MAX7312_7_BIT_I2C_ADDR,
        .tx_buf = max_7312_i2c_buff,
        .tx_len = num_bytes_to_write,
        .rx_buf = NULL,
        .rx_len = 0,
        .restart = 0,
        .callback = NULL,
    };

    return MXC_I2C_MasterTransaction(&req);
}

int max7312_reg_read(MAX7312_Register_Addr_t reg, uint8_t *read_byte)
{
    const uint16_t num_bytes_to_write = 1u;
    const uint16_t num_bytes_to_read = 1u;
    max_7312_i2c_buff[0u] = reg;

    mxc_i2c_req_t req = {
        .i2c = bsp_i2c_3v3_i2c_handle,
        .addr = MAX7312_7_BIT_I2C_ADDR,
        .tx_buf = max_7312_i2c_buff,
        .tx_len = num_bytes_to_write,
        .rx_buf = max_7312_i2c_buff,
        .rx_len = num_bytes_to_read,
        .restart = 0,
        .callback = NULL,
    };
    const int res = MXC_I2C_MasterTransaction(&req);

    *read_byte = max_7312_i2c_buff[0u];

    return res;
}
