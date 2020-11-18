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

/* NB Library Definitions */
#include <netbios.h>

/* Product Definitions */
#include "nbfactory.h"

/* Ethernet to Serial Application Definitions */
#include "serialburnerdata.h"

/* SSL User Routines */
#include "ssluser.h"

#define LOGME iprintf("%d->%s:%d\n", OSTaskID(), __FILE__, __LINE__);

/* Data file acquisition */
BOOL UserGetData(char *dataPtr, char *fileName, int dataSize);
bool IsSSL_CertNKeyValid(const char *keyPEM, ssize_t keylen, const char *certPEM, ssize_t certlen);

/* Default or installed user PEM encoded certificate */
char gSslCert[(SERIAL_BURNER_CERTIFICATE_SIZE_MAX_PEM + 1)];
char gSslKey[(SERIAL_BURNER_KEY_SIZE_MAX_PEM + 1)];
bool bCertLoaded;

/* Installed decoded certificate */
int sCertLength;
unsigned char sCert[(SERIAL_BURNER_CERTIFICATE_SIZE_MAX + 1)];

/* SSL permanent certificate and key */
extern const unsigned long comp_cert_len;
extern const unsigned char comp_cert[];
extern const unsigned long comp_key_len;
extern const unsigned char comp_key[];

void ReadyCertAndKeys()
{
    BOOL certificateOK = false;
    BOOL keyOK = false;
    BOOL success = 0; /* false */

    do
    {
        if (NV_Settings.SslCertificateSource == SERIAL_BURNER_LIBRARY_DEFAULT)
        {
            /* Certificate not included or installed */
            break;
        }
        if (NV_Settings.SslCertificateLength == 0)
        {
            debug_iprintf("Certificate status and size mismatch");
            break;
        }

        /* RSA Private Key */
        if (NV_Settings.SslKeyLength == 0)
        {
            debug_iprintf("Certificate key status and size mismatch");
            break;
        }

        if (!UserGetData(gSslCert, NB_FACTORY_SSL_FILE_NAME_CERT, NV_Settings.SslCertificateLength))
        { debug_iprintf("Could not load Certificate file"); }
        if (!UserGetData(gSslKey, NB_FACTORY_SSL_FILE_NAME_KEY, NV_Settings.SslKeyLength)) { debug_iprintf("Could not load Key file"); }
        if (!IsSSL_CertNKeyValid(gSslKey, strlen(gSslKey), gSslCert, strlen(gSslCert)))
        {
            debug_iprintf("Certificate key conversion failed");
            break;
        }

        /* Certificate is decoded */
        certificateOK = true;
        /* Key decoded */
        keyOK = true;
    } while (0);

    /* Success check */
    if ((certificateOK == true) && (keyOK == true)) { bCertLoaded = true; }
}

/**
 * @brief Returns the compiled-in private key (local) or the private key
 * from the other (alternate) source.
 */
const char *GetPrivateKeyPEM()
{
    if (NV_Settings.SslCertificateSource != SERIAL_BURNER_LIBRARY_DEFAULT)
    {
        if (!bCertLoaded) { ReadyCertAndKeys(); }
        if (bCertLoaded) { return gSslKey; }
    }

    /* Certificate not included or installed */
    return (const char *)comp_key;
}

/**
 * @brief Returns the compiled-in cert (local) or the cert from the
 * other (alternate) source.
 */
const char *GetCertificatePEM()
{
    if (NV_Settings.SslCertificateSource != SERIAL_BURNER_LIBRARY_DEFAULT)
    {
        if (!bCertLoaded) { ReadyCertAndKeys(); }
        if (bCertLoaded) { return gSslCert; }
    }

    /* Certificate not included or installed */
    return (const char *)comp_key;
}
/**
 * "C" Functions
 */
extern "C"
{
    /**
     * @brief Resets the SSL certificate to the NetBurner default.
     */
    void SslUserSetDefault(void)
    {
        ssize_t keyLength = comp_key_len;
        ssize_t certLength = comp_cert_len;

        /* Reset settings */
        NV_Settings.SslCertificateSource = SERIAL_BURNER_LIBRARY_DEFAULT;
        NV_Settings.SslCertificateLength = 0;
        NV_Settings.SslKeyLength = 0;

        while (true)
        {
            /* Size checks */
            if ((keyLength <= 0) || (certLength <= 0))
            {
                debug_iprintf("Default certificate and key size problems");
                break;
            }
            /* Validate certificate and key pair */
            if (IsSSL_CertNKeyValid((const char *)comp_key, keyLength, (const char *)comp_cert, certLength) == false)
            {
                debug_iprintf("Default certificate and key invalid");
                break;
            }

            /* Certificate */
            if (UserSaveData((char *)comp_cert, certLength, NB_FACTORY_SSL_FILE_NAME_CERT) == false)
            {
                debug_iprintf("Certificate save failed");
                break;
            }

            /* Key */
            if (UserSaveData((char *)comp_key, keyLength, NB_FACTORY_SSL_FILE_NAME_KEY) == false)
            {
                debug_iprintf("Key save failed");
                break;
            }

            /* Success */
            debug_iprintf("Certificate and key valid and installed");
            NV_Settings.SslCertificateLength = certLength;
            NV_Settings.SslKeyLength = keyLength;
            NV_Settings.SslCertificateSource = SERIAL_BURNER_DEFAULT;

            /* Only once */
            break;

        } /* End while ( true ) */

        return;
    }

    /**
     * @brief Retrieves the stored certificate and key, and resets them to their default values
     * if it's unable to do so.
     */
    void SslUserRetrieveCertificateNKey(void)
    {
        BOOL resetKey = false;

        /* Key */
        while (NV_Settings.SslCertificateSource != SERIAL_BURNER_LIBRARY_DEFAULT)
        {
            if (NV_Settings.SslKeyLength > 0)
            {
                (void)memset(gSslKey, 0, sizeof(gSslKey));
                if (UserGetData((char *)gSslKey, (char *)NB_FACTORY_SSL_FILE_NAME_KEY, NV_Settings.SslKeyLength) == false)
                {
                    /* Problems, Reset data */
                    debug_iprintf("Error retrieving->%s", NB_FACTORY_SSL_FILE_NAME_KEY);
                    resetKey = true;
                    break;
                }
            }
            else
            {
                debug_iprintf("Ssl key size error");
                resetKey = true;
                break;
            }

            /* Certificate */
            if (NV_Settings.SslCertificateLength > 0)
            {
                (void)memset(gSslCert, 0, sizeof(gSslCert));
                if (UserGetData((char *)gSslCert, (char *)NB_FACTORY_SSL_FILE_NAME_CERT, NV_Settings.SslCertificateLength) == false)
                {
                    /* Problems, Reset data */
                    debug_iprintf("Error retrieving->%s", NB_FACTORY_SSL_FILE_NAME_CERT);
                    resetKey = true;
                    break;
                }
            }
            else
            {
                debug_iprintf("Ssl certificate size error");
                resetKey = true;
                break;
            }

            /* Only once */
            break;

        } /* End while ( NV_Settings.SslCertificateSource != 0 ) */

        /* Reset on error */
        if (resetKey == true)
        {
            /* Reset settings */
            NV_Settings.SslCertificateSource = SERIAL_BURNER_LIBRARY_DEFAULT;
            NV_Settings.SslCertificateLength = 0;
            NV_Settings.SslKeyLength = 0;
            SaveUserParameters(&NV_Settings, sizeof(NV_Settings));
            debug_iprintf("Reset SSL keys to library default caused by error\r\n");
            resetKey = false;
        }

        return;
    }
};
/* End extern "C" */
