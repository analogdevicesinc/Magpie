/*
 * MAX17261.h
 *
 *  Created on: Jul 2, 2024
 *      Author: VKarra
 */

#ifndef MAX17261_H_
#define MAX17261_H_

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include "mxc_device.h"
#include "mxc_sys.h"
#include "nvic_table.h"
#include "i2c.h"
#include "i2c_regs.h"
#include "mxc_delay.h"
#include "tmr.h"
#include "dev_i2c.h"



// Fuel gauge parameters -- customer defined for Option 1 EZ Config.
//See App Note: "MAX1726x Software Implementation Guide"
//https://www.analog.com/media/en/technical-documentation/user-guides/modelgauge-m5-host-side-software-implementation-guide.pdf
//https://www.analog.com/media/en/technical-documentation/user-guides/max1726x-modelgauge-m5-ez-user-guide.pdf

enum masks
{
	ModelCfg = 0x8000, //Since charge voltage is less than 4.275 volts
	DesignCap= 0x0578,
	IChgTerm = 0x01C0,
	VEmpty   = 0x9661,
    POR_BIT  = 0x0002,
    DNR_BIT  = 0x0001,
};


// Fuel gauge parameter addresses

enum registers_addr
{
	//ModelGauge M5 EZ configuration registers
	DesignCap_addr = 0x18u,
	VEmpty_addr    = 0x3Au,
	ModelCfg_addr  = 0xDBu,
	IChgTerm_addr  = 0x1Eu,
	Config1_addr = 0x1Du,
	Config2_addr = 0xBBu,

	//ModelGauge m5 Register memory map
	FullCapNom_addr = 0x23u,
	FullCapRep_addr =0x10u,
	FullSocThr_addr =0x13u,
	FullCap_addr    =0x35u,

	LearnCfg_addr   =0x28u,
	FilterCfg_addr  =0x29u,
	RelaxCfg_addr   =0x2Au,
	MiscCfg_addr    =0x2Bu,
	QRTable00_addr  =0x12u,
	QRTable10_addr  =0x22u,
	QRTable20_addr  =0x32u,
	QRTable30_addr  =0x42u,
	RComp0_addr     =0x38u,
	TempCo_addr     =0x39u,


	// Fuel gauge parameter other can be found in
	//https://www.analog.com/media/en/technical-documentation/user-guides/max1726x-modelgauge-m5-ez-user-guide.pdf
	FStat_addr      =0x3Du,
	HibCFG_addr     =0xBAu,
	Soft_Reset_addr =0x60u,
	DevName_addr    =0x21u,
	Status_addr     =0x00u,

	// Fuel Gauge parameters to read/log
	VCell_addr      =0x09u,
	AvgVCell_addr   =0x19u,
	Temp_addr       =0x08u,
	AvgTA_addr      =0x16u,
	Current_addr    =0x0Au,
	AvgCurrent_addr =0x0Bu,
	TTF_addr        =0x20u,
	RepCap_addr     =0x05u,
	VFRemCap_addr   =0x4Au,
	MixCap_addr     =0x0Fu,
	QResidual_addr  =0x0Cu,
	REPSOC_addr     =0x06u,
	AvSOC_addr      =0x0Eu,
	Timer_addr      =0x3Eu,
	TimerH_addr     =0xBEu,
	QH_addr         =0x4Du,
	AvCap_addr      =0x1Fu,
	MixSOC_addr     =0x0Du,
	TTE_addr        =0x11u,
};


/* MAX17262 specific channels */
enum max17262_channel
{
	MAX17262_COULOMB_COUNTER,
};

// Fuel Gauge I2C Address
#define MAX17261_I2C_ADDR 0x36u //7 bit address, you can verify this by running I2C_Scan() function



/***** Functions *****/

/**
 * I2C functions used in this code
 */

/***** Function Prototypes *****/

/**
 * @brief   max17261_write_reg. Generic function to read MAX17261 registers.
 * @param[out]  dev_addr. The 1-byte slave address of MAX17261.
 * @param[out]  reg_addr. The 1-byte address of the register on the I2C slave to start writing to.
 * @param[in]  *reg_data. Array of uint8_t data to write to the I2C slave.
 * @param[out]  len. Number of uint16_t registers to write.
 ****************************************************************************/

void max17261_write_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);


/**
 * @brief   max17261_read_reg. Generic function to read max17261 registers.
 * @param[out]  dev_addr. The 1-byte slave address of energy harvester.
 * @param[out]  reg_addr. The 1-byte address of the register on the I2C slave to start reading from.
 * @param[in]  *reg_data. Array of uint8_t data to read from the I2C slave.
 * @param[out]  len. Number of uint16_t registers to read.
 ****************************************************************************/
void max17261_read_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);

/**
 * @brief   max17261_write_verify_reg. Generic function to read max17261 registers.
 * @param[out]  dev_addr. The 1-byte slave address of energy harvester.
 * @param[out]  reg_addr. The 1-byte address of the register on the I2C slave to start reading from.
 * @param[in]  *reg_data. Array of uint8_t data to read from the I2C slave.
 * @param[out]  num_of_byts. Number of bytes to read.
 * @return      true on success, false on failure
 ****************************************************************************/
bool max17261_write_verify_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t num_of_bytes);


/**
 * @brief    max17261_i2c_test(). Function to check that MAX17261 is present and
 * responding on I2C bus.
 * @return         true on success, false on failure
 ****************************************************************************/
int max17261_i2c_test(void);

/**
 * @brief    max17261_soft_reset(). Function to "soft reset" the MAX17261 device completely.
 * @return         true on success, false on failure
 ****************************************************************************/
void max17261_soft_reset(void);

/**
 * @brief    max17261_por_detected(). Function to check if a power-on-reset
 * has occurred with the max17261 device.
 * @return         true on success, false on failure
 ****************************************************************************/
bool max17261_por_detected(void);

/**
 * @brief    max17261_clear_por_bit(). Function to clear a power-on-reset flag that
 * has occurred on the max17261 device.
 ****************************************************************************/
void max17261_clear_por_bit(void);

/**
 * @brief    max17261_wait_dnr(). Function to wait for the MAX1726x to complete
 * its startup operations. See "MAX1726x Software Implementation Guide" for details
 ****************************************************************************/
void max17261_wait_dnr(void);



/**
 * @brief    max17261_read_repsoc(). This function gets the state of charge (SOC) as
 * described in Software Implementation guide for the MAX17261. It gives how much
 * charge is left on the battery.
 * @return the "HiByte" of RepSOC register for %1 resolution
 ****************************************************************************/
uint8_t max17261_read_repsoc(void);

/**
 * @brief    max17261_read_repcap(). This function gets the RepCap register data
 * as described in Software Implementation guide. It gives how many milli amp
 * hours is left on the battery.
 * @return the 2-byte register data as a uint16_t.
 ****************************************************************************/
uint16_t max17261_read_repcap(void);
/**
 * @brief    max17261_read_tte(). This function the TTE register data
 * as described in Software Implementation guide. It gives how much milli amp
 * hours is left on the battery. This function is currently not used or tested.
 * @param[out]  *hrs. Hours
 * @param[out]  *mins. Minutes
 * @param[out]  *secs. Seconds
 ****************************************************************************/
void max17261_read_tte(uint8_t *hrs, uint8_t *mins, uint8_t *secs);



/**
 * @brief    max17261_config_ez(). This function performs EZ configuraiton
 * as described in Software Implementation guide.
 * @return void
 ****************************************************************************/
void max17261_config_ez(void);

/**
 * @brief    Fuel_gauge_data_collect. This function reads the fuel gauge registers and outputs to OutputString Variable
 * @param[in] unsigned long timer_count_value. This is a time stamp in seconds.
 * @param[out]  char * OutputString.  This is the output string of fuel gauge readings.
 *
 ****************************************************************************/
void Fuel_gauge_data_collect(unsigned long timer_count_value, char * OutputString);

#endif /* MAX17261_H_ */
