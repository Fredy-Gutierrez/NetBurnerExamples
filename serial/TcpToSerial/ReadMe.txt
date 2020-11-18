Doxygen Format
/** @pagegroup{pageExamples-serial,pageExamples-serial-TcpToSerial,The TCP to Serial Example Program}

 This is a very simple TCP to Serial example. It listens on a TCP
 port for a single incoming connection. Once a TCP connection is
 accepted, all data recevied will be sent out the serial port,
 and all data recevied by the serial port will be sent out the TCP
 connection.
 
 A simple way to test this program is to use the Telnet program on
 a PC and connect to the NetBurner device. Telnet is available to
 run from a command prompt, or with a program such as PUTTY.
 
 If you would like to extend this example and create a robust Serial To
 Ethernet application, the next step would be to add error detection and
 timeouts to handle a situation such as the TCP client crashing or going
 away without a proper TCP close connection sequence. That would result
 in a half open socket condition and the NetBurner device would have no
 way of knowing what happened.
 
 To help combat situations such as these this application takes a simple approach
 of closing the connection is no activity has taken place for a period of time.

*/