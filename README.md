# SWIFTV2_DSS
## Introduction
The code within this repository consists of multiple firmware projects that implement and/or vet different functional requirements of the SWIFTV2 Digital Subsystems (DSS). SWIFTV2 is a design collaboration between Cornell University’s K. Lisa Yang Center for Conservation Bioacoustics and Analog Device’s Central Applications Group using ADI’s microcontroller, A2D, and other analog products to power Cornell’s second generation SWIFT autonomous recording units hardware/firmware platform.  This repository represents a good portion of Analog's firmware contribution to the project.

There are two Eclipse based programs and Two Matlab codes please find the description below

SwiftV2_DSS:

This program is targeted towards MAX32666FTHR which talks to two other designs Mother board and Daughter Board. The Mother board houses power supplies, oscillators and AD4630 ADC where as the daughter board has all the Mics, OP-AMPS used for recording data. The code initiates SPI communication between MAX32666FTHR and AD4630 reading the recorded data from Mics. This data is simultaneously transferred to PC using USB on the FTHR. Please press SW1 on FTHR and start speaking (Blue LED is ON). We only have two seconds to hold the button. Donot hold it for long because it fills up the buffer completely. As soon as the SW1 is released the data is transferred to PC through USB (Red LED will be ON) Please check the board.c file in ${MSDKPath}\Libraries\Boards\MAX32665${BoardName}\Source path to learn switch and LED pins for specific board.

SwiftV2_DSS_Press_Start:

This program is targeted towards MAX32666FTHR which talks to two other designs Mother board and Daughter Board. The Mother board houses power supplies, oscillators and AD4630 ADC where as the daughter board has all the Mics, OP-AMPS used for recording data. The code initiates SPI communication between MAX32666FTHR and AD4630 reading the recorded data from Mics. This data is simultaneously transferred to PC using USB on the FTHR. Please press SW1 on FTHR and start speaking (Blue LED is ON). Now the data recording time starts. Once the buffer is full the data is automatically moved to PC(RED LED is ON while data is being transferred to PC). Please check the board.c file in ${MSDKPath}\Libraries\Boards\MAX32665${BoardName}\Source path to learn switch and LED pins for specific board.

SwiftV2_DSS_48KHz_384KHz:

This program is targeted towards MAX32666FTHR which talks to two other designs Mother board and Daughter Board.  The Mother board houses power supplies, oscillators and AD4630 ADC where as the daughter board has all the Mics, OP-AMPS used for recording data.  The code initiates SPI communication between MAX32666FTHR and AD4630 reading the recorded data from Mics. This data is simultaneously transferred to PC using USB on the FTHR.
Please press SW1 on FTHR and start speaking (Blue LED is ON).  Now the data recording time starts.  Once the buffer is full the data is automatically moved to PC(RED LED is ON while data is being transferred to PC).  This program works for both 48KHz really well. This code has baremetal implementation which helps in reducing the latency between interrupt and SPI routine.
 Please check the board.c file in ${MSDKPath}\Libraries\Boards\MAX32665${BoardName}\Source path to learn switch and LED pins for specific board.


Matlab Parse_SPI_fft.m:

Please use an application like tera term or putty to log the USB data into a text file using the SwiftV2_DSS example. Now create a new folder and save the logged text file and also the Parse_SPI_fft.m file. This matlab code reads the log.txt file and performs the FFT on the data and finally plays the recording

Matlab Serial-USB:

This code reads the data from the COM port and plays it back
