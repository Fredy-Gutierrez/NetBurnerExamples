/* Revision: 3.2.0 */

/******************************************************************************
* Copyright 1998-2020 NetBurner, Inc.  ALL RIGHTS RESERVED
*
*    Permission is hereby granted to purchasers of NetBurner Hardware to use or
*    modify this computer program for any use as long as the resultant program
*    is only executed on NetBurner provided hardware.
*
*    No other rights to use this program or its derivatives in part or in
*    whole are granted.
*
*    It may be possible to license this or other NetBurner software for use on
*    non-NetBurner Hardware. Contact sales@Netburner.com for more information.
*
*    NetBurner makes no representation or warranties with respect to the
*    performance of this computer program, and specifically disclaims any
*    responsibility for any damages, special or consequential, connected with
*    the use of this program.
*
* NetBurner
* 5405 Morehouse Dr.
* San Diego, CA 92121
* www.netburner.com
******************************************************************************/

/*-----------------------------------------------------------------------------
 * Example to demonstrate how to read and write data to the on-board flash memory
 * User Parameter Area. This example is here for reverse compatibility with
 * tools/applications prior to NetBurner 3.0. If compatibility is not an issue,
 * we recommend using the Configuration App section. It is significantly more
 * flexible and provides much easier access. See the configuration users guide
 * and the configuration examples for more information.
 *
 * Important notes on packed vs. integer alignment when using structures and classes.
 * This example uses a structure named NV_SettingsStruct, which consists of types
 * that are 8, 16 and 32 bits long. If you do not add an attribute tag at the
 * end of the structure, the default will be integer-aligned. This means that
 * padding will be added to ensure each member will be on an integer boundary.
 * While this can increase execution speed, it also means that the stored data will
 * be a bit larger, and doing something like an overlay or indexing into the
 * structure with a pointer will not work correctly. To tell the compiler not
 * to use any padding, add "__attribute__((packed));" to the end of the structure
 * definition as demonstrated in this example. But again, be aware that when
 * using a packed structure elsewher in your application you access is as
 * packed as well.
 --------------------------------------------------------------------------------*/

#include <predef.h>
#include <ctype.h>
#include <init.h>
#include <nbrtos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <system.h>

const char *AppName = "Save User Params";

BOOL bCheckedNV;

#define ARRAY_SIZE (10)

// This key can be updated if you modify your structure. For example, you may need to do something
// special if you load a new version of s/w in a device that has the old structure values.
#define VERIFY_KEY (0x48666052)   // NV Settings key code

// This is the structure that will be stored in Flash. It is also used to update variables at runtime
// *** WARNING: CHANGE THE VERIFY KEY ANY TIME YOU MAKE A CHANGE TO THIS STRUCTURE ***
struct NV_SettingsStruct
{
    uint32_t VerifyKey;    // Changes when the structure is modified so we can detect the change
    uint32_t StructSize;   // Store structure size
    uint16_t nvWord;       // non volatile word
    char nvChar;           // non volatile char
    uint8_t nvByte;        // non volatile byte
    uint8_t byteArray[ARRAY_SIZE];
} __attribute__((packed));

NV_SettingsStruct NV_Settings;

/*-------------------------------------------------------------------
 * Assign default values if VerifyKey has changed or is not initialized
 *------------------------------------------------------------------*/
void CheckNVSettings()
{
    iprintf("Checking NV_Settings User Parameter Flash...\r\n");
    iprintf("Size of NV Structure: %lu Bytes\r\n", sizeof(NV_Settings));

    NV_SettingsStruct *pData = (NV_SettingsStruct *)GetUserParameters();
    iprintf("Verify key = 0x%lX\r\n", pData->VerifyKey);

    // We will check the struct size as well to try and protect those who forgot to change
    // the verify key in order to avoid a crash or incorrect results
    if ((pData->VerifyKey != VERIFY_KEY) || (pData->StructSize != sizeof(NV_Settings)))
    {
        if (pData->VerifyKey != VERIFY_KEY) iprintf("Flash verification key has changed - initializing Flash\r\n");
        if (pData->StructSize != sizeof(NV_Settings)) iprintf("Struct size has changed - initializing Flash\r\n");
        NV_Settings.VerifyKey = VERIFY_KEY;
        NV_Settings.nvWord = 0;
        NV_Settings.nvChar = 0;
        NV_Settings.nvByte = 0;
        NV_Settings.StructSize = sizeof(NV_Settings);

        for (int i = 0; i < ARRAY_SIZE; i++)
            NV_Settings.byteArray[i] = 0;

        SaveUserParameters(&NV_Settings, sizeof(NV_Settings));
    }
    else
    {
        iprintf("Flash verification is VALID - reading values from Flash\r\n");
        NV_Settings.VerifyKey = pData->VerifyKey;
        NV_Settings.nvWord = pData->nvWord;
        NV_Settings.nvChar = pData->nvChar;
        NV_Settings.nvByte = pData->nvByte;
        NV_Settings.StructSize = pData->StructSize;

        for (int i = 0; i < ARRAY_SIZE; i++)
            NV_Settings.byteArray[i] = pData->byteArray[i];
    }

    bCheckedNV = TRUE;
}

/*-------------------------------------------------------------------
 Display the values of the NV_Settings structure.
 ------------------------------------------------------------------*/
void DisplayNVSettings()
{
    if (!bCheckedNV) CheckNVSettings();

    iprintf("NV_Settings.nvWord = %d\r\n", NV_Settings.nvWord);
    iprintf("NV_Settings.nvChar = %d\r\n", NV_Settings.nvChar);
    iprintf("NV_Settings.nvByte = %d\r\n", NV_Settings.nvByte);

    for (int i = 0; i < ARRAY_SIZE; i++)
        iprintf("0x%02X ", NV_Settings.byteArray[i]);
    iprintf("\r\n");
}

/*-------------------------------------------------------------------
 UserMain
 ------------------------------------------------------------------*/
void UserMain(void *pd)
{
    bCheckedNV = FALSE;
    init();

    uint32_t maxUserParamSize = 0;

    extern const char PlatformName[];
    iprintf("\r\nPlatform Name: %s\r\n", PlatformName);

#if (defined(SB800EX) || defined(SB700EX) || defined(NANO54415) || defined(MODM7AE70))
    maxUserParamSize = 8 * 1024;
#elif (defined(MOD5441X))
    maxUserParamSize = 128 * 1024;
#else
    // If you get this message it indicates that your platform was not tested, and we want to ensure no flash
    // memory corruption takes place. You can check your platform to verify the size of the User Parameter
    // area, then modify this example so that it allows the test to proceed. If you have any questions please
    // contact NetBurner support.
    iprintf("*** ERROR: You platform was not tested with this example. Please see comments in source code\r\n");
    while (1)
        OSTimeDly(TICKS_PER_SECOND);
#endif

    iprintf("Maximum User Parameter Size = %ldK Bytes\r\n", maxUserParamSize / 1024);

    while (1)
    {
        CheckNVSettings();
        DisplayNVSettings();

        iprintf("Incrementing all NV_Settings values by 1\r\n");
        NV_Settings.nvWord++;
        NV_Settings.nvChar++;
        NV_Settings.nvByte++;

        for (int i = 0; i < ARRAY_SIZE; i++)
            NV_Settings.byteArray[i] = (NV_Settings.byteArray[i] + 1) % 256;

        if (sizeof(NV_Settings) < maxUserParamSize)
        {
            iprintf("Saving...");
            SaveUserParameters(&NV_Settings, sizeof(NV_Settings));
            iprintf("complete\r\n");
        }
        else
        {
            iprintf("*** ERROR: Your structure exceeds User Param Flash space, save aborted\r\n");
        }

        iprintf("\r\n\r\n");
        iprintf("Hit any key to repeat\r\n");
        getchar();
    }
}
