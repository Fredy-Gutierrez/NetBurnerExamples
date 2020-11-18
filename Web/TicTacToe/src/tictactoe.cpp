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

#include <startnet.h>
#include <stdlib.h>

// The following functions are referenced from the compressed HTML and need to be declared as Extern C
extern "C"
{
    void DoCounter(int sock, PCSTR url);
    void DoTickTacToe(int sock, PCSTR url);
};

/**
 *  DoCounter
 *
 *  A Trivial function that shows how many times a page has been reloaded!
 */
void DoCounter(int sock, PCSTR url)
{
    static uint32_t dw;
    char buffer[10];
    sniprintf(buffer, 10, "%ld", dw++);
    // Write out the value into the HTML page...
    writestring(sock, buffer);
}

/******************************************************************************
 *                TICTACTOE from here on
 *
 *  General Notes:
 *      The comments all refer to he and I. He is the human playing the game and
 *      I is this program. The state of the Game is stored in the URL. The url
 *      will read like:
 *
 *                TT.HTM?number
 *
 *      Where number is a number that encodes the state of the game. This number
 *      is an or of all the square values...
 *
 *      X's (His values)
 *                0x01    0x04   0x10
 *                0x40    0x100  0x400
 *                0x1000  0x4000 0x10000
 *
 *      O's (My values)
 *
 *                0x02    0x08  0x20
 *                0x80    0x200  0x800
 *                0x2000  0x8000 0x20000
 ******************************************************************************/

/**
 *  HeWins
 *
 *  Returns true if the User wins.... Should never happen!
 */
int HeWins(int i)
{
    return (((i & 0x1041) == 0x1041) || ((i & 0x4104) == 0x4104) || ((i & 0x10410) == 0x10410) || ((i & 0x15) == 0x15) ||
            ((i & 0x540) == 0x540) || ((i & 0x15000) == 0x15000) || ((i & 0x10101) == 0x10101) || ((i & 0x1110) == 0x1110));
}

/**
 *  IWin
 *
 *  Test to see if I (The computer wins)
 */
int IWin(int i)
{
    return HeWins(i >> 1);
}

// The preferred moves....
const int perfmovesm[] = {512, 8, 128, 2048, 32768, 2, 32, 8192, 0x20000};

/*The cell numbers
    1,2          4,8           16,32

    64,128     256,512,      1024,2048

    4096,8192 16384,32768, 0x10000,0x20000


    x..             x.x
    .*.      -> .*.  65794 -> |=32
    ..*             ..*

    x..             x.*
    .*.     and .*.  Automatic block
    *..             ...
*/

/**
 *  Move
 *
 *  Given the present board state return the next board state... (Our move)
 */
int Move(int now)
{
    // Opening book
    switch (now)
    {
        case 1:
        case 64:
        case 4096:
        case 0x10000: return 512 | now;
        case 4612:
        case 1540:
        case 65794: return now |= 32;
        case 16984: return now |= 8192;
        case 16897: return now |= 8192;
        case 16960: return now |= 8192;
        case 16912:
        case 17929: return now |= 0x20000;
        case 17920: return now |= 0x20000;
        case 66049: return now |= 32768;
        case 4672:
        case 256:
        case 532:
        case 580: return 2 | now;
    }

    int bm = 0;
    int pm = 0;
    for (int im = 0; im < 9; im++)
    {
        pm = perfmovesm[im];
        if (((pm & now) == 0) && (((pm >> 1) & now) == 0))
        {
            // The move is legal
            int hw = 0;
            int tnow = (now | pm);
            if (IWin(tnow)) { return (tnow); }
            bm = pm;
            for (int hm = 1; hm < (1 << 18); hm = hm << 2)
            {
                if (((hm & tnow) == 0) && (((hm << 1) & tnow) == 0))
                {
                    if (HeWins(hm | tnow)) { hw = 1; }
                }
            }   // For hm
            if (hw == 0) { return tnow; }
        }   // for pm
    }
    return bm | now;
}

/**
 *  DoTickTacToe
 *
 *  This function fills in an HTML table with the TicTacToe game in it.
 */
void DoTickTacToe(int sock, PCSTR url)
{
    int v = 0;
    iprintf("Doing TTT for [%s]\r\n", url);

    // Get a buffer to store outgoing data....
    PoolPtr pp = GetBuffer();
    pp->usedsize = 0;
    char *po = (char *)pp->pData;

    if ((*(url + 6)) == '?') { v = atoi((char *)(url + 7)); }
    else
    {
        v = 0;
    }

    // There are nine squares each with three states yours,mine,empty
    // 00 ==empty
    // 01 == yours
    // 10 == mine
    // 11 ==bad

    iprintf("Tick Tack Toe V= from %d to ", v);
    int orgv = v;
    if ((v != 0) && (!HeWins(v))) { v = Move(v); }
    iprintf("%d\n", v);

    int vshift = v;
    int won = 0;

    // Write out the HTML Table header stuff...
    if (HeWins(v))
    {
        won = 1;
        writestring(sock, "<tr><td colspan=3>You win!<A HREF=\"TT.HTM\">(Again?) </A></td></tr>\n");
    }
    else if (IWin(v))
    {
        won = 1;
        writestring(sock, "<tr><td colspan=3>I win!<A HREF=\"TT.HTM\">(Again?) </A></td></tr>\n");
    }
    else if ((orgv) && (orgv == v))
    {
        won = 1;
        writestring(sock, "<tr><td colspan=3>No One Wins! <BR><A HREF=\"TT.HTM\">Again? </A></td></tr>\n");
    }

    // Now write out the board
    for (int i = 0; i < 9; i++)
    {
        // The beginning of a row
        if ((i % 3) == 0) { append(po, (char *)"<TR>"); }
        append(po, (char *)"<TD>");

        switch (vshift & 3)
        {
            case 0:
                // IF the game is not over include the HREF for the next move...
                // Only if the square is blank
                if (!won)
                {
                    siprintf(po, "<A HREF = \"TT.HTM?%d\"><IMG SRC=\"BLANK.GIF\"></A>", v + (1 << (2 * i)));
                    while (*po)
                    {
                        po++;
                    }
                }
                else
                {
                    append(po, (char *)"<IMG SRC=\"BLANK.GIF\">");
                }
                break;
            case 1:
                // The square belongs to the evil human!
                append(po, (char *)"<IMG SRC=\"CROSS.GIF\">");
                break;
            case 2:
                // The square is Mine all mine
                append(po, (char *)"<IMG SRC=\"NOT.GIF\">");
                break;
        }

        // Now shift the next square into view...
        vshift = vshift >> 2;

        // Add the element end
        append(po, (char *)"</TD>");

        // If it is the end of the row add the row end...
        if ((i % 3) == 2) { append(po, (char *)"</TR>\n"); }
    }

    // We have written all of this stuff into the Pool buffer we allocated.
    // Now write it to the socket...
    pp->usedsize = ((unsigned char *)po - (pp->pData));
    write(sock, (char *)pp->pData, pp->usedsize);

    // Now free our storage...
    FreeBuffer(pp);
    return;
}
