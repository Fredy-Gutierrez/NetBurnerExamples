Doxygen Format
/** @pagegroup{SSL,SslServer,SSL/TLS Server}

This program will create a SSL/TLS server task that uses Eliptic Curve Cryptographic (ECC) 
keys. To test the application you can use another NetBurner device configured as a client, or openssl.
The command line options for openssl as of 13-Nov-2018 are:

 `openssl s_client -cipher ECDHE-ECDSA-AES256-GCM-SHA384 -connect <ip address>:<port>`

 For example: `openssl s_client -cipher ECDHE-ECDSA-AES256-GCM-SHA384 -connect 10.1.1.191:8883`

If you need an RSA key for your specific application needs, you can generate them using the scripts found
in `\nburn\CreateCerts\RSA`. Just replace ServerKey.cpp and ServerCert.cpp in this example with those that
you generate from the scripts and rebuild the application. You can test this with the following OpenSSL
commands.

 @code openssl s_client -connect <ip address>:<port> @endcode
 
 For example: @code openssl s_client -connect 10.1.1.191:8883 @endcode
 
This example uses a simple `read()` function to receive data from a TCP client.

*/