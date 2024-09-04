/*
 * BME688_defs.h
 *
 *  Created on: Jun 27, 2024
 *      Author: VKarra
 */

#ifndef BME688_DEFS_H_
#define BME688_DEFS_H_





#define BME68X_SOFT_RESET_CMD   (0xB6u) /* Soft reset command */
#define I2C_ADDR (0x77u);  //Device I2C address
/* BME68X unique chip identifier */
#define BME68X_CHIP_ID                            UINT8_C(0x61)

//=========================================
/* Register map addresses in I2C */
//=========================================
/* Register for 3rd group of coefficients */
#define BME68X_REG_COEFF3       (0x00u)

/* 0th Field address*/
#define BME68X_REG_FIELD0       (0x1du)

/* 0th Current DAC address*/
#define BME68X_REG_IDAC_HEAT0   (0x50u)

/* 0th Res heat address */
#define BME68X_REG_RES_HEAT0    (0x5au)

/* 0th Gas wait address */
#define BME68X_REG_GAS_WAIT0    (0x64u)

/* Shared heating duration address */
#define BME68X_REG_SHD_HEATR_DUR   (0x6Eu)

/* CTRL_GAS_0 address */
#define BME68X_REG_CTRL_GAS_0      (0x70u)

/* CTRL_GAS_1 address */
#define BME68X_REG_CTRL_GAS_1      (0x71u)

/* CTRL_HUM address */
#define BME68X_REG_CTRL_HUM        (0x72u)

/* CTRL_MEAS address */
#define BME68X_REG_CTRL_MEAS       (0x74u)

/* CONFIG address */
#define BME68X_REG_CONFIG          (0x75u)

/* MEM_PAGE address */
#define BME68X_REG_MEM_PAGE        (0xf3u)

/* Unique ID address */
#define BME68X_REG_UNIQUE_ID       (0x83u)

/* Register for 1st group of coefficients */
#define BME68X_REG_COEFF1          (0x8au)

/* Chip ID address */
#define BME68X_REG_CHIP_ID         (0xd0u)

/* Soft reset address */
#define BME68X_REG_SOFT_RESET      (0xe0u)

/* Register for 2nd group of coefficients */
#define BME68X_REG_COEFF2          (0xe1u)

/* Variant ID Register */
#define BME68X_REG_VARIANT_ID      (0xF0u)

/* Enable/Disable macros */

/* Enable */
#define BME68X_ENABLE              (0x01u)

/* Disable */
#define BME68X_DISABLE             (0x00u)

/* Variant ID macros */

/* Low Gas variant */
#define BME68X_VARIANT_GAS_LOW     (0x00u)

/* High Gas variant */
#define BME68X_VARIANT_GAS_HIGH    (0x01u)


//=========================================
/* Oversampling setting macros */
//=========================================
/* Switch off measurement */
#define BME68X_OS_NONE              (0u)

/* Perform 1 measurement */
#define BME68X_OS_1X                (1u)

/* Perform 2 measurements */
#define BME68X_OS_2X                (2u)

/* Perform 4 measurements */
#define BME68X_OS_4X                (3u)

/* Perform 8 measurements */
#define BME68X_OS_8X                (4u)

/* Perform 16 measurements */
#define BME68X_OS_16X               (5u)


//=========================================
/* IIR Filter settings */
//=========================================


/* Switch off the filter */
#define BME68X_FILTER_OFF            (0u)

/* Filter coefficient of 2 */
#define BME68X_FILTER_SIZE_1         (1u)

/* Filter coefficient of 4 */
#define BME68X_FILTER_SIZE_3         (2u)

/* Filter coefficient of 8 */
#define BME68X_FILTER_SIZE_7         (3u)

/* Filter coefficient of 16 */
#define BME68X_FILTER_SIZE_15        (4u)

/* Filter coefficient of 32 */
#define BME68X_FILTER_SIZE_31        (5u)

/* Filter coefficient of 64 */
#define BME68X_FILTER_SIZE_63        (6u)

/* Filter coefficient of 128 */
#define BME68X_FILTER_SIZE_127       (7u)


//=========================================
/* ODR/Standby time macros */
//=========================================


/* Standby time of 0.59ms */
#define BME68X_ODR_0_59_MS          (0u)

/* Standby time of 62.5ms */
#define BME68X_ODR_62_5_MS          (1u)

/* Standby time of 125ms */
#define BME68X_ODR_125_MS           (2u)

/* Standby time of 250ms */
#define BME68X_ODR_250_MS           (3u)

/* Standby time of 500ms */
#define BME68X_ODR_500_MS           (4u)

/* Standby time of 1s */
#define BME68X_ODR_1000_MS          (5u)

/* Standby time of 10ms */
#define BME68X_ODR_10_MS            (6u)

/* Standby time of 20ms */
#define BME68X_ODR_20_MS            (7u)

/* No standby time */
#define BME68X_ODR_NONE             (8u)

//=========================================
/* Operating mode macros */
//=========================================

/* Sleep operation mode */
#define BME68X_SLEEP_MODE           (0u)

/* Forced operation mode */
#define BME68X_FORCED_MODE          (1u)

/* Parallel operation mode */
#define BME68X_PARALLEL_MODE        (2u)

/* Sequential operation mode */
#define BME68X_SEQUENTIAL_MODE      (3u)


//=========================================
/* Coefficient index macros */
//=========================================

/* Length for all coefficients */
#define BME68X_LEN_COEFF_ALL                    (42u)

/* Length for 1st group of coefficients */
#define BME68X_LEN_COEFF1                       (23u)

/* Length for 2nd group of coefficients */
#define BME68X_LEN_COEFF2                       (14u)

/* Length for 3rd group of coefficients */
#define BME68X_LEN_COEFF3                        (5u)

/* Length of the field */
#define BME68X_LEN_FIELD                        (17u)

/* Length between two fields */
#define BME68X_LEN_FIELD_OFFSET                 (17u)

/* Length of the configuration register */
#define BME68X_LEN_CONFIG                       (5u)

/* Length of the interleaved buffer */
#define BME68X_LEN_INTERLEAVE_BUFF              (20u)


//=========================================
/* Coefficient index macros */
//=========================================

/* Coefficient T2 LSB position */
#define BME68X_IDX_T2_LSB                         (0)

/* Coefficient T2 MSB position */
#define BME68X_IDX_T2_MSB                         (1)

/* Coefficient T3 position */
#define BME68X_IDX_T3                             (2)

/* Coefficient P1 LSB position */
#define BME68X_IDX_P1_LSB                         (4)

/* Coefficient P1 MSB position */
#define BME68X_IDX_P1_MSB                         (5)

/* Coefficient P2 LSB position */
#define BME68X_IDX_P2_LSB                         (6)

/* Coefficient P2 MSB position */
#define BME68X_IDX_P2_MSB                         (7)

/* Coefficient P3 position */
#define BME68X_IDX_P3                             (8)

/* Coefficient P4 LSB position */
#define BME68X_IDX_P4_LSB                         (10)

/* Coefficient P4 MSB position */
#define BME68X_IDX_P4_MSB                         (11)

/* Coefficient P5 LSB position */
#define BME68X_IDX_P5_LSB                         (12)

/* Coefficient P5 MSB position */
#define BME68X_IDX_P5_MSB                         (13)

/* Coefficient P7 position */
#define BME68X_IDX_P7                             (14)

/* Coefficient P6 position */
#define BME68X_IDX_P6                             (15)

/* Coefficient P8 LSB position */
#define BME68X_IDX_P8_LSB                         (18)

/* Coefficient P8 MSB position */
#define BME68X_IDX_P8_MSB                         (19)

/* Coefficient P9 LSB position */
#define BME68X_IDX_P9_LSB                         (20)

/* Coefficient P9 MSB position */
#define BME68X_IDX_P9_MSB                         (21)

/* Coefficient P10 position */
#define BME68X_IDX_P10                            (22)

/* Coefficient H2 MSB position */
#define BME68X_IDX_H2_MSB                         (23)

/* Coefficient H2 LSB position */
#define BME68X_IDX_H2_LSB                         (24)

/* Coefficient H1 LSB position */
#define BME68X_IDX_H1_LSB                         (24)

/* Coefficient H1 MSB position */
#define BME68X_IDX_H1_MSB                         (25)

/* Coefficient H3 position */
#define BME68X_IDX_H3                             (26)

/* Coefficient H4 position */
#define BME68X_IDX_H4                             (27)

/* Coefficient H5 position */
#define BME68X_IDX_H5                             (28)

/* Coefficient H6 position */
#define BME68X_IDX_H6                             (29)

/* Coefficient H7 position */
#define BME68X_IDX_H7                             (30)

/* Coefficient T1 LSB position */
#define BME68X_IDX_T1_LSB                         (31)

/* Coefficient T1 MSB position */
#define BME68X_IDX_T1_MSB                         (32)

/* Coefficient GH2 LSB position */
#define BME68X_IDX_GH2_LSB                        (33)

/* Coefficient GH2 MSB position */
#define BME68X_IDX_GH2_MSB                        (34)

/* Coefficient GH1 position */
#define BME68X_IDX_GH1                            (35)

/* Coefficient GH3 position */
#define BME68X_IDX_GH3                            (36)

/* Coefficient res heat value position */
#define BME68X_IDX_RES_HEAT_VAL                   (37)

/* Coefficient res heat range position */
#define BME68X_IDX_RES_HEAT_RANGE                 (39)

/* Coefficient range switching error position */
#define BME68X_IDX_RANGE_SW_ERR                   (41)


//=========================================
/* Gas measurement macros */
//=========================================

/* Disable gas measurement */
#define BME68X_DISABLE_GAS_MEAS                  (0x00u)

/* Enable gas measurement low */
#define BME68X_ENABLE_GAS_MEAS_L                 (0x01u)

/* Enable gas measurement high */
#define BME68X_ENABLE_GAS_MEAS_H                 (0x02u)


//=========================================
/* Heater control macros */
//=========================================

/* Enable heater */
#define BME68X_ENABLE_HEATER                     (0x00u)

/* Disable heater */
#define BME68X_DISABLE_HEATER                    (0x01u)
#define BME68X_USE_FPU 1
enum bme68x_intf {
    /*! SPI interface */
    BME68X_SPI_INTF,
    /*! I2C interface */
    BME68X_I2C_INTF
};
#ifdef BME68X_USE_FPU

/* 0 degree Celsius */
#define BME68X_MIN_TEMPERATURE                    (0)

/* 60 degree Celsius */
#define BME68X_MAX_TEMPERATURE                    (60)

/* 900 hecto Pascals */
#define BME68X_MIN_PRESSURE                       (90000)

/* 1100 hecto Pascals */
#define BME68X_MAX_PRESSURE                       (110000ul)

/* 20% relative humidity */
#define BME68X_MIN_HUMIDITY                       (20ul)

/* 80% relative humidity*/
#define BME68X_MAX_HUMIDITY                       (80ul)
#else

/* 0 degree Celsius */
#define BME68X_MIN_TEMPERATURE                    (0u)

/* 60 degree Celsius */
#define BME68X_MAX_TEMPERATURE                    (6000u)

/* 900 hecto Pascals */
#define BME68X_MIN_PRESSURE                       (90000ul)

/* 1100 hecto Pascals */
#define BME68X_MAX_PRESSURE                       (110000ul)

/* 20% relative humidity */
#define BME68X_MIN_HUMIDITY                       (20000ul)

/* 80% relative humidity*/
#define BME68X_MAX_HUMIDITY                       (80000ul)
#endif


#define BME68X_HEATR_DUR1                        (1000u)
#define BME68X_HEATR_DUR2                        (2000u)
#define BME68X_HEATR_DUR1_DELAY                  (1000000ul)
#define BME68X_HEATR_DUR2_DELAY                  (2000000ul)
#define BME68X_N_MEAS                            (6u)
#define BME68X_LOW_TEMP                          (150u)
#define BME68X_HIGH_TEMP                         (350u)

//=========================================
/* Mask macros */
//=========================================


/* Mask for number of conversions */
#define BME68X_NBCONV_MSK                        (0X0fu)

/* Mask for IIR filter */
#define BME68X_FILTER_MSK                        (0X1cu)

/* Mask for ODR[3] */
#define BME68X_ODR3_MSK                          (0x80u)

/* Mask for ODR[2:0] */
#define BME68X_ODR20_MSK                         (0xe0u)

/* Mask for temperature oversampling */
#define BME68X_OST_MSK                           (0Xe0u)

/* Mask for pressure oversampling */
#define BME68X_OSP_MSK                           (0X1cu)

/* Mask for humidity oversampling */
#define BME68X_OSH_MSK                           (0X07u)

/* Mask for heater control */
#define BME68X_HCTRL_MSK                         (0x08u)

/* Mask for run gas */
#define BME68X_RUN_GAS_MSK                       (0x30u)

/* Mask for operation mode */
#define BME68X_MODE_MSK                          (0x03u)

/* Mask for res heat range */
#define BME68X_RHRANGE_MSK                       (0x30u)

/* Mask for range switching error */
#define BME68X_RSERROR_MSK                       (0xf0u)

/* Mask for new data */
#define BME68X_NEW_DATA_MSK                      (0x80u)

/* Mask for gas index */
#define BME68X_GAS_INDEX_MSK                     (0x0fu)

/* Mask for gas range */
#define BME68X_GAS_RANGE_MSK                     (0x0fu)

/* Mask for gas measurement valid */
#define BME68X_GASM_VALID_MSK                    (0x20u)

/* Mask for heater stability */
#define BME68X_HEAT_STAB_MSK                     (0x10u)

/* Mask for SPI memory page */
#define BME68X_MEM_PAGE_MSK                      (0x10u)

/* Mask for reading a register in SPI */
#define BME68X_SPI_RD_MSK                        (0x80u)

/* Mask for writing a register in SPI */
#define BME68X_SPI_WR_MSK                        (0x7fu)

/* Mask for the H1 calibration coefficient */
#define BME68X_BIT_H1_DATA_MSK                   (0x0fu)

/* Position macros */

/* Filter bit position */
#define BME68X_FILTER_POS                        (2u)

/* Temperature oversampling bit position */
#define BME68X_OST_POS                           (5u)

/* Pressure oversampling bit position */
#define BME68X_OSP_POS                           (2u)

/* ODR[3] bit position */
#define BME68X_ODR3_POS                          (7u)

/* ODR[2:0] bit position */
#define BME68X_ODR20_POS                         (5u)

/* Run gas bit position */
#define BME68X_RUN_GAS_POS                       (4u)

/* Heater control bit position */
#define BME68X_HCTRL_POS                         (3u)

/* Macro to combine two 8 bit data's to form a 16 bit data */
#define BME68X_CONCAT_BYTES(msb, lsb)            (((uint16_t)msb << 8) | (uint16_t)lsb)

/* Macro to set bits */
#define BME68X_SET_BITS(reg_data, bitname, data) \
    ((reg_data & ~(bitname##_MSK)) | \
     ((data << bitname##_POS) & bitname##_MSK))

/* Macro to get bits */
#define BME68X_GET_BITS(reg_data, bitname)        ((reg_data & (bitname##_MSK)) >> \
                                                   (bitname##_POS))

/* Macro to set bits starting from position 0 */
#define BME68X_SET_BITS_POS_0(reg_data, bitname, data) \
    ((reg_data & ~(bitname##_MSK)) | \
     (data & bitname##_MSK))

/* Macro to get bits starting from position 0 */
#define BME68X_GET_BITS_POS_0(reg_data, bitname)  (reg_data & (bitname##_MSK))


//=========================================
/* BME688 return codes */
//=========================================

/* Success */
#define BME68X_OK                                 0

/* Errors */

#define BME68X_E_NULL_PTR                         -1

/* Communication failure */
#define BME68X_E_COM_FAIL                         -2

/* Sensor not found */
#define BME68X_E_DEV_NOT_FOUND                    -3

/* Incorrect length parameter */
#define BME68X_E_INVALID_LENGTH                   -4

/* Self test fail error */
#define BME68X_E_SELF_TEST                        -5
//PULLED FROM THE BME68X GIT
#define BME68X_PERIOD_RESET                       UINT32_C(10000)
#define BME68X_PERIOD_POLL                        BME68X_PERIOD_RESET
/* No new data was found */
#define BME68X_W_NO_NEW_DATA                      INT8_C(2)
/* Define a valid operation mode */
#define BME68X_W_DEFINE_OP_MODE                   INT8_C(1)
#define BME68X_W_DEFINE_SHD_HEATR_DUR             INT8_C(3)


#endif /* BME688_DEFS_H_ */
