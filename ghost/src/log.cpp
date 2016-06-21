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
    mode = LOG_ALL|LOG_FILE_INVALID;
}

//
// Log destructor
//
CLog :: ~CLog() {
    // close file if still opened
    if( IS(mode,LOG_KEEPOPENED ) && IS_NOT(mode,LOG_FILE_INVALID) ){
        fileStream.close( );
    }

    cout << "[" << GetLocalTimeString() << "] Log closed" << endl;
}

bool CLog :: Open(string sFilePath, uint8_t cMode) {

    mode = cMode;
    filePath = sFilePath;

    // test if filePath is valid
    if( filePath.empty() ) {
        SET(mode,LOG_FILE_INVALID);
    }
    else {

        // test file access
        fileStream.open( filePath.c_str( ), ios :: app );

        // file access failed
        if( fileStream.fail( ) ) {
            SET(mode,LOG_FILE_INVALID);
        }
        // file access granted
        else {
            UNSET(mode,LOG_FILE_INVALID);
            
            if( IS(mode,LOG_KEEPOPENED) ){
                // log method LOG_KEEPOPENED: open the log on startup, flush the log for every message, close the log on shutdown
                // the log file CANNOT be edited/moved/deleted while GHost++ is running

                cout << "[" << GetLocalTimeString() << "] Logfile opened and locked (" << filePath << ")" << endl;

            }
            else {
                // log method !LOG_KEEPOPENED: open, append, and close the log for every message
                // this works well on Linux but poorly on Windows, particularly as the log file grows in size
                // the log file can be edited/moved/deleted while GHost++ is running
                fileStream.close();
                cout << "[" << GetLocalTimeString() << "] Logfile opened, but not locked (" << filePath << ")" << endl;
            }
        }
        return true;
    }

    // file invalid ?
    if( IS(mode,LOG_FILE_INVALID) ){
        cout << "[ERROR] Can't open/write to log file (" << filePath << ")!";
        return false;
    }
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
void CLog :: Debug( BYTEARRAY b )
{
    stringstream ss;

    ss << "{ ";
    for( unsigned int i = 0; i < b.size( ); ++i )
        ss << hex << (int)b[i] << " ";
    ss << "}";

    Write(ss.str(),LOG_DEBUG);
}

//
// Write a message to log
//
void CLog :: Write( const string sMessage, uint8_t msgMode )
{
    //skip file writing if logging disabled or file invalid
    if (
        ( IS(mode,LOG_FILE_INVALID) ) ||
        ( IS(msgMode,LOG_ERROR) && IS_NOT(mode,LOG_ERROR) ) ||
        ( IS(msgMode,LOG_WARNING) && IS_NOT(mode,LOG_WARNING) ) ||
        ( IS(msgMode,LOG_INFO) && IS_NOT(mode,LOG_INFO) ) ||
        ( IS(msgMode,LOG_DEBUG) && IS_NOT(mode,LOG_DEBUG) )
            ) {
        SET(msgMode,LOG_COUT);
        UNSET(msgMode,LOG_FILE);
    }

    // Print to stdout?
    if ( IS(msgMode,LOG_COUT) ) {
        cout << sMessage << endl;
    }

    // Log File valid ?
    if( IS(msgMode,LOG_FILE) )
    {
        // if log file not opened, then open it
        if(IS_NOT(mode,LOG_KEEPOPENED) )
            fileStream.open( filePath.c_str( ), ios :: app );

        if( fileStream.fail( ) ) {
            SET(mode,LOG_FILE_INVALID);
            cout << "[ERROR] Can't write to log file (" << filePath << ")!";
        }
        else
        {
            fileStream << "[" << GetLocalTimeString() << "] " << sMessage << endl;
        }

        if ( IS(mode,LOG_KEEPOPENED) )
            fileStream.flush( );
        else
            fileStream.close( );

    }
}