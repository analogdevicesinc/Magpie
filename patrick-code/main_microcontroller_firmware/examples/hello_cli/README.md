## Description

A basic CLI hellp world program.

Toggles the 3 status LEDs via CLI commands.


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
***********Hello CLI!***********

CLI Initialized! Enter 'help' to see a list of available commands.

$ help

led_toggle:
  Usage: [led_toggle r/g/b] on command line
  Description: `led_toggle c` toggles LED color `c`

$ led_toggle r # <- the RED LED is toggled ON here

$ led_toggle r # <- the RED LED is toggled back OFF here

$ led_toggle g # <- the GREEN LED is toggled ON

$ led_toggle foo # <- error, the arg must be a single char

$ led_toggle q # <- this is an error, the color must be in [r, g, b]

```
