/*********************************************************************
*                   (c) SEGGER Microcontroller GmbH                  *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************
-------------------------- END-OF-HEADER -----------------------------
File    : SWO.c
Purpose : Simple implementation for output via SWO for Cortex-M processors.
          It can be used with any IDE. This sample implementation ensures
          that output via SWO is enabled in order to guarantee that the
          application does not hang.
Literature:
  [1]  UM08001 J-Link / J-Trace User Guide
*/
#include <stdint.h>

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define ITM_ENA   (*(volatile unsigned int*)0xE0000E00) // ITM Enable
#define ITM_TPR   (*(volatile unsigned int*)0xE0000E40) // Trace Privilege
                                                        // Register
#define ITM_TCR   (*(volatile unsigned int*)0xE0000E80) // ITM Trace Control Reg.
#define ITM_LSR   (*(volatile unsigned int*)0xE0000FB0) // ITM Lock Status
                                                        // Register
#define DHCSR     (*(volatile unsigned int*)0xE000EDF0) // Debug register
#define DEMCR     (*(volatile unsigned int*)0xE000EDFC) // Debug register
#define TPIU_ACPR (*(volatile unsigned int*)0xE0040010) // Async Clock
                                                        // prescaler register
#define TPIU_SPPR (*(volatile unsigned int*)0xE00400F0) // Selected Pin Protocol
                                                        // Register
#define DWT_CTRL  (*(volatile unsigned int*)0xE0001000) // DWT Control Register
#define FFCR      (*(volatile unsigned int*)0xE0040304) // Formatter and flush
                                                        // Control Register

/*********************************************************************
*
*       Defines for Cortex-M debug unit
*
**********************************************************************
*/
#define ITM_STIM_U32 (*(volatile unsigned int*)0xE0000000) // STIM word access
#define ITM_STIM_U8  (*(volatile         char*)0xE0000000) // STIM Byte access
#define ITM_ENA      (*(volatile unsigned int*)0xE0000E00) // ITM Enable Register
#define ITM_TCR      (*(volatile unsigned int*)0xE0000E80) // ITM Trace Control
                                                           // Register

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
uint32_t _ITMPort  = 0; // The stimulus port from which SWO data is received
                   // and displayed.
uint32_t TargetDiv = 16; // Has to be calculated according to
                   // the CPU speed and the output baud rate
                   //CPU = 63834 kHz, Baud = 4000 kHz

/*********************************************************************
*
*       Global functions
*
**********************************************************************
*/
/*********************************************************************
*
*       EnableSWO()
*
* Function description
*   Enables SWO from target side.
*
*/
void EnableSWO() {
  uint32_t StimulusRegs;
  //
  // Enable access to SWO registers
  //
  DEMCR |= (1 << 24);
  ITM_LSR = 0xC5ACCE55;
  //
  // Initially disable ITM and stimulus port
  // To make sure that nothing is transferred via SWO
  // when changing the SWO prescaler etc.
  //
  StimulusRegs = ITM_ENA;
  StimulusRegs &= ~(1 << _ITMPort);
  ITM_ENA = StimulusRegs; // Disable ITM stimulus port
  ITM_TCR = 0; // Disable ITM
  //
  // Initialize SWO (prescaler, etc.)
  //
  TPIU_SPPR = 0x00000002; // Select NRZ mode
  TPIU_ACPR = TargetDiv - 1; // Example: 72/48 = 1,5 MHz
  ITM_TPR = 0x00000000;
  DWT_CTRL = 0x400003FE;
  FFCR = 0x00000100;
  //
  // Enable ITM and stimulus port
  //
  ITM_TCR = 0x1000D; // Enable ITM
  ITM_ENA = StimulusRegs | (1 << _ITMPort); // Enable ITM stimulus port
}

/*********************************************************************
*
*       SWO_PrintChar()
*
* Function description
*   Checks if SWO is set up. If it is not, return,
*   to avoid program hangs if no debugger is connected.
*   If it is set up, print a character to the ITM_STIM register
*   in order to provide data for SWO.
* Parameters
*   c:    The character to be printed.
* Notes
*   Additional checks for device specific registers can be added.
*/
void SWO_PrintChar(char c) {
  //
  // Check if ITM_TCR.ITMENA is set
  //
  if ((ITM_TCR & 1) == 0) {
    return;
  }
  //
  // Check if stimulus port is enabled
  //
  if ((ITM_ENA & 1) == 0) {
    return;
  }
  //
  // Wait until STIMx is ready,
  // then send data
  //
  while ((ITM_STIM_U8 & 1) == 0);
  ITM_STIM_U8 = c;
}

/*********************************************************************
*
*       SWO_PrintString()
*
* Function description
*   Print a string via SWO.
*
*/
void SWO_PrintString(const char *s) {
  //
  // Print out character per character
  //
  while (*s) {
    SWO_PrintChar(*s++);
  }
}

/*************************** End of file ****************************/