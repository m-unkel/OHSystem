//
// Created by ZeMi on 10.06.2016.
//

#ifndef LOG_H
#define LOG_H

#include "includes.h"

#define LOG_FILE_INVALID    1
#define LOG_ERROR           2
#define LOG_WARNING         4
#define LOG_DEBUG           8
#define LOG_INFO            16
#define LOG_KEEPOPENED      32
#define LOG_COUT            64
#define LOG_FILE            128

class CLog {
private:
    unsigned char mode;
    string filePath;
    ofstream *fileStream;
    string GetLocalTimeString();
public:
    CLog(string sFilePath,unsigned char cMode);
    CLog(string sFilePath): CLog(sFilePath,LOG_ERROR){};
    ~CLog();

    bool is( unsigned char cMode);
    CLog* set( unsigned char cMode );
    CLog* unset( unsigned char cMode );

    CLog* Debug( string sMessage ): Write(sMessage,LOG_DEBUG|LOG_FILE|LOG_COUT){};
    CLog* Debug( BYTEARRAY b );
    CLog* Chat ( string sMessage ): Write(sMessage,LOG_INFO|LOG_FILE|LOG_COUT){};
    CLog* Cout ( string sMessage ): Write(sMessage,LOG_COUT){};
    CLog* Info( string sMessage ): Write(sMessage,LOG_INFO|LOG_COUT){};
    CLog* Warning( string sMessage ): Write(sMessage,LOG_WARNING|LOG_FILE|LOG_COUT){};
    CLog* Write( const string sMessage ): Write(sMessage,LOG_ERROR|LOG_FILE|LOG_COUT){};
    CLog* Write( const string sMessage, unsigned char writeMode );
};

#endif //LOG_H
