# This file can be used to set build configuration
# variables.  These variables are defined in a file called 
# "Makefile" that is located next to this one.

# For instructions on how to use this system, see
# https://analogdevicesinc.github.io/msdk/USERGUIDE/#build-system

# **********************************************************

# If you have secure version of MCU (MAX32666), set SBT=1 to generate signed binary
# For more information on how sing process works, see
# https://www.analog.com/en/education/education-library/videos/6313214207112.html
SBT=0

# Set hardware floating-point acceleration
##MFLOAT_ABI = hard
MXC_OPTIMIZE_CFLAGS = -O2 ## note, this kills some debugging, use volatile debug vars
#PROJ_CFLAGS+=-mno-unaligned-access

# Include the CMSIS-DSP library. Set to 0 if we have a custom folder
LIB_CMSIS_DSP = 0

# Enable SDHC library. Set to 0 if we have a custom folder
LIB_SDHC = 0
# Use FatFS version R0.15
FATFS_VERSION = ff15

PROJ_LDFLAGS += -Wl,--print-memory-usage

VPATH += SD_CARD
IPATH += SD_CARD

#VPATH += RTT
#IPATH += RTT

VPATH += AFE
IPATH += AFE

VPATH += CLI
IPATH += CLI
#Use internal CLI Handler
PROJ_CFLAGS += -DUSE_CLI_LIB_IRQHANDLER

VPATH += MAX17261
IPATH += MAX17261

