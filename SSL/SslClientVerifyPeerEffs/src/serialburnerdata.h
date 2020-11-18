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

#ifndef _SERIALBURNERDATA_H_
#define _SERIALBURNERDATA_H_

#include <config_obj.h>
#include <netbios.h>

extern MonitorRecord monitor_config;

#define DEVICE_NAME_LENGTH (15)

/**
 * Key or certificate size
 *    SSL Certificate size 2200 (empirical)
 *    OpenSSL format is Privacy-enhanced Electronic Mail (PEM) encoded
 *    NULL terminated for conversion
 */
#define SERIAL_BURNER_CERTIFICATE_SIZE_MAX ((5 * 1024) - 1)
#define SERIAL_BURNER_CERTIFICATE_SIZE_MAX_PEM ((5 * 1024) - 1)

/* Certificate and key status */
#define SERIAL_BURNER_LIBRARY_DEFAULT ((uint8_t)0x00)
#define SERIAL_BURNER_DEFAULT ((uint8_t)0x01)
#define SERIAL_BURNER_USER_INSTALLED ((uint8_t)0x02)

/**
 * Key size
 *    SSH MAX_PRIVKEY_SIZE 1700 (options.h)
 *    SSH key size (PEM) < 4K (empirical)
 *    NULL terminated for conversion
 */
#define SERIAL_BURNER_KEY_SIZE_MAX_PEM ((4 * 1024) - 1)

/**
 * Booting support
 */
#define boot_iprintf(...)                             \
    {                                                 \
        if (monitor_config.Quiet == 0)                \
        {                                             \
            iprintf("%s : ", NV_Settings.DeviceName); \
            iprintf(__VA_ARGS__);                     \
            iprintf("\r\n");                          \
        }                                             \
    }

/**
 * Debug support
 */
#define debug_iprintf(...)                            \
    {                                                 \
        if (bShowDebug == true)                       \
        {                                             \
            iprintf("%s : ", NV_Settings.DeviceName); \
            iprintf(__VA_ARGS__);                     \
            iprintf("\r\n");                          \
        }                                             \
    }

/**
 * @brief Configuration Settings
 *
 * DeviceName              - Device name for DHCP
 * NetBIOSName             - NetBIOS name
 *
 * * SSL and SSH *
 * CertificateRsaLength    - Certificate length
 * CertificateData         - Certificate
 * KeyHttpsRsaLength       - RSA key for HTTPS length, 0 is none
 * KeyHttpsRsaData         - RSA key for HTTPS
 * KeyRsaLength            - RSA key length, 0 is none
 * KeyRsaData              - RSA key
 * KeyDsaLength            - DSA key length, 0 is none
 * KeyDsaData              - DSA key
 *
 * * Version change key *
 * VerifyKey               - Version change key
 */
struct NV_SettingsStruct
{
    /* NetBurner address configuration */
    char DeviceName[(DEVICE_NAME_LENGTH + 1)];
    char NetBIOSName[(NETBIOS_NAME_SIZE_IN_CHARS + 1)];

    /* SSL certificate and keys file lengths */
    uint8_t SslCertificateSource;
    uint16_t SslCertificateLength;
    uint16_t SslKeyLength;

    /* Version verification key */
    uint32_t VerifyKey;
    /* Flash File System Version verification key */
    uint32_t STDEFFSVerifyKey;
};

/* User parameters */
extern NV_SettingsStruct NV_Settings;

/* User parameters change candidate */
extern NV_SettingsStruct gNV_SettingsChangeCopy;

/* User parameters change flag */
extern volatile bool gChangedUserParameters;

/* Debugging flag */
extern bool bShowDebug;

/* Register a post */
extern void RegisterPost(void);

extern void CheckNVSettings(bool returnToFactory = false);

extern void SetAndSaveDefaults(void);

/* For processing hexadecimal break key value */
extern char GetHexByte(const char *cp);

#endif /* _SERIALBURNERDATA_H_ */
