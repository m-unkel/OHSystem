/**
* Copyright [2013-2014] [OHsystem]
*
* We spent a lot of time writing this code, so show some respect:
* - Do not remove this copyright notice anywhere (bot, website etc.)
* - We do not provide support to those who removed copyright notice
*
* OHSystem is free software: You can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* You can contact the developers on: admin@ohsystem.net
* or join us directly here: http://forum.ohsystem.net/
*
* Visit us also on http://ohsystem.net/ and keep track always of the latest
* features and changes.
*
*
* This is modified from GHOST++: http://ohbotplusplus.googlecode.com/
*/

#include "../ohbot.h"
#include "../utils/util.h"
#include "gcbiprotocol.h"

//
// CGCBIProtocol
//

CGCBIProtocol :: CGCBIProtocol( )
{

}

CGCBIProtocol :: ~CGCBIProtocol( )
{

}

///////////////////////
// RECEIVE FUNCTIONS //
///////////////////////

CIncomingGarenaUser *CGCBIProtocol :: RECEIVE_GCBI_INIT( BYTEARRAY data )
{
    // 2 bytes					-> Header
    // 2 bytes					-> Length
    // 4 bytes					-> actual IP address (big endian)
    // 4 bytes					-> Garena user ID (big endian)
    // 4 bytes					-> Garena room ID (big endian)
    // 4 bytes					-> Garena user experience (big endian)
    // 2 bytes					-> country string from Garena

    if( ValidateLength( data ) && data.size( ) == 22 )
    {
        BYTEARRAY IP = BYTEARRAY( data.begin( ) + 4, data.begin( ) + 8 );
        BYTEARRAY UserID = BYTEARRAY( data.begin( ) + 8, data.begin( ) + 12 );
        BYTEARRAY RoomID = BYTEARRAY( data.begin( ) + 12, data.begin( ) + 16 );
        BYTEARRAY UserExp = BYTEARRAY( data.begin( ) + 16, data.begin( ) + 20 );
        BYTEARRAY Country = BYTEARRAY( data.begin( ) + 20, data.begin( ) + 22 );

        return new CIncomingGarenaUser(UTIL_ByteArrayToUInt32( IP, true ),
                                       UTIL_ByteArrayToUInt32( UserID, true ),
                                       UTIL_ByteArrayToUInt32( RoomID, true ),
                                       UTIL_ByteArrayToUInt32( UserExp, true ),
                                       string( Country.begin( ), Country.end( ) ) );
    }

    return NULL;
}

////////////////////
// SEND FUNCTIONS //
////////////////////

/////////////////////
// OTHER FUNCTIONS //
/////////////////////

bool CGCBIProtocol :: AssignLength( BYTEARRAY &content )
{
    // insert the actual length of the content array into bytes 3 and 4 (indices 2 and 3)

    BYTEARRAY LengthBytes;

    if( content.size( ) >= 4 && content.size( ) <= 65535 )
    {
        LengthBytes = UTIL_CreateByteArray( (uint16_t)content.size( ), false );
        content[2] = LengthBytes[0];
        content[3] = LengthBytes[1];
        return true;
    }

    return false;
}

bool CGCBIProtocol :: ValidateLength( BYTEARRAY &content )
{
    // verify that bytes 3 and 4 (indices 2 and 3) of the content array describe the length

    uint16_t Length;
    BYTEARRAY LengthBytes;

    if( content.size( ) >= 4 && content.size( ) <= 65535 )
    {
        LengthBytes.push_back( content[2] );
        LengthBytes.push_back( content[3] );
        Length = UTIL_ByteArrayToUInt16( LengthBytes, false );

        if( Length == content.size( ) )
            return true;
    }

    return false;
}

//
// CIncomingGarenaUser
//

CIncomingGarenaUser :: CIncomingGarenaUser( uint32_t nIP, uint32_t nUserID, uint32_t nRoomID, uint32_t nUserExp, string nCountryCode ) : m_IP( nIP ), m_UserID( nUserID ), m_RoomID( nRoomID ), m_UserExp( nUserExp ), m_CountryCode( nCountryCode )
{

}

CIncomingGarenaUser :: ~CIncomingGarenaUser( )
{

}
