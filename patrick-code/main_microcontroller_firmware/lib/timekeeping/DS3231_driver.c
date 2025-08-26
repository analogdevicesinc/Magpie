/*  Module: DS3231.c
|========================================================================
|
|  DESCRIPTION:     Module to handle interacting with the DS3231 RTC IC.
|
|  LANGUAGE:        'C' Language
|
|  TARGET SYSTEM:   MAX32666
|
|========================================================================
*/
// --------------------------------------------------------------------------

// ---------------------------  Includes   ------------------------------
#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include "DS3231_driver.h"

// -------------------------- Debug Defines --------------------------


// -------------------------- Local Defines ---------------------------
#define DS3231_BUFFER_LEN         32U
#define DS3231_DATETIME_LEN       7U // Number of bytes needed to read/write the date  
#define DS3231_TEMPERATURE_LEN    2U 

// DS3231 Register Addresses
#define SEC_REG_ADDR                0x00U
#define MIN_REG_ADDR                0x01U
#define HOUR_REG_ADDR               0x02U
#define DAY_REG_ADDR                0x03U
#define DATE_REG_ADDR               0x04U
#define MONTH_REG_ADDR              0x05U
#define YEAR_REG_ADDR               0x06U
#define ALRM1_SEC_REG_ADDR          0x07U
#define ALRM1_MIN_REG_ADDR          0x08U
#define ALRM1_HOUR_REG_ADDR         0x09U
#define ALRM1_DAY_DATE_REG_ADDR     0x0AU
#define ALRM2_MIN_REG_ADDR          0x0BU
#define ALRM2_HOUR_REG_ADDR         0x0CU
#define ALRM2_DAY_DATE_REG_ADDR     0x0DU
#define CNTL_REG_ADDR               0x0EU
#define STATUS_REG_ADDR             0x0FU
#define AGING_OFFSET_REG_ADDR       0x10U
#define TEMP_MSB_REG_ADDR           0x11U
#define TEMP_LSB_REG_ADDR           0x12U
#define DS3231_TEMPERATUREREG       0x11U ///< Temperature register (high byte - low byte is at 0x12), 10-bit
                                          ///< temperature value

// Masks for the Hour register
#define HOUR_12_24_MASK             0x40U
#define HOUR_20HOUR_MASK            0x20U
#define HOUR_10HOUR_MASK            0x10U

// Masks for the Month Register
#define MONTH_CENTURY_MASK          0x80U
#define MONTH_10MONTH_MASK          0x10U

// Masks for the Control Register
#define CNTL_EOSC_MASK              0x80U
#define CNTL_BBSQW_MASK             0x40U
#define CNTL_CONV_MASK              0x20U
#define CNTL_RS2_MASK               0x10U
#define CNTL_RS1_MASK               0x08U
#define CNTL_INTCN_MASK             0x04U
#define CNTL_A2IE_MASK              0x02U
#define CNTL_A1IE_MASK              0x01U

// Masks for the Status Register 
#define STATUS_OSF_MASK             0x80U
#define STATUS_EN32KHZ_MASK         0x08U
#define STATUS_BSY_MASK             0x04U
#define STATUS_A2F_MASK             0x02U
#define STATUS_A1F_MASK             0x01U

// ------------------------- Global Variables -------------------------

//-------------------------- Private Variables ------------------------
mxc_i2c_regs_t *ds3231_i2c_reg;

// ------------------------- Static Variables -------------------------
static bool ds3231_initialized = false;
static uint8_t ds3231_buffer[DS3231_BUFFER_LEN];
static mxc_i2c_req_t ds3231_req; ///< I2C request

// -------------------------- Local Functions -------------------------

/*********************************************************************************************
*
* @name     DS3231_init
*
* @brief Function to initialize the RTC DS3231 IC.
*
* @param    none
*
* @returns  retCode uint32_t,  E_NO_ERROR 0, E_FAIL -255
*
**********************************************************************************************/
uint32_t DS3231_init(mxc_i2c_regs_t *i2c)
{  
  uint32_t retCode = E_NO_ERROR;
  ds3231_initialized = true;

  ds3231_req.i2c = i2c;
  ds3231_req.addr = DS3231_DEVICE_ADDR;
  ds3231_req.tx_buf = NULL;
  ds3231_req.tx_len = 0;
  ds3231_req.rx_buf = NULL;
  ds3231_req.rx_len = 0;
  ds3231_req.restart = 0;
  ds3231_req.callback = NULL;
 
  // Disable Square-wave and alarms
  ds3231_buffer[0] = CNTL_INTCN_MASK;
  // Enable 32kHz signal, and alarms
  ds3231_buffer[1] = STATUS_EN32KHZ_MASK;
  
  if( E_NO_ERROR != DS3231_I2C_WriteRegister(DS3231_DEVICE_ADDR,
                                            CNTL_REG_ADDR,
                                            ds3231_buffer,
                                            2))
  {
    ds3231_initialized = false;  
    retCode = E_FAIL;
  }

  return retCode;

}

/*********************************************************************************************
*
* @name     DS3231_initialized
*
* @brief Function to test if the DS3231 is initialized for use.
*
* @param    none
*
* @returns  bool TRUE if inititialized, FALSE otherwise
*
**********************************************************************************************/
bool DS3231_initialized(void)
{
  return ds3231_initialized;
}

/*********************************************************************************************
*
* @name     DS3231_clearInitialized
*
* @brief Function to clear the DS3231 initialized flag.
*
* @param    none
*
* @returns  none
**********************************************************************************************/
void DS3231_clearInitialized(void)
{
  ds3231_initialized = false;
}

/*********************************************************************************************
*
* @name    DS3231_testDSPresence
*
* @brief Function to test if the DS3231 is present on the I2C bus, by 
*          attempting to read the status register.
* @param    none
*
* @returns  bool true if the read was successful, false otherwise
*
*/
bool DS3231_testDSPresence(void)
{
  if(E_NO_ERROR != DS3231_I2C_ReadRegister(DS3231_DEVICE_ADDR,
                                           STATUS_REG_ADDR,
                                           ds3231_buffer,
                                           1))
  {
    ds3231_initialized = false;
  }
  else
  {
    ds3231_initialized = true;
  }
  return ds3231_initialized;
}

/*********************************************************************************************
*
*@name     DS3231_getTemperature
*
*@brief Function to get temperature data from the DS3231. Data is stored in the ds3231_dateTimeInfo
*        struct tm* pointer to time struct with current DS3231 date / time
*
*@returns  ret_code uint32_t,  E_NO_ERROR = 0
*
**********************************************************************************************/
uint32_t DS3231_getTemperature(float* result)
{
  
  uint32_t ret_code = E_NO_ERROR;

  bool status = true;
  
  // Ensure the device was initialized successfully
  if (false == ds3231_initialized)
  {
    DS3231_init(ds3231_i2c_reg);
  }
  
  if (true == ds3231_initialized)
  {
    // Read the current date / time from the DS3231
    if(E_NO_ERROR != DS3231_I2C_ReadRegister(DS3231_DEVICE_ADDR,
                                DS3231_TEMPERATUREREG,
                                ds3231_buffer,
                                DS3231_TEMPERATURE_LEN))
    {
      status = false;
    }
    
    // Check for successful read
    if (true == status)
    {
      *result = (float)ds3231_buffer[0] + (ds3231_buffer[1] >> 6) * 0.25f;      
    }
    else
    {
      ds3231_initialized = false;
      ret_code = E_FAIL;
    }
  }
  return ret_code;
}


/*********************************************************************************************
*
*@name     DS3231_getDateTime
*
*@brief Function to get the current date time from the DS3231. Data is stored in the ds3231_dateTimeInfo
*        struct tm* pointer to time struct with current DS3231 date / time
*
*@returns  ret_code uint32_t,  E_NO_ERROR = 0
*
**********************************************************************************************/
uint32_t DS3231_getDateTime(struct tm* datetime_result, char* datetime_str)
{
  
  uint32_t ret_code = E_NO_ERROR;

  bool status = true;
  
  // Ensure the device was initialized successfully
  if (false == ds3231_initialized)
  {
    DS3231_init(ds3231_i2c_reg);
  }
  
  if (true == ds3231_initialized)
  {
    // Read the current date / time from the DS3231
    if(E_NO_ERROR != DS3231_I2C_ReadRegister(DS3231_DEVICE_ADDR,
                                SEC_REG_ADDR,
                                ds3231_buffer,
                                DS3231_DATETIME_LEN))
    {
      status = false;
    }
    
    // Check for successful read
    if (true == status)
    {
      // Now parse into the date time struct
      datetime_result->tm_sec = (((ds3231_buffer[0] & 0x70U) >> 4) * 10) + (ds3231_buffer[0] & 0x0FU);
      datetime_result->tm_min = (((ds3231_buffer[1] & 0x70U) >> 4) * 10) + (ds3231_buffer[1] & 0x0FU);
      datetime_result->tm_hour = (ds3231_buffer[2] & 0x0FU);
      if ((ds3231_buffer[2] & HOUR_20HOUR_MASK) == HOUR_20HOUR_MASK)
      {
        datetime_result->tm_hour += 20U; 
      }
      else if ((ds3231_buffer[2] & HOUR_10HOUR_MASK) == HOUR_10HOUR_MASK)
      {
        datetime_result->tm_hour += 10U;
      }
      datetime_result->tm_mday = (((ds3231_buffer[4] & 0x30U) >> 4) * 10) + (ds3231_buffer[4] & 0x0FU);
     
      datetime_result->tm_mon =(ds3231_buffer[5] & 0x0FU);
      if ((ds3231_buffer[5] & MONTH_10MONTH_MASK) == MONTH_10MONTH_MASK)
      {
        datetime_result->tm_mon += 10U;
      }
      // In struct tm, the member tm_mon is 0-11 - not 1-12 as is kept in DS3231
      --(datetime_result->tm_mon);
      
      datetime_result->tm_year = (((ds3231_buffer[6] & 0xF0U) >> 4) * 10) + (ds3231_buffer[6] & 0x0FU);
      if ((ds3231_buffer[5] & MONTH_CENTURY_MASK) == MONTH_CENTURY_MASK)
      {
        datetime_result->tm_year += 100U;
      }
      // In struct tm, the member tm_year is the number of years since 1900. DS3231 is from 2000.
      datetime_result->tm_year += 100U;

      strftime((char*)datetime_str, 17, "%Y%m%d_%H%M%SZ", datetime_result);
		

    }
    else
    {
      ds3231_initialized = false;
      ret_code = E_FAIL;
    }
  }
  return ret_code;
}

/*********************************************************************************************
*
* @name     DS3231_setDateTime
*
* @brief Function to set the date time on the DS3231.
*
* @param    newTime struct tm* pointer to structure containing new date / time info
*
* @returns  retCode uint32_t   E_NO_ERROR   0, E_FAIL   -255
*
**********************************************************************************************/
uint32_t DS3231_setDateTime(struct tm * newTime)
{
  uint32_t retCode = E_NO_ERROR;
  bool status = false;
  uint8_t highNibble = 0;
  uint8_t lowNibble = 0;
  uint8_t month;
  uint8_t year;
  
  // Ensure the device was initialized successfully
  if (false == ds3231_initialized)
  {
    DS3231_init(ds3231_i2c_reg);
  }
  
  if (true == ds3231_initialized)
  {
    // Convert struct to BCD values
    highNibble = ((newTime->tm_sec / 10) << 4);
    lowNibble = ((newTime->tm_sec) % 10);
    ds3231_buffer[0] = highNibble | lowNibble;
    
    highNibble = ((newTime->tm_min / 10) << 4);
    lowNibble = ((newTime->tm_min) % 10);
    ds3231_buffer[1] = highNibble | lowNibble;
    
    highNibble = ((newTime->tm_hour / 10) << 4);
    lowNibble = ((newTime->tm_hour) % 10);
    ds3231_buffer[2] = highNibble | lowNibble;
  
    // Skip over the Day of the Week entry.. 
    // Write the data now
    if(E_NO_ERROR == DS3231_I2C_WriteRegister(DS3231_DEVICE_ADDR,
                        SEC_REG_ADDR,
                        ds3231_buffer,
                        3))
    {
      status = true;
    }
    if (true == status)
    {
      highNibble = ((newTime->tm_mday / 10) << 4);
      lowNibble = ((newTime->tm_mday) % 10);
      ds3231_buffer[0] = highNibble | lowNibble;
      
      // tm_mon is 0-11, while DS3231 represents month as 1-12... add one before setting
      month = newTime->tm_mon + 1U;
      highNibble = (((month) / 10) << 4);
      lowNibble = ((month) % 10);
      ds3231_buffer[1] = highNibble | lowNibble;
     
      // tm_year is years since 1900, while DS3231 is from 2000, so subtract 100 to get 0 = year 2000
      year = newTime->tm_year - 100U;
      if (year >= 100U) 
      {
        ds3231_buffer[1] |= MONTH_CENTURY_MASK;
        year -= 100U;
      }

      highNibble = ((year / 10) << 4);
      lowNibble = (year % 10);
      ds3231_buffer[2] = highNibble | lowNibble;
      
      // Read the current date / time from the DS3231
      if(E_NO_ERROR  != DS3231_I2C_WriteRegister(DS3231_DEVICE_ADDR,
                          DATE_REG_ADDR,
                          ds3231_buffer,
                          3))
      {
        status = false;          
      }
    }
    // If there were problems, clear initialization flag
    ds3231_initialized = status;
  }
  else
  {
    ds3231_initialized = false;
    retCode = E_FAIL;
  }
  return retCode;
}

/*********************************************************************************************
*
* @name     DS3231_setAlarm
*
* @brief Function to enable the DS3231 alarm functionality and set the alarm 
*          to a specific date & time.
*
* @param    alarmTime struct tm* pointer to structure containing dleayed start date / time info
*
* @returns  none
*
**********************************************************************************************/
void DS3231_setAlarm(struct tm * alarmTime)
{
  uint8_t highNibble = 0;
  uint8_t lowNibble = 0;
  
  // Ensure the device was initialized successfully
  if (false == ds3231_initialized)
  {
    DS3231_init(ds3231_i2c_reg);
  }
  
  // In order to set the alarm, previous interrupts must be cleared
  DS3231_clearInterrupts();
  
  if (true == ds3231_initialized)
  {
    // Convert struct to BCD values - 
    // NOTE: High knibble MUST have MSB clear for alarm to work as expected!!
    highNibble = ((alarmTime->tm_sec / 10) << 4) & 0x7FU;
    lowNibble = ((alarmTime->tm_sec) % 10);
    ds3231_buffer[0] = highNibble | lowNibble;
    
    highNibble = ((alarmTime->tm_min / 10) << 4) & 0x7FU;
    lowNibble = ((alarmTime->tm_min) % 10);
    ds3231_buffer[1] = highNibble | lowNibble;
    
    // NOTE: Clearing Bit 7 & 6  of this byte, for alarm to work, and be in 24 hr format
    highNibble = ((alarmTime->tm_hour / 10) << 4) & 0x3FU;
    lowNibble = ((alarmTime->tm_hour) % 10);
    ds3231_buffer[2] = highNibble | lowNibble;
    
    // NOTE: Clearing Bit 7 & 6  of this byte, for alarm to work, and match a date, not Day of the week
    highNibble = ((alarmTime->tm_mday / 10) << 4) & 0x3FU;
    lowNibble = ((alarmTime->tm_mday) % 10);
    ds3231_buffer[3] = highNibble | lowNibble;

    if(E_NO_ERROR != DS3231_I2C_WriteRegister(DS3231_DEVICE_ADDR,
                                              ALRM1_SEC_REG_ADDR,
                                              ds3231_buffer,
                                              4))
    {
      ds3231_initialized = false;
    }
                                            
    // Enable interrupt function and alarm 1 interrupt
    ds3231_buffer[0] = (CNTL_INTCN_MASK | CNTL_A1IE_MASK);
    if(E_NO_ERROR != DS3231_I2C_WriteRegister(DS3231_DEVICE_ADDR, 
                                              CNTL_REG_ADDR, 
                                              ds3231_buffer, 
                                              1))
    {
      ds3231_initialized = false;
    }
    
    // Enable GPIO interrupt used by the alarm 
    //TODO:
    //BOARD_setRtcAlarm(ENABLE);
  }
}

/*********************************************************************************************
*
* @name     DS3231_clearInterrupts
*
* @brief Function to clear the interrupt flags in the DS3231 status register.
*
* @param    none
*
* @returns  none
*
**********************************************************************************************/
void DS3231_clearInterrupts(void)
{
  if (false == ds3231_initialized)
  {
    DS3231_init(ds3231_i2c_reg);
  }
  
  if (true == ds3231_initialized)
  {
    ds3231_buffer[0] = STATUS_EN32KHZ_MASK;
    if(E_NO_ERROR != DS3231_I2C_WriteRegister(DS3231_DEVICE_ADDR, STATUS_REG_ADDR, ds3231_buffer, 1))
    {
      ds3231_initialized = false;
    }
  }
}

/*********************************************************************************************
*
* @name     DS3231_clearDisableInterrupts
*
* @brief Function to clear the interrupt flags and then disable interrupts 
*          in the DS3231 registers.     
* @param    none
*
* @returns  none
*
**********************************************************************************************/
void DS3231_clearDisableInterrupts(void)
{
  if (false == ds3231_initialized)
  {
    DS3231_init(ds3231_i2c_reg);
  }
  
  if (true == ds3231_initialized)
  {
    // Disable Square-wave and alarms
    ds3231_buffer[0] = CNTL_INTCN_MASK;
    // Enable 32kHz signal, clear alarm flags
    ds3231_buffer[1] = STATUS_EN32KHZ_MASK;
    
    if(E_NO_ERROR != DS3231_I2C_WriteRegister(DS3231_DEVICE_ADDR,
                                              CNTL_REG_ADDR,
                                              ds3231_buffer,
                                              2))
    {
       ds3231_initialized = false;
    }
  }
  // Disable interrupt on the MCU
  //TODO:
  //BOARD_setRtcAlarm(DISABLE);
}


/*********************************************************************************************
* @name     DS3231_i2c_transfer
*
* @brief This function is used to read data from an I2C device, starting at the specified
*          register address. This is done by first transmitting the register address before 
*          switching to the reciever mode.
*
* @param    req     mxc_i2c_req_t*   I2C request
* @param    txData  uint8_t*  Data buffer for write
* @param    txSize  uint8_t   number of bytes to write
* @param    rxData  uint8_t*  Data buffer for read
* @param    rxSize  uint8_t   number of bytes to read
*
* @returns  uint32_t E_NO_ERROR   0, E_SUCCESS   0, E_NULL_PTR   -1,
*            E_NO_DEVICE   -2, E_BAD_PARAM   -3, E_INVALID   -4,
*            E_UNINITIALIZED   -5, E_BUSY   -6, E_BAD_STATE   -7,
*            E_UNKNOWN   -8, E_COMM_ERR   -9, E_TIME_OUT   -10,
*            E_NO_RESPONSE   -11, E_OVERFLOW   -12, E_UNDERFLOW   -13,
*            E_NONE_AVAIL   -14, E_SHUTDOWN   -15, E_ABORT   -16,
*            E_NOT_SUPPORTED   -17, E_FAIL   -255
*
***********************************************************************************************/
uint32_t DS3231_i2c_transfer(mxc_i2c_req_t *req, uint8_t *txData, int txSize, uint8_t *rxData,
                        int rxSize)
{
    req->tx_buf = txData; // Write data buffer
    req->tx_len = txSize; // Number of bytes to write
    req->rx_buf = rxData; // Read data buffer
    req->rx_len = rxSize; // Number of bytes to read

    uint32_t retCode = MXC_I2C_MasterTransaction(req);

    return retCode;
}

/*********************************************************************************************
* @name     bool DS3231_I2C_ReadRegister
*
* @brief This function is used to read data from an I2C device, starting at the specified
*          register address. This is done by first transmitting the register address before 
*          switching to the reciever mode.
*
* @param    devAddr uint8_t   address of the slave to communicate with
* @param    regAddr uint8_t   address of the first register to read within the device
* @param    buffer  uint8_t*  buffer to store the read register data in (must be atleast count bytes)
* @param    count   uint8_t   number of bytes to read
*
* @returns  uint32_t E_NO_ERROR   0, E_SUCCESS   0, E_NULL_PTR   -1,
*            E_NO_DEVICE   -2, E_BAD_PARAM   -3, E_INVALID   -4,
*            E_UNINITIALIZED   -5, E_BUSY   -6, E_BAD_STATE   -7,
*            E_UNKNOWN   -8, E_COMM_ERR   -9, E_TIME_OUT   -10,
*            E_NO_RESPONSE   -11, E_OVERFLOW   -12, E_UNDERFLOW   -13,
*            E_NONE_AVAIL   -14, E_SHUTDOWN   -15, E_ABORT   -16,
*            E_NOT_SUPPORTED   -17, E_FAIL   -255
*
**********************************************************************************************/
uint32_t DS3231_I2C_ReadRegister(uint8_t devAddr, uint8_t regAddr, uint8_t *buffer, uint8_t count)
{ 
  //ds3231_txRegisterAddr = regAddr;
  uint8_t mybuff[2]; 
  ds3231_txBuffer = mybuff;
  ds3231_rxCount = count;
  ds3231_txCount = 1;
  ds3231_txBuffer[0] = regAddr;
  ds3231_rxLocation = buffer;

  uint32_t retCode = DS3231_i2c_transfer(&ds3231_req, ds3231_txBuffer, ds3231_txCount, ds3231_rxLocation, count); // Create I2C read request;
 
  return retCode;
}

/*********************************************************************************************
*
* @name     DS3231_I2C_WriteRegister
*
* @brief This function is used to write data to an I2C device, starting at the specified
*          register address.
*
* @param    devAddr uint8_t   address of the slave to communicate with
* @param    regAddr uint8_t   address of the first register to write within the device
* @param    buffer  uint8_t*  buffer with data to be written
* @param    count   uint8_t   number of bytes to write
*
* @returns  uint32_t E_NO_ERROR   0, E_SUCCESS   0, E_NULL_PTR   -1,
*            E_NO_DEVICE   -2, E_BAD_PARAM   -3, E_INVALID   -4,
*            E_UNINITIALIZED   -5, E_BUSY   -6, E_BAD_STATE   -7,
*            E_UNKNOWN   -8, E_COMM_ERR   -9, E_TIME_OUT   -10,
*            E_NO_RESPONSE   -11, E_OVERFLOW   -12, E_UNDERFLOW   -13,
*            E_NONE_AVAIL   -14, E_SHUTDOWN   -15, E_ABORT   -16,
*            E_NOT_SUPPORTED   -17, E_FAIL   -255
*
**********************************************************************************************/
uint32_t DS3231_I2C_WriteRegister(uint8_t devAddr, uint8_t regAddr, uint8_t *buffer, uint8_t count)
{
  
  //ds3231_txRegisterAddr = regAddr;
  ds3231_rxCount = 0;
  ds3231_txCount = count + 1;   //Add one to the count since we are appending the reg address to index 0

  uint8_t tempBuffer[10];
  tempBuffer[0] = regAddr;

  ds3231_txBuffer[0] = regAddr;

  //Parse the txBuffer array from regAddr at [0] following by the buffer  
  for (size_t i = 0; i < count; i++)
  {
    tempBuffer[i+1] = buffer[i];
  }


  uint32_t retCode = DS3231_i2c_transfer(&ds3231_req, tempBuffer, ds3231_txCount, NULL, 0);


  return retCode; 
}

/*********************************************************************************************
*
* @name     DS3231_Open
*
* @brief A public function to open the DS3231 driver. Any fuction using this driver needs
*        to first call this function.
*
* @param    none
*
* @returns  ds3231_driver_t 
*
**********************************************************************************************/
ds3231_driver_t DS3231_Open(void)
{
    ds3231_driver_t SD;
    SD.init = DS3231_init;
    SD.read_datetime = ds3231_read_datetime;
    SD.read_temperature = ds3231_read_temperature;
    SD.set_datetime = ds3231_set_datetime;
    return SD;
}

/*==================== Implementation of Functions Linked To Driver Function Pointers ===============*/

uint32_t ds3231_mx_init(mxc_i2c_regs_t *i2c)
{
    ds3231_i2c_reg = i2c;  //store this to the driver variable so we can keep calling it internally
    return DS3231_init(ds3231_i2c_reg);
}


uint32_t ds3231_read_datetime(void *buff, void *strbuff)
{
    return DS3231_getDateTime((struct tm*)buff, (char*)strbuff);
}

uint32_t ds3231_read_temperature(void *temp)
{
    return DS3231_getTemperature((float*)temp);
}

uint32_t ds3231_set_datetime(struct tm * newTime)
{
    return DS3231_setDateTime(newTime);
}

#ifdef USE_DEBUG_MENU
/**
* @name     DS3231_readAllRegisters
*
* @brief Function to read  all the registers of the DS3231.
*
* @param    none
*
* @returns  none
*/
uint32_t DS3231_readAllRegisters(uint8_t * buffer, uint16_t size)
{
  uint32_t result = 0;
  
  if (false == ds3231_initialized)
  {

    DS3231_init();
  }
  
  if (true == ds3231_initialized)
  {
    // read all registers
    ds3231_initialized = DS3231_I2C_ReadRegister(DS3231_DEVICE_ADDR, SEC_REG_ADDR, ds3231_buffer, 19);
    
    
    result = sprintf((char*)buffer, "  Reg:   0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F 0x10 0x11 0x12\r\n" \
             "  Value: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\r\n",
                           ds3231_buffer[0], ds3231_buffer[1], ds3231_buffer[2], ds3231_buffer[3], ds3231_buffer[4], ds3231_buffer[5], ds3231_buffer[6], 
                           ds3231_buffer[7], ds3231_buffer[8], ds3231_buffer[9], ds3231_buffer[10], ds3231_buffer[11], ds3231_buffer[12], ds3231_buffer[13], 
                           ds3231_buffer[14], ds3231_buffer[15], ds3231_buffer[16], ds3231_buffer[17], ds3231_buffer[18]);
  }
  return result;
}

/**
*
* @name     DS3231_setTestAlarm
*
* @brief Function to setup the DS3231 to trigger an interrupt once / minute.
*
* @param    none
*
* @returns  none
*/
void DS3231_setTestAlarm(void)
{
  if (false == ds3231_initialized)
  {
    DS3231_init(); 
  }
  
  if (true == ds3231_initialized)
  {
    ds3231_buffer[0] = 0x10U; // Set AIM1 - Trigger on 10th second of every minute
    ds3231_buffer[1] = 0x80U; // Set A1M2
    ds3231_buffer[2] = 0x80U; // Set A1M3
    ds3231_buffer[3] = 0x80U; // Set A1M4
    
    ds3231_initialized = DS3231_I2C_WriteRegister(DS3231_DEVICE_ADDR,
                                              ALRM1_SEC_REG_ADDR,
                                              ds3231_buffer,
                                              4);
                                            
    // Enable interrupt function and alarm 1 interrupt
    ds3231_buffer[0] = (CNTL_INTCN_MASK | CNTL_A1IE_MASK);
    ds3231_initialized = DS3231_I2C_WriteRegister(DS3231_DEVICE_ADDR, CNTL_REG_ADDR, ds3231_buffer, 1);
  }
}
#endif
