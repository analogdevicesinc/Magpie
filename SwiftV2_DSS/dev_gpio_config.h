/*
 * Port_config.h
 *
 *  Created on: Dec 14, 2023
 *      Author: VKarra
 */

#ifndef DEV_GPIO_CONFIG_H_
#define DEV_GPIO_CONFIG_H_

#include "mxc_device.h"
#include "gpio.h"
#include "board.h"

//Pin definitions
//Pin 0.0
#define MXC_GPIO_PORT_OUT0 MXC_GPIO0
#define MXC_GPIO_PIN_OUT0 MXC_GPIO_PIN_0

//Pin 0.1
#define MXC_GPIO_PORT_OUT0 MXC_GPIO0
#define MXC_GPIO_PIN_OUT1 MXC_GPIO_PIN_1

//Pin 0.2
#define MXC_GPIO_PORT_OUT0 MXC_GPIO0
#define MXC_GPIO_PIN_OUT2 MXC_GPIO_PIN_2

//Pin 0.4
#define MXC_GPIO_PORT_OUT0 MXC_GPIO0
#define MXC_GPIO_PIN_OUT4 MXC_GPIO_PIN_4

//Pin 0.12
#define MXC_GPIO_PORT_OUT0 MXC_GPIO0
#define MXC_GPIO_PIN_OUT12 MXC_GPIO_PIN_12

//Pin 0.3
#define MXC_GPIO_PORT_OUT0 MXC_GPIO0
#define MXC_GPIO_PIN_OUT3 MXC_GPIO_PIN_3

//Pin 0.5
#define MXC_GPIO_PORT_OUT0 MXC_GPIO0
#define MXC_GPIO_PIN_OUT5 MXC_GPIO_PIN_5

//Pin 0.20
#define MXC_GPIO_PORT_OUT0 MXC_GPIO0
#define MXC_GPIO_PIN_OUT20 MXC_GPIO_PIN_20

//Pin 0.9
#define MXC_GPIO_PORT_OUT0 MXC_GPIO0
#define MXC_GPIO_PIN_OUT9 MXC_GPIO_PIN_9


//*******************************************
//GPIO functions used in the code
//*******************************************


/***************************************************************************
* @brief   LDO start up function to fire up GPIOs in sequence of 0.0,0.4,0.1,0.2
****************************************************************************/

void LDO_startup();

/***************************************************************************
* @brief  Enable the P0.12 for selecting mic through MAX20327
****************************************************************************/

void MAX20327_mic_sel();


/***************************************************************************
* @brief  Green LED enable P0.5
****************************************************************************/
void enable_green_led();

/***************************************************************************
* @brief  Blue LED enable P0.3
****************************************************************************/
void enable_blue_led();

/***************************************************************************
* @brief  Green LED disable P0.5
****************************************************************************/
void disable_green_led();

/***************************************************************************
* @brief  Blue LED disable P0.3
****************************************************************************/
void disable_blue_led();


/***************************************************************************
* @brief Enabling 74LV4060_MR P0.20
****************************************************************************/
void enable_74LV4060();

/***************************************************************************
* @brief Enabling AD4807 P0.9
****************************************************************************/
void DISABLEn_AD4807();


#endif /* DEV_GPIO_CONFIG_H_ */
