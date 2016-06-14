//
// Created by ZeMi on 10.06.2016.
//

#ifndef LOG_H
#define LOG_H

#define LOG_FILE_INVALID    1

#define LOG_ERROR           2
#define LOG_WARNING         4
#define LOG_DEBUG           8
#define LOG_INFO            16

#define LOG_KEEPOPENED      32
#define LOG_COUT            64
#define LOG_FILE            128

#define LOG_ALL             30


#ifdef WIN32
#include "utils/ms_stdint.h"
#else
#include <stdint.h>
#endif

class CLog {
private:
    uint8_t mode;
    string filePath;
    ofstream fileStream;
    string GetLocalTimeString();
public:
    CLog(string sFilePath,uint8_t cMode);
    CLog(string sFilePath): CLog(sFilePath,LOG_ALL){};
    CLog(): CLog(string(),LOG_ALL|LOG_FILE_INVALID){};
    ~CLog();

    bool Open(string sFilePath,uint8_t cMode);
    bool Open(string sFilePath){ Open(sFilePath,LOG_ALL);};

    void Write( const string sMessage, uint8_t msgMode );

    void Debug( BYTEARRAY b );
    void Debug( string sMessage ){ Write(string("[Debug] ")+sMessage,LOG_DEBUG|LOG_FILE|LOG_COUT);};
    void Chat ( string sMessage ){ Write(string("[Chat] ")+sMessage,LOG_INFO|LOG_FILE|LOG_COUT);};
    void Cout ( string sMessage ){ Write(string("[Info] ")+sMessage,LOG_COUT);};
    void Info( string sMessage ){ Write(string("[Info] ")+sMessage,LOG_INFO|LOG_COUT);};
    void Warning( string sMessage ){ Write(string("[Warning] ")+sMessage,LOG_WARNING|LOG_FILE|LOG_COUT);};
    void Write( const string sMessage ){ Write(string("[Error] ")+sMessage,LOG_ERROR|LOG_FILE|LOG_COUT);};
};

#endif //LOG_H
