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

# use the custom BSP located in ./BSP/
BSP_SEARCH_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
BOARD = BSP

PROJ_CFLAGS+=-mno-unaligned-access

MXC_OPTIMIZE_CFLAGS = -O2

PROJ_LDFLAGS += -Wl,--print-memory-usage

# do manual pin config, don't use the pin constants in msdk/Libraries/PeriphDrivers/Source/SYS/pins_me14.c
# this means we need to explicitly set up all the pins for the peripherals we use
PROJ_CFLAGS += -DMSDK_NO_GPIO_CLK_INIT

# Use the SDHC lib in ./MSDK_overrides/SDHC/ instead of the files supplied by the MSKD, this is because the MSKD
# version is hardcoded to 1-bit mode, and we want 4-bit mode
FATFS_VERSION = ff15
SDHC_DRIVER_DIR = ./MSDK_overrides/SDHC/

include ./MSDK_overrides/SDHC/sdhc.mk
include ./MSDK_overrides/SDHC/ff15/fat32.mk

IPATH += ./MSDK_overrides/SDHC/Include/
IPATH += ./MSDK_overrides/SDHC/ff15/source/
IPATH += ./MSDK_overrides/SDHC/ff15/source/conf/

LIB_CMSIS_DSP = 1

# 3rd party GNSS parsing lib
IPATH += ./third_party/minmea/
SRCS += ./third_party/minmea/minmea.c
PROJ_CFLAGS += -Dtimegm=mktime # needed for the minmea lib

IPATH += ./third_party/RTT
VPATH += ./third_party/RTT

IPATH += ./lib/audio/
VPATH += ./lib/audio/

IPATH += ./lib/utils/
VPATH += ./lib/utils/

VPATH += ./app/
