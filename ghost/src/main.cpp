//
// Created by ZeMi on 10.06.2016.
//
#include "ohbot.h"

// bot
COHBot *gGHost = NULL;

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

    Log->Debug("<DUMPSTACK>");

    for(int i = 0; i < levels; i++)
        Log->Debug(strings[i]);

    Log->Debug("</DUMPSTACK>");

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
    Log->Warning( "[!!!] caught signal " + UTIL_ToString( s ) + ", exiting NOW" );

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

    Log->Info( "[!!!] caught signal " + UTIL_ToString( s ) + ", exiting nicely" );

    if( gGHost )
        gGHost->m_ExitingNice = true;
    else
        exit( 1 );
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

    cout << "***************************************************************************************" << endl
         << "**                WELCOME TO BE4M.DE's REWORKED OHSYSTEM BOT V3                      **" << endl
         << "**       PLEASE DO NOT REMOVE ANY COPYRIGHT NOTICE TO RESPECT THE PROJECT            **" << endl
         << "**       ----------------------------------------------------------------            **" << endl
         << "**        For any questions and required support use our git repository              **" << endl
         << "**                    https://github.com/m-unkel/OHSystem                            **" << endl
         << "***************************************************************************************" << endl
         << endl;

    CFG = new CConfig();

    // read config file
    CFG->Read( "default.cfg" );

    // read overloaded config file
    if( argc > 1 && argv[1] ) {
        sCFGFile = argv[1];
        CFG->Read( sCFGFile );
    }
    else {
        sCFGFile = string();
    }

    string gLogFile = CFG->GetString( "bot_log", "ohbot_" + UTIL_ToString( GetTime( ) ) + ".log" );
    unsigned char gLogMethod = CFG->GetInt( "bot_logmethod", 1 );

    Log = new CLog(gLogFile,gLogMethod);
    Log->Cout("[GHOST] starting up");

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
            Log->Write( "[GHOST] error setting Windows timer resolution to " + UTIL_ToString( i ) + " milliseconds, trying a higher resolution" );
        else
        {
            Log->Write( "[GHOST] error setting Windows timer resolution" );
            return 1;
        }
    }

    Log->Info( "[GHOST] using Windows timer with resolution " + UTIL_ToString( TimerResolution ) + " milliseconds" );
#elif __APPLE__
    // not sure how to get the resolution
#else
    // print the timer resolution

    struct timespec Resolution;

    if( clock_getres( CLOCK_MONOTONIC, &Resolution ) == -1 )
        Log->Write( "[GHOST] error getting monotonic timer resolution" );
    else
        Log->Write( "[GHOST] using monotonic timer with resolution " + UTIL_ToString( (double)( Resolution.tv_nsec / 1000 ), 2 ) + " microseconds" );
#endif

#ifdef WIN32
    // initialize winsock

    Log->Info( "[GHOST] starting winsock" );
    WSADATA wsadata;

    if( WSAStartup( MAKEWORD( 2, 2 ), &wsadata ) != 0 )
    {
        Log->Write( "[GHOST] error starting winsock" );
        return 1;
    }

    // increase process priority

    Log->Info( "[GHOST] setting process priority to \"above normal\"" );
    SetPriorityClass( GetCurrentProcess( ), ABOVE_NORMAL_PRIORITY_CLASS );
#endif
    // initialize ohbot

    gGHost = new COHBot( CFG );

    while( 1 )
    {
        // block for 50ms on all sockets - if you intend to perform any timed actions more frequently you should change this
        // that said it's likely we'll loop more often than this due to there being data waiting on one of the sockets but there aren't any guarantees

        if( gGHost->Update( 50000 ) )
            break;
    }

    // shutdown ohbot

    Log->Write( "[GHOST] shutting down..." );
    delete gGHost;
    gGHost = NULL;
#ifdef WIN32
    // shutdown winsock

    Log->Info( "[GHOST] shutting down winsock" );
    WSACleanup( );

    // shutdown timer

    timeEndPeriod( TimerResolution );
#endif

    delete CFG;
    delete Log;

    cout << "** Exited nicely! **" << endl;

    return 0;
}