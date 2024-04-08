## Description

This program is targeted towards MAX32666FTHR which talks to two other designs Mother board and Daughter Board.  The Mother board houses power supplies, oscillators and AD4630 ADC where as the daughter board has all the Mics, OP-AMPS used for recording data.  The code initiates SPI communication between MAX32666FTHR and AD4630 reading the recorded data from Mics. This data is simultaneously transferred to PC using USB on the FTHR.
Please press SW1 on FTHR and start speaking (Blue LED is ON).  We only have two seconds to hold the button.  Donot hold it for long because it fills up the buffer completely.  As soon as the SW1 is released the data is transferred to PC through VBUS (Red LED will be ON)
Please check the board.c file in ${MSDKPath}\Libraries\Boards\MAX32665\${BoardName}\Source path to learn switch and LED pins for specific board.


## Software

### Project Usage

Universal instructions on building, flashing, and debugging this project can be found in the **[MSDK User Guide](https://analogdevicesinc.github.io/msdk/USERGUIDE/)**.

### Project-Specific Build Notes

(None - this project builds as a standard example)

## Required Connections

-   Connect a USB cable between the PC and the CN2 (USB/PWR) connector.
-   Open an terminal application on the PC and connect to the EV kit's console UART at 115200, 8-N-1.
-   Power supply capable of generating 3.6Volts connected to P12 of the Mother board

## Expected Output

The VBUS UART of the device will output these messages:
0X7fe4
0X7fe7
0X7fd6
0X7fba
0X7fb6
0X7fb4
0X7f8b
0X7fac
0X7f98
0X7fe7
0X7fc3
0X7fe9
0X7fd1
0X7fe7
0X8001
0X7ffc
0X8024
0X8050
0X804b
0X8070
0X8079
0X807f



