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
#include <predef.h>

/* C Standard Library */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Portability & uCos Definitions */
#include <basictypes.h>    /* startnet.h & includes.h */

/* NB Runtime Libraries */
#include <constants.h>     /* startnet.h */
#include <system.h>

/* NB TCP/IP Infrastructure */
#include <nettypes.h>
#include <buffers.h>

/* NB  Dynamic Host Configuration (DHCP) Client */
#include <dhcpclient.h>

#include "effs_std.h"

/* NB Secure Shell (SSH)  */
#include <nbssh/nbssh.h>

/* Product Definitions */
#include "nbfactory.h"

#include "sshuser.h"
#include "nvsettings.h"

extern BOOL bShowDebug;
extern NV_SettingsStruct NV_Settings;


/* SSH keys PEM encoded (sshuser.cpp) */
char* gSshRsaKeyPemEncoded[ ( SSH_KEY_SIZE_MAX_PEM + 1 ) ];
char* gSshDsaKeyPemEncoded[ ( SSH_KEY_SIZE_MAX_PEM + 1 ) ];


/* SSH permanent keys */
static const char* sSshKeyRsaDefaultPemEncoded =
   #include "permanentkeyrsa.h"
   "\0";

static const char* sSshKeyDsaDefaultPemEncoded =
   #include "permanentkeydss.h"
   "\0";


/*-------------------------------------------------------------------
   User SSH username and password authenticate callback routine.
   Once assigned, this callback will be executed by the SSH library
   upon acceptance of a SSH connection. Add you application
   specific authentication code to this function.

   Parameters:
      usernamePtr          - Username in plain text
      passwordPtr          - Password in plain text

   Return:
      1 - Authenticated, all else error

   Notes:
      None
 ---------------------------------------------------------------------*/
int SshUserAuthenticate( const char* usernamePtr, const char* passwordPtr )
{
   // For testing reject the set if they are the same
   if ( strcmp( usernamePtr, passwordPtr ) == 0 )
      return FALSE;  // fail
   else
      return TRUE;   // pass
}

/*-------------------------------------------------------------------
  UserSaveData
  Saves the key in dataPtr to the file system.

   Parameters:
      dataPtr              - Key data to save
      dataSize             - Size in bytes
      fileName             - File name

   Return:
      TRUE - OK, FALSE invalid.

   Notes:
      Overwrites existing file.
 *-------------------------------------------------------------------*/
BOOL UserSaveData( char* dataPtr, int dataSize, const char* fileName )
{
   BOOL saved = FALSE;

   if ( ( dataPtr != NULL ) && ( dataSize > 0 ) && ( fileName != NULL ) )
   {
      FS_FILE* fsFilePtr;

      (void)fs_delete( fileName );

      fsFilePtr = fs_open( fileName, "w" );
      if ( fsFilePtr != NULL )
      {
         long written = fs_write( dataPtr, 1, dataSize, fsFilePtr );
         if ( dataSize == written )
         {
            saved = TRUE;
         }
         else
         {
            debug_iprintf( "UserSaveData fs_write error->%d, %d\r\n", dataSize, written );
         }
         (void)fs_close( fsFilePtr );
         fsFilePtr = NULL;
      }
      else
      {
         debug_iprintf( "UserSaveData open failed" );
      }

   }

   return saved;
}


/*------------------------------------------------------------------
   UserGetData
   Reads key file from file system and stores it in specified buffer.

   Parameters:
      dataPtr              - Data
      fileName             - File name
      dataSize             - Size in bytes

   Return:
      TRUE - File successfully read, otherwise FALSE
 *-------------------------------------------------------------------*/
BOOL UserGetData( char* dataPtr, char* fileName, int dataSize )
{
   BOOL success = FALSE;

   if ( ( dataPtr != NULL ) && ( dataSize > 0 ) && ( fileName != NULL ) )
   {
      FS_FILE* fsFilePtr;
      fsFilePtr = fs_open( fileName, "r" );
      if ( fsFilePtr != NULL )
      {
         long bytesRead = fs_read( dataPtr, 1, dataSize, fsFilePtr );
         if ( bytesRead == dataSize )
         {
            success = TRUE;
         }
         else
         {
            debug_iprintf( "UserGetData fs_read error ->%d, %d", dataSize, bytesRead );
         }

         int status = fs_close( fsFilePtr );
         if ( status != FS_NOERR )
         {
            debug_iprintf( "UserGetData fs_close error->%d", status );
         }

         fsFilePtr = NULL;
      }
      else
      {
         debug_iprintf( "UserGetData open failed on->%s", fileName );
      }
   }

   return success;
}



/*-------------------------------------------------------------------
   Modifies the keyBufferPtr pointer to point at the current SSH
   public/private key pair for the specified keyRequested (RSA or DSA).

   If defined, this callback function will always be called on an SshAccept() or
   SshNegotiate().

   Parameters:
      keyRequested   - SSH_KEY_RSA or SSH_KEY_DSS
      keyBufferPtr   - Pointer to character buffer to hold SSH key
      keyLengthPtr   - Number of bytes in SSH key for storage purposes.
                       This has no relation to the key bit size.

   Return:
      -1 - No valid key
       0 - Success

   Notes:
      None
 ---------------------------------------------------------------------*/
int SshUserGetKey( int keyRequested, const unsigned char** keyBufferPtr, int* keyLengthPtr )
{
   int keyValid = -1;

   if ( ( keyBufferPtr != NULL ) && ( keyLengthPtr != NULL ) )
   {
      if ( ( keyRequested == SSH_KEY_RSA ) &&
            ( NV_Settings.SshKeyRsaSource != 0 ) &&
            ( NV_Settings.SshKeyRsaLength > 0 ) )
      {
            *keyLengthPtr = NV_Settings.SshKeyRsaLength;
            *keyBufferPtr = (const unsigned char*)gSshRsaKeyPemEncoded;
            keyValid = 0;
      }
      else if ( ( keyRequested == SSH_KEY_DSS ) &&
            ( NV_Settings.SshKeyDsaSource != 0 ) &&
            ( NV_Settings.SshKeyDsaLength > 0 ) )
      {
            *keyLengthPtr = NV_Settings.SshKeyDsaLength;
            *keyBufferPtr = (const unsigned char*)gSshDsaKeyPemEncoded;
            keyValid = 0;
      }
   }  /* End if ( ( keyBufferPtr != NULL ) && ( keyLengthPtr != NULL ) ) */

   return keyValid;
}


/*-------------------------------------------------------------------
   Validate specified SSH PEM Key

   Parameters:
      pemKeyPtr      - Pointer to buffer containing SSH key to test
      pemKeySize     - Number of bytes in SSH key for storage purposes.
                       This has no relation to the key bit size.
      pemKeyTypePtr  - SSH_KEY_RSA or SSH_KEY_DSS

   Return:
      -1 - No valid key
       0 - Success

   Notes:
      None
 ---------------------------------------------------------------------*/
BOOL SshUserVerifyKey( char* pemKeyPtr, int pemKeySize, int* keyTypePtr )
{
   int keyType = 0;
   BOOL keyIsValid = FALSE;

   if ( pemKeySize > 0 )
   {
      /* Check SSH validity */
      if ( SshValidateKey( pemKeyPtr, pemKeySize, &keyType ) == TRUE )
      {
         keyIsValid = TRUE;
      }
   }

   /* Key type */
   if ( ( keyIsValid == TRUE ) && ( keyTypePtr != NULL ) )
   {
      *keyTypePtr = keyType;
   }

   return keyIsValid;
}



/*-------------------------------------------------------------------
   Called when no user configuration exists, or to reset configuration
   to factory defaults. The RSA and DSA key files will be overwritten
   with the Application Compiled-in keys if they are valid, or the
   NetBurner SSH library keys if they are not valid.

   Parameters: None

   Returns: Nothing
 ---------------------------------------------------------------------*/
void SshUserSetDefault( void )
{
   ssize_t rsaKeyLength = strnlen( sSshKeyRsaDefaultPemEncoded, SSH_KEY_SIZE_MAX_PEM );
   ssize_t dsaKeyLength = strnlen( sSshKeyDsaDefaultPemEncoded, SSH_KEY_SIZE_MAX_PEM );
   int keyType;

   /* RSA key */
   debug_iprintf( "RSA Key->%d\r\n", rsaKeyLength );
   NV_Settings.SshKeyRsaSource = SSH_KEY_LIBRARY_DEFAULT;
   NV_Settings.SshKeyRsaLength = 0;

   if ( SshUserVerifyKey( (char*)sSshKeyRsaDefaultPemEncoded, rsaKeyLength, &keyType ) == TRUE )
   {
      debug_iprintf( "RSA Key Verified->%d\r\n", keyType );
      if ( UserSaveData( (char*)sSshKeyRsaDefaultPemEncoded, rsaKeyLength, NB_FACTORY_SSH_FILE_NAME_KEY_RSA ) == TRUE )
      {
         debug_iprintf( "RSA Key Saved->%d\r\n", keyType );
         NV_Settings.SshKeyRsaSource = SSH_KEY_DEFAULT;
         NV_Settings.SshKeyRsaLength = rsaKeyLength;
      }
   }

   /* DSA key */
   debug_iprintf( "DSA Key->%d\r\n", rsaKeyLength );
   NV_Settings.SshKeyDsaSource = SSH_KEY_LIBRARY_DEFAULT;
   NV_Settings.SshKeyDsaLength = 0;

   if ( SshUserVerifyKey( (char*)sSshKeyDsaDefaultPemEncoded, dsaKeyLength, &keyType ) == TRUE )
   {
      debug_iprintf( "Dsa Key Verified->%d\r\n", keyType );
      if ( UserSaveData( (char*)sSshKeyDsaDefaultPemEncoded, dsaKeyLength,
            NB_FACTORY_SSH_FILE_NAME_KEY_DSA ) == TRUE )
      {
         debug_iprintf( "Dsa Key Saved->%d\r\n", keyType );
         NV_Settings.SshKeyDsaSource = SSH_KEY_DEFAULT;
         NV_Settings.SshKeyDsaLength = dsaKeyLength;
      }
   }

   return;
}



/*-------------------------------------------------------------------
   Called when no user configuration exists, or to reset configuration
   to factory defaults. Copies the keys from the file system into
   the global key variables and sets corresponding the NV_Settings values.

   Parameters: None

   Returns: Nothing
 ---------------------------------------------------------------------*/
void SshUserRetrieveKeys( void )
{
   BOOL resetKey = FALSE;

   /* RSA key */
   (void)memset( gSshRsaKeyPemEncoded, '\0', sizeof( gSshRsaKeyPemEncoded ) );
   if ( NV_Settings.SshKeyRsaSource != SSH_KEY_LIBRARY_DEFAULT )
   {
      if ( NV_Settings.SshKeyRsaLength <= SSH_KEY_SIZE_MAX_PEM )
      {
         if ( UserGetData( (char*)gSshRsaKeyPemEncoded, NB_FACTORY_SSH_FILE_NAME_KEY_RSA,
               NV_Settings.SshKeyRsaLength ) == FALSE )
         {
            /* Problems, Reset data */
            debug_iprintf( "Error retrieving->%s", NB_FACTORY_SSH_FILE_NAME_KEY_RSA );
            NV_Settings.SshKeyRsaSource = SSH_KEY_LIBRARY_DEFAULT;
            NV_Settings.SshKeyRsaLength = 0;
            resetKey = TRUE;
         }
      }
      else
      {
         /* Problems, Reset data */
         debug_iprintf( "Size error retrieving->%s", NB_FACTORY_SSH_FILE_NAME_KEY_RSA );
         NV_Settings.SshKeyRsaSource = SSH_KEY_LIBRARY_DEFAULT;
         NV_Settings.SshKeyRsaLength = 0;
         resetKey = TRUE;
      }
   }

   /* DSA key */
   (void)memset( gSshDsaKeyPemEncoded, '\0', sizeof( gSshDsaKeyPemEncoded ) );
   if ( NV_Settings.SshKeyDsaSource != SSH_KEY_LIBRARY_DEFAULT )
   {
      if ( NV_Settings.SshKeyDsaLength <= SSH_KEY_SIZE_MAX_PEM )
      {
         if ( UserGetData( (char*)gSshDsaKeyPemEncoded, NB_FACTORY_SSH_FILE_NAME_KEY_DSA,
               NV_Settings.SshKeyDsaLength ) == FALSE )
         {
            /* Problems, Reset data */
            debug_iprintf( "Error retrieving->%s", NB_FACTORY_SSH_FILE_NAME_KEY_DSA );
            NV_Settings.SshKeyDsaSource = SSH_KEY_LIBRARY_DEFAULT;
            NV_Settings.SshKeyDsaLength = 0;
            resetKey = TRUE;
         }
      }
      else
      {
         /* Problems, Reset data */
         debug_iprintf( "Size error retrieving->%s", NB_FACTORY_SSH_FILE_NAME_KEY_DSA );
         NV_Settings.SshKeyDsaSource = SSH_KEY_LIBRARY_DEFAULT;
         NV_Settings.SshKeyDsaLength = 0;
         resetKey = TRUE;
      }
   }

   /* Reset defaults on error */
   if ( resetKey == TRUE )
   {
      SaveUserParameters( &NV_Settings, sizeof( NV_Settings ) );
      debug_iprintf("Reset SSH keys to library default caused by error\r\n");
      resetKey = FALSE;
   }

   return;
}




