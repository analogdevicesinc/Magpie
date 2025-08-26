
/* Private includes --------------------------------------------------------------------------------------------------*/

#include <stdio.h>

#include "i2c.h"

#include "bsp_i2c.h"
#include "bsp_pins.h"
#include "real_time_clock.h"
#include "time_helpers.h"

/* Private defines ---------------------------------------------------------------------------------------------------*/

#define DS3231_7_BIT_I2C_ADDR (0x68u)

/* Private enumerations ----------------------------------------------------------------------------------------------*/

/**
 * @brief enumerated DS3231 register addresses are represented here.
 */
typedef enum
{
    DS3231_REGISTER_SECONDS = 0x00u,
    DS3231_REGISTER_MINUTES = 0x01u,
    DS3231_REGISTER_HOUR = 0x02u,
    DS3231_REGISTER_DAY = 0x03u,
    DS3231_REGISTER_DATE = 0x04u,
    DS3231_REGISTER_MONTH = 0x05u,
    DS3231_REGISTER_YEAR = 0x06u,
    DS3231_REGISTER_ALARM1_SECONDS = 0x07u,
    DS3231_REGISTER_ALARM1_MINUTES = 0x08u,
    DS3231_REGISTER_ALARM1_HOUR = 0x09u,
    DS3231_REGISTER_ALARM1_DAY = 0x0Au,
    DS3231_REGISTER_CONTROL = 0x0Eu,
    DS3231_REGISTER_STATUS = 0x0Fu,
    DS3231_REGISTER_TEMP_MSB = 0x11u,
    DS3231_REGISTER_TEMP_LSB = 0x12u,
} DS3231_Register_t;

/**
 * @brief enumerated DS3231 hour register flags are represented here.
 */
typedef enum
{
    DS3231_HOUR_REGISTER_FLAG_10_HOUR_BIT = (1u << 4u),
    DS3231_HOUR_REGISTER_FLAG_20_HOUR_BIT = (1u << 5u),
    DS3231_HOUR_REGISTER_FLAG_12_HR_FORMAT = (1u << 6u),
} DS3231_Hour_Register_Flags_t;

typedef enum
{
    DS3231_MONTH_REGISTER_FLAG_CENTURY = (1u << 7u)
} DS3231_Month_Register_Flags_t;

/**
 * @brief enumerated DS3231 control register flags are represented here.
 */
typedef enum
{
    DS3231_CONTROL_REGISTER_FLAG_A1IE = (1u << 0u),
    DS3231_CONTROL_REGISTER_FLAG_A2IE = (1u << 1u),
    DS3231_CONTROL_REGISTER_FLAG_INTCN = (1u << 2u),
    DS3231_CONTROL_REGISTER_FLAG_RS1 = (1u << 3u),
    DS3231_CONTROL_REGISTER_FLAG_RS2 = (1u << 4u),
    DS3231_CONTROL_REGISTER_FLAG_CONV = (1u << 5u),
    DS3231_CONTROL_REGISTER_FLAG_BBSQW = (1u << 6u),
    DS3231_CONTROL_REGISTER_FLAG_nEOSC = (1u << 7u),
} DS3231_Control_Register_Flags_t;

/**
 * @brief enumerated DS3231 staatus register flags are represented here.
 */
typedef enum
{
    DS3231_STATUS_REGISTER_FLAG_A1F = (1u << 0u),
    DS3231_STATUS_REGISTER_FLAG_A2F = (1u << 1u),
    DS3231_STATUS_REGISTER_FLAG_BSY = (1u << 2u),
    DS3231_STATUS_REGISTER_FLAG_EN32kHz = (1u << 3u),
    DS3231_STATUS_REGISTER_FLAG_RESERVED_1 = (1u << 4u),
    DS3231_STATUS_REGISTER_FLAG_RESERVED_2 = (1u << 5u),
    DS3231_STATUS_REGISTER_FLAG_RESERVED_3 = (1u << 6u),
    DS3231_STATUS_REGISTER_FLAG_OSF = (1u << 7u),
} DS3231_Status_Register_Flags_t;

/* Private function declarations -------------------------------------------------------------------------------------*/

/**
 * @brief `ds3231_i2c_read(r, b, n)` reads `n` bytes from the DS3231 into `b` starting at DS3231 register `r`.
 *
 * @pre the real time clock has been initialized.
 *
 * @param start_reg the enumerated DS3231 register address to start reading at.
 *
 * @param read_buff a buffer to store the DS3231 data into, this will be mutated. Must be at least `num_bytes_to_read` long.
 *
 * @param num_bytes_to_read the number of bytes from the DS3231 to read into `read_buff`.
 *
 * @post `num_bytes_to_read` bytes of data are stored in `read_buff`.
 *
 * @retval `E_NO_ERROR` if successful, else a negative error code
 */
static int ds3231_i2c_read(DS3231_Register_t start_reg, uint8_t *read_buff, uint32_t num_bytes_to_read);

/**
 * @brief `ds3231_write(b, n)` writes `n` bytes of data from `b` to the DS3231 starting at the register address in `b[0]`.
 *
 * @pre the real time clock has been initialized.
 *
 * @param write_buff a buffer of data to write to the DS3231. The byte in `write_buff[0]` must be the starting register
 * address you want to write to. Must be at least `num_bytes_to_write` long. Will not be mutated.
 *
 * @param num_bytes_to_write the number of bytes from `write_buff` to write to the DS3231. Includes the mandatory address
 * at `write_buff[0]`.
 *
 * @post `num_bytes_to_write` bytes of data written from `write_buff` to the DS3231
 *
 * @retval `E_NO_ERROR` if successful, else a negative error code
 */
static int ds3231_i2c_write(uint8_t *write_buff, uint32_t num_bytes_to_write);

/* Public function definitions ---------------------------------------------------------------------------------------*/

int real_time_clock_init()
{
    MXC_GPIO_Config(&bsp_pins_rtc_int_cfg);

    uint8_t write_buff[] = {
        DS3231_REGISTER_CONTROL,             // start at the control reg, we'll increment into the status reg
        DS3231_CONTROL_REGISTER_FLAG_INTCN,  // use the square-wave/int pin for interrupts
        DS3231_STATUS_REGISTER_FLAG_EN32kHz, // enable 32kHz clock out
    };

    // TODO: init the MAX32666 RTC here as well

    return ds3231_i2c_write(write_buff, 3);
}

int real_time_clock_set_datetime(const tm_t *new_time)
{
    int res = E_NO_ERROR;

    uint8_t write_buff[4];

    // do the seconds, minutes, and hours
    write_buff[0] = DS3231_REGISTER_SECONDS;
    write_buff[1] = time_helpers_decimal_0_99_to_bcd8(new_time->tm_sec);
    write_buff[2] = time_helpers_decimal_0_99_to_bcd8(new_time->tm_min);
    write_buff[3] = time_helpers_decimal_0_99_to_bcd8(new_time->tm_hour);
    if ((res = ds3231_i2c_write(write_buff, 4)) != E_NO_ERROR)
    {
        return res;
    }

    // skip over the weekdays at DS3231 register address 0x03, we don't care about this

    // do date, month, year
    write_buff[0] = DS3231_REGISTER_DATE;
    write_buff[1] = time_helpers_decimal_0_99_to_bcd8(new_time->tm_mday);
    write_buff[2] = time_helpers_decimal_0_99_to_bcd8(new_time->tm_mon) + 1; // tm_mon is [0..11], DS3231 months in [1..12]

    // year needs a little extra care
    const int orig_year = new_time->tm_year - 100; // tm is years since 1900, DS3231 since 2000

    if (orig_year >= 100)
    {
        write_buff[2] |= DS3231_MONTH_REGISTER_FLAG_CENTURY; // century flag is in the month register

        int year_10s_and_1s = orig_year - 100; // keep year in [0..99]
        write_buff[3] = time_helpers_decimal_0_99_to_bcd8(year_10s_and_1s);
    }
    else // year is in [0..99], we can use the BCD->decimal helper directly on the year
    {
        write_buff[3] = time_helpers_decimal_0_99_to_bcd8(orig_year);
    }

    if ((res = ds3231_i2c_write(write_buff, 4)) != E_NO_ERROR)
    {
        return res;
    }

    return E_NO_ERROR;
}

int real_time_clock_get_datetime(tm_t *out_time)
{
    const size_t num_bytes_to_read = 7; // seconds, minutes, hours, day (not used), date, month, year
    uint8_t read_buff[num_bytes_to_read];

    int res = E_NO_ERROR;
    if ((res = ds3231_i2c_read(DS3231_REGISTER_SECONDS, read_buff, num_bytes_to_read)) != E_NO_ERROR)
    {
        return res;
    }

    out_time->tm_sec = time_helpers_bcd8_byte_to_decimal(read_buff[0]);
    out_time->tm_min = time_helpers_bcd8_byte_to_decimal(read_buff[1]);
    out_time->tm_hour = time_helpers_bcd8_byte_to_decimal(read_buff[2]); // the 12/~24 bit is always zero since we use 24 hr time
    // we don't care about the weekday field at read_buff[3]
    out_time->tm_mday = time_helpers_bcd8_byte_to_decimal(read_buff[4]);

    const uint8_t month_byte = read_buff[5];
    out_time->tm_mon = time_helpers_bcd8_byte_to_decimal(month_byte & 0x1Fu); // mask out the century bit
    // time struct wants months in [0..11], DS3231 gives us months in [1..12]
    out_time->tm_mon -= 1;

    out_time->tm_year = time_helpers_bcd8_byte_to_decimal(read_buff[6]);
    out_time->tm_year += ((month_byte >> 7) & 0x01u) * 100; // century flag in the month register

    out_time->tm_year += 100; // tm is years since 1900, DS3231 since 2000

    return E_NO_ERROR;
}

int real_time_clock_get_milliseconds(int *out_msec)
{
    // TODO need to interact with MAX32666 RTC here
    return E_NOT_SUPPORTED;
}

int real_time_clock_set_alarm(const tm_t *alarm_time)
{
    int res = E_NO_ERROR;

    uint8_t write_buff[5];

    // with this scheme A1M1..A1M4 bits are all guaranteed to be zero, 12/~24 is set to 24 hour time, and DY/~DT bit is 0
    // this sets an alarm to match on Date, Hours, Minutes, and Seconds
    write_buff[0] = DS3231_REGISTER_ALARM1_SECONDS;
    write_buff[1] = time_helpers_decimal_0_99_to_bcd8(alarm_time->tm_sec) & 0x7Fu;
    write_buff[2] = time_helpers_decimal_0_99_to_bcd8(alarm_time->tm_min) & 0x7Fu;
    write_buff[3] = time_helpers_decimal_0_99_to_bcd8(alarm_time->tm_hour) & 0x3Fu;
    write_buff[4] = time_helpers_decimal_0_99_to_bcd8(alarm_time->tm_mday) & 0x3Fu;
    if ((res = ds3231_i2c_write(write_buff, 5)) != E_NO_ERROR)
    {
        return res;
    }

    // now enable the interrupt
    write_buff[0] = DS3231_REGISTER_CONTROL;
    write_buff[1] = DS3231_CONTROL_REGISTER_FLAG_INTCN | DS3231_CONTROL_REGISTER_FLAG_A1IE;
    if ((res = ds3231_i2c_write(write_buff, 2)) != E_NO_ERROR)
    {
        return res;
    }

    return E_NO_ERROR;
}

/* Private function definitions --------------------------------------------------------------------------------------*/

int ds3231_i2c_read(DS3231_Register_t start_reg, uint8_t *read_buff, uint32_t num_bytes_to_read)
{
    // we need to first send the starting address we want to read from
    read_buff[0] = start_reg;

    mxc_i2c_req_t req = {
        .i2c = bsp_i2c_3v3_i2c_handle,
        .addr = DS3231_7_BIT_I2C_ADDR,
        .tx_buf = read_buff,
        .tx_len = 1,
        .rx_buf = read_buff,
        .rx_len = num_bytes_to_read,
        .restart = 0,
        .callback = NULL,
    };

    return MXC_I2C_MasterTransaction(&req);
}

int ds3231_i2c_write(uint8_t *write_buff, uint32_t num_bytes_to_write)
{
    mxc_i2c_req_t req = {
        .i2c = bsp_i2c_3v3_i2c_handle,
        .addr = DS3231_7_BIT_I2C_ADDR,
        .tx_buf = write_buff,
        .tx_len = num_bytes_to_write,
        .rx_buf = NULL,
        .rx_len = 0,
        .restart = 0,
        .callback = NULL,
    };

    return MXC_I2C_MasterTransaction(&req);
}
