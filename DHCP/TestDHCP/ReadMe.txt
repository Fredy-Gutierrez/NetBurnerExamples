Doxygen Format
/** @pagegroup{DHCP,TestDHCP,TestDHCP}

Example of advanced DHCP functionality
Most applications can simply use the GetDhcpAddress() function call to
automatically handle obtaining a DHCP address and configuration. If you
need finer control, you can create a DHCP Object and manage the DHCP
functions from within your application. This example shows how you can
create a DHCP object and use a pointer to the object to monitor and
control DHCP services.

It also will show you the internal workings of DHCP by displaying UDP packet
information and DHCP variables on stdio, which is UART0 by default (view
with MTTTY). It is not the intent of the example to be a tutorial on
DHCP. For detailed DHCP information please refer to RFC 2131.

For the purposes of education/demonstration, this application will access
the C++ private class variables of a DHCP object. This is NOT required for
end applications, it is just used here for education.

To obtain access to private class variables in the DHCP object, you will
need to enable debugging in the "\nburn\system\dhcpc.cpp" file by removing
the comments from the following line:

       `#define DHCP_DEBUG 1`

*** COMMENT OUT THE #define WHEN YOU ARE DONE WITH THIS EXAMPLE. DO NOT
*** LEAVE IT IN FOR RELEASE APPLICATIONS!

You will then need to recompile the system library with by clicking on
Rebuild Modified System Files or Rebuild System Files in NBEclipse,
or a "make" in the "\nburn\system" directory if you are using command
line tools.

 This example will:
- Create a DHCP object global variable
- Start DHCP and monitor a semaphore for completion
- Provide an interactive menu to start and stop dhcp services,
  modify the system timer, and display dhcp status.

*/