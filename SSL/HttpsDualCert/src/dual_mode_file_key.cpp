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


/**
 * This is an example of how to use both a compiled-in certificate and
 * private key, and also a cert/key loaded from an alternate "other" source
 * such as an external flash card. The term "other" is used because the example
 * can be modified to use sources other than an external flash card.
 */

// NB Libs
#include <malloc.h>
#include <stdio.h>

#include "cardtype.h"
#include "FileSystemUtils.h"
#include <effs_fat/fat.h>
#ifdef USE_MMC
#include <effs_fat/mmc_mcf.h>
#endif
#ifdef USE_CFC
#include <effs_fat/cfc_mcf.h>
#endif

#include <effs_fat/effs_utils.h>

bool bChecked_For_key = false;

// Name of files on Flash card
#define CERT_FILE_NAME "DEVICE.CRT"
#define KEY_FILE_NAME "DEVICE.KEY"

extern const unsigned long comp_cert_len;
extern const unsigned char comp_cert[];
extern const unsigned long comp_key_len;
extern const unsigned char comp_key[];;

char *fCert;
int fCertLen;
char *fKey;
int fKeyLen;

bool IsSSL_CertNKeyValid(const char *keyPEM, ssize_t keylen,
    const char *certPEM, ssize_t certlen);

/**
 * @brief Read a file from the Flash card and return an allocated buffer
 * (malloc). The buffer will be released after the key & cert file
 * data is converted to the NetBurner binary format.
 *
 * @param name The name of the file.
 * @param[out] buf The buffer to read the file into.
 * @param[out] len The length of the buffer.
 *
 * @retval A pointer to the allocated buffer (the same as `buf`)
 */
char * ReadFileToBlob(const char *name, char *&buf, int &len)
{
    F_FILE* fp = f_open(name, "r");

    if (fp)
    {
        f_seek(fp, 0, SEEK_END);
        int file_length = (int)f_tell(fp);
        f_seek(fp, 0, SEEK_SET);
        if (file_length > 0)
        {
            // If we have a buffer, but the current length is less than what we need, free it
            if (buf && (len < file_length))
            {
                free(buf);
            }
            
            // If we don't have a buffer, try to get one
            if( buf == nullptr )
            {
                buf = (char *)malloc(file_length);
            }
            
            if (!buf) { len = 0; return nullptr; }
            int n = f_read(buf, 1, file_length, fp);
            f_close(fp);

            if (n > 0)
            {
                len = file_length;
                return buf;
            }
        }
        else {
            f_close(fp);
        }
    }
    if (buf) { free(buf); }
    len = 0;
    return nullptr;
}

/**
 * @brief Check the Flash card for the certificate and key files. If found,
 * read the data and convert from PEM format to the NetBurner binary
 * format used by the NetBurner SSL library. If successful, free
 * the buffers malloced in the Read function.
 */
void Check_For_FileKeys()
{
    bChecked_For_key = true;

    f_enterFS();
    f_chdrive(EXT_FLASH_DRV_NUM);

    // Create pointer to certificate blob
    ReadFileToBlob(CERT_FILE_NAME, fCert, fCertLen);

    // Create pointer to private key blob
    ReadFileToBlob(KEY_FILE_NAME, fKey, fKeyLen);

    f_releaseFS();

    // Check if the cert and key are valid
    if (IsSSL_CertNKeyValid(fKey, fKeyLen, fCert, fCertLen)) {
        return;
    }
    else {
        iprintf("*** Error reading key or certificate\r\n");
    }

    if (fCert) {
        free(fCert);
        fCert = nullptr;
        fCertLen = 0;
    }

    if (fKey) {
        free(fKey);
        fKey = nullptr;
        fKeyLen = 0;
    }
}


/**
 * @brief Returns the compiled-in private key (local) or the private key
 * from the other (alternate) source.
 *
 * @retval Pointer to the found key
 */
const char* GetPrivateKeyPEM()
{
    if (!bChecked_For_key)
    {
        Check_For_FileKeys();
    }

    return (fKey) ? (fKey) : (const char *)comp_key;
}

/**
* @brief Returns the compiled-in cert (local) or the cert from the
* other (alternate) source.
*
* @retval Pointer to the found cert
*/
const char* GetCertificatePEM()
{
    if (!bChecked_For_key)
    {
        Check_For_FileKeys();
    }

    return (fCert) ? (fCert) : (const char *)comp_cert;
}

