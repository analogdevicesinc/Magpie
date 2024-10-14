
BOARD_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))


SRCS += board.c
SRCS += stdio.c

SRCS += bsp_i2c.c
SRCS += bsp_pins.c
SRCS += bsp_pushbutton.c
SRCS += bsp_sdhc.c
SRCS += bsp_spi.c
SRCS += bsp_spixf.c
SRCS += bsp_status_led.c
SRCS += bsp_uart.c

MISC_DRIVERS_DIR ?= $(MAXIM_PATH)/Libraries/MiscDrivers

VPATH += $(BOARD_DIR)
IPATH += $(BOARD_DIR)

VPATH += $(MISC_DRIVERS_DIR)
IPATH += $(MISC_DRIVERS_DIR)
