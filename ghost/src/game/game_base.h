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

#ifndef GAME_BASE_H
#define GAME_BASE_H

#include "gameslot.h"
#include "../database/ghostdb.h"

//
// CBaseGame
//

class CDBBan;
class CTCPServer;
class CGameProtocol;
class CPotentialPlayer;
class CGamePlayer;
class CMap;
class CSaveGame;
class CReplay;
class CIncomingJoinPlayer;
class CIncomingAction;
class CIncomingChatPlayer;
class CIncomingMapSize;
class CCallableScoreCheck;
class CIncomingGarenaPlayer;
class CCallablePWCheck;
class CCallablepm;
class CCallableStatsPlayerSummaryCheck;
class CCallablepenp;
class CCallableBanAdd;
class CCallableBanCheck2;
class CCallableStoreLog;
class CCallablePList;
class CCallableBanList;
class CCallableTBRemove;
class CCallableConnectCheck;
class CCallableGameDBInit;
class CCallableGameUpdate;
class CDBInbox;
class CCallableInboxSummaryCheck;
class CCallableGamePlayerAdd;
struct ReservedPlayer;
struct DeniedPlayer;
class CCallableStatsSystem;

typedef pair<string,CCallablePWCheck *> PairedPWCheck;
typedef pair<string,CCallablepm *> Pairedpm;
typedef pair<string,CCallableStatsPlayerSummaryCheck *> PairedWPCheck;
typedef pair<string,CCallableBanCheck2 *> PairedBanCheck2;
typedef pair<string,CCallablepenp *> Pairedpenp;
typedef pair<string,CCallableBanAdd *> PairedBanAdd;
typedef pair<string,CCallableStoreLog *> PairedLogUpdate;
typedef pair<string,CCallableInboxSummaryCheck *> PairedINCheck;
typedef pair<string,CCallableGamePlayerAdd *> PairedGPAdd;
typedef pair<string,CCallableGameUpdate *> PairedGameUpdate;
typedef pair<string,CCallableStatsSystem *> PairedSS;

class CBaseGame
{
public:
    COHBot *m_OHBot;
    vector<ReservedPlayer> m_ReservedPlayers;
    vector<PairedSS> m_PairedSSs;

protected:
    CDBBan *m_DBBanLast;						// last ban for the !banlast command - this is a pointer to one of the items in m_DBBans
    vector<CDBBan *> m_DBBans;					// vector of potential ban data for the database (see the Update function for more info, it's not as straightforward as you might think)
    CTCPServer *m_Socket;							// listening socket
    CGameProtocol *m_Protocol;						// game protocol
    map<uint32_t, CPotentialPlayer*> m_BannedPlayers;
    vector<CPotentialPlayer *> m_Potentials;		// vector of potential players (connections that haven't sent a W3GS_REQJOIN packet yet)
    vector<CGamePlayer *> m_DeletedPlayers;			// vector of deleted players
    vector<CCallableScoreCheck *> m_ScoreChecks;
    vector<PairedPWCheck> m_PairedPWChecks;				// vector for checking if a player joined with a password
    vector<Pairedpm> m_Pairedpms;
    vector<PairedGPAdd> m_PairedGPAdds;
    vector<PairedWPCheck> m_PairedWPChecks;
    vector<PairedBanCheck2> m_PairedBanCheck2s;
    vector<PairedLogUpdate> m_PairedLogUpdates;
    vector<PairedINCheck> m_PairedINChecks;       // vector of paired threaded database ingame checks in progress
    vector<PairedGameUpdate> m_GameUpdate;
    CCallableGameDBInit *m_CallableGameDBInit;
    queue<CIncomingAction *> m_Actions;				// queue of actions to be sent
    vector<string> m_Reserved;						// vector of player names with reserved slots (from the !hold command)
    set<string> m_IgnoredNames;						// set of player names to NOT print ban messages for when joining because they've already been printed
    set<string> m_IPBlackList;						// set of IP addresses to blacklist from joining (todotodo: convert to uint32's for efficiency)
    vector<CGameSlot> m_EnforceSlots;				// vector of slots to force players to use (used with saved games)
    vector<PIDPlayer> m_EnforcePlayers;				// vector of pids to force players to use (used with saved games)
    CCallableBanList *m_CallableBanList;			// threaded database ban list in progress
    vector<CDBBan *> m_Bans;						// vector of cached bans
    vector<CCallableConnectCheck *> m_ConnectChecks;        // session validation for wc3connect system
    CCallableTBRemove *m_CallableTBRemove;
    CCallablePList *m_CallablePList;                // threaded database permission list in progress
    CSaveGame *m_SaveGame;							// savegame data (this is a pointer to global data)
    CReplay *m_Replay;								// replay
    bool m_Exiting;									// set to true and this class will be deleted next update
    bool m_Saving;									// if we're currently saving game data to the database
    unsigned char m_GameState;						// game state, public or private
    unsigned char m_VirtualHostPID;					// virtual host's PID
    unsigned char m_FakePlayerPID;					// the fake player's PID (if present)
    unsigned char m_GProxyEmptyActions;
    string m_LastGameName;							// last game name (the previous game name before it was rehosted)
    string m_VirtualHostName;						// virtual host's name
    string m_OwnerName;								// name of the player who owns this game (should be considered an admin)
    string m_CreatorName;							// name of the player who created this game
    string m_CreatorServer;							// battle.net server the player who created this game was on
    uint32_t m_GameType;							// game type of this game
    string m_AnnounceMessage;						// a message to be sent every m_AnnounceInterval seconds
    string m_StatString;							// the stat string when the game started (used when saving replays)
    string m_KickVotePlayer;						// the player to be kicked with the currently running kick vote
    string m_HCLCommandString;						// the "HostBot Command Library" command string, used to pass a limited amount of data to specially designed maps
    uint32_t m_RandomSeed;							// the random seed sent to the Warcraft III clients
    uint32_t m_Latency;								// the number of ms to wait between sending action packets (we queue any received during this time)
    uint32_t m_SyncLimit;							// the maximum number of packets a player can fall out of sync before starting the lag screen
    uint32_t m_SyncCounter;							// the number of actions sent so far (for determining if anyone is lagging)
    uint32_t m_LastPingTime;						// GetTime when the last ping was sent
    uint32_t m_LastRefreshTime;						// GetTime when the last game refresh was sent
    uint32_t m_LastReservedRefreshTime;
    uint32_t m_LastDownloadTicks;					// GetTicks when the last map download cycle was performed
    uint32_t m_DownloadCounter;						// # of map bytes downloaded in the last second
    uint32_t m_LastDownloadCounterResetTicks;		// GetTicks when the download counter was last reset
    uint32_t m_LastAnnounceTime;					// GetTime when the last announce message was sent
    uint32_t m_AnnounceInterval;					// how many seconds to wait between sending the m_AnnounceMessage
    uint32_t m_LastAutoStartTime;					// the last time we tried to auto start the game
    uint32_t m_AutoStartPlayers;					// auto start the game when there are this many players or more
    uint32_t m_LastCountDownTicks;					// GetTicks when the last countdown message was sent
    uint32_t m_CountDownCounter;					// the countdown is finished when this reaches zero
    uint32_t m_StartedLoadingTicks;					// GetTicks when the game started loading
    uint32_t m_StartPlayers;						// number of players when the game started
    uint32_t m_LastLagScreenResetTime;				// GetTime when the "lag" screen was last reset
    uint32_t m_LastActionSentTicks;					// GetTicks when the last action packet was sent
    uint32_t m_LastActionLateBy;					// the number of ticks we were late sending the last action packet by
    uint32_t m_StartedLaggingTime;					// GetTime when the last lag screen started
    uint32_t m_LastLagScreenTime;					// GetTime when the last lag screen was active (continuously updated)
    uint32_t m_LastReservedSeen;					// GetTime when the last reserved player was seen in the lobby
    uint32_t m_StartedKickVoteTime;					// GetTime when the kick vote was started
    uint32_t m_GameOverTime;						// GetTime when the game was over
    uint32_t m_LastPlayerLeaveTicks;				// GetTicks when the most recent player left the game
    double m_MinimumScore;							// the minimum allowed score for matchmaking mode
    double m_MaximumScore;							// the maximum allowed score for matchmaking mode
    bool m_SlotInfoChanged;							// if the slot info has changed and hasn't been sent to the players yet (optimization)
    bool m_Locked;									// if the game owner is the only one allowed to run game commands or not
    bool m_RefreshMessages;							// if we should display "game refreshed..." messages or not
    bool m_RefreshError;							// if there was an error refreshing the game
    bool m_RefreshRehosted;							// if we just rehosted and are waiting for confirmation that it was successful
    bool m_MuteAll;									// if we should stop forwarding ingame chat messages targeted for all players or not
    bool m_MuteLobby;								// if we should stop forwarding lobby chat messages
    bool m_CountDownStarted;						// if the game start countdown has started or not
    bool m_LoadInGame;								// if the load-in-game feature is enabled or not
    bool m_Lagging;									// if the lag screen is active or not
    bool m_AutoSave;								// if we should auto save the game before someone disconnects
    bool m_MatchMaking;								// if matchmaking mode is enabled
    bool m_LocalAdminMessages;						// if local admin messages should be relayed or not
    string m_GetMapType;							// map_type
    uint32_t m_LastLogDataUpdate;
    bool m_Balanced;
    bool m_GameBalance;
    uint32_t m_GameLoadedTime;
    bool m_SoftGameOver;
    uint32_t m_LastProcessedTicks;
    bool m_SendAnnounce;
    vector<DeniedPlayer> m_Denied;					//vector for denied Users
    bool m_LimitCountries;
    bool m_DenieCountries;
    vector<string> m_LimitedCountries;
    bool m_GameNoGarena;
    uint32_t m_LastInGameAnnounce;
    vector<string> m_LobbyLog;
    vector<string> m_GameLog;
    uint32_t m_LastPingWarn;
    bool m_ModeVoted;
    uint32_t m_LatestSlot;
    uint32_t m_Leavers;
    uint32_t m_StartedVoteStartTime;
    uint32_t m_VoteMuteEventTime;
    string m_VoteMutePlayer;
    uint32_t m_VoteMuteTargetTeam;
    uint32_t m_MuteVotes;
    uint32_t m_EnemyVotes;
    uint32_t m_MuteType;
    uint32_t m_LoosingTeam;
    bool m_EndGame;
    uint32_t m_BreakAutoEndVotesNeeded;
    uint32_t m_BreakAutoEndVotes;
    uint32_t m_EndTicks;
    uint32_t m_StartEndTicks;
    bool m_Voted;                                                           // define if the vote process is already done when this option is enabled
    uint32_t m_VotedTimeStart;                                              // when the voting has started, a timer function
    bool m_AllowMapTrading;
    uint32_t m_PartTime;
    string m_LobbyLanguage;
    uint32_t m_LastGameUpdateTime;					// last time game update database callable
    bool m_ForcedMode;
    uint32_t m_ForcedGameMode;
    uint32_t m_ObservingPlayers;
    int m_DoDelete;									// notifies thread to exit
    uint32_t m_LastReconnectHandleTime;				// last time we tried to handle GProxy reconnects

public:
    vector<string> m_DoSayGames;					// vector of strings we should announce to the current game
    boost::mutex m_SayGamesMutex;					// mutex for the above vector

public:
    CBaseGame( COHBot *nOHBot, CMap *nMap, CSaveGame *nSaveGame, uint16_t nHostPort, unsigned char nGameState, string nGameName, string nOwnerName, string nCreatorName, string nCreatorServer, uint32_t nGameType, uint32_t nHostCounter );
    virtual ~CBaseGame( );

    virtual void loop( );
    virtual void doDelete( );
    virtual bool readyDelete( );
    uint32_t m_DatabaseID;                          // the ID number from the database, which we'll use to save replay
    vector<string> m_ModesToVote;                                           // modes which are possible to vote in the current game
    CMap *m_Map;
    uint16_t m_HostPort;                                                    // the port to host games on
    uint32_t m_EntryKey;                                                    // random entry key for LAN, used to prove that a player is actually joining from LAN
    uint32_t m_HostCounter;                                                 // a unique game number
    bool m_GameLoading;                                                             // if the game is currently loading or not
    bool m_GameLoaded;                                                              // if the game has loaded or not
    string m_GameName;
    vector<string> m_BasicDenied;
    vector<Pairedpenp> m_Pairedpenps;
    vector<PairedBanAdd> m_PairedBanAdds;           // vector of paired threaded database ban adds in progress
    vector<CGameSlot> m_Slots;                                              // vector of slots
    vector<CGamePlayer *> m_Players;                        // vector of players
    uint32_t m_CreationTime;						// GetTime when the game was created
    virtual vector<CGameSlot> GetEnforceSlots( )	{
        return m_EnforceSlots;
    }
    virtual vector<PIDPlayer> GetEnforcePlayers( )	{
        return m_EnforcePlayers;
    }
    virtual CSaveGame *GetSaveGame( )				{
        return m_SaveGame;
    }
    virtual uint16_t GetHostPort( )					{
        return m_HostPort;
    }
    virtual unsigned char GetGameState( )			{
        return m_GameState;
    }
    virtual unsigned char GetGProxyEmptyActions( )	{
        return m_GProxyEmptyActions;
    }
    virtual string GetGameName( )					{
        return m_GameName;
    }
    virtual string GetMapName( );
    virtual string GetLastGameName( )				{
        return m_LastGameName;
    }
    virtual string GetVirtualHostName( )			{
        return m_VirtualHostName;
    }
    virtual string GetOwnerName( )					{
        return m_OwnerName;
    }
    virtual string GetCreatorName( )				{
        return m_CreatorName;
    }
    virtual string GetCreatorServer( )				{
        return m_CreatorServer;
    }
    virtual uint32_t GetHostCounter( )				{
        return m_HostCounter;
    }
    virtual uint32_t GetGameType( )                              {
        return m_GameType;
    }
    virtual uint32_t GetLastLagScreenTime( )		{
        return m_LastLagScreenTime;
    }
    virtual bool GetLocked( )						{
        return m_Locked;
    }
    virtual bool GetRefreshMessages( )				{
        return m_RefreshMessages;
    }
    virtual bool GetCountDownStarted( )				{
        return m_CountDownStarted;
    }
    virtual bool GetGameLoading( )					{
        return m_GameLoading;
    }
    virtual bool GetGameLoaded( )					{
        return m_GameLoaded;
    }
    virtual bool GetLagging( )						{
        return m_Lagging;
    }
    virtual uint32_t GetCreationTime( )                                     {
        return m_CreationTime;
    }

    virtual void SetEnforceSlots( vector<CGameSlot> nEnforceSlots )		{
        m_EnforceSlots = nEnforceSlots;
    }
    virtual void SetEnforcePlayers( vector<PIDPlayer> nEnforcePlayers )	{
        m_EnforcePlayers = nEnforcePlayers;
    }
    virtual void SetExiting( bool nExiting )							{
        m_Exiting = nExiting;
    }
    virtual void SetAutoStartPlayers( uint32_t nAutoStartPlayers )		{
        m_AutoStartPlayers = nAutoStartPlayers;
    }
    virtual void SetMinimumScore( double nMinimumScore )				{
        m_MinimumScore = nMinimumScore;
    }
    virtual void SetMaximumScore( double nMaximumScore )				{
        m_MaximumScore = nMaximumScore;
    }
    virtual void SetRefreshError( bool nRefreshError )					{
        m_RefreshError = nRefreshError;
    }
    virtual void SetMatchMaking( bool nMatchMaking )					{
        m_MatchMaking = nMatchMaking;
    }
    virtual void SetCreationTime( uint32_t nCreationTime )                                                         {
        m_CreationTime = nCreationTime;
    }

    virtual uint32_t GetNextTimedActionTicks( );
    virtual uint32_t GetSlotsOccupied( );
    virtual uint32_t GetSlotsAllocated( );
    virtual uint32_t GetSlotsOpen( );
    virtual uint32_t GetNumPlayers( );
    virtual uint32_t GetNumHumanPlayers( );
    virtual string GetDescription( );

    virtual void SetAnnounce( uint32_t interval, string message );

    // processing functions

    virtual unsigned int SetFD( void *fd, void *send_fd, int *nfds );
    virtual bool Update( void *fd, void *send_fd );
    virtual void UpdatePost( void *send_fd );

    // generic functions to send packets to players

    virtual void Send( CGamePlayer *player, BYTEARRAY data );
    virtual void Send( unsigned char PID, BYTEARRAY data );
    virtual void Send( BYTEARRAY PIDs, BYTEARRAY data );
    virtual void SendAll( BYTEARRAY data );

    // functions to send packets to players

    virtual void SendChat( unsigned char fromPID, CGamePlayer *player, string message );
    virtual void SendChat( unsigned char fromPID, unsigned char toPID, string message );
    virtual void SendChat( CGamePlayer *player, string message );
    virtual void SendChat( unsigned char toPID, string message );
    virtual void SendAllChat( unsigned char fromPID, string message );
    virtual void SendAllChat( string message );
    virtual void SendLocalAdminChat( string message );
    virtual void SendAllSlotInfo( );
    virtual void SendVirtualHostPlayerInfo( CGamePlayer *player );
    virtual void SendFakePlayerInfo( CGamePlayer *player );
    virtual void ReservedSlotInfo( CGamePlayer *player );
    virtual void SendAllActions( );
    virtual void SendWelcomeMessage( CGamePlayer *player );
    virtual void SendEndMessage( );
    virtual void SendVirtualLobbyInfo( CPotentialPlayer *player, CDBBan *Ban, uint32_t type, bool gproxy );

    // events
    // note: these are only called while iterating through the m_Potentials or m_Players vectors
    // therefore you can't modify those vectors and must use the player's m_DeleteMe member to flag for deletion

    virtual void EventPlayerDeleted( CGamePlayer *player);
    virtual void EventPlayerDisconnectTimedOut( CGamePlayer *player );
    virtual void EventPlayerDisconnectPlayerError( CGamePlayer *player );
    virtual void EventPlayerDisconnectSocketError( CGamePlayer *player );
    virtual void EventPlayerDisconnectConnectionClosed( CGamePlayer *player );
    virtual void EventPlayerJoined( CPotentialPlayer *potential, CIncomingJoinPlayer *joinPlayer );
    virtual void EventPlayerLeft( CGamePlayer *player, uint32_t reason );
    virtual void EventPlayerLoaded( CGamePlayer *player );
    virtual bool EventPlayerAction( CGamePlayer *player, CIncomingAction *action );
    virtual void EventPlayerKeepAlive( CGamePlayer *player, uint32_t checkSum );
    virtual void EventPlayerChatToHost( CGamePlayer *player, CIncomingChatPlayer *chatPlayer );
    virtual bool EventPlayerBotCommand( CGamePlayer *player, string command, string payload , bool force = false, string execplayer = "");
    virtual void EventPlayerChangeTeam( CGamePlayer *player, unsigned char team );
    virtual void EventPlayerChangeColour( CGamePlayer *player, unsigned char colour );
    virtual void EventPlayerChangeRace( CGamePlayer *player, unsigned char race );
    virtual void EventPlayerChangeHandicap( CGamePlayer *player, unsigned char handicap );
    virtual void EventPlayerDropRequest( CGamePlayer *player );
    virtual void EventPlayerMapSize( CGamePlayer *player, CIncomingMapSize *mapSize );
    virtual void EventPlayerPongToHost( CGamePlayer *player, uint32_t pong );

    // these events are called outside of any iterations

    virtual void EventGameRefreshed( string server );
    virtual void EventGameStarted( );
    virtual void EventGameLoaded( );

    // other functions

    virtual unsigned char GetSIDFromPID( unsigned char PID );
    virtual CGamePlayer *GetPlayerFromPID( unsigned char PID );
	virtual CGamePlayer *GetPlayerFromPID2( unsigned char PID );
	virtual CGamePlayer *GetPlayerFromSID( unsigned char SID );
	virtual CGamePlayer *GetPlayerFromSID2( unsigned char SID );
    virtual CGamePlayer *GetPlayerFromName( string name, bool sensitive );
    virtual uint32_t GetPlayerFromNamePartial( string name, CGamePlayer **player );
    virtual CGamePlayer *GetPlayerFromColour( unsigned char colour );
    virtual string GetPlayerList( );
    virtual unsigned char GetNewPID( );
    virtual unsigned char GetNewColour( );
    virtual BYTEARRAY GetPIDs( );
    virtual BYTEARRAY GetPIDs( unsigned char excludePID );
    virtual unsigned char GetHostPID( );
    virtual unsigned char GetEmptySlot( bool reserved );
    virtual unsigned char GetEmptySlot( unsigned char team, unsigned char PID );
    virtual void SwapSlots( unsigned char SID1, unsigned char SID2 );
    virtual void OpenSlot( unsigned char SID, bool kick );
    virtual void CloseSlot( unsigned char SID, bool kick );
    virtual void ComputerSlot( unsigned char SID, unsigned char skill, bool kick );
    virtual void ColourSlot( unsigned char SID, unsigned char colour );
    virtual void OpenAllSlots( );
    virtual void CloseAllSlots( );
    virtual void ShuffleSlots( );
    virtual vector<unsigned char> BalanceSlotsRecursive( vector<unsigned char> PlayerIDs, unsigned char *TeamSizes, double *PlayerScores, unsigned char StartTeam );
    virtual void BalanceSlots( );
    virtual void OHFixedBalance( );
    virtual void AddToSpoofed( string server, string name, bool sendMessage );
    virtual void AddToReserved( string name, unsigned char SID, uint32_t level );
    virtual bool IsOwner( string name );
    virtual bool IsReserved( string name );
    virtual bool IsDownloading( );
    virtual bool IsGameDataSaved( );
    virtual void SaveGameData( );
    virtual void StartCountDown( bool force );
    virtual void StartCountDownAuto( bool requireSpoofChecks );
    virtual void StopPlayers( string reason );
    virtual void StopLaggers( string reason );
    virtual void CreateVirtualHost( );
    virtual void DeleteVirtualHost( );
    virtual void CreateFakePlayer( );
    virtual void DeleteFakePlayer( );
    virtual bool is_digits( const std::string &str );
    double m_ScourgeWinPoints;						 // scourge value to calculate the winperc
    double m_SentinelWinPoints;						 // sentinel value to calculate the winperc
    double m_TotalWinPoints;						 // total value to calculate the winperc
    uint32_t m_LockedPlayers;
    virtual uint32_t GetGameTicks( )				{
        return m_GameTicks;
    }
    string m_LogData;
    vector<string> m_AdminLog;
    uint32_t m_GameTicks;
    bool m_PlayerUpdate;
    uint32_t m_ChatID;
    virtual bool IsDenied( string username, string ip );
    virtual bool HasDeniedWordPharse( string username );
    bool m_PauseReq;
    uint32_t m_PauseIntroTime;
    bool m_Paused;
    uint32_t m_PauseTicks;
    uint32_t m_PauseTime;
    bool m_SendPauseInfo;
    virtual BYTEARRAY Silence(BYTEARRAY PIDs);
    virtual void GAME_Print( uint32_t type, string MinString, string SecString, string Player1, string Player2, string message );
    virtual void AnnounceEvent( uint32_t randomnumber );
    string GetColoredName( string defaultname );
    virtual string GetJoinedRealm( uint32_t hostcounter );
    uint32_t m_GameAlias;
    string m_GameAliasName;
    string m_lGameAliasName;
    virtual void StartVoteMode( );
    void GetVotingModes( string allmodes );
    virtual void DoGameUpdate(bool reset);
    virtual vector<PlayerOfPlayerList> GetPlayerListOfGame( );
    virtual void BanPlayerByPenality( string player, string playerid, string admin, uint32_t points, string reason );
    virtual bool AllSlotsOccupied();
    void DenyPlayer( string name, string ip, bool perm );
    void AppendLogData(string toAppend);
    string DolanTime( string Out );
public:
	static void RegisterPythonClass( );
};

struct ReservedPlayer {
    unsigned char SID;
    unsigned char PID;
    string Name;
    uint32_t Time;
    uint32_t Level;
};
struct DeniedPlayer {
    string Name;
    string IP;
    uint32_t Time;
};
#endif

