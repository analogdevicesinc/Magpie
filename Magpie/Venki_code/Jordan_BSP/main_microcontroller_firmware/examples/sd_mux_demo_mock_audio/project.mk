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

# use the BSP defined in the root of the main firmware location
BSP_SEARCH_DIR := $(abspath ../../)
BOARD = BSP

PROJ_CFLAGS+=-mno-unaligned-access

# do manual pin config, don't use the pin constants in msdk/Libraries/PeriphDrivers/Source/SYS/pins_me14.c
# this means we need to explicitly set up all the pins for the peripherals we use
PROJ_CFLAGS += -DMSDK_NO_GPIO_CLK_INIT

FATFS_VERSION = ff15
SDHC_DRIVER_DIR = ../../MSDK_overrides/SDHC/

include ../../MSDK_overrides/SDHC/sdhc.mk
include ../../MSDK_overrides/SDHC/ff15/fat32.mk

IPATH += ../../MSDK_overrides/SDHC/Include/
IPATH += ../../MSDK_overrides/SDHC/ff15/source/
IPATH += ../../MSDK_overrides/SDHC/ff15/source/conf/

IPATH += ../../lib/sd_card/
VPATH += ../../lib/sd_card/

PROJ_LDFLAGS += -Wl,--print-memory-usage

VPATH += RTT
IPATH += RTT