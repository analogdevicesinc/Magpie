/*  Module: DS3231.h
|========================================================================
|
|  DESCRIPTION:     Declarations for global constants, variables and 
|                   funcitons defined in MODULE-XXX.c.
|
|  LANGUAGE:        'C' Language
|
|  TARGET SYSTEM:   MAX32666
|  
|========================================================================
*/

#ifndef DS3231_H_INCLUDE
#define DS3231_H_INCLUDE


#include <stdbool.h>
#include "i2c.h"
#include "mxc_delay.h"

// ------------------------------------------------------------------------
// ------------------------------- Contants -------------------------------

//--------------------------- defines -------------------------------------
#define MAX32666_I2C_OP_DELAY       200U
#define MAX32666_I2C_BUFFER_SIZE         8U
#define DS3231_DEVICE_ADDR    0x68U

//----------------------------Static Defines ------------------------------

// Variable to store the register address for transmission
static uint8_t ds3231_txRegisterAddr = 0U;
// Local variables for recieve buffer
static uint8_t * ds3231_rxLocation;       // Pointer to where next byte rx'd should be stored
static uint8_t ds3231_rxCount;           // Number of bytes to read
// Local variables for transmit buffer
static uint8_t * ds3231_txBuffer;       //  Pointer to next byte to tx
static uint8_t ds3231_txCount;           // Number of bytes to write

// ------------------------------- Typedefs -------------------------------


/**
 * @brief Structure with sensor function pointers
 */
typedef struct {
    int (*init)(mxc_i2c_regs_t *i2c); ///< Pointer to
    int (*read_datetime)(void *buff, void *strbuff);
    int (*read_temperature)(void *temp);
    int (*set_datetime)(struct tm *newTime);
} ds3231_driver_t;
// --------------------- Global Variable Declarations --------------------- 

#ifdef USE_DEBUG_MENU
uint32_t DS3231_readAllRegisters(uint8_t * buffer, uint16_t size);
void DS3231_setTestAlarm(void);
#endif

// ---------------------- Inline Functions Declarations ------------------- 


// ---------------------- Global Functions Declarations ------------------- 

/**
 * @brief Prepare I2C_SensorDriver function pointers
 *
 * @return I2C_SensorDriver instance
 */
ds3231_driver_t DS3231_Open();
uint32_t ds3231_mx_init(mxc_i2c_regs_t *i2c);
uint32_t ds3231_read_datetime(void *buff, void *strbuff);
uint32_t ds3231_read_temperature(void *temp);
uint32_t ds3231_set_datetime(struct tm * newTime);


uint32_t DS3231_init(mxc_i2c_regs_t *i2c);
bool DS3231_initialized(void);
void DS3231_clearInitialized(void);
bool DS3231_testDSPresence(void);
uint32_t DS3231_getDateTime(struct tm* result, char* datetime_str);
uint32_t DS3231_getTemperature(float *result);
uint32_t DS3231_setDateTime(struct tm * newTime);


void DS3231_setAlarm(struct tm * alarmTime);
void DS3231_clearInterrupts(void);
void DS3231_clearDisableInterrupts(void);


uint32_t DS3231_i2c_transfer(mxc_i2c_req_t *req, uint8_t *txData, int txSize, uint8_t *rxData,
                        int rxSize);
uint32_t DS3231_I2C_ReadRegister(uint8_t devAddr, uint8_t regAddr, uint8_t *buffer, uint8_t count);
uint32_t DS3231_I2C_WriteRegister(uint8_t devAddr, uint8_t regAddr, uint8_t *buffer, uint8_t count);


#endif