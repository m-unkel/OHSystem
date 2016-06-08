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

#ifndef STATS_H
#define STATS_H

//
// CStats
//

// the stats class is passed a copy of every player action in ProcessAction when it's received
// then when the game is over the Save function is called
// so the idea is that you parse the actions to gather data about the game, storing the results in any member variables you need in your subclass
// and in the Save function you write the results to the database
// e.g. for dota the number of kills/deaths/assists, etc...
// the base class is almost completely empty

class CIncomingAction;
class COHBotDB;

class CStats
{
protected:
    CBaseGame *m_Game;
    bool m_Locked;

public:
    CStats( CBaseGame *nGame );
    virtual ~CStats( );

    virtual bool ProcessAction( CIncomingAction *Action );
    virtual void Save( COHBot *GHost, COHBotDB *DB, uint32_t GameID );
    virtual void SetWinner( uint32_t nWinner ) {}
    inline uint32_t GetWinner( ) {}
    virtual void LockStats( ) {
        m_Locked = true;
    }
};

#endif
