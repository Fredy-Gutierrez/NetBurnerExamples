/*
 * dev_test.cpp
 *
 *  Created on: Oct 20, 2016
 *      Author: maxam
 */

#include <predef.h>
#include <stdio.h>
#include <init.h>
#include <sim.h>           /*on-chip register definitions*/
#include <pins.h>
#include <nbrtos.h>
#include <math.h>

#include <constants.h>
#include <effs_fat/fat.h>
//#include <effs_fat/effs_utils.h>
#include "dev_test.h"
#include "FileSystemUtils.h"

extern volatile uint32_t TimeTick;

//static char *read_buff[8*1024];

#define	KB_SIZE (1024UL)
#define	MB_SIZE (KB_SIZE*1024UL)

#define TO_MB(x) (x/MB_SIZE)
#define TO_KB(x) (x/KB_SIZE)

int SpeedTest(char *pFileName, unsigned int size)
{
	if (!pFileName)
		pFileName = "TEST.BIN";

	f_chdir( "/" );

	iprintf("\r\nOpening test file: %s\r\n", pFileName);

	F_FILE* fp_src = f_open( pFileName, "r" );
	if (!fp_src)
	{
		iprintf("Cannot open file %s\r\n", pFileName);
		iprintf("Application requires a file named TEST.BIN to be on the flash card.\r\n");
		iprintf("You can use FTP, or copy it to the card with a PC.\r\n");
		return 1;
	}

	long file_size = f_filelength( pFileName );
	if (file_size == -1)
	{
		iprintf("Cannot detect file size %s\r\n", pFileName);
		return 2;
	}

	char *pWrBuff = new char[file_size];
	if (pWrBuff == NULL)
	{
		iprintf("Not enough memory to read file %s\r\n", pFileName);
		return 3;
	}

	unsigned long BytesRead = 0;
	unsigned long LastBytesRead = 0;

	iprintf("Reading file data...\r\n");

	int RetryAttempts = 0;
	uint32_t t_start = TimeTick;
	uint32_t t_end = TimeTick;

	while(TimeTick == t_end);
	t_start = TimeTick;

	while(RetryAttempts < 10 && BytesRead < (unsigned long)file_size)
	{
		BytesRead += f_read( pWrBuff + BytesRead, 1, (unsigned long)file_size - BytesRead, fp_src );
		if(BytesRead == LastBytesRead)
		{
			int error = f_getlasterror();
			if( error ) DisplayEffsErrorCode( error );
			OSTimeDly( /*TICKS_PER_SECOND / 4*/ 1 );
			RetryAttempts++;
			iprintf("Retry for read = %d\r\n", RetryAttempts);
		}
		LastBytesRead = BytesRead;
	}

	t_end = TimeTick;

	if (BytesRead == 0)
	{
		iprintf("Failed to read file %s\n\r", pFileName);
		f_close(fp_src);
		if (pWrBuff)
			delete[](pWrBuff);
		return 4;
	}
	else
		iprintf("%lu bytes were read into the memory\n\r", BytesRead);

	uint32_t readTime_ms = (t_end - t_start)*1000 / TICKS_PER_SECOND;
	iprintf("Read Time - %lu sec %lu ms\n\r", readTime_ms / 1000, readTime_ms % 1000);

	float readSpeed = ((float)BytesRead * 1000.0) / readTime_ms;
	if (readSpeed > MB_SIZE)
		iprintf("Read speed - %lu.%lu MB/sec\n\r", TO_MB((uint32_t)readSpeed), TO_MB(((uint32_t)readSpeed % MB_SIZE)*1000));
	else if(readSpeed > KB_SIZE)
		iprintf("Read speed - %lu.%lu KB/sec\n\r", TO_KB((uint32_t)readSpeed), TO_KB(((uint32_t)readSpeed % KB_SIZE)*1000));
	else
		iprintf("Read speed - %lu Bytes/sec\n\r", (uint32_t)readSpeed);

	f_close(fp_src);

	char testFileName[32] = {0};
	sprintf(testFileName, "tmp_%s", pFileName);
	f_delete(testFileName);

	F_FILE* wfile = f_open(testFileName, "w" );
	if(wfile)
	{
		unsigned long BytesWritten = 0;
		unsigned long LastBytesWritten = 0;
		RetryAttempts = 0;

		iprintf("Writing test data...\n\r");
		t_start = TimeTick;
		t_end = TimeTick;
		while(TimeTick == t_end);
		t_start = TimeTick;

		while (  ( RetryAttempts < 10 ) && ( BytesWritten < BytesRead ) )
		{
			BytesWritten += f_write( pWrBuff+BytesWritten, 1, BytesRead-BytesWritten, wfile );
		    if ( BytesWritten == LastBytesWritten )
		    {
		    	int error = f_getlasterror();
		        if( error ) DisplayEffsErrorCode( error );
		        OSTimeDly( /*TICKS_PER_SECOND / 4*/ 1 );
		        RetryAttempts++;
		        iprintf("Retry for write = %d\r\n", RetryAttempts);
		    }
		    LastBytesWritten = BytesWritten;
		}
		t_end = TimeTick;

		iprintf("Writing complete\n\r");
		iprintf("%lu byte(s) have been written\n\r", BytesWritten);

		float writeTime_ms = (t_end - t_start)*1000 / TICKS_PER_SECOND;
		iprintf("Write Time - %lu sec %lu ms\n\r", (uint32_t)writeTime_ms / 1000, (uint32_t)writeTime_ms %  1000);

		if (BytesWritten != 0)
		{
			float writeSpeed = ((float)BytesWritten*1000.0) / writeTime_ms;

			if (writeSpeed > MB_SIZE)
				iprintf("Write speed - %lu.%lu MB/sec\n\r", TO_MB((uint32_t)writeSpeed), TO_MB(((uint32_t)writeSpeed % MB_SIZE)*1000));
			else if(writeSpeed > KB_SIZE)
				iprintf("Write speed - %lu.%lu KB/sec\n\r", TO_KB((uint32_t)writeSpeed), TO_KB(((uint32_t)writeSpeed % KB_SIZE)*1000));
			else
				iprintf("Write speed - %lu Bytes/sec\n\r", (uint32_t)writeSpeed);
		}

		f_flush(wfile);
		f_close(wfile);

		//f_delete(testFileName);
	}
	else
	{
		iprintf("Cannot create file %s\r\n", testFileName);
		if (pWrBuff)
			delete[](pWrBuff);
		return 5;
	}

	if (pWrBuff)
		delete[](pWrBuff);

	return 0;
}



