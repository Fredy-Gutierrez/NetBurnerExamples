Doxygen Format
/** @pagegroup{pageExamples,pageExamples-DeviceName,DNS Device Name}

The device name is stored in the device's flash congiguration.
There are two ways to give your device a name:
1. Use the configuration web interface, which is your device's
   `<device ip address>:20034`.
2. You can change the configuration flash setting in your application.
   You will then need to reboot the device for the name to take
   effect.

This example demonstrates how to change the device name in an application.

<b>Additional information on DNS naming</b>

 When a network device (such as the NetBurner module) requests an IP
 address from a DHCP Server, the device sends it MAC address and an
 optional device name (this is supported by your NetBurner device).
 The DHCP Server supplies the dynamic IP address and maintains a table
 containing the MAC address, IP address and device name for each device
 that has accepted a DHCP lease.

 The purpose of a DNS server is to convert a name into an IP address. To
 do this it must get the information from your DHCP Server. On windows
 server platforms that run both a DHCP server and DNS server, this can
 be configured in a dialog box. On liniux/unix it may be more complicated.
 Basically, every time the DHCP server gives out a new dynamic IP address
 it updates the DNS server. If you are not using DHCP, and are assigning
 static IP addresses, then you must update the DNS server with the entries
 manually (ie name and IP address).

 If you do not have a DNS server, you can modify the lmhosts file on your
 PC to add the IP address and name of each network device you want to address
 by name. You would need to do this on every PC that wants to use the name.

 Windows has a proprietary protocol called WINS (only microsoft products
 can use it). Windows machines can have a name, and the protocol enables
 windows machines to talk to one another. This is achieved by having one
 windows PC maintain the table of MAC address, IP address and names.

*/