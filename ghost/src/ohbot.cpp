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
#include "utils/util.h"
#include "utils/crc32.h"
#include "utils/sha1.h"
#include "config.h"
#include "language.h"
#include "network/socket.h"
#include "database/ghostdb.h"
#include "database/ghostdbmysql.h"
#include "bnet/bnet.h"
#include "map.h"
#include "utils/packed.h"
#include "replay.h"
#include "savegame.h"
#include "game/gameplayer.h"
#include "game/gameprotocol.h"
#include "game/gameprotocol.h"
#include "network/gpsprotocol.h"
#include "garena/gcbiprotocol.h"
#include "game/game_base.h"
#include "game/game.h"
#include "bnls/bnlsprotocol.h"
#include "bnls/bnlsclient.h"
#include "bnet/bnetprotocol.h"
#include "utils/bncsutilinterface.h"

#include <signal.h>
#include <stdlib.h>

#ifdef WIN32
#include <ws2tcpip.h>		// for WSAIoctl
#endif

#define __STORMLIB_SELF__
#include <stormlib/StormLib.h>

#ifdef WIN32
#include <windows.h>
#include <winsock.h>
#endif

#include <time.h>

#ifndef WIN32
//woot woot?
#include <time.h>
#include <execinfo.h>
#endif

#ifdef __APPLE__
#include <mach/mach_time.h>
#endif

#include <boost/filesystem.hpp>

using namespace boost :: filesystem;

string gCFGFile;
string gLogFile;
uint32_t gLogMethod;
ofstream *gLog = NULL;
COHBot *gGHost = NULL;
CConfig CFG;

uint32_t GetTime( )
{
    return GetTicks( ) / 1000;
}

uint32_t GetTicks( )
{
#ifdef WIN32
    // don't use GetTickCount anymore because it's not accurate enough (~16ms resolution)
    // don't use QueryPerformanceCounter anymore because it isn't guaranteed to be strictly increasing on some systems and thus requires "smoothing" code
    // use timeGetTime instead, which typically has a high resolution (5ms or more) but we request a lower resolution on startup

    return timeGetTime( );
#elif __APPLE__
    uint64_t current = mach_absolute_time( );
    static mach_timebase_info_data_t info = { 0, 0 };
    // get timebase info
    if( info.denom == 0 )
        mach_timebase_info( &info );
    uint64_t elapsednano = current * ( info.numer / info.denom );
    // convert ns to ms
    return elapsednano / 1e6;
#else
    uint32_t ticks;
    struct timespec t;
    clock_gettime( CLOCK_MONOTONIC, &t );
    ticks = t.tv_sec * 1000;
    ticks += t.tv_nsec / 1000000;
    return ticks;
#endif
}
#ifndef WIN32
void dumpstack(void){
        static void *backbuf[ 50 ];
        int levels = backtrace( backbuf, 50 );
        char** strings = backtrace_symbols(backbuf, levels);

        CONSOLE_Print("<DUMPSTACK>");

        for(int i = 0; i < levels; i++)
               CONSOLE_Print(strings[i]);

        CONSOLE_Print("</DUMPSTACK>");

        return;
}

void SignalSIGSEGV( int s ) {
        cout << "SEGMENTATION FAULT ... DUMPING STACK ... " << s << endl;
        dumpstack();
        exit( 0 );
}

void SignalSIGILL( int s ) {
        cout << "ILLEGAL INSTRUCTION ... DUMPING STACK ... " << s << endl;
        dumpstack();
        exit( 0 );
}
#endif
void SignalCatcher2( int s )
{
    CONSOLE_Print( "[!!!] caught signal " + UTIL_ToString( s ) + ", exiting NOW" );

    if( gGHost )
    {
        if( gGHost->m_Exiting )
            exit( 1 );
        else
            gGHost->m_Exiting = true;
    }
    else
        exit( 1 );
}

void SignalCatcher( int s )
{
    // signal( SIGABRT, SignalCatcher2 );
    signal( SIGINT, SignalCatcher2 );

    CONSOLE_Print( "[!!!] caught signal " + UTIL_ToString( s ) + ", exiting nicely" );

    if( gGHost )
        gGHost->m_ExitingNice = true;
    else
        exit( 1 );
}

CConfig GetCFG( )
{
        return CFG;
}

void CONSOLE_Print( string message )
{

    cout << message << endl;

    // logging

    if( !gLogFile.empty( ) )
    {
        if( gLogMethod == 1 )
        {
            ofstream Log;
            Log.open( gLogFile.c_str( ), ios :: app );

            if( !Log.fail( ) )
            {
                time_t Now = time( NULL );
                string Time = asctime( localtime( &Now ) );

                // erase the newline

                Time.erase( Time.size( ) - 1 );
                Log << "[" << Time << "] " << message << endl;
                Log.close( );
            }
        }
        else if( gLogMethod == 2 )
        {
            if( gLog && !gLog->fail( ) )
            {
                time_t Now = time( NULL );
                string Time = asctime( localtime( &Now ) );

                // erase the newline

                Time.erase( Time.size( ) - 1 );
                *gLog << "[" << Time << "] " << message << endl;
                gLog->flush( );
            }
        }
    }
}


void DEBUG_Print( string message )
{
    cout << message << endl;
}

void DEBUG_Print( BYTEARRAY b )
{
    cout << "{ ";

    for( unsigned int i = 0; i < b.size( ); ++i )
        cout << hex << (int)b[i] << " ";

    cout << "}" << endl;
}

//
// main
//

int main( int argc, char **argv )
{
#ifndef WIN32
    signal( SIGILL, SignalSIGSEGV );
    signal( SIGSEGV, SignalSIGSEGV );
    signal( SIGFPE, SignalSIGSEGV );
#endif
    srand( time( NULL ) );

    CONSOLE_Print("***************************************************************************************");
    CONSOLE_Print("**                      WELCOME TO THE OHSYSTEM BOT V2                               **");
    CONSOLE_Print("**       PLEASE DO NOT REMOVE ANY COPYRIGHT NOTICE TO RESPECT THE PROJECT            **");
    CONSOLE_Print("**       ----------------------------------------------------------------            **");
    CONSOLE_Print("**        For any questions and required support use our git repository              **");
    CONSOLE_Print("**                    https://github.com/OHSystem/OHSystem                           **");
    CONSOLE_Print("***************************************************************************************");
    CONSOLE_Print("");
    CONSOLE_Print("***************************************************************************************");
    CONSOLE_Print("**                             INITIALIZE GHOST MODULE                               **");
    CONSOLE_Print("***************************************************************************************");
    CONSOLE_Print("");

    gCFGFile = "ohbot.cfg";

    if( argc > 1 && argv[1] )
        gCFGFile = argv[1];

    // read config file

    CFG.Read( "default.cfg" );
    gLogFile = CFG.GetString( "bot_log", "ohbot_" + UTIL_ToString( GetTime( ) ) + ".log" );
    gLogMethod = CFG.GetInt( "bot_logmethod", 1 );

    if( !gLogFile.empty( ) )
    {
        if( gLogMethod == 1 )
        {
            // log method 1: open, append, and close the log for every message
            // this works well on Linux but poorly on Windows, particularly as the log file grows in size
            // the log file can be edited/moved/deleted while GHost++ is running
        }
        else if( gLogMethod == 2 )
        {
            // log method 2: open the log on startup, flush the log for every message, close the log on shutdown
            // the log file CANNOT be edited/moved/deleted while GHost++ is running

            gLog = new ofstream( );
            gLog->open( gLogFile.c_str( ), ios :: app );
        }
    }

    CONSOLE_Print( "[GHOST] starting up" );

    if( !gLogFile.empty( ) )
    {
        if( gLogMethod == 1 )
            CONSOLE_Print( "[GHOST] using log method 1, logging is enabled and [" + gLogFile + "] will not be locked" );
        else if( gLogMethod == 2 )
        {
            if( gLog->fail( ) )
                CONSOLE_Print( "[GHOST] using log method 2 but unable to open [" + gLogFile + "] for appending, logging is disabled" );
            else
                CONSOLE_Print( "[GHOST] using log method 2, logging is enabled and [" + gLogFile + "] is now locked" );
        }
    }
    else
        CONSOLE_Print( "[GHOST] no log file specified, logging is disabled" );

    // catch SIGABRT and SIGINT

    // signal( SIGABRT, SignalCatcher );
    signal( SIGINT, SignalCatcher );

#ifndef WIN32
    // disable SIGPIPE since some systems like OS X don't define MSG_NOSIGNAL

    signal( SIGPIPE, SIG_IGN );
#endif

#ifdef WIN32
    // initialize timer resolution
    // attempt to set the resolution as low as possible from 1ms to 5ms

    unsigned int TimerResolution = 0;

    for( unsigned int i = 1; i <= 5; ++i )
    {
        if( timeBeginPeriod( i ) == TIMERR_NOERROR )
        {
            TimerResolution = i;
            break;
        }
        else if( i < 5 )
            CONSOLE_Print( "[GHOST] error setting Windows timer resolution to " + UTIL_ToString( i ) + " milliseconds, trying a higher resolution" );
        else
        {
            CONSOLE_Print( "[GHOST] error setting Windows timer resolution" );
            return 1;
        }
    }

    CONSOLE_Print( "[GHOST] using Windows timer with resolution " + UTIL_ToString( TimerResolution ) + " milliseconds" );
#elif __APPLE__
    // not sure how to get the resolution
#else
    // print the timer resolution

    struct timespec Resolution;

    if( clock_getres( CLOCK_MONOTONIC, &Resolution ) == -1 )
        CONSOLE_Print( "[GHOST] error getting monotonic timer resolution" );
    else
        CONSOLE_Print( "[GHOST] using monotonic timer with resolution " + UTIL_ToString( (double)( Resolution.tv_nsec / 1000 ), 2 ) + " microseconds" );
#endif

#ifdef WIN32
    // initialize winsock

    CONSOLE_Print( "[GHOST] starting winsock" );
    WSADATA wsadata;

    if( WSAStartup( MAKEWORD( 2, 2 ), &wsadata ) != 0 )
    {
        CONSOLE_Print( "[GHOST] error starting winsock" );
        return 1;
    }

    // increase process priority

    CONSOLE_Print( "[GHOST] setting process priority to \"above normal\"" );
    SetPriorityClass( GetCurrentProcess( ), ABOVE_NORMAL_PRIORITY_CLASS );
#endif
    // initialize ohbot

    gGHost = new COHBot( &CFG );

    while( 1 )
    {
        // block for 50ms on all sockets - if you intend to perform any timed actions more frequently you should change this
        // that said it's likely we'll loop more often than this due to there being data waiting on one of the sockets but there aren't any guarantees

        if( gGHost->Update( 50000 ) )
            break;
    }

    // shutdown ohbot

    CONSOLE_Print( "[GHOST] shutting down" );
    delete gGHost;
    gGHost = NULL;
#ifdef WIN32
    // shutdown winsock

    CONSOLE_Print( "[GHOST] shutting down winsock" );
    WSACleanup( );

    // shutdown timer

    timeEndPeriod( TimerResolution );
#endif

    if( gLog )
    {
        if( !gLog->fail( ) )
            gLog->close( );

        delete gLog;
    }

    return 0;
}

//
// COHBot
//

COHBot :: COHBot( CConfig *CFG )
{
    m_UDPSocket = new CUDPSocket( );
    m_GarenaSocket = new CUDPSocket( );
    m_UDPSocket->SetBroadcastTarget( CFG->GetString( "udp_broadcasttarget", string( ) ) );
    m_UDPSocket->SetDontRoute( CFG->GetInt( "udp_dontroute", 0 ) == 0 ? false : true );
    m_GarenaSocket->SetBroadcastTarget( CFG->GetString( "garena_broadcasttarget", string( ) ) );
    m_GarenaSocket->SetDontRoute( true );
    m_ReconnectSocket = NULL;
    m_GPSProtocol = new CGPSProtocol( );
    m_GCBIProtocol = new CGCBIProtocol( );
    m_CRC = new CCRC32( );
    m_CRC->Initialize( );
    m_SHA = new CSHA1( );
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
    m_RanksLoaded = true;
    m_ReservedHostCounter = 0;
    m_TicksCollectionTimer = GetTicks();
    m_TicksCollection = 0;
    m_MaxTicks = 0;
    m_MinTicks = -1;
    m_Sampler = 0;
    string DBType = CFG->GetString( "db_type", "mysql" );
    CONSOLE_Print( "[GHOST] opening primary database" );

    m_DB = new COHBotDBMySQL( CFG );

    // get a list of local IP addresses
    // this list is used elsewhere to determine if a player connecting to the bot is local or not

    CONSOLE_Print( "[GHOST] attempting to find local IP addresses" );

#ifdef WIN32
    // use a more reliable Windows specific method since the portable method doesn't always work properly on Windows
    // code stolen from: http://tangentsoft.net/wskfaq/examples/getifaces.html

    SOCKET sd = WSASocket( AF_INET, SOCK_DGRAM, 0, 0, 0, 0 );

    if( sd == SOCKET_ERROR )
        CONSOLE_Print( "[GHOST] error finding local IP addresses - failed to create socket (error code " + UTIL_ToString( WSAGetLastError( ) ) + ")" );
    else
    {
        INTERFACE_INFO InterfaceList[20];
        unsigned long nBytesReturned;

        if( WSAIoctl( sd, SIO_GET_INTERFACE_LIST, 0, 0, &InterfaceList, sizeof(InterfaceList), &nBytesReturned, 0, 0 ) == SOCKET_ERROR )
            CONSOLE_Print( "[GHOST] error finding local IP addresses - WSAIoctl failed (error code " + UTIL_ToString( WSAGetLastError( ) ) + ")" );
        else
        {
            int nNumInterfaces = nBytesReturned / sizeof(INTERFACE_INFO);

            for( int i = 0; i < nNumInterfaces; ++i )
            {
                sockaddr_in *pAddress;
                pAddress = (sockaddr_in *)&(InterfaceList[i].iiAddress);
                CONSOLE_Print( "[GHOST] local IP address #" + UTIL_ToString( i + 1 ) + " is [" + string( inet_ntoa( pAddress->sin_addr ) ) + "]" );
                m_LocalAddresses.push_back( UTIL_CreateByteArray( (uint32_t)pAddress->sin_addr.s_addr, false ) );
            }
        }

        closesocket( sd );
    }
#else
    // use a portable method

    char HostName[255];

    if( gethostname( HostName, 255 ) == SOCKET_ERROR )
        CONSOLE_Print( "[GHOST] error finding local IP addresses - failed to get local hostname" );
    else
    {
        CONSOLE_Print( "[GHOST] local hostname is [" + string( HostName ) + "]" );
        struct hostent *HostEnt = gethostbyname( HostName );

        if( !HostEnt )
            CONSOLE_Print( "[GHOST] error finding local IP addresses - gethostbyname failed" );
        else
        {
            for( int i = 0; HostEnt->h_addr_list[i] != NULL; ++i )
            {
                struct in_addr Address;
                memcpy( &Address, HostEnt->h_addr_list[i], sizeof(struct in_addr) );
                CONSOLE_Print( "[GHOST] local IP address #" + UTIL_ToString( i + 1 ) + " is [" + string( inet_ntoa( Address ) ) + "]" );
                m_LocalAddresses.push_back( UTIL_CreateByteArray( (uint32_t)Address.s_addr, false ) );
            }
        }
    }
#endif
    m_Language = NULL;
    isCreated = false;
    m_Exiting = false;
    m_ExitingNice = false;
    m_Enabled = true;
    m_Version = "17.2";
    m_AutoHostMaximumGames = CFG->GetInt( "autohost_maxgames", 0 );
    m_AutoHostAutoStartPlayers = CFG->GetInt( "autohost_startplayers", 0 );
    m_AutoHostGameName = CFG->GetString( "autohost_gamename", string( ) );
    m_AutoHostOwner = CFG->GetString( "autohost_owner", string( ) );
    m_LastAutoHostTime = GetTime( );
    m_LastCommandListTime = GetTime( );
    m_LastFlameListUpdate = 0;
    m_LastGProxyListUpdate=0;
    m_LastAliasListUpdate = 0;
    m_LastAnnounceListUpdate = 0;
    m_LastDNListUpdate = 0;
    m_LastDCountryUpdate = 0;
    m_LastHCUpdate = GetTime();
    m_AutoHostMatchMaking = false;
    m_AutoHostMinimumScore = 0.0;
    m_AutoHostMaximumScore = 0.0;
    if( m_TFT )
        CONSOLE_Print( "[GHOST] acting as Warcraft III: The Frozen Throne" );
    else
        CONSOLE_Print( "[GHOST] acting as Warcraft III: Reign of Chaos" );

    m_HostPort = CFG->GetInt( "bot_hostport", 6112 );
    m_Reconnect = CFG->GetInt( "bot_reconnect", 1 ) == 0 ? false : true;
    m_ReconnectPort = CFG->GetInt( "bot_reconnectport", 6114 );
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
        string RootAdmin = CFG->GetString( Prefix + "rootadmin", string( ) );
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
            CONSOLE_Print( "[GHOST] missing " + Prefix + "cdkeyroc, skipping this battle.net connection" );
            continue;
        }

        if( m_TFT && CDKeyTFT.empty( ) )
        {
            CONSOLE_Print( "[GHOST] missing " + Prefix + "cdkeytft, skipping this battle.net connection" );
            continue;
        }

        if( UserName.empty( ) )
        {
            CONSOLE_Print( "[GHOST] missing " + Prefix + "username, skipping this battle.net connection" );
            continue;
        }

        if( UserPassword.empty( ) )
        {
            CONSOLE_Print( "[GHOST] missing " + Prefix + "password, skipping this battle.net connection" );
            continue;
        }

        CONSOLE_Print( "[GHOST] found battle.net connection #" + UTIL_ToString( i ) + " for server [" + Server + "]" );

        if( Locale == "system" )
        {
#ifdef WIN32
            CONSOLE_Print( "[GHOST] using system locale of " + UTIL_ToString( LocaleID ) );
#else
            CONSOLE_Print( "[GHOST] unable to get system locale, using default locale of 1033" );
#endif
        }

        m_BNETs.push_back( new CBNET( this, Server, ServerAlias, BNLSServer, (uint16_t)BNLSPort, (uint32_t)BNLSWardenCookie, CDKeyROC, CDKeyTFT, CountryAbbrev, Country, LocaleID, UserName, UserPassword, FirstChannel, BNETCommandTrigger[0], HoldFriends, HoldClan, PublicCommands, War3Version, EXEVersion, EXEVersionHash, PasswordHashType, PVPGNRealmName, MaxMessageLength, i, UpTime ) );
        counter++;
    }
    CONSOLE_Print( "[GHOST] Adding hardcoded Garena Realm & WC3Connect Realm." );
    m_BNETs.push_back( new CBNET( this, "Garena", "Garena", string( ), 0, 0, string( ), string( ), string( ), string( ), 1033, string( ), string( ), string( ), m_CommandTrigger, 0, 0, 1, 26, UTIL_ExtractNumbers( string( ), 4 ), UTIL_ExtractNumbers( string( ), 4 ), string( ), string( ), 200, counter+1, 0 ) );
    m_BNETs.push_back( new CBNET( this, m_WC3ConnectAlias, "WC3Connect", string( ), 0, 0, string( ), string( ), string( ), string( ), 1033, string( ), string( ), string( ), m_CommandTrigger, 0, 0, 1, 26, UTIL_ExtractNumbers( string( ), 4 ), UTIL_ExtractNumbers( string( ), 4 ), string( ), string( ), 200, counter+2, 0 ) );

    if( m_BNETs.size( ) == 2 ) {
        CONSOLE_Print( "[GHOST] warning - no battle.net connections found in config file. Only the hardcoded" );
    }
    // extract common.j and blizzard.j from War3Patch.mpq if we can
    // these two files are necessary for calculating "map_crc" when loading maps so we make sure to do it before loading the default map
    // see CMap :: Load for more information

    ExtractScripts( );

    // load the default maps (note: make sure to run ExtractScripts first)

    if( m_DefaultMap.size( ) < 4 || m_DefaultMap.substr( m_DefaultMap.size( ) - 4 ) != ".cfg" )
    {
        m_DefaultMap += ".cfg";
        CONSOLE_Print( "[GHOST] adding \".cfg\" to default map -> new default is [" + m_DefaultMap + "]" );
    }

    CConfig MapCFG;
    MapCFG.Read( m_MapCFGPath + m_DefaultMap );
    m_Map = new CMap( this, &MapCFG, m_MapCFGPath + m_DefaultMap );

    m_AutoHostMap = new CMap( *m_Map );
    m_SaveGame = new CSaveGame( );

    if( m_BNETs.empty( ) )
    {
        CONSOLE_Print( "[GHOST] warning - no battle.net connections found and no admin game created" );
    }

    CONSOLE_Print( "[GHOST] GHost++ Version " + m_Version + " (with MySQL support)" );
}

COHBot :: ~COHBot( )
{
    delete m_UDPSocket;
    delete m_GarenaSocket;
    delete m_ReconnectSocket;

    for( vector<CTCPSocket *> :: iterator i = m_ReconnectSockets.begin( ); i != m_ReconnectSockets.end( ); ++i )
        delete *i;

    delete m_GPSProtocol;
    delete m_GCBIProtocol;
    delete m_CRC;
    delete m_SHA;

    for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); ++i )
        delete *i;

    if( m_CurrentGame )
	m_CurrentGame->doDelete();

    for( vector<CBaseGame *> :: iterator i = m_Games.begin( ); i != m_Games.end( ); ++i ) {
	(*i)->doDelete();
    }

    delete m_DB;

    // warning: we don't delete any entries of m_Callables here because we can't be guaranteed that the associated threads have terminated
    // this is fine if the program is currently exiting because the OS will clean up after us
    // but if you try to recreate the COHBot object within a single session you will probably leak resources!

//	if( !m_Callables.empty( ) )
//		CONSOLE_Print( "[GHOST] warning - " + UTIL_ToString( m_Callables.size( ) ) + " orphaned callables were leaked (this is not an error)" );

    delete m_Language;
    delete m_Map;
    delete m_AdminMap;
    delete m_AutoHostMap;
    delete m_SaveGame;
}

bool COHBot :: Update( long usecBlock )
{
    m_StartTicks = GetTicks();

    // todotodo: do we really want to shutdown if there's a database error? is there any way to recover from this?

    if( m_DB->HasError( ) )
    {
        CONSOLE_Print( "[GHOST] database error - " + m_DB->GetError( ) );
        return true;
    }

	boost::mutex::scoped_lock gamesLock( m_GamesMutex );
	
	// get rid of any deleted games
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


    // try to exit nicely if requested to do so

    if( m_ExitingNice )
    {
        if( !m_BNETs.empty( ) )
        {
            CONSOLE_Print( "[GHOST] deleting all battle.net connections in preparation for exiting nicely" );

            for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); ++i )
                delete *i;

            m_BNETs.clear( );
        }

        if( m_CurrentGame )
        {
            CONSOLE_Print( "[GHOST] deleting current game in preparation for exiting nicely" );
            m_CurrentGame->doDelete( );
            m_CurrentGame = NULL;
        }

        if( m_Games.empty( ) )
        {
            if( !m_AllGamesFinished )
            {
                CONSOLE_Print( "[GHOST] all games finished, waiting 60 seconds for threads to finish" );
                CONSOLE_Print( "[GHOST] there are " + UTIL_ToString( m_Callables.size( ) ) + " threads in progress" );
                m_AllGamesFinished = true;
                m_AllGamesFinishedTime = GetTime( );
            }
            else
            {
                if( m_Callables.empty( ) )
                {
                    CONSOLE_Print( "[GHOST] all threads finished, exiting nicely" );
                    m_Exiting = true;
                }
                else if( GetTime( ) - m_AllGamesFinishedTime >= 60 )
                {
                    CONSOLE_Print( "[GHOST] waited 60 seconds for threads to finish, exiting anyway" );
                    CONSOLE_Print( "[GHOST] there are " + UTIL_ToString( m_Callables.size( ) ) + " threads still in progress which will be terminated" );
                    m_Exiting = true;
                }
            }
        }
    }

    // update callables

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

    // create the GProxy++ reconnect listener

    if( m_Reconnect )
    {
        if( !m_ReconnectSocket )
        {
            m_ReconnectSocket = new CTCPServer( );

            if( m_ReconnectSocket->Listen( m_BindAddress, m_ReconnectPort ) )
                CONSOLE_Print( "[GHOST] listening for GProxy++ reconnects on port " + UTIL_ToString( m_ReconnectPort ) );
            else
            {
                CONSOLE_Print( "[GHOST] error listening for GProxy++ reconnects on port " + UTIL_ToString( m_ReconnectPort ) );
                delete m_ReconnectSocket;
                m_ReconnectSocket = NULL;

                m_Reconnect = false;
            }
        }
        else if( m_ReconnectSocket->HasError( ) )
        {
            CONSOLE_Print( "[GHOST] GProxy++ reconnect listener error (" + m_ReconnectSocket->GetErrorString( ) + ")" );
            delete m_ReconnectSocket;
            m_ReconnectSocket = NULL;
            m_Reconnect = false;
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

    if( m_Reconnect && m_ReconnectSocket )
    {
        m_ReconnectSocket->SetFD( &fd, &send_fd, &nfds );
        ++NumFDs;
    }

    for( vector<CTCPSocket *> :: iterator i = m_ReconnectSockets.begin( ); i != m_ReconnectSockets.end( ); ++i )
    {
        (*i)->SetFD( &fd, &send_fd, &nfds );
        ++NumFDs;
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

    // update GProxy++ reliable reconnect sockets

    if( m_Reconnect && m_ReconnectSocket )
    {
        CTCPSocket *NewSocket = m_ReconnectSocket->Accept( &fd );

        if( NewSocket )
            m_ReconnectSockets.push_back( NewSocket );
    }

    for( vector<CTCPSocket *> :: iterator i = m_ReconnectSockets.begin( ); i != m_ReconnectSockets.end( ); )
    {
        if( (*i)->HasError( ) || !(*i)->GetConnected( ) || GetTime( ) - (*i)->GetLastRecv( ) >= 10 )
        {
            delete *i;
            i = m_ReconnectSockets.erase( i );
            continue;
        }

        (*i)->DoRecv( &fd );
        string *RecvBuffer = (*i)->GetBytes( );
        BYTEARRAY Bytes = UTIL_CreateByteArray( (unsigned char *)RecvBuffer->c_str( ), RecvBuffer->size( ) );

        // a packet is at least 4 bytes

        if( Bytes.size( ) >= 4 )
        {
            if( Bytes[0] == GPS_HEADER_CONSTANT )
            {
                // bytes 2 and 3 contain the length of the packet

                uint16_t Length = UTIL_ByteArrayToUInt16( Bytes, false, 2 );

                if( Length >= 4 )
                {
                    if( Bytes.size( ) >= Length )
                    {
                        if( Bytes[1] == CGPSProtocol :: GPS_RECONNECT && Length == 13 )
                        {
				GProxyReconnector *Reconnector = new GProxyReconnector;
				Reconnector->PID = Bytes[4];
				Reconnector->ReconnectKey = UTIL_ByteArrayToUInt32( Bytes, false, 5 );
				Reconnector->LastPacket = UTIL_ByteArrayToUInt32( Bytes, false, 9 );
				Reconnector->PostedTime = GetTicks( );
				Reconnector->socket = (*i);
							
				// update the receive buffer
				*RecvBuffer = RecvBuffer->substr( Length );
				i = m_ReconnectSockets.erase( i );
				// post in the reconnects buffer and wait to see if a game thread will pick it up
				boost::mutex::scoped_lock lock( m_ReconnectMutex );
				m_PendingReconnects.push_back( Reconnector );
				lock.unlock();
				continue;
                        }
                        else
                        {
                            (*i)->PutBytes( m_GPSProtocol->SEND_GPSS_REJECT( REJECTGPS_INVALID ) );
                            (*i)->DoSend( &send_fd );
                            delete *i;
                            i = m_ReconnectSockets.erase( i );
                            continue;
                        }
                    }
                }
                else
                {
                    (*i)->PutBytes( m_GPSProtocol->SEND_GPSS_REJECT( REJECTGPS_INVALID ) );
                    (*i)->DoSend( &send_fd );
                    delete *i;
                    i = m_ReconnectSockets.erase( i );
                    continue;
                }
            }
            else
            {
                (*i)->PutBytes( m_GPSProtocol->SEND_GPSS_REJECT( REJECTGPS_INVALID ) );
                (*i)->DoSend( &send_fd );
                delete *i;
                i = m_ReconnectSockets.erase( i );
                continue;
            }
        }

        (*i)->DoSend( &send_fd );
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

    // autohost

    if( !m_AutoHostGameName.empty( ) && m_AutoHostMaximumGames != 0 && m_AutoHostAutoStartPlayers != 0 && GetTime( ) - m_LastAutoHostTime >= 30 && m_ReservedHostCounter != 0 )
    {
        // copy all the checks from COHBot :: CreateGame here because we don't want to spam the chat when there's an error
        // instead we fail silently and try again soon

        if( !m_ExitingNice && m_Enabled && !m_CurrentGame && m_Games.size( ) < m_MaxGames && m_Games.size( ) < m_AutoHostMaximumGames )
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
                                    CONSOLE_Print( "[GHOST] autohostmm - map_matchmakingcategory [" + m_Map->GetMapMatchMakingCategory( ) + "] found but matchmaking can only be used with fixed player settings, matchmaking disabled" );
                                else
                                {
                                    CONSOLE_Print( "[GHOST] autohostmm - map_matchmakingcategory [" + m_Map->GetMapMatchMakingCategory( ) + "] found, matchmaking enabled" );

                                    m_CurrentGame->SetMatchMaking( true );
                                    m_CurrentGame->SetMinimumScore( m_AutoHostMinimumScore );
                                    m_CurrentGame->SetMaximumScore( m_AutoHostMaximumScore );
                                }
                            }
                            else
                                CONSOLE_Print( "[GHOST] autohostmm - map_matchmakingcategory not found, matchmaking disabled" );
                        }
                    }
                }
                else
                {
                    CONSOLE_Print( "[GHOST] stopped auto hosting, next game name [" + GameName + "] is too long (the maximum is 31 characters)" );
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
                CONSOLE_Print( "[GHOST] stopped auto hosting, map config file [" + m_AutoHostMap->GetCFGFile( ) + "] is invalid" );
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

	string command;

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
            m_Exiting = true;
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
        CONSOLE_Print("[OHSystem-Performance-Check] AVGTicks: "+UTIL_ToString(m_AVGTicks, 0)+"ms | MaxTicks: "+UTIL_ToString(m_MaxTicks)+"ms | MinTicks: "+UTIL_ToString(m_MinTicks)+"ms | Updates: "+UTIL_ToString(m_Sampler));
        CONSOLE_Print("[OHSystem-Performance-CHeck] MySQL: "+m_DB->GetStatus( ));
        m_MinTicks = -1;
        m_MaxTicks = 0;
        m_TicksCollection = 0;
        m_Sampler = 0;
    }

    return m_Exiting || AdminExit || BNETExit;
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

void COHBot :: ReloadConfigs( )
{
    CConfig CFG;
    CFG.Read( "default.cfg" );
    CFG.Read( gCFGFile );
    SetConfigs( &CFG );
}

void COHBot :: SetConfigs( CConfig *CFG )
{
    // this doesn't set EVERY config value since that would potentially require reconfiguring the battle.net connections
    // it just set the easily reloadable values
    m_LanCFGPath = UTIL_AddPathSeperator( CFG->GetString( "languages_path", "languages/" ) );
    delete m_Language;
    m_Language = new CLanguage( CFG->GetString( "bot_language", "en.cfg" )+m_LanguageFile );
    m_Warcraft3Path = UTIL_AddPathSeperator( CFG->GetString( "bot_war3path", "C:\\Program Files\\Warcraft III\\" ) );
    m_BindAddress = CFG->GetString( "bot_bindaddress", string( ) );
    m_ReconnectWaitTime = CFG->GetInt( "bot_reconnectwaittime", 3 );
    m_MaxGames = CFG->GetInt( "bot_maxgames", 5 );
    string BotCommandTrigger = CFG->GetString( "bot_commandtrigger", "!" );

    if( BotCommandTrigger.empty( ) )
        BotCommandTrigger = "!";

    m_CommandTrigger = BotCommandTrigger[0];
    m_MapCFGPath = UTIL_AddPathSeperator( CFG->GetString( "bot_mapcfgpath", string( ) ) );
    m_GameLogging = CFG->GetInt( "game_logging", 0 ) == 0 ? false : true;
    m_GameLoggingID = CFG->GetInt( "game_loggingid", 1 );
    m_GameLogFilePath = UTIL_AddPathSeperator( CFG->GetString( "game_logpath", string( ) ) );
    m_ColoredNamePath = UTIL_AddPathSeperator( CFG->GetString( "oh_coloredname", string( ) ) );
    m_SaveGamePath = UTIL_AddPathSeperator( CFG->GetString( "bot_savegamepath", string( ) ) );
    m_MapPath = UTIL_AddPathSeperator( CFG->GetString( "bot_mappath", string( ) ) );
    m_SaveReplays = CFG->GetInt( "bot_savereplays", 0 ) == 0 ? false : true;
    m_ReplayPath = UTIL_AddPathSeperator( CFG->GetString( "bot_replaypath", string( ) ) );
    m_VirtualHostName = CFG->GetString( "bot_virtualhostname", "|cFF4080C0GHost" );
    m_HideIPAddresses = CFG->GetInt( "bot_hideipaddresses", 0 ) == 0 ? false : true;
    m_CheckMultipleIPUsage = CFG->GetInt( "bot_checkmultipleipusage", 1 ) == 0 ? false : true;

    if( m_VirtualHostName.size( ) > 15 )
    {
        m_VirtualHostName = "|cFF4080C0GHost";
        CONSOLE_Print( "[GHOST] warning - bot_virtualhostname is longer than 15 characters, using default virtual host name" );
    }

    m_SpoofChecks = CFG->GetInt( "bot_spoofchecks", 2 );
    m_RequireSpoofChecks = CFG->GetInt( "bot_requirespoofchecks", 0 ) == 0 ? false : true;
    m_ReserveAdmins = CFG->GetInt( "bot_reserveadmins", 1 ) == 0 ? false : true;
    m_RefreshMessages = CFG->GetInt( "bot_refreshmessages", 0 ) == 0 ? false : true;
    m_AutoLock = CFG->GetInt( "bot_autolock", 0 ) == 0 ? false : true;
    m_AutoSave = CFG->GetInt( "bot_autosave", 0 ) == 0 ? false : true;
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
    m_VoteKickAllowed = CFG->GetInt( "bot_votekickallowed", 1 ) == 0 ? false : true;
    m_VoteKickPercentage = CFG->GetInt( "bot_votekickpercentage", 100 );
    if( m_VoteKickPercentage > 100 )
    {
        m_VoteKickPercentage = 100;
        CONSOLE_Print( "[GHOST] warning - bot_votekickpercentage is greater than 100, using 100 instead" );
    }

    m_MOTDFile = CFG->GetString( "bot_motdfile", "motd.txt" );
    m_GameLoadedFile = CFG->GetString( "bot_gameloadedfile", "gameloaded.txt" );
    m_GameOverFile = CFG->GetString( "bot_gameoverfile", "gameover.txt" );
    m_LocalAdminMessages = CFG->GetInt( "bot_localadminmessages", 1 ) == 0 ? false : true;
    m_TCPNoDelay = CFG->GetInt( "tcp_nodelay", 0 ) == 0 ? false : true;
    m_MatchMakingMethod = CFG->GetInt( "bot_matchmakingmethod", 1 );
    m_MapGameType = CFG->GetUInt( "bot_mapgametype", 0 );
    m_AutoHostGameType = CFG->GetInt( "oh_autohosttype", 3 );
    m_AllGamesFinished = false;
    m_AllGamesFinishedTime = 0;
    m_MinVIPGames = CFG->GetInt( "vip_mingames", 25 );
    m_RegVIPGames = CFG->GetInt( "vip_reg", 0 ) == 0 ? false : true;
    m_TFT = CFG->GetInt( "bot_tft", 1 ) == 0 ? false : true;
    m_OHBalance = CFG->GetInt( "oh_balance", 1 ) == 0 ? false : true;
    m_HighGame = CFG->GetInt( "oh_rankedgame", 0 ) == 0 ? false : true;
    m_MinLimit = CFG->GetInt( "oh_mingames", 25 );
    m_ObserverFake = CFG->GetInt( "oh_observer", 1 ) == 0 ? false : true;
    m_NoGarena = CFG->GetInt( "oh_nogarena", 0 ) == 0 ? false : true;
    m_CheckIPRange = CFG->GetInt( "oh_checkiprangeonjoin", 0 ) == 0 ? false : true;
    m_DenieProxy = CFG->GetInt( "oh_proxykick", 0 ) == 0 ? false : true;
    m_LiveGames = CFG->GetInt( "oh_general_livegames", 1 ) == 0 ? false : true;
    m_MinFF = CFG->GetInt( "oh_minff", 20 );
    m_MinimumLeaverKills = CFG->GetInt( "antifarm_minkills", 3 );
    m_MinimumLeaverDeaths = CFG->GetInt( "antifarm_mindeaths", 3 );
    m_MinimumLeaverAssists = CFG->GetInt( "antifarm_minassists", 3 );
    m_DeathsByLeaverReduction =  CFG->GetInt( "antifarm_deathsbyleaver", 1 );
    m_MinPlayerAutoEnd = CFG->GetInt( "autoend_minplayer", 2 );
    m_MaxAllowedSpread = CFG->GetInt( "autoend_maxspread", 2 );
    m_EarlyEnd = CFG->GetInt( "autoend_earlyend", 1 ) == 0 ? false : true;
    m_StatsUpdate = CFG->GetInt( "oh_general_updatestats", 0 ) == 0 ? false : true;
    m_MessageSystem = CFG->GetInt("oh_general_messagesystem", 1 ) == 0 ? false : true;
    m_FunCommands = CFG->GetInt("oh_general_funcommands", 1 ) == 0 ? false : true;
    m_BetSystem = CFG->GetInt("oh_general_betsystem", 1 ) == 0 ? false : true;
    m_AccountProtection = CFG->GetInt("oh_general_accountprotection", 1 ) == 0 ? false : true;
    m_Announce = CFG->GetInt("oh_announce", 0 ) == 0 ? false : true;
    m_AnnounceHidden = CFG->GetInt("oh_hiddenAnnounce", 0 ) == 0 ? false : true;
    m_FountainFarmWarning = CFG->GetInt("oh_fountainfarm_warning", 0 ) == 0 ? false : true;
    m_FountainFarmMessage = CFG->GetString("oh_fountainfarm_message", "Reminder: Any kind of fountainfarming, or even an attempt, is bannable." );
    m_AutoDenyUsers = CFG->GetInt("oh_general_autodeny", 0) == 0 ? false : true;
    m_AllowVoteStart = CFG->GetInt("oh_allowvotestart", 0) == 0 ? false : true;
    m_VoteStartMinPlayers = CFG->GetInt("oh_votestartminimumplayers", 3);
    m_AutoMuteSpammer = CFG->GetInt( "oh_mutespammer", 1 ) == 0 ? false : true;
    m_FlameCheck = CFG->GetInt("oh_flamecheck", 0) == 0 ? false : true;
    m_BotManagerName = CFG->GetString( "oh_general_botmanagername", "PeaceMaker" );
    m_IngameVoteKick = CFG->GetInt("oh_ingamevotekick", 1) == 0 ? false : true;
    m_LeaverAutoBanTime = CFG->GetInt("oh_leaverautobantime", 259200);
    m_DisconnectAutoBanTime = CFG->GetInt("oh_disconnectautobantime", 86400);
    m_FirstFlameBanTime = CFG->GetInt("oh_firstflamebantime", 172800 );
    m_SecondFlameBanTime = CFG->GetInt("oh_secondflamebantime", 345600);
    m_SpamBanTime = CFG->GetInt("oh_spambantime", 172800 );
    m_VKAbuseBanTime = CFG->GetInt("oh_votekickabusebantime", 432000);
    m_VoteMuting = CFG->GetInt("oh_votemute", 1) == 0 ? false : true;
    m_VoteMuteTime = CFG->GetInt("oh_votemutetime", 180);
    m_AutoEndTime = CFG->GetInt("autoend_votetime", 120);
    m_AllowHighPingSafeDrop = CFG->GetInt("oh_allowsafedrop", 1) == 0 ? false : true;
    m_MinPauseLevel = CFG->GetInt("oh_minpauselevel", 3);
    m_MinScoreLimit = CFG->GetInt("oh_minscorelimit", 0);
    m_MaxScoreLimit = CFG->GetInt("oh_maxscorelimit", 0);
    m_AutobanAll = CFG->GetInt("oh_autobanall", 1) == 0 ? false : true;
    m_WC3ConnectAlias = CFG->GetString("wc3connect_alias", "WC3Connect");
    m_ChannelBotOnly = CFG->GetInt("oh_channelbot", 0) == 0 ? false : true;
    m_NonAllowedDonwloadMessage = CFG->GetString("oh_downloadmessage", string());
    m_VoteMode = CFG->GetInt( "oh_votemode", 0 ) == 0 ? false : true;
    m_MaxVotingTime = CFG->GetInt( "oh_votemode_time", 30 );
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
    m_Website = CFG->GetString("oh_general_domain", "http://ohsystem.net/" );
    m_SharedFilesPath = UTIL_AddPathSeperator( CFG->GetString( "bot_sharedfilespath", string( ) ) );
    m_BroadCastPort = CFG->GetInt("oh_broadcastport", 6112 );
    m_SpoofPattern = CFG->GetString("oh_spoofpattern", string());
    m_DelayGameLoaded = CFG->GetInt("oh_delaygameloaded", 300);
    m_FountainFarmDetection = CFG->GetInt("oh_fountainfarmdetection", 1) == 0 ? false : true;
    m_AutokickSpoofer = CFG->GetInt("oh_autokickspoofer", 1) == 0 ? false : true;
    m_ReadGlobalMySQL = CFG->GetInt("oh_readglobalmysql", 0) == 0 ? false : true;
    m_GlobalMySQLPath = UTIL_AddPathSeperator( CFG->GetString( "oh_globalmysqlpath", "../" ) );
    m_PVPGNMode = CFG->GetInt("oh_pvpgn_mode", 0) == 0 ? false : true;
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
 
    LoadDatas();
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
        CONSOLE_Print( "[GHOST] loading MPQ file [" + PatchMPQFileName + "]" );
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
                    CONSOLE_Print( "[GHOST] extracting Scripts\\common.j from MPQ file to [" + m_MapCFGPath + "common.j]" );
                    UTIL_FileWrite( m_MapCFGPath + "common.j", (unsigned char *)SubFileData, BytesRead );
                }
                else
                    CONSOLE_Print( "[GHOST] warning - unable to extract Scripts\\common.j from MPQ file" );

                delete [] SubFileData;
            }

            SFileCloseFile( SubFile );
        }
        else
            CONSOLE_Print( "[GHOST] couldn't find Scripts\\common.j in MPQ file" );

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
                    CONSOLE_Print( "[GHOST] extracting Scripts\\blizzard.j from MPQ file to [" + m_MapCFGPath + "blizzard.j]" );
                    UTIL_FileWrite( m_MapCFGPath + "blizzard.j", (unsigned char *)SubFileData, BytesRead );
                }
                else
                    CONSOLE_Print( "[GHOST] warning - unable to extract Scripts\\blizzard.j from MPQ file" );

                delete [] SubFileData;
            }

            SFileCloseFile( SubFile );
        }
        else
            CONSOLE_Print( "[GHOST] couldn't find Scripts\\blizzard.j in MPQ file" );

        SFileCloseArchive( PatchMPQ );
    }
    else
        CONSOLE_Print( "[GHOST] warning - unable to load MPQ file [" + PatchMPQFileName + "] - error code " + UTIL_ToString( GetLastError( ) ) );
}

void COHBot :: CreateGame( CMap *map, unsigned char gameState, bool saveGame, string gameName, string ownerName, string creatorName, string creatorServer, uint32_t gameType, bool whisper, uint32_t m_HostCounter )
{
    if( !m_Enabled )
    {
        for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); ++i )
        {
            if( (*i)->GetServer( ) == creatorServer )
                (*i)->QueueChatCommand( m_Language->UnableToCreateGameDisabled( gameName ), creatorName, whisper );
        }

        return;
    }

    if( gameName.size( ) > 31 )
    {
        for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); ++i )
        {
            if( (*i)->GetServer( ) == creatorServer )
                (*i)->QueueChatCommand( m_Language->UnableToCreateGameNameTooLong( gameName ), creatorName, whisper );
        }

        return;
    }

    if( !map->GetValid( ) )
    {
        for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); ++i )
        {
            if( (*i)->GetServer( ) == creatorServer )
                (*i)->QueueChatCommand( m_Language->UnableToCreateGameInvalidMap( gameName ), creatorName, whisper );
        }
        return;
    }

    if( saveGame )
    {
        if( !m_SaveGame->GetValid( ) )
        {
            for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); ++i )
            {
                if( (*i)->GetServer( ) == creatorServer )
                    (*i)->QueueChatCommand( m_Language->UnableToCreateGameInvalidSaveGame( gameName ), creatorName, whisper );
            }

            return;
        }

        string MapPath1 = m_SaveGame->GetMapPath( );
        string MapPath2 = map->GetMapPath( );
        transform( MapPath1.begin( ), MapPath1.end( ), MapPath1.begin( ), ::tolower );
        transform( MapPath2.begin( ), MapPath2.end( ), MapPath2.begin( ), ::tolower );

        if( MapPath1 != MapPath2 )
        {
            CONSOLE_Print( "[GHOST] path mismatch, saved game path is [" + MapPath1 + "] but map path is [" + MapPath2 + "]" );

            for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); ++i )
            {
                if( (*i)->GetServer( ) == creatorServer )
                    (*i)->QueueChatCommand( m_Language->UnableToCreateGameSaveGameMapMismatch( gameName ), creatorName, whisper );
            }
            return;
        }

        if( m_EnforcePlayers.empty( ) )
        {
            for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); ++i )
            {
                if( (*i)->GetServer( ) == creatorServer )
                    (*i)->QueueChatCommand( m_Language->UnableToCreateGameMustEnforceFirst( gameName ), creatorName, whisper );
            }

            return;
        }
    }

    boost::mutex::scoped_lock lock( m_GamesMutex );

    if( m_CurrentGame )
    {
        for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); ++i )
        {
            if( (*i)->GetServer( ) == creatorServer )
                (*i)->QueueChatCommand( m_Language->UnableToCreateGameAnotherGameInLobby( gameName, m_CurrentGame->GetDescription( ) ), creatorName, whisper );
        }

        return;
    }

    if( m_Games.size( ) >= m_MaxGames )
    {
        for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); ++i )
        {
            if( (*i)->GetServer( ) == creatorServer )
                (*i)->QueueChatCommand( m_Language->UnableToCreateGameMaxGamesReached( gameName, UTIL_ToString( m_MaxGames ) ), creatorName, whisper );
        }

        return;
    }

    lock.unlock( );

    CONSOLE_Print( "[GHOST] creating game [" + gameName + "]" );
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
    }

    // if we're creating a private game we don't need to send any game refresh messages so we can rejoin the chat immediately
    // unfortunately this doesn't work on PVPGN servers because they consider an enterchat message to be a gameuncreate message when in a game
    // so don't rejoin the chat if we're using PVPGN

    if( gameState == GAME_PRIVATE )
    {
        for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); ++i )
        {
            if( (*i)->GetPasswordHashType( ) != "pvpgn" )
                (*i)->QueueEnterChat( );
        }
    }

    // hold friends and/or clan members

    for( vector<CBNET *> :: iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); ++i )
    {
        if( (*i)->GetHoldFriends( ) )
            (*i)->HoldFriends( m_CurrentGame );

        if( (*i)->GetHoldClan( ) )
            (*i)->HoldClan( m_CurrentGame );
    }

    boost::thread(&CBaseGame::loop, m_CurrentGame);
    CONSOLE_Print("[GHOST] Created a new Game Thread.");
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

void COHBot :: LoadDatas( )
{
    CConfig CFG;
    CFG.Read( "default.cfg" );

    for( uint32_t i = 1; i < 20; ++i )
    {
        string Prefix;

        if( i == 1 )
            Prefix = "oh_";
        else
            Prefix = "oh" + UTIL_ToString( i ) + "_";

        string CName = CFG.GetString( Prefix + "cname", string( ) );
        //string Mode = CFG.GetString( "mode", string( ) );
        //m_Modes.push_back( Mode );
        m_ColoredNames.push_back( CName );
    }
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
        CONSOLE_Print( "Unable to open rules.txt" );
}

uint32_t COHBot :: GetNewHostCounter( )
{
    if( m_ReservedHostCounter != 0 )
    {
        m_HostCounter = m_ReservedHostCounter;
        m_ReservedHostCounter = 0;
        m_LastHCUpdate = GetTime();
        CONSOLE_Print( "[INFO] Set new hostcounter to: "+UTIL_ToString(m_HostCounter));
        return m_HostCounter;
    }
    return m_HostCounter;
}
void COHBot :: LoadRanks( )
{
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
        CONSOLE_Print("[GHOST] warning - unable to read file [ranks.txt]");
        m_RanksLoaded = false;
    }

    if(m_Ranks.size() < 10 && m_RanksLoaded) {
        CONSOLE_Print("[CONFIG] warning - ranks.txt doesn't contain enough levelnames. You require at least 11 rank names (Level 0 - Level 10, with 0).");
        m_RanksLoaded = false;
    } else if(m_RanksLoaded) {
		CONSOLE_Print("[GHOST] loading file [ranks.txt]");
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
	CONSOLE_Print("[GHOST] loading file [insult.txt]");
        in.close( );
    }
    else
        CONSOLE_Print("[GHOST] warning - unable to read file [insult.txt].");
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
    string s;
    bool ok = false;
    int l = RoomID.size();
    int DPos;
    if (m_LanRoomName.size()==0)
        return s=string();
    else if (l>4)
    {
        for (uint32_t i = 0; i<m_LanRoomName.size(); i++)
        {
            DPos = m_LanRoomName[i].find(RoomID);
            if (DPos!= string ::npos) {
                return s=m_LanRoomName[i].substr(DPos+l+2);
                ok = true;
                break;
            }
        }
    }
    if (!ok)
        return s=string(); //room matching that RoomID is not found
    return s;
}

void COHBot :: ReadRoomData()
{
    string file = m_SharedFilesPath + "rooms.txt";
    ifstream in;
    in.open( file.c_str( ) );
    m_LanRoomName.clear();
    if( in.fail( ) )
        CONSOLE_Print( "[GHOST] warning - unable to read file [" + file + "]" );
    else
    {
        CONSOLE_Print( "[GHOST] loading file [" + file + "]" );
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

string COHBot :: GetAliasName( uint32_t alias ) {
    if( m_Aliases.size( ) != 0 && m_Aliases.size( ) >= alias && alias != 0 ) {
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
 * This function does switch a long mode into a saved shorten mode which is avaible for lod.
 * Modes which contain more than 10 caracters cant be encoded on LoD, so the map owners added shorten modes
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

string COHBot :: GetMonthInWords( string month ) {
    if(month=="1")
        return "January";
    else if(month=="2")
        return "February";
    else if(month=="3")
        return "March";
    else if(month=="4")
        return "April";
    else if(month=="5")
        return "May";
    else if(month=="6")
        return "June";
    else if(month=="7")
        return "July";
    else if(month=="8")
        return "August";
    else if(month=="9")
        return "September";
    else if(month=="10")
        return "October";
    else if(month=="11")
        return "November";
    else if(month=="12")
        return "December";
    else
        return "unknown";
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
		DEBUG_Print("Bad input for RCON command #1");
	else {
		SS >> gameid;
		if( SS.fail( ) )
			DEBUG_Print("Bad input for RCON command #2");
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

void COHBot :: LoadLanguages( ) {
    /*
    try
    {
        path LanCFGPath( m_LanCFGPath );

        if( !exists( LanCFGPath ) )
        {
            CONSOLE_Print ("[ERROR] Could not find any language file. Shutting down.");
            m_Exiting = true;
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
        CONSOLE_Print( "[ERROR] error listing language files - caught exception " + *ex.what( ) );
    }
    */
}
