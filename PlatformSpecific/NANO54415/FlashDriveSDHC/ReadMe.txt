Doxygen Format
/** @pagegroup{platformNANO54415, sdhcFlashDriveNANO54415, Onboard SDHC Flash Drive}

Application to demonstrate the NANO54415 SDHC Flash drive.

There are two main types of Flash drive interfaces: native SPI and SDCH. The major difference
is that native SPI uses a single signal for data, and SDHC uses 4 signals for data at the same time. 
The NANO54415 deveopment board with the standard SD Flash connector uses native SPI. If you are using
the development board SD Flash interface, please refer to the `\nburn\examples\EFFS\Fat` examples. 

The optional microSD that can be mounted on the back of the NANO54415 PCB uses the SDHC interface. 

This example demonstrates how to:
- Mount the flash drive
- Determine the amount of used and free drive space
- Use FTP to upload and download files using a FTP client, such as WinSCP
 
The program uses the serial debug port to display status information, which can be viewed with a 
serial terminal program such as MTTTY. The baud rate is 115,200 by default. 
 
 
NBEclipse users will need to do two additional tasks: 

1. Modify the linker settings to add the FatFile.a library:
- In NBEclipse, right-click on your project, and select "Properties"
- Select "C/C++ Builds -> Settings" on the left-hand side
- Select "GNU C/C++ Linker -> Libraries" under the "Tool Settings" tab
- In the "Libraries" list box, add "FatFile" by using the action icons provided in top-right corner of the list
 
2. Copy the EFFS FAT common files into your project src folder. This can be done with Windows File Explorer, or 
by using the NBEclipse Import feature described below:
 - In NBEclipse, right-click on your project, and select "Import"
 - From the "Import" window that pops up, select General -> File System
 - Click the "Browse" button, navigate to `/nburn/examples/_common/EFFS/FAT`
 - Select the following files: cardtype.h, effs_time.h, effs_time.cpp, FileSystemUtils.h, FileSystemUtils.cpp, ftp_f.h, ftp_f.cpp
 
Command line users do not need to import or copy the _common files. It is handled by the makefile.  
 
 
*/

