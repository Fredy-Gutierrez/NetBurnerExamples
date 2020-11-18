#include <ctype.h>
#include <init.h>
#include <nbrtos.h>
#include <predef.h>
#include <stdio.h>
#include <system.h>

const char *AppName = "Clear User Parameters";

void UserMain(void *pd)
{
    init();
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

    iprintf("Wait a few seconds for stable power...\r\n");
    OSTimeDly(TICKS_PER_SECOND * 5);

    iprintf("Erasing User Parameter Flash...");
    static uint8_t buffer[4096];
    SaveUserParameters(buffer, 4095);
    iprintf("complete\r\n");

    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND * 1);
    }
}
