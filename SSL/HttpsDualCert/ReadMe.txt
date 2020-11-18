Doxygen Format
/** @pagegroup{SSL,HttpsDualCert,SSL HTTPS Dual Cert}

SSL Server Example for multiple certificates
This example creates a web page that can be viewed as normal or
encrypted with SSL. An example certificate is included in
file key.cpp. Since this is an example cert, the web browser
will display warning messages to this effect. For this example
select the "continue to page" options to allow the HTTPS for a single
session.

In addition this example illustrates how to have both a permanent
compiled-in certificate and key, as well as one that can be loaded
from an external Flash card using the EFFS FAT file system. If
a certificate and key are present on the Flash card they will be used
instead of the local, compiled-in certificate and key.

If you are using NBEclipse, then you will also need to tell the linker to
include the FatFile.a library. To do this, complete the following steps:

 - In NBEclipse, right-click on your project, and select "Properties"
 - Select "C/C++ Builds -> Settings" on the left-hand side
 - Select "GNU C/C++ Linker -> Libraries" under the "Tool Settings" tab
 - In the "Libraries" list box, add "FatFile" by using the action icons
   provided in top-right corner of the list box

Example Features:
- Compiled-in certificate and key
- Optional Flash card certificate and key
- NTP client to set the time
- FTP to enable the upload of a certificate and key to a Flash card
- Flash card test code to verify the card can be read/written to
- Web page and links to load a HTTPS or HTTP version

*/