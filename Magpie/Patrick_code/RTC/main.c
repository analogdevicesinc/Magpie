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
 * @brief       Configures and starts the RTC and demonstrates the use of the alarms.
 * @details     The RTC is enabled and the sub-second alarm set to trigger every 250 ms.
 *              P2.25 (LED0) is toggled each time the sub-second alarm triggers.  The
 *              time-of-day alarm is set to 2 seconds.  When the time-of-day alarm
 *              triggers, the rate of the sub-second alarm is switched to 500 ms.  The
 *              time-of-day alarm is then rearmed for another 2 sec.  Pressing SW2 will
 *              output the current value of the RTC to the console UART.
 */

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include "mxc_device.h"
#include "mxc_delay.h"
#include "mxc_errors.h"
#include "nvic_table.h"
#include "board.h"
#include "rtc.h"
#include "led.h"
#include "pb.h"

#include "config.h"
//#include "SEGGER_RTT.h"
#include "DS3231_driver.h"
#include "time.h"

/***** Definitions *****/

#define LED_ALARM 0

#define TIME_OF_DAY_SEC 5
#define SUBSECOND_MSEC_0 1000
#define SUBSECOND_MSEC_1 250

#define MSEC_TO_RSSA(x) \
    (0 - ((x * 4096) /  \
          1000)) /* Converts a time in milleseconds to the equivalent RSSA register value. */

#define SECS_PER_MIN 60
#define SECS_PER_HR (60 * SECS_PER_MIN)
#define SECS_PER_DAY (24 * SECS_PER_HR)


/***** Globals *****/
bool isAlarmTriggered = false;
int ss_interval = SUBSECOND_MSEC_0;
static ds3231_driver_t DS3231_RTC;

 mxc_gpio_cfg_t gpio_interrupt;

/***** Functions *****/

static struct tm ds3231_datetime;
static char ds3231_datetime_str[17];
static float ds3231_temperature;
static uint8_t output_msgBuffer[OUTPUT_MSG_BUFFER_SIZE];
const struct tm ds3231_dateTimeDefault = {
	.tm_year = 118U,
	.tm_mon = 00U,
	.tm_mday = 1U,
	.tm_hour = 0U,
	.tm_min = 0U,
	.tm_sec = 0U
};


/*********************************************************************************************
*
* @name     DS3231_I2C_init
*
* @brief Function to initialize the USCI B1 peripheral. Setting this up
*           as a I2C bus.
*
*           P1_15 - SCL
*           P1_16 - SDA
*
* @return DS3231_i2cError enum type:   DS3231_I2C_NO_ERROR, 
*                                      DS3231_I2C_INIT_ERROR,
*                                      DS3231_I2C_FREQSET_ERROR
*
**********************************************************************************************/
DS3231_i2cError DS3231_I2C_init()
{
  if (MXC_I2C_Init(MAX32666_I2C_BUS_3V3_PULLUPS, MAX32666_I2C_CFG_MASTER_MODE, 0) != E_NO_ERROR)
  {
	printf("Unable to initialize I2C.\n");
    return MCU_MASTER_I2C_INIT_ERROR;
  }

  // I2C pins default to VDDIO for the logical high voltage, we want VDDIOH for 3.3v pullups
  const mxc_gpio_cfg_t i2c_2_pins = {
      .port = MXC_GPIO1,
      .mask = (MXC_GPIO_PIN_14 | MXC_GPIO_PIN_15),
      .pad = MXC_GPIO_PAD_NONE,
      .func = MXC_GPIO_FUNC_ALT1,
      .vssel = MXC_GPIO_VSSEL_VDDIOH,
      .drvstr = MXC_GPIO_DRVSTR_0,
  };
  MXC_GPIO_Config(&i2c_2_pins);

  if (MXC_I2C_SetFrequency(MAX32666_I2C_BUS_3V3_PULLUPS, MAX32666_I2C_CLK_SPEED) != MAX32666_I2C_CLK_SPEED)
  {
	printf("Unable to set I2C frequency.\n");
    return DS3231_I2C_FREQSET_ERROR;
  }

  DS3231_RTC = DS3231_Open();

  if(E_NO_ERROR != DS3231_RTC.init(MAX32666_I2C_BUS_3V3_PULLUPS))
  {
	printf("Unable to initialize RTC driver.\n");
	return DS3231_I2C_INIT_ERROR;
  }
 
  return DS3231_I2C_NO_ERROR;
}


static void enable_DS3231_Interrupt(void)
{
      /*   Set up interrupt on MXC_GPIO_PORT_INTERRUPT_IN.
  *   DS3231 INT is pulled high when not signaling.  Use an internal pull-up so pin
  *     reads high when button is not pressed.
  */
    gpio_interrupt.port = MXC_GPIO_PORT_INTERRUPT_IN;
    gpio_interrupt.mask = MXC_GPIO_PIN_INTERRUPT_IN;
    gpio_interrupt.pad = MXC_GPIO_PAD_PULL_UP;
    gpio_interrupt.func = MXC_GPIO_FUNC_IN;
    gpio_interrupt.vssel = MXC_GPIO_VSSEL_VDDIOH;
    MXC_GPIO_RegisterCallback(&gpio_interrupt, ds3231_ISR, NULL);
    MXC_GPIO_IntConfig(&gpio_interrupt, MXC_GPIO_INT_FALLING);
    MXC_GPIO_EnableInt(gpio_interrupt.port, gpio_interrupt.mask);
    NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(MXC_GPIO_GET_IDX(MXC_GPIO_PORT_INTERRUPT_IN)));
}

static void ds3231_ISR(void)
{
    //When DS3231 interrupt trigger, the time should be ALARM_SYNC_DELAY_S seconds later from the time was read
    NVIC_EnableIRQ(RTC_IRQn);
    reset_MAX_RTC(ds3231_datetime.tm_hour, ds3231_datetime.tm_min, ds3231_datetime.tm_sec + ALARM_SYNC_DELAY_S);
    //reset_MAX_RTC(0,0);
    isAlarmTriggered = true;
}

static void Sync_RTC_to_DS3231(void)
{    
    printf("Syncing with DS3231.\n");
    enable_DS3231_Interrupt();

    //Init DS3231 RTC peripheral
	if(DS3231_I2C_NO_ERROR != DS3231_I2C_init())
	{
		printf("Unable to initialize DS3231 driver.\n");
		LED_On(LED_RED);
		return 1;
	}

	#ifdef FIRST_SET_RTC
	// //Set Date Time to something
	//Year is always Year - 1900
	//Month is 0-11 so subtract 1 from the month you want to set
	//Time is in UTC so set appropriately
	// hour is 0-23
	// min is 0-59
	// sec is 0-59
	struct tm newTime = {
		.tm_year = 2024 - 1900U,
		.tm_mon = 7 - 1U,
		.tm_mday = 23U,
		.tm_hour = 3U,
		.tm_min = 52U,
		.tm_sec = 0U
	};
	
	//Set Date Time on RTC. 	
	if (E_NO_ERROR != DS3231_RTC.set_datetime(&newTime)) {
		printf("\nDS3231 set time error\n");
	} else {
		strftime((char*)output_msgBuffer, OUTPUT_MSG_BUFFER_SIZE, "\n-->Set DS3231 DateTime: %F %TZ\r\n", &newTime);
		printf(output_msgBuffer);
	}
	#endif

    //Get Date Time from RTC	
	if (E_NO_ERROR != DS3231_RTC.read_datetime(&ds3231_datetime, ds3231_datetime_str)) {
		printf("\nDS3231 read datetime error\n");
	} else {
		strftime((char*)output_msgBuffer, OUTPUT_MSG_BUFFER_SIZE, "\n-->Read DS3231 DateTime: %F %TZ\r\n", &ds3231_datetime);
		printf(output_msgBuffer);
		strftime((char*)output_msgBuffer, OUTPUT_MSG_BUFFER_SIZE, "\n-->FileStampTime: %Y%m%d_%H%M%SZ\r\n", &ds3231_datetime);
		printf(output_msgBuffer);
		printf(ds3231_datetime_str);	
	}

    //Set DS3231 alarm ALARM_SYNC_DELAY_S second from now
    struct tm alarmTime = {
		.tm_year = ds3231_datetime.tm_year,
		.tm_mon = ds3231_datetime.tm_mon,
		.tm_mday = ds3231_datetime.tm_mday,
		.tm_hour = ds3231_datetime.tm_hour,
		.tm_min = ds3231_datetime.tm_min,
		.tm_sec = ds3231_datetime.tm_sec + ALARM_SYNC_DELAY_S
	};
    
    if (E_NO_ERROR != DS3231_RTC.set_alarm(&alarmTime))
    {
        printf("\nDS3231 set alarm error\n");
    } else {
        strftime((char*)output_msgBuffer, OUTPUT_MSG_BUFFER_SIZE, "\n-->DS3231 Alarm DateTime: %F %TZ\r\n", &alarmTime);
		printf(output_msgBuffer);
    }

}

static void reset_MAX_RTC(int hour, int minute, int sec)
{
    //uint32_t time;
    int total_sec = (hour * SECS_PER_HR ) + (minute * SECS_PER_MIN) + sec;


    //Stop RTC interrupt monintoring
    NVIC_DisableIRQ(RTC_IRQn);

    if (MXC_RTC_Init(total_sec, 0) != E_NO_ERROR) {
        printf("Failed RTC Initialization\n");
        printf("Example Failed\n");
        while (1) {}
    }

    //Disable the Day(i.e.Second) Alarm
    if (MXC_RTC_DisableInt(MXC_RTC_INT_EN_LONG) == E_BUSY) {
        return E_BUSY;
    }

    //MXC_RTC_GetSeconds(&time);

    //Set the day alarm to the time offset from the total_sec
    if (MXC_RTC_SetTimeofdayAlarm((uint32_t)total_sec + TIME_OF_DAY_SEC) != E_NO_ERROR) {
        printf("Failed RTC_SetTimeofdayAlarm\n");
        printf("Example Failed\n");
        while (1) {}
    }

    //Enable the Day Alarm
    if (MXC_RTC_EnableInt(MXC_RTC_INT_EN_LONG) == E_BUSY) {
        return E_BUSY;
    }

    //Disable sub-second alarm
    if (MXC_RTC_DisableInt(MXC_RTC_INT_EN_SHORT) == E_BUSY) {
        return E_BUSY;
    }

    //Set sub-second alarm
    if (MXC_RTC_SetSubsecondAlarm(MSEC_TO_RSSA(SUBSECOND_MSEC_0)) != E_NO_ERROR) {
        printf("Failed RTC_SetSubsecondAlarm\n");
        printf("Example Failed\n");
        while (1) {}
    }

    //Enable sub-second alarm
    if (MXC_RTC_EnableInt(MXC_RTC_INT_EN_SHORT) == E_BUSY) {
        return E_BUSY;
    }

    //Enable the clock ticking at 512Hz (~500 ms)
    if (MXC_RTC_SquareWaveStart(MXC_RTC_F_512HZ) == E_BUSY) {
        return E_BUSY;
    }
    //start RTC interrupt monitoring
    NVIC_EnableIRQ(RTC_IRQn);

    if (MXC_RTC_Start() != E_NO_ERROR) 
    {
        printf("Failed RTC_Start\n");
        printf("Example Failed\n");
        while (1) {}
    }
}

void RTC_IRQHandler(void)
{
    //****MXC_RTC_INT_FL_SHORT is the Sub-Second Alarm */
    //****MXC_RTC_INT_FL_LONG is the Second Alarm */

    uint32_t time;
    int flags = MXC_RTC_GetFlags();

    /* Check sub-second alarm flag. */
    if (flags & MXC_RTC_INT_FL_SHORT) {
        LED_Toggle(LED_ALARM);
        MXC_RTC_ClearFlags(MXC_RTC_INT_FL_SHORT);
    }

    /* Check time-of-day alarm flag. */
    if (flags & MXC_RTC_INT_FL_LONG) {
        LED_Toggle(FTHR2_LED_BLUE);
        MXC_RTC_ClearFlags(MXC_RTC_INT_FL_LONG);

        while (MXC_RTC_DisableInt(MXC_F_RTC_CTRL_ADE) == E_BUSY) {}

        /* Set a new alarm TIME_OF_DAY_SEC seconds from current time. */
        /* Don't need to check busy here as it was checked in MXC_RTC_DisableInt() */
        MXC_RTC_GetSeconds(&time);

        if (MXC_RTC_SetTimeofdayAlarm(time + TIME_OF_DAY_SEC) != E_NO_ERROR) {
            /* Handle Error */
        }

        while (MXC_RTC_EnableInt(MXC_F_RTC_CTRL_ADE) == E_BUSY) {}

        // Toggle the sub-second alarm interval.
        if (ss_interval == SUBSECOND_MSEC_0) {
            ss_interval = SUBSECOND_MSEC_1;
        } else {
            ss_interval = SUBSECOND_MSEC_0;
        }

        while (MXC_RTC_DisableInt(MXC_F_RTC_CTRL_ASE) == E_BUSY) {}

        if (MXC_RTC_SetSubsecondAlarm(MSEC_TO_RSSA(ss_interval)) != E_NO_ERROR) {
            /* Handle Error */
        }

        while (MXC_RTC_EnableInt(MXC_F_RTC_CTRL_ASE) == E_BUSY) {}
    }

    return;
}

volatile int buttonPressed = 0;
void buttonHandler()
{
    buttonPressed = 1;
}

void printTime()
{
    int year, month, day, hr, min, err;
    uint32_t sec, rtc_readout;
    double subsec;

    do {
        err = MXC_RTC_GetSubSeconds(&rtc_readout);
    } while (err != E_NO_ERROR);
    subsec = rtc_readout / 4096.0;

    do {
        err = MXC_RTC_GetSeconds(&rtc_readout);
    } while (err != E_NO_ERROR);
    sec = rtc_readout;

    hr = sec / SECS_PER_HR;
    sec -= hr * SECS_PER_HR;

    min = sec / SECS_PER_MIN;
    sec -= min * SECS_PER_MIN;

    subsec += sec;

    year = ds3231_datetime.tm_year + 1900; 
	month = ds3231_datetime.tm_mon + 1;
    day = ds3231_datetime.tm_mday;

    printf("\nMAX3666 RTC Current DateTime : %d/%02d/%02d %02d:%02d:%05.2fZ\n", year, month, day, hr, min, subsec);

    if (E_NO_ERROR != DS3231_RTC.read_datetime(&ds3231_datetime, ds3231_datetime_str)) {
        printf("\nDS3231 read datetime error\n");
    } else {
        strftime((char*)output_msgBuffer, OUTPUT_MSG_BUFFER_SIZE, "DS3231 RTC Current DateTime: %F %TZ\n\n", &ds3231_datetime);
        printf(output_msgBuffer);
    }

}

// *****************************************************************************
int main(void)
{
    printf("\n*************************** RTC Example ****************************\n\n");
    printf("The RTC is enabled and the sub-second alarm set to trigger every %d ms.\n",
           SUBSECOND_MSEC_0);
    printf("LED0 is toggled each time the sub-second alarm triggers.\n\n");
    printf("The time-of-day alarm is set to %d seconds.  When the time-of-day alarm\n",
           TIME_OF_DAY_SEC);
    printf("triggers, the rate of the sub-second alarm is switched to %d ms.\n\n",
           SUBSECOND_MSEC_1);
    printf(
        "The time-of-day alarm is then rearmed for another %d sec.  Pressing push button 0 will\n",
        TIME_OF_DAY_SEC);
    printf("output the current value of the RTC to the console UART.\n\n");


    /* Setup callback to receive notification of when button is pressed. */
    PB_RegisterCallback(0, (pb_callback)buttonHandler);
 
    Sync_RTC_to_DS3231();

    printf("Waiting for DS3231 Alarm ...\n");
    while(!isAlarmTriggered)
    {
        MXC_Delay(MXC_DELAY_MSEC(100));
    }

    LED_On(FTHR2_LED_GREEN);
    printf("RTC synced and started\n");

    /* Turn LED off initially */
    LED_Off(LED_ALARM);
        
    printTime();

     //Init DS3231 RTC peripheral
	if(DS3231_I2C_NO_ERROR != DS3231_I2C_init())
	{
		printf("Unable to initialize DS3231 driver.\n");
		LED_On(LED_RED);
		return 1;
	}

    while (1) {
        if (buttonPressed) {
            /* Show the time */
            printTime();           
            
            /* Delay for switch debouncing. */
            MXC_Delay(MXC_DELAY_MSEC(100));
            /* Re-arm switch detection. */           

            buttonPressed = 0;
        }
    }
}
