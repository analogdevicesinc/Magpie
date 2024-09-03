## Description

This example acts as a driver https://www.ti.com/lit/ds/symlink/tps22994.pdf


## Software

### Project Usage

Universal instructions on building, flashing, and debugging this project can be found in the **[MSDK User Guide](https://analogdevicesinc.github.io/msdk/USERGUIDE/)**.

### Project-Specific Build Notes
- This example requires the Magpie hardware stack to be assembled and the AFE installed.

## Required Connections

-   Connect a USB cable between the PC and the CN2 (USB/PWR) connector.
-   Open an terminal application on the PC and connect to the EV kit's console UART at 115200, 8-N-1..

## Expected Output



If there is a problem writing or reading to the TPS22994, or if the gain written does not match the gain that is read back, then an error message will be shown in the terminal.
