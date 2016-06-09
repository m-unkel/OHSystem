/**
 * Copyright [2016] [m-unkel]
 * Mail: info@unive.de
 * URL: https://github.com/m-unkel/OHSystem
 *
 * This is free software: You can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 */

#ifndef COMMAND_H
#define COMMAND_H

#define CONTEXT_BNET        1
#define CONTEXTT_PVPGN      2
#define CONTEXT_INGAME      3
#define CONTEXT_WHISPER     4
#define CONTEXT_CHANNEL     5
#define CONTEXT_RCON        6


class CCommand
{
public:
    COHBot *m_OHBot;
private:
    string m_Name;
    vector<string> m_Synonyms;
    unsigned char m_AclLevel;
    unsigned char m_NumArguments;
    unsigned char m_Context;

public:
    CCommand( COHBot *nOHBot, string nNameList, unsigned char nAclLevel, unsigned char nNumArguments, unsigned char nContext );
    ~CCommand( );

    string GetName();
    string GetUsage();
    bool isAllowed( unsigned char context, unsigned char level );
};

#endif //COMMAND_H
