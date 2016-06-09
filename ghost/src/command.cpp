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

#include "ohbot.h"
#include "language.h"
#include "command.h"


std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}


//
// Command Constructor
//
CCommand :: CCommand(COHBot *nOHBot, string nNameList, unsigned char nAclLevel, unsigned char nNumArguments, unsigned char nContext):
                            m_OHBot(nOHBot), m_AclLevel(nAclLevel), m_NumArguments(nNumArguments), m_Context(nContext) {

    // split command synonyms from DB  by semicolon
    m_Synonyms = split(nNameList,';');

    // first synonym is the main command name
    if( m_Synonyms.size() > 0){
        m_Name = m_Synonyms[0];
    }
    else {
        m_Name = string("Unknown");
    }
}

//
// Command Destructor
//
CCommand ::~CCommand() {

}

//
// Get primary command name
//
string CCommand::GetName() {
    return m_Name;
}

//
// Get usage info / how to use this command in case of wrong usage
//
string CCommand::GetUsage() {
    return m_OHBot->m_Language->CommandUsageInfo( m_Name );
}

//
// Test if command can be used
//
bool CCommand::isAllowed(unsigned char context, unsigned char level) {

    // test if allowed context
    if( m_Context && (1<<context) ) {

        //test if required level is less or equal user level
        if ( m_AclLevel <= level ) {
            return true;
        }
    }

    return false;
}