Doxygen Format
/** @pagegroup{pageExamples-serial,pageExamples-serial-SerialBurner,Serial To Ethernet Example}

This program is designed to connect a RS-232 device to a network. It opens a
 serial port and an Ethernet port, then connect them.This allows any data
 sent to the Ethernet port to be forwarded to the serial port, and vice versa.
 
 USER NOTES
 This program was originally written for the NetBurner CFV2-40 platform, which
 does not support serial port hardware flow control.
 
 The TCP protocol does not have a "keep alive" message.If a client computer
 connects using TCP/IP and then dies without closing the connection, it will
 not be detected by the server.This problem is solved with a connection
 timeout.If no activity occurs across the link for a specified number of
 seconds, the connection is closed by the server.The timeout value is set
 with TIMEOUT_LIMIT.A value of 0 will disable the timeout feature.
 
 Only 1 connection is allowed at a time.Additional client connections will
 be queued, and serviced as the active connection completes.
 
 Override Timeout:
 When a new TCP connection is attempted while there is already an active
 connection, one of 3 actions can be taken:
 
 1. Don't ever disconnect the active connection.
    Set `OVERRIDE_TIMEOUT to 0xFFFFFFFF`.
 2. Only disconnect the existing connection if it has been idle
    for x seconds. Set `OVERRIDE_TIMEOUT` to the number of seconds.
 3. Always disconnect the existing connection.
    Set `OVERRIDE_TIMEOUT` to 0.

*/