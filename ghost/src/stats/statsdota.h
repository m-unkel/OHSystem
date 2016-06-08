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
#ifndef STATSDOTA_H
#define STATSDOTA_H

//
// CStatsDOTA
//

class CDBDotAPlayer;

class CStatsDOTA : public CStats
{
private:
    CDBDotAPlayer *m_Players[12];
    uint32_t m_Winner;
    uint32_t m_Min;
    uint32_t m_Sec;

    // Custom
    uint32_t m_TowerLimit; // win condition on number of towers destroyed
    uint32_t m_KillLimit; // win condition on number of kills
    uint32_t m_TimeLimit; // time limit win condition; winner is more kills, or if even then higher (creep kills + creep denies) value
    uint32_t m_SentinelTowers;
    uint32_t m_ScourgeTowers;
    uint32_t m_SentinelKills;
    uint32_t m_ScourgeKills;
    uint32_t m_LastCreepTime; // last time we received creep stats, for the time limit win condition
    string victim;
    string killer;
    bool m_FirstBlood;
    uint32_t m_LeaverKills[12];
    uint32_t m_LeaverDeaths[12];
    uint32_t m_AssistsOnLeaverKills[12];
    uint32_t m_DeathsByLeaver[12];
    uint32_t m_LatestKill[12];
    uint32_t m_KillCounter[12];
    string TypePrefix;
    uint32_t m_KillStreakCounter[12];
    string m_BufferedItemOne[12];
    string m_BufferedItemTwo[12];
    string m_BufferedItemThree[12];
    string m_BufferedItemFour[12];
    string m_BufferedItemFive[12];
    string m_BufferedItemSix[12];
    uint32_t m_BufferedItemCount[12];
    uint32_t m_FFKills[12];
    uint32_t m_MaphackClickCounter[12];

public:
    CStatsDOTA( CBaseGame *nGame );
    virtual ~CStatsDOTA( );

    virtual bool ProcessAction( CIncomingAction *Action );
    virtual void Save( COHBot *GHost, COHBotDB *DB, uint32_t GameID );
    virtual void SetWinner( uint32_t nWinner ) {
        m_Winner = nWinner;
    }
    virtual uint32_t GetWinner( ) { return m_Winner; }
};

#endif
