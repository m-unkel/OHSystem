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
 *
 */

#include "ohbot.h"
#include "network/utils.h"
#include "utils/util.h"
#include "utils/crc32.h"
#include "utils/sha1.h"
#include "language.h"
#include "network/socket.h"
#include "database/ghostdb.h"
#include "database/ghostdbmysql.h"
#include "bnet/bnet.h"
#include "map.h"
#include "replay/packed.h"
#include "replay/replay.h"
#include "replay/savegame.h"
#include "game/gameplayer.h"
#include "game/gameprotocol.h"
#include "game/gameprotocol.h"
#include "gproxy/gpsprotocol.h"
#include "garena/gcbiprotocol.h"
#include "game/game_base.h"
#include "game/game.h"
#include "bnls/bnlsprotocol.h"
#include "bnls/bnlsclient.h"
#include "bnet/bnetprotocol.h"
#include "utils/bncsutilinterface.h"

#define __STORMLIB_SELF__
#include <stormlib/StormLib.h>
#include <boost/filesystem.hpp>

using namespace boost :: filesystem;

//
// COHBot
//

COHBot :: COHBot( CConfig *CFG )
{
    m_RunMode = MODE_ENABLED;

    m_DB = NULL;
    m_Language = NULL;
    m_Map = NULL;
    m_AdminMap = NULL;
    m_AutoHostMap = NULL;

    // enable lan mode ?
    if ( CFG->GetInt("lan_mode", 0) ) {

        SET(m_RunMode,MODE_LAN);

        // lan broadcast socket
        m_UDPSocket = new CUDPSocket( );
        m_UDPSocket->SetBroadcastTarget( CFG->GetString( "udp_broadcasttarget", string( ) ) );
        m_UDPSocket->SetDontRoute(CFG->GetInt("udp_dontroute", 0 ) != 0);

    }else{
        m_UDPSocket = NULL;
    }

    // enable garena mode ?
    if ( CFG->GetInt("garena_mode", 0) ) {

        SET(m_RunMode,MODE_GARENA);

        // garena socket
        m_GarenaSocket = new CUDPSocket( );
        m_GarenaSocket->SetBroadcastTarget( CFG->GetString( "garena_broadcasttarget", string( ) ) );
        m_GarenaSocket->SetDontRoute( true );

        // garena protocol
        m_GCBIProtocol = new CGCBIProtocol( );
    }else{
        m_GarenaSocket = NULL;
        m_GCBIProtocol = NULL;
    }

    // enable gproxy mode ?
    if ( CFG->GetInt("gproxy_mode", 0) ) {

        SET(m_RunMode,MODE_GPROXY);
        m_ReconnectPort = CFG->GetInt( "gproxy_port", 6114 );

        // gproxy socket
        m_ReconnectSocket = NULL;

        // gpsprotocol
        m_GPSProtocol = new CGPSProtocol( );
    }else{
        m_ReconnectSocket = NULL;
        m_GPSProtocol = NULL;
    }


    m_CRC = new CCRC32( );
    m_CRC->Initialize( );
    m_SHA = new CSHA1( );

    // database connection
    Log->Info( "[GHOST] opening primary database" );
    m_DB = new COHBotDBMySQL( CFG );

    // get a list of local IP addresses
    // this list is used elsewhere to determine if a player connecting to the bot is local or not
    Log->Info( "[GHOST] attempting to find local IP addresses" );
    GetLocalAddresses( &m_LocalAddresses );

    // initialize vars
    m_CurrentGame = NULL;
    m_FinishedGames = 0;
    m_CallableFlameList = NULL;
    m_CallableForcedGProxyList = NULL;
    m_CallableAliasList = NULL;
    m_CallableAnnounceList = NULL;
    m_CallableDCountryList = NULL;
    m_CallableCommandList = NULL;
    m_CallableDeniedNamesList = NULL;
    m_CallableHC = NULL;
    m_CheckForFinishedGames = 0;
    m_RanksLoaded = false;
    m_ReservedHostCounter = 0;
    m_TicksCollectionTimer = GetTicks();
    m_TicksCollection = 0;
    m_MaxTicks = 0;
    m_MinTicks = -1;
    m_Sampler = 0;

    isCreated = false;
    m_Version = "0.8 (Ghost 17.1, OHSystem 2)";

    m_AutoHostMaximumGames = CFG->GetInt( "autohost_maxgames", 0 );
    m_AutoHostAutoStartPlayers = CFG->GetInt( "autohost_startplayers", 0 );
    m_AutoHostGameName = CFG->GetString( "autohost_gamename", string( ) );
    m_AutoHostOwner = CFG->GetString( "autohost_owner", string( ) );
    m_AutoHostMatchMaking = false;
    m_AutoHostMinimumScore = 0.0;
    m_AutoHostMaximumScore = 0.0;

    m_LastAutoHostTime = GetTime( );
    m_LastCommandListTime = GetTime( );
    m_LastFlameListUpdate = 0;
    m_LastGProxyListUpdate=0;
    m_LastAliasListUpdate = 0;
    m_LastAnnounceListUpdate = 0;
    m_LastDNListUpdate = 0;
    m_LastDCountryUpdate = 0;
    m_LastHCUpdate = GetTime();

    m_Warcraft3Path = UTIL_AddPathSeperator( CFG->GetString( "bot_war3path", "C:\\Program Files\\Warcraft III\\" ) );

    m_BindAddress = CFG->GetString( "bot_bindaddress", string( ) );

    m_HostPort = CFG->GetInt( "bot_hostport", 6112 );
    m_DefaultMap = CFG->GetString( "bot_defaultmap", "map" );
    m_LANWar3Version = CFG->GetInt( "lan_war3version", 26 );
    m_ReplayWar3Version = CFG->GetInt( "replay_war3version", 26 );
    m_ReplayBuildNumber = CFG->GetInt( "replay_buildnumber", 6059 );
    m_GameIDReplays = CFG->GetInt( "bot_gameidreplays", 1 ) == 0 ? false : true;

    m_BotID = CFG->GetInt( "db_mysql_botid", 0 );
    
    SetConfigs( CFG );

    // load the battle.net connections
    // we're just loading the config data and creating the CBNET classes here, the connections are established later (in the Update function)
    int counter = 1;
    for( uint32_t i = 1; i < 10; ++i )
    {
        string Prefix;

        if( i == 1 )
            Prefix = "bnet_";
        else
            Prefix = "bnet" + UTIL_ToString( i ) + "_";

        string Server = CFG->GetString( Prefix + "server", string( ) );
        string ServerAlias = CFG->GetString( Prefix + "serveralias", string( ) );
        string CDKeyROC = CFG->GetString( Prefix + "cdkeyroc", string( ) );
        string CDKeyTFT = CFG->GetString( Prefix + "cdkeytft", string( ) );
        string CountryAbbrev = CFG->GetString( Prefix + "countryabbrev", "USA" );
        string Country = CFG->GetString( Prefix + "country", "United States" );
        string Locale = CFG->GetString( Prefix + "locale", "system" );
        uint32_t LocaleID;

        if( Locale == "system" )
        {
#ifdef WIN32
            LocaleID = GetUserDefaultLangID( );
#else
            LocaleID = 1033;
#endif
        }
        else
            LocaleID = UTIL_ToUInt32( Locale );

        string UserName = CFG->GetString( Prefix + "username", string( ) );
        string UserPassword = CFG->GetString( Prefix + "password", string( ) );
        string FirstChannel = CFG->GetString( Prefix + "firstchannel", "The Void" );
        string BNETCommandTrigger = CFG->GetString( Prefix + "commandtrigger", "!" );

        if( BNETCommandTrigger.empty( ) )
            BNETCommandTrigger = "!";

        bool HoldFriends = CFG->GetInt( Prefix + "holdfriends", 1 ) == 0 ? false : true;
        bool HoldClan = CFG->GetInt( Prefix + "holdclan", 1 ) == 0 ? false : true;
        bool PublicCommands = CFG->GetInt( Prefix + "publiccommands", 1 ) == 0 ? false : true;
        string BNLSServer = CFG->GetString( Prefix + "bnlsserver", string( ) );
        int BNLSPort = CFG->GetInt( Prefix + "bnlsport", 9367 );
        int BNLSWardenCookie = CFG->GetInt( Prefix + "bnlswardencookie", 0 );
        unsigned char War3Version = CFG->GetInt( Prefix + "custom_war3version", 26 );
        BYTEARRAY EXEVersion = UTIL_ExtractNumbers( CFG->GetString( Prefix + "custom_exeversion", string( ) ), 4 );
        BYTEARRAY EXEVersionHash = UTIL_ExtractNumbers( CFG->GetString( Prefix + "custom_exeversionhash", string( ) ), 4 );
        string PasswordHashType = CFG->GetString( Prefix + "custom_passwordhashtype", string( ) );
        string PVPGNRealmName = CFG->GetString( Prefix + "custom_pvpgnrealmname", "PvPGN Realm" );
        uint32_t MaxMessageLength = CFG->GetInt( Prefix + "custom_maxmessagelength", 200 );
        bool UpTime = CFG->GetInt( Prefix + "uptime", 0 );
	if(UpTime>180) {UpTime = 180;}

        if( Server.empty( ) )
            break;

        if( CDKeyROC.empty( ) )
        {
            Log->Write( "[GHOST] missing " + Prefix + "cdkeyroc, skipping this battle.net connection" );
            continue;
        }

        if( m_TFT && CDKeyTFT.empty( ) )
        {
            Log->Write( "[GHOST] missing " + Prefix + "cdkeytft, skipping this battle.net connection" );
            continue;
        }

        if( UserName.empty( ) )
        {
            Log->Write( "[GHOST] missing " + Prefix + "username, skipping this battle.net connection" );
            continue;
        }

        if( UserPassword.empty( ) )
        {
            Log->Write( "[GHOST] missing " + Prefix + "password, skipping this battle.net connection" );
            continue;
        }

        Log->Info( "[GHOST] found battle.net connection #" + UTIL_ToString( i ) + " for server [" + Server + "]" );

        if( Locale == "system" )
        {
#ifdef WIN32
            Log->Info( "[GHOST] using system locale of " + UTIL_ToString( LocaleID ) );
#else
            Log->Warning( "[GHOST] unable to get system locale, using default locale of 1033" );
#endif
        }

        m_BNETs.push_back( new CBNET( this, Server, ServerAlias, BNLSServer, (uint16_t)BNLSPort, (uint32_t)BNLSWardenCookie, CDKeyROC, CDKeyTFT, CountryAbbrev, Country, LocaleID, UserName, UserPassword, FirstChannel, BNETCommandTrigger[0], HoldFriends, HoldClan, PublicCommands, War3Version, EXEVersion, EXEVersionHash, PasswordHashType, PVPGNRealmName, MaxMessageLength, i, UpTime ) );
        counter++;
    }

    // @TODO hardcoded ??? What Server / IP / Port is used?
    Log->Info( "[GHOST] Adding hardcoded Garena Realm & WC3Connect Realm." );
    m_BNETs.push_back( new CBNET( this, "Garena", "Garena", string( ), 0, 0, string( ), string( ), string( ), string( ), 1033, string( ), string( ), string( ), m_CommandTrigger, 0, 0, 1, 26, UTIL_ExtractNumbers( string( ), 4 ), UTIL_ExtractNumbers( string( ), 4 ), string( ), string( ), 200, counter+1, 0 ) );
    m_BNETs.push_back( new CBNET( this, m_WC3ConnectAlias, "WC3Connect", string( ), 0, 0, string( ), string( ), string( ), string( ), 1033, string( ), string( ), string( ), m_CommandTrigger, 0, 0, 1, 26, UTIL_ExtractNumbers( string( ), 4 ), UTIL_ExtractNumbers( string( ), 4 ), string( ), string( ), 200, counter+2, 0 ) );

    if( m_BNETs.size( ) == 2 ) {
        Log->Warning( "[GHOST] no battle.net connections found in config file. Only the hardcoded" );
    } else {
        SET(m_RunMode,MODE_BNET);
    }

    // extract common.j and blizzard.j from War3Patch.mpq if we can
    // these two files are necessary for calculating "map_crc" when loading maps so we make sure to do it before loading the default map
    // see CMap :: Load for more information
    ExtractScripts( );

    CConfig *MapCFG = new CConfig();
    path MapCfgPath;

    //DefaultMap: Config Path
    if ( m_DefaultMap.size( ) < 4 || m_DefaultMap.substr( m_DefaultMap.size( ) - 4 ) != ".cfg" ) {
        MapCfgPath = path(m_MapCFGPath + m_DefaultMap + ".cfg");
        m_DefaultMap += ".cfg";
        Log->Info( "[GHOST] adding \".cfg\" to default map -> new default is [" + m_DefaultMap + "]" );
    } else
        MapCfgPath = path( m_MapCFGPath + m_DefaultMap );
    //DefaultMap: Map Path
    path MapPath( m_MapPath + m_DefaultMap );

    // MapCFG-File Found
    if( exists(MapCfgPath) ) {
        MapCFG->Read( MapCfgPath.string() );
    }
    // No MapCFG-File found, but a MapFile.. creating minimal config
    else if( exists(MapPath) ) {
        MapCFG->Set("map_path", "Maps\\Download\\" + m_DefaultMap);
        MapCFG->Set("map_localpath", m_DefaultMap);
    }
    else {
        m_DefaultMap = string();
    }

    //@TODO autohosting check if map is valid ?
    //if( m_DefaultMap.empty() ) {
    //    m_Map = NULL;
    //    m_AutoHostMap = NULL;
    //}else{
        m_Map = new CMap( this, MapCFG, m_MapCFGPath + m_DefaultMap );
        m_AutoHostMap = new CMap( *m_Map );
    //}
    delete MapCFG;

    m_SaveGame = new CSaveGame( );

    if( m_BNETs.empty( ) )
    {
        Log->Warning( "[GHOST] no battle.net connections found and no admin game created" );
    }

    //
    // Database CLEANUP
    //

    //reset games in DB for this BotID
    vector<PlayerOfPlayerList> pList;
    m_DB->GameUpdate( m_HostCounter, 0, "", 0, "", "", "", "", 0, 0, pList);

    Log->Info( "[GHOST] BE4MHost++ Version " + m_Version + " (MySQL)" );
}

COHBot :: ~COHBot( )
{
    // remove games
    if( m_CurrentGame )
        m_CurrentGame->doDelete();

    for( vector<CBaseGame *> :: iterator i = m_Games.begin( ); i != m_Games.end( ); ++i ) {
        (*i)->doDelete();
    }

    delete m_CRC;
    delete m_SHA;

    // remove battle net sockets
    for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); ++i )
        delete *i;

    // remove lan socket
    if( IS(m_RunMode,MODE_LAN) )
        delete m_UDPSocket;

    // remove garena socket and protocol
    if( IS(m_RunMode,MODE_GARENA) ) {
        delete m_GarenaSocket;
        delete m_GCBIProtocol;
    }

    // remove gproxy socket and protocol
    if( IS(m_RunMode,MODE_GPROXY) ) {

        for( vector<CTCPSocket *> :: iterator i = m_ReconnectSockets.begin( ); i != m_ReconnectSockets.end( ); ++i )
            delete *i;

        delete m_ReconnectSocket;
        delete m_GPSProtocol;
    }

    // warning: we don't delete any entries of m_Callables here because we can't be guaranteed that the associated threads have terminated
    // this is fine if the program is currently exiting because the OS will clean up after us
    // but if you try to recreate the COHBot object within a single session you will probably leak resources!

//	if( !m_Callables.empty( ) )
//		Log->Warning( "[GHOST] warning - " + UTIL_ToString( m_Callables.size( ) ) + " orphaned callables were leaked (this is not an error)" );


    delete m_DB;

    delete m_Language;
    delete m_Map;
    delete m_AdminMap;
    delete m_AutoHostMap;
    delete m_SaveGame;
}

bool COHBot :: Update( long usecBlock )
{
    m_StartTicks = GetTicks();

    // @TODO: do we really want to shutdown if there's a database error? is there any way to recover from this?
    // @TODO: catch block at uncritical cases ?

    //
    //  Test for database errors
    //
    if( m_DB->HasError( ) )
    {
        Log->Write( "[GHOST] database error - " + m_DB->GetError( ) );
        SET( m_RunMode, MODE_EXIT_NICELY);
    }

	boost::mutex::scoped_lock gamesLock( m_GamesMutex );


	//
    // Delete Games if ready to
    //
	for( vector<CBaseGame *> :: iterator i = m_Games.begin( ); i != m_Games.end( ); )
	{
		if( (*i)->readyDelete( ) )
		{
			delete *i;
			m_Games.erase( i );
		} else {
			++i;
		}
	}


    //
    // Delete CurrentGame if ready to
    //
	if( m_CurrentGame && m_CurrentGame->readyDelete( ) )
	{
	        for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); ++i )
        	{
            		(*i)->QueueGameUncreate( );
            		(*i)->QueueEnterChat( );
        	}

		delete m_CurrentGame;
		m_CurrentGame = NULL;
	}
	
	gamesLock.unlock( );


    //
    // try to exit nicely if requested to do so
    //
    if( IS(m_RunMode,MODE_EXIT_NICELY) )
    {
        if( !m_BNETs.empty( ) )
        {
            Log->Info( "[GHOST] deleting all battle.net connections in preparation for exiting nicely" );

            for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); ++i )
                delete *i;

            m_BNETs.clear( );
        }

        if( m_CurrentGame )
        {
            Log->Info( "[GHOST] deleting current game in preparation for exiting nicely" );
            m_CurrentGame->doDelete( );
            m_CurrentGame = NULL;
        }

        if( m_Games.empty( ) )
        {
            if( !m_AllGamesFinished )
            {
                Log->Info( "[GHOST] all games finished, waiting 60 seconds for threads to finish" );
                Log->Warning( "[GHOST] there are " + UTIL_ToString( m_Callables.size( ) ) + " threads in progress" );
                m_AllGamesFinished = true;
                m_AllGamesFinishedTime = GetTime( );
            }
            else
            {
                if( m_Callables.empty( ) )
                {
                    Log->Info( "[GHOST] all threads finished, exiting nicely" );
                    SET( m_RunMode, MODE_EXIT );
                }
                else if( GetTime( ) - m_AllGamesFinishedTime >= 60 )
                {
                    Log->Info( "[GHOST] waited 60 seconds for threads to finish, exiting anyway" );
                    Log->Warning( "[GHOST] there are " + UTIL_ToString( m_Callables.size( ) ) + " threads still in progress which will be terminated" );
                    SET( m_RunMode, MODE_EXIT );
                }
            }
        }
    }


    //
    // update callables
    //
    boost::mutex::scoped_lock callablesLock( m_CallablesMutex );
    for( vector<CBaseCallable *> :: iterator i = m_Callables.begin( ); i != m_Callables.end( ); )
    {
        if( (*i)->GetReady( ) )
        {
            m_DB->RecoverCallable( *i );
            delete *i;
            i = m_Callables.erase( i );
        }
        else
            ++i;
    }
    callablesLock.unlock( );


    //
    // create and test GProxy++ reconnect listener, if enabled
    //
    if( IS(m_RunMode,MODE_GPROXY) )
    {
        // not created ? then create server
        if( !m_ReconnectSocket )
        {
            m_ReconnectSocket = new CTCPServer( );

            if( m_ReconnectSocket->Listen( m_BindAddress, m_ReconnectPort ) )
                Log->Info( "[GHOST] listening for GProxy++ reconnects on port " + UTIL_ToString( m_ReconnectPort ) );
            else
            {
                Log->Write( "[GHOST] listening for GProxy++ reconnects FAILED on port " + UTIL_ToString( m_ReconnectPort ) );
                delete m_ReconnectSocket;
                m_ReconnectSocket = NULL;
                UNSET(m_RunMode,MODE_GPROXY);
            }
        }
        // has errors? stop reconnect listener
        else if( m_ReconnectSocket->HasError( ) )
        {
            Log->Write( "[GHOST] GProxy++ reconnect listener error (" + m_ReconnectSocket->GetErrorString( ) + ")" );
            delete m_ReconnectSocket;
            m_ReconnectSocket = NULL;
            UNSET(m_RunMode,MODE_GPROXY);
        }
    }

    unsigned int NumFDs = 0;

    // take every socket we own and throw it in one giant select statement so we can block on all sockets

    int nfds = 0;
    fd_set fd;
    fd_set send_fd;
    FD_ZERO( &fd );
    FD_ZERO( &send_fd );

    // 1. all battle.net sockets

    for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); ++i )
        NumFDs += (*i)->SetFD( &fd, &send_fd, &nfds );

    // 2. the GProxy++ reconnect socket(s)

    if( IS(m_RunMode,MODE_GPROXY) ) {

        if (m_ReconnectSocket) {
            m_ReconnectSocket->SetFD(&fd, &send_fd, &nfds);
            ++NumFDs;
        }

        for (vector<CTCPSocket *>::iterator i = m_ReconnectSockets.begin(); i != m_ReconnectSockets.end(); ++i) {
            (*i)->SetFD(&fd, &send_fd, &nfds);
            ++NumFDs;
        }
    }

    // before we call select we need to determine how long to block for
    // previously we just blocked for a maximum of the passed usecBlock microseconds
    // however, in an effort to make game updates happen closer to the desired latency setting we now use a dynamic block interval
    // note: we still use the passed usecBlock as a hard maximum

    for( vector<CBaseGame *> :: iterator i = m_Games.begin( ); i != m_Games.end( ); ++i )
    {
        if( (*i)->GetNextTimedActionTicks( ) * 1000 < usecBlock )
            usecBlock = (*i)->GetNextTimedActionTicks( ) * 1000;
    }

    // always block for at least 1ms just in case something goes wrong
    // this prevents the bot from sucking up all the available CPU if a game keeps asking for immediate updates
    // it's a bit ridiculous to include this check since, in theory, the bot is programmed well enough to never make this mistake
    // however, considering who programmed it, it's worthwhile to do it anyway

    if( usecBlock < 1000 )
        usecBlock = 1000;

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = usecBlock;

    struct timeval send_tv;
    send_tv.tv_sec = 0;
    send_tv.tv_usec = 0;

#ifdef WIN32
    select( 1, &fd, NULL, NULL, &tv );
    select( 1, NULL, &send_fd, NULL, &send_tv );
#else
    select( nfds + 1, &fd, NULL, NULL, &tv );
    select( nfds + 1, NULL, &send_fd, NULL, &send_tv );
#endif

    if( NumFDs == 0 )
    {
        // we don't have any sockets (i.e. we aren't connected to battle.net maybe due to a lost connection and there aren't any games running)
        // select will return immediately and we'll chew up the CPU if we let it loop so just sleep for 50ms to kill some time

        MILLISLEEP( 50 );
    }

    bool AdminExit = false;
    bool BNETExit = false;

    // update battle.net connections

    for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); ++i )
    {
        if( (*i)->Update( &fd, &send_fd ) )
            BNETExit = true;
    }


    //
    // update GProxy++ reliable reconnect sockets
    //
    // @TODO export GProxy++ to new classfile
    if ( IS(m_RunMode,MODE_GPROXY) ) {

        if (m_ReconnectSocket) {
            CTCPSocket *NewSocket = m_ReconnectSocket->Accept(&fd);

            if (NewSocket)
                m_ReconnectSockets.push_back(NewSocket);
        }

        for (vector<CTCPSocket *>::iterator i = m_ReconnectSockets.begin(); i != m_ReconnectSockets.end();) {
            if ((*i)->HasError() || !(*i)->GetConnected() || GetTime() - (*i)->GetLastRecv() >= 10) {
                delete *i;
                i = m_ReconnectSockets.erase(i);
                continue;
            }

            (*i)->DoRecv(&fd);
            string *RecvBuffer = (*i)->GetBytes();
            BYTEARRAY Bytes = UTIL_CreateByteArray((unsigned char *) RecvBuffer->c_str(), RecvBuffer->size());

            // a packet is at least 4 bytes

            if (Bytes.size() >= 4) {
                if (Bytes[0] == GPS_HEADER_CONSTANT) {
                    // bytes 2 and 3 contain the length of the packet

                    uint16_t Length = UTIL_ByteArrayToUInt16(Bytes, false, 2);

                    if (Length >= 4) {
                        if (Bytes.size() >= Length) {
                            if (Bytes[1] == CGPSProtocol::GPS_RECONNECT && Length == 13) {
                                GProxyReconnector *Reconnector = new GProxyReconnector;
                                Reconnector->PID = Bytes[4];
                                Reconnector->ReconnectKey = UTIL_ByteArrayToUInt32(Bytes, false, 5);
                                Reconnector->LastPacket = UTIL_ByteArrayToUInt32(Bytes, false, 9);
                                Reconnector->PostedTime = GetTicks();
                                Reconnector->socket = (*i);

                                // update the receive buffer
                                *RecvBuffer = RecvBuffer->substr(Length);
                                i = m_ReconnectSockets.erase(i);
                                // post in the reconnects buffer and wait to see if a game thread will pick it up
                                boost::mutex::scoped_lock lock(m_ReconnectMutex);
                                m_PendingReconnects.push_back(Reconnector);
                                lock.unlock();
                                continue;
                            }
                            else {
                                (*i)->PutBytes(m_GPSProtocol->SEND_GPSS_REJECT(REJECTGPS_INVALID));
                                (*i)->DoSend(&send_fd);
                                delete *i;
                                i = m_ReconnectSockets.erase(i);
                                continue;
                            }
                        }
                    }
                    else {
                        (*i)->PutBytes(m_GPSProtocol->SEND_GPSS_REJECT(REJECTGPS_INVALID));
                        (*i)->DoSend(&send_fd);
                        delete *i;
                        i = m_ReconnectSockets.erase(i);
                        continue;
                    }
                }
                else {
                    (*i)->PutBytes(m_GPSProtocol->SEND_GPSS_REJECT(REJECTGPS_INVALID));
                    (*i)->DoSend(&send_fd);
                    delete *i;
                    i = m_ReconnectSockets.erase(i);
                    continue;
                }
            }

            (*i)->DoSend(&send_fd);
            ++i;
        }


        // delete any old pending reconnects that have not been handled by games
        if( !m_PendingReconnects.empty( ) ) {
            boost::mutex::scoped_lock lock( m_ReconnectMutex );

            for( vector<GProxyReconnector *> :: iterator i = m_PendingReconnects.begin( ); i != m_PendingReconnects.end( ); )
            {
                if( GetTicks( ) - (*i)->PostedTime > 1500 )
                {
                    (*i)->socket->PutBytes( m_GPSProtocol->SEND_GPSS_REJECT( REJECTGPS_NOTFOUND ) );
                    (*i)->socket->DoSend( &send_fd );
                    delete (*i)->socket;
                    delete (*i);
                    i = m_PendingReconnects.erase( i );
                    continue;
                }

                i++;
            }

            lock.unlock();
        }
    } // IS(m_RunMode,MODE_GPROXY)


    //
    // GAME: Autohost
    //
    if( !m_AutoHostGameName.empty( ) && m_AutoHostMaximumGames != 0 && m_AutoHostAutoStartPlayers != 0 && GetTime( ) - m_LastAutoHostTime >= 30 && m_ReservedHostCounter != 0 )
    {
        // copy all the checks from COHBot :: CreateGame here because we don't want to spam the chat when there's an error
        // instead we fail silently and try again soon

        if( IS_NOT(m_RunMode,MODE_EXIT_NICELY) && IS(m_RunMode,MODE_ENABLED) && !m_CurrentGame && m_Games.size( ) < m_MaxGames && m_Games.size( ) < m_AutoHostMaximumGames )
        {
            if( m_AutoHostMap->GetValid( ) )
            {
                string GameName = m_AutoHostGameName + " #" + UTIL_ToString( GetNewHostCounter( ) );

                if( GameName.size( ) <= 31 )
                {
                    CreateGame( m_AutoHostMap, GAME_PUBLIC, false, GameName, m_AutoHostOwner, m_AutoHostOwner, m_AutoHostServer, m_AutoHostGameType, false, m_HostCounter );

                    if( m_CurrentGame )
                    {
                        if( m_ObserverFake )
                            m_CurrentGame->CreateFakePlayer( );
                        m_CurrentGame->SetAutoStartPlayers( m_AutoHostAutoStartPlayers );

                        if( m_AutoHostMatchMaking )
                        {
                            if( !m_Map->GetMapMatchMakingCategory( ).empty( ) )
                            {
                                if( !( m_Map->GetMapOptions( ) & MAPOPT_FIXEDPLAYERSETTINGS ) )
                                    Log->Warning( "[GHOST] autohostmm - map_matchmakingcategory [" + m_Map->GetMapMatchMakingCategory( ) + "] found but matchmaking can only be used with fixed player settings, matchmaking disabled" );
                                else
                                {
                                    Log->Info( "[GHOST] autohostmm - map_matchmakingcategory [" + m_Map->GetMapMatchMakingCategory( ) + "] found, matchmaking enabled" );

                                    m_CurrentGame->SetMatchMaking( true );
                                    m_CurrentGame->SetMinimumScore( m_AutoHostMinimumScore );
                                    m_CurrentGame->SetMaximumScore( m_AutoHostMaximumScore );
                                }
                            }
                            else
                                Log->Warning( "[GHOST] autohostmm - map_matchmakingcategory not found, matchmaking disabled" );
                        }
                    }
                }
                else
                {
                    Log->Warning( "[GHOST] stopped auto hosting, next game name [" + GameName + "] is too long (the maximum is 31 characters)" );
                    m_AutoHostGameName.clear( );
                    m_AutoHostOwner.clear( );
                    m_AutoHostServer.clear( );
                    m_AutoHostMaximumGames = 0;
                    m_AutoHostAutoStartPlayers = 0;
                    m_AutoHostMatchMaking = false;
                    m_AutoHostMinimumScore = 0.0;
                    m_AutoHostMaximumScore = 0.0;
                }
            }
            else
            {
                Log->Warning( "[GHOST] stopped auto hosting, map config file [" + m_AutoHostMap->GetCFGFile( ) + "] is invalid" );
                m_AutoHostGameName.clear( );
                m_AutoHostOwner.clear( );
                m_AutoHostServer.clear( );
                m_AutoHostMaximumGames = 0;
                m_AutoHostAutoStartPlayers = 0;
                m_AutoHostMatchMaking = false;
                m_AutoHostMinimumScore = 0.0;
                m_AutoHostMaximumScore = 0.0;
            }
        } else {
            stringstream ss;
            int a = IS_NOT(m_RunMode,MODE_EXIT_NICELY)?1:0;
            int b = IS(m_RunMode,MODE_ENABLED)?1:0;
            ss << "mode:" << a << b << " games:" << m_Games.size( ) << " maxGames:" << m_MaxGames << " maxAutoHostGames:" << m_AutoHostMaximumGames;
            Log->Debug( "[GHOST] autohost error: " + ss.str() );
        }

        m_LastAutoHostTime = GetTime( );
    }

    // refresh flamelist all 60 minutes
    if( m_FlameCheck && !m_CallableFlameList && ( GetTime( ) - m_LastFlameListUpdate >= 1200 || m_LastFlameListUpdate==0 ) )
    {
        m_CallableFlameList = m_DB->ThreadedFlameList( );
        m_LastFlameListUpdate = GetTime( );
    }

    if( m_CallableFlameList && m_CallableFlameList->GetReady( )&& m_FlameCheck)
    {
        m_Flames = m_CallableFlameList->GetResult( );
        m_DB->RecoverCallable( m_CallableFlameList );
        delete m_CallableFlameList;
        m_CallableFlameList = NULL;
    }

    // refresh alias list all 5 minutes
    if( !m_CallableAliasList && ( GetTime( ) - m_LastAliasListUpdate >= 300 || m_LastAliasListUpdate == 0 ) )
    {
        m_CallableAliasList = m_DB->ThreadedAliasList( );
        m_LastAliasListUpdate = GetTime( );
    }

    if( m_CallableAliasList && m_CallableAliasList->GetReady( ))
    {
        m_Aliases = m_CallableAliasList->GetResult( );
        m_DB->RecoverCallable( m_CallableAliasList );
        delete m_CallableAliasList;
        m_CallableAliasList = NULL;
    }

    // refresh forcedgproxy list all 5 minutes
    if( !m_CallableForcedGProxyList && ( GetTime( ) - m_LastGProxyListUpdate >= 300 || m_LastGProxyListUpdate == 0 ) )
    {
        m_CallableForcedGProxyList = m_DB->ThreadedForcedGProxyList( );
        m_LastGProxyListUpdate = GetTime( );
    }

    if( m_CallableForcedGProxyList && m_CallableForcedGProxyList->GetReady( ))
    {
        m_GProxyList = m_CallableForcedGProxyList->GetResult( );
        m_DB->RecoverCallable( m_CallableForcedGProxyList );
        delete m_CallableForcedGProxyList;
        m_CallableForcedGProxyList = NULL;
    }

    // refresh denied names list all 60 minutes
    if( !m_CallableDeniedNamesList && ( GetTime( ) - m_LastDNListUpdate >= 3600 || m_LastDNListUpdate == 0 ) )
    {
        m_CallableDeniedNamesList = m_DB->ThreadedDeniedNamesList( );
        m_LastDNListUpdate = GetTime( );
    }

    if( m_CallableDeniedNamesList && m_CallableDeniedNamesList->GetReady( ) )
    {
        m_DeniedNamePartials = m_CallableDeniedNamesList->GetResult( );
        m_DB->RecoverCallable( m_CallableDeniedNamesList );
        delete m_CallableDeniedNamesList;
        m_CallableDeniedNamesList = NULL;
    }

    // refresh announce list all 60 minutes
    if( !m_CallableAnnounceList && ( GetTime( ) - m_LastAnnounceListUpdate >= 3600 || m_LastAnnounceListUpdate==0 ) )
    {
        m_CallableAnnounceList = m_DB->ThreadedAnnounceList( );
        m_LastAnnounceListUpdate = GetTime( );
    }

    if( m_CallableAnnounceList && m_CallableAnnounceList->GetReady( ) )
    {
        m_Announces = m_CallableAnnounceList->GetResult( );
        m_DB->RecoverCallable( m_CallableAnnounceList );
        delete m_CallableAnnounceList;
        m_CallableAnnounceList = NULL;
        //update announcenumber
        m_AnnounceLines = m_Announces.size();
    }

    // refresh denied country list all 60 minutes
    if( !m_CallableDCountryList && ( GetTime( ) - m_LastDCountryUpdate >= 1200 || m_LastDCountryUpdate == 0 ) )
    {
        m_CallableDCountryList = m_DB->ThreadedDCountryList( );
        m_LastDCountryUpdate = GetTime( );
    }

    if( m_CallableDCountryList && m_CallableDCountryList->GetReady( ) )
    {
        m_DCountries = m_CallableDCountryList->GetResult( );
        m_DB->RecoverCallable( m_CallableDCountryList );
        delete m_CallableDCountryList;
        m_CallableDCountryList = NULL;
    }

    // load a new m_ReservedHostCounter
    if( m_ReservedHostCounter == 0 && m_LastHCUpdate != 0 && GetTime( ) - m_LastHCUpdate >= 5 )
    {
        m_CallableHC = m_DB->ThreadedGameDBInit( vector<CDBBan *>(), m_AutoHostGameName, 0, 0 );
        m_LastHCUpdate = 0;
    }

    if( m_CallableHC && m_CallableHC->GetReady( ) )
    {
        m_ReservedHostCounter = m_CallableHC->GetResult( );
        m_DB->RecoverCallable( m_CallableHC );
        delete m_CallableHC;
        m_CallableHC = NULL;
    }

    //refresh command list every 5 seconds
    if( !m_CallableCommandList && GetTime( ) - m_LastCommandListTime >= 5 )
    {
        m_CallableCommandList = m_DB->ThreadedCommandList( );
        m_LastCommandListTime = GetTime();
    }

    if( m_CallableCommandList && m_CallableCommandList->GetReady( ) )
    {
        vector<string> commands = m_CallableCommandList->GetResult( );

        for( vector<string> :: iterator i = commands.begin( ); i != commands.end( ); ++i )
        {
		    HandleRCONCommand(*i);
        }

        m_DB->RecoverCallable( m_CallableCommandList );
        delete m_CallableCommandList;
        m_CallableCommandList = NULL;
        m_LastCommandListTime = GetTime();
    }

    if( m_CurrentGame ) {
        if( ( GetTime() - m_CurrentGame->m_CreationTime ) >= 259200 ) {
            SET( m_RunMode, MODE_EXIT );
        }
    }

    m_EndTicks = GetTicks();
    m_Sampler++;
    uint32_t SpreadTicks = m_EndTicks - m_StartTicks;
    if(SpreadTicks > m_MaxTicks) {
        m_MaxTicks = SpreadTicks;
    }
    if(SpreadTicks < m_MinTicks) {
        m_MinTicks = SpreadTicks;
    }
    m_TicksCollection += SpreadTicks;
    if(GetTicks() - m_TicksCollectionTimer >= 60000) {
        m_AVGTicks = m_TicksCollection/m_Sampler;
        m_TicksCollectionTimer = GetTicks();
        Log->Info("[OHSystem-Performance-Check] AVGTicks: "+UTIL_ToString(m_AVGTicks, 0)+"ms | MaxTicks: "+UTIL_ToString(m_MaxTicks)+"ms | MinTicks: "+UTIL_ToString(m_MinTicks)+"ms | Updates: "+UTIL_ToString(m_Sampler));
        Log->Info("[OHSystem-Performance-CHeck] MySQL: "+m_DB->GetStatus( ));
        m_MinTicks = -1;
        m_MaxTicks = 0;
        m_TicksCollection = 0;
        m_Sampler = 0;
    }

    return IS(m_RunMode,MODE_EXIT) || AdminExit || BNETExit;
}

void COHBot :: EventBNETConnecting( CBNET *bnet )
{

}

void COHBot :: EventBNETConnected( CBNET *bnet )
{

}

void COHBot :: EventBNETDisconnected( CBNET *bnet )
{

}

void COHBot :: EventBNETLoggedIn( CBNET *bnet )
{

}

void COHBot :: EventBNETGameRefreshed( CBNET *bnet )
{
	boost::mutex::scoped_lock lock( m_GamesMutex );

 	if( m_CurrentGame )
 		m_CurrentGame->EventGameRefreshed( bnet->GetServer( ));

	lock.unlock( );
}

void COHBot :: EventBNETGameRefreshFailed( CBNET *bnet )
{
    boost::mutex::scoped_lock lock( m_GamesMutex );

    if( m_CurrentGame )
    {
        for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); ++i )
        {
            (*i)->QueueChatCommand( m_Language->UnableToCreateGameTryAnotherName( bnet->GetServer( ), m_CurrentGame->GetGameName( ) ) );

            if( (*i)->GetServer( ) == m_CurrentGame->GetCreatorServer( ) )
                (*i)->QueueChatCommand( m_Language->UnableToCreateGameTryAnotherName( bnet->GetServer( ), m_CurrentGame->GetGameName( ) ), m_CurrentGame->GetCreatorName( ), true );
        }

        m_CurrentGame->SendAllChat( m_Language->UnableToCreateGameTryAnotherName( bnet->GetServer( ), m_CurrentGame->GetGameName( ) ) );

        // we take the easy route and simply close the lobby if a refresh fails
        // it's possible at least one refresh succeeded and therefore the game is still joinable on at least one battle.net (plus on the local network) but we don't keep track of that
        // we only close the game if it has no players since we support game rehosting (via !priv and !pub in the lobby)

        if( m_CurrentGame->GetNumHumanPlayers( ) == 0 )
            m_CurrentGame->SetExiting( true );

        m_CurrentGame->SetRefreshError( true );
    }

    lock.unlock( );
}

void COHBot :: EventBNETConnectTimedOut( CBNET *bnet )
{

}

void COHBot :: EventBNETWhisper( CBNET *bnet, string user, string message )
{

}

void COHBot :: EventBNETChat( CBNET *bnet, string user, string message )
{

}

void COHBot :: EventBNETEmote( CBNET *bnet, string user, string message )
{

}

void COHBot :: EventGameDeleted( CBaseGame *game )
{
    for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); ++i )
    {
        (*i)->QueueChatCommand( m_Language->GameIsOver( game->GetDescription( ) ) );

        if( (*i)->GetServer( ) == game->GetCreatorServer( ) )
            (*i)->QueueChatCommand( m_Language->GameIsOver( game->GetDescription( ) ), game->GetCreatorName( ), true );
    }
}

bool COHBot :: IsMode( uint8_t gMode ) {
    return IS( m_RunMode, gMode );
}

void COHBot :: SetMode( uint8_t gMode, bool bEnable ) {
    if( bEnable )
        SET( m_RunMode, gMode );
    else
        UNSET( m_RunMode, gMode );
}

void COHBot :: ReloadConfigs( )
{
    delete CFG;
    CFG = new CConfig();
    CFG->Read( "default.cfg" );
    // load extended botid config
    if ( sCFGFile != "" ) {
        CFG->Read(sCFGFile);
    }
    SetConfigs( CFG );
}

void COHBot :: SetConfigs( CConfig *CFG )
{
    // this doesn't set EVERY config value since that would potentially require reconfiguring the battle.net connections
    // it just set the easily reloadable values
    m_LanguagesPath = UTIL_AddPathSeperator( CFG->GetString( "languages_path", "languages/" ) );
    delete m_Language;
    m_Language = new CLanguage( m_LanguagesPath + CFG->GetString( "bot_language", "en" ) + ".cfg" );

    m_ReconnectWaitTime = CFG->GetInt( "gproxy_reconnectwaittime", 3 );

    m_MaxGames = CFG->GetInt( "bot_maxgames", 5 );

    // set command trigger
    string BotCommandTrigger = CFG->GetString( "bot_commandtrigger", "!" );
    if( BotCommandTrigger.empty( ) )
        BotCommandTrigger = "!";
    m_CommandTrigger = BotCommandTrigger[0];

    // folders
    m_MapPath = UTIL_AddPathSeperator( CFG->GetString( "bot_mappath", string( ) ) );
    m_MapCFGPath = UTIL_AddPathSeperator( CFG->GetString( "bot_mapcfgpath", string( ) ) );
    m_SaveGamePath = UTIL_AddPathSeperator( CFG->GetString( "bot_savegamepath", string( ) ) );
    m_ReplayPath = UTIL_AddPathSeperator( CFG->GetString( "bot_replaypath", string( ) ) );
    m_SaveReplays = CFG->GetInt( "bot_savereplays", 0 ) == 0 ? false : true;

    // messages
    m_MOTDFile = CFG->GetString( "bot_motdfile", "motd.txt" );
    m_GameLoadedFile = CFG->GetString( "bot_gameloadedfile", "gameloaded.txt" );
    m_GameOverFile = CFG->GetString( "bot_gameoverfile", "gameover.txt" );

    // logging
    m_GameLogging = CFG->GetInt( "game_logging", 0 ) == 0 ? false : true;
    m_GameLoggingID = CFG->GetInt( "game_loggingid", 1 );
    m_GameLogFilePath = UTIL_AddPathSeperator( CFG->GetString( "game_logpath", string( ) ) );

    m_HideIPAddresses = CFG->GetInt( "bot_hideipaddresses", 0 ) == 0 ? false : true;
    m_CheckMultipleIPUsage = CFG->GetInt( "bot_checkmultipleipusage", 1 ) == 0 ? false : true;

    // names
    m_ColoredNamePath = UTIL_AddPathSeperator( CFG->GetString( "oh_coloredname", string( ) ) );
    m_ColoredNameHide = CFG->GetInt("oh_hiddenColoredName", 0 ) == 0 ? false : true;
    m_VirtualHostName = CFG->GetString( "bot_virtualhostname", "|cFF4080C0GHost" );
    if( m_VirtualHostName.size( ) > 15 )
    {
        m_VirtualHostName = "|cFF4080C0GHost";
        Log->Warning( "[GHOST] bot_virtualhostname is longer than 15 characters, using default virtual host name" );
    }

    // ohsystem features
    m_StatsUpdate = CFG->GetInt( "oh_general_updatestats", 0 ) == 0 ? false : true;
    m_MessageSystem = CFG->GetInt("oh_general_messagesystem", 1 ) == 0 ? false : true;
    m_FunCommands = CFG->GetInt("oh_general_funcommands", 1 ) == 0 ? false : true;
    m_BetSystem = CFG->GetInt("oh_general_betsystem", 1 ) == 0 ? false : true;
    m_AccountProtection = CFG->GetInt("oh_general_accountprotection", 1 ) == 0 ? false : true;
    m_AutoDenyUsers = CFG->GetInt("oh_general_autodeny", 0) == 0 ? false : true;
    m_BotManagerName = CFG->GetString( "oh_general_botmanagername", "PeaceMaker" );
    m_Website = CFG->GetString("oh_general_domain", "https://wc3.be4m.de/" );
    m_ChannelBotOnly = CFG->GetInt("oh_channelbot", 0) == 0 ? false : true;


    m_SpoofChecks = CFG->GetInt( "bot_spoofchecks", 2 );
    m_RequireSpoofChecks = CFG->GetInt( "bot_requirespoofchecks", 0 ) == 0 ? false : true;
    m_ReserveAdmins = CFG->GetInt( "bot_reserveadmins", 1 ) == 0 ? false : true;
    m_RefreshMessages = CFG->GetInt( "bot_refreshmessages", 0 ) == 0 ? false : true;
    m_AutoLock = CFG->GetInt( "bot_autolock", 0 ) == 0 ? false : true;
    m_AutoSave = CFG->GetInt( "bot_autosave", 0 ) == 0 ? false : true;

    // downloads
    m_AllowDownloads = CFG->GetInt( "bot_allowdownloads", 0 );
    m_PingDuringDownloads = CFG->GetInt( "bot_pingduringdownloads", 0 ) == 0 ? false : true;
    m_MaxDownloaders = CFG->GetInt( "bot_maxdownloaders", 3 );
    m_MaxDownloadSpeed = CFG->GetInt( "bot_maxdownloadspeed", 100 );

    m_LCPings = CFG->GetInt( "bot_lcpings", 1 ) == 0 ? false : true;
    m_AutoKickPing = CFG->GetInt( "bot_autokickping", 400 );
    m_BanMethod = CFG->GetInt( "bot_banmethod", 1 );
    m_IPBlackListFile = CFG->GetString( "bot_ipblacklistfile", "ipblacklist.txt" );
    m_LobbyTimeLimit = CFG->GetInt( "bot_lobbytimelimit", 10 );
    m_Latency = CFG->GetInt( "bot_latency", 100 );
    m_SyncLimit = CFG->GetInt( "bot_synclimit", 50 );

    m_LocalAdminMessages = CFG->GetInt( "bot_localadminmessages", 1 ) == 0 ? false : true;
    m_TCPNoDelay = CFG->GetInt( "tcp_nodelay", 0 ) == 0 ? false : true;
    m_MatchMakingMethod = CFG->GetInt( "bot_matchmakingmethod", 1 );
    m_MapGameType = CFG->GetUInt( "bot_mapgametype", 0 );
    m_AllGamesFinished = false;
    m_AllGamesFinishedTime = 0;
    m_MinVIPGames = CFG->GetInt( "vip_mingames", 25 );
    m_RegVIPGames = CFG->GetInt( "vip_reg", 0 ) == 0 ? false : true;


    m_TFT = CFG->GetInt( "bot_tft", 1 ) == 0 ? false : true;
    if( m_TFT )
        Log->Info( "[GHOST] acting as Warcraft III: The Frozen Throne" );
    else
        Log->Info( "[GHOST] acting as Warcraft III: Reign of Chaos" );


    m_OHBalance = CFG->GetInt( "oh_balance", 1 ) == 0 ? false : true;
    m_HighGame = CFG->GetInt( "oh_rankedgame", 0 ) == 0 ? false : true;
    m_MinLimit = CFG->GetInt( "oh_mingames", 25 );
    m_ObserverFake = CFG->GetInt( "oh_observer", 1 ) == 0 ? false : true;
    m_CheckIPRange = CFG->GetInt( "oh_checkiprangeonjoin", 0 ) == 0 ? false : true;
    m_DenieProxy = CFG->GetInt( "oh_proxykick", 0 ) == 0 ? false : true;
    m_LiveGames = CFG->GetInt( "oh_general_livegames", 1 ) == 0 ? false : true;
    m_MinFF = CFG->GetInt( "oh_minff", 20 );

    // antifarm
    m_MinimumLeaverKills = CFG->GetInt( "antifarm_minkills", 3 );
    m_MinimumLeaverDeaths = CFG->GetInt( "antifarm_mindeaths", 3 );
    m_MinimumLeaverAssists = CFG->GetInt( "antifarm_minassists", 3 );
    m_DeathsByLeaverReduction =  CFG->GetInt( "antifarm_deathsbyleaver", 1 );

    // autoend
    m_MinPlayerAutoEnd = CFG->GetInt( "autoend_minplayer", 2 );
    m_MaxAllowedSpread = CFG->GetInt( "autoend_maxspread", 2 );
    m_EarlyEnd = CFG->GetInt( "autoend_earlyend", 1 ) == 0 ? false : true;
    m_AutoEndTime = CFG->GetInt("autoend_votetime", 120);


    m_Announce = CFG->GetInt("oh_announce", 0 ) == 0 ? false : true;
    m_AnnounceHidden = CFG->GetInt("oh_hiddenAnnounce", 0 ) == 0 ? false : true;

    m_FountainFarmWarning = CFG->GetInt("oh_fountainfarm_warning", 0 ) == 0 ? false : true;
    m_FountainFarmMessage = CFG->GetString("oh_fountainfarm_message", "Reminder: Any kind of fountainfarming, or even an attempt, is bannable." );

    m_AllowVoteStart = CFG->GetInt("oh_allowvotestart", 0) == 0 ? false : true;
    m_VoteStartMinPlayers = CFG->GetInt("oh_votestartminimumplayers", 3);
    m_AutoMuteSpammer = CFG->GetInt( "oh_mutespammer", 1 ) == 0 ? false : true;
    m_FlameCheck = CFG->GetInt("oh_flamecheck", 0) == 0 ? false : true;
    m_IngameVoteKick = CFG->GetInt("oh_ingamevotekick", 1) == 0 ? false : true;
    m_LeaverAutoBanTime = CFG->GetInt("oh_leaverautobantime", 259200);
    m_DisconnectAutoBanTime = CFG->GetInt("oh_disconnectautobantime", 86400);
    m_FirstFlameBanTime = CFG->GetInt("oh_firstflamebantime", 172800 );
    m_SecondFlameBanTime = CFG->GetInt("oh_secondflamebantime", 345600);
    m_SpamBanTime = CFG->GetInt("oh_spambantime", 172800 );


    // votes: kick
    m_VoteKickAllowed = CFG->GetInt( "bot_votekickallowed", 1 ) == 0 ? false : true;
    m_VoteKickPercentage = CFG->GetInt( "bot_votekickpercentage", 100 );
    if( m_VoteKickPercentage > 100 )
    {
        m_VoteKickPercentage = 100;
        Log->Warning( "[GHOST] bot_votekickpercentage is greater than 100, using 100 instead" );
    }
    m_VKAbuseBanTime = CFG->GetInt("oh_votekickabusebantime", 432000);

    // votes: mute
    m_VoteMuting = CFG->GetInt("oh_votemute", 1) == 0 ? false : true;
    m_VoteMuteTime = CFG->GetInt("oh_votemutetime", 180);

    // votes: mode
    m_VoteMode = CFG->GetInt( "oh_votemode", 0 ) == 0 ? false : true;
    m_MaxVotingTime = CFG->GetInt( "oh_votemode_time", 30 );


    m_AllowHighPingSafeDrop = CFG->GetInt("oh_allowsafedrop", 1) == 0 ? false : true;
    m_MinPauseLevel = CFG->GetInt("oh_minpauselevel", 3);
    m_MinScoreLimit = CFG->GetInt("oh_minscorelimit", 0);
    m_MaxScoreLimit = CFG->GetInt("oh_maxscorelimit", 0);
    m_AutobanAll = CFG->GetInt("oh_autobanall", 1) == 0 ? false : true;
    m_WC3ConnectAlias = CFG->GetString("wc3connect_alias", "WC3Connect");
    m_NonAllowedDonwloadMessage = CFG->GetString("oh_downloadmessage", string());
    m_RandomMode = CFG->GetInt( "oh_votemode_random", 0 ) == 0 ? false : true;
    m_HideMessages = CFG->GetInt( "oh_hideleavermessages", 1 ) == 0 ? false : true;
    m_DenieCountriesOnThisBot = CFG->GetInt( "oh_deniedcountries", 1 ) == 0 ? false : true;
    m_KickSlowDownloader = CFG->GetInt("oh_kickslowdownloader", 1 ) == 0 ? false :  true;
    m_VirtualLobby = CFG->GetInt("oh_virtuallobby", 1 ) == 0 ? false : true;
    m_VirtualLobbyTime = CFG->GetInt("oh_virtuallobbytime", 20);
    m_CustomVirtualLobbyInfoBanText = CFG->GetString("oh_virtuallobbybantext", string( ));
    m_SimpleAFKScript = CFG->GetInt("oh_simpleafksystem", 1 ) == 0 ? false :  true;
    m_APMAllowedMinimum = CFG->GetInt("oh_apmallowedminimum", 20);
    m_APMMaxAfkWarnings = CFG->GetInt("oh_apmmaxafkwarnings", 5);
    m_SharedFilesPath = UTIL_AddPathSeperator( CFG->GetString( "bot_sharedfilespath", string( ) ) );
    m_BroadCastPort = CFG->GetInt("oh_broadcastport", 6112 );
    m_SpoofPattern = CFG->GetString("oh_spoofpattern", string());
    m_DelayGameLoaded = CFG->GetInt("oh_delaygameloaded", 300);
    m_FountainFarmDetection = CFG->GetInt("oh_fountainfarmdetection", 1) == 0 ? false : true;
    m_AutokickSpoofer = CFG->GetInt("oh_autokickspoofer", 1) == 0 ? false : true;
    m_PVPGNMode = CFG->GetInt("oh_pvpgn_mode", 0) == 0 ? false : true;

    // AutoHost
    m_AutoHostGameType = CFG->GetInt( "oh_autohosttype", 3 );
    m_AutoRehostTime = CFG->GetInt("oh_auto_rehost_time", 0);
    if(m_AutoRehostTime<10 && m_AutoRehostTime!=0) { 
	    m_AutoRehostTime=10;
    }

    m_DenyLimit = CFG->GetInt("oh_cc_deny_limit", 2);
    m_SwapLimit = CFG->GetInt("oh_cc_swap_limit", 2);
    m_SendAutoStartInfo = CFG->GetInt("oh_sendautostartalert", 0) == 0 ? false : true;
    m_FountainFarmBan = CFG->GetInt("oh_fountainfarmban", 0) == 0 ? false : true;
    m_GarenaPort = CFG->GetInt("garena_broadcastport", 1338);
    m_RejectingGameCheats = CFG->GetInt("ohs_rejectgamecheats", 1) == 0 ? false : true;

    LoadRules();
    LoadRanks();
    ReadRoomData();
    if( m_FunCommands )
        LoadInsult( );

}

void COHBot :: ExtractScripts( )
{
    string PatchMPQFileName = m_Warcraft3Path + "War3Patch.mpq";
    HANDLE PatchMPQ;

    if( SFileOpenArchive( PatchMPQFileName.c_str( ), 0, MPQ_OPEN_FORCE_MPQ_V1, &PatchMPQ ) )
    {
        Log->Info( "[GHOST] loading MPQ file [" + PatchMPQFileName + "]" );
        HANDLE SubFile;

        // common.j

        if( SFileOpenFileEx( PatchMPQ, "Scripts\\common.j", 0, &SubFile ) )
        {
            uint32_t FileLength = SFileGetFileSize( SubFile, NULL );

            if( FileLength > 0 && FileLength != 0xFFFFFFFF )
            {
                char *SubFileData = new char[FileLength];
                DWORD BytesRead = 0;

                if( SFileReadFile( SubFile, SubFileData, FileLength, &BytesRead ) )
                {
                    Log->Info( "[GHOST] extracting Scripts\\common.j from MPQ file to [" + m_MapCFGPath + "common.j]" );
                    UTIL_FileWrite( m_MapCFGPath + "common.j", (unsigned char *)SubFileData, BytesRead );
                }
                else
                    Log->Warning( "[GHOST] unable to extract Scripts\\common.j from MPQ file" );

                delete [] SubFileData;
            }

            SFileCloseFile( SubFile );
        }
        else
            Log->Write( "[GHOST] couldn't find Scripts\\common.j in MPQ file" );

        // blizzard.j

        if( SFileOpenFileEx( PatchMPQ, "Scripts\\blizzard.j", 0, &SubFile ) )
        {
            uint32_t FileLength = SFileGetFileSize( SubFile, NULL );

            if( FileLength > 0 && FileLength != 0xFFFFFFFF )
            {
                char *SubFileData = new char[FileLength];
                DWORD BytesRead = 0;

                if( SFileReadFile( SubFile, SubFileData, FileLength, &BytesRead ) )
                {
                    Log->Info( "[GHOST] extracting Scripts\\blizzard.j from MPQ file to [" + m_MapCFGPath + "blizzard.j]" );
                    UTIL_FileWrite( m_MapCFGPath + "blizzard.j", (unsigned char *)SubFileData, BytesRead );
                }
                else
                    Log->Warning( "[GHOST] unable to extract Scripts\\blizzard.j from MPQ file" );

                delete [] SubFileData;
            }

            SFileCloseFile( SubFile );
        }
        else
            Log->Write( "[GHOST] couldn't find Scripts\\blizzard.j in MPQ file" );

        SFileCloseArchive( PatchMPQ );
    }
    else
        Log->Warning( "[GHOST] unable to load MPQ file [" + PatchMPQFileName + "] - error code " + UTIL_ToString( GetLastError( ) ) );
}

//
// Host a new Game
//
void COHBot :: CreateGame( CMap *map, unsigned char gameState, bool saveGame, string gameName, string ownerName, string creatorName, string creatorServer, uint32_t gameType, bool whisper, uint32_t m_HostCounter )
{
    //
    //  Error: Bot is disabled
    //
    if( IS_NOT( m_RunMode, MODE_ENABLED ) )
    {
        SendMessageToBNET(m_Language->UnableToCreateGameDisabled(gameName), creatorServer, creatorName, whisper);
        return;
    }

    //
    // Error: GameName too long
    //
    if( gameName.size( ) > 31 )
    {
        SendMessageToBNET(m_Language->UnableToCreateGameNameTooLong(gameName), creatorServer, creatorName, whisper);
        return;
    }

    //
    // Error: Map is not valid
    //
    if( !map->GetValid( ) )
    {
        SendMessageToBNET(m_Language->UnableToCreateGameInvalidMap(gameName), creatorServer, creatorName, whisper);
        return;
    }

    //
    // SaveGame Mode
    //
    if( saveGame )
    {

        //
        //  Error: SaveGame enabled, but SaveGane not valid
        //
        if( !m_SaveGame->GetValid( ) )
        {
            SendMessageToBNET(m_Language->UnableToCreateGameInvalidSaveGame(gameName), creatorServer, creatorName, whisper);
            return;
        }

        string MapPath1 = m_SaveGame->GetMapPath( );
        string MapPath2 = map->GetMapPath( );
        transform( MapPath1.begin( ), MapPath1.end( ), MapPath1.begin( ), ::tolower );
        transform( MapPath2.begin( ), MapPath2.end( ), MapPath2.begin( ), ::tolower );

        //
        //  Error: SaveGame-MapPath is not equal to current MapPath
        //
        if( MapPath1 != MapPath2 )
        {
            Log->Warning( "[GHOST] path mismatch, saved game path is [" + MapPath1 + "] but map path is [" + MapPath2 + "]" );

            SendMessageToBNET(m_Language->UnableToCreateGameSaveGameMapMismatch(gameName), creatorServer, creatorName, whisper);
            return;
        }

        //
        // Error: Player not enforced
        //
        if( m_EnforcePlayers.empty( ) )
        {
            SendMessageToBNET(m_Language->UnableToCreateGameMustEnforceFirst( gameName ), creatorServer, creatorName, whisper);
            return;
        }
    }

    boost::mutex::scoped_lock lock( m_GamesMutex );

    //
    // Error: Another Game already created in Lobby
    //
    if( m_CurrentGame )
    {
        SendMessageToBNET(m_Language->UnableToCreateGameAnotherGameInLobby( gameName, m_CurrentGame->GetDescription( ) ), creatorServer, creatorName, whisper);
        return;
    }

    //
    // Error: MaxGames reached
    //
    if( m_Games.size( ) >= m_MaxGames )
    {
        SendMessageToBNET(m_Language->UnableToCreateGameMaxGamesReached( gameName, UTIL_ToString( m_MaxGames ) ), creatorServer, creatorName, whisper);
        return;
    }

    //
    // Creating a new Game
    //
    lock.unlock( );

    Log->Info( "[GHOST] creating game [" + gameName + "]" );
    if( m_HostCounter == 0 )
        m_HostCounter = GetNewHostCounter( );

    if( saveGame )
        m_CurrentGame = new CGame( this, map, m_SaveGame, m_HostPort, gameState, gameName, ownerName, creatorName, creatorServer, gameType, m_HostCounter );
    else
        m_CurrentGame = new CGame( this, map, NULL, m_HostPort, gameState, gameName, ownerName, creatorName, creatorServer, gameType, m_HostCounter );

    // todotodo: check if listening failed and report the error to the user

    if( m_SaveGame )
    {
        m_CurrentGame->SetEnforcePlayers( m_EnforcePlayers );
        m_EnforcePlayers.clear( );
    }

    //
    // Inform player or lobby about new created game
    //
    for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); ++i )
    {
        if( whisper && (*i)->GetServer( ) == creatorServer )
        {
            // note that we send this whisper only on the creator server

            if( gameState == GAME_PRIVATE )
                (*i)->QueueChatCommand( m_Language->CreatingPrivateGame( gameName, ownerName ), creatorName, whisper );
            else if( gameState == GAME_PUBLIC )
                (*i)->QueueChatCommand( m_Language->CreatingPublicGame( gameName, ownerName ), creatorName, whisper );
        }
        else
        {
            // note that we send this chat message on all other bnet servers

            if( gameState == GAME_PRIVATE )
                (*i)->QueueChatCommand( m_Language->CreatingPrivateGame( gameName, ownerName ) );
            else if( gameState == GAME_PUBLIC )
                (*i)->QueueChatCommand( m_Language->CreatingPublicGame( gameName, ownerName ) );
        }

        if( saveGame )
            (*i)->QueueGameCreate( gameState, gameName, string( ), map, m_SaveGame, m_CurrentGame->GetHostCounter( ) );
        else
            (*i)->QueueGameCreate( gameState, gameName, string( ), map, NULL, m_CurrentGame->GetHostCounter( ) );

        // if we're creating a private game we don't need to send any game refresh messages so we can rejoin the chat immediately
        // unfortunately this doesn't work on PVPGN servers because they consider an enterchat message to be a gameuncreate message when in a game
        // so don't rejoin the chat if we're using PVPGN
        if( gameState == GAME_PRIVATE && (*i)->GetPasswordHashType( ) != "pvpgn" )
            (*i)->QueueEnterChat( );

        // hold friends
        if( (*i)->GetHoldFriends( ) )
            (*i)->HoldFriends( m_CurrentGame );

        // hold clan members
        if( (*i)->GetHoldClan( ) )
            (*i)->HoldClan( m_CurrentGame );

    }

    boost::thread(&CBaseGame::loop, m_CurrentGame);
    Log->Info("[GHOST] Created a new Game Thread.");
}

bool COHBot :: FlameCheck( string message )
{
    transform( message.begin( ), message.end( ), message.begin( ), ::tolower );

    char forbidden[] = {",.!ï¿½$%&/()={[]}*'+#-_.:,;?|"};
    char *check;
    int len = message.length();
    int c = 1;

    for( std::string::iterator i=message.begin( ); i!=message.end( );)
    {
        check=forbidden;
        while(*check)
        {
            if ( *i == *check )
            {
                if( c != len )
                {
                    i=message.erase(i);
                    c++;
                    continue;
                }
            }
            check++;
        }
        i++;
        c++;
    }

    for( size_t i = 0; i < m_Flames.size( ); )
    {
        if( message.find( m_Flames[i] ) != string :: npos )
        {
            return true;
        }
        i++;
    }

    return false;
}

void COHBot :: LoadRules( )
{
    string File = m_SharedFilesPath + "rules.txt";
    string line;
    ifstream myfile(File.c_str());
    m_Rules.clear();
    if (myfile.is_open())
    {
        while ( getline (myfile,line) )
        {
            m_Rules.push_back( line );
        }
        myfile.close();
    }
    else
        Log->Write( "Unable to open rules.txt" );
}

uint32_t COHBot :: GetNewHostCounter( )
{
    if( m_ReservedHostCounter != 0 )
    {
        m_HostCounter = m_ReservedHostCounter;
        m_ReservedHostCounter = 0;
        m_LastHCUpdate = GetTime();
        Log->Info( "[INFO] Set new hostcounter to: "+UTIL_ToString(m_HostCounter));
        return m_HostCounter;
    }
    return m_HostCounter;
}
void COHBot :: LoadRanks( )
{
    m_RanksLoaded = true;

    string File = m_SharedFilesPath + "ranks.txt";
    ifstream in;
    in.open( File.c_str() );
    m_Ranks.clear();
    if( !in.fail( ) )
    {
        uint32_t Count = 0;
        string Line;
        while( !in.eof( ) && Count < 11 )
        {
            getline( in, Line );
            if( Line.empty( ) )
            {
                if( !in.eof( ) )
                    m_Ranks.push_back("Missing Rank on: "+UTIL_ToString(Count));
            }
            else
                m_Ranks.push_back(Line);
            ++Count;
        }
        in.close( );
    }
    else
    {
        Log->Write("[GHOST] unable to read file [ranks.txt]");
        m_RanksLoaded = false;
    }

    if(m_Ranks.size() < 11 && m_RanksLoaded) {
        Log->Warning("[CONFIG] ranks.txt doesn't contain enough levelnames. You require at least 11 rank names (Level 0 - Level 10, with 0).");
        m_RanksLoaded = false;
    } else if(m_RanksLoaded) {
		Log->Info("[GHOST] loading file [ranks.txt]");
    }
}

void COHBot :: LoadInsult()
{
    string File = m_SharedFilesPath + "insult.txt";
    ifstream in;
    in.open( File.c_str() );
    m_Insults.clear();
    if( !in.fail( ) )
    {
        string Line;
        while( !in.eof( )  )
        {
            getline( in, Line );
            m_Insults.push_back(Line);
        }
	    Log->Info("[GHOST] loading file [insult.txt]");
        in.close( );
    }
    else
        Log->Warning("[GHOST] unable to read file [insult.txt].");
}

string COHBot :: GetTimeFunction( uint32_t type )
{
    //should work on windows also. This should be tested.

    time_t theTime = time(NULL);
    struct tm *aTime = localtime(&theTime);
    int Time = 0;
    if( type == 1)
        Time = aTime->tm_mon + 1;
    if( Time == 0)
        Time = aTime->tm_year + 1900;
    return UTIL_ToString(Time);
}

string COHBot :: GetRoomName (string RoomID)
{
    int l = RoomID.size();
    int DPos;
    if (m_LanRoomName.size()==0)
        return string();
    else if (l>4)
    {
        for (uint32_t i = 0; i<m_LanRoomName.size(); i++)
        {
            DPos = m_LanRoomName[i].find(RoomID);
            if (DPos!= string ::npos) {
                return m_LanRoomName[i].substr(DPos+l+2);
            }
        }
    }

    //room matching that RoomID is not found
    return string();
}

void COHBot :: ReadRoomData()
{
    string file = m_SharedFilesPath + "rooms.txt";
    ifstream in;
    in.open( file.c_str( ) );
    m_LanRoomName.clear();
    if( in.fail( ) )
        Log->Warning( "[GHOST] unable to read file [" + file + "]" );
    else
    {
        Log->Info( "[GHOST] loading file [" + file + "]" );
        string Line;
        while( !in.eof( ) )
        {
            getline( in, Line );
            if( Line.empty( ) || Line[0] == '#' )
                continue;
            m_LanRoomName.push_back(Line);
        }
    }
    in.close( );
}

//
//  Get name from AliasId (1 based)
//
string COHBot :: GetAliasName( uint32_t alias ) {
    if( alias != 0 && m_Aliases.size( ) != 0 && m_Aliases.size( ) >= alias) {
        return m_Aliases[alias-1];
    }
    return "failed";
}

uint32_t COHBot :: GetStatsAliasNumber( string alias ) {
    uint32_t m_StatsAlias = 0;
    uint32_t c = 1;
    if(! alias.empty() ) {
        transform( alias.begin( ), alias.end( ), alias.begin( ), ::tolower );
        for( vector<string> :: iterator i = m_Aliases.begin( ); i != m_Aliases.end( ); ++i ) {
            string Alias = *i;
            transform( Alias.begin( ), Alias.end( ), Alias.begin( ), ::tolower );
            if( Alias.substr(0, alias.size( ) ) == alias || Alias == alias ) {
                m_StatsAlias = c;
                break;
            }
            c++;
        }

    } else if( m_CurrentGame ) {
        m_StatsAlias = m_CurrentGame->m_GameAlias;
    }
    return m_StatsAlias;
}

/**
 *
 * This function does switch a long mode into a saved shorten mode which is available for lod.
 * Modes which contain more than 10 characters cant be encoded on LoD, so the map owners added shorten modes
 * The modes can be found here: http://legendsofdota.com/index.php?/page/index.html
 *
 * @param fullmode
 * @return shorten mode
 */
string COHBot :: GetLODMode( string fullmode ) {
    string shortenmode = fullmode;
    if(fullmode == "sdzm3lseb")
        shortenmode = "rgc";
    else if(fullmode == "sds5ebzm")
        shortenmode = "rgc2";
    else if(fullmode == "sds6d2oseb")
        shortenmode = "rgc3";
    else if(fullmode == "md3lsebzm" )
        shortenmode = "rgc4";
    else if(fullmode == "mds5zmebulos" )
        shortenmode = "rgc5";
    else if(fullmode == "mdd2s6ebzmulos" )
        shortenmode = "rgc6";
    else if(fullmode == "mds6d5ulabosfnzm")
        shortenmode = "md";
    else if(fullmode == "sds6d5ulabosfnzm")
        shortenmode = "sd";
    else if(fullmode == "aps6ulabosfnzm")
        shortenmode = "ap";
    else if(fullmode == "ardms6omfrulabzm")
        shortenmode = "ar";
    else if(fullmode == "sds6fnulboabd3")
        shortenmode = "ohs1";
    else if(fullmode == "ardms6fnulboab")
        shortenmode = "ohs2";
    else if(fullmode == "aps6fnulboabssosls")
        shortenmode = "ohs3";
    else if(fullmode == "sdd5s6ulabbofnfrer")
        shortenmode = "ohs4";
    else if(fullmode == "mds6d3scfnulbonm")
        shortenmode = "ohs5";
    else if(fullmode == "mds6d2omfnulbo")
        shortenmode = "o1";
    else if(fullmode == "ardms6boomfnul")
        shortenmode = "o2";

    return shortenmode;
}

bool COHBot :: IsForcedGProxy( string input ) {
    transform( input.begin( ), input.end( ), input.begin( ), ::tolower );

    for( vector<string> :: iterator i = m_GProxyList.begin( ); i != m_GProxyList.end( ); ++i )
    {
        string BanIP = *i;
        if( BanIP[0] == ':' )
        {
            BanIP = BanIP.substr( 1 );
            size_t len = BanIP.length( );

            if( input.length( ) >= len && input.substr( 0, len ) == BanIP )
            {
                return true;
            }
            else if( BanIP.length( ) >= 3 && BanIP[0] == 'h' && input.length( ) >= 3 && input[0] == 'h' && input.substr( 1 ).find( BanIP.substr( 1 ) ) != string::npos )
            {

                return true;
            }
        }

        if( *i == input )
        {
            return true;
        }
    }

    return false;
}

bool COHBot :: FindHackFiles( string input ) {
    transform( input.begin( ), input.end( ), input.begin( ), ::tolower );
    bool HasNoHackFiles = true;
    vector<string> m_HackFiles;
    string File;
    stringstream SS;
    SS << input;
    while( SS >> File )
    {
        m_HackFiles.push_back( File );
    }

    for( vector<string> :: iterator i = m_HackFiles.begin( ); i != m_HackFiles.end( ); ++i ) {
        string FileAndSize = *i;
        if( FileAndSize.find("-")!=string::npos ) {
            uint32_t filelength =  FileAndSize.length( );
            std::size_t pos =  FileAndSize.find("-");
            string File = FileAndSize.substr( 0, pos);
            string Size = FileAndSize.substr( pos+1, ( filelength-pos+1 ) );
            if( Size != "7680" && !Size.empty() ) {
                HasNoHackFiles = false;
            }
        }
    }
    return HasNoHackFiles;
}

bool COHBot ::  PlayerCached( string playername ) {

    transform( playername.begin( ), playername.end( ), playername.begin( ), ::tolower );

    for( vector<cachedPlayer> :: iterator i = m_PlayerCache.begin( ); i != m_PlayerCache.end( );)
    {
        if(  GetTime( ) - i->time <= 7200 )
        {
            if( i->name == playername )
                return true;

            i++;
        }
        else
            i=m_PlayerCache.erase( i );
    }

    return false;
}

bool COHBot :: CanAccessCommand( string name, string command ) {
    transform( name.begin( ), name.end( ), name.begin( ), ::tolower );

    for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); ++i ) {
	for( vector<permission> :: iterator j = (*i)->m_Permissions.begin( ); j != (*i)->m_Permissions.end( ); ++j ) {
            if( j->player == name ) {
		string bin = j->binaryPermissions;
		if(    (command=="ping" && bin.substr(0,1) == "1" )
		    || (command=="from" && bin.substr(1,1) == "1" )
                    || (command=="drop" && bin.substr(2,1) == "1" )
                    || ((command=="mute"||command=="unmute") && bin.substr(3,1) == "1" )
                    || (command=="swap" && bin.substr(4,1) == "1" )
                    || (command=="deny" && bin.substr(5,1) == "1" )
                    || (command=="insult" && bin.substr(6,1) == "1" )
                    || (command=="forcemode" && bin.substr(7,1) == "1" )
                    || (command=="ppadd" && bin.substr(8,1) == "1" )
                  )
                return true;
	    }
    	}
    }
    return false;
}

void COHBot :: HandleRCONCommand( string incommingcommand ) {
	string waste;
	uint32_t gameid;
	string execplayer;
	string command;
	stringstream SS;

	SS << incommingcommand;
	SS >> waste;

	if( SS.fail( ) || waste.empty() )
		Log->Debug("Bad input for RCON command #1");
	else {
		SS >> gameid;
		if( SS.fail( ) )
			Log->Debug("Bad input for RCON command #2");
		else {
			SS >> execplayer;
                        if( !SS.eof( ) )
                        {
                           getline( SS, command );
                           string :: size_type Start = command.find_first_not_of( " " );
                           if( Start != string :: npos )
                               command = command.substr( Start );
                        }

			string Command;
			string Payload;
			string :: size_type PayloadStart = command.find( " " );
			if( PayloadStart != string :: npos )
			{
			    Command = command.substr( 1, PayloadStart - 1 );
			    Payload = command.substr( PayloadStart + 1 );
			}
			else
			    Command = command.substr( 1 );

			transform( Command.begin( ), Command.end( ), Command.begin( ), ::tolower );

			// Test for announcer
			bool announce = Command == "botannounce";
			bool saygame = Command == "saygame";
			bool wasCurrentGame = false;
			if( m_CurrentGame ) {
				if( m_CurrentGame->GetHostCounter( ) == gameid ) {
					if(!saygame) {
						m_CurrentGame->EventPlayerBotCommand( NULL, Command, Payload, true, execplayer);
					} else {
						m_CurrentGame->SendAllChat("[WEB: "+execplayer+"] " + Payload);
					}
				}
				else if(announce) {
					m_CurrentGame->SendAllChat("[ANNOUNCE: "+execplayer+"] " + Payload);
				}
			}
			for( vector<CBaseGame *> :: iterator i = m_Games.begin( ); i != m_Games.end( ); ++i ) {
				if( (*i)->GetHostCounter( ) == gameid ) {
					if(!saygame) {
						(*i)->EventPlayerBotCommand( NULL, Command, Payload, true);
					} else {
						(*i)->SendAllChat("[WEB: "+execplayer+"]" + Payload);
					}	
				}
				else if(announce) {
					(*i)->SendAllChat("[ANNOUNCE: "+execplayer+"] " + Payload);
				}
			}
		}
	}
	
}

// @TODO remove or recover
void COHBot :: LoadLanguages( ) {
    /*
    try
    {
        path LanCFGPath( m_LanguagesPath );

        if( !exists( LanCFGPath ) )
        {
            Log->Write("[ERROR] Could not find any language file. Shutting down.");
            m_ExitingNice = true;
        }
        else
        {
            directory_iterator EndIterator;

            for( directory_iterator i( LanCFGPath ); i != EndIterator; ++i )
            {
                string FileName = i->path( ).filename( ).string( );
                string Stem = i->path( ).stem( ).string( );
                transform( FileName.begin( ), FileName.end( ), FileName.begin( ), ::tolower );
                transform( Stem.begin( ), Stem.end( ), Stem.begin( ), ::tolower );

                if( !is_directory( i->status( ) ) && i->path( ).extension( ) == ".cfg" )
                {
                    delete m_Language;
                    m_Language = new CLanguage( FileName );
                    translationTree translation;
                    string languageSuffix = FileName.substr(0, 2);
                    if ( languageSuffix == "en" )
                        m_FallBackLanguage = i;
                    translation.suffix = languageSuffix;
                    translation.m_Translation = m_Language;
                    m_LanguageBundle.push_back(translation);
                }
            }
        }
    }
    catch( const exception &ex )
    {
        Log->Write( "[ERROR] error listing language files - caught exception " + *ex.what( ) );
    }
    */
}

void COHBot ::SendMessageToBNET(string message, string server, string user, bool whisper) {
    for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); ++i )
    {
        if( (*i)->GetServer( ) == server ) {
            (*i)->QueueChatCommand(message, user, whisper);
            return;
        }
    }
}
