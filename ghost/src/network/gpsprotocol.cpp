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
#include "gpsprotocol.h"

//
// CGPSProtocol
//

CGPSProtocol :: CGPSProtocol( )
{

}

CGPSProtocol :: ~CGPSProtocol( )
{

}

///////////////////////
// RECEIVE FUNCTIONS //
///////////////////////

////////////////////
// SEND FUNCTIONS //
////////////////////

BYTEARRAY CGPSProtocol :: SEND_GPSC_INIT( uint32_t version )
{
    BYTEARRAY packet;
    packet.push_back( GPS_HEADER_CONSTANT );
    packet.push_back( GPS_INIT );
    packet.push_back( 0 );
    packet.push_back( 0 );
    UTIL_AppendByteArray( packet, version, false );
    AssignLength( packet );
    return packet;
}

BYTEARRAY CGPSProtocol :: SEND_GPSC_RECONNECT( unsigned char PID, uint32_t reconnectKey, uint32_t lastPacket )
{
    BYTEARRAY packet;
    packet.push_back( GPS_HEADER_CONSTANT );
    packet.push_back( GPS_RECONNECT );
    packet.push_back( 0 );
    packet.push_back( 0 );
    packet.push_back( PID );
    UTIL_AppendByteArray( packet, reconnectKey, false );
    UTIL_AppendByteArray( packet, lastPacket, false );
    AssignLength( packet );
    return packet;
}

BYTEARRAY CGPSProtocol :: SEND_GPSC_ACK( uint32_t lastPacket )
{
    BYTEARRAY packet;
    packet.push_back( GPS_HEADER_CONSTANT );
    packet.push_back( GPS_ACK );
    packet.push_back( 0 );
    packet.push_back( 0 );
    UTIL_AppendByteArray( packet, lastPacket, false );
    AssignLength( packet );
    return packet;
}

BYTEARRAY CGPSProtocol :: SEND_GPSS_INIT( uint16_t reconnectPort, unsigned char PID, uint32_t reconnectKey, unsigned char numEmptyActions )
{
    BYTEARRAY packet;
    packet.push_back( GPS_HEADER_CONSTANT );
    packet.push_back( GPS_INIT );
    packet.push_back( 0 );
    packet.push_back( 0 );
    UTIL_AppendByteArray( packet, reconnectPort, false );
    packet.push_back( PID );
    UTIL_AppendByteArray( packet, reconnectKey, false );
    packet.push_back( numEmptyActions );
    AssignLength( packet );
    return packet;
}

BYTEARRAY CGPSProtocol :: SEND_GPSS_RECONNECT( uint32_t lastPacket )
{
    BYTEARRAY packet;
    packet.push_back( GPS_HEADER_CONSTANT );
    packet.push_back( GPS_RECONNECT );
    packet.push_back( 0 );
    packet.push_back( 0 );
    UTIL_AppendByteArray( packet, lastPacket, false );
    AssignLength( packet );
    return packet;
}

BYTEARRAY CGPSProtocol :: SEND_GPSS_ACK( uint32_t lastPacket )
{
    BYTEARRAY packet;
    packet.push_back( GPS_HEADER_CONSTANT );
    packet.push_back( GPS_ACK );
    packet.push_back( 0 );
    packet.push_back( 0 );
    UTIL_AppendByteArray( packet, lastPacket, false );
    AssignLength( packet );
    return packet;
}

BYTEARRAY CGPSProtocol :: SEND_GPSS_REJECT( uint32_t reason )
{
    BYTEARRAY packet;
    packet.push_back( GPS_HEADER_CONSTANT );
    packet.push_back( GPS_REJECT );
    packet.push_back( 0 );
    packet.push_back( 0 );
    UTIL_AppendByteArray( packet, reason, false );
    AssignLength( packet );
    return packet;
}

/////////////////////
// OTHER FUNCTIONS //
/////////////////////

bool CGPSProtocol :: AssignLength( BYTEARRAY &content )
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

bool CGPSProtocol :: ValidateLength( BYTEARRAY &content )
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
