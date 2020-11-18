Doxygen Format
/** @pagegroup{FTP,FTP_Client,FTP Client}

This program will run a FTP Client program on the NetBurner board
 to do the following:
 - Connect to a FTP Server
 - Change to a remote directory called "test1"
 - Obtain a directory listing and print the listing to stdout
 - Create a file on the FTP Server called WriteTest.txt
 - Read back contents of WriteTest.txt and send to stdout
 
 Setup Requirements:
 1. Access to a FTP Server
 2. User name and password for the FTP Server
 3. A directory called "test1" must exist on the FTP Server
 4. Write permissions to the test1 directory
 5. You must modify the @code #define @endcode values in this file to match your FTP server.

*/