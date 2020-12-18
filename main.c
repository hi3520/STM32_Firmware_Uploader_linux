/*
 * File:    main.c
 * Author:  Raveen
 *
 */

/*
retun value 0 -> Sucess
retun value 1 -> Time Out (Target Time Out)
retun value 2 -> Failed to Open File
retun value 3 -> Failed to Open Uart Port
*/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>   /* File Control Definitions           */
#include <termios.h> /* POSIX Terminal Control Definitions */
#include <unistd.h>  /* UNIX Standard Definitions 	   */ 
#include <errno.h>   /* ERROR Number Definitions           */
#include <stddef.h>

#include "checksum.h"

#define ChunkLeng 512
/*Proto Type */
int open_uart(char ucport[]);
int close_uart(int ifile);
int send_binary(char cpFile[]);
void *read_uart(void *var);
void *fota_process();
uint16_t ConvertBig162Little16( uint16_t ui16InData );

int imyuart = 0;
int iRetVal = -1;
uint8_t ui8read_byte = 0;
char read_buf [256];
char *cpPort;
char *cpFile;

pthread_t ptSendBin, ptRecData;

int main(int argc ,char *ucPort[])
{

    cpPort = ucPort[1];
    cpFile  = ucPort[2];
    
    printf("UART OTA on %s\r\n",ucPort[1]);
    //printf("Binary File Name %s\r\n",ucPort[2]);
    imyuart = open_uart(ucPort[1]);
    if(imyuart < 0)
    {
        printf("Failed to Open %s\r\n",ucPort[1]);
        return 3;
    }
    //close_uart(imyuart);    
    
    pthread_create( &ptSendBin, NULL, fota_process,NULL);
    pthread_create( &ptRecData, NULL, read_uart, NULL);

    pthread_join(ptSendBin,NULL);
    pthread_join(ptRecData,NULL);

    printf("Return : %d\n",iRetVal);


    return iRetVal;
    //exit(0);
}

void *fota_process()
{
    int iRet = -1;

    while(1)
    {
        //int n = read(imyuart, &ui8read_byte, 1);
        if(ui8read_byte == 'C')
        {
            //memset(read_buf,0x00,sizeof(read_buf));
            printf("Requect from Chip : %C\r\n",ui8read_byte);
            ui8read_byte = 0;
            usleep(10);
            iRet = send_binary(cpFile);
            switch(iRet)
            {
                case 0:
                    printf("Firmware update Success\r\n");
                    iRetVal = iRet;
                    return iRet;
                break;
                case 1:
                    printf("Response TimeOut\r\n");
                    iRetVal = iRet;
                    return iRet;
                    
                break;
                case 2:
                    printf("Failed to open binary file\r\n");
                    iRetVal = iRet;
                    return iRet;
                break;
                default:
                    printf("Somthing Wrong\r\n");
                    return 9;
            }
        }
        usleep(10);
    }
}

void *read_uart(void *var)
{
    int n = 0;
    while (1)
    {
        printf("polling Data from uart\r\n");
        n = read(imyuart, &ui8read_byte, 1);
        tcflush(imyuart,TCIOFLUSH);
        usleep(10);
        /* code */
    }
    
}
int send_binary(char cpFile[])
{
    FILE *fBinFile = NULL;
    char* ptrMalloc;
    char cFByte;
    char buffer[600] = {0};

    uint16_t ui16FileSize = 0;
    int i,iCurrentByte = 0;
    int iNoByteToSend = 0;
    int iSendPacket = 0;
    int iRetryCnt = 0;

    uint16_t uiTmpPkt = 0;
    uint16_t uiPacket = 0;
    uint16_t uiNoOfPack = 0;
    uint16_t ui16CRC;
    uint16_t ui16CRC_bin;

    char cImgHRD[2] = {0xBB,0xDD};
    char cImgFtr[2] = {0xBE,0xAF};
    char cPktHDR[2] = {0xA5,0xA5};
    char cPktFtr[2] = {0xDE,0xAD};
    char cTmpCRC_bin[2] = {0};
    char cTmpCRC[2] = {0};

    printf("Prepare to Send binary file\r\n");
    printf(" Binary File Name : %s\r\n",cpFile);

    fBinFile = fopen(cpFile,"rb");

    if(fBinFile == NULL)
    {
        printf("Err..Failed to open file : %s\r\n",cpFile);
        close_uart(imyuart);
        pthread_cancel(ptRecData);
        return 2;
    }

    fseek(fBinFile,0L,SEEK_END);
    ui16FileSize = ftell(fBinFile);

    if(ui16FileSize != 0)
    {
        printf(" File Size : %d\r\n",ui16FileSize);
    }
    
    fseek(fBinFile,0L,SEEK_SET);

    /* Getting No of Packet to be send */
    uiNoOfPack = ui16FileSize/ChunkLeng;
    if((ui16FileSize)%(ChunkLeng))
    {
       uiNoOfPack = uiNoOfPack +1;
    }    
    printf(" No of Packet : %02X\r\n",uiNoOfPack);
    printf("File Size : %02X\r\n",ui16FileSize);

    /* Getting CRC for Binary */
    ptrMalloc = (char*)malloc(uiNoOfPack*ChunkLeng);
    memset(ptrMalloc,0x00,uiNoOfPack*ChunkLeng);
    fread(ptrMalloc, 1, ui16FileSize, fBinFile);
    ui16CRC_bin = crc_16(ptrMalloc,ui16FileSize);
    printf(" %s -> CRC %02X\r\n",cpFile,ui16CRC_bin);
    memset(buffer,0x00,sizeof(buffer));
    free(ptrMalloc);

    fseek(fBinFile,0L,SEEK_SET);
    printf(" Sending Image Header\r\n");
    
    //cTmpCRC_bin[0] =  ((ui16CRC_bin  & 0xFF00) >> 8);
    //cTmpCRC_bin[1] =  ((ui16CRC_bin  & 0x00FF));
    uint16_t ui16FileSizeSnd = 0;
    ui16CRC_bin =  ConvertBig162Little16(ui16CRC_bin);
    uiNoOfPack =  ConvertBig162Little16(uiNoOfPack);
    ui16FileSizeSnd = ConvertBig162Little16(ui16FileSize);
    /* Send Image Header */    
    write(imyuart, &cImgHRD, 2);  //Header
    write(imyuart, &ui16FileSizeSnd, 2);  //*.bin Length
    write(imyuart, &ui16CRC_bin, 2);  //CRC-16 For *.bin
    write(imyuart, &uiNoOfPack, 2);  //No of Packet
    write(imyuart, &cImgFtr, 2);  //Footer
    //while (iRetryCnt < 50)
    while (1)	
    {
        if(ui8read_byte == (uint8_t)0x55)
        {
		ui8read_byte = 0x00;		
		iRetryCnt = 0;	
		break;
        }
        if(iRetryCnt >= 50)
        {
            printf("************** Time Out **************\r\n");
            iRetryCnt = 0;
            fclose(fBinFile); 
            close_uart(imyuart);
            pthread_cancel(ptRecData);
            iSendPacket = 1;
            return 1;
            break;
        }
        iRetryCnt++;
        sleep(1);
    }
    
    
    printf(" Sending Image Chunk : %d \r\n",ChunkLeng);
    while (iSendPacket == 0) 
    { 
        uiPacket++;
        for(i=0;i<ChunkLeng;i++)
        {
            cFByte = fgetc(fBinFile);
            buffer[i] = cFByte;
            //printf ("%02X ", buffer[i]);
            iCurrentByte++;
            iNoByteToSend++;
            if(ui16FileSize == iCurrentByte)
            {
                iSendPacket = 1;
                break;
            }
        }
        ui16CRC = 0;
        ui16CRC = crc_16(buffer,ChunkLeng);
        printf("\n Packet No Hex : %02X\r\n",uiPacket);
        //printf("\nCRC Actl: %02X\r\n",ui16CRC);
        ui16CRC =  ConvertBig162Little16(ui16CRC);
        uiTmpPkt =  ConvertBig162Little16(uiPacket);
        printf("\nCRC Convt: %02X\r\n",uiTmpPkt);

        write(imyuart, &cPktHDR, 2);  //Header
        write(imyuart, &ui16CRC, 2);//CRC-16
        write(imyuart, &uiTmpPkt, 2);//Packet No
        write(imyuart, buffer, ChunkLeng);//Data
        write(imyuart, &cPktFtr, 2);//End Of Packet

        while(iRetryCnt < 50)
        {
            //int n = read(imyuart, &ui8read_byte, 1);
            if(ui8read_byte == (uint8_t)0x55) //Sucess
            {
                ui8read_byte = 0x00;
                break;
            }
            if(ui8read_byte == (uint8_t)0xEE)
            {
                printf("Re Sending Previous\r\n");
                write(imyuart, &cPktHDR, 2);  //Header
                write(imyuart, &ui16CRC, 2);//CRC-16
                write(imyuart, &uiPacket, 2);//Packet No
                //write(imyuart, buffer, iNoByteToSend);//Data
                write(imyuart, &buffer[0], ChunkLeng);//Data
                write(imyuart, &cPktFtr, 2);//End Of Packet
                ui8read_byte = 0x00;
            }
            printf("ui8read_byte %02X \r\n",(uint8_t)ui8read_byte);
            iRetryCnt++;
            sleep(1);
        }
        if(iRetryCnt >= 50)
        {
            printf("************** Time Out **************\r\n");
            iRetryCnt = 0;
            fclose(fBinFile); 
            close_uart(imyuart);
            pthread_cancel(ptRecData);
            return 1;
            break;
        }
        memset(buffer,0x77,sizeof(buffer));
        iNoByteToSend = 0;
        //j = j+ChunkLeng;       
    } 
    printf("\nNumber of bytes Sent %d\r\n",iCurrentByte);

    fclose(fBinFile); 
    close_uart(imyuart);
    pthread_cancel(ptRecData);
    return 0;
}

int open_uart(char ucport[])
{
    int fd;/*File Descriptor*/
    fd = open(ucport,O_RDWR | O_NOCTTY);	/* ttyUSB0 is the FT232 based USB2SERIAL Converter   */
			   					/* O_RDWR   - Read/Write access to serial port       */
								/* O_NOCTTY - No terminal will control the process   */
								/* Open in blocking mode,read will wait              */

    if(fd == -1)						/* Error Checking */
    {
        printf("Error! in Opening %s\r\n",ucport);
        return fd;
    }
    else
    {
        printf("%s Opened Successfully\r\n",ucport);
    }
	
    /*---------- Setting the Attributes of the serial port using termios structure --------- */
    
    struct termios SerialPortSettings;	/* Create the structure                          */

    tcgetattr(fd, &SerialPortSettings);	/* Get the current attributes of the Serial port */

    /* Setting the Baud rate */
    cfsetispeed(&SerialPortSettings,B9600); /* Set Read  Speed as 9600                       */
    cfsetospeed(&SerialPortSettings,B9600); /* Set Write Speed as 9600                       */

    /* 8N1 Mode */
    SerialPortSettings.c_cflag &= ~PARENB;   /* Disables the Parity Enable bit(PARENB),So No Parity   */
    SerialPortSettings.c_cflag &= ~CSTOPB;   /* CSTOPB = 2 Stop bits,here it is cleared so 1 Stop bit */
    SerialPortSettings.c_cflag &= ~CSIZE;	 /* Clears the mask for setting the data size             */
    SerialPortSettings.c_cflag |=  CS8;      /* Set the data bits = 8                                 */
    
    SerialPortSettings.c_cflag &= ~CRTSCTS;       /* No Hardware flow Control                         */
    SerialPortSettings.c_cflag |= CREAD | CLOCAL; /* Enable receiver,Ignore Modem Control lines       */ 
    
    
    SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY);          /* Disable XON/XOFF flow control both i/p and o/p */
    //SerialPortSettings.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);  /* Non Cannonical mode                            */
    SerialPortSettings.c_iflag &= ~(ECHO | ECHOE | ISIG);  /* ECHO OFF  */
    SerialPortSettings.c_lflag &= ~(ICANON);// Non Cannonical mode 

    SerialPortSettings.c_oflag &= ~OPOST;/*No Output Processing*/
    
    /* Setting Time outs */
    SerialPortSettings.c_cc[VMIN] = 1; /* Read at least 10 characters */
    SerialPortSettings.c_cc[VTIME] = 0; /* Wait indefinetly   */


    if((tcsetattr(fd,TCSANOW,&SerialPortSettings)) != 0) /* Set the attributes to the termios structure*/
    {
        printf("ERROR ! in Setting attributes\n");
    }
    else{
                printf("\n  BaudRate = 9600 \n  StopBits = 1 \n  Parity   = none\r\n");
                tcflush(fd,TCIOFLUSH);
                return fd;
    }
}

int close_uart(int ifile)
{
    close(ifile);
    printf("UART Closed Sucessfully\r\n");
}

uint16_t ConvertBig162Little16( uint16_t ui16InData )
{

	return(uint16_t) ( ((ui16InData&0xFF00) >> 8)| ((ui16InData & 0x00FF) << 8));

}
