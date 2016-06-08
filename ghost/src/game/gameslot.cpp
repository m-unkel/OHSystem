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
#include "gameslot.h"

//
// CGameSlot
//

CGameSlot :: CGameSlot( BYTEARRAY &n ) : m_PID( 0 ), m_DownloadStatus( 255 ), m_SlotStatus( SLOTSTATUS_OPEN ), m_Computer( 0 ), m_Team( 0 ), m_Colour( 1 ), m_Race( SLOTRACE_RANDOM ), m_ComputerType( SLOTCOMP_NORMAL ), m_Handicap( 100 )
{
    if( n.size( ) >= 7 )
    {
        m_PID = n[0];
        m_DownloadStatus = n[1];
        m_SlotStatus = n[2];
        m_Computer = n[3];
        m_Team = n[4];
        m_Colour = n[5];
        m_Race = n[6];

        if( n.size( ) >= 8 )
            m_ComputerType = n[7];

        if( n.size( ) >= 9 )
            m_Handicap = n[8];
    }
}

CGameSlot :: CGameSlot( unsigned char nPID, unsigned char nDownloadStatus, unsigned char nSlotStatus, unsigned char nComputer, unsigned char nTeam, unsigned char nColour, unsigned char nRace, unsigned char nComputerType, unsigned char nHandicap )
    : m_PID( nPID ), m_DownloadStatus( nDownloadStatus ), m_SlotStatus( nSlotStatus ), m_Computer( nComputer ), m_Team( nTeam ), m_Colour( nColour ), m_Race( nRace ), m_ComputerType( nComputerType ), m_Handicap( nHandicap )
{

}

CGameSlot :: ~CGameSlot( )
{

}

BYTEARRAY CGameSlot :: GetByteArray( ) const
{
    BYTEARRAY b;
    b.push_back( m_PID );
    b.push_back( m_DownloadStatus );
    b.push_back( m_SlotStatus );
    b.push_back( m_Computer );
    b.push_back( m_Team );
    b.push_back( m_Colour );
    b.push_back( m_Race );
    b.push_back( m_ComputerType );
    b.push_back( m_Handicap );
    return b;
}
