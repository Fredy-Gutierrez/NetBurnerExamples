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

/* NB Runtime Libraries */
#include <iosys.h>

/* NB Internals */
#include <iointernal.h>

/* NB Conversion */
#include <http.h>
#include <httppost.h>

/* NB EFFS-STD Library */
#include <file/fsf.h>

/* NB SSL/TLS Library */
#include <crypto/SharkSslCert.h>
#include <crypto/ssl.h>

/* Product Definitions */
#include "nbfactory.h"

/* Ethernet to Serial Application Definitions */
#include "serialburnerdata.h"

/**
 * HTML Certificate and Key file status
 */
#define SERIAL_BURNER_VALID (0)
#define SERIAL_BURNER_NOT_FOUND (1)
#define SERIAL_BURNER_INVALID (2)
#define SERIAL_BURNER_CERTIFICATE_INVALID (3)
#define SERIAL_BURNER_KEY_INVALID (4)

/**
 * The following is used for Certificate Authority Lists
 */
#define MAX_CA_RECORDS (20)

// Used for certificate authority list operations
typedef struct
{
    int n;
    char sname[SHARKSSL_MAX_SNAME_LEN];
} certFileMap_t;
certFileMap_t CertFileMap[MAX_CA_RECORDS];
SharkSslCertStore gCAStore;
SharkSslCAList gCAList = 0;
char BigBuffer[SERIAL_BURNER_CERTIFICATE_SIZE_MAX_PEM + 1];
static bool bCaCertsScanned = false;

// Checks for changes in UserParameters
void CheckForChanges();

extern "C"
{
    void DisplayClientCert(int sock, PCSTR url);
    void CCertAction(int sock, PCSTR url);
}

/**
 * @brief Capture file specified on the web page.
 *
 * @param fileDescriptor File set on the page
 * @param bufferPtr Buffer to receive file
 * @param bufferSize Maximum number of bytes
 * @param capturedSize Byte count received
 *
 * @retval TRUE Buffer has contents of file
 * @retval FALSE File not accessible or too big or invalid parameters.
 *
 * <b>Notes:</b> `keyBufferSize` should be one more than is allowed, but
 * the buffer should be that size.
 */
bool captureFile(int fileDescriptor, unsigned char *bufferPtr, ssize_t bufferSize, ssize_t *capturedSize)
{
    ssize_t bytesRead = 0;
    bool isCaptured = false;

    /* Open file */
    if ((bufferPtr != nullptr) && (bufferSize > 0) && (capturedSize != nullptr))
    {
        *capturedSize = 0;
        bytesRead = read(fileDescriptor, (char *)bufferPtr, bufferSize);
        if ((bytesRead > 0) && (bytesRead < bufferSize))
        {
            /* Got it */
            *capturedSize = bytesRead;
            isCaptured = true;
        }
    }

    return isCaptured;
}

/**
 * @brief Save data
 *
 * @param dataPtr Data
 * @param dataSize Size in bytes
 * @param fileName File name
 *
 * @retval TRUE Okay
 * @retval FALSE Invalid
 *
 * <b>Notes:</b> Overwrites existing file.
 */
bool UserSaveData(char *dataPtr, int dataSize, const char *fileName)
{
    bool saved = false;

    if ((dataPtr != nullptr) && (dataSize > 0) && (fileName != nullptr))
    {
        FS_FILE *fsFilePtr;
        int rc = fs_delete(fileName);
        if (rc != FS_NOERR) debug_iprintf("UserSaveData fs_delete error->%d, %s\r\n", rc, fileName);

        fsFilePtr = fs_open(fileName, "w");
        if (fsFilePtr != nullptr)
        {
            long written = fs_write(dataPtr, 1, dataSize, fsFilePtr);
            if (dataSize == written) { saved = true; }
            else
            {
                debug_iprintf("UserSaveData fs_write error->%d, %ld\r\n", dataSize, written);
            }
            (void)fs_close(fsFilePtr);
            fsFilePtr = nullptr;
        }
        else
        {
            debug_iprintf("UserSaveData open failed");
        }
    }

    return saved;
}

/**
 * @brief Get data
 *
 * @param dataPtr Data
 * @param fileName File name
 * @param dataSize Size in bytes
 *
 * @retval TRUE Okay
 * @retval FALSE Unable to get user data
 */
bool UserGetData(char *dataPtr, char *fileName, int dataSize)
{
    bool gotten = false;

    if ((dataPtr != nullptr) && (dataSize > 0) && (fileName != nullptr))
    {
        FS_FILE *fsFilePtr;
        fsFilePtr = fs_open(fileName, "r");
        if (fsFilePtr != nullptr)
        {
            long bytesRead = fs_read(dataPtr, 1, dataSize, fsFilePtr);
            if (bytesRead == dataSize) { gotten = true; }
            else
            {
                debug_iprintf("UserGetData fs_read error ->%d, %ld", dataSize, bytesRead);
            }
            int status = fs_close(fsFilePtr);
            if (status != FS_NOERR) { debug_iprintf("UserGetData fs_close error->%d", status); }
            fsFilePtr = nullptr;
        }
        else
        {
            debug_iprintf("UserGetData open failed on->%s", fileName);
        }
    }

    return gotten;
}

/**
 * @brief Build the Certificate Authority List objects needed by SharkSSL from the files
 * in the file system. It will search for all files matching the regular expression
 * `CACert*.crt`.
 */
void RescanCACerts()
{
    FS_FIND finder;   // location to store the information retrieved

    for (int i = 0; i < MAX_CA_RECORDS; i++)
    {
        CertFileMap[i].n = 0;
        CertFileMap[i].sname[0] = '\0';
    }

    /**
     * Find first file or subdirectory in specified directory. First call the
     * f_findfirst function, and if file was found get the next file with
     * f_findnext function. Files with the system attribute set will be ignored.
     *
     * Note: If f_findfirst() is called with "*.*" and the current directory is
     * not the root directory, the first entry found will be "." - the current
     * directory.
     */
    volatile int rc = fs_findfirst("CACert*.crt", &finder);
    if (rc == F_NO_ERROR)   // found a file or directory
    {
        // Make sure we don't leak memory if we've called RescanCACerts() before
        SharkSslCertStore_destructor(&gCAStore);
        gCAList = 0;
        int Refill_pos = 0;

        SharkSslCertStore_constructor(&gCAStore);
        do
        {
            if ((finder.attr & FS_ATTR_DIR)) {}
            else
            {
                int n = 0;
                n = atoi(finder.filename + 6);
                FS_FILE *fsFilePtr;
                fsFilePtr = fs_open(finder.filename, "r");
                if (fsFilePtr != nullptr)
                {
                    long bytesRead = fs_read(BigBuffer, 1, SERIAL_BURNER_CERTIFICATE_SIZE_MAX_PEM, fsFilePtr);
                    (void)fs_close(fsFilePtr);
                    fsFilePtr = nullptr;
                    if (bytesRead > 100)
                    {
                        SharkSslCertStore nameStore;
                        SharkSslCertStore_constructor(&nameStore);

                        SharkSslCertStore_add(&gCAStore, BigBuffer, bytesRead);
                        SharkSslCertStore_add(&nameStore, BigBuffer, bytesRead);

                        DoubleListEnumerator iter;
                        SharkSslCSCert *cert;
                        DoubleListEnumerator_constructor(&iter, &nameStore.certList);
                        if ((Refill_pos < MAX_CA_RECORDS) && (cert = (SharkSslCSCert *)DoubleListEnumerator_getElement(&iter)))
                        {
                            strncpy(CertFileMap[Refill_pos].sname, cert->name, SHARKSSL_MAX_SNAME_LEN);
                            CertFileMap[Refill_pos++].n = n;
                        }

                        // Note that we call the destructor for nameStore here because it's just used to populate the CertFileMap.
                        // gCAStore must stick around so that when connections are made, the data is available to use in validation.
                        SharkSslCertStore_destructor(&nameStore);
                    }
                }
            }
        } while (!fs_findnext(&finder));

        if (!SharkSslCertStore_assemble(&gCAStore, &gCAList))
        { iprintf("Unable to assemble the CA List.  Verify peer will not work.\r\n"); }
    }

    bCaCertsScanned = true;
}

/**
 * @brief Writes the certs in the Certificate Authority List.
 *
 * @param sock The socket to write the certificate data to.
 * @param url
 */
void DisplayClientCert(int sock, PCSTR url)
{
    static char name_buffer[80];
    static char ssl_cert_tempbuf[256];

    if (!bCaCertsScanned) RescanCACerts();

    for (int i = 0; i < MAX_CA_RECORDS; i++)
        if (CertFileMap[i].sname[0])
        {
            name_buffer[0] = 0;
            snprintf(ssl_cert_tempbuf, 256,
                     "<tr><td>%s</td><td><A href="
                     "CCertAction.html?S%d"
                     ">Show PublicKey</A></td><td><A href="
                     "CCertAction.html?D%d"
                     ">Delete %s</A></td></tr>\r\n",
                     CertFileMap[i].sname, i, i, CertFileMap[i].sname);
            writestring(sock, ssl_cert_tempbuf);
        }
}

/**
 * @brief Add a Certificate Authority certificate file the the file system.
 *
 * @param cert The certificate data.
 * @param len The length of the certificate data.
 */
void AddCACert(const unsigned char *cert, int len)
{
    static char fnbuffer[20];
    int CertSeq = 0;
    FS_FIND finder;   // location to store the information retrieved

    /** Find first file or subdirectory in specified directory. First call the
     *  f_findfirst function, and if file was found get the next file with
     *  f_findnext function. Files with the system attribute set will be ignored.
     *  Note: If f_findfirst() is called with "*.*" and the current directory is
     *  not the root directory, the first entry found will be "." - the current
     *  directory.
     */
    volatile int rc = fs_findfirst("CACert*.crt", &finder);
    if (rc == F_NO_ERROR)   // found a file or directory
    {
        do
        {
            if ((finder.attr & FS_ATTR_DIR)) {}
            else
            {
                int n = 0;
                n = atoi(finder.filename + 6);
                if (n > CertSeq) CertSeq = n;
            }
        } while (!fs_findnext(&finder));
    }

    sniprintf(fnbuffer, 20, "CACert%d.crt", CertSeq + 1);
    UserSaveData((char *)cert, len, fnbuffer);
}

/**
 * @brief Deletes a certificate authority file from the CA List, and then tries to rebuild
 * the SharkSSL certificate authority information based on the remaining files.
 *
 * @param sock HTTP socket.
 * @param event The kind of post event that is currently being handled with this callback.
 * @param pName The name of the post element that is currently being handled.
 * @param pValue The value of the post element that is currently being handled.
 */
void CaDelPost(int sock, PostEvents event, const char *pName, const char *pValue)
{
    PCSTR responsePage = "CaCert.html";
    static int n = -1;
    static char exbuffer[80];
    exbuffer[0] = 0;

    // We received a variable
    if (event == eVariable)
    {
        if (strcmp(pName, "filetodelete") == 0) { n = atoi(pValue); }
        else if (strcmp(pName, "action") == 0)
        {
            if (strcmp(pValue, "Confirm") == 0)
            {
                sniprintf(exbuffer, 80, "CACert%d.crt", n);
                fs_delete(exbuffer);
                RescanCACerts();
            }
        }
    }
    else if (event == eEndOfPost)
    {
        RedirectResponse(sock, responsePage);
    }
}

/**
 * @brief Handles the request to add a certificate.
 *
 * @param sock HTTP Page
 * @param url URL of calling page
 * @param pData Data from the web page which should contain the cert data.
 * @param allDataPtr Not used
 *
 * @retval 0 Success
 * @retval !0 Errors
 *
 * <b>Notes:</b> Certificate and keys used for secure web site.
 */
void CaCertPost(int sock, PostEvents event, const char *pName, const char *pValue)
{
    PCSTR responsePage = "CaCert.html";
    char *certificateDataPtr = nullptr;
    ssize_t certificateFileSize = 0;

    // We received a file
    if (event == eFile)
    {
        gNV_SettingsChangeCopy = NV_Settings;
        gNV_SettingsChangeCopy.VerifyKey = NB_FACTORY_VERIFY_KEY;
        gChangedUserParameters = false;
        FilePostStruct *pFps = (FilePostStruct *)pValue;

        /* Extract files */
        int CaCertificateFileStatus = SERIAL_BURNER_VALID;

        /* Extract certificate file content */
        certificateFileSize = 0;
        certificateDataPtr = (char *)calloc(1, (SERIAL_BURNER_CERTIFICATE_SIZE_MAX_PEM + 1));
        if (dataavail(pFps->fd) && (certificateDataPtr != nullptr))
        {
            if (captureFile(pFps->fd, (unsigned char *)certificateDataPtr, SERIAL_BURNER_CERTIFICATE_SIZE_MAX_PEM, &certificateFileSize) ==
                false)
            { CaCertificateFileStatus = SERIAL_BURNER_INVALID; }
            else
            {
                AddCACert((const unsigned char *)certificateDataPtr, certificateFileSize);
                RescanCACerts();
                gChangedUserParameters = true;
            }
        }
        else
        {
            CaCertificateFileStatus = SERIAL_BURNER_NOT_FOUND;
        }

        /* Release buffers */
        if (certificateDataPtr != nullptr)
        {
            free(certificateDataPtr);
            certificateDataPtr = nullptr;
            certificateFileSize = 0;
        }

        FreeExtraFd(pFps->fd);
    }
    else if (event == eEndOfPost)
    {
        CheckForChanges();
        RedirectResponse(sock, responsePage);
    }
}

/**
 * @brief Provides web content for confirming removal of certs, as well as displaying
 * the contents of a cert.
 *
 * @param sock The socket to write the web content to.
 * @param url The URL that was passed to the server from the browser. It's used to hold form
 * parameters for this page.
 */
void CCertAction(int sock, PCSTR url)
{
    static char Resultbuffer[512];
    while ((*url) && (*url != '?'))
    {
        url++;
    }
    if (url)
    {
        int n = atoi(url + 2);
        if ((n < MAX_CA_RECORDS) && (n >= 0))
        {
            if (url[1] == 'D')
            {
                writestring(sock,
                            "<form action="
                            "cadelpost.html"
                            "  method=POST>\r\n");
                static char FileNameBuf[20];
                sniprintf(FileNameBuf, 20, "CACert%d.crt", CertFileMap[n].n);
                sniprintf(Resultbuffer, 512, "Delete Cert [%s]", CertFileMap[n].sname);
                writestring(sock, Resultbuffer);
                writestring(sock,
                            "<input type="
                            "hidden"
                            "  name="
                            "filetodelete"
                            " value="
                            "");
                sniprintf(FileNameBuf, 20,
                          "%d"
                          " />\r\n",
                          CertFileMap[n].n);
                writestring(sock, FileNameBuf);
                writestring(sock,
                            "<br><hr><input type="
                            "submit"
                            " name="
                            "action"
                            " value="
                            "Confirm"
                            ">  <input type="
                            "submit"
                            " name="
                            "action"
                            " value="
                            "Cancel"
                            "></form>\r\n");
            }
            else
            {
                if (url[1] == 'S')
                {
                    sniprintf(Resultbuffer, 512, "Show Certificate Authority record for [%s] <BR><BR>", CertFileMap[n].sname);
                    writestring(sock, Resultbuffer);

                    if (CertFileMap[n].sname[0] == '\0')
                    {
                        writestring(sock, "No Certificate");
                        return;
                    }
                    static char FileNameBuf[20];
                    sniprintf(FileNameBuf, 20, "CACert%d.crt", CertFileMap[n].n);
                    FS_FILE *fsFilePtr = fs_open(FileNameBuf, "r");
                    if (!fsFilePtr)
                    {
                        sniprintf(Resultbuffer, 512, "Invalid file [%s]", FileNameBuf);
                        writestring(sock, Resultbuffer);
                        return;
                    }
                    static char BigBuffer[SERIAL_BURNER_CERTIFICATE_SIZE_MAX_PEM + 1];
                    long bytesRead = fs_read(BigBuffer, 1, SERIAL_BURNER_CERTIFICATE_SIZE_MAX_PEM, fsFilePtr);
                    (void)fs_close(fsFilePtr);
                    writestring(sock, "<FONT face=Courier New>\r\n");

                    int out = 0;
                    for (int i = 0; i < bytesRead; i++)
                    {
                        switch (BigBuffer[i])
                        {
                            case '\r':
                                if (BigBuffer[i + 1] != '\n')
                                {
                                    Resultbuffer[out++] = BigBuffer[i++];
                                    break;
                                }
                                // fallthrough
                            case '\n':
                                Resultbuffer[out++] = '<';
                                Resultbuffer[out++] = 'B';
                                Resultbuffer[out++] = 'R';
                                Resultbuffer[out++] = '>';
                                break;
                            default: Resultbuffer[out++] = BigBuffer[i];
                        }
                        if (out > 500)
                        {
                            Resultbuffer[out] = 0;
                            writestring(sock, Resultbuffer);
                            Resultbuffer[0] = 0;
                            out = 0;
                        }
                    }
                    if (Resultbuffer[0])
                    {
                        Resultbuffer[out] = 0;
                        writestring(sock, Resultbuffer);
                    }
                    writestring(sock, "<BR></font>\r\n");
                }
                else
                {
                    sniprintf(Resultbuffer, 512, "Unknown action [%c]", url[1]);
                    writestring(sock, Resultbuffer);
                }
            }
        }
    }
    else
    {
        writestring(sock, "Error processing request");
    }
}
