Doxygen Format
/** @pagegroup{pageExamples-udp,pageExamples-udp-UDPSendReceive,NetBurner simple UDP send/receive example.}

This example program will receive a UDP packet from another device or
 host computer, and then send a response. To run the example, connect a serial
 port from your PC to the debug serial port on your NetBurner device and run a
 terminal program such as MTTTY. On the PC, run the NetBurner UDP Terminal
 (be sure to set the IP address and port numbers to match). You will then
 be able to type characters in the UDP Terminal and see them in MTTTY,
 and vice versa.
 
 You will be prompted for the port number to send/receive data and the
 destination IP address of the other device or host. Note that the application
 uses the same port number to send and receive data, but you can use any other
 port number you wish.
 
 The application will create a thread to receive packets and display them
 on the debug port, while the main task will take any data you type in
 to the MTTTY terminal and send it as a UDP packet to the destination IP
 address.

*/