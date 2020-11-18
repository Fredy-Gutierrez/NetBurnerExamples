Doxygen Format
/** @pagegroup{platformExampleMODM7AE70, i2cAddressScan, I2C Address Scan}

I2CAddressScan example demonstrates how to scan the I2C bus for devices and display their address.

ScanI2CBus sends out a call on the bus and waits for an ack, scanning each address
in the address space.  If a device responds, it reports that it's found a device
at xx address. This is really useful for figuring out what address you should
give to the sendbuf and readbuf commands to talk to your i2c devices.

This should be helpful in two ways:
1) It tells you that you have the hardware hooked up right.
2) It avoids confusing datasheets. (Anyone familiar with i2c datasheets will know
their address are written 3 different ways and 2 of them are wrong depending on
who you ask.)

*/
