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

#include <init.h>
#include <nbrtos.h>
#include <pinconstant.h>
#include <pins.h>
#include <stddef.h>
#include <system.h>

const char AppName[] = "gpio";

#if (defined MODM7AE70)
#define Conn P2 
#define PinOutGpioFn PinIO::PIN_FN_OUT
#elif (!defined PK70 && !defined SB70LC && !defined NANO54415 && !defined SB800EX)
#define Conn J2
#define PinOutGpioFn PIN_GPIO
#else
#define Conn Pins
#define PinOutGpioFn PIN_GPIO
#endif

#define xstr(x) str(x)
#define str(x) #x

#define NBUFFERS (5)
#define CHARBUFSIZE 80

typedef struct
{
    char buf[CHARBUFSIZE];
} cmd_buffer;

struct command_rec
{
    const char *pName;
    void (*pfunc)(uint32_t *pDat);
};

void (*plastfunc)(uint32_t *pdat) = NULL;
extern command_rec commands[];

int GetNumber(const char **data, uint32_t *dv)
{
    const char *cp = *data;
    int rv = 0;
    *dv = 0;

    while ((*cp) && (*cp == ' '))
    {
        cp++;
    }

    while ((*cp >= '0') && (*cp <= '9'))
    {
        (*dv) *= 10;
        (*dv) += *cp - '0';
        rv = 1;
        cp++;
    }
    *data = cp;
    return rv;
}

void parsedata(const char *typestr, const char *datastr, unsigned long *params)
{
    unsigned long n = 0;

    while (*typestr)
    {
        while (*datastr == ' ')
        {
            datastr++;
        }
        if (*datastr == 0) { break; }

        switch (*typestr)
        {
            case 'd':
                if (GetNumber(&datastr, params + n + 1)) { n++; }

                break;
        }   // Switch
        typestr++;
    }   // While
    params[0] = n;
}

void processcommand(char *pstr)
{
    int i;
    unsigned long params[4];
    params[0] = 0;
    i = 0;
    if (pstr[0] == 0)
    {
        if (plastfunc) { plastfunc(params); }
        else
        {
            iprintf("\r\n");
        }
        return;
    }

    while (commands[i].pName[0] != 0)
    {
        if (commands[i].pName[0] == pstr[0])
        {
            int n = 1;
            while ((commands[i].pName[n] == pstr[n]) && (commands[i].pName[n] != 0))
            {
                n++;
            }
            if (commands[i].pName[n] == 0)
            {
                parsedata(commands[i].pName + n + 1, pstr + n, params);
                plastfunc = 0;
                commands[i].pfunc(params);
                return;
                break;
            }
        }
        i++;
    }

    puts("Unrecognized command\r\n");
}

void HI(uint32_t *params)
{
    uint32_t count = params[0];
    if (!count) { return; }
    uint32_t pin = params[1];
    Conn[pin].function(PinOutGpioFn);
    Conn[pin] = 1;
    iprintf(xstr(Conn) "[%lu]_out: %d\n", pin, 1);
}

void LO(uint32_t *params)
{
    uint32_t count = params[0];
    if (!count) { return; }
    uint32_t pin = params[1];
    Conn[pin].function(PinOutGpioFn);
    Conn[pin] = 0;
    iprintf(xstr(Conn) "[%lu]_out: %d\n", pin, 0);
}

void TGL(uint32_t *params)
{
    uint32_t count = params[0];
    static uint32_t pin;
    if (count) { pin = params[1]; }
    else if (!pin)
    {
        return;
    }
    Conn[pin].function(PinOutGpioFn);
    bool val = Conn[pin].toggle();
    plastfunc = TGL;
    iprintf(xstr(Conn) "[%lu]_out: %d\n", pin, val);
}

void OUT(uint32_t *params)
{
    uint32_t count = params[0];
    if (!count) { return; }
    uint32_t pin = params[1];
    Conn[pin].function(PinOutGpioFn);
    Conn[pin].drive();
}

void IN(uint32_t *params)
{
    uint32_t count = params[0];
    if (!count) { return; }
    uint32_t pin = params[1];
    Conn[pin].function(PinOutGpioFn);
    Conn[pin].hiz();
}

void RD(uint32_t *params)
{
    uint32_t count = params[0];
    static uint32_t pin;
    if (count) { pin = params[1]; }
    else if (!pin)
    {
        return;
    }
    Conn[pin].function(PinOutGpioFn);
    bool val = Conn[pin].read();
    iprintf(xstr(Conn) "[%lu]_in: %d\n", pin, val);
    plastfunc = RD;
}

void HELP(unsigned long *pd)
{
    int i;
    i = 0;
    iprintf("\r\n");
    while (commands[i].pName[0] != 0)
    {
        const char *cp = commands[i].pName;
        while (*cp)
        {
            cp++;
        }
        cp++;
        while (*cp)
        {
            cp++;
        }
        cp++;
        iprintf("%s \r\n", cp);
        i++;
    }
}

command_rec commands[] = {{"HI\0d\0Set Pin - High          HI <pin>", HI},    {"LO\0d\0Set Pin - Low           LO <pin>", LO},
                          {"TGL\0d\0Toggle Pin State        TGL <pin>", TGL}, {"OUT\0d\0Drive Pin Output        OUT <pin>", OUT},
                          {"IN\0d\0Tristate Pin            IN <pin>", IN},    {"RD\0d\0Tristate and Read Pin   RD <pin>", RD},
                          {"HELP\0\0Show this Help         HELP", HELP},      {"", nullptr}};

void Run()
{
    char c;
    unsigned int n;
    int nFirst = 0;
    cmd_buffer cmds[NBUFFERS];
    unsigned int nLastUsed;

    iprintf("HELP for help\r\n");
    nLastUsed = 0;
    plastfunc = 0;

    for (c = 0; c < NBUFFERS; c++)
    {
        cmds[(int)c].buf[0] = 0;
    }

    iprintf(">");
    while (1)
    {
        c = getchar();
        switch (c)
        {
            case 4:
                /*CTRL D */
                if (cmds[(nLastUsed - 1) % NBUFFERS].buf[0])
                {
                    nLastUsed = (nLastUsed - 1) % NBUFFERS;
                    putchar('\r');
                    iprintf(cmds[nLastUsed].buf);
                    nFirst = 0;
                }
                break;
            case 21:
                /*CTRL D */
                if (cmds[(nLastUsed + 1) % NBUFFERS].buf[0])
                {
                    nLastUsed = (nLastUsed + 1) % NBUFFERS;
                    putchar('\r');
                    iprintf(cmds[nLastUsed].buf);
                    nFirst = 0;
                }
                break;
            case 18:
                /*CTRL-R */
                if (cmds[(nLastUsed - 1) % NBUFFERS].buf[0])
                {
                    nLastUsed = (nLastUsed - 1) % NBUFFERS;
                    putchar('\r');
                    iprintf(cmds[nLastUsed].buf);
                }
                nFirst = 0;
                c = 0x0d;
                // No break here
            case 0x0d:
            case 0x0a:
                // Enter
                if (nFirst) { cmds[nLastUsed].buf[0] = 0; }
                processcommand(cmds[nLastUsed].buf);
                nLastUsed = (nLastUsed + 1) % NBUFFERS;

                iprintf(">");
                nFirst = 1;
                break;
            case 0x08:
                if (cmds[nLastUsed].buf[0])
                {
                    putchar(c);   /* Echo BS */
                    putchar(' '); /*Overwrite last */
                    putchar(c);   /*Backup again */
                    n = 0;
                    while ((cmds[nLastUsed].buf[n] != 0) && (n < (CHARBUFSIZE - 2)))
                    {
                        n++;
                    }
                    cmds[nLastUsed].buf[n - 1] = 0;
                }
                break;
            default:
                if (nFirst) { cmds[nLastUsed].buf[0] = 0; }
                nFirst = 0;
                if ((c >= 'a') && (c <= 'z')) { c -= 0x20; }
                n = 0;
                while ((cmds[nLastUsed].buf[n] != 0) && (n < (CHARBUFSIZE - 2)))
                {
                    n++;
                }
                cmds[nLastUsed].buf[n] = c;
                cmds[nLastUsed].buf[n + 1] = 0;
                break;
        }   // Switch
    }       // While
}

char readBuf[0x100];

void UserMain(void *pd)
{
    init();                                       // Initialize network stack
    StartHttp();                                  // Start web server, default port 80
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    iprintf("Application: %s\r\nNNDK Revision: %s\r\n", AppName, GetReleaseTag());

    Run();

    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND);
    }
}
