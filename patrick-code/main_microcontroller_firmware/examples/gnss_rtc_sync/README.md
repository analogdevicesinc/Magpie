## Description

- This demo attempts to sync the RTC to the time reported by the GNSS module
- The RTC is a DS3231 controlled via I2C
- The GNSS module can be any module that sends at least GGA and RMC messages periodically via 3.3V UART, and has a 3.3v PPS signal
- You need to take the GNSS module/antenna outside with a clear view of the sky for it to work

## Software

### Project Usage

Universal instructions on building, flashing, and debugging this project can be found in the **[MSDK User Guide](https://analogdevicesinc.github.io/msdk/USERGUIDE/)**.

### Project-Specific Build Notes

If you want to add J-Link flashing and debugging capability need to do the following steps:
1. Create a **c:/temp** folder so that the temporary elf file can go there before J-Link flashing
2. Add to User's settings.json , **"JLINK_path":"c:/Program Files/SEGGER/JLink_Vxxx",** where JLink_Vxxx is your version of J-Link program.  You should specify the exact path where the Jlink prorgarm is.  To open User's settings.json in VS Code, do Ctrl+Shift+P then type "Open User Settings (JSON)"

Example:
```C
{
    ... various personal settings.json settings
    "JLINK_path":"c:/Program Files/SEGGER/JLink_V810k",
}
```

## Required Connections

- This demo requires the custom FTC spin hardware or similar (FTHR/FTHR2 won't work)
- A GNSS module must be connected to the optional GNSS port with an appropriate antenna attached
- Connect a JLink debug probe to the Magpie board via 10 pin 0.1" header
- Connect a USB cable between the Jink and PC.
- Open [Segger RTT viewer](https://www.segger.com/products/debug-probes/j-link/tools/rtt-viewer/) on the host PC

## Expected Output

- At startup, we set the RTC time to a default time in the past starting at year 2000
- The GNSS repeatedly attempts to get a GPS fix and sync the RTC time to the satellite time
- If the GNSS fix is unsuccessful, print and error and continue
- If the GNSS is successful, print out the syncronized RTC time, this should be exactly the real life current UTC time
