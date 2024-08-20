
BUILD_DIR = ./build/
C_LIB = $(BUILD_DIR)clib.so

SRC_DIR = ../../

OUT_DIR = ./out/

OVERRIDES_DIR = ../unit_tests/header_overrides/

C_SRC  = $(SRC_DIR)decimation_filter.c
C_SRC += $(SRC_DIR)arm_fir_decimate_fast_q31_bob.c
C_SRC += $(SRC_DIR)data_converters.c

C_SRC += $(OVERRIDES_DIR)arm_fir_decimate_init_q31.c

all: $(C_LIB)

$(C_LIB): $(BUILD_DIR)
	gcc -fPIC -shared -o $(C_LIB) $(C_SRC) -I $(SRC_DIR) -I $(OVERRIDES_DIR)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(OUT_DIR)*.wav

# usage: make plot SR=<sample rate in kHz>, example: make plot SR=96
plot: $(C_LIB)
	python filter_test_main.py $(C_LIB) $(SR)
