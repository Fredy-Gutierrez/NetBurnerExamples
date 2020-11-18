Doxygen Format
/** @pagegroup{pageExamples, ppp, PPP}

This PPP example shows how to setup and use a modem for a PPP connection.
It demonstrates:
 - Dialing in to a Netburner using a PPP client. (Like Window dial up networking)
 - Dialing out to an ISP.
 - Sending E-Mail from a Netburner.

 There are two types of high level connections that can be made with PPP
 is either direct or modem.  A direct connection is ussually performed with only a
 serial cable connection and will jump right into the PPP negotiations with the host
 or client.  A modem connection is one where there is a modem between the PC and
 NetBurner.  The modem PPP functions will send AT commands before any PPP negotiation
 to initialize the modem.

 Note:  When setting up a NetBurner to be a Direct connect server with a
 Windows XP machine a special loop back adapter must be used on the serial
 cable going to the PC.  This cable can be made manually by tying pin 1 to
 pin 4 on the DB9 serial cable to properly loop back the carrier detect signal.
 This cable is not required if the Windows XP machine is configured to be the
 direct connect server.  Software flow control should also be disabled on both
 the NetBurner and XP configurations.  These configurations might also apply to
 any other devices, or OSs, that make a PPP direct connection to a NetBurner.

*/