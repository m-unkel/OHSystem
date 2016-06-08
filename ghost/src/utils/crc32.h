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

#ifndef CRC32_H
#define CRC32_H

#define CRC32_POLYNOMIAL 0x04c11db7

class CCRC32
{
public:
    void Initialize( );
    uint32_t FullCRC( unsigned char *sData, uint32_t ulLength );
    void PartialCRC( uint32_t *ulInCRC, unsigned char *sData, uint32_t ulLength );

private:
    uint32_t Reflect( uint32_t ulReflect, char cChar );
    uint32_t ulTable[256];
};

#endif
