Doxygen Format
/** @pagegroup{EFFS-FAT,pageExamples-EFFS-Fat-RamMinimal,FAT RAM}

This program illustrates basic file system operations for RAM drives:
 - Mounting a drive
 - Determining amount of used and free file space
 - Creating files
 - Writing data
 - Reading data
 - Unmounting a drive
 
 When the program executes it will display program status information
 through the debug serial port.
 
 This example is primarily different from the EFFS-BASIC example in that
 it by default uses the RAM drive for the file system and is used to show
 what is necessary to use the RAM drive. Namely, the project must include
 the content contained within the 'ramdrvMcf.cpp' source file.To build
 any of the EFFS- examples to use the RAM drive, simply add the
 'ramdrvMcf.cpp' file to the build list and modify 'cardType.h' to define
 USE_RAM instead of USE_MMC or USE_CFC and set the EXT_FLASH_DRV_NUM
 to the correct RAM drive number.
 
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