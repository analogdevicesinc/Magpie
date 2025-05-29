echo
echo "Setting MAXIM_PATH environment variable..."
export MAXIM_PATH=/Users/robertadams/MaximSDK666
echo $MAXIM_PATH
echo
echo
echo "Setting MAXIM_PATH environment variable..."
export MAXIM_PATH=/Users/robertadams/MaximSDK666
echo $MAXIM_PATH
echo

# this came from setenv.sh in SDK dir
export FIRMWARE_PATH=$MAXIM_PATH/Libraries

export TOOLCHAIN_PATH=$MAXIM_PATH/Tools

export LD_LIBRARY_PATH=$TOOLCHAIN_PATH/OpenOCD:$LD_LIBRARY_PATH

#Default SBT device is ME13
export MAXIM_SBT_DEVICE="MAX32570"
export MAXIM_SBT_DIR=$TOOLCHAIN_PATH/SBT

# Add Arm GCC to PATH
ARM_GCC_ROOT=$MAXIM_PATH/Tools/GNUTools
ARM_GCC_VERSION=10.3
ARM_GCC_BIN=$ARM_GCC_ROOT/$ARM_GCC_VERSION/bin
export PATH=$ARM_GCC_BIN:$PATH

# Add deprecated legacy RISC-V GCC to PATH so we don't break
# old projects if it's not installed.
export PATH=$MAXIM_PATH/Tools/xPack/riscv-none-embed-gcc/10.2.0-1.2/bin:$PATH

# Add xPack RISC-V GCC to PATH
XPACK_GCC_ROOT=$MAXIM_PATH/Tools/xPack/riscv-none-elf-gcc
XPACK_GCC_VERSION=12.2.0-3.1
XPACK_GCC_BIN=$XPACK_GCC_ROOT/$XPACK_GCC_VERSION/bin
export PATH=$XPACK_GCC_BIN:$PATH

# Add OpenOCD to the PATH
OPENOCD_ROOT=$MAXIM_PATH/Tools/OpenOCD
export PATH=$OPENOCD_ROOT:$PATH

# Add MSDK's version of Make to the PATH
MSDK_MAKE_ROOT=$MAXIM_PATH/Tools/GNUTools/Make
export PATH=$MSDK_MAKE_ROOT:$PATH

echo $PATH


rm -rf build
mkdir build

# this came from runnng make -n on command line

echo -  CC    /Users/robertadams/MaximSDK666/Libraries/MiscDrivers/ExtMemory/mx25.c
arm-none-eabi-gcc -mthumb -mcpu=cortex-m4 -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -Wa,-mimplicit-it=thumb -Og -ffunction-sections -fdata-sections -fsingle-precision-constant -Wall -Wno-format -Wdouble-promotion -MD -c -fno-isolate-erroneous-paths-dereference -DTARGET=MAX32665 -DMAX32665 -DTARGET_REV=0x4131 -g3 -ggdb -DDEBUG -Wall  -DMXC_ASSERT_ENABLE -fdiagnostics-color=always -DLIB_BOARD -DEXT_FLASH_MX25 -Werror=implicit-function-declaration -I. -Iinclude -I/Users/robertadams/MaximSDK666/Libraries/Boards/MAX32665/EvKit_V1/Include -I/Users/robertadams/MaximSDK666/Libraries/MiscDrivers -I/Users/robertadams/MaximSDK666/Libraries/MiscDrivers/LED -I/Users/robertadams/MaximSDK666/Libraries/MiscDrivers/PushButton -I/Users/robertadams/MaximSDK666/Libraries/MiscDrivers/ExtMemory -I/Users/robertadams/MaximSDK666/Libraries/MiscDrivers/Display -I/Users/robertadams/MaximSDK666/Libraries/MiscDrivers/CODEC -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Include/MAX32665/ -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/ADC -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/CORE1 -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/DMA -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/FLC -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/GPIO -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/HTMR -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/I2C -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/ICC -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/LP -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/OWM -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/PT -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/RPU -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/RTC -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/SDHC -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/SEMA -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/SIMO -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/SPI -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/SPIXF -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/SPIXR -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/SRCC -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/TMR -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/TPU -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/TRNG -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/UART -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/WDT -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/WUT -I/Users/robertadams/MaximSDK666/Libraries/CMSIS/Device/Maxim/MAX32665/Include -I/Users/robertadams/MaximSDK666/Libraries/CMSIS/5.9.0/Core/Include -o /Users/robertadams/eclipse-workspace/spi_gpio/build/mx25.o /Users/robertadams/MaximSDK666/Libraries/MiscDrivers/ExtMemory/mx25.c
echo -  CC    /Users/robertadams/MaximSDK666/Libraries/MiscDrivers/Display/sharp_mip.c
arm-none-eabi-gcc -mthumb -mcpu=cortex-m4 -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -Wa,-mimplicit-it=thumb -Og -ffunction-sections -fdata-sections -fsingle-precision-constant -Wall -Wno-format -Wdouble-promotion -MD -c -fno-isolate-erroneous-paths-dereference -DTARGET=MAX32665 -DMAX32665 -DTARGET_REV=0x4131 -g3 -ggdb -DDEBUG -Wall  -DMXC_ASSERT_ENABLE -fdiagnostics-color=always -DLIB_BOARD -DEXT_FLASH_MX25 -Werror=implicit-function-declaration -I. -Iinclude -I/Users/robertadams/MaximSDK666/Libraries/Boards/MAX32665/EvKit_V1/Include -I/Users/robertadams/MaximSDK666/Libraries/MiscDrivers -I/Users/robertadams/MaximSDK666/Libraries/MiscDrivers/LED -I/Users/robertadams/MaximSDK666/Libraries/MiscDrivers/PushButton -I/Users/robertadams/MaximSDK666/Libraries/MiscDrivers/ExtMemory -I/Users/robertadams/MaximSDK666/Libraries/MiscDrivers/Display -I/Users/robertadams/MaximSDK666/Libraries/MiscDrivers/CODEC -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Include/MAX32665/ -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/ADC -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/CORE1 -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/DMA -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/FLC -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/GPIO -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/HTMR -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/I2C -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/ICC -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/LP -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/OWM -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/PT -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/RPU -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/RTC -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/SDHC -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/SEMA -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/SIMO -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/SPI -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/SPIXF -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/SPIXR -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/SRCC -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/TMR -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/TPU -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/TRNG -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/UART -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/WDT -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/WUT -I/Users/robertadams/MaximSDK666/Libraries/CMSIS/Device/Maxim/MAX32665/Include -I/Users/robertadams/MaximSDK666/Libraries/CMSIS/5.9.0/Core/Include -o /Users/robertadams/eclipse-workspace/spi_gpio/build/sharp_mip.o /Users/robertadams/MaximSDK666/Libraries/MiscDrivers/Display/sharp_mip.c
echo -  CC    /Users/robertadams/MaximSDK666/Libraries/MiscDrivers/CODEC/max9867.c
arm-none-eabi-gcc -mthumb -mcpu=cortex-m4 -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -Wa,-mimplicit-it=thumb -Og -ffunction-sections -fdata-sections -fsingle-precision-constant -Wall -Wno-format -Wdouble-promotion -MD -c -fno-isolate-erroneous-paths-dereference -DTARGET=MAX32665 -DMAX32665 -DTARGET_REV=0x4131 -g3 -ggdb -DDEBUG -Wall  -DMXC_ASSERT_ENABLE -fdiagnostics-color=always -DLIB_BOARD -DEXT_FLASH_MX25 -Werror=implicit-function-declaration -I. -Iinclude -I/Users/robertadams/MaximSDK666/Libraries/Boards/MAX32665/EvKit_V1/Include -I/Users/robertadams/MaximSDK666/Libraries/MiscDrivers -I/Users/robertadams/MaximSDK666/Libraries/MiscDrivers/LED -I/Users/robertadams/MaximSDK666/Libraries/MiscDrivers/PushButton -I/Users/robertadams/MaximSDK666/Libraries/MiscDrivers/ExtMemory -I/Users/robertadams/MaximSDK666/Libraries/MiscDrivers/Display -I/Users/robertadams/MaximSDK666/Libraries/MiscDrivers/CODEC -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Include/MAX32665/ -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/ADC -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/CORE1 -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/DMA -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/FLC -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/GPIO -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/HTMR -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/I2C -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/ICC -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/LP -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/OWM -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/PT -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/RPU -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/RTC -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/SDHC -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/SEMA -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/SIMO -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/SPI -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/SPIXF -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/SPIXR -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/SRCC -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/TMR -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/TPU -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/TRNG -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/UART -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/WDT -I/Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/Source/WUT -I/Users/robertadams/MaximSDK666/Libraries/CMSIS/Device/Maxim/MAX32665/Include -I/Users/robertadams/MaximSDK666/Libraries/CMSIS/5.9.0/Core/Include -o /Users/robertadams/eclipse-workspace/spi_gpio/build/max9867.o /Users/robertadams/MaximSDK666/Libraries/MiscDrivers/CODEC/max9867.c
echo -T /Users/robertadams/MaximSDK666/Libraries/CMSIS/Device/Maxim/MAX32665/Source/GCC/max32665.ld                                       \
      --entry Reset_Handler                                                       \
      -mthumb -mcpu=cortex-m4 -mfloat-abi=softfp -mfpu=fpv4-sp-d16 -Xlinker --gc-sections -Xlinker -Map -Xlinker /Users/robertadams/eclipse-workspace/spi_gpio/build/spi_gpio.map  -L/Users/robertadams/MaximSDK666/Libraries/CMSIS/Device/Maxim/MAX32665/Source/GCC                                             \
      -o /Users/robertadams/eclipse-workspace/spi_gpio/build/spi_gpio.elf   \
      /Users/robertadams/eclipse-workspace/spi_gpio/build/main.o /Users/robertadams/eclipse-workspace/spi_gpio/build/board.o /Users/robertadams/eclipse-workspace/spi_gpio/build/stdio.o /Users/robertadams/eclipse-workspace/spi_gpio/build/led.o /Users/robertadams/eclipse-workspace/spi_gpio/build/mx25.o /Users/robertadams/eclipse-workspace/spi_gpio/build/pb.o /Users/robertadams/eclipse-workspace/spi_gpio/build/sharp_mip.o /Users/robertadams/eclipse-workspace/spi_gpio/build/max9867.o /Users/robertadams/eclipse-workspace/spi_gpio/build/startup_max32665.o /Users/robertadams/eclipse-workspace/spi_gpio/build/backup_max32665.o /Users/robertadams/eclipse-workspace/spi_gpio/build/heap.o /Users/robertadams/eclipse-workspace/spi_gpio/build/system_max32665.o                                    \
      -Xlinker --start-group                                                 \
      /Users/robertadams/MaximSDK666/Libraries/PeriphDrivers/bin/MAX32665/softfp/libPeriphDriver_softfp.a                                    \
                                                                 \
      -lc -lm -lnosys                                                            \
      -Xlinker --end-group                                                   \
      > /Users/robertadams/eclipse-workspace/spi_gpio/build/ln_args.txt
echo -  LD    /Users/robertadams/eclipse-workspace/spi_gpio/build/spi_gpio.elf
arm-none-eabi-gcc @/Users/robertadams/eclipse-workspace/spi_gpio/build/ln_args.txt
arm-none-eabi-size --format=berkeley /Users/robertadams/eclipse-workspace/spi_gpio/build/spi_gpio.elf
