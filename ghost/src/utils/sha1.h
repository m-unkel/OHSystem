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

#ifndef ___SHA1_H___
#define ___SHA1_H___

#include <stdio.h> // Needed for file access
#include <memory.h> // Needed for memset and memcpy
#include <string.h> // Needed for strcat and strcpy

// standard integer sizes for 64 bit compatibility

#ifdef WIN32
#include "ms_stdint.h"
#else
#include <stdint.h>
#endif

#define MAX_FILE_READ_BUFFER 8000

class CSHA1
{
public:
    // Rotate x bits to the left
#define ROL32(value, bits) (((value)<<(bits))|((value)>>(32-(bits))))

#ifdef GHOST_BIG_ENDIAN
#define SHABLK0(i) (block->l[i])
#else
#define SHABLK0(i) (block->l[i] = (ROL32(block->l[i],24) & 0xFF00FF00) \
			| (ROL32(block->l[i],8) & 0x00FF00FF))
#endif

#define SHABLK(i) (block->l[i&15] = ROL32(block->l[(i+13)&15] ^ block->l[(i+8)&15] \
		^ block->l[(i+2)&15] ^ block->l[i&15],1))

    // SHA-1 rounds
#define R0(v,w,x,y,z,i) { z+=((w&(x^y))^y)+SHABLK0(i)+0x5A827999+ROL32(v,5); w=ROL32(w,30); }
#define R1(v,w,x,y,z,i) { z+=((w&(x^y))^y)+SHABLK(i)+0x5A827999+ROL32(v,5); w=ROL32(w,30); }
#define R2(v,w,x,y,z,i) { z+=(w^x^y)+SHABLK(i)+0x6ED9EBA1+ROL32(v,5); w=ROL32(w,30); }
#define R3(v,w,x,y,z,i) { z+=(((w|x)&y)|(w&x))+SHABLK(i)+0x8F1BBCDC+ROL32(v,5); w=ROL32(w,30); }
#define R4(v,w,x,y,z,i) { z+=(w^x^y)+SHABLK(i)+0xCA62C1D6+ROL32(v,5); w=ROL32(w,30); }

    typedef union {
        unsigned char c[64];
        uint32_t l[16];
    } SHA1_WORKSPACE_BLOCK;

    // Two different formats for ReportHash(...)
    enum { REPORT_HEX = 0, REPORT_DIGIT = 1 };

    // Constructor and Destructor
    CSHA1();
    virtual ~CSHA1();

    uint32_t m_state[5];
    uint32_t m_count[2];
    unsigned char m_buffer[64];
    unsigned char m_digest[20];

    void Reset();

    // Update the hash value
    void Update(unsigned char* data, unsigned int len);

    // Finalize hash and report
    void Final();
    void ReportHash(char *szReport, unsigned char uReportType = REPORT_HEX);
    void GetHash(unsigned char *uDest);

private:
    // Private SHA-1 transformation
    void Transform(uint32_t state[5], unsigned char buffer[64]);
};

#endif // ___SHA1_H___
