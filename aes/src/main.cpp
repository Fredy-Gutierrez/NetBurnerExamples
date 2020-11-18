/* Revision: 3.2.0 */

/******************************************************************************
* Copyright 1998-2020 NetBurner, Inc.  ALL RIGHTS RESERVED
*
*    Permission is hereby granted to purchasers of NetBurner Hardware to use or
*    modify this computer program for any use as long as the resultant program
*    is only executed on NetBurner provided hardware.
*
*    No other rights to use this program or its derivatives in part or in
*    whole are granted.
*
*    It may be possible to license this or other NetBurner software for use on
*    non-NetBurner Hardware. Contact sales@Netburner.com for more information.
*
*    NetBurner makes no representation or warranties with respect to the
*    performance of this computer program, and specifically disclaims any
*    responsibility for any damages, special or consequential, connected with
*    the use of this program.
*
* NetBurner
* 5405 Morehouse Dr.
* San Diego, CA 92121
* www.netburner.com
******************************************************************************/

#include <aes.h>
#include <init.h>
#include <string.h>
#include <startnet.h>

const char *AppName = "AES test Example";

/*
 * AES-ECB test vectors (source: NIST, rijndael-vals.zip)
 */
static const uint8_t aes_enc_test[ 3 ][ 16 ] =
{
    { 0xC3, 0x4C, 0x05, 0x2C, 0xC0, 0xDA, 0x8D, 0x73,
      0x45, 0x1A, 0xFE, 0x5F, 0x03, 0xBE, 0x29, 0x7F },
    { 0xF3, 0xF6, 0x75, 0x2A, 0xE8, 0xD7, 0x83, 0x11,
      0x38, 0xF0, 0x41, 0x56, 0x06, 0x31, 0xB1, 0x14 },
    { 0x8B, 0x79, 0xEE, 0xCC, 0x93, 0xA0, 0xEE, 0x5D,
      0xFF, 0x30, 0xB4, 0xEA, 0x21, 0x63, 0x6D, 0xA4 }
};


static const uint8_t aes_dec_test[ 3 ][ 16 ] =
{
    { 0x44, 0x41, 0x6A, 0xC2, 0xD1, 0xF5, 0x3C, 0x58,
      0x33, 0x03, 0x91, 0x7E, 0x6B, 0xE9, 0xEB, 0xE0 },
    { 0x48, 0xE3, 0x1E, 0x9E, 0x25, 0x67, 0x18, 0xF2,
      0x92, 0x29, 0x31, 0x9C, 0x19, 0xF1, 0x5B, 0xA4 },
    { 0x05, 0x8C, 0xCF, 0xFD, 0xBB, 0xCB, 0x38, 0x2D,
      0x1F, 0x6F, 0x56, 0x58, 0x5D, 0x8A, 0x4A, 0xDE }
};


/*
 *  aes_self_test()
 *
 *  This function will encode and decode blocks of data 10,000 times
 *  and check the result against a known good value. This function
 *  is not at all necessary for AES, it is simply an implementation
 *  test.
 */
int aes_self_test( int verbose )
{
    int i, j, u, v = 0;
    aes_context ctx;
    unsigned char buf[ 32 ];

    for( i = 0; i < 6; i++ )
    {
        u = i >> 1;
        v = i & 1;

        if( verbose != 0 )
        {
            printf( "  AES-ECB-%3d (%s): ", 
                    128 + u * 64,
                    ( v == 0 ) ? "enc" : "dec" );
        }

        memset( buf, 0, 32 );
        aes_set_key( &ctx, buf, 128 + u * 64 );

        for( j = 0; j < 10000; j++ )
        {
            if( v == 0 ){ aes_encrypt( &ctx, buf, buf ); }
            if( v == 1 ){ aes_decrypt( &ctx, buf, buf ); }
        }

        if( (v == 0 && memcmp( buf, aes_enc_test[ u ], 16 ) != 0) ||
            (v == 1 && memcmp( buf, aes_dec_test[ u ], 16 ) != 0) )
        {
            if( verbose != 0 )
            {
                iprintf( "failed\n" );
            }

            return(1);
        }

        if( verbose != 0 )
        {
            iprintf( "passed\n" );
        }
    }

    if( verbose != 0 )
    {
        iprintf( "\n" );
    }

    return(0);
}

/*
 *  UserMain()
 *
 *  This is the main entry point for the example
 */
void UserMain( void *pd )
{
    init();

    iprintf( "AES test program\r\n" );

    // Run self test
    aes_self_test( 1 );

    // 16 bytes of plain text data to encrypt
    const  char * plain_text = "The Plain Text..";
    // 16 byte encryption key
    const  char * the_key = "The Decode Key..";
    unsigned char encodedresult[ 16 ];
    unsigned char decodedresult[ 16 ];

    aes_context ctx;

    if( strlen( plain_text ) != 16 )
    {
        iprintf( "Encoding/decoding must occur in 16 byte blocks.\r\n" );
    }
    if( strlen( the_key ) != 16 )
    {
        iprintf( "Please use a 16 byte (128 bit)key.\r\n" );
    }

    // Create the key
    aes_set_key( &ctx, (unsigned char *)the_key, 16 * 8 );

    // Encrypt the 16 byte block of data
    aes_encrypt( &ctx, (unsigned char *)plain_text, encodedresult );

    // Decrypt the 16 byte block of data
    aes_decrypt( &ctx, encodedresult, decodedresult );

    iprintf( "The encoded result\r\n" );
    ShowData( encodedresult, 16 );

    iprintf( "The decoded result\r\n" );
    ShowData( decodedresult, 16 );

    iprintf( "Test complete\r\n" );

    while( 1 )
    {
        OSTimeDly( 10 * TICKS_PER_SECOND );
    };
}
