/******************************************************************************
 *
 * Copyright (C) 2022-2023 Maxim Integrated Products, Inc. All Rights Reserved.
 * (now owned by Analog Devices, Inc.),
 * Copyright (C) 2023 Analog Devices, Inc. All Rights Reserved. This software
 * is proprietary to Analog Devices, Inc. and its licensors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************/

/**
 * @file    main.c
 * @brief   read and write sdhc
 * @details This example uses the sdhc and ffat to read/write the file system on
 *          an SD card. The Fat library used supports long filenames (see ffconf.h)
 *          the max length is 256 characters.
 *
 *          You must connect an sd card to the sd card slot.
 */

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "mxc_device.h"
#include "mxc_sys.h"
#include "mxc_delay.h"
#include "mxc_errors.h"
#include "sdhc_regs.h"
#include "led.h"
#include "tmr.h"
#include "uart.h"
#include "gpio.h"
#include "sdhc_lib.h"
#include "ff.h"
#include "board.h"
#include <stdarg.h>

/***** Definitions *****/

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define MAXLEN 256

/***** Globals *****/
FATFS *fs; //FFat Filesystem Object
FATFS fs_obj;
FIL file; //FFat File Object
FRESULT err; //FFat Result (Struct)
FILINFO fno; //FFat File Information Object
DIR dir; //FFat Directory Object
TCHAR message[32768];
WORD message1[32768];
TCHAR directory[MAXLEN], cwd[MAXLEN], filename[MAXLEN], volume_label[24],
volume = '0';
TCHAR *FF_ERRORS[20];
DWORD clusters_free = 0, sectors_free = 0, sectors_total = 0, volume_sn = 0;
UINT bytes_written = 0, bytes_read = 0, mounted = 0;
BYTE work[4096];
static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!";



/***** FUNCTIONS *****/

//*************************************************
//SD-Card related functions
//*************************************************
void generateMessage(unsigned length)
{
    for (int i = 0; i < length; i++)
    {
        /*Generate some random data to put in file*/
        message[i] = charset[rand() % (sizeof(charset) - 1)];
    }
}

int mount()
{
	fs = &fs_obj;
	if ((err = f_mount(fs, "", 1)) != FR_OK) { //Mount the default drive to fs now
		printf("Error opening SD card: %s\n", FF_ERRORS[err]);
		f_mount(NULL, "", 0);
	} else {
		printf("SD card mounted.\n");
		mounted = 1;
	}

	f_getcwd(cwd, sizeof(cwd)); //Set the Current working directory

	return err;
}

int umount()
{
	if ((err = f_mount(NULL, "", 0)) != FR_OK) { //Unmount the default drive from its mount point
		printf("Error unmounting volume: %s\n", FF_ERRORS[err]);
	} else {
		printf("SD card unmounted.\n");
		mounted = 0;
	}

	return err;
}

int formatSDHC()
{
	printf("\n\n*****THE DRIVE WILL BE FORMATTED IN 5 SECONDS*****\n");
	printf("**************PRESS ANY KEY TO ABORT**************\n\n");
	MXC_UART_ClearRXFIFO(MXC_UART0);
	MXC_TMR_Delay(MXC_TMR0, MXC_DELAY_MSEC(5000));
	if (MXC_UART_GetRXFIFOAvailable(MXC_UART0) > 0) {
		return E_ABORT;
	}

	printf("FORMATTING DRIVE\n");

	MKFS_PARM format_options = { .fmt = FM_ANY };

	if ((err = f_mkfs("", &format_options, work, sizeof(work))) !=
			FR_OK) { //Format the default drive to FAT32
		printf("Error formatting SD card: %s\n", FF_ERRORS[err]);
	} else {
		printf("Drive formatted.\n");
	}

	mount();

	if ((err = f_setlabel("MAXIM")) != FR_OK) {
		printf("Error setting drive label: %s\n", FF_ERRORS[err]);
		f_mount(NULL, "", 0);
	}

	umount();

	return err;
}

int getSize()
{
	QWORD disksize = 0;
	QWORD available_bytes = 0;

	if (!mounted) {
		mount();
	}

	if ((err = f_getfree(&volume, &clusters_free, &fs)) != FR_OK) {
		printf("Error finding free size of card: %s\n", FF_ERRORS[err]);
		f_mount(NULL, "", 0);
	}

	sectors_total = (fs->n_fatent - 2) * fs->csize;
	sectors_free = clusters_free * fs->csize;
	disksize = (QWORD) (sectors_total / 2) * (QWORD) (1024); // for cards over 3GB, we need QWORD to hold size
	available_bytes = (QWORD) (sectors_free / 2) * (QWORD) (1024);

	printf("Disk Size: %llu bytes\n", disksize);
	printf("Available: %llu bytes\n", available_bytes);

	return err;
}

int ls()
{
	if (!mounted) {
		mount();
	}

	printf("Listing Contents of %s - \n", cwd);

	if ((err = f_opendir(&dir, cwd)) == FR_OK) {
		while (1) {
			err = f_readdir(&dir, &fno);
			if (err != FR_OK || fno.fname[0] == 0)
				break;

			printf("%s/%s", cwd, fno.fname);

			if (fno.fattrib & AM_DIR) {
				printf("/");
			}

			printf("\n");
		}
		f_closedir(&dir);
	} else {
		printf("Error opening directory!\n");
		return err;
	}

	printf("\nFinished listing contents\n");

	return err;
}

int createFile()
{
	unsigned int length = 128;

	if (!mounted) {
		mount();
	}

	printf("Enter the name of the text file: \n");
	scanf("%255s", filename);
	printf("Enter the length of the file: (256 max)\n");
	scanf("%d", &length);
	printf("Creating file %s with length %d\n", filename, length);

	if ((err = f_open(&file, (const TCHAR *)filename, FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK) {
		printf("Error opening file: %s\n", FF_ERRORS[err]);
		f_mount(NULL, "", 0);
		return err;
	}
	printf("File opened!\n");

	generateMessage(length);

	if ((err = f_write(&file, &message, length, &bytes_written)) != FR_OK) {
		printf("Error writing file: %s\n", FF_ERRORS[err]);
		f_mount(NULL, "", 0);
		return err;
	}
	printf("%d bytes written to file!\n", bytes_written);

	if ((err = f_close(&file)) != FR_OK) {
		printf("Error closing file: %s\n", FF_ERRORS[err]);
		f_mount(NULL, "", 0);
		return err;
	}
	printf("File Closed!\n");
	return err;
}

int appendFile()
{
	unsigned int length = 0;

	if (!mounted) {
		mount();
	}

	printf("Type name of file to append: \n");
	scanf("%255s", filename);
	printf("Type length of random data to append: \n");
	scanf("%d", &length);

	if ((err = f_stat((const TCHAR *)filename, &fno)) == FR_NO_FILE) {
		printf("File %s doesn't exist!\n", (const TCHAR *)filename);
		return err;
	}
	if ((err = f_open(&file, (const TCHAR *)filename, FA_OPEN_APPEND | FA_WRITE)) != FR_OK) {
		printf("Error opening file %s\n", FF_ERRORS[err]);
		return err;
	}
	printf("File opened!\n");

	generateMessage(length);

	if ((err = f_write(&file, &message, length, &bytes_written)) != FR_OK) {
		printf("Error writing file: %s\n", FF_ERRORS[err]);
		return err;
	}
	printf("%d bytes written to file\n", bytes_written);

	if ((err = f_close(&file)) != FR_OK) {
		printf("Error closing file: %s\n", FF_ERRORS[err]);
		return err;
	}
	printf("File closed.\n");
	return err;
}

int mkdir()
{
	if (!mounted) {
		mount();
	}

	printf("Enter directory name: \n");
	scanf("%255s", directory);

	err = f_stat((const TCHAR *)directory, &fno);
	if (err == FR_NO_FILE) {
		printf("Creating directory...\n");

		if ((err = f_mkdir((const TCHAR *)directory)) != FR_OK) {
			printf("Error creating directory: %s\n", FF_ERRORS[err]);
			f_mount(NULL, "", 0);
			return err;
		} else {
			printf("Directory %s created.\n", directory);
		}

	} else {
		printf("Directory already exists.\n");
	}
	return err;
}

int cd()
{
	if (!mounted) {
		mount();
	}

	printf("Directory to change into: \n");
	scanf("%255s", directory);

	if ((err = f_stat((const TCHAR *)directory, &fno)) == FR_NO_FILE) {
		printf("Directory doesn't exist (Did you mean mkdir?)\n");
		return err;
	}

	if ((err = f_chdir((const TCHAR *)directory)) != FR_OK) {
		printf("Error in chdir: %s\n", FF_ERRORS[err]);
		f_mount(NULL, "", 0);
		return err;
	}

	printf("Changed to %s\n", directory);
	f_getcwd(cwd, sizeof(cwd));

	return err;
}

int delete ()
								{
	if (!mounted) {
		mount();
	}

	printf("File or directory to delete (always recursive!)\n");
	scanf("%255s", filename);

	if ((err = f_stat((const TCHAR *)filename, &fno)) == FR_NO_FILE) {
		printf("File or directory doesn't exist\n");
		return err;
	}

	if ((err = f_unlink(filename)) != FR_OK) {
		printf("Error deleting file\n");
		return err;
	}
	printf("Deleted file %s\n", filename);
	return err;
								}
/*** the example() asks for a number of iterations to write random 32768 bytes to a file ****/
/*int example()
{
	QWORD num_of_iterations = 1000; // defaults to 1,000 iterations
	uint32_t delay_in_microseconds = 250000; // defaults to 250 milliseconds
	unsigned int length = 32768;

	printf("Number of 32768-byte chunks to write to file -- 1000 = 32MB: \n");
	scanf("%llu",&num_of_iterations);

	printf("Delay in microseconds -- 250000 = 250ms: \n");
	scanf("%u",&delay_in_microseconds);

	if (!mounted) {
		mount();
	}

    if ((err = f_getfree(&volume, &clusters_free, &fs)) != FR_OK) {
        printf("Error finding free size of card: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }

    if ((err = f_getlabel(&volume, volume_label, &volume_sn)) != FR_OK) {
        printf("Error reading drive label: %s\n", FF_ERRORS[err]);
        f_mount(NULL, "", 0);
        return err;
    }

	if ((err = f_open(&file, "0:SWIFTV2.w", FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK) {
		printf("Error opening file: %s\n", FF_ERRORS[err]);
		f_mount(NULL, "", 0);
		return err;
	}
	printf("File opened!\n");

	generateMessage(length);

	if ((err = f_write(&file, &message, length, &bytes_written)) != FR_OK)
	{
		printf("Error writing file: %s\n", FF_ERRORS[err]);
		f_mount(NULL, "", 0);
		return err;
	}
	printf("Test write: %d bytes written to file!\n", bytes_written);

	if ((err = f_close(&file)) != FR_OK)
	{
		printf("Error closing file: %s\n", FF_ERRORS[err]);
		f_mount(NULL, "", 0);
		return err;
	}
	printf("File Closed!\n");

	// Open the file to append
	printf("Appending...\n");

	if ((err = f_open(&file, "0:SWIFTV2.txt", FA_OPEN_APPEND | FA_WRITE)) != FR_OK)
	{

		printf("Error opening file %s\n", FF_ERRORS[err]);
		return err;
	}

	// loop to continue appending 32768 bytes to the SWIFTV2.txt file
	QWORD i = 0;
	while (i < num_of_iterations) {
		MXC_Delay(delay_in_microseconds); // blocking delay between iterations
		generateMessage(length);
		if ((err = f_write(&file, &message1, length, &bytes_written)) != FR_OK) {

			printf("Error writing file: %s\n", FF_ERRORS[err]);
			f_mount(NULL, "", 0);
			return err;
		}

		printf("%llu\n", (i+1));
		i++;
	}

	// close file
	if ((err = f_close(&file)) != FR_OK) {

		printf("Error closing file: %s\n", FF_ERRORS[err]);
		f_mount(NULL, "", 0);
		return err;
	}

	printf("File Closed!\n");

	// unmount SD Card

	if ((err = f_mount(NULL, "", 0)) != FR_OK) {
		printf("Error unmounting volume: %s\n", FF_ERRORS[err]);
		return err;
	}
	else {
		mounted = 0;
	}

	return 0;
}*/


//**********************************************************
//Magpie related custom functions
//**********************************************************

//The following function creates an audio header for the wave file
uint8_t AudioFiles_WriteHeader(FIL* MyFile)
{

	UINT bw;
	DWORD FileEndPointer;
	volatile uint8_t SampleRateByte;
	volatile uint32_t SampleRateWord;

	BYTE WAVFileHeader[46] =
	{
			0x52, 0x49, 0x46, 0x46, //Bytes 1 - 4: "RIFF"
			0x00, 0x00, 0x00, 0x00, //Bytes 5 - 8: ((Size of overall file) - 8)
			0x57, 0x41, 0x56, 0x45, //Bytes 9 - 12: "WAVE"
			0x66, 0x6D, 0x74, 0x20, //Bytes 13 - 16: Format Chunk Marker i.e. "fmt "
			0x12, 0x00, 0x00, 0x00, //Bytes 17 - 20: Length of format data (18 DECIMAL)
			0x01, 0x00, 0x01, 0x00, //Bytes 21 - 22: Type of format (1 for PCM) Bytes 23 - 24: Number of channels (1 or mono)
			0x40, 0x1F, 0x00, 0x00, //Bytes 25 - 28: Sample Rate
			0x80, 0x3E, 0x00, 0x00, //Bytes 29 - 32: ((SampleRate * BitsPerSample * Channels)/8) i.e. 16000
			0x02, 0x00, 0x10, 0x00, //Bytes 33 - 34: ((Channels * BitsPerSample)/8) i.e. 2 Bytes 35 - 36: Bits Per Sample (16)
			0x00, 0x00,             //Bytes 37 - 38: I dont know why these two bytes are added
			0x64, 0x61, 0x74, 0x61, //Bytes 39 - 42: "data" chunk header i.e. "data"
			0x00, 0x20, 0x04, 0x00  //Bytes 43 - 46: Size of data section (Number DOES INCLUDE these four bytes)
	};

	SampleRateWord = 0x00DC0500; //384KHz


	WAVFileHeader[4] = (BYTE)((MyFile->obj.objsize - 8) & 0xFF);
	WAVFileHeader[5] = (BYTE)(((MyFile->obj.objsize - 8) >> 8) & 0xFF);
	WAVFileHeader[6] = (BYTE)(((MyFile->obj.objsize - 8) >> 16) & 0xFF);
	WAVFileHeader[7] = (BYTE)(((MyFile->obj.objsize - 8) >> 24) & 0xFF);

	WAVFileHeader[24] = (BYTE)((SampleRateWord) & 0xFF);
	WAVFileHeader[25] = (BYTE)(((SampleRateWord) >> 8) & 0xFF);
	WAVFileHeader[26] = (BYTE)(((SampleRateWord) >> 16) & 0xFF);
	WAVFileHeader[27] = (BYTE)(((SampleRateWord) >> 24) & 0xFF);

	SampleRateWord = ((SampleRateWord * 16) >> 3); //Shifting Right by three is the same as dividing by 8

	WAVFileHeader[28] = (BYTE)((SampleRateWord) & 0xFF);
	WAVFileHeader[29] = (BYTE)(((SampleRateWord) >> 8) & 0xFF);
	WAVFileHeader[30] = (BYTE)(((SampleRateWord) >> 16) & 0xFF);
	WAVFileHeader[31] = (BYTE)(((SampleRateWord) >> 24) & 0xFF);

	WAVFileHeader[42] = (BYTE)((MyFile->obj.objsize - 46) & 0xFF);
	WAVFileHeader[43] = (BYTE)(((MyFile->obj.objsize - 46) >> 8) & 0xFF);
	WAVFileHeader[44] = (BYTE)(((MyFile->obj.objsize - 46) >> 16) & 0xFF);
	WAVFileHeader[45] = (BYTE)(((MyFile->obj.objsize - 46) >> 24) & 0xFF);

	FileEndPointer = MyFile->obj.objsize;

	  err = f_lseek(MyFile, 0);
	  if (err != FR_OK)
	  {
	    return 1; //ERROR
	  }

	  err = f_write(MyFile, WAVFileHeader, 46, &bw);
	  if (err != FR_OK)
	  {
	    return 1; //ERROR
	  }

	  err = f_lseek(MyFile, FileEndPointer);
	  if (err != FR_OK)
	  {
	    return 1; //ERROR
	  }

	  return 0;
}


//This function writes the Magpie data to the SD-Card
int ADC_Data_write()
{



	printf("Writing ADC_Data to SD_Card \n");



	//Mount the SD_Card
	if (!mounted)
	{
		mount();
	}

	if ((err = f_open(&file, "0:SWIFTV2_log2.wav", FA_CREATE_ALWAYS | FA_WRITE)) != FR_OK)
	{
		printf("Error opening file: %s\n", FF_ERRORS[err]);
		f_mount(NULL, "", 0);
		return err;
	}
	printf("File opened!\n");

//Writing  Audio header file

	if(AudioFiles_WriteHeader(&file))
	      {
	         //Error Handling
	      }


	if ((err = f_close(&file)) != FR_OK)
	{
		printf("Error closing file: %s\n", FF_ERRORS[err]);
		f_mount(NULL, "", 0);
		return err;
	}

	printf("File Closed!\n");


	// unmount SD Card

	if ((err = f_mount(NULL, "", 0)) != FR_OK)
	{
		printf("Error unmounting volume: %s\n", FF_ERRORS[err]);
		return err;
	}
	else
	{
		mounted = 0;
	}

	return 0;
}

/******************************************************************************/
int main(void)
{
	mxc_sdhc_cfg_t cfg;

	FF_ERRORS[0] = "FR_OK";
	FF_ERRORS[1] = "FR_DISK_ERR";
	FF_ERRORS[2] = "FR_INT_ERR";
	FF_ERRORS[3] = "FR_NOT_READY";
	FF_ERRORS[4] = "FR_NO_FILE";
	FF_ERRORS[5] = "FR_NO_PATH";
	FF_ERRORS[6] = "FR_INVLAID_NAME";
	FF_ERRORS[7] = "FR_DENIED";
	FF_ERRORS[8] = "FR_EXIST";
	FF_ERRORS[9] = "FR_INVALID_OBJECT";
	FF_ERRORS[10] = "FR_WRITE_PROTECTED";
	FF_ERRORS[11] = "FR_INVALID_DRIVE";
	FF_ERRORS[12] = "FR_NOT_ENABLED";
	FF_ERRORS[13] = "FR_NO_FILESYSTEM";
	FF_ERRORS[14] = "FR_MKFS_ABORTED";
	FF_ERRORS[15] = "FR_TIMEOUT";
	FF_ERRORS[16] = "FR_LOCKED";
	FF_ERRORS[17] = "FR_NOT_ENOUGH_CORE";
	FF_ERRORS[18] = "FR_TOO_MANY_OPEN_FILES";
	FF_ERRORS[19] = "FR_INVALID_PARAMETER";
	srand(12347439);
	int run = 1, input = -1;

	printf("\n\n***** SWIFTV2 SDHC FAT Filesystem Testing R1 *****\n");
	printf("SwiftV2 is changed to Magpie \r\n");



	// Initialize SDHC peripheral
	cfg.bus_voltage = MXC_SDHC_Bus_Voltage_3_3;
	cfg.block_gap = 0;
	cfg.clk_div =
			0x0b0; // Maximum divide ratio, frequency must be >= 400 kHz during Card Identification phase
	if (MXC_SDHC_Init(&cfg) != E_NO_ERROR)
	{
		printf("Unable to initialize SDHC driver.\n");
		return 1;
	}


	// wait for card to be inserted
	while (!MXC_SDHC_Card_Inserted()) {}
	printf("Card inserted.\n");

	// set up card to get it ready for a transaction
	if (MXC_SDHC_Lib_InitCard(10) == E_NO_ERROR) {
		printf("Card Initialized.\n");
	} else {
		printf("No card response! Remove card, reset EvKit, and try again.\n");

		return -1;
	}

	if (MXC_SDHC_Lib_Get_Card_Type() == CARD_SDHC) {
		printf("Card type: SDHC\n");
	} else {
		printf("Card type: MMC/eMMC\n");
	}

	/* Configure for fastest possible clock, must not exceed 52 MHz for eMMC */
	if (SystemCoreClock > 96000000) {
		printf("SD clock ratio (at card) 4:1\n");
		MXC_SDHC_Set_Clock_Config(1);
	} else {
		printf("SD clock ratio (at card) 2:1\n");
		MXC_SDHC_Set_Clock_Config(0);
	}

	while (run) {
		f_getcwd(cwd, sizeof(cwd));

		printf("\nChoose one of the following options: \n");
		printf("0. Find the Size of the SD Card and Free Space\n");
		printf("1. Format the Card\n");
		printf("2. Manually Mount Card\n");
		printf("3. List Contents of Current Directory\n");
		printf("4. Create a Directory\n");
		printf("5. Move into a Directory (cd)\n");
		printf("6. Create a File of Random Data\n");
		printf("7. Add Random Data to an Existing File\n");
		printf("8. Delete a File\n");
		printf("9. Run SWIFTV2 FatFS Operations\n");
		printf("10. Unmount Card and Quit\n");
		printf("%s>>", cwd);

		input = -1;
		scanf("%d", &input);
		printf("%d\n", input);

		err = 0;

		switch (input) {
		case 0:
			getSize();
			break;
		case 1:
			formatSDHC();
			break;
		case 3:
			ls();
			break;
		case 6:
			createFile();
			break;
		case 7:
			appendFile();
			break;
		case 4:
			mkdir();
			break;
		case 5:
			cd();
			break;
		case 9:
			ADC_Data_write();
			break;
		case 10:
			umount();
			run = 0;
			break;
		case 2:
			mount();
			break;
		case 8:
			delete ();
			break;
		default:
			printf("Invalid Selection %d!\n", input);
			err = -1;
			break;
		}
		/*
        if (err >= 0 && err <= 20) {
            printf("Function Returned with code: %s\n", FF_ERRORS[err]);
        } else {
            printf("Function Returned with code: %d\n", err);
        }
		 */
		MXC_TMR_Delay(MXC_TMR0, MXC_DELAY_MSEC(500));
	}
	printf("End of example, please try to read the card.\n");
	return 0;
}
