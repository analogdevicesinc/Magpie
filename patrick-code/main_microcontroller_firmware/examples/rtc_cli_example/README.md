## Description

- This demo sets and displays the RTC time via CLI
- The RTC is a DS3231 controlled via I2C
- Milliseconds are not used in this demo, seconds are the finest unit of time handled

## Software

### Project Usage

Universal instructions on building, flashing, and debugging this project can be found in the **[MSDK User Guide](https://analogdevicesinc.github.io/msdk/USERGUIDE/)**.

### Project-Specific Build Notes

(None - this project builds as a standard example)

## Required Connections

- This demo requires the custom FTC spin hardware or similar (FTHR/FTHR2 won't work)
- A DAP interface is used for programming and printf UART output
- A terminal application on your PC connected to the DAP UART port with settings 115200, 8-None-1-None

## Expected Output

An example Console UART session is shown below:

```
*********** RTC CLI example ***********

[SUCCESS]--> I2C init
[SUCCESS]--> RTC init

RTC time at startup: 20241031_152446

CLI Initialized! Enter 'help' to see a list of available commands.

$ help

set_rtc:
  Usage: [set_rtc] [year] [month] [day] [hour] [min] [sec]
  Description: Sets Datetime to the given UTC time, hour is 0-23. min is 0-59,sec is 0-59

show_rtc:
  Usage: show_rtc
  Description: Prints the current RTC time to the terminal

$ show_rtc
20241031_152455

$ set_rtc 2099 5 6 7 8 9

$ show_rtc
20990506_070813

$ show_rtc
20990506_070823

$ set_rtc 2024 10 31 18 25 30

$ show_rtc
20241031_182532

$ show_rtc
20241031_182538
```
