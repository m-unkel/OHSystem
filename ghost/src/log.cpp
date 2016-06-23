/**
 * Copyright [2016] [m-unkel]
 * Mail: info@unive.de
 * URL: https://github.com/m-unkel/OHSystem
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
 * This is modified from OHSYSTEM https://github.com/OHSystem
 * and from GHOST++: http://ohbotplusplus.googlecode.com/
 *
 */


#include "includes.h"


//
// Log constructor
//
CLog :: CLog() {
    filePath = string();
    state = STATE_LOG_DISABLED;
    logLevel = LOG_ERROR|LOG_WARNING|LOG_INFO|LOG_CHAT;
}

//
// Log destructor
//
CLog :: ~CLog() {
    // close file if still opened
    if( state == STATE_LOG_KEEP_OPEN ){
        fileStream.close( );
    }

    cout << "[" << GetLocalTimeString() << "] Log closed" << endl;
}

bool CLog :: Open( const string sFilePath, const uint8_t iLogLevel, const bool bKeepOpened) {

    // Log level
    if( iLogLevel == 0){
        cout << "[Info] Loglevel not defined or invalid, using default Loglevel (Error,Warning,Info,Chat)" << endl;
    }
    else {
        logLevel = iLogLevel;
    }

    // Log file
    filePath = sFilePath;
    // test if filePath is empty
    if( filePath.empty() ) {
        cout << "[Warning] Logfile not defined or invalid, logging is disabled" << endl;
    }
    else {

        // test file access
        fileStream.open( filePath.c_str( ), ios :: app );

        // file access failed
        if( fileStream.is_open() ) {

            if( bKeepOpened ){
                state = STATE_LOG_KEEP_OPEN;
                // log method LOG_KEEPOPENED: open the log on startup, flush the log for every message, close the log on shutdown
                // the log file CANNOT be edited/moved/deleted while GHost++ is running
                cout << "[" << GetLocalTimeString() << "] Logfile opened and locked (" << filePath << ")" << endl;

            }
            else {
                state = STATE_LOG_DEFAULT;
                // log method !LOG_KEEPOPENED: open, append, and close the log for every message
                // this works well on Linux but poorly on Windows, particularly as the log file grows in size
                // the log file can be edited/moved/deleted while GHost++ is running
                fileStream.close();
                cout << "[" << GetLocalTimeString() << "] Logfile opened, but not locked (" << filePath << ")" << endl;
            }
            return true;
        }
        cout << "[ERROR] Can't open/write to log file (" << filePath << ")!" << endl;
    }

    return false;
}

//
// Get current time in local string representation
//
string CLog :: GetLocalTimeString() {
    time_t Now = time( NULL );
    string sTime = asctime( localtime( &Now ) );
    sTime.erase( sTime.size( ) - 1 );
    return sTime;
}

//
// print/debug bytearray as hex
//
void CLog :: Debug( const BYTEARRAY b )
{
    stringstream ss;

    ss << "{ ";
    for( unsigned int i = 0; i < b.size( ); ++i )
        ss << hex << (int)b[i] << " ";
    ss << "}";

    Debug(ss.str());
}

//
// Write a message to log
//
void CLog :: Write( const string sMessage, const TLogLevel iLevel )
{
    //if logging disabled, but message is not a debug message, then cout message
    if ( state == STATE_LOG_DISABLED ){
        if ( iLevel != LOG_DEBUG ){
            cout << sMessage << endl;
        }
        return;
    }

    // LogLevel enabled: Write to stdout and file ?
    if( IS(logLevel,iLevel) )
    {
        // write to stdout
        cout << sMessage << endl;

        // if log file not opened, then open it
        if( state == STATE_LOG_DEFAULT )
            fileStream.open( filePath.c_str( ), ios :: app );

        if( fileStream.is_open() ) {
            fileStream << "[" << GetLocalTimeString() << "] " << sMessage << endl;
        }
        else
        {
            state = STATE_LOG_DISABLED;
            cout << "[ERROR] Can't write to log file (" << filePath << ")!" << endl;
        }

        if ( state == STATE_LOG_KEEP_OPEN )
            fileStream.flush( );
        else
            fileStream.close( );

    }

    // LogLevel disabled: write to stdout if not a debug message
    else if (iLevel != LOG_DEBUG){
        cout << sMessage << endl;
    }
}

void CLog :: Cout( const string sMessage) {
    cout << sMessage << endl;
}