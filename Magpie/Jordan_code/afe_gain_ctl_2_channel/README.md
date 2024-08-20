## Description

This example uses the I2C Master to control the gain of the AFE via MAX14662 analog switch. It is for the Magpie custom hardware and runs on a MAX32666 microcontroller.

## NOTE
The code here can not be easily tested until the batch of FTC boards is manufactured, because the GTC spin of boards only has one AFE channel with one I2C address.

## Software

### Project Usage

Universal instructions on building, flashing, and debugging this project can be found in the **[MSDK User Guide](https://analogdevicesinc.github.io/msdk/USERGUIDE/)**.

### Project-Specific Build Notes
- This example requires the Magpie hardware stack to be assembled and the AFE installed.

## Required Connections

-   Connect a USB cable between the PC and the CN2 (USB/PWR) connector.
-   Open an terminal application on the PC and connect to the EV kit's console UART at 115200, 8-N-1..

## Expected Output

TODO: update me once we get the FTC spin boards in and example code to exercise the 2-channel AFE is written
