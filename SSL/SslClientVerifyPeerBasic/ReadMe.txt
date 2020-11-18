Doxygen Format
/** @pagegroup{SSL,SslVerifyPeerBasic,SSL Client Verify Peer Basic}

This program will demonstrate how to use CA Lists for use in support of
peer verification.  Two certificates are defined in caList.h.  These are
the root certificates for DuckDuckGo and NetBurner. They are stored in a
SharkSslCertStore object, and then associated with a `SharkSslCAList` object in
`CreateCAList()`, and then passed to the call to SSL_connect.  This list can
only be associated one time.

To test this example, use the command 'C' at the debug prompt, and then type
the name of the site (either www.duckduckgo.com or www.netburner.com) that
you wish to test a connection to.
*/