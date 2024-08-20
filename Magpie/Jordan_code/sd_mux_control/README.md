## Description

- This demo exercises the experimental 6-slot SD MUX hardware under development. Ask Jordan for hardware if you need a board.
- It runs on a MAX32666 FTHR2 board

## Software

### Project Usage

Universal instructions on building, flashing, and debugging this project can be found in the **[MSDK User Guide](https://analogdevicesinc.github.io/msdk/USERGUIDE/)**.

### Project-Specific Build Notes

(None - this project builds as a standard example)

## Required Connections

- Set FTHR2 jumper J4 to the 3.3v position to tie the I2C pullups to 3.3v 
- Connect the custom SD MUX PCB I2C, power, and SDHC lines to the FTHR2 (ask Jordan for help)
- Insert one or more SD cards in the 6 slots
- Connect a USB cable between the PC and the FTHR2 to power the boards

## What to Expect

This demo attempts to write a simple file to each of the 6 SD cards in the card bank and blinks the LEDs according to the pattern below:

- If a given card is not inserted then the Blue LED blinks a few times before moving on to the next card
- If there is an error mounting the card then the Red LED blinks a few times and it moves to the next card
- Errors opening or writing the file result in the Yellow (RED + GREEN) LED blinking before moving to the next card
- If writing the file is successful  then the Green LED blinks, and onto the next card

After all 6 card slots are handled, a LED pattern indicating the successes and failures of the whole run blinks forever:

- The Green LED blinks once per each card where writing was successful
- One Blue blink for each slot with no card detected
- The Red LED blinks once for each slot with mount errors
- The orange LED blinks once for each slot with file IO errors

After the demo runs through all 6 cards, you can take them out and verify that a simple file is written to the root of the file system. The file should contain the text "Hello from SD card < N >" where < N > is the slot number.
