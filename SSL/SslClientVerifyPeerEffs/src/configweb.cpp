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

#include <crypto/ssl.h>
#include <fdprintf.h>
#include <htmlfiles.h>
#include <http.h>
#include <iosys.h>
#include <ipv6/ipv6_interface.h>
#include <netinterface.h>

#include "effs_std.h"
#include "nbfactory.h"
#include "serialburnerdata.h"

#ifdef DIAG_LOG
#include "log.h"
#endif

extern "C"
{
    /* Banner and help menu */
    void DisplayHeader(int sock, PCSTR url);
    void DisplayHelpBookmark(int sock, PCSTR url);
}

/* User parameters */
NV_SettingsStruct NV_Settings;

/* User parameters change candidate */
NV_SettingsStruct gNV_SettingsChangeCopy;

/* User parameters change flag */
volatile bool gChangedUserParameters;

/* Used in determining the bookmark header on the website */
static char *sBookMarkPtr = (char *)"";

/**
 * @brief Set user parameter defaults in NV_Settings.
 *
 * <b>Notes:</b>
 *    These NV_SettingsStruct fields must be set after calling this routine for a valid record:
 *       UserName
 *       Password
 *       IP_Addr_mode
 *       DeviceName
 *
 *    Any user installed keys and the certificate are erased as well.
 */
void SetDefaults(void)
{
    /* Reset all to zeros */
    (void)memset(&NV_Settings, 0, sizeof(NV_Settings));
}

/**
 * @brief Resets user parameters to their default values and saves them.
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
 * <b>Notes:</b>
 *    These NV_SettingsStruct fields a reset after calling this the default
 *          setting routine for a valid record:
 *       UserName
 *       Password       - Resetting requires a configuration record setting.
 *       IP_Addr_mode
 *       DeviceName
 *
 *    Any user installed keys and the certificate are erased if it is reset.
 */
void CheckNVSettings(bool returnToFactory)
{
    NV_SettingsStruct *pData = (NV_SettingsStruct *)GetUserParameters();
    NV_Settings = *pData;

    if ((NV_Settings.STDEFFSVerifyKey != STD_EFFS_VERIFY_KEY) || (returnToFactory == true))
    {
        debug_iprintf("Formatting Flash Drive...\r\n");
        int rc = fs_format(NOR_DRV_NUM);
        if (rc != FS_NOERR) debug_iprintf("*** ERROR Formatting Flash Drive: %d\r\n", rc);
        NV_Settings.STDEFFSVerifyKey = STD_EFFS_VERIFY_KEY;
        SaveUserParameters(&NV_Settings, sizeof(NV_Settings));
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

    return;
}

/**
 * @brief Displays the current NV settings
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
        if (httpstricmp(url, "HTTPS.HTM") == 1) { sBookMarkPtr = (char *)"#HTTPSConfig"; }
    }
    SendFileFragment((char *)"header.htm", sock);
    sBookMarkPtr = (char *)"";
    return;
}

/**
 * @brief Displays the help option on the website's bookmark.
 *
 * @param sock HTTP Socket
 * @param url Calling page
 */
void DisplayHelpBookmark(int sock, PCSTR url)
{
    writestring(sock, "<a href=\"help.htm");
    writestring(sock, sBookMarkPtr);
    writestring(sock, "\">Help");
    return;
}
