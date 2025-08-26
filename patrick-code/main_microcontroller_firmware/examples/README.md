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
    - `Makefile`, this just imports the top level MSDK supplied Makefile, copy this from one of the existing examples
    - `project.mk`, project specific Makefile, use this to choose RTT or the Console UART for terminal/debug IO
    - `README.md` instructions on how to use the example
    - `.vscode`, a directory with VScode specific build files, copy this from an existing example
    - Other .h and .c files to support the example can be added if necessary

### Common Makefile with config options shared by all the demos

The Makefile `common_makefile.mk` at the root of the examples directory is shared by all of the examples. This includes common configuration options that all the demos use.

It points to the BSP, lib code, and other source files so that we can build the demos, and sets some common compiler flags and such.

### How to configure `project.mk` for new examples

The primary task of the `project.mk` files in each demo folder is to choose whether to use RTT or the Console UART for terminal feedback.

Open the `project.mk` file for your demo of interest and uncomment __one__ of the lines shown below to select the terminal output.
```
#-----------------------------------------------------------------------------------------------------------------------
# Choose either RTT or the Console UART for debug/printf output stream here
# Uncomment exactly 1 of the terminal IO variant lines below
#-----------------------------------------------------------------------------------------------------------------------

TERMINAL_IO_VARIANT = TERMINAL_IO_USE_SEGGER_RTT
# TERMINAL_IO_VARIANT = TERMINAL_IO_USE_CONSOLE_UART

```
If you're using a JLink, then RTT is a nicer experience. If you only have a DAPLink, then you must choose the UART.

The CLI examples at this time do not work with RTT, so those are UART-only.

### For a quick way to add new examples, copy-paste the `hello_world` example directory, rename it, and update `project.mk` with the desired RTT/UART files for your example.
