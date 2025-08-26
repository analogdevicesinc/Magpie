
BUILD_DIR = ./build/
C_LIB = $(BUILD_DIR)clib.so

OUT_DIR = ./out/

C_SRC  = ../../../lib/audio/decimation_filter.c
C_SRC += ../../../lib/utils/data_converters.c

INCLUDES  = -I ../../../lib/audio/
INCLUDES += -I ../../../lib/utils/
INCLUDES += -I ../../unit_tests/header_overrides/

all: $(C_LIB)

$(C_LIB): $(BUILD_DIR)
	gcc -fPIC -shared -o $(C_LIB) $(C_SRC) $(INCLUDES)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)
	mkdir $(OUT_DIR)

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(OUT_DIR)*.wav

# usage: make plot SR=<sample rate in kHz> PB=<n>, example: make plot SR=96 PB=1
plot: $(C_LIB)
	python filter_test_main.py $(C_LIB) $(SR) $(PB)
