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

#ifndef _SSLUSER_H_
#define _SSLUSER_H_

/* NB Secure Sockets Layer (SSL)  */
#include <crypto/ssl.h>

/* Default or installed user PEM encoded certificate */
extern char gSslCert[(SERIAL_BURNER_CERTIFICATE_SIZE_MAX_PEM + 1)];
extern char gSslKey[(SERIAL_BURNER_KEY_SIZE_MAX_PEM + 1)];

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Checks and installs SSL default certificate and key.
     *
     * <b>Notes:</b> Sets NV_Settings elements:
     *   - SslCertificateSource;
     *   - SslCertificateLength;
     *   - SslKeyLength;
     */
    void SslUserSetDefault(void);

    /**
     * @brief Retrieves and set certificate and key.
     *
     * <b>Notes:</b> Clears SSL settings for CertificateNKeysDataStatus element of
     * `struct NV_SettingsStruct` if retrieval error occurs.
     */
    void SslUserRetrieveCertificateNKey(void);

#ifdef __cplusplus
};
#endif

#endif /* _SSLUSER_H_ */
