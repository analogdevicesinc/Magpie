## Description

A basic demo program to write mock audio WAVE files to the SD card, it runs on a MAX32666 microcontroller.


## Software

### Project Usage

Universal instructions on building, flashing, and debugging this project can be found in the **[MSDK User Guide](https://analogdevicesinc.github.io/msdk/USERGUIDE/)**.

A few parameters for the demo can be configured by altering the definitions in `demo_config.h`

### Project-Specific Build Notes

(None - this project builds as a standard example)

## Required Connections

- Insert an SD card which has previously been formatted to exFAT or FAT32 format into the FTHR SD card slot.
- The FTHR may be plugged into the Motherboard header, but it is not required. This demo can also be run on a bare FTHR with no custom Motherboard.
- Either the original FTHR or FTHR2 can be used.

## Expected Output

### Data on the SD card
A WAVE file will be created at the root of the SD card file system. You can listen to this file with an audio player
and inspect the contents with a text editor able to view files as raw hex.

The resulting wav file has 2 channels, each with a synthetic sine wave for the duration of the recording. The two sine waves have different frequencies, so you can tell the channels apart and verify that two different channels were written to the SD card.

### GPIO pins toggling to time SD card writes

- GPIO pin P0.5 (FTRH pin 6 on the short header) goes high at the start of the file write and goes low when the file is finished writing
- GPIO pin P0.12 (FTHR pin 4 on the short header) goes high at the start of a buffer write and goes low when the buffer is finished writing

These pins may be monitored with an oscilloscope. We expect P0.5 to be high for much longer than P0.12, and for there to be several high/low transitions of P0.12 during the single high/low transition of P0.5.

### LEDs blinking during execution
During code execution the onboard FTHR LEDs blink a pattern to indicate the success or failure of the audio writes to the SD card:

| Pattern | Meaning |
|---|---|
| slow green blink | all ok, audio successfully written |
| fast red blink | SD card initialization or mounting error |
| fast blue blink | SD card file opening, closing, or seeking error |
| fast green blink | SD card file writing error|

Note that if the audio file duration is long, it may take many seconds to see any LED activity. Errors usually happen quite quickly, but the slow green blink to indicate success can take some time.
