## Description

This is a MAX17261 driver that collects coulomb data and other register information


## Software

### Project Usage

Universal instructions on building, flashing, and debugging this project can be found in the **[MSDK User Guide](https://analogdevicesinc.github.io/msdk/USERGUIDE/)**.

### Project-Specific Build Notes

(None - this project builds as a standard example)

## Required Connections

-   Connect a USB cable between the PC and the CN2 (USB/PWR) connector.
-   Open an terminal application on the PC and connect to the EV kit's console UART at 115200, 8-N-1.
-   Power supply capable of generating 3.6Volts connected to P12 of the Mother board

## Expected Output

The VBUS UART of the device will output these messages:
/================================/
Data poll count is 584
/================================/
State Of Charge = 5
voltage per cell of the batterypack = 0.000000
AVerage of VCell register readings = 3.589922
Current register = -4.687500
AvgCurrent register = -4.062500
QResidual register = 0.000000
QH register(coulomb count) = 0.004551

/================================/
Data poll count is 585
/================================/
State Of Charge = 5
voltage per cell of the batterypack = 0.000000
AVerage of VCell register readings = 3.589922
Current register = -4.687500
AvgCurrent register = -4.062500
QResidual register = 0.000000
QH register(coulomb count) = 0.004551

/================================/
Data poll count is 586
/================================/
State Of Charge = 5
voltage per cell of the batterypack = 0.000000
AVerage of VCell register readings = 3.589922
Current register = -4.687500
AvgCurrent register = -4.062500
QResidual register = 0.000000
QH register(coulomb count) = 0.004551