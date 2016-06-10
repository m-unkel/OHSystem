//
// Created by ZeMi on 10.06.2016.
//

#include <iostream>
#include <fstream>
#include <time.h>
#include "log.h"

using namespace std;

//
// Log constructor
//
CLog :: CLog(string sFilePath, unsigned char cMode):
        filePath(sFilePath),mode(cMode) {

    fileStream = new ofstream;

    // test if filePath is valid
    if( filePath.empty() ) {
        mode = LOG_FILE_INVALID;
    }
    else {

        // test file access
        fileStream->open( filePath.c_str( ), ios :: app );

        // file access failed
        if( fileStream->fail( ) ) {
            mode = LOG_FILE_INVALID;
        }

        // file access granted
        else if( mode & LOG_KEEPOPENED ){
            // log method LOG_KEEPOPENED: open the log on startup, flush the log for every message, close the log on shutdown
            // the log file CANNOT be edited/moved/deleted while GHost++ is running

            cout << "[" << GetLocalTimeString() << "] Logfile opened and locked (\" << filePath << \")" << endl;

        }
        else {
            // log method !LOG_KEEPOPENED: open, append, and close the log for every message
            // this works well on Linux but poorly on Windows, particularly as the log file grows in size
            // the log file can be edited/moved/deleted while GHost++ is running
            fileStream->close();
            cout << "[" << GetLocalTimeString() << "] Logfile opened, but not locked (\" << filePath << \")" << endl;
        }
    }

    // file invalid ?
    if( mode & LOG_FILE_INVALID ){
        cout << "[ERROR] Can't open/write to log file (" << filePath << ")!";
    }

}

//
// Log destructor
//
CLog :: ~CLog() {
    // close file if still opened
    if( mode & LOG_KEEPOPENED ){
        fileStream->close( );
    }

    // free memory
    delete fileStream;

    cout << "[" << GetLocalTimeString() << "] Log closed" << endl;
}

//
// Get current time in local string representation
//
string CLog ::GetLocalTimeString() {
    time_t Now = time( NULL );
    string sTime = asctime( localtime( &Now ) );
    sTime.erase( sTime.size( ) - 1 );
    return sTime;
}

//
// test if mode is set
//
bool CLog :: is(unsigned char cMode){
    return mode & cMode != 0;
}

//
// set one or more modes ( or conjunction )
//
CLog* CLog :: set(unsigned char cMode){
    mode |= cMode;
    return this;
}

//
// unset one or more modes ( and not conjunction )
//
CLog* CLog :: unset(unsigned char cMode){
    mode &= !cMode;
    return this;
}

//
// print/debug bytearray as hex
//
CLog* CLog :: Debug( BYTEARRAY b )
{
    stringstream ss;

    ss << "{ ";
    for( unsigned int i = 0; i < b.size( ); ++i )
        ss << hex << (int)b[i] << " ";
    ss << "}";

    return Write(ss.str(),LOG_DEBUG);
}

//
// Write a message to log
//
CLog* CLog :: Write( const string sMessage, unsigned char writeMode )
{
    string fMessage;
    if ( writeMode & LOG_ERROR ){
        // skip file writing if error logging is disabled
        if( !mode & LOG_ERROR )
            writeMode = LOG_COUT_ONLY;

        fMessage = string("[ERROR] ");
    }
    else if ( writeMode & LOG_WARNING ){
        // skip file writing if warning logging is disabled
        if( !mode & LOG_WARNING )
            writeMode = LOG_COUT_ONLY;

        fMessage = string("[WARNG] ");
    }
    else if ( writeMode & LOG_DEBUG ){
        // skip file writing if debug logging is disabled
        if( !mode & LOG_DEBUG )
            writeMode = LOG_COUT_ONLY;

        fMessage = string("[DEBUG] ");
    }

    // Print to stdout?
    if ( !writeMode & LOG_FILE_ONLY ) {
        cout << fMessage << sMessage << endl;
    }

    // Log File valid ?
    else if( (!writeMode & LOG_COUT_ONLY) && (!mode & LOG_FILE_INVALID) )
    {
        // write and flush opened logfile
        if( mode & LOG_KEEPOPENED && fileStream && !fileStream->fail( ) ) {
            *fileStream << fMessage << "[" << GetLocalTimeString() << "] " << sMessage << endl;
            fileStream->flush( );
        }
        // open, write and close logfile
        else
        {
            fileStream->open( filePath.c_str( ), ios :: app );

            if( fileStream->fail( ) ) {
                mode = LOG_FILE_INVALID;
                cout << "[ERROR] Can't open/write to log file (" << filePath << ")!";
            }
            else
            {
                *fileStream << fMessage << "[" << GetLocalTimeString() << "] " << sMessage << endl;
                fileStream->close( );
            }
            delete fileStream;
            fileStream = NULL;
        }
    }
    return this;
}