Doxygen Format
/** @pagegroup{SSL,SslMailto,SSL Mail To}

 This program illustrates how to send an email message to a server that uses SLL.
 
 <b>All the magic happens in the `webfuncs.cpp` file, this code just initializes the app</b>
 
 Note that this application uses a secure SSL connection when accessing the web server. All
 connections made through it will need to take place via HTTPS.
 
 There are two ways the SSL SMTP handshake can happen, with or without "STARTTLS".
 With servers that don't require STARTTLS, all that needs to be done differently than
 a regular SMTP handshake is to connect using "SSL_connect" to port 465 (instead of a
 regular connect to port 25), and from there proceed like a normal SMTP handshake.
 This method is used when we know for sure the server accepts SSL mail and we definitely want to
 use SLL (since some server like Yahoo allow you to connect in both ways)
 
 If we are not sure if a server supports SSL, or, we only want to use SSL if it is required,
 we use the STARTTLS method. With this method, the send_mail function first opens a connection
 to the regular SMTP port 25 and does an "EHLO". if the server has the key word "STARTTLS" in its
 response, it means that it supports TLS, from there, the client says "STARTTLS" and then calls
 the SSL_negotiate function which establishes a secure connection with the server. After the secure
 connection is established, we say "EHLO" again and proceed as normal.
 
 Gmail requires SSL, so if we try to connect to it on port 25 (the non SLL port)
 it will say STARTTLS without giving the option AUTH, which means SSL is required and we
 must start the SSL negotiation. Yahoo allows both, but but does not offer STARTTLS (doesn't let you
 know that SSL is available if connected to on the regular SMTP port, it expects you to connect
 to port 465 if you want to use SSL, and port 25 for non SSL.
 
 When trying to connect to a Google Account, you will likely need to enable less secure
 application access.  If you use two-factor authentication with your account, you will need
 to setup an application password for the account being connected to. This can be done at
 the following URL: https://myaccount.google.com/apppasswords
 
 If you are using NBEclipse, then you will also need to tell the linker to include the
 `/nburn/platform/<platform>/original/lib/libStdFFile.a library`.  To do this, complete the following steps:
 
  - In NBEclipse, right-click on your project, and select "Properties"
  - Select "C/C++ Builds -> Settings" on the left-hand side
  - Select "GNU C/C++ Linker -> Libraries" under the "Tool Settings" tab
  - In the "Libraries" list box, add "StdFFile" by using the action icons
    provided in top-right corner of the list box

*/