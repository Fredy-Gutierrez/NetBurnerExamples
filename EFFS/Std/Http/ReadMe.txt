Doxygen Format
/** @pagegroup{EFFS-STD,EFFS-STD-HTTP,STD HTTP}

Before you can compile and run this example, you need to modify the linker
 to replace libFatFile.a and use the libStdFFile.a library for on-chip flash.The
 compcode flags will also have to be modified so that the application will
 only occupy a specific space in flash, leaving room for the on-chip flash
 file system.
 
 1. Changes to compcode flags:In NBEclipse or your command line makefile,
change the following line so the application will only occupy the
specified application space.The first parameter is the start of the
application space, and the second is the address just below the flash file
system space.What range you use depends on the flash chip your module is
using.
 
If you are using the Spansion AM29LV160B, S29AL016D, or Atmel AT49BV163D
flash chip, use the following memory address range:
 
 `COMPCODEFLAGS = 0xFFC0800 0xFFD80000`
 
If you are using the SST39VF040 flash chip, use the following memory
address range:
 
 `COMPCODEFLAGS = 0xFFC0800 0xFFC70000`
 
Chips for other devices can be referenced at the top of effStdFlashDrv.cpp.
 
If you are using NBEclipse, do the following to make the `COMPCODEFLAGS`
modification (assuming you have imported this example into an existing
NBEclipse project and already open):
 
1) Right-click on your opened project in the Project Explorer and select
 "Properties" from the pop-up list box.
2) Expand the "C/C++ Build" tree on the left-hand side of the "Properties"
 dialog box and select "Settings".
3) In the "Settings" section on the right-hand side, select the "Tool
 Settings" tab and scroll down to "NetBurner Compcode".Select its
 "General" sub-section.
4) On the right-hand side that appears for the "General" sub-section,
 modify the "Memory range" text box with the appropriate values for your
 flash chip as indicated above.
5) Press the "OK" button when done.
 
 2. If you are using NBEclipse, then you will also need to tell the linker to
include the "/nburn/platform/<platform>/original/lib/libStdFFile.a" library.
 
To do this:
- In NBEclipse, right-click on your project, and select "Properties"
- Select "C/C++ Builds -> Settings" on the left-hand side
- Select "GNU C/C++ Linker -> Libraries" under the "Tool Settings" tab
- In the "Libraries" list box, add "StdFFile" by using the action icons
provided in top-right corner of the list box
 
Additionally, the EFFS STD common files will need to be imported directly into the
project.
 
To do this:
- In NBEclipse, right-click on your project, and select "Import"
- From the "Import" window that pops up, select General -> File System
- Click the "Browse" button, navigate to "/nburn/examples/_common/EFFS/STD"
- Select all of the files listed except for the makefile, and click
"Finish"

 This example program sets up HTTP access to the EFFS-STD on-chip flash file
 system.  To run this example:
 
 1. When the program runs, status messages will be displayed on the module
    debug serial port.
 
 2. You can view the files on the flash card with any web browser by typing in
    the IP address of the module, followed by "/DIR" (not case-sensitive).
 
    Example:  http://10.1.1.57/DIR
 
 3. When accessing the module via browser, what web pages will be displayed
    depends on the following order and their availability on the flash card:
 
    1) "index.htm" or "index.html" files
    2) The first available *.htm file (if no "index.htm or "index.html" files)
    3) The first available *.html file (if no *.htm files)
    4) A web page list of all directories and files that exist on the flash
       card
 
 4. If *.htm/html files exist on the flash chip, then the program will always
    access the first available web page file by default.  If you want to
    access the web page list of all existing directories and files, type in
    the URL of the module, followed by "/DIR".

*/