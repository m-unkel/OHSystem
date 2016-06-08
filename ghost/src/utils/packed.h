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

#ifndef PACKED_H
#define PACKED_H

//
// CPacked
//

class CCRC32;

class CPacked
{
public:
    CCRC32 *m_CRC;

protected:
    bool m_Valid;
    string m_Compressed;
    string m_Decompressed;
    uint32_t m_HeaderSize;
    uint32_t m_CompressedSize;
    uint32_t m_HeaderVersion;
    uint32_t m_DecompressedSize;
    uint32_t m_NumBlocks;
    uint32_t m_War3Identifier;
    uint32_t m_War3Version;
    uint16_t m_BuildNumber;
    uint16_t m_Flags;
    uint32_t m_ReplayLength;

public:
    CPacked( );
    virtual ~CPacked( );

    virtual bool GetValid( )				{
        return m_Valid;
    }
    virtual uint32_t GetHeaderSize( )		{
        return m_HeaderSize;
    }
    virtual uint32_t GetCompressedSize( )	{
        return m_CompressedSize;
    }
    virtual uint32_t GetHeaderVersion( )	{
        return m_HeaderVersion;
    }
    virtual uint32_t GetDecompressedSize( )	{
        return m_DecompressedSize;
    }
    virtual uint32_t GetNumBlocks( )		{
        return m_NumBlocks;
    }
    virtual uint32_t GetWar3Identifier( )	{
        return m_War3Identifier;
    }
    virtual uint32_t GetWar3Version( )		{
        return m_War3Version;
    }
    virtual uint16_t GetBuildNumber( )		{
        return m_BuildNumber;
    }
    virtual uint16_t GetFlags( )			{
        return m_Flags;
    }
    virtual uint32_t GetReplayLength( )		{
        return m_ReplayLength;
    }

    virtual void SetWar3Version( uint32_t nWar3Version )			{
        m_War3Version = nWar3Version;
    }
    virtual void SetBuildNumber( uint16_t nBuildNumber )			{
        m_BuildNumber = nBuildNumber;
    }
    virtual void SetFlags( uint16_t nFlags )						{
        m_Flags = nFlags;
    }
    virtual void SetReplayLength( uint32_t nReplayLength )			{
        m_ReplayLength = nReplayLength;
    }

    virtual void Load( string fileName, bool allBlocks );
    virtual bool Save( bool TFT, string fileName );
    virtual bool Extract( string inFileName, string outFileName );
    virtual bool Pack( bool TFT, string inFileName, string outFileName );
    virtual void Decompress( bool allBlocks );
    virtual void Compress( bool TFT );

public:
	static void RegisterPythonClass( );
};

#endif
