#-----------------------------------------------------------------------------------------------------------------------
# Choose either RTT or the Console UART for debug/printf output stream here
# Uncomment exactly 1 of the terminal IO variant lines below
#-----------------------------------------------------------------------------------------------------------------------

TERMINAL_IO_VARIANT = TERMINAL_IO_USE_SEGGER_RTT
# TERMINAL_IO_VARIANT = TERMINAL_IO_USE_CONSOLE_UART

FIRST_SET_RTC = 1

#-----------------------------------------------------------------------------------------------------------------------
# all common makefile commands which apply to all the demos are in this makefile
#-----------------------------------------------------------------------------------------------------------------------
include ../common_makefile.mk
