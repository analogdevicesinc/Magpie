## Description

- This demo reads audio samples from the ADC at 384kHz and writes .wav files with various sample rates and bit depths to the SD card
- It runs on a MAX32666 FTHR2 board
- In this directory is also the start of a unit testing framework in `./test/unit_tests/`, this is very much an early-days work in progress

## Software

### Project Usage

Universal instructions on building, flashing, and debugging this project can be found in the **[MSDK User Guide](https://analogdevicesinc.github.io/msdk/USERGUIDE/)**.

### Project-Specific Build Notes

(None - this project builds as a standard example)

## Required Connections

- This demo requires the FTHR2 to be installed in the custom Magpie hardware stack with AFE board installed
- Set FTHR2 jumper J4 to the 1.8v position to tie the I2C pullups to 1.8v (required by the motherboard, I2C is not used in this demo)
- Insert an SD card into the FTHR2 slot (format the SD card to exFAT prior to inserting)
- Connect a USB cable between the PC and the FTHR2 to power the boards

### Note about FTHR2
- The FTHR2 is used because the SD card slot is in the front, where the original FTHR has it in the back
    - This way you can take the SD card in and out without removing the FTHR from the Motherboard stack
- The FTHR2 hits the AFE board with normal length headers, we made extra long headers so the FTHR2 can clear the AFE

## Expected Output

The demo attempts to write a short audio files to the root of the SD card file system. LEDs indicate the state of the system.

During execution the LEDs show which stage we are on

- Blue while initializing the system
- Green while recording audio
- Note that some stages may finish very quickly, so you may not see the LEDs on during initialization

If there is a problem then one of the onboard FTHR2 LEDs rapidly blink forever as a primitive error handler

- rapid Red LED indicates an error initializing, mounting, or writing to the SD card
- rapid Blue LED indicates a problem with the ADC or DMA stream


At the end a slow Green LED blink indicates that all operations were successful

After execution is complete a few WAVE files will be created at the root of the SD card file system. You can listen to these
files with an audio player and inspect the contents with a text editor able to view files as raw hex.

## Quirks/limitations
- Not all sample rates are handled yet
- Of the sample rates that are handled, the FIR coefficients for 192kHz and 96kHz may not be where we want them
- I have observed it triggering the error handler on occasion, likely from a DMA overrun, investigation is required
