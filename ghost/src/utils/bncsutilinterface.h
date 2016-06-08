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

#ifndef BNCSUTIL_INTERFACE_H
#define BNCSUTIL_INTERFACE_H

//
// CBNCSUtilInterface
//

class CBNCSUtilInterface
{
private:
    void *m_NLS;
    BYTEARRAY m_EXEVersion;			// set in HELP_SID_AUTH_CHECK
    BYTEARRAY m_EXEVersionHash;		// set in HELP_SID_AUTH_CHECK
    string m_EXEInfo;				// set in HELP_SID_AUTH_CHECK
    BYTEARRAY m_KeyInfoROC;			// set in HELP_SID_AUTH_CHECK
    BYTEARRAY m_KeyInfoTFT;			// set in HELP_SID_AUTH_CHECK
    BYTEARRAY m_ClientKey;			// set in HELP_SID_AUTH_ACCOUNTLOGON
    BYTEARRAY m_M1;					// set in HELP_SID_AUTH_ACCOUNTLOGONPROOF
    BYTEARRAY m_PvPGNPasswordHash;	// set in HELP_PvPGNPasswordHash

public:
    CBNCSUtilInterface( string userName, string userPassword );
    ~CBNCSUtilInterface( );

    BYTEARRAY GetEXEVersion( )								{
        return m_EXEVersion;
    }
    BYTEARRAY GetEXEVersionHash( )							{
        return m_EXEVersionHash;
    }
    string GetEXEInfo( )									{
        return m_EXEInfo;
    }
    BYTEARRAY GetKeyInfoROC( )								{
        return m_KeyInfoROC;
    }
    BYTEARRAY GetKeyInfoTFT( )								{
        return m_KeyInfoTFT;
    }
    BYTEARRAY GetClientKey( )								{
        return m_ClientKey;
    }
    BYTEARRAY GetM1( )										{
        return m_M1;
    }
    BYTEARRAY GetPvPGNPasswordHash( )						{
        return m_PvPGNPasswordHash;
    }

    void SetEXEVersion( BYTEARRAY &nEXEVersion )			{
        m_EXEVersion = nEXEVersion;
    }
    void SetEXEVersionHash( BYTEARRAY &nEXEVersionHash )	{
        m_EXEVersionHash = nEXEVersionHash;
    }

    void Reset( string userName, string userPassword );

    bool HELP_SID_AUTH_CHECK( bool TFT, string war3Path, string keyROC, string keyTFT, string valueStringFormula, string mpqFileName, BYTEARRAY clientToken, BYTEARRAY serverToken );
    bool HELP_SID_AUTH_ACCOUNTLOGON( );
    bool HELP_SID_AUTH_ACCOUNTLOGONPROOF( BYTEARRAY salt, BYTEARRAY serverKey );
    bool HELP_PvPGNPasswordHash( string userPassword );

private:
    BYTEARRAY CreateKeyInfo( string key, uint32_t clientToken, uint32_t serverToken );
public:
	static void RegisterPythonClass( );
};

#endif
