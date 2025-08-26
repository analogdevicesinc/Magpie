# Magpie Main Microcontroller Firmware

## This ADI MAX32666 MSDK project makes up the bulk of the Magpie firmware

### For general information about the MSDK see the [Analog Devices MSDK user guide](https://analogdevicesinc.github.io/msdk/USERGUIDE/)

### See the [VSCode section](https://analogdevicesinc.github.io/msdk/USERGUIDE/#visual-studio-code) for specific instructions on how to set up and run the code with VSCode 

### Project organization

- `./app`/ contains the main user application entry point, currently this is only a skeleton, the main application has not been started yet as of this writing
- `./BSP/` contains a custom MAX32666 Board Support Package, this has the startup routine, pin definitions, and init/de-init functions for most of the major peripherals
- `./examples/` contains example applications, simplified and self-contained demos of various features
- `./lib/` contains custom library files for peripherals and algorithms
- `./MSDK_overrides/` contains some slightly modified MSDK files, these are modified to suit our desired behavior. Future updates to the MSDK may make this directory unnecessary
- `./third_party/` contains miscellaneous third party files used by the system
- `./test/` contains various test code

For each subdirectory look for `README.md` files for more information
