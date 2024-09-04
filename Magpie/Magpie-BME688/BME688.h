/*
 * BME688.h
 *
 *  Created on: Jun 17, 2024
 *      Author: VKarra
 */

#ifndef BME688_H_
#define BME688_H_


#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "mxc_device.h"
#include "mxc_delay.h"
#include "nvic_table.h"
#include "BME688_defs.h"
#include "i2c.h"






//I2C defines

int I2C_device_scan();//  Looks for all the devices on the I2C line
/*
 * @brief Sensor field data structure
 */
struct bme68x_data
{
    /*! Contains new_data, gasm_valid & heat_stab */
    uint8_t status;

    /*! The index of the heater profile used */
    uint8_t gas_index;

    /*! Measurement index to track order */
    uint8_t meas_index;

    /*! Heater resistance */
    uint8_t res_heat;

    /*! Current DAC */
    uint8_t idac;

    /*! Gas wait period */
    uint8_t gas_wait;
#ifndef BME68X_USE_FPU

    /*! Temperature in degree celsius x100 */
    int16_t temperature;

    /*! Pressure in Pascal */
    uint32_t pressure;

    /*! Humidity in % relative humidity x1000 */
    uint32_t humidity;

    /*! Gas resistance in Ohms */
    uint32_t gas_resistance;
#else

    /*! Temperature in degree celsius */
    float temperature;

    /*! Pressure in Pascal */
    float pressure;

    /*! Humidity in % relative humidity x1000 */
    float humidity;

    /*! Gas resistance in Ohms */
    float gas_resistance;

#endif

};
typedef struct bme68x_calib_data
{
    /*! Calibration coefficient for the humidity sensor */
    uint16_t par_h1;

    /*! Calibration coefficient for the humidity sensor */
    uint16_t par_h2;

    /*! Calibration coefficient for the humidity sensor */
    int8_t par_h3;

    /*! Calibration coefficient for the humidity sensor */
    int8_t par_h4;

    /*! Calibration coefficient for the humidity sensor */
    int8_t par_h5;

    /*! Calibration coefficient for the humidity sensor */
    uint8_t par_h6;

    /*! Calibration coefficient for the humidity sensor */
    int8_t par_h7;

    /*! Calibration coefficient for the gas sensor */
    int8_t par_gh1;

    /*! Calibration coefficient for the gas sensor */
    int16_t par_gh2;

    /*! Calibration coefficient for the gas sensor */
    int8_t par_gh3;

    /*! Calibration coefficient for the temperature sensor */
    uint16_t par_t1;

    /*! Calibration coefficient for the temperature sensor */
    int16_t par_t2;

    /*! Calibration coefficient for the temperature sensor */
    int8_t par_t3;

    /*! Calibration coefficient for the pressure sensor */
    uint16_t par_p1;

    /*! Calibration coefficient for the pressure sensor */
    int16_t par_p2;

    /*! Calibration coefficient for the pressure sensor */
    int8_t par_p3;

    /*! Calibration coefficient for the pressure sensor */
    int16_t par_p4;

    /*! Calibration coefficient for the pressure sensor */
    int16_t par_p5;

    /*! Calibration coefficient for the pressure sensor */
    int8_t par_p6;

    /*! Calibration coefficient for the pressure sensor */
    int8_t par_p7;

    /*! Calibration coefficient for the pressure sensor */
    int16_t par_p8;

    /*! Calibration coefficient for the pressure sensor */
    int16_t par_p9;

    /*! Calibration coefficient for the pressure sensor */
    uint8_t par_p10;
#ifndef BME68X_USE_FPU

    /*! Variable to store the intermediate temperature coefficient */
    int32_t t_fine;
#else

    /*! Variable to store the intermediate temperature coefficient */
    float t_fine;
#endif

    /*! Heater resistance range coefficient */
    uint8_t res_heat_range;

    /*! Heater resistance value coefficient */
    int8_t res_heat_val;

    /*! Gas resistance range switching error coefficient */
    int8_t range_sw_err;
}bme68x_calib_data;


struct bme68x_conf
{
    /*! Humidity oversampling. Refer @ref osx*/
    uint8_t os_hum;

    /*! Temperature oversampling. Refer @ref osx */
    uint8_t os_temp;

    /*! Pressure oversampling. Refer @ref osx */
    uint8_t os_pres;

    /*! Filter coefficient. Refer @ref filter*/
    uint8_t filter;

    /*!
     * Standby time between sequential mode measurement profiles.
     * Refer @ref odr
     */
    uint8_t odr;
};

struct bme68x_heatr_conf
{
    /*! Enable gas measurement. Refer @ref en_dis */
    uint8_t enable;

    /*! Store the heater temperature for forced mode degree Celsius */
    uint16_t heatr_temp;

    /*! Store the heating duration for forced mode in milliseconds */
    uint16_t heatr_dur;

    /*! Store the heater temperature profile in degree Celsius */
    uint16_t *heatr_temp_prof;

    /*! Store the heating duration profile in milliseconds */
    uint16_t *heatr_dur_prof;

    /*! Variable to store the length of the heating profile */
    uint8_t profile_len;

    /*!
     * Variable to store heating duration for parallel mode
     * in milliseconds
     */
    uint16_t shared_heatr_dur;
};
int8_t BME688_Read(uint8_t reg_addr, uint8_t *data, uint8_t rx_len);
#define BME68X_INTF_RET_TYPE                      int8_t
// int8_t BME688_Write(uint8_t reg_addr, uint8_t tx_data, uint8_t tx_len);
int8_t BME688_Write(uint8_t reg_addr, uint8_t* tx_data, uint8_t tx_len);
int8_t BME688_Write_Interleave(uint8_t reg_addr, uint8_t* tx_data, uint8_t tx_len);

struct bme68x_dev
{
    /*! Chip Id */
    uint8_t chip_id;

    /*!
     * The interface pointer is used to enable the user
     * to link their interface descriptors for reference during the
     * implementation of the read and write interfaces to the
     * hardware.
     */
    void *intf_ptr;

    /*!
     *             Variant id
     * ----------------------------------------
     *     Value   |           Variant
     * ----------------------------------------
     *      0      |   BME68X_VARIANT_GAS_LOW
     *      1      |   BME68X_VARIANT_GAS_HIGH
     * ----------------------------------------
     */
    uint32_t variant_id;

    /*! SPI/I2C interface */
    enum bme68x_intf intf;

    /*! Memory page used */
    uint8_t mem_page;

    /*! Ambient temperature in Degree C*/
    int8_t amb_temp;

    /*! Sensor calibration data */
    struct bme68x_calib_data calib;

    /*! Read function pointer */
    int8_t (*read)(uint8_t reg_addr, uint8_t *data, uint8_t rx_len);
    // bme68x_read_fptr_t read;

    /*! Write function pointer */
    int8_t (*write)(uint8_t reg_addr, uint8_t* tx_data, uint8_t tx_len);

    /*! Delay function pointer */
    int (*delay_us)(uint32_t us);

    /*! To store interface pointer error */
    BME68X_INTF_RET_TYPE intf_rslt;

    /*! Store the info messages */
    uint8_t info_msg;
};



// int8_t setMode(const uint8_t op_mode);
// int8_t bme68x_set_op_mode(const uint8_t op_mode, struct bme68x_dev *Empty);
// int8_t bme68x_set_heatr_conf(uint8_t op_mode, const struct bme68x_heatr_conf *conf, int amb_temp, struct bme68x_dev *dev, int Variant_id, struct bme68x_calib_data* calib);
// int8_t bme68x_get_regs(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, struct bme68x_dev *dev);
// int8_t BME688_Write(uint8_t reg_addr, uint8_t data, uint8_t len); //Writes to BME688
// int8_t bme68x_set_regs(const uint8_t *reg_addr, const uint8_t *reg_data, uint32_t len, struct bme68x_dev *Empty);
//  int8_t boundary_check(uint8_t *value, uint8_t max, struct bme68x_dev *Empty);
// int8_t bme68x_get_conf(struct bme68x_conf *conf, struct bme68x_dev *Empty);
// int8_t BME688_Read(uint8_t reg_addr, uint8_t *data, uint8_t rx_len); //Reads from BME688
// uint8_t calc_gas_wait(uint16_t dur);
// int BME688_soft_reset(); //Writing OxE0 to 0XB6 register soft resets the devices which is same as power on reset
// uint8_t calc_res_heat(uint16_t temp, const struct bme68x_calib_data* calib, float amb_temp);
// int8_t bme68x_init();
// static int read_variant_id(); //This function reads variant ID of the device
// int8_t bme68x_set_conf(struct bme68x_conf *conf, struct bme68x_dev *dev);
// int8_t bme68x_get_data(uint8_t op_mode, struct bme68x_data *data, uint8_t *n_data, struct bme68x_dev *dev, int variant_id,struct bme68x_calib_data* calib);
// int8_t set_conf(struct bme68x_heatr_conf *conf, uint8_t op_mode, uint8_t *nb_conv,const struct bme68x_calib_data* calib,int amb_temp);
// int8_t bme68x_set_conf(struct bme68x_conf *conf, struct bme68x_dev *dev);
int8_t bme68x_init(struct bme68x_dev *dev);

/**
 * \ingroup bme68x
 * \defgroup bme68xApiRegister Registers
 * @brief Generic API for accessing sensor registers
 */

/*!
 * \ingroup bme68xApiRegister
 * \page bme68x_api_bme68x_set_regs bme68x_set_regs
 * \code
 * int8_t bme68x_set_regs(const uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, struct bme68x_dev *dev)
 * \endcode
 * @details This API writes the given data to the register address of the sensor
 *
 * @param[in] reg_addr : Register addresses to where the data is to be written
 * @param[in] reg_data : Pointer to data buffer which is to be written
 *                       in the reg_addr of sensor.
 * @param[in] len      : No of bytes of data to write
 * @param[in,out] dev  : Structure instance of bme68x_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bme68x_set_regs(const uint8_t *reg_addr, const uint8_t *reg_data, uint32_t len, struct bme68x_dev *dev);

/*!
 * \ingroup bme68xApiRegister
 * \page bme68x_api_bme68x_get_regs bme68x_get_regs
 * \code
 * int8_t bme68x_get_regs(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, struct bme68x_dev *dev)
 * \endcode
 * @details This API reads the data from the given register address of sensor.
 *
 * @param[in] reg_addr  : Register address from where the data to be read
 * @param[out] reg_data : Pointer to data buffer to store the read data.
 * @param[in] len       : No of bytes of data to be read.
 * @param[in,out] dev   : Structure instance of bme68x_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bme68x_get_regs(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, struct bme68x_dev *dev);

/**
 * \ingroup bme68x
 * \defgroup bme68xApiSystem System
 * @brief API that performs system-level operations
 */

/*!
 * \ingroup bme68xApiSystem
 * \page bme68x_api_bme68x_soft_reset bme68x_soft_reset
 * \code
 * int8_t bme68x_soft_reset(struct bme68x_dev *dev);
 * \endcode
 * @details This API soft-resets the sensor.
 *
 * @param[in,out] dev : Structure instance of bme68x_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bme68x_soft_reset(struct bme68x_dev *dev);

/**
 * \ingroup bme68x
 * \defgroup bme68xApiOm Operation mode
 * @brief API to configure operation mode
 */

/*!
 * \ingroup bme68xApiOm
 * \page bme68x_api_bme68x_set_op_mode bme68x_set_op_mode
 * \code
 * int8_t bme68x_set_op_mode(const uint8_t op_mode, struct bme68x_dev *dev);
 * \endcode
 * @details This API is used to set the operation mode of the sensor
 * @param[in] op_mode : Desired operation mode.
 * @param[in] dev     : Structure instance of bme68x_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bme68x_set_op_mode(const uint8_t op_mode, struct bme68x_dev *dev);

/*!
 * \ingroup bme68xApiOm
 * \page bme68x_api_bme68x_get_op_mode bme68x_get_op_mode
 * \code
 * int8_t bme68x_get_op_mode(uint8_t *op_mode, struct bme68x_dev *dev);
 * \endcode
 * @details This API is used to get the operation mode of the sensor.
 *
 * @param[out] op_mode : Desired operation mode.
 * @param[in,out] dev : Structure instance of bme68x_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bme68x_get_op_mode(uint8_t *op_mode, struct bme68x_dev *dev);

/*!
 * \ingroup bme68xApiConfig
 * \page bme68x_api_bme68x_get_meas_dur bme68x_get_meas_dur
 * \code
 * uint32_t bme68x_get_meas_dur(const uint8_t op_mode, struct bme68x_conf *conf, struct bme68x_dev *dev);
 * \endcode
 * @details This API is used to get the remaining duration that can be used for heating.
 *
 * @param[in] op_mode : Desired operation mode.
 * @param[in] conf    : Desired sensor configuration.
 * @param[in] dev     : Structure instance of bme68x_dev
 *
 * @return Measurement duration calculated in microseconds
 */
uint32_t bme68x_get_meas_dur(const uint8_t op_mode, struct bme68x_conf *conf, struct bme68x_dev *dev);

/**
 * \ingroup bme68x
 * \defgroup bme68xApiData Data Read out
 * @brief Read our data from the sensor
 */

/*!
 * \ingroup bme68xApiData
 * \page bme68x_api_bme68x_get_data bme68x_get_data
 * \code
 * int8_t bme68x_get_data(uint8_t op_mode, struct bme68x_data *data, uint8_t *n_data, struct bme68x_dev *dev);
 * \endcode
 * @details This API reads the pressure, temperature and humidity and gas data
 * from the sensor, compensates the data and store it in the bme68x_data
 * structure instance passed by the user.
 *
 * @param[in]  op_mode : Expected operation mode.
 * @param[out] data    : Structure instance to hold the data.
 * @param[out] n_data  : Number of data instances available.
 * @param[in,out] dev  : Structure instance of bme68x_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bme68x_get_data(uint8_t op_mode, struct bme68x_data *data, uint8_t *n_data, struct bme68x_dev *dev);

/**
 * \ingroup bme68x
 * \defgroup bme68xApiConfig Configuration
 * @brief Configuration API of sensor
 */

/*!
 * \ingroup bme68xApiConfig
 * \page bme68x_api_bme68x_set_conf bme68x_set_conf
 * \code
 * int8_t bme68x_set_conf(struct bme68x_conf *conf, struct bme68x_dev *dev);
 * \endcode
 * @details This API is used to set the oversampling, filter and odr configuration
 *
 * @param[in] conf    : Desired sensor configuration.
 * @param[in,out] dev : Structure instance of bme68x_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bme68x_set_conf(struct bme68x_conf *conf, struct bme68x_dev *dev);

/*!
 * \ingroup bme68xApiConfig
 * \page bme68x_api_bme68x_get_conf bme68x_get_conf
 * \code
 * int8_t bme68x_get_conf(struct bme68x_conf *conf, struct bme68x_dev *dev);
 * \endcode
 * @details This API is used to get the oversampling, filter and odr
 * configuration
 *
 * @param[out] conf   : Present sensor configuration.
 * @param[in,out] dev : Structure instance of bme68x_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bme68x_get_conf(struct bme68x_conf *conf, struct bme68x_dev *dev);

/*!
 * \ingroup bme68xApiConfig
 * \page bme68x_api_bme68x_set_heatr_conf bme68x_set_heatr_conf
 * \code
 * int8_t bme68x_set_heatr_conf(uint8_t op_mode, const struct bme68x_heatr_conf *conf, struct bme68x_dev *dev);
 * \endcode
 * @details This API is used to set the gas configuration of the sensor.
 *
 * @param[in] op_mode : Expected operation mode of the sensor.
 * @param[in] conf    : Desired heating configuration.
 * @param[in,out] dev : Structure instance of bme68x_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bme68x_set_heatr_conf(uint8_t op_mode, const struct bme68x_heatr_conf *conf, struct bme68x_dev *dev);

/*!
 * \ingroup bme68xApiConfig
 * \page bme68x_api_bme68x_get_heatr_conf bme68x_get_heatr_conf
 * \code
 * int8_t bme68x_get_heatr_conf(const struct bme68x_heatr_conf *conf, struct bme68x_dev *dev);
 * \endcode
 * @details This API is used to get the gas configuration of the sensor.
 *
 * @param[out] conf   : Current configurations of the gas sensor.
 * @param[in,out] dev : Structure instance of bme68x_dev.
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bme68x_get_heatr_conf(const struct bme68x_heatr_conf *conf, struct bme68x_dev *dev);

/*!
 * \ingroup bme68xApiSystem
 * \page bme68x_api_bme68x_selftest_check bme68x_selftest_check
 * \code
 * int8_t bme68x_selftest_check(const struct bme68x_dev *dev);
 * \endcode
 * @details This API performs Self-test of low gas variant of BME68X
 *
 * @param[in, out]   dev  : Structure instance of bme68x_dev
 *
 * @return Result of API execution status
 * @retval 0 -> Success
 * @retval < 0 -> Fail
 */
int8_t bme68x_selftest_check(const struct bme68x_dev *dev);

#endif /* BME688_H_ */
