Doxygen Format
/** @pagegroup{EFFS-FAT,pageExamples-EFFS-Fat-HttpVar,FAT HTML Variables}

This example program sets up HTTP access to the FAT file system on either
 MultiMedia Cards (MMC) or CompactFlash Cards (CFC).
 
 To run this example:
 
 1. Compile the example for your chosen flash card in cardtype.h. The
NetBurner development board has a card slot for MMC/SD cards. You need
a CompactFlash interface board if you want to use CompactFlash cards.
Note: If you compile this example for CFC without a CFC interface board
installed on the development board, then the program will cause a trap.
 
 2. Copy index.htm and and MIME.txt from the example's root directory onto
your flashcard, or put your own there.
 
 3. When the program runs, status messages will be displayed on the module
debug serial port.
 
 4. You can view the files on the flash card with any web browser by typing
in the IP address of the module at the "/DIR" folder
Example: "http://10.1.1.57/DIR").
 
 5. When accessing the module via browser, what web pages will be displayed
depends on the following order and their availability on the flash card:
 
1) index.htm or index.html files
2) The first available .htm file (if no index.htm or index.html files)
3) The first available .html file (if no .htm files)
4) A web page list of all directories and files that exist on the flash
 card
 
 6. If .htm/html files exist on the flash card, then the program will
always access the first available web page file by default. If you want
to access the web page list of all existing directories and files on the
flash card, then you type in the URL of the module, followed by "/dir"
(Example: "http://10.1.1.57/dir").
 
 If you are using NBEclipse, then you will also need to tell the linker to
 include the FatFile.a library.
 
 To do this:
- In NBEclipse, right-click on your project, and select "Properties"
- Select "C/C++ Builds -> Settings" on the left-hand side
- Select "GNU C/C++ Linker -> Libraries" under the "Tool Settings" tab
- In the "Libraries" list box, add "FatFile" by using the action icons
provided in top-right corner of the list
 
 Additionally, the EFFS FAT common files will need to be imported directly into the
 project.
 
 To do this:
 - In NBEclipse, right-click on your project, and select "Import"
 - From the "Import" window that pops up, select General -> File System
 - Click the "Browse" button, navigate to `\nburn\examples\_common\EFFS\FAT`
 - Select all of the files listed except for the makefile, and click
 "Finish"

*/