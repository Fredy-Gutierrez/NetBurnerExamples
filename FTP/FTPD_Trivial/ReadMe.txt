Doxygen Format
/** @pagegroup{FTP,FTPD_Trivial,Trivial FTP Server}

This example will create an FTP Server running on the NetBurner board. The
 FTP Server will start up and wait for a FTP client connection. The FTP
 Server provides two functions:
 
 1. Allows the FTP Client to download a single file, which has fixed content.
After downloading, you can view this file on your host computer.
 
 2. Allows the FTP Client to upload an ASCII text file. The file is not
stored in memory, but is sent out the COM1 serial port so you can see it
with a serial terminal program like MTTTY.
 
 This example only allows one file to be sent to the board, which must be named
 WriteFile.txt. The file contents are sent to the COM1 serial port to be displayed; 
 the file is not stored in memory. The method you would use to store a file depends 
 on your application. One way is to allocate memory and store the file data as an 
 array of bytes. A second way is to add a full file system.

*/