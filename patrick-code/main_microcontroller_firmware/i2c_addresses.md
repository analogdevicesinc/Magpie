# I2C Address List

## 3.3V I2C Bus Addresses

| 7-bit address | Ref Des | Part num | Description | Used by |
|---|---|---|---|---|
| 0x20 | U13 (on SD card board) | MAX7312 | GPIO expander for SD MUX control | sd_card_bank_ctl |
| 0x68 | U19 | DS3231 | Real Time Clock |  real_time_clock |
| 0x71 | U3 | TPS22994 | Ti load switch |  afe_control |

## 1.8V I2C Bus Addresses

| 7-bit address | Ref Des | Part num | Description | Used by |
|---|---|---|---|---|
| 0x36 | U18 | MAX17261 | Fuel gauge | TODO |
| 0x4E | U6 | MAX14662 | AFE channel 1 gain | afe_control |
| 0x4F | U4 | MAX14662 | AFE channel 0 gain | afe_control |
| 0x77 | U17 | BME688 | Environmental sensor | TODO |
