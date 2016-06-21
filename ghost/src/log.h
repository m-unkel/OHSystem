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

class CLog {
private:
    uint8_t mode;
    string filePath;
    ofstream fileStream;
public:
    CLog( );
    ~CLog( );

    bool Open(string sFilePath,uint8_t cMode);

    void Write( const string sMessage, uint8_t msgMode );

    void Debug( BYTEARRAY b );
    void Debug( string sMessage ){ Write(string("[Debug] ")+sMessage,LOG_DEBUG|LOG_FILE|LOG_COUT);};
    void Chat ( string sMessage ){ Write(string("[Chat] ")+sMessage,LOG_INFO|LOG_FILE|LOG_COUT);};
    void Cout ( string sMessage ){ Write(string("[Info] ")+sMessage,LOG_COUT);};
    void Info( string sMessage ){ Write(string("[Info] ")+sMessage,LOG_INFO|LOG_COUT);};
    void Warning( string sMessage ){ Write(string("[Warning] ")+sMessage,LOG_WARNING|LOG_FILE|LOG_COUT);};
    void Write( const string sMessage ){ Write(string("[Error] ")+sMessage,LOG_ERROR|LOG_FILE|LOG_COUT);};

    string GetLocalTimeString();

public:
    static void RegisterPythonClass( );
};

#endif //LOG_H
