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

#ifndef GCBIPROTOCOL_H
#define GCBIPROTOCOL_H

//
// CGCBIProtocol
//

#define GCBI_HEADER_CONSTANT		249

#define REJECTGCBI_INVALID			1
#define REJECTGCBI_NOTFOUND			2

class CIncomingGarenaUser;

class CGCBIProtocol
{
public:
    enum Protocol {
        GCBI_INIT				= 1
    };

    CGCBIProtocol( );
    ~CGCBIProtocol( );

    // receive functions
    CIncomingGarenaUser *RECEIVE_GCBI_INIT( BYTEARRAY data );

    // send functions

    // other functions

private:
    bool AssignLength( BYTEARRAY &content );
    bool ValidateLength( BYTEARRAY &content );
};

//
// CIncomingGarenaUser
//

class CIncomingGarenaUser
{
private:
    uint32_t m_IP;
    uint32_t m_UserID;
    uint32_t m_RoomID;
    uint32_t m_UserExp;
    string m_CountryCode;

public:
    CIncomingGarenaUser( uint32_t nIP, uint32_t nUserID, uint32_t nRoomID, uint32_t nUserExp, string nCountryCode );
    ~CIncomingGarenaUser( );

    uint32_t GetIP( ) {
        return m_IP;
    }
    uint32_t GetUserID( ) {
        return m_UserID;
    }
    uint32_t GetRoomID( ) {
        return m_RoomID;
    }
    uint32_t GetUserExp( ) {
        return m_UserExp;
    }
    string GetCountryCode( ) {
        return m_CountryCode;
    }
};

#endif
