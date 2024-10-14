## Description

- This demo attempts to sync the RTC to the time reported by the GNSS module
- The RTC is a DS3231 controlled via I2C
- The GNSS module can be any module that sends at least GGA and RMC messages periodically via 3.3V UART, and has a 3.3v PPS signal
- You need to take the GNSS module/antenna outside with a clear view of the sky for it to work

## Software

### Project Usage

Universal instructions on building, flashing, and debugging this project can be found in the **[MSDK User Guide](https://analogdevicesinc.github.io/msdk/USERGUIDE/)**.

### Project-Specific Build Notes

(None - this project builds as a standard example)

## Required Connections

- This demo requires the custom FTC spin hardware or similar (FTHR/FTHR2 won't work)
- A GNSS module must be connected to the optional GNSS port with an appropriate antenna attached
- A DAP interface is used for programming and printf UART output
- A terminal application on your PC connected to the DAP UART port with settings 115200, 8-None-1-None

## Expected Output

- At startup, we set the RTC time to a default time in the past starting at year 2000
- The GNSS repeatedly attempts to get a GPS fix and sync the RTC time to the satellite time
- If the GNSS fix is unsuccessful, print and error and continue
- If the GNSS is successful, print out the syncronized RTC time, this should be exactly the real life current UTC time
