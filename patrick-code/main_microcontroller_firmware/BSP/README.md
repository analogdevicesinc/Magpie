# Magpie Main Microcontroller Board Support Package (BSP)

## See the MSDK user manual for information on how the BSP system works

- https://analogdevicesinc.github.io/msdk//USERGUIDE/#custom-bsps

## GPIO pin table


| Pin  | Type | Function | Special | Description | Voltage Domain | Used By |
|------|------|----------|---------|-------------|----------------|------------|
| P0.0 | Output/high-Z | AF1 | SPIXF_SSO | FLASH memory Chip Select | 3.3V | bsp_spixf, TODO |
| P0.1 | Output/high-Z | AF1 | SPIXF_MOSI | FLASH memory MOSI | 3.3V | bsp_spixf, TODO |
| P0.2 | Input/high-Z | AF1 | SPIXF_MISO | FLASH memory MISO | 3.3V | bsp_spixf, TODO |
| P0.3| Output/high-Z | AF1 | SPIXF_SCK | FLASH memory clock | 3.3V | bsp_spixf, TODO |
| P0.4 | - |  | | | |  |
| P0.5 | Input | GPIO | Pullup | User interaction pushbutton, active LOW | 3.3V | bsp_pushbutton, TODO |
| P0.6 | Open Drain | AF1 | I2C0_SCL | 1.8V domain I2C bus clock | 1.8V | bsp_i2c, afe_control, environmental_sensor, fuel_gauge |
| P0.7 | Open Drain | AF1 | I2C0_SDA | 1.8V domain I2C bus data | 1.8V |  bsp_i2c, afe_control, environmental_sensor, fuel_gauge |
| P0.8 | Input | GPIO | Pullup | BLE enable pushbutton, active LOW | 3.3V | bsp_pushbutton, TODO |
| P0.9 | Output | AF3 | UART0_TX | Nordic UART, TX from point of view of MAX32666 | 3.3V | TODO |
| P0.10 | Input |AF3 | UART0_RX | Nordic UART, RX from point of view of MAX32666 | 3.3V | TODO |
| P0.11 | Output | GPIO |  | AFE channel 0 enable | 3.3V | afe_control |
| P0.12 | Output | GPIO |  | AFE channel 1 enable | 3.3V | afe_control |
| P0.13 | Input | GPIO | Interrupt | RTC interrupt, signal comes from the DS3231 RTC | 3.3V | real_time_clock |
| P0.14 | Open Drain | AF1 | I2C1_SCL | 3.3V domain I2C bus clock | 3.3V |  bsp_i2c, afe_control, gnss_module, real_time_clock, sd_card_bank_ctl |
| P0.15 | Open Drain | AF1 | I2C1_SDA | 3.3V domain I2C bus data | 3.3V |  bsp_i2c, afe_control, gnss_module, real_time_clock, sd_card_bank_ctl |
| P0.16 | I/O | AF2 | QSPI1_SSO | AD4630 ADC chip select (shorted to P0.4, P1.8) | 1.8V | ad4630, audio_dma |
| P0.17 | Input | AF2 | QSPI1_SDIO0 | AD4630 ADC channel 0 data (shorted to P0.26) | 1.8V | ad4630, audio_dma |
| P0.18 | - |  |  |  |  |  |
| P0.19 | Input | AF2 | QSPI1_SCK | AD4630 ADC clock (shorted to P1.11) | 1.8V | ad4630, audio_dma |
| P0.20 | Outut | GPIO |  | ADC clock enable | 1.8V | ad4630 |
| P0.21 | Output | GPIO | | ADC reset | 1.8V | ad4630 |
| P0.22 | Output | GPIO |  | ADC clock divider reset | 1.8V | ad4630 |
| P0.23 | Output | GPIO |  | GNSS module enable | 3.3V | gnss_module |
| P0.24 | Input | GPIO  | Interrupt | GNSS PSS signal | 3.3V | gnss_module |
| P0.25 | Output | AF2 | QSPI2_MOSI | AD4630 ADC SDI (config data to ADC) | 1.8V | ad4630 |
| P0.26 | Input | AF2 | QSPI2_MISO | AD4630 ADC SDO0 (config data from ADC, shorted to P0.17) | 1.8V | ad4630 |
| P0.27 | Output| AF2 | QSPI2_SCK | AD4630 ADC serial clock | 1.8V | ad4630 |
| P0.28 | Input | AF2 | UART2_RX | GNSS module UART, RX from point of view of MAX32666 | 3.3V | bsp_uart, gnss_module |
| P0.29 | Output | AF2 |UART2_TX | GNSS module UART, TX from point of view of MAX32666 | 3.3V | bsp_uart, gnss_module |
| P0.30 | Output | GPIO | | LDO enable (enable all non-uC serving LDOs) | 3.3V | TODO |
| P0.31 | Output | GPIO | | Red LED, active LOW | 3.3V | bsp_status_led, TODO |
| P1.0 | I/O | AF1 | SDHC_DAT3 | SD card D3 | 3.3V | bsp_sdhc, sd_card |
| P1.1 | I/O | AF1 | SDHC_CMD | SD card CMD | 3.3V | bsp_sdhc, sd_card |
| P1.2 | I/O | AF1 | SDHC_DAT0 | SD card D0 | 3.3V | bsp_sdhc, sd_card |
| P1.3 | Output | AF1 | SDHC_CLK | SD card CLK | 3.3V | bsp_sdhc, sd_card |
| P1.4 | I/O | AF1 | SDHC_DAT1 | SD card D1 | 3.3V | bsp_sdhc, sd_card |
| P1.5 | I/O | AF1 | SDHC_DAT2 | SD card D2 | 3.3V | bsp_sdhc, sd_card |
| P1.6 | Output | GPIO |  | ADC chip select enable | 1.8V | ad4630 |
| P1.7 | Input | AF1 | SDHC_CDN | SD card card detect (unused) | 3.3V | - |
| P1.8 | I/O | AF1 | QSPI0_SSO | AD4630 ADC chip select (shorted to P0.4, P0.16) | 1.8V | ad4630, audio_dma |
| P1.9 | Input | AF1 | QSPI0_SDIO0 | AD4630 ADC channel 1 data | 1.8V | ad4630, audio_dma |
| P1.10 | - |  |  |  |  |  |
| P1.11 | Input | AF1 | QSPI0_SCK | AD4630 ADC clock (shorted to P0.19) | 1.8V | ad4630, audio_dma |
| P1.12 | Input | AF3 | UART1_RX | Console UART, RX from point of view of MAX326676 | 1.8V | bsp_uart, board |
| P1.13 | Output | AF3 | UART1_TX | Console UART, TX from point of view of MAX326676 | 1.8V | bsp_uart, board |
| P1.14 | Output | GPIO | | Green LED, active LOW | 3.3V | bsp_status_led, TODO |
| P1.15 | Output | GPIO | | Blue LED, active LOW | 3.3V | bsp_status_led, TODO |
