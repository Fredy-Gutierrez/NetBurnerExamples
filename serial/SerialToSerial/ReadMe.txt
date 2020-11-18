Doxygen Format
/** @pagegroup{pageExamples-serial,pageExamples-serial-SerialToSerial,Serial to serial example}

This simple program opens both serial ports and sends data from one
 to another using the select() function, which can pend on multiple
 file descriptors at one time.
 
 The serial ports are initialized by the monitor in polled mode. To
 enable buffered interrupt driven mode, the serial ports are closed
 and then reopened with OpenSerial().

*/