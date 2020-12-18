<<<<<<< HEAD
STM32 OTA via UART

Compiling For Android 
--
1.Goto Android root directory 

2.source build/envsetup.sh 

3.lunch target 

4.mmm your_source_directory 

	eg. mmm STM32_Firmware_Uploader
	

Run binary
--
./fota_mc /dev/tty* mybinary.bin

retun value 0 -> Sucess 

retun value 1 -> Time Out (Target Time Out) 

retun value 2 -> Failed to Open File

retun value 3 -> Failed to Open Uart Port
 
=======
# STM32_Firmware_uploader_linux
>>>>>>> dbfc8ed1cf6a1cdb2378f49f45e16f95908c085b
