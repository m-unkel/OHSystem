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
#include "crc32.h"

void CCRC32 :: Initialize( )
{
    for( int iCodes = 0; iCodes <= 0xFF; ++iCodes )
    {
        ulTable[iCodes] = Reflect( iCodes, 8 ) << 24;

        for( int iPos = 0; iPos < 8; iPos++ )
            ulTable[iCodes] = ( ulTable[iCodes] << 1 ) ^ ( ulTable[iCodes] & (1 << 31) ? CRC32_POLYNOMIAL : 0 );

        ulTable[iCodes] = Reflect( ulTable[iCodes], 32 );
    }
}

uint32_t CCRC32 :: Reflect( uint32_t ulReflect, char cChar )
{
    uint32_t ulValue = 0;

    for( int iPos = 1; iPos < ( cChar + 1 ); ++iPos )
    {
        if( ulReflect & 1 )
            ulValue |= 1 << ( cChar - iPos );

        ulReflect >>= 1;
    }

    return ulValue;
}

uint32_t CCRC32 :: FullCRC( unsigned char *sData, uint32_t ulLength )
{
    uint32_t ulCRC = 0xFFFFFFFF;
    PartialCRC( &ulCRC, sData, ulLength );
    return ulCRC ^ 0xFFFFFFFF;
}

void CCRC32 :: PartialCRC( uint32_t *ulInCRC, unsigned char *sData, uint32_t ulLength )
{
    while( ulLength-- )
        *ulInCRC = ( *ulInCRC >> 8 ) ^ ulTable[( *ulInCRC & 0xFF ) ^ *sData++];
}
