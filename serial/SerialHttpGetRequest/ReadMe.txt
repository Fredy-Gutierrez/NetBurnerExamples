Doxygen Format
/** @pagegroup{pageExamples-serial,pageExamples-serial-SerialHttpGetRequest,Serial HTTP Get Request Example}

Example of sending a HTTP GET request through a NetBurner serial to Ethernet
 device such as a SB70LC, SB700EX or SBL2e. Testing was done with these devices
 running the standard factory applications.
 
 Hardware Configuration:
 
 Serial Device <--serial--> NetBurner Serial to Ethernet Device <--ethernet--> Internet
 
 In this example we will be using a SB700EX as the "Serial Device", but
 this would typically be a microprocessor or FPGA with a UART that needs to
 communicate over a network. The source code in this file is an example of
 what you would need to implement in your own microprocessor to send a GET
 request to the NetBurner device.
 
 The code in the main.cpp source file was tested with the SB700EX, but should
 work with any of the NetBurner standard devices. A serial cable was connected
 from port 1 of the SB700EX to port 1 of the SBL2e development board. Since the
 SBL2e development board DB9 connector is cross-wired for RX and TX, all you need
 is a straight through serial cable.
 
 SB700EX <--serial--> SBL2e <--ethernet--> Internet
 
 The web page contacted is myip.dnsdynamic.com, which return the IP address
 of the HTTP GET request.
 
 Software Configuration for SBL2e, serial port 1:
 
 Running the SBL2e factory serial to Ethernet application. This was done through the web
 page, but you can also do it through serial AT commands.
 
 1. Incoming TCP Settings: Uncheck Listen for Incoming Network Connections
 2. Outgoing TCP Settings: Make outgoing connections: If serial data received
 3. Outgoing TCP Settings: Connect on network port: 80
 4. Outgoing TCP Settings: Connect to this address: myip.dnsdynamic.com

*/