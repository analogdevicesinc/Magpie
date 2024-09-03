/*
 * BME688.c
 *
 *  Created on: Jun 17, 2024
 *      Author: VKarra
 */

#include "BME688.h"
#include "i2c.h"
#include <stddef.h> // for NULL

/***** Definitions *****/

#define BME688_TX_BUFF_LEN (2u)
uint8_t tx_buff[BME688_TX_BUFF_LEN];

#define BME688_RX_BUFF_LEN (1u)
uint8_t rx_buff[BME688_RX_BUFF_LEN];

/**********************************/
//Register Definitions
/*********************************/
//Writes to BME688


void get_coeff_data(struct bme68x_calib_data* calib){
	printf("At the coeff data");
	fflush(stdout);
	uint8_t coeff_array[BME68X_LEN_COEFF_ALL];
    printf("debug 1");
	fflush(stdout);

	BME688_Read(BME68X_REG_COEFF1,coeff_array,    BME68X_LEN_COEFF1);
	printf("debug 2");
	fflush(stdout);

	BME688_Read(BME68X_REG_COEFF2,&coeff_array[BME68X_LEN_COEFF1],BME68X_LEN_COEFF2);
	BME688_Read(BME68X_REG_COEFF3,&coeff_array[BME68X_LEN_COEFF2],BME68X_LEN_COEFF3);
	printf("Finished reading into coeff arrays");
	fflush(stdout);

	calib->par_t1=(uint16_t)(BME68X_CONCAT_BYTES(coeff_array[BME68X_IDX_T1_MSB], coeff_array[BME68X_IDX_T1_LSB]));
    calib->par_t2=(int16_t)(BME68X_CONCAT_BYTES(coeff_array[BME68X_IDX_T2_MSB], coeff_array[BME68X_IDX_T2_LSB]));
	calib->par_t3 = (int8_t)(coeff_array[BME68X_IDX_T3]);

	calib->par_p1 =
            (uint16_t)(BME68X_CONCAT_BYTES(coeff_array[BME68X_IDX_P1_MSB], coeff_array[BME68X_IDX_P1_LSB]));
	calib->par_p2 =
            (int16_t)(BME68X_CONCAT_BYTES(coeff_array[BME68X_IDX_P2_MSB], coeff_array[BME68X_IDX_P2_LSB]));
	calib->par_p3 = (int8_t)coeff_array[BME68X_IDX_P3];
	calib->par_p4 =
            (int16_t)(BME68X_CONCAT_BYTES(coeff_array[BME68X_IDX_P4_MSB], coeff_array[BME68X_IDX_P4_LSB]));
	calib->par_p5 =
            (int16_t)(BME68X_CONCAT_BYTES(coeff_array[BME68X_IDX_P5_MSB], coeff_array[BME68X_IDX_P5_LSB]));
	calib->par_p6 = (int8_t)(coeff_array[BME68X_IDX_P6]);
	calib->par_p7 = (int8_t)(coeff_array[BME68X_IDX_P7]);
	calib->par_p8 =
            (int16_t)(BME68X_CONCAT_BYTES(coeff_array[BME68X_IDX_P8_MSB], coeff_array[BME68X_IDX_P8_LSB]));
	calib->par_p9 =
            (int16_t)(BME68X_CONCAT_BYTES(coeff_array[BME68X_IDX_P9_MSB], coeff_array[BME68X_IDX_P9_LSB]));
	calib->par_p10 = (uint8_t)(coeff_array[BME68X_IDX_P10]);

        /* Humidity related coefficients */
	calib->par_h1 =
            (uint16_t)(((uint16_t)coeff_array[BME68X_IDX_H1_MSB] << 4) |
                       (coeff_array[BME68X_IDX_H1_LSB] & BME68X_BIT_H1_DATA_MSK));
	calib->par_h2 =
            (uint16_t)(((uint16_t)coeff_array[BME68X_IDX_H2_MSB] << 4) | ((coeff_array[BME68X_IDX_H2_LSB]) >> 4));
	calib->par_h3 = (int8_t)coeff_array[BME68X_IDX_H3];
	calib->par_h4 = (int8_t)coeff_array[BME68X_IDX_H4];
	calib->par_h5 = (int8_t)coeff_array[BME68X_IDX_H5];
	calib->par_h6 = (uint8_t)coeff_array[BME68X_IDX_H6];
	calib->par_h7 = (int8_t)coeff_array[BME68X_IDX_H7];

        /* Gas heater related coefficients */
	calib->par_gh1 = (int8_t)coeff_array[BME68X_IDX_GH1];
	calib->par_gh2 =
            (int16_t)(BME68X_CONCAT_BYTES(coeff_array[BME68X_IDX_GH2_MSB], coeff_array[BME68X_IDX_GH2_LSB]));
	calib->par_gh3 = (int8_t)coeff_array[BME68X_IDX_GH3];

        /* Other coefficients */
	calib->res_heat_range = ((coeff_array[BME68X_IDX_RES_HEAT_RANGE] & BME68X_RHRANGE_MSK) / 16);
	calib->res_heat_val = (int8_t)coeff_array[BME68X_IDX_RES_HEAT_VAL];
	calib->range_sw_err = ((int8_t)(coeff_array[BME68X_IDX_RANGE_SW_ERR] & BME68X_RSERROR_MSK)) / 16;
    printf("\nParameters found\n");
    fflush(stdout);
	
}
int8_t BME688_Write(uint8_t reg_addr, uint8_t tx_data, uint8_t tx_len)
{
	// the format for reading is [addr, dummy data, data_read]
	tx_buff[0] = reg_addr;
	tx_buff[1] = tx_data;
    printf(" write call! ");
    printf("\nAddr: %x\n", reg_addr);
    printf("\nTx Data: %x\n", tx_data);
    fflush(stdout);

	mxc_i2c_req_t req;
	req.i2c =  MXC_I2C0_BUS0;
	req.addr = I2C_ADDR;
	req.tx_buf = tx_buff;
	req.tx_len = tx_len;
	req.rx_buf = NULL;
	req.rx_len = 0;
	req.restart = 0;
	req.callback = NULL;

	
	return MXC_I2C_MasterTransaction(&req);
}
/* This internal API is used to limit the max value of a parameter */
int8_t boundary_check(uint8_t *value, uint8_t max, struct bme68x_dev *dev)
{
    int8_t rslt =BME68X_OK; //my change right here
    printf("VALue:%d\n", *value);
    // rslt = null_ptr_check(dev);
    if ((value != NULL))
    {
        /* Check if value is above maximum value */
        if (*value > max)
        {
            /* Auto correct the invalid value to maximum value */
            *value = max;
            // dev->info_msg |= BME68X_I_PARAM_CORR;
        }
    }
    else
    {
        rslt = BME68X_E_NULL_PTR;
    }

    return rslt;
}

int8_t setMode(const uint8_t op_mode){
    int8_t rslt = BME68X_OK;
    uint8_t tmp_pow_mode=0;
    uint8_t pow_mode = 0;
    uint8_t reg_addr = BME68X_REG_CTRL_MEAS;

    /* Call until in sleep */
    do
    {
        if(BME688_Read(BME68X_REG_CTRL_MEAS,&tmp_pow_mode,1)>=0){
            printf("\nread out value from set mode1: %x\n",tmp_pow_mode);
        }
        else{
            printf("CATSRTOPHIC FAILURE ON READ");
            fflush(stdout);
        }
            /* Put to sleep before changing mode */
        pow_mode = (tmp_pow_mode & BME68X_MODE_MSK);
        if (pow_mode != BME68X_SLEEP_MODE)
        {
            printf("\nwriting value from set mode 0: %x\n",tmp_pow_mode);
            tmp_pow_mode &= ~BME68X_MODE_MSK; /* Set to sleep */
            printf("\nwriting value from set mode 1: %x\n",tmp_pow_mode);
            BME688_Write(BME68X_REG_CTRL_MEAS,tmp_pow_mode,1);
            printf("\nwriting value from set mode 2: %x\n",tmp_pow_mode);
            MXC_Delay(MXC_DELAY_MSEC(100000));
            if(BME688_Read(BME68X_REG_CTRL_MEAS,&tmp_pow_mode,1)>=0){
            printf("\nread out value from set mode3: %x\n",tmp_pow_mode);
        }
        else{
            printf("CATSRTOPHIC FAILURE ON READ");
            fflush(stdout);
        }
        }
    } while ((pow_mode != BME68X_SLEEP_MODE) && (rslt == BME68X_OK));

    /* Already in sleep */
    if ((op_mode != BME68X_SLEEP_MODE) && (rslt == BME68X_OK))
    {
        tmp_pow_mode = (tmp_pow_mode & ~BME68X_MODE_MSK) | (op_mode & BME68X_MODE_MSK);
        BME688_Write(reg_addr,tmp_pow_mode,1);
        printf("writing value from set mode 2: %x\n",tmp_pow_mode);
    }
    printf("exiting setMode");
    fflush(stdout);
    return rslt;
}


int8_t bme68x_set_op_mode(const uint8_t op_mode, struct bme68x_dev *Empty){
    return setMode(op_mode);
}

/* This internal API is used to calculate the gas wait */
uint8_t calc_gas_wait(uint16_t dur)
{
    uint8_t factor = 0;
    uint8_t durval;

    if (dur >= 0xfc0)
    {
        durval = 0xff; /* Max duration*/
    }
    else
    {
        while (dur > 0x3F)
        {
            dur = dur / 4;
            factor += 1;
        }

        durval = (uint8_t)(dur + (factor * 64));
    }

    return durval;
}
int8_t bme68x_get_op_mode(uint8_t *op_mode, struct bme68x_dev *dev)
{
    int8_t rslt;
    uint8_t mode;

    if (op_mode)
    {
        rslt = bme68x_get_regs(BME68X_REG_CTRL_MEAS, &mode, 1, dev);

        /* Masking the other register bit info*/
        *op_mode = mode & BME68X_MODE_MSK;
    }
    else
    {
        rslt = BME68X_E_NULL_PTR;
    }

    return rslt;
}
int8_t bme68x_get_regs(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, struct bme68x_dev *dev){
	return BME688_Read(reg_addr,reg_data,len);
}
/**
 * @brief Retrieves the oversampling, filter, and output data rate (ODR) configuration of the BME68X sensor.
 *
 *
 * @param conf Pointer to a `bme68x_conf` structure where the extracted configuration will be stored.
 * @param dev EMptyy this is legacy from the original code that I recived I will delete later
 * 
 * @return int8_t Returns `BME68X_OK` if the configuration was successfully retrieved, otherwise returns an
 *         error code indicating the failure reason. Possible error codes include `BME68X_E_NULL_PTR`
 *         for null pointers and other specific error codes defined by the BME68X driver.
 */

int8_t bme68x_get_conf(struct bme68x_conf *conf, struct bme68x_dev *dev)
{
    int8_t rslt;

    /* starting address of the register array for burst read*/
    uint8_t reg_addr = BME68X_REG_CTRL_GAS_1;
    uint8_t data_array[BME68X_LEN_CONFIG];

    rslt = bme68x_get_regs(reg_addr, data_array, 5, dev);
    if (!conf)
    {
        rslt = BME68X_E_NULL_PTR;
    }
    else if (rslt == BME68X_OK)
    {
        conf->os_hum = BME68X_GET_BITS_POS_0(data_array[1], BME68X_OSH);
        conf->filter = BME68X_GET_BITS(data_array[4], BME68X_FILTER);
        conf->os_temp = BME68X_GET_BITS(data_array[3], BME68X_OST);
        conf->os_pres = BME68X_GET_BITS(data_array[3], BME68X_OSP);
        if (BME68X_GET_BITS(data_array[0], BME68X_ODR3))
        {
            conf->odr = BME68X_ODR_NONE;
        }
        else
        {
            conf->odr = BME68X_GET_BITS(data_array[4], BME68X_ODR20);
        }
    }

    return rslt;
}



/* This internal API is used to calculate the heater resistance value using float */
uint8_t calc_res_heat(uint16_t temp, const struct bme68x_calib_data* calib, float amb_temp)
{
    float var1;
    float var2;
    float var3;
    float var4;
    float var5;
    uint8_t res_heat;

    if (temp > 400) /* Cap temperature */
    {
        temp = 400;
    }

    var1 = (((float)calib->par_gh1 / (16.0f)) + 49.0f);
    var2 = ((((float)calib->par_gh2 / (32768.0f)) * (0.0005f)) + 0.00235f);
    var3 = ((float)calib->par_gh3 / (1024.0f));
    var4 = (var1 * (1.0f + (var2 * (float)temp)));
    var5 = (var4 + (var3 * (float)amb_temp));
    res_heat =
        (uint8_t)(3.4f *
                  ((var5 * (4 / (4 + (float)calib->res_heat_range)) *
                    (1 / (1 + ((float)calib->res_heat_val * 0.002f)))) -
                   25));

    return res_heat;
}

//Reads from the BME688
int8_t BME688_Read(uint8_t reg_addr, uint8_t *data, uint8_t rx_len)
{

	// the format for reading is [addr, dummy data, data_read]
    printf("\nCALLED READ FUNCTION! \n");
	tx_buff[0] = reg_addr;
	// tx_buff[1] = *data;

	int rslt=BME68X_OK;

	mxc_i2c_req_t req;
	req.i2c =  MXC_I2C0_BUS0;
	req.addr = I2C_ADDR;
	req.tx_buf = tx_buff;
	// req.tx_len = BME688_TX_BUFF_LEN;
	req.tx_len = 1;
    
	req.rx_buf = data;
	req.rx_len = rx_len;
	req.restart = 0;
	req.callback = NULL;
    printf("\n Read Addr %x \n", reg_addr);

	
	rslt= MXC_I2C_MasterTransaction(&req);
	if(rslt<0){
		printf("FAILURE ON I2C READ, RETURN CODE: %d", rslt);
		fflush(stdout);
	}
	else{
		printf("\nSucess ON I2C READ, RETURN CODE: %x", rslt);
		fflush(stdout);
	}
	printf("\nExiting Read function");
		fflush(stdout);

	return rslt;
}




//Looks for all the available I2C devices
int I2C_device_scan()
{
	int error;
	uint8_t counter = 0;


	printf("-->Scanning started\n");

	mxc_i2c_req_t reqMaster;
	reqMaster.i2c = MXC_I2C0_BUS0;
	reqMaster.addr = 0;
	reqMaster.tx_buf = NULL;
	reqMaster.tx_len = 0;
	reqMaster.rx_buf = NULL;
	reqMaster.rx_len = 0;
	reqMaster.restart = 0;
	reqMaster.callback = NULL;

	for (uint8_t address = 8; address < 120; address++)
	{
		reqMaster.addr = address;
		printf(".");

		if ((MXC_I2C_MasterTransaction(&reqMaster)) == 0)
		{
			printf("\nFound slave ID %03d; 0x%02X\n", address, address);
			counter++;
		}
		MXC_Delay(MXC_DELAY_MSEC(200));
	}

	printf("\n-->Scan finished. %d devices found\n", counter);
	return E_NO_ERROR;
}


//Writing OxE0 to 0XB6 register soft resets the devices which is same as power on reset
int BME688_soft_reset()
{
	int result;
	uint8_t reg_addr = BME68X_REG_SOFT_RESET;

	/* 0xb6 is the soft reset command */
	uint8_t soft_rst_cmd = BME68X_SOFT_RESET_CMD;

	result = BME688_Write(reg_addr, soft_rst_cmd, 2);
	MXC_Delay(MXC_DELAY_MSEC(5));
	return result;

}
float calc_temperature(uint32_t temp_adc, struct bme68x_calib_data  *calib)
{
    float var1;
    float var2;
    float calc_temp;

    /* calculate var1 data */
    var1 = ((((float)temp_adc / 16384.0f) - ((float)calib->par_t1 / 1024.0f)) * ((float)calib->par_t2));
    printf("var1 temp: %f", var1);
    fflush(stdout);

    /* calculate var2 data */
    var2 =
        (((((float)temp_adc / 131072.0f) - ((float)calib->par_t1 / 8192.0f)) *
          (((float)temp_adc / 131072.0f) - ((float)calib->par_t1 / 8192.0f))) * ((float)calib->par_t3 * 16.0f));
    printf("var2 temp: %f", var2);
    fflush(stdout);

    /* t_fine value*/
    calib->t_fine = (var1 + var2);

    /* compensated temperature data*/
    calc_temp = ((calib->t_fine) / 5120.0f);
    printf("From within calc temp: %f", calc_temp);
    fflush(stdout);

    return calc_temp;
}

/* @brief This internal API is used to calculate the pressure value. */
static float calc_pressure(uint32_t pres_adc, const struct bme68x_calib_data *calib)
{
    float var1;
    float var2;
    float var3;
    float calc_pres;

    var1 = (((float)calib->t_fine / 2.0f) - 64000.0f);
    var2 = var1 * var1 * (((float)calib->par_p6) / (131072.0f));
    var2 = var2 + (var1 * ((float)calib->par_p5) * 2.0f);
    var2 = (var2 / 4.0f) + (((float)calib->par_p4) * 65536.0f);
    var1 = (((((float)calib->par_p3 * var1 * var1) / 16384.0f) + ((float)calib->par_p2 * var1)) / 524288.0f);
    var1 = ((1.0f + (var1 / 32768.0f)) * ((float)calib->par_p1));
    calc_pres = (1048576.0f - ((float)pres_adc));

    /* Avoid exception caused by division by zero */
    if ((int)var1 != 0)
    {
        calc_pres = (((calc_pres - (var2 / 4096.0f)) * 6250.0f) / var1);
        var1 = (((float)calib->par_p9) * calc_pres * calc_pres) / 2147483648.0f;
        var2 = calc_pres * (((float)calib->par_p8) / 32768.0f);
        var3 = ((calc_pres / 256.0f) * (calc_pres / 256.0f) * (calc_pres / 256.0f) * (calib->par_p10 / 131072.0f));
        calc_pres = (calc_pres + (var1 + var2 + var3 + ((float)calib->par_p7 * 128.0f)) / 16.0f);
    }
    else
    {
        calc_pres = 0;
    }

    return calc_pres;
}
static float calc_gas_resistance_high(uint16_t gas_res_adc, uint8_t gas_range)
{
    float calc_gas_res;
    uint32_t var1 = UINT32_C(262144) >> gas_range;
    int32_t var2 = (int32_t)gas_res_adc - INT32_C(512);

    var2 *= INT32_C(3);
    var2 = INT32_C(4096) + var2;

    calc_gas_res = 1000000.0f * (float)var1 / (float)var2;

    return calc_gas_res;
}

/* This internal API is used to calculate the humidity in integer */
static float calc_humidity(uint16_t hum_adc, const struct bme68x_calib_data *calib)
{
    float calc_hum;
    float var1;
    float var2;
    float var3;
    float var4;
    float temp_comp;

    /* compensated temperature data*/
    temp_comp = ((calib->t_fine) / 5120.0f);
    var1 = (float)((float)hum_adc) -
           (((float)calib->par_h1 * 16.0f) + (((float)calib->par_h3 / 2.0f) * temp_comp));
    var2 = var1 *
           ((float)(((float)calib->par_h2 / 262144.0f) *
                    (1.0f + (((float)calib->par_h4 / 16384.0f) * temp_comp) +
                     (((float)calib->par_h5 / 1048576.0f) * temp_comp * temp_comp))));
    var3 = (float)calib->par_h6 / 16384.0f;
    var4 = (float)calib->par_h7 / 2097152.0f;
    calc_hum = var2 + ((var3 + (var4 * temp_comp)) * var2 * var2);
    if (calc_hum > 100.0f)
    {
        calc_hum = 100.0f;
    }
    else if (calc_hum < 0.0f)
    {
        calc_hum = 0.0f;
    }

    return calc_hum;
}

static float calc_gas_resistance_low(uint16_t gas_res_adc, uint8_t gas_range, const struct  bme68x_calib_data* calib)
{
    float calc_gas_res;
    float var1;
    float var2;
    float var3;
    float gas_res_f = gas_res_adc;
    float gas_range_f = (1U << gas_range); /*lint !e790 / Suspicious truncation, integral to float */
    const float lookup_k1_range[16] = {
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, -0.8f, 0.0f, 0.0f, -0.2f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f
    };
    const float lookup_k2_range[16] = {
        0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.7f, 0.0f, -0.8f, -0.1f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
    };

    var1 = (1340.0f + (5.0f * calib->range_sw_err));
    var2 = (var1) * (1.0f + lookup_k1_range[gas_range] / 100.0f);
    var3 = 1.0f + (lookup_k2_range[gas_range] / 100.0f);
    calc_gas_res = 1.0f / (float)(var3 * (0.000000125f) * gas_range_f * (((gas_res_f - 512.0f) / var2) + 1.0f));

    return calc_gas_res;
}


/* This internal API is used to read a single data of the sensor */

int read_field_data(struct bme68x_calib_data* calib,	struct bme68x_data* data, int variant_id) {
	get_coeff_data(calib);
    uint8_t buff[BME68X_LEN_FIELD] = { 0 };
	//46 0X2E
	uint32_t adc_temp;
    uint32_t adc_pres;
    uint16_t adc_hum;
	uint8_t gas_range_l, gas_range_h;
    uint16_t adc_gas_res_low, adc_gas_res_high;
    int tries = 5;
    while (tries>0){
        tries--;
        printf("\nGiving it a try\n");
        fflush(stdout);
        BME688_Read(BME68X_REG_FIELD0+17,buff,(uint16_t)BME68X_LEN_FIELD);
        printf("got the buffer filled");
        fflush(stdout);
        data->status = buff[0] & BME68X_NEW_DATA_MSK;
        data->gas_index = buff[0] & BME68X_GAS_INDEX_MSK;
        data->meas_index = buff[1];
        adc_pres = (uint32_t)(((uint32_t)buff[2] * 4096) | ((uint32_t)buff[3] * 16) | ((uint32_t)buff[4] / 16));
        adc_temp = (uint32_t)(((uint32_t)buff[5] * 4096) | ((uint32_t)buff[6] * 16) | ((uint32_t)buff[7] / 16));
        adc_hum = (uint16_t)(((uint32_t)buff[8] * 256) | (uint32_t)buff[9]);
        adc_gas_res_low = (uint16_t)((uint32_t)buff[13] * 4 | (((uint32_t)buff[14]) / 64));
        adc_gas_res_high = (uint16_t)((uint32_t)buff[15] * 4 | (((uint32_t)buff[16]) / 64));
        gas_range_l = buff[14] & BME68X_GAS_RANGE_MSK;
        gas_range_h = buff[16] & BME68X_GAS_RANGE_MSK;
        if (variant_id == BME68X_VARIANT_GAS_HIGH)
        {
            data->status |= buff[16] & BME68X_GASM_VALID_MSK;
            data->status |= buff[16] & BME68X_HEAT_STAB_MSK;
        }
        else
        {
            data->status |= buff[14] & BME68X_GASM_VALID_MSK;
            data->status |= buff[14] & BME68X_HEAT_STAB_MSK;
        }

        if (data->status & BME68X_NEW_DATA_MSK)
        {
            int gas_index= buff[0] & BME68X_GAS_INDEX_MSK;
            BME688_Read(BME68X_REG_RES_HEAT0+gas_index,&data->res_heat,1);
            BME688_Read(BME68X_REG_IDAC_HEAT0+gas_index,&data->idac,1);
            BME688_Read(BME68X_REG_GAS_WAIT0+gas_index,&data->gas_wait,1);

            data->temperature = calc_temperature(adc_temp, calib);
            data->pressure = calc_pressure(adc_pres, calib);
            data->humidity = calc_humidity(adc_hum, calib);
            if (variant_id == BME68X_VARIANT_GAS_HIGH)
            {
                data->gas_resistance = calc_gas_resistance_high(adc_gas_res_high, gas_range_h);
            }
            else
            {
                data->gas_resistance = calc_gas_resistance_low(adc_gas_res_low, gas_range_l, calib);
            }

            }
        printf("exiting the get Temp function");
        fflush(stdout);

        MXC_Delay(MXC_DELAY_MSEC(500));
    }
    return 0;
}
static int read_variant_id()
{
	int8_t rslt;
	uint8_t reg_data = 0;

	/* Read variant ID information register */
	rslt = BME688_Read(BME68X_REG_VARIANT_ID, &reg_data, 1);

	if (rslt != BME68X_OK)
	{
		printf("Error reading variant ID\r\n");
	}

	return BME68X_OK;
}
int8_t bme68x_set_regs(const uint8_t *reg_addr, const uint8_t *reg_data, uint32_t len, struct bme68x_dev *Empty){
    int err = 0;
    for (int i=0;i<len;i++){
	    err = BME688_Write(reg_addr[i],reg_data[i],1);
        if(err<0){
            return err;
        }
    }
    return err;
}
int8_t bme68x_init()
{
	int result;
	uint8_t chip_id;
	(void) BME688_soft_reset(); //Perform soft reset

	//Now we read the chip ID
	result = BME688_Read(BME68X_REG_CHIP_ID, &chip_id, 1);

	if(result == BME68X_OK )
	{
        
		if(chip_id == BME68X_CHIP_ID)
		{
			result = read_variant_id();
			if(result == BME68X_OK)
			{

			}
		}
	}
	return result;
}
/**
 * @brief This API is used to set the oversampling, filter and odr configuration
 * @param conf Pointer to a `bme68x_conf` structure where the extracted configuration will be stored.
 * @param dev EMptyy this is legacy from the original code that I recived I will delete later
 * 
 * @return int8_t Returns `BME68X_OK` if the configuration was successfully retrieved, otherwise returns an
 *         error code indicating the failure reason. Possible error codes include `BME68X_E_NULL_PTR`
 *         for null pointers and other specific error codes defined by the BME68X driver.
 */
int8_t bme68x_set_conf(struct bme68x_conf *conf, struct bme68x_dev *dev)
{
    int8_t rslt;
    uint8_t odr20 = 0, odr3 = 1;
    uint8_t current_op_mode;

    /* Register data starting from BME68X_REG_CTRL_GAS_1(0x71) up to BME68X_REG_CONFIG(0x75) */
    uint8_t reg_array[BME68X_LEN_CONFIG] = { 0x71, 0x72, 0x73, 0x74, 0x75 };
    uint8_t data_array[BME68X_LEN_CONFIG] = { 0 };

    rslt = bme68x_get_op_mode(&current_op_mode, dev);
    if (rslt == BME68X_OK)
    {
        /* Configure only in the sleep mode */
        printf("\nsetting set mode one bme68x\n");
        rslt = bme68x_set_op_mode(BME68X_SLEEP_MODE, dev);
    }

    if (conf == NULL)
    {
        rslt = BME68X_E_NULL_PTR;
    }
    else if (rslt == BME68X_OK)
    {
        /* Read the whole configuration and write it back once later */
        rslt = bme68x_get_regs(reg_array[0], data_array, BME68X_LEN_CONFIG, dev);
        
        // dev->info_msg = BME68X_OK;
        // if (rslt == BME68X_OK)
        // {
            rslt = boundary_check(&conf->filter, BME68X_FILTER_SIZE_127, dev);
            if(rslt!=BME68X_OK){
                printf("boundaryCheck on conf.filter failed");
            }
        // }
        // else{
        // }

        // if (rslt == BME68X_OK)
        // {
            rslt = boundary_check(&conf->os_temp, BME68X_OS_16X, dev);
              if(rslt!=BME68X_OK){
                printf("boundaryCheck on conf.os_temp failed\n");
            }
        // }

        // if (rslt == BME68X_OK)
        // {
            rslt = boundary_check(&conf->os_pres, BME68X_OS_16X, dev);
              if(rslt!=BME68X_OK){
                printf("boundaryCheck on conf.os_pres failed\n");
            }
        // }

        // if (rslt == BME68X_OK)
        // {
            rslt = boundary_check(&conf->os_hum, BME68X_OS_16X, dev);
              if(rslt!=BME68X_OK){
                printf("boundaryCheck on conf.os_hum failed\n");
            }
        // }

        // if (rslt == BME68X_OK)
        // {
            rslt = boundary_check(&conf->odr, BME68X_ODR_NONE, dev);
              if(rslt!=BME68X_OK){
                printf("boundaryCheck on conf.odr failed\n");
            }
        // }

        if (rslt == BME68X_OK)
        {
            data_array[4] = BME68X_SET_BITS(data_array[4], BME68X_FILTER, conf->filter);
            data_array[3] = BME68X_SET_BITS(data_array[3], BME68X_OST, conf->os_temp);
            data_array[3] = BME68X_SET_BITS(data_array[3], BME68X_OSP, conf->os_pres);
            data_array[1] = BME68X_SET_BITS_POS_0(data_array[1], BME68X_OSH, conf->os_hum);
            if (conf->odr != BME68X_ODR_NONE)
            {
                odr20 = conf->odr;
                odr3 = 0;
            }

            data_array[4] = BME68X_SET_BITS(data_array[4], BME68X_ODR20, odr20);
            data_array[0] = BME68X_SET_BITS(data_array[0], BME68X_ODR3, odr3);
        }
    }
    printf("\nRSlt at set conf: %d\n", rslt);
    fflush(stdout);
    // data_array[3]=0x69;
    if (rslt == BME68X_OK)
    {
        for (int i = 0; i < BME68X_LEN_CONFIG; i++)
        {
            printf("\nData arr:%x\n",data_array[i]);
            fflush(stdout);
        }
        
        rslt = bme68x_set_regs(reg_array, data_array, BME68X_LEN_CONFIG, dev);
    }


    if ((current_op_mode != BME68X_SLEEP_MODE) && (rslt == BME68X_OK))
    {
        printf("\nsetting set mode 2 bme68x\n");
        fflush(stdout);
        rslt = bme68x_set_op_mode(current_op_mode, dev);
    }
    printf("\nexiting Bme set_conf mode\n");
    fflush(stdout);

    return rslt;
}
/**
 * @brief Configures the heater settings for the BME68X sensor
 *
 * This internal function sets the heater configuration parameters for the BME68X sensor, including
 * the heating resistance and gas wait times, according to the selected operation mode. It handles
 * different modes such as forced mode, sequential mode, and parallel mode, configuring the sensor's
 * registers accordingly. It also checks for null pointers and handles errors in setting the configuration.
 *
 * @param conf Pointer to a `bme68x_heatr_conf` structure containing the heater configuration parameters.
 * @param op_mode Operation mode of the sensor (only tested for sequential at the moment!)
 * @param nb_conv Pointer to a uint8_t variable where the number of conversions to be performed will be stored.
 * @param calib Pointer to a `bme68x_calib_data` structure containing calibration data for the sensor.
 * @param amb_temp Ambient temperature used for calculations in degrees Celsius.
 * 
 * @return int8_t Returns `BME68X_OK` if the configuration was successfully set, otherwise returns an error code
 *         indicating the failure reason. Possible error codes include `BME68X_E_NULL_PTR` for null pointers,
 *         `BME68X_W_DEFINE_SHD_HEATR_DUR` for issues with shared heater duration, and other specific error codes
 *         defined by the BME68X driver.
 */
int8_t set_conf(struct bme68x_heatr_conf *conf, uint8_t op_mode, uint8_t *nb_conv,const struct bme68x_calib_data* calib,int amb_temp)
{
    int8_t rslt = BME68X_OK;
    uint8_t i;
    // uint8_t shared_dur;
    uint8_t write_len = 0;
    // uint8_t heater_dur_shared_addr = BME68X_REG_SHD_HEATR_DUR;
    uint8_t rh_reg_addr[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    uint8_t rh_reg_data[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    uint8_t gw_reg_addr[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    uint8_t gw_reg_data[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    switch (op_mode)
    {
        case BME68X_FORCED_MODE:
            rh_reg_addr[0] = BME68X_REG_RES_HEAT0;
            rh_reg_data[0] = calc_res_heat(conf->heatr_temp,calib,(float)amb_temp);
            gw_reg_addr[0] = BME68X_REG_GAS_WAIT0;
            gw_reg_data[0] = calc_gas_wait(conf->heatr_dur);
            (*nb_conv) = 0;
            write_len = 1;
            break;
        case BME68X_SEQUENTIAL_MODE:
            if ((!conf->heatr_dur_prof) || (!conf->heatr_temp_prof))
            {
                rslt = BME68X_E_NULL_PTR;
                break;
            }
            printf("passed on the temp prof");
            fflush(stdout);

            for (i = 0; i < conf->profile_len; i++)
            {
                rh_reg_addr[i] = BME68X_REG_RES_HEAT0 + i;
                rh_reg_data[i] = calc_res_heat(conf->heatr_temp_prof[i],calib,amb_temp);
                gw_reg_addr[i] = BME68X_REG_GAS_WAIT0 + i; //ok this seems to be the issue not sure why it's happenning.
                gw_reg_data[i] = calc_gas_wait(conf->heatr_dur_prof[i]);
            }

            (*nb_conv) = conf->profile_len;
            write_len = conf->profile_len;
            break;
            case BME68X_PARALLEL_MODE:
                printf("IN THE WRONG MODE");
                fflush(stdout);
      /*  case BME68X_PARALLEL_MODE:
            if ((!conf->heatr_dur_prof) || (!conf->heatr_temp_prof))
            {
                rslt = BME68X_E_NULL_PTR;
                break;
            }

            if (conf->shared_heatr_dur == 0)
            {
                rslt = BME68X_W_DEFINE_SHD_HEATR_DUR;
            }

            for (i = 0; i < conf->profile_len; i++)
            {
                rh_reg_addr[i] = BME68X_REG_RES_HEAT0 + i;
                rh_reg_data[i] = calc_res_heat(conf->heatr_temp_prof[i], dev);
                gw_reg_addr[i] = BME68X_REG_GAS_WAIT0 + i;
                gw_reg_data[i] = (uint8_t) conf->heatr_dur_prof[i];
            }

            (*nb_conv) = conf->profile_len;
            write_len = conf->profile_len;
            shared_dur = calc_heatr_dur_shared(conf->shared_heatr_dur);
            if (rslt == BME68X_OK)
            {
                rslt = bme68x_set_regs(&heater_dur_shared_addr, &shared_dur, 1, dev);
            }
*/
            break;
        default:
            rslt = -99;
    }

    if (rslt == BME68X_OK)
    {
        rslt = bme68x_set_regs(rh_reg_addr, rh_reg_data, write_len, 0);
    }

    if (rslt == BME68X_OK)
    {
        rslt = bme68x_set_regs(gw_reg_addr, gw_reg_data, write_len, 0); //this is where the error source is, regardles of what I put in for write len. 
    }

    return rslt;
}

/* This internal API is used to read all data fields of the sensor */
static int8_t read_all_field_data(struct bme68x_data * const data[], struct bme68x_dev *dev,int variant_id,bme68x_calib_data* calib)
{
        printf("Made it here!");
    fflush(stdout);

    int8_t rslt = BME68X_OK;
    uint8_t buff[BME68X_LEN_FIELD * 3] = { 0 };
    uint8_t gas_range_l, gas_range_h;
    uint32_t adc_temp;
    uint32_t adc_pres;
    uint16_t adc_hum;
    uint16_t adc_gas_res_low, adc_gas_res_high;
    uint8_t off;
    uint8_t set_val[30] = { 0 }; /* idac, res_heat, gas_wait */
    uint8_t i;
    printf("Made it here2!");
    fflush(stdout);
    if (!data[0] && !data[1] && !data[2])
    {
        rslt = BME68X_E_NULL_PTR;
        printf("\nError on Data, NULL DATA DEFINED EVERYWHERE\n");
        fflush(stdout);
        return rslt;
    }

    // if (rslt == BME68X_OK)
    // {
        printf("Made it here 3!");
        fflush(stdout);
        rslt = bme68x_get_regs(BME68X_REG_FIELD0, buff, (uint32_t) BME68X_LEN_FIELD * 3, 0);
        printf("\n regs gets 1\n");
        fflush(stdout);
        for(int i=0;i<51;i++){
            printf("Buf :%d ",buff[i]);
            fflush(stdout);
        }
        printf("\n");
        fflush(stdout);
    // }

    // if (rslt == BME68X_OK)
    // {
        rslt = bme68x_get_regs(BME68X_REG_IDAC_HEAT0, set_val, 30, 0);
        printf("\n regs gets 2\n");
        fflush(stdout);

    // }

    for (i = 0; ((i < 3) && (rslt == BME68X_OK)); i++)
    {
        off = (uint8_t)(i * BME68X_LEN_FIELD);
        printf("\nI am iron man nahnahnahanah\n");
        fflush(stdout);
        data[i]->status = buff[off] & BME68X_NEW_DATA_MSK;
        printf("\nThe status: %d\n",data[i]->status);
        fflush(stdout);

        data[i]->gas_index = buff[off] & BME68X_GAS_INDEX_MSK;
        data[i]->meas_index = buff[off + 1];

        /* read the raw data from the sensor */
        adc_pres =
            (uint32_t) (((uint32_t) buff[off + 2] * 4096) | ((uint32_t) buff[off + 3] * 16) |
                        ((uint32_t) buff[off + 4] / 16));
        adc_temp =
            (uint32_t) (((uint32_t) buff[off + 5] * 4096) | ((uint32_t) buff[off + 6] * 16) |
                        ((uint32_t) buff[off + 7] / 16));
        adc_hum = (uint16_t) (((uint32_t) buff[off + 8] * 256) | (uint32_t) buff[off + 9]);
        adc_gas_res_low = (uint16_t) ((uint32_t) buff[off + 13] * 4 | (((uint32_t) buff[off + 14]) / 64));
        adc_gas_res_high = (uint16_t) ((uint32_t) buff[off + 15] * 4 | (((uint32_t) buff[off + 16]) / 64));
        gas_range_l = buff[off + 14] & BME68X_GAS_RANGE_MSK;
        gas_range_h = buff[off + 16] & BME68X_GAS_RANGE_MSK;
        if (variant_id == BME68X_VARIANT_GAS_HIGH)
        {
            data[i]->status |= buff[off + 16] & BME68X_GASM_VALID_MSK;
            data[i]->status |= buff[off + 16] & BME68X_HEAT_STAB_MSK;
        }
        else
        {
            data[i]->status |= buff[off + 14] & BME68X_GASM_VALID_MSK;
            data[i]->status |= buff[off + 14] & BME68X_HEAT_STAB_MSK;
        }

        data[i]->idac = set_val[data[i]->gas_index];
        data[i]->res_heat = set_val[10 + data[i]->gas_index];
        data[i]->gas_wait = set_val[20 + data[i]->gas_index];
        data[i]->temperature = calc_temperature(adc_temp, calib);
        printf("after calc temperature, %f",data[i]->temperature);
        printf("after calc temperature, %.2f",data[i]->temperature);

        data[i]->pressure = calc_pressure(adc_pres, calib);
        data[i]->humidity = calc_humidity(adc_hum, calib);
    if (variant_id == BME68X_VARIANT_GAS_HIGH)
        {
            data[i]->gas_resistance = calc_gas_resistance_high(adc_gas_res_high, gas_range_h);
        }
        else
        {
            data[i]->gas_resistance = calc_gas_resistance_low(adc_gas_res_low, gas_range_l, calib);
        }
    }
    
    printf("Made it here4!");
    fflush(stdout);
    return rslt;
}
static void swap_fields(uint8_t index1, uint8_t index2, struct bme68x_data *field[])
{
    struct bme68x_data *temp;

    temp = field[index1];
    field[index1] = field[index2];
    field[index2] = temp;
}


/* This internal API is used sort the sensor data */
static void sort_sensor_data(uint8_t low_index, uint8_t high_index, struct bme68x_data *field[])
{
    int16_t meas_index1;
    int16_t meas_index2;

    meas_index1 = (int16_t)field[low_index]->meas_index;
    meas_index2 = (int16_t)field[high_index]->meas_index;
    if ((field[low_index]->status & BME68X_NEW_DATA_MSK) && (field[high_index]->status & BME68X_NEW_DATA_MSK))
    {
        int16_t diff = meas_index2 - meas_index1;
        if (((diff > -3) && (diff < 0)) || (diff > 2))
        {
            swap_fields(low_index, high_index, field);
        }
    }
    else if (field[high_index]->status & BME68X_NEW_DATA_MSK)
    {
        swap_fields(low_index, high_index, field);
    }
}

/*
 * @brief This API reads the pressure, temperature and humidity and gas data
 * from the sensor, compensates the data and store it in the bme68x_data
 * structure instance passed by the user.
 */
int8_t bme68x_get_data(uint8_t op_mode, struct bme68x_data *data, uint8_t *n_data, struct bme68x_dev *dev, int variant_id,struct bme68x_calib_data* calib)
{
    int8_t rslt;
    uint8_t i = 0, j = 0, new_fields = 0;
    struct bme68x_data *field_ptr[3] = { 0 };
    struct bme68x_data field_data[3] = { { 0 } };

    field_ptr[0] = &field_data[0];
    field_ptr[1] = &field_data[1];
    field_ptr[2] = &field_data[2];
    printf("here!");
    fflush(stdout);
    // rslt = null_ptr_check(dev);
    if (data != NULL)
    {
        printf("Data is rea;");
        /* Reading the sensor data in forced mode only */
        if (op_mode == BME68X_FORCED_MODE) //Not being used
        {
            rslt = read_field_data(calib,data,0); //Not being used
            printf("Got field data"); //Not being used
            fflush(stdout); //Not being used
            if (rslt == BME68X_OK)
            {
                if (data->status & BME68X_NEW_DATA_MSK)
                {
                    new_fields = 1;
                }
                else
                {
                    new_fields = 0;
                    // rslt = BME68X_W_NO_NEW_DATA;
                }
            }
        }
        else if ((op_mode == BME68X_PARALLEL_MODE) || (op_mode == BME68X_SEQUENTIAL_MODE))
        {
            /* Read the 3 fields and count the number of new data fields */
            rslt = read_all_field_data(field_ptr, dev,variant_id,calib);
            printf("Got field data");
            fflush(stdout);

            printf("rslst %d",(int)rslt);
            fflush(stdout);

            new_fields = 0;
            for (i = 0; (i < 3) && (rslt == BME68X_OK); i++)
            {

                printf("\nf ptr: %d\n",field_ptr[i]->status);
                if (field_ptr[i]->status & BME68X_NEW_DATA_MSK)
                {
                    new_fields++;
                    printf("\nAdding new field, new_fields: %d\n", new_fields);
                    fflush(stdout);
                }
            }

            /* Sort the sensor data in parallel & sequential modes*/
            for (i = 0; (i < 2) && (rslt == BME68X_OK); i++)
            {
                for (j = i + 1; j < 3; j++)
                {
                    sort_sensor_data(i, j, field_ptr);
                }
            }

            /* Copy the sorted data */
            for (i = 0; ((i < 3) && (rslt == BME68X_OK)); i++)
            {
                data[i] = *field_ptr[i];
            }

            if (new_fields == 0)
            {
                // rslt = BME68X_W_NO_NEW_DATA;
            }
        }
        else
        {
            // rslt = BME68X_W_DEFINE_OP_MODE;
        }

        if (n_data == NULL)
        {
            rslt = BME68X_E_NULL_PTR;
        }
        else
        {
            *n_data = new_fields;
        }
    }
    else
    {
        rslt = BME68X_E_NULL_PTR;
    }

    return rslt;
}


/**
 * @brief This does the gas configuration and then does the heater configuation
 * IT IS SEPERATE FROM THE SET CONF WHICH JUST SETS THE SAMPLING RATE, FILTER & ODR
 * 
 */
int8_t bme68x_set_heatr_conf(uint8_t op_mode, const struct bme68x_heatr_conf *conf, int amb_temp, struct bme68x_dev *dev, int Variant_id, struct bme68x_calib_data* calib)
{
    int8_t rslt;
    uint8_t nb_conv = 0;
    uint8_t hctrl, run_gas = 0;
    uint8_t ctrl_gas_data[2];
    uint8_t ctrl_gas_addr[2] = { BME68X_REG_CTRL_GAS_0, BME68X_REG_CTRL_GAS_1 };

    if (conf != NULL)
    {
        printf("\n\n\nentering bme68x_set_heatr_conf set mode\n\n\n");
        fflush(stdout);
        rslt = bme68x_set_op_mode(BME68X_SLEEP_MODE, dev);
        if (rslt == BME68X_OK)
        {
            rslt = set_conf(conf, op_mode, &nb_conv,calib,amb_temp); //this does the heater configuraiton
            printf("\nHere in set conf\n");
            fflush(stdout);
            }

        if (rslt == BME68X_OK)
        {
            rslt = bme68x_get_regs(BME68X_REG_CTRL_GAS_0, ctrl_gas_data, 2, dev);
            if (rslt == BME68X_OK)
            {
                if (conf->enable == BME68X_ENABLE)
                {
                        printf("Here in set enable\n");
                        fflush(stdout);
                    hctrl = BME68X_ENABLE_HEATER;
                    if (Variant_id == BME68X_VARIANT_GAS_HIGH)
                    {
                        run_gas = BME68X_ENABLE_GAS_MEAS_H;
                    }
                    else
                    {
                        run_gas = BME68X_ENABLE_GAS_MEAS_L;
                    }
                }
                else
                {
                    hctrl = BME68X_DISABLE_HEATER;
                    run_gas = BME68X_DISABLE_GAS_MEAS;
                }

                ctrl_gas_data[0] = BME68X_SET_BITS(ctrl_gas_data[0], BME68X_HCTRL, hctrl);
                ctrl_gas_data[1] = BME68X_SET_BITS_POS_0(ctrl_gas_data[1], BME68X_NBCONV, nb_conv);
                ctrl_gas_data[1] = BME68X_SET_BITS(ctrl_gas_data[1], BME68X_RUN_GAS, run_gas);
                rslt = bme68x_set_regs(ctrl_gas_addr, ctrl_gas_data, 2, dev);
            }
        }
    }
    else
    {
        rslt = BME68X_E_NULL_PTR;
    }
    printf("exiting set heatr Conf");
    fflush(stdout);
    return rslt;
}



