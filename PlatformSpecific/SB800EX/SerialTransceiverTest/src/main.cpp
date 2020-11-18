
/*------------------------------------------------------------------------------
 * SB800EX software programmable serial transceiver test app
 *
 * Other 5441x devices use UART0 and UART1 as default serial ports. In order for
 * the SB800EX to support a wide variety of uses including RS-232, RS-485, CAN
 * bus, Wifi, Blue Tooth, and GPS, the alternate functions of the processor pins
 * required us use UART1 and UART2 as the back panel serial ports. If you are
 * porting to the SB800EX you will likely need to change UART0 and UART1 in your
 * program to UART1 and UART2.
 *
 * The jumper-less serial port configuration for RS-232, 485 and 422 requires an
 * additional function call to set the transceivers: SetSerialMode(). For example:
 *
 * #include <serial_config.h>
 *
 * int uartNumber = 1;  // can be uart 1 or uart 2
 * SetSerialMode(uartNumber, SERIAL_XCVR_RS232);
 * fdDataUart  = SimpleOpenSerial(uartNumber, 115200);
 *
 *----------------------------------------------------------------------------*/
#include <init.h>
#include <stdio.h>
#include <iosys.h>
#include <string.h>
#include <nbrtos.h>
#include <serial.h>
#include <serial_config.h>

const char *AppName = "SB800EX Serial Mode Test";

int fdDebugUart, fdDataUart;

// Select the debug UART number, which will be configured to RS-232 mode
int debugUartNum = 2;       // can be 1 or 2
int dataUartNum  = 1;       // must be different than Debug UART

/*-----------------------------------------------------------------------------
 * Display debug menu
 *-----------------------------------------------------------------------------*/
void displayMenu(void)
{
    iprintf("\r\n----- Debug UART = %d, all commands apply to UART %d -----\r\n", debugUartNum, dataUartNum);

    switch (GetSerialMode(dataUartNum))
    {
        case SERIAL_XCVR_LOOPBACK:
            iprintf("Current mode: Loopback\r\n");
            break;
        case SERIAL_XCVR_RS232:
            iprintf("Current mode: RS-232\r\n");
            break;
        case SERIAL_XCVR_RS485HALF:
            iprintf("Current mode: RS-485 Half Duplex\r\n");
            break;
        case SERIAL_XCVR_RS485FULL:
            iprintf("Current mode: RS-485 Full Duplex\r\n");
            break;
    }

    iprintf("1. Send \"Hello World\"\r\n");
    iprintf("2. Read data\r\n");
    iprintf("3. Set serial mode to 232 loopback (00)\r\n");
    iprintf("4. Set serial mode to 232(10)\r\n");
    iprintf("5. Set serial mode to 485 half (01)\r\n");
    iprintf("6. Set serial mode to 485 full (11)\r\n");
}

/*-----------------------------------------------------------------------------
 * Handle commands with a simple command dispatcher
 *----------------------------------------------------------------------------*/
void processCommand(char c, bool bResetUart)
{
    char buf[80];

    iprintf("\r");

    if (bResetUart)
    {
        close( fdDataUart );
        fdDataUart = 0;
        SerialClose(dataUartNum);
        fdDataUart = SimpleOpenSerial(dataUartNum, 115200);
    }

    int uartMode = GetSerialMode(dataUartNum);

    switch (toupper(c)) {
        case '1':
            if ((uartMode == SERIAL_XCVR_RS485HALF) || (uartMode == SERIAL_XCVR_RS485FULL))
                Set485TxEnable(dataUartNum, true);

            iprintf("Writing Hello World\r\n");
            writestring(fdDataUart, "Hello World\r\n");

            OSTimeDly(TICKS_PER_SECOND / 2);
            if ((uartMode == SERIAL_XCVR_RS485HALF) || (uartMode == SERIAL_XCVR_RS485FULL))
                Set485TxEnable(dataUartNum, false);
            break;

        case '2':
        {
            if (uartMode == SERIAL_XCVR_RS485HALF)
                Set485TxEnable(dataUartNum, false);

            int n = ReadWithTimeout(fdDataUart, buf, 80, TICKS_PER_SECOND);
            if(n>0)
            {
              buf[n]= '\0';
              iprintf("Read %d bytes: \"%s\"\r\n", n, buf);
            }
            else
              iprintf("Error: Read returned %d\r\n", n);
            break;
        }

        case '3':
            iprintf("Setting serial mode to 232 loopback\r\n");
            SetSerialMode(dataUartNum, SERIAL_XCVR_LOOPBACK);
            Serial485HalfDupMode( dataUartNum, 0 );
            break;

        case '4':
            iprintf("Setting serial mode to 232\r\n");
            SetSerialMode(dataUartNum, SERIAL_XCVR_RS232);
            break;

        case '5':
            iprintf("Setting serial mode to 485 half\r\n");
            SetSerialMode(dataUartNum, SERIAL_XCVR_RS485HALF);
            Serial485HalfDupMode( dataUartNum, 1 );
            break;

        case '6':
            iprintf("Setting serial mode to 485 full\r\n");
            SetSerialMode(dataUartNum, SERIAL_XCVR_RS485FULL);
            Serial485HalfDupMode( dataUartNum, 0 );
            break;

        default:
            displayMenu();
    }
}

/*-----------------------------------------------------------------------------
 * The main task
 *-----------------------------------------------------------------------------*/
void UserMain(void *pd)
{
	// Set both transceivers to RS-232
	SetSerialMode(debugUartNum, SERIAL_XCVR_RS232);
    SetSerialMode(dataUartNum, SERIAL_XCVR_RS232);

    init();
    WaitForActiveNetwork();

    OSTimeDly(TICKS_PER_SECOND);    // Allow any previous data to be transmitted on serial port
    SerialClose(debugUartNum);
    SerialClose(dataUartNum);

    fdDebugUart = SimpleOpenSerial(debugUartNum, 115200);
    fdDataUart  = SimpleOpenSerial(dataUartNum, 115200);

    ReplaceStdio( 0, fdDebugUart );
    ReplaceStdio( 1, fdDebugUart );
    ReplaceStdio( 2, fdDebugUart );

    displayMenu();
    while (1)
    {
        if (charavail())
        {
            char c = getchar();
            processCommand(c, false);
        }
    }
}
