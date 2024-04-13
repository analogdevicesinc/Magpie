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
 * @brief   SwiftV2_DSS_48KHz_384KHz
 * @details  This program works for both 48KHz and 384KHz interrupt frequency
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
#include "AD4630.h"  //ADC configuration data

/***** Definitions *****/
//USB_CDC_Protocol definitions
#define EVENT_ENUM_COMP MAXUSB_NUM_EVENTS
#define EVENT_REMOTE_WAKE (EVENT_ENUM_COMP + 1)
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)


//Definitions to access the LEDs and Push button on FTHR board
#define blue_led_port MXC_GPIO0    //Blue LED on FTHR board
#define blue_led_pin  MXC_GPIO_PIN_30

#define red_led_port MXC_GPIO0 //RED LED on FTHR board
#define red_led_pin MXC_GPIO_PIN_29

#define MXC_GPIO_PORT_IN MXC_GPIO1  //Push button switch
#define MXC_GPIO_PIN_IN MXC_GPIO_PIN_10

// bare-metal spi registers for direct spi control
#define SPI1_BASE_addr  0x40046000
#define DMA0_BASE_addr 0x40028000
#define SPI1_DATA0_direct  (*( ( volatile unsigned int * )SPI1_DATA0_addr ) )
#define SPI1_CTRL0_direct  (*( ( volatile unsigned int * )SPI1_CTRL0_addr ) )


/***** Global Data *****/
/****************************/
//USB_Data
volatile int configured;
volatile int suspended;
volatile unsigned int event_flags;
int remote_wake_en;
/****************************/


/****************************/
//SPI-Data
/****************************/
volatile uint8_t ADC_BUSY = 0;
uint8_t SPI1_rx[DATA_LEN_SPI1];
uint8_t SPI1_tx[DATA_LEN_SPI1];
int retVal, fail = 0;
mxc_spi_req_t SPI1_req;
volatile int SPI_FLAG;
// bare metal spi control
static uint32_t SPI1_DATA0_addr =  SPI1_BASE_addr + 0x00000000;
static uint32_t SPI1_CTRL0_addr =  SPI1_BASE_addr + 0x00000004;
static uint8_t lowByte=0, highByte=0;
static uint32_t temp32u=0;

/****************************/
//Push and Pop transfer data to buffer data
/****************************/
uint16_t stack[150000], spi_data;
int32_t Storage_max_size = 150000;
int location = -1;
int a,i;

/****************************/
//ADC configurations
/****************************/
uint16_t dummyaddr = 0x3FFF;
uint8_t  read_address = 0x00;
uint16_t scratchpad_address = AD463X_REG_SCRATCH_PAD;
uint8_t scratchpad_data = 0xAA;
uint8_t pwr_mode_normal = 0x00;
uint8_t pwr_mode_lowepower = 0x01;
uint8_t drive_strenght_normal =0x00;
uint8_t drive_strenght_double =0x01;
uint8_t frame_len_24 = 0x00;
uint8_t ch_idx0 = 0x00;
uint8_t ch_idx1 = 0x01;
uint8_t gain_lb = 0x00;
uint8_t gain_hb = 0x80;
uint32_t offset = 0;

/***** Function Prototypes *****/

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


/***** File Scope Variables *****/

/* This EP assignment must match the Configuration Descriptor */
static acm_cfg_t acm_cfg = {
		1, /* EP OUT */
		MXC_USBHS_MAX_PACKET, /* OUT max packet size */
		2, /* EP IN */
		MXC_USBHS_MAX_PACKET, /* IN max packet size */
		3, /* EP Notify */
		MXC_USBHS_MAX_PACKET, /* Notify max packet size */
};

static volatile int usb_read_complete;

/* User-supplied function to delay usec micro-seconds */
void delay_us(unsigned int usec)
{
	/* mxc_delay() takes unsigned long, so can't use it directly */
	MXC_Delay(usec);
}


/*****************************************************************/
//Custom functions design
/*****************************************************************/

/***Custom printf USB write design***/
void debugPrint(char *fmt, ... )
{
	char buffer[1024];
	uint16_t len = 0;
	va_list args;
	va_start( args, fmt );
	len = vsnprintf( buffer, 1024,fmt, args );
	va_end( args );

	if(len < 0)
		return;

	acm_write((uint8_t*)buffer, len);
}

/***Custom USB read function***/
int debugRead(const char *fmt, ...) {
	uint8_t buffer[256]; // Adjust the buffer size

	int canRead = 0;
	int readLen = 0;
	int num = 0;
	uint16_t inx = 0;

	while(1) {
		canRead = acm_canread();
		readLen = acm_read(&buffer[inx], canRead); // read a byte.
		acm_write(&buffer[inx],readLen); // echo the byte.
		if (buffer[inx] == '\r')
		{ // check for end of line.
			buffer[inx] = '\n';
			num+=readLen;
			inx+=readLen;
			break;
		}
		else
		{
			num+=readLen;
			inx+=readLen;
		}

	}

	va_list args;
	va_start(args, fmt);

	int itemsRead = vsscanf((const char*)buffer, fmt, args);

	va_end(args);

	return itemsRead;
}

/*****ADC Busy signal interrupt function****/

void BUSY_ISR(void)

{
	uint32_t stat;
	stat = MXC_GPIO_GetFlags(MXC_GPIO0);
	MXC_GPIO_ClearFlags(MXC_GPIO0, stat);
	ADC_BUSY = 1;

}




/******************************************************************************/
int main(void)
{
	printf("Starting USB initializations\r\n");

	//*****************************************************************************/
	//USB initializations
	//*****************************************************************************/
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

	printf("Performing ADC hard reset\r\n");
	hardware_reset();  //Hardware reset for ADC

	//*****************************************************************************/
	//GPIO and SPI1 initializations for reading data from ADC
	//*****************************************************************************/
	printf("Initializing the ADC_BUSY interrupt, push button and LEDs on the FTHR board\r\n");
	mxc_gpio_cfg_t BUSY;
	mxc_gpio_cfg_t blue_led;
	mxc_gpio_cfg_t red_led;
	mxc_gpio_cfg_t gpio_in;


	gpio_in.port = MXC_GPIO_PORT_IN;
	gpio_in.mask = MXC_GPIO_PIN_IN;
	gpio_in.pad = MXC_GPIO_PAD_PULL_UP;
	gpio_in.func = MXC_GPIO_FUNC_IN;
	gpio_in.vssel = MXC_GPIO_VSSEL_VDDIO;
	gpio_in.drvstr = MXC_GPIO_DRVSTR_0;
	MXC_GPIO_Config(&gpio_in);

	BUSY.port = BUSY_PORT;
	BUSY.mask = BUSY_PIN;
	BUSY.pad = MXC_GPIO_PAD_PULL_UP;
	BUSY.func = MXC_GPIO_FUNC_IN;
	BUSY.vssel = MXC_GPIO_VSSEL_VDDIO;
	MXC_GPIO_IntConfig(&BUSY, MXC_GPIO_INT_FALLING);
	MXC_GPIO_EnableInt(BUSY.port, BUSY.mask);
	NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(BUSY_PORT)));
	MXC_NVIC_SetVector(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(BUSY_PORT)), BUSY_ISR);


	blue_led.port = blue_led_port;
	blue_led.mask = blue_led_pin;
	blue_led.pad = MXC_GPIO_PAD_NONE;
	blue_led.func = MXC_GPIO_FUNC_OUT;
	blue_led.vssel = MXC_GPIO_VSSEL_VDDIO;
	blue_led.drvstr = MXC_GPIO_DRVSTR_0;
	MXC_GPIO_Config(&blue_led);

	red_led.port = red_led_port;
	red_led.mask = red_led_pin;
	red_led.pad = MXC_GPIO_PAD_NONE;
	red_led.func = MXC_GPIO_FUNC_OUT;
	red_led.vssel = MXC_GPIO_VSSEL_VDDIO;
	red_led.drvstr = MXC_GPIO_DRVSTR_0;
	MXC_GPIO_Config(&red_led);

	printf("Initializing the SPI to read ADC data\r\n");
	// Initialize the SPI
	retVal = MXC_SPI_Init(SPI_MASTER1, 1, 0, 1, 0, SPI_SPEED_SPI1, MAP_A);
	if (retVal != E_NO_ERROR)
	{
		printf("\nSPI INITIALIZATION ERROR\n");
		return retVal;
	}



	SPI1_req.spi = SPI_MASTER1;
	SPI1_req.txData = (uint8_t *)SPI1_tx;
	SPI1_req.rxData = (uint8_t *)SPI1_rx;
	SPI1_req.txLen = 0;
	SPI1_req.rxLen = DATA_LEN_SPI1;
	SPI1_req.ssIdx = 0;
	SPI1_req.ssDeassert = 1;
	SPI1_req.txCnt = 0;
	SPI1_req.rxCnt = 0;
	SPI1_req.completeCB = NULL;

	//Setting the data size
	retVal = MXC_SPI_SetDataSize(SPI_MASTER1, DATA_SIZE_SPI1);
	if (retVal != E_NO_ERROR)
	{
		printf("\nSPI MASTER INITIALIZATION ERROR\n");
		return E_FAIL;
	}

	//Setting width of the SPI in this case 3- wire SPI for SPI1 master
	retVal = MXC_SPI_SetWidth(SPI_MASTER1, SPI_WIDTH_3WIRE);
	if (retVal != E_NO_ERROR)
	{
		printf("\nSPI MASTER INITIALIZATION ERROR\n");
		return E_FAIL;
	}

	//Setting the SPI mode
	retVal = MXC_SPI_SetMode(SPI_MASTER1, SPI_MODE_0);
	if (retVal != E_NO_ERROR)
	{
		printf("\nSPI MASTER INITIALIZATION ERROR\n");
		return E_FAIL;
	}

	memset(SPI1_rx, 0x0, DATA_LEN_SPI1 * sizeof(uint8_t)); //Filling a block of memory
	// call this function to execute the reg init part of the function, the data is not used
	MXC_SPI_MasterTransaction(&SPI1_req); // complete the init; don't use the data!

	int button_press;
	/* Wait for events */

	while (1)
	{
		button_press = !MXC_GPIO_InGet(gpio_in.port, gpio_in.mask); //Read and assign the status of the push button

		if (button_press == 1) // Check for the switch press
		{
			MXC_GPIO_EnableInt(BUSY.port, BUSY.mask);
			MXC_GPIO_OutClr(blue_led.port, blue_led.mask); //ON blue LED
			MXC_GPIO_OutSet(red_led.port, red_led.mask);  //OFF RED LED
			while(location!=150000) //This loop runs until the buffer location reaches 150000
			{

				if((ADC_BUSY == 1)) // ADC busy signal check
				{

					SPI1_CTRL0_direct |= 0x00000020; // bare-metal spi start

					//note, the next bare-metal read from SPI1_DATA0_direct is happening while the spi read is still active
					// since the following 4 lines only take about 1us, the spi read is not finished yet
					// so we are really getting data from the previous read, stored in the fifo

					temp32u = SPI1_DATA0_direct; // read spi fifo, result has byte order in wrong order, need to fix
					lowByte = (temp32u >> 8) & 0xff;
					highByte = temp32u & 0xff;
					spi_data = lowByte | (highByte << 8); // 16-but 2's comp, need to sign-extend to 32 bits
					stack[location] = spi_data^0x8000;
					location = location + 1; //Incrementing the location
					ADC_BUSY = 0;


				}
			}


			if(location == 150000) //Checks if the buffer reached 150000 locations
			{
				MXC_GPIO_DisableInt(BUSY.port, BUSY.mask); // stop interupts when reach max
				MXC_GPIO_OutSet(blue_led.port, blue_led.mask); //OFF blue LED
				MXC_GPIO_OutClr(red_led.port, red_led.mask);  //ON RED LED
				while(location--) //Starts moving data to the PC
				{

					//debugPrint("0X%x\r\n",stack[location]);// stack[location]);
					debugPrint("0X%x ",stack[location]);// stack[location]);

				}
				debugPrint("\n");// matlab must be set for \n as seperator

			}
			MXC_GPIO_OutSet(red_led.port, red_led.mask);
			button_press = 0; //Clearing the switch press so the loop waits for new button press



		}


	}



}


/******************************************************************************/
/* This callback is used to allow the driver to call part specific initialization functions. */
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
	} else if (sud->wValue == 0)
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
	} else {
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
	} else
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


