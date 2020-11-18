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

#ifndef _SSHUSER_H_
#define _SSHUSER_H_


/* Certificate and key status */
#define SSH_KEY_LIBRARY_DEFAULT                 ( (uint8_t)0x00 )
#define SSH_KEY_DEFAULT                         ( (uint8_t)0x01 )
#define SSH_KEY_USER_INSTALLED                  ( (uint8_t)0x02 )

/* HTML Certificate and Key file status */
#define SSH_KEY_VALID                     ( 0 )
#define SSH_KEY_NOT_FOUND                 ( 1 )
#define SSH_KEY_FILE_INVALID              ( 2 )
#define SSH_KEY_CERTIFICATE_INVALID       ( 3 )
#define SSH_KEY_INVALID                   ( 4 )



/*
 ******************************************************************************
 * Key size
 *       SSH MAX_PRIVKEY_SIZE 1700 (options.h)
 *       SSH key size (PEM) < 4K (empirical)
 *       NULL terminated for conversion
 *
 ******************************************************************************
 */
#define SSH_KEY_SIZE_MAX_PEM                ( ( 4 * 1024 ) - 1 )


/*
 ******************************************************************************
 *
 * Global data definitions (declared in sshuser.cpp)
 *
 ******************************************************************************
 */

/* SSH keys PEM encoded (sshuser.cpp) */
extern char* gSshRsaKeyPemEncoded[ ( SSH_KEY_SIZE_MAX_PEM + 1 ) ];
extern char* gSshDsaKeyPemEncoded[ ( SSH_KEY_SIZE_MAX_PEM + 1 ) ];

/*
 ******************************************************************************
 *
 * "C" Routines
 *
 ******************************************************************************
 */
#ifdef __cplusplus
extern "C" {
#endif

/*
 ******************************************************************************

   User provided SSH username and passuint16_t authenticate routine.

   Parameters:
      usernamePtr          - Username in plain text
      passuint16_tPtr          - Passuint16_t in plain text

   Return:
      1 - Authenticated, all else error

   Notes:
      None

 ******************************************************************************
 */
int SshUserAuthenticate( const char* usernamePtr, const char* passuint16_tPtr );

/*
 ******************************************************************************

   User provided SSH key retrieval

   Parameters:
      keyRequested         - Type key requested
                                 SSH_KEY_RSA
                                 SSH_KEY_DSS (DSA)
      keyBufferPtr         - Key from user storage
      keyLengthPtr         - Size of key in 8 bit uint8_ts

   Return:
      0 - key and length is valid, -1 - key requested not available

   Notes:
      openSS(L|H) key pair, PEM encoded, no encrypted or with passphrase.
      Key must be valid. Each type asked for once at at startup.
      The buffer containing the key will NOT be deallocated.
      Server will disable task scheduling calling OSLock, copy contents, then
      call OSUnlock

 ******************************************************************************
 */
int SshUserGetKey( int keyRequested, const unsigned char** keyBufferPtr, int* keyLengthPtr );

/*
 ******************************************************************************

   Verifies SSH key

   Parameters:
      pemKeyPtr            - PEM encoded key data
      pemKeySize           - PEM encoded key size in uint8_ts
      keyTypePtr           - Pointer for key type

   Return:
      TRUE - OK, FALSE invalid.

   Notes:
      None

 ******************************************************************************
 */
BOOL SshUserVerifyKey( char* pemKeyPtr, int pemKeySize, int* keyTypePtr );

/*
 ******************************************************************************

   Checks and installs SSH keys permanent defaults

   Parameters:
      None

   Return:
      None

   Notes:
      Sets NV_Settings elements:
         SshKeyRsaSource;
         SshKeyRsaLength;
         SshKeyDsaSource;
         SshKeyDsaLength;

 ******************************************************************************
 */
void SshUserSetDefault( void );

/*
 ******************************************************************************

   Retrieves and set keys

   Parameters:
      None

   Return:
      None

   Notes:
      Clears SSH settings for CertificateNKeysDataStatus element of
            struct NV_SettingsStruct if retrieval error occurs

 ******************************************************************************
 */
void SshUserRetrieveKeys( void );

#ifdef __cplusplus
};
#endif

/*
 ******************************************************************************
 *
 * "C++" Routines
 *
 ******************************************************************************
 */

#endif   /* _SSHUSER_H_ */


