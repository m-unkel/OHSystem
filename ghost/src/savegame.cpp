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

#include "ohbot.h"
#include "utils/util.h"
#include "utils/packed.h"
#include "savegame.h"

//
// CSaveGame
//

CSaveGame :: CSaveGame( ) : CPacked( ), m_NumSlots( 0 ), m_RandomSeed( 0 )
{

}

CSaveGame :: ~CSaveGame( )
{

}

#define READB( x, y, z )	(x).read( (char *)(y), (z) )
#define READSTR( x, y )		getline( (x), (y), '\0' )

void CSaveGame :: ParseSaveGame( )
{
    m_MapPath.clear( );
    m_GameName.clear( );
    m_NumSlots = 0;
    m_Slots.clear( );
    m_RandomSeed = 0;
    m_MagicNumber.clear( );

    if( m_Flags != 0 )
    {
        CONSOLE_Print( "[SAVEGAME] invalid replay (flags mismatch)" );
        m_Valid = false;
        return;
    }

    istringstream ISS( m_Decompressed );

    // savegame format figured out by Varlock:
    // string		-> map path
    // 0 (string?)	-> ??? (no idea what this is)
    // string		-> game name
    // 0 (string?)	-> ??? (maybe original game password)
    // string		-> stat string
    // 4 bytes		-> ??? (seems to be # of slots)
    // 4 bytes		-> ??? (seems to be 0x01 0x28 0x49 0x00 on both of the savegames examined)
    // 2 bytes		-> ??? (no idea what this is)
    // slot structure
    // 4 bytes		-> magic number

    unsigned char Garbage1;
    uint16_t Garbage2;
    uint32_t Garbage4;
    string GarbageString;
    uint32_t MagicNumber;

    READSTR( ISS, m_MapPath );				// map path
    READSTR( ISS, GarbageString );			// ???
    READSTR( ISS, m_GameName );				// game name
    READSTR( ISS, GarbageString );			// ???
    READSTR( ISS, GarbageString );			// stat string
    READB( ISS, &Garbage4, 4 );				// ???
    READB( ISS, &Garbage4, 4 );				// ???
    READB( ISS, &Garbage2, 2 );				// ???
    READB( ISS, &m_NumSlots, 1 );			// number of slots

    if( m_NumSlots > 12 )
    {
        CONSOLE_Print( "[SAVEGAME] invalid savegame (too many slots)" );
        m_Valid = false;
        return;
    }

    for( unsigned char i = 0; i < m_NumSlots; i++ )
    {
        unsigned char SlotData[9];
        READB( ISS, SlotData, 9 );			// slot data
        m_Slots.push_back( CGameSlot( SlotData[0], SlotData[1], SlotData[2], SlotData[3], SlotData[4], SlotData[5], SlotData[6], SlotData[7], SlotData[8] ) );
    }

    READB( ISS, &m_RandomSeed, 4 );			// random seed
    READB( ISS, &Garbage1, 1 );				// GameType
    READB( ISS, &Garbage1, 1 );				// number of player slots (non observer)
    READB( ISS, &MagicNumber, 4 );			// magic number

    if( ISS.eof( ) || ISS.fail( ) )
    {
        CONSOLE_Print( "[SAVEGAME] failed to parse savegame header" );
        m_Valid = false;
        return;
    }

    m_MagicNumber = UTIL_CreateByteArray( MagicNumber, false );
    m_Valid = true;
}
