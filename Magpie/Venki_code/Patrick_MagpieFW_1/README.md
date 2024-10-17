## Description

Magpie test code to do the core activities of the system - Setting up ADC, DMA, SDHC, external RTC.  The RTC can be initialized and set a date.  Afterward, the date can be retrived to use for filename date/time stamp as well as assigning the same date/time stamp to the file system on the SD Card.  The code contains decimation filters for various sampling rate.

MSDK library modules that are used for the program got imported locally inside the project so that the code can be tweaked without modifying the original MDSK library modules.


## Software
ADI-MSDK

### Project Usage

Universal instructions on building, flashing, and debugging this project can be found in the **[MSDK User Guide](https://analogdevicesinc.github.io/msdk/USERGUIDE/)**.

### Project-Specific Build Notes
Currently CUSTOM is chosen for the board since it's modeled after FTHR2 which is the actual board being tested.

Modify .vscode/seetings.json line 18 "board":"CUSTOM" to correctly choose the board you are working with. 
The board options are
```
CUSTOM  (based on FTHR2)
CUSTOM2  (basted on FTHR)
FTHR
FTHR2
```
## Required Connections

-   Connect a USB cable between the PC and the USB-C connector on the MAX32666FTHR2 board
-   Open an terminal application on the PC and connect to the FTHR2's console UART at 115200, 8-N-1.

## Expected Output



