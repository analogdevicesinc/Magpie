	/*
	* MAX17261.c
	*
	*  Created on: Jul 2, 2024
	*      Author: VKarra
	*/

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
	//#include "dev_i2c.c"
	#include "max17261.h"
	#include "stdbool.h"
	#include "math.h"
	#include "string.h"

	/***** Globals *****/


	volatile unsigned long mode_tick_count = 0;

	typedef enum { FAILED, PASSED } test_t;



	// MAX17261 specific globals
	uint8_t max17261_regs[256]; // represents all the max17261 registers.


	// General firmware globals
	int elapsed_time_multiplier = 1;

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

	MAX17261_Error_t max17261_write_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
	{
		uint16_t i;
		uint8_t i2c_data[256];

		for(i=0; i<len; i++)
		{
			i2c_data[i] = *(reg_data+i);
		}

		if((MAX32665_I2C_Write(dev_addr, reg_addr, i2c_data, len)) == E_NO_ERROR)
		{
			return MAX17261_ERROR_ALL_OK;
		}
	}


	/**
	 * @brief   max17261_read_reg. Generic function to read max17261 registers.
	 * @param[out]  dev_addr. The 1-byte slave address of energy harvester.
	 * @param[out]  reg_addr. The 1-byte address of the register on the I2C slave to start reading from.
	 * @param[in]  *reg_data. Array of uint8_t data to read from the I2C slave.
	 * @param[out]  num_of_reg. Number of uint16_t registers to read.
	 ****************************************************************************/
	MAX17261_Error_t max17261_read_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
	{

		if((MAX32665_I2C_Read(dev_addr, reg_addr, reg_data,len))== E_SUCCESS)
		{
			return MAX17261_ERROR_ALL_OK;
			
		}
		
	}

	/**
	 * @brief   max17261_write_verify_reg. Generic function to read max17261 registers.
	 * @param[out]  dev_addr. The 1-byte slave address of energy harvester.
	 * @param[out]  reg_addr. The 1-byte address of the register on the I2C slave to start reading from.
	 * @param[in]  *reg_data. Array of uint8_t data to read from the I2C slave.
	 * @param[out]  num_of_byts. Number of bytes to read.
	 * @return      true on success, false on failure
	 ****************************************************************************/
	bool max17261_write_verify_reg(uint8_t dev_addr, uint8_t reg_addr, uint8_t *reg_data, uint16_t num_of_bytes)
	{
		bool is_verified = false;
		int i = 0;
		uint8_t read_data[256];
		while (!is_verified)
		{
			max17261_write_reg(dev_addr, reg_addr, reg_data, num_of_bytes);
			MXC_Delay(3000);// delay 3ms with timer 1
			max17261_read_reg(dev_addr, reg_addr, &read_data[0], num_of_bytes);
			printf("write_and_verify reg_data = ");
			for (i = 0; i < num_of_bytes; i++)
			{
				printf("%02X",*(reg_data + i));
				if(read_data[i] != *(reg_data+i))
				{
					is_verified = false;
					return is_verified;
				}
			}
			printf("\n");
			is_verified = true;
		}
		return is_verified;
	}

	/**
	 * @brief    max17261_i2c_test(). Function to check that max17261 is present and
	 * responding on I2C bus.
	 * @return         true on success, false on failure
	 ****************************************************************************/
	MAX17261_Error_t max17261_i2c_test(void)
	{
		uint8_t address;
		printf("\n******** This example finds if MAX17261 is connected to I2C line *********\r\n");
		int error;

		//Setup the I2CM
		error = MXC_I2C_Init(I2C_MASTER, 1, 0);
		if (error != MAX17261_ERROR_ALL_OK)
		{
			printf("-->Failed master\r\n");
			return MAX17261_ERROR_FG_ERROR;
		}
		else
		{
			printf("\n-->I2C Master Initialization Complete\r\n");
		}

		printf("-->Scanning started\r\n");
		MXC_I2C_SetFrequency(I2C_MASTER, I2C_FREQ);

		mxc_i2c_req_t reqMaster;
		reqMaster.i2c = I2C_MASTER;
		reqMaster.addr = MAX17261_I2C_ADDR;
		reqMaster.tx_buf = NULL;
		reqMaster.tx_len = 0;
		reqMaster.rx_buf = NULL;
		reqMaster.rx_len = 0;
		reqMaster.restart = 0;
		reqMaster.callback = NULL;

		if ((MXC_I2C_MasterTransaction(&reqMaster)) == MAX17261_ERROR_ALL_OK)
		{
			address = MAX17261_I2C_ADDR;
			printf("\nFound slave ID %03d; 0x%02X\r\n", address, address);

		}
		else
		{
            printf("-->No slave device found\r\n");
			return MAX17261_ERROR_FG_ERROR;
		}
		MXC_Delay(MXC_DELAY_MSEC(200));
		return MAX17261_ERROR_ALL_OK;
	}




	/**
	 * @brief    max17261_soft_reset(). Function to "soft reset" the MAX17261 device completely.
	 * @return         true on success, false on failure
	 ****************************************************************************/
	MAX17261_Error_t max17261_soft_reset(void)
	{
		//The procedure for a MAX17261 reset is below:
		//Write 0x000F to 0x60.
        int result;
		max17261_regs[0] = 0x0F;
		max17261_regs[1] = 0x00;
		uint8_t message[] = {max17261_regs[0],max17261_regs[1]};
		result = max17261_write_reg(MAX17261_I2C_ADDR,Soft_Reset_addr,&message, 2);
		if(result == MAX17261_ERROR_ALL_OK)
		{
			return MAX17261_ERROR_ALL_OK;
			printf("Soft reset succesful\r\n");
			
		}
		else
		{
			printf("Soft reset failed\r\n");
			return MAX17261_ERROR_FG_ERROR;
		
		}
		
	}

	/**
	 * @brief    max17261_por_detected(). Function to check if a power-on-reset
	 * has occurred with the max17261 device.
	 * @return         true on success, false on failure
	 ***************************************************************************/
	bool max17261_por_detected(void)
	{
		uint16_t status_register = 0;
		max17261_read_reg(MAX17261_I2C_ADDR,Status_addr, &max17261_regs[0x00],2); // read status register
		status_register = (max17261_regs[1]<<8) + max17261_regs[0]; // make a 16-bit integer representing status register
		printf("status_register = %04x\n",status_register);
		if ((status_register & 0x0002) > 0) // POR bit is in the 2nd position (bit 1 position of Status register)
		{
			printf("MAX17261 POR detected\n");
			return true;
		}
		printf("MAX17261 POR not detected\n");
		return false;
	}

	/**
	 * @brief    max17261_clear_por_bit(). Function to clear a power-on-reset flag that
	 * has occurred on the MAX17261 device.
	 ***************************************************************************/
	void max17261_clear_por_bit(void)
	{
		max17261_read_reg(MAX17261_I2C_ADDR, Status_addr, &max17261_regs[0x00], 2); // read POR bit again
		max17261_regs[0] = max17261_regs[0] & 0xFD; // LSB -- Set POR bit to 0. Bit position 1
		max17261_write_reg(MAX17261_I2C_ADDR,Status_addr,&max17261_regs[0x00], 2); // clear POR bit
	}

	/*
	* @brief    max17261_wait_dnr(). Function to wait for the MAX1726x to complete
	* its startup operations. See "MAX1726x Software Implementation Guide" for details
	***************************************************************************/
	void max17261_wait_dnr(void)
	{
		uint16_t FStat_register = 0;
		//max17261_write_reg(MAX17261_I2C_ADDR,FStat_addr,&max17261_regs[0x00], 2);
		max17261_read_reg(MAX17261_I2C_ADDR,FStat_addr,&max17261_regs[0x00], 2);
		FStat_register = (max17261_regs[1]<<8) + max17261_regs[0];

		while((FStat_register & 0x0001) == 0x0001)
		{

			MXC_Delay(11000); //11 ms delay
			//max17261_write_reg(MAX17261_I2C_ADDR,FStat_addr,&max17261_regs[0x00], 2);
			max17261_read_reg(MAX17261_I2C_ADDR,FStat_addr,&max17261_regs[0x00], 2);
			FStat_register = (max17261_regs[1]<<8) + max17261_regs[0];
		}
	}


	/**
	 * @brief    max17261_config_ez(). Function to initialize configuration of the
	 * max17261 with OPTION 1 EZ Config. See "MAX1726x Software Implementation Guide" for details
	 ****************************************************************************/
	void max17261_config_ez(void)
	{
		uint16_t tempdata;
		/// Store original HibCFG value, read in HibCfg; prepare to load model
		uint8_t hibcfg_reg[2];
		max17261_read_reg(MAX17261_I2C_ADDR,HibCFG_addr, &hibcfg_reg[0x00],2); // read hibcfg register

		/// Exit Hibernate Mode step
		max17261_regs[0] = 0x90;
		max17261_regs[1] = 0x00;
		max17261_write_reg(MAX17261_I2C_ADDR,Soft_Reset_addr,&max17261_regs[0x00], 2);//Soft wakeup (Step 1)
		max17261_regs[0] = 0x00;
		max17261_write_reg(MAX17261_I2C_ADDR,HibCFG_addr,&max17261_regs[0x00], 2);//Exit hibernate mode Step 2
		max17261_write_reg(MAX17261_I2C_ADDR,Soft_Reset_addr,&max17261_regs[0x00], 2);// Exit hibernate mode Step 3

		/// OPTION 1 EZ Config (No INI file is needed)

		// load DesignCap
		tempdata = DesignCap;
		max17261_regs[0] = tempdata & 0x00FF;
		max17261_regs[1] = tempdata >> 8;
		max17261_write_reg(MAX17261_I2C_ADDR,DesignCap_addr,&max17261_regs[0x00], 2);

		// load IChgTerm
		tempdata = IChgTerm;
		max17261_regs[0] = tempdata & 0x00FF;
		max17261_regs[1] = tempdata >> 8;
		max17261_write_reg(MAX17261_I2C_ADDR,IChgTerm_addr,&max17261_regs[0x00], 2);

		// load VEmpty
		tempdata = VEmpty;
		max17261_regs[0] = tempdata & 0x00FF;
		max17261_regs[1] = tempdata >> 8;
		max17261_write_reg(MAX17261_I2C_ADDR,VEmpty_addr,&max17261_regs[0x00], 2);

		// load ModelCfg
		tempdata = ModelCfg;
		max17261_regs[0] = tempdata & 0x00FF;
		max17261_regs[1] = tempdata >> 8;
		max17261_write_reg(MAX17261_I2C_ADDR,ModelCfg_addr,&max17261_regs[0x00], 2);

		// Poll ModelCFG.Refresh bit, do not continue until ModelCFG.Refresh==0
		max17261_read_reg(MAX17261_I2C_ADDR,ModelCfg_addr, &max17261_regs[0x00],2); // read status register
		tempdata = (max17261_regs[1]<<8) + max17261_regs[0];

		while((tempdata & 0x8000) == 0x8000)
		{
			MXC_Delay(11000); //11 ms delay // delay 11ms
			max17261_read_reg(MAX17261_I2C_ADDR,ModelCfg_addr, &max17261_regs[0x00],2); // read ModelCfg register
			tempdata = (max17261_regs[1]<<8) + max17261_regs[0];
		}

		// Restore Original HibCFG value
		max17261_write_reg(MAX17261_I2C_ADDR,HibCFG_addr,&hibcfg_reg[0x00], 2);
	}



	/*
	* @brief    max17261_read_repsoc(). This function gets the state of charge (SOC) as
	* described in Software Implementation guide for the MAX17261. It gives how much
	* charge is left on the battery.
	* @return the "HiByte" of RepSOC register for %1 resolution
	***************************************************************************/
	uint8_t max17261_read_repsoc(void)
	{
		max17261_read_reg(MAX17261_I2C_ADDR,REPSOC_addr,&max17261_regs[0x00],2); // Read RepSOC
		return (max17261_regs[1]); // The RepSOC "HiByte" can be directly displayed to the user for 1% resolution.
	}

	/*
	* @brief    max17261_read_repcap(). This function gets the RepCap register data
	* as described in Software Implementation guide. It gives how many milli amp
	* hours is left on the battery.
	* @return the 2-byte register data as a uint16_t.
	***************************************************************************/
	uint16_t max17261_read_repcap(void)
	{
		uint16_t tempdata = 0;
		max17261_read_reg(MAX17261_I2C_ADDR,0x05,&max17261_regs[0x00],2); // Read RepCap
		tempdata = (max17261_regs[1]<<8) + max17261_regs[0];
		return (tempdata);
	}

	/*
	* @brief    max17261_read_tte(). This function the TTE register data
	* as described in Software Implementation guide. It gives how much milli amp
	* hours is left on the battery. This function is currently not used or tested.
	* @param[out]  *hrs. Hours
	* @param[out]  *mins. Minutes
	* @param[out]  *secs. Seconds
	***************************************************************************/
	void max17261_read_tte(uint8_t *hrs, uint8_t *mins, uint8_t *secs)
	{
		// This function gets the "Time to Empty" (TTE) value. TTE is in memory location 0x11.
		// The LSB of the TTE register is 5.625 seconds.
		uint16_t tte_register = 0;

		double tte_hrs = 0;  // Bits 15:10 unit = 1.6 hours
		double tte_mins = 0; // Bits 9:4 unit = 1.5 minutes
		double tte_secs = 0; // Bits 3:0 unit = 5.625 seconds

		double secs_from_hrs = 0.0;
		double secs_from_mins = 0.0;
		double secs_from_secs = 0.0;
		double total_seconds = 0.0;

		max17261_read_reg(MAX17261_I2C_ADDR,TTE_addr,&max17261_regs[0x00],2); // Read Time To Empty
		tte_register = (((uint16_t) max17261_regs[0x01]) << 8) + max17261_regs[0x00];

		tte_hrs = (double) (max17261_regs[1] >> 2) * (double) 1.6;
		secs_from_hrs = tte_hrs * 3600;
		tte_mins = (double) (tte_register & 0x03F0) * (double) 1.5;
		secs_from_mins = tte_mins * 60;
		tte_secs = (double) (max17261_regs[0] & 0x0F) * (double) 5.625;
		secs_from_secs = tte_secs;
		total_seconds = round (secs_from_hrs + secs_from_mins + secs_from_secs); // add all seconds up

		*hrs = (uint8_t) (total_seconds/3600);
		*mins = (uint8_t) ((total_seconds - (3600 * (*hrs)))/60);
		*secs = (uint8_t) (total_seconds - (3600 * (*hrs)) - ((*mins)*60));
	}

/*

	
	 * @brief    Fuel_gauge_data_collect. This function reads the fuel gauge registers and outputs to OutputString Variable
	 * @param[in] None
	 * @param[out]  char * OutputString.  This is the output string of fuel gauge readings.
	 *

	void Fuel_gauge_data_collect()
	{
		// local variables
		uint16_t tempdata = 0;
		int16_t stempdata = 0;
		char tempstring[20] = {0};
		double timerh = 0;  // timerh + timer = time since last por
		double timer = 0;

		// Read repsoc
		tempdata = 0;
		memset(tempstring,0,20);
		tempdata = max17261_read_repsoc();
		printf("State Of Charge = %i\r\n",tempdata);


		// Read VCell
		tempdata = 0;
		memset(tempstring,0,20);
		max17261_read_reg(MAX17261_I2C_ADDR,VCell_addr,&max17261_regs[0x00],2); // Read register
		tempdata = (max17261_regs[1]<<8) + max17261_regs[0];
		printf("voltage per cell of the batterypack = 0x%04x\r\n",tempdata);
		

		// Read AvgVCell
		tempdata = 0;
		memset(tempstring,0,20);
		max17261_read_reg(MAX17261_I2C_ADDR,AvgVCell_addr,&max17261_regs[0x00],2); // Read register
		tempdata = (max17261_regs[1]<<8) + max17261_regs[0];
		printf("AVerage of VCell register readings = %lf\r\n",(double)((1.25*tempdata/16)/1000));


		tempdata = 0;
		memset(tempstring,0,20);
		max17261_read_reg(MAX17261_I2C_ADDR,Temp_addr,&max17261_regs[0x00],2); // Read register
		tempdata = (max17261_regs[1]<<8) + max17261_regs[0]; // put register into unsigned int
		stempdata = (int16_t) tempdata; // this is going to be signed number, so convert to 16-bit int
		// to calculate temperature, see Table 2 on page 16 of max17261 data sheet
		printf("Temp register = %f\r\n",(double)stempdata/256);


		// Read AvgTA (temperature in degrees C)
		tempdata = 0;
		memset(tempstring,0,20);
		max17261_read_reg(MAX17261_I2C_ADDR,AvgTA_addr,&max17261_regs[0x00],2); // Read register
		tempdata = (max17261_regs[1]<<8) + max17261_regs[0];
		stempdata = (int16_t) tempdata; // this is going to be signed number, so convert to 16-bit int
		printf("AvgTA register = %f\r\n",(double)stempdata/256);


		// Read Current (in mA)
		tempdata = 0;
		memset(tempstring,0,20);
		max17261_read_reg(MAX17261_I2C_ADDR,Current_addr,&max17261_regs[0x00],2); // Read register
		tempdata = (max17261_regs[1]<<8) + max17261_regs[0];
		stempdata = (int16_t) tempdata; // this is going to be signed number, so convert to 16-bit int
		printf("Current register = %f\r\n",(double)stempdata*(double)156.25/1000);


		// Read AvgCurrent
		tempdata = 0;
		memset(tempstring,0,20);
		max17261_read_reg(MAX17261_I2C_ADDR,AvgCurrent_addr,&max17261_regs[0x00],2); // Read register
		tempdata = (max17261_regs[1]<<8) + max17261_regs[0];
		stempdata = (int16_t) tempdata; // this is going to be signed number, so convert to 16-bit int
		printf("AvgCurrent register = %f\r\n",(double)stempdata*(double)156.25/1000);


		// Read TTF
		tempdata = 0;
		memset(tempstring,0,20);
		max17261_read_reg(MAX17261_I2C_ADDR,TTF_addr,&max17261_regs[0x00],2); // Read register
		tempdata = (max17261_regs[1]<<8) + max17261_regs[0];
		stempdata = (int16_t) tempdata; // this is going to be signed number, so convert to 16-bit int
		printf("TTF register = %i\r\n",stempdata);
		

		// Read RepCap
		tempdata = 0;
		memset(tempstring,0,20);
		max17261_read_reg(MAX17261_I2C_ADDR,RepCap_addr,&max17261_regs[0x00],2); // Read register
		tempdata = (max17261_regs[1]<<8) + max17261_regs[0];
		printf("RepCap register = 0x%04x\r\n",tempdata);
		stempdata = (int16_t) tempdata; // this is going to be signed number, so convert to 16-bit int
		printf("RepCap register = %f\r\n",(double)stempdata/2);


		// Read VFRemCap
		tempdata = 0;
		memset(tempstring,0,20);
		max17261_read_reg(MAX17261_I2C_ADDR,VFRemCap_addr,&max17261_regs[0x00],2); // Read register
		tempdata = (max17261_regs[1]<<8) + max17261_regs[0];
		stempdata = (int16_t) tempdata; // this is going to be signed number, so convert to 16-bit int
		printf("VFRemCap register = %f\r\n",(double)stempdata/2);


		// Read MixCap
		tempdata = 0;
		memset(tempstring,0,20);
		max17261_read_reg(MAX17261_I2C_ADDR,MixCap_addr,&max17261_regs[0x00],2); // Read register
		tempdata = (max17261_regs[1]<<8) + max17261_regs[0];
		stempdata = (int16_t) tempdata; // this is going to be signed number, so convert to 16-bit int
		printf("MixCap register = %f\r\n",(double)stempdata*(double)0.5);


		// Read FullCapRep
		tempdata = 0;
		memset(tempstring,0,20);
		max17261_read_reg(MAX17261_I2C_ADDR,FullCapRep_addr,&max17261_regs[0x00],2); // Read register
		tempdata = (max17261_regs[1]<<8) + max17261_regs[0];
		stempdata = (int16_t) tempdata; // this is going to be signed number, so convert to 16-bit int
		printf("FullCapRep register = %f\r\n",(double)stempdata*(double)0.5);


		// Read FullCapNom
		tempdata = 0;
		memset(tempstring,0,20);
		max17261_read_reg(MAX17261_I2C_ADDR,FullCapNom_addr,&max17261_regs[0x00],2); // Read register
		tempdata = (max17261_regs[1]<<8) + max17261_regs[0];
		stempdata = (int16_t) tempdata; // this is going to be signed number, so convert to 16-bit int
		printf("FullCapNom register = %f\r\n",(double)stempdata*(double)0.5);


		// Read QResidual
		tempdata = 0;
		memset(tempstring,0,20);
		max17261_read_reg(MAX17261_I2C_ADDR,QResidual_addr,&max17261_regs[0x00],2); // Read register
		tempdata = (max17261_regs[1]<<8) + max17261_regs[0];
		stempdata = (int16_t) tempdata; // this is going to be signed number, so convert to 16-bit int
		printf("QResidual register = %f\r\n",(double)stempdata*(double)0.5);


		// Read REPSOC
		tempdata = 0;
		memset(tempstring,0,20);
		max17261_read_reg(MAX17261_I2C_ADDR,REPSOC_addr,&max17261_regs[0x00],2); // Read register
		tempdata = (max17261_regs[1]<<8) + max17261_regs[0];
		printf("REPSOC register = %f\r\n",(double)tempdata/256);


		// Read AvSOC
		tempdata = 0;
		memset(tempstring,0,20);
		max17261_read_reg(MAX17261_I2C_ADDR,AvSOC_addr,&max17261_regs[0x00],2); // Read register
		tempdata = (max17261_regs[1]<<8) + max17261_regs[0];
		printf("AvSOC register = %f\r\n",(double)tempdata/256);


		// Read Timer
		tempdata = 0;
		memset(tempstring,0,20);
		max17261_read_reg(MAX17261_I2C_ADDR,Timer_addr,&max17261_regs[0x00],2); // Read register
		tempdata = (max17261_regs[1]<<8) + max17261_regs[0];
		timer = (double)tempdata * (double)175.8/3600000; // see User Guide page 34 for Timer register
		printf("Timer register (hours) = %f\r\n",timer);

		// Read TimerH
		tempdata = 0;
		memset(tempstring,0,20);
		max17261_read_reg(MAX17261_I2C_ADDR,TimerH_addr,&max17261_regs[0x00],2); // Read register
		tempdata = (max17261_regs[1]<<8) + max17261_regs[0];
		timerh = (double)tempdata * (double)3.2; // see User Guide page 34 for TimerH register calculation in hours
		printf("TimerH register (hours) = %f\r\n",timerh); // see page User Guide page 34


		// Read QH
		tempdata = 0;
		memset(tempstring,0,20);
		max17261_read_reg(MAX17261_I2C_ADDR,QH_addr,&max17261_regs[0x00],2); // Read register
		tempdata = (max17261_regs[1]<<8) + max17261_regs[0];
		printf("QH register(coulomb count) = %f\r\n",(double)tempdata * (double)0.25/3600000);// We have 20mOhm sense resistor so the LSB is 0.25


		// Read AvCap
		tempdata = 0;
		memset(tempstring,0,20);
		max17261_read_reg(MAX17261_I2C_ADDR,AvCap_addr,&max17261_regs[0x00],2); // Read register
		tempdata = (max17261_regs[1]<<8) + max17261_regs[0];
		printf("AvCap register = %f\r\n",(double)tempdata * (double)0.5);


		// Read MixSOC
		tempdata = 0;
		memset(tempstring,0,20);
		max17261_read_reg(MAX17261_I2C_ADDR,MixSOC_addr,&max17261_regs[0x00],2); // Read register
		tempdata = (max17261_regs[1]<<8) + max17261_regs[0];
		printf("MixSOC register = %f\r\n",(double)tempdata/256);


		memset(tempstring,0,20);
		tempdata = max17261_i2c_test();
		printf("max17261 I2C Test = %i\r\n",tempdata);
		if (tempdata < E_NO_ERROR)
		{
			printf("max17261 I2C Test = %i\r\n",tempdata);

		}
	}

*/

