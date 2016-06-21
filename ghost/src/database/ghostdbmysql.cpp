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
#include "../utils/util.h"
#include "ghostdb.h"
#include "ghostdbmysql.h"
#include <signal.h>

#ifdef WIN32
    #include <winsock.h>
    // fix for win32 cpp ide
    #include "mysql/include/mysql/mysql.h"
#else
    #include <mysql/mysql.h>
#endif

#include <boost/thread.hpp>

//
// COHBotDBMySQL
//


COHBotDBMySQL::calls COHBotDBMySQL::outstandingCalls;

COHBotDBMySQL :: COHBotDBMySQL( CConfig *CFG ) : COHBotDB( CFG )
{
    bool m_ReadGlobalMySQL = CFG->GetInt("oh_readglobalmysql", 0) == 0 ? false : true;
    if(m_ReadGlobalMySQL) {
      string m_GlobalMySQLPath = UTIL_AddPathSeperator( CFG->GetString( "oh_globalmysqlpath", "../" ) );
      CConfig CFG2;
      CFG2.Read( m_GlobalMySQLPath+"mysql.cfg" );
      m_Server = CFG2.GetString( "db_mysql_server", string( ) );
      m_Database = CFG2.GetString( "db_mysql_database", "ohbot" );
      m_User = CFG2.GetString( "db_mysql_user", string( ) );
      m_Password = CFG2.GetString( "db_mysql_password", string( ) );
      m_Port = CFG2.GetInt( "db_mysql_port", 0 );
    } else {
      m_Server = CFG->GetString( "db_mysql_server", string( ) );
      m_Database = CFG->GetString( "db_mysql_database", "ohbot" );
      m_User = CFG->GetString( "db_mysql_user", string( ) );
      m_Password = CFG->GetString( "db_mysql_password", string( ) );
      m_Port = CFG->GetInt( "db_mysql_port", 0 );
    }

    m_BotID = CFG->GetInt( "db_mysql_botid", 0 );
    m_NumConnections = 1;
    m_OutstandingCallables = 0;

    mysql_library_init( 0, NULL, NULL );

    // create the first connection

    MYSQL *Connection = NULL;

    if( !( Connection = mysql_init( NULL ) ) )
    {
        Log->Write( string( "[MYSQL] " ) + mysql_error( Connection ) );
        m_HasError = true;
        m_Error = "error initializing MySQL connection";
        return;
    }

    my_bool Reconnect = true;
    mysql_options( Connection, MYSQL_OPT_RECONNECT, &Reconnect );

    if( !( mysql_real_connect( Connection, m_Server.c_str( ), m_User.c_str( ), m_Password.c_str( ), m_Database.c_str( ), m_Port, NULL, 0 ) ) )
    {
        Log->Write( string( "[MYSQL] " ) + mysql_error( Connection ) );
        m_HasError = true;
        m_Error = "error connecting to MySQL server";
        return;
    }

    m_IdleConnections.push( Connection );
}

COHBotDBMySQL :: ~COHBotDBMySQL( )
{
    boost::mutex::scoped_lock lock(m_DatabaseMutex);

    Log->Info( "[MYSQL] closing " + UTIL_ToString( m_IdleConnections.size( ) ) + "/" + UTIL_ToString( m_NumConnections ) + " idle MySQL connections" );

    while( !m_IdleConnections.empty( ) )
    {
        mysql_close( (MYSQL *)m_IdleConnections.front( ) );
        m_IdleConnections.pop( );
    }

    if( m_OutstandingCallables > 0 )
        Log->Warning( "[MYSQL] " + UTIL_ToString( m_OutstandingCallables ) + " outstanding callables were never recovered" );

    mysql_library_end( );
}

string COHBotDBMySQL :: GetStatus( )
{

    string response = "DB STATUS --- Connections: " + UTIL_ToString( m_IdleConnections.size( ) ) + "/" + UTIL_ToString( m_NumConnections ) + " idle. Outstanding callables: " + UTIL_ToString( m_OutstandingCallables ) + ".";

    for(map<string,uint16_t>::iterator iter = COHBotDBMySQL::outstandingCalls.begin(); iter != COHBotDBMySQL::outstandingCalls.end(); ++iter) {
	if(iter->second >=1 ) {
        Log->Warning("UNRECOVERED CALLABLE FOUND - ["+iter->first+"] with ["+UTIL_ToString(iter->second)+"] unrecovered callables!");
	}
    }

    return response;
}

void COHBotDBMySQL :: RecoverCallable( CBaseCallable *callable )
{
    boost::mutex::scoped_lock lock(m_DatabaseMutex);
    CMySQLCallable *MySQLCallable = dynamic_cast<CMySQLCallable *>( callable );

    if( MySQLCallable )
    {
        if( !MySQLCallable->GetError( ).empty( ) )
            Log->Write( "[MYSQL] error --- " + MySQLCallable->GetError( ) );

        if( m_IdleConnections.size( ) > 30 || !MySQLCallable->GetError( ).empty( ) )
        {
            mysql_close( (MYSQL *)MySQLCallable->GetConnection( ) );
            --m_NumConnections;
        }
        else
            m_IdleConnections.push( MySQLCallable->GetConnection( ) );

        if( m_OutstandingCallables == 0 )
            Log->Write( "[MYSQL] recovered a mysql callable with zero outstanding" );
        else
            --m_OutstandingCallables;
    }
    else
        Log->Write( "[MYSQL] tried to recover a non-mysql callable" );
}

std::vector<std::string> &split2(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split2(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split2(s, delim, elems);
    return elems;
}

bool HasSpecialCharacters(const char *str)
{
    return str[strspn(str, "0123456789.")] != 0;
}

void COHBotDBMySQL :: CreateThread( CBaseCallable *callable )
{
    try
    {
        boost :: thread Thread( boost :: ref( *callable ) );
    }
    catch( boost :: thread_resource_error tre )
    {
        Log->Write( "[MYSQL] error spawning thread on attempt #1 [" + string( tre.what( ) ) + "], pausing execution and trying again in 50ms" );
        MILLISLEEP( 50 );

        try
        {
            boost :: thread Thread( boost :: ref( *callable ) );
        }
        catch( boost :: thread_resource_error tre2 )
        {
            Log->Write( "[MYSQL] error spawning thread on attempt #2 [" + string( tre2.what( ) ) + "], giving up" );
            callable->SetReady( true );
        }
    }
}

CCallableRegAdd *COHBotDBMySQL :: ThreadedRegAdd( string user, string server, string mail, string password, string type )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableRegAdd *Callable = new CMySQLCallableRegAdd( user, server, mail, password, type, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port,  "RegisterAdd");
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableStatsSystem *COHBotDBMySQL :: ThreadedStatsSystem( string user, string input, uint32_t one, string type )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableStatsSystem *Callable = new CMySQLCallableStatsSystem( user, input, one, type, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "StatsSystem" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallablePWCheck *COHBotDBMySQL :: ThreadedPWCheck( string user )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallablePWCheck *Callable = new CMySQLCallablePWCheck( user, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "PWCheck" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallablePassCheck *COHBotDBMySQL :: ThreadedPassCheck( string user, string pass, uint32_t st )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallablePassCheck *Callable = new CMySQLCallablePassCheck( user, pass, st, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "PassCheck" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallablepm *COHBotDBMySQL :: Threadedpm( string user, string listener, uint32_t status, string message, string type )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallablepm *Callable = new CMySQLCallablepm( user, listener, status, message, type, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "PM" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallablePList *COHBotDBMySQL :: ThreadedPList( string server )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallablePList *Callable = new CMySQLCallablePList( server, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "PList" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableFlameList *COHBotDBMySQL :: ThreadedFlameList( )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableFlameList *Callable = new CMySQLCallableFlameList( Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "FlameList" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableForcedGProxyList *COHBotDBMySQL :: ThreadedForcedGProxyList( )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableForcedGProxyList *Callable = new CMySQLCallableForcedGProxyList( Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "ForcedGProxy" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableAliasList *COHBotDBMySQL :: ThreadedAliasList( )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableAliasList *Callable = new CMySQLCallableAliasList( Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "Alias" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableDeniedNamesList *COHBotDBMySQL :: ThreadedDeniedNamesList( )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableDeniedNamesList *Callable = new CMySQLCallableDeniedNamesList( Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "DeniedNameList" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableAnnounceList *COHBotDBMySQL :: ThreadedAnnounceList( )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableAnnounceList *Callable = new CMySQLCallableAnnounceList( Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "AnnounceList" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableDCountryList *COHBotDBMySQL :: ThreadedDCountryList( )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableDCountryList *Callable = new CMySQLCallableDCountryList( Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "DCounterList" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableStoreLog *COHBotDBMySQL :: ThreadedStoreLog( uint32_t chatid, string game, vector<string> admin )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableStoreLog *Callable =new CMySQLCallableStoreLog( chatid, game, admin, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "StoreLog" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallablegs *COHBotDBMySQL :: Threadedgs( uint32_t chatid, string gn, uint32_t st, uint32_t gametype, uint32_t gamealias )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallablegs *Callable =new CMySQLCallablegs( chatid, gn, st, gametype, gamealias, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "GameStatus" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallablepenp *COHBotDBMySQL :: Threadedpenp( string name, string reason, string admin, uint32_t amount, string type )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallablepenp *Callable =new CMySQLCallablepenp( name, reason, admin, amount, type, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "PenalityPoint" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableBanCount *COHBotDBMySQL :: ThreadedBanCount( string server )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableBanCount *Callable = new CMySQLCallableBanCount( server, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "BanCount" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableBanCheck *COHBotDBMySQL :: ThreadedBanCheck( string server, string user, string ip )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableBanCheck *Callable = new CMySQLCallableBanCheck( server, user, ip, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "BanCheck" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableBanCheck2 *COHBotDBMySQL :: ThreadedBanCheck2( string server, string user, string type )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableBanCheck2 *Callable = new CMySQLCallableBanCheck2( server, user, type, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "BanCheck2" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableBanAdd *COHBotDBMySQL :: ThreadedBanAdd( string server, string user, string ip, string gamename, string admin, string reason, uint32_t bantime, string country )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableBanAdd *Callable = new CMySQLCallableBanAdd( server, user, ip, gamename, admin, reason, bantime, country, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "BanAdd" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallablePUp *COHBotDBMySQL :: ThreadedPUp( string name, uint32_t level, string realm, string user )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallablePUp *Callable = new CMySQLCallablePUp( name, level, realm, user, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "PUp" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableBanRemove *COHBotDBMySQL :: ThreadedBanRemove( string server, string user )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableBanRemove *Callable = new CMySQLCallableBanRemove( server, user, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "BanRemove" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableBanRemove *COHBotDBMySQL :: ThreadedBanRemove( string user )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableBanRemove *Callable = new CMySQLCallableBanRemove( string( ), user, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "BanRemove" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableBanList *COHBotDBMySQL :: ThreadedBanList( string server )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableBanList *Callable = new CMySQLCallableBanList( server, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "BanList" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableTBRemove *COHBotDBMySQL :: ThreadedTBRemove( string server )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableTBRemove *Callable = new CMySQLCallableTBRemove( server, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "TBremove" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableCommandList *COHBotDBMySQL :: ThreadedCommandList( )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableCommandList *Callable = new CMySQLCallableCommandList( Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "CommandList" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableGameAdd *COHBotDBMySQL :: ThreadedGameAdd( string server, string map, string gamename, string ownername, uint32_t duration, uint32_t gamestate, string creatorname, string creatorserver, uint32_t gametype, vector<string> lobbylog, vector<string> gamelog, uint32_t databaseid, uint32_t lobbytime )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableGameAdd *Callable = new CMySQLCallableGameAdd( server, map, gamename, ownername, duration, gamestate, creatorname, creatorserver, gametype, lobbylog, gamelog, databaseid, lobbytime, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "GameAdd" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableGameDBInit *COHBotDBMySQL :: ThreadedGameDBInit( vector<CDBBan *> players, string gamename, uint32_t gameid, uint32_t gamealias )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableGameDBInit *Callable = new CMySQLCallableGameDBInit( players, gamename, gameid, gamealias, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "DBInit" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableGamePlayerAdd *COHBotDBMySQL :: ThreadedGamePlayerAdd( uint32_t gameid, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t reserved, uint32_t loadingtime, uint32_t left, string leftreason, uint32_t team, uint32_t colour, uint32_t id )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableGamePlayerAdd *Callable = new CMySQLCallableGamePlayerAdd( gameid, name, ip, spoofed, spoofedrealm, reserved, loadingtime, left, leftreason, team, colour, id, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "GamePlayerAdd");
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableGameUpdate *COHBotDBMySQL :: ThreadedGameUpdate( uint32_t hostcounter, uint32_t lobby, string map_type, uint32_t duration, string gamename, string ownername, string creatorname, string map, uint32_t players, uint32_t total, vector<PlayerOfPlayerList> playerlist )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableGameUpdate *Callable = new CMySQLCallableGameUpdate( hostcounter, lobby, map_type, duration, gamename, ownername, creatorname, map, players, total,  playerlist, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "GameUpdate" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableGamePlayerSummaryCheck *COHBotDBMySQL :: ThreadedGamePlayerSummaryCheck( string name )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableGamePlayerSummaryCheck *Callable = new CMySQLCallableGamePlayerSummaryCheck( name, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "PlayerSummaryCheck" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableStatsPlayerSummaryCheck *COHBotDBMySQL :: ThreadedStatsPlayerSummaryCheck( string name, string month, string year, uint32_t alias )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableStatsPlayerSummaryCheck *Callable = new CMySQLCallableStatsPlayerSummaryCheck( name, month, year, alias, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "StatsPlayerSummaryCheck" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}


CCallableInboxSummaryCheck *COHBotDBMySQL :: ThreadedInboxSummaryCheck( string name )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableInboxSummaryCheck *Callable = new CMySQLCallableInboxSummaryCheck( name, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "InboxSummaryCheck" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableDotAGameAdd *COHBotDBMySQL :: ThreadedDotAGameAdd( uint32_t gameid, uint32_t winner, uint32_t min, uint32_t sec )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableDotAGameAdd *Callable = new CMySQLCallableDotAGameAdd( gameid, winner, min, sec, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "DotAGameAdd" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableDotAPlayerAdd *COHBotDBMySQL :: ThreadedDotAPlayerAdd( uint32_t gameid, string m_Data )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableDotAPlayerAdd *Callable = new CMySQLCallableDotAPlayerAdd( gameid, m_Data, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "DotaPlayerAdd" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableDotAPlayerSummaryCheck *COHBotDBMySQL :: ThreadedDotAPlayerSummaryCheck( string name )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableDotAPlayerSummaryCheck *Callable = new CMySQLCallableDotAPlayerSummaryCheck( name, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "DotaPlayerSummaryCheck" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableDownloadAdd *COHBotDBMySQL :: ThreadedDownloadAdd( string map, uint32_t mapsize, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t downloadtime )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableDownloadAdd *Callable = new CMySQLCallableDownloadAdd( map, mapsize, name, ip, spoofed, spoofedrealm, downloadtime, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "DownloadAdd" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableScoreCheck *COHBotDBMySQL :: ThreadedScoreCheck( string category, string name, string server )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableScoreCheck *Callable = new CMySQLCallableScoreCheck( category, name, server, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "ScoreCheck" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}


CCallableConnectCheck *COHBotDBMySQL :: ThreadedConnectCheck( string name, uint32_t sessionkey )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableConnectCheck *Callable = new CMySQLCallableConnectCheck( name, sessionkey, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "ConnectCheck" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableW3MMDPlayerAdd *COHBotDBMySQL :: ThreadedW3MMDPlayerAdd( string category, uint32_t gameid, uint32_t pid, string name, string flag, uint32_t leaver, uint32_t practicing )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableW3MMDPlayerAdd *Callable = new CMySQLCallableW3MMDPlayerAdd( category, gameid, pid, name, flag, leaver, practicing, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "W33mmdplayeradd" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableW3MMDVarAdd *COHBotDBMySQL :: ThreadedW3MMDVarAdd( uint32_t gameid, map<VarP,int32_t> var_ints )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableW3MMDVarAdd *Callable = new CMySQLCallableW3MMDVarAdd( gameid, var_ints, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "w3mmdvaradd" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableW3MMDVarAdd *COHBotDBMySQL :: ThreadedW3MMDVarAdd( uint32_t gameid, map<VarP,double> var_reals )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableW3MMDVarAdd *Callable = new CMySQLCallableW3MMDVarAdd( gameid, var_reals, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "w3mmdvaradd" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableW3MMDVarAdd *COHBotDBMySQL :: ThreadedW3MMDVarAdd( uint32_t gameid, map<VarP,string> var_strings )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableW3MMDVarAdd *Callable = new CMySQLCallableW3MMDVarAdd( gameid, var_strings, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "w3mmdvaradd" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableBotStatusCreate *COHBotDBMySQL :: ThreadedBotStatusCreate( string username, string gamename, string ip, uint16_t hostport, string roc, string tft )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableBotStatusCreate *Callable = new CMySQLCallableBotStatusCreate( username, gamename, ip, hostport, roc, tft, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "StatusCreate" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

CCallableBotStatusUpdate *COHBotDBMySQL :: ThreadedBotStatusUpdate( string server, uint32_t status )
{
    void *Connection = GetIdleConnection( );

    if( !Connection )
        ++m_NumConnections;

    CCallableBotStatusUpdate *Callable = new CMySQLCallableBotStatusUpdate(  server, status, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port, "BotStatusUpdate" );
    CreateThread( Callable );
    ++m_OutstandingCallables;
    return Callable;
}

void *COHBotDBMySQL :: GetIdleConnection( )
{
    boost::mutex::scoped_lock lock(m_DatabaseMutex);

    void *Connection = NULL;

    if( !m_IdleConnections.empty( ) )
    {
        Connection = m_IdleConnections.front( );
        m_IdleConnections.pop( );
    }

    return Connection;
}

//
// unprototyped global helper functions
//

string MySQLEscapeString( void *conn, string str )
{
    char *to = new char[str.size( ) * 2 + 1];
    unsigned long size = mysql_real_escape_string( (MYSQL *)conn, to, str.c_str( ), str.size( ) );
    string result( to, size );
    delete [] to;
    return result;
}

vector<string> MySQLFetchRow( MYSQL_RES *res )
{
    vector<string> Result;

    MYSQL_ROW Row = mysql_fetch_row( res );

    if( Row )
    {
        unsigned long *Lengths;
        Lengths = mysql_fetch_lengths( res );

        for( unsigned int i = 0; i < mysql_num_fields( res ); ++i )
        {
            if( Row[i] )
                Result.push_back( string( Row[i], Lengths[i] ) );
            else
                Result.push_back( string( ) );
        }
    }

    return Result;
}

//
// global helper functions
//

uint32_t MySQLRegAdd( void *conn, string *error, uint32_t botid, string user, string server, string mail, string password, string type )
{
    uint32_t RowID = 0;
    transform( user.begin( ), user.end( ), user.begin( ), ::tolower );
    transform( mail.begin( ), mail.end( ), mail.begin( ), ::tolower );
    string EscServer = MySQLEscapeString( conn, server );
    string EscName = MySQLEscapeString( conn, user );
    string EscMail = MySQLEscapeString( conn, mail );
    string EscPassword = MySQLEscapeString( conn, password );
    string QueryCheck = "SELECT `bnet_username`, `user_ppwd`, `user_email` from oh_users where user_name = '" + EscName + "' or bnet_username = '" + EscName + "' or user_email = '" + EscMail + "'";
    bool isUser = false;
    string Pass = "";
    string Mail = "";

    if( mysql_real_query( (MYSQL *)conn, QueryCheck.c_str( ), QueryCheck.size( ) ) != 0 )
    {
        *error = mysql_error( (MYSQL *)conn );
        return 0;
    }
    else
    {
        //RowID = mysql_insert_id( (MYSQL *)conn );
        MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );
        if (Result)
        {
            vector<string> Row = MySQLFetchRow( Result );
            if (Row.size( ) == 3)
            {
                isUser = true;
                Pass = Row[1];
                Mail = Row[2];
            }
            mysql_free_result( Result );
        }
        else
            *error = mysql_error( (MYSQL *)conn );
    }

    if( type == "r" && !isUser )
    {
        string Query = "INSERT INTO oh_users ( user_name, bnet_username, user_email, user_realm, admin_realm, user_password, user_bnet, user_joined ) VALUES ( '" + EscName + "', '" + EscName + "', '" + EscMail + "', '" + EscServer + "', '" + EscServer + "', '" + EscPassword + "', '1', UNIX_TIMESTAMP() )";

        if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );

        return 1;
    }
    else if( type == "r" && isUser )
        return 5;
    else if( type == "c" && isUser )
    {
        if( Pass != EscPassword )
            return 3;
        else if( Mail != EscMail )
            return 4;
        else
        {
            string Query = "UPDATE `oh_users` SET `user_bnet` = '2', `admin_realm` = '" + EscServer + "', `bnet_username` = '" + EscName + "' WHERE `user_email` = '" + EscMail + "';";
            if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
                *error = mysql_error( (MYSQL *)conn );
            else
                return 2;
        }
    }
    else if( type == "c" && !isUser )
        return 6;

    return 0;
}

string MySQLStatsSystem( void *conn, string *error, uint32_t botid, string user, string input, uint32_t one, string type )
{
    transform( user.begin( ), user.end( ), user.begin( ), ::tolower );
    string EscUser = MySQLEscapeString( conn, user );
    string EscInput = MySQLEscapeString( conn, input );

    if( type == "top" )
    {
        string ReturnResult = "failed";
        string Query = "SELECT player, score FROM oh_stats ORDER BY score DESC LIMIT 10";

        if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );
        else
        {
            MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

            if( Result )
            {
                vector<string> Row = MySQLFetchRow( Result );
                uint32_t c = 1;
                while( Row.size( ) == 2 )
                {
                    if( ReturnResult.empty())
                    {
                        ReturnResult = "Top Players: 1."+Row[0]+"("+Row[1]+")";
                    } else {
                        c++;
                        ReturnResult = ", "+UTIL_ToString(c)+"."+Row[0]+"("+Row[1]+")";
                    }
                }

                mysql_free_result( Result );
            }
            else
                *error = mysql_error( (MYSQL *)conn );
        }
        return ReturnResult;
    }
    else if( type == "betcheck" || type == "bet" )
    {
        string CheckQuery = "SELECT `points`, `points_bet` FROM `oh_stats_players` WHERE `player_lower` = '" + EscUser + "' AND month=MONTH(NOW()) AND year=YEAR(NOW());";
        uint32_t currentpoints = 0;
        uint32_t betpoints = 0;
        if( mysql_real_query( (MYSQL *)conn, CheckQuery.c_str( ), CheckQuery.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );
        else
        {
            MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

            if( Result )
            {
                vector<string> Row = MySQLFetchRow( Result );

                if( Row.size( ) == 2 )
                {
                    currentpoints = UTIL_ToUInt32( Row[0] );
                    betpoints = UTIL_ToUInt32( Row[1] );
                }
                else
                    return "not listed";

                mysql_free_result( Result );
            }
        }

        if( type == "betcheck" )
            return UTIL_ToString( currentpoints );
        else if( type == "bet" && betpoints != 0 )
            return "already bet";
        else if( type == "bet" && one > currentpoints )
            return UTIL_ToString( currentpoints  );
        else if( type == "bet" )
        {
            string BetQuery = "UPDATE `oh_stats_players` SET `points_bet` = '" + UTIL_ToString( one ) + "' WHERE `player_lower` = '" + EscUser + "' AND month=MONTH(NOW()) AND year=YEAR(NOW());";
            if( mysql_real_query( (MYSQL *)conn, BetQuery.c_str( ), BetQuery.size( ) ) != 0 )
                *error = mysql_error( (MYSQL *)conn );
            else
            {
                return "successfully bet";
            }
        }
        return "failed";
    }
    else if( type == "aliascheck" )
    {
        string Aliases = "";
        string Query = "SELECT name,spoofedrealm,COUNT(*) FROM oh_gameplayers INNER JOIN (SELECT DISTINCT ip FROM oh_gameplayers WHERE AND ip != '0' AND ip != '0.0.0.0' WHERE  name='" + EscUser + "') a USING (ip) GROUP  BY player_id ORDER BY COUNT(*) DESC LIMIT 5";

        if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );
        else
        {
            MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

            if( Result )
            {
                vector<string> Row = MySQLFetchRow( Result );

                while( Row.size( ) == 3 )
                {
                    Aliases += ", " + Row[0] + "@" + Row[1]+"("+Row[2]+")";
                    Row = MySQLFetchRow( Result );
                }

                mysql_free_result( Result );
            }
            else
                *error = mysql_error( (MYSQL *)conn );
        }

        if( Aliases.length( ) < 5 )
            return "failed";
        else
            return "Aliases: " + Aliases.substr( 2 );

    }
    else if( type == "rpp" )
    {
        string LimitString;
        if( one != 0 )
            LimitString = "Limit "+UTIL_ToString(one);

        if( EscInput.empty( ) )
        {
            string Query = "DELETE FROM oh_game_offenses WHERE player_name = '"+EscUser+"' ORDER BY id ASC "+LimitString+";";
            if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
                *error = mysql_error( (MYSQL *)conn );
            else
            {
                if( one != 0 )
                    return "Successfully removed the first penality points["+UTIL_ToString(one)+"] from User ["+EscUser+"]";
                else
                    return "Successfully removed all penality points for User ["+EscUser+"]";
            }
        }
        else
        {
            string Query = "DELETE FROM oh_game_offenses WHERE player_name = '"+EscUser+"' AND reason LIKE '%"+EscInput+"%' ORDER BY id ASC "+LimitString+";";
            if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
                *error = mysql_error( (MYSQL *)conn );
            else
            {
                if( one != 0 )
                    return "Removed the first ["+UTIL_ToString(one)+"] of player ["+EscUser+"] which had ["+EscInput+"] as reason.";
                else
                    return "Successfully removed all penality points for User ["+EscUser+"] for reason ["+EscInput+"]";
            }
        }
        return "failed";
    }
    else if(type=="forcegproxy")
    {
        bool success=false;
        string Query="INSERT INTO oh_gproxy ( player, added, added_by ) VALUES ('"+EscUser+"', NOW(), '"+EscInput+"')";
        if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );
        else
            success=true;
        if(success)
            return "Successfully forced from now User ["+EscUser+"] to use gproxy.";

        return "failed";
    }
    else if(type=="gamelist") {
        string Query = "SELECT gamename,players,total FROM oh_gamelist WHERE lobby = 1 AND gamename LIKE '%"+EscUser+"%'";

        if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );
        else
        {
            MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );
            string response;
            int num = 0;
            int totg = 0;
            int totp = 0;
            if( Result )
            {
                vector<string> Row = MySQLFetchRow( Result );

                while( !Row.empty( ) )
                {
                    if(Row[0] != "") {
                        response += Row[0] + " (" + Row[1] + "/" + Row[2] + "), ";
                        num++;
                    }

                    Row = MySQLFetchRow( Result );
                }

                mysql_free_result( Result );
            }
            else
                *error = mysql_error( (MYSQL *)conn );

            if(num == 0) {
                response = "No games available";
            } else {
                response = response.substr(0, response.length() - 2);
            }

            return "Current Games: "+response;
        }
    }

    return "error";
}

uint32_t MySQLPWCheck( void *conn, string *error, uint32_t botid, string user )
{
    transform( user.begin( ), user.end( ), user.begin( ), ::tolower );
    string EscUser = MySQLEscapeString( conn, user );
    uint32_t IsPWUser = false;
    string Query = "SELECT `user_ppwd` FROM oh_users WHERE bnet_username = '" + EscUser + "' AND `user_bnet` = '2';";

    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
    {
        MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

        if( Result )
        {
            vector<string> Row = MySQLFetchRow( Result );

            if( Row.size( ) == 1 )
            {
                string Password = Row[0];
                if( Password == "" )
                    return 1;
                else
                    return 2;
            }
            else
                return 0;

            mysql_free_result( Result );
        }
        else
            *error = mysql_error( (MYSQL *)conn );
    }

    return 0;
}

uint32_t MySQLPassCheck( void *conn, string *error, uint32_t botid, string user, string pass, uint32_t st )
{
    transform( user.begin( ), user.end( ), user.begin( ), ::tolower );
    transform( pass.begin( ), pass.end( ), pass.begin( ), ::tolower );
    string EscUser = MySQLEscapeString( conn, user );
    string EscPass = MySQLEscapeString( conn, pass );
    if( st == 0 )
    {
        string Query = "SELECT `user_ppwd` FROM oh_users WHERE bnet_username = '" + EscUser + "' AND `user_bnet` = '2';";
        if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        {
            *error = mysql_error( (MYSQL *)conn );
            return 0;
        }
        else
        {
            MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );
            if (Result)
            {
                vector<string> Row = MySQLFetchRow( Result );

                if( Row.size( ) == 1 ) {
                    string Pass = Row[0];
                    if( Pass == EscPass )
                        return 1;
                    else
                        return 2;
                }
                else
                    return 3;

                mysql_free_result( Result );
            }
        }
    }

    if( st == 1 )
    {
        string Query = "UPDATE `oh_users` SET `user_ppwd` = '' WHERE bnet_username = '" + EscUser + "' AND `user_bnet` = '2';";
        if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        {
            *error = mysql_error( (MYSQL *)conn );
            return 0;
        }
        else
            return 4;
    }
    return 0;
}

uint32_t MySQLpm( void *conn, string *error, uint32_t botid, string user, string listener, uint32_t status, string message, string type )
{

    transform( user.begin( ), user.end( ), user.begin( ), ::tolower );
    transform( listener.begin( ), listener.end( ), listener.begin( ), ::tolower );
    string EscUser = MySQLEscapeString( conn, user );
    string EscListener = MySQLEscapeString( conn, listener );
    string EscMessage = MySQLEscapeString( conn, message );

    if( type == "add" )
    {
        string Query = "INSERT INTO `oh_pm` ( `m_from`, `m_to`, `m_time`, `m_read`, `m_message` ) VALUES ('" + EscUser + "', '" + EscListener + "', CURRENT_TIMESTAMP( ), '0', '" + EscMessage + "' );";
        if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );
        else
            return -1;
    }
    if( type == "join" )
    {
        string Query2 = "SELECT COUNT(*) FROM `oh_pm` WHERE `m_to` = '" + EscUser + "' AND `m_read` = '0';";
        if( mysql_real_query( (MYSQL *)conn, Query2.c_str( ), Query2.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );
        else
        {
            MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

            if( Result )
            {
                vector<string> Row = MySQLFetchRow( Result );

                if( !Row.empty( ) )
                {
                    return UTIL_ToUInt32( Row[0] );
                }

                mysql_free_result( Result );
            }
            else
                *error = mysql_error( (MYSQL *)conn );
        }

    }
    return 0;
}

vector<permission> MySQLPList( void *conn, string *error, uint32_t botid, string server )
{
    string EscServer = MySQLEscapeString( conn, server );

    vector<permission> PList;
    string Query = "SELECT `bnet_username`, `user_level`, `user_custom_permission`, `user_color`, `user_name` FROM oh_users WHERE `user_bnet` >= '1' AND ( user_level_expire > NOW( ) OR user_level_expire = '0000-00-00 00:00:00' OR user_level_expire = '' ) AND `admin_realm` = '" + EscServer + "' AND ( `user_level` > 0 OR `user_name` != '' )";

    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
    {
        MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

        if( Result )
        {
            vector<string> Row = MySQLFetchRow( Result );

            while( !Row.empty( ) )
            {
		permission newPlayer;
		newPlayer.player = Row[0];
		newPlayer.level = UTIL_ToUInt32( Row[1] );
		newPlayer.binaryPermissions = Row[2];
		newPlayer.coloredName = Row[3] + Row[4];
                PList.push_back(newPlayer);
                Row = MySQLFetchRow( Result );
            }

            mysql_free_result( Result );
        }
        else
            *error = mysql_error( (MYSQL *)conn );
    }
    return PList;
}

vector<string> MySQLFlameList( void *conn, string *error, uint32_t botid )
{
    vector<string> FlameList;
    string Query = "SELECT `field_value` FROM `oh_custom_fields` WHERE `field_id` = '1' AND	`field_name` = 'oh_badwords'";

    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
    {
        MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

        if( Result )
        {
            vector<string> Row = MySQLFetchRow( Result );

            if( !Row.empty( ) )
            {
                string Word;
                stringstream SS;
                SS << Row[0];
                while( SS >> Word )
                {
                    FlameList.push_back( Word );
                }
            }

            mysql_free_result( Result );
        }
        else
            *error = mysql_error( (MYSQL *)conn );
    }

    return FlameList;
}

vector<string> MySQLForcedGProxyList( void *conn, string *error, uint32_t botid )
{
    vector<string> ForcedGProxyList;
    string Query = "SELECT `player`, `ip` FROM oh_gproxy;";

    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
    {
        MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

        if( Result )
        {
            vector<string> Row = MySQLFetchRow( Result );

            while( !Row.empty( ) )
            {
                if(! Row[0].empty( ) )
                    ForcedGProxyList.push_back( Row[0] );
                if(! Row[1].empty( ) )
                    ForcedGProxyList.push_back( Row[1] );

                Row = MySQLFetchRow( Result );
            }

            mysql_free_result( Result );
        }
        else
            *error = mysql_error( (MYSQL *)conn );
    }

    return ForcedGProxyList;
}

vector<string> MySQLAliasList( void *conn, string *error, uint32_t botid )
{
    vector<string> AliasList;
    string Query = "SELECT `alias_name` FROM `oh_aliases` ORDER BY alias_id ASC;";

    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
    {
        MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

        if( Result )
        {
            vector<string> Row = MySQLFetchRow( Result );

            while( Row.size( ) == 1 )
            {
                AliasList.push_back(Row[0]);
                Row = MySQLFetchRow( Result );
            }

            mysql_free_result( Result );

        }
        else
            *error = mysql_error( (MYSQL *)conn );
    }

    return AliasList;
}

vector<string> MySQLDeniedNamesList( void *conn, string *error, uint32_t botid )
{
    vector<string> DeniedNamesList;
    string Query = "SELECT `field_value` FROM `oh_custom_fields` WHERE `field_id` = '1' AND	`field_name` = 'oh_bannednamepartials'";

    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
    {
        MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

        if( Result )
        {
            vector<string> Row = MySQLFetchRow( Result );

            if( !Row.empty( ) )
            {
                string Word;
                stringstream SS;
                SS << Row[0];
                while( SS >> Word )
                {
                    DeniedNamesList.push_back( Word );
                }
            }

            mysql_free_result( Result );
        }
        else
            *error = mysql_error( (MYSQL *)conn );
    }

    return DeniedNamesList;
}

vector<string> MySQLAnnounceList( void *conn, string *error, uint32_t botid )
{
    vector<string> AnnounceList;
    string Query = "SELECT `field_value` FROM `oh_custom_fields` WHERE `field_id` = '1' AND `field_name` = 'oh_announcements'";

    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
    {
        MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

        if( Result )
        {
            vector<string> Row = MySQLFetchRow( Result );

            if( !Row.empty( ) )
            {
                const char *row = Row[0].c_str();
                char delims = '\n';
                std::stringstream ss(row);
                std::string to;

                if( row != NULL )
                {
                    while(std::getline(ss,to,'\n'))
                    {
                        AnnounceList.push_back( to );
                    }
                }
            }

            mysql_free_result( Result );
        }
        else
            *error = mysql_error( (MYSQL *)conn );
    }

    return AnnounceList;
}

vector<string> MySQLDCountryList( void *conn, string *error, uint32_t botid )
{
    vector<string> DCountryList;
    string Query = "SELECT `field_value` FROM `oh_custom_fields` WHERE `field_id` = '1' AND `field_name` = 'oh_country_ban'";

    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
    {
        MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

        if( Result )
        {
            vector<string> Row = MySQLFetchRow( Result );

            if( !Row.empty( ) )
            {
                string CC;
                stringstream SS;
                SS << Row[0];
                while( SS >> CC )
                {
                    DCountryList.push_back( CC );
                }
            }

            mysql_free_result( Result );
        }
        else
            *error = mysql_error( (MYSQL *)conn );
    }

    return DCountryList;
}

uint32_t MySQLStoreLog( void *conn, string *error, uint32_t botid, uint32_t chatid, string game, vector<string> admin )
{
    uint32_t RowID = 0;
    string EscGameInfo = MySQLEscapeString( conn, game );

    if( EscGameInfo != "" ) {
        string GIQuery = "INSERT INTO oh_game_log ( `botid`, `gameid`, `log_time`, `log_data` ) VALUES ( '" + UTIL_ToString( botid ) + "', '" + UTIL_ToString( chatid ) + "', CURRENT_TIMESTAMP(), '" + EscGameInfo + "' );";
        if( mysql_real_query( (MYSQL *)conn, GIQuery.c_str( ), GIQuery.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );
        else
            RowID = mysql_insert_id( (MYSQL *)conn );
    }

    for( vector<string> :: iterator i = admin.begin( ); i != admin.end( ); ++i )
    {
        string Admin;
        string Rest;
        stringstream SS;
        SS << *i;
        SS >> Admin;
        if( !SS.eof( ) )
        {
            getline( SS, Rest );
            string :: size_type Start = Rest.find_first_not_of( " " );

            if( Start != string :: npos )
                Rest = Rest.substr( Start );
        }

        string EscAdmin = MySQLEscapeString( conn, Admin );
        string EscRest = MySQLEscapeString( conn, Rest );
        if( EscAdmin != "" && EscRest != "" )
        {
            string GIQuery = "INSERT INTO oh_adminlog ( `botid`, `gameid`, `log_time`, `log_admin`, `log_data` ) VALUES ( '" + UTIL_ToString( botid ) + "', '" + UTIL_ToString( chatid ) + "', CURRENT_TIMESTAMP(),'" + EscAdmin + "',  '" + EscRest + "' );";
            if( mysql_real_query( (MYSQL *)conn, GIQuery.c_str( ), GIQuery.size( ) ) != 0 )
                *error = mysql_error( (MYSQL *)conn );
            else
                RowID = mysql_insert_id( (MYSQL *)conn );
        }
    }

    admin.clear( );
    return RowID;
}

uint32_t MySQLgs( void *conn, string *error, uint32_t botid, uint32_t chatid, string gn, uint32_t st, uint32_t gametype, uint32_t gamealias )
{
    uint32_t RowID = 0;
    string EscGN = MySQLEscapeString( conn, gn );
    if( st == 1 ) {
        string CRQuery = "INSERT INTO oh_game_status ( `botid`, `gameid`, `gamestatus`, `gamename`, `gametime`, `gametype`, `alias_id` ) VALUES ( '" + UTIL_ToString( botid ) + "', '" + UTIL_ToString( chatid ) + "', 1, '" + EscGN + "', CURRENT_TIMESTAMP( ), '" + UTIL_ToString( gametype ) + "', '"+UTIL_ToString(gamealias)+"'  );";
        if( mysql_real_query( (MYSQL *)conn, CRQuery.c_str( ), CRQuery.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );
        else
            RowID = mysql_insert_id( (MYSQL *)conn );
    } else if( st == 2 ) {
        string UQuery = "UPDATE oh_game_status SET `gamestatus`='" + UTIL_ToString( st ) + "', `gametime` = CURRENT_TIMESTAMP( ), alias_Id ='"+UTIL_ToString(gamealias)+"' WHERE `gameid` = '" + UTIL_ToString( chatid ) + "' AND `botid` = '" + UTIL_ToString( botid ) + "';";
        if( mysql_real_query( (MYSQL *)conn, UQuery.c_str( ), UQuery.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );
        else
            RowID = mysql_insert_id( (MYSQL *)conn );
    } else {
        string DQuery = "DELETE FROM oh_game_status WHERE `gameid` = '" + UTIL_ToString( chatid ) + "' AND `botid` = '" + UTIL_ToString( botid ) + "';";
        if( mysql_real_query( (MYSQL *)conn, DQuery.c_str( ), DQuery.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );
        else
            RowID = mysql_insert_id( (MYSQL *)conn );
    }
    return RowID;
}

uint32_t MySQLpenp( void *conn, string *error, uint32_t botid, string name, string reason, string admin, uint32_t amount, string type )
{
    uint32_t Result = 0;
    transform( name.begin( ), name.end( ), name.begin( ), ::tolower );
    string EscName = MySQLEscapeString( conn, name );
    string EscReason = MySQLEscapeString( conn, reason );
    string EscAdmin = MySQLEscapeString( conn, admin );
    uint32_t RecentPP = 0;
    string CheckQuery = "SELECT SUM(pp) FROM `oh_game_offenses` WHERE `player_name` = '" + EscName + "';";
    if( mysql_real_query( (MYSQL *)conn, CheckQuery.c_str( ), CheckQuery.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
    {
        MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

        if( Result )
        {
            vector<string> Row = MySQLFetchRow( Result );

            while( Row.size( ) == 1 )
            {
                if( !Row[0].empty() )
                    RecentPP += UTIL_ToUInt32( Row[0] );

                Row = MySQLFetchRow( Result );
            }
            mysql_free_result( Result );
        }
        else
            *error = mysql_error( (MYSQL *)conn );
    }

    if( type == "check" )
        return RecentPP;

    else if( type == "add" )
    {
        string AddQuery = "INSERT INTO `oh_game_offenses` ( player_name, reason, offence_time, pp, admin ) VALUES ( '" + EscName + "', '" + EscReason + "', CURRENT_TIMESTAMP(), '" + UTIL_ToString( amount ) + "', '" + EscAdmin + "' ); ";
        //string StatsQ = "UPDATE `oh_stats` SET `penalty`='penalty+1' WHERE `player_lower` = '" + EscName + "';";
        //if( mysql_real_query( (MYSQL *)conn, StatsQ.c_str( ), StatsQ.size( ) ) != 0 )
        //    *error = mysql_error( (MYSQL *)conn );

        if( mysql_real_query( (MYSQL *)conn, AddQuery.c_str( ), AddQuery.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );
        else
        {
            uint32_t banamount = 1;
            if ( RecentPP<5 && RecentPP + amount >=5)
                banamount = 86400*7;
            if ( RecentPP<10 && RecentPP + amount >=10)
                banamount = 86400*14;
            if ( RecentPP<15 && RecentPP + amount >=15)
                banamount = 86400*30;
            if ( RecentPP<20 && RecentPP + amount >=20 || RecentPP > 20 )
                banamount = 0;

            if( banamount != 1 )
            {
                uint32_t AddBan = MySQLBanAdd( (MYSQL *)conn, error, botid, "", EscName, "", "", EscAdmin, "Too many penalty points", banamount, "" );
                return 2;
            }
            else
                return 1;
        }
    }
    return 0;
}

uint32_t MySQLBanCount( void *conn, string *error, uint32_t botid, string server )
{
    string EscServer = MySQLEscapeString( conn, server );
    uint32_t Count = 0;
    string Query = "SELECT COUNT(*) FROM oh_bans WHERE server='" + EscServer + "'";

    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
    {
        MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

        if( Result )
        {
            vector<string> Row = MySQLFetchRow( Result );

            if( Row.size( ) == 1 )
                Count = UTIL_ToUInt32( Row[0] );
            else
                *error = "error counting bans [" + server + "] - row doesn't have 1 column";

            mysql_free_result( Result );
        }
        else
            *error = mysql_error( (MYSQL *)conn );
    }
    return Count;
}

CDBBan *MySQLBanCheck( void *conn, string *error, uint32_t botid, string server, string user, string ip )
{
    transform( user.begin( ), user.end( ), user.begin( ), ::tolower );
    string EscServer = MySQLEscapeString( conn, server );
    string EscUser = MySQLEscapeString( conn, user );
    string EscIP = MySQLEscapeString( conn, ip );
    CDBBan *Ban = NULL;
    string Query;

    if( ip.empty( ) )
        Query = "SELECT id, name, ip, DATE(date), gamename, admin, reason, DATE(expiredate), TIMESTAMPDIFF(WEEK, NOW( ), expiredate) AS MONTH, TIMESTAMPDIFF(DAY, NOW( ), expiredate)-TIMESTAMPDIFF(WEEK, NOW( ), expiredate)*7 AS DAY, TIMESTAMPDIFF(HOUR ,NOW( ) ,expiredate)-TIMESTAMPDIFF(DAY, NOW( ),  expiredate)*24 AS HOUR, TIMESTAMPDIFF(MINUTE, NOW( ), expiredate)-TIMESTAMPDIFF(HOUR ,NOW( ) ,expiredate) *60 AS MINUTE FROM oh_bans WHERE name='" + EscUser + "' AND expiredate = '' OR expiredate='0000-00-00 00:00:00' OR expiredate>CURRENT_TIMESTAMP()";
    else
        Query = "SELECT id, name, ip, DATE(date), gamename, admin, reason, DATE(expiredate), TIMESTAMPDIFF(WEEK, NOW( ), expiredate) AS MONTH, TIMESTAMPDIFF(DAY, NOW( ), expiredate)-TIMESTAMPDIFF(WEEK, NOW( ), expiredate)*7 AS DAY, TIMESTAMPDIFF(HOUR ,NOW( ) ,expiredate)-TIMESTAMPDIFF(DAY, NOW( ),  expiredate)*24 AS HOUR, TIMESTAMPDIFF(MINUTE, NOW( ), expiredate)-TIMESTAMPDIFF(HOUR ,NOW( ) ,expiredate) *60 AS MINUTE FROM oh_bans WHERE name='" + EscUser + "' OR ip='" + EscIP + "' AND expiredate = '' OR expiredate='0000-00-00 00:00:00' OR expiredate>CURRENT_TIMESTAMP()";

    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
    {
        MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

        if( Result )
        {
            vector<string> Row = MySQLFetchRow( Result );

            if( Row.size( ) == 12 )
                Ban = new CDBBan( UTIL_ToUInt32( Row[0] ), server, Row[1], Row[2], Row[3], Row[4], Row[5], Row[6], Row[7], Row[8], Row[9], Row[10], Row[11], 0 );
            /* else
                *error = "error checking ban [" + server + " : " + user + "] - row doesn't have 6 columns"; */

            mysql_free_result( Result );
        }
        else
            *error = mysql_error( (MYSQL *)conn );
    }

    return Ban;
}

string MySQLBanCheck2( void *conn, string *error, uint32_t botid, string server, string user, string type )
{
    transform( user.begin( ), user.end( ), user.begin( ), ::tolower );
    string EscUser = MySQLEscapeString( conn, user );
    string GetIP = "SELECT `ip` FROM `oh_gameplayers` WHERE name = '" + EscUser + "' AND `ip` != '0' AND `ip` != '0.0.0.0' ORDER BY `id` DESC;";
    string UserIP = "";
    if( mysql_real_query( (MYSQL *)conn, GetIP.c_str( ), GetIP.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
    {
        MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );
        if( Result )
        {
            vector<string> Row = MySQLFetchRow( Result );
            if( Row.size( ) == 1 )
                UserIP = Row[0];
            mysql_free_result( Result );
        }
    }
    if( !UserIP.empty() )
    {
        uint32_t count = 0;
        std::vector<std::string> x = split2(UserIP, '.');
        string CheckIPRange = "SELECT `name` FROM `oh_bans` WHERE `ip_part` = '" + x[0] + "." + x[1] + "' GROUP BY `name`;";
        string AllNames = "";
        if( mysql_real_query( (MYSQL *)conn, CheckIPRange.c_str( ), CheckIPRange.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );
        else
        {
            MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

            if( Result )
            {
                vector<string> Row = MySQLFetchRow( Result );

                while( Row.size( ) == 1 )
                {
                    if( AllNames.empty() )
                        AllNames = Row[0];
                    else
                        AllNames += ", " + Row[0];
                    count += 1;
                    Row = MySQLFetchRow( Result );
                }

                mysql_free_result( Result );
            }
            else
                *error = mysql_error( (MYSQL *)conn );
        }
        if( type == "joincheck" )
            return UTIL_ToString( count );

        if( !AllNames.empty() )
            return AllNames;
    }
    if( UserIP.empty() )
        return "norec";

    return "fail";
}

uint32_t MySQLBanAdd( void *conn, string *error, uint32_t botid, string server, string user, string ip, string gamename, string admin, string reason, uint32_t bantime, string country )
{
    string EscServer = MySQLEscapeString( conn, server );
    string EscUser = MySQLEscapeString( conn, user );
    string EscIP = MySQLEscapeString( conn, ip );
    string EscGameName = MySQLEscapeString( conn, gamename );
    string EscAdmin = MySQLEscapeString( conn, admin );
    string EscReason = MySQLEscapeString( conn, reason );
    string EscCountry = MySQLEscapeString( conn, country );
    transform( EscUser.begin( ), EscUser.end( ), EscUser.begin( ), ::tolower );

    bool alreadybanned = false;
    uint32_t currentbantime = 0;
    string CheckQuery = "SELECT UNIX_TIMESTAMP( expiredate )-UNIX_TIMESTAMP() as currentbantime FROM oh_bans WHERE name = '" + EscUser + "';";
    if( mysql_real_query( (MYSQL *)conn, CheckQuery.c_str( ), CheckQuery.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
    {
        MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );
        if( Result )
        {
            vector<string> Row = MySQLFetchRow( Result );
            if( Row.size( ) == 1 )
            {
                currentbantime = UTIL_ToUInt32( Row[0] );
                if( currentbantime > 20000000 )
                    return 1;
                else if ( currentbantime > bantime && bantime != 0 )
                    return 2;
            }

            mysql_free_result( Result );
        }
    }

    if( ip.empty( ) || server.empty( ) )
    {
        EscIP = "0.0.0.0";
        string IPQuery = "SELECT `ip`,`spoofedrealm` FROM `oh_gameplayers` WHERE `name` = '" + EscUser + "' ORDER BY `id` DESC LIMIT 1;";
        if( mysql_real_query( (MYSQL *)conn, IPQuery.c_str( ), IPQuery.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );
        else
        {
            MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );
            if( Result )
            {
                vector<string> Row = MySQLFetchRow( Result );
                if( Row.size( ) == 2 )
                {
                    EscIP = Row[0];
                    EscServer = Row[1];
                }
                mysql_free_result( Result );
            }
        }
    }

    string InsertUserField = "UPDATE oh_stats_players SET banned='1' WHERE player_lower='"+EscUser+"'";
    mysql_real_query( (MYSQL *)conn, InsertUserField.c_str( ), InsertUserField.size( ) );

    if( EscReason.substr(0, 4)=="left" || EscReason.substr(0, 4)=="disc" )
    {
        string UpdateQuery = "UPDATE oh_stats_players SET last_leaver_time = FROM_UNIXTIME( UNIX_TIMESTAMP( ) + 604800 ), leaver_level=leaver_level+1 WHERE player_lower='"+EscUser+"'";
        mysql_real_query( (MYSQL *)conn, UpdateQuery.c_str( ), UpdateQuery.size( ) );
    }

    bool Success = false;
    std::vector<std::string> x = split2(EscIP, '.');

    if( country.empty( ) )
    {
        string FromQuery = "SELECT code FROM  `oh_geoip` WHERE INET_ATON('"+EscIP+"') BETWEEN ip_start_int AND ip_end_int";
        if( mysql_real_query( (MYSQL *)conn, FromQuery.c_str( ), FromQuery.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );
        else
        {
            MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );
            if( Result )
            {
                vector<string> Row = MySQLFetchRow( Result );
                if( Row.size( ) == 1 )
                    EscCountry = Row[0];

                mysql_free_result( Result );
            }
        }
    }

    if( currentbantime == 0 )
    {
        string Query = "";
        string OffenseQuery = "";
        if( bantime == 0 )
        {
            Query = "INSERT INTO oh_bans ( botid, server, name, ip, ip_part, date, gamename, admin, reason, country ) VALUES ( " + UTIL_ToString( botid ) + ", '" + EscServer + "', '" + EscUser + "', '" + EscIP + "', '" + x[0] + "." + x[1] + "', CURRENT_TIMESTAMP( ), '" + EscGameName + "', '" + EscAdmin + "', '" + EscReason + "', '" + EscCountry + "' )";
            OffenseQuery = "INSERT INTO oh_game_offenses ( player_name, reason, offence_time, pp, admin ) VALUES ( '" + EscUser + "', '" + EscReason + "', CURRENT_TIMESTAMP( ), 1, '" + EscAdmin + "' );";
        }
        else
        {
            Query = "INSERT INTO oh_bans ( botid, server, name, ip, ip_part,date, gamename, admin, reason, expiredate, country ) VALUES ( " + UTIL_ToString( botid ) + ", '" + EscServer + "', '" + EscUser + "', '" + EscIP + "', '" + x[0] + "." + x[1] + "', CURRENT_TIMESTAMP( ), '" + EscGameName + "', '" + EscAdmin + "', '" + EscReason + "', FROM_UNIXTIME( UNIX_TIMESTAMP( ) + " + UTIL_ToString(bantime) + "), '" + EscCountry + "' );";
            OffenseQuery = "INSERT INTO oh_game_offenses ( player_name, reason, offence_time, pp, admin, offence_expire ) VALUES ( '" + EscUser + "', '" + EscReason + "', CURRENT_TIMESTAMP( ), 1, '" + EscAdmin + "', FROM_UNIXTIME( UNIX_TIMESTAMP( ) + " + UTIL_ToString(bantime) + ") );";
        }

        if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );
        else
            Success = true;

        if( EscReason != "Too many penalty points" )
        {
            if( mysql_real_query( (MYSQL *)conn, OffenseQuery.c_str( ), OffenseQuery.size( ) ) != 0 )
                *error = mysql_error( (MYSQL *)conn );
        }

        return 3;
    }
    else
    {
        string Query = "";
        string OffenseQuery = "";
        if( bantime == 0 )
        {
            Query = "UPDATE oh_bans SET expiredate = '0000-00-00 00:00:00', reason = '"+EscReason+"' WHERE name = '"+EscUser+"';";
            OffenseQuery = "INSERT INTO oh_game_offenses ( player_name, reason, offence_time, pp, admin ) VALUES ( '" + EscUser + "', '" + EscReason + "', CURRENT_TIMESTAMP( ), 1, '" + EscAdmin + "' );";
        }
        else
        {
            Query = "UPDATE oh_bans SET expiredate = FROM_UNIXTIME( UNIX_TIMESTAMP( ) + " + UTIL_ToString(bantime) + "), reason = '" + EscReason + "' WHERE name = '"+EscUser+"';";
            OffenseQuery = "INSERT INTO oh_game_offenses ( player_name, reason, offence_time, pp, admin, offence_expire ) VALUES ( '" + EscUser + "', '" + EscReason + "', CURRENT_TIMESTAMP( ), 1, '" + EscAdmin + "', FROM_UNIXTIME( UNIX_TIMESTAMP( ) + " + UTIL_ToString(bantime) + ") );";
        }

        if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );
        else
            Success = true;

        if( EscReason != "Too many penalty points" )
        {
            if( mysql_real_query( (MYSQL *)conn, OffenseQuery.c_str( ), OffenseQuery.size( ) ) != 0 )
                *error = mysql_error( (MYSQL *)conn );
        }
        if( bantime == 0 )
            return 5;
        else
            return 4;
    }

    return 0;
}

bool MySQLPUp( void *conn, string *error, uint32_t botid, string name, uint32_t level, string realm, string user )
{
    bool Success = false;
    transform( user.begin( ), user.end( ), user.begin( ), ::tolower );
    transform( name.begin( ), name.end( ), name.begin( ), ::tolower );
    string EscName = MySQLEscapeString( conn, name );
    string EscRealm = MySQLEscapeString( conn, realm );
    string EscUser = MySQLEscapeString( conn, user );
    uint32_t time = 31120000;
    if( level == 6 || level == 5 )
        time = 15551000;
    if( level == 3 || level == 2 )
        time = 2592000;

    if( EscRealm == "Garena")
    {
        string CQuery = "SELECT `user_level` from `oh_users` WHERE `bnet_username` = '" + EscName + "' AND `admin_realm` = 'Garena';";
        if( mysql_real_query( (MYSQL *)conn, CQuery.c_str( ), CQuery.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );
        else
        {
            MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

            if( Result )
            {
                vector<string> Row = MySQLFetchRow( Result );
                if( Row.size( ) == 1 )
                {
                    string Query = "UPDATE `oh_users` SET `user_level` = '" + UTIL_ToString( level ) + "', `expire_date` = 'FROM_UNIXTIME( UNIX_TIMESTAMP( ) + " + UTIL_ToString(time) + ")' WHERE `bnet_username` = '" + EscName + "' AND `admin_realm` = 'Garena';";
                    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
                        *error = mysql_error( (MYSQL *)conn );
                    else
                        Success = true;
                    return true;
                } else
                    return false;

                mysql_free_result( Result );
            }
        }

    }

    string CQuery = "SELECT `user_level` from `oh_users` WHERE `bnet_username` = '" + EscName + "' AND `admin_realm` = '" + EscRealm + "';";
    if( mysql_real_query( (MYSQL *)conn, CQuery.c_str( ), CQuery.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
    {
        MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

        if( Result )
        {
            vector<string> Row = MySQLFetchRow( Result );

            if( Row.size( ) == 0 )
                return false;

            mysql_free_result( Result );
        }
    }

    string Query = "UPDATE `oh_users` SET `user_level` = '" + UTIL_ToString( level ) + "', `expire_date` = 'FROM_UNIXTIME( UNIX_TIMESTAMP( ) + " + UTIL_ToString(time) + ")' WHERE `bnet_username` = '" + EscName + "' AND `admin_realm` = '" + EscRealm + "';";

    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
        Success = true;

    return Success;
}

bool MySQLBanRemove( void *conn, string *error, uint32_t botid, string server, string user )
{
    transform( user.begin( ), user.end( ), user.begin( ), ::tolower );
    string EscServer = MySQLEscapeString( conn, server );
    string EscUser = MySQLEscapeString( conn, user );
    bool Success = false;
    string Query = "UPDATE oh_bans SET `expiredate` = CURRENT_TIMESTAMP WHERE server='" + EscServer + "' AND name='" + EscUser + "'";

    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
        Success = true;

    return Success;
}

bool MySQLBanRemove( void *conn, string *error, uint32_t botid, string user )
{
    transform( user.begin( ), user.end( ), user.begin( ), ::tolower );
    string EscUser = MySQLEscapeString( conn, user );
    bool Success = false;
    string Query = "DELETE FROM oh_bans WHERE name='" + EscUser + "' ORDER BY id ASC LIMIT 1";

    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
        Success = true;

    return Success;
}

bool MySQLTBRemove( void *conn, string *error, uint32_t botid, string server )
{
    bool Success = false;
    string Query = "DELETE FROM oh_bans WHERE expiredate <= CURRENT_TIMESTAMP() AND expiredate != '' AND expiredate!='0000-00-00 00:00:00'";

    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
        Success = true;

    return Success;
}

vector<CDBBan *> MySQLBanList( void *conn, string *error, uint32_t botid, string server )
{
    string EscServer = MySQLEscapeString( conn, server );
    vector<CDBBan *> BanList;
    string Query = "";
    if( EscServer == "Garena")
        Query = "SELECT id, name, ip, DATE(date), gamename, admin, reason, DATE(expiredate), TIMESTAMPDIFF(WEEK, NOW( ), expiredate) AS MONTH, TIMESTAMPDIFF(DAY, NOW( ), expiredate)-TIMESTAMPDIFF(WEEK, NOW( ), expiredate)*7 AS DAY, TIMESTAMPDIFF(HOUR ,NOW( ) ,expiredate)-TIMESTAMPDIFF(DAY, NOW( ),  expiredate)*24 AS HOUR, TIMESTAMPDIFF(MINUTE, NOW( ), expiredate)-TIMESTAMPDIFF(HOUR ,NOW( ) ,expiredate) *60 AS MINUTE FROM oh_bans WHERE server='" + EscServer + "' OR server='WC3Connect' AND expiredate = '' OR expiredate='0000-00-00 00:00:00' OR expiredate>CURRENT_TIMESTAMP()";
    else
        Query = "SELECT id, name, ip, DATE(date), gamename, admin, reason, DATE(expiredate), TIMESTAMPDIFF(WEEK, NOW( ), expiredate) AS MONTH, TIMESTAMPDIFF(DAY, NOW( ), expiredate)-TIMESTAMPDIFF(WEEK, NOW( ), expiredate)*7 AS DAY, TIMESTAMPDIFF(HOUR ,NOW( ) ,expiredate)-TIMESTAMPDIFF(DAY, NOW( ),  expiredate)*24 AS HOUR, TIMESTAMPDIFF(MINUTE, NOW( ), expiredate)-TIMESTAMPDIFF(HOUR ,NOW( ) ,expiredate) *60 AS MINUTE FROM oh_bans WHERE server='" + EscServer + "' AND expiredate = '' OR expiredate='0000-00-00 00:00:00' OR expiredate>CURRENT_TIMESTAMP()";

    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
    {
        MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

        if( Result )
        {
            vector<string> Row = MySQLFetchRow( Result );

            while( Row.size( ) == 12 )
            {
                BanList.push_back( new CDBBan( UTIL_ToUInt32( Row[0] ), server, Row[1], Row[2], Row[3], Row[4], Row[5], Row[6], Row[7], Row[8], Row[9], Row[10], Row[11], 0 ) );
                Row = MySQLFetchRow( Result );
            }

            mysql_free_result( Result );
        }
        else
            *error = mysql_error( (MYSQL *)conn );
    }

    return BanList;
}

vector<string> MySQLCommandList( void *conn, string *error, uint32_t botid )
{
    vector<string> CommandList;
    string Query = "SELECT command, id FROM oh_commands_queue WHERE ( botid='" + UTIL_ToString(botid) + "' OR botid='0' ) AND command != '' ORDER BY id ASC LIMIT 3;";

    vector<string> ids;
    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
    {
        MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

        if( Result )
        {
            vector<string> Row = MySQLFetchRow( Result );

            while( Row.size( ) == 2 )
            {
                CommandList.push_back( Row[0] );
                ids.push_back(Row[1]);
                Row = MySQLFetchRow( Result );
            }

            mysql_free_result( Result );
        }
        else
            *error = mysql_error( (MYSQL *)conn );
    }

    string ToDelete;
    for( vector<string> :: iterator i = ids.begin( ); i != ids.end( ); ++i) {
        if(!ToDelete.empty())
            ToDelete += ", '"+*i+"' ";
        else
            ToDelete += "'"+*i+"'";
    }
    if(ToDelete.size()>0) {
        string DeleteQuery = "DELETE FROM oh_commands WHERE id IN( " + ToDelete + " )";

        if( mysql_real_query( (MYSQL *)conn, DeleteQuery.c_str( ), DeleteQuery.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );
    }
    return CommandList;
}

uint32_t MySQLGameAdd( void *conn, string *error, uint32_t botid, string server, string map, string gamename, string ownername, uint32_t duration, uint32_t gamestate, string creatorname, string creatorserver, uint32_t gametype, vector<string> lobbylog, vector<string> gamelog, uint32_t databaseid, uint32_t lobbytime )
{
    uint32_t RowID = 0;
    string EscServer = MySQLEscapeString( conn, server );
    string EscMap = MySQLEscapeString( conn, map );
    string EscGameName = MySQLEscapeString( conn, gamename );
    string EscOwnerName = MySQLEscapeString( conn, ownername );
    string EscCreatorName = MySQLEscapeString( conn, creatorname );
    string EscCreatorServer = MySQLEscapeString( conn, creatorserver );
    string Query = 	"UPDATE oh_games SET botid="+UTIL_ToString(botid)+", server='" + EscServer + "',map='" + EscMap + "', datetime= NOW(), ownername='" + EscOwnerName + "', duration=" + UTIL_ToString( duration ) + ", gamestate=" + UTIL_ToString( gamestate ) + ", gametype= "+ UTIL_ToString( gametype ) + ", creatorname='" + EscCreatorName + "', creatorserver='" + EscCreatorServer + "', gamestatus = 1 WHERE id = "+UTIL_ToString(databaseid)+";";
    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
        RowID = mysql_insert_id( (MYSQL *)conn );

    if( databaseid != 0 )
    {
        string LobbyLog;
        for( vector<string> :: iterator i = lobbylog.begin( ); i != lobbylog.end( ); i++ )
            LobbyLog.append( (*i) + '\n' );

        string EscLobbyLog = MySQLEscapeString( conn, LobbyLog );

        string GameLog;
        for( vector<string> :: iterator i = gamelog.begin( ); i != gamelog.end( ); i++ )
            GameLog.append( (*i) + '\n' );

        string EscGameLog = MySQLEscapeString( conn, GameLog );

        string InsertQ = "INSERT INTO oh_lobby_game_logs ( gameid, botid, gametype, lobbylog, gamelog ) VALUES ( "+UTIL_ToString(databaseid)+", "+UTIL_ToString(botid)+", "+UTIL_ToString(gametype)+", '"+EscLobbyLog+"', '"+EscGameLog+"' )";

        if( mysql_real_query( (MYSQL *)conn, InsertQ.c_str( ), InsertQ.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );

        lobbylog.clear( );
        gamelog.clear( );
    }

    return databaseid;
}

uint32_t MySQLGameDBInit( void *conn, string *error, uint32_t botid, vector<CDBBan *> players, string gamename, uint32_t gameid, uint32_t gamealias )
{
    uint32_t RowID = 0;
    string EscGameName = MySQLEscapeString(conn, gamename);
    if(!EscGameName.empty() && gameid == 0)
    {
        string Query = "INSERT INTO oh_games (botid, gamename, gamestatus, datetime) VALUES ("+UTIL_ToString(botid)+", '"+gamename+"', 0, CURRENT_TIMESTAMP());";
        if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );
        else
            RowID = mysql_insert_id( (MYSQL *)conn );

        string newgamename = gamename+" #"+UTIL_ToString(RowID);
        string UpdateQuery = "UPDATE oh_games SET gamename='"+newgamename+"', alias_id='"+UTIL_ToString(gamealias)+"' WHERE id ='"+UTIL_ToString(RowID)+"';";
        if( mysql_real_query( (MYSQL *)conn, UpdateQuery.c_str( ), UpdateQuery.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );

        return RowID;
    }
    if(! gameid == 0 ) {
        string UQuery = "UPDATE oh_games SET alias_id='"+UTIL_ToString(gamealias)+"' WHERE id ='"+UTIL_ToString(gameid)+"';";
        if( mysql_real_query( (MYSQL *)conn, UQuery.c_str( ), UQuery.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );
    }

    for( vector<CDBBan *> :: iterator i = players.begin( ); i != players.end( ); i++ )
    {
        string PlayerQuery = "INSERT INTO oh_gameplayers (botid, gameid, name, ip, player_id) VALUES ("+UTIL_ToString(botid)+", "+UTIL_ToString(gameid)+", '"+(*i)->GetName()+"', '"+(*i)->GetIP( )+"', '"+(*i)->GetDate( )+"');";
        if( mysql_real_query( (MYSQL *)conn, PlayerQuery.c_str( ), PlayerQuery.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );
    }

    for( vector<CDBBan *> :: iterator i = players.begin( ); i != players.end( ); i++ )
        delete *i;
    players.clear( );

    return gameid;
}
string MySQLGameUpdate( void *conn, string *error, uint32_t botid, uint32_t hostcounter, uint32_t lobby, string map_type, uint32_t duration, string gamename, string ownername, string creatorname, string map, uint32_t players, uint32_t total, vector<PlayerOfPlayerList> playerlist )
{
    if( !gamename.empty( ) ) {
        string EscMapType = MySQLEscapeString( conn, map_type );
        string EscGameName = MySQLEscapeString( conn, gamename );
        string EscOwnerName = MySQLEscapeString( conn, ownername );
        string EscCreatorName = MySQLEscapeString( conn, creatorname );
        string EscMap = MySQLEscapeString( conn, map );
        string Users ="";
        string Splitter =",";
        string PlayerSlpitter="#";
        for( vector<PlayerOfPlayerList> :: iterator i = playerlist.begin( ); i != playerlist.end( ); ++i ) {
            Users += UTIL_ToString(i->Slot)+Splitter+UTIL_ToString(i->Team)+Splitter+UTIL_ToString(i->Color)+Splitter+i->Username+Splitter+i->Realm+Splitter+UTIL_ToString(i->Ping)+Splitter+i->IP+Splitter+UTIL_ToString(i->LeftTime)+Splitter+i->LeftReason+PlayerSlpitter;
        }
        string EscPlayerList = MySQLEscapeString( conn, Users );

        string Query = "INSERT INTO oh_gamelist (botid, gameid, lobby, map_type, gamename, ownername, creatorname, map) VALUES ('" + UTIL_ToString( botid ) + "', '" + UTIL_ToString( hostcounter ) + "', '" + UTIL_ToString( lobby ) + "', '" + EscMapType + "', '" + EscGameName + "', '" + EscOwnerName + "', '" + EscCreatorName + "', '" + EscMap + "') ON DUPLICATE KEY UPDATE lobby = '" + UTIL_ToString( lobby ) + "', duration = '" + UTIL_ToString( duration ) + "', ownername = '" + EscOwnerName + "', players = '" + UTIL_ToString( players ) + "', total = '" + UTIL_ToString( total ) + "', users = '" + EscPlayerList + "'";

        if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );
    } else {
        string Query = "DELETE FROM oh_gamelist WHERE botid = " + UTIL_ToString( botid ) + " AND ( gameid = " + UTIL_ToString( hostcounter ) + " OR lobby = 1 )";

        if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );
    }
        /*
    } else {
        string Query = "SELECT gamename,slotstaken,slotstotal,totalgames,totalplayers FROM oh_gamelist WHERE gamename LIKE '%"+EscGameName+"%'";

        if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );
        else
        {
            MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );
            string response;
            int num = 0;
            int totg = 0;
            int totp = 0;
            if( Result )
            {
                vector<string> Row = MySQLFetchRow( Result );

                while( !Row.empty( ) )
                {
                    if(Row[0] != "") {
                        totg += UTIL_ToUInt32( Row[3] );
                        totp += UTIL_ToUInt32( Row[4] );
                        response += Row[0] + " (" + Row[1] + "/" + Row[2] + "), ";
                        num++;
                    }

                    Row = MySQLFetchRow( Result );
                }

                mysql_free_result( Result );
            }
            else
                *error = mysql_error( (MYSQL *)conn );

            if(num == 0) {
                response = "No games available";
            } else {
                response = response.substr(0, response.length() - 2);
            }

            return "Current Games ["+UTIL_ToString( totg )+"|"+UTIL_ToString(totp)+"]: "+response;
        }

        return "";
    }*/
    return "";
}

uint32_t MySQLGamePlayerAdd( void *conn, string *error, uint32_t botid, uint32_t gameid, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t reserved, uint32_t loadingtime, uint32_t left, string leftreason, uint32_t team, uint32_t colour, uint32_t id )
{
    string EscName = MySQLEscapeString( conn, name );
    transform( name.begin( ), name.end( ), name.begin( ), ::tolower );
    string EscNameLOW = MySQLEscapeString( conn, name );
    uint32_t RowID = 0;
    string EscIP = MySQLEscapeString( conn, ip );
    string EscSpoofedRealm = MySQLEscapeString( conn, spoofedrealm );
    string EscLeftReason = MySQLEscapeString( conn, leftreason );
    string Query = "";
    if(gameid!=0) {
        Query = "UPDATE oh_gameplayers SET colour="+UTIL_ToString(colour)+", spoofed="+UTIL_ToString( spoofed )+", reserved="+UTIL_ToString( reserved )+", loadingtime="+UTIL_ToString( loadingtime )+", `left`="+UTIL_ToString( left )+", leftreason='"+EscLeftReason+"', team="+UTIL_ToString( team )+", spoofedrealm='"+EscSpoofedRealm+"' WHERE gameid='"+UTIL_ToString(gameid)+"' AND player_id='"+UTIL_ToString(id)+"';";
    } else {
        Query = "INSERT INTO oh_stats_players (player, player_lower, ip, realm, player_language) VALUES ('"+EscName+"','"+EscNameLOW+"','"+EscIP+"', '"+EscSpoofedRealm+"', 'en');";
    }

    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
        RowID = mysql_insert_id( (MYSQL *)conn );

    return RowID;
}

CDBGamePlayerSummary *MySQLGamePlayerSummaryCheck( void *conn, string *error, uint32_t botid, string name )
{
    transform( name.begin( ), name.end( ), name.begin( ), ::tolower );
    string EscName = MySQLEscapeString( conn, name );
    CDBGamePlayerSummary *GamePlayerSummary = NULL;
    string Query = "SELECT MIN(DATE(datetime)), MAX(DATE(datetime)), COUNT(*), MIN(loadingtime), AVG(loadingtime), MAX(loadingtime), MIN(`left`/duration)*100, AVG(`left`/duration)*100, MAX(`left`/duration)*100, MIN(duration), AVG(duration), MAX(duration) FROM oh_gameplayers LEFT JOIN oh_games ON oh_games.id=gameid WHERE name LIKE '" + EscName + "'";

    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
    {
        MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

        if( Result )
        {
            vector<string> Row = MySQLFetchRow( Result );

            if( Row.size( ) == 12 )
            {
                string FirstGameDateTime = Row[0];
                string LastGameDateTime = Row[1];
                uint32_t TotalGames = UTIL_ToUInt32( Row[2] );
                uint32_t MinLoadingTime = UTIL_ToUInt32( Row[3] );
                uint32_t AvgLoadingTime = UTIL_ToUInt32( Row[4] );
                uint32_t MaxLoadingTime = UTIL_ToUInt32( Row[5] );
                uint32_t MinLeftPercent = UTIL_ToUInt32( Row[6] );
                uint32_t AvgLeftPercent = UTIL_ToUInt32( Row[7] );
                uint32_t MaxLeftPercent = UTIL_ToUInt32( Row[8] );
                uint32_t MinDuration = UTIL_ToUInt32( Row[9] );
                uint32_t AvgDuration = UTIL_ToUInt32( Row[10] );
                uint32_t MaxDuration = UTIL_ToUInt32( Row[11] );
                GamePlayerSummary = new CDBGamePlayerSummary( string( ), name, FirstGameDateTime, LastGameDateTime, TotalGames, MinLoadingTime, AvgLoadingTime, MaxLoadingTime, MinLeftPercent, AvgLeftPercent, MaxLeftPercent, MinDuration, AvgDuration, MaxDuration );
            }
            else
                *error = "error checking gameplayersummary [" + name + "] - row doesn't have 12 columns";

            mysql_free_result( Result );
        }
        else
            *error = mysql_error( (MYSQL *)conn );
    }

    return GamePlayerSummary;
}

CDBStatsPlayerSummary *MySQLStatsPlayerSummaryCheck( void *conn, string *error, uint32_t botid, string name, string month, string year, uint32_t alias )
{
    string lowername = name;
    transform( lowername.begin( ), lowername.end( ), lowername.begin( ), ::tolower );
    string EscName = MySQLEscapeString( conn, name );
    string EscLowerName = MySQLEscapeString( conn, lowername );
    string EscMonth = MySQLEscapeString( conn, month);
    string EscYear = MySQLEscapeString( conn, year );
    CDBStatsPlayerSummary *StatsPlayerSummary = NULL;

    /* init values which should be returned */
    uint32_t id = 0;
    double score = 0;
    uint32_t games = 0;
    uint32_t wins = 0;
    uint32_t losses = 0;
    uint32_t draw = 0;
    uint32_t kills = 0;
    uint32_t deaths = 0;
    uint32_t assists = 0;
    uint32_t creeps = 0;
    uint32_t denies = 0;
    uint32_t neutrals = 0;
    uint32_t towers = 0;
    uint32_t rax = 0;
    uint32_t streak = 0;
    uint32_t maxstreak = 0;
    uint32_t losingstreak = 0;
    uint32_t maxlosingstreak = 0;
    uint32_t zerodeaths = 0;
    string realm = "unknown";
    uint32_t leaves = 0;
    bool hiddenacc = false;
    string country = "unknown";
    string countryCode = "??";
    uint32_t exp = 0;
    uint32_t allcount = 0;
    uint32_t rankcount = 0;
    uint32_t points;
    double reputation = 0;
    string languageSuffix = "en";
    uint32_t leaver_level = 0;
    bool update_leaver_level = false;

    string GlobalPlayerQuery = "SELECT id, realm, country, country_code, hide, exp, points, player_language, leaver_level, last_leaver_time<NOW() FROM oh_stats_players WHERE player_lower='"+EscLowerName+"'";
    if( mysql_real_query( (MYSQL *)conn, GlobalPlayerQuery.c_str( ), GlobalPlayerQuery.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
    {
        MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

        if( Result )
        {
            vector<string> Row = MySQLFetchRow( Result );

            if( Row.size( ) == 10 )
            {
                id = UTIL_ToUInt32(Row[0]);
                realm = Row[1];
                country = Row[2];
                countryCode = Row[3];
                hiddenacc = UTIL_ToUInt32(Row[4]);
                exp = UTIL_ToUInt32(Row[5]);
                points = UTIL_ToUInt32(Row[6]);
                languageSuffix = Row[7];
                leaver_level = UTIL_ToUInt32(Row[8]);
                update_leaver_level = UTIL_ToUInt32(Row[9]);
                if( update_leaver_level && leaver_level != 0 ) {
                    string UpdatePlayerQuery = "UPDATE oh_stats_players SET last_leaver_time = FROM_UNIXTIME( UNIX_TIMESTAMP( ) + 604800 ) , leaver_level=leaver_level-1 WHERE player_lower='"+EscLowerName+"'";
                    mysql_real_query( (MYSQL *)conn, UpdatePlayerQuery.c_str( ), UpdatePlayerQuery.size( ) );
                }
                mysql_free_result( Result );
            }
        }
        else
            *error = mysql_error( (MYSQL *)conn );
    }

    string StatsQuery = "";
    string StatsQueryCondition = "";
    if( alias != 0 ) {
        StatsQueryCondition += "`alias_id` = '"+UTIL_ToString(alias)+"' AND ";
    }

    if( !EscMonth.empty() && EscMonth != "0" && !EscYear.empty() && EscYear != "0" )
        StatsQueryCondition += "month='"+EscMonth+"' AND year='"+EscYear+"' AND";
    else if( !EscMonth.empty() && EscMonth != "0" && EscYear.empty())
        StatsQueryCondition += "month='"+EscMonth+"' AND year=YEAR(NOW()) AND";
    else if( EscMonth.empty() && EscYear.empty())
        StatsQueryCondition += "month=MONTH(NOW()) AND year=YEAR(NOW()) AND";
    else if( EscMonth == "0" && EscYear == "0" && alias == 0 )
        StatsQuery = "SELECT SUM(`score`), SUM(`games`), SUM(`wins`), SUM(`losses`), SUM(`draw`), SUM(`kills`), SUM(`deaths`), SUM(`assists`), SUM(`creeps`), SUM(`denies`), SUM(`neutrals`), SUM(`towers`), SUM(`rax`), MAX(`streak`), MAX(`maxstreak`), MAX(`losingstreak`), MAX(`maxlosingstreak`), MAX(`zerodeaths`), SUM(`leaver`) FROM oh_stats WHERE `pid` = '"+UTIL_ToString (id)+"';";
    else if( EscMonth == "0" && EscYear == "0" && alias != 0 )
        StatsQuery = "SELECT SUM(`score`), SUM(`games`), SUM(`wins`), SUM(`losses`), SUM(`draw`), SUM(`kills`), SUM(`deaths`), SUM(`assists`), SUM(`creeps`), SUM(`denies`), SUM(`neutrals`), SUM(`towers`), SUM(`rax`), MAX(`streak`), MAX(`maxstreak`), MAX(`losingstreak`), MAX(`maxlosingstreak`), MAX(`zerodeaths`), SUM(`leaver`) FROM oh_stats WHERE `pid` = '"+UTIL_ToString (id)+"' AND `alias_id` = '"+UTIL_ToString(alias)+"';";

    StatsQuery = "SELECT score, games, wins, losses, draw, kills, deaths, assists, creeps, denies, neutrals, towers, rax, streak, maxstreak, losingstreak, maxlosingstreak, zerodeaths, leaver FROM `oh_stats` WHERE "+StatsQueryCondition+" `pid` = '"+UTIL_ToString (id)+"';";

    if( mysql_real_query( (MYSQL *)conn, StatsQuery.c_str( ), StatsQuery.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
    {
        MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

        if( Result )
        {
            vector<string> Row = MySQLFetchRow( Result );

            if( Row.size( ) == 19 )
            {
                score = UTIL_ToDouble( Row[0] );
                games = UTIL_ToUInt32( Row[1] );
                wins = UTIL_ToUInt32( Row[2] );
                losses = UTIL_ToUInt32( Row[3] );
                draw = UTIL_ToUInt32( Row[4] );
                kills = UTIL_ToUInt32( Row[5] );
                deaths = UTIL_ToUInt32( Row[6] );
                assists = UTIL_ToUInt32( Row[7] );
                creeps = UTIL_ToUInt32( Row[8] );
                denies = UTIL_ToUInt32( Row[9] );
                neutrals = UTIL_ToUInt32( Row[10] );
                towers = UTIL_ToUInt32( Row[11] );
                rax = UTIL_ToUInt32( Row[12] );
                streak = UTIL_ToUInt32( Row[13] );
                maxstreak = UTIL_ToUInt32( Row[14] );
                losingstreak = UTIL_ToUInt32( Row[15] );
                maxlosingstreak = UTIL_ToUInt32( Row[16] );
                zerodeaths = UTIL_ToUInt32( Row[17] );
                leaves = UTIL_ToUInt32( Row[18] );
                if( score > 0 )
                {
                    string ALLQuery = "SELECT COUNT(*) FROM oh_stats WHERE alias_id='"+UTIL_ToString(alias)+"' AND month=MONTH(NOW()) AND year=YEAR(NOW());";
                    string CountQuery = "SELECT COUNT(*) FROM oh_stats WHERE score > '"+UTIL_ToString(score, 0)+"' AND alias_id='"+UTIL_ToString(alias)+"' AND month=MONTH(NOW()) AND year=YEAR(NOW());";
                    if( mysql_real_query( (MYSQL *)conn, ALLQuery.c_str( ), ALLQuery.size( ) ) != 0 )
                        *error = mysql_error( (MYSQL *)conn );
                    else
                    {
                        MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );
                        if( Result )
                        {
                            vector<string> Row = MySQLFetchRow( Result );
                            if( Row.size( ) == 1 )
                                allcount = UTIL_ToUInt32( Row[0] );
                        }
                    }
                    if( mysql_real_query( (MYSQL *)conn, CountQuery.c_str( ), CountQuery.size( ) ) != 0 )
                        *error = mysql_error( (MYSQL *)conn );
                    else
                    {
                        MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );
                        if( Result )
                        {
                            vector<string> Row = MySQLFetchRow( Result );
                            if( Row.size( ) == 1 )
                                rankcount = UTIL_ToUInt32( Row[0] );
                        }
                    }
                }
            }

            mysql_free_result( Result );
        }
        else
            *error = mysql_error( (MYSQL *)conn );
    }

    string ReputationQuery = "SELECT SUM(rate), COUNT(*) as reputation FROM oh_gameplayers_rating WHERE player='"+EscLowerName+"';";
    if( mysql_real_query( (MYSQL *)conn, ReputationQuery.c_str( ), ReputationQuery.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
    {
        MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

        if( Result )
        {
            vector<string> Row = MySQLFetchRow( Result );

            if(! Row[0].empty() && ! Row[1].empty())
            {
                reputation =  UTIL_ToDouble(Row[0]) / UTIL_ToDouble(Row[1]);
            }
        }
    }

    StatsPlayerSummary = new CDBStatsPlayerSummary( id, EscName, EscLowerName, score, games, wins, losses, draw, kills, deaths, assists, creeps, denies, neutrals, towers, rax, streak, maxstreak, losingstreak, maxlosingstreak, zerodeaths, realm, leaves, allcount, rankcount, hiddenacc, country, countryCode, exp, reputation, languageSuffix, leaver_level );

    return StatsPlayerSummary;
}

CDBInboxSummary *MySQLInboxSummaryCheck( void *conn, string *error, uint32_t botid, string name )
{
    string Res = string();
    transform( name.begin( ), name.end( ), name.begin( ), ::tolower );
    string EscName = MySQLEscapeString( conn, name );
    CDBInboxSummary *InboxSummary = NULL;
    string Query = "SELECT `id`, `m_from`, `m_message` FROM `oh_pm` WHERE `m_to` = '" + EscName + "';";

    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
    {
        MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

        if( Result )
        {
            vector<string> Row = MySQLFetchRow( Result );

            if( Row.size( ) == 3 )
            {
                uint32_t id = UTIL_ToUInt32( Row[0] );
                string User = Row[1];
                string Message = Row[2];
                InboxSummary = new CDBInboxSummary( User, Message );
                string Query3 = "DELETE FROM `oh_pm` WHERE `id` = '" + UTIL_ToString( id ) + "';";
                if( mysql_real_query( (MYSQL *)conn, Query3.c_str( ), Query3.size( ) ) != 0 )
                    *error = mysql_error( (MYSQL *)conn );

            }
            //else
                //*error = "error checking message [" + name + "] - row doesn't have 4 columns";

            mysql_free_result( Result );
        }
        else
            *error = mysql_error( (MYSQL *)conn );
    }
    return InboxSummary;
}

uint32_t MySQLDotAGameAdd( void *conn, string *error, uint32_t botid, uint32_t gameid, uint32_t winner, uint32_t min, uint32_t sec )
{
    uint32_t RowID = 0;
    string Query = "INSERT INTO oh_dotagames ( botid, gameid, winner, min, sec ) VALUES ( " + UTIL_ToString( botid ) + ", " + UTIL_ToString( gameid ) + ", " + UTIL_ToString( winner ) + ", " + UTIL_ToString( min ) + ", " + UTIL_ToString( sec ) + " )";

    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
        RowID = mysql_insert_id( (MYSQL *)conn );

    return RowID;
}

uint32_t MySQLDotAPlayerAdd( void *conn, string *error, uint32_t botid, uint32_t gameid, string data )
{
    string Query = "INSERT INTO oh_dotaplayers ( botid, gameid, colour, kills, deaths, creepkills, creepdenies, assists, gold, neutralkills, item1, item2, item3, item4, item5, item6, spell1, spell2, spell3, spell4, spell5, spell6, hero, newcolour, towerkills, raxkills, courierkills, level ) VALUES " + data + ";";
    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );

    return 1;
}

CDBDotAPlayerSummary *MySQLDotAPlayerSummaryCheck( void *conn, string *error, uint32_t botid, string name )
{
    transform( name.begin( ), name.end( ), name.begin( ), ::tolower );
    string EscName = MySQLEscapeString( conn, name );
    CDBDotAPlayerSummary *DotAPlayerSummary = NULL;
    string Query = "SELECT COUNT(oh_dotaplayers.id), SUM(kills), SUM(deaths), SUM(creepkills), SUM(creepdenies), SUM(assists), SUM(neutralkills), SUM(towerkills), SUM(raxkills), SUM(courierkills) FROM oh_gameplayers LEFT JOIN oh_games ON oh_games.id=oh_gameplayers.gameid LEFT JOIN oh_dotaplayers ON oh_dotaplayers.gameid=oh_games.id AND oh_dotaplayers.colour=oh_gameplayers.colour WHERE name LIKE '" + EscName + "'";

    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
    {
        MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

        if( Result )
        {
            vector<string> Row = MySQLFetchRow( Result );

            if( Row.size( ) == 10 )
            {
                uint32_t TotalGames = UTIL_ToUInt32( Row[0] );

                if( TotalGames > 0 )
                {
                    uint32_t TotalWins = 0;
                    uint32_t TotalLosses = 0;
                    uint32_t TotalKills = UTIL_ToUInt32( Row[1] );
                    uint32_t TotalDeaths = UTIL_ToUInt32( Row[2] );
                    uint32_t TotalCreepKills = UTIL_ToUInt32( Row[3] );
                    uint32_t TotalCreepDenies = UTIL_ToUInt32( Row[4] );
                    uint32_t TotalAssists = UTIL_ToUInt32( Row[5] );
                    uint32_t TotalNeutralKills = UTIL_ToUInt32( Row[6] );
                    uint32_t TotalTowerKills = UTIL_ToUInt32( Row[7] );
                    uint32_t TotalRaxKills = UTIL_ToUInt32( Row[8] );
                    uint32_t TotalCourierKills = UTIL_ToUInt32( Row[9] );

                    // calculate total wins

                    string Query2 = "SELECT COUNT(*) FROM oh_gameplayers LEFT JOIN oh_games ON oh_games.id=oh_gameplayers.gameid LEFT JOIN oh_dotaplayers ON oh_dotaplayers.gameid=oh_games.id AND oh_dotaplayers.colour=oh_gameplayers.colour LEFT JOIN oh_dotagames ON oh_games.id=oh_dotagames.gameid WHERE name='" + EscName + "' AND ((winner=1 AND dotaplayers.newcolour>=1 AND dotaplayers.newcolour<=5) OR (winner=2 AND dotaplayers.newcolour>=7 AND dotaplayers.newcolour<=11))";

                    if( mysql_real_query( (MYSQL *)conn, Query2.c_str( ), Query2.size( ) ) != 0 )
                        *error = mysql_error( (MYSQL *)conn );
                    else
                    {
                        MYSQL_RES *Result2 = mysql_store_result( (MYSQL *)conn );

                        if( Result2 )
                        {
                            vector<string> Row2 = MySQLFetchRow( Result2 );

                            if( Row2.size( ) == 1 )
                                TotalWins = UTIL_ToUInt32( Row2[0] );
                            else
                                *error = "error checking dotaplayersummary wins [" + name + "] - row doesn't have 1 column";

                            mysql_free_result( Result2 );
                        }
                        else
                            *error = mysql_error( (MYSQL *)conn );
                    }

                    // calculate total losses

                    string Query3 = "SELECT COUNT(*) FROM oh_gameplayers LEFT JOIN oh_games ON oh_games.id=oh_gameplayers.gameid LEFT JOIN oh_dotaplayers ON oh_dotaplayers.gameid=oh_games.id AND oh_dotaplayers.colour=oh_gameplayers.colour LEFT JOIN oh_dotagames ON oh_games.id=oh_dotagames.gameid WHERE name='" + EscName + "' AND ((winner=2 AND dotaplayers.newcolour>=1 AND dotaplayers.newcolour<=5) OR (winner=1 AND dotaplayers.newcolour>=7 AND dotaplayers.newcolour<=11))";

                    if( mysql_real_query( (MYSQL *)conn, Query3.c_str( ), Query3.size( ) ) != 0 )
                        *error = mysql_error( (MYSQL *)conn );
                    else
                    {
                        MYSQL_RES *Result3 = mysql_store_result( (MYSQL *)conn );

                        if( Result3 )
                        {
                            vector<string> Row3 = MySQLFetchRow( Result3 );

                            if( Row3.size( ) == 1 )
                                TotalLosses = UTIL_ToUInt32( Row3[0] );
                            else
                                *error = "error checking dotaplayersummary losses [" + name + "] - row doesn't have 1 column";

                            mysql_free_result( Result3 );
                        }
                        else
                            *error = mysql_error( (MYSQL *)conn );
                    }

                    // done

                    DotAPlayerSummary = new CDBDotAPlayerSummary( string( ), name, TotalGames, TotalWins, TotalLosses, TotalKills, TotalDeaths, TotalCreepKills, TotalCreepDenies, TotalAssists, TotalNeutralKills, TotalTowerKills, TotalRaxKills, TotalCourierKills );
                }
            }
            else
                *error = "error checking dotaplayersummary [" + name + "] - row doesn't have 10 columns";

            mysql_free_result( Result );
        }
        else
            *error = mysql_error( (MYSQL *)conn );
    }

    return DotAPlayerSummary;
}

bool MySQLDownloadAdd( void *conn, string *error, uint32_t botid, string map, uint32_t mapsize, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t downloadtime )
{
    bool Success = false;
    string EscMap = MySQLEscapeString( conn, map );
    string EscName = MySQLEscapeString( conn, name );
    string EscIP = MySQLEscapeString( conn, ip );
    string EscSpoofedRealm = MySQLEscapeString( conn, spoofedrealm );
    string Query = "INSERT INTO oh_downloads ( botid, map, mapsize, datetime, name, ip, spoofed, spoofedrealm, downloadtime ) VALUES ( " + UTIL_ToString( botid ) + ", '" + EscMap + "', " + UTIL_ToString( mapsize ) + ", NOW( ), '" + EscName + "', '" + EscIP + "', " + UTIL_ToString( spoofed ) + ", '" + EscSpoofedRealm + "', " + UTIL_ToString( downloadtime ) + " )";

    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
        Success = true;

    return Success;
}

double MySQLScoreCheck( void *conn, string *error, uint32_t botid, string category, string name, string server )
{
    transform( name.begin( ), name.end( ), name.begin( ), ::tolower );
    string EscCategory = MySQLEscapeString( conn, category );
    string EscName = MySQLEscapeString( conn, name );
    string EscServer = MySQLEscapeString( conn, server );
    double Score = -100000.0;
    string Query = "SELECT score FROM oh_stats WHERE player_lower='" + EscName + "' AND realm='" + EscServer + "'";

    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
    {
        MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

        if( Result )
        {
            vector<string> Row = MySQLFetchRow( Result );

            if( Row.size( ) == 1 )
                Score = UTIL_ToDouble( Row[0] );
            /* else
                *error = "error checking score [" + category + " : " + name + " : " + server + "] - row doesn't have 1 column"; */

            mysql_free_result( Result );
        }
        else
            *error = mysql_error( (MYSQL *)conn );
    }

    return Score;
}

bool MySQLConnectCheck( void *conn, string *error, uint32_t botid, string name, uint32_t sessionkey )
{
    transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
    string EscName = MySQLEscapeString( conn, name );
    bool Check = false;
    string Query = "SELECT sessionkey FROM wc3connect WHERE username='" + EscName + "' AND TIMESTAMPDIFF(HOUR, time, NOW()) < 10";

    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
    {
        MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

        if( Result )
        {
            vector<string> Row = MySQLFetchRow( Result );

            if( Row.size( ) == 1 )
            {
                if( UTIL_ToUInt32( Row[0] ) == sessionkey )
                    Check = true;
            }

            mysql_free_result( Result );
        }
        else
            *error = mysql_error( (MYSQL *)conn );
    }

    return Check;
}

uint32_t MySQLW3MMDPlayerAdd( void *conn, string *error, uint32_t botid, string category, uint32_t gameid, uint32_t pid, string name, string flag, uint32_t leaver, uint32_t practicing )
{
    transform( name.begin( ), name.end( ), name.begin( ), ::tolower );
    uint32_t RowID = 0;
    string EscCategory = MySQLEscapeString( conn, category );
    string EscName = MySQLEscapeString( conn, name );
    string EscFlag = MySQLEscapeString( conn, flag );
    string Query = "INSERT INTO oh_w3mmdplayers ( botid, category, gameid, pid, name, flag, leaver, practicing ) VALUES ( " + UTIL_ToString( botid ) + ", '" + EscCategory + "', " + UTIL_ToString( gameid ) + ", " + UTIL_ToString( pid ) + ", '" + EscName + "', '" + EscFlag + "', " + UTIL_ToString( leaver ) + ", " + UTIL_ToString( practicing ) + " )";

    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
        RowID = mysql_insert_id( (MYSQL *)conn );

    return RowID;
}

bool MySQLW3MMDVarAdd( void *conn, string *error, uint32_t botid, uint32_t gameid, map<VarP,int32_t> var_ints )
{
    if( var_ints.empty( ) )
        return false;

    bool Success = false;
    string Query;

    for( map<VarP,int32_t> :: iterator i = var_ints.begin( ); i != var_ints.end( ); ++i )
    {
        string EscVarName = MySQLEscapeString( conn, i->first.second );

        if( Query.empty( ) )
            Query = "INSERT INTO oh_w3mmdvars ( botid, gameid, pid, varname, value_int ) VALUES ( " + UTIL_ToString( botid ) + ", " + UTIL_ToString( gameid ) + ", " + UTIL_ToString( i->first.first ) + ", '" + EscVarName + "', " + UTIL_ToString( i->second ) + " )";
        else
            Query += ", ( " + UTIL_ToString( botid ) + ", " + UTIL_ToString( gameid ) + ", " + UTIL_ToString( i->first.first ) + ", '" + EscVarName + "', " + UTIL_ToString( i->second ) + " )";
    }

    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
        Success = true;

    return Success;
}

bool MySQLW3MMDVarAdd( void *conn, string *error, uint32_t botid, uint32_t gameid, map<VarP,double> var_reals )
{
    if( var_reals.empty( ) )
        return false;

    bool Success = false;
    string Query;

    for( map<VarP,double> :: iterator i = var_reals.begin( ); i != var_reals.end( ); ++i )
    {
        string EscVarName = MySQLEscapeString( conn, i->first.second );

        if( Query.empty( ) )
            Query = "INSERT INTO oh_w3mmdvars ( botid, gameid, pid, varname, value_real ) VALUES ( " + UTIL_ToString( botid ) + ", " + UTIL_ToString( gameid ) + ", " + UTIL_ToString( i->first.first ) + ", '" + EscVarName + "', " + UTIL_ToString( i->second, 10 ) + " )";
        else
            Query += ", ( " + UTIL_ToString( botid ) + ", " + UTIL_ToString( gameid ) + ", " + UTIL_ToString( i->first.first ) + ", '" + EscVarName + "', " + UTIL_ToString( i->second, 10 ) + " )";
    }

    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
        Success = true;

    return Success;
}

bool MySQLW3MMDVarAdd( void *conn, string *error, uint32_t botid, uint32_t gameid, map<VarP,string> var_strings )
{
    if( var_strings.empty( ) )
        return false;

    bool Success = false;
    string Query;

    for( map<VarP,string> :: iterator i = var_strings.begin( ); i != var_strings.end( ); ++i )
    {
        string EscVarName = MySQLEscapeString( conn, i->first.second );
        string EscValueString = MySQLEscapeString( conn, i->second );

        if( Query.empty( ) )
            Query = "INSERT INTO oh_w3mmdvars ( botid, gameid, pid, varname, value_string ) VALUES ( " + UTIL_ToString( botid ) + ", " + UTIL_ToString( gameid ) + ", " + UTIL_ToString( i->first.first ) + ", '" + EscVarName + "', '" + EscValueString + "' )";
        else
            Query += ", ( " + UTIL_ToString( botid ) + ", " + UTIL_ToString( gameid ) + ", " + UTIL_ToString( i->first.first ) + ", '" + EscVarName + "', '" + EscValueString + "' )";
    }

    if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
        *error = mysql_error( (MYSQL *)conn );
    else
        Success = true;

    return Success;
}

bool MySQLBotStatusCreate( void *conn, string *error, uint32_t botid, string username, string gamename, string ip, uint16_t hostport, string roc, string tft )
{
    string InsertNow = "INSERT INTO oh_bot_status (botid, name, gamename, ip, hostport, roc, tft, last_update) VALUES ('"+UTIL_ToString(botid)+"', '"+username+"','"+gamename+"', '"+ip+"','"+UTIL_ToString(hostport)+"','"+roc+"','"+tft+"', NOW()) ON DUPLICATE KEY UPDATE name='"+username+"',gamename='"+gamename+"',ip='"+ip+"',hostport='"+UTIL_ToString(hostport)+"', roc='"+roc+"', tft='"+tft+"', last_update=NOW()";
    mysql_real_query( (MYSQL *)conn, InsertNow.c_str( ), InsertNow.size( ) );
    return 0;
}

bool MySQLBotStatusUpdate( void *conn, string *error, uint32_t botid, string server, uint32_t status )
{
    if(server == "europe.battle.net" || server == "uswest.battle.net" || server == "useast.battle.net" ||server == "asia.battle.net" || server == "server.eurobattle.net") {
        string Query = "UPDATE oh_bot_status SET "+server+" = '"+UTIL_ToString(status)+"', last_update=NOW() WHERE botid ="+UTIL_ToString(botid);
        if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
            *error = mysql_error( (MYSQL *)conn );
    }
    return 0;
}

//
// MySQL Callables
//

void CMySQLCallable :: Init( )
{
    CBaseCallable :: Init( );

#ifndef WIN32
    // disable SIGPIPE since this is (or should be) a new thread and it doesn't inherit the spawning thread's signal handlers
    // MySQL should automatically disable SIGPIPE when we initialize it but we do so anyway here

    signal( SIGPIPE, SIG_IGN );
#endif

    mysql_thread_init( );

    if( !m_Connection )
    {
        if( !( m_Connection = mysql_init( NULL ) ) )
            m_Error = mysql_error( (MYSQL *)m_Connection );

        my_bool Reconnect = true;
        mysql_options( (MYSQL *)m_Connection, MYSQL_OPT_RECONNECT, &Reconnect );

        if( !( mysql_real_connect( (MYSQL *)m_Connection, m_SQLServer.c_str( ), m_SQLUser.c_str( ), m_SQLPassword.c_str( ), m_SQLDatabase.c_str( ), m_SQLPort, NULL, 0 ) ) )
            m_Error = mysql_error( (MYSQL *)m_Connection );
    }
    else if( mysql_ping( (MYSQL *)m_Connection ) != 0 )
        m_Error = mysql_error( (MYSQL *)m_Connection );
}

void CMySQLCallable :: Close( )
{
    mysql_thread_end( );

    CBaseCallable :: Close( );
}

void CMySQLCallableRegAdd :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLRegAdd( m_Connection, &m_Error, m_SQLBotID, m_User, m_Server, m_Mail, m_Password, m_Type );

    Close( );
}

void CMySQLCallableStatsSystem :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLStatsSystem( m_Connection, &m_Error, m_SQLBotID, m_User, m_Input, m_One, m_Type );

    Close( );
}

void CMySQLCallablePWCheck :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLPWCheck( m_Connection, &m_Error, m_SQLBotID, m_User );

    Close( );
}

void CMySQLCallablePassCheck :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLPassCheck( m_Connection, &m_Error, m_SQLBotID, m_User, m_Pass, m_ST );

    Close( );
}

void CMySQLCallablepm :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLpm( m_Connection, &m_Error, m_SQLBotID, m_User, m_Listener, m_Status, m_Message, m_Type );

    Close( );
}

void CMySQLCallablePList :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLPList( m_Connection, &m_Error, m_SQLBotID, m_Server );

    Close( );
}

void CMySQLCallableFlameList :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLFlameList( m_Connection, &m_Error, m_SQLBotID );

    Close( );
}

void CMySQLCallableForcedGProxyList :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLForcedGProxyList( m_Connection, &m_Error, m_SQLBotID );

    Close( );
}


void CMySQLCallableAliasList :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLAliasList( m_Connection, &m_Error, m_SQLBotID );

    Close( );
}

void CMySQLCallableDeniedNamesList :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLDeniedNamesList( m_Connection, &m_Error, m_SQLBotID );

    Close( );
}

void CMySQLCallableAnnounceList :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLAnnounceList( m_Connection, &m_Error, m_SQLBotID );

    Close( );
}

void CMySQLCallableDCountryList :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLDCountryList( m_Connection, &m_Error, m_SQLBotID );

    Close( );
}

void CMySQLCallableStoreLog :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        MySQLStoreLog( m_Connection, &m_Error, m_SQLBotID, m_ChatID, m_Game, m_Admin );

    Close( );
}

void CMySQLCallablegs :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        MySQLgs( m_Connection, &m_Error, m_SQLBotID, m_ChatID, m_GN, m_ST, m_GameType, m_GameAlias );

    Close( );
}

void CMySQLCallablepenp :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLpenp( m_Connection, &m_Error, m_SQLBotID, m_Name, m_Reason, m_Admin, m_Amount, m_Type );

    Close( );
}

void CMySQLCallableBanCount :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLBanCount( m_Connection, &m_Error, m_SQLBotID, m_Server );

    Close( );
}

void CMySQLCallableBanCheck :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLBanCheck( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User, m_IP );

    Close( );
}

void CMySQLCallableBanCheck2 :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLBanCheck2( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User, m_Type );

    Close( );
}

void CMySQLCallableBanAdd :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLBanAdd( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User, m_IP, m_GameName, m_Admin, m_Reason, m_BanTime, m_Country );

    Close( );
}

void CMySQLCallablePUp :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLPUp( m_Connection, &m_Error, m_SQLBotID, m_Name, m_Level, m_Realm, m_User );

    Close( );
}

void CMySQLCallableBanRemove :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
    {
        if( m_Server.empty( ) )
            m_Result = MySQLBanRemove( m_Connection, &m_Error, m_SQLBotID, m_User );
        else
            m_Result = MySQLBanRemove( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User );
    }

    Close( );
}

void CMySQLCallableBanList :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLBanList( m_Connection, &m_Error, m_SQLBotID, m_Server );

    Close( );
}

void CMySQLCallableTBRemove :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLTBRemove( m_Connection, &m_Error, m_SQLBotID, m_Server );

    Close( );
}

void CMySQLCallableCommandList :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLCommandList( m_Connection, &m_Error, m_SQLBotID );

    Close( );
}

void CMySQLCallableGameAdd :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLGameAdd( m_Connection, &m_Error, m_SQLBotID, m_Server, m_Map, m_GameName, m_OwnerName, m_Duration, m_GameState, m_CreatorName, m_CreatorServer, m_GameType, m_LobbyLog, m_GameLog, m_DatabaseID, m_LobbyTime );

    Close( );
}

void CMySQLCallableGameDBInit :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLGameDBInit( m_Connection, &m_Error, m_SQLBotID, m_Players, m_GameName, m_GameID, m_GameAlias );

    Close( );
}

void CMySQLCallableGameUpdate :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLGameUpdate( m_Connection, &m_Error, m_SQLBotID, m_Hostcounter, m_Lobby, m_MapType, m_Duration, m_GameName, m_OwnerName, m_CreatorName, m_Map, m_Players, m_Total, m_Playerlist );

    Close( );
}

void CMySQLCallableGamePlayerAdd :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLGamePlayerAdd( m_Connection, &m_Error, m_SQLBotID, m_GameID, m_Name, m_IP, m_Spoofed, m_SpoofedRealm, m_Reserved, m_LoadingTime, m_Left, m_LeftReason, m_Team, m_Colour, m_ID );

    Close( );
}

void CMySQLCallableGamePlayerSummaryCheck :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLGamePlayerSummaryCheck( m_Connection, &m_Error, m_SQLBotID, m_Name );

    Close( );
}

void CMySQLCallableStatsPlayerSummaryCheck :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLStatsPlayerSummaryCheck( m_Connection, &m_Error, m_SQLBotID, m_Name, m_Month, m_Year, m_GameAlias );

    Close( );
}

void CMySQLCallableInboxSummaryCheck :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLInboxSummaryCheck( m_Connection, &m_Error, m_SQLBotID, m_Name );

    Close( );
}

void CMySQLCallableDotAGameAdd :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLDotAGameAdd( m_Connection, &m_Error, m_SQLBotID, m_GameID, m_Winner, m_Min, m_Sec );

    Close( );
}

void CMySQLCallableDotAPlayerAdd :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLDotAPlayerAdd( m_Connection, &m_Error, m_SQLBotID, m_GameID, m_Data );

    Close( );
}

void CMySQLCallableDotAPlayerSummaryCheck :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLDotAPlayerSummaryCheck( m_Connection, &m_Error, m_SQLBotID, m_Name );

    Close( );
}

void CMySQLCallableDownloadAdd :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLDownloadAdd( m_Connection, &m_Error, m_SQLBotID, m_Map, m_MapSize, m_Name, m_IP, m_Spoofed, m_SpoofedRealm, m_DownloadTime );

    Close( );
}

void CMySQLCallableScoreCheck :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLScoreCheck( m_Connection, &m_Error, m_SQLBotID, m_Category, m_Name, m_Server );

    Close( );
}

void CMySQLCallableConnectCheck :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLConnectCheck( m_Connection, &m_Error, m_SQLBotID, m_Name, m_SessionKey );

    Close( );
}

void CMySQLCallableW3MMDPlayerAdd :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLW3MMDPlayerAdd( m_Connection, &m_Error, m_SQLBotID, m_Category, m_GameID, m_PID, m_Name, m_Flag, m_Leaver, m_Practicing );

    Close( );
}

void CMySQLCallableW3MMDVarAdd :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
    {
        if( m_ValueType == VALUETYPE_INT )
            m_Result = MySQLW3MMDVarAdd( m_Connection, &m_Error, m_SQLBotID, m_GameID, m_VarInts );
        else if( m_ValueType == VALUETYPE_REAL )
            m_Result = MySQLW3MMDVarAdd( m_Connection, &m_Error, m_SQLBotID, m_GameID, m_VarReals );
        else
            m_Result = MySQLW3MMDVarAdd( m_Connection, &m_Error, m_SQLBotID, m_GameID, m_VarStrings );
    }

    Close( );
}

void CMySQLCallableBotStatusCreate :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLBotStatusCreate ( m_Connection, &m_Error, m_SQLBotID, m_Username, m_Gamename, m_Ip, m_Hostport, m_Roc, m_Tft);

    Close( );
}

void CMySQLCallableBotStatusUpdate :: operator( )( )
{
    Init( );

    if( m_Error.empty( ) )
        m_Result = MySQLBotStatusUpdate( m_Connection, &m_Error, m_SQLBotID, m_Server, m_Status);

    Close( );
}
