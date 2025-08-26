#-----------------------------------------------------------------------------------------------------------------------
# Only the Console UART works with the CLI at the moment, you can't use RTT for this demo
#-----------------------------------------------------------------------------------------------------------------------

TERMINAL_IO_VARIANT = TERMINAL_IO_USE_CONSOLE_UART

#-----------------------------------------------------------------------------------------------------------------------
# Use the CLI lib in ./MSDK_overrides/CLI/ instead of the files supplied by the MSDK, this is because the MSDK
# version does not have the right definitions for MAX32666, even through it works when you add them
#-----------------------------------------------------------------------------------------------------------------------
LIB_CLI = 1

include ../../MSDK_overrides/CLI/CLI.mk

IPATH += ../../MSDK_overrides/CLI/inc
VPATH += ../../MSDK_overrides/CLI/src

#-----------------------------------------------------------------------------------------------------------------------
# all common makefile commands which apply to all the demos are in this makefile
#-----------------------------------------------------------------------------------------------------------------------
include ../common_makefile.mk
