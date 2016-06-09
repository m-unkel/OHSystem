//
// Created by ZeMi on 09.06.2016.
//

#include "../ohbot.h"
#include "utils.h"
#include "../utils/util.h"
#include "../network/socket.h"
#include <stdlib.h>

#ifdef WIN32
#include <ws2tcpip.h>		// for WSAIoctl
//#include <windows.h>
//#include <winsock.h>
#endif

void GetLocalAddresses(vector<BYTEARRAY> *vec){

#ifdef WIN32
    // use a more reliable Windows specific method since the portable method doesn't always work properly on Windows
    // code stolen from: http://tangentsoft.net/wskfaq/examples/getifaces.html

    SOCKET sd = WSASocket(AF_INET, SOCK_DGRAM, 0, 0, 0, 0);

    if (sd == SOCKET_ERROR)
        CONSOLE_Print("[GHOST] error finding local IP addresses - failed to create socket (error code " + UTIL_ToString(WSAGetLastError()) + ")");
    else {
        INTERFACE_INFO InterfaceList[20];
        unsigned long nBytesReturned;

        if (WSAIoctl(sd, SIO_GET_INTERFACE_LIST, 0, 0, &InterfaceList, sizeof(InterfaceList), &nBytesReturned, 0, 0) == SOCKET_ERROR)
            CONSOLE_Print("[GHOST] error finding local IP addresses - WSAIoctl failed (error code " + UTIL_ToString(WSAGetLastError()) + ")");
        else {
            int nNumInterfaces = nBytesReturned / sizeof(INTERFACE_INFO);

            for (int i = 0; i < nNumInterfaces; ++i) {
                sockaddr_in *pAddress;
                pAddress = (sockaddr_in * ) & (InterfaceList[i].iiAddress);
                CONSOLE_Print("[GHOST] local IP address #" + UTIL_ToString(i + 1) + " is [" + string(inet_ntoa(pAddress->sin_addr)) + "]");
                vec->push_back(UTIL_CreateByteArray((uint32_t) pAddress->sin_addr.s_addr, false));
            }
        }

        closesocket(sd);
    }
#else
    // use a portable method

    char HostName[255];

    if( gethostname( HostName, 255 ) == SOCKET_ERROR )
        CONSOLE_Print( "[GHOST] error finding local IP addresses - failed to get local hostname" );
    else
    {
        CONSOLE_Print( "[GHOST] local hostname is [" + string( HostName ) + "]" );
        struct hostent *HostEnt = gethostbyname( HostName );

        if( !HostEnt )
            CONSOLE_Print( "[GHOST] error finding local IP addresses - gethostbyname failed" );
        else
        {
            for( int i = 0; HostEnt->h_addr_list[i] != NULL; ++i )
            {
                struct in_addr Address;
                memcpy( &Address, HostEnt->h_addr_list[i], sizeof(struct in_addr) );
                CONSOLE_Print( "[GHOST] local IP address #" + UTIL_ToString( i + 1 ) + " is [" + string( inet_ntoa( Address ) ) + "]" );
                vec->push_back( UTIL_CreateByteArray( (uint32_t)Address.s_addr, false ) );
            }
        }
    }
#endif
}