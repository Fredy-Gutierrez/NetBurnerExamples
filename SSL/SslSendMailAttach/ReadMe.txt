Doxygen Format
/** @pagegroup{SSL,SendMailAttach,Send Mail w/ Attachment}

Send an email with a plain text attachment

Note that this application uses a secure SSL connection when
accessing the web server. All connections made through it will
need to take place via HTTPS.

This example illustrates how to send a MIME email attachment
in plain text, without using the EFFS FAT32 file system
(there is a different example for EFFS). Once the example is
running, all interaction is done through the web page.

When trying to connect to a Google Account, you will likely need
to enable less secure application access.  If you use two-factor
authentication with your account, you will need to setup an application
password for the account being connected to. This can be done at
the following URL: https://myaccount.google.com/apppasswords

The default STMP port value will be set to 465 for a connection using
SSL/TLS. This example is not structured to handle a STARTTLS response.
Please see the SslSendMail example for more information.

*/