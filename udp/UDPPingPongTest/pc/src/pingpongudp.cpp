#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <errno.h>
#include <winsock.h>

#define PORT_TO_USE (10000)

#define NUM_CYCLES (1000)

char ResultsBuf[ NUM_CYCLES ][ 2048 ];
int Results[ NUM_CYCLES ];

typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;

bool CheckPacket( char * p, uint32_t seq, int len )
{
    uint32_t seq1 = htonl( *(uint32_t *)p );
    uint32_t seq2 = htonl( *(uint32_t *)(p + len - 4) );
    if( (seq1 != seq2) || (seq1 != seq) )
    {
        printf( "Sequence mismatch expected %d got %d and %d\n", seq, seq1, seq2 );
        return false;
    }

    return true;

}

int main( int argc, char ** argv )
{
    uint16_t wVersionRequested = MAKEuint16_t( 1, 1 );
    WSADATA wsaData;
    LPHOSTENT   lpHostEntry;

    // Initialize WinSock and check the version
    int nRet = WSAStartup( wVersionRequested, &wsaData );
    if( wsaData.wVersion != wVersionRequested )
    {
        fprintf( stderr, "\n Wrong winsocket version\n" );
        return -1;
    }

    /* Setup the socket */
    SOCKET sock = socket( AF_INET, SOCK_DGRAM, 0 );
    sockaddr_in saddr;
    memset( &saddr, 0, sizeof( saddr ) );
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons( PORT_TO_USE );
    saddr.sin_addr.s_addr = INADDR_ANY;

    const char * ipAddr = (char *)argv[ 1 ];

    printf( "Getting IP For %s\r\n", ipAddr );
    lpHostEntry = gethostbyname( ipAddr );
    if( lpHostEntry == NULL )
    {
        printf( "Failed to get host for %s\n", ipAddr );
        return -1;
    }

    /* Now send the packet */
    int result = bind( sock, (struct sockaddr *)&saddr, sizeof( saddr ) );

    sockaddr_in saddro;
    memset( &saddro, 0, sizeof( saddro ) );
    saddro.sin_family = AF_INET;
    saddro.sin_port = htons( PORT_TO_USE );
    saddro.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list); //  Server's address 

    uint32_t startTime = GetTickCount();           // Start time in milliseconds
    result = sendto( sock, "DATA", 4, 0, (struct sockaddr *)&saddro, sizeof( saddro ) );

    fd_set readfd;
    FD_ZERO( &readfd );
    FD_SET( sock, &readfd );
    timeval tout;

    tout.tv_sec = 2;
    tout.tv_usec = 0;
    unsigned char buffer[ 2048 ];
    int saddrlen = sizeof( saddr );
    int nTimes = 0;

    /* Now wait for each UDP packet to come back */
    while( (select( 1, &readfd, NULL, NULL, &tout ) > 0) && (nTimes < NUM_CYCLES) )
    {
        saddrlen = sizeof( saddr );
        Results[ nTimes ] = recvfrom( sock, ResultsBuf[ nTimes ], 2048, 0, (struct sockaddr *)&saddr, &saddrlen );
        //printf("Read %d\n",result);

        result = sendto( sock, "DATA", 4, 0, (struct sockaddr *)&saddro, sizeof( saddro ) );
        // printf("Sent=%d\n",result);
        FD_ZERO( &readfd );
        FD_SET( sock, &readfd );
        tout.tv_sec = 2;
        tout.tv_usec = 0;
        nTimes++;
        if( nTimes % 10 == 0 )printf( "." );
    }

    closesocket( sock );
    uint32_t stopTime = GetTickCount();
    float timeInSeconds = (float)(stopTime - startTime) / 1000;

    printf( "\nGot %d packets in %0.3f Sec\n", nTimes, timeInSeconds );
    printf( "Checking results..." );
    int maxv = Results[ 0 ];
    int minv = Results[ 0 ];

    for( int i = 0; i < nTimes; i++ )
    {
        if( Results[ i ] > maxv ) maxv = Results[ i ];
        if( Results[ i ] < minv ) minv = Results[ i ];
    }

    int good = 0;
    int bad = 0;

    if( maxv == minv )
    {
        printf( "All received packets were %d bytes\nChecking content..", maxv );
        uint32_t StartVal = htonl( *(uint32_t *)ResultsBuf[ 0 ] );
        for( int i = 0; i < nTimes; i++ )
        {
            if( CheckPacket( ResultsBuf[ i ], i + StartVal, Results[ i ] ) ) good++;
            else
                bad++;
        }
        printf( "Results Good:%d  Bad %d\n", good, bad );
    }
    else
    {
        printf( "ERROR: Max rx=%d min=%d should be the same\n", maxv, minv );
    }

    return 0;
}
