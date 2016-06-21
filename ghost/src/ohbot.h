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
 * This is free software: You can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This is modified from GHOST++: http://ohbotplusplus.googlecode.com/
 *
 */

#ifndef GHOST_H
#define GHOST_H

#include "includes.h"

#define MODE_LAN			1
#define MODE_GARENA			2
#define MODE_BNET			4
#define MODE_GPROXY			8
#define MODE_EXIT_NICELY	16
#define MODE_EXIT			32
#define MODE_ENABLED		128


//
// COHBot
//
class CUDPSocket;
class CTCPServer;
class CTCPSocket;
class CTCPClient;
class CGPSProtocol;
class CGCBIProtocol;
class CCRC32;
class CSHA1;
class CBNET;
class CBaseGame;
class COHBotDB;
class CBaseCallable;
class CLanguage;
class CMap;
class CSaveGame;
class CConfig;
class CCallableCommandList;
class CCallableFlameList;
class CCallableForcedGProxyList;
class CCallableAnnounceList;
class CCallableDCountryList;
class CCallableGameDBInit;
class CCallableDeniedNamesList;
class CCallableAliasList;
struct translationTree;
struct permission;
struct cachedPlayer;
struct GProxyReconnector {
	CTCPSocket *socket;
	unsigned char PID;
	uint32_t ReconnectKey;
	uint32_t LastPacket;
	uint32_t PostedTime;
};

class COHBot
{
public:

	// bot settings
	uint8_t m_RunMode;
	string m_Version;						// GHost++ version string
	uint32_t m_BotID;

	// sockets, network protocols and checksums
    CUDPSocket *m_UDPSocket;				// a UDP socket for sending broadcasts and other junk (used with !sendlan)
    CUDPSocket *m_GarenaSocket;
    CTCPServer *m_ReconnectSocket;			// listening socket for GProxy++ reliable reconnects
    vector<CTCPSocket *> m_ReconnectSockets;// vector of sockets attempting to reconnect (connected but not identified yet)
    CGPSProtocol *m_GPSProtocol;
    CGCBIProtocol *m_GCBIProtocol;
    vector<CBNET *> m_BNETs;				// all our battle.net connections (there can be more than one)
	CCRC32 *m_CRC;							// for calculating CRC's
	CSHA1 *m_SHA;							// for calculating SHA1's

	// games and gamethreads
    CBaseGame *m_CurrentGame;				// this game is still in the lobby state
    vector<CBaseGame *> m_Games;			// these games are in progress
    boost::thread_group m_GameThreads;		// the threads for games in progress and stuff
    boost::mutex m_GamesMutex;

	// database
    COHBotDB *m_DB;							// database
    COHBotDB *m_DBLocal;					// local database (for temporary data)
    vector<CBaseCallable *> m_Callables;	// vector of orphaned callables waiting to die
    boost::mutex m_CallablesMutex;
    vector<BYTEARRAY> m_LocalAddresses;		// vector of local IP addresses

	// language and texts
	string m_LanguagesPath;
	uint32_t m_FallBackLanguage;
    CLanguage *m_Language;					// language
    vector<translationTree> m_LanguageBundle;
	string m_MOTDFile;						// config value: motd.txt
	string m_GameLoadedFile;				// config value: gameloaded.txt
	string m_GameOverFile;					// config value: gameover.txt

	// config
    string LanCFGPath;
	unsigned char m_LANWar3Version;			// config value: LAN warcraft 3 version
	string m_Warcraft3Path;					// config value: Warcraft 3 path
	bool m_TFT;								// config value: TFT enabled or not

	// maps
	string m_DefaultMap;					// config value: default map (map.cfg)
	string m_MapPath;						// config value: map path
	string m_MapCFGPath;					// config value: map cfg path
    CMap *m_Map;							// the currently loaded map
    CMap *m_AdminMap;						// the map to use in the admin game
    CMap *m_AutoHostMap;					// the map to use when autohosting
    CSaveGame *m_SaveGame;					// the save game to use
    vector<PIDPlayer> m_EnforcePlayers;		// vector of pids to force players to use in the next game (used with saved games)

	//  Files, rules, ranks
	string m_SharedFilesPath;
	string m_IPBlackListFile;				// config value: IP blacklist file (ipblacklist.txt)
	vector<string> m_Rules;
	bool m_RanksLoaded;
	vector<string> m_Ranks;

	// autohosting config
    string m_AutoHostGameName;				// the base game name to auto host with
    string m_AutoHostOwner;
    string m_AutoHostServer;
    uint32_t m_AutoHostGameType;
	uint32_t m_AutoHostMaximumGames;		// maximum number of games to auto host
	uint32_t m_AutoHostAutoStartPlayers;	// when using auto hosting auto start the game when this many players have joined
	uint32_t m_LastAutoHostTime;			// GetTime when the last auto host was attempted
	bool m_AutoHostMatchMaking;
	double m_AutoHostMinimumScore;
	double m_AutoHostMaximumScore;
	uint32_t m_AutoRehostTime;

	uint32_t m_HostCounter;					// the current host counter (a unique number to identify a game, incremented each time a game is created)
	uint32_t m_ReservedHostCounter;

    uint32_t m_MinVIPGames;
    uint32_t m_RegVIPGames;
    bool m_OHBalance;
    bool m_HighGame;
    uint32_t m_MinLimit;
    bool m_ObserverFake;
    uint32_t m_MinFF;
    bool m_CheckIPRange;
    bool m_DenieProxy;
    bool m_LiveGames;
    uint32_t m_MinPlayerAutoEnd;
    uint32_t m_MaxAllowedSpread;
    bool m_EarlyEnd;
    uint32_t m_LastCommandListTime;			// GetTime when last refreshed command list
    vector<string> m_ColoredNames;
    vector<string> m_Modes;
    CCallableFlameList *m_CallableFlameList;
    uint32_t m_LastFlameListUpdate;
    CCallableForcedGProxyList *m_CallableForcedGProxyList;
    CCallableAliasList *m_CallableAliasList;
    uint32_t m_LastGProxyListUpdate;
    vector<string> m_GProxyList;
    uint32_t m_LastAliasListUpdate;
    uint32_t m_LastDNListUpdate;
    vector<string> m_Flames;
    CCallableAnnounceList *m_CallableAnnounceList;
    uint32_t m_LastAnnounceListUpdate;
    uint32_t m_AnnounceLines;
    vector<string> m_Announces;
    CCallableDCountryList *m_CallableDCountryList;
    uint32_t m_LastDCountryUpdate;
    vector<string> m_DCountries;
    CCallableCommandList *m_CallableCommandList;			// threaded database command list in progress
    CCallableGameDBInit *m_CallableHC;
    CCallableDeniedNamesList *m_CallableDeniedNamesList;
    vector<string> m_DeniedNamePartials;


    bool m_AllGamesFinished;				// if all games finished (used when exiting nicely)
    uint32_t m_AllGamesFinishedTime;		// GetTime when all games finished (used when exiting nicely)

    string m_BindAddress;					// config value: the address to host games on
    uint16_t m_HostPort;					// config value: the port to host games on
    uint16_t m_ReconnectPort;				// config value: the port to listen for GProxy++ reliable reconnects on
    uint32_t m_ReconnectWaitTime;			// config value: the maximum number of minutes to wait for a GProxy++ reliable reconnect

    uint32_t m_MaxGames;					// config value: maximum number of games in progress
    char m_CommandTrigger;					// config value: the command trigger inside games
    string m_ColoredNamePath;
    bool m_ColoredNameHide;

	string m_GameLogFilePath;
    bool m_GameLogging;
    uint32_t m_GameLoggingID;
    string m_SaveGamePath;					// config value: savegame path

	bool m_GameIDReplays;					// config value: save replays with database game id or not
    bool m_SaveReplays;						// config value: save replays
    string m_ReplayPath;					// config value: replay path
	uint32_t m_ReplayWar3Version;			// config value: replay warcraft 3 version (for saving replays)
	uint32_t m_ReplayBuildNumber;			// config value: replay build number (for saving replays)


    string m_VirtualHostName;				// config value: virtual host name
    bool m_HideIPAddresses;					// config value: hide IP addresses from players
    bool m_CheckMultipleIPUsage;			// config value: check for multiple IP address usage
    uint32_t m_SpoofChecks;					// config value: do automatic spoof checks or not
    bool m_RequireSpoofChecks;				// config value: require spoof checks or not
    bool m_ReserveAdmins;					// config value: consider admins to be reserved players or not
    bool m_RefreshMessages;					// config value: display refresh messages or not (by default)
    bool m_AutoLock;						// config value: auto lock games when the owner is present
    bool m_AutoSave;						// config value: auto save before someone disconnects
    uint32_t m_AllowDownloads;				// config value: allow map downloads or not
    bool m_PingDuringDownloads;				// config value: ping during map downloads or not
    uint32_t m_MaxDownloaders;				// config value: maximum number of map downloaders at the same time
    uint32_t m_MaxDownloadSpeed;			// config value: maximum total map download speed in KB/sec
    bool m_LCPings;							// config value: use LC style pings (divide actual pings by two)
    uint32_t m_AutoKickPing;				// config value: auto kick players with ping higher than this
    uint32_t m_BanMethod;					// config value: ban method (ban by name/ip/both)
    uint32_t m_LobbyTimeLimit;				// config value: auto close the game lobby after this many minutes without any reserved players
    uint32_t m_Latency;						// config value: the latency (by default)
    uint32_t m_SyncLimit;					// config value: the maximum number of packets a player can fall out of sync before starting the lag screen (by default)
    bool m_VoteKickAllowed;					// config value: if votekicks are allowed or not
    uint32_t m_VoteKickPercentage;			// config value: percentage of players required to vote yes for a votekick to pass

    bool m_LocalAdminMessages;				// config value: send local admin messages or not
    bool m_AdminGameCreate;					// config value: create the admin game or not
    uint16_t m_AdminGamePort;				// config value: the port to host the admin game on
    string m_AdminGamePassword;				// config value: the admin game password
    string m_AdminGameMap;					// config value: the admin game map config to use


    bool m_TCPNoDelay;						// config value: use Nagle's algorithm or not
    uint32_t m_MatchMakingMethod;			// config value: the matchmaking method
    uint32_t m_MapGameType;                 // config value: the MapGameType overwrite (aka: refresh hack)
    uint32_t m_AutoMuteSpammer;				// config value: auto mute spammers?

    uint32_t m_FinishedGames;
    uint32_t m_CheckForFinishedGames;
    uint32_t m_MinimumLeaverKills;
    uint32_t m_MinimumLeaverDeaths;
    uint32_t m_MinimumLeaverAssists;
    uint32_t m_DeathsByLeaverReduction;

    bool m_StatsUpdate;
    bool m_MessageSystem;
    bool m_FunCommands;
    bool m_BetSystem;
    bool m_AccountProtection;
    bool m_Announce;
    bool m_AnnounceHidden;
    bool m_FountainFarmWarning;
    string m_FountainFarmMessage;
    bool m_AutoDenyUsers;
    bool m_AllowVoteStart;
    uint32_t m_VoteStartMinPlayers;
    bool m_FlameCheck;
    string m_BotManagerName;
    bool m_IngameVoteKick;
    uint32_t m_LeaverAutoBanTime;
    uint32_t m_FirstFlameBanTime;
    uint32_t m_SecondFlameBanTime;
    uint32_t m_SpamBanTime;
    uint32_t m_VKAbuseBanTime;
    bool m_VoteMuting;
    uint32_t m_VoteMuteTime;
    uint32_t m_AutoEndTime;
    bool m_AllowHighPingSafeDrop;
    uint32_t m_MinPauseLevel;
    uint32_t m_MinScoreLimit;
    uint32_t m_MaxScoreLimit;
    bool m_AutobanAll;
    string m_WC3ConnectAlias;
    uint32_t m_LastHCUpdate;
    vector<string> m_Insults;
    bool m_ChannelBotOnly;
    vector<string> m_LanRoomName;
    string m_NonAllowedDonwloadMessage;
    vector<string> m_Aliases;
    bool m_VoteMode;
    uint32_t m_MaxVotingTime;
    bool m_RandomMode;
    bool m_HideMessages;
    bool m_DenieCountriesOnThisBot;
    bool m_KickSlowDownloader;

    bool m_VirtualLobby;
    uint32_t m_VirtualLobbyTime;
    string m_CustomVirtualLobbyInfoBanText;

    bool m_SimpleAFKScript;
    uint32_t m_APMAllowedMinimum;
    uint32_t m_APMMaxAfkWarnings;
    string m_Website;
    uint32_t m_DisconnectAutoBanTime;
    vector<cachedPlayer> m_PlayerCache;
    uint32_t m_BroadCastPort;

    bool isCreated;
    uint32_t m_StartTicks;
    uint32_t m_EndTicks;
    uint32_t m_MaxTicks;
    uint32_t m_MinTicks;
    float m_AVGTicks;
    uint32_t m_TicksCollection;
    uint32_t m_TicksCollectionTimer;
    uint32_t m_Sampler;
    string m_SpoofPattern;
    uint32_t m_DelayGameLoaded;
    bool m_FountainFarmDetection;
    bool m_AutokickSpoofer;
    bool m_PVPGNMode;
    uint32_t m_DenyLimit;
    uint32_t m_SwapLimit;
    vector<GProxyReconnector *> m_PendingReconnects;
    boost::mutex m_ReconnectMutex;
    bool m_SendAutoStartInfo;
    bool m_FountainFarmBan;
    uint32_t m_GarenaPort;
    bool m_RejectingGameCheats;

    COHBot( CConfig *CFG );
    ~COHBot( );

    // processing functions

    bool Update( long usecBlock );

    // events

    void EventBNETConnecting( CBNET *bnet );
    void EventBNETConnected( CBNET *bnet );
    void EventBNETDisconnected( CBNET *bnet );
    void EventBNETLoggedIn( CBNET *bnet );
    void EventBNETGameRefreshed( CBNET *bnet );
    void EventBNETGameRefreshFailed( CBNET *bnet );
    void EventBNETConnectTimedOut( CBNET *bnet );
    void EventBNETWhisper( CBNET *bnet, string user, string message );
    void EventBNETChat( CBNET *bnet, string user, string message );
    void EventBNETEmote( CBNET *bnet, string user, string message );
    void EventGameDeleted( CBaseGame *game );

    // other functions

	bool IsMode( uint8_t gMode );
	void SetMode( uint8_t gMode, bool bEnable );
	void SetMode( uint8_t gMode ){ SetMode( gMode, true ); };

    void ReloadConfigs( );
    void SetConfigs( CConfig *CFG );
    void ExtractScripts( );
    void CreateGame( CMap *map, unsigned char gameState, bool saveGame, string gameName, string ownerName, string creatorName, string creatorServer, uint32_t gameType, bool whisper, uint32_t m_HostCounter );
    bool FlameCheck( string message );
    void GetDeniedCountries( );
    void LoadRules( );
    void LoadRanks( );
    virtual uint32_t GetNewHostCounter( );
    void LoadInsult( );
    virtual string GetTimeFunction( uint32_t type );
    void ReadRoomData();
    string GetAliasName( uint32_t alias );
    string GetRoomName(string RoomID);
    uint32_t GetStatsAliasNumber( string alias );
    string GetLODMode( string fullmode );
    bool IsForcedGProxy(string input );
    bool FindHackFiles( string input );
    bool PlayerCached( string playername );
    void LoadLanguages( );
    bool CanAccessCommand( string name, string command );
    void HandleRCONCommand( string command );
private:
	void SendMessageToBNET( string message, string server, string user, bool whisper);

public:
        static void RegisterPythonClass( );
};

struct translationTree {
    string suffix;
    CLanguage *m_Translation;
};

struct permission {
    string player;
    uint32_t level;
    string binaryPermissions;
    string coloredName;
};

struct cachedPlayer {
    string name;
    uint32_t time;
};
#endif
