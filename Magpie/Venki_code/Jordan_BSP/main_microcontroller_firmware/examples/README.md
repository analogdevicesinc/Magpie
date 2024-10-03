# Magpie Main Microcontroller Firmware Examples

## This directory is for firmware example programs

- Each example is a contained firmware application that exercised a subset of the firmware functionality
- Examples are inteded to test, troubleshoot, and understand software modules and peripherals in relative isolation
- Each example should clearly show how the given module/peripheral is used and some expected use cases
- See the example subdirectory README files for specific instructions on how to build and flash, and what to expect

## How to add new examples

- Create a new folder for the example to go in, choose a name that is easy to interpret
- Inside the new example folder you need to add a few files:
    - `main.c`, the main application code for the example
    - `Makefile`, a mostly generic Makefile, copy this from one of the existing examples
    - `project.mk`, project specific Makefile, use this to select which libs are pulled in
    - `README.md` instructions on how to use the example
    - `.vscode`, a directory with VScode specific build files, copy this from an existing example
    - Other .h and .c files to support the example can be added if necessary

### How to configure `project.mk` for new examples

We need to point `project.mk` at the correct BSP, this can be done by adding the following to the top of `project.mk`:
```
BSP_SEARCH_DIR := $(abspath ../../)
BOARD = BSP
```

To pull in library code from the custom Magpie lib, we need to look up a few directory levels to the `lib` directory.

As an example, if we want to use the SD card module in our example, add the following to `project.mk`:
```
IPATH += ../../lib/sd_card/
VPATH += ../../lib/sd_card/
```

### How to configure `.vscode` for new examples

We just need to make sure to point VScode to the correct BSP. This can be done in `.vscode/settings.json` by ensuring the below K/V pairs are set as shown:
```
"target": "MAX32665",
"board": "BSP",
```

### For a quick way to add new examples, copy-paste the `hello_world` example directory, rename it, and update `project.mk` to select the necessary files for your example.
