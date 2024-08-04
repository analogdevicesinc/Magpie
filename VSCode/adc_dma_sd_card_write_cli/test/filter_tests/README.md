# Test scripts to verify the operation of the Decimation Filters

## Brief

- Python scripts call compiled C code and exercise the decimation filters
- The necessary C functions are compiled by this script, and are then called by the Python ctypes module
- Synthetic test audio data is fed into the filters to see how they do at rejecting out of band signals
- The results are plotted, and WAV files of the filtered test data are generated so they can be checked in Raven, Audacity, or similar programs

## Prereqs

- Python 3
    - ctypes
    - matplotlib
    - numpy
    - scipy
- GNU Make to run the Makefile
    - You could run the commands manually as well

## To run the script and generate the outputs

- `$ make plot SD=<desired sample rate in kHz>`
    - Example for 48k: `$ make plot SR=48`
    - Valid sample rates are `[16, 24, 32, 48, 96, 192]`
- Observe the visual plot, and check the resulting WAV file in `./out/` if desired
- `$ make clean` to delete the compiled C library and any output WAV files
