Doxygen Format
/** @pagegroup{EFFS-FAT,pageExamples-EFFS-Fat-Performance,FAT File Sys Speed Test}

This example is used to test the speed of the EFFS FAT file system for operations
 such as file creation and data storage speed. All testing was done on a MOD54415.
 
 Hardware: MOD54415-100IR on MOD-DEV-70 development board
 Kingston microSDHC 32GB Class 4, with MidroSC Adapter
 
 Software: NetBurner tools revision 2.8.6
 Date: February 7, 2018
 
 Test results for file creation:
1 file: 15ms
100 files:32ms
1,000 files:93ms
 
 Test results for data transfer, using 1MB of data:
Write speed: 0.61 MBytes per second
Read speed: 2.18 MBytes per second
 
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