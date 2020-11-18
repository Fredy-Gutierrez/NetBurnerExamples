#include <predef.h>
#include <stdio.h>
#include <nbrtos.h>
#include <http.h>
#include <init.h>
#include <netinterface.h>
#include <hal.h>


const char * AppName="ClearConfigFlash";

extern void  EraseWholeConfigRecord();

void UserMain(void * pd)
{
    init();
    GetInterfaceBlock()->discovery_server = "discover.netburner.com";
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait up to 5 seconds for active network activity 

    iprintf("Press any key to erase configuration flash\r\n");
    getchar();
    EraseWholeConfigRecord();
    iprintf("complete\r\n");

    iprintf("*** Changes will take effect after reboot ***\r\n");

    while (1)
    {
        OSTimeDly(TICKS_PER_SECOND);
    }
}
