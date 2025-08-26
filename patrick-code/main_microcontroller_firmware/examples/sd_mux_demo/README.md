## Description

- This demo attempts to write a simple text file to each of the 6 SD cards and prints the status via RTT

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
- Between 1 and 6 SD cards inserted into the card slots
- SD cards should be formatted to ExFAT prior to inserting in the custom Magpie hardware
- Connect a JLink debug probe to the Magpie board via 10 pin 0.1" header
- Connect a USB cable between the Jink and PC.
- Open [Segger RTT viewer](https://www.segger.com/products/debug-probes/j-link/tools/rtt-viewer/) on the host PC

## Expected Output

- As the demo attempts to write a test file to each SD card brief messages are printed showing the success or failure of the various steps
- At the end a summary of successes and failures is printed out
