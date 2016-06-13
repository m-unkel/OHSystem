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
#ifndef SAVEGAME_H
#define SAVEGAME_H

#include "../game/gameslot.h"

//
// CSaveGame
//

class CSaveGame : public CPacked
{
private:
    string m_FileName;
    string m_FileNameNoPath;
    string m_MapPath;
    string m_GameName;
    unsigned char m_NumSlots;
    vector<CGameSlot> m_Slots;
    uint32_t m_RandomSeed;
    BYTEARRAY m_MagicNumber;

public:
    CSaveGame( );
    virtual ~CSaveGame( );

    string GetFileName( )				{
        return m_FileName;
    }
    string GetFileNameNoPath( )			{
        return m_FileNameNoPath;
    }
    string GetMapPath( )				{
        return m_MapPath;
    }
    string GetGameName( )				{
        return m_GameName;
    }
    unsigned char GetNumSlots( )		{
        return m_NumSlots;
    }
    vector<CGameSlot> GetSlots( )		{
        return m_Slots;
    }
    uint32_t GetRandomSeed( )			{
        return m_RandomSeed;
    }
    BYTEARRAY GetMagicNumber( )			{
        return m_MagicNumber;
    }

    void SetFileName( string nFileName )				{
        m_FileName = nFileName;
    }
    void SetFileNameNoPath( string nFileNameNoPath )	{
        m_FileNameNoPath = nFileNameNoPath;
    }

    void ParseSaveGame( );
public:
	static void RegisterPythonClass( );
};

#endif
