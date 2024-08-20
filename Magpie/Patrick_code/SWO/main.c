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
 * @file    main.c
 * @brief   Hello World!
 * @details This example uses the UART to print to a terminal and flashes an LED.
 */

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include "mxc_device.h"
#include "led.h"
#include "board.h"
#include "mxc_delay.h"
//#include "SWO.h"

void SWO_Init(uint32_t portBits, uint32_t cpuCoreFreqHz);
void SWO_PrintChar2(char c, uint8_t portNo);
void SWO_PrintString2(const char *s, uint8_t portNumber);

/***** Definitions *****/
#define OUTPUT_MSG_BUFFER_SIZE       128U


/***** Globals *****/
static char output_msgBuffer[OUTPUT_MSG_BUFFER_SIZE];
/***** Functions *****/

// *****************************************************************************
int main(void)
{
    //EnableSWO();
    SWO_Init(0x1,64);
    int count = 0;

    printf("Hello World!\n");
    SWO_PrintString2("Hello World, SWO!\n",0);

    while (1) {
        LED_On(0);
        MXC_Delay(500000);
        LED_Off(0);
        MXC_Delay(500000);
        printf("count = %d\n", count++);

        sprintf((char*)output_msgBuffer, "count: %d\n", count);
        SWO_PrintString2(output_msgBuffer,1);
        SWO_PrintString2("Hello World, SWO!\n",1);
    }


}

void SWO_Init(uint32_t portBits, uint32_t cpuCoreFreqHz) {
	uint32_t SWOSpeed = 4 ; /* default 4MHz baud rate */
	uint32_t SWOPrescaler = (cpuCoreFreqHz / SWOSpeed) - 1; /* SWOSpeed in Hz, note that cpuCoreFreqHz is expected to be match the CPU core clock */
    
	CoreDebug->DEMCR = CoreDebug_DEMCR_TRCENA_Msk; /* enable trace in core debug */
    (*(volatile unsigned int*)0xE0042004) = 0x00000027u;  //DBGMCU_CR : TRACE_IOEN DBG_STANDBY DBG_STOP 	DBG_SLEEP
	TPI->SPPR	= 0x00000002u ;	 /* "Selected PIN Protocol Register": Select which protocol to use for trace output (2: SWO NRZ, 1: SWO Manchester encoding) */
    TPI->ACPR	= SWOPrescaler ;						// Async Clock Prescaler Register: Scale the baud rate of the asynchronous output
	ITM->LAR	= 0xC5ACCE55u ;							// ITM Lock Access Register: C5ACCE55 enables more write access to Control Register 0xE00 :: 0xFFC
	ITM->TCR = ITM_TCR_TraceBusID_Msk | ITM_TCR_SWOENA_Msk | ITM_TCR_SYNCENA_Msk | ITM_TCR_ITMENA_Msk; /* ITM Trace Control Register */
	ITM->TPR = ITM_TPR_PRIVMASK_Msk; /* ITM Trace Privilege Register */
	ITM->TER = portBits; /* ITM Trace Enable Register. Enabled tracing on stimulus ports. One bit per stimulus port. */
	DWT->CTRL	= 0x400003FEu ;							// Data Watchpoint and Trace Register
	TPI->FFCR	= 0x00000100u ;							// Formatter and Flush Control Register
    

}

void SWO_PrintChar2(char c, uint8_t portNo) {
	volatile int timeout;

	/* Check if Trace Control Register (ITM->TCR at 0xE0000E80) is set */
	if ((ITM->TCR&ITM_TCR_ITMENA_Msk) == 0) { /* check Trace Control Register if ITM trace is enabled*/
		return; /* not enabled? */
	}
	/* Check if the requested channel stimulus port (ITM->TER at 0xE0000E00) is enabled */
	if ((ITM->TER & (1ul << portNo)) == 0) { /* check Trace Enable Register if requested port is enabled */
		return; /* requested port not enabled? */
	}
	timeout = 5000; /* arbitrary timeout value */
	while (ITM->PORT[portNo].u32 == 0) {
		/* Wait until STIMx is ready, then send data */
		timeout--;
		if (timeout == 0) {
			return; /* not able to send */
		}
	}
	//ITM->PORT[portNo].u16 = 0x08 | (c << 8);
	ITM->PORT[portNo].u8 = c;
}

/*!
 * \brief Sends a string over SWO to the host
 * \param s String to send
 * \param portNumber Port number, 0-31, use 0 for normal debug strings
 */
void SWO_PrintString2(const char *s, uint8_t portNumber) {
	while (*s != '\0') {
		SWO_PrintChar2(*s++, portNumber);
	}
}