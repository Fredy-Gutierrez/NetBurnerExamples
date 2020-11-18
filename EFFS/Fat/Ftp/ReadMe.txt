Doxygen Format
/** @pagegroup{EFFS-FAT,pageExamples-EFFS-Fat-Ftp,FAT FTP}

This program illustrates file system and FTP operations for SD/MMC
 and Compact Flash cards:
- Mounting a flash drive
- Determining amount of used and free file space
- Using FTP to upload and download files using Internet Explorer or
other FTP client. Note that at the time this example was written
Firefox did not support FTP uploading.
 
 When the program executes it will display program status information
 through the debug serial port.
 
 Modules with an onboard microSD flash socket should use the multi MMC header
 files and functions because the modules are capable of supporting both onboard
 and external flash cards (even if you application only uses one).
 
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