Doxygen Format
/** @pagegroup{FTP,FTPD_AppUpdate,FTP App Update}

 This program is an example of how you can use the FTP server to allow application firmware updates via FTP. To run the example:
 
 1. Compile this example program
 2. Download the image file to the NetBurner board
 3. Run a FTP client program from a host computer on the network. In this example, we use the Windows FTP program called "ftp.exe" 
    which is run from a command prompt. Type @code ftp <ip address>, where <ip address> @endcode is the IP address of the NetBurner board. 
	For example, @code ftp 10.1.1.21 @endcode
 
 4. You will be prompted for a user name and password. You can enter anything here.
 
 5. In the FTP client program, type "ls" and hit the return key to see a list of files. Below is an example of a FTP session showing 
    commands and responses so far:

@code 
ftp 10.1.1.21
Connected to 10.1.1.21.
220 Welcome to the NetBurner FTP server.
User (10.1.1.21:(none)): asdf
331 User name okay, need password.
Password:
230 User logged in, proceed.
ftp> ls
200 Port Command okay.
125 Data connection already open; transfer starting.
UserFlash.s19
UserFlash.bin
Application_APP.s19
226 Closing data connection. Listing complete.
ftp: 51 bytes received in 0.00Seconds 51000.00Kbytes/sec.
@endcode 
 
 6. The file name we are interested in is "Application_APP.s19". Uploading an image file with this file name will program 
    that application into flash memory.
 
 7. Choose any valid image file to upload, and rename it to Appliation_APP.s19. 
 
 8. Now that you have the file Application_APP.s19 in the same directory you are running
ftp.exe from, at the ftp> prompt type "put Application_APP.s19". After this command
executes, the tic-tac-toe application will be programmed in flash. However, the
board is still running the ftp application in SDRAM, so you won't see a change
until the board reboots.
 
 9. All that's left is to reboot the board so the tic-tac-toe application starts up.
The example is written so that closing the Windows client ftp session will
reboot the board. At the Windows ftp> prompt, type "quit" and press the enter
key. An example session is shown below:
 
@code 
ftp> put Application_APP.s19
200 Port Command okay.
125 Data connection already open; transfer starting.
226 Closing data connection. File transfer complete.
ftp: 184620 bytes sent in 1.16Seconds 159.16Kbytes/sec.
ftp> quit
221 Service closing control connection.
@endcode
 
 10. Use a web browser and go to the board's IP address. You should see the tic-tac-toe application running.

*/