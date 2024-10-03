## Description

- This demo attempts to write a simple text file to each of the 6 SD cards and prints the status to the terminal

## Software

### Project Usage

Universal instructions on building, flashing, and debugging this project can be found in the **[MSDK User Guide](https://analogdevicesinc.github.io/msdk/USERGUIDE/)**.

### Project-Specific Build Notes

(None - this project builds as a standard example)

## Required Connections

- This demo requires the custom FTC spin hardware or similar (FTHR/FTHR2 won't work)
- Between 1 and 6 SD cards inserted into the card slots
- SD cards should be formatted to ExFAT prior to inserting in the custom Magpie hardware
- Connect a DAP link to the Magpie board via 10 pin 0.1" header
- Connect a USB cable between the DAP link and PC.
- Open an terminal application on the PC and connect to the console UART at 115200, 8-N-1.

## Expected Output

- As the demo attempts to write a test file to each SD card brief messages are printed showing the success or failure of the various steps
- At the end a summary of successes and failures is printed out

