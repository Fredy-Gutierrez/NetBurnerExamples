Doxygen Format
/** @pagegroup{EFFS-FAT,pageExamples-EFFS-Fat-Basic,FAT Basic}

This program illustrates basic file system operations for SD/MMC
 and Compact Flash cards:
 - Mounting a drive
 - Determining amount of used and free file space
 - Creating files
 - Writing data
 - Reading data
 - Unmounting a drive
 
 When the program executes it will display program status information
 through the debug serial port.
 
 This application has web server support for onboard flash/ram only,
 web pages cannot be run from external flash cards. This capability
 will be added in the EFFS-HTTP example to illustrate the difference
 between the two operations.
 
 If you are using NBEclipse, then you will also need to tell the linker to
 include the "/nburn/platform/<platform>/original/lib/libFatFile.a" library.
 
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
 - Click the "Browse" button, navigate to "/nburn/examples/_common/EFFS/FAT"
 - Select all of the files listed except for the makefile, and click
 "Finish"

*/