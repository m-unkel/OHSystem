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

#ifndef GAMEPLAYER_H
#define GAMEPLAYER_H

class CTCPSocket;
class CCommandPacket;
class CGameProtocol;
class CGame;
class CIncomingJoinPlayer;
class CIncomingGarenaUser;

//
// CPotentialPlayer
//

class CPotentialPlayer
{
public:
    CGameProtocol *m_Protocol;
    CBaseGame *m_Game;

protected:
    // note: we permit m_Socket to be NULL in this class to allow for the virtual host player which doesn't really exist
    // it also allows us to convert CPotentialPlayers to CGamePlayers without the CPotentialPlayer's destructor closing the socket

    CTCPSocket *m_Socket;
    queue<CCommandPacket *> m_Packets;
    bool m_DeleteMe;
    bool m_Error;
    string m_ErrorString;
    CIncomingJoinPlayer *m_IncomingJoinPlayer;
    CIncomingGarenaUser *m_IncomingGarenaUser;
    string m_RoomName;
    bool m_Banned;
	string m_CachedIP;

public:
    CPotentialPlayer( CGameProtocol *nProtocol, CBaseGame *nGame, CTCPSocket *nSocket );
    virtual ~CPotentialPlayer( );

    virtual CTCPSocket *GetSocket( )				{
        return m_Socket;
    }
    virtual BYTEARRAY GetExternalIP( );
    virtual string GetExternalIPString( );
    virtual queue<CCommandPacket *> GetPackets( )	{
        return m_Packets;
    }
    virtual bool GetDeleteMe( )						{
        return m_DeleteMe;
    }
    virtual bool GetError( )						{
        return m_Error;
    }
    virtual string GetErrorString( )				{
        return m_ErrorString;
    }
    virtual CIncomingJoinPlayer *GetJoinPlayer( )	{
        return m_IncomingJoinPlayer;
    }
    virtual CIncomingGarenaUser *GetGarenaUser( )	{
        return m_IncomingGarenaUser;
    }
    virtual BYTEARRAY GetGarenaIP( );
    string GetRoomName( )                                           {
        return m_RoomName;
    }

    virtual void SetSocket( CTCPSocket *nSocket )	{
        m_Socket = nSocket;
    }
    virtual void SetDeleteMe( bool nDeleteMe )		{
        m_DeleteMe = nDeleteMe;
    }
    virtual void SetGarenaUser( CIncomingGarenaUser *nIncomingGarenaUser ) {
        m_IncomingGarenaUser = nIncomingGarenaUser;
    }
    virtual void SetBanned( )            {
        m_Banned = true;
    }

    // processing functions

    virtual bool Update( void *fd );
    virtual void ExtractPackets( );
    virtual void ProcessPackets( );

    // other functions

    virtual void Send( BYTEARRAY data );

public:
	static void RegisterPythonClass( );
};

//
// CGamePlayer
//

class CGamePlayer : public CPotentialPlayer
{
private:
    unsigned char m_PID;
    uint32_t m_Count;
    string m_Name;								// the player's name
    BYTEARRAY m_InternalIP;						// the player's internal IP address as reported by the player when connecting
    vector<uint32_t> m_Pings;					// store the last few (20) pings received so we can take an average
    queue<uint32_t> m_CheckSums;				// the last few checksums the player has sent (for detecting desyncs)
    string m_LeftReason;						// the reason the player left the game
    string m_SpoofedRealm;						// the realm the player last spoof checked on
    string m_JoinedRealm;						// the realm the player joined on (probable, can be spoofed)
    uint32_t m_TotalPacketsSent;
    uint32_t m_TotalPacketsReceived;
	uint32_t m_Left;
    uint32_t m_LeftCode;						// the code to be sent in W3GS_PLAYERLEAVE_OTHERS for why this player left the game
    uint32_t m_LoginAttempts;					// the number of attempts to login (used with CAdminGame only)
    uint32_t m_SyncCounter;						// the number of keepalive packets received from this player
    uint32_t m_JoinTime;						// GetTime when the player joined the game (used to delay sending the /whois a few seconds to allow for some lag)
    uint32_t m_LastMapPartSent;					// the last mappart sent to the player (for sending more than one part at a time)
    uint32_t m_LastMapPartAcked;				// the last mappart acknowledged by the player
    uint32_t m_StartedDownloadingTicks;			// GetTicks when the player started downloading the map
    uint32_t m_FinishedDownloadingTime;			// GetTime when the player finished downloading the map
    uint32_t m_FinishedLoadingTicks;			// GetTicks when the player finished loading the game
    uint32_t m_StartedLaggingTicks;				// GetTicks when the player started lagging
    uint32_t m_StatsSentTime;					// GetTime when we sent this player's stats to the chat (to prevent players from spamming !stats)
    uint32_t m_StatsDotASentTime;				// GetTime when we sent this player's dota stats to the chat (to prevent players from spamming !statsdota)
    uint32_t m_LastGProxyWaitNoticeSentTime;
    queue<BYTEARRAY> m_LoadInGameData;			// queued data to be sent when the player finishes loading when using "load in game"
    double m_Score;								// the player's generic "score" for the matchmaking algorithm
    double m_WinPerc;							// the player's generic "winperc" for the new matchmaking algorithm
    double m_LeavePerc;
    uint32_t m_Games;
    bool m_LoggedIn;							// if the player has logged in or not (used with CAdminGame only)
    bool m_Spoofed;								// if the player has spoof checked or not
    bool m_PasswordProt;
    bool m_Registered;
    bool m_Locked;
    bool m_Reserved;							// if the player is reserved (VIP) or not
    bool m_WhoisShouldBeSent;					// if a battle.net /whois should be sent for this player or not
    bool m_WhoisSent;							// if we've sent a battle.net /whois for this player yet (for spoof checking)
    bool m_DownloadAllowed;						// if we're allowed to download the map or not (used with permission based map downloads)
    bool m_DownloadStarted;						// if we've started downloading the map or not
    bool m_DownloadFinished;					// if we've finished downloading the map or not
    bool m_FinishedLoading;						// if the player has finished loading or not
    bool m_Lagging;								// if the player is lagging or not (on the lag screen)
    bool m_DropVote;							// if the player voted to drop the laggers or not (on the lag screen)
    bool m_KickVote;							// if the player voted to kick a player or not
    bool m_ForfeitVote;
    bool m_DrawVote;
    bool m_Muted;								// if the player is muted or not
    bool m_Autoban;
    uint32_t m_Cookies;
    vector<uint32_t> m_MuteMessages;			// times player sent messages to determine if we should automute
    uint32_t m_MutedTicks;
    vector<uint32_t> m_FlameMessages;			// times player sent messages to determine flame level
    bool m_MutedAuto;
    bool m_LeftMessageSent;						// if the playerleave message has been sent or not
    bool m_GProxy;								// if the player is using GProxy++
    bool m_GProxyDisconnectNoticeSent;			// if a disconnection notice has been sent or not when using GProxy++
    queue<BYTEARRAY> m_GProxyBuffer;
    uint32_t m_GProxyReconnectKey;
    uint32_t m_LastGProxyAckTime;
    uint32_t m_TimeActive;           // AFK detection
    bool m_UsedPause;
    uint32_t m_PauseTried;
    string m_CLetter;
    string m_Country;
    bool m_Silence;
    bool m_HasLeft;
    bool m_AFKMarked;
    bool m_SafeDrop;
    uint32_t m_FeedLevel;
    uint32_t m_VKTimes;
    uint32_t m_HighPingTimes;
    uint32_t m_AnnounceTime;
    uint32_t m_Level;
    string m_LevelName;
    bool m_StartVoted;
    bool m_GlobalChatMuted;
    string m_InsultM;
    uint32_t m_DownloadTicks;
    uint32_t m_Checked;
    bool m_VotedForInterruption;
    uint32_t m_VotedMode;
    bool m_NoLag;                // if the player wants to ignore lag screen
    uint32_t m_ActionCounter;
    uint32_t m_FirstPartOfMinute;
    uint32_t m_SecondPartOfMinute;
    uint32_t m_ThirdPartOfMinute;
    uint32_t m_FourthPartOfMinute;
    uint32_t m_FifthPartOfMinute;
    uint32_t m_SixthPartOfMinute;
    uint32_t m_AFKWarnings;
    uint32_t m_LastAfkWarn;
    uint32_t m_EXP;
    uint32_t m_ID;
    bool m_VotedForBalance;
    double m_Reputation;
    string m_PlayerLanguage;
    uint32_t m_TheThingAmount;
    string m_TheThing;
    uint32_t m_TheThingType;
    uint32_t m_PenalityLevel;
    bool m_RequestedSwap;
    uint8_t m_SwapTarget;
    uint32_t m_LastAttackCommandToFountain;
    uint32_t m_FFLevel;
    uint32_t m_DenyLimit;
    uint32_t m_SwapLimit;

public:
    CGamePlayer( CGameProtocol *nProtocol, CBaseGame *nGame, CTCPSocket *nSocket, unsigned char nPID, string nJoinedRealm, string nName, BYTEARRAY nInternalIP, bool nReserved );
    CGamePlayer( CPotentialPlayer *potential, unsigned char nPID, string nJoinedRealm, string nName, BYTEARRAY nInternalIP, bool nReserved );
    virtual ~CGamePlayer( );

    vector<string> m_IgnoreList;                            // list of usernames this player is ignoring
    unsigned char GetPID( )						{
        return m_PID;
    }
    string GetName( )							{
        return m_Name;
    }
    BYTEARRAY GetInternalIP( )					{
        return m_InternalIP;
    }
    unsigned int GetNumPings( )					{
        return m_Pings.size( );
    }
    unsigned int GetNumCheckSums( )				{
        return m_CheckSums.size( );
    }
    queue<uint32_t> *GetCheckSums( )			{
        return &m_CheckSums;
    }
    string GetLeftReason( )						{
        return m_LeftReason;
    }
    string GetSpoofedRealm( )					{
        return m_SpoofedRealm;
    }
    string GetJoinedRealm( )					{
        return m_JoinedRealm;
    }
    uint32_t GetLeftCode( )						{
        return m_LeftCode;
    }
    uint32_t GetLoginAttempts( )				{
        return m_LoginAttempts;
    }
    uint32_t GetSyncCounter( )					{
        return m_SyncCounter;
    }
    uint32_t GetJoinTime( )						{
        return m_JoinTime;
    }
    uint32_t GetLastMapPartSent( )				{
        return m_LastMapPartSent;
    }
    uint32_t GetLastMapPartAcked( )				{
        return m_LastMapPartAcked;
    }
    uint32_t GetStartedDownloadingTicks( )		{
        return m_StartedDownloadingTicks;
    }
    uint32_t GetFinishedDownloadingTime( )		{
        return m_FinishedDownloadingTime;
    }
    uint32_t GetFinishedLoadingTicks( )			{
        return m_FinishedLoadingTicks;
    }
    uint32_t GetStartedLaggingTicks( )			{
        return m_StartedLaggingTicks;
    }
    uint32_t GetStatsSentTime( )				{
        return m_StatsSentTime;
    }
    uint32_t GetStatsDotASentTime( )			{
        return m_StatsDotASentTime;
    }
    uint32_t GetLastGProxyWaitNoticeSentTime( )	{
        return m_LastGProxyWaitNoticeSentTime;
    }
    uint32_t GetLeftTime( )                     {
        return m_Left;
    }
    uint32_t GetPenalityLevel( )                  {
        return m_PenalityLevel;
    }

    queue<BYTEARRAY> *GetLoadInGameData( )		{
        return &m_LoadInGameData;
    }
    double GetScore( )							{
        return m_Score;
    }
    double GetWinPerc( )							{
        return m_WinPerc;
    }
    double GetLeavePerc( )							{
        return m_LeavePerc;
    }
    uint32_t GetGames( )							{
        return m_Games;
    }
    bool GetLoggedIn( )							{
        return m_LoggedIn;
    }
    bool GetSpoofed( )							{
        return m_Spoofed;
    }
    bool GetPasswordProt( )							{
        return m_PasswordProt;
    }
    bool GetRegistered( )							{
        return m_Registered;
    }
    bool GetLocked( )                                                 {
        return m_Locked;
    }
    bool GetReserved( )							{
        return m_Reserved;
    }
    bool GetWhoisShouldBeSent( )				{
        return m_WhoisShouldBeSent;
    }
    bool GetWhoisSent( )						{
        return m_WhoisSent;
    }
    bool GetDownloadAllowed( )					{
        return m_DownloadAllowed;
    }
    bool GetDownloadStarted( )					{
        return m_DownloadStarted;
    }
    bool GetDownloadFinished( )					{
        return m_DownloadFinished;
    }
    bool GetFinishedLoading( )					{
        return m_FinishedLoading;
    }
    bool GetLagging( )							{
        return m_Lagging;
    }
    bool GetDropVote( )							{
        return m_DropVote;
    }
    bool GetKickVote( )							{
        return m_KickVote;
    }
    bool GetForfeitVote( )						{
        return m_ForfeitVote;
    }
    bool GetDrawVote( )							{
        return m_DrawVote;
    }
    bool GetMuted( )							{
        return m_Muted;
    }
    bool GetAutoban( )   					           {
        return m_Autoban;
    }
    uint32_t GetCookies( )							{
        return m_Cookies;
    }
    bool GetLeftMessageSent( )					{
        return m_LeftMessageSent;
    }
    bool GetGProxy( )							{
        return m_GProxy;
    }
    bool GetGProxyDisconnectNoticeSent( )		{
        return m_GProxyDisconnectNoticeSent;
    }
    uint32_t GetGProxyReconnectKey( )			{
        return m_GProxyReconnectKey;
    }
    bool GetUsedPause( )					{
        return m_UsedPause;
    }
    uint32_t GetPauseTried( )				{
        return m_PauseTried;
    }
    string GetCLetter( )					{
        return m_CLetter;
    }
    string GetCountry( )					{
        return m_Country;
    }
    bool GetSilence( )                                              {
        return m_Silence;
    }
    bool GetLeft( )							{
        return m_HasLeft;
    }
    bool GetAFKMarked( )						{
        return m_AFKMarked;
    }
    bool GetSafeDrop( )						{
        return m_SafeDrop;
    }
    uint32_t GetFeedLevel( )					{
        return m_FeedLevel;
    }
    uint32_t GetVKTimes( )						{
        return m_VKTimes;
    }
    uint32_t GetHighPingTimes( )					{
        return m_HighPingTimes;
    }
    uint32_t GetAnnounceTime( )					{
        return m_AnnounceTime;
    }
    uint32_t GetLevel( )                                            {
        return m_Level;
    }
    string GetLevelName( )                                          {
        return m_LevelName;
    }
    bool GetStartVote( )                                            {
        return m_StartVoted;
    }
    bool GetGlobalChatMuted( )                                      {
        return m_GlobalChatMuted;
    }
    string GetInsultM( )                                            {
        return m_InsultM;
    }
    uint32_t GetDownloadTicks( )                                    {
        return m_DownloadTicks;
    }
    uint32_t GetChecked( )                                              {
        return m_Checked;
    }
    bool GetVotedForInterruption( )                                        {
        return m_VotedForInterruption;
    }
    uint32_t GetVotedMode( )                                                {
        return m_VotedMode;
    }
    bool GetNoLag( )                                                        {
        return m_NoLag;
    }
    uint32_t GetActions( )                      {
        return m_ActionCounter;
    }
    uint32_t GetFirstActionsForFirstPart( )     {
        return m_FirstPartOfMinute;
    }
    uint32_t GetFirstActionsForSecondPart( )     {
        return m_SecondPartOfMinute;
    }
    uint32_t GetFirstActionsForThirdPart( )     {
        return m_ThirdPartOfMinute;
    }
    uint32_t GetFirstActionsForFourthPart( )     {
        return m_FourthPartOfMinute;
    }
    uint32_t GetFirstActionsForFifthPart( )     {
        return m_FifthPartOfMinute;
    }
    uint32_t GetFirstActionsForSixthPart( )     {
        return m_SixthPartOfMinute;
    }
    uint32_t GetAFKWarnings( )                  {
        return m_AFKWarnings;
    }
    uint32_t GetLastAFKWarn( )                   {
        return m_LastAfkWarn;
    }
    uint32_t GetEXP( ) {
        return m_EXP;
    }
    uint32_t GetID( ) {
        return m_ID;
    }
    bool GetVotedForBalance( ) {
        return m_VotedForBalance;
    }
    double GetReputation( ) {
        return m_Reputation;
    }
	void SetLeftTime( uint32_t nLeft)												{
		m_Left = nLeft;
	}
    string GetPlayerLanguage( ) {
        return m_PlayerLanguage;
    }
    uint32_t GetTheThingAmount( ) {
        return m_TheThingAmount;
    }
    string GetTheThing( ) {
        return m_TheThing;
    }
    uint32_t GetTheThingType( ) {
        return m_TheThingType;
    }
    bool GetSwapRequested( ) {
        return m_RequestedSwap;
    }
    uint8_t GetSwapTarget( ) {
        return m_SwapTarget;
    }
    uint32_t GetLastAttackTimeToFountain( ) {
        return m_LastAttackCommandToFountain;
    }
    uint32_t GetFFLevel( ) {
        return m_FFLevel;
    }
    uint32_t GetDenyLimit( ) {
        return m_DenyLimit;
    }
    uint32_t GetSwapLimit( ) {
        return m_SwapLimit;
    }
    void SetDenyLimit( ) {
	m_DenyLimit--;
    }
    void SetSwapLimit( ) {
	m_SwapLimit--;
    }
    void SetLeftReason( string nLeftReason )										{
        m_LeftReason = nLeftReason;
    }
    void SetSpoofedRealm( string nSpoofedRealm )									{
        m_SpoofedRealm = nSpoofedRealm;
    }
    void SetLeftCode( uint32_t nLeftCode )											{
        m_LeftCode = nLeftCode;
    }
    void SetLoginAttempts( uint32_t nLoginAttempts )								{
        m_LoginAttempts = nLoginAttempts;
    }
    void SetSyncCounter( uint32_t nSyncCounter )									{
        m_SyncCounter = nSyncCounter;
    }
    void SetLastMapPartSent( uint32_t nLastMapPartSent )							{
        m_LastMapPartSent = nLastMapPartSent;
    }
    void SetLastMapPartAcked( uint32_t nLastMapPartAcked )							{
        m_LastMapPartAcked = nLastMapPartAcked;
    }
    void SetStartedDownloadingTicks( uint32_t nStartedDownloadingTicks )			{
        m_StartedDownloadingTicks = nStartedDownloadingTicks;
    }
    void SetFinishedDownloadingTime( uint32_t nFinishedDownloadingTime )			{
        m_FinishedDownloadingTime = nFinishedDownloadingTime;
    }
    void SetStartedLaggingTicks( uint32_t nStartedLaggingTicks )					{
        m_StartedLaggingTicks = nStartedLaggingTicks;
    }
    void SetStatsSentTime( uint32_t nStatsSentTime )								{
        m_StatsSentTime = nStatsSentTime;
    }
    void SetStatsDotASentTime( uint32_t nStatsDotASentTime )						{
        m_StatsDotASentTime = nStatsDotASentTime;
    }
    void SetLastGProxyWaitNoticeSentTime( uint32_t nLastGProxyWaitNoticeSentTime )	{
        m_LastGProxyWaitNoticeSentTime = nLastGProxyWaitNoticeSentTime;
    }
    void SetScore( double nScore )													{
        m_Score = nScore;
    }
    void SetWinPerc( double nWinPerc )                                                                                                  {
        m_WinPerc = nWinPerc;
    }
    void SetLeavePerc( double nLeavePerc )												{
        m_LeavePerc = nLeavePerc;
    }
    void SetGames( uint32_t nGames )												{
        m_Games = nGames;
    }
    void SetLoggedIn( bool nLoggedIn )												{
        m_LoggedIn = nLoggedIn;
    }
    void SetSpoofed( bool nSpoofed )												{
        m_Spoofed = nSpoofed;
    }
    void SetPasswordProt( bool nPasswordProt )                                                                                                {
        m_PasswordProt = nPasswordProt;
    }
    void SetRegistered( bool nRegistered )												{
        m_Registered = nRegistered;
    }
    void SetLocked( bool nLocked )                                                                                                {
        m_Locked = nLocked;
    }
    void SetReserved( bool nReserved )												{
        m_Reserved = nReserved;
    }
    void SetWhoisShouldBeSent( bool nWhoisShouldBeSent )							{
        m_WhoisShouldBeSent = nWhoisShouldBeSent;
    }
    void SetDownloadAllowed( bool nDownloadAllowed )								{
        m_DownloadAllowed = nDownloadAllowed;
    }
    void SetDownloadStarted( bool nDownloadStarted )								{
        m_DownloadStarted = nDownloadStarted;
    }
    void SetDownloadFinished( bool nDownloadFinished )								{
        m_DownloadFinished = nDownloadFinished;
    }
    void SetLagging( bool nLagging )												{
        m_Lagging = nLagging;
    }
    void SetDropVote( bool nDropVote )												{
        m_DropVote = nDropVote;
    }
    void SetKickVote( bool nKickVote )												{
        m_KickVote = nKickVote;
    }
    void SetForfeitVote( bool nForfeitVote )										{
        m_ForfeitVote = nForfeitVote;
    }
    void SetDrawVote( bool nDrawVote )												{
        m_DrawVote = nDrawVote;
    }
    void SetMuted( bool nMuted )													{
        m_Muted = nMuted;
        m_MutedTicks = GetTicks( );
        m_MutedAuto = false;
    }
    void SetAutoban( bool nAutoban )                        								{
        m_Autoban = nAutoban;
    }
    void SetCookie( uint32_t nCookies )											{
        m_Cookies = nCookies;
    }
    void SetLeftMessageSent( bool nLeftMessageSent )								{
        m_LeftMessageSent = nLeftMessageSent;
    }
    void SetGProxyDisconnectNoticeSent( bool nGProxyDisconnectNoticeSent )			{
        m_GProxyDisconnectNoticeSent = nGProxyDisconnectNoticeSent;
    }
    void SetUsedPause( bool nUsedPause )                    	        	        		{
        m_UsedPause = nUsedPause;
    }
    void SetPauseTried( )			        	        	               {
        m_PauseTried += 1;
    }
    void SetCLetter( string nCLetter )							{
        m_CLetter = nCLetter;
    }
    void SetCountry( string nCountry )                                                      {
        m_Country = nCountry;
    }
    void SetSilence( bool nSilence )                                                                                {
        m_Silence = nSilence;
    }
    void SetLeft( bool nHasLeft )											{
        m_HasLeft = nHasLeft;
    }
    void SetAFKMarked( bool nAFKMarked )										{
        m_AFKMarked = nAFKMarked;
    }
    void SetSafeDrop( bool nSafeDrop )										{
        m_SafeDrop = nSafeDrop;
    }
    void SetFeedLevel( uint32_t nFeedLevel )									{
        m_FeedLevel = nFeedLevel;
    }
    void SetVKTimes( )												{
        m_VKTimes += 1;
    }
    void SetHighPingTimes( )											{
        m_HighPingTimes += 1;
    }
    void SetAnnounceTime( )											{
        m_AnnounceTime = GetTime();
    }
    void SetLevel( uint32_t nLevel )                                                                        {
        m_Level = nLevel;
    }
    void SetLevelName( string nLevelName )                                                                    {
        m_LevelName = nLevelName;
    }
    void SetStartVote( bool nVoted )                                                                        {
        m_StartVoted = nVoted;
    }
    string GetNameTerminated( );
    uint32_t GetPing( bool LCPing );
    bool GetIsIgnoring( string username );
    void Ignore( string username );
    void UnIgnore( string username );
    void SetGlobalChatMuted( bool nGlobalChatMuted )                                                             {
        m_GlobalChatMuted = nGlobalChatMuted;
    }

    void SetPenalityLevel( uint32_t nLevel ) {
        m_PenalityLevel = nLevel;
    }

    void SetInsultM( string nInsultM )                                                                              {
        m_InsultM = nInsultM;
    }
    void SetDownloadTicks( uint32_t nTicks )                                                                        {
        m_DownloadTicks = nTicks;
    }
    void SetChecked( )                                                                                              {
        m_Checked++;
    }
    void AddLoadInGameData( BYTEARRAY nLoadInGameData )								{
        m_LoadInGameData.push( nLoadInGameData );
    }
    void SetVotedForInterruption ( bool nVoted )                                                                    {
        m_VotedForInterruption = nVoted;
    }
    void SetVotedMode( uint32_t nVotedMode )                                                                        {
        m_VotedMode = nVotedMode;
    }
    void SetNoLag( bool nNoLag )                          {
        m_NoLag = nNoLag;
    }
    void SetEXP( uint32_t nEXP ) {
        m_EXP = nEXP;
    }
    void SetID( uint32_t nID ) {
        m_ID = nID;
    }

    void SetActions( )                      {
        m_ActionCounter++;
    }
    void SetFirstActionsForFirstPart( uint32_t actions )     {
        m_FirstPartOfMinute = actions;
    }
    void SetFirstActionsForSecondPart( uint32_t actions )     {
        m_SecondPartOfMinute = actions;
    }
    void SetFirstActionsForThirdPart( uint32_t actions )     {
        m_ThirdPartOfMinute = actions;
    }
    void SetFirstActionsForFourthPart( uint32_t actions )     {
        m_FourthPartOfMinute = actions;
    }
    void SetFirstActionsForFifthPart( uint32_t actions )     {
        m_FifthPartOfMinute = actions;
    }
    void SetFirstActionsForSixthPart( uint32_t actions )     {
        m_SixthPartOfMinute = actions;
    }
    void SetAFKWarnings( )                                      {
        m_AFKWarnings;
    }
    void SetLastAFKWarning( )                                   {
        m_LastAfkWarn = GetTime( );
    }
    void SetVotedForBalance( ) {
        m_VotedForBalance = true;
    }
    void SetReputation( double nReputation ) {
        m_Reputation = nReputation;
    }
    void SetPlayerLanguage( string nPlayerLanguage ) {
        m_PlayerLanguage = nPlayerLanguage;
    }

    void SetTheThingAmount(uint32_t nTheThingAmount) {
        m_TheThingAmount = nTheThingAmount;
    }
    void SetTheThing( string nTheThing ) {
        m_TheThing = nTheThing;
    }
    void SetTheThingType(uint32_t nType) {
        m_TheThingType = nType;
    }
    void SetSwapRequested( bool nRequestedSwap ) {
        m_RequestedSwap = nRequestedSwap;
    }
    void SetSwapTarget( uint8_t nSwapTarget) {
        m_SwapTarget = nSwapTarget;
    }
    void SetLastAttackCommandToFountain( uint32_t time ) {
        m_LastAttackCommandToFountain = time;
    }
    void SetFFLevel( ) {
        m_FFLevel++;
    }

    // processing functions

    // AFK detection
    uint32_t  GetTimeActive( )     {
        return m_TimeActive;
    }
    void SetTimeActive      ( uint32_t nTimeActive )          {
        m_TimeActive = nTimeActive;
    }

    virtual bool Update( void *fd );
    virtual void ExtractPackets( );
    virtual void ProcessPackets( );

    // other functions

    virtual void Send( BYTEARRAY data );
    virtual void EventGProxyReconnect( CTCPSocket *NewSocket, uint32_t LastPacket );


public:
	static void RegisterPythonClass( );
};

#endif
