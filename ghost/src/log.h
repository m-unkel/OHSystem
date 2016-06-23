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

enum TLogLevel {LOG_ERROR = 1,LOG_WARNING = 2,LOG_INFO = 4, LOG_CHAT = 8,LOG_DEBUG = 16};
enum TLogState {STATE_LOG_DISABLED,STATE_LOG_DEFAULT,STATE_LOG_KEEP_OPEN};

class CLog {
private:
    uint8_t logLevel;
    TLogState state;
    string filePath;
    ofstream fileStream;
public:
    CLog( );
    ~CLog( );

    bool Open( const string sFilePath, const uint8_t iLogLevel, const bool bKeepOpened);

    void Write( const string sMessage, const TLogLevel iLevel );

    void Debug( const BYTEARRAY b );
    void Debug( const string sMessage ){ Write(string("[Debug] ")+sMessage,LOG_DEBUG);};
    void Chat ( const string sMessage ){ Write(string("[Chat] ")+sMessage,LOG_CHAT);};
    void Cout ( const string sMessage );
    void Info( const string sMessage ){ Write(string("[Info] ")+sMessage,LOG_INFO);};
    void Warning( const string sMessage ){ Write(string("[Warning] ")+sMessage,LOG_WARNING);};
    void Write( const string sMessage ){ Write(string("[Error] ")+sMessage,LOG_ERROR);};

    string GetLocalTimeString();

public:
    static void RegisterPythonClass( );
};

#endif //LOG_H
