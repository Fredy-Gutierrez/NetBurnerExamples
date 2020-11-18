Doxygen Format
/** @pagegroup{Wifi,Wifi-Ap,Wifi AP}

This example shows how to run the wifi module in Access Point mode.
It also launches a DHCP Server running on the wifi interface, making it easier
for a client to configure when connecting to the access point.

To setup your access point properly, you will need to configure the IP address
and Mask of the Wifi interface. This can be done using IPSetup. You will also need
to set the start address for the DHCP server to begin assigning IP
addresses to connected devices. The variable 'startAddr' is used to so in this
application.

For example, a proper AP configuration would be a WiFi interface with an IP address
of 192.168.0.1, Mask of 255.255.255.0, and a DHCP server start address of
192.168.0.2

*/