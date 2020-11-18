Doxygen Format
/** @pagegroup{pageExamples,syslog,syslog}

The Syslog utility enables you to send logging information
to a destination host computer using UDP (port number 514). This
example uses syslog to send a simple counting variable to a host
computer.

To run the example:
1. Verify you have proper network communication with you NetBurner
   device. It must have an IP address and mask.
2. Modify the source code line: `SysLogAddress = AsciiToIp("10.1.1.191");`
   so that it specifies the address of your computer. If this line
   is commented out and no SysLogAddress is specified, then the syslog
   data will be sent as a UDP broadcast.
3. Run the NetBurner "UDP Terminal Tool" application. Make sure the
   local listening port field is set to 514, the default syslog port.
4. Download and run the application on your NetBurner device.

*/