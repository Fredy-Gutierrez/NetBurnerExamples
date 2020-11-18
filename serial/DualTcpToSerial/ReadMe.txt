Doxygen Format
/** @pagegroup{pageExamples-serial,pageExamples-serial-DualTcpToSerial,Dual TCP To Serial Example}

This sample application connects two serial ports to two different TCP
 ports, and continuously transfers data between them. In other words, you
 have a dual serial-to-Ethernet device. An easy way to test this operation is
 to open two telnet sessions and two MTTTY sessions as shown below. To start
 a telnet session on a windows pc, open a command prompt and type
 `telnet 10.1.1.2 2000`, where you would replace 10.1.1.2 with the IP address
 of your NetBurner device.
 
 Telnet on port 2000 >-----------< MTTTY on serial port 0
 Telnet on port 2001 >-----------< MTTTY on serial port 1
 
 Now anything you type in the first telnet session will be sent to serial
 port 0, and vice versa. Anything you type in the second telnet session will
 be sent to serial port 1 and vice versa.

*/