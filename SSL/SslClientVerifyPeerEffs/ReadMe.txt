Doxygen Format
/** @pagegroup{SSL,SslVerifyPeerEffs,SSL Client Verify Peer EFFS-STD}

This program will demonstrate how to upload CA Lists to use in support of
verify peer. The uploaded information is stored in the onchip flash memory
using the standard file system. The serial port provides a debug menu.

If CA's are added to the list after a connection has already been established,
the device will need to be rebooted before they can be used.

HOW TO BUILD THIS APPLICATION:

This application requires a few items to be configured in NBEclipse or your
command line makefile.

1.  Enable SSL by editing `\nburn\include\predef.h`, and uncomment the following lines to enable SSL:

    @code #define NB_SSL_SUPPORTED   ( 1 ) @endcode

2.  Rebuild the system libraries.  In NBEclipse, select

       NBEclipse -> Rebuild All System Files

    from the main menu bar.

3.  This example uses the on-chip flash file system, which is different from
    the FAT file system for SD/MMC cards.  You will need to replace the
    "FatFile.a" library with the "StdFFile.a" library through the project
    properties:
       - In NBEclipse, right-click on your project, and select "Properties"
       - Select "C/C++ Builds -> Settings" on the left-hand side
       - Select "GNU C/C++ Linker -> Libraries" under the "Tool Settings" tab
       - In the "Libraries" list box, remove `\nburn\lib\FatFile.a` from
         the list and add `\nburn\lib\StdFFile.a` by using the action icons
         provided in top-right corner of the list box

4.  Modify the compcode memory address range to specify the amount of on-chip
    flash to be used for the file system:
       - In NBEclipse, right-click on your project, and select "Properties"
       - Select "C/C++ Builds -> Settings" on the left-hand side
       - Select "NetBurner Compcode -> General" under the "Tool Settings" tab
       - Modify the Memory Range based on the `COMPCODEFLAGS` information
         in the project's `nbfactory.h` header file.  The range used depends
         on the platform.  For example, the SB70LC would change from this:

            `0xFFC08000 0xFFC80000`

         to this:

            `0xFFC08000 0xFFC70000`

         which specifies 10000 hex (64k) to the file system, and the rest to
         to the application.

5.  Note that any changes made in NBEclipse project properties will only
    apply to that specific project.  The changes will need to be made again
    if you create a new project.
*/