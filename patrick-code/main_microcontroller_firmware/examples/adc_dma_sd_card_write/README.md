## Description

- This demo reads audio samples from the ADC at 384kHz and writes .wav files with various sample rates and bit depths to the SD cards

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
- An SD card must be inserted into slot 0
- SD cards should be formatted to ExFAT prior to inserting in the custom Magpie hardware
- Connect a JLink debug probe to the Magpie board via 10 pin 0.1" header
- Connect a USB cable between the Jink and PC.
- Open [Segger RTT viewer](https://www.segger.com/products/debug-probes/j-link/tools/rtt-viewer/) on the host PC

## Expected Output

The demo attempts to write a short audio files to the root of the SD card file system. A stream of RTT messages along with the LEDs indicate the state of the system.

During execution the LEDs show which stage we are on

- Red->Green->Blue pattern at startup, then all LEDs off
- Solid Green while recording audio

If there is a problem then one of the onboard LEDs rapidly blink forever as a primitive error handler

- rapid Red LED indicates an error initializing, mounting, or writing to the SD card
- rapid Blue LED indicates a problem with the ADC or DMA stream
- rapid Green LED indicates a problem with I2C


At the end a slow Green LED blink indicates that all operations were successful

After execution is complete a few WAVE files will be created at the root of the SD card file system. You can listen to these
files with an audio player and inspect the contents with a text editor able to view files as raw hex.
