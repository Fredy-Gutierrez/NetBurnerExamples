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


/*-------------------------------------------------------------------
 *   This module contains code for the web server interface.
 *------------------------------------------------------------------*/
#include <predef.h>
#include <stdio.h>
#include <stdlib.h>
#include <htmlfiles.h>
#include <http.h>
#include <string.h>
#include <iosys.h>
#include <iointernal.h>
#include <ctype.h>
#include <startnet.h>
//#include <multipartpost.h>
#include <nbssh/nbssh.h>
#include <httppost.h>

#include "http_f.h"
#include "effs_std.h"
#include "nvsettings.h"
#include "sshuser.h"
#include "nbfactory.h"

extern "C" {
   void DisplayRsaPublicKey( int sock, PCSTR url );
   void DisplayDsaPublicKey( int sock, PCSTR url );
   void DisplayRSAKeyInstalled( int sock, PCSTR url );
   void DisplayDSAKeyInstalled( int sock, PCSTR url );
   void DisplayKeyFileSelectionStatus( int sock, PCSTR url );
}

extern BOOL UserSaveData( char* dataPtr, int dataSize, const char* fileName );
extern NV_SettingsStruct NV_Settings;
extern BOOL bShowDebug;

/* Key file status */
static int sKeyFileStatus;


/*-------------------------------------------------------------------
 * DisplayRsaPublicKey
 * Called when a user clicks on the link to display the key
 *-----------------------------------------------------------------*/
void DisplayRsaPublicKey( int sock, PCSTR url )
{
   writestring( sock, "<BR>" );
   writestring( sock, "<BR>" );
   writestring( sock, "SSH RSA Public Key" );
   if ( NV_Settings.SshKeyRsaLength > 0 )
   {
      /* Publish key */
      writestring( sock, "<BR>" );
      writestring( sock, "<PRE>" );
      SshWritePublicKey( sock, (unsigned char*)gSshRsaKeyPemEncoded, NV_Settings.SshKeyRsaLength );
      writestring( sock, "</PRE>" );
   }
   else
   {
      writestring( sock, NB_FACTORY_SSH_PERMANENT_KEY_DESC_DEFAULT );
      writestring( sock, "Not Accessible" );
      writestring( sock, "<BR>" );
   }

   return;
}

/*-------------------------------------------------------------------
 * DisplayDsaPublicKey
 * Called when a user clicks on the link to display the key
 *-----------------------------------------------------------------*/
void DisplayDsaPublicKey( int sock, PCSTR url )
{
   writestring( sock, "<BR>" );
   writestring( sock, "<BR>" );
   writestring( sock, "SSH DSA Public Key" );
   if ( NV_Settings.SshKeyDsaLength > 0 )
   {
      /* Publish key */
      writestring( sock, "<BR>" );
      writestring( sock, "<PRE>" );
      SshWritePublicKey( sock, (unsigned char*)gSshDsaKeyPemEncoded, NV_Settings.SshKeyDsaLength );
      writestring( sock, "</PRE>" );
   }
   else
   {
      writestring( sock, NB_FACTORY_SSH_PERMANENT_KEY_DESC_DEFAULT );
      writestring( sock, "Not Accessible" );
      writestring( sock, "<BR>" );
   }

   return;
}


/*-------------------------------------------------------------------
 * DisplayRsaPublicKeyInstalled
 * Updates section of web page the displays the source of the active
 * key.
 *-----------------------------------------------------------------*/
void DisplayRSAKeyInstalled( int sock, PCSTR url )
{
#ifdef NB_FACTORY_INCLUDE_SSH
   if ( NV_Settings.SshKeyRsaSource == SSH_KEY_DEFAULT )
   {
      writestring( sock, NB_FACTORY_SSH_INCLUDED_KEY_DESC_DEFAULT );
   }
   else if ( NV_Settings.SshKeyRsaSource == SSH_KEY_USER_INSTALLED )
   {
      writestring( sock, NB_FACTORY_SSH_INSTALLED_KEY_DESC_DEFAULT );
   }
   else
   {
      writestring( sock, NB_FACTORY_SSH_PERMANENT_KEY_DESC_DEFAULT );
   }
#else /* #ifdef NB_FACTORY_INCLUDE_SSH */
   writestring( sock, "Not Available" );
#endif /* #ifdef NB_FACTORY_INCLUDE_SSH */
   return;
}

/*-------------------------------------------------------------------
 * DisplayDsaPublicKeyInstalled
 *-----------------------------------------------------------------*/
void DisplayDSAKeyInstalled( int sock, PCSTR url )
{
#ifdef NB_FACTORY_INCLUDE_SSH
   if ( NV_Settings.SshKeyDsaSource == SSH_KEY_DEFAULT )
   {
      writestring( sock, NB_FACTORY_SSH_INCLUDED_KEY_DESC_DEFAULT );
   }
   else if ( NV_Settings.SshKeyDsaSource == SSH_KEY_USER_INSTALLED )
   {
      writestring( sock, NB_FACTORY_SSH_INSTALLED_KEY_DESC_DEFAULT );
   }
   else
   {
      writestring( sock, NB_FACTORY_SSH_PERMANENT_KEY_DESC_DEFAULT );
   }
#else /* #ifdef NB_FACTORY_INCLUDE_SSH */
   writestring( sock, "Not Available" );
#endif /* #ifdef NB_FACTORY_INCLUDE_SSH */
   return;
}


/*-------------------------------------------------------------------
 * DisplayKeyFileSelectionStatus
 * Interface to the key file selection box on the web page.
 *-----------------------------------------------------------------*/
void DisplayKeyFileSelectionStatus( int sock, PCSTR url )
{
   if ( sKeyFileStatus != SSH_KEY_VALID )
   {
      writestring( sock, "<FONT COLOR=\"RED\">" );
   }
   switch ( sKeyFileStatus )
   {
      case SSH_KEY_VALID:
         writestring( sock, "                           " );
         break;
      case SSH_KEY_NOT_FOUND:
         writestring( sock, " File not found            " );
         break;
      case SSH_KEY_FILE_INVALID:
         writestring( sock, " File invalid              " );
         break;
      case SSH_KEY_INVALID:
         writestring( sock, " Key is invalid            " );
         break;
      default:
         writestring( sock, " Unknown                   " );
         break;
   }
   if ( sKeyFileStatus != SSH_KEY_VALID )
   {
      writestring( sock, "</FONT>" );
   }
   sKeyFileStatus = SSH_KEY_VALID;

   return;
}



/*-------------------------------------------------------------------
  captureFile
  Reads multipart form file from web browser.

   Parameters:
      fileDescriptor             - Active socket connected to web browser
      bufferPtr                  - Buffer to receive file
      bufferSize                 - Maximum number of uint8_ts
      capturedSize               - uint8_t count received

   Return:
      TRUE     - Buffer has contents of file
      FALSE    - File not accessible or too big or invalid parameters.

   Notes:
      keyBufferSize should be one more than is allowed, but the buffer should
      be that size.
 *-----------------------------------------------------------------------*/
BOOL captureFile( int fileDescriptor, unsigned char* bufferPtr, ssize_t bufferSize, ssize_t* capturedSize )
{
   ssize_t uint8_tsRead  = 0;
   BOOL isCaptured = FALSE;

   /* Open file */
   if ( ( bufferPtr != NULL ) && ( bufferSize > 0 ) && ( capturedSize != NULL ) )
   {
      *capturedSize = 0;
      uint8_tsRead = read( fileDescriptor, (char*)bufferPtr, bufferSize );
      if ( ( uint8_tsRead > 0 ) && ( uint8_tsRead < bufferSize ) )
      {
         /* Got it */
         *capturedSize = uint8_tsRead;
         isCaptured = TRUE;
      }
   }

   return isCaptured;
}


/*-------------------------------------------------------------------------
   SshKeyPost
   Called when a web browser from POST is received to upload a SSH key.
   The uploaded key file is tested with SshUserVerifyKey(), and if valid,
   the key is stored in the file system and NV_Settings is updated to
   indicate the application will use the new key.

   If the new key is valid, the application will automatically begin
   using that key for all new connections.

   Parameters:
      sock                       - fd of web page socket
      pData                      - Pointer to data submitted in form
      allDataPtr                 - Not used

   Return:
      0 - success, all else errors

   Notes:
      RSA or DSA keys for SSH protocols.

 *--------------------------------------------------------------------------*/
int SshKeysPost( int sock, const char* pData, const char* allDataPtr )
{
   int keyFd   = -1;
   int keyType = 0;
   int status  = 0;
   char* keyDataPtr    = NULL;
   ssize_t keyFileSize = 0;
   PCSTR responsePage  = "INDEX.HTM";

   /* Extract files */
   sKeyFileStatus = SSH_KEY_VALID;

   while ( TRUE )
   {
      /* Extract key file content first */
      keyFd = 0; //TODO:Fix from 2.x ExtractPostFile( "keysFile", pData, NULL );
      keyFileSize = 0;
      keyDataPtr = (char*)calloc( 1, ( SSH_KEY_SIZE_MAX_PEM + 1 ) );

      if ( ( keyFd > 0 ) && ( keyDataPtr != NULL ) )
      {
         // Receive file from web browser
         if ( captureFile( keyFd, (unsigned char*)keyDataPtr, SSH_KEY_SIZE_MAX_PEM, &keyFileSize ) == TRUE )
         {
            /* Validate key */
            if ( SshUserVerifyKey( keyDataPtr, keyFileSize, &keyType ) == TRUE )
            {
               debug_iprintf( "Key Verified->%d\r\n", keyType );
               if ( UserSaveData( keyDataPtr, keyFileSize, ( ( keyType == SSH_KEY_RSA )?
                     NB_FACTORY_SSH_FILE_NAME_KEY_RSA :
                     NB_FACTORY_SSH_FILE_NAME_KEY_DSA ) ) == TRUE )
               {
                  debug_iprintf( "Key Saved->%d\r\n", keyType );
                  if ( keyType == SSH_KEY_RSA )
                  {
                     (void)memset( gSshRsaKeyPemEncoded, '\0', sizeof( gSshRsaKeyPemEncoded ) );
                     (void)memcpy( gSshRsaKeyPemEncoded, keyDataPtr, keyFileSize );
                     NV_Settings.SshKeyRsaLength = keyFileSize;
                     NV_Settings.SshKeyRsaSource = SSH_KEY_USER_INSTALLED;
                     responsePage = "RSAKEY.HTM";
                  }
                  else
                  {
                     (void)memset( gSshDsaKeyPemEncoded, '\0', sizeof( gSshDsaKeyPemEncoded ) );
                     (void)memcpy( gSshDsaKeyPemEncoded, keyDataPtr, keyFileSize );
                     NV_Settings.SshKeyDsaLength = keyFileSize;
                     NV_Settings.SshKeyDsaSource = SSH_KEY_USER_INSTALLED;
                     responsePage = "DSAKEY.HTM";
                  }

                  /* Report success */
                  debug_iprintf( "Key valid and changed" );
                  SaveUserParameters( &NV_Settings, sizeof(NV_Settings) );
                  break;
               }
               else
               {
                  /* Problem with file or file system */
                  sKeyFileStatus = SSH_KEY_INVALID;
                  break;
               }
            }
            else
            {
               /* Key invalid */
               sKeyFileStatus = SSH_KEY_INVALID;
               break;
            }
         }
         else
         {
            /* Web page problem */
            sKeyFileStatus = SSH_KEY_INVALID;
            break;
         }
      }
      else
      {
         /* File not found on browser */
         sKeyFileStatus = SSH_KEY_NOT_FOUND;
         break;
      }
      /* Only once */
      break;

   } /* End while ( TRUE ) */

   /* Release memory */
   if ( keyDataPtr != NULL )
   {
      free( keyDataPtr );
      keyDataPtr = NULL;
   }

   /* Close file descriptors */
   if ( keyFd > 0 )
   {
      FreeExtraFd( keyFd );
      keyFd = -1;
   }

   RedirectResponse( sock, responsePage );

   return status;
}


/*-------------------------------------------------------------------
 * MyDoPost
 * Process the form post for uploading the file
 *------------------------------------------------------------------*/
//int MyDoPost( int sock, char *url, char *pData, char *rxBuffer )
//{
//   iprintf("Processing post for %s\r\n", url );

//   fs_chdrive( NOR_DRV_NUM );
//   fs_chdir( "\\" );

//   if ( strstr( url, "KEYPOST" ) != NULL )
//   {
//      iprintf("Entered SSH key post\r\n");
//      return SshKeysPost( sock, url, pData, rxBuffer );
//   }
//   else if ( strstr( url, "KEYRESET" ) != NULL )
//   {
//      iprintf("Resetting keys to factory defaults\r\n");
//      CheckNVSettings( TRUE );
//      RedirectResponse( sock, "INDEX.HTM" );
//   }

//   return 0;
//}





/*-------------------------------------------------------------------
 * WebListDir() - Displays a list of directories and files to the web
 * browser. Use a URL of http://x.x.x.x/dir to access.
 *------------------------------------------------------------------*/
void WebListDir( int sock, const char *dir )
{
   writestring( sock, "HTTP/1.0 200 OK\r\n" );
   writestring( sock, "Pragma: no-cache\r\n" );
   writestring( sock, "MIME-version: 1.0\r\n" );
   writestring( sock, "Content-Type: text/html\r\n\r\n" );
   writestring( sock, "<html>\r\n" );
   writestring( sock, "   <body>\r\n" );
   writestring( sock, "      <h2><font face=\"Arial\">Directory of " );
   writestring( sock, dir );
   writestring( sock, "</font></h2>\r\n" );
   writestring( sock, "      <hr>\r\n" );
   writestring( sock, "      <ul><font face=\"Courier New\" size=\"2\">\r\n" );

   FS_FIND f;
   int rc = fs_findfirst( "*", &f );

   while ( rc == 0 )
   {
      if ( f.attr & FS_ATTR_DIR )
      {
         writestring( sock, "         <li><img src=\"/folder.gif\"><a href=\"" );
         writestring( sock, f.filename );
         writestring( sock, "/\">" );
         writestring( sock, f.filename );
         writestring( sock, "</a>\r\n" );
      }
      else
      {
         writestring( sock, "         <li><img src=\"/text.gif\"><a href=\"" );
         writestring( sock, f.filename );
         writestring( sock, "\">" );
         writestring( sock, f.filename );
         writestring( sock, "</a>\r\n" );
      }

      rc = fs_findnext( &f );
   }

   writestring( sock, "      </font></ul>\r\n" );
   writestring( sock, "      <hr>\r\n" );
   writestring( sock, "   </body>\r\n" );
   writestring( sock, "</html>" );
}


/*-------------------------------------------------------------------
 * Parse file name in URL into 3 parts: name, extension and directory.
 * The standard file system functions typically ask for each of the
 * 3 parameters.
 *------------------------------------------------------------------*/
const int MaxFileNameLen = 256;
const int MaxFileExtLen = 10;
const int MaxFileDirLen = 256;

struct FileNamePartsType {
   char name[MaxFileNameLen];
   char ext[MaxFileExtLen];
   char dir[MaxFileDirLen];
};

void ParseFileNameFromUrl( char *Url, FileNamePartsType *Fname )
{
   //----- Parse and store file extension portion of URL -----
   //iprintf( "  URL: \"%s\"\r\n", Url );
   char *pext = Url + strlen( Url );

   while ( ( *pext != '.' ) && ( *pext != '/' ) && ( *pext != '\\' ) && ( pext > Url ) )
   {
      pext--;
   }

   if ( ( *pext == '.' ) || ( *pext == '\\' ) || ( *pext == '/' ) )
   {
      pext++;
   }

   strncpy( Fname->ext, pext, MaxFileExtLen - 1 );
   //iprintf("  URL extension: \"%s\"\r\n", Fname->ext );


   //----- Parse and store file name portion of URL -----
   char *pName = Url + strlen( Url );

   while ( ( *pName != '/' ) && ( *pName != '\\' ) && ( pName > Url ) )
   {
      pName--;
   }

   if ( ( *pName == '\\' ) || ( *pName == '/' ) )
   {
      pName++;
   }

   strncpy( Fname->name, pName, MaxFileNameLen );
   //iprintf("  URL file name: \"%s\"\r\n", Fname->name);

   // Store directory portion of URL
   strncpy( Fname->dir + 1, Url, ( pName - Url ) );
   Fname->dir[0] = '/';
   Fname->dir[( pName - Url ) + 1] = 0;
   //iprintf("  URL directory portion: \"%s\"\r\n", Fname->dir );
}


/*-------------------------------------------------------------------
 * MyDoGet
 * Process the HTTP Get request, then pass control to sytem handler.
 *------------------------------------------------------------------*/
//static http_gethandler *oldhand;

//int MyDoGet( int sock, PSTR url, PSTR rxBuffer )
//{
//   FileNamePartsType FileNamePart;
//   ParseFileNameFromUrl( url, &FileNamePart );

   // If the work "DIR" is specified in the URL at any directory level,
   // this code will result in a directory listng.
//   if ( (httpstricmp( FileNamePart.name, "DIR" ) == 1 ) && ( FileNamePart.name[0] != '\0' ) )
//   {
//      fs_chdrive( NOR_DRV_NUM );
//      fs_chdir( "\\" );
//      WebListDir( sock, FileNamePart.dir );
//      return 0;
//   }

//   return ( *oldhand ) ( sock, url, rxBuffer );
//}


int MyDoGet(int sock, HTTP_Request & pr )
{
    FileNamePartsType FileNamePart;
    ParseFileNameFromUrl( pr.pURL, &FileNamePart );
   
    fs_chdrive( NOR_DRV_NUM );
    fs_chdir( "\\" );
    WebListDir( sock, FileNamePart.dir );
    return 1;
}

void KeyPost( int sock, PostEvents event, const char * pName, const char * pValue)
{
   iprintf("pName: %s\r\n", pName );
   iprintf("pValue: %s\r\n", pValue );
   iprintf("event: %d\r\n", event );

   fs_chdrive( NOR_DRV_NUM );
   fs_chdir( "\\" );

   iprintf("Entered SSH key post\r\n");
   SshKeysPost( sock, pName, pValue );
}

void KeyReset( int sock,PostEvents event, const char * pName, const char * pValue)
{
   iprintf("pName: %s\r\n", pName );
   iprintf("pValue: %s\r\n", pValue );
   iprintf("event: %d\r\n", event );

   fs_chdrive( NOR_DRV_NUM );
   fs_chdir( "\\" );

   iprintf("Resetting keys to factory defaults\r\n");
   CheckNVSettings( TRUE );
   RedirectResponse( sock, "INDEX.HTM" );;
}

/*-------------------------------------------------------------------
 * RegisterWebFuncs
 *------------------------------------------------------------------*/
//void RegisterWebFuncs()
//{
//   oldhand = SetNewGetHandler( MyDoGet );
//   SetNewPostHandler( MyDoPost );
//}

// Handle Post
HtmlPostVariableListCallback p1("KEYPOST", KeyPost);
HtmlPostVariableListCallback p2("KEYPOST", KeyReset);

// Handle Get Requests
CallBackFunctionPageHandler g1( "DIR?", MyDoGet);

