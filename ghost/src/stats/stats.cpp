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
#include "stats.h"

//
// CStats
//

CStats :: CStats( CBaseGame *nGame ) : m_Game( nGame ), m_Locked( false )
{

}

CStats :: ~CStats( )
{

}

bool CStats :: ProcessAction( CIncomingAction *Action )
{
    return false;
}

void CStats :: Save( COHBot *GHost, COHBotDB *DB, uint32_t GameID )
{

}
