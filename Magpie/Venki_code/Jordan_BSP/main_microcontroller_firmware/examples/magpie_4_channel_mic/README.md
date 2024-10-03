## Description

The example demonstrates the use of USB Audio Class 2.0 device using the TinyUSB stack
on a bare-metal system
. After doing the required connections given below, run the program and ...


## Software

### Project Usage

Universal instructions on building, flashing, and debugging this project can be found in the **[MSDK User Guide](https://analogdevicesinc.github.io/msdk/USERGUIDE/)**.

### Project-Specific Build Notes

tusb_config.h by default has logging disabled via the CFG_TUSB_DEBUG definition.
For TinyUSB console logging, set the debug level to 1, 2, or 3, with a higher value
indicating more verbose logging.

## Required Connections

-   Connect a USB cable between the PC and the FTHR board USB connector.
-   Connect a USB cable from the MAX debugger, connect debugger header to FTHR debugger
-   Open a terminal application on the PC and connect to the debugger's console UART at 115200, 8-N-1.

## Expected Output


