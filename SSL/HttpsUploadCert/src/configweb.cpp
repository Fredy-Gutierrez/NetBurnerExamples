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

// NB Libs
#include <fdprintf.h>
#include <htmlfiles.h>
#include <http.h>
#include <iosys.h>
#include <ipv6/ipv6_interface.h>
#include <netbios.h>
#include <netinterface.h>

// Product Definitions
#include "nbfactory.h"

// Ethernet to Serial Application Definitions
#include "serialburnerdata.h"

// SSL User Routines
#include "ssluser.h"

// NB EFFS-STD library
#include "effs_std.h"

// HTML Form Creation and Data Extraction
#include "formtools.h"

#ifdef DIAG_LOG
#include "log.h"
#endif

extern "C"
{
    /* Banner, menu and trailer */
    void DisplayHeader(int sock, PCSTR url);
    void DisplayHelpBookmark(int sock, PCSTR url);
    void DisplayMenu(int sock, PCSTR url);
    void DisplayTrailer(int sock, PCSTR url);

    void DisplayDeviceName(int sock, PCSTR url);
    void DisplayPageReloadCount(int sock, PCSTR url);
    void webShowAddress(int socket, char *url);
    void DisplayFirmwareVersion(int sock, PCSTR url);

    /* Certificate and Keys */
    void DisplayCertificateInstalled(int sock, PCSTR url);
    void DisplayHttpsKeyInstalled(int sock, PCSTR url);
    void DisplayKeyFileStatus(int sock, PCSTR url);
}

void fShowRecord(FILE *filePtr);

extern BOOL bShowDebug;

/* User parameters */
NV_SettingsStruct NV_Settings;

/* User parameters change candidate */
NV_SettingsStruct gNV_SettingsChangeCopy;

/* User parameters change flag */
volatile BOOL gChangedUserParameters;

static char *sBookMarkPtr = (char *)"";

IPADDR6 IPv6LinkLocalAddress;
IPADDR4 IPv4Address;

/**
 * @brief Set user parameter defaults in NV_Settings.
 *
 * <b>Notes:</b> These NV_SettingsStruct fields must be set after calling this routine
 * for a valid record:
 *   - UserName
 *   - Password
 *   - IP_Addr_mode
 *   - DeviceName
 *
 * Any user installed keys and the certificate are erased as well.
 */
void SetDefaults(void)
{
    /* Reset all to zeros */
    (void)memset(&NV_Settings, 0, sizeof(NV_Settings));

    /* SSL Permanent keys */
    SslUserSetDefault();
}

/**
 * @brief Restores the NV settings to their default values
 */
void SetAndSaveDefaults(void)
{
    CheckNVSettings(true);
}

/**
 * @brief Verify user parameters, if invalid reset to defaults
 *
 * @param returnToFactory Returning to factory settings.
 *
 * <b>Notes:</b> These NV_SettingsStruct fields are set after calling this the default
 * setting routine for a valid record:
 *   - UserName
 *   - Password - Resetting requires a configuration record setting.
 *   - IP_Addr_mode
 *   - DeviceName
 *
 * Any user installed keys and the certificate are erased if it is reset.
 */
void CheckNVSettings(BOOL returnToFactory)
{
    NV_SettingsStruct *pData = (NV_SettingsStruct *)GetUserParameters();
    NV_Settings = *pData;

    if ((NV_Settings.STDEFFSVerifyKey != STD_EFFS_VERIFY_KEY) || (returnToFactory == true))
    {
        debug_iprintf("Formatting Flash Drive...\r\n");
        int rc = fs_format(NOR_DRV_NUM);
        if (rc != FS_NOERR) debug_iprintf("*** ERROR Formatting Flash Drive: %d\r\n", rc);
        NV_Settings.STDEFFSVerifyKey = STD_EFFS_VERIFY_KEY;
        if (SaveUserParameters(&NV_Settings, sizeof(NV_Settings)) != 0) { iprintf("NV Settings Saved\r\n"); }
    }
    else
    {
        debug_iprintf("STD_EFFS_VERIFY_KEY is Valid\r\n");
    }

    if ((NV_Settings.VerifyKey != NB_FACTORY_VERIFY_KEY) || (returnToFactory == true))
    {
        /* Save password */
        gNV_SettingsChangeCopy = NV_Settings;

        /* Reset user parameters */
        SetDefaults();

        /* Set device name */
        InterfaceBlock *pIfb = GetInterfaceBlock();
        uint8_t macByte4 = 0;
        uint8_t macByte5 = 0;
        if (pIfb != nullptr)
        {
            macByte4 = MACADR(pIfb->MAC).GetByte(4);
            macByte5 = MACADR(pIfb->MAC).GetByte(5);
        }

        sniprintf(NV_Settings.DeviceName, DEVICE_NAME_LENGTH, NB_FACTORY_MODULE_BASE_NAME NB_FACTORY_FEATURE_NAME "-%02X%02X", macByte4,
                  macByte5);
        NetbiosConvertName(NV_Settings.NetBIOSName, NV_Settings.DeviceName, sizeof(NV_Settings.NetBIOSName));

        /* Set to current verify key */
        NV_Settings.VerifyKey = NB_FACTORY_VERIFY_KEY;
        NV_Settings.STDEFFSVerifyKey = STD_EFFS_VERIFY_KEY;

        SaveUserParameters(&NV_Settings, sizeof(NV_Settings));

        debug_iprintf("Setting up new default storage info\r\n");
    }
    else
    {
        debug_iprintf("VerifyKey is Valid for NV_Settings\r\n");
    }

    /* SSL Keys */
    SslUserRetrieveCertificateNKey();

    return;
}

/**
 * @brief Prints the current NV settings to the display console.
 */
void DisplayNvSettings(void)
{
    /* NetBurner address configuration */
    iprintf("DeviceName: %s\r\n", NV_Settings.DeviceName);
    iprintf("NetBiosName: %s\r\n", NV_Settings.NetBIOSName);

    /* SSL certificate and keys file lengths */
    iprintf("SslCertificateSource: %d\r\n", NV_Settings.SslCertificateSource);
    iprintf("SslCertificateLength: %d\r\n", NV_Settings.SslCertificateLength);
    iprintf("SslKeyLength: %d\r\n", NV_Settings.SslKeyLength);

    /* Version verification key */
    iprintf("VerifyKey: %ld\r\n", NV_Settings.VerifyKey);
    /* Flash File System Version verification key */
    iprintf("StdEffsVerifyKey: %ld\r\n", NV_Settings.STDEFFSVerifyKey);
}

/**
 * @brief Displays the header on the web page.
 *
 * @param sock HTTP Socket
 * @param url Calling page
 */
void DisplayHeader(int sock, PCSTR url)
{
    if (url != nullptr)
    {
        if (httpstricmp(url, "HTTPS.HTML") == 1) { sBookMarkPtr = (char *)"#HTTPSConfig"; }
    }
    SendFileFragment((char *)"header.html", sock);
    sBookMarkPtr = (char *)"";
    return;
}

/**
 * @brief Displays the help bookmark on the web page.
 *
 * @param sock HTTP Socket
 * @param url Calling page
 */
void DisplayHelpBookmark(int sock, PCSTR url)
{
    writestring(sock, "<a href=\"help.html");
    writestring(sock, sBookMarkPtr);
    writestring(sock, "\">Help");
    return;
}

/**
 * @brief Displays the firmware version on the web page.
 *
 * @param sock HTTP Socket
 * @param url Calling page
 */
void DisplayFirmwareVersion(int sock, PCSTR url)
{
    writestring(sock, NB_FACTORY_DEFAULTS_VERSION_STRING);
    return;
}

/**
 * @brief Displays the device's name on the web page.
 *
 * @param sock HTTP Socket
 * @param url Calling page
 */
void DisplayDeviceName(int sock, PCSTR url)
{
    writestring(sock, NV_Settings.DeviceName);
    return;
}

/**
 * @brief Displays the trailing text at the bottom of the web page.
 *
 * @param sock HTTP Socket
 * @param url Calling page
 */
void DisplayTrailer(int sock, PCSTR url)
{
    writestring(sock, "<b>");
    writestring(sock, "Device Name: ");
    DisplayDeviceName(sock, url);
    writestring(sock, " | ");
    writestring(sock, "Version: ");
    DisplayFirmwareVersion(sock, url);
    writestring(sock, "</b>");
    writestring(sock, "<br><br><br>");
    return;
}

/**
 * @brief Displays which certificates are installed on the web site.
 *
 * @param sock HTTP Socket.
 * @param url Calling page.
 */
void DisplayCertificateInstalled(int sock, PCSTR url)
{
    if (NV_Settings.SslCertificateSource == SERIAL_BURNER_DEFAULT) { writestring(sock, NB_FACTORY_SSL_INCLUDED_DESC_DEFAULT); }
    else if (NV_Settings.SslCertificateSource == SERIAL_BURNER_USER_INSTALLED)
    {
        writestring(sock, NB_FACTORY_SSL_INSTALLED_DESC_DEFAULT);
    }
    else
    {
        writestring(sock, NB_FACTORY_SSL_PERMANENT_DESC_DEFAULT);
    }
    return;
}

/**
 * @brief Displays which keys are installed on the web site.
 *
 * @param sock HTTP Socket.
 * @param url Calling page.
 */
void DisplayHttpsKeyInstalled(int sock, PCSTR url)
{
    if (NV_Settings.SslCertificateSource == SERIAL_BURNER_DEFAULT) { writestring(sock, NB_FACTORY_SSL_INCLUDED_DESC_DEFAULT); }
    else if (NV_Settings.SslCertificateSource == SERIAL_BURNER_USER_INSTALLED)
    {
        writestring(sock, NB_FACTORY_SSL_INSTALLED_DESC_DEFAULT);
    }
    else
    {
        writestring(sock, NB_FACTORY_SSL_PERMANENT_DESC_DEFAULT);
    }
    return;
}

/**
 * @brief Displays the number of times the page has been reloaded.
 *
 * @param sock HTTP Socket.
 * @param url Calling page.
 */
void DisplayPageReloadCount(int sock, PCSTR url)
{
    static unsigned long count = 0;

    count++;
    fdprintf(sock, "%ld", count);
}

/**
 * @brief Displays the IP addresses for the different interfaces.
 *
 * @param sock HTTP Socket.
 * @param url Calling page.
 */
void webShowAddress(int socket, char *url)
{
    const int bufLen = 80;   // max IPv6 IP text representation is 45 chars
    static bool bInitialized = false;
    char buf[bufLen];

#if (defined IPV6)

    if (!bInitialized)
    {
        IPv6Interface *pIPv6If = IPv6Interface::GetFirst_IP6_Interface();
        IPv6LinkLocalAddress = pIPv6If->GetMyFirstAddress();
        IPv4Address = InterfaceIP(GetFirstInterface());
        bInitialized = true;
    }

    IPADDR localIP = SSL_GetSocketLocalAddr(socket);
    int32_t localPort = SSL_GetSocketLocalPort(socket);

    if (localIP.IsEmbeddedIPV4())
    {
        fdprintf(socket, "<br>Web browser request received on IPv4 address: ");
        localIP.fdprint(socket);   // will display either the IPv4 or v6 address as appropriate
        fdprintf(socket, " : %d<br>\r\n", localPort);
    }
    else
    {
        fdprintf(socket, "<br>Web browser request received on IPv6 address: ");
        localIP.fdprint(socket);   // will display either the IPv4 or v6 socket address as appropriate
        fdprintf(socket, " : %d<br>", localPort);
    }

    int len = snShowIP(buf, bufLen - 1, IPv4Address);
    buf[len] = '\0';
    writestring(socket, "<br><table border=\"0\">\r\n");
    writestring(socket, "<tr><td>Make a HTTP  IPv4 request to: </td>\r\n");
    fdprintf(socket, "<td><a href=\"http://%s\">http://%s</a></td></tr>\r\n", buf, buf);
    writestring(socket, "<tr><td>Make a HTTPS  IPv4 request to: </td>\r\n");
    fdprintf(socket, "<td><a href=\"https://%s\">https://%s</a></td></tr>\r\n", buf, buf);

    len = snShowIP(buf, bufLen - 1, IPv6LinkLocalAddress);
    buf[len] = '\0';
    writestring(socket, "<tr><td>Make a HTTP  IPv6 request to: </td>\r\n");
    fdprintf(socket, "<td><a href=\"http://[%s]\">http://%s</a></td></tr>\r\n", buf, buf);
    writestring(socket, "<tr><td>Make a HTTPS IPv6 request to: </td>\r\n");
    fdprintf(socket, "<td><a href=\"https://[%s]\">https://%s</a></td></tr>\r\n", buf, buf);
    writestring(socket, "</table>");

#endif
}
