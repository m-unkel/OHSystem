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

#ifndef GHOSTDBMYSQL_H
#define GHOSTDBMYSQL_H

/**************
 *** SCHEMA ***
 **************

CREATE TABLE bans (
    id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
    botid INT NOT NULL,
    server VARCHAR(100) NOT NULL,
    name VARCHAR(15) NOT NULL,
    ip VARCHAR(15) NOT NULL,
    date DATETIME NOT NULL,
    gamename VARCHAR(31) NOT NULL,
    admin VARCHAR(15) NOT NULL,
    reason VARCHAR(255) NOT NULL,
    l_lefttime VARCHAR(15) NOT NULL,
    l_reason VARCHAR(255) NOT NULL,
    l_gameid INT NOT NULL
)

CREATE TABLE games (
    id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
    botid INT NOT NULL,
    server VARCHAR(100) NOT NULL,
    map VARCHAR(100) NOT NULL,
    datetime DATETIME NOT NULL,
    gamename VARCHAR(31) NOT NULL,
    ownername VARCHAR(15) NOT NULL,
    duration INT NOT NULL,
    gamestate INT NOT NULL,
    creatorname VARCHAR(15) NOT NULL,
    creatorserver VARCHAR(100) NOT NULL
)

CREATE TABLE gameplayers (
    id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
    botid INT NOT NULL,
    gameid INT NOT NULL,
    name VARCHAR(15) NOT NULL,
    ip VARCHAR(15) NOT NULL,
    spoofed INT NOT NULL,
    reserved INT NOT NULL,
    loadingtime INT NOT NULL,
    `left` INT NOT NULL,
    leftreason VARCHAR(100) NOT NULL,
    team INT NOT NULL,
    colour INT NOT NULL,
    spoofedrealm VARCHAR(100) NOT NULL,
    INDEX( gameid )
)

CREATE TABLE dotagames (
    id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
    botid INT NOT NULL,
    gameid INT NOT NULL,
    winner INT NOT NULL,
    min INT NOT NULL,
    sec INT NOT NULL
)

CREATE TABLE dotaplayers (
    id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
    botid INT NOT NULL,
    gameid INT NOT NULL,
    colour INT NOT NULL,
    kills INT NOT NULL,
    deaths INT NOT NULL,
    creepkills INT NOT NULL,
    creepdenies INT NOT NULL,
    assists INT NOT NULL,
    gold INT NOT NULL,
    neutralkills INT NOT NULL,
    item1 CHAR(4) NOT NULL,
    item2 CHAR(4) NOT NULL,
    item3 CHAR(4) NOT NULL,
    item4 CHAR(4) NOT NULL,
    item5 CHAR(4) NOT NULL,
    item6 CHAR(4) NOT NULL,
    hero CHAR(4) NOT NULL,
    newcolour INT NOT NULL,
    towerkills INT NOT NULL,
    raxkills INT NOT NULL,
    courierkills INT NOT NULL,
    INDEX( gameid, colour )
)

CREATE TABLE downloads (
    id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
    botid INT NOT NULL,
    map VARCHAR(100) NOT NULL,
    mapsize INT NOT NULL,
    datetime DATETIME NOT NULL,
    name VARCHAR(15) NOT NULL,
    ip VARCHAR(15) NOT NULL,
    spoofed INT NOT NULL,
    spoofedrealm VARCHAR(100) NOT NULL,
    downloadtime INT NOT NULL
)

CREATE TABLE scores (
    id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
    category VARCHAR(25) NOT NULL,
    name VARCHAR(15) NOT NULL,
    server VARCHAR(100) NOT NULL,
    score REAL NOT NULL
)

CREATE TABLE w3mmdplayers (
    id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
    botid INT NOT NULL,
    category VARCHAR(25) NOT NULL,
    gameid INT NOT NULL,
    pid INT NOT NULL,
    name VARCHAR(15) NOT NULL,
    flag VARCHAR(32) NOT NULL,
    leaver INT NOT NULL,
    practicing INT NOT NULL
)

CREATE TABLE w3mmdvars (
    id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
    botid INT NOT NULL,
    gameid INT NOT NULL,
    pid INT NOT NULL,
    varname VARCHAR(25) NOT NULL,
    value_int INT DEFAULT NULL,
    value_real REAL DEFAULT NULL,
    value_string VARCHAR(100) DEFAULT NULL
)

 **************
 *** SCHEMA ***
 **************/

//
// COHBotDBMySQL
//
class COHBotDBMySQL : public COHBotDB
{
private:
    string m_Server;
    string m_Database;
    string m_User;
    string m_Password;
    uint16_t m_Port;
    uint32_t m_BotID;
    queue<void *> m_IdleConnections;
    uint32_t m_NumConnections;
    uint32_t m_OutstandingCallables;
    boost::mutex m_DatabaseMutex;
public:
    typedef map<string, uint16_t> calls;
    static calls outstandingCalls;
    static calls initMap() {
	calls ini = {};
	return ini;
    }
public:
    COHBotDBMySQL( CConfig *CFG );
    virtual ~COHBotDBMySQL( );

    virtual string GetStatus( );

    virtual void RecoverCallable( CBaseCallable *callable );

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
    virtual CCallableTBRemove *ThreadedTBRemove( string server );
    virtual CCallableBanRemove *ThreadedBanRemove( string user );
    virtual CCallableBanList *ThreadedBanList( string server );
    virtual CCallableCommandList *ThreadedCommandList(  );
    virtual CCallableGameAdd *ThreadedGameAdd( string server, string map, string gamename, string ownername, uint32_t duration, uint32_t gamestate, string creatorname, string creatorserver, uint32_t gametype, vector<string> lobbylog, vector<string> gamelog, uint32_t databaseid, uint32_t lobbytime );
    virtual CCallableGameDBInit *ThreadedGameDBInit( vector<CDBBan *> players, string gamename,uint32_t gameid, uint32_t gamealias );
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
    virtual CCallableConnectCheck *ThreadedConnectCheck( string name, uint32_t sessionkey );
    virtual CCallableW3MMDPlayerAdd *ThreadedW3MMDPlayerAdd( string category, uint32_t gameid, uint32_t pid, string name, string flag, uint32_t leaver, uint32_t practicing );
    virtual CCallableW3MMDVarAdd *ThreadedW3MMDVarAdd( uint32_t gameid, map<VarP,int32_t> var_ints );
    virtual CCallableW3MMDVarAdd *ThreadedW3MMDVarAdd( uint32_t gameid, map<VarP,double> var_reals );
    virtual CCallableW3MMDVarAdd *ThreadedW3MMDVarAdd( uint32_t gameid, map<VarP,string> var_strings );
    virtual CCallableBotStatusCreate *ThreadedBotStatusCreate( string username, string gamename, string ip, uint16_t hostport, string roc, string tft);
    virtual CCallableBotStatusUpdate *ThreadedBotStatusUpdate( string server, uint32_t status);

    // other database functions

    virtual void *GetIdleConnection( );
};

//
// global helper functions
//

uint32_t MySQLRegAdd( void *conn, string *error, uint32_t botid, string user, string server, string mail, string password, string type );
string MySQLStatsSystem( void *conn, string *error, uint32_t botid, string user, string input, uint32_t one, string type );
uint32_t MySQLPWCheck( void *conn, string *error, uint32_t botid, string user );
uint32_t MySQLPassCheck( void *conn, string *error, uint32_t botid, string user, string pass, uint32_t st );
uint32_t MySQLpm( void *conn, string *error, uint32_t botid, string user, string listener, uint32_t status, string message, string type );
vector<permission> MySQLPList( void *conn, string *error, uint32_t botid, string server );
vector<string> MySQLFlameList( void *conn, string *error, uint32_t botid );
vector<string> MySQLForcedGProxyList( void *conn, string *error, uint32_t botid );
vector<string> MySQLAliasList( void *conn, string *error, uint32_t botid );
vector<string> MySQLDeniedNamesList( void *conn, string *error, uint32_t botid );
vector<string> MySQLAnnounceList( void *conn, string *error, uint32_t botid );
vector<string> MySQLDCountryList( void *conn, string *error, uint32_t botid );
uint32_t MySQLStoreLog( void *conn, string *error, uint32_t botid, uint32_t chatid, string game, vector<string> admin );
uint32_t MySQLgs( void *conn, string *error, uint32_t botid, uint32_t chatid, string gn, uint32_t st, uint32_t gametype, uint32_t gamealias );
uint32_t MySQLpenp( void *conn, string *error, uint32_t botid, string name, string reason, string admin, uint32_t amount, string type );
uint32_t MySQLBanCount( void *conn, string *error, uint32_t botid, string server );
CDBBan *MySQLBanCheck( void *conn, string *error, uint32_t botid, string server, string user, string ip );
string MySQLBanCheck2( void *conn, string *error, uint32_t botid, string server, string user, string type );
uint32_t MySQLBanAdd( void *conn, string *error, uint32_t botid, string server, string user, string ip, string gamename, string admin, string reason, uint32_t bantime, string country );
bool MySQLPUp( void *conn, string *error, uint32_t botid, string name, uint32_t level, string realm, string user );
bool MySQLBanRemove( void *conn, string *error, uint32_t botid, string server, string user );
bool MySQLTBRemove( void *conn, string *error, uint32_t botid, string server );
bool MySQLBanRemove( void *conn, string *error, uint32_t botid, string user );
vector<CDBBan *> MySQLBanList( void *conn, string *error, uint32_t botid, string server );
vector<string> MySQLCommandList( void *conn, string *error, uint32_t botid );
uint32_t MySQLGameAdd( void *conn, string *error, uint32_t botid, string server, string map, string gamename, string ownername, uint32_t duration, uint32_t gamestate, string creatorname, string creatorserver, uint32_t gametype, vector<string> lobbylog, vector<string> gamelog, uint32_t databaseid, uint32_t lobbytime );
uint32_t MySQLGameDBInit( void *conn, string *error, uint32_t botid, vector<CDBBan *> players, string gamename, uint32_t gamealias );
string MySQLGameUpdate( void *conn, string *error, uint32_t botid, uint32_t hostcounter, uint32_t lobby, string map_type, uint32_t duration, string gamename, string ownername, string creatorname, string map, uint32_t players, uint32_t total, vector<PlayerOfPlayerList> playerlist );
uint32_t MySQLGamePlayerAdd( void *conn, string *error, uint32_t botid, uint32_t gameid, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t reserved, uint32_t loadingtime, uint32_t left, string leftreason, uint32_t team, uint32_t colour, uint32_t id );
CDBGamePlayerSummary *MySQLGamePlayerSummaryCheck( void *conn, string *error, uint32_t botid, string name );
CDBStatsPlayerSummary *MySQLStatsPlayerSummaryCheck( void *conn, string *error, uint32_t botid, string name, string month, string year, uint32_t alias );
CDBInboxSummary *MySQLInboxSummaryCheck( void *conn, string *error, uint32_t botid, string name );
uint32_t MySQLDotAGameAdd( void *conn, string *error, uint32_t botid, uint32_t gameid, uint32_t winner, uint32_t min, uint32_t sec );
uint32_t MySQLDotAPlayerAdd( void *conn, string *error, uint32_t botid, uint32_t gameid, string data );
CDBDotAPlayerSummary *MySQLDotAPlayerSummaryCheck( void *conn, string *error, uint32_t botid, string name );
bool MySQLDownloadAdd( void *conn, string *error, uint32_t botid, string map, uint32_t mapsize, string name, string ip, uint32_t spoofed, string spoofedrealm, uint32_t downloadtime );
double MySQLScoreCheck( void *conn, string *error, uint32_t botid, string category, string name, string server );
bool MySQLConnectCheck( void *conn, string *error, uint32_t botid, string name, uint32_t sessionkey );
uint32_t MySQLW3MMDPlayerAdd( void *conn, string *error, uint32_t botid, string category, uint32_t gameid, uint32_t pid, string name, string flag, uint32_t leaver, uint32_t practicing );
bool MySQLW3MMDVarAdd( void *conn, string *error, uint32_t botid, uint32_t gameid, map<VarP,int32_t> var_ints );
bool MySQLW3MMDVarAdd( void *conn, string *error, uint32_t botid, uint32_t gameid, map<VarP,double> var_reals );
bool MySQLW3MMDVarAdd( void *conn, string *error, uint32_t botid, uint32_t gameid, map<VarP,string> var_strings );
bool MySQLBotStatusCreate( string username, string gamename, string ip, uint16_t hostport, string roc, string tft);
bool MySQLBotStatusUpdate( string server, uint32_t status);


//
// MySQL Callables
//
class CMySQLCallable : virtual public CBaseCallable
{
protected:
    void *m_Connection;
    string m_SQLServer;
    string m_SQLDatabase;
    string m_SQLUser;
    string m_SQLPassword;
    uint16_t m_SQLPort;
    uint32_t m_SQLBotID;
    string m_CallableName;

public:
    CMySQLCallable( void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), m_Connection( nConnection ), m_SQLBotID( nSQLBotID ), m_SQLServer( nSQLServer ), m_SQLDatabase( nSQLDatabase ), m_SQLUser( nSQLUser ), m_SQLPassword( nSQLPassword ), m_SQLPort( nSQLPort), m_CallableName( nCallableName ) {

	map<string, uint16_t>::iterator i = COHBotDBMySQL::outstandingCalls.find(m_CallableName);
	if(i != COHBotDBMySQL::outstandingCalls.end())
		i->second = i->second+1;
	else
		COHBotDBMySQL::outstandingCalls.insert(make_pair(m_CallableName, 1));
    }

    virtual ~CMySQLCallable( ) {
        map<string, uint16_t>::iterator i = COHBotDBMySQL::outstandingCalls.find(m_CallableName);
        if(i != COHBotDBMySQL::outstandingCalls.end())
                i->second = i->second-1;
 	else
		Log->Debug("[MYSQL_DEBUG] Unknown Callable got recovered: " + m_CallableName );
    }

    virtual void *GetConnection( )	{
        return m_Connection;
    }

    virtual string GetOutstanding() {
        map<string, uint16_t>::iterator i = COHBotDBMySQL::outstandingCalls.find(m_CallableName);
	uint16_t out = 0;
        if(i != COHBotDBMySQL::outstandingCalls.end()) {
		out = i->second - 1;
		if(out!=0) {
			return "WARNING - UNRECOVERED CALLABLE DETECTED: ["+m_CallableName+"] with ["+UTIL_ToString(out)+"] callables";
		}
	}
	return "";	  
    }

    virtual void Init( );
    virtual void Close( );
};


class CMySQLCallableRegAdd : public CCallableRegAdd, public CMySQLCallable
{
public:
    CMySQLCallableRegAdd( string nUser, string nServer, string nMail, string nPassword, string nType, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableRegAdd( nUser, nServer, nMail, nPassword, nType ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableRegAdd( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableStatsSystem : public CCallableStatsSystem, public CMySQLCallable
{
public:
    CMySQLCallableStatsSystem( string nUser, string nInput, uint32_t nOne, string nType, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableStatsSystem( nUser, nInput, nOne, nType ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableStatsSystem( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallablePWCheck : public CCallablePWCheck, public CMySQLCallable
{
public:
    CMySQLCallablePWCheck( string nUser, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallablePWCheck( nUser ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallablePWCheck( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallablePassCheck : public CCallablePassCheck, public CMySQLCallable
{
public:
    CMySQLCallablePassCheck( string nUser, string nPass, uint32_t nST, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallablePassCheck( nUser, nPass, nST ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName) { }
    virtual ~CMySQLCallablePassCheck( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallablepm : public CCallablepm, public CMySQLCallable
{
public:
    CMySQLCallablepm( string nUser, string nListener, uint32_t nStatus, string nMessage, string nType, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallablepm( nUser, nListener, nStatus, nMessage, nType ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName) { }
    virtual ~CMySQLCallablepm( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallablePList : public CCallablePList, public CMySQLCallable
{
public:
    CMySQLCallablePList( string nServer, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallablePList( nServer ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallablePList( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableFlameList : public CCallableFlameList, public CMySQLCallable
{
public:
    CMySQLCallableFlameList( void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableFlameList( ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableFlameList( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableForcedGProxyList : public CCallableForcedGProxyList, public CMySQLCallable
{
public:
    CMySQLCallableForcedGProxyList( void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableForcedGProxyList( ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableForcedGProxyList( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableAliasList : public CCallableAliasList, public CMySQLCallable
{
public:
    CMySQLCallableAliasList( void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableAliasList( ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableAliasList( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableDeniedNamesList : public CCallableDeniedNamesList, public CMySQLCallable
{
public:
    CMySQLCallableDeniedNamesList( void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableDeniedNamesList( ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableDeniedNamesList( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableAnnounceList : public CCallableAnnounceList, public CMySQLCallable
{
public:
    CMySQLCallableAnnounceList( void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableAnnounceList( ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableAnnounceList( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableDCountryList : public CCallableDCountryList, public CMySQLCallable
{
public:
    CMySQLCallableDCountryList( void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableDCountryList( ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableDCountryList( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableStoreLog : public CCallableStoreLog, public CMySQLCallable
{
public:
    CMySQLCallableStoreLog( uint32_t nChatID, string nGame, vector<string> nAdmin, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableStoreLog( nChatID, nGame, nAdmin ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableStoreLog( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallablegs : public CCallablegs, public CMySQLCallable
{
public:
    CMySQLCallablegs( uint32_t nChatID, string nGN, uint32_t nST, uint32_t nGameType, uint32_t nGameAlias, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallablegs( nChatID, nGN, nST, nGameType, nGameAlias ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallablegs( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallablepenp : public CCallablepenp, public CMySQLCallable
{
public:
    CMySQLCallablepenp( string nName, string nReason, string nAdmin, uint32_t nAmount, string nType, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallablepenp( nName, nReason, nAdmin, nAmount, nType ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallablepenp( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableBanCount : public CCallableBanCount, public CMySQLCallable
{
public:
    CMySQLCallableBanCount( string nServer, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableBanCount( nServer ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableBanCount( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableBanCheck : public CCallableBanCheck, public CMySQLCallable
{
public:
    CMySQLCallableBanCheck( string nServer, string nUser, string nIP, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableBanCheck( nServer, nUser, nIP ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableBanCheck( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableBanCheck2 : public CCallableBanCheck2, public CMySQLCallable
{
public:
    CMySQLCallableBanCheck2( string nServer, string nUser, string nType, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableBanCheck2( nServer, nUser, nType ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableBanCheck2( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};


class CMySQLCallableBanAdd : public CCallableBanAdd, public CMySQLCallable
{
public:
    CMySQLCallableBanAdd( string nServer, string nUser, string nIP, string nGameName, string nAdmin, string nReason, uint32_t nBanTime, string nCountry, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableBanAdd( nServer, nUser, nIP, nGameName, nAdmin, nReason, nBanTime, nCountry ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableBanAdd( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallablePUp : public CCallablePUp, public CMySQLCallable
{
public:
    CMySQLCallablePUp( string nName, uint32_t nLevel, string nRealm, string nUser, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallablePUp( nName, nLevel, nRealm, nUser ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallablePUp( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableBanRemove : public CCallableBanRemove, public CMySQLCallable
{
public:
    CMySQLCallableBanRemove( string nServer, string nUser, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableBanRemove( nServer, nUser ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableBanRemove( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableTBRemove : public CCallableTBRemove, public CMySQLCallable
{
public:
    CMySQLCallableTBRemove( string nServer, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableTBRemove( nServer ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableTBRemove( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableBanList : public CCallableBanList, public CMySQLCallable
{
public:
    CMySQLCallableBanList( string nServer, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableBanList( nServer ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableBanList( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableCommandList : public CCallableCommandList, public CMySQLCallable
{
public:
    CMySQLCallableCommandList( void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableCommandList( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableGameAdd : public CCallableGameAdd, public CMySQLCallable
{
public:
    CMySQLCallableGameAdd( string nServer, string nMap, string nGameName, string nOwnerName, uint32_t nDuration, uint32_t nGameState, string nCreatorName, string nCreatorServer, uint32_t nGameType, vector<string> nLobbyLog, vector<string> nGameLog, uint32_t nDatabaseID, uint32_t lobbytime, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableGameAdd( nServer, nMap, nGameName, nOwnerName, nDuration, nGameState, nCreatorName, nCreatorServer, nGameType, nLobbyLog, nGameLog, nDatabaseID, lobbytime ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableGameAdd( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableGameDBInit : public CCallableGameDBInit, public CMySQLCallable
{
public:
    CMySQLCallableGameDBInit( vector<CDBBan *> nPlayers, string nGameName, uint32_t nGameID, uint32_t nGameAlias, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableGameDBInit( nPlayers, nGameName, nGameID, nGameAlias ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableGameDBInit( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableGameUpdate : public CCallableGameUpdate, public CMySQLCallable
{
public:
    CMySQLCallableGameUpdate( uint32_t hostcounter, uint32_t lobby, string map_type, uint32_t duration, string gamename, string ownername, string creatorname, string map, uint32_t players, uint32_t total, vector<PlayerOfPlayerList> playerlist, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableGameUpdate( hostcounter, lobby, map_type, duration, gamename, ownername, creatorname, map, players, total,  playerlist ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableGameUpdate( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableGamePlayerAdd : public CCallableGamePlayerAdd, public CMySQLCallable
{
public:
    CMySQLCallableGamePlayerAdd( uint32_t nGameID, string nName, string nIP, uint32_t nSpoofed, string nSpoofedRealm, uint32_t nReserved, uint32_t nLoadingTime, uint32_t nLeft, string nLeftReason, uint32_t nTeam, uint32_t nColour, uint32_t nID, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableGamePlayerAdd( nGameID, nName, nIP, nSpoofed, nSpoofedRealm, nReserved, nLoadingTime, nLeft, nLeftReason, nTeam, nColour, nID ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableGamePlayerAdd( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableGamePlayerSummaryCheck : public CCallableGamePlayerSummaryCheck, public CMySQLCallable
{
public:
    CMySQLCallableGamePlayerSummaryCheck( string nName, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableGamePlayerSummaryCheck( nName ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableGamePlayerSummaryCheck( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableStatsPlayerSummaryCheck : public CCallableStatsPlayerSummaryCheck, public CMySQLCallable
{
public:
    CMySQLCallableStatsPlayerSummaryCheck( string nName, string nMonth, string nYear, uint32_t nGameAlias, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableStatsPlayerSummaryCheck( nName, nMonth, nYear, nGameAlias ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableStatsPlayerSummaryCheck( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableInboxSummaryCheck : public CCallableInboxSummaryCheck, public CMySQLCallable
{
public:
    CMySQLCallableInboxSummaryCheck( string nName, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableInboxSummaryCheck( nName ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableInboxSummaryCheck( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableDotAGameAdd : public CCallableDotAGameAdd, public CMySQLCallable
{
public:
    CMySQLCallableDotAGameAdd( uint32_t nGameID, uint32_t nWinner, uint32_t nMin, uint32_t nSec, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableDotAGameAdd( nGameID, nWinner, nMin, nSec ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableDotAGameAdd( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableDotAPlayerAdd : public CCallableDotAPlayerAdd, public CMySQLCallable
{
public:
    CMySQLCallableDotAPlayerAdd( uint32_t nGameID, string nData, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableDotAPlayerAdd( nGameID, nData ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableDotAPlayerAdd( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableDotAPlayerSummaryCheck : public CCallableDotAPlayerSummaryCheck, public CMySQLCallable
{
public:
    CMySQLCallableDotAPlayerSummaryCheck( string nName, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableDotAPlayerSummaryCheck( nName ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableDotAPlayerSummaryCheck( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableDownloadAdd : public CCallableDownloadAdd, public CMySQLCallable
{
public:
    CMySQLCallableDownloadAdd( string nMap, uint32_t nMapSize, string nName, string nIP, uint32_t nSpoofed, string nSpoofedRealm, uint32_t nDownloadTime, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableDownloadAdd( nMap, nMapSize, nName, nIP, nSpoofed, nSpoofedRealm, nDownloadTime ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableDownloadAdd( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableScoreCheck : public CCallableScoreCheck, public CMySQLCallable
{
public:
    CMySQLCallableScoreCheck( string nCategory, string nName, string nServer, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableScoreCheck( nCategory, nName, nServer ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableScoreCheck( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableConnectCheck : public CCallableConnectCheck, public CMySQLCallable
{
public:
    CMySQLCallableConnectCheck( string nName, uint32_t nSessionKey, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableConnectCheck( nName, nSessionKey ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableConnectCheck( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableW3MMDPlayerAdd : public CCallableW3MMDPlayerAdd, public CMySQLCallable
{
public:
    CMySQLCallableW3MMDPlayerAdd( string nCategory, uint32_t nGameID, uint32_t nPID, string nName, string nFlag, uint32_t nLeaver, uint32_t nPracticing, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableW3MMDPlayerAdd( nCategory, nGameID, nPID, nName, nFlag, nLeaver, nPracticing ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableW3MMDPlayerAdd( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableW3MMDVarAdd : public CCallableW3MMDVarAdd, public CMySQLCallable
{
public:
    CMySQLCallableW3MMDVarAdd( uint32_t nGameID, map<VarP,int32_t> nVarInts, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableW3MMDVarAdd( nGameID, nVarInts ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    CMySQLCallableW3MMDVarAdd( uint32_t nGameID, map<VarP,double> nVarReals, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableW3MMDVarAdd( nGameID, nVarReals ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    CMySQLCallableW3MMDVarAdd( uint32_t nGameID, map<VarP,string> nVarStrings, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableW3MMDVarAdd( nGameID, nVarStrings ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableW3MMDVarAdd( ) { }

    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableBotStatusCreate : public CCallableBotStatusCreate, public CMySQLCallable
{
public:
    CMySQLCallableBotStatusCreate( string nUsername, string nGamename, string nIP, uint16_t nHostport, string nRoc, string nTft, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableBotStatusCreate(nUsername, nGamename, nIP, nHostport, nRoc, nTft), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableBotStatusCreate( ) { }
    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};

class CMySQLCallableBotStatusUpdate : public CCallableBotStatusUpdate, public CMySQLCallable
{
public:
    CMySQLCallableBotStatusUpdate( string nServer, uint32_t nStatus, void *nConnection, uint32_t nSQLBotID, string nSQLServer, string nSQLDatabase, string nSQLUser, string nSQLPassword, uint16_t nSQLPort, string nCallableName ) : CBaseCallable( ), CCallableBotStatusUpdate(nServer, nStatus), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort, nCallableName ) { }
    virtual ~CMySQLCallableBotStatusUpdate( ) { }
    virtual void operator( )( );
    virtual void Init( ) {
        CMySQLCallable :: Init( );
    }
    virtual void Close( ) {
        CMySQLCallable :: Close( );
    }
};
#endif
