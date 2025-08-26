# Magpie Firmware

## Brief

- This repository represents the firmware for the "Magpie" audio recorder project
- Magpie is a hardware/software collaboration between Cornell University’s K. Lisa Yang Center for Conservation Bioacoustics and Analog Device’s Central Applications Group
- It is a 2-channel Passive Acoustic Monitoring (PAM) device capable of recording high quality audio for long unsupervised durations
- The intended use is the collection of bioacoustic data (natural sounds) for conservation research purposes

## Features

- 2 high fidelity audio channels
- Sample rates [24k, 48k, 96k, 192k, and 384k]
- 24-bit or 16-bit recordings
- Adjustable analog gain from 5dB to 40dB in 5dB steps
- Bank of up to 6 SD cards for archiving audio data and logs
- Accurate Real Time Clock keeps track of time for recording schedule
- Optional GNSS module syncs RTC to accurate UTC time from satellites
- Environmental sensor for temperature, humidity, and pressure
- Fuel gauge to monitor battery status
- FLASH memory for scheduling, configuration, and logging
- BLE module communicates with configuration app and stores settings to shared FLASH memory

## Repository Organization

- The heart of this repo is a C language embedded firmware application targeted towards the main microcontroller
- There is a second firmware application just for the Nordic BLE module
- README files are sprinked throughout the directories to explain the purpose of each subdir
