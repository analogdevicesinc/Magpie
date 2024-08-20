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

PROJ_CFLAGS+=-mno-unaligned-access

DEBUG=1

PROJ_LDFLAGS += -Wl,--print-memory-usage


VPATH += RTT
IPATH += RTT

#SRCS += SEGGER_RTT_ASM_ARMv7M.S

#SRCS += SEGGER_RTT.c
#SRCS += SEGGER_RTT_printf.c
#SRCS += SEGGER_RTT_Syscalls_GCC.c
#SRCS += SEGGER_RTT_Syscalls_IAR.c
#SRCS += SEGGER_RTT_Syscalls_KEIL.c
#SRCS += SEGGER_RTT_Syscalls_SES.c




