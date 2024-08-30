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





//I2C defines

int I2C_device_scan();//  Looks for all the devices on the I2C line
typedef struct bme68x_dev{

}bme68x_dev;

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
int8_t setMode(const uint8_t op_mode);
int8_t bme68x_set_op_mode(const uint8_t op_mode, struct bme68x_dev *Empty);
int8_t bme68x_set_heatr_conf(uint8_t op_mode, const struct bme68x_heatr_conf *conf, int amb_temp, struct bme68x_dev *dev, int Variant_id, struct bme68x_calib_data* calib);
int8_t bme68x_get_regs(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, struct bme68x_dev *dev);
int8_t BME688_Write(uint8_t reg_addr, uint8_t data, uint8_t len); //Writes to BME688
int8_t bme68x_set_regs(const uint8_t *reg_addr, const uint8_t *reg_data, uint32_t len, struct bme68x_dev *Empty);
 int8_t boundary_check(uint8_t *value, uint8_t max, struct bme68x_dev *Empty);
int8_t bme68x_get_conf(struct bme68x_conf *conf, struct bme68x_dev *Empty);
int8_t BME688_Read(uint8_t reg_addr, uint8_t *data, uint8_t rx_len); //Reads from BME688
uint8_t calc_gas_wait(uint16_t dur);
int BME688_soft_reset(); //Writing OxE0 to 0XB6 register soft resets the devices which is same as power on reset
uint8_t calc_res_heat(uint16_t temp, const struct bme68x_calib_data* calib, float amb_temp);
int8_t bme68x_init();
static int read_variant_id(); //This function reads variant ID of the device
int8_t bme68x_set_conf(struct bme68x_conf *conf, struct bme68x_dev *dev);
int8_t bme68x_get_data(uint8_t op_mode, struct bme68x_data *data, uint8_t *n_data, struct bme68x_dev *dev, int variant_id,struct bme68x_calib_data* calib);
int8_t set_conf(struct bme68x_heatr_conf *conf, uint8_t op_mode, uint8_t *nb_conv,const struct bme68x_calib_data* calib,int amb_temp);
int8_t bme68x_set_conf(struct bme68x_conf *conf, struct bme68x_dev *dev);
#endif /* BME688_H_ */
