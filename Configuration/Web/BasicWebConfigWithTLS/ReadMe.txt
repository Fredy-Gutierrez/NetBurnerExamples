Doxygen Format
/** @pagegroup{WebConfig, WebConfigBasicWithTLS, Basic Web Config Demo with TLS}

This example is identical to the BasicWebConfig example, with the additon of TLS. 
The makeca.bat, makeserver.bat files were run in `\nburn\CreateCerts` to generate a self-signed
certificate authority, which was then used to generate a server certificate and key. The files
are names ServerCert.cpp and ServerKey.cpp. You will want to run those same batch files to 
create your own CA and device server certificate with a common name that matches the IP address
of your specific NetBurner device. Once you have those .cpp files, copy them into your project. 

*/