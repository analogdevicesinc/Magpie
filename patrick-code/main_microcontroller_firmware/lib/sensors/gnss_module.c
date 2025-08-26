
/* Private includes --------------------------------------------------------------------------------------------------*/

#include <stdio.h>

#include "tmr.h"

#include "board.h"
#include "bsp_pins.h"
#include "bsp_uart.h"
#include "gnss_module.h"
#include "real_time_clock.h"

#include "minmea.h" // 3rd party NMEA parsing lib

/* Private defines ---------------------------------------------------------------------------------------------------*/

#define START_OF_NMEA_SENTENCE ('$')
#define END_OF_NMEA_SENTENCE ('\n')

/* Private enumerations ----------------------------------------------------------------------------------------------*/

/**
 * @brief enumerated NMEA parser states are represented here. We build up NMEA strings from chars received from the GPS
 * UART.
 */
typedef enum
{
    NMEA_PARSER_STATE_WAITING,         // waiting for the start of sentence char '$
    NMEA_PARSER_STATE_BUILDING_STRING, // in the middle of building up a string
    NMEA_PARSER_STATE_LINE_COMPLETE,   // we saw the end of sentence char '\n'
} NMEA_Parser_State_t;

/* Private function declarations -------------------------------------------------------------------------------------*/

/**
 * @brief `is_ascii(c)` is true iff integer `c` represents a valid ascii character
 */
static bool is_ascii(int c);

/* Public function definitions ---------------------------------------------------------------------------------------*/

int gnss_module_init()
{
    // power up the GNSS module
    MXC_GPIO_Config(&bsp_pins_gps_en_cfg);
    gpio_write_pin(&bsp_pins_gps_en_cfg, true);

    // set up the PPS pin
    MXC_GPIO_Config(&bsp_pins_gps_pps_cfg);

    // configure the UART pins & clock for communicating with the GNSS module
    return bsp_gnss_uart_init();
}

int gnss_module_deinit()
{
    gpio_write_pin(&bsp_pins_gps_en_cfg, false);

    return bsp_gnss_uart_deinit();
}

int gnss_module_sync_RTC_to_GNSS_time(int timeout_sec)
{
    // a buffer to build the NMEA strings into, chars from the GPS/UART will fill this string
    char nmea_line[MINMEA_MAX_SENTENCE_LENGTH];
    int nmea_str_pos = 0;

    NMEA_Parser_State_t parser_state = NMEA_PARSER_STATE_WAITING;

    // GGA quality is an integer in 0..9
    int gga_quality = 0;

    // TODO: we should probably move the timeout timer into its own module so it can be reused by other modules
    MXC_TMR_TO_Start(MXC_TMR0, timeout_sec * 1000 * 1000); // convert to usec as needed by the MXC function

    while (MXC_TMR_TO_Check(MXC_TMR0) != E_TIME_OUT) // we'll either return successfully from within this loop, or timeout
    {
        // this will either be a char if there is a char from the GNSS and the read is successful, or a negative error
        const int uart_res = MXC_UART_ReadCharacterRaw(bsp_uart_gps_uart_handle);

        if (!is_ascii(uart_res))
        {
            // skip this round if we get a non-ascii result from the UART read (it returns negative codes on failure)
            continue;
        }

        const char this_char = (char)uart_res;

        switch (parser_state)
        {
        case NMEA_PARSER_STATE_WAITING:

            nmea_str_pos = 0;

            if (this_char == START_OF_NMEA_SENTENCE)
            {
                nmea_line[0] = START_OF_NMEA_SENTENCE;
                nmea_str_pos = 1;

                parser_state = NMEA_PARSER_STATE_BUILDING_STRING;
            }
            break; // case NMEA_PARSER_STATE_WAITING

        case NMEA_PARSER_STATE_BUILDING_STRING:

            nmea_line[nmea_str_pos] = this_char;
            nmea_str_pos++;

            if (nmea_str_pos >= MINMEA_MAX_SENTENCE_LENGTH)
            {
                // if our string gets too long it can't be a valid sentence, go back to wwating to start a new sentence
                parser_state = NMEA_PARSER_STATE_WAITING;
            }
            else if (this_char == END_OF_NMEA_SENTENCE)
            {
                nmea_line[nmea_str_pos] = 0; // null-terminate the string
                parser_state = NMEA_PARSER_STATE_LINE_COMPLETE;
            }
            break; // case NMEA_PARSER_STATE_BUILDING_STRING

        case NMEA_PARSER_STATE_LINE_COMPLETE:
        {
            // here is where most of the actual work happens, we got an NMEA string, now we parse it to get the GPS info
            const bool strict_mode = false; // if true, minmea only allows checksummed sentences

            const enum minmea_sentence_id id = minmea_sentence_id(nmea_line, strict_mode);

            if (id == MINMEA_SENTENCE_GGA) // GGA gives us the fix-quality, we can use this to make sure we have a good fix
            {
                struct minmea_sentence_gga frame;
                if (minmea_parse_gga(&frame, nmea_line))
                {
                    gga_quality = frame.fix_quality;
                }

                // we either got a fix quality from GGA or not, either way we need to build at least 1 more NMEA string
                parser_state = NMEA_PARSER_STATE_WAITING;
            }
            else if (id == MINMEA_SENTENCE_RMC) // RMC has the datetime, this is where we actually sync the RTC
            {
                struct minmea_sentence_rmc frame;
                if (minmea_parse_rmc(&frame, nmea_line))
                {
                    // only sync the time if we are sure the GPS signal is good
                    if (gga_quality >= 1 && frame.valid) // TODO: learn more about GNSS, is this a good way to check if we have a good GPS connection?
                    {
                        tm_t gps_time;

                        if (minmea_getdatetime(&gps_time, &frame.date, &frame.time) == E_NO_ERROR)
                        {
                            // TODO: also work out how to use the PPS signal to sync time right at a second boundary

                            if (real_time_clock_set_datetime(&gps_time) == E_NO_ERROR)
                            {
                                return E_NO_ERROR;
                            }
                        }
                    }

                    // we saw an RMC sentence but we weren't able to sync the RTC
                    // we can keep try again with new NMEA strings until we time out
                    parser_state = NMEA_PARSER_STATE_WAITING;
                }
            }
            else // we got some other NMEA sentence that is not relevant right now, start over with  a new NMEA sentence
            {
                parser_state = NMEA_PARSER_STATE_WAITING;
            }

            break; // case NMEA_PARSER_STATE_LINE_COMPLETE
        }
        } // switch on parser_State
    } // while !timeout

    return E_TIME_OUT;
}

/* Private function definitions --------------------------------------------------------------------------------------*/

bool is_ascii(int c)
{
    return (0 <= c && c <= 127);
}
