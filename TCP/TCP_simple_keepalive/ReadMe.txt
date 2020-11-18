Doxygen Format
/** @pagegroup{pageExamples-TCP,pageExamples-TCP-TCP_simple_keepalive,Simple Keepalive Example}

This example program will create a TCP server that also implements keep
alive functionality with the TCP client.

Keep-alive is implemented with the TcpGetLastRxTime() and TcpSendKeepAlive()
functions. TcpGetLastRxTime() returns the number of time ticks since the
last time a packet was received. TcpSendKeepAlive() sends a keep-alive
packet to the client, which is used when no data is has been transmitted
within the timeout period.

The general concept is to call TcpGetLastRxTime(), call TcpSendKeepAlive(),
wait a bit, then call TcpSendKeepAlive() a second time and verify the number
of time ticks is different. If not, no packets were received and the client
is not responding. Make sure to allow time for the client to respond to the
keep alive packet. Do not call TcpGetLastRxTime more often than once every
second to avoid performance issues.

This example creates a TCP server task that listens on port 23 by default.
The server blocks with "ReadWithTimeOut" until data is received or the timer
times out. If a timeout occurs the time of the last received packet is
recorded and a keep-alive packet is sent. Then, the server goes back to
waiting for a read again. If ReadWithTimeOut times out a second time, the
value of the last received TCP packet is checked again. If the client is
still active the value of lastRxTime will be different than the previous
value (due to the keep-alive packet that was sent). If the client did not
respond to the keep-alive packet the number will remain the same and the
client is assumed to be non-responsive, and the connetion is closed.

To test the application you can use a TCP client such as telnet or putty.
For example, from a windows command prompt, type @code telnet <ip address of NetBurner> @endcode.
Status messages are sent to the debug/console serial port.

*/