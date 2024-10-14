# NOT YET WORKING! Gets stuck in an infinite loop waiting for DMA buffers to be available.

## Description

- This demo reads audio samples from the ADC at 384kHz and writes .wav files with various sample rates and bit depths to the SD cards

## Software

### Project Usage

Universal instructions on building, flashing, and debugging this project can be found in the **[MSDK User Guide](https://analogdevicesinc.github.io/msdk/USERGUIDE/)**.

### Project-Specific Build Notes

(None - this project builds as a standard example)

## Required Connections

- This demo requires the custom FTC spin hardware or similar (FTHR/FTHR2 won't work)
- An SD card must be inserted into slot 0
- SD cards should be formatted to ExFAT prior to inserting in the custom Magpie hardware
- Connect a DAP link to the Magpie board via 10 pin 0.1" header
- Connect a USB cable between the DAP link and PC.
- Open an terminal application on the PC and connect to the console UART at 115200, 8-N-1.

## Expected Output

The demo attempts to write a short audio files to the root of the SD card file system. LEDs indicate the state of the system.

During execution the LEDs show which stage we are on

- Blue while initializing the system
- Green while recording audio
- Note that some stages may finish very quickly, so you may not see the LEDs on during initialization

If there is a problem then one of the onboard LEDs rapidly blink forever as a primitive error handler

- rapid Red LED indicates an error initializing, mounting, or writing to the SD card
- rapid Blue LED indicates a problem with the ADC or DMA stream


At the end a slow Green LED blink indicates that all operations were successful

After execution is complete a few WAVE files will be created at the root of the SD card file system. You can listen to these
files with an audio player and inspect the contents with a text editor able to view files as raw hex.
