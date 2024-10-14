## Description

A CLI program to power on and off the AFE and control the AFE gain.


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

An example Console UART session is shown below:

```
*********** CLI AFE Control Example ***********

[SUCCESS]--> 3.3V I2C init
[SUCCESS]--> 1.8V I2C init
[SUCCESS]--> AFE control init
CLI Initialized! Enter 'help' to see a list of available commands.

$ help

afe_pwr:
  Usage: afe_pwr [0/1] [0/1]
  Description: afe_pwr [channel] [state] sets the given channel to the given state

afe_gain:
  Usage: afe_gain [0/1] [5, 10, 15, ... 40]
  Description: afe_gain [channel] [gain] sets the gain of the channel, the channel must be powered on first

$ afe_pwr 0 1
[SUCCESS]--> AFE channel 0 power enable

$ afe_gain 0 25
[SUCCESS]--> AFE ch 0 set gain to 25

$ afe_gain 1 15
[ERROR]--> AFE ch 1 must be enabled

$ afe_pwr 1 1
[SUCCESS]--> AFE channel 1 power enable

$ afe_gain 1 15
[SUCCESS]--> AFE ch 1 set gain to 15

$ afe_pwr 1 0
[SUCCESS]--> AFE channel 1 power disable

$ afe_pwr 0 0
[SUCCESS]--> AFE channel 0 power disable

```
