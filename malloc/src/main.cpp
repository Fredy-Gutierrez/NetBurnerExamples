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

#include <hal.h>
#include <init.h>
#include <malloc.h>
#include <nbrtos.h>

const char *AppName = "Malloc Example";

/**
 *  ShowHeapSpace()
 *
 *  Uses the mallinfo() funcation call to return the following structure:
 *
 *      struct mallinfo {
 *         int arena;    // total space allocated from system
 *         int ordblks;  // number of non-inuse chunks
 *         int smblks;   // unused -- always zero
 *         int hblks;    // number of mmapped regions
 *         int hblkhd;   // total space in mmapped regions
 *         int usmblks;  // unused -- always zero
 *         int fsmblks;  // unused -- always zero
 *         int uordblks; // total allocated space
 *         int fordblks; // total non-inuse space
 *         int keepcost; // top-most, releasable (via malloc_trim) space
 *      };
 */
void ShowHeapSpace()
{
    struct mallinfo mi;
    mi = mallinfo();

    iprintf("-heapinfo---------------\r\n");
    iprintf("used      : %ld\r\n", mi.uordblks);
    iprintf("spaceleft : %ld\r\n", spaceleft());
    iprintf("free      : %ld\r\n\r\n", spaceleft() + mi.fordblks);
}

/**
 * UserMain()
 */
void UserMain(void *pd)
{
    init();

    while (1)
    {
        iprintf("=========================================\r\n\r\n");

        uint8_t *pA, *pB, *pC = nullptr;   // Pointers to allocated memory blocks

        // Allocating in order of A, B, C.
        iprintf("Allocating 1,000,000 bytes (pA)\r\n");
        pA = (uint8_t *)malloc(sizeof(uint8_t) * 1000000);
        if (pA == nullptr) { iprintf("malloc failed\r\n"); }
        ShowHeapSpace();

        iprintf("Allocating 3,000,000 bytes (pB)\r\n");
        pB = (uint8_t *)malloc(sizeof(uint8_t) * 3000000);
        if (pB == nullptr) { iprintf("malloc failed\r\n"); }
        ShowHeapSpace();

        iprintf("Allocating 512,000 bytes (pC)\r\n");
        pC = (uint8_t *)malloc(sizeof(uint8_t) * 512000);
        if (pC == nullptr) { iprintf("malloc failed\r\n"); }
        ShowHeapSpace();

        // Now freeing data in order B, A, C. This illustrates that the spaceleft
        // value can not increase until the last block allocated (pC) is
        // freed.
        if (pB != nullptr)
        {
            iprintf("Free (pB) \r\n");
            free(pB);
            ShowHeapSpace();
        }

        if (pA != nullptr)
        {
            iprintf("Free (pA) \r\n");
            free(pA);
            ShowHeapSpace();
        }

        if (pC != nullptr)
        {
            iprintf("Free (pC) \r\n");
            free(pC);
            ShowHeapSpace();
        }

        OSTimeDly(TICKS_PER_SECOND * 10);
    }
}
