Doxygen Format
/** @pagegroup{SSL,HttpsUploadCert,SSL Https Upload Cert}

This program will demonstrate how to upload certificates and keys to support
SSL/TLS web page access. The uploaded information is stored in the onchip
flash memory using the standard file system. The serial port provides a debug
menu.

HOW TO BUILD THIS APPLICATION:

This application requires a few items to be configured in NBEclipse or your
command line makefile.

1.  This example uses the on-chip flash file system, which is different from
    the FAT file system for SD/MMC cards.  You will need to add the
    "libStdFFile.a" library through the project properties:
       - In NBEclipse, right-click on your project, and select "Properties"
       - Select "C/C++ Builds -> Settings" on the left-hand side
       - Select "GNU C/C++ Linker -> Libraries" under the "Tool Settings" tab
       - In the "Libraries" list box, add "StdFFile" by using the action icons
         provided in top-right corner of the list box
       - In the "Library search path (-L)", add the path to the library, which
         should be `${NNDK_ROOT}/platform/${PLATFORM}/original/lib`

2.  Modify the compcode memory address range to specify the amount of on-chip
    flash to be used for the file system:
       - In NBEclipse, right-click on your project, and select "Properties"
       - Select "C/C++ Builds -> Settings" on the left-hand side
       - Select "NetBurner Compcode -> General" under the "Tool Settings" tab
       - Modify the "Memory Range" based on the "COMPCODEFLAGS" information
         in the project's "nbfactory.h" header file.  The range used depends
         on the platform.  For example, the MODM7AE70 would change from this:

            0xFFC08000 0xFFC80000

         to this:

            0x00406004 0x005A0000
            
         For our latest version of NBEclipse, the default values are already provided
         for each platform, so no changes should be necessary, unless you want to
         change how much storage is available.

Note that any changes made in NBEclipse project properties will only
apply to that specific project.  The changes will need to be made again
if you create a new project.

*/