# This file can be used to set build configuration
# variables.  These variables are defined in a file called 
# "Makefile" that is located next to this one.

# For instructions on how to use this system, see
# https://analogdevicesinc.github.io/msdk/USERGUIDE/#build-system

# **********************************************************

# Add your config here!

# If you have secure version of MCU (MAX32666), set SBT=1 to generate signed binary
# For more information on how sing process works, see
# https://www.analog.com/en/education/education-library/videos/6313214207112.html
SBT=0

LIB_SDHC = 1

FATFS_VERSION = ff15

# PROJ_CFLAGS+=-mno-unaligned-access
# Enable TINYUSB library
#TINYUSB_DIR=C:/usb/tinyusb/
TINYUSB_DIR = C:/Users/Vkarra/Documents/GitHub/tinyusb/tinyusb/
include ./tinyusb.mk

# DEBUG=0
