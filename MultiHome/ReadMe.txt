Doxygen Format
/** @pagegroup{pageExamples,pageExamples-MultiHome,Multihome}

Multihome application example (IPv4 only)

This program will demonstrate how to implement both a DHCP address
and static IP address using the Multihome functionality of the
NetBurner TCP/IP Stack. The NetBurner device will try to obtain
a dynamic IP address from a DHCP Server for the first Network
Interface, and set a static IP address for the second Network Interface.
The end result is that the NetBurner device will respond to either
IP address. The example will print debug information out the debug
serial port, and display the IP address information on a web page
that can be accessed from either IP address.

To enable multihome capability, you must uncomment the MULTIHOME
definition in the include file `\nburn\nbrtos\include\predef.h`, and rebuild
the system files. If using the IDE, select Build->Rebuild All, If
using the command line, go to `\nburn\system` and run "make clean".

*/