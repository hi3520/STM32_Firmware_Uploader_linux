# Custom OTA Firmware Update Program with STM32 MCU via UART Interface

This is a program that is designed to update the firmware of an STM32 microcontroller unit (MCU) using a Linux platform. It is a customized OTA, which means that it can perform over-the-air updates. To use it, you need to have a firmware file for the STM32 MCU in the .bin format. The program reads this file and updates the firmware via the UART interface.

To make this work, you will also need to add a custom bootloader to your STM32 MCU. You can find a suitable bootloader for your MCU at this link: https://github.com/RTX93/STM32F030RCT6_BL.git. This bootloader will enable you to perform firmware updates via the UART interface. Once you have the bootloader installed, you can use this program to update the firmware of your STM32 MCU easily.

Run binary
--
./FOTA /dev/tty* mybinary.bin

retun value 0 -> Sucess 

retun value 1 -> Time Out (Target Time Out) 

retun value 2 -> Failed to Open File

retun value 3 -> Failed to Open Uart Port
 

