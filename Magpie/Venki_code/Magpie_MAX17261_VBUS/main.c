/******************************************************************************
 * Copyright (C) 2023 Maxim Integrated Products, Inc., All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the>
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of Maxim Integrated
 * Products, Inc. shall not be used except as stated in the Maxim Integrated
 * Products, Inc. Branding Policy.
 *
 * The mere transfer of this software does not imply any licenses
 * of trade secrets, proprietary technology, copyrights, patents,
 * trademarks, maskwork rights, or any other form of intellectual
 * property whatsoever. Maxim Integrated Products, Inc. retains all
 * ownership rights.
 *
 ******************************************************************************/

/**
 * @file    main.c
 * @brief   Magpie_MAX17261_VBUS
 * @details .
 */

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "led.h"
#include "pb.h"
#include "mxc_delay.h"
#include "usb.h"
#include "usb_event.h"
#include "enumerate.h"
#include "cdc_acm.h"
#include "descriptors.h"
#include "gcr_regs.h"
#include "mxc_sys.h"
#include "board.h"
#include "spi.h"
#include <stdarg.h>
#include "nvic_table.h"
#include "gpio.h"
#include "mxc_device.h"
#include "dev_gpio_config.h" //SwiftV2 specific GPIO configuration
#include "tmr.h"
#include "MAX17261.h"

/**************************************************************************************************************
VBUS USB Function seciton
 ***************************************************************************************************************/

/***** Definitions *****/
// USB_CDC_Protocol definitions
#define EVENT_ENUM_COMP MAXUSB_NUM_EVENTS
#define EVENT_REMOTE_WAKE (EVENT_ENUM_COMP + 1)
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

/***** Global Data *****/
/****************************/
// USB_Data
volatile int configured;
volatile int suspended;
volatile unsigned int event_flags;
int remote_wake_en;
/****************************/

/*******USB function prototypes **/
int usbStartupCallback();
int usbShutdownCallback();
static void usbAppSleep(void);
static void usbAppWakeup(void);
static int setconfigCallback(MXC_USB_SetupPkt *sud, void *cbdata);
static int setfeatureCallback(MXC_USB_SetupPkt *sud, void *cbdata);
static int clrfeatureCallback(MXC_USB_SetupPkt *sud, void *cbdata);
static int eventCallback(maxusb_event_t evt, void *data);
static int usbReadCallback(void);
static volatile int usb_read_complete;

/***** File Scope Variables *****/
/* This EP assignment must match the Configuration Descriptor */
static acm_cfg_t acm_cfg = {
	1,					  /* EP OUT */
	MXC_USBHS_MAX_PACKET, /* OUT max packet size */
	2,					  /* EP IN */
	MXC_USBHS_MAX_PACKET, /* IN max packet size */
	3,					  /* EP Notify */
	MXC_USBHS_MAX_PACKET, /* Notify max packet size */
};

/* User-supplied function to delay usec micro-seconds */
void delay_us(unsigned int usec)
{
	/* mxc_delay() takes unsigned long, so can't use it directly */
	MXC_Delay(usec);
}

/*****************************************************************/
// Custom functions design for printf using acm_write and acm_read
/*****************************************************************/

/***Custom printf USB write design***/
void debugPrint(char *fmt, ...)
{
	char buffer[1024];
	uint16_t len = 0;
	va_list args;
	va_start(args, fmt);
	len = vsnprintf(buffer, 1024, fmt, args);
	va_end(args);

	if (len < 0)
		return;

	acm_write((uint8_t *)buffer, len);
}

/***Custom USB read function***/
int debugRead(const char *fmt, ...)
{
	uint8_t buffer[256]; // Adjust the buffer size

	int canRead = 0;
	int readLen = 0;
	int num = 0;
	uint16_t inx = 0;

	while (1)
	{
		canRead = acm_canread();
		readLen = acm_read(&buffer[inx], canRead); // read a byte.
		acm_write(&buffer[inx], readLen);		   // echo the byte.
		if (buffer[inx] == '\r')
		{ // check for end of line.
			buffer[inx] = '\n';
			num += readLen;
			inx += readLen;
			break;
		}
		else
		{
			num += readLen;
			inx += readLen;
		}
	}

	va_list args;
	va_start(args, fmt);

	int itemsRead = vsscanf((const char *)buffer, fmt, args);

	va_end(args);

	return itemsRead;
}

/*****************************************************************************
MAX17261 Globals
******************************************************************************/
uint8_t counter = 0;
volatile unsigned long timer_count = 0;
int error, tempdata1;
int retvalue = 0;
char log_string[256] = {0}; // Used to print the paramter values on terminal
// MAX17261 specific globals
uint8_t max17261_regs1[256]; // represents all the max17261 registers.
unsigned long timer_count_value;
char *OutputString;

/**
 * @brief    Fuel_gauge_data_collect. This function reads the fuel gauge registers and outputs to OutputString Variable
 * @param[in] None
 * @param[out]  char * OutputString.  This is the output string of fuel gauge readings.
 *
 ****************************************************************************/
void Fuel_gauge_data_collect()
{
	// local variables
	uint16_t tempdata = 0;
	int16_t stempdata = 0;
	char tempstring[20] = {0};
	double timerh = 0; // timerh + timer = time since last por
	double timer = 0;

	// Read repsoc
	tempdata = 0;
	memset(tempstring, 0, 20);
	tempdata = max17261_read_repsoc();
	debugPrint("State Of Charge = %i\r\n", tempdata);
	// printf("State Of Charge = %i\r\n", tempdata);

	// Read VCell
	tempdata = 0;
	memset(tempstring, 0, 20);
	max17261_read_reg(MAX17261_I2C_ADDR, VCell_addr, &max17261_regs1[0x00], 2); // Read register
	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
	tempdata = (double)((1.25 * tempdata / 16) / 1000);
	debugPrint("voltage per cell of the batterypack = %lf\r\n", tempdata);
	// printf("voltage per cell of the batterypack = %lf\r\n", tempdata);

	// Read AvgVCell
	tempdata = 0;
	memset(tempstring, 0, 20);
	max17261_read_reg(MAX17261_I2C_ADDR, AvgVCell_addr, &max17261_regs1[0x00], 2); // Read register
	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
	debugPrint("AVerage of VCell register readings = %lf\r\n", (double)((1.25 * tempdata / 16) / 1000));
    // printf("AVerage of VCell register readings = %lf\r\n", (double)((1.25 * tempdata / 16) / 1000));

	tempdata = 0;
	memset(tempstring, 0, 20);
	max17261_read_reg(MAX17261_I2C_ADDR, Temp_addr, &max17261_regs1[0x00], 2); // Read register
	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];				   // put register into unsigned int
	stempdata = (int16_t)tempdata;											   // this is going to be signed number, so convert to 16-bit int
	// to calculate temperature, see Table 2 on page 16 of max17261 data sheet
	debugPrint("Temp register = %f\r\n",(double)stempdata/256);

	// Read AvgTA (temperature in degrees C)
	tempdata = 0;
	memset(tempstring, 0, 20);
	max17261_read_reg(MAX17261_I2C_ADDR, AvgTA_addr, &max17261_regs1[0x00], 2); // Read register
	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
	stempdata = (int16_t)tempdata; // this is going to be signed number, so convert to 16-bit int
	debugPrint("AvgTA register = %f\r\n",(double)stempdata/256);

	// Read Current (in mA)
	tempdata = 0;
	memset(tempstring, 0, 20);
	max17261_read_reg(MAX17261_I2C_ADDR, Current_addr, &max17261_regs1[0x00], 2); // Read register
	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
	stempdata = (int16_t)tempdata; // this is going to be signed number, so convert to 16-bit int
	//debugPrint("Current register = %f\r\n", (double)stempdata * (double)156.25 / 1000);
	debugPrint("Current register = %f\r\n", (double)stempdata * (double)78.125 / 1000);
	// printf("Current register = %f\r\n", (double)stempdata * (double)156.25 / 1000);

	// Read AvgCurrent
	tempdata = 0;
	memset(tempstring, 0, 20);
	max17261_read_reg(MAX17261_I2C_ADDR, AvgCurrent_addr, &max17261_regs1[0x00], 2); // Read register
	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
	stempdata = (int16_t)tempdata; // this is going to be signed number, so convert to 16-bit int
	//debugPrint("AvgCurrent register = %f\r\n", (double)stempdata * (double)156.25 / 1000);
	// printf("AvgCurrent register = %f\r\n", (double)stempdata * (double)156.25 / 1000);
	debugPrint("AvgCurrent register = %f\r\n", (double)stempdata * (double)78.125 / 1000);

	// Read TTF
	tempdata = 0;
	memset(tempstring, 0, 20);
	max17261_read_reg(MAX17261_I2C_ADDR, TTF_addr, &max17261_regs1[0x00], 2); // Read register
	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
	stempdata = (int16_t)tempdata; // this is going to be signed number, so convert to 16-bit int
	debugPrint("TTF register = %i\r\n",stempdata);

	// Read RepCap
	tempdata = 0;
	memset(tempstring, 0, 20);
	max17261_read_reg(MAX17261_I2C_ADDR, RepCap_addr, &max17261_regs1[0x00], 2); // Read register
	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
	// debugPrint("RepCap register = 0x%04x\r\n",tempdata);
	stempdata = (int16_t)tempdata; // this is going to be signed number, so convert to 16-bit int
	// debugPrint("RepCap register = %f\r\n",(double)stempdata/2);

	// Read VFRemCap
	tempdata = 0;
	memset(tempstring, 0, 20);
	max17261_read_reg(MAX17261_I2C_ADDR, VFRemCap_addr, &max17261_regs1[0x00], 2); // Read register
	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
	stempdata = (int16_t)tempdata; // this is going to be signed number, so convert to 16-bit int
	// debugPrint("VFRemCap register = %f\r\n",(double)stempdata/2);

	// Read MixCap
	tempdata = 0;
	memset(tempstring, 0, 20);
	max17261_read_reg(MAX17261_I2C_ADDR, MixCap_addr, &max17261_regs1[0x00], 2); // Read register
	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
	stempdata = (int16_t)tempdata; // this is going to be signed number, so convert to 16-bit int
	// debugPrint("MixCap register = %f\r\n",(double)stempdata*(double)0.5);

	// Read FullCapRep
	tempdata = 0;
	memset(tempstring, 0, 20);
	max17261_read_reg(MAX17261_I2C_ADDR, FullCapRep_addr, &max17261_regs1[0x00], 2); // Read register
	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
	stempdata = (int16_t)tempdata; // this is going to be signed number, so convert to 16-bit int
	// debugPrint("FullCapRep register = %f\r\n",(double)stempdata*(double)0.5);

	// Read FullCapNom
	tempdata = 0;
	memset(tempstring, 0, 20);
	max17261_read_reg(MAX17261_I2C_ADDR, FullCapNom_addr, &max17261_regs1[0x00], 2); // Read register
	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
	stempdata = (int16_t)tempdata; // this is going to be signed number, so convert to 16-bit int
	// debugPrint("FullCapNom register = %f\r\n",(double)stempdata*(double)0.5);

	// Read QResidual
	tempdata = 0;
	memset(tempstring, 0, 20);
	max17261_read_reg(MAX17261_I2C_ADDR, QResidual_addr, &max17261_regs1[0x00], 2); // Read register
	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
	stempdata = (int16_t)tempdata; // this is going to be signed number, so convert to 16-bit int
	debugPrint("QResidual register = %f\r\n", (double)stempdata * (double)0.5);
	// printf("QResidual register = %f\r\n", (double)stempdata * (double)0.5);

	// Read REPSOC
	tempdata = 0;
	memset(tempstring, 0, 20);
	max17261_read_reg(MAX17261_I2C_ADDR, REPSOC_addr, &max17261_regs1[0x00], 2); // Read register
	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
	// debugPrint("REPSOC register = %f\r\n",(double)tempdata/256);

	// Read AvSOC
	tempdata = 0;
	memset(tempstring, 0, 20);
	max17261_read_reg(MAX17261_I2C_ADDR, AvSOC_addr, &max17261_regs1[0x00], 2); // Read register
	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
	// debugPrint("AvSOC register = %f\r\n",(double)tempdata/256);

	// Read Timer
	tempdata = 0;
	memset(tempstring, 0, 20);
	max17261_read_reg(MAX17261_I2C_ADDR, Timer_addr, &max17261_regs1[0x00], 2); // Read register
	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
	timer = (double)tempdata * (double)175.8 / 3600000; // see User Guide page 34 for Timer register
	// debugPrint("Timer register (hours) = %f\r\n",timer);

	// Read TimerH
	tempdata = 0;
	memset(tempstring, 0, 20);
	max17261_read_reg(MAX17261_I2C_ADDR, TimerH_addr, &max17261_regs1[0x00], 2); // Read register
	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
	timerh = (double)tempdata * (double)3.2; // see User Guide page 34 for TimerH register calculation in hours
	// debugPrint("TimerH register (hours) = %f\r\n",timerh); // see page User Guide page 34

	// Read QH
	tempdata = 0;
	memset(tempstring, 0, 20);
	max17261_read_reg(MAX17261_I2C_ADDR, QH_addr, &max17261_regs1[0x00], 2); // Read register
	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
	debugPrint("QH register(coulomb count) = %f\r\n", (double)tempdata * (double)0.25); // We have 20mOhm sense resistor so the LSB is 0.25
	// printf("QH register(coulomb count) = %f\r\n", (double)tempdata * (double)0.25 / 3600000); // We have 20mOhm sense resistor so the LSB is 0.25


	// Read AvCap
	tempdata = 0;
	memset(tempstring, 0, 20);
	max17261_read_reg(MAX17261_I2C_ADDR, AvCap_addr, &max17261_regs1[0x00], 2); // Read register
	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
	//	debugPrint("AvCap register = %f\r\n",(double)tempdata * (double)0.5);

	// Read MixSOC
	tempdata = 0;
	memset(tempstring, 0, 20);
	max17261_read_reg(MAX17261_I2C_ADDR, MixSOC_addr, &max17261_regs1[0x00], 2); // Read register
	tempdata = (max17261_regs1[1] << 8) + max17261_regs1[0];
	//	debugPrint("MixSOC register = %f\r\n",(double)tempdata/256);
}

/******************************************************************************
Your main code starts here
******************************************************************************/
int main(void)
{

	LDO_startup();

	//*****************************************************************************/
	//USB initializations
	//*****************************************************************************/
	printf("Starting USB initializations\r\n");
	maxusb_cfg_options_t usb_opts;
	/* Initialize state */
	configured = 0;
	suspended = 0;
	event_flags = 0;
	remote_wake_en = 0;

	/* Start out in full speed */
	usb_opts.enable_hs = 0;
	usb_opts.delay_us = delay_us; /* Function which will be used for delays */
	usb_opts.init_callback = usbStartupCallback;
	usb_opts.shutdown_callback = usbShutdownCallback;

	/* Initialize the usb module */
	if (MXC_USB_Init(&usb_opts) != 0)
	{
		printf("MXC_USB_Init() failed\n");

		while (1)
		{

		}
	}

	/* Initialize the enumeration module */
	if (enum_init() != 0)
	{
		printf("enum_init() failed\n");

		while (1)
		{

		}
	}

	/* Register enumeration data */
	enum_register_descriptor(ENUM_DESC_DEVICE, (uint8_t *)&device_descriptor, 0);
	enum_register_descriptor(ENUM_DESC_CONFIG, (uint8_t *)&config_descriptor, 0);
	enum_register_descriptor(ENUM_DESC_STRING, lang_id_desc, 0);
	enum_register_descriptor(ENUM_DESC_STRING, mfg_id_desc, 1);
	enum_register_descriptor(ENUM_DESC_STRING, prod_id_desc, 2);

	/* Handle configuration */
	enum_register_callback(ENUM_SETCONFIG, setconfigCallback, NULL);

	/* Handle feature set/clear */
	enum_register_callback(ENUM_SETFEATURE, setfeatureCallback, NULL);
	enum_register_callback(ENUM_CLRFEATURE, clrfeatureCallback, NULL);

	/* Initialize the class driver */
	if (acm_init(&config_descriptor.comm_interface_descriptor) != 0)
	{
		printf("acm_init() failed\n");

		while (1)
		{

		}
	}

	/* Register callbacks */
	MXC_USB_EventEnable(MAXUSB_EVENT_NOVBUS, eventCallback, NULL);
	MXC_USB_EventEnable(MAXUSB_EVENT_VBUS, eventCallback, NULL);
	acm_register_callback(ACM_CB_READ_READY, usbReadCallback);
	usb_read_complete = 0;

	/* Start with USB in low power mode */
	usbAppSleep();
	NVIC_EnableIRQ(USB_IRQn);

	//*****************************************************************************/
	//MAX17261 Code
	//*****************************************************************************/
	//check if slave is responding on I2C
	max17261_i2c_test();
	MXC_Delay(2000);
	
	max17261_soft_reset();		   // perform soft reset
	MXC_Delay(250);
	if (max17261_por_detected())
	{
		// load max17261 configuration
		printf("POR detected. Load fuel gauge config\r\n");
	    max17261_wait_dnr();
		max17261_config_ez();
		max17261_clear_por_bit();
	}
	else
	{
		printf("POR not detected\r\n");
	}
	// debugPrint("MAX17261 driver \r\n");
	printf("MAX17261 driver \r\n");	

	// main spin loop.  Please note that the Learn and Save Parameters section (step 3.5 and
	// 3.6 of the max17261 Software Implementation Guide) are not implemented in this solution.
	// If many power-on-resets are expected occur, it is a good idea to implement.
	int count = 0;
	while (1)
	{

		
		
		debugPrint("/================================/\r\n");
		// printf("/================================/\r\n");
		debugPrint("Data poll count is %d\r\n", count);
		//printf("Data poll count is %d\r\n", count);
		debugPrint("/================================/\r\n");
		// printf("/================================/\r\n");
		Fuel_gauge_data_collect();
		debugPrint("\r\n");
		// printf("/================================/\r\n");
       MXC_Delay(500000);
		 
		count = count + 1;
	}
}

/******************************************************************************/
/* This callback is used to allow the driver to call part specific initialization
 functions. */
/******************************************************************************/
int usbStartupCallback()
{
	// Startup the HIRC96M clock if it's not on already
	if (!(MXC_GCR->clkcn & MXC_F_GCR_CLKCN_HIRC96M_EN))
	{
		MXC_GCR->clkcn |= MXC_F_GCR_CLKCN_HIRC96M_EN;

		if (MXC_SYS_Clock_Timeout(MXC_F_GCR_CLKCN_HIRC96M_RDY) != E_NO_ERROR)
		{
			return E_TIME_OUT;
		}
	}

	MXC_SYS_ClockEnable(MXC_SYS_PERIPH_CLOCK_USB);

	return E_NO_ERROR;
}

/******************************************************************************/
int usbShutdownCallback()
{
	MXC_SYS_ClockDisable(MXC_SYS_PERIPH_CLOCK_USB);

	return E_NO_ERROR;
}

/******************************************************************************/
static void usbAppSleep(void)
{
	/* TODO: Place low-power code here */
	suspended = 1;
}

/******************************************************************************/
static void usbAppWakeup(void)
{
	/* TODO: Place low-power code here */
	suspended = 0;
}

int a = 0;

/******************************************************************************/
static int setconfigCallback(MXC_USB_SetupPkt *sud, void *cbdata)
{
	/* Confirm the configuration value */
	if (sud->wValue == config_descriptor.config_descriptor.bConfigurationValue)
	{
		configured = 1;
		MXC_SETBIT(&event_flags, EVENT_ENUM_COMP);

		acm_cfg.out_ep = config_descriptor.endpoint_descriptor_1.bEndpointAddress & 0x7;
		acm_cfg.out_maxpacket = config_descriptor.endpoint_descriptor_1.wMaxPacketSize;
		acm_cfg.in_ep = config_descriptor.endpoint_descriptor_2.bEndpointAddress & 0x7;
		acm_cfg.in_maxpacket = config_descriptor.endpoint_descriptor_2.wMaxPacketSize;
		acm_cfg.notify_ep = config_descriptor.endpoint_descriptor_3.bEndpointAddress & 0x7;
		acm_cfg.notify_maxpacket = config_descriptor.endpoint_descriptor_3.wMaxPacketSize;

		return acm_configure(&acm_cfg); /* Configure the device class */
	}
	else if (sud->wValue == 0)
	{
		configured = 0;
		return acm_deconfigure();
	}

	return -1;
}

/******************************************************************************/
static int setfeatureCallback(MXC_USB_SetupPkt *sud, void *cbdata)
{
	if (sud->wValue == FEAT_REMOTE_WAKE)
	{
		remote_wake_en = 1;
	}
	else
	{
		// Unknown callback
		return -1;
	}

	return 0;
}

/******************************************************************************/
static int clrfeatureCallback(MXC_USB_SetupPkt *sud, void *cbdata)
{
	if (sud->wValue == FEAT_REMOTE_WAKE)
	{
		remote_wake_en = 0;
	}
	else
	{
		// Unknown callback
		return -1;
	}

	return 0;
}

/******************************************************************************/
static int eventCallback(maxusb_event_t evt, void *data)
{
	/* Set event flag */
	MXC_SETBIT(&event_flags, evt);

	switch (evt)
	{
	case MAXUSB_EVENT_NOVBUS:
		MXC_USB_EventDisable(MAXUSB_EVENT_BRST);
		MXC_USB_EventDisable(MAXUSB_EVENT_SUSP);
		MXC_USB_EventDisable(MAXUSB_EVENT_DPACT);
		MXC_USB_Disconnect();
		configured = 0;
		enum_clearconfig();
		acm_deconfigure();
		usbAppSleep();
		break;

	case MAXUSB_EVENT_VBUS:
		MXC_USB_EventClear(MAXUSB_EVENT_BRST);
		MXC_USB_EventEnable(MAXUSB_EVENT_BRST, eventCallback, NULL);
		MXC_USB_EventClear(MAXUSB_EVENT_SUSP);
		MXC_USB_EventEnable(MAXUSB_EVENT_SUSP, eventCallback, NULL);
		MXC_USB_Connect();
		usbAppSleep();
		break;

	case MAXUSB_EVENT_BRST:
		usbAppWakeup();
		enum_clearconfig();
		acm_deconfigure();
		configured = 0;
		suspended = 0;
		break;

	case MAXUSB_EVENT_SUSP:
		usbAppSleep();
		break;

	case MAXUSB_EVENT_DPACT:
		usbAppWakeup();
		break;

	default:
		break;
	}

	return 0;
}

/******************************************************************************/
static int usbReadCallback(void)
{
	usb_read_complete = 1;
	return 0;
}

/******************************************************************************/
void USB_IRQHandler(void)
{
	MXC_USB_EventHandler();
}

/******************************************************************************/
void SysTick_Handler(void)
{
	MXC_DelayHandler();
}
