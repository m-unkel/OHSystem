/*

   Copyright [2008] [Trevor Hogan]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   CODE PORTED FROM THE ORIGINAL GHOST PROJECT: http://ohbot.pwner.org/

*/

#ifndef INCLUDES_H
#define INCLUDES_H

#define IS(a,b) (a&b)!=0
#define IS_NOT(a,b) (!a&b)!=0
#define SET(a,b) a|=b
#define UNSET(a,b) a&=!b

// standard integer sizes for 64 bit compatibility

#ifdef WIN32
 #include "utils/ms_stdint.h"
#else
 #include <stdint.h>
#endif

// STL

#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <map>
#include <queue>
#include <set>
#include <string>
#include <vector>
#include <boost/thread.hpp>

#ifndef WIN32
//woot woot?
#include <time.h>
#endif

#ifdef __APPLE__
#include <mach/mach_time.h>
#endif

using namespace std;

typedef vector<unsigned char> BYTEARRAY;
typedef pair<unsigned char,string> PIDPlayer;

// time
uint32_t GetTime( );		// seconds
uint32_t GetTicks( );		// milliseconds

#ifdef WIN32
 #define MILLISLEEP( x ) Sleep( x )
#else
 #define MILLISLEEP( x ) usleep( ( x ) * 1000 )
#endif

// network

#undef FD_SETSIZE
#define FD_SETSIZE 512

//class COHBot;

// config
#include "config.h"
extern CConfig *CFG;
extern string sCFGFile;

// log
#include "log.h"
extern CLog *Log;

class COHBot;
extern COHBot *gGHost;

#endif //INCLUDES_H
