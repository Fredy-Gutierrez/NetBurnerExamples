Doxygen Format
/** @pagegroup{EFFS-FAT,pageExamples-EFFS-Fat-AppUpdate,FAT AppUpdate}

This application demonstrates how to update a NetBurner application in onboard
 flash memory from an external SD flash card. The application will look for
 a file on the flash card with the name specified by APPFILENAME. Use MTTTY
 to monitor the serial output of your NetBurner device. If this file
 is found, an application update can be performed by typing a character in
 response to the MTTTY prompt.
 
 If you are using NBEclipse, then you will also need to tell the linker to
 include the "\nburn\platform\<platform>\original\lib\libFatFile.a" library.
 
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
 - Click the "Browse" button, navigate to "\nburn\examples\_common\EFFS\FAT"
 - Select all of the files listed except for the makefile, and click
 "Finish"

*/