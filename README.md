STM32 OTA via UART

Run binary
--
./FOTA /dev/tty* mybinary.bin

retun value 0 -> Sucess 

retun value 1 -> Time Out (Target Time Out) 

retun value 2 -> Failed to Open File

retun value 3 -> Failed to Open Uart Port
 
STM32 bootloader
--

https://github.com/RTX93/STM32F030RCT6_BL.git

