Doxygen Format
/** @pagegroup{SSL,clientcert,SSL Client Certificate}

PROGRAM DESCRIPTION:
 SSL Client Certificate Example
 This example demonstrates:
 1. How SSL Client connections can be made.
 2. How to load and use Client Certificates.
 3. This example will only work if you are connecting to a SSL server that
is configured to request and receive client certificates. The NetBurner
SSL server is not configured to request client certificates. In this
example the client certificate is compiled as part of the application.
 
 The application will attempt to connect to the specified SSL Server and keep
 track of the number successful and failed attempts. Status messages will be
 sent to both the SSL Server, and to the serial debug serial port on the
 NetBurner device.
 
 
 IMPORTANT:
 A Client Certificate is a very different thing than when a client checks a
 server certificate against a list of Certificate Authorities (CA):
 
 Certificate Checking against a CA:
 In this mode the Client will check the CA portion of the server certificate
 against a list of CA's maintained by the client. If the CA's match, the connection
 is allowed. This mode requires a modification to predef.h to enable client
 certificate checking. This is NOTthe purpose of this example application.
 
 Sending a Client Certificate to the Server:
 In this mode, in addition to the server sending a certificate to the client
 as in the above case, the client is required to send a certificate to the
 server. That is the purpose of this example application, and in order to
 successfully compile the application you must create a certificate. There
 is a description of one way to make a self signed certificate in the security
 documentation in the NetBurner Development Kit Security Libraries document.
 
 CREATING A CERTIFICATE AND CA:
 Please refer to the NetBurner Security Library document for a detailed
 description. For the convience of this example, we have pregenerated one
 for you.
*/