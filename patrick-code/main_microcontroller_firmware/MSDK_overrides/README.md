# Magpie Main Microcontroller Firmware MSDK Overrides

## MSDK override files go here

- Some of the MSDK files do things we don't want
- Files placed here are copied from the MSDK and modified by us to have the desired behavior
- We compile and link these files instead of the default MSDK version using the build system
- ADI may fix the issues in future MSDK releases, if/when they do, we can remove the overrides here


## Explanation of code modifications

### CLI

- The CLI will not compile without a small modification because it does not support the MAX32665/32666 micro
- See [this github issue](https://github.com/analogdevicesinc/msdk/issues/1177) for an explanation
- Note that all the console UART initialization happens in our custom BSP
    - We edit `cli.c` only to allow the code to compile, not to change any behavior
    - We don't actually care about the `UART_Init()` function that gets defined in the CLI override, because this function is never called
    - This means that the UART map (MAP_A/MAP_B) does not matter
- Relevant file: `./CLI/src/cli.c`, around line 54

### SDHC

- The SDHC is hardcoded to 1-bit mode in the version of MSDK available at the time of this writing
- See [this github issue](https://github.com/analogdevicesinc/msdk/issues/1044) for an explanation
    - Note that this issue has been closed, but the fix was not rolled out to the MSDK at the time of this writing
    - When the fix is rolled out, it may require some minor config to get 4-bit mode working
- In our overriden SDHC files we hard code it to 4-bit mode
- Relevant file: `./SDHC/ff15/source/diskio.c`, around lines 169 and 212
