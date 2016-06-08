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

#ifndef GHOSTDB_H
#define GHOSTDB_H

//
// COHBotDB
//

class CBaseCallable;
class CCallableRegAdd;
class CCallableStatsSystem;
class CCallablePList;
class CCallableFlameList;
class CCallableForcedGProxyList;
class CCallableAliasList;
class CCallableDeniedNamesList;
class CCallableAnnounceList;
class CCallableDCountryList;
class CCallableBanCount;
class CCallableBanCheck;
class CCallableBanCheck2;
class CCallablePWCheck;
class CCallablePassCheck;
class CCallablepm;
class CCallableStoreLog;
class CCallablegs;
class CCallablepenp;
class CCallableBanAdd;
class CCallablePUp;
class CCallableBanRemove;
class CCallableBanList;
class CCallableTBRemove;
class CCallableCommandList;
class CCallableGameAdd;
class CCallableGameDBInit;
class CCallableGamePlayerAdd;
class CCallableGamePlayerSummaryCheck;
class CCallableStatsPlayerSummaryCheck;
class CCallableInboxSummaryCheck;
class CCallableDotAGameAdd;
class CCallableDotAPlayerAdd;
class CCallableDotAPlayerSummaryCheck;
class CCallableDownloadAdd;
class CCallableScoreCheck;
class CCallableConnectCheck;
class CCallableW3MMDPlayerAdd;
class CCallableW3MMDVarAdd;
class CDBBan;
class CDBGame;
class CDBGamePlayer;
class CDBGamePlayerSummary;
class CDBStatsPlayerSummary;
class CDBInboxSummary;
class CDBDotAPlayerSummary;
class CCallableGameUpdate;
class CCallableBotStatusUpdate;
class CCallableBotStatusCreate;
struct PlayerOfPlayerList;
class CDBDotAPlayer;

typedef pair<uint32_t,string> VarP;

class COHBotDB
{
protected:
    bool m_HasError;
    string m_Error;

public:
    COHBotDB( CConfig *CFG );
    virtual ~COHBotDB( );

    bool HasError( )			{
        return m_HasError;
    }
    string GetError( )			{
        return m_Error;
    }
    virtual string GetStatus( )	{
        return "DB STATUS --- OK";
    }

    virtual void RecoverCallable( CBaseCallable *callable );

    // standard (non-threaded) database functions

    virtual bool Begin( );
    virtual bool Commit( );
    virtual uint32_t RegAdd( string user, string server, string mail, string password, string type );
    virtual string StatsSystem( string user, string input, uint32_t one, string type );
    virtual uint32_t StoreLog( uint32_t chatid, string game, vector<string> admin );
    virtual uint32_t gs( uint32_t chatid, string gn, uint32_t st, uint32_t gametype );
    virtual uint32_t penp( string name, string reason, string admin, uint32_t amount, string type );
    virtual vector<permission> PList( string server );
    virtual vector<string> FlameList( );
    virtual vector<string> ForcedGProxyList( );
    virtual vector<string> AliasList( );
    virtual vector<string> DeniedNamesList( );
    virtual vector<string> AnnounceList( );
    virtual vector<string> DCountryList( );
    virtual uint32_t BanCount( string server );
    virtual CDBBan *BanCheck( string server, string user, string ip );
    virtual string BanCheck2( string server, string user, string type );
    virtual uint32_t PWCheck( string user );
    virtual uint32_t PassCheck( string user, string pass, uint32_t st );
    virtual uint32_t pm( string user, string listener, uint32_t status, string message, string type );
    virtual uint32_t BanAdd( string server, string user, string ip, string gamename, string admin, string reason, uint32_t bantime, string country );
    virtual bool PUp( string name, uint32_t level, string realm, string user );
    virtual bool BanRemove( string server, string user );
    virtual bool TBRemove( string server );
    virtual bool BanRemove( string user );
    virtual vector<CDBBan *> BanList( string server );
    virtual vector<string> CommandList( );
    virtual uint32_t GameAdd( string server, string map, string gamename, string ownername, uint32_t duration, uint32_t gamestate, string creatorname, string creatorserver, uint32_t gametype, uint32_t lobbytime );
    virtual string GameUpdate( uint32_t hostcounter, uint32_t lobby, string map_type, uint32_t duration, string gamename, string ownername, string creatorname, string map, uint32_t players, uint32_t total, vector<PlayerOfPlayerList> playerlist );
    virtual uint32_t GamePlayerAdd( uint32_t gameid, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t reserved, uint32_t loadingtime, uint32_t left, string leftreason, uint32_t team, uint32_t colour, uint32_t id );
    virtual uint32_t GamePlayerCount( string name );
    virtual CDBGamePlayerSummary *GamePlayerSummaryCheck( string name );
    virtual CDBStatsPlayerSummary *StatsPlayerSummaryCheck( string name, string month, string year, uint32_t alias );
    virtual CDBInboxSummary *InboxSummaryCheck( string name );
    virtual uint32_t DotAGameAdd( uint32_t gameid, uint32_t winner, uint32_t min, uint32_t sec );
    virtual uint32_t DotAPlayerAdd( uint32_t gameid, string data );
    virtual uint32_t DotAPlayerCount( string name );
    virtual CDBDotAPlayerSummary *DotAPlayerSummaryCheck( string name );
    virtual bool DownloadAdd( string map, uint32_t mapsize, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t downloadtime );
    virtual CCallableConnectCheck *ThreadedConnectCheck( string name, uint32_t sessionkey );
    virtual uint32_t W3MMDPlayerAdd( string category, uint32_t gameid, uint32_t pid, string name, string flag, uint32_t leaver, uint32_t practicing );
    virtual bool W3MMDVarAdd( uint32_t gameid, map<VarP,int32_t> var_ints );
    virtual bool W3MMDVarAdd( uint32_t gameid, map<VarP,double> var_reals );
    virtual bool W3MMDVarAdd( uint32_t gameid, map<VarP,string> var_strings );
    virtual bool BotStatusCreate( string username, string gamename, string ip, uint16_t hostport, string roc, string tft);
    virtual bool BotStatusUpdate( string server, uint32_t status );

    // threaded database functions

    virtual void CreateThread( CBaseCallable *callable );
    virtual CCallableRegAdd *ThreadedRegAdd( string user, string server, string mail, string password, string type );
    virtual CCallableStatsSystem *ThreadedStatsSystem( string user, string input, uint32_t one, string type );
    virtual CCallablePWCheck *ThreadedPWCheck( string user );
    virtual CCallablePassCheck *ThreadedPassCheck( string user, string pass, uint32_t st );
    virtual CCallablepm *Threadedpm( string user, string listener, uint32_t status, string message, string type );
    virtual CCallablePList *ThreadedPList( string server );
    virtual CCallableFlameList *ThreadedFlameList( );
    virtual CCallableForcedGProxyList *ThreadedForcedGProxyList( );
    virtual CCallableAliasList *ThreadedAliasList( );
    virtual CCallableDeniedNamesList *ThreadedDeniedNamesList( );
    virtual CCallableAnnounceList *ThreadedAnnounceList( );
    virtual CCallableDCountryList *ThreadedDCountryList( );
    virtual CCallableStoreLog *ThreadedStoreLog( uint32_t chatid, string game, vector<string> admin );
    virtual CCallablegs *Threadedgs( uint32_t chatid, string gn, uint32_t st, uint32_t gametype, uint32_t gamealias );
    virtual CCallablepenp *Threadedpenp( string name, string reason, string admin, uint32_t amount, string type );
    virtual CCallableBanCount *ThreadedBanCount( string server );
    virtual CCallableBanCheck *ThreadedBanCheck( string server, string user, string ip );
    virtual CCallableBanCheck2 *ThreadedBanCheck2( string server, string user, string type );
    virtual CCallableBanAdd *ThreadedBanAdd( string server, string user, string ip, string gamename, string admin, string reason, uint32_t bantime, string country );
    virtual CCallablePUp *ThreadedPUp( string name, uint32_t level, string realm, string user );
    virtual CCallableBanRemove *ThreadedBanRemove( string server, string user );
    virtual CCallableBanRemove *ThreadedBanRemove( string user );
    virtual CCallableBanList *ThreadedBanList( string server );
    virtual CCallableTBRemove *ThreadedTBRemove( string server );
    virtual CCallableCommandList *ThreadedCommandList( );
    virtual CCallableGameAdd *ThreadedGameAdd( string server, string map, string gamename, string ownername, uint32_t duration, uint32_t gamestate, string creatorname, string creatorserver, uint32_t gametype, vector<string> lobbylog, vector<string> gamelog, uint32_t databaseid, uint32_t lobbytime );
    virtual CCallableGameDBInit *ThreadedGameDBInit( vector<CDBBan *> players, string gamename, uint32_t gameid, uint32_t gamealias );
    virtual CCallableGameUpdate *ThreadedGameUpdate( uint32_t hostcounter, uint32_t lobby, string map_type, uint32_t duration, string gamename, string ownername, string creatorname, string map, uint32_t players, uint32_t total, vector<PlayerOfPlayerList> playerlist );
    virtual CCallableGamePlayerAdd *ThreadedGamePlayerAdd( uint32_t gameid, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t reserved, uint32_t loadingtime, uint32_t left, string leftreason, uint32_t team, uint32_t colour, uint32_t id );
    virtual CCallableGamePlayerSummaryCheck *ThreadedGamePlayerSummaryCheck( string name );
    virtual CCallableStatsPlayerSummaryCheck *ThreadedStatsPlayerSummaryCheck( string name, string month, string year, uint32_t alias );
    virtual CCallableInboxSummaryCheck *ThreadedInboxSummaryCheck( string name );
    virtual CCallableDotAGameAdd *ThreadedDotAGameAdd( uint32_t gameid, uint32_t winner, uint32_t min, uint32_t sec );
    virtual CCallableDotAPlayerAdd *ThreadedDotAPlayerAdd( uint32_t gameid, string data );
    virtual CCallableDotAPlayerSummaryCheck *ThreadedDotAPlayerSummaryCheck( string name );
    virtual CCallableDownloadAdd *ThreadedDownloadAdd( string map, uint32_t mapsize, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t downloadtime );
    virtual CCallableScoreCheck *ThreadedScoreCheck( string category, string name, string server );
    virtual CCallableW3MMDPlayerAdd *ThreadedW3MMDPlayerAdd( string category, uint32_t gameid, uint32_t pid, string name, string flag, uint32_t leaver, uint32_t practicing );
    virtual CCallableW3MMDVarAdd *ThreadedW3MMDVarAdd( uint32_t gameid, map<VarP,int32_t> var_ints );
    virtual CCallableW3MMDVarAdd *ThreadedW3MMDVarAdd( uint32_t gameid, map<VarP,double> var_reals );
    virtual CCallableW3MMDVarAdd *ThreadedW3MMDVarAdd( uint32_t gameid, map<VarP,string> var_strings );
    virtual CCallableBotStatusCreate *ThreadedBotStatusCreate( string username, string gamename, string ip, uint16_t hostport, string roc, string tft);
    virtual CCallableBotStatusUpdate *ThreadedBotStatusUpdate( string server, uint32_t status);
};

//
// Callables
//

// life cycle of a callable:
//  - the callable is created in one of the database's ThreadedXXX functions
//  - initially the callable is NOT ready (i.e. m_Ready = false)
//  - the ThreadedXXX function normally creates a thread to perform some query and (potentially) store some result in the callable
//  - at the time of this writing all threads are immediately detached, the code does not join any threads (the callable's "readiness" is used for this purpose instead)
//  - when the thread completes it will set m_Ready = true
//  - DO NOT DO *ANYTHING* TO THE CALLABLE UNTIL IT'S READY OR YOU WILL CREATE A CONCURRENCY MESS
//  - THE ONLY SAFE FUNCTION IN THE CALLABLE IS GetReady
//  - when the callable is ready you may access the callable's result which will have been set within the (now terminated) thread

// example usage:
//  - normally you will call a ThreadedXXX function, store the callable in a vector, and periodically check if the callable is ready
//  - when the callable is ready you will consume the result then you will pass the callable back to the database via the RecoverCallable function
//  - the RecoverCallable function allows the database to recover some of the callable's resources to be reused later (e.g. MySQL connections)
//  - note that this will NOT free the callable's memory, you must do that yourself after calling the RecoverCallable function
//  - be careful not to leak any callables, it's NOT safe to delete a callable even if you decide that you don't want the result anymore
//  - you should deliver any to-be-orphaned callables to the main vector in COHBot so they can be properly deleted when ready even if you don't care about the result anymore
//  - e.g. if a player does a stats check immediately before a game is deleted you can't just delete the callable on game deletion unless it's ready

class CBaseCallable
{
protected:
    string m_Error;
    volatile bool m_Ready;
    uint32_t m_StartTicks;
    uint32_t m_EndTicks;

public:
    CBaseCallable( ) : m_Error( ), m_Ready( false ), m_StartTicks( 0 ), m_EndTicks( 0 ) { }
    virtual ~CBaseCallable( ) { }

    virtual void operator( )( ) { }

    virtual void Init( );
    virtual void Close( );

    virtual string GetError( )				{
        return m_Error;
    }
    virtual bool GetReady( )				{
        return m_Ready;
    }
    virtual void SetReady( bool nReady )	{
        m_Ready = nReady;
    }
    virtual uint32_t GetElapsed( )			{
        return m_Ready ? m_EndTicks - m_StartTicks : 0;
    }
};

class CCallableRegAdd : virtual public CBaseCallable
{
protected:
    string m_User;
    string m_Server;
    string m_Mail;
    string m_Password;
    string m_Type;
    uint32_t m_Result;

public:
    CCallableRegAdd( string nUser, string nServer, string nMail, string nPassword, string nType ) : CBaseCallable( ), m_User( nUser ), m_Server( nServer ), m_Mail( nMail ), m_Password( nPassword ), m_Type( nType ), m_Result( 0 ) { }
    virtual ~CCallableRegAdd( );

    virtual string GetServer( )                             {
        return m_Server;
    }
    virtual string GetUser( )                               {
        return m_User;
    }
    virtual uint32_t GetResult( )                               {
        return m_Result;
    }
    virtual void SetResult( uint32_t nResult )  {
        m_Result = nResult;
    }
};

class CCallableStatsSystem : virtual public CBaseCallable
{
protected:
    string m_User;
    string m_Input;
    uint32_t m_One;
    string m_Type;
    string m_Result;
public:
    CCallableStatsSystem( string nUser, string nInput, uint32_t nOne, string nType ) : CBaseCallable( ), m_User( nUser ), m_Input( nInput ), m_One( nOne ), m_Type( nType ), m_Result( "" ) { }
    virtual ~CCallableStatsSystem( );

    virtual string GetUser( )                               {
        return m_User;
    }
    virtual string GetType( )				{
        return m_Type;
    }
    virtual string GetInput( )				{
        return m_Input;
    }
    virtual uint32_t GetOne( )				{
        return m_One;
    }
    virtual string GetResult( )                               {
        return m_Result;
    }
    virtual void SetResult( string nResult )  {
        m_Result = nResult;
    }
};

class CCallablePWCheck : virtual public CBaseCallable
{
protected:
    string m_User;
    uint32_t m_Result;

public:
    CCallablePWCheck( string nUser ) : CBaseCallable( ), m_User( nUser ), m_Result( 0 ) { }
    virtual ~CCallablePWCheck( );

    virtual string GetUser( )                               {
        return m_User;
    }
    virtual uint32_t GetResult( )                               {
        return m_Result;
    }
    virtual void SetResult( uint32_t nResult )  {
        m_Result = nResult;
    }
};

class CCallablePassCheck : virtual public CBaseCallable
{
protected:
    string m_User;
    string m_Pass;
    uint32_t m_ST;
    uint32_t m_Result;

public:
    CCallablePassCheck( string nUser, string nPass, uint32_t nST) : CBaseCallable( ), m_User( nUser ), m_Pass( nPass ), m_ST( nST ), m_Result( 0 ) { }
    virtual ~CCallablePassCheck( );

    virtual uint32_t GetResult( )                               {
        return m_Result;
    }
    virtual void SetResult( uint32_t nResult )  {
        m_Result = nResult;
    }
};

class CCallablepm : virtual public CBaseCallable
{
protected:
    string m_User;
    string m_Listener;
    uint32_t m_Status;
    string m_Message;
    string m_Type;
    uint32_t m_Result;
public:
    CCallablepm( string nUser, string nListener, uint32_t nStatus, string nMessage, string nType ) : CBaseCallable( ), m_User( nUser ), m_Listener( nListener ), m_Status( nStatus ), m_Message( nMessage ), m_Type( nType ), m_Result( 0 ) { }
    virtual ~CCallablepm( );

    virtual uint32_t GetResult( )                               {
        return m_Result;
    }
    virtual void SetResult( uint32_t nResult )  {
        m_Result = nResult;
    }
};

class CCallablePList : virtual public CBaseCallable
{
protected:
    string m_Server;
    vector<permission> m_Result;

public:
    CCallablePList( string nServer ) : CBaseCallable( ), m_Server( nServer ) { }
    virtual ~CCallablePList( );

    virtual vector<permission> GetResult( )                                     {
        return m_Result;
    }
    virtual void SetResult( vector<permission> nResult )        {
        m_Result = nResult;
    }
};

class CCallableFlameList : virtual public CBaseCallable
{
protected:
    vector<string> m_Result;

public:
    CCallableFlameList( ) : CBaseCallable( ) { }
    virtual ~CCallableFlameList( );

    virtual vector<string> GetResult( )                                     {
        return m_Result;
    }
    virtual void SetResult( vector<string> nResult )        {
        m_Result = nResult;
    }
};

class CCallableForcedGProxyList : virtual public CBaseCallable
{
protected:
    vector<string> m_Result;

public:
    CCallableForcedGProxyList( ) : CBaseCallable( ) { }
    virtual ~CCallableForcedGProxyList( );

    virtual vector<string> GetResult( )                                     {
        return m_Result;
    }
    virtual void SetResult( vector<string> nResult )        {
        m_Result = nResult;
    }
};

class CCallableAliasList : virtual public CBaseCallable
{
protected:
    vector<string> m_Result;

public:
    CCallableAliasList( ) : CBaseCallable( ) { }
    virtual ~CCallableAliasList( );

    virtual vector<string> GetResult( )                                     {
        return m_Result;
    }
    virtual void SetResult( vector<string> nResult )        {
        m_Result = nResult;
    }
};

class CCallableDeniedNamesList : virtual public CBaseCallable
{
protected:
    vector<string> m_Result;

public:
    CCallableDeniedNamesList( ) : CBaseCallable( ) { }
    virtual ~CCallableDeniedNamesList( );

    virtual vector<string> GetResult( )                                     {
        return m_Result;
    }
    virtual void SetResult( vector<string> nResult )        {
        m_Result = nResult;
    }
};

class CCallableAnnounceList : virtual public CBaseCallable
{
protected:
    vector<string> m_Result;

public:
    CCallableAnnounceList( ) : CBaseCallable( ) { }
    virtual ~CCallableAnnounceList( );

    virtual vector<string> GetResult( )                                     {
        return m_Result;
    }
    virtual void SetResult( vector<string> nResult )        {
        m_Result = nResult;
    }
};

class CCallableDCountryList : virtual public CBaseCallable
{
protected:
    vector<string> m_Result;

public:
    CCallableDCountryList( ) : CBaseCallable( ) { }
    virtual ~CCallableDCountryList( );

    virtual vector<string> GetResult( )                                     {
        return m_Result;
    }
    virtual void SetResult( vector<string> nResult )        {
        m_Result = nResult;
    }
};

class CCallableStoreLog : virtual public CBaseCallable
{
protected:
    uint32_t m_ChatID;
    string m_Game;
    vector<string> m_Admin;
    uint32_t m_Result;

public:
    CCallableStoreLog( uint32_t nChatID, string nGame, vector<string> nAdmin ) : CBaseCallable( ), m_ChatID( nChatID ), m_Game( nGame ), m_Admin( nAdmin ), m_Result( 0 ) { }
    virtual ~CCallableStoreLog( );
};

class CCallablegs : virtual public CBaseCallable
{
protected:
    uint32_t m_ChatID;
    string m_GN;
    uint32_t m_ST;
    uint32_t m_GameType;
    uint32_t m_GameAlias;
    uint32_t m_Result;

public:
    CCallablegs( uint32_t nChatID, string nGN, uint32_t nST, uint32_t nGameType, uint32_t nGameAlias ) : CBaseCallable( ), m_ChatID( nChatID ), m_GN( nGN ), m_ST( nST ), m_GameType( nGameType ), m_GameAlias( nGameAlias ), m_Result( 0 ) { }
    virtual ~CCallablegs( );
};

class CCallablepenp : virtual public CBaseCallable
{
protected:
    string m_Name;
    string m_Reason;
    string m_Admin;
    uint32_t m_Amount;
    string m_Type;
    uint32_t m_Result;

public:
    CCallablepenp( string nName, string nReason, string nAdmin, uint32_t nAmount, string nType ) : CBaseCallable( ), m_Name( nName ), m_Reason( nReason ), m_Admin( nAdmin ), m_Amount( nAmount ), m_Type( nType ), m_Result( 0 ) { }
    virtual ~CCallablepenp( );

    virtual string GetName( )                                     {
        return m_Name;
    }
    virtual string GetReason( )                                     {
        return m_Reason;
    }
    virtual string GetAdmin( )                                     {
        return m_Admin;
    }
    virtual uint32_t GetAmount( )					{
        return m_Amount;
    }
    virtual string GetType( )					{
        return m_Type;
    }
    virtual uint32_t GetResult( )                           {
        return m_Result;
    }
    virtual void SetResult( uint32_t nResult )      {
        m_Result = nResult;
    }
};


class CCallableBanCount : virtual public CBaseCallable
{
protected:
    string m_Server;
    uint32_t m_Result;

public:
    CCallableBanCount( string nServer ) : CBaseCallable( ), m_Server( nServer ), m_Result( 0 ) { }
    virtual ~CCallableBanCount( );

    virtual string GetServer( )					{
        return m_Server;
    }
    virtual uint32_t GetResult( )				{
        return m_Result;
    }
    virtual void SetResult( uint32_t nResult )	{
        m_Result = nResult;
    }
};

class CCallableBanCheck : virtual public CBaseCallable
{
protected:
    string m_Server;
    string m_User;
    string m_IP;
    CDBBan *m_Result;

public:
    CCallableBanCheck( string nServer, string nUser, string nIP ) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_IP( nIP ), m_Result( NULL ) { }
    virtual ~CCallableBanCheck( );

    virtual string GetServer( )					{
        return m_Server;
    }
    virtual string GetUser( )					{
        return m_User;
    }
    virtual string GetIP( )						{
        return m_IP;
    }
    virtual CDBBan *GetResult( )				{
        return m_Result;
    }
    virtual void SetResult( CDBBan *nResult )	{
        m_Result = nResult;
    }
};

class CCallableBanCheck2 : virtual public CBaseCallable
{
protected:
    string m_Server;
    string m_User;
    string m_Type;
    string m_Result;
public:
    CCallableBanCheck2( string nServer, string nUser, string nType ) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_Type( nType ), m_Result( "" ) { }
    virtual ~CCallableBanCheck2( );

    virtual string GetType( )					{
        return m_Type;
    }
    virtual string GetServer( )                                     {
        return m_Server;
    }
    virtual string GetUser( )                                       {
        return m_User;
    }
    virtual string GetResult( )                           {
        return m_Result;
    }
    virtual void SetResult( string nResult )      {
        m_Result = nResult;
    }
};

class CCallableBanAdd : virtual public CBaseCallable
{
protected:
    string m_Server;
    string m_User;
    string m_IP;
    string m_GameName;
    string m_Admin;
    string m_Reason;
    uint32_t m_BanTime;
    string m_Country;
    uint32_t m_Result;

public:
    CCallableBanAdd( string nServer, string nUser, string nIP, string nGameName, string nAdmin, string nReason, uint32_t nBanTime, string nCountry ) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_IP( nIP ), m_GameName( nGameName ), m_Admin( nAdmin ), m_Reason( nReason ), m_BanTime( nBanTime ), m_Country( nCountry ), m_Result( 0 ) { }
    virtual ~CCallableBanAdd( );

    virtual string GetServer( )				{
        return m_Server;
    }
    virtual string GetUser( )				{
        return m_User;
    }
    virtual string GetIP( )					{
        return m_IP;
    }
    virtual string GetGameName( )			{
        return m_GameName;
    }
    virtual string GetAdmin( )				{
        return m_Admin;
    }
    virtual string GetReason( )				{
        return m_Reason;
    }
    virtual uint32_t GetBanTime( )				{
        return m_BanTime;
    }
    virtual uint32_t GetResult( )				{
        return m_Result;
    }
    virtual void SetResult( uint32_t nResult )	{
        m_Result = nResult;
    }
};

class CCallablePUp : virtual public CBaseCallable
{
protected:
    string m_Name;
    uint32_t m_Level;
    string m_Realm;
    string m_User;
    bool m_Result;

public:
    CCallablePUp( string nName, uint32_t nLevel, string nRealm, string nUser ) : CBaseCallable( ), m_Name( nName ), m_Level( nLevel ), m_Realm( nRealm ), m_User( nUser ), m_Result( false ) { }
    virtual ~CCallablePUp( );

    virtual string GetName( )                             {
        return m_Name;
    }
    virtual uint32_t GetLevel( )				{
        return m_Level;
    }
    virtual string GetRealm( )                             {
        return m_Realm;
    }
    virtual bool GetResult( )                               {
        return m_Result;
    }
    virtual void SetResult( bool nResult )  {
        m_Result = nResult;
    }
};

class CCallableBanRemove : virtual public CBaseCallable
{
protected:
    string m_Server;
    string m_User;
    bool m_Result;

public:
    CCallableBanRemove( string nServer, string nUser ) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_Result( false ) { }
    virtual ~CCallableBanRemove( );

    virtual string GetServer( )				{
        return m_Server;
    }
    virtual string GetUser( )				{
        return m_User;
    }
    virtual bool GetResult( )				{
        return m_Result;
    }
    virtual void SetResult( bool nResult )	{
        m_Result = nResult;
    }
};

class CCallableBanList : virtual public CBaseCallable
{
protected:
    string m_Server;
    vector<CDBBan *> m_Result;

public:
    CCallableBanList( string nServer ) : CBaseCallable( ), m_Server( nServer ) { }
    virtual ~CCallableBanList( );

    virtual vector<CDBBan *> GetResult( )				{
        return m_Result;
    }
    virtual void SetResult( vector<CDBBan *> nResult )	{
        m_Result = nResult;
    }
};

class CCallableTBRemove : virtual public CBaseCallable
{
protected:
    string m_Server;
    bool m_Result;

public:
    CCallableTBRemove( string nServer ) : CBaseCallable( ), m_Server( nServer ), m_Result( false ) { }
    virtual ~CCallableTBRemove( );

    virtual bool GetResult( )                               {
        return m_Result;
    }
    virtual void SetResult( bool nResult )  {
        m_Result = nResult;
    }
};

class CCallableCommandList : virtual public CBaseCallable
{
protected:
    vector<string> m_Result;

public:
    CCallableCommandList( ) : CBaseCallable( ) { }
    virtual ~CCallableCommandList( );

    virtual vector<string> GetResult( )				{
        return m_Result;
    }
    virtual void SetResult( vector<string> nResult )	{
        m_Result = nResult;
    }
};

class CCallableGameAdd : virtual public CBaseCallable
{
protected:
    string m_Server;
    string m_Map;
    string m_GameName;
    string m_OwnerName;
    uint32_t m_Duration;
    uint32_t m_GameState;
    string m_CreatorName;
    string m_CreatorServer;
    uint32_t m_GameType;
    vector<string> m_LobbyLog;
    vector<string> m_GameLog;
    uint32_t m_DatabaseID;
    uint32_t m_LobbyTime;
    uint32_t m_Result;

public:
    CCallableGameAdd( string nServer, string nMap, string nGameName, string nOwnerName, uint32_t nDuration, uint32_t nGameState, string nCreatorName, string nCreatorServer, uint32_t nGameType, vector<string> nLobbyLog, vector<string> nGameLog, uint32_t nDatabaseID, uint32_t nLobbyTime ) : CBaseCallable( ), m_Server( nServer ), m_Map( nMap ), m_GameName( nGameName ), m_OwnerName( nOwnerName ), m_Duration( nDuration ), m_GameState( nGameState ), m_CreatorName( nCreatorName ), m_CreatorServer( nCreatorServer ), m_GameType( nGameType ), m_LobbyLog( nLobbyLog ), m_GameLog( nGameLog ), m_DatabaseID( nDatabaseID ), m_LobbyTime( nLobbyTime ), m_Result( 0 ) { }
    virtual ~CCallableGameAdd( );

    virtual uint32_t GetResult( )				{
        return m_Result;
    }
    virtual void SetResult( uint32_t nResult )	{
        m_Result = nResult;
    }
};

class CCallableGameDBInit : virtual public CBaseCallable
{
protected:
    vector<CDBBan *> m_Players;
    string m_GameName;
    uint32_t m_GameID;
    uint32_t m_GameAlias;
    uint32_t m_Result;

public:
    CCallableGameDBInit( vector<CDBBan *> nPlayers, string nGameName,uint32_t nGameID, uint32_t nGameAlias ) : CBaseCallable( ), m_Players( nPlayers ), m_GameName( nGameName ), m_GameID( nGameID ), m_GameAlias( nGameAlias), m_Result( 0 ) { }
    virtual ~CCallableGameDBInit( );

    virtual uint32_t GetResult( )				{
        return m_Result;
    }
    virtual void SetResult( uint32_t nResult )	{
        m_Result = nResult;
    }
};

class CCallableGameUpdate : virtual public CBaseCallable
{
protected:
    uint32_t m_Hostcounter;
    uint32_t m_Lobby;
    string m_MapType;
    uint32_t m_Duration;
    string m_Map;
    string m_GameName;
    string m_OwnerName;
    string m_CreatorName;
    bool m_Add;
    uint32_t m_Players;
    uint32_t m_Total;
    vector<PlayerOfPlayerList> m_Playerlist;
    string m_Result;

public:
    CCallableGameUpdate( uint32_t hostcounter, uint32_t lobby, string map_type, uint32_t duration, string gamename, string ownername, string creatorname, string map, uint32_t players, uint32_t total, vector<PlayerOfPlayerList> playerlist ) : CBaseCallable( ), m_Map(map), m_GameName(gamename), m_OwnerName(ownername), m_CreatorName(creatorname), m_Playerlist(playerlist), m_Hostcounter(hostcounter), m_Lobby(lobby), m_MapType(map_type), m_Players(players), m_Total(total), m_Duration(duration) { }
    virtual ~CCallableGameUpdate( );

    virtual string GetResult( ) {
        return m_Result;
    }
    virtual void SetResult( string nResult )	{
        m_Result = nResult;
    }
};

class CCallableGamePlayerAdd : virtual public CBaseCallable
{
protected:
    uint32_t m_GameID;
    string m_Name;
    string m_IP;
    uint32_t m_Spoofed;
    string m_SpoofedRealm;
    uint32_t m_Reserved;
    uint32_t m_LoadingTime;
    uint32_t m_Left;
    string m_LeftReason;
    uint32_t m_Team;
    uint32_t m_Colour;
    uint32_t m_ID;
    uint32_t m_Result;

public:
    CCallableGamePlayerAdd( uint32_t nGameID, string nName, string nIP, uint32_t nSpoofed, string nSpoofedRealm, uint32_t nReserved, uint32_t nLoadingTime, uint32_t nLeft, string nLeftReason, uint32_t nTeam, uint32_t nColour, uint32_t nID ) : CBaseCallable( ), m_GameID( nGameID ), m_Name( nName ), m_IP( nIP ), m_Spoofed( nSpoofed ), m_SpoofedRealm( nSpoofedRealm ), m_Reserved( nReserved ), m_LoadingTime( nLoadingTime ), m_Left( nLeft ), m_LeftReason( nLeftReason ), m_Team( nTeam ), m_Colour( nColour ), m_ID( nID ), m_Result( 0 ) { }
    virtual ~CCallableGamePlayerAdd( );

    virtual uint32_t GetResult( )				{
        return m_Result;
    }
    virtual void SetResult( uint32_t nResult )	{
        m_Result = nResult;
    }
    virtual string GetName( )				{
        return m_Name;
    }
};

class CCallableGamePlayerSummaryCheck : virtual public CBaseCallable
{
protected:
    string m_Name;
    CDBGamePlayerSummary *m_Result;

public:
    CCallableGamePlayerSummaryCheck( string nName ) : CBaseCallable( ), m_Name( nName ), m_Result( NULL ) { }
    virtual ~CCallableGamePlayerSummaryCheck( );

    virtual string GetName( )								{
        return m_Name;
    }
    virtual CDBGamePlayerSummary *GetResult( )				{
        return m_Result;
    }
    virtual void SetResult( CDBGamePlayerSummary *nResult )	{
        m_Result = nResult;
    }
};

class CCallableStatsPlayerSummaryCheck : virtual public CBaseCallable
{
protected:
    string m_Name;
    string m_Month;
    string m_Year;
    uint32_t m_GameAlias;
    CDBStatsPlayerSummary *m_Result;

public:
    CCallableStatsPlayerSummaryCheck( string nName, string nMonth, string nYear, uint32_t nGameAlias ) : CBaseCallable( ), m_Name( nName ), m_Month( nMonth ), m_Year( nYear ), m_GameAlias( nGameAlias ), m_Result( NULL ) { }
    virtual ~CCallableStatsPlayerSummaryCheck( );

    virtual string GetName( )                                                               {
        return m_Name;
    }
    virtual string GetMonth( )                                                              {
        return m_Month;
    }
    virtual string GetYear( )                                                               {
        return m_Year;
    }
    virtual uint32_t GetAlias( )                                                            {
        return m_GameAlias;
    }
    virtual CDBStatsPlayerSummary *GetResult( )                              {
        return m_Result;
    }
    virtual void SetResult( CDBStatsPlayerSummary *nResult ) {
        m_Result = nResult;
    }
};

class CCallableInboxSummaryCheck : virtual public CBaseCallable
{
protected:
    string m_Name;
    CDBInboxSummary *m_Result;

public:
    CCallableInboxSummaryCheck( string nName ) : CBaseCallable( ), m_Name( nName ), m_Result( NULL ) { }
    virtual ~CCallableInboxSummaryCheck( );

    virtual string GetName( )                                                               {
        return m_Name;
    }
    virtual CDBInboxSummary *GetResult( )                              {
        return m_Result;
    }
    virtual void SetResult( CDBInboxSummary *nResult ) {
        m_Result = nResult;
    }
};

class CCallableDotAGameAdd : virtual public CBaseCallable
{
protected:
    uint32_t m_GameID;
    uint32_t m_Winner;
    uint32_t m_Min;
    uint32_t m_Sec;
    uint32_t m_Result;

public:
    CCallableDotAGameAdd( uint32_t nGameID, uint32_t nWinner, uint32_t nMin, uint32_t nSec ) : CBaseCallable( ), m_GameID( nGameID ), m_Winner( nWinner ), m_Min( nMin ), m_Sec( nSec ), m_Result( 0 ) { }
    virtual ~CCallableDotAGameAdd( );

    virtual uint32_t GetResult( )				{
        return m_Result;
    }
    virtual void SetResult( uint32_t nResult )	{
        m_Result = nResult;
    }
};

class CCallableDotAPlayerAdd : virtual public CBaseCallable
{
protected:
    uint32_t m_GameID;
    string m_Data;
    uint32_t m_Result;

public:
    CCallableDotAPlayerAdd( uint32_t nGameID, string nData ) : CBaseCallable( ), m_GameID( nGameID ), m_Data( nData ), m_Result( 0 ) { }
    virtual ~CCallableDotAPlayerAdd( );

    virtual uint32_t GetResult( )				{
        return m_Result;
    }
    virtual void SetResult( uint32_t nResult )	{
        m_Result = nResult;
    }
};

class CCallableDotAPlayerSummaryCheck : virtual public CBaseCallable
{
protected:
    string m_Name;
    CDBDotAPlayerSummary *m_Result;

public:
    CCallableDotAPlayerSummaryCheck( string nName ) : CBaseCallable( ), m_Name( nName ), m_Result( NULL ) { }
    virtual ~CCallableDotAPlayerSummaryCheck( );

    virtual string GetName( )								{
        return m_Name;
    }
    virtual CDBDotAPlayerSummary *GetResult( )				{
        return m_Result;
    }
    virtual void SetResult( CDBDotAPlayerSummary *nResult )	{
        m_Result = nResult;
    }
};

class CCallableDownloadAdd : virtual public CBaseCallable
{
protected:
    string m_Map;
    uint32_t m_MapSize;
    string m_Name;
    string m_IP;
    uint32_t m_Spoofed;
    string m_SpoofedRealm;
    uint32_t m_DownloadTime;
    bool m_Result;

public:
    CCallableDownloadAdd( string nMap, uint32_t nMapSize, string nName, string nIP, uint32_t nSpoofed, string nSpoofedRealm, uint32_t nDownloadTime ) : CBaseCallable( ), m_Map( nMap ), m_MapSize( nMapSize ), m_Name( nName ), m_IP( nIP ), m_Spoofed( nSpoofed ), m_SpoofedRealm( nSpoofedRealm ), m_DownloadTime( nDownloadTime ), m_Result( false ) { }
    virtual ~CCallableDownloadAdd( );

    virtual bool GetResult( )				{
        return m_Result;
    }
    virtual void SetResult( bool nResult )	{
        m_Result = nResult;
    }
};

class CCallableScoreCheck : virtual public CBaseCallable
{
protected:
    string m_Category;
    string m_Name;
    string m_Server;
    double m_Result;

public:
    CCallableScoreCheck( string nCategory, string nName, string nServer ) : CBaseCallable( ), m_Category( nCategory ), m_Name( nName ), m_Server( nServer ), m_Result( 0.0 ) { }
    virtual ~CCallableScoreCheck( );

    virtual string GetName( )					{
        return m_Name;
    }
    virtual double GetResult( )					{
        return m_Result;
    }
    virtual void SetResult( double nResult )	{
        m_Result = nResult;
    }
};

class CCallableConnectCheck : virtual public CBaseCallable
{
protected:
    string m_Name;
    uint32_t m_SessionKey;
    bool m_Result;

public:
    CCallableConnectCheck( string nName, uint32_t nSessionKey ) : CBaseCallable( ), m_Name( nName ), m_SessionKey( nSessionKey ), m_Result( false ) { }
    virtual ~CCallableConnectCheck( );

    virtual string GetName( )                                        {
        return m_Name;
    }
    virtual bool GetResult( )                                        {
        return m_Result;
    }
    virtual void SetResult( bool nResult )                {
        m_Result = nResult;
    }
};

class CCallableW3MMDPlayerAdd : virtual public CBaseCallable
{
protected:
    string m_Category;
    uint32_t m_GameID;
    uint32_t m_PID;
    string m_Name;
    string m_Flag;
    uint32_t m_Leaver;
    uint32_t m_Practicing;
    uint32_t m_Result;

public:
    CCallableW3MMDPlayerAdd( string nCategory, uint32_t nGameID, uint32_t nPID, string nName, string nFlag, uint32_t nLeaver, uint32_t nPracticing ) : CBaseCallable( ), m_Category( nCategory ), m_GameID( nGameID ), m_PID( nPID ), m_Name( nName ), m_Flag( nFlag ), m_Leaver( nLeaver ), m_Practicing( nPracticing ), m_Result( 0 ) { }
    virtual ~CCallableW3MMDPlayerAdd( );

    virtual uint32_t GetResult( )				{
        return m_Result;
    }
    virtual void SetResult( uint32_t nResult )	{
        m_Result = nResult;
    }
};

class CCallableW3MMDVarAdd : virtual public CBaseCallable
{
protected:
    uint32_t m_GameID;
    map<VarP,int32_t> m_VarInts;
    map<VarP,double> m_VarReals;
    map<VarP,string> m_VarStrings;

    enum ValueType {
        VALUETYPE_INT = 1,
        VALUETYPE_REAL = 2,
        VALUETYPE_STRING = 3
    };

    ValueType m_ValueType;
    bool m_Result;

public:
    CCallableW3MMDVarAdd( uint32_t nGameID, map<VarP,int32_t> nVarInts ) : CBaseCallable( ), m_GameID( nGameID ), m_VarInts( nVarInts ), m_ValueType( VALUETYPE_INT ), m_Result( false ) { }
    CCallableW3MMDVarAdd( uint32_t nGameID, map<VarP,double> nVarReals ) : CBaseCallable( ), m_GameID( nGameID ), m_VarReals( nVarReals ), m_ValueType( VALUETYPE_REAL ), m_Result( false ) { }
    CCallableW3MMDVarAdd( uint32_t nGameID, map<VarP,string> nVarStrings ) : CBaseCallable( ), m_GameID( nGameID ), m_VarStrings( nVarStrings ), m_ValueType( VALUETYPE_STRING ), m_Result( false ) { }
    virtual ~CCallableW3MMDVarAdd( );

    virtual bool GetResult( )				{
        return m_Result;
    }
    virtual void SetResult( bool nResult )	{
        m_Result = nResult;
    }
};

class CCallableBotStatusCreate : virtual public CBaseCallable
{
protected:
    string m_Username;
    string m_Gamename;
    string m_Ip;
    uint16_t m_Hostport;
    string m_Roc;
    string m_Tft;
    bool m_Result;

public:
    CCallableBotStatusCreate( string nUsername, string nGamename, string nIP, uint16_t nHostport, string nRoc, string nTft ) : CBaseCallable( ), m_Username(nUsername), m_Gamename(nGamename), m_Ip(nIP), m_Hostport(nHostport), m_Roc(nRoc), m_Tft(nTft), m_Result(false) { }
    virtual ~CCallableBotStatusCreate( );
};

class CCallableBotStatusUpdate : virtual public CBaseCallable
{
protected:
    string m_Server;
    uint32_t m_Status;
    bool m_Result;

public:
    CCallableBotStatusUpdate( string nServer, uint32_t nStatus ) : CBaseCallable( ), m_Server(nServer), m_Status(nStatus), m_Result(false) { }
    virtual ~CCallableBotStatusUpdate( );
};

//
// CDBBan
//

class CDBBan
{
private:
    uint32_t m_ID;
    string m_Server;
    string m_Name;
    string m_IP;
    string m_Date;
    string m_GameName;
    string m_Admin;
    string m_Reason;
    string m_ExpireDate;
    string m_Months;
    string m_Days;
    string m_Hours;
    string m_Minutes;
    uint32_t m_PenalityLevel;
public:
    CDBBan( uint32_t id, string nServer, string nName, string nIP, string nDate, string nGameName, string nAdmin, string nReason, string nExpireDate, string nMonths, string nDays, string nHours, string nMinutes, uint32_t nPenalityLevel );
    ~CDBBan( );

    uint32_t GetID( ) {
        return m_ID;
    }
    string GetServer( )		{
        return m_Server;
    }
    string GetName( )		{
        return m_Name;
    }
    string GetIP( )			{
        return m_IP;
    }
    string GetDate( )		{
        return m_Date;
    }
    string GetGameName( )	{
        return m_GameName;
    }
    string GetAdmin( )		{
        return m_Admin;
    }
    string GetReason( )		{
        return m_Reason;
    }
    string GetExpire( )             {
        return m_ExpireDate;
    }
    string GetMonths( )             {
        return m_Months;
    }
    string GetDays( )             {
        return m_Days;
    }
    string GetHours( )             {
        return m_Hours;
    }
    string GetMinutes( )             {
        return m_Minutes;
    }
    uint32_t GetPenalityLevel( ) { return m_PenalityLevel; }
};

//
// CDBGame
//

class CDBGame
{
private:
    uint32_t m_ID;
    string m_Server;
    string m_Map;
    string m_DateTime;
    string m_GameName;
    string m_OwnerName;
    uint32_t m_Duration;

public:
    CDBGame( uint32_t nID, string nServer, string nMap, string nDateTime, string nGameName, string nOwnerName, uint32_t nDuration );
    ~CDBGame( );

    uint32_t GetID( )		{
        return m_ID;
    }
    string GetServer( )		{
        return m_Server;
    }
    string GetMap( )		{
        return m_Map;
    }
    string GetDateTime( )	{
        return m_DateTime;
    }
    string GetGameName( )	{
        return m_GameName;
    }
    string GetOwnerName( )	{
        return m_OwnerName;
    }
    uint32_t GetDuration( )	{
        return m_Duration;
    }

    void SetDuration( uint32_t nDuration )	{
        m_Duration = nDuration;
    }
};

//
// CDBGamePlayer
//

class CDBGamePlayer
{
private:
    uint32_t m_ID;
    uint32_t m_GameID;
    string m_Name;
    string m_IP;
    uint32_t m_Spoofed;
    string m_SpoofedRealm;
    uint32_t m_Reserved;
    uint32_t m_LoadingTime;
    uint32_t m_Left;
    string m_LeftReason;
    uint32_t m_Team;
    uint32_t m_Colour;
    uint32_t m_LeaverLevel;

public:
    CDBGamePlayer( uint32_t nID, uint32_t nGameID, string nName, string nIP, uint32_t nSpoofed, string nSpoofedRealm, uint32_t nReserved, uint32_t nLoadingTime, uint32_t nLeft, string nLeftReason, uint32_t nTeam, uint32_t nColour, uint32_t nLeaverLevel );
    ~CDBGamePlayer( );

    uint32_t GetID( )			{
        return m_ID;
    }
    uint32_t GetGameID( )		{
        return m_GameID;
    }
    string GetName( )			{
        return m_Name;
    }
    string GetIP( )				{
        return m_IP;
    }
    uint32_t GetSpoofed( )		{
        return m_Spoofed;
    }
    string GetSpoofedRealm( )	{
        return m_SpoofedRealm;
    }
    uint32_t GetReserved( )		{
        return m_Reserved;
    }
    uint32_t GetLoadingTime( )	{
        return m_LoadingTime;
    }
    uint32_t GetLeft( )			{
        return m_Left;
    }
    string GetLeftReason( )		{
        return m_LeftReason;
    }
    uint32_t GetTeam( )			{
        return m_Team;
    }
    uint32_t GetColour( )		{
        return m_Colour;
    }
    uint32_t GetLeaverLevel( )  {
        return m_LeaverLevel;
    }

    void SetLoadingTime( uint32_t nLoadingTime )	{
        m_LoadingTime = nLoadingTime;
    }
    void SetLeft( uint32_t nLeft )					{
        m_Left = nLeft;
    }
    void SetLeftReason( string nLeftReason )		{
        m_LeftReason = nLeftReason;
    }
};

//
// CDBGamePlayerSummary
//

class CDBGamePlayerSummary
{
private:
    string m_Server;
    string m_Name;
    string m_FirstGameDateTime;		// datetime of first game played
    string m_LastGameDateTime;		// datetime of last game played
    uint32_t m_TotalGames;			// total number of games played
    uint32_t m_MinLoadingTime;		// minimum loading time in milliseconds (this could be skewed because different maps have different load times)
    uint32_t m_AvgLoadingTime;		// average loading time in milliseconds (this could be skewed because different maps have different load times)
    uint32_t m_MaxLoadingTime;		// maximum loading time in milliseconds (this could be skewed because different maps have different load times)
    uint32_t m_MinLeftPercent;		// minimum time at which the player left the game expressed as a percentage of the game duration (0-100)
    uint32_t m_AvgLeftPercent;		// average time at which the player left the game expressed as a percentage of the game duration (0-100)
    uint32_t m_MaxLeftPercent;		// maximum time at which the player left the game expressed as a percentage of the game duration (0-100)
    uint32_t m_MinDuration;			// minimum game duration in seconds
    uint32_t m_AvgDuration;			// average game duration in seconds
    uint32_t m_MaxDuration;			// maximum game duration in seconds

public:
    CDBGamePlayerSummary( string nServer, string nName, string nFirstGameDateTime, string nLastGameDateTime, uint32_t nTotalGames, uint32_t nMinLoadingTime, uint32_t nAvgLoadingTime, uint32_t nMaxLoadingTime, uint32_t nMinLeftPercent, uint32_t nAvgLeftPercent, uint32_t nMaxLeftPercent, uint32_t nMinDuration, uint32_t nAvgDuration, uint32_t nMaxDuration );
    ~CDBGamePlayerSummary( );

    string GetServer( )					{
        return m_Server;
    }
    string GetName( )					{
        return m_Name;
    }
    string GetFirstGameDateTime( )		{
        return m_FirstGameDateTime;
    }
    string GetLastGameDateTime( )		{
        return m_LastGameDateTime;
    }
    uint32_t GetTotalGames( )			{
        return m_TotalGames;
    }
    uint32_t GetMinLoadingTime( )		{
        return m_MinLoadingTime;
    }
    uint32_t GetAvgLoadingTime( )		{
        return m_AvgLoadingTime;
    }
    uint32_t GetMaxLoadingTime( )		{
        return m_MaxLoadingTime;
    }
    uint32_t GetMinLeftPercent( )		{
        return m_MinLeftPercent;
    }
    uint32_t GetAvgLeftPercent( )		{
        return m_AvgLeftPercent;
    }
    uint32_t GetMaxLeftPercent( )		{
        return m_MaxLeftPercent;
    }
    uint32_t GetMinDuration( )			{
        return m_MinDuration;
    }
    uint32_t GetAvgDuration( )			{
        return m_AvgDuration;
    }
    uint32_t GetMaxDuration( )			{
        return m_MaxDuration;
    }
};

//
// CDBStatsPlayerSummary
//

class CDBStatsPlayerSummary
{
private:
    uint32_t m_ID;
    string m_Player;
    string m_Playerlower;
    double m_Score;
    uint32_t m_Games;
    uint32_t m_Wins;
    uint32_t m_Losses;
    uint32_t m_Draw;
    uint32_t m_Kills;
    uint32_t m_Deaths;
    uint32_t m_Assists;
    uint32_t m_Creeps;
    uint32_t m_Denies;
    uint32_t m_Neutrals;
    uint32_t m_Towers;
    uint32_t m_Rax;
    uint32_t m_Streak;
    uint32_t m_Maxstreak;
    uint32_t m_Losingstreak;
    uint32_t m_Maxlosingstreak;
    uint32_t m_Zerodeaths;
    string m_Realm;
    uint32_t m_Leaves;
    uint32_t m_ALLCount;
    uint32_t m_RankCount;
    bool m_Hidden;
    string m_Country;
    string m_CountryCode;
    uint32_t m_EXP;
    double m_Reputation;
    string m_LanguageSuffix;
    uint32_t m_LeaverLevel;

public:
    CDBStatsPlayerSummary( uint32_t nID, string nPlayer, string nPlayerlower, double nScore, uint32_t nGames, uint32_t nWins, uint32_t nLosses, uint32_t nDraw, uint32_t nKills, uint32_t nDeaths, uint32_t nAssists, uint32_t nCreeps, uint32_t nDenies, uint32_t nNeutrals, uint32_t nTowers, uint32_t nRax, uint32_t nStreak, uint32_t nMaxstreak, uint32_t nLosingstreak, uint32_t nMaxlosingstreak, uint32_t nZerodeaths, string nRealm, uint32_t nLeaves, uint32_t nALLCount, uint32_t nRankCount, bool nHidden, string nCountry, string nCountryCode, uint32_t nEXP, double nReputation, string nLanguageSuffix, uint32_t nLeaverLevel );
    ~CDBStatsPlayerSummary( );

    uint32_t GetID( )                                     {
        return m_ID;
    }
    string GetPlayer( )					{
        return m_Player;
    }
    string GetPlayerLower( )                                     {
        return m_Playerlower;
    }
    double GetScore( )                                     {
        return m_Score;
    }
    uint32_t GetGames( )                                     {
        return m_Games;
    }
    uint32_t GetWins( )                                     {
        return m_Wins;
    }
    uint32_t GetLosses( )                                     {
        return m_Losses;
    }
    uint32_t GetDraw( )                                     {
        return m_Draw;
    }
    uint32_t GetKills( )                                     {
        return m_Kills;
    }
    uint32_t GetDeaths( )                                     {
        return m_Deaths;
    }
    uint32_t GetAssists( )                                     {
        return m_Assists;
    }
    uint32_t GetCreeps( )                                     {
        return m_Creeps;
    }
    uint32_t GetDenies( )                                     {
        return m_Denies;
    }
    uint32_t GetNeutrals( )                                     {
        return m_Neutrals;
    }
    uint32_t GetTowers( )                                     {
        return m_Towers;
    }
    uint32_t GetRax( )                                     {
        return m_Rax;
    }
    uint32_t GetStreak( )                                     {
        return m_Streak;
    }
    uint32_t GetMaxStreak( )                                     {
        return m_Maxstreak;
    }
    uint32_t GetLosingStreak( )                                     {
        return m_Losingstreak;
    }
    uint32_t GetMaxLosingStreak( )                                     {
        return m_Maxlosingstreak;
    }
    uint32_t GetZeroDeaths( )                                     {
        return m_Zerodeaths;
    }
    string GetRealm( )						{
        return m_Realm;
    }
    uint32_t GetLeaves( )						{
        return m_Leaves;
    }
    string GetRank( )						{
        return "#"+UTIL_ToString(m_RankCount+1)+"/"+UTIL_ToString(m_ALLCount);
    }
    bool GetHidden( )                                               {
        return m_Hidden;
    }
    string GetCountry( )                                            {
        return m_Country;
    }
    string GetCountryCode( )                                        {
        return m_CountryCode;
    }

    float GetAvgKills( )                            {
        return m_Games > 0 ? (float)m_Kills / m_Games : 0;
    }
    float GetAvgDeaths( )                           {
        return m_Games > 0 ? (float)m_Deaths / m_Games : 0;
    }
    float GetAvgCreeps( )                       {
        return m_Games > 0 ? (float)m_Creeps / m_Games : 0;
    }
    float GetAvgDenies( )                      {
        return m_Games > 0 ? (float)m_Denies / m_Games : 0;
    }
    float GetAvgAssists( )                          {
        return m_Games > 0 ? (float)m_Assists / m_Games : 0;
    }
    float GetAvgNeutrals( )                     {
        return m_Games > 0 ? (float)m_Neutrals / m_Games : 0;
    }
    float GetAvgTowers( )                       {
        return m_Games > 0 ? (float)m_Towers / m_Games : 0;
    }
    float GetAvgRax( )                         {
        return m_Games > 0 ? (float)m_Rax / m_Games : 0;
    }
    float GetWinPerc( )                         {
        return m_Wins > 0 ? (float)(m_Wins*100) / ( m_Wins+m_Losses ) : 0;
    }
    float GetLeavePerc( )				{
        return m_Games > 0 ? (float)(m_Leaves*100) / ( m_Games ) : 0;
    }
    uint32_t GetEXP( ) {
        return m_EXP;
    }
    double GetReputation( ) {
        return m_Reputation;
    }
    string GetLanguageSuffix( ) {
        return m_LanguageSuffix;
    }
    uint32_t GetLeaverLevel( ) {
        return m_LeaverLevel;
    }
};

//
// CDBInbox
//
class CDBInboxSummary
{
private:
    string m_User;
    string m_Message;

public:
    CDBInboxSummary( string nUser, string nMessage );
    ~CDBInboxSummary( );

    string GetUser( )                                     {
        return m_User;
    }
    string GetMessage( )                                     {
        return m_Message;
    }
};

//
// CDBDotAGame
//

class CDBDotAGame
{
private:
    uint32_t m_ID;
    uint32_t m_GameID;
    uint32_t m_Winner;
    uint32_t m_Min;
    uint32_t m_Sec;

public:
    CDBDotAGame( uint32_t nID, uint32_t nGameID, uint32_t nWinner, uint32_t nMin, uint32_t nSec );
    ~CDBDotAGame( );

    uint32_t GetID( )		{
        return m_ID;
    }
    uint32_t GetGameID( )	{
        return m_GameID;
    }
    uint32_t GetWinner( )	{
        return m_Winner;
    }
    uint32_t GetMin( )		{
        return m_Min;
    }
    uint32_t GetSec( )		{
        return m_Sec;
    }
};

//
// CDBDotAPlayer
//

class CDBDotAPlayer
{
private:
    uint32_t m_ID;
    uint32_t m_GameID;
    uint32_t m_Colour;
    uint32_t m_Kills;
    uint32_t m_Deaths;
    uint32_t m_CreepKills;
    uint32_t m_CreepDenies;
    uint32_t m_Assists;
    uint32_t m_Gold;
    uint32_t m_NeutralKills;
    string m_Items[6];
    string m_Spell[6];
    string m_Hero;
    uint32_t m_NewColour;
    uint32_t m_TowerKills;
    uint32_t m_RaxKills;
    uint32_t m_CourierKills;
    uint32_t m_Level;

public:
    CDBDotAPlayer( );
    CDBDotAPlayer( uint32_t nID, uint32_t nGameID, uint32_t nColour, uint32_t nKills, uint32_t nDeaths, uint32_t nCreepKills, uint32_t nCreepDenies, uint32_t nAssists, uint32_t nGold, uint32_t nNeutralKills, string nItem1, string nItem2, string nItem3, string nItem4, string nItem5, string nItem6, string nSpell1, string nSpell2, string nSpell3, string nSpell4, string nSpell5, string nSpell6, string nHero, uint32_t nNewColour, uint32_t nTowerKills, uint32_t nRaxKills, uint32_t nCourierKills, uint32_t nLevel );
    ~CDBDotAPlayer( );

    uint32_t GetID( )			{
        return m_ID;
    }
    uint32_t GetGameID( )		{
        return m_GameID;
    }
    uint32_t GetColour( )		{
        return m_Colour;
    }
    uint32_t GetKills( )		{
        return m_Kills;
    }
    uint32_t GetDeaths( )		{
        return m_Deaths;
    }
    uint32_t GetCreepKills( )	{
        return m_CreepKills;
    }
    uint32_t GetCreepDenies( )	{
        return m_CreepDenies;
    }
    uint32_t GetAssists( )		{
        return m_Assists;
    }
    uint32_t GetGold( )			{
        return m_Gold;
    }
    uint32_t GetNeutralKills( )	{
        return m_NeutralKills;
    }
    string GetItem( unsigned int i );
    string GetSpell( unsigned int i );
    string GetHero( )			{
        return m_Hero;
    }
    uint32_t GetNewColour( )	{
        return m_NewColour;
    }
    uint32_t GetTowerKills( )	{
        return m_TowerKills;
    }
    uint32_t GetRaxKills( )		{
        return m_RaxKills;
    }
    uint32_t GetCourierKills( )	{
        return m_CourierKills;
    }
    uint32_t GetLevel()	{
        return m_Level;
    }

    void SetColour( uint32_t nColour )				{
        m_Colour = nColour;
    }
    void SetKills( uint32_t nKills )				{
        m_Kills = nKills;
    }
    void SetDeaths( uint32_t nDeaths )				{
        m_Deaths = nDeaths;
    }
    void SetCreepKills( uint32_t nCreepKills )		{
        m_CreepKills = nCreepKills;
    }
    void SetCreepDenies( uint32_t nCreepDenies )	{
        m_CreepDenies = nCreepDenies;
    }
    void SetAssists( uint32_t nAssists )			{
        m_Assists = nAssists;
    }
    void SetGold( uint32_t nGold )					{
        m_Gold = nGold;
    }
    void SetNeutralKills( uint32_t nNeutralKills )	{
        m_NeutralKills = nNeutralKills;
    }
    void SetItem( unsigned int i, string item );
    void SetSpell( unsigned int i, string spell );
    void SetHero( string nHero )					{
        m_Hero = nHero;
    }
    void SetNewColour( uint32_t nNewColour )		{
        m_NewColour = nNewColour;
    }
    void SetTowerKills( uint32_t nTowerKills )		{
        m_TowerKills = nTowerKills;
    }
    void SetRaxKills( uint32_t nRaxKills )			{
        m_RaxKills = nRaxKills;
    }
    void SetCourierKills( uint32_t nCourierKills )	{
        m_CourierKills = nCourierKills;
    }
    void SetLevel( uint32_t nLevel )	{
        m_Level = nLevel;
    }
};

//
// CDBDotAPlayerSummary
//

class CDBDotAPlayerSummary
{
private:
    string m_Server;
    string m_Name;
    uint32_t m_TotalGames;			// total number of dota games played
    uint32_t m_TotalWins;			// total number of dota games won
    uint32_t m_TotalLosses;			// total number of dota games lost
    uint32_t m_TotalKills;			// total number of hero kills
    uint32_t m_TotalDeaths;			// total number of deaths
    uint32_t m_TotalCreepKills;		// total number of creep kills
    uint32_t m_TotalCreepDenies;	// total number of creep denies
    uint32_t m_TotalAssists;		// total number of assists
    uint32_t m_TotalNeutralKills;	// total number of neutral kills
    uint32_t m_TotalTowerKills;		// total number of tower kills
    uint32_t m_TotalRaxKills;		// total number of rax kills
    uint32_t m_TotalCourierKills;	// total number of courier kills

public:
    CDBDotAPlayerSummary( string nServer, string nName, uint32_t nTotalGames, uint32_t nTotalWins, uint32_t nTotalLosses, uint32_t nTotalKills, uint32_t nTotalDeaths, uint32_t nTotalCreepKills, uint32_t nTotalCreepDenies, uint32_t nTotalAssists, uint32_t nTotalNeutralKills, uint32_t nTotalTowerKills, uint32_t nTotalRaxKills, uint32_t nTotalCourierKills );
    ~CDBDotAPlayerSummary( );

    string GetServer( )					{
        return m_Server;
    }
    string GetName( )					{
        return m_Name;
    }
    uint32_t GetTotalGames( )			{
        return m_TotalGames;
    }
    uint32_t GetTotalWins( )			{
        return m_TotalWins;
    }
    uint32_t GetTotalLosses( )			{
        return m_TotalLosses;
    }
    uint32_t GetTotalKills( )			{
        return m_TotalKills;
    }
    uint32_t GetTotalDeaths( )			{
        return m_TotalDeaths;
    }
    uint32_t GetTotalCreepKills( )		{
        return m_TotalCreepKills;
    }
    uint32_t GetTotalCreepDenies( )		{
        return m_TotalCreepDenies;
    }
    uint32_t GetTotalAssists( )			{
        return m_TotalAssists;
    }
    uint32_t GetTotalNeutralKills( )	{
        return m_TotalNeutralKills;
    }
    uint32_t GetTotalTowerKills( )		{
        return m_TotalTowerKills;
    }
    uint32_t GetTotalRaxKills( )		{
        return m_TotalRaxKills;
    }
    uint32_t GetTotalCourierKills( )	{
        return m_TotalCourierKills;
    }

    float GetAvgKills( )				{
        return m_TotalGames > 0 ? (float)m_TotalKills / m_TotalGames : 0;
    }
    float GetAvgDeaths( )				{
        return m_TotalGames > 0 ? (float)m_TotalDeaths / m_TotalGames : 0;
    }
    float GetAvgCreepKills( )			{
        return m_TotalGames > 0 ? (float)m_TotalCreepKills / m_TotalGames : 0;
    }
    float GetAvgCreepDenies( )			{
        return m_TotalGames > 0 ? (float)m_TotalCreepDenies / m_TotalGames : 0;
    }
    float GetAvgAssists( )				{
        return m_TotalGames > 0 ? (float)m_TotalAssists / m_TotalGames : 0;
    }
    float GetAvgNeutralKills( )			{
        return m_TotalGames > 0 ? (float)m_TotalNeutralKills / m_TotalGames : 0;
    }
    float GetAvgTowerKills( )			{
        return m_TotalGames > 0 ? (float)m_TotalTowerKills / m_TotalGames : 0;
    }
    float GetAvgRaxKills( )				{
        return m_TotalGames > 0 ? (float)m_TotalRaxKills / m_TotalGames : 0;
    }
    float GetAvgCourierKills( )			{
        return m_TotalGames > 0 ? (float)m_TotalCourierKills / m_TotalGames : 0;
    }
};

struct PlayerOfPlayerList  {
    string Username;
    string Realm;
    uint16_t Ping;
    string IP;
    uint8_t Color;
    uint16_t LeftTime;
    string LeftReason;
    uint8_t Team;
    uint8_t Slot;
};

#endif
