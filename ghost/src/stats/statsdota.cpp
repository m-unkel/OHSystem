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

#include "../ohbot.h"
#include "../utils/util.h"
#include "../database/ghostdb.h"
#include "../game/gameplayer.h"
#include "../game/gameprotocol.h"
#include "../game/game_base.h"
#include "stats.h"
#include "statsdota.h"
#include "../language.h"

//
// CStatsDOTA
//

CStatsDOTA :: CStatsDOTA( CBaseGame *nGame ) : CStats( nGame ), m_Winner( 0 ), m_Min( 0 ), m_Sec( 0 ), m_TowerLimit( false ), m_KillLimit( 0 ), m_TimeLimit( 0 ), m_SentinelTowers( 0 ), m_ScourgeTowers( 0 ), m_SentinelKills( 0 ), m_ScourgeKills( 0 ), m_LastCreepTime( 0 )
{
    TypePrefix=m_Game->m_OHBot->GetAliasName(m_Game->m_GameAlias);
    transform( TypePrefix.begin( ), TypePrefix.end( ), TypePrefix.begin( ), (int(*)(int))toupper );
    Log->Info( "[STATS"+TypePrefix+"] using "+TypePrefix+" stats" );

    for( unsigned int i = 0; i < 12; ++i )
    {
        m_Players[i] = NULL;
        m_LeaverKills[i] = 0;
        m_LeaverDeaths[i] = 0;
        m_AssistsOnLeaverKills[i] = 0;
        m_DeathsByLeaver[i] = 0;
        m_LatestKill[i] = 0;
        m_KillCounter[i] = 0;   //The killcounter for checking as double kill etc (not the general kills of a player.)
        m_FFKills[i] = 0;
        m_KillStreakCounter[i] = 0;
        m_BufferedItemOne[i] = "";
        m_BufferedItemTwo[i] = "";
        m_BufferedItemThree[i] = "";
        m_BufferedItemFour[i] = "";
        m_BufferedItemFive[i] = "";
        m_BufferedItemSix[i] = "";
        m_BufferedItemCount[i] = 0;
	m_MaphackClickCounter[i] = 0;
    }
    m_FirstBlood = false;
}

CStatsDOTA :: ~CStatsDOTA( )
{
    for( unsigned int i = 0; i < 12; ++i )
    {
        if( m_Players[i] )
            delete m_Players[i];
    }
}

bool CStatsDOTA :: ProcessAction( CIncomingAction *Action )
{
    if( m_Locked )
        return m_Winner != 0;

    unsigned int i = 0;
    BYTEARRAY *ActionData = Action->GetAction( );
    BYTEARRAY Data;
    BYTEARRAY Key;
    BYTEARRAY Value;

    // dota actions with real time replay data start with 0x6b then the null terminated string "dr.x"
    // unfortunately more than one action can be sent in a single packet and the length of each action isn't explicitly represented in the packet
    // so we have to either parse all the actions and calculate the length based on the type or we can search for an identifying sequence
    // parsing the actions would be more correct but would be a lot more difficult to write for relatively little gain
    // so we take the easy route (which isn't always guaranteed to work) and search the data for the sequence "6b 64 72 2e 78 00" and hope it identifies an action

    while( ActionData->size( ) >= i + 6 )
    {
        string MinString = UTIL_ToString( ( m_Game->m_GameTicks / 1000 ) / 60 );
        string SecString = UTIL_ToString( ( m_Game->m_GameTicks / 1000 ) % 60 );

        if( MinString.size( ) == 1 )
            MinString.insert( 0, "0" );

        if( SecString.size( ) == 1 )
            SecString.insert( 0, "0" );

        if( (*ActionData)[i] == 0x6b && (*ActionData)[i + 1] == 0x64 && (*ActionData)[i + 2] == 0x72 && (*ActionData)[i + 3] == 0x2e && (*ActionData)[i + 4] == 0x78 && (*ActionData)[i + 5] == 0x00 )
        {
            // we think we've found an action with real time replay data (but we can't be 100% sure)
            // next we parse out two null terminated strings and a 4 byte integer

            if( ActionData->size( ) >= i + 7 )
            {
                // the first null terminated string should either be the strings "Data" or "Global" or a player id in ASCII representation, e.g. "1" or "2"

                Data = UTIL_ExtractCString( *ActionData, i + 6 );

                if( ActionData->size( ) >= i + 8 + Data.size( ) )
                {
                    // the second null terminated string should be the key

                    Key = UTIL_ExtractCString( *ActionData, i + 7 + Data.size( ) );

                    if( ActionData->size( ) >= i + 12 + Data.size( ) + Key.size( ) )
                    {
                        // the 4 byte integer should be the value

                        Value = BYTEARRAY( ActionData->begin( ) + i + 8 + Data.size( ) + Key.size( ), ActionData->begin( ) + i + 12 + Data.size( ) + Key.size( ) );
                        string DataString = string( Data.begin( ), Data.end( ) );
                        string KeyString = string( Key.begin( ), Key.end( ) );
                        uint32_t ValueInt = UTIL_ByteArrayToUInt32( Value, false );

//                        m_Game->SendAllChat( "[STATS] " + DataString + ", " + KeyString + ", " + UTIL_ToString( ValueInt ) );
                        if( DataString == "Data" )
                        {
                            // these are received during the game
                            // you could use these to calculate killing sprees and double or triple kills (you'd have to make up your own time restrictions though)
                            // you could also build a table of "who killed who" data

                            if( KeyString.size( ) >= 5 && KeyString.substr( 0, 4 ) == "Hero" )
                            {
                                // a hero died

                                string VictimColourString = KeyString.substr( 4 );
                                uint32_t VictimColour = UTIL_ToUInt32( VictimColourString );
                                CGamePlayer *Killer = m_Game->GetPlayerFromColour( ValueInt );
                                CGamePlayer *Victim = m_Game->GetPlayerFromColour( VictimColour );

                                if( Killer )
                                {
                                    m_KillStreakCounter[ValueInt]++;
                                    if( ( m_KillCounter[ValueInt] == 0 && m_LatestKill[ValueInt] == 0 ) || GetTime() - m_LatestKill[ValueInt] >= 18)
                                    {
                                        m_KillCounter[ValueInt] = 1;
                                        m_LatestKill[ValueInt] = GetTime();
                                    }
                                    else if( m_KillCounter[ValueInt] != 0 && GetTime() - m_LatestKill[ValueInt] < 18 )
                                    {
                                        m_KillCounter[ValueInt]++;
                                        m_LatestKill[ValueInt] = GetTime();
                                        if(m_KillCounter[ValueInt] == 2 )
                                        {
                                            m_Game->GAME_Print( 131, MinString, SecString, Killer->GetName(), "", "" );
                                            Log->Debug("[STATS"+TypePrefix+"] "+Killer->GetName()+" got a double kill.");
                                            m_Game->AppendLogData("4\tk\t" + Killer->GetName( ) + "\t-\t" + m_Players[ValueInt]->GetHero( ) + "\t-\t" + MinString + ":" + SecString + "\tdk\n");
                                        }
                                        if(m_KillCounter[ValueInt] == 3 )
                                        {
                                            m_Game->GAME_Print( 132, MinString, SecString, Killer->GetName(), "", "" );
                                            Log->Debug("[STATS"+TypePrefix+"] "+Killer->GetName()+" got a tripple kill.");
                                            m_Game->AppendLogData("4\tk\t" + Killer->GetName( ) + "\t-\t" + m_Players[ValueInt]->GetHero( ) + "\t-\t" + MinString + ":" + SecString + "\ttk\n");
                                        }
                                        if(m_KillCounter[ValueInt] == 4 )
                                        {
                                            m_Game->GAME_Print( 133, MinString, SecString, Killer->GetName(), "", "" );
                                            Log->Debug("[STATS"+TypePrefix+"] "+Killer->GetName()+" got an ultra kill.");
                                            m_Game->AppendLogData("4\tk\t" + Killer->GetName( ) + "\t-\t" + m_Players[ValueInt]->GetHero( ) + "\t-\t" + MinString + ":" + SecString + "\tuk\n");
                                        }
                                        if(m_KillCounter[ValueInt] >= 5 )
                                        {
                                            m_Game->GAME_Print( 134, MinString, SecString, Killer->GetName(), "", "" );
                                            Log->Debug("[STATS"+TypePrefix+"] "+Killer->GetName()+" got a rampage.");
                                            m_Game->AppendLogData("4\tk\t" + Killer->GetName( ) + "\t-\t" + m_Players[ValueInt]->GetHero( ) + "\t-\t" + MinString + ":" + SecString + "\trk\n");
                                        }
                                    }
                                    switch(m_KillStreakCounter[ValueInt])
                                    {
                                    case 3:
                                        Log->Debug("[STATS"+TypePrefix+"] "+Killer->GetName()+" is on a killing spree.");
                                        break;
                                    case 4:
                                        Log->Debug("[STATS"+TypePrefix+"] "+Killer->GetName()+" is dominating.");
                                        break;
                                    case 5:
                                        Log->Debug("[STATS"+TypePrefix+"] "+Killer->GetName()+" has a mega kill.");
                                        break;
                                    case 6:
                                        Log->Debug("[STATS"+TypePrefix+"] "+Killer->GetName()+" is unstoppable.");
                                        break;
                                    case 7:
                                        Log->Debug("[STATS"+TypePrefix+"] "+Killer->GetName()+" is wicked sick.");
                                        break;
                                    case 8:
                                        Log->Debug("[STATS"+TypePrefix+"] "+Killer->GetName()+" has a monster kill.");
                                        break;
                                    case 9:
                                        Log->Debug("[STATS"+TypePrefix+"] "+Killer->GetName()+" is godlike.");
                                        break;
                                    case 10:
                                        Log->Debug("[STATS"+TypePrefix+"] "+Killer->GetName()+" is beyond godlike.");
                                        break;
                                    }
                                }
                                if( Killer && Victim )
                                {
                                    if( ( ValueInt >= 1 && ValueInt <= 5 ) || ( ValueInt >= 7 && ValueInt <= 11 ) )
                                    {
                                        if ((ValueInt <= 5 && VictimColour <= 5) || (ValueInt >= 7 && VictimColour >= 7))
                                        {
                                            m_Game->GAME_Print( 12, MinString, SecString, Killer->GetName(), Victim->GetName(), "" );
                                            // He denied a team-mate, don't count that.
                                        }
                                        else
                                        {
                                            // A legit kill, lets count that.

                                            if (!m_Players[ValueInt])
                                                m_Players[ValueInt] = new CDBDotAPlayer( );

                                            if (ValueInt != VictimColour)
                                                m_Players[ValueInt]->SetKills( m_Players[ValueInt]->GetKills() + 1 );

                                            if( ValueInt >= 1 && ValueInt <= 5 )
                                                m_SentinelKills++;
                                            else
                                                m_ScourgeKills++;

                                        }
                                    }

                                    if( ( VictimColour >= 1 && VictimColour <= 5 ) || ( VictimColour >= 7 && VictimColour <= 11 ) )
                                    {
                                        if (!m_Players[VictimColour])
                                            m_Players[VictimColour] = new CDBDotAPlayer( );

                                        m_Players[VictimColour]->SetDeaths( m_Players[VictimColour]->GetDeaths() + 1 );
                                        m_KillStreakCounter[VictimColour] = 0;
                                    }
                                    if( !m_FirstBlood )
                                    {
                                        m_Game->GAME_Print( 13, MinString, SecString, Killer->GetName(), Victim->GetName(), "" );
                                        Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) +  "] player [" + Killer->GetName( ) + "] slained player [" + Victim->GetName( ) + "] as first blood." );
                                        m_Game->AppendLogData("4\tfb\t" + Killer->GetName( ) + "\t" + Victim->GetName( ) + "[" + Victim->GetName( ) + "]");
                                        m_FirstBlood = true;
                                    }
                                    else
                                    {
                                        m_Game->GAME_Print( 14, MinString, SecString, Killer->GetName(), Victim->GetName(), "" );
                                        Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] player [" + Killer->GetName( ) + "] killed player [" + Victim->GetName( ) + "]" );
                                        m_Game->AppendLogData("4\tk\t" + Killer->GetName( ) + "\t" + Victim->GetName( ) + "\t" + m_Players[ValueInt]->GetHero( ) + "\t" + m_Players[VictimColour]->GetHero( ) + "\t" + MinString + ":" + SecString + "\t-\n");
                                    }

                                    if( GetTime() - Killer->GetLastAttackTimeToFountain() <=5 && m_Game->m_OHBot->m_FountainFarmDetection) {
                                        m_Game->SendChat(Killer->GetPID(), m_Game->m_OHBot->m_Language->YouHaveBeenDetectedAsFountainFarmer());
                                        m_FFKills[ValueInt]++;
                                        Killer->SetFFLevel();

                                        if(Killer->GetFFLevel() == 3 ) {
	                                         m_Game->BanPlayerByPenality( Killer->GetName(), Killer->GetExternalIPString(), m_Game->m_OHBot->m_BotManagerName, Killer->GetPenalityLevel(), "FountainFarm" );
                                        }
                                    }
                                }
                                else if( Victim )
                                {

                                    if( ( VictimColour >= 1 && VictimColour <= 5 ) || ( VictimColour >= 7 && VictimColour <= 11 ) )
                                    {
                                        if (!m_Players[VictimColour])
                                            m_Players[VictimColour] = new CDBDotAPlayer( );

                                        m_Game->GAME_Print( 15, MinString, SecString, Victim->GetName(), "", "" );
                                        m_Players[VictimColour]->SetDeaths( m_Players[VictimColour]->GetDeaths() + 1 );
                                        m_Game->AppendLogData("4\ts\t-\t" + Victim->GetName( ) + "\t-\t" + m_Players[VictimColour]->GetHero( ) + "\t" + MinString + ":" + SecString + "\t-\n");
                                    }
                                    if( ValueInt == 0 )
                                    {
                                        m_Game->GAME_Print( 16, MinString, SecString, Victim->GetName(), "", "" );
                                        m_SentinelKills++;
                                        Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] the Sentinel killed player [" + Victim->GetName( ) + "]" );
                                        m_Game->AppendLogData("4\tsek\tSentinel\t" + Victim->GetName( ) + "\t-\t" + m_Players[VictimColour]->GetHero( ) + "\t" + MinString + ":" + SecString + "\t-\n");
                                        m_KillStreakCounter[VictimColour] = 0;
                                    }
                                    else if( ValueInt == 6 )
                                    {
                                        m_Game->GAME_Print( 17, MinString, SecString, Victim->GetName(), "", "" );
                                        m_ScourgeKills++;
                                        Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] the Scourge killed player [" + Victim->GetName( ) + "]" );
                                        m_Game->AppendLogData("4\tsck\tScourge\t" + Victim->GetName( ) + "\t-\t" + m_Players[VictimColour]->GetHero( ) + "\t" + MinString + ":" + SecString + "\t-\n");
                                        m_KillStreakCounter[VictimColour] = 0;
                                    }
                                    else
                                    {
                                        // removing this, occur probably on neutral kills
                                        /*
                                        Log->Debug( "[ANTIFARM] player [" + Victim->GetName() + "] got killed by a leaver." );
                                        if( m_LeaverKills[VictimColour] == 0 && m_AssistsOnLeaverKills[VictimColour] == 0 )
                                                m_DeathsByLeaver[VictimColour]++;
                                        else
                                                m_Game->SendAllChat( "[ANTIFARM] Player ["+Victim->GetName()+"] killed already ["+UTIL_ToString(m_DeathsByLeaver[VictimColour])+"] leavers and assisted on ["+UTIL_ToString(m_AssistsOnLeaverKills[VictimColour])+"] kills. Justice has been done!" );
                                        */
                                    }
                                }

                                if( Killer && !Victim )
                                {
                                    Log->Debug( "[ANTIFARM] player [" + Killer->GetName() + "] killed a leaver." );
                                    m_LeaverKills[ValueInt]++;
                                    if( m_LeaverKills[ValueInt] >= m_Game->m_OHBot->m_MinimumLeaverKills )
                                        m_Game->SendAllChat( "[ANTIFARM] Player ["+Killer->GetName()+"] killed already ["+UTIL_ToString(m_LeaverKills[ValueInt])+"] leavers, the stats wont be recorded anymore." );
                                    m_LeaverDeaths[VictimColour]++;
                                    if( m_LeaverDeaths[ValueInt] >= m_Game->m_OHBot->m_MinimumLeaverDeaths )
                                        m_Game->SendAllChat( "[ANTIFARM] A leaver ["+UTIL_ToString(VictimColour)+"] was already ["+UTIL_ToString(m_LeaverDeaths[ValueInt])+"] times killed while he left. All remaining deaths wont be recorded." );

                                    if( GetTime() - Killer->GetLastAttackTimeToFountain() <=5 && m_Game->m_OHBot->m_FountainFarmDetection) {
                                        m_Game->SendChat(Killer->GetPID(), m_Game->m_OHBot->m_Language->YouHaveBeenDetectedAsFountainFarmer() );
                                        m_FFKills[ValueInt]++;
                                        Killer->SetFFLevel();

                                        if(Killer->GetFFLevel() == 3 ) {
					                                     m_Game->BanPlayerByPenality( Killer->GetName(), Killer->GetExternalIPString(), m_Game->m_OHBot->m_BotManagerName, Killer->GetPenalityLevel(), "FountainFarm" );
                                        }
                                    }
                                }

                                if( Victim != NULL )
                                {
                                    int32_t kills = m_Players[VictimColour]->GetKills( );;
                                    int32_t deaths = m_Players[VictimColour]->GetDeaths( );
                                    int32_t assists = m_Players[VictimColour]->GetAssists( );

                                    if( ( deaths - kills - ( assists * 0.5 ) ) >= 8 )
                                    {
                                        if( Victim->GetFeedLevel( ) != 2 )
                                            m_Game->SendAllChat( "[INFO] Player ["+Victim->GetName( )+"] got marked as feeder, you may votekick him." );
                                        Victim->SetFeedLevel( 2 );
                                    }
                                    else if( ( deaths - kills - ( assists * 0.5 ) ) > 5 )
                                    {
                                        m_Game->SendChat( Victim, "[INFO] Feed Detection triggered. Please stop dying to avoid a potential kick for feeding." );
                                        m_Game->SendChat( Victim, "[INFO] Feeding on purpose is bannable, for more information check out '!rule feeding'." );
                                        Victim->SetFeedLevel( 1 );
                                    }
                                }
                            }
                            else if( KeyString.size( ) >= 4 && KeyString.substr( 0, 6 ) == "Assist" )
                            {
                                // assist
                                string AssistentColourString = KeyString.substr( 6 );
                                uint32_t AssistentColour = UTIL_ToUInt32( AssistentColourString );
                                CGamePlayer *Assistent = m_Game->GetPlayerFromColour( AssistentColour );
                                CGamePlayer *Victim = m_Game->GetPlayerFromColour( ValueInt );

                                if( !m_Players[AssistentColour] )
                                    m_Players[AssistentColour] = new CDBDotAPlayer( );

                                if ( Assistent && Victim )
                                {
                                    m_Game->GAME_Print( 18,  MinString, SecString, Assistent->GetName(), Victim->GetName( ), "" );
                                    m_Players[AssistentColour]->SetAssists(m_Players[AssistentColour]->GetAssists() + 1);
                                    Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] player [" + Assistent->GetName( ) + "] assist to kill player [" + Victim->GetName( ) + "]" );
                                    m_Game->AppendLogData("4\ta\t" + Assistent->GetName( ) + "\t" + Victim->GetName( ) + "\t" + m_Players[AssistentColour]->GetHero( ) + "\t" + m_Players[ValueInt]->GetHero( ) + "\t" + MinString + ":" + SecString + "\t-\n");
                                }

                                if( Assistent && !Victim )
                                {
                                    Log->Debug( "[ANTIFARM] Player ["+Assistent->GetName( )+"] assisted to kill a leaver." );
                                    m_AssistsOnLeaverKills[AssistentColour]++;
                                    if( m_AssistsOnLeaverKills[AssistentColour] >= m_Game->m_OHBot->m_MinimumLeaverKills )
                                        m_Game->SendAllChat( "[ANTIFARM] Player ["+Assistent->GetName()+"] assisted to kill ["+UTIL_ToString(m_AssistsOnLeaverKills[AssistentColour])+"] leavers, all assists for leavers will be removed." );
                                }
                            }
                            else if( KeyString.size( ) >= 8 && KeyString.substr( 0, 7 ) == "Courier" )
                            {
                                // a courier died

                                if( ( ValueInt >= 1 && ValueInt <= 5 ) || ( ValueInt >= 7 && ValueInt <= 11 ) )
                                {
                                    if( !m_Players[ValueInt] )
                                        m_Players[ValueInt] = new CDBDotAPlayer( );

                                    m_Players[ValueInt]->SetCourierKills( m_Players[ValueInt]->GetCourierKills( ) + 1 );
                                }

                                string VictimColourString = KeyString.substr( 7 );
                                uint32_t VictimColour = UTIL_ToUInt32( VictimColourString );
                                CGamePlayer *Killer = m_Game->GetPlayerFromColour( ValueInt );
                                CGamePlayer *Victim = m_Game->GetPlayerFromColour( VictimColour );
                                /*
                                    if( Killer && Victim )
                                        Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] player [" + Killer->GetName( ) + "] killed a courier owned by player [" + Victim->GetName( ) + "]" );
                                    else if( Victim )
                                    {
                                        if( ValueInt == 0 )
                                            Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] the Sentinel killed a courier owned by player [" + Victim->GetName( ) + "]" );
                                        else if( ValueInt == 6 )
                                            Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] the Scourge killed a courier owned by player [" + Victim->GetName( ) + "]" );
                                    }
                                */
                            }
                            else if( KeyString.size( ) >= 8 && KeyString.substr( 0, 5 ) == "Tower" )
                            {
                                // a tower died

                                if( ( ValueInt >= 1 && ValueInt <= 5 ) || ( ValueInt >= 7 && ValueInt <= 11 ) )
                                {
                                    if( !m_Players[ValueInt] )
                                        m_Players[ValueInt] = new CDBDotAPlayer( );

                                    m_Players[ValueInt]->SetTowerKills( m_Players[ValueInt]->GetTowerKills( ) + 1 );
                                }

                                string Alliance = KeyString.substr( 5, 1 );
                                string Level = KeyString.substr( 6, 1 );
                                string Side = KeyString.substr( 7, 1 );
                                CGamePlayer *Killer = m_Game->GetPlayerFromColour( ValueInt );
                                string AllianceString;
                                string SideString;

                                if( Alliance == "0" )
                                {
                                    m_ScourgeTowers++;
                                    AllianceString = "Sentinel";
                                }
                                else if( Alliance == "1" )
                                {
                                    m_SentinelTowers++;
                                    AllianceString = "Scourge";
                                }
                                else
                                    AllianceString = "unknown";

                                if( Side == "0" )
                                    SideString = "top";
                                else if( Side == "1" )
                                    SideString = "mid";
                                else if( Side == "2" )
                                    SideString = "bottom";
                                else
                                    SideString = "unknown";
                                if( Level == "3" && m_Game->m_OHBot->m_FountainFarmWarning ) {
                                    m_Game->SendAllChat( "["+m_Game->m_OHBot->m_BotManagerName+"] "+m_Game->m_OHBot->m_FountainFarmMessage );
                                }
                                if( Killer ) {
                                    m_Game->GAME_Print( 19, MinString, SecString, Killer->GetName( ), AllianceString, SideString+" "+Level );
                                    Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] player [" + Killer->GetName( ) + "] destroyed a level [" + Level + "] " + AllianceString + " tower (" + SideString + ")" );
                                    m_Game->AppendLogData("4\ttok\t" + Killer->GetName( ) + "\t" + AllianceString + "\t" + m_Players[ValueInt]->GetHero( ) + "\t-\t" + MinString + ":" + SecString + "\t" + SideString + ":" + Level + "\n");
                                }
                                else
                                {
                                    if( ValueInt == 0 ) {
                                        m_Game->GAME_Print( 20, MinString, SecString, "", "", SideString+" "+Level );
                                        Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] the Sentinel destroyed a level [" + Level + "] " + AllianceString + " tower (" + SideString + ")" );
                                        m_Game->AppendLogData("4\tsetk\tSentinel\tScourge\t-\t-\t" + MinString + ":" + SecString + "\t" + SideString + ":" + Level + "\n");
                                    }
                                    else if( ValueInt == 6 ) {
                                        m_Game->GAME_Print( 21, MinString, SecString, "", "", SideString+" "+Level );
                                        Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] the Scourge destroyed a level [" + Level + "] " + AllianceString + " tower (" + SideString + ")" );
                                        m_Game->AppendLogData("4\tsctk\tScourge\tSentinel\t-\t-\t" +MinString + ":" + SecString + "\t" + SideString + ":" + Level + "\n");
                                    }
                                }
                            }
                            else if( KeyString.size( ) >= 6 && KeyString.substr( 0, 3 ) == "Rax" )
                            {
                                // a rax died

                                if( ( ValueInt >= 1 && ValueInt <= 5 ) || ( ValueInt >= 7 && ValueInt <= 11 ) )
                                {
                                    if( !m_Players[ValueInt] )
                                        m_Players[ValueInt] = new CDBDotAPlayer( );

                                    m_Players[ValueInt]->SetRaxKills( m_Players[ValueInt]->GetRaxKills( ) + 1 );
                                }

                                string Alliance = KeyString.substr( 3, 1 );
                                string Side = KeyString.substr( 4, 1 );
                                string Type = KeyString.substr( 5, 1 );
                                CGamePlayer *Killer = m_Game->GetPlayerFromColour( ValueInt );
                                string AllianceString;
                                string SideString;
                                string TypeString;

                                if( Alliance == "0" )
                                    AllianceString = "Sentinel";
                                else if( Alliance == "1" )
                                    AllianceString = "Scourge";
                                else
                                    AllianceString = "unknown";

                                if( Side == "0" )
                                    SideString = "top";
                                else if( Side == "1" )
                                    SideString = "mid";
                                else if( Side == "2" )
                                    SideString = "bottom";
                                else
                                    SideString = "unknown";

                                if( Type == "0" )
                                    TypeString = "melee";
                                else if( Type == "1" )
                                    TypeString = "ranged";
                                else
                                    TypeString = "unknown";

                                if( Killer ) {
                                    m_Game->GAME_Print( 22, MinString, SecString, Killer->GetName(), AllianceString, SideString+" "+TypeString );
                                    Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] player [" + Killer->GetName( ) + "] destroyed a " + TypeString + " " + AllianceString + " rax (" + SideString + ")" );
                                    m_Game->AppendLogData("4\trk\t" + Killer->GetName( ) + "\t" + AllianceString + "\t" + m_Players[ValueInt]->GetHero( ) + "\t-\t" + MinString + ":" + SecString + "\t" + SideString + ":" + TypeString + "\n");
                                }
                                else
                                {
                                    if( ValueInt == 0 ) {
                                        m_Game->GAME_Print( 23, MinString, SecString, "", "", SideString+" "+TypeString );
                                        Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] the Sentinel destroyed a " + TypeString + " " + AllianceString + " rax (" + SideString + ")" );
                                        m_Game->AppendLogData("4\tserk\tSentinel\t" + AllianceString + "\t-\t-\t" + MinString + ":" + SecString + "\t" + SideString + ":" + TypeString + "\n");
                                    }
                                    else if( ValueInt == 6 ) {
                                        m_Game->GAME_Print( 24, MinString, SecString, "", "", SideString+" "+TypeString );
                                        Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] the Scourge destroyed a " + TypeString + " " + AllianceString + " rax (" + SideString + ")" );
                                        m_Game->AppendLogData("4\tscrk\tScourge\t" + AllianceString + "\t-\t-\t" + MinString + ":" + SecString + "\t" + SideString + ":" + TypeString + "\n");
                                    }
                                }
                            }
                            else if( KeyString.size( ) >= 6 && KeyString.substr( 0, 5 ) == "Level" )
                            {
                                string LevelString = KeyString.substr( 5 );
                                uint32_t Level = UTIL_ToUInt32( LevelString );
                                CGamePlayer *Player = m_Game->GetPlayerFromColour( ValueInt );
                                if (Player)
                                {
                                    if (!m_Players[ValueInt])
                                        m_Players[ValueInt] = new CDBDotAPlayer( );

                                    m_Players[ValueInt]->SetLevel(Level);
                                    Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] "+ Player->GetName() + " is now level " + UTIL_ToString(m_Players[ValueInt]->GetLevel()) );
                                    m_Game->AppendLogData("4\tlu\t" + Player->GetName( ) + "\t-\t" + m_Players[ValueInt]->GetHero( ) + "\t-\t" + MinString + ":" + SecString + "\t" + UTIL_ToString(m_Players[ValueInt]->GetLevel()) + "\n");
                                }
                            }
                            else if( KeyString.size( ) >= 8 && KeyString.substr( 0, 4 ) == "SWAP" )
                            {

                                // swap players
                                int i = KeyString.find( "_") + 1;
                                int y = KeyString.find( "_", i );
                                string FromString = KeyString.substr( i, y-i );
                                uint32_t FromColour = UTIL_ToUInt32( FromString );
                                CGamePlayer *FromPlayer = m_Game->GetPlayerFromColour( FromColour );
                                string ToString = KeyString.substr( y + 1 );
                                uint32_t ToColour = UTIL_ToUInt32( ToString );
                                CGamePlayer *ToPlayer = m_Game->GetPlayerFromColour( ToColour );
                                m_Game->GAME_Print( 25, MinString, SecString, FromPlayer->GetName( ), ToPlayer->GetName( ), "req" );

                                if ((FromColour >= 1 && FromColour <= 5 ) || ( FromColour >= 7 && FromColour <= 11 ))
                                    if ((ToColour >= 1 && ToColour <= 5 ) || ( ToColour >= 7 && ToColour <= 11 ))
                                    {
                                        m_Players[ToColour]->SetNewColour( FromColour );
                                        m_Players[FromColour]->SetNewColour( ToColour );
                                        //                                                                      Log->Debug( m_Players[ToColour] +" / "+ m_Players[FromColour] );
                                        CDBDotAPlayer* bufferPlayer = m_Players[ToColour];
                                        m_Players[ToColour] = m_Players[FromColour];
                                        m_Players[FromColour] = bufferPlayer;

                                        if ( FromPlayer ) FromString = FromPlayer->GetName( );
                                        if ( ToPlayer ) ToString = ToPlayer->GetName( );
                                        Log->Debug( "SWITCH " + FromPlayer->GetName( ) + " with " + ToPlayer->GetName( ) );
                                        m_Game->GAME_Print( 25, MinString, SecString, FromPlayer->GetName( ), ToPlayer->GetName( ), "succ" );
                                        Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] swap players from ["+FromString+"] to ["+ToString+"]." );
                                        m_Game->AppendLogData("4\tsw\t" + FromPlayer->GetName() + "\t" + ToPlayer->GetName() + "\t" + m_Players[FromColour]->GetHero() + "\t" + m_Players[ToColour]->GetHero() + "\t" + MinString + ":" + SecString + "\t-\n");
                                    }
                            }
                            else if( KeyString.size( ) >= 6 && KeyString.substr( 0, 6 ) == "Throne" )
                            {
                                // the frozen throne got hurt
                                m_Game->GAME_Print( 26, MinString, SecString, "", "", UTIL_ToString( ValueInt ) );
                                Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] the Frozen Throne is now at " + UTIL_ToString( ValueInt ) + "% HP" );
                                m_Game->AppendLogData("4\tftk\t-\tFrozen Throne\t-\t-\t" + MinString + ":" + SecString + "\t" + UTIL_ToString( ValueInt ) + "\n");

                            }
                            else if ( KeyString.size( ) >= 6 && KeyString.substr( 0, 6 ) == "Roshan" )
                            {
                                if( ValueInt == 0 ) {
                                    m_Game->GAME_Print( 28, MinString, SecString, "Sentinel", "", "" );
                                    Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] Roshan killed by the Sentinel" );
                                    m_Game->AppendLogData("4\trosh\tSentinel\t-\t-\t-\t" + MinString + ":" + SecString + "\t-\n");
                                }
                                else if( ValueInt == 6 ) {
                                    m_Game->GAME_Print( 28, MinString, SecString, "Scourge", "", "");
                                    Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] Roshan killed by the Scourge" );
                                    m_Game->AppendLogData("4\trosh\tScourge\t-\t-\t-\t" + MinString + ":" + SecString + "\t-\n");
                                }
                            }
                            else if ( KeyString.size( ) >= 7 && KeyString.substr( 0, 7 ) == "AegisOn")
                            {
                                CGamePlayer *Player = m_Game->GetPlayerFromColour( ValueInt );
                                if (Player) {
                                    Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] "+Player->GetName( ) + " picked up AEGIS." );
                                    m_Game->GAME_Print( 29, MinString, SecString, Player->GetName( ), "", "pick" );
                                    m_Game->AppendLogData("4\tap\t" + Player->GetName( ) + "\t-\t-\t-\t" + MinString + ":" + SecString + "\t-\n");
                                }
                            }
                            else if ( KeyString.size( ) >= 8 && KeyString.substr( 0, 8 ) == "AegisOff")
                            {
                                CGamePlayer *Player = m_Game->GetPlayerFromColour( ValueInt );
                                if (Player) {
                                    Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] "+Player->GetName( ) + " dropped AEGIS." );
                                    m_Game->GAME_Print( 29, MinString, SecString, Player->GetName( ), "", "drop" );
                                    m_Game->AppendLogData("4\tad\t" + Player->GetName( ) + "\t-\t-\t-\t" + MinString + ":" + SecString + "\t-\n");
                                }
                            }
                            else if( KeyString.size( ) >= 4 && KeyString.substr( 0, 4 ) == "Tree" )
                            {
                                // the world tree got hurt

                                m_Game->GAME_Print( 27, MinString, SecString, "", "", UTIL_ToString( ValueInt ) );
                                Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] the World Tree is now at " + UTIL_ToString( ValueInt ) + "% HP" );
                                m_Game->AppendLogData("4\twtk\t-\tWorld Tree\t-\t-\t" + MinString + ":" + SecString + "\t" + UTIL_ToString( ValueInt ) + "\n");

                            }
                            else if( KeyString.size( ) >= 2 && KeyString.substr( 0, 2 ) == "CK" )
                            {
                                // a player disconnected
                            }
                            else if( KeyString.size( ) >= 3 && KeyString.substr( 0, 3 ) == "CSK" )
                            {
                                /*
                                  if( ( ValueInt >= 1 && ValueInt <= 5 ) || ( ValueInt >= 7 && ValueInt <= 11 ) )
                                  {
                                          if( !m_Players[ValueInt] )
                                                  m_Players[ValueInt] = new CDBDotAPlayer( );

                                          m_Players[ValueInt]->SetCreepKills( m_Players[ValueInt]->GetCreepKills( ) + 1 );
                                  }

                                */
                                // creep kill value recieved (aprox every 3 - 4)
                                string PlayerID = KeyString.substr( 3 );
                                uint32_t ID = UTIL_ToUInt32( PlayerID );

                                if( ( ID >= 1 && ID <= 5 ) || ( ID >= 7 && ID <= 11 ) )
                                {
                                    if (!m_Players[ID])
                                        m_Players[ID] = new CDBDotAPlayer( );

                                    m_Players[ID]->SetCreepKills(ValueInt);
                                }
                            }
                            else if( KeyString.size( ) >= 3 && KeyString.substr( 0, 3 ) == "CSD" )
                            {
                                // creep denie value recieved (aprox every 3 - 4)
                                string PlayerID = KeyString.substr( 3 );
                                uint32_t ID = UTIL_ToUInt32( PlayerID );

                                if( ( ID >= 1 && ID <= 5 ) || ( ID >= 7 && ID <= 11 ) )
                                {

                                    if (!m_Players[ID])
                                        m_Players[ID] = new CDBDotAPlayer( );

                                    m_Players[ID]->SetCreepDenies(ValueInt);
                                }
                            }
                            else if( KeyString.size( ) >= 2 && KeyString.substr( 0, 2 ) == "NK" )
                            {
                                // creep denie value recieved (aprox every 3 - 4)
                                string PlayerID = KeyString.substr( 2 );
                                uint32_t ID = UTIL_ToUInt32( PlayerID );

                                if( ( ID >= 1 && ID <= 5 ) || ( ID >= 7 && ID <= 11 ) )
                                {
                                    if (!m_Players[ID])
                                        m_Players[ID] = new CDBDotAPlayer( );

                                    m_Players[ID]->SetNeutralKills(ValueInt);
                                }
                            }
                            else if (KeyString.size( ) >= 10 && KeyString.substr( 0, 9 ) == "RuneStore")
                            {
                                string RuneType = KeyString.substr( 9, 1 );
                                /*
                                    RuneType:
                                    1. Haste
                                    2. Regeneration
                                    3. Double Damage
                                    4. Illusion
                                    5. Illusion
                                */
                                string Rune = "unknown";
                                if( RuneType == "1" ) {
                                    Rune = "Haste";
                                }
                                else if( RuneType == "2" ) {
                                    Rune = "Regeneration";
                                }
                                else if( RuneType == "3" ) {
                                    Rune = "Double Damage";
                                }
                                else if( RuneType == "4" ) {
                                    Rune = "Illusion";
                                }
                                else if( RuneType == "5" ) {
                                    Rune = "Invisible";
                                }
                                else if( RuneType == "6" ) {
                                  Rune = "Bounty";
                                }
                                CGamePlayer *Player = m_Game->GetPlayerFromColour( ValueInt );
                                if (Player) {
                                    m_Game->GAME_Print( 30, MinString, SecString, Player->GetName(), "", RuneType );
                                    Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] Player " + Player->GetName() + " bottled a " + Rune + " Rune." );
                                    m_Game->AppendLogData("4\trs\t" + Player->GetName( ) + "\t-\t-\t-\t" + MinString + ":" + SecString + "\t" + Rune + "\n");
                                }
                            }
                            else if (KeyString.size( ) >= 8 && KeyString.substr( 0, 7 ) == "RuneUse")
                            {
                                string RuneType = KeyString.substr( 7, 1 );
                                /*
                                        RuneType:
                                        1. Haste
                                        2. Regeneration
                                        3. Double Damage
                                        4. Illusion
                                        5: Invis
                                */
                                string Rune = "unknown";
                                if( RuneType == "1" ) {
                                    Rune = "Haste";
                                }
                                else if( RuneType == "2" ) {
                                    Rune = "Regeneration";
                                }
                                else if( RuneType == "3" ) {
                                    Rune = "Double Damage";
                                }
                                else if( RuneType == "4" ) {
                                    Rune = "Illusion";
                                }
                                else if( RuneType == "5" ) {
                                    Rune = "Invisible";
                                }
                                else if( RuneType == "6" ) {
                                  Rune = "Bounty";
                                }
                                CGamePlayer *Player = m_Game->GetPlayerFromColour( ValueInt );
                                if (Player) {
                                    m_Game->GAME_Print( 31, MinString, SecString, Player->GetName(), "", RuneType );
                                    Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] Player " + Player->GetName() + " used a " + Rune + " Rune." );
                                    m_Game->AppendLogData("4\tru\t" + Player->GetName( ) + "\t-\t-\t-\t" + MinString + ":" + SecString + "\t" + Rune + "\n");
                                }
                            }
                            else if(KeyString.size( ) >= 5 && KeyString.substr( 0, 4 ) == "PUI_")
                            {
                                string Item = string( Value.rbegin( ), Value.rend( ) );
                                string PlayerID = KeyString.substr( 4 );
                                uint32_t ID = UTIL_ToUInt32( PlayerID );
                                m_BufferedItemCount[ID]++;
                                if(m_BufferedItemCount[ID]==1)
                                    m_BufferedItemOne[ID] = Item;
                                if(m_BufferedItemCount[ID]==2)
                                    m_BufferedItemTwo[ID] = Item;
                                if(m_BufferedItemCount[ID]==3)
                                    m_BufferedItemThree[ID] = Item;
                                if(m_BufferedItemCount[ID]==4)
                                    m_BufferedItemFour[ID] = Item;
                                if(m_BufferedItemCount[ID]==5)
                                    m_BufferedItemFive[ID] = Item;
                                if(m_BufferedItemCount[ID]==6) {
                                    m_BufferedItemSix[ID] = Item;
                                    m_BufferedItemCount[ID]=0;
                                }
                                CGamePlayer *Player = m_Game->GetPlayerFromColour( ID );
                                if (Player) {
                                    m_Game->AppendLogData("4\tinv\t" + Player->GetName( ) + "\t-\t-\t-\t" + MinString + ":" + SecString + "\t" + Item + "\n");
                                }
                            }
                            else if(KeyString.size( ) >= 5 && KeyString.substr( 0, 4 ) == "DRI_")
                            {
                                string Item = string( Value.rbegin( ), Value.rend( ) );
                                string PlayerID = KeyString.substr( 4 );
                                uint32_t ID = UTIL_ToUInt32( PlayerID );
                                CGamePlayer *Player = m_Game->GetPlayerFromColour( ID );
                                if (Player)
                                    m_Game->AppendLogData("4\tinv\t" + Player->GetName( ) + "\t-\t-\t-\t" + MinString + ":" + SecString + "\t" + Item + "\n");
                            }
                            else if(KeyString.size( ) >= 11 && KeyString.substr( 0, 11 ) == "GameStarted")
                            {
                                m_Game->GAME_Print( 1, MinString, SecString, "System", "", "Game started, Creeps spawned" );
                                Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] Game Started, creeps spawned" );
                                m_Game->AppendLogData("4\tcreep_spawn\t\t-\t-\t-\t" + MinString + ":" + SecString + "\t\n");
                            }
                            else if(KeyString.size( ) >= 4 && KeyString.substr( 0, 4 ) == "Mode")
                            {
                                string ModeString = KeyString.substr( 4 );
                                Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] Mode Set: " + ModeString );
                                m_Game->GAME_Print( 1, MinString, SecString, "System", "", ModeString );
                                m_Game->AppendLogData("4\tmode\t-\t-\t-\t-\t" + MinString + ":" + SecString + "\t" + ModeString + "\n");
                            }
                  			    else if( KeyString.size( ) >= 2 && KeyString.substr( 0, 2 ) == "SP" )
                  			    {
                      				string Spell = string( Value.rbegin( ), Value.rend( ) );
                      				string PlayerID = KeyString.substr( 4 );
                      				string SpellID = KeyString.substr( 2, 1 );
                      				uint32_t ID = UTIL_ToUInt32( PlayerID );
                      				CGamePlayer *Player = m_Game->GetPlayerFromColour( ID );
                      				Log->Debug( "[DEBUG:  Found Spell: "+Spell+ " | PlayerID: "+PlayerID+" | SpellID: "+SpellID  );
                              if( ( ID >= 1 && ID <= 5 ) || ( ID >= 7 && ID <= 11 ) ) {
                              	if( !m_Players[ID] )
                                	m_Players[ID] = new CDBDotAPlayer( );

                                m_Players[ID]->SetSpell( UTIL_ToUInt32( SpellID ) - 1, Spell );
                      					m_Game->AppendLogData("4\tspell\t" + Player->GetName( ) + "\t" + SpellID + "\t" + Spell + "\t-\t" + MinString + ":" + SecString + "\t\n");
                      				}
                  			    }
                  		   	  else if( KeyString.size( ) >= 2 && KeyString.substr( 0, 2 ) == "MH" )
                            {
                                string Spell = string( Value.rbegin( ), Value.rend( ) );
                                string PlayerID = KeyString.substr( 2 );
                                uint32_t ID = UTIL_ToUInt32( PlayerID );
                  				      uint32_t Time = ValueInt;
                                CGamePlayer *Player = m_Game->GetPlayerFromColour( ID );
                        				if( Player ) {
                        					m_MaphackClickCounter[ID]++;
                        					Log->Info("[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] Increased MaphackClickCounter for ["+Player->GetName()+"] to ["+UTIL_ToString(m_MaphackClickCounter[ID])+"]");
                        					if(m_MaphackClickCounter[ID] >= 2) {
                        						m_Game->m_PairedSSs.push_back(PairedSS( string(), m_Game->m_OHBot->m_DB->ThreadedStatsSystem( Player->GetName(), m_Game->m_GameName, ValueInt, "mh_detection" ) ) );
                        						m_Game->m_PairedBanAdds.push_back( PairedBanAdd( string(), m_Game->m_OHBot->m_DB->ThreadedBanAdd( Player->GetSpoofedRealm( ), Player->GetName( ), Player->GetExternalIPString( ), m_Game->m_GameName, m_Game->m_OHBot->m_BotManagerName, "Maphack", 0, "" ) ) );

                        						Player->SetDeleteMe( true );
                        						Player->SetLeftReason( m_Game->m_OHBot->m_Language->WasKickedByPlayer( Player->GetName() ) );
                        						Player->SetLeftCode( PLAYERLEAVE_LOST );
                        						m_Game->SendAllChat("Player ["+Player->GetName()+"] got removed from the game for using unallowed 3rd party tools.");

                        					}
                        				}
                           }
                  			   else if( KeyString.size( ) >= 12 && KeyString.substr( 0, 12 ) == "LoD_HeroResp" ) {
                      				string ID = KeyString.substr(13);
                      				uint32_t PlayerID = UTIL_ToUInt32(ID);
                      				string type = string( Value.rbegin( ), Value.rend( ) );
                      				uint32_t bb  = UTIL_ToUInt32( type );

                      				CGamePlayer *Player = m_Game->GetPlayerFromColour( PlayerID );
                      				if(Player) {
                      					Log->Info("[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] Player ["+Player->GetName( )+"] respawned." + (bb==1?"(buyback)":"(typical)"));
                      				}
                  			   }
                  			   else if(KeyString.size() >= 11 && KeyString.substr(0, 11) == "LoD_DeadFor" ) {
                                string ID = KeyString.substr(12);
                                uint32_t PlayerID = UTIL_ToUInt32(ID);
                                string time = string( Value.rbegin( ), Value.rend( ) );

                                CGamePlayer *Player = m_Game->GetPlayerFromColour( PlayerID );
                                if(Player) {
                                        Log->Info("[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] Player ["+Player->GetName( )+"] is dead for the next ["+time+"] seconds.");
                                }
			                     }
                           else if(KeyString.size() >= 7 && KeyString.substr(0, 7) == "LoD_INV" ) {
                                string ID = KeyString.substr(8);
                                uint32_t PlayerID = UTIL_ToUInt32(ID);
                                string item = string( Value.rbegin( ), Value.rend( ) );

                                CGamePlayer *Player = m_Game->GetPlayerFromColour( PlayerID );
                                if(Player) {
                                        Log->Info("[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] Player ["+Player->GetName( )+"] got item in inventory ["+item+"]");
                                }
                           }
                           else if(KeyString.size() >= 9 && KeyString.substr(0, 9) == "LoD_HeroX" ) {
                                string val = KeyString.substr(10);
                                CGamePlayer *Player = m_Game->GetPlayerFromColour( ValueInt );
                                if(Player) {
                                        Log->Info("[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] Player ["+Player->GetName( )+"] has a noted X-Position ["+val+"]");
                                }
                           }
                           else if(KeyString.size() >= 9 && KeyString.substr(0, 9) == "LoD_HeroY" ) {
                                string val = KeyString.substr(10);
                                CGamePlayer *Player = m_Game->GetPlayerFromColour( ValueInt );
                                if(Player) {
                                        Log->Info("[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] Player ["+Player->GetName( )+"] has a noted Y-Position ["+val+"]");
                                }
                           }
                        }
                        else if( DataString == "Global" )
                        {
                            // these are only received at the end of the game

                            if( KeyString == "Winner" && m_Winner != 1 && m_Winner != 2 )
                            {
                                // Value 1 -> sentinel
                                // Value 2 -> scourge

                                m_Winner = ValueInt;

                                if( m_Winner == 1 )
                                    Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] detected winner: Sentinel" );
                                else if( m_Winner == 2 )
                                    Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] detected winner: Scourge" );
                                else
                                    Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] detected winner: " + UTIL_ToString( ValueInt ) );
                            }
                            else if( KeyString == "m" )
                                m_Min = ValueInt;
                            else if( KeyString == "s" )
                                m_Sec = ValueInt;

                            if( m_Winner == 1 )
                                m_Game->GAME_Print( 1, MinString, SecString, "", "", "Winner detected: [Sentinel]" );
                            else if( m_Winner == 2 )
                                m_Game->GAME_Print( 1, MinString, SecString, "", "", "Winner detected: [Scourge]" );
                            else
                                m_Game->GAME_Print( 1, MinString, SecString, "", "", "No winner detected, game ended in a draw" );


                        }
                        else if( DataString.size( ) <= 2 && DataString.find_first_not_of( "1234567890" ) == string :: npos )
                        {
                            // these are only received at the end of the game

                            uint32_t ID = UTIL_ToUInt32( DataString );

                            if( ( ID >= 1 && ID <= 5 ) || ( ID >= 7 && ID <= 11 ) )
                            {
                                if( !m_Players[ID] )
                                {
                                    m_Players[ID] = new CDBDotAPlayer( );
                                    m_Players[ID]->SetColour( ID );
                                }

                                // Key "1"		-> Kills
                                // Key "2"		-> Deaths
                                // Key "3"		-> Creep Kills
                                // Key "4"		-> Creep Denies
                                // Key "5"		-> Assists
                                // Key "6"		-> Current Gold
                                // Key "7"		-> Neutral Kills
                                // Key "8_0"	-> Item 1
                                // Key "8_1"	-> Item 2
                                // Key "8_2"	-> Item 3
                                // Key "8_3"	-> Item 4
                                // Key "8_4"	-> Item 5
                                // Key "8_5"	-> Item 6
                                // Key "id"		-> ID (1-5 for sentinel, 6-10 for scourge, accurate after using -sp and/or -switch)

                                if( KeyString == "1" )
                                {
                                    if ( m_LeaverKills[ID] >= m_Game->m_OHBot->m_MinimumLeaverKills )
                                    {
                                        Log->Debug( "[ANTIFARM] Player with colour [" + UTIL_ToString(ID) + "] got [" + UTIL_ToString(ValueInt) + "] kills, removing [" + UTIL_ToString(m_LeaverKills[ID]) + "] leaver kills." );
                                        m_Players[ID]->SetKills( ValueInt - m_LeaverKills[ID] );
                                    }
                                    else
                                        m_Players[ID]->SetKills( ValueInt - m_FFKills[ID] );
                                    if( ValueInt > 500 )
                                    {
                                        m_Players[ID]->SetKills( 0 );
                                        Log->Debug("[INFO] Noted an invalid statsrecord on playercolour ["+UTIL_ToString(ID)+"]. Set the kills to [0]" );
                                    }
                                }
                                else if( KeyString == "2" )
                                {
                                    if( m_LeaverDeaths[ID] >= m_Game->m_OHBot->m_MinimumLeaverDeaths )
                                    {
                                        if( m_DeathsByLeaver[ID] != 0 && m_Game->m_OHBot->m_DeathsByLeaverReduction )
                                        {
                                            Log->Debug( "[ANTIFARM] Player with colour [" + UTIL_ToString(ID) + "] got [" + UTIL_ToString(ValueInt) + "] deaths, removing [" + UTIL_ToString(m_LeaverDeaths[ID]) + "] leaver deaths and ["+UTIL_ToString(m_LeaverDeaths[ID])+"] kills from leavers." );
                                            m_Players[ID]->SetDeaths( ValueInt - m_LeaverDeaths[ID] - m_LeaverDeaths[ID]);
                                        }
                                        else
                                        {
                                            Log->Debug( "[ANTIFARM] Player with colour [" + UTIL_ToString(ID) + "] got [" + UTIL_ToString(ValueInt) + "] deaths, removing [" + UTIL_ToString(m_LeaverDeaths[ID]) + "] leaver deaths." );
                                            m_Players[ID]->SetDeaths( ValueInt - m_LeaverDeaths[ID] );
                                        }
                                    }
                                    else if( m_DeathsByLeaver[ID] != 0 && m_Game->m_OHBot->m_DeathsByLeaverReduction && m_LeaverDeaths[ID] < m_Game->m_OHBot->m_MinimumLeaverDeaths )
                                    {
                                        Log->Debug( "[ANTIFARM] Player with colour [" + UTIL_ToString(ID) + "] got [" + UTIL_ToString(ValueInt) + "] deaths, removing ["+UTIL_ToString(m_DeathsByLeaver[ID])+"] kills from leavers." );
                                        m_Players[ID]->SetDeaths( ValueInt - m_LeaverDeaths[ID] - m_LeaverDeaths[ID]);
                                    }
                                    else
                                        m_Players[ID]->SetDeaths( ValueInt );
                                    if( ValueInt > 500 )
                                    {
                                        m_Players[ID]->SetDeaths( 0 );
                                        Log->Debug("[INFO] Noted an invalid statsrecord on playercolour ["+UTIL_ToString(ID)+"]. Set the deaths to [0]" );
                                    }
                                }
                                else if( KeyString == "3" )
                                {
                                    m_Players[ID]->SetCreepKills( ValueInt );
                                    if( ValueInt > 5000 )
                                    {
                                        m_Players[ID]->SetCreepKills( 0 );
                                        Log->Debug("[INFO] Noted an invalid statsrecord on playercolour ["+UTIL_ToString(ID)+"]. Set the creepkills to [0]" );
                                    }
                                }
                                else if( KeyString == "4" )
                                {
                                    m_Players[ID]->SetCreepDenies( ValueInt );
                                    if( ValueInt > 5000 )
                                    {
                                        m_Players[ID]->SetCreepDenies( 0 );
                                        Log->Debug("[INFO] Noted an invalid statsrecord on playercolour ["+UTIL_ToString(ID)+"]. Set the creepdenies to [0]" );
                                    }
                                }
                                else if( KeyString == "5" )
                                {
                                    if( m_AssistsOnLeaverKills[ID] >= m_Game->m_OHBot->m_MinimumLeaverAssists )
                                    {
                                        Log->Debug( "[ANTIFARM] Player with colour [" + UTIL_ToString(ID) + "] got [" + UTIL_ToString(ValueInt) + "] assists, removing [" + UTIL_ToString( m_AssistsOnLeaverKills[ID] ) + "] assist kills." );
                                        m_Players[ID]->SetAssists( ValueInt - m_AssistsOnLeaverKills[ID] );
                                    }
                                    else
                                        m_Players[ID]->SetAssists( ValueInt );
                                    if( ValueInt > 500 )
                                    {
                                        m_Players[ID]->SetAssists( 0 );
                                        Log->Debug("[INFO] Noted an invalid statsrecord on playercolour ["+UTIL_ToString(ID)+"]. Set the assists to [0]" );
                                    }
                                }
                                else if( KeyString == "6" )
                                    m_Players[ID]->SetGold( ValueInt );
                                else if( KeyString == "7" )
                                {
                                    m_Players[ID]->SetNeutralKills( ValueInt );
                                    if( ValueInt > 5000 )
                                    {
                                        m_Players[ID]->SetNeutralKills( 0 );
                                        Log->Debug("[INFO] Noted an invalid statsrecord on playercolour ["+UTIL_ToString(ID)+"]. Set the neutralkills to [0]" );
                                    }
                                }
                                else if( KeyString == "8_0" )
                                    m_Players[ID]->SetItem( 0, string( Value.rbegin( ), Value.rend( ) ) );
                                else if( KeyString == "8_1" )
                                    m_Players[ID]->SetItem( 1, string( Value.rbegin( ), Value.rend( ) ) );
                                else if( KeyString == "8_2" )
                                    m_Players[ID]->SetItem( 2, string( Value.rbegin( ), Value.rend( ) ) );
                                else if( KeyString == "8_3" )
                                    m_Players[ID]->SetItem( 3, string( Value.rbegin( ), Value.rend( ) ) );
                                else if( KeyString == "8_4" )
                                    m_Players[ID]->SetItem( 4, string( Value.rbegin( ), Value.rend( ) ) );
                                else if( KeyString == "8_5" )
                                    m_Players[ID]->SetItem( 5, string( Value.rbegin( ), Value.rend( ) ) );
                                else if( KeyString == "9" ) {
                                    m_Players[ID]->SetHero( string( Value.rbegin( ), Value.rend( ) ) );
                                    CGamePlayer *Player = m_Game->GetPlayerFromColour( ID );
                                    if( Player )
                                        m_Game->AppendLogData("4\thp\t" + Player->GetName( ) + "\t-\t" + string( Value.rbegin( ), Value.rend( ) ) + "\t-\t" + MinString + ":" + SecString + "\t-\n");
                                }
                                else if( KeyString == "id" )
                                {
                                    // DotA sends id values from 1-10 with 1-5 being sentinel players and 6-10 being scourge players
                                    // unfortunately the actual player colours are from 1-5 and from 7-11 so we need to deal with this case here

                                    if( ValueInt >= 6 )
                                        m_Players[ID]->SetNewColour( ValueInt + 1 );
                                    else
                                        m_Players[ID]->SetNewColour( ValueInt );
                                }
                            }
                        }

                        if( !m_Game->m_PlayerUpdate && m_Game->m_LogData != "" && m_Game->m_OHBot->m_LiveGames )
                        {
                            unsigned int Players = 0;
                            for( unsigned int i = 0; i < 12; ++i )
                            {
                                if( m_Players[i] )
                                {
                                    CGamePlayer *Player = m_Game->GetPlayerFromColour( m_Players[i]->GetNewColour( ) );
                                    if( Player)
                                        m_Game->AppendLogData("6\t" + UTIL_ToString( m_Players[i]->GetNewColour( ) ) + "\t" + m_Players[i]->GetHero( ) + "\t" + Player->GetName( ) + "\t" + UTIL_ToString( m_Players[i]->GetLevel( ) ) + "\t" + UTIL_ToString( m_Players[i]->GetKills( ) ) + "/" + UTIL_ToString( m_Players[i]->GetDeaths( ) ) + "/" + UTIL_ToString( m_Players[i]->GetAssists( ) ) + "\t" + UTIL_ToString( m_Players[i]->GetTowerKills( ) ) + "/" + UTIL_ToString( m_Players[i]->GetRaxKills( ) ) + "\n");
                                    else
                                        m_Game->AppendLogData("6\t-\n");

                                    ++Players;
                                }
                            }
                            m_Game->m_PlayerUpdate = true;
                        }

                        i += 12 + Data.size( ) + Key.size( );
                    }
                    else
                        ++i;
                }
                else
                    ++i;
            }
            else
                ++i;
        }
        else
            ++i;
    }

    // set winner if any win conditions have been met
    // that is actually from ENT bot sources and not complete binded into this bot
    // Todo: Add conditions option into the header
    if( m_Winner == 0 )
    {
        if( m_KillLimit != 0 )
        {
            if( m_SentinelKills >= m_KillLimit )
            {
                m_Winner = 1;
                return true;
            }
            else if( m_ScourgeKills >= m_KillLimit )
            {
                m_Winner = 2;
                return true;
            }
        }

        if( m_TowerLimit != 0)
        {
            if( m_SentinelTowers >= m_TowerLimit )
            {
                m_Winner = 1;
                return true;
            }
            else if( m_ScourgeTowers >= m_TowerLimit )
            {
                m_Winner = 2;
                return true;
            }
        }

        if( m_TimeLimit != 0 && m_Game->GetGameTicks( ) > m_TimeLimit * 1000 )
        {
            // we must determine a winner at this point
            // or at least we must try...!

            if( m_SentinelKills > m_ScourgeKills )
            {
                m_Winner = 1;
                return true;
            }
            else if( m_SentinelKills < m_ScourgeKills )
            {
                m_Winner = 2;
                return true;
            }

            // ok, base on creep kills then?
            uint32_t SentinelTotal = 0;
            uint32_t ScourgeTotal = 0;

            for( unsigned int i = 0; i < 12; ++i )
            {
                if( m_Players[i] )
                {
                    uint32_t Colour = i;

                    if( m_Players[i]->GetNewColour( ) != 0 )
                        Colour = m_Players[i]->GetNewColour( );

                    if( Colour >= 1 && Colour <= 5 )
                        SentinelTotal += m_Players[i]->GetCreepKills( ) + m_Players[i]->GetCreepDenies( );
                    if( Colour >= 7 && Colour <= 11 )
                        ScourgeTotal += m_Players[i]->GetCreepKills( ) + m_Players[i]->GetCreepDenies( );
                }
            }

            if( SentinelTotal > ScourgeTotal )
            {
                m_Winner = 1;
                return true;
            }
            else if( SentinelTotal < ScourgeTotal )
            {
                m_Winner = 2;
                return true;
            }
        }
    }

    return m_Winner != 0;
}

void CStatsDOTA :: Save( COHBot *GHost, COHBotDB *DB, uint32_t GameID )
{

    if( DB->Begin( ) )
    {
        // since we only record the end game information it's possible we haven't recorded anything yet if the game didn't end with a tree/throne death
        // this will happen if all the players leave before properly finishing the game
        // the dotagame stats are always saved (with winner = 0 if the game didn't properly finish)
        // the dotaplayer stats are only saved if the game is properly finished

        unsigned int Players = 0;

        // save the dotagame

        GHost->m_Callables.push_back( DB->ThreadedDotAGameAdd( GameID, m_Winner, m_Min, m_Sec ) );

        // check for invalid colours and duplicates
        // this can only happen if DotA sends us garbage in the "id" value but we should check anyway

        for( unsigned int i = 0; i < 12; ++i )
        {
            if( m_Players[i] )
            {
                uint32_t Colour = m_Players[i]->GetNewColour( );

                if( !( ( Colour >= 1 && Colour <= 5 ) || ( Colour >= 7 && Colour <= 11 ) ) )
                {
                    //Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] discarding player data, invalid colour found" );
                    DB->Commit( );
                    return;
                }

                for( unsigned int j = i + 1; j < 12; ++j )
                {
                    if( m_Players[j] && Colour == m_Players[j]->GetNewColour( ) )
                    {
                        //Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] discarding player data, duplicate colour found" );
                        DB->Commit( );
                        return;
                    }
                }
            }
        }

	      string data = "";

        for( unsigned int i = 0; i < 12; ++i )
        {
            if( m_Players[i] )
            {
                if(m_Players[i]->GetItem( 0 ) != "" || m_Players[i]->GetItem( 1 ) != "" || m_Players[i]->GetItem( 2 ) != "" || m_Players[i]->GetItem( 3 ) != "" || m_Players[i]->GetItem( 4 ) != "" || m_Players[i]->GetItem( 5 ) != "")
		               data += "( " + UTIL_ToString( m_Game->m_OHBot->m_BotID ) + ", " + UTIL_ToString( GameID ) + ", " + UTIL_ToString( m_Players[i]->GetColour( ) ) + ", " + UTIL_ToString( m_Players[i]->GetKills( ) ) + ", " + UTIL_ToString(  m_Players[i]->GetDeaths( ) ) + ", " + UTIL_ToString( m_Players[i]->GetCreepKills( ) ) + ", " + UTIL_ToString( m_Players[i]->GetCreepDenies( ) ) + ", " + UTIL_ToString( m_Players[i]->GetAssists( ) ) + ", " + UTIL_ToString( m_Players[i]->GetGold( ) ) + ", " + UTIL_ToString(  m_Players[i]->GetNeutralKills( ) ) + ", '" + m_Players[i]->GetItem( 0 ) + "', '" + m_Players[i]->GetItem( 1 ) + "', '" + m_Players[i]->GetItem( 2 ) + "', '" + m_Players[i]->GetItem( 3 ) + "', '" + m_Players[i]->GetItem( 4 ) + "', '" + m_Players[i]->GetItem( 5 ) + "', '" + m_Players[i]->GetSpell( 0 ) + "', '" + m_Players[i]->GetSpell( 1 ) + "', '" + m_Players[i]->GetSpell( 2 ) + "', '" + m_Players[i]->GetSpell( 3 ) + "', '" + m_Players[i]->GetSpell( 4 ) + "', '" + m_Players[i]->GetSpell( 5 ) + "', '" + m_Players[i]->GetHero( ) + "', " + UTIL_ToString( m_Players[i]->GetNewColour( ) ) + ", " + UTIL_ToString( m_Players[i]->GetTowerKills( ) ) + ", " + UTIL_ToString( m_Players[i]->GetRaxKills( ) ) + ", " + UTIL_ToString( m_Players[i]->GetCourierKills( ) ) + ", " + UTIL_ToString( m_Players[i]->GetLevel( ) ) + " )";
                else
			             data += "( " + UTIL_ToString( m_Game->m_OHBot->m_BotID ) + ", " + UTIL_ToString( GameID ) + ", " + UTIL_ToString( m_Players[i]->GetColour( ) ) + ", " + UTIL_ToString( m_Players[i]->GetKills( ) ) + ", " + UTIL_ToString(  m_Players[i]->GetDeaths( ) ) + ", " + UTIL_ToString( m_Players[i]->GetCreepKills( ) ) + ", " + UTIL_ToString( m_Players[i]->GetCreepDenies( ) ) + ", " + UTIL_ToString( m_Players[i]->GetAssists( ) ) + ", " + UTIL_ToString( m_Players[i]->GetGold( ) ) + ", " + UTIL_ToString(  m_Players[i]->GetNeutralKills( ) ) + ", '" + m_BufferedItemOne[i] + "', '" +  m_BufferedItemTwo[i] + "', '" + m_BufferedItemThree[i] + "', '" + m_BufferedItemFour[i] + "', '" + m_BufferedItemFive[i] + "', '" + m_BufferedItemSix[i] + "', '" + m_Players[i]->GetSpell( 0 ) + "', '" + m_Players[i]->GetSpell( 1 ) + "', '" + m_Players[i]->GetSpell( 2 ) + "', '" + m_Players[i]->GetSpell( 3 ) + "', '" + m_Players[i]->GetSpell( 4 ) + "', '" + m_Players[i]->GetSpell( 5 ) + "', '" + m_Players[i]->GetHero( ) + "', " + UTIL_ToString( m_Players[i]->GetNewColour( ) ) + ", " + UTIL_ToString( m_Players[i]->GetTowerKills( ) ) + ", " + UTIL_ToString( m_Players[i]->GetRaxKills( ) ) + ", " + UTIL_ToString( m_Players[i]->GetCourierKills( ) ) + ", " + UTIL_ToString( m_Players[i]->GetLevel( ) ) + " )";

            		if(i!=11)
            			data += ", ";
                ++Players;
            }
        }

	      GHost->m_Callables.push_back( DB->ThreadedDotAPlayerAdd( GameID, data ) );

        if( DB->Commit( ) )
            Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] saving " + UTIL_ToString( Players ) + " players" );
        else
            Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] unable to commit database transaction, data not saved" );
    }
    else
        Log->Debug( "[STATS"+TypePrefix+": " + m_Game->GetGameName( ) + "] unable to begin database transaction, data not saved" );
}
