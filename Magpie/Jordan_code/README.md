# Magpie Code Snippets

## Brief

- This repo is a dumping ground for code snippets related to the ADI and  K. Lisa Yang Center for Conservation Bioacoustics collaboration "Magpie" audio recorder project
- These snippets are investigations and demos for various peripherals and features, not finished projects
- Most snippets are written to run on MAX32666 microcontrollers, but some may use other micros
    - This is usually becuase the right pins needed to run the demo are not broken out on the MAX32666 FTHR board
- It is intended that eventually everything runs on the MAX32666
- Some demos require specialized Magpie hardware

## Intended Audience

- ADI and Yang Center engineers working on the Magpie
- It's probably not very interesting to anyone else

## Repo Structure

- Individual snippets exercising a specific peripheral or feature each get their own directories
- Each directory should have all you need to flash the Microcontroller and run the demo
- The demos are typically either an ADI MSDK project, or an STM32CubeIDE project
