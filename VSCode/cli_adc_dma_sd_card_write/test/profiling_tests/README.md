# Test to profile the time spent on decimation filtering and SD card writing

## Brief
- This test reads a csv file and generates a histogram of the times represented in the csv
- The csv is created by the MAX32666 microcontroller and written to the SD card
- The csv has one row per sample-rate/bit-depth combo
- Each row has one cell per filter/SD-write block

## Prereqs
- python, pandas, matplotlib

## To generate the histogram plot
- Flash the main C application to a MAX32666 FTHR2 board inserted into the custom motherboard stack
- Insert an ExFAT formatted SD card and let the microcontroller run
- When execution is complete, remove the SD card and excact the `block_write_times_microsec.csv` file, place the file into this directory
- `$ python block_write_hist.py`
- Observe the generated plot
