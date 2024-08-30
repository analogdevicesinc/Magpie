/******************************************************************************
 *
 * Copyright (C) 2022-2023 Maxim Integrated Products, Inc. All Rights Reserved.
 * (now owned by Analog Devices, Inc.),
 * Copyright (C) 2023 Analog Devices, Inc. All Rights Reserved. This software
 * is proprietary to Analog Devices, Inc. and its licensors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************/

/**
 * @file        main.c
 * @brief     Example code for scanning the available addresses on an I2C bus
 * @details     This example uses the I2C Master to found addresses of the I2C Slave devices 
 *              connected to the bus. You must connect the pull-up jumpers (JP21 and JP22) 
 *              to the proper I/O voltage.
 */

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "mxc_device.h"
#include "mxc_delay.h"
#include "nvic_table.h"
#include "i2c.h"
#include "board.h"
#include "BME688.h"
// #include "C:/Users/ATarmu/Desktop/Swift/Magpie-BME688/BME68x_SensorAPI/bme68x.h"
// #include "C:/Users/ATarmu/Desktop/Swift/Magpie-BME688/BME68x_SensorAPI/examples/sequential_mode/sequential_mode.c"
// #define MASTERDMA //Comment this line out if standard I2C transaction is required

#define I2C_MASTER MXC_I2C0_BUS0
#if defined(BOARD_FTHR2)
#define I2C_SLAVE MXC_I2C2_BUS0
#define I2C_SLAVE_IRQn I2C2_IRQn
#else
#define I2C_SLAVE MXC_I2C1_BUS0
#define I2C_SLAVE_IRQn I2C1_IRQn
#endif

#define I2C_FREQ 100000
#define I2C_SLAVE_ADDR 0x51
#define I2C_BYTES 255

#define BME68X_CONCAT_BYTES(msb, lsb)             (((uint16_t)msb << 8) | (uint16_t)lsb)

#define I2C_FREQ 100000 // 100kHZ
typedef enum { FAILED, PASSED } test_t;
/***** Globals *****/

static uint8_t Stxdata[I2C_BYTES];
static uint8_t Srxdata[I2C_BYTES];
int8_t DMA0_TX_CH;
int8_t DMA0_RX_CH;
volatile int I2C_FLAG;
volatile int txnum = 0;
volatile int txcnt = 0;
volatile int rxnum = 0;
#define TEMP_XLSB_REG 0x22 //temp is temperature.
#define TEMP_LSB_REG  0x23
#define TEMP_MSB_REG  0x24

#define HUMIDITY_XLSB_REG 0x25
#define HUMIDITY_LSB_REG  0x26
#define HUMIDITY_MSB_REG  0x27

#define PRESSURE_XLSB_REG 0x1F
#define PRESSURE_LSB_REG  0x20
#define PRESSURE_MSB_REG  0x21

//Slave interrupt handler
void I2C_Slave_IRQHandler(void)
{
    MXC_I2C_AsyncHandler(I2C_SLAVE);
}
void I2C_Callback(mxc_i2c_req_t *req, int error)
{
    I2C_FLAG = error;
}


// void i2c_write(uint8_t addr, uint8_t reg, uint8_t data) {
//     uint8_t buf[2] = { reg, data };
//     MXC_I2C_WriteByte(MXC_I2C0_BUS0, addr, buf);
// }

// // Read data from an I2C register
// uint8_t i2c_read(uint8_t addr, uint8_t reg) {
//     uint8_t data;
//     MXC_I2C_WriteByte(MXC_I2C0_BUS0, addr, &reg);  // Write register address
//     MXC_I2C_ReadByte(MXC_I2C0_BUS0, addr, &data);  // Read data
//     return data;
// }
int slaveHandler(mxc_i2c_regs_t *i2c, mxc_i2c_slave_event_t event, void *data)
{
    switch (event) {
    case MXC_I2C_EVT_MASTER_WR:
        // If we're being written to
        // Clear bytes written
        rxnum = 0;
        break;

    case MXC_I2C_EVT_MASTER_RD:
        // Serve as a 16 byte loopback, returning data*2
        txnum = I2C_BYTES;
        txcnt = 0;
        i2c->int_fl0 = MXC_F_I2C_INT_FL0_TX_LOCK_OUT | MXC_F_I2C_INT_FL0_ADDR_MATCH;
        break;

    case MXC_I2C_EVT_RX_THRESH:
    case MXC_I2C_EVT_OVERFLOW:
        rxnum += MXC_I2C_ReadRXFIFO(i2c, &Srxdata[rxnum], MXC_I2C_GetRXFIFOAvailable(i2c));
        if (rxnum == I2C_BYTES) {
            i2c->int_en0 |= MXC_F_I2C_INT_EN0_ADDR_MATCH;
        }

        break;

    case MXC_I2C_EVT_TX_THRESH:
    case MXC_I2C_EVT_UNDERFLOW:
        // Write as much data as possible into TX FIFO
        // Unless we're at the end of the transaction (only write what's needed)
        if (txcnt >= txnum) {
            break;
        }

        int num = MXC_I2C_GetTXFIFOAvailable(i2c);
        num = (num > (txnum - txcnt)) ? (txnum - txcnt) : num;
        txcnt += MXC_I2C_WriteTXFIFO(i2c, &Stxdata[txcnt], num);
        break;

    default:
        if (*((int *)data) == E_COMM_ERR) {
            printf("I2C Slave Error!\n");
            printf("i2c->int_fl0 = 0x%08x\n", i2c->int_fl0);
            printf("i2c->status  = 0x%08x\n", i2c->status);
            I2C_Callback(NULL, E_COMM_ERR);
            return 1;

        } else if (*((int *)data) == E_NO_ERROR) {
            rxnum += MXC_I2C_ReadRXFIFO(i2c, &Srxdata[rxnum], MXC_I2C_GetRXFIFOAvailable(i2c));
            I2C_Callback(NULL, E_NO_ERROR);
            return 1;
        }
    }

    return 0;
}



//was having issues w/ it not working doing the following fixed it will investigate why later.
int reset_test(){
	uint8_t temp=0xfe, readTemp=0;
	int t= BME688_Write(0xE0, temp,1);
	if(t!=0){
		printf("err on write %d",t);
		return -99;
	}
	t= BME688_Read(0xe0, &readTemp,1);
	if(t!=0){
		printf("err on read %d",t);
		return -99;
	}
		printf("Data Read: 0x%x  \r\n",readTemp);

	return 0;
}


// // *****************************************************************************
// float bme688_read_humidity(void) {
//     uint8_t humidity_msb  = i2c_read_register(HUMIDITY_MSB_REG);
//     uint8_t humidity_lsb  = i2c_read_register(HUMIDITY_LSB_REG);
//     uint8_t humidity_xlsb = i2c_read_register(HUMIDITY_XLSB_REG);

//     // Combine the bytes to get the 16-bit humidity value
//     uint16_t humidity_raw = (humidity_msb << 8) | humidity_lsb;

//     // Convert raw humidity value to relative humidity percentage
//     // The exact formula may depend on calibration data
//     // For simplicity, assuming a generic conversion factor
//     float humidity = humidity_raw / 1024.0; // Adjust based on calibration

//     return humidity;
// }

// // Function to read pressure from BME688
// float bme688_read_pressure(void) {
//     uint8_t pressure_msb = i2c_read_register(PRESSURE_MSB_REG);
//     uint8_t pressure_lsb = i2c_read_register(PRESSURE_LSB_REG);
//     uint8_t pressure_xlsb = i2c_read_register(PRESSURE_XLSB_REG);

//     // Combine the bytes to get the 20-bit pressure value
//     uint32_t pressure_raw = ((uint32_t)pressure_msb << 12) |
//                             ((uint32_t)pressure_lsb << 4) |
//                             (pressure_xlsb >> 4);

//     // Convert raw pressure value to pressure in hPa
//     // The exact formula may depend on calibration data
//     // For simplicity, assuming a generic conversion factor
//     float pressure = pressure_raw / 100.0; // Adjust based on calibration

//     return pressure;
// }

/*
 * @brief This API is used to get the remaining duration that can be used for heating.
 */
uint32_t bme68x_get_meas_dur(const uint8_t op_mode, struct bme68x_conf *conf)
{
    int8_t rslt;
    uint32_t meas_dur = 0; /* Calculate in us */
    uint32_t meas_cycles;
    uint8_t os_to_meas_cycles[6] = { 0, 1, 2, 4, 8, 16 };

    if (conf != NULL)
    {
        /* Boundary check for temperature oversampling */
        rslt = boundary_check(&conf->os_temp, BME68X_OS_16X, 0);

        if (rslt == BME68X_OK)
        {
            /* Boundary check for pressure oversampling */
            rslt = boundary_check(&conf->os_pres, BME68X_OS_16X, 0);
        }

        if (rslt == BME68X_OK)
        {
            /* Boundary check for humidity oversampling */
            rslt = boundary_check(&conf->os_hum, BME68X_OS_16X, 0);
        }

        if (rslt == BME68X_OK)
        {
            meas_cycles = os_to_meas_cycles[conf->os_temp];
            meas_cycles += os_to_meas_cycles[conf->os_pres];
            meas_cycles += os_to_meas_cycles[conf->os_hum];

            /* TPH measurement duration */
            meas_dur = meas_cycles * UINT32_C(1963);
            meas_dur += UINT32_C(477 * 4); /* TPH switching duration */
            meas_dur += UINT32_C(477 * 5); /* Gas measurement duration */

            if (op_mode != BME68X_PARALLEL_MODE)
            {
                meas_dur += UINT32_C(1000); /* Wake up duration of 1ms */
            }
        }
    }

    return meas_dur;
}

int getVariantId(uint8_t* Variant_ID){
    int r = BME688_Read(BME68X_REG_VARIANT_ID, Variant_ID,1);
    if(r<0){
        printf("error on getting variant id");
        return r;
    }
    else{
        printf("Variant Id recieved");
        return r;
    }
    printf("Variant ID: 0x%x  \r\n",Variant_ID);

}
/**
 * @brief Initializes the I2C interface, sets its frequency, performs a soft reset on the BME688 sensor,
 *        and retrieves the variant ID of the sensor.
 *
 * This function initializes the I2C bus, sets the frequency for the I2C communication, performs
 * a soft reset on the BME688 sensor, and then retrieves the sensor's variant ID. The function prints
 * status messages to the standard output indicating the success or failure of each operation.
 *
 * @param Variant_ID  where the sensor's variant ID will be stored.
 * @return int Returns 0 if all operations were successful, or an error code if any operation failed.
 */

int init(uint8_t* Variant_ID){
	int error = MXC_I2C_Init(MXC_I2C0_BUS0, 1, 0);
    fflush(stdout);

	if (error != E_NO_ERROR)
	{
		printf("-->Failed on I2C init\n");
		// return error;
	}
	else
	{
		printf("\n-->I2C Master Initialization Complete\n");
	}
    fflush(stdout);

	error = MXC_I2C_SetFrequency(MXC_I2C0_BUS0, I2C_FREQ); //returns a freq value not a code. 
	if (error <0)
	{
		printf("-->Failed on setting freq error code: %x\n",error);
        
		// return error;
	}
	else
	{
		printf("\n-->freq Initialization Complete\n");
	}
    fflush(stdout);

	error = BME688_soft_reset();
    if(error<0){
        printf("Error on reset");
        fflush(stdout);
    }
    error = getVariantId(Variant_ID);
    fflush(stdout);

    return error;
}
int main(){
	printf("\n******** I2C tst BME688 *********\n");
	uint8_t Chip_ID, rslt, Variant_ID=0x99u;
    struct bme68x_dev bme;
    uint8_t data=0xaa; 
    uint8_t dataBack=0x00; 

    rslt = init(&Variant_ID);
    if(rslt<0){
        printf("\nError on setting up the initlization\n");
        fflush(stdout);
    }else{
        printf("\nInitilization completed succesfully\n");
        fflush(stdout);
    }
    printf("START TEST");
    fflush(stdout);
    // rslt = bme68x_set_op_mode(BME68X_SEQUENTIAL_MODE, &bme);
    BME688_Write(0x74,0x10,1);
    BME688_Read(0x74,&dataBack,1);
    printf("\nRX DATA: %x\n", dataBack);
    fflush(stdout);

    // BME688_Write(0x71,data,1);
    // printf("\nDATA: %x\n", data);
    // fflush(stdout);
    // BME688_Write(0x72,0xbb,1);
    // printf("\nDATA: %x\n", 0xbb);
    // fflush(stdout);
    for(int i= 0x5e; i<0x73; i++){
        BME688_Write(i,i,1);
        printf("\nDATA: %x\n", i);
        fflush(stdout);
        BME688_Read(0x74,&dataBack,1);
        printf("\nRX DATA: %x\n", dataBack);
        fflush(stdout);

    }
    BME688_Write(0x74,0x10,1);
    BME688_Read(0x74,&dataBack,1);
    printf("\nRX DATA: %x\n", dataBack);
    fflush(stdout);
    return 0;


}
int tst()
{
	printf("\n******** I2C example to read BME688 *********\n");
	printf("\nsame bus as I2C0 (SCL - P0.6, SDA - P0.7).");
    fflush(stdout);
    int8_t rslt;
    struct bme68x_conf conf; //the config settings were going to use
    struct bme68x_heatr_conf heatr_conf; //the config settings specific to the heat sensor
    struct bme68x_data data[3];
    struct bme68x_calib_data calib;
    struct bme68x_dev bme;
    int amb_temp=22;
    uint8_t nb_conv=0;
    uint32_t del_period;
    uint32_t time_ms = 0;
    uint8_t n_fields;
    uint16_t sample_count = 1;

    /* Heater temperature in degree Celsius */
    uint16_t temp_prof[10] = { 200, 240, 280, 320, 360, 360, 320, 280, 240, 200 };

    /* Heating duration in milliseconds */
    uint16_t dur_prof[10] = { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 };

	int error;
	uint8_t Chip_ID, Variant_ID=0x99u;
	uint8_t temp=0xfeu, addr=0x6eu,readTemp=0xa0u;




	//Setup the I2CM
	rslt = init(&Variant_ID);
    if(rslt<0){
        printf("Error on setting up the initlization");
        fflush(stdout);
    }else{
        printf("Initilization completed succesfully");
        fflush(stdout);
    }

    rslt = bme68x_get_conf(&conf, &bme);

    conf.filter = BME68X_FILTER_OFF;
    conf.odr = BME68X_ODR_NONE; /* This parameter defines the sleep duration after each profile */
    conf.os_hum = BME68X_OS_16X;
    conf.os_pres = BME68X_OS_1X;
    conf.os_temp = BME68X_OS_2X;
    rslt = bme68x_set_conf(&conf, &bme);





    heatr_conf.enable = BME68X_ENABLE;
    heatr_conf.heatr_temp_prof = temp_prof;
    heatr_conf.heatr_dur_prof = dur_prof;
    // heatr_conf.profile_len = 1;
    heatr_conf.profile_len = 10;

    rslt = bme68x_set_heatr_conf(BME68X_SEQUENTIAL_MODE, &heatr_conf, amb_temp, &bme,Variant_ID,&calib);
    // return;
    rslt = bme68x_set_op_mode(BME68X_SEQUENTIAL_MODE, &bme);
    printf(
        "Temperature(deg C), Pressure(Pa), Humidity(%%), Gas resistance(ohm), Status, Profile index, Measurement index\n");


	while(1)
	{
        del_period = bme68x_get_meas_dur(BME68X_SEQUENTIAL_MODE, &conf) + (heatr_conf.heatr_dur_prof[0] * 1000);
        printf("\nDEL PERIOD: %d\n",del_period);
		MXC_Delay(MXC_DELAY_MSEC(del_period));
        rslt = bme68x_get_data(BME68X_SEQUENTIAL_MODE, data, &n_fields, &bme,(int)Variant_ID,&calib);
        

		for (uint8_t i = 0; i < n_fields; i++){
            printf("%.2f, %.2f, %.2f, %.2f, 0x%x, %d, %d\n",
                //    sample_count,
                //    (long unsigned int)time_ms + (i * (del_period / 2000)),
                   data[i].temperature,
                   data[i].pressure,
                   data[i].humidity,
                   data[i].gas_resistance,
                   data[i].status,
                   data[i].gas_index,
                   data[i].meas_index);
            		fflush(stdout);
        }
	}

  
	return E_NO_ERROR;
}
