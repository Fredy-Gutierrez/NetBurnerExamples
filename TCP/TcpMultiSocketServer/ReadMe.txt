Doxygen Format
/** @pagegroup{pageExamples-TCP,pageExamples-TCP-TcpMultiSocketServer,MultiSocket Server Example}

This example creates a TCP server that listens on the specified TCP port
number and can handle multiple TCP connections simultaneously (10 in this
example). The select() function is a great way method to pend and process
multiple connections.

An easy way to test the example is to use multiple Telnet sessions to create
simultaneous connections to the NetBurner device. Status messages are sent
out stdio to the debug serial port, and to the client TCP connections.

*/