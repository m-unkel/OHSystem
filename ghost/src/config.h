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
 * This is free software: You can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version. *
 *
 * This is modified from GHOST++: http://ohbotplusplus.googlecode.com/
 *
 */

#ifndef CONFIG_H
#define CONFIG_H

//
// CConfig
//

class CConfig
{
private:
    map<string, string> m_CFG;

public:
    CConfig( );
    ~CConfig( );

    void Read( string file );
    bool Exists( string key );
    int GetInt( string key, int x );
    uint32_t GetUInt( string key, uint32_t x );
    string GetString( string key, string x );
    void Set( string key, string x );

public:
	static void RegisterPythonClass( );
};

#endif
