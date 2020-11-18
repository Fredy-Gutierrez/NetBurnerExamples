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

#include <NetworkDebug.h>
#include <arp.h>
#include <buffers.h>
#include <ctype.h>
#include <init.h>
#include <ip.h>
#include <mailto.h>
#include <ppp.h>
#include <serial.h>
#include <stdio.h>
#include <string.h>
#include <utils.h>

//----- global variables -----
static char ACCOUNT_NAME[80];
static char ACCOUNT_PASS[80];
static char DIAL_STRING[80];
static IPADDR4 SMTP_SERVER;
static char USERID[80];
static char MAILFROM[80];
static char MAILTO[80];
static char SUBJECT[80];
static char MAILBODY[255];

#define SERIALPORT_TO_USE (1)   // 0 for the main port, 1 for the 10pin aux serial port

//----- function prototypes -----

const char *AppName = "PPP Example";

int test_Auth(const char *name, const char *pass);
void GetField(PCSTR prompt, PSTR field);
void DisplayMenu();
void SetIp(IPADDR4 &ip);
void DoPing(IPADDR4 ipa);
void ShowErrors(const char *msg, int errv);

void UserMain(void *pd)
{
    init();                                       // Initialize network stack
    StartHttp();                                  // Start web server, default port 80
    WaitForActiveNetwork(TICKS_PER_SECOND * 5);   // Wait for DHCP address

#ifdef _DEBUG
    InitializeNetworkGDB();
// InitializeNetworkGDB_and_Wait();
#endif

    pppoptions pppo;
    char c;
    BOOL bDial_out = FALSE;
    IPADDR4 TheIp;

    do
    {
        iprintf("Starting %s\r\n", AppName);

        //------------------------------------------------------------------------------
        /* Initialize the globals used in ppp driver */

        /* Set the modem global settings (These are the defaults if not included) */
        PPP_BAUDRATE_TO_USE = 57600;
        PPP_SW_RX_FLOW = TRUE;
        PPP_SW_TX_FLOW = TRUE;
        MODEM_AUTOBAUD = "AT\r";
        MODEM_RESET = "ATZ\r";
        MODEM_ANSWER = "ATA\r";
        MODEM_HANGSTRING = "ATH0\r";
        MODEM_INITSTRING = "AT&K0&C0&D0\r";
        // MODEM_INITSTRING = "AT&K4&C0&D0/G0\r" // Newcom
        MODEM_ENDCMDMODE = "ATO0\r";
        MODEM_POSTDIAL = "\r";
        MODEM_ATTNSTRING = "+++";

        /* A reasonable set of PPP limits and negotiation settings */
        pppo.Restart_Interval = 3; /* The restart interval for PPP Negotiations */
        pppo.Max_Terminate = 2;
        pppo.Max_Configure = 25;
        pppo.Max_Failure = 25;
        pppo.pUserName = "nburn";
        pppo.pPassword = "nburn";
        // pppo.TX_DESIRED_ACCM = 0xFFFFFFFF;
        // pppo.RX_DESIRED_ACCM = 0xFFFFFFFF;
        // pppo.TX_DESIRED_ACCM = 0x000A0000;
        // pppo.RX_DESIRED_ACCM = 0x000A0000;
        pppo.TX_DESIRED_ACCM = 0x00000000;
        pppo.RX_DESIRED_ACCM = 0x00000000;
        pppo.Chat_Login_disable = FALSE;
        pppo.CHAPenable = TRUE;
        /*------------------------------------------------------------------------------*/
        /* Initialize the globals used in this example */

        USERID[0] = 0;
        MAILFROM[0] = 0;
        MAILTO[0] = 0;
        SUBJECT[0] = 0;
        MAILBODY[0] = 0;
        bDial_out = FALSE;
        /*------------------------------------------------------------------------------*/
        // Ask user what type of connection to make through serial terminal
        do
        {
            iprintf("(M)odem or (D)irect?");
            c = tolower(getchar());
        } while ((c != 'm') && (c != 'd'));
        iprintf("\r\n");
        switch (c)
        {
            case 'm':
                do
                {
                    iprintf("(D)ial or (R)ecieve?");
                    c = toupper(getchar());
                } while ((c != 'R') && (c != 'D'));
                break;
            case 'd':
                do
                {
                    iprintf("(S)erver or (C)lient?");
                    c = toupper(getchar());
                } while ((c != 'S') && (c != 'C'));
                break;
        }
        iprintf("\r\n");
        /*------------------------------------------------------------------------------*/

        /* We are dialing through a modem*/
        if (c == 'D')
        {
            pppo.SetThisInterfaceAddress = 0; /* We expect the other side to provide an address so we set ours to 0 */
            pppo.SetThatInterfaceAddress = 0;
            pppo.authenticate_func = NULL; /* We don't require the ISP to provide us with Username/password, we dialed him */

            do /* Promt for the dialing information */
            {
                iprintf("\r\n");
                GetField("ISP Account User ID", ACCOUNT_NAME);
                GetField("ISP Account Password", ACCOUNT_PASS);
                strcpy(DIAL_STRING, "ATD");
                GetField("ISP Account Phone Number", DIAL_STRING + 3);

                iprintf("Account User ID = %s\r\n", ACCOUNT_NAME);
                iprintf("Account Password= %s\r\n", ACCOUNT_PASS);
                iprintf("Account Number  = %s\r\n", DIAL_STRING + 3);
                iprintf("Is this correct? (y/n/a (a=abort))");
                c = toupper(getchar());
            } while (c == 'N');

            pppo.pUserName = ACCOUNT_NAME; /* The user name password we are providing the ISP if he asks */
            pppo.pPassword = ACCOUNT_PASS;

            if (c != 'A') /* Are we dialing */
            {
                bDial_out = TRUE;
                iprintf("\r\nDialing...\r\n");
                int rv = DialPPP(SERIALPORT_TO_USE, &pppo, DIAL_STRING);
                if (rv != ERR_PPP_SUCCESS) { ShowErrors("Dial Failed", rv); }
                else
                {
                    iprintf("Connection Established\r\n");
                }
            }
        }
        /*------------------------------------------------------------------------------*/

        /* We are hosting a server directly (without a modem)*/
        else if (c == 'S')
        {
            pppo.authenticate_func = NULL;
            pppo.SetThisInterfaceAddress = AsciiToIp4("192.1.1.1"); /* We expect to provide an IP address to anyone dialing in to us */
            pppo.SetThatInterfaceAddress = AsciiToIp4("192.1.1.2");
            int rv = StartPPPDirect(SERIALPORT_TO_USE, &pppo);
            if (rv != ERR_PPP_SUCCESS) { ShowErrors("Listen Failed", rv); }
            else
            {
                iprintf("PPP Direct Server Started\r\n");
            }
        }
        /*------------------------------------------------------------------------------*/

        /* We are directly connecting as a client (without a modem)*/
        else if (c == 'C')
        {
            pppo.SetThisInterfaceAddress = 0; /* We expect the other side to provide an address so we set ours to 0 */
            pppo.SetThatInterfaceAddress = 0;
            pppo.authenticate_func = NULL; /* We don't require the ISP to provide us with Username/password, we dialed him */
            pppo.TX_DESIRED_ACCM = 0xFFFFFFFF;
            pppo.RX_DESIRED_ACCM = 0x000A0000;

            do /* Promt for the client authentication information */
            {
                iprintf("\r\n");
                GetField("ISP Account User ID", ACCOUNT_NAME);
                GetField("ISP Account Password", ACCOUNT_PASS);

                iprintf("Account User ID = %s\r\n", ACCOUNT_NAME);
                iprintf("Account Password= %s\r\n", ACCOUNT_PASS);
                iprintf("Is this correct? (y/n/a (a=abort))");
                c = toupper(getchar());
            } while (c == 'N');

            pppo.pUserName = ACCOUNT_NAME; /* The user name password we are providing the ISP if he asks */
            pppo.pPassword = ACCOUNT_PASS;

            if (c != 'A') /* Did user select abort */
            {
                iprintf("\r\nConnecting...\r\n");

                strcpy(DIAL_STRING, "CLIENT");

                int rv = DirectConnectPPP(SERIALPORT_TO_USE, &pppo, DIAL_STRING);
                if (rv != ERR_PPP_SUCCESS) { ShowErrors("Connect Failed", rv); }
                else
                {
                    iprintf("Connection Established\r\n");
                }
            }
        }
        /*------------------------------------------------------------------------------*/

        /* We are hosting a server through a modem */
        else
        {
            iprintf("Require Authentication? (y/n)");
            c = toupper(getchar());
            if (c == 'N')
            {
                pppo.authenticate_func = NULL;
                iprintf("\r\nNo authentication\r\n");
            }
            else
            {
                iprintf("\r\nPAP authentication\r\n");
                pppo.authenticate_func = test_Auth;
            }

            pppo.SetThisInterfaceAddress = AsciiToIp4("192.1.1.1"); /* We expect to provide an IP address to anyone dialing in to us */
            pppo.SetThatInterfaceAddress = AsciiToIp4("192.1.1.2");
            iprintf("Starting PPP Receive Dameon\r\n");

            int rv = StartPPPDaemon(SERIALPORT_TO_USE, &pppo);

            if (rv != ERR_PPP_SUCCESS) { ShowErrors("Listen Failed", rv); }
            else
            {
                iprintf("PPP Dameon Started\r\n");
            }
        }

        /* We have dialed or listened for a receive and the PPP Deamon is running */
        /* Normally your program would do whatever it does.... */
        /* we choose to use the PPP in various arbitrary ways....*/

        if (GetPPPState() != eClosed)
        {
            DisplayMenu();
            do
            {
                c = toupper(getchar());
                switch (c)
                {
                    case 'A': SendCHAPChallenge(); break;
                    case 'C': ShowCounters(); break;
                    case 'S':
                    {
                        enum_PPPState es = GetPPPState();
                        iprintf("PPP State = %d = ", (int)es);
                        switch (es)
                        {
                            case eClosed: iprintf("eClosed"); break;
                            case eInitializingModem: iprintf("eInitializingModem"); break;
                            case eDialing: iprintf("eDialing"); break;
                            case eWait4Ring: iprintf("eWait4Ring"); break;
                            case eAnswering: iprintf("eAnswering"); break;
                            case eWaitForTrain: iprintf("eWaitForTrain"); break;
                            case eLCPNegotiate: iprintf("eLCPNegotiate"); break;
                            case ePAPAuthenticate: iprintf("ePAPAuthenticate"); break;
                            case eNCPNegotiate: iprintf("eNCPNegotiate"); break;
                            case eOpen: iprintf("eOpen"); break;
                            case eClosing: iprintf("eClosing"); break;
                            default: iprintf("Unknown");
                        }
                        iprintf("\r\nRemote IP:");
                        ShowIP(GetThatPPP_IP());
                        iprintf("\r\nNetBurner IP:");
                        ShowIP(GetThisPPP_IP());
                        iprintf("\r\n");
                        break;
                    }
#ifdef _DEBUG
                    case 'L': SetLogLevel(); break;
#endif

                    case 'M':
                    {
                        do
                        {
                            iprintf("\r\n");
                            GetField("Mail Account User ID", USERID);
                            GetField("Mail From", MAILFROM);
                            GetField("Mail To", MAILTO);
                            GetField("Mail Subject", SUBJECT);
                            GetField("SMTP Server IP", MAILBODY);
                            SMTP_SERVER = AsciiToIp4(MAILBODY);
                            GetField("Mail Body:", MAILBODY);

                            iprintf("Mail Account User ID= %s\r\n", USERID);
                            iprintf("Mail From= %s\r\n", MAILFROM);
                            iprintf("Mail To= %s\r\n", MAILTO);
                            iprintf("Mail Subject= %s\r\n", SUBJECT);
                            iprintf("Mail Body= %s\r\n", MAILBODY);
                            iprintf("SMTP Server =");
                            ShowIP(SMTP_SERVER);
                            iprintf("\r\n");

                            iprintf("Is this correct? (y/n/a (a=abort))");
                            c = toupper(getchar());
                        } while (c == 'N');

                        if (c != 'A')
                        {
                            iprintf("Sending Mail...\r\n");
                            TheIp = SMTP_SERVER;

                            if (SendMail(TheIp, USERID, MAILFROM, MAILTO, SUBJECT, MAILBODY)) { iprintf("Sent Mail\r\n"); }
                            else
                            {
                                iprintf("Mail Failed\r\n");
                            }
                        }
                    }
                    break;
                    case 'H':
                        ClosePPPSesion();
                        if (bDial_out) { c = 'X'; }
                        break;
                    case 'N':
                        if (TheIp.IsNull())
                        {
                            GetField("SMTP Server IP", MAILBODY);
                            SMTP_SERVER = AsciiToIp4(MAILBODY);
                            TheIp = SMTP_SERVER;
                        }
                        DoPing(TheIp);
                        break;
                    case 'P': DoPing(GetThatPPP_IP()); break;
                    case 'I':
                        SetIp(TheIp);
                        iprintf("The IP now =");
                        ShowIP(TheIp);
                        iprintf("\r\n");
                        break;

                    case ' ': iprintf("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n*********************\r\n");
                    case '?': DisplayMenu();
                    default: break;
                }
            } while (c != 'X');
            StopPPPDaemon();
        }   // Not eClosed
    } while (1);
}

/**
 * This function should return '1' if the passed-in username and password are
 * valid. This particular sample function always returns OK, but in the real
 * world, you would want to check the username and password. A pointer to this
 * function is passed into the PPP options structure on initialization. This is
 * done inside UserMain() in this example
 */
int test_Auth(const char *name, const char *pass)
{
    iprintf("Auth %s:%s\r\n", name, pass);
    return 1;
}

void DisplayMenu()
{
    iprintf("\r\n----- Options Menu -----\r\n");
    iprintf("A - Send CHAP Authentication Challenge\r\n");
    iprintf("C - Show Counters\r\n");
    iprintf("H - Force a Hangup\r\n");
    iprintf("I - Set IP address\r\n");
    iprintf("M - Send Mail\r\n");
    iprintf("N - Ping SMTP Server\r\n");
    iprintf("P - Ping Remote Host\r\n");
    iprintf("S - Show PPP Status\r\n");
    iprintf("X - Exit\r\n");
    iprintf("? - Show Help\r\n");
}

void GetField(PCSTR prompt, PSTR field)
{
    iprintf("Please enter the %s?", prompt);
    gets(field);
    iprintf("\r\n");
}

/**
 * This function pings the address given in buffer
 */
void DoPing(IPADDR4 ipa)
{
    IPADDR4 addr_to_ping;
    addr_to_ping = ipa;
    iprintf("\nPinging :");
    ShowIP(addr_to_ping);
    iprintf("\n");

    int rv = Ping4(addr_to_ping, 1 /* Id */, 1 /* Seq */, 100 /* Max Ticks */);

    if (rv == -1)
    {
        iprintf("No response, ping failed!\n");
        iprintf("Not all ISPs will allow a ping to the PPP server\r\n");
    }
    else
        iprintf("Response took %dms\n", (rv * 1000 / TICKS_PER_SECOND));
}

/**
 * Prompt for and change an IP address
 */
void SetIp(IPADDR4 &ip)
{
    char buffer[40];
    int n = 0;
    char c;
    iprintf("IP addr?");

    do
    {
        c = getchar();
        buffer[n++] = c;
        if (n > 39) n = 0;
        if (((c == '\r') || (c == '\n')) && (n < 5))
        {
            n = 0;
            c = 0;
        }   // Incomplete IP string
    } while (((c != '\r') && (c != '\n')) || (n == 0));

    if (n != 0)
    {
        buffer[n - 1] = 0;   // Append null character to end of IP string
        ip = AsciiToIp4(buffer);
    }
}

void ShowErrors(const char *msg, int errv)
{
    switch (errv)
    {
        case ERR_PPP_ALREADY_OPEN: iprintf("%s:ERR_PPP_ALREADY_OPEN\r\n", msg); break;
        case ERR_PPP_NO_DIALTONE: iprintf("%s:ERR_PPP_NO_DIALTONE\r\n", msg); break;
        case ERR_PPP_NO_ANSWER: iprintf("%s:ERR_PPP_NO_ANSWER\r\n", msg); break;
        case ERR_PPP_BUSY: iprintf("%s:ERR_PPP_BUSY\r\n", msg); break;
        case ERR_PPP_FAIL: iprintf("%s:ERR_PPP_FAIL\r\n", msg); break;
        case ERR_PPP_PASSFAIL: iprintf("%s:ERR_PPP_PASSFAIL\r\n", msg); break;
        case ERR_PPP_LOSTCARRIER: iprintf("%s:ERR_PPP_LOSTCARRIER\r\n", msg); break;
        case ERR_PPP_NO_MODEM: iprintf("%s:ERR_PPP_NO_MODEM\r\n", msg); break;
        case ERR_PPP_LCP_FAILED: iprintf("%s:Negotiation Loop\r\n", msg); break;
    }
}
