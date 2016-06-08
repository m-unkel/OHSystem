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

#ifndef GAME_H
#define GAME_H

//
// CGame
//

class CDBBan;
class CDBGame;
class CDBGamePlayer;
class CStats;
class CCallableBanCheck;
class CCallablePassCheck;
class CCallableGameAdd;
class CCallableGamePlayerSummaryCheck;
class CCallableStatsPlayerSummaryCheck;
class CCallablePUp;
class CCallableRegAdd;

typedef pair<string,CCallableBanCheck *> PairedBanCheck;
typedef pair<string,CCallablePassCheck *> PairedPassCheck;
typedef pair<string,CCallableStatsPlayerSummaryCheck *> PairedGSCheck;
typedef pair<string,CCallableStatsPlayerSummaryCheck *> PairedRankCheck;
typedef pair<string,CCallableStatsPlayerSummaryCheck *> PairedStreakCheck;
typedef pair<string,CCallableStatsPlayerSummaryCheck *> PairedSCheck;
typedef pair<string,CCallablePUp *> PairedPUp;
typedef pair<string,CCallableRegAdd *> PairedRegAdd;

class CGame : public CBaseGame
{
protected:
    CDBBan *m_DBBanLast;						// last ban for the !banlast command - this is a pointer to one of the items in m_DBBans
    vector<CDBBan *> m_DBBans;					// vector of potential ban data for the database (see the Update function for more info, it's not as straightforward as you might think)
    CDBGame *m_DBGame;							// potential game data for the database
    vector<CDBGamePlayer *> m_DBGamePlayers;	// vector of potential gameplayer data for the database
    CStats *m_Stats;							// class to keep track of game stats such as kills/deaths/assists in dota
    CCallableGameAdd *m_CallableGameAdd;		// threaded database game addition in progress
    vector<PairedBanCheck> m_PairedBanChecks;	// vector of paired threaded database ban checks in progress
    vector<PairedPassCheck> m_PairedPassChecks;       // vector of paired threaded database password checks in progress
    vector<PairedGSCheck> m_PairedGSChecks;	// vector of paired threaded database game player summary checks in progress
    vector<PairedRankCheck> m_PairedRankChecks;
    vector<PairedStreakCheck> m_PairedStreakChecks;       // vector of paired threaded database ingame checks in progress
    vector<PairedSCheck> m_PairedSChecks;	// vector of paired threaded database DotA player summary checks in progress
    vector<PairedPUp> m_PairedPUps;                   // vector of paired threaded database permission changes in progress
    vector<PairedRegAdd> m_PairedRegAdds;               // vector of paired threaded database reg adds in progress
    vector<string> m_AutoBans;
    bool m_EarlyDraw;
    bool IsAutoBanned( string name );
    uint32_t m_ForfeitTime;						// time that players forfeited, or 0 if not forfeited
    uint32_t m_ForfeitTeam;						// id of team that forfeited
    uint32_t m_LoosingTeam;
    uint32_t m_LastLeaverTime;
    uint32_t m_BalanceVotes;

public:
    CGame( COHBot *nOHBot, CMap *nMap, CSaveGame *nSaveGame, uint16_t nHostPort, unsigned char nGameState, string nGameName, string nOwnerName, string nCreatorName, string nCreatorServer, uint32_t nGameType, uint32_t nHostCounter );
    virtual ~CGame( );

    virtual bool Update( void *fd, void *send_fd );
    virtual void EventPlayerDeleted( CGamePlayer *player);
    virtual bool EventPlayerAction( CGamePlayer *player, CIncomingAction *action );
    virtual bool EventPlayerBotCommand( CGamePlayer *player, string command, string payload, bool force = false, string ExecName = "");
    virtual void EventGameStarted( );
    virtual bool IsGameDataSaved( );
    virtual void SaveGameData( );
    virtual bool CustomVoteKickReason( string reason );
    virtual string GetRule( string tag );
    virtual string GetRuleTags( );
    virtual void PlayerUsed( string thething, uint32_t thetype, string playername );

public:
	static void RegisterPythonClass( );
};

#endif
