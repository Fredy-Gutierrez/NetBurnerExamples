Doxygen Format
/** @pagegroup{platformExampleMODM7AE70, serialEmptyCallbackMODM7AE70, Serial empty callback example }

 The Serial Empty Callback example demonstrates a robust process for
 triggering a specified action upon complete transmission of all data written
 to a specified serial port. The expected use case for this would be disabling
 a transmitter connected to a shared physical layer such as with RS485 or a 
 wireless link (when using one of the device's UART serial ports).

 As an aside, it should be noted that when enabling the Serial485HalfDupMode
 for one of the MODM7AE70s _USART_ serial ports, the underlying hardware
 module will automatically toggle the RTS line for enabling and disabling the
 transmitter.
*/
