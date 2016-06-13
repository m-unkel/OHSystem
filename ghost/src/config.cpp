/**
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
*/

#include "ohbot.h"
#include "config.h"
#include <stdlib.h>
#include "includes.h"

//
// CConfig
//

CConfig :: CConfig( )
{

}

CConfig :: ~CConfig( )
{

}

void CConfig :: Read( string file )
{
    ifstream in;
    in.open( file.c_str( ) );

    if( in.fail( ) )
        Log->Warning( "[CONFIG] unable to read file [" + file + "]" );
    else
    {
        Log->Info( "[CONFIG] loading file [" + file + "]" );
        string Line;

        while( !in.eof( ) )
        {
            getline( in, Line );

            // ignore blank lines and comments

            if( Line.empty( ) || Line[0] == '#' )
                continue;

            // remove newlines and partial newlines to help fix issues with Windows formatted config files on Linux systems

            Line.erase( remove( Line.begin( ), Line.end( ), '\r' ), Line.end( ) );
            Line.erase( remove( Line.begin( ), Line.end( ), '\n' ), Line.end( ) );

            string :: size_type Split = Line.find( "=" );

            if( Split == string :: npos )
                continue;

            string :: size_type KeyStart = Line.find_first_not_of( " " );
            string :: size_type KeyEnd = Line.find( " ", KeyStart );
            string :: size_type ValueStart = Line.find_first_not_of( " ", Split + 1 );
            string :: size_type ValueEnd = Line.size( );

            if( ValueStart != string :: npos )
                m_CFG[Line.substr( KeyStart, KeyEnd - KeyStart )] = Line.substr( ValueStart, ValueEnd - ValueStart );
        }

        in.close( );
    }
}

bool CConfig :: Exists( string key )
{
    return m_CFG.find( key ) != m_CFG.end( );
}

int CConfig :: GetInt( string key, int x )
{
    if( m_CFG.find( key ) == m_CFG.end( ) )
        return x;
    else
        return atoi( m_CFG[key].c_str( ) );
}

uint32_t CConfig :: GetUInt( string key, uint32_t x )
{
    if( m_CFG.find( key ) == m_CFG.end( ) )
        return x;
    else
        return strtoul( m_CFG[key].c_str( ), NULL, 0 );
}

string CConfig :: GetString( string key, string x )
{
    if( m_CFG.find( key ) == m_CFG.end( ) )
        return x;
    else
        return m_CFG[key];
}

void CConfig :: Set( string key, string x )
{
    m_CFG[key] = x;

}
