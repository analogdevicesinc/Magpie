## Description

A basic getting started program.

This version of Hello_World prints an incrementing count to the console UART and toggles a GPIO (LED0) once every 500 ms.

Please check the BSP files in `../../BSP/`.


## Software

### Project Usage

Universal instructions on building, flashing, and debugging this project can be found in the **[MSDK User Guide](https://analogdevicesinc.github.io/msdk/USERGUIDE/)**.

### Project-Specific Build Notes

(None - this project builds as a standard example)

## Required Connections

- Connect a DAP link to the Magpie board via 10 pin 0.1" header
- Connect a USB cable between the DAP link and PC.
- Open an terminal application on the PC and connect to the console UART at 115200, 8-N-1.

## Expected Output

The Console UART of the device will output these messages:

```
***********Hello Magpie!***********

Count = 0
Count = 1
Count = 2
Count = 3
```
