/**
 * Copyright [2016] [m-unkel]
 * Mail: info@unive.de
 * URL: https://github.com/m-unkel/OHSystem
 *
 * This is modified from OHSYSTEM https://github.com/OHSystem
 *
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
 *
 */

#ifndef GAMESLOT_H
#define GAMESLOT_H

#define SLOTSTATUS_OPEN		0
#define SLOTSTATUS_CLOSED	1
#define SLOTSTATUS_OCCUPIED	2

#define SLOTRACE_HUMAN		1
#define SLOTRACE_ORC		2
#define SLOTRACE_NIGHTELF	4
#define SLOTRACE_UNDEAD		8
#define SLOTRACE_RANDOM		32
#define SLOTRACE_SELECTABLE	64

#define SLOTCOMP_EASY		0
#define SLOTCOMP_NORMAL		1
#define SLOTCOMP_HARD		2

#define SLOTTEAM_OBSERVER   12

#define SLOT_UNKNOWN        255

//
// CGameSlot
//

class CGameSlot
{
private:
    unsigned char m_PID;				// player id
    unsigned char m_DownloadStatus;		// download status (0% to 100%)
    unsigned char m_SlotStatus;			// slot status (0 = open, 1 = closed, 2 = occupied)
    unsigned char m_Computer;			// computer (0 = no, 1 = yes)
    unsigned char m_Team;				// team
    unsigned char m_Colour;				// colour
    unsigned char m_Race;				// race (1 = human, 2 = orc, 4 = night elf, 8 = undead, 32 = random, 64 = selectable)
    unsigned char m_ComputerType;		// computer type (0 = easy, 1 = human or normal comp, 2 = hard comp)
    unsigned char m_Handicap;			// handicap

public:
    CGameSlot( BYTEARRAY &n );
    CGameSlot( unsigned char nPID, unsigned char nDownloadStatus, unsigned char nSlotStatus, unsigned char nComputer, unsigned char nTeam, unsigned char nColour, unsigned char nRace, unsigned char nComputerType = 1, unsigned char nHandicap = 100 );
    ~CGameSlot( );

    unsigned char GetPID( )				{
        return m_PID;
    }
    unsigned char GetDownloadStatus( )	{
        return m_DownloadStatus;
    }
    unsigned char GetSlotStatus( )		{
        return m_SlotStatus;
    }
    unsigned char GetComputer( )		{
        return m_Computer;
    }
    unsigned char GetTeam( )			{
        return m_Team;
    }
    unsigned char GetColour( )			{
        return m_Colour;
    }
    unsigned char GetRace( )			{
        return m_Race;
    }
    unsigned char GetComputerType( )	{
        return m_ComputerType;
    }
    unsigned char GetHandicap( )		{
        return m_Handicap;
    }

    void SetPID( unsigned char nPID )							{
        m_PID = nPID;
    }
    void SetDownloadStatus( unsigned char nDownloadStatus )		{
        m_DownloadStatus = nDownloadStatus;
    }
    void SetSlotStatus( unsigned char nSlotStatus )				{
        m_SlotStatus = nSlotStatus;
    }
    void SetComputer( unsigned char nComputer )					{
        m_Computer = nComputer;
    }
    void SetTeam( unsigned char nTeam )							{
        m_Team = nTeam;
    }
    void SetColour( unsigned char nColour )						{
        m_Colour = nColour;
    }
    void SetRace( unsigned char nRace )							{
        m_Race = nRace;
    }
    void SetComputerType( unsigned char nComputerType )			{
        m_ComputerType = nComputerType;
    }
    void SetHandicap( unsigned char nHandicap )					{
        m_Handicap = nHandicap;
    }

    BYTEARRAY GetByteArray( ) const;
public:
	static void RegisterPythonClass( );
};

#endif
