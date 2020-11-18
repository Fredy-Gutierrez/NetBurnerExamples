Doxygen Format
/** @pagegroup{SSL,SslWebDemo,SSL Web Demo}

SSL Server example program with web page redirection for unauthorized access.
The example demonstrates how you can have both secure and unsecure access to
files and directories.
 
 
OPERATION
This example project will start the web server with SSL capability. The
directory structure is such that index.htm and the files in the images
directory can be viewed with http or https connections, but any files
in the httpsdir directory (ie repeat.htm) can only be viewed with https.
 
 html
   |-- index.htm
   |-- images
         |-- (various image files)
   |-- httpsdir
         |-- repeat.htm
 
 
The MyDoGet() function parses the URL to determine what files are being
accessed, and will allow or redirect the access depending on the type.

*/