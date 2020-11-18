Doxygen Format
/** @pagegroup{SSL,sslclient,SSL/TLS Client}

SSL Client Example
 This example demonstrates how SSL Client connections can be made.
 It will attempt to connect to the specified SSL Server and keep
 track of the number successful and failed attempts. Status messages
 will be sent to both the SSL Server, and to the serial debug
 port on the NetBurner device.
 
 Certificate checking is disabled by default. To enable certificate checking you must:
1. Include a CA certificate list in your project
2. Uncomment @code #define NB_SSL_CLIENT_CERTIFICATE_CHECKING_ENABLED @endcode in sslclient.cpp
3. Rebuild the system libraries

*/