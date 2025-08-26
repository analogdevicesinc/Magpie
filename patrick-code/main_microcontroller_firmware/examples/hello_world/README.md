## Description

A basic getting started program.

This version of Hello_World prints an incrementing count via RTT and toggles a GPIO (LED0) once every 500 ms.

Please check the BSP files in `../../BSP/`.


## Software

### Project Usage

Universal instructions on building, flashing, and debugging this project can be found in the **[MSDK User Guide](https://analogdevicesinc.github.io/msdk/USERGUIDE/)**.

### Project-Specific Build Notes

If you want to add J-Link flashing and debugging capability need to do the following steps:
1. Create a **c:/temp** folder so that the temporary elf file can go there before J-Link flashing
2. Add to User's settings.json , **"JLINK_path":"c:/Program Files/SEGGER/JLink_Vxxx",** where JLink_Vxxx is your version of J-Link program.  You should specify the exact path where the Jlink prorgarm is.  To open User's settings.json in VS Code, do Ctrl+Shift+P then type "Open User Settings (JSON)"

Example:
```C
{
    ... various personal settings.json settings
    "JLINK_path":"c:/Program Files/SEGGER/JLink_V810k",
}
```

## Required Connections

- Connect a JLink debug probe to the Magpie board via 10 pin 0.1" header
- Connect a USB cable between the Jink and PC.
- Open [Segger RTT viewer](https://www.segger.com/products/debug-probes/j-link/tools/rtt-viewer/) on the host PC

## Expected Output

The RTT viewer will output these messages:

```
***********Hello Magpie!***********

Count = 0
Count = 1
Count = 2
Count = 3
...
```
