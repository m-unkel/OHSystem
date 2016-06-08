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
* or join us directly here: http://ohsystem.net/forum/
*
* Visit us also on http://ohsystem.net/ and keep track always of the latest
* features and changes.
*
*
* This is modified from GHOST++: http://ohbotplusplus.googlecode.com/
* Official GhostPP-Forum: http://ohbotpp.com/
*/

#ifndef BNLSCLIENT_H
#define BNLSCLIENT_H

//
// CBNLSClient
//

class CTCPClient;
class CBNLSProtocol;
class CCommandPacket;

class CBNLSClient
{
private:
    CTCPClient *m_Socket;							// the connection to the BNLS server
    CBNLSProtocol *m_Protocol;						// battle.net protocol
    queue<CCommandPacket *> m_Packets;				// queue of incoming packets
    bool m_WasConnected;
    string m_Server;
    uint16_t m_Port;
    uint32_t m_LastNullTime;
    uint32_t m_WardenCookie;						// the warden cookie
    queue<BYTEARRAY> m_OutPackets;					// queue of outgoing packets to be sent
    queue<BYTEARRAY> m_WardenResponses;				// the warden responses to be sent to battle.net
    uint32_t m_TotalWardenIn;
    uint32_t m_TotalWardenOut;

public:
    CBNLSClient( string nServer, uint16_t nPort, uint32_t nWardenCookie );
    ~CBNLSClient( );

    BYTEARRAY GetWardenResponse( );
    uint32_t GetTotalWardenIn( )		{
        return m_TotalWardenIn;
    }
    uint32_t GetTotalWardenOut( )		{
        return m_TotalWardenOut;
    }

    // processing functions

    unsigned int SetFD( void *fd, void *send_fd, int *nfds );
    bool Update( void *fd, void *send_fd );
    void ExtractPackets( );
    void ProcessPackets( );

    // other functions

    void QueueWardenSeed( uint32_t seed );
    void QueueWardenRaw( BYTEARRAY wardenRaw );

public:
	static void RegisterPythonClass( );
};

#endif
