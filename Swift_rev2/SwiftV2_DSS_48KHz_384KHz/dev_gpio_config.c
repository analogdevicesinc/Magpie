/*
 * dev_gpio_config.c
 *
 *  Created on: Dec 14, 2023
 *      Author: VKarra
 */

/*
| Number  |  GPIO | Power supply sequence |
|---------|-------|-----------------------|
| 1       |   P0.0| EN_5V3                |
| 2       |   P0.4| EN_1V8                |
| 3       |   P0.1| EB_5V0                |
| 4       |   P0.2| EN_2V8                |

*/

#include <stdio.h>
#include <string.h>
#include "mxc_device.h"
#include "board.h"
#include "gpio.h"
#include"mxc_delay.h"
#include "dev_gpio_config.h"

//****************************************************
//Function prototypes
//****************************************************

//LDO start up function
void LDO_startup()
{



    mxc_gpio_cfg_t gpio_out0;
    mxc_gpio_cfg_t gpio_out4;
    mxc_gpio_cfg_t gpio_out1;
    mxc_gpio_cfg_t gpio_out2;

    printf("\n\n****** Power up sequence******\n");
    printf("Now we are powering the 4 LDOs in the sequence of 5.3V, 1.8V, 5.0V, 2.8V with a delay of 500us between each supply\n");
    printf("We enable pins in order of 0.0, 0.4, 0.1,0.2\n");

    /* Setup output pin. P0.0 */
    gpio_out0.port = MXC_GPIO_PORT_OUT0;
    gpio_out0.mask = MXC_GPIO_PIN_OUT0;
    gpio_out0.pad = MXC_GPIO_PAD_NONE;
    gpio_out0.func = MXC_GPIO_FUNC_OUT;
    gpio_out0.vssel = MXC_GPIO_VSSEL_VDDIO;

    MXC_GPIO_Config(&gpio_out0);

    /* Setup output pin. P0.4 */
    gpio_out4.port = MXC_GPIO_PORT_OUT0;
    gpio_out4.mask = MXC_GPIO_PIN_OUT4;
    gpio_out4.pad = MXC_GPIO_PAD_NONE;
    gpio_out4.func = MXC_GPIO_FUNC_OUT;
    gpio_out4.vssel = MXC_GPIO_VSSEL_VDDIO;

    MXC_GPIO_Config(&gpio_out4);


    /* Setup output pin. P0.1 */
    gpio_out1.port = MXC_GPIO_PORT_OUT0;
    gpio_out1.mask = MXC_GPIO_PIN_OUT1;
    gpio_out1.pad = MXC_GPIO_PAD_NONE;
    gpio_out1.func = MXC_GPIO_FUNC_OUT;
    gpio_out1.vssel = MXC_GPIO_VSSEL_VDDIO;

    MXC_GPIO_Config(&gpio_out1);


    /* Setup output pin. P0.2 */
    gpio_out2.port = MXC_GPIO_PORT_OUT0;
    gpio_out2.mask = MXC_GPIO_PIN_OUT2;
    gpio_out2.pad = MXC_GPIO_PAD_NONE;
    gpio_out2.func = MXC_GPIO_FUNC_OUT;
    gpio_out2.vssel = MXC_GPIO_VSSEL_VDDIO;

    MXC_GPIO_Config(&gpio_out2);


    /* Setup output pin. P0.0 as high */
    MXC_GPIO_OutSet(gpio_out0.port, gpio_out0.mask);
    printf("LDO 5V3 enabled \n");
    MXC_Delay(500);

    /* Setup output pin. P0.4 as high */
    MXC_GPIO_OutSet(gpio_out4.port, gpio_out4.mask);
    printf("LDO 1V8 enabled \n");
    MXC_Delay(500);

    /* Setup output pin. P0.1 as high */
    MXC_GPIO_OutSet(gpio_out1.port, gpio_out1.mask);
    printf("LDO 5V0 enabled \n");
    MXC_Delay(500);

    /* Setup output pin. P0.2 as high */
    MXC_GPIO_OutSet(gpio_out2.port, gpio_out2.mask);
    printf("LDO 2V8 enabled \n");
    MXC_Delay(500);

}


//**********************************************************
//Enabling the mic select pin of MAX20327 through P0.12 GPIO
//**********************************************************
void MAX20327_mic_sel()
{
	mxc_gpio_cfg_t gpio_out12;
	printf("\n\n****** Enabling the mic select pin******\n");
	 //Setup output pin. P0.12
	gpio_out12.port = MXC_GPIO_PORT_OUT0;
	gpio_out12.mask = MXC_GPIO_PIN_OUT12;
	gpio_out12.pad = MXC_GPIO_PAD_NONE;
	gpio_out12.func = MXC_GPIO_FUNC_OUT;
	gpio_out12.vssel = MXC_GPIO_VSSEL_VDDIO;

	MXC_GPIO_Config(&gpio_out12);

	// Setup output pin. P0.12 as high
	MXC_GPIO_OutClr(gpio_out12.port, gpio_out12.mask);
	printf("MAX20327 mic pin enabled \n");
	MXC_GPIO_OutSet(gpio_out12.port, gpio_out12.mask);


}


/***************************************************************************
* Enabling Green LED through P0.5
****************************************************************************/
void enable_green_led()
{
	mxc_gpio_cfg_t gpio_out5;
	printf("\n\n****** Turning ON Green LED******\n");
	/* Setup output pin. P0.5 */
	gpio_out5.port = MXC_GPIO_PORT_OUT0;
	gpio_out5.mask = MXC_GPIO_PIN_OUT5;
	gpio_out5.pad = MXC_GPIO_PAD_NONE;
	gpio_out5.func = MXC_GPIO_FUNC_OUT;
	gpio_out5.vssel = MXC_GPIO_VSSEL_VDDIO;

	MXC_GPIO_Config(&gpio_out5);

	/* Setup output pin. P0.5 as high */
	MXC_GPIO_OutClr(gpio_out5.port, gpio_out5.mask);


}

/***************************************************************************
* Enabling Blue LED through P0.3
****************************************************************************/
void enable_blue_led()
{
	mxc_gpio_cfg_t gpio_out3;
	printf("\n\n****** Turning ON Blue LED******\n");
	/* Setup output pin. P0.3 */
	gpio_out3.port = MXC_GPIO_PORT_OUT0;
	gpio_out3.mask = MXC_GPIO_PIN_OUT3;
	gpio_out3.pad = MXC_GPIO_PAD_NONE;
	gpio_out3.func = MXC_GPIO_FUNC_OUT;
	gpio_out3.vssel = MXC_GPIO_VSSEL_VDDIO;

	MXC_GPIO_Config(&gpio_out3);

	/* Setup output pin. P0.3 as high */
	MXC_GPIO_OutClr(gpio_out3.port, gpio_out3.mask);


}



/***************************************************************************
* Disabling Green LED disable P0.5
****************************************************************************/
void disable_green_led()
{
	mxc_gpio_cfg_t gpio_out5;
	printf("\n\n****** Turning OFF green LED******\n");
	/* Setup output pin. P0.5 */
	gpio_out5.port = MXC_GPIO_PORT_OUT0;
	gpio_out5.mask = MXC_GPIO_PIN_OUT5;
	gpio_out5.pad = MXC_GPIO_PAD_NONE;
	gpio_out5.func = MXC_GPIO_FUNC_OUT;
	gpio_out5.vssel = MXC_GPIO_VSSEL_VDDIO;

	MXC_GPIO_Config(&gpio_out5);

    /*Clearing the GPIO to a low value*/
	MXC_GPIO_OutSet(gpio_out5.port, gpio_out5.mask);

}

/***************************************************************************
* Disabling Blue LED disable P0.3
****************************************************************************/
void disable_blue_led()
{
	mxc_gpio_cfg_t gpio_out3;
	printf("\n\n****** Turning OFF Blue LED******\n");
	/* Setup output pin. P0.5 */
	gpio_out3.port = MXC_GPIO_PORT_OUT0;
	gpio_out3.mask = MXC_GPIO_PIN_OUT3;
	gpio_out3.pad = MXC_GPIO_PAD_NONE;
	gpio_out3.func = MXC_GPIO_FUNC_OUT;
	gpio_out3.vssel = MXC_GPIO_VSSEL_VDDIO;

	MXC_GPIO_Config(&gpio_out3);

    /*Clearing the GPIO to a low value*/
	MXC_GPIO_OutSet(gpio_out3.port, gpio_out3.mask);


}

/***************************************************************************
* @brief Enabling AD4807 P0.9
****************************************************************************/
void DISABLEn_AD4807()
{
	mxc_gpio_cfg_t gpio_out9;
	printf("\n\n****** Disable AD4807******\n");
	/* Setup output pin. P0.5 */
	gpio_out9.port = MXC_GPIO_PORT_OUT0;
	gpio_out9.mask = MXC_GPIO_PIN_OUT9;
	gpio_out9.pad = MXC_GPIO_PAD_NONE;
	gpio_out9.func = MXC_GPIO_FUNC_OUT;
	gpio_out9.vssel = MXC_GPIO_VSSEL_VDDIO;
    MXC_GPIO_Config(&gpio_out9);
    /*ENabling the chip*/
	MXC_GPIO_OutSet(gpio_out9.port, gpio_out9.mask);

}


