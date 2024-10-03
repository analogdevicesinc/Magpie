# Magpie Main Microcontroller Firmware MSDK Overrides

## MSDK override files go here

- Some of the MSDK files do things we don't want
    - For example, the SDHC lib hardcodes the SD card interface to 1-bit mode, when we want 4-bit mode
- Files placed here are copied from the MSDK and modified by us to have the desired behavior
- We compile and link these files instead of the default MSDK version using the build system
- TODO add an explanation of how to add new files here and ensure that this version is compiled and linked
