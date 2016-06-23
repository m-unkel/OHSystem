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
#include "../language.h"
#include "../network/socket.h"
#include "../database/ghostdb.h"
#include "../bnet/bnet.h"
#include "../map.h"
#include "../replay/packed.h"
#include "../replay/savegame.h"
#include "gameplayer.h"
#include "gameprotocol.h"
#include "game_base.h"
#include "game.h"
#include "../stats/stats.h"
#include "../stats/statsdota.h"
#include "../stats/statsw3mmd.h"

#include <stdio.h>
#include <cmath>
#include <string.h>
#include <time.h>
#include <boost/thread.hpp>

//
// sorting classes
//

class CGamePlayerSortAscByPing {
public:
    bool operator( )(CGamePlayer *Player1, CGamePlayer *Player2) const {
        return Player1->GetPing(false) < Player2->GetPing(false);
    }
};

class CGamePlayerSortDescByPing {
public:
    bool operator( )(CGamePlayer *Player1, CGamePlayer *Player2) const {
        return Player1->GetPing(false) > Player2->GetPing(false);
    }
};

//
// CGame
//

CGame::CGame(COHBot *nOHBot, CMap *nMap, CSaveGame *nSaveGame, uint16_t nHostPort, unsigned char nGameState,
             string nGameName, string nOwnerName, string nCreatorName, string nCreatorServer, uint32_t nGameType,
             uint32_t nHostCounter) : CBaseGame(nOHBot, nMap, nSaveGame, nHostPort, nGameState, nGameName, nOwnerName,
                                                nCreatorName, nCreatorServer, nGameType, nHostCounter),
                                      m_DBBanLast(NULL), m_Stats(NULL), m_CallableGameAdd(NULL), m_ForfeitTime(0),
                                      m_ForfeitTeam(0), m_EarlyDraw(false), m_BalanceVotes(0) {
    m_DBGame = new CDBGame(0, string(), m_Map->GetMapPath(), string(), string(), string(), 0);
    m_GameAlias = m_Map->GetAlias();
    m_GameAliasName = m_OHBot->GetAliasName(m_GameAlias);
    m_lGameAliasName = m_GameAliasName;
    transform(m_lGameAliasName.begin(), m_lGameAliasName.end(), m_lGameAliasName.begin(), ::tolower);
    if (m_lGameAliasName.find("lod") != string::npos || m_lGameAliasName.find("dota") != string::npos ||
        m_lGameAliasName.find("imba") != string::npos) {
        m_Stats = new CStatsDOTA(this);
    } else if (m_Map->GetAlias() != 0) {
        m_Stats = new CStatsW3MMD(this, m_Map->GetMapStatsW3MMDCategory());
    } else {
        Log->Warning(m_OHBot->m_Language->NoMapAliasRecordFound());
    }

    if (m_OHBot->m_VoteMode && !m_Map->GetPossibleModesToVote().empty())
        GetVotingModes(m_Map->GetPossibleModesToVote());

    m_LobbyLog.clear();
    m_GameLog.clear();
    m_ObservingPlayers = 0;
    m_LastLeaverTime = GetTime();
}

CGame::~CGame() {
    boost::mutex::scoped_lock callablesLock(m_OHBot->m_CallablesMutex);

    // autoban
    uint32_t EndTime = m_GameTicks / 1000;
    uint32_t Counter = 0;
    for (vector<CDBGamePlayer *>::iterator i = m_DBGamePlayers.begin(); i != m_DBGamePlayers.end(); ++i) {
        if (IsAutoBanned((*i)->GetName())) {
            uint32_t VictimLevel = 0;
            for (vector<CBNET *>::iterator k = m_OHBot->m_BNETs.begin(); k != m_OHBot->m_BNETs.end(); ++k) {
                if ((*k)->GetServer() == (*i)->GetSpoofedRealm() ||
                    (*i)->GetSpoofedRealm() == m_OHBot->m_WC3ConnectAlias) {
                    VictimLevel = (*k)->IsLevel((*i)->GetName());
                    break;
                }
            }

            uint32_t LeftTime = (*i)->GetLeft();
            // make sure that draw games will not ban people who didnt leave.
            if (EndTime - LeftTime > 300 || m_EarlyDraw && LeftTime != EndTime) {
                if (m_EarlyDraw)
                    Counter++;
                if (Counter <= 2 && VictimLevel <= 2) {
                    string Reason = m_OHBot->m_Language->DisconnectedAt() + " ";
                    if ((*i)->GetLeftReason().find("left") != string::npos) {
                        Reason = m_OHBot->m_Language->LeftAt() + " ";
                    }
                    if (EndTime < 300) {
                        Reason += UTIL_ToString(LeftTime / 60) + "/" + UTIL_ToString(EndTime / 60) + "min";
                    } else {
                        string EndMin = UTIL_ToString(EndTime / 60);
                        string EndSec = UTIL_ToString(EndTime % 60);
                        string LeftMin = UTIL_ToString(LeftTime / 60);
                        string LeftSec = UTIL_ToString(LeftTime % 60);
                        if (1 == EndSec.size())
                            EndSec = "0" + EndSec;
                        if (1 == LeftSec.size())
                            LeftSec = "0" + LeftSec;

                        Reason += LeftMin + ":" + LeftSec + "/" + EndMin + ":" + EndSec;
                    }
                    BanPlayerByPenality((*i)->GetName(), (*i)->GetIP(), m_OHBot->m_BotManagerName,
                                        (*i)->GetLeaverLevel(), Reason);
                }
            }
        }
    }

    /* last update before the game is over */
    if (m_LogData != "") {
        AppendLogData("1\tpl");
        //UPDATE SLOTS
        for (unsigned char i = 0; i < m_Slots.size(); ++i) {
            if (m_Slots[i].GetSlotStatus() == SLOTSTATUS_OCCUPIED && m_Slots[i].GetComputer() == 0) {
                CGamePlayer *player = GetPlayerFromSID(i);
                if (player)
                    AppendLogData("\t" + player->GetName());
                else if (!player && m_GameLoaded)
                    AppendLogData("\t-");
            }
            else if (m_Slots[i].GetSlotStatus() == SLOTSTATUS_OPEN)
                AppendLogData("\t-");
        }
        AppendLogData("\n");
        m_PairedLogUpdates.push_back(
                PairedLogUpdate(string(), m_OHBot->m_DB->ThreadedStoreLog(m_HostCounter, m_LogData, m_AdminLog)));
        m_LogData = string();
        m_AdminLog = vector<string>();
        m_PlayerUpdate = false;
        m_LastLogDataUpdate = GetTime();
    }

    if (m_CallableGameAdd && m_CallableGameAdd->GetReady()) {
        if (m_CallableGameAdd->GetResult() > 0) {
            Log->Info("[GAME: " + m_GameName + "] saving player/stats data to database");

            // store the CDBGamePlayers in the database

            for (vector<CDBGamePlayer *>::iterator i = m_DBGamePlayers.begin(); i != m_DBGamePlayers.end(); ++i)
                m_OHBot->m_Callables.push_back(
                        m_OHBot->m_DB->ThreadedGamePlayerAdd(m_CallableGameAdd->GetResult(), (*i)->GetName(),
                                                             (*i)->GetIP(), (*i)->GetSpoofed(), (*i)->GetSpoofedRealm(),
                                                             (*i)->GetReserved(), (*i)->GetLoadingTime(),
                                                             (*i)->GetLeft(), (*i)->GetLeftReason(), (*i)->GetTeam(),
                                                             (*i)->GetColour(), (*i)->GetID()));

            if (m_Stats) {
                m_Stats->Save(m_OHBot, m_OHBot->m_DB, m_CallableGameAdd->GetResult());
            }
        }
        else
            Log->Write("[GAME: " + m_GameName + "] unable to save player/stats data to database");

        m_OHBot->m_DB->RecoverCallable(m_CallableGameAdd);
        delete m_CallableGameAdd;
        m_CallableGameAdd = NULL;
    }

    for (vector<PairedPUp>::iterator i = m_PairedPUps.begin(); i != m_PairedPUps.end(); ++i)
        m_OHBot->m_Callables.push_back(i->second);

    for (vector<PairedBanCheck>::iterator i = m_PairedBanChecks.begin(); i != m_PairedBanChecks.end(); ++i)
        m_OHBot->m_Callables.push_back(i->second);

    for (vector<Pairedpm>::iterator i = m_Pairedpms.begin(); i != m_Pairedpms.end(); ++i)
        m_OHBot->m_Callables.push_back(i->second);

    for (vector<PairedGSCheck>::iterator i = m_PairedGSChecks.begin(); i != m_PairedGSChecks.end(); ++i)
        m_OHBot->m_Callables.push_back(i->second);

    for (vector<PairedRankCheck>::iterator i = m_PairedRankChecks.begin(); i != m_PairedRankChecks.end(); ++i)
        m_OHBot->m_Callables.push_back(i->second);

    for (vector<PairedStreakCheck>::iterator i = m_PairedStreakChecks.begin(); i != m_PairedStreakChecks.end(); ++i)
        m_OHBot->m_Callables.push_back(i->second);

    for (vector<PairedSCheck>::iterator i = m_PairedSChecks.begin(); i != m_PairedSChecks.end(); ++i)
        m_OHBot->m_Callables.push_back(i->second);

    for (vector<PairedPWCheck>::iterator i = m_PairedPWChecks.begin(); i != m_PairedPWChecks.end(); ++i)
        m_OHBot->m_Callables.push_back(i->second);

    for (vector<PairedPassCheck>::iterator i = m_PairedPassChecks.begin(); i != m_PairedPassChecks.end(); ++i)
        m_OHBot->m_Callables.push_back(i->second);

    for (vector<PairedRegAdd>::iterator i = m_PairedRegAdds.begin(); i != m_PairedRegAdds.end(); ++i)
        m_OHBot->m_Callables.push_back(i->second);

    callablesLock.unlock();

    for (vector<CDBBan *>::iterator i = m_DBBans.begin(); i != m_DBBans.end(); ++i)
        delete *i;

    delete m_DBGame;

    for (vector<CDBGamePlayer *>::iterator i = m_DBGamePlayers.begin(); i != m_DBGamePlayers.end(); ++i)
        delete *i;

    delete m_Stats;

    // it's a "bad thing" if m_CallableGameAdd is non NULL here
    // it means the game is being deleted after m_CallableGameAdd was created (the first step to saving the game data) but before the associated thread terminated
    // rather than failing horribly we choose to allow the thread to complete in the orphaned callables list but step 2 will never be completed
    // so this will create a game entry in the database without any gameplayers and/or DotA stats

    if (m_CallableGameAdd) {
        Log->Write("[GAME: " + m_GameName + "] game is being deleted before all game data was saved, game data has been lost");
        boost::mutex::scoped_lock lock(m_OHBot->m_CallablesMutex);
        m_OHBot->m_Callables.push_back(m_CallableGameAdd);
        lock.unlock();
    }
}

bool CGame::Update(void *fd, void *send_fd) {
    // update callables
    for (vector<PairedPUp>::iterator i = m_PairedPUps.begin(); i != m_PairedPUps.end();) {
        if (i->second->GetReady()) {
            bool Result = i->second->GetResult();
            if (Result)
                SendAllChat(m_OHBot->m_Language->ChangedRankOfUser(i->second->GetName(),
                                                                   UTIL_ToString(i->second->GetLevel()),
                                                                   i->second->GetRealm()));
            else
                SendAllChat(m_OHBot->m_Language->NoRecordFoundForUser(i->second->GetName()));

            m_OHBot->m_DB->RecoverCallable(i->second);
            delete i->second;
            i = m_PairedPUps.erase(i);
        }
        else
            ++i;
    }
    for (vector<PairedPWCheck>::iterator i = m_PairedPWChecks.begin(); i != m_PairedPWChecks.end();) {
        if (i->second->GetReady()) {
            uint32_t Result = i->second->GetResult();
            CGamePlayer *Player = GetPlayerFromName(i->first, true);
            if (Player) {
                if (Result == 2 && !m_OHBot->PlayerCached(Player->GetName())) {
                    SendChat(Player, m_OHBot->m_Language->PassProtAcc());
                    Player->SetPasswordProt(true);
                    Player->SetRegistered(true);
                } else if (Result == 2 && m_OHBot->PlayerCached(Player->GetName())) {
                    SendChat(Player, m_OHBot->m_Language->SuccessfullyUsedCachedPass());
                    Player->SetRegistered(true);
                } else if (Result == 1) {
                    Player->SetRegistered(true);
                }
            }

            m_OHBot->m_DB->RecoverCallable(i->second);
            delete i->second;
            i = m_PairedPWChecks.erase(i);
        }
        else
            ++i;
    }

    for (vector<PairedPassCheck>::iterator i = m_PairedPassChecks.begin(); i != m_PairedPassChecks.end();) {
        if (i->second->GetReady()) {
            uint32_t Result = i->second->GetResult();
            CGamePlayer *Player = GetPlayerFromName(i->first, true);
            if (Player) {
                if (Result == 1) {
                    cachedPlayer p;
                    p.name = Player->GetName();
                    p.time = GetTime();
                    SendChat(Player, m_OHBot->m_Language->SuccessfullyTypedPassword(Player->GetName()));
                    m_OHBot->m_PlayerCache.push_back(p);
                    Player->SetPasswordProt(false);
                    Player->SetSpoofed(true);
                }
                else if (Result == 2)
                    SendChat(Player, m_OHBot->m_Language->WrongPassword());
                else if (Result == 3)
                    SendChat(Player, m_OHBot->m_Language->NoPassProtAcc());
                else if (Result == 4)
                    SendChat(Player, m_OHBot->m_Language->RemovedPassProtAcc());
                else
                    SendAllChat(m_OHBot->m_Language->WrongContactBotOwner());
            }

            m_OHBot->m_DB->RecoverCallable(i->second);
            delete i->second;
            i = m_PairedPassChecks.erase(i);
        }
        else
            ++i;
    }

    for (vector<PairedBanCheck>::iterator i = m_PairedBanChecks.begin(); i != m_PairedBanChecks.end();) {
        if (i->second->GetReady()) {
            CDBBan *Ban = i->second->GetResult();
            m_OHBot->m_DB->RecoverCallable(i->second);
            delete i->second;
            i = m_PairedBanChecks.erase(i);
        }
        else
            ++i;
    }

    for (vector<Pairedpm>::iterator i = m_Pairedpms.begin(); i != m_Pairedpms.end();) {
        if (i->second->GetReady()) {
            uint32_t Result = i->second->GetResult();
            CGamePlayer *Player = GetPlayerFromName(i->first, true);
            if (Player) {
                if (Result == -1)
                    SendChat(Player, m_OHBot->m_Language->SuccessfullyStoredMessage());
                else if (Result > 0)
                    SendChat(Player, m_OHBot->m_Language->NewMessages(Player->GetName(), UTIL_ToString(Result)));
            }

            m_OHBot->m_DB->RecoverCallable(i->second);
            delete i->second;
            i = m_Pairedpms.erase(i);
        }
        else
            ++i;
    }

    for (vector<PairedGSCheck>::iterator i = m_PairedGSChecks.begin(); i != m_PairedGSChecks.end();) {
        if (i->second->GetReady()) {
            CDBStatsPlayerSummary *StatsPlayerSummary = i->second->GetResult();

            string Month = i->second->GetMonth();
            string Year = i->second->GetYear();
            if (Month.empty())
                Month = m_OHBot->GetTimeFunction(1);
            if (Year.empty())
                Year = m_OHBot->GetTimeFunction(0);
            if (StatsPlayerSummary) {
                if (!StatsPlayerSummary->GetHidden()) {
                    if (i->first.empty()) {
                        string Streak = UTIL_ToString(StatsPlayerSummary->GetStreak());
                        if (StatsPlayerSummary->GetStreak() < 0)
                            Streak = "-" + Streak;

                        SendAllChat(m_OHBot->m_Language->HasPlayedGamesWithThisBot(StatsPlayerSummary->GetPlayer(),
                                                                                   UTIL_ToString(
                                                                                           StatsPlayerSummary->GetScore(),
                                                                                           0),
                                                                                   UTIL_ToString(
                                                                                           StatsPlayerSummary->GetGames()),
                                                                                   UTIL_ToString(
                                                                                           StatsPlayerSummary->GetWinPerc(),
                                                                                           2),
                                                                                   Streak,
                                                                                   m_OHBot->m_Language->GetMonthInWords(Month),
                                                                                   Year
                        ));
                    }
                    else {
                        CGamePlayer *Player = GetPlayerFromName(i->first, true);

                        if (Player) {
                            uint32_t sn = StatsPlayerSummary->GetStreak();
                            string Streak = "";
                            if (sn < 0)
                                string Streak = "-" + UTIL_ToString(sn);
                            else
                                string Streak = UTIL_ToString(sn);

                            SendChat(Player, m_OHBot->m_Language->HasPlayedGamesWithThisBot(Player->GetName(),
                                                                                            UTIL_ToString(
                                                                                                    StatsPlayerSummary->GetScore(),
                                                                                                    0),
                                                                                            UTIL_ToString(
                                                                                                    StatsPlayerSummary->GetGames()),
                                                                                            UTIL_ToString(
                                                                                                    StatsPlayerSummary->GetWinPerc(),
                                                                                                    2),
                                                                                            Streak,
                                                                                            m_OHBot->m_Language->GetMonthInWords(
                                                                                                    Month),
                                                                                            Year
                            ));
                        }
                    }
                }
                else {
                    CGamePlayer *Player = GetPlayerFromName(i->first, true);

                    if (Player) {
                        if (Player->GetName() != StatsPlayerSummary->GetPlayer())
                            SendChat(Player, m_OHBot->m_Language->UserHasAHiddenAcc(Player->GetName()));
                        else {
                            uint32_t sn = StatsPlayerSummary->GetStreak();
                            string Streak = "";
                            if (sn < 0)
                                string Streak = "-" + UTIL_ToString(sn);
                            else
                                string Streak = UTIL_ToString(sn);

                            SendChat(Player, m_OHBot->m_Language->HasPlayedGamesWithThisBot(Player->GetName(),
                                                                                            UTIL_ToString(
                                                                                                    StatsPlayerSummary->GetScore(),
                                                                                                    0),
                                                                                            UTIL_ToString(
                                                                                                    StatsPlayerSummary->GetGames()),
                                                                                            UTIL_ToString(
                                                                                                    StatsPlayerSummary->GetWinPerc(),
                                                                                                    2),
                                                                                            Streak,
                                                                                            m_OHBot->m_Language->GetMonthInWords(
                                                                                                    Month),
                                                                                            Year
                            ));
                        }
                    }
                }

            }
            else {
                if (i->first.empty())
                    SendAllChat(m_OHBot->m_Language->HasntPlayedAliasGamesWithThisBot(i->second->GetName(),
                                                                                      m_OHBot->m_Language->GetMonthInWords(Month),
                                                                                      Year,
                                                                                      m_OHBot->GetAliasName(
                                                                                              i->second->GetAlias())
                    ));
                else {
                    CGamePlayer *Player = GetPlayerFromName(i->first, true);

                    if (Player)
                        SendAllChat(m_OHBot->m_Language->HasntPlayedAliasGamesWithThisBot(Player->GetName(),
                                                                                          m_OHBot->m_Language->GetMonthInWords(
                                                                                                  Month),
                                                                                          Year,
                                                                                          m_OHBot->GetAliasName(
                                                                                                  i->second->GetAlias())
                        ));
                }
            }

            m_OHBot->m_DB->RecoverCallable(i->second);
            delete i->second;
            i = m_PairedGSChecks.erase(i);
        }
        else
            ++i;
    }

    for (vector<PairedRankCheck>::iterator i = m_PairedRankChecks.begin(); i != m_PairedRankChecks.end();) {
        if (i->second->GetReady()) {
            CDBStatsPlayerSummary *StatsPlayerSummary = i->second->GetResult();
            string Month = i->second->GetMonth();
            string Year = i->second->GetYear();
            if (Month.empty())
                Month = m_OHBot->GetTimeFunction(1);
            if (Year.empty())
                Year = m_OHBot->GetTimeFunction(0);
            if (StatsPlayerSummary) {
                string Time = Month + "/" + Year;
                if (!StatsPlayerSummary->GetHidden()) {
                    if (i->first.empty()) {
                        uint32_t Level = 0;
                        string LevelName;
                        for (vector<CBNET *>::iterator k = m_OHBot->m_BNETs.begin(); k != m_OHBot->m_BNETs.end(); ++k) {
                            if (((*k)->GetServer() == StatsPlayerSummary->GetRealm() ||
                                 (*k)->GetServer() == m_OHBot->m_WC3ConnectAlias) && m_OHBot->m_RanksLoaded) {
                                Level = (*k)->IsLevel(i->second->GetName());
                                LevelName = (*k)->GetLevelName(Level);
                                break;
                            }
                        }
                        if (m_OHBot->m_RanksLoaded)
                            SendAllChat(m_OHBot->m_Language->RankOfUser(StatsPlayerSummary->GetPlayer(), Time,
                                                                        m_OHBot->GetAliasName(i->second->GetAlias()),
                                                                        StatsPlayerSummary->GetRank(),
                                                                        UTIL_ToString(Level), LevelName));
                        else {
                            SendAllChat(
                                    m_OHBot->m_Language->RankOfUserWithoutLevel(StatsPlayerSummary->GetPlayer(), Time,
                                                                                m_OHBot->GetAliasName(
                                                                                        i->second->GetAlias()),
                                                                                StatsPlayerSummary->GetRank()));
                            Log->Write(m_OHBot->m_Language->RanksNotLoaded());
                        }
                    }
                    else {
                        CGamePlayer *Player = GetPlayerFromName(i->first, true);

                        if (Player) {
                            uint32_t Level = 0;
                            string LevelName;
                            for (vector<CBNET *>::iterator k = m_OHBot->m_BNETs.begin();
                                 k != m_OHBot->m_BNETs.end(); ++k) {
                                if (((*k)->GetServer() == Player->GetSpoofedRealm() ||
                                     Player->GetSpoofedRealm() == m_OHBot->m_WC3ConnectAlias) &&
                                    m_OHBot->m_RanksLoaded) {
                                    Level = (*k)->IsLevel(Player->GetName());
                                    LevelName = (*k)->GetLevelName(Level);
                                    break;
                                }
                            }
                            if (m_OHBot->m_RanksLoaded)
                                SendChat(Player, m_OHBot->m_Language->RankOfUser(StatsPlayerSummary->GetPlayer(), Time,
                                                                                 m_OHBot->GetAliasName(
                                                                                         i->second->GetAlias()),
                                                                                 StatsPlayerSummary->GetRank(),
                                                                                 UTIL_ToString(Level), LevelName));
                            else {
                                SendChat(Player,
                                         m_OHBot->m_Language->RankOfUserWithoutLevel(StatsPlayerSummary->GetPlayer(),
                                                                                     Time, m_OHBot->GetAliasName(
                                                         i->second->GetAlias()), StatsPlayerSummary->GetRank()));
                                Log->Write(m_OHBot->m_Language->RanksNotLoaded());
                            }
                        }
                    }
                } else {
                    CGamePlayer *Player = GetPlayerFromName(i->first, true);

                    if (Player) {
                        if (Player->GetName() != StatsPlayerSummary->GetPlayer())
                            SendChat(Player, m_OHBot->m_Language->UserHasAHiddenAcc(StatsPlayerSummary->GetPlayer()));
                        else {
                            uint32_t Level = 0;
                            string LevelName;
                            for (vector<CBNET *>::iterator k = m_OHBot->m_BNETs.begin();
                                 k != m_OHBot->m_BNETs.end(); ++k) {
                                if (((*k)->GetServer() == Player->GetSpoofedRealm() ||
                                     Player->GetSpoofedRealm() == m_OHBot->m_WC3ConnectAlias) &&
                                    m_OHBot->m_RanksLoaded) {
                                    Level = (*k)->IsLevel(Player->GetName());
                                    LevelName = (*k)->GetLevelName(Level);
                                    break;
                                }
                            }
                            if (m_OHBot->m_RanksLoaded)
                                SendChat(Player, m_OHBot->m_Language->RankOfUser(StatsPlayerSummary->GetPlayer(), Time,
                                                                                 m_OHBot->GetAliasName(
                                                                                         i->second->GetAlias()),
                                                                                 StatsPlayerSummary->GetRank(),
                                                                                 UTIL_ToString(Level), LevelName));
                            else {
                                SendChat(Player,
                                         m_OHBot->m_Language->RankOfUserWithoutLevel(StatsPlayerSummary->GetPlayer(),
                                                                                     Time, m_OHBot->GetAliasName(
                                                         i->second->GetAlias()), StatsPlayerSummary->GetRank()));
                                Log->Write(m_OHBot->m_Language->RanksNotLoaded());
                            }
                        }
                    }
                }
            }
            else {
                if (i->first.empty())
                    SendAllChat(m_OHBot->m_Language->HasntPlayedAliasGamesWithThisBot(i->second->GetName(),
                                                                                      m_OHBot->m_Language->GetMonthInWords(Month),
                                                                                      Year,
                                                                                      m_OHBot->GetAliasName(
                                                                                              i->second->GetAlias())
                    ));
                else {
                    CGamePlayer *Player = GetPlayerFromName(i->first, true);

                    if (Player)
                        SendAllChat(m_OHBot->m_Language->HasntPlayedAliasGamesWithThisBot(Player->GetName(),
                                                                                          m_OHBot->m_Language->GetMonthInWords(
                                                                                                  Month),
                                                                                          Year,
                                                                                          m_OHBot->GetAliasName(
                                                                                                  i->second->GetAlias())
                        ));
                }
            }

            m_OHBot->m_DB->RecoverCallable(i->second);
            delete i->second;
            i = m_PairedRankChecks.erase(i);
        }
        else
            ++i;
    }


    for (vector<PairedStreakCheck>::iterator i = m_PairedStreakChecks.begin(); i != m_PairedStreakChecks.end();) {
        if (i->second->GetReady()) {
            CDBStatsPlayerSummary *StatsPlayerSummary = i->second->GetResult();
            string Month = i->second->GetMonth();
            string Year = i->second->GetYear();
            if (Month.empty())
                Month = m_OHBot->GetTimeFunction(1);
            if (Year.empty())
                Year = m_OHBot->GetTimeFunction(0);
            if (StatsPlayerSummary) {
                string Time = Month + "/" + Year;
                if (!StatsPlayerSummary->GetHidden()) {
                    SendAllChat(m_OHBot->m_Language->StreakOfUser(StatsPlayerSummary->GetPlayer(), Time,
                                                                  m_OHBot->GetAliasName(i->second->GetAlias()),
                                                                  (StatsPlayerSummary->GetStreak() != 0 ? UTIL_ToString(
                                                                          StatsPlayerSummary->GetStreak())
                                                                                                        : UTIL_ToString(
                                                                                  StatsPlayerSummary->GetLosingStreak())),
                                                                  UTIL_ToString(StatsPlayerSummary->GetMaxStreak()),
                                                                  UTIL_ToString(
                                                                          StatsPlayerSummary->GetMaxLosingStreak())));
                } else {
                    CGamePlayer *Player = GetPlayerFromName(i->first, true);

                    if (Player) {
                        if (Player->GetName() != StatsPlayerSummary->GetPlayer())
                            SendChat(Player, m_OHBot->m_Language->UserHasAHiddenAcc(StatsPlayerSummary->GetPlayer()));
                        else
                            SendAllChat(m_OHBot->m_Language->StreakOfUser(StatsPlayerSummary->GetPlayer(), Time,
                                                                          m_OHBot->GetAliasName(i->second->GetAlias()),
                                                                          (StatsPlayerSummary->GetStreak() != 0
                                                                           ? UTIL_ToString(
                                                                                          StatsPlayerSummary->GetStreak())
                                                                           : UTIL_ToString(
                                                                                          StatsPlayerSummary->GetLosingStreak())),
                                                                          UTIL_ToString(
                                                                                  StatsPlayerSummary->GetMaxStreak()),
                                                                          UTIL_ToString(
                                                                                  StatsPlayerSummary->GetMaxLosingStreak())));
                    }
                }
            }
            else
                SendAllChat(m_OHBot->m_Language->HasntPlayedAliasGamesWithThisBot(i->second->GetName(),
                                                                                  m_OHBot->m_Language->GetMonthInWords(Month),
                                                                                  Year,
                                                                                  m_OHBot->GetAliasName(
                                                                                          i->second->GetAlias())
                ));

            m_OHBot->m_DB->RecoverCallable(i->second);
            delete i->second;
            i = m_PairedStreakChecks.erase(i);
        }
        else
            ++i;
    }

    for (vector<PairedSCheck>::iterator i = m_PairedSChecks.begin(); i != m_PairedSChecks.end();) {
        if (i->second->GetReady()) {
            CDBStatsPlayerSummary *StatsPlayerSummary = i->second->GetResult();
            string Month = i->second->GetMonth();
            string Year = i->second->GetYear();
            if (Month.empty())
                Month = m_OHBot->GetTimeFunction(1);
            if (Year.empty())
                Year = m_OHBot->GetTimeFunction(0);
            if (StatsPlayerSummary) {
                string Alias = m_OHBot->GetAliasName(i->second->GetAlias());
                transform(Alias.begin(), Alias.end(), Alias.begin(), ::tolower);
                string Summary = "";
                if (Alias.find("lod") != string::npos || Alias.find("dota") != string::npos ||
                    Alias.find("imba") != string::npos) {
                    Summary = m_OHBot->m_Language->HasPlayedAliasGamesWithThisBot(
                            UTIL_ToString(StatsPlayerSummary->GetGames()),
                            UTIL_ToString(StatsPlayerSummary->GetWins()),
                            UTIL_ToString(StatsPlayerSummary->GetLosses()),
                            UTIL_ToString(StatsPlayerSummary->GetDraw()),
                            UTIL_ToString(StatsPlayerSummary->GetKills()),
                            UTIL_ToString(StatsPlayerSummary->GetDeaths()),
                            UTIL_ToString(StatsPlayerSummary->GetCreeps()),
                            UTIL_ToString(StatsPlayerSummary->GetDenies()),
                            UTIL_ToString(StatsPlayerSummary->GetAssists()),
                            UTIL_ToString(StatsPlayerSummary->GetNeutrals()),
                            UTIL_ToString(StatsPlayerSummary->GetTowers()),
                            UTIL_ToString(StatsPlayerSummary->GetRax()),
                            UTIL_ToString(StatsPlayerSummary->GetAvgKills(), 1),
                            UTIL_ToString(StatsPlayerSummary->GetAvgDeaths(), 1),
                            UTIL_ToString(StatsPlayerSummary->GetAvgCreeps(), 1),
                            UTIL_ToString(StatsPlayerSummary->GetAvgDenies(), 1),
                            UTIL_ToString(StatsPlayerSummary->GetAvgAssists(), 1),
                            UTIL_ToString(StatsPlayerSummary->GetAvgNeutrals(), 1),
                            UTIL_ToString(StatsPlayerSummary->GetAvgTowers(), 1),
                            UTIL_ToString(StatsPlayerSummary->GetAvgRax(), 1)
                    );
                } else if (Alias.find("legion") != string::npos) {
                    /**
                     * Legion TD template
                     */
                    Summary = "G: " + UTIL_ToString(StatsPlayerSummary->GetGames()) + " Score: " +
                              UTIL_ToString(StatsPlayerSummary->GetScore(), 0) + " W/L/D: " +
                              UTIL_ToString(StatsPlayerSummary->GetWins()) + "/" +
                              UTIL_ToString(StatsPlayerSummary->GetLosses()) + "/" +
                              UTIL_ToString(StatsPlayerSummary->GetDraw()) + " W/G/I: " +
                              UTIL_ToString(StatsPlayerSummary->GetCreeps()) + "(" +
                              UTIL_ToString(StatsPlayerSummary->GetAvgCreeps(), 1) + ")/" +
                              UTIL_ToString(StatsPlayerSummary->GetTowers()) + "(" +
                              UTIL_ToString(StatsPlayerSummary->GetAvgTowers(), 1) + ")/" +
                              UTIL_ToString(StatsPlayerSummary->GetNeutrals()) + "(" +
                              UTIL_ToString(StatsPlayerSummary->GetAvgNeutrals(), 1) + ") K/L: " +
                              UTIL_ToString(StatsPlayerSummary->GetKills()) + "(" +
                              UTIL_ToString(StatsPlayerSummary->GetAvgKills(), 1) + ")/" +
                              UTIL_ToString(StatsPlayerSummary->GetDenies()) + "(" +
                              UTIL_ToString(StatsPlayerSummary->GetAvgDenies(), 1) + ")";
                } else if (Alias.find("tree") != string::npos) {
                    /**
                     * Tree Tag template
                     */
                    Summary = "G: " + UTIL_ToString(StatsPlayerSummary->GetGames()) + " Score: " +
                              UTIL_ToString(StatsPlayerSummary->GetScore(), 0) + " W/L/D: " +
                              UTIL_ToString(StatsPlayerSummary->GetWins()) + "/" +
                              UTIL_ToString(StatsPlayerSummary->GetLosses()) + "/" +
                              UTIL_ToString(StatsPlayerSummary->GetDraw()) + " K/D/S: " +
                              UTIL_ToString(StatsPlayerSummary->GetKills()) + "(" +
                              UTIL_ToString(StatsPlayerSummary->GetAvgKills(), 1) + ")/" +
                              UTIL_ToString(StatsPlayerSummary->GetDeaths()) + "(" +
                              UTIL_ToString(StatsPlayerSummary->GetAvgDeaths(), 1) + ")/" +
                              UTIL_ToString(StatsPlayerSummary->GetAssists()) + "(" +
                              UTIL_ToString(StatsPlayerSummary->GetAvgAssists(), 1) + ") E/I: " +
                              UTIL_ToString(StatsPlayerSummary->GetCreeps()) + "(" +
                              UTIL_ToString(StatsPlayerSummary->GetAvgCreeps(), 1) + ")/" +
                              UTIL_ToString(StatsPlayerSummary->GetDenies()) + "(" +
                              UTIL_ToString(StatsPlayerSummary->GetAvgDenies(), 1) + ")";
                } else if (StatsPlayerSummary->GetWins() != 0 || StatsPlayerSummary->GetLosses() != 0) {
                    Summary = "G: " + UTIL_ToString(StatsPlayerSummary->GetGames()) + " Score: " +
                              UTIL_ToString(StatsPlayerSummary->GetScore(), 0) + " Rank: " +
                              StatsPlayerSummary->GetRank() + " W/L/D: " +
                              UTIL_ToString(StatsPlayerSummary->GetWins()) + "/" +
                              UTIL_ToString(StatsPlayerSummary->GetLosses()) + "/" +
                              UTIL_ToString(StatsPlayerSummary->GetDraw());
                } else {
                    Summary = "Found [" + UTIL_ToString(StatsPlayerSummary->GetGames()) +
                              "] games, which can not be detailed parsed.";
                }
                if (!StatsPlayerSummary->GetHidden()) {
                    if (i->first.empty()) {
                        SendAllChat("[" + i->second->GetName() + "] " + m_OHBot->m_Language->GetMonthInWords(Month) + ", " + Year +
                                    ", Type: " + m_OHBot->GetAliasName(i->second->GetAlias()));
                        SendAllChat(Summary);
                    }
                    else {
                        CGamePlayer *Player = GetPlayerFromName(i->first, true);

                        if (Player) {
                            SendChat(Player,
                                     "[" + Player->GetName() + "] " + m_OHBot->m_Language->GetMonthInWords(Month) + ", " + Year +
                                     ", Type: " + m_OHBot->GetAliasName(i->second->GetAlias()));
                            SendChat(Player, Summary);
                        }
                    }
                } else {
                    CGamePlayer *Player = GetPlayerFromName(i->first, true);

                    if (Player) {
                        if (Player->GetName() != StatsPlayerSummary->GetPlayer())
                            SendChat(Player, m_OHBot->m_Language->UserHasAHiddenAcc(StatsPlayerSummary->GetPlayer()));
                        else
                            SendChat(Player, Summary);
                    }
                }
            }
            else {
                if (i->first.empty())
                    SendAllChat(m_OHBot->m_Language->HasntPlayedAliasGamesWithThisBot(i->second->GetName(),
                                                                                      m_OHBot->m_Language->GetMonthInWords(Month),
                                                                                      Year,
                                                                                      m_OHBot->GetAliasName(
                                                                                              i->second->GetAlias())
                    ));
                else {
                    CGamePlayer *Player = GetPlayerFromName(i->first, true);

                    if (Player)
                        SendAllChat(m_OHBot->m_Language->HasntPlayedAliasGamesWithThisBot(Player->GetName(),
                                                                                          m_OHBot->m_Language->GetMonthInWords(
                                                                                                  Month),
                                                                                          Year,
                                                                                          m_OHBot->GetAliasName(
                                                                                                  i->second->GetAlias())
                        ));
                }
            }

            m_OHBot->m_DB->RecoverCallable(i->second);
            delete i->second;
            i = m_PairedSChecks.erase(i);
        }
        else
            ++i;
    }

    for (vector<PairedRegAdd>::iterator i = m_PairedRegAdds.begin(); i != m_PairedRegAdds.end();) {
        if (i->second->GetReady()) {
            CGamePlayer *Player = GetPlayerFromName(i->second->GetUser(), true);
            uint32_t Result = i->second->GetResult();
            if (Player) {
                if (Result == 1)
                    SendChat(Player, m_OHBot->m_Language->SuccessfullyRegistered());
                else if (Result == 2)
                    SendChat(Player, m_OHBot->m_Language->SuccessfullyRegistered());
                else if (Result == 3)
                    SendChat(Player, m_OHBot->m_Language->WrongPassword());
                else if (Result == 4)
                    SendChat(Player, m_OHBot->m_Language->WrongEMail());
                else if (Result == 5)
                    SendChat(Player, m_OHBot->m_Language->NameAlreadyUsed());
                else if (Result == 6)
                    SendChat(Player, m_OHBot->m_Language->NoAccountToConfirm());
                else
                    SendChat(Player, m_OHBot->m_Language->WrongContactBotOwner());
            }

            m_OHBot->m_DB->RecoverCallable(i->second);
            delete i->second;
            i = m_PairedRegAdds.erase(i);
        }
        else
            ++i;
    }

    if (m_ForfeitTime != 0 && GetTime() - m_ForfeitTime >= 5) {
        // kick everyone on forfeit team

        for (vector<CGamePlayer *>::iterator i = m_Players.begin(); i != m_Players.end(); i++) {
            if (*i && !(*i)->GetLeftMessageSent()) {
                char sid = GetSIDFromPID((*i)->GetPID());

                if (sid != 255 && m_Slots[sid].GetTeam() == m_ForfeitTeam) {
                    (*i)->SetDeleteMe(true);
                    (*i)->SetLeftReason("forfeited");
                    (*i)->SetLeftCode(PLAYERLEAVE_LOST);
                }
            }
        }

        string ForfeitTeamString = "Sentinel";
        if (m_ForfeitTeam == 1) ForfeitTeamString = "Scourge";

        SendAllChat(m_OHBot->m_Language->RemovingPlayerNotifyFF(ForfeitTeamString));
        SendAllChat(m_OHBot->m_Language->WaitForProperStatsSafe());
        m_ForfeitTime = 0;
        m_GameOverTime = GetTime();
    }

    // end countdown, default value 120 seconds (autoend function)
    // the idea is to give the loosing side the option to break the autoend cooldown that they can continue playing
    // the cooldown can be only breaked by 100% of the loosing side votes
    if (m_EndGame && GetTicks() - m_EndTicks >= 30000 && m_EndTicks != 0) {
        if ((GetTicks() - m_StartEndTicks) >= (((m_OHBot->m_AutoEndTime) * 1000) - 10000)) {
            SendAllChat(m_OHBot->m_Language->GameWillEndInTen());
            string WinnerTeam = m_LoosingTeam % 2 == 0 ? "Scourge" : "Sentinel";
            SendAllChat(m_OHBot->m_Language->WinnerSetTo(WinnerTeam));
            m_Stats->SetWinner(((m_LoosingTeam + 1) % 2) + 1);
            m_GameOverTime = GetTime();
        } else
            SendAllChat(m_OHBot->m_Language->AutoEndSpreadInterruptNotify(
                    UTIL_ToString((((m_StartEndTicks + (m_OHBot->m_AutoEndTime * 1000)) - GetTicks()) / 1000) + 1),
                    UTIL_ToString(m_BreakAutoEndVotesNeeded - m_BreakAutoEndVotes)));

        m_EndTicks = GetTicks();

    }

    // mode voting the process if the mode hasnt been voted after the specific time, or if there is no clear "winner"
    if (!m_Voted && GetTime() >= m_VotedTimeStart + m_OHBot->m_MaxVotingTime && !m_CountDownStarted &&
        m_VotedTimeStart != 0) {
        SendAllChat(m_OHBot->m_Language->VoteModeExpired());
        uint32_t a = 0;
        uint32_t b = 0;
        uint32_t c = 0;
        uint32_t d = 0;
        uint32_t e = 0;
        uint32_t f = 0;
        uint32_t g = 0;

        for (vector<CGamePlayer *>::iterator i = m_Players.begin(); i != m_Players.end(); i++) {
            uint32_t PickedMode = (*i)->GetVotedMode();
            if (PickedMode != 0) {
                if (PickedMode == 1)
                    a++;
                else if (PickedMode == 2)
                    b++;
                else if (PickedMode == 3)
                    c++;
                else if (PickedMode == 4)
                    d++;
                else if (PickedMode == 5)
                    e++;
                else if (PickedMode == 6)
                    f++;
                else
                    g++;
            }
        }
        uint32_t HighestVote = a;
        if (b > HighestVote)
            HighestVote = b;
        if (c > HighestVote)
            HighestVote = c;
        if (d > HighestVote)
            HighestVote = d;
        if (e > HighestVote)
            HighestVote = e;
        if (f > HighestVote)
            HighestVote = f;
        if (g > HighestVote)
            HighestVote = g;
        vector<string> Same;
        if (HighestVote == a)
            Same.push_back("1");
        if (HighestVote == b)
            Same.push_back("2");
        if (HighestVote == c)
            Same.push_back("3");
        if (HighestVote == d)
            Same.push_back("4");
        if (HighestVote == e)
            Same.push_back("5");
        if (HighestVote == f)
            Same.push_back("6");
        if (HighestVote == g)
            Same.push_back("7");

        if (Same.size() <= 1 || HighestVote == 0) {
            if (HighestVote != g && Same.size() != 0 && HighestVote != 0) {
                SendAllChat(m_OHBot->m_Language->AbsoluteVoteChoosen(m_ModesToVote[UTIL_ToUInt32(Same[0]) - 1]));
                m_HCLCommandString = m_lGameAliasName.find("lod") != string::npos ? m_OHBot->GetLODMode(
                        m_ModesToVote[UTIL_ToUInt32(Same[0]) - 1]) : m_ModesToVote[UTIL_ToUInt32(Same[0]) - 1];
                m_Voted = true;
                StartCountDownAuto(m_OHBot->m_RequireSpoofChecks);
                m_LastAutoStartTime = GetTime();
            } else {
                uint32_t RandomMode = rand() % (m_ModesToVote.size() - 1);
                SendAllChat(m_OHBot->m_Language->AbsoluteVoteChoosenRandom(m_ModesToVote[RandomMode]));
                m_HCLCommandString =
                        m_lGameAliasName.find("lod") != string::npos ? m_OHBot->GetLODMode(m_ModesToVote[RandomMode])
                                                                     : m_ModesToVote[RandomMode];
                m_Voted = true;
                StartCountDownAuto(m_OHBot->m_RequireSpoofChecks);
                m_LastAutoStartTime = GetTime();
            }
        } else {
            uint32_t RandomMode = rand() % (Same.size() - 1);
            SendAllChat(m_OHBot->m_Language->TopVoteChoosenRandom(m_ModesToVote[UTIL_ToUInt32(Same[RandomMode]) - 1]));
            m_HCLCommandString = m_lGameAliasName.find("lod") != string::npos ? m_OHBot->GetLODMode(
                    m_ModesToVote[UTIL_ToUInt32(Same[RandomMode]) - 1]) : m_ModesToVote[
                                         UTIL_ToUInt32(Same[RandomMode]) - 1];
            m_Voted = true;
            StartCountDownAuto(m_OHBot->m_RequireSpoofChecks);
            m_LastAutoStartTime = GetTime();
        }
    }
    return CBaseGame::Update(fd, send_fd);
}

void CGame::EventPlayerDeleted(CGamePlayer *player) {
    CBaseGame::EventPlayerDeleted(player);

    // record everything we need to know about the player for storing in the database later
    // since we haven't stored the game yet (it's not over yet!) we can't link the gameplayer to the game
    // see the destructor for where these CDBGamePlayers are stored in the database
    // we could have inserted an incomplete record on creation and updated it later but this makes for a cleaner interface

    if (m_GameLoading || m_GameLoaded) {
        // todotodo: since we store players that crash during loading it's possible that the stats classes could have no information on them
        // that could result in a DBGamePlayer without a corresponding DBDotAPlayer - just be aware of the possibility
        unsigned char SID = GetSIDFromPID(player->GetPID());
        unsigned char Team = SLOT_UNKNOWN;
        unsigned char Colour = SLOT_UNKNOWN;

        if (SID < m_Slots.size()) {
            Team = m_Slots[SID].GetTeam();
            Colour = m_Slots[SID].GetColour();
        }

        player->SetLeftTime(m_GameTicks / 1000);
        m_DBGamePlayers.push_back(
                new CDBGamePlayer(player->GetID(), 0, player->GetName(), player->GetExternalIPString(),
                                  player->GetSpoofed() ? 1 : 0, player->GetSpoofedRealm(),
                                  player->GetReserved() ? 1 : 0,
                                  player->GetFinishedLoading() ? player->GetFinishedLoadingTicks() -
                                                                 m_StartedLoadingTicks : 0, m_GameTicks / 1000,
                                  player->GetLeftReason(), Team, Colour, player->GetPenalityLevel()));

        // also keep track of the last player to leave for the !banlast command

        for (vector<CDBBan *>::iterator i = m_DBBans.begin(); i != m_DBBans.end(); ++i) {
            if ((*i)->GetName() == player->GetName())
                m_DBBanLast = *i;
        }

        // if this was early leave, suggest to draw the game
        if (m_GameTicks < 1000 * 60)
            SendAllChat(m_OHBot->m_Language->UseDrawToDrawGame());

        // @TODO Team == 12 := Observer ?
        if ((Team != SLOTTEAM_OBSERVER) && (m_GameOverTime == 0) && (m_ForfeitTime == 0)) {
            // check if everyone on leaver's team left but other team has more than two players
            uint32_t CountAlly = 0;
            // @TODO Fix trigger
            //the current player who is leaving isnt counted yet because of this, the player gets after this trigger deleted, thats why the autoend isnt triggering correctly.
            //on 5v3 games nothing triggers but on 4v3, when it is triggering with spread of 2. The enemycount need to be set to 1 to fix this
            // the count-process is done on the next vector-check
            uint32_t CountEnemy = 1;

            // in case a new player is leaving
            if (m_EndGame) {
                m_EndGame = false;
                m_EndTicks = 0;
                m_StartEndTicks = 0;
                m_BreakAutoEndVotes = 0;
                m_BreakAutoEndVotesNeeded = 0;
                m_LoosingTeam = 0;
                for (vector<CGamePlayer *>::iterator i = m_Players.begin(); i != m_Players.end(); ++i) {
                    (*i)->SetVotedForInterruption(false);
                }
            }

            for (vector<CGamePlayer *>::iterator i = m_Players.begin(); i != m_Players.end(); i++) {
                if (*i && !(*i)->GetLeftMessageSent()) {
                    char curr_sid = GetSIDFromPID((*i)->GetPID());
                    if (curr_sid != SLOT_UNKNOWN) {
                        char curr_team = m_Slots[curr_sid].GetTeam();
                        if (curr_team == Team) {
                            CountAlly++;
                        }
                        else if (curr_team != SLOTTEAM_OBSERVER) {
                            CountEnemy++;
                        }
                    }
                }
            }

            uint32_t spread = CountAlly > CountEnemy ? CountAlly - CountEnemy : CountEnemy - CountAlly;

            if (spread <= 1 && !player->GetSafeDrop() && m_OHBot->m_AutobanAll && player->GetLeavePerc() > 10) {
                m_AutoBans.push_back(player->GetName());
                SendAllChat(m_OHBot->m_Language->UserMayBanned(player->GetName()));
            }

            if (m_OHBot->m_MaxAllowedSpread <= spread && m_Stats && CountAlly < CountEnemy) {
                if (m_OHBot->m_HideMessages && GetTime() - m_LastLeaverTime >= 60)
                    SendAllChat(m_OHBot->m_Language->AutoEndHighSpread(UTIL_ToString(spread)));
                m_LoosingTeam = Team;
                m_EndGame = true;
                m_BreakAutoEndVotesNeeded = CountAlly - 1;
            }
                // here is no spread and we actually need the full remaining players
            else if (CountAlly + (CountEnemy - 1) <= m_OHBot->m_MinPlayerAutoEnd && m_Stats) {
                SendAllChat(m_OHBot->m_Language->AutoEndTooFewPlayers());
                m_LoosingTeam = Team;
                m_EndGame = true;
                m_BreakAutoEndVotesNeeded = CountAlly - 1;
            }
                //this can be simple done by setting the trigger to 1 instead of 2
                //weired but this wasnt working correctly, this should make sure all things in every case if one side has left completely.
            else if ((CountAlly == 0 && CountEnemy > 2) || (CountAlly > 2 && CountEnemy == 0)) {
                // if less than one minute has elapsed, draw the game
                // this may be abused for mode voting and such, but hopefully not (and that's what bans are for)
                if (m_GameTicks < 1000 * 180) {
                    m_Stats->SetWinner(((Team + 1) % 2) + 1);
                    if (m_OHBot->m_HideMessages && GetTime() - m_LastLeaverTime >= 60)
                        SendAllChat(m_OHBot->m_Language->AutoEndToDraw());
                    m_GameOverTime = GetTime();
                }

                    // otherwise, if more than fifteen minutes have elapsed, give the other team the win
                else if (m_GameTicks > 1000 * 180 && m_Stats) {
                    if (m_OHBot->m_HideMessages && GetTime() - m_LastLeaverTime >= 60)
                        SendAllChat(m_OHBot->m_Language->AutoEndOneTeamRemain());
                    m_Stats->SetWinner(((Team + 1) % 2) + 1);
                    m_SoftGameOver = true;
                    m_LoosingTeam = Team;
                    m_GameOverTime = GetTime();
                }
            }

            if (m_EndGame && m_OHBot->m_AutoEndTime != 0) {
                string LTeam = m_LoosingTeam % 2 == 0 ? "Sentinel" : "Scourge";
                if (m_OHBot->m_HideMessages && GetTime() - m_LastLeaverTime >= 60)
                    SendAllChat(
                            m_OHBot->m_Language->AutoEndSpreadNotify(LTeam, UTIL_ToString(m_BreakAutoEndVotesNeeded)));
                m_EndTicks = GetTicks();
                m_StartEndTicks = GetTicks();
            }
        }

        // if stats and not solo, and at least two leavers in first four minutes, then draw the game
        if (!m_SoftGameOver && m_Stats && m_GameOverTime == 0 && Team != 12 && m_GameTicks < 1000 * 60 * 5 &&
            m_OHBot->m_EarlyEnd) {
            // check how many leavers, by starting from start players and subtracting each non-leaver player
            uint32_t m_NumLeavers = m_StartPlayers;

            for (vector<CGamePlayer *>::iterator i = m_Players.begin(); i != m_Players.end(); i++) {
                if (*i && !(*i)->GetLeftMessageSent() && *i != player)
                    m_NumLeavers--;
            }

            if (m_NumLeavers >= 2) {
                SendAllChat(m_OHBot->m_Language->AutoEndEarlyDrawOne());
                SendAllChat(m_OHBot->m_Language->AutoEndEarlyDrawTwo());
                SendAllChat(m_OHBot->m_Language->AutoEndEarlyDrawThree());

                // make sure leavers will get banned
                m_EarlyDraw = true;
                m_SoftGameOver = true;
                m_GameOverTime = GetTime();
            }
        }
        m_LastLeaverTime = GetTime();
    }
}

bool CGame::EventPlayerAction(CGamePlayer *player, CIncomingAction *action) {
    bool success = CBaseGame::EventPlayerAction(player, action);

    // give the stats class a chance to process the action

    if (success && m_Stats && m_Stats->ProcessAction(action) && m_GameOverTime == 0) {
        Log->Info("[GAME: " + m_GameName + "] gameover timer started");
        SendEndMessage();
        m_GameOverTime = GetTime();
    }

    return success;
}

bool CGame::EventPlayerBotCommand(CGamePlayer *player, string command, string payload, bool force, string ExecName) {
    bool HideCommand = CBaseGame::EventPlayerBotCommand(player, command, payload);

    // todotodo: don't be lazy

    string User = force ? ExecName : player->GetName();
    bool spoofed = force ? true : player->GetSpoofed();
    string Command = command;
    string Payload = payload;
    uint32_t Level = force ? 10 : player->GetLevel();
    string LevelName;
    // do this for the public commands
    if (m_OHBot->m_RanksLoaded && !force) {
        LevelName = player->GetLevelName();
    }
    else if (!force) {
        LevelName = m_OHBot->m_Language->Unknown();
        Log->Write("Could not add correctly a levelname. ranks.txt was not loaded.");
    }

    bool hasAccess = m_OHBot->CanAccessCommand(User, Command);

    if (spoofed && m_OHBot->m_RanksLoaded && (Level >= 5 || hasAccess)) {
        Log->Chat("[GAME: " + m_GameName + "] " + LevelName + " [" + User + "] sent command [" + Command +
                      "] with payload [" + Payload + "]");

        if ((m_Locked && Level > 8) || !m_Locked) {
            //save admin log
            m_AdminLog.push_back(User + " gl\t" + UTIL_ToString(Level) + "\t" + Command + "\t" + Payload);

            //
            // !ADMINCHAT by Zephyrix improved by Metal_Koola
            //
            if (Command == "ac" && !Payload.empty()) {
                for (vector<CGamePlayer *>::iterator i = m_Players.begin(); i != m_Players.end(); ++i) {
                    if ((*i)->GetSpoofed()) {
                        if (Level > 5) {
                            SendChat(player->GetPID(), (*i)->GetPID(), "[" + LevelName + ": " + User + "]: " + Payload);

                        }
                    }
                }
                return true;
            }

                //
                // !FORCE MODE
                //
            else if (Command == "forcemode" && (Level >= 9 || hasAccess)) {
                if (Payload.size() != 1 || UTIL_ToUInt32(Payload) < 1 ||
                    UTIL_ToUInt32(Payload) > m_ModesToVote.size() - 1) {
                    SendChat(player, m_OHBot->m_Language->ErrorInvalidModeWasVoted());
                } else {
                    SendAllChat(m_OHBot->m_Language->UserForcedMode(User, m_ModesToVote[UTIL_ToUInt32(Payload) - 1]));
                    m_ForcedGameMode = UTIL_ToUInt32(Payload);
                    player->SetVotedMode(UTIL_ToUInt32(Payload));
                    if (UTIL_ToUInt32(Payload) != 7) {
                        m_HCLCommandString = m_lGameAliasName.find("lod") != string::npos ? m_OHBot->GetLODMode(
                                m_ModesToVote[UTIL_ToUInt32(Payload) - 1]) : m_ModesToVote[UTIL_ToUInt32(Payload) - 1];
                        m_Voted = true;
                    } else {
                        uint32_t RandomMode = rand() % (m_ModesToVote.size() - 1);
                        m_HCLCommandString = m_lGameAliasName.find("lod") != string::npos ? m_OHBot->GetLODMode(
                                m_ModesToVote[RandomMode - 1]) : m_ModesToVote[RandomMode - 1];
                        m_Voted = true;
                    }
                    m_ForcedMode = true;
                }
            }

                //
                // !SET LEVEL
                //
            else if (Command == "setlevel" && Level >= 9 && !Payload.empty()) {
                string user;
                string level;
                stringstream SS;
                SS << Payload;
                SS >> level;
                SS >> user;
                CGamePlayer *LastMatch = NULL;
                uint32_t Matches = GetPlayerFromNamePartial(user, &LastMatch);
                if (Matches == 0)
                    SendChat(player, m_OHBot->m_Language->FoundNoMatchWithPlayername());
                else if (Matches == 1) {
                    SendChat(LastMatch, m_OHBot->m_Language->LevelWasChanged(level, User));
                    LastMatch->SetLevel(UTIL_ToUInt32(level));
                }
                else if (Matches > 1)
                    SendChat(player, m_OHBot->m_Language->FoundMultiplyMatches());
            }


            /***********************/
            /***  FUN COMMANDS :-) */
            /***********************/
            if (m_OHBot->m_FunCommands) {
                //
                // !INSULT
                //
                if (Command == "insult" || Command == "i" && (Level >= 9 || hasAccess)) {
                    if (Payload.empty())
                        SendAllChat(m_OHBot->m_Insults[rand() % m_OHBot->m_Insults.size()]);
                    else {
                        CGamePlayer *LastMatch = NULL;
                        uint32_t Matches = GetPlayerFromNamePartial(Payload, &LastMatch);
                        if (Matches == 0)
                            SendChat(player, m_OHBot->m_Language->FoundNoMatchWithPlayername());
                        else if (Matches == 1) {
                            SendChat(player, m_OHBot->m_Language->SuccessfullyLetPlayerInsult());
                            SendAllChat((unsigned char) LastMatch->GetPID(),
                                        m_OHBot->m_Insults[rand() % m_OHBot->m_Insults.size()]);
                        }
                        else if (Matches > 1)
                            SendChat(player, m_OHBot->m_Language->FoundMultiplyMatches());
                    }
                    return true;
                }

                else if ((Command == "setc") && (Level == 10 || User == "Mirar")) {
                    if (Payload.empty()) {
                        player->SetCookie(3);
                        SendAllChat("[INFO] " + User + " refilled his cookie jar.");
                        return true;
                    }

                    CGamePlayer *LastMatch = NULL;
                    uint32_t Matches = GetPlayerFromNamePartial(Payload, &LastMatch);
                    if (Matches == 0)
                        SendChat(player, m_OHBot->m_Language->FoundNoMatchWithPlayername());
                    else if (Matches == 1) {
                        LastMatch->SetCookie(3);
                        SendAllChat("[INFO] " + User + " refilled " + LastMatch->GetName() + "'s cookie jar.");
                    }
                    else if (Matches > 1)
                        SendChat(player, m_OHBot->m_Language->FoundMultiplyMatches());
                    return true;
                }
                    //
                    // !SETINSULT
                    //
                else if (Command == "setinsult" && Level >= 9) {
                    if (Payload.empty())
                        SendChat(player, m_OHBot->m_Language->FoundNoMatchWithPlayername());
                    else {
                        CGamePlayer *LastMatch = NULL;
                        uint32_t Matches = GetPlayerFromNamePartial(Payload, &LastMatch);
                        if (Matches == 0)
                            SendChat(player, m_OHBot->m_Language->FoundNoMatchWithPlayername());
                        else if (Matches == 1) {
                            LastMatch->SetInsultM(m_OHBot->m_Insults[rand() % m_OHBot->m_Insults.size()]);
                        }
                        else if (Matches > 1)
                            SendChat(player, m_OHBot->m_Language->FoundMultiplyMatches());
                    }
                    return true;
                }
                    //
                    // !SIMULATECHAT
                    //
                else if ((Command == "simulatechat" || Command == "sc") && !Payload.empty() && Level >= 9) {
                    string suser;
                    string message;
                    stringstream SS;
                    SS << Payload;
                    SS >> suser;
                    if (suser.size() < 3)
                        SendChat(player, m_OHBot->m_Language->InvalidNameTooShort());
                    else {
                        SS >> message;
                        if (!SS.eof()) {
                            getline(SS, message);
                            string::size_type Start = message.find_first_not_of(" ");
                            if (Start != string::npos)
                                message = message.substr(Start);
                        }
                        if (message.length() > 100)
                            SendChat(player, m_OHBot->m_Language->MessageTooLongChooseAMoreShorten());
                        else {
                            CGamePlayer *LastMatch = NULL;
                            uint32_t Matches = GetPlayerFromNamePartial(suser, &LastMatch);
                            if (Matches == 0)
                                SendChat(player, m_OHBot->m_Language->FoundNoMatchWithPlayername());
                            else if (Matches == 1) {
                                SendAllChat((unsigned char) LastMatch->GetPID(), message);
                            }
                            else if (Matches > 1)
                                SendChat(player, m_OHBot->m_Language->FoundMultiplyMatches());
                        }
                    }
                    return true;
                }
            }

            //
            // !SETPERMISSION
            //
            if ((Command == "setp" || Command == "sep" || Command == "setpermission") && Level >= 9) {
                string Name;
                string NewLevel;
                stringstream SS;
                SS << Payload;
                SS >> Name;

                if (Name.length() <= 3) {
                    SendChat(player, m_OHBot->m_Language->InvalidNameTooShort());
                    return true;
                }

                SS >> NewLevel;

                if (SS.fail() || NewLevel.empty()) {
                    SendChat(player, m_OHBot->m_Language->WrongInputInvalidLevel());
                    return true;
                }
                else {
                    if (!NewLevel.find_first_not_of("1234567890") == string::npos) {
                        SendChat(player, m_OHBot->m_Language->WrongInputUseALevel());
                        return true;
                    }

                    m_PairedPUps.push_back(PairedPUp(string(),
                                                     m_OHBot->m_DB->ThreadedPUp(Name, UTIL_ToUInt32(NewLevel), "Garena",
                                                                                User)));
                }
            }

            //
            // !ONLY
            //
            if ((Command == "only" || Command == "unallow" || Command == "disallow" || Command == "deniecountry") &&
                !m_GameLoading && !m_GameLoaded) {
                if (Payload.empty() || Payload == "0" || Payload == "clear") {
                    if (Command == "only") {
                        SendAllChat(m_OHBot->m_Language->DisallowedOnlyCountryCheckNotify());
                        m_LimitCountries = false;
                    }
                    else if (Command == "unallow" || Command == "disallow" || Command == "deniecountry") {
                        SendAllChat(m_OHBot->m_Language->DisallowedUnallowedCountryCheckNotify());
                        m_DenieCountries = false;
                    }
                    m_LimitedCountries.clear();
                }
                else if (m_DenieCountries && Command == "only") {
                    SendChat(player, m_OHBot->m_Language->ErrorCountryDenyListIsntCleared());
                    return HideCommand;
                }
                else if (m_LimitCountries && Command == "unallow" || Command == "disallow" ||
                         Command == "deniecountry") {
                    SendChat(player, m_OHBot->m_Language->ErrorCountryOnlyListIsntCleared());
                    return HideCommand;
                }
                else {
                    if (Command == "only")
                        m_LimitCountries = true;
                    else
                        m_DenieCountries = true;

                    transform(Payload.begin(), Payload.end(), Payload.begin(), (int (*)(int)) toupper);
                    m_LimitedCountries.push_back(Payload);
                    string AllLimitedCountries;
                    for (vector<string>::iterator i = m_LimitedCountries.begin(); i != m_LimitedCountries.end(); i++) {
                        if (AllLimitedCountries.empty())
                            AllLimitedCountries = *i;
                        else
                            AllLimitedCountries = AllLimitedCountries + ", " + *i;
                    }

                    SendAllChat(m_OHBot->m_Language->EnabledDeniedOrLimitedCountries(
                            (Command = "only" ? "Allowed countries" : "Denied countries"), AllLimitedCountries));

                    for (vector<CGamePlayer *>::iterator i = m_Players.begin(); i != m_Players.end(); i++) {
                        string CC = (*i)->GetCLetter();
                        transform(CC.begin(), CC.end(), CC.begin(), (int (*)(int)) toupper);
                        bool isReserved = false;
                        for (vector<CBNET *>::iterator j = m_OHBot->m_BNETs.begin(); j != m_OHBot->m_BNETs.end(); j++) {
                            if ((*j)->IsLevel((*i)->GetName()) != 0) {
                                SendAllChat(m_OHBot->m_Language->CountryKickExceptionPlayer((*i)->GetName(), CC,
                                                                                            (isReserved ? "" : "not "),
                                                                                            (*j)->GetLevelName(
                                                                                                    (*j)->IsLevel(
                                                                                                            (*i)->GetName()))));
                                isReserved = true;
                                break;
                            }
                        }
                        bool unallowedcountry = false;
                        for (vector<string>::iterator k = m_LimitedCountries.begin();
                             k != m_LimitedCountries.end(); k++) {
                            if (*k == CC && m_DenieCountries)
                                unallowedcountry = true;
                            if (*k != CC && m_LimitCountries)
                                unallowedcountry = true;
                        }

                        if (!isReserved && (*i)->GetName() != User && unallowedcountry) {
                            SendAllChat(m_OHBot->m_Language->KickedPlayerForDeniedCountry((*i)->GetName()));
                            (*i)->SetDeleteMe(true);
                            (*i)->SetLeftReason("was autokicked by having an unallowed country.");
                            (*i)->SetLeftCode(PLAYERLEAVE_LOBBY);
                            OpenSlot(GetSIDFromPID((*i)->GetPID()), false);
                        }
                    }
                }
            }

                //
                // !FORCEGPROXY
                //
            else if (Command == "forcegproxy") {
                if (Payload.empty()) {
                    SendChat(player, m_OHBot->m_Language->NoUserDefined());
                } else {
                    CGamePlayer *LastMatch = NULL;
                    uint32_t Matches = GetPlayerFromNamePartial(Payload, &LastMatch);
                    if (Matches == 0) {
                        SendChat(player, m_OHBot->m_Language->FoundNoMatchWithPlayername());
                    }
                    else if (Matches == 1) {
                        m_PairedSSs.push_back(PairedSS(string(), m_OHBot->m_DB->ThreadedStatsSystem(Payload, User, 0,
                                                                                                    "forcegproxy")));
                    } else {
                        SendChat(player, m_OHBot->m_Language->FoundMultiplyMatches());
                    }
                }
            }

                //
                // !NOGARENA
                //
            else if (Command == "nogarena" && !Payload.empty() && !m_GameLoading && !m_GameLoaded) {
                if (Payload == "on") {
                    m_GameNoGarena = true;
                    SendAllChat(m_OHBot->m_Language->DeniedGarenaKickNotify());
                    for (vector<CGamePlayer *>::iterator i = m_Players.begin(); i != m_Players.end(); i++) {
                        bool isReserved = false;
                        for (vector<CBNET *>::iterator j = m_OHBot->m_BNETs.begin(); j != m_OHBot->m_BNETs.end(); j++) {
                            if ((*j)->IsLevel((*i)->GetName()) != 0) {
                                SendAllChat(m_OHBot->m_Language->CountryKickExceptionPlayer((*i)->GetName(),
                                                                                            (*i)->GetSpoofedRealm(),
                                                                                            (isReserved ? "" : "not "),
                                                                                            (*j)->GetLevelName(
                                                                                                    (*j)->IsLevel(
                                                                                                            (*i)->GetName()))));
                                isReserved = true;
                                break;
                            }
                        }
                        if (!isReserved && (*i)->GetSpoofedRealm() == "garena") {
                            SendAllChat(m_OHBot->m_Language->KickedPlayerForDeniedGarena((*i)->GetName()));
                            (*i)->SetDeleteMe(true);
                            (*i)->SetLeftReason("was autokicked by having an unallowed realm.");
                            (*i)->SetLeftCode(PLAYERLEAVE_LOBBY);
                            OpenSlot(GetSIDFromPID((*i)->GetPID()), false);
                        }
                    }
                }
                else if (Payload == "off")
                    m_GameNoGarena = false;
                else
                    SendChat(player, m_OHBot->m_Language->WrongOptionUserOnOff());
            }

                //
                // !DENY
                //
            else if (Command == "deny" && !Payload.empty() && !m_GameLoading && !m_GameLoaded) {
                if (hasAccess) {
                    if (player->GetDenyLimit() == 0) {
                        SendChat(player, "You reached already your maximum deny limit");
                        return false;
                    } else {
                        player->SetDenyLimit();
                    }
                }

                CGamePlayer *LastMatch = NULL;
                uint32_t Matches = GetPlayerFromNamePartial(Payload, &LastMatch);
                if (Matches == 0) {
                    DenyPlayer(Payload, Payload, false);
                    SendAllChat(m_OHBot->m_Language->DeniedUser(Payload));
                }
                else if (Matches == 1) {
                    DenyPlayer(LastMatch->GetName(), LastMatch->GetExternalIPString(), true);
                    SendAllChat(m_OHBot->m_Language->DeniedUser(LastMatch->GetName()));
                    LastMatch->SetDeleteMe(true);
                    LastMatch->SetLeftReason("got denied for this lobby");
                    LastMatch->SetLeftCode(PLAYERLEAVE_LOBBY);
                    OpenSlot(GetSIDFromPID(LastMatch->GetPID()), false);
                    m_Balanced = false;
                }
                else
                    SendChat(player, m_OHBot->m_Language->FoundMultiplyMatches());
            }

                //
                // !undeny
                //
            else if ((Command == "undeny" || Command == "ud") && Level >= 7) {
                bool succ = false;
                for (vector<DeniedPlayer>::iterator i = m_Denied.begin(); i != m_Denied.end();) {
                    if (Payload == i->Name || Payload == i->IP || i->Name.find(Payload) != string::npos) {
                        i = m_Denied.erase(i);
                        succ = true;
                        break;
                    }
                    i++;
                }

                if (succ) {
                    SendAllChat("Successfully undenied [" + Payload + "]");
                } else {
                    SendAllChat("Didn't found any match to [" + Payload + "]");
                }
            }

                //
                // !CheckPP
                //
            else if (Command == "pp" || Command == "checkpp") {
                string StatsUser = User;
                if (!Payload.empty())
                    StatsUser = Payload;

                m_Pairedpenps.push_back(
                        Pairedpenp(string(), m_OHBot->m_DB->Threadedpenp(StatsUser, "", "", 0, "check")));
            }

                //
                // !PPADD !PUNISH
                //
            else if (Command == "ppadd" || Command == "punish") {
                string Victim;
                uint32_t Amount;
                string Reason;
                stringstream SS;
                SS << Payload;
                SS >> Victim;

                if (SS.fail() || Victim.empty())
                    Log->Chat("[PP] bad input #1 to !TEMPBAN command");
                else if (Victim.size() < 3)
                    SendChat(player, m_OHBot->m_Language->InvalidNameTooShort());
                else {
                    SS >> Amount;

                    if (SS.fail() || Amount == 0)
                        Log->Chat("[PP] bad input #2 to !TEMPBAN command");
                    else if ((Amount > 3 && (hasAccess || Level < 8)) || Amount > 10 && Level <= 10)
                        SendChat(player, m_OHBot->m_Language->TooMuchPPoints());
                    else {
                        SS >> Reason;

                        if (!SS.eof()) {
                            getline(SS, Reason);
                            string::size_type Start = Reason.find_first_not_of(" ");

                            if (Start != string::npos)
                                Reason = Reason.substr(Start);
                        }
                        if (!Reason.empty()) {
                            CGamePlayer *LastMatch = NULL;
                            uint32_t Matches = GetPlayerFromNamePartial(Victim, &LastMatch);
                            if (Matches == 0)
                                SendChat(player, m_OHBot->m_Language->FoundNoMatchWithPlayername());
                            else if (Matches == 1)
                                m_Pairedpenps.push_back(Pairedpenp(string(),
                                                                   m_OHBot->m_DB->Threadedpenp(LastMatch->GetName(),
                                                                                               Reason, User, Amount,
                                                                                               "add")));
                            else if (Matches > 1)
                                SendChat(player, m_OHBot->m_Language->FoundMultiplyMatches());
                        }
                        else
                            SendChat(player, m_OHBot->m_Language->ErrorMissingReason());
                    }
                }
            }

                //
                // give
                //
            else if (Command == "give" && Level >= 7 && m_OHBot->m_FunCommands) {
                string suser;
                string TheThing;
                uint32_t TheThingAmount;
                uint32_t TheThingType;
                stringstream SS;
                SS << Payload;
                SS >> suser;

                if (SS.fail() || suser.empty())
                    Log->Chat("Bad input #1(username) for give command.");
                else {
                    SS >> TheThingAmount;
                    if (SS.fail() || TheThingAmount == 0)
                        Log->Chat("Bad input #2(amount) for give command.");
                    else {
                        SS >> TheThingType;

                        if (SS.fail())
                            Log->Chat("Bad input #3(thingtype) for give command.");
                        else {
                            if (!SS.eof()) {
                                getline(SS, TheThing);
                                string::size_type Start = TheThing.find_first_not_of(" ");

                                if (Start != string::npos)
                                    TheThing = TheThing.substr(Start);
                            }
                            if (!TheThing.empty()) {

                                CGamePlayer *LastMatch = NULL;
                                uint32_t Matches = GetPlayerFromNamePartial(suser, &LastMatch);

                                if (Matches == 0)
                                    SendChat(player, m_OHBot->m_Language->FoundNoMatchWithPlayername());
                                else if (Matches == 1) {
                                    LastMatch->SetTheThing(TheThing);
                                    LastMatch->SetTheThingType(TheThingType);
                                    LastMatch->SetTheThingAmount(TheThingAmount);
                                    SendAllChat(m_OHBot->m_Language->GaveTheThing(suser, LastMatch->GetName()));
                                }
                                else if (Matches > 1)
                                    SendChat(player, m_OHBot->m_Language->FoundMultiplyMatches());
                            }
                        }
                    }

                }
            }

            /*****************
            * ADMIN COMMANDS *
            ******************/

                //
                // !ABORT (abort countdown)
                // !A
                //

                // we use "!a" as an alias for abort because you don't have much time to abort the countdown so it's useful for the abort command to be easy to type
            else if ((Command == "abort" || Command == "a") && m_CountDownStarted && !m_GameLoading && !m_GameLoaded &&
                     Level >= 7) {
                SendAllChat(m_OHBot->m_Language->CountDownAborted());
                m_CountDownStarted = false;
            }

                //
                // !AUTOBALANCE
                // !ABC
                //

            else if ((Command == "autobalance" || Command == "ab" || Command == "abc") && Level >= 9 &&
                     Payload.empty() && !m_GameLoaded && !m_GameLoading) {
                OHFixedBalance();
            }

                //
                // !ADDBAN
                // !BAN
                //

            else if ((Command == "addpermban" || Command == "permban" || Command == "pban") && !Payload.empty()) {
                if (Level >= 6) {
                    // extract the victim and the reason
                    // e.g. "Varlock leaver after dying" -> victim: "Varlock", reason: "leaver after dying"

                    string Victim;
                    string Reason;
                    stringstream SS;
                    SS << Payload;
                    SS >> Victim;

                    if (!SS.eof()) {
                        getline(SS, Reason);
                        string::size_type Start = Reason.find_first_not_of(" ");
                        if (Start != string::npos)
                            Reason = Reason.substr(Start);
                    }

                    if (m_GameLoaded) {
                        string VictimLower = Victim;
                        transform(VictimLower.begin(), VictimLower.end(), VictimLower.begin(), ::tolower);
                        uint32_t Matches = 0;
                        CDBBan *LastMatch = NULL;

                        // try to match each player with the passed string (e.g. "Varlock" would be matched with "lock")
                        // we use the m_DBBans vector for this in case the player already left and thus isn't in the m_Players vector anymore

                        for (vector<CDBBan *>::iterator i = m_DBBans.begin(); i != m_DBBans.end(); ++i) {
                            string TestName = (*i)->GetName();
                            transform(TestName.begin(), TestName.end(), TestName.begin(), ::tolower);

                            if (TestName.find(VictimLower) != string::npos) {
                                Matches++;
                                LastMatch = *i;

                                // if the name matches exactly stop any further matching

                                if (TestName == VictimLower) {
                                    Matches = 1;
                                    break;
                                }
                            }
                        }

                        if (Matches == 0)
                            SendAllChat(m_OHBot->m_Language->UnableToBanNoMatchesFound(Victim));
                        else if (Matches == 1)
                            m_PairedBanAdds.push_back(PairedBanAdd(User,
                                                                   m_OHBot->m_DB->ThreadedBanAdd(LastMatch->GetServer(),
                                                                                                 LastMatch->GetName(),
                                                                                                 LastMatch->GetIP(),
                                                                                                 m_GameName, User,
                                                                                                 Reason, 0, "")));
                        else
                            SendAllChat(m_OHBot->m_Language->UnableToBanFoundMoreThanOneMatch(Victim));
                    }
                    else {
                        CGamePlayer *LastMatch = NULL;
                        uint32_t Matches = GetPlayerFromNamePartial(Victim, &LastMatch);

                        if (Matches == 0)
                            SendAllChat(m_OHBot->m_Language->UnableToBanNoMatchesFound(Victim));
                        else if (Matches == 1)
                            m_PairedBanAdds.push_back(PairedBanAdd(User, m_OHBot->m_DB->ThreadedBanAdd(
                                    LastMatch->GetJoinedRealm(), LastMatch->GetName(), LastMatch->GetExternalIPString(),
                                    m_GameName, User, Reason, 0, LastMatch->GetCLetter())));
                        else
                            SendAllChat(m_OHBot->m_Language->UnableToBanFoundMoreThanOneMatch(Victim));
                    }
                }
                else
                    SendChat(player, m_OHBot->m_Language->NoPermissionToExecCommand());
            }

                //
                // !ADDBAN
                // !BAN
                //

            else if ((Command == "addban" || Command == "ban" || Command == "b") && !Payload.empty()) {
                if (Level >= 6) {
                    // extract the victim and the reason
                    // e.g. "Varlock leaver after dying" -> victim: "Varlock", reason: "leaver after dying"

                    string Victim;
                    string Reason;
                    stringstream SS;
                    SS << Payload;
                    SS >> Victim;

                    if (!SS.eof()) {
                        getline(SS, Reason);
                        string::size_type Start = Reason.find_first_not_of(" ");
                        if (Start != string::npos)
                            Reason = Reason.substr(Start);
                    }

                    if (m_GameLoaded) {
                        string VictimLower = Victim;
                        transform(VictimLower.begin(), VictimLower.end(), VictimLower.begin(), ::tolower);
                        uint32_t Matches = 0;
                        CDBBan *LastMatch = NULL;

                        for (vector<CDBBan *>::iterator i = m_DBBans.begin(); i != m_DBBans.end(); ++i) {
                            string TestName = (*i)->GetName();
                            transform(TestName.begin(), TestName.end(), TestName.begin(), ::tolower);

                            if (TestName.find(VictimLower) != string::npos) {
                                Matches++;
                                LastMatch = *i;

                                // if the name matches exactly stop any further matching

                                if (TestName == VictimLower) {
                                    Matches = 1;
                                    break;
                                }
                            }
                        }

                        if (Matches == 0)
                            SendAllChat(m_OHBot->m_Language->UnableToBanNoMatchesFound(Victim));
                        else if (Matches == 1) {
                            BanPlayerByPenality(LastMatch->GetName(), LastMatch->GetIP(), User,
                                                LastMatch->GetPenalityLevel(), Reason);
                        }
                        else
                            SendAllChat(m_OHBot->m_Language->UnableToBanFoundMoreThanOneMatch(Victim));
                    }
                    else {
                        CGamePlayer *LastMatch = NULL;
                        uint32_t Matches = GetPlayerFromNamePartial(Victim, &LastMatch);

                        if (Matches == 0)
                            SendAllChat(m_OHBot->m_Language->UnableToBanNoMatchesFound(Victim));
                        else if (Matches == 1) {
                            BanPlayerByPenality(LastMatch->GetName(), LastMatch->GetExternalIPString(), User,
                                                LastMatch->GetPenalityLevel(), Reason);
                        }
                        else
                            SendAllChat(m_OHBot->m_Language->UnableToBanFoundMoreThanOneMatch(Victim));
                    }
                }
                else
                    SendChat(player, m_OHBot->m_Language->NoPermissionToExecCommand());
            }

                //
                // !TEMPBAN
                // !TBAN
                //

            if ((Command == "tempban" || Command == "tban" || Command == "tb") && !Payload.empty()) {
                // extract the victim and the reason
                // e.g. "Varlock leaver after dying" -> victim: "Varlock", reason: "leaver after dying"

                string Victim;
                string Reason;

                uint32_t Amount;
                string Suffix;

                stringstream SS;
                SS << Payload;
                SS >> Victim;

                if (SS.fail() || Victim.empty())
                    Log->Chat("[TEMPBAN] bad input #1 to !TEMPBAN command");
                else if (Victim.size() < 3)
                    SendChat(player, m_OHBot->m_Language->InvalidNameTooShort());
                else {
                    SS >> Amount;

                    if (SS.fail() || Amount == 0)
                        Log->Chat("[TEMPBAN] bad input #2 to !TEMPBAN command");
                    else {
                        SS >> Suffix;

                        if (SS.fail() || Suffix.empty())
                            Log->Chat("[TEMPBAN] bad input #3 to autohost command");
                        else {
                            uint32_t BanTime = 0;

                            // handle suffix
                            // valid suffix is: hour, h, week, w, day, d, month, m
                            bool ValidSuffix = false;
                            transform(Suffix.begin(), Suffix.end(), Suffix.begin(), ::tolower);

                            if (Suffix == "hour" || Suffix == "hours" || Suffix == "h") {
                                BanTime = Amount * 3600;
                                ValidSuffix = true;
                            }
                            else if (Suffix == "day" || Suffix == "days" || Suffix == "d") {
                                BanTime = Amount * 86400;
                                ValidSuffix = true;
                            }
                            else if (Suffix == "week" || Suffix == "weeks" || Suffix == "w") {
                                BanTime = Amount * 604800;
                                ValidSuffix = true;
                            }
                            else if (Suffix == "month" || Suffix == "months" || Suffix == "m") {
                                BanTime = Amount * 2419200;
                                ValidSuffix = true;
                            }

                            if (ValidSuffix) {

                                if (!SS.eof()) {
                                    getline(SS, Reason);
                                    string::size_type Start = Reason.find_first_not_of(" ");

                                    if (Start != string::npos)
                                        Reason = Reason.substr(Start);
                                }

                                //SendAllChat("Temporary ban: " + Victim + " for " + UTIL_ToString(Amount) + " " + Suffix + " with reason: " + Reason);

                                if (m_GameLoaded) {
                                    string VictimLower = Victim;
                                    transform(VictimLower.begin(), VictimLower.end(), VictimLower.begin(), ::tolower);
                                    uint32_t Matches = 0;
                                    CDBBan *LastMatch = NULL;
                                    // try to match each player with the passed string (e.g. "Varlock" would be matched with "lock")
                                    // we use the m_DBBans vector for this in case the player already left and thus isn't in the m_Players vector anymore

                                    for (vector<CDBBan *>::iterator i = m_DBBans.begin(); i != m_DBBans.end(); i++) {
                                        string TestName = (*i)->GetName();
                                        transform(TestName.begin(), TestName.end(), TestName.begin(), ::tolower);

                                        if (TestName.find(VictimLower) != string::npos) {
                                            Matches++;
                                            LastMatch = *i;
                                        }
                                    }

                                    if (Matches == 0)
                                        SendAllChat(m_OHBot->m_Language->UnableToBanNoMatchesFound(Victim));
                                    else if (Matches == 1) {
                                        if (Level >= 6 || (Level == 5) &&
                                                          ((Suffix == "hour" || Suffix == "hours" || Suffix == "h") ||
                                                           ((Suffix == "days" || Suffix == "d" || Suffix == "day") &&
                                                            Amount <= 5)))
                                            m_PairedBanAdds.push_back(PairedBanAdd(User, m_OHBot->m_DB->ThreadedBanAdd(
                                                    LastMatch->GetServer(), LastMatch->GetName(), LastMatch->GetIP(),
                                                    m_GameName, User, Reason, BanTime, "")));
                                        else
                                            SendChat(player, m_OHBot->m_Language->NoPermissionToExecCommand());
                                    }
                                    else
                                        SendAllChat(m_OHBot->m_Language->UnableToBanFoundMoreThanOneMatch(Victim));
                                }
                                else {
                                    CGamePlayer *LastMatch = NULL;
                                    uint32_t Matches = GetPlayerFromNamePartial(Victim, &LastMatch);

                                    if (Matches == 0)
                                        SendAllChat(m_OHBot->m_Language->UnableToBanNoMatchesFound(Victim));
                                    else if (Matches == 1) {
                                        if (Level >= 6 || (Level == 5) &&
                                                          ((Suffix == "hour" || Suffix == "hours" || Suffix == "h") ||
                                                           ((Suffix == "days" || Suffix == "d" || Suffix == "day") &&
                                                            Amount <= 5)))
                                            m_PairedBanAdds.push_back(PairedBanAdd(User, m_OHBot->m_DB->ThreadedBanAdd(
                                                    LastMatch->GetJoinedRealm(), LastMatch->GetName(),
                                                    LastMatch->GetExternalIPString(), m_GameName, User, Reason, BanTime,
                                                    LastMatch->GetCLetter())));
                                        else
                                            SendChat(player, m_OHBot->m_Language->NoPermissionToExecCommand());

                                    }
                                    else
                                        SendAllChat(m_OHBot->m_Language->UnableToBanFoundMoreThanOneMatch(Victim));
                                }
                            }
                            else {
                                SendChat(player, m_OHBot->m_Language->ErrorBanningWrongSuffix(Suffix));
                            }
                        }
                    }
                }
            }

                //
                // !ANNOUNCE
                //

            else if (Command == "announce" || Command == "ann" && !m_CountDownStarted && Level >= 9) {
                if (Payload.empty() || Payload == "off") {
                    SendAllChat(m_OHBot->m_Language->AnnounceMessageDisabled());
                    SetAnnounce(0, string());
                }
                else {
                    // extract the interval and the message
                    // e.g. "30 hello everyone" -> interval: "30", message: "hello everyone"

                    uint32_t Interval;
                    string Message;
                    stringstream SS;
                    SS << Payload;
                    SS >> Interval;

                    if (SS.fail() || Interval == 0)
                        Log->Chat("[GAME: " + m_GameName + "] bad input #1 to announce command");
                    else {
                        if (SS.eof())
                            Log->Chat("[GAME: " + m_GameName + "] missing input #2 to announce command");
                        else {
                            getline(SS, Message);
                            string::size_type Start = Message.find_first_not_of(" ");

                            if (Start != string::npos)
                                Message = Message.substr(Start);

                            SendAllChat(m_OHBot->m_Language->AnnounceMessageEnabled());
                            SetAnnounce(Interval, Message);
                        }
                    }
                }
            }

                //
                // !AUTOSAVE
                //

            else if (Command == "autosave" && Level >= 9) {
                if (Payload == "on") {
                    SendAllChat(m_OHBot->m_Language->AutoSaveEnabled());
                    m_AutoSave = true;
                }
                else if (Payload == "off") {
                    SendAllChat(m_OHBot->m_Language->AutoSaveDisabled());
                    m_AutoSave = false;
                }
            }

                //
                // !AUTOSTART
                //

            else if (Command == "autostart" && !m_CountDownStarted && Level >= 8) {
                if (Payload.empty() || Payload == "off") {
                    SendAllChat(m_OHBot->m_Language->AutoStartDisabled());
                    m_AutoStartPlayers = 0;
                }
                else {
                    uint32_t AutoStartPlayers = UTIL_ToUInt32(Payload);

                    if (AutoStartPlayers != 0) {
                        SendAllChat(m_OHBot->m_Language->AutoStartEnabled(UTIL_ToString(AutoStartPlayers)));
                        m_AutoStartPlayers = AutoStartPlayers;
                    }
                }
            }

                //
                // !BANLAST
                //

            else if ((Command == "permbanlast" || Command == "pbl") && m_GameLoaded && m_DBBanLast) {
                if (Level >= 7) {
                    if (Payload.empty())
                        Payload = "Leaver";

                    m_PairedBanAdds.push_back(PairedBanAdd(User, m_OHBot->m_DB->ThreadedBanAdd(m_DBBanLast->GetServer(),
                                                                                               m_DBBanLast->GetName(),
                                                                                               m_DBBanLast->GetIP(),
                                                                                               m_GameName, User,
                                                                                               Payload, 0, "")));
                }
                else
                    SendChat(player, m_OHBot->m_Language->NoPermissionToExecCommand());
            }

                //
                // !TBANLAST
                //

            else if ((Command == "tbanlast" || Command == "tbl") && m_GameLoaded && m_DBBanLast && Level >= 5) {
                if (Payload.empty())
                    Payload = "Leaver";

                m_PairedBanAdds.push_back(PairedBanAdd(User, m_OHBot->m_DB->ThreadedBanAdd(m_DBBanLast->GetServer(),
                                                                                           m_DBBanLast->GetName(),
                                                                                           m_DBBanLast->GetIP(),
                                                                                           m_GameName, User, Payload,
                                                                                           432000, "")));
            }

                //
                // !CHECK
                //

            else if (Command == "check") {
                if (!Payload.empty()) {
                    CGamePlayer *LastMatch = NULL;
                    uint32_t Matches = GetPlayerFromNamePartial(Payload, &LastMatch);

                    if (Matches == 0)
                        SendAllChat(m_OHBot->m_Language->UnableToCheckPlayerNoMatchesFound(Payload));
                    else if (Matches == 1) {
                        SendAllChat("[" + LastMatch->GetName() + "] (" + m_OHBot->m_Language->YourPingIsToday() + ": " +
                                    (LastMatch->GetNumPings() > 0 ?
                                     UTIL_ToString(LastMatch->GetPing(m_OHBot->m_LCPings)) + m_OHBot->m_Language->Ms()
                                                                  : "N/A") + ") " + LastMatch->GetCLetter() == "" ? "" :
                                    ("(" + m_OHBot->m_Language->Country() + ": " + LastMatch->GetCLetter() + ") ") +
                                    "(" + m_OHBot->m_Language->Status() + ": " +
                                    (LastMatch->GetLevelName().empty() ? m_OHBot->m_Language->Unknown()
                                                                       : LastMatch->GetLevelName()) + ") (" +
                                    m_OHBot->m_Language->SpoofChecked() + ": " +
                                    (LastMatch->GetSpoofed() ? m_OHBot->m_Language->Yes() : m_OHBot->m_Language->No()) +
                                    ") (" + m_OHBot->m_Language->Realm() + ": " +
                                    (LastMatch->GetSpoofedRealm().empty() ? "N/A" : LastMatch->GetSpoofedRealm()) +
                                    ")");
                    }
                    else
                        SendAllChat(m_OHBot->m_Language->UnableToCheckPlayerFoundMoreThanOneMatch(Payload));
                }
                else
                    SendAllChat("[" + User + "] (" + m_OHBot->m_Language->YourPingIsToday() + ": " +
                                (player->GetNumPings() > 0 ? UTIL_ToString(player->GetPing(m_OHBot->m_LCPings)) +
                                                             m_OHBot->m_Language->Ms() : "N/A") + ") " +
                                player->GetCLetter() == "" ? "" :
                                ("(" + m_OHBot->m_Language->Country() + ": " + player->GetCLetter() + ") ") + "(" +
                                m_OHBot->m_Language->Status() + ": " +
                                (LevelName.empty() ? m_OHBot->m_Language->Unknown() : LevelName) + ") (" +
                                m_OHBot->m_Language->SpoofChecked() + ": " +
                                (player->GetSpoofed() ? m_OHBot->m_Language->Yes() : m_OHBot->m_Language->No()) +
                                ") (" + m_OHBot->m_Language->Realm() + ": " +
                                (player->GetSpoofedRealm().empty() ? "N/A" : player->GetSpoofedRealm()) + ")");
            }

                //
                // !CHECKBAN
                //

            else if (Command == "checkban" && !Payload.empty()) {
                for (vector<CBNET *>::iterator i = m_OHBot->m_BNETs.begin(); i != m_OHBot->m_BNETs.end(); ++i)
                    m_PairedBanChecks.push_back(PairedBanCheck(User, m_OHBot->m_DB->ThreadedBanCheck((*i)->GetServer(),
                                                                                                     Payload,
                                                                                                     string())));
            }

                //
                // !CLEARHCL
                //

            else if (Command == "clearhcl" && !m_CountDownStarted && Level >= 8) {
                m_HCLCommandString.clear();
                SendAllChat(m_OHBot->m_Language->ClearingHCL());
            }

                //
                // !CLOSE (close slot)
                //

            else if (Command == "close" && !Payload.empty() && !m_GameLoading && !m_GameLoaded && Level >= 9) {
                // close as many slots as specified, e.g. "5 10" closes slots 5 and 10

                stringstream SS;
                SS << Payload;

                while (!SS.eof()) {
                    uint32_t SID;
                    SS >> SID;

                    if (SS.fail()) {
                        Log->Chat("[GAME: " + m_GameName + "] bad input to close command");
                        break;
                    }
                    else
                        CloseSlot((unsigned char) (SID - 1), true);
                }
            }

                //
                // !CLOSEALL
                //

            else if (Command == "closeall" && !m_GameLoading && !m_GameLoaded && Level >= 9)
                CloseAllSlots();

                //
                // !COMP (computer slot)
                //

            else if (Command == "comp" && !Payload.empty() && !m_GameLoading && !m_GameLoaded && !m_SaveGame &&
                     Level >= 8) {
                // extract the slot and the skill
                // e.g. "1 2" -> slot: "1", skill: "2"

                uint32_t Slot;
                uint32_t Skill = 1;
                stringstream SS;
                SS << Payload;
                SS >> Slot;

                if (SS.fail())
                    Log->Chat("[GAME: " + m_GameName + "] bad input #1 to comp command");
                else {
                    if (!SS.eof())
                        SS >> Skill;

                    if (SS.fail())
                        Log->Chat("[GAME: " + m_GameName + "] bad input #2 to comp command");
                    else
                        ComputerSlot((unsigned char) (Slot - 1), (unsigned char) Skill, true);
                }
            }

                //
                // !COMPCOLOUR (computer colour change)
                //

            else if (Command == "compcolour" && !Payload.empty() && !m_GameLoading && !m_GameLoaded && !m_SaveGame &&
                     Level >= 8) {
                // extract the slot and the colour
                // e.g. "1 2" -> slot: "1", colour: "2"

                uint32_t Slot;
                uint32_t Colour;
                stringstream SS;
                SS << Payload;
                SS >> Slot;

                if (SS.fail())
                    Log->Chat("[GAME: " + m_GameName + "] bad input #1 to compcolour command");
                else {
                    if (SS.eof())
                        Log->Chat("[GAME: " + m_GameName + "] missing input #2 to compcolour command");
                    else {
                        SS >> Colour;

                        if (SS.fail())
                            Log->Chat("[GAME: " + m_GameName + "] bad input #2 to compcolour command");
                        else {
                            unsigned char SID = (unsigned char) (Slot - 1);

                            if (!(m_Map->GetMapOptions() & MAPOPT_FIXEDPLAYERSETTINGS) && Colour < 12 &&
                                SID < m_Slots.size()) {
                                if (m_Slots[SID].GetSlotStatus() == SLOTSTATUS_OCCUPIED &&
                                    m_Slots[SID].GetComputer() == 1)
                                    ColourSlot(SID, Colour);
                            }
                        }
                    }
                }
            }

                //
                // !COMPHANDICAP (computer handicap change)
                //

            else if (Command == "comphandicap" && !Payload.empty() && !m_GameLoading && !m_GameLoaded && !m_SaveGame &&
                     Level >= 8) {
                // extract the slot and the handicap
                // e.g. "1 50" -> slot: "1", handicap: "50"

                uint32_t Slot;
                uint32_t Handicap;
                stringstream SS;
                SS << Payload;
                SS >> Slot;

                if (SS.fail())
                    Log->Chat("[GAME: " + m_GameName + "] bad input #1 to comphandicap command");
                else {
                    if (SS.eof())
                        Log->Chat("[GAME: " + m_GameName + "] missing input #2 to comphandicap command");
                    else {
                        SS >> Handicap;

                        if (SS.fail())
                            Log->Chat("[GAME: " + m_GameName + "] bad input #2 to comphandicap command");
                        else {
                            unsigned char SID = (unsigned char) (Slot - 1);

                            if (!(m_Map->GetMapOptions() & MAPOPT_FIXEDPLAYERSETTINGS) &&
                                (Handicap == 50 || Handicap == 60 || Handicap == 70 || Handicap == 80 ||
                                 Handicap == 90 || Handicap == 100) && SID < m_Slots.size()) {
                                if (m_Slots[SID].GetSlotStatus() == SLOTSTATUS_OCCUPIED &&
                                    m_Slots[SID].GetComputer() == 1) {
                                    m_Slots[SID].SetHandicap((unsigned char) Handicap);
                                    SendAllSlotInfo();
                                }
                            }
                        }
                    }
                }
            }

                //
                // !COMPRACE (computer race change)
                //

            else if (Command == "comprace" && !Payload.empty() && !m_GameLoading && !m_GameLoaded && !m_SaveGame &&
                     Level >= 8) {
                // extract the slot and the race
                // e.g. "1 human" -> slot: "1", race: "human"

                uint32_t Slot;
                string Race;
                stringstream SS;
                SS << Payload;
                SS >> Slot;

                if (SS.fail())
                    Log->Chat("[GAME: " + m_GameName + "] bad input #1 to comprace command");
                else {
                    if (SS.eof())
                        Log->Chat("[GAME: " + m_GameName + "] missing input #2 to comprace command");
                    else {
                        getline(SS, Race);
                        string::size_type Start = Race.find_first_not_of(" ");

                        if (Start != string::npos)
                            Race = Race.substr(Start);

                        transform(Race.begin(), Race.end(), Race.begin(), ::tolower);
                        unsigned char SID = (unsigned char) (Slot - 1);

                        if (!(m_Map->GetMapOptions() & MAPOPT_FIXEDPLAYERSETTINGS) &&
                            !(m_Map->GetMapFlags() & MAPFLAG_RANDOMRACES) && SID < m_Slots.size()) {
                            if (m_Slots[SID].GetSlotStatus() == SLOTSTATUS_OCCUPIED &&
                                m_Slots[SID].GetComputer() == 1) {
                                if (Race == "human") {
                                    m_Slots[SID].SetRace(SLOTRACE_HUMAN | SLOTRACE_SELECTABLE);
                                    SendAllSlotInfo();
                                }
                                else if (Race == "orc") {
                                    m_Slots[SID].SetRace(SLOTRACE_ORC | SLOTRACE_SELECTABLE);
                                    SendAllSlotInfo();
                                }
                                else if (Race == "night elf") {
                                    m_Slots[SID].SetRace(SLOTRACE_NIGHTELF | SLOTRACE_SELECTABLE);
                                    SendAllSlotInfo();
                                }
                                else if (Race == "undead") {
                                    m_Slots[SID].SetRace(SLOTRACE_UNDEAD | SLOTRACE_SELECTABLE);
                                    SendAllSlotInfo();
                                }
                                else if (Race == "random") {
                                    m_Slots[SID].SetRace(SLOTRACE_RANDOM | SLOTRACE_SELECTABLE);
                                    SendAllSlotInfo();
                                }
                                else
                                    Log->Chat("[GAME: " + m_GameName + "] unknown race [" + Race +
                                                  "] sent to comprace command");
                            }
                        }
                    }
                }
            }

                //
                // !COMPTEAM (computer team change)
                //

            else if (Command == "compteam" && !Payload.empty() && !m_GameLoading && !m_GameLoaded && !m_SaveGame &&
                     Level >= 8) {
                // extract the slot and the team
                // e.g. "1 2" -> slot: "1", team: "2"

                uint32_t Slot;
                uint32_t Team;
                stringstream SS;
                SS << Payload;
                SS >> Slot;

                if (SS.fail())
                    Log->Chat("[GAME: " + m_GameName + "] bad input #1 to compteam command");
                else {
                    if (SS.eof())
                        Log->Chat("[GAME: " + m_GameName + "] missing input #2 to compteam command");
                    else {
                        SS >> Team;

                        if (SS.fail())
                            Log->Chat("[GAME: " + m_GameName + "] bad input #2 to compteam command");
                        else {
                            unsigned char SID = (unsigned char) (Slot - 1);

                            if (!(m_Map->GetMapOptions() & MAPOPT_FIXEDPLAYERSETTINGS) && Team < 12 &&
                                SID < m_Slots.size()) {
                                if (m_Slots[SID].GetSlotStatus() == SLOTSTATUS_OCCUPIED &&
                                    m_Slots[SID].GetComputer() == 1) {
                                    m_Slots[SID].SetTeam((unsigned char) (Team - 1));
                                    SendAllSlotInfo();
                                }
                            }
                        }
                    }
                }
            }

                //
                // !DBSTATUS
                //

            else if (Command == "dbstatus" && Level >= 9)
                SendAllChat(m_OHBot->m_DB->GetStatus());

                //
                // !DOWNLOAD
                // !DL
                //

            else if ((Command == "download" || Command == "dl") && !Payload.empty() && !m_GameLoading &&
                     !m_GameLoaded && Level >= 8) {
                CGamePlayer *LastMatch = NULL;
                uint32_t Matches = GetPlayerFromNamePartial(Payload, &LastMatch);

                if (Matches == 0)
                    SendAllChat(m_OHBot->m_Language->UnableToStartDownloadNoMatchesFound(Payload));
                else if (Matches == 1) {
                    if (!LastMatch->GetDownloadStarted() && !LastMatch->GetDownloadFinished()) {
                        unsigned char SID = GetSIDFromPID(LastMatch->GetPID());

                        if (SID < m_Slots.size() && m_Slots[SID].GetDownloadStatus() != 100) {
                            // inform the client that we are willing to send the map

                            Log->Chat("[GAME: " + m_GameName + "] map download started for player [" +
                                          LastMatch->GetName() + "]");
                            Send(LastMatch, m_Protocol->SEND_W3GS_STARTDOWNLOAD(GetHostPID()));
                            LastMatch->SetDownloadAllowed(true);
                            LastMatch->SetDownloadStarted(true);
                            LastMatch->SetStartedDownloadingTicks(GetTicks());
                        }
                    }
                }
                else
                    SendAllChat(m_OHBot->m_Language->UnableToStartDownloadFoundMoreThanOneMatch(Payload));
            }

                //
                // !DROP
                //

            else if (Command == "drop" && m_GameLoaded && (Level >= 5 || hasAccess))
                StopLaggers("lagged out (dropped by admin)");

                //
                // !END
                //

            else if (Command == "end" && m_GameLoaded && Level >= 8) {
                Log->Info("[GAME: " + m_GameName + "] is over (admin ended game)");
                StopPlayers("was disconnected (admin ended game)");
            }

                //
                // !FAKEPLAYER
                //

            else if (Command == "fakeplayer" && !m_CountDownStarted && Level >= 8 && !m_OHBot->m_ObserverFake) {
                if (m_FakePlayerPID == 255)
                    CreateFakePlayer();
                else
                    DeleteFakePlayer();
            }

                //
                // !FROM
                //

            else if (Command == "from" && (Level >= 5 || hasAccess)) {
                string Froms;

                for (vector<CGamePlayer *>::iterator i = m_Players.begin(); i != m_Players.end(); ++i) {
                    if ((*i)->GetCLetter() != "" && (*i)->GetCountry() != "") {
                        // we reverse the byte order on the IP because it's stored in network byte order
                        Froms += (*i)->GetNameTerminated();
                        Froms += ": (";
                        Froms += (*i)->GetCLetter() + "|" + (*i)->GetCountry();
                        Froms += ")";
                        if (i != m_Players.end() - 1)
                            Froms += ", ";
                        if ((m_GameLoading || m_GameLoaded) && Froms.size() > 100) {
                            // cut the text into multiple lines ingame
                            SendAllChat(Froms);
                            Froms.clear();
                        }
                    }
                }
                if (!Froms.empty())
                    SendAllChat(Froms);
            }

                //
                // !HCL
                //

            else if (Command == "hcl" && !m_CountDownStarted && Level >= 8) {
                if (!Payload.empty()) {
                    if (Payload.size() <= m_Slots.size()) {
                        string HCLChars = "abcdefghijklmnopqrstuvwxyz0123456789 -=,.";

                        if (Payload.find_first_not_of(HCLChars) == string::npos) {
                            m_HCLCommandString = Payload;
                            SendAllChat(m_OHBot->m_Language->SettingHCL(m_HCLCommandString));
                        }
                        else
                            SendAllChat(m_OHBot->m_Language->UnableToSetHCLInvalid());
                    }
                    else
                        SendAllChat(m_OHBot->m_Language->UnableToSetHCLTooLong());
                }
                else
                    SendAllChat(m_OHBot->m_Language->TheHCLIs(m_HCLCommandString));
            }

                //
                // !HOLD (hold a slot for someone)
                //

            else if (Command == "hold" && !Payload.empty() && !m_GameLoading && !m_GameLoaded && Level >= 8) {
                // hold as many players as specified, e.g. "Varlock Kilranin" holds players "Varlock" and "Kilranin"

                stringstream SS;
                SS << Payload;

                while (!SS.eof()) {
                    string HoldName;
                    SS >> HoldName;

                    if (SS.fail()) {
                        Log->Chat("[GAME: " + m_GameName + "] bad input to hold command");
                        break;
                    }
                    else {
                        SendAllChat(m_OHBot->m_Language->AddedPlayerToTheHoldList(HoldName));
                        AddToReserved(HoldName, 255, 1);
                    }
                }
            }

                //
                // !HOLDSLOT
                //

            else if ((Command == "holdslot" || Command == "hs") && !Payload.empty() && !m_GameLoading &&
                     !m_GameLoaded && Level >= 9) {
                string username;
                uint32_t playersid;
                stringstream SS;
                SS << Payload;
                SS >> username;

                if (SS.fail()) {
                    SendChat(player, "Bad input for holdslot. !hs <name> <slot>");
                    return true;
                } else {
                    SS >> playersid;
                    if (SS.fail()) {
                        SendChat(player, "Bad input for holdslot. !hs <name> <slot>");
                        return true;
                    } else {
                        if (playersid < m_Slots.size()) {
                            SendAllChat(
                                    "Reserving slot [" + UTIL_ToString(playersid) + "] for user [" + username + "]");
                            AddToReserved(username, (unsigned char) (playersid - 1), 1);
                        }
                        else {
                            SendChat(player, "Bad input for holdslot.");
                        }
                    }
                }

            }

                //
                // !KICK (kick a player) !K
                //

            else if ((Command == "kick" || Command == "k" || Command == "skick" || Command == "sk") &&
                     !Payload.empty() && Level >= 5) {
                CGamePlayer *LastMatch = NULL;
                uint32_t Matches = GetPlayerFromNamePartial(Payload, &LastMatch);

                if (Matches == 0)
                    SendAllChat(m_OHBot->m_Language->UnableToKickNoMatchesFound(Payload));
                else if (Matches == 1) {
                    uint32_t VictimLevel = LastMatch->GetLevel();
                    string VictimLevelName = LastMatch->GetLevelName();
                    if (VictimLevel <= 1 || Level >= 9) {
                        LastMatch->SetDeleteMe(true);
                        if (Command != "skick" || Command != "sk") {
                            LastMatch->SetLeftReason(m_OHBot->m_Language->WasKickedByPlayer(User));

                            if (!m_GameLoading && !m_GameLoaded)
                                LastMatch->SetLeftCode(PLAYERLEAVE_LOBBY);
                            else
                                LastMatch->SetLeftCode(PLAYERLEAVE_LOST);
                        } else {
                            LastMatch->SetLeftReason(m_OHBot->m_Language->HasLostConnectionClosedByRemoteHost());
                            LastMatch->SetLeftCode(PLAYERLEAVE_DISCONNECT);
                            return true;
                        }
                        if (!m_GameLoading && !m_GameLoaded) {
                            OpenSlot(GetSIDFromPID(LastMatch->GetPID()), false);
                            m_Balanced = false;
                        }
                    }
                    else
                        SendChat(player, m_OHBot->m_Language->NoPermissionToExecCommand());
                }
                else
                    SendAllChat(m_OHBot->m_Language->UnableToKickFoundMoreThanOneMatch(Payload));
            }

                //
                // !LATENCY (set game latency)
                //

            else if (Command == "latency" && Level >= 9) {
                if (Payload.empty())
                    SendAllChat(m_OHBot->m_Language->LatencyIs(UTIL_ToString(m_Latency)));
                else {
                    m_Latency = UTIL_ToUInt32(Payload);

                    if (m_Latency <= 20) {
                        m_Latency = 20;
                        SendAllChat(m_OHBot->m_Language->SettingLatencyToMinimum("20"));
                    }
                    else if (m_Latency >= 500) {
                        m_Latency = 500;
                        SendAllChat(m_OHBot->m_Language->SettingLatencyToMaximum("500"));
                    }
                    else
                        SendAllChat(m_OHBot->m_Language->SettingLatencyTo(UTIL_ToString(m_Latency)));
                }
            }

                //
                // !GAMELOCK
                //

            else if (Command == "gamelock" && Level >= 9) {
                SendAllChat(m_OHBot->m_Language->GameLocked());
                m_Locked = true;
            }

                //
                // !MESSAGES
                //

            else if (Command == "messages" && Level >= 9) {
                if (Payload == "on") {
                    SendAllChat(m_OHBot->m_Language->LocalAdminMessagesEnabled());
                    m_LocalAdminMessages = true;
                }
                else if (Payload == "off") {
                    SendAllChat(m_OHBot->m_Language->LocalAdminMessagesDisabled());
                    m_LocalAdminMessages = false;
                }
            }

                //
                // !OHBALANCE
                //

            else if (Command == "ohbalance" && Level >= 9 && !Payload.empty()) {
                if (Payload == "on") {
                    SendAllChat(m_OHBot->m_Language->EnabledAutobalance());
                    m_OHBot->m_OHBalance = true;
                }
                else if (Payload == "off") {
                    SendAllChat(m_OHBot->m_Language->DisabledAutobalance());
                    m_OHBot->m_OHBalance = false;
                }
            }

                //
                // !MUTE
                //

            else if (Command == "mute" && (Level >= 5 || hasAccess)) {
                CGamePlayer *LastMatch = NULL;
                uint32_t Matches = GetPlayerFromNamePartial(Payload, &LastMatch);

                if (Matches == 0)
                    SendAllChat(m_OHBot->m_Language->UnableToMuteNoMatchesFound(Payload));
                else if (Matches == 1) {
                    uint32_t VictimLevel = LastMatch->GetLevel();
                    string VictimLevelName = LastMatch->GetLevelName();
                    if (VictimLevel <= 1 || Level >= 9) {
                        SendAllChat(m_OHBot->m_Language->MutedPlayer(LastMatch->GetName(), User));
                        LastMatch->SetMuted(true);
                    }
                    else
                        SendChat(player, m_OHBot->m_Language->NoPermissionToExecCommand());
                }
                else
                    SendAllChat(m_OHBot->m_Language->UnableToMuteFoundMoreThanOneMatch(Payload));
            }

                //
                // !MUTEALL
                //

            else if (Command == "muteall" && m_GameLoaded && Level >= 5) {
                if (Payload.empty()) {
                    SendAllChat(m_OHBot->m_Language->GlobalChatMuted());
                    m_MuteAll = true;
                } else {
                    CGamePlayer *LastMatch = NULL;
                    uint32_t Matches = GetPlayerFromNamePartial(Payload, &LastMatch);
                    if (Matches == 0)
                        SendAllChat(m_OHBot->m_Language->UnableToMuteNoMatchesFound(Payload));
                    else if (Matches == 1) {
                        uint32_t VictimLevel = LastMatch->GetLevel();
                        string VictimLevelName = LastMatch->GetLevelName();
                        if (VictimLevel <= 1 || Level >= 9) {
                            SendAllChat(m_OHBot->m_Language->MutedAllChatOfUser(LastMatch->GetName(), User));
                            LastMatch->SetGlobalChatMuted(true);
                        }
                        else
                            SendChat(player, m_OHBot->m_Language->NoPermissionToExecCommand());
                    }
                    else
                        SendAllChat(m_OHBot->m_Language->UnableToMuteFoundMoreThanOneMatch(Payload));
                }
            }

                //
                // !OPEN (open slot)
                //

            else if (Command == "open" && !Payload.empty() && !m_GameLoading && !m_GameLoaded && Level >= 8) {
                // open as many slots as specified, e.g. "5 10" opens slots 5 and 10

                stringstream SS;
                SS << Payload;

                while (!SS.eof()) {
                    uint32_t SID;
                    SS >> SID;

                    if (SS.fail()) {
                        Log->Chat("[GAME: " + m_GameName + "] bad input to open command");
                        break;
                    }
                    else {
                        OpenSlot((unsigned char) (SID - 1), true);
                        m_Balanced = false;
                    }
                }
            }

                //
                // !OPENALL
                //

            else if (Command == "openall" && !m_GameLoading && !m_GameLoaded && Level >= 9)
                OpenAllSlots();

                //
                // !OWNER (set game owner)
                //

            else if (Command == "owner" && Level >= 9) {
                if (Level >= 8 || !GetPlayerFromName(m_OwnerName, false)) {
                    if (!Payload.empty()) {
                        SendAllChat(m_OHBot->m_Language->SettingGameOwnerTo(Payload));
                        m_OwnerName = Payload;
                    }
                    else {
                        SendAllChat(m_OHBot->m_Language->SettingGameOwnerTo(User));
                        m_OwnerName = User;
                    }
                }
                else
                    SendAllChat(m_OHBot->m_Language->UnableToSetGameOwner(m_OwnerName));
            }

                //
                // !PING
                //

            else if (Command == "ping" && Level >= 5) {
                // kick players with ping higher than payload if payload isn't empty
                // we only do this if the game hasn't started since we don't want to kick players from a game in progress

                uint32_t Kicked = 0;
                uint32_t KickPing = 0;

                if (!m_GameLoading && !m_GameLoaded && !Payload.empty())
                    KickPing = UTIL_ToUInt32(Payload);

                // copy the m_Players vector so we can sort by descending ping so it's easier to find players with high pings

                vector<CGamePlayer *> SortedPlayers = m_Players;
                sort(SortedPlayers.begin(), SortedPlayers.end(), CGamePlayerSortDescByPing());
                string Pings;

                for (vector<CGamePlayer *>::iterator i = SortedPlayers.begin(); i != SortedPlayers.end(); ++i) {
                    Pings += (*i)->GetNameTerminated();
                    Pings += ": ";

                    if ((*i)->GetNumPings() > 0) {
                        Pings += UTIL_ToString((*i)->GetPing(m_OHBot->m_LCPings));

                        if (!m_GameLoading && !m_GameLoaded && !(*i)->GetReserved() && KickPing > 0 &&
                            (*i)->GetPing(m_OHBot->m_LCPings) > KickPing) {
                            (*i)->SetDeleteMe(true);
                            (*i)->SetLeftReason("was kicked for excessive ping " +
                                                UTIL_ToString((*i)->GetPing(m_OHBot->m_LCPings)) + " > " +
                                                UTIL_ToString(KickPing));
                            (*i)->SetLeftCode(PLAYERLEAVE_LOBBY);
                            OpenSlot(GetSIDFromPID((*i)->GetPID()), false);
                            Kicked++;
                        }

                        Pings += m_OHBot->m_Language->Ms();
                    }
                    else
                        Pings += "N/A";

                    if (i != SortedPlayers.end() - 1)
                        Pings += ", ";

                    if ((m_GameLoading || m_GameLoaded) && Pings.size() > 100) {
                        // cut the text into multiple lines ingame

                        SendAllChat(Pings);
                        Pings.clear();
                    }
                }

                if (!Pings.empty())
                    SendAllChat(Pings);

                if (Kicked > 0)
                    SendAllChat(m_OHBot->m_Language->KickingPlayersWithPingsGreaterThan(UTIL_ToString(Kicked),
                                                                                        UTIL_ToString(KickPing)));
            }

                //
                // !PRIV (rehost as private game)
                //

            else if (Command == "priv" && !Payload.empty() && !m_CountDownStarted && !m_SaveGame && Level >= 8) {
                if (Payload.length() < 31) {
                    Log->Info("[GAME: " + m_GameName + "] trying to rehost as private game [" + Payload + "]");
                    SendAllChat(m_OHBot->m_Language->TryingToRehostAsPrivateGame(Payload));
                    m_GameState = GAME_PRIVATE;
                    m_LastGameName = m_GameName;
                    m_GameName = Payload;
                    m_RefreshError = false;
                    m_RefreshRehosted = true;

                    for (vector<CBNET *>::iterator i = m_OHBot->m_BNETs.begin(); i != m_OHBot->m_BNETs.end(); ++i) {
                        // unqueue any existing game refreshes because we're going to assume the next successful game refresh indicates that the rehost worked
                        // this ignores the fact that it's possible a game refresh was just sent and no response has been received yet
                        // we assume this won't happen very often since the only downside is a potential false positive

                        (*i)->UnqueueGameRefreshes();
                        (*i)->QueueGameUncreate();
                        (*i)->QueueEnterChat();

                        // we need to send the game creation message now because private games are not refreshed

                        (*i)->QueueGameCreate(m_GameState, m_GameName, string(), m_Map, NULL, m_HostCounter);

                        if ((*i)->GetPasswordHashType() != "pvpgn")
                            (*i)->QueueEnterChat();
                    }

                    m_CreationTime = GetTime();
                    m_LastRefreshTime = GetTime();
                }
                else
                    SendAllChat(m_OHBot->m_Language->UnableToCreateGameNameTooLong(Payload));
            }

                //
                // !PUB (rehost as public game)
                //

            else if (Command == "pub" && !Payload.empty() && !m_CountDownStarted && !m_SaveGame && Level >= 8) {
                if (Payload.length() < 31) {
                    Log->Info("[GAME: " + m_GameName + "] trying to rehost as public game [" + Payload + "]");
                    SendAllChat(m_OHBot->m_Language->TryingToRehostAsPublicGame(Payload));
                    m_GameState = GAME_PUBLIC;
                    m_LastGameName = m_GameName;
                    m_GameName = Payload;
                    m_RefreshError = false;
                    m_RefreshRehosted = true;
                    for (vector<CBNET *>::iterator i = m_OHBot->m_BNETs.begin(); i != m_OHBot->m_BNETs.end(); ++i) {
                        // unqueue any existing game refreshes because we're going to assume the next successful game refresh indicates that the rehost worked
                        // this ignores the fact that it's possible a game refresh was just sent and no response has been received yet
                        // we assume this won't happen very often since the only downside is a potential false positive

                        (*i)->UnqueueGameRefreshes();
                        (*i)->QueueGameUncreate();
                        (*i)->QueueEnterChat();

                        // the game creation message will be sent on the next refresh
                    }

                    m_CreationTime = GetTime();
                    m_LastRefreshTime = GetTime();
                }
                else
                    SendAllChat(m_OHBot->m_Language->UnableToCreateGameNameTooLong(Payload));
            }

                //
                // !REFRESH (turn on or off refresh messages)
                //

            else if (Command == "refresh" && !m_CountDownStarted && Level == 10) {
                if (Payload == "on") {
                    SendAllChat(m_OHBot->m_Language->RefreshMessagesEnabled());
                    m_RefreshMessages = true;
                }
                else if (Payload == "off") {
                    SendAllChat(m_OHBot->m_Language->RefreshMessagesDisabled());
                    m_RefreshMessages = false;
                }
            }

                //
                // !SAY
                //

            else if (Command == "say" && !Payload.empty() && Level >= 8) {
                for (vector<CBNET *>::iterator i = m_OHBot->m_BNETs.begin(); i != m_OHBot->m_BNETs.end(); ++i)
                    (*i)->QueueChatCommand(Payload);

                HideCommand = true;
            }

                //
                // !SENDLAN
                //

            else if (Command == "sendlan" && !Payload.empty() && !m_CountDownStarted && Level >= 9 && m_OHBot->IsMode(BOT_MODE_LAN)) {
                // extract the ip and the port
                // e.g. "1.2.3.4 6112" -> ip: "1.2.3.4", port: "6112"

                string IP;
                uint32_t Port = m_OHBot->m_BroadCastPort;
                stringstream SS;
                SS << Payload;
                SS >> IP;

                if (!SS.eof())
                    SS >> Port;

                if (SS.fail())
                    Log->Chat("[GAME: " + m_GameName + "] bad inputs to sendlan command");
                else {
                    // construct a fixed host counter which will be used to identify players from this "realm" (i.e. LAN)
                    // the fixed host counter's 4 most significant bits will contain a 4 bit ID (0-15)
                    // the rest of the fixed host counter will contain the 28 least significant bits of the actual host counter
                    // since we're destroying 4 bits of information here the actual host counter should not be greater than 2^28 which is a reasonable assumption
                    // when a player joins a game we can obtain the ID from the received host counter
                    // note: LAN broadcasts use an ID of 0, battle.net refreshes use an ID of 1-10, the rest are unused

                    uint32_t FixedHostCounter = m_HostCounter & 0x0FFFFFFF;

                    // we send 12 for SlotsTotal because this determines how many PID's Warcraft 3 allocates
                    // we need to make sure Warcraft 3 allocates at least SlotsTotal + 1 but at most 12 PID's
                    // this is because we need an extra PID for the virtual host player (but we always delete the virtual host player when the 12th person joins)
                    // however, we can't send 13 for SlotsTotal because this causes Warcraft 3 to crash when sharing control of units
                    // nor can we send SlotsTotal because then Warcraft 3 crashes when playing maps with less than 12 PID's (because of the virtual host player taking an extra PID)
                    // we also send 12 for SlotsOpen because Warcraft 3 assumes there's always at least one player in the game (the host)
                    // so if we try to send accurate numbers it'll always be off by one and results in Warcraft 3 assuming the game is full when it still needs one more player
                    // the easiest solution is to simply send 12 for both so the game will always show up as (1/12) players

                    if (m_SaveGame) {
                        // note: the PrivateGame flag is not set when broadcasting to LAN (as you might expect)

                        uint32_t MapGameType = MAPGAMETYPE_SAVEDGAME;
                        BYTEARRAY MapWidth;
                        MapWidth.push_back(0);
                        MapWidth.push_back(0);
                        BYTEARRAY MapHeight;
                        MapHeight.push_back(0);
                        MapHeight.push_back(0);
                        m_OHBot->m_UDPSocket->SendTo(IP, Port, m_Protocol->SEND_W3GS_GAMEINFO(m_OHBot->m_TFT,
                                                                                              m_OHBot->m_LANWar3Version,
                                                                                              UTIL_CreateByteArray(
                                                                                                      MapGameType,
                                                                                                      false),
                                                                                              m_Map->GetMapGameFlags(),
                                                                                              MapWidth, MapHeight,
                                                                                              m_GameName,
                                                                                              m_OHBot->m_BNETs[0]->GetUserName(),
                                                                                              GetTime() -
                                                                                              m_CreationTime,
                                                                                              "Save\\Multiplayer\\" +
                                                                                              m_SaveGame->GetFileNameNoPath(),
                                                                                              m_SaveGame->GetMagicNumber(),
                                                                                              12, 12, m_HostPort,
                                                                                              FixedHostCounter,
                                                                                              m_EntryKey));
                    }
                    else {
                        // note: the PrivateGame flag is not set when broadcasting to LAN (as you might expect)
                        // note: we do not use m_Map->GetMapGameType because none of the filters are set when broadcasting to LAN (also as you might expect)

                        uint32_t MapGameType = MAPGAMETYPE_UNKNOWN0;
                        m_OHBot->m_UDPSocket->SendTo(IP, Port, m_Protocol->SEND_W3GS_GAMEINFO(m_OHBot->m_TFT,
                                                                                              m_OHBot->m_LANWar3Version,
                                                                                              UTIL_CreateByteArray(
                                                                                                      MapGameType,
                                                                                                      false),
                                                                                              m_Map->GetMapGameFlags(),
                                                                                              m_Map->GetMapWidth(),
                                                                                              m_Map->GetMapHeight(),
                                                                                              m_GameName,
                                                                                              m_OHBot->m_BNETs[0]->GetUserName(),
                                                                                              GetTime() -
                                                                                              m_CreationTime,
                                                                                              m_Map->GetMapPath(),
                                                                                              m_Map->GetMapCRC(), 12,
                                                                                              12, m_HostPort,
                                                                                              FixedHostCounter,
                                                                                              m_EntryKey));
                    }
                }
            }

                //
                // !SP
                //

            else if (Command == "sp" && !m_CountDownStarted && Level >= 8) {
                SendAllChat(m_OHBot->m_Language->ShufflingPlayers());
                ShuffleSlots();
            }

                //
                // !START
                //

            else if (Command == "start" && !m_CountDownStarted && Level >= 6) {
                // if the player sent "!start force" skip the checks and start the countdown
                // otherwise check that the game is ready to start

                if (Payload == "force")
                    StartCountDown(true);
                else {
                    if (GetTicks() - m_LastPlayerLeaveTicks >= 2000)
                        StartCountDown(false);
                    else
                        SendAllChat(m_OHBot->m_Language->CountDownAbortedSomeoneLeftRecently());
                }
            }

                //
                // !SWAP (swap slots)
                //

            else if (Command == "swap" && !Payload.empty() && !m_GameLoading && !m_GameLoaded && !m_CountDownStarted &&
                     (Level >= 5 || hasAccess)) {
                if (hasAccess) {
                    if (player->GetSwapLimit() == 0) {
                        SendChat(player, "You reached already your maximum swap limit");
                        return false;
                    } else {
                        player->SetSwapLimit();
                    }
                }
                uint32_t SID1;
                uint32_t SID2;
                stringstream SS;
                SS << Payload;
                SS >> SID1;

                if (SS.fail())
                    Log->Chat("[GAME: " + m_GameName + "] bad input #1 to swap command");
                else {
                    if (SS.eof())
                        Log->Chat("[GAME: " + m_GameName + "] missing input #2 to swap command");
                    else {
                        SS >> SID2;

                        if (SS.fail())
                            Log->Chat("[GAME: " + m_GameName + "] bad input #2 to swap command");
                        else {
                            SwapSlots((unsigned char) (SID1 - 1), (unsigned char) (SID2 - 1));
                            m_Balanced = false;
                        }
                    }
                }
            }

                //
                // !SYNCLIMIT
                //

            else if (Command == "synclimit" && Level >= 9) {
                if (Payload.empty())
                    SendAllChat(m_OHBot->m_Language->SyncLimitIs(UTIL_ToString(m_SyncLimit)));
                else {
                    m_SyncLimit = UTIL_ToUInt32(Payload);

                    if (m_SyncLimit <= 10) {
                        m_SyncLimit = 10;
                        SendAllChat(m_OHBot->m_Language->SettingSyncLimitToMinimum("10"));
                    }
                    else if (m_SyncLimit >= 10000) {
                        m_SyncLimit = 10000;
                        SendAllChat(m_OHBot->m_Language->SettingSyncLimitToMaximum("10000"));
                    }
                    else
                        SendAllChat(m_OHBot->m_Language->SettingSyncLimitTo(UTIL_ToString(m_SyncLimit)));
                }
            }

                //
                // !UNHOST
                //

            else if (Command == "unhost" && !m_CountDownStarted && Level >= 7)
                m_Exiting = true;

                //
                // !GAMEUNLOCK
                //

            else if (Command == "gameunlock" && Level >= 9) {
                SendAllChat(m_OHBot->m_Language->GameUnlocked());
                m_Locked = false;
            }

                //
                // !UNMUTE
                //

            else if (Command == "unmute" && (Level >= 5 || hasAccess)) {
                CGamePlayer *LastMatch = NULL;
                uint32_t Matches = GetPlayerFromNamePartial(Payload, &LastMatch);

                if (Matches == 0)
                    SendAllChat(m_OHBot->m_Language->UnableToMuteNoMatchesFound(Payload));
                else if (Matches == 1) {
                    SendAllChat(m_OHBot->m_Language->UnmutedPlayer(LastMatch->GetName(), User));
                    LastMatch->SetMuted(false);
                }
                else
                    SendAllChat(m_OHBot->m_Language->UnableToMuteFoundMoreThanOneMatch(Payload));
            }

                //
                // !UNMUTEALL
                //

            else if (Command == "unmuteall" && m_GameLoaded && Level >= 5) {
                if (Payload.empty()) {
                    SendAllChat(m_OHBot->m_Language->GlobalChatUnmuted());
                    m_MuteAll = false;
                } else {
                    CGamePlayer *LastMatch = NULL;
                    uint32_t Matches = GetPlayerFromNamePartial(Payload, &LastMatch);
                    if (Matches == 0)
                        SendAllChat(m_OHBot->m_Language->UnableToMuteNoMatchesFound(Payload));
                    else if (Matches == 1) {
                        uint32_t VictimLevel = LastMatch->GetLevel();
                        string VictimLevelName = LastMatch->GetLevelName();
                        if (VictimLevel <= 1 || Level >= 9) {
                            SendAllChat(m_OHBot->m_Language->UnMutedAllChatOfUser(LastMatch->GetName(), User));
                            LastMatch->SetGlobalChatMuted(false);
                        }
                        else
                            SendChat(player, m_OHBot->m_Language->NoPermissionToExecCommand());
                    }
                    else
                        SendAllChat(m_OHBot->m_Language->UnableToMuteFoundMoreThanOneMatch(Payload));
                }

            }

                //
                // !VIRTUALHOST
                //

            else if (Command == "virtualhost" && !Payload.empty() && Payload.size() <= 15 && !m_CountDownStarted &&
                     Level >= 9) {
                DeleteVirtualHost();
                m_VirtualHostName = Payload;
            }

                //
                // !VOTECANCEL
                //

            else if (Command == "votecancel" && !m_KickVotePlayer.empty() && Level >= 8) {
                SendAllChat(m_OHBot->m_Language->VoteKickCancelled(m_KickVotePlayer));
                m_KickVotePlayer.clear();
                m_StartedKickVoteTime = 0;
            }

                //
                // !W
                //

            else if (Command == "w" && !Payload.empty() && Level >= 9) {
                // extract the name and the message
                // e.g. "Varlock hello there!" -> name: "Varlock", message: "hello there!"

                string Name;
                string Message;
                string::size_type MessageStart = Payload.find(" ");

                if (MessageStart != string::npos) {
                    Name = Payload.substr(0, MessageStart);
                    Message = Payload.substr(MessageStart + 1);

                    for (vector<CBNET *>::iterator i = m_OHBot->m_BNETs.begin(); i != m_OHBot->m_BNETs.end(); ++i)
                        (*i)->QueueChatCommand(Message, Name, true);
                }

                HideCommand = true;
            }
                //
                // !WINNER
                //

            else if (Command == "winner" && m_GameLoaded && Level >= 9) {
                m_Stats->SetWinner(UTIL_ToUInt32(Payload));
                SendAllChat(m_OHBot->m_Language->SetWinnerByUser(User, (Payload == "1" ? "Sentinel" : "Scourge")));
                m_GameOverTime = GetTime();
                m_EndGame = false;
            }
        }
        else {
            Log->Chat("[GAME: " + m_GameName + "] admin command ignored, the game is locked");
            SendChat(player, m_OHBot->m_Language->TheGameIsLocked());
        }
    }
    else if (!m_OHBot->m_RanksLoaded)
        Log->Write(m_OHBot->m_Language->RanksNotLoaded());
    else {
        if (!spoofed)
            Log->Chat("[GAME: " + m_GameName + "] non-spoofchecked user [" + User + "] sent command [" + Command +
                          "] with payload [" + Payload + "]");
        else
            Log->Chat("[GAME: " + m_GameName + "] non-admin [" + User + "] sent command [" + Command +
                          "] with payload [" + Payload + "]");
    }

    /*********************
    * NON ADMIN COMMANDS *
    *********************/

    //
    // !CHECKME
    //

    if (Command == "checkme" || Command == "cm") {
        SendChat(player, "[" + User + "] (" + m_OHBot->m_Language->YourPingIsToday() + ": " +
                         (player->GetNumPings() > 0 ? UTIL_ToString(player->GetPing(m_OHBot->m_LCPings)) +
                                                      m_OHBot->m_Language->Ms() : "N/A") + ") (" +
                         m_OHBot->m_Language->Country() + ": " + player->GetCLetter() + ") (" +
                         m_OHBot->m_Language->Status() + ": " +
                         (LevelName.empty() ? m_OHBot->m_Language->Unknown() : LevelName) + ") (" +
                         m_OHBot->m_Language->SpoofChecked() + ": " +
                         (player->GetSpoofed() ? m_OHBot->m_Language->Yes() : m_OHBot->m_Language->No()) + ") (" +
                         m_OHBot->m_Language->Realm() + ": " +
                         (player->GetSpoofedRealm().empty() ? "N/A" : player->GetSpoofedRealm()) + ")");
        if (player->GetForfeitVote())
            SendChat(player, m_OHBot->m_Language->UserAlreadyVotedForFF());
        if (player->GetDrawVote())
            SendChat(player, m_OHBot->m_Language->UserAlreadyVotedForDraw());
        string IgnoredPlayers;
        for (vector<string>::iterator i = player->m_IgnoreList.begin(); i != player->m_IgnoreList.end(); ++i) {
            if (IgnoredPlayers.empty())
                IgnoredPlayers = *i;
            else
                IgnoredPlayers = ", " + *i;
        }
        if (!IgnoredPlayers.empty())
            SendChat(player, m_OHBot->m_Language->DisplayIgnoredPlayersForUser(IgnoredPlayers));
        if (player->GetAFKMarked())
            SendChat(player, m_OHBot->m_Language->AlreadyMarkedAsAFK());
        if (player->GetFeedLevel() == 1)
            SendChat(player, m_OHBot->m_Language->AlreadyMarkedAsFeeder());
        if (player->GetFeedLevel() == 2)
            SendChat(player, m_OHBot->m_Language->AlreadyMarkedAsFullFeeder());
        if (player->GetHighPingTimes() > 0)
            SendChat(player,
                     m_OHBot->m_Language->AlreadyMarkedXTimesWithHighPing(UTIL_ToString(player->GetHighPingTimes())));
    }
        //
        // !STATS
        //

        /**
     * All over statistics, games isnt relating to any of the gamealiases, only detailed statistics should be shown
     * in relation to the gamealias.
     */
    else if (Command == "stats" && GetTime() - player->GetStatsSentTime() >= 5) {
        string StatsUser = User;
        string Month = "";
        string Year = "";
        if (!Payload.empty()) {
            stringstream SS;
            SS << Payload;
            SS >> StatsUser;
            SS >> Month;
            SS >> Year;
        }
        CGamePlayer *LastMatch = NULL;
        uint32_t Matches = GetPlayerFromNamePartial(StatsUser, &LastMatch);
        if (Matches == 0) {
            if (spoofed && Level >= 8)
                m_PairedGSChecks.push_back(PairedGSCheck(string(),
                                                         m_OHBot->m_DB->ThreadedStatsPlayerSummaryCheck(StatsUser,
                                                                                                        Month, Year,
                                                                                                        0)));
            else
                m_PairedGSChecks.push_back(
                        PairedGSCheck(User, m_OHBot->m_DB->ThreadedStatsPlayerSummaryCheck(StatsUser, Month, Year, 0)));
        }
        else if (Matches == 1) {
            if (spoofed && Level >= 8)
                m_PairedGSChecks.push_back(PairedGSCheck(string(), m_OHBot->m_DB->ThreadedStatsPlayerSummaryCheck(
                        LastMatch->GetName(), Month, Year, 0)));
            else
                m_PairedGSChecks.push_back(PairedGSCheck(User, m_OHBot->m_DB->ThreadedStatsPlayerSummaryCheck(
                        LastMatch->GetName(), Month, Year, 0)));
        }
        else if (Matches > 1)
            SendChat(player, m_OHBot->m_Language->FoundMultiplyMatches());

        player->SetStatsSentTime(GetTime());
    }

        //
        // !RANKDOTA
        // !RANKLOD
        // !RANKIMBA
        // !RD
        // !RL
        // !RI
        //

    else if ((Command.substr(0, 4) == "rank" || (Command == "rd" || Command == "rl" || Command == "ri")) &&
             GetTime() - player->GetStatsSentTime() >= 5) {
        uint32_t m_StatsAlias = 0;
        string alias = "";
        if (Command.size() > 4)
            alias = Command.substr(4, Command.size() - 4);
        else if (Command.size() < 4)
            alias = Command.substr(1, Command.size() - 1);

        m_StatsAlias = m_OHBot->GetStatsAliasNumber(alias);

        if (m_StatsAlias == 0) {
            SendChat(player, m_OHBot->m_Language->DidNotFoundAlias(alias));
            if (m_OHBot->m_Aliases.size() != 0 && m_OHBot->m_Aliases.size() > m_GameAlias) {
                SendChat(player, m_OHBot->m_Language->UsingDefaultAlias(m_OHBot->m_Aliases[m_GameAlias - 1]));
            } else {
                SendChat(player, m_OHBot->m_Language->LostConnectionPleaseTryLater());
            }
            m_StatsAlias = m_GameAlias;
        }

        if (m_StatsAlias != 0) {
            string StatsUser = User;
            string Month = "";
            string Year = "";
            if (!Payload.empty()) {
                stringstream SS;
                SS << Payload;
                SS >> StatsUser;
                SS >> Month;
                SS >> Year;
            }
            CGamePlayer *LastMatch = NULL;
            uint32_t Matches = GetPlayerFromNamePartial(StatsUser, &LastMatch);
            if (Matches == 0) {
                if (spoofed && Level >= 8)
                    m_PairedRankChecks.push_back(PairedRankCheck(string(),
                                                                 m_OHBot->m_DB->ThreadedStatsPlayerSummaryCheck(
                                                                         StatsUser, Month, Year, m_StatsAlias)));
                else
                    m_PairedRankChecks.push_back(PairedRankCheck(User, m_OHBot->m_DB->ThreadedStatsPlayerSummaryCheck(
                            StatsUser, Month, Year, m_StatsAlias)));
            }
            else if (Matches == 1) {
                if (spoofed && Level >= 8)
                    m_PairedRankChecks.push_back(PairedRankCheck(string(),
                                                                 m_OHBot->m_DB->ThreadedStatsPlayerSummaryCheck(
                                                                         LastMatch->GetName(), Month, Year,
                                                                         m_StatsAlias)));
                else
                    m_PairedRankChecks.push_back(PairedRankCheck(User, m_OHBot->m_DB->ThreadedStatsPlayerSummaryCheck(
                            LastMatch->GetName(), Month, Year, m_StatsAlias)));
            }
            else if (Matches > 1)
                SendChat(player, m_OHBot->m_Language->FoundMultiplyMatches());
        }

        player->SetStatsSentTime(GetTime());
    }

        //
        // !STATSDOTA
        // !STATSLOD
        // !STATSIMBA
        // !SD
        // !SL
        // !SI
        //

    else if (((Command != "statsaliases" && Command.substr(0, 5) == "stats" && Command.size() > 5) ||
              (Command == "sl" || Command == "sd" || Command == "si")) &&
             GetTime() - player->GetStatsDotASentTime() >= 5) {
        uint32_t m_StatsAlias = 0;
        string alias = "";
        if (Command.size() > 5)
            alias = Command.substr(5, Command.size() - 5);
        else if (Command.size() < 5)
            alias = Command.substr(1, Command.size() - 1);

        m_StatsAlias = m_OHBot->GetStatsAliasNumber(alias);

        if (m_StatsAlias == 0) {
            SendChat(player, m_OHBot->m_Language->DidNotFoundAlias(alias));
            if (m_OHBot->m_Aliases.size() != 0 && m_OHBot->m_Aliases.size() > m_GameAlias) {
                SendChat(player, m_OHBot->m_Language->UsingDefaultAlias(m_OHBot->m_Aliases[m_GameAlias - 1]));
            } else {
                SendChat(player, m_OHBot->m_Language->LostConnectionPleaseTryLater());
            }
            m_StatsAlias = m_GameAlias;
        }

        if (m_StatsAlias != 0) {
            string StatsUser = User;
            string Month = "";
            string Year = "";

            if (!Payload.empty()) {
                stringstream SS;
                SS << Payload;
                SS >> StatsUser;
                SS >> Month;
                SS >> Year;
            }

            CGamePlayer *LastMatch = NULL;
            uint32_t Matches = GetPlayerFromNamePartial(StatsUser, &LastMatch);

            if (Matches == 0) {
                if (spoofed && Level >= 8)
                    m_PairedSChecks.push_back(PairedSCheck(string(),
                                                           m_OHBot->m_DB->ThreadedStatsPlayerSummaryCheck(StatsUser,
                                                                                                          Month, Year,
                                                                                                          m_StatsAlias)));
                else
                    m_PairedSChecks.push_back(PairedSCheck(User,
                                                           m_OHBot->m_DB->ThreadedStatsPlayerSummaryCheck(StatsUser,
                                                                                                          Month, Year,
                                                                                                          m_StatsAlias)));
            }
            else if (Matches == 1) {
                if (spoofed && Level >= 8)
                    m_PairedSChecks.push_back(PairedSCheck(string(), m_OHBot->m_DB->ThreadedStatsPlayerSummaryCheck(
                            LastMatch->GetName(), Month, Year, m_StatsAlias)));
                else
                    m_PairedSChecks.push_back(PairedSCheck(User, m_OHBot->m_DB->ThreadedStatsPlayerSummaryCheck(
                            LastMatch->GetName(), Month, Year, m_StatsAlias)));
            }
            else if (Matches > 1)
                SendChat(player, m_OHBot->m_Language->FoundMultiplyMatches());
        }

        player->SetStatsDotASentTime(GetTime());
    }

        //
        // !STREAKDOTA
        // !STREAKLOD
        // !STRAKIMBA
        // !STD
        // !STL
        // !STI
        //
    else if (Command.substr(0, 6) == "streak" || (Command == "std" || Command == "stl" || Command == "sti")) {
        uint32_t m_StatsAlias = 0;
        string alias = "";
        if (Command.size() > 6)
            alias = Command.substr(6, Command.size() - 6);
        else if (Command.size() < 6)
            alias = Command.substr(2, Command.size() - 2);

        m_StatsAlias = m_OHBot->GetStatsAliasNumber(alias);

        if (m_StatsAlias == 0) {
            SendChat(player, m_OHBot->m_Language->DidNotFoundAlias(alias));
            if (m_OHBot->m_Aliases.size() != 0 && m_OHBot->m_Aliases.size() > m_GameAlias) {
                SendChat(player, m_OHBot->m_Language->UsingDefaultAlias(m_OHBot->m_Aliases[m_GameAlias - 1]));
            } else {
                SendChat(player, m_OHBot->m_Language->LostConnectionPleaseTryLater());
            }
            m_StatsAlias = m_GameAlias;
        }

        if (m_StatsAlias != 0) {
            string StatsUser = User;
            string Month = "";
            string Year = "";
            if (!Payload.empty()) {
                stringstream SS;
                SS << Payload;
                SS >> StatsUser;
                SS >> Month;
                SS >> Year;
            }

            // check for potential abuse

            if (!StatsUser.empty() && StatsUser.size() < 16 && StatsUser[0] != '/')
                m_PairedStreakChecks.push_back(PairedStreakCheck(User, m_OHBot->m_DB->ThreadedStatsPlayerSummaryCheck(
                        StatsUser, Month, Year, m_StatsAlias)));
        }

        player->SetStatsDotASentTime(GetTime());
    }

        //
        // !STATSALIASES
        //
    else if (Command == "statsaliases") {
        string Aliases = "";
        uint32_t c = 1;
        for (vector<string>::iterator i = m_OHBot->m_Aliases.begin(); i != m_OHBot->m_Aliases.end(); ++i) {
            if (c == 1)
                Aliases += *i;
            else
                Aliases += ", " + *i;

            if (Aliases.size() > 100) {
                SendChat(player, "[StatsAliases] " + Aliases);
                Aliases.clear();
            }
            c++;
        }
        SendChat(player, "[StatsAliases] " + Aliases);
    }

        //
        // !VERSION
        //

    else if (Command == "version") {
        if (spoofed && Level >= 8)
            SendAllChat(m_OHBot->m_Language->VersionAdmin(m_OHBot->m_Version));
        else
            SendAllChat(m_OHBot->m_Language->VersionNotAdmin(m_OHBot->m_Version));
    }

        //
        // !WHOVOTEKICKED
        //
    else if ((Command == "wvk" || Command == "whovk" || Command == "whovoted" || Command == "whovotekicked") &&
             !m_KickVotePlayer.empty()) {
        SendChat(player, m_OHBot->m_Language->CurrentVoteKickProcessUser(m_KickVotePlayer));
        SendChat(player, m_OHBot->m_Language->CurrentVoteKickRunTime(UTIL_ToString(GetTime() - m_StartedKickVoteTime),
                                                                     UTIL_ToString(60 - (GetTime() -
                                                                                         m_StartedKickVoteTime))));
        uint32_t VotesNeeded = (float) ((GetNumHumanPlayers() - 1) / 2) * 0.75;
        uint32_t Votes = 0;
        string VotedPlayers;
        for (vector<CGamePlayer *>::iterator i = m_Players.begin(); i != m_Players.end(); ++i) {
            if ((*i)->GetKickVote()) {
                ++Votes;
                if (!VotedPlayers.empty())
                    VotedPlayers += ", " + (*i)->GetName();
                else
                    VotedPlayers = (*i)->GetName();
            }
        }
        SendChat(player, "[" + UTIL_ToString(Votes) + "/" + UTIL_ToString(VotesNeeded) + "]: " + VotedPlayers);
    }

        //
        // !VOTEKICK
        //

    else if (Command == "votekick" && m_OHBot->m_VoteKickAllowed && !Payload.empty() &&
             (!m_GameLoaded || (m_GameLoaded && m_OHBot->m_IngameVoteKick))) {
        player->SetVKTimes();
        if (player->GetVKTimes() == 8)
            BanPlayerByPenality(User, player->GetExternalIPString(), m_OHBot->m_BotManagerName,
                                player->GetPenalityLevel(), "votekick abuse");
        else if (player->GetVKTimes() == 5)
            m_Pairedpenps.push_back(Pairedpenp(string(), m_OHBot->m_DB->Threadedpenp(User, "votekick abuse",
                                                                                     m_OHBot->m_BotManagerName, 1,
                                                                                     "add")));
        else if (player->GetVKTimes() >= 2)
            SendChat(player, m_OHBot->m_Language->NotifyForAbusiveVotekick());

        if (!m_KickVotePlayer.empty())
            SendChat(player, m_OHBot->m_Language->UnableToVoteKickAlreadyInProgress());
        else if (m_Players.size() <= 3)
            SendChat(player, m_OHBot->m_Language->UnableToVoteKickNotEnoughPlayers());
        else {
            string name;
            string reason;
            stringstream SS;
            SS << Payload;
            SS >> name;
            SS >> reason;
            if (SS.fail() || reason.empty()) {
                SendChat(player, m_OHBot->m_Language->ErrorMissingReason());
                return HideCommand;
            }
            if (!CustomVoteKickReason(reason)) {
                SendChat(player, m_OHBot->m_Language->NotifyForCustomVotekickReason());
                return HideCommand;
            }
            else {
                CGamePlayer *LastMatch = NULL;
                uint32_t Matches = GetPlayerFromNamePartial(name, &LastMatch);

                if (Matches == 0)
                    SendChat(player, m_OHBot->m_Language->UnableToVoteKickNoMatchesFound(name));
                else if (Matches == 1) {
                    //see if the player is the only one left on his team
                    unsigned char SID = GetSIDFromPID(LastMatch->GetPID());
                    bool OnlyPlayer = false;

                    if (m_GameLoaded && SID < m_Slots.size()) {
                        unsigned char Team = m_Slots[SID].GetTeam();
                        OnlyPlayer = true;
                        char sid, team;

                        for (vector<CGamePlayer *>::iterator i = m_Players.begin(); i != m_Players.end(); i++) {
                            if (*i && LastMatch != *i && !(*i)->GetLeftMessageSent()) {
                                sid = GetSIDFromPID((*i)->GetPID());
                                if (sid != 255) {
                                    team = m_Slots[sid].GetTeam();
                                    if (team == Team) {
                                        OnlyPlayer = false;
                                        break;
                                    }
                                }
                            }
                        }
                    }

                    uint32_t VLevel = 0;
                    string VLevelName;
                    for (vector<CBNET *>::iterator i = m_OHBot->m_BNETs.begin(); i != m_OHBot->m_BNETs.end(); ++i) {
                        if ((*i)->GetServer() == LastMatch->GetSpoofedRealm() ||
                            LastMatch->GetSpoofedRealm() == m_OHBot->m_WC3ConnectAlias) {
                            VLevel = (*i)->IsLevel(LastMatch->GetName());
                            VLevelName = (*i)->GetLevelName(Level);
                            break;
                        }
                    }

                    if (reason.find("feeding") != string::npos && LastMatch->GetFeedLevel() == 0) {
                        SendChat(player,
                                 m_OHBot->m_Language->VoteKickedUserWhoWasntMarkedAsFeeder(LastMatch->GetName()));
                        SendChat(player, m_OHBot->m_Language->VoteKickFeederAbuseReason());
                        player->SetVKTimes();
                        return HideCommand;
                    }

                    if (reason.find("feeding") != string::npos && LastMatch->GetFeedLevel() == 1) {
                        SendChat(player,
                                 m_OHBot->m_Language->VoteKickedUserWhoWasMarkedAsFeederLevelOne(LastMatch->GetName()));
                        SendChat(player, m_OHBot->m_Language->ReminederForVotekickAFeederLevelOne());
                        return HideCommand;
                    }

                    if (VLevel > 2 || Level < VLevel)
                        SendChat(player, m_OHBot->m_Language->NoPermissionToExecCommand());
                    else if (OnlyPlayer)
                        SendChat(player,
                                 m_OHBot->m_Language->VoteKickNotPossiblePlayerIsInAnotherTeam(LastMatch->GetName()));
                    else if (LastMatch == player)
                        SendChat(player, m_OHBot->m_Language->UnableToVotekickYourself());
                    else {
                        m_KickVotePlayer = LastMatch->GetName();
                        m_StartedKickVoteTime = GetTime();

                        for (vector<CGamePlayer *>::iterator i = m_Players.begin(); i != m_Players.end(); ++i)
                            (*i)->SetKickVote(false);

                        player->SetKickVote(true);
                        SendAllChat(m_OHBot->m_Language->StartedVoteKick(LastMatch->GetName(), User, UTIL_ToString(
                                (float) ((GetNumHumanPlayers() - 1) / 2) * 0.75, 0)));
                        SendAllChat(m_OHBot->m_Language->TypeYesToVote(string(1, m_OHBot->m_CommandTrigger)));
                    }
                }
                else
                    SendChat(player, m_OHBot->m_Language->UnableToVoteKickFoundMoreThanOneMatch(name));
            }
        }
    }

        //
        // !VKREASONS
        //
    else if (Command == "vkreasons" && m_KickVotePlayer.empty()) {
        SendChat(player, m_OHBot->m_Language->CustomVoteKickReasons());
        SendChat(player, "Maphack, Fountainfarm, Feeding, Flaming & Gameruin");
        return HideCommand;
    }

    //
    // !VOTESTART
    //

    if ((Command == "votestart" || Command == "vs") && !m_CountDownStarted && m_OHBot->m_AllowVoteStart) {

        if (!m_OHBot->m_CurrentGame->GetLocked()) {
            if (m_StartedVoteStartTime == 0) { //need >minplayers or admin to START a votestart
                if (GetNumHumanPlayers() < m_OHBot->m_VoteStartMinPlayers) { //need at least eight players to votestart
                    SendChat(player, m_OHBot->m_Language->UnableToVoteStartMissingPlayers(
                            UTIL_ToString(m_OHBot->m_VoteStartMinPlayers)));
                    return false;
                }

                for (vector<CGamePlayer *>::iterator i = m_Players.begin(); i != m_Players.end(); ++i)
                    (*i)->SetStartVote(false);
                m_StartedVoteStartTime = GetTime();
            }

            player->SetStartVote(true);

            uint32_t VotesNeeded = GetNumHumanPlayers() - 1;

            if (VotesNeeded < 2)
                VotesNeeded = 2;

            uint32_t Votes = 0;

            for (vector<CGamePlayer *>::iterator i = m_Players.begin(); i != m_Players.end(); ++i) {
                if ((*i)->GetStartVote())
                    ++Votes;
            }

            if (Votes < VotesNeeded) {
                SendAllChat(m_OHBot->m_Language->UnableToVoteStartMissingVotesd(UTIL_ToString(VotesNeeded - Votes)));
            } else {
//                                if( m_MatchMaking && m_AutoStartPlayers != 0 )
//                                        BalanceSlots( );
                SendAllChat(m_OHBot->m_Language->SuccessfullyVoteStarted());
                StartCountDown(true);
            }
        }
        else {
            SendChat(player, m_OHBot->m_Language->UnableToVoteStartOwnerInGame());
        }
    }

        //
        // !YES
        //

    else if (Command == "yes" && !m_KickVotePlayer.empty() && User != m_KickVotePlayer && !player->GetKickVote()) {
        player->SetKickVote(true);
        uint32_t VotesNeeded = (float) ((GetNumHumanPlayers() - 1) / 2) * 0.75;
        uint32_t Votes = 0;
        uint32_t voteplayersid;

        for (vector<CGamePlayer *>::iterator i = m_Players.begin(); i != m_Players.end(); ++i) {
            if ((*i)->GetName() == m_KickVotePlayer)
                voteplayersid = GetSIDFromPID((*i)->GetPID());
        }
        uint32_t playersid = GetSIDFromPID(player->GetPID());
        if ((playersid >= 0 && playersid <= 4 && voteplayersid >= 0 && voteplayersid <= 4) ||
            (playersid >= 5 && playersid <= 9 && voteplayersid >= 5 && voteplayersid <= 9)) {
            for (vector<CGamePlayer *>::iterator i = m_Players.begin(); i != m_Players.end(); ++i) {
                if ((*i)->GetKickVote())
                    ++Votes;
            }

            if (Votes >= VotesNeeded) {
                CGamePlayer *Victim = GetPlayerFromName(m_KickVotePlayer, true);

                if (Victim) {
                    Victim->SetDeleteMe(true);
                    Victim->SetLeftReason(m_OHBot->m_Language->WasKickedByVote());

                    if (!m_GameLoading && !m_GameLoaded)
                        Victim->SetLeftCode(PLAYERLEAVE_LOBBY);
                    else
                        Victim->SetLeftCode(PLAYERLEAVE_LOST);

                    if (!m_GameLoading && !m_GameLoaded)
                        OpenSlot(GetSIDFromPID(Victim->GetPID()), false);

                    SendAllChat(m_OHBot->m_Language->VoteKickPassed(m_KickVotePlayer));
                }
                else
                    SendAllChat(m_OHBot->m_Language->ErrorVoteKickingPlayer(m_KickVotePlayer));

                m_KickVotePlayer.clear();
                m_StartedKickVoteTime = 0;
            }
            else
                SendAllChat(m_OHBot->m_Language->VoteKickAcceptedNeedMoreVotes(m_KickVotePlayer, User,
                                                                               UTIL_ToString(VotesNeeded - Votes)));
        }
        else
            SendChat(player, m_OHBot->m_Language->UnableToVoteKickNotUsersTeam());

    }

    //
    // !DRAW
    //
    if (m_GameLoaded && (Command == "draw" || Command == "undraw") && !m_SoftGameOver) {
        unsigned char SID = GetSIDFromPID(player->GetPID());
        if (SID != 11) {
            if (Command == "draw") {
                bool ChangedVote = true;

                if (!player->GetDrawVote())
                    player->SetDrawVote(true);
                else
                    ChangedVote = false; //continue in case someone left and now we have enough votes

                uint32_t VotesNeeded = (float) ceil((GetNumHumanPlayers() - m_ObservingPlayers) * 0.75);
                uint32_t Votes = 0;

                for (vector<CGamePlayer *>::iterator i = m_Players.begin(); i != m_Players.end(); i++) {
                    if ((*i)->GetDrawVote()) {
                        Votes++;
                    }
                }

                if (Votes >= VotesNeeded) {
                    SendAllChat(m_OHBot->m_Language->AutoEndEarlyDrawTwo());
                    m_SoftGameOver = true;
                    m_GameOverTime = GetTime();
                }
                else if (ChangedVote) //only display message if they actually changed vote
                {
                    SendAllChat(m_OHBot->m_Language->UserVotedForDraw(User, UTIL_ToString(VotesNeeded - Votes)));
                    SendChat(player, m_OHBot->m_Language->UndrawNotify());
                }
            }
            else if (Command == "undraw" && player->GetDrawVote()) {
                player->SetDrawVote(false);
                SendAllChat(m_OHBot->m_Language->UserRecalledDrawVote(User));
            }
        }
        else
            SendChat(player, m_OHBot->m_Language->ObserverTriesToDraw());
    }

    //
    // !FORFEIT
    //
    if (m_GameLoaded && m_ForfeitTime == 0 && (Command == "ff" || Command == "forfeit") && !m_SoftGameOver) {
        if (GetTime() - m_GameLoadedTime <= (m_OHBot->m_MinFF * 60 - 400 * m_Leavers))
            SendChat(player, m_OHBot->m_Language->RemainFFTime(UTIL_ToString(m_OHBot->m_MinFF), UTIL_ToString(
                    ((m_GameLoadedTime + ((m_OHBot->m_MinFF - m_Leavers * 2) * 60)) - GetTime()) / 60)));
        else {
            bool ChangedVote = true;

            if (!player->GetForfeitVote())
                player->SetForfeitVote(true);
            else
                ChangedVote = false;

            char playerSID = GetSIDFromPID(player->GetPID());

            if (playerSID != 255) {
                char playerTeam = m_Slots[playerSID].GetTeam();

                // whether or not all players on the team of the player who typed the command forfeited
                bool AllVoted = true;
                int numVoted = 0;
                int numTotal = 0;

                for (vector<CGamePlayer *>::iterator i = m_Players.begin(); i != m_Players.end(); i++) {
                    if (*i && !(*i)->GetLeftMessageSent()) {
                        char sid = GetSIDFromPID((*i)->GetPID());

                        if (sid != 255 && m_Slots[sid].GetTeam() == playerTeam) {
                            numTotal++;

                            if (!(*i)->GetForfeitVote())
                                AllVoted = false;
                            else
                                numVoted++;
                        }
                    }
                }

                m_ForfeitTeam = playerTeam;

                // observers cannot forfeit!
                if (m_ForfeitTeam == 0 || m_ForfeitTeam == 1) {
                    string ForfeitTeamString = "Sentinel";
                    if (m_ForfeitTeam == 1) ForfeitTeamString = "Scourge";

                    if (AllVoted) {
                        m_Stats->SetWinner(((playerTeam + 1) % 2) + 1);
                        m_ForfeitTime = GetTime();
                        SendAllChat(m_OHBot->m_Language->TeamForfeited(ForfeitTeamString));
                        SendAllChat(m_OHBot->m_Language->StayToSafeStats());
                    }

                    else if (ChangedVote) {
                        SendAllChat(m_OHBot->m_Language->UserForfeitedGame(User));
                        SendAllChat(
                                m_OHBot->m_Language->UserForfeitedGameNotify(ForfeitTeamString, UTIL_ToString(numVoted),
                                                                             UTIL_ToString(numTotal)));
                    }
                }
            }
        }
    }

        /**********************
    * GRIEF-CODE COMMANDS *
    **********************/

        // !ID
    else if (Command == "id") {
        SendChat(player, m_OHBot->m_Language->YourUniqueId(UTIL_ToString(player->GetID())));
    }

        //
        // !WFF !WHOFF
        //
    else if (Command == "whoff" || Command == "wff") {
        int Votes = 0;
        int SeVotes = 0;
        int ScVotes = 0;
        int SePlayers = 0;
        int ScPlayers = 0;
        string SeNames;
        string ScNames;
        bool twoteammap = true;
        for (vector<CGamePlayer *>::iterator i = m_Players.begin(); i != m_Players.end(); i++) {
            char sid = GetSIDFromPID((*i)->GetPID());
            if (sid <= 4 && sid >= 0) {
                if ((*i)->GetForfeitVote()) {
                    if (!SeNames.empty())
                        SeNames += ", " + (*i)->GetName();
                    else
                        SeNames = (*i)->GetName();
                    SeVotes++;
                }
                SePlayers++;
            }
            else if (sid >= 5 && sid <= 9) {
                if ((*i)->GetForfeitVote()) {

                    if (!ScNames.empty())
                        ScNames += ", " + (*i)->GetName();
                    else
                        ScNames = (*i)->GetName();
                    ScVotes++;
                }
                ScPlayers++;
            }
            else if (m_Slots[sid].GetTeam() != 12)
                twoteammap = false;
            Votes++;
        }
        if (Votes == 0)
            SendChat(player, m_OHBot->m_Language->NoOneHasForfeitedYet());
        else if (!twoteammap)
            SendChat(player, m_OHBot->m_Language->ErrorForfeitingNoTwoTeamMap());
        else {
            SendChat(player, "[Sentinel (" + UTIL_ToString(SeVotes) + "/" + UTIL_ToString(SePlayers) + "): " + SeNames);
            SendChat(player, "[Scourge (" + UTIL_ToString(ScVotes) + "/" + UTIL_ToString(ScPlayers) + "): " + ScNames);
        }
    }

        //
        // !OB
        //
    else if ((Command == "ob" || Command == "obs" || Command == "observe") && !m_CountDownStarted && !m_GameLoaded &&
             !m_GameLoading) {
        if (Level > 2) {
            if (m_Slots[11].GetSlotStatus() != SLOTSTATUS_OCCUPIED) {
                m_ObservingPlayers += 1;
                unsigned char oldsid = GetSIDFromPID(player->GetPID());
                SwapSlots(oldsid, 11);
                OpenSlot(oldsid, true);
                m_AutoStartPlayers = m_AutoStartPlayers + 1;
                SendAllChat(m_OHBot->m_Language->UserWillObserveGame(User));
                SendAllChat(m_OHBot->m_Language->AutoStartEnabled("11"));
            }
            else
                SendChat(player, m_OHBot->m_Language->ErrorObserverGameAlreadyObserver());
        }
        else
            SendChat(player, m_OHBot->m_Language->NoPermissionToExecCommand());
    }

        //
        // !UNOB
        //
    else if ((Command == "unob" || Command == "unobs" || Command == "unobserve") && !m_GameLoaded && !m_GameLoading &&
             !m_CountDownStarted) {
        unsigned char SID = GetSIDFromPID(player->GetPID());
        if (Level > 2 && SID == 11) {
            int32_t newslot = -1;
            int c = 0;
            for (vector<CGameSlot>::iterator i = m_Slots.begin(); i != m_Slots.end(); ++i) {
                if ((*i).GetSlotStatus() == SLOTSTATUS_OPEN) {
                    newslot = c;
                    break;
                }
                c++;
            }

            if (newslot == -1)
                newslot = m_LatestSlot;

            m_ObservingPlayers -= 1;
            SwapSlots(newslot, 11);
            CloseSlot(11, true);
            m_AutoStartPlayers = m_AutoStartPlayers - 1;
            SendAllChat(m_OHBot->m_Language->UserWillNoLongerObserveGame(User));
            SendAllChat(m_OHBot->m_Language->AutoStartEnabled("10"));
        }
        else
            SendChat(player, m_OHBot->m_Language->NoPermissionToExecCommand());
    }

        //
        // !PW  !PASS   !PASSWORD
        //

    else if ((Command == "pw" || Command == "pass" || Command == "password") && m_OHBot->m_AccountProtection) {
        string Status;
        string Password;
        stringstream SS;
        SS << Payload;
        SS >> Password;
        if (!SS.eof()) {
            getline(SS, Status);
            string::size_type Start = Status.find_first_not_of(" ");

            if (Start != string::npos)
                Status = Status.substr(Start);
        }

        if (Status.empty()) {
            if (player->GetPasswordProt()) {
                m_PairedPassChecks.push_back(
                        PairedPassCheck(User, m_OHBot->m_DB->ThreadedPassCheck(User, Password, 0)));
            }
        }
        else if (Status == "0" || Status == "clear")
            m_PairedPassChecks.push_back(PairedPassCheck(User, m_OHBot->m_DB->ThreadedPassCheck(User, Password, 1)));
        else
            SendChat(player, m_OHBot->m_Language->ErrorRemovingPassProtectionInvalidOption());

        HideCommand = true;
    }

        //
        // !LG   !LASTGAME
        //
    else if ((Command == "lg" || Command == "lastgame") && Payload.empty() && m_OHBot->m_MessageSystem)
        m_PairedINChecks.push_back(PairedINCheck(User, m_OHBot->m_DB->ThreadedInboxSummaryCheck(User)));

        //
        // !PM
        //
    else if (Command == "pm" && !Payload.empty() && m_OHBot->m_MessageSystem) {
        string UserTo;
        string Message;
        stringstream SS;
        SS << Payload;
        SS >> UserTo;
        if (!SS.eof()) {
            getline(SS, Message);
            string::size_type Start = Message.find_first_not_of(" ");

            if (Start != string::npos)
                Message = Message.substr(Start);
        }
        else
            SendChat(player, m_OHBot->m_Language->ErrorWrongInputForMessage());

        if (UserTo.length() >= 3) {
            if (Payload.length() > 3) {
                m_Pairedpms.push_back(Pairedpm(User, m_OHBot->m_DB->Threadedpm(User, UserTo, 0, Message, "add")));
            }
            else
                SendChat(player, m_OHBot->m_Language->ErrorMessageTooShort());
        }
        else
            SendChat(player, m_OHBot->m_Language->InvalidName());
    }

        //
        // !POINTS      !P
        //
    else if ((Command == "points" || Command == "p" || Command == "pts") && m_OHBot->m_BetSystem) {
        string StatsUser = User;

        if (!Payload.empty())
            StatsUser = Payload;

        // check for potential abuse

        if (!StatsUser.empty() && StatsUser.size() < 16 && StatsUser[0] != '/')
            m_PairedSSs.push_back(
                    PairedSS(User, m_OHBot->m_DB->ThreadedStatsSystem(StatsUser, "betsystem", 0, "betcheck")));
    }

        //
        // !BET !B
        //
    else if ((Command == "bet" || Command == "b") && m_GameLoaded && !m_GameLoading && m_OHBot->m_BetSystem) {
        if (!Payload.find_first_not_of("1234567890") == string::npos) {
            SendChat(player, m_OHBot->m_Language->ErrorBetInvalidAmount());
            return HideCommand;
        }
        if (UTIL_ToUInt32(Payload) > 50 || UTIL_ToUInt32(Payload) <= 0) {
            SendChat(player, m_OHBot->m_Language->ErrorBetInvalidLogicalAmount());
            return HideCommand;
        }
        if (GetTime() - m_GameLoadedTime >= 300) {
            SendChat(player, m_OHBot->m_Language->ErrorBetAlreadyTooLate());
            return HideCommand;
        }
        else
            m_PairedSSs.push_back(PairedSS(User,
                                           m_OHBot->m_DB->ThreadedStatsSystem(User, "betsystem", UTIL_ToUInt32(Payload),
                                                                              "bet")));
    }

        //
        // !TOP        !TOP10
        //
    else if (Command == "top" || Command == "top10") {
        m_PairedSSs.push_back(PairedSS(User, m_OHBot->m_DB->ThreadedStatsSystem("", "", 0, "top")));
    }
        //
        // !PAUSE
        //
    else if (Command == "pause" && !m_GameLoading && m_GameLoaded && !player->GetUsedPause()) {
        if (Level >= m_OHBot->m_MinPauseLevel) {
            if (!m_PauseReq) {
                SendAllChat(m_OHBot->m_Language->UserRequestedToPauseGame(User));
                SendAllChat(m_OHBot->m_Language->GamePauseNotify());
                player->SetUsedPause(true);
                m_PauseReq = true;
                m_PauseIntroTime = GetTime();
                m_LastCountDownTicks = 0;
                m_Paused = false;
            }
            else
                SendChat(player, m_OHBot->m_Language->ErrorPausingAlreadyRequested());
        }
        else
            SendChat(player, m_OHBot->m_Language->NoPermissionToExecCommand());
    }

        //
        // !UNPAUSE
        //
    else if (Command == "unpause" && !m_GameLoading && m_GameLoaded && !player->GetUsedPause()) {
        if (Level > 2) {
            if (m_Paused) {
                SendAllChat(m_OHBot->m_Language->UserRequestedToUnPauseGame(User));
                m_PauseTime = 55;
                m_PauseTicks = 5;
                m_LastCountDownTicks = 0;
            }
            else
                SendChat(player, m_OHBot->m_Language->ErrorUnPausingGameIsntPaused());
        }
        else
            SendChat(player, m_OHBot->m_Language->NoPermissionToExecCommand());
    }

        //
        // !WP  !WINPERC        !CB     !CHECKBALANCE
        //
    else if ((Command == "wp" || Command == "cb" || Command == "winperc" || Command == "checkbalance") &&
             Payload.empty()) {
        vector<float> teamWP;
        float totalWP = 0.0;
        for (vector<CGamePlayer *>::iterator i = m_Players.begin(); i != m_Players.end(); ++i) {

            char sid = GetSIDFromPID((*i)->GetPID());
            int8_t team = m_Slots[sid].GetTeam();
            if (team == 12) { continue; }
            float rate = 0;
            if ((*i)->GetGames() > 0)
                rate = ((*i)->GetScore() / (*i)->GetGames());
            if (team >= teamWP.size()) {
                teamWP.resize(team + 1);
                teamWP[team] = 0.0;
            }
            teamWP[team] += rate;
            totalWP += rate;
        }
        if (totalWP != 0) {
            string gen = "";
            int teams = teamWP.size();
            for (int i = 0; i < teams; i++) {
                if (teamWP[i] != 0.0) {
                    gen += " [TEAM " + UTIL_ToString(i + 1) + ": " + UTIL_ToString(((100 * teamWP[i]) / totalWP), 1) +
                           "%]";
                }
            }
            SendChat(player, "Win rate:" + gen);
        }
        else
            SendChat(player, m_OHBot->m_Language->WinChance("50", "50"));
    }

        //
        // !SLAP
        //

    else if (Command == "slap" && !Payload.empty() && m_OHBot->m_FunCommands) {
        CGamePlayer *LastMatch = NULL;
        uint32_t Matches = GetPlayerFromNamePartial(Payload, &LastMatch);

        if (Matches != 1)
            return HideCommand;

        uint32_t VLevel = 0;
        string VLevelName;
        for (vector<CBNET *>::iterator i = m_OHBot->m_BNETs.begin(); i != m_OHBot->m_BNETs.end(); ++i) {
            if (((*i)->GetServer() == LastMatch->GetSpoofedRealm() ||
                 LastMatch->GetSpoofedRealm() == m_OHBot->m_WC3ConnectAlias) && m_OHBot->m_RanksLoaded) {
                VLevel = (*i)->IsLevel(LastMatch->GetName());
                VLevelName = (*i)->GetLevelName(VLevel);
                break;
            }
        }

        if (VLevel > Level) {
            SendChat(player->GetPID(), "You can't slap a " + VLevelName);
            return HideCommand;
        }

        srand((unsigned) time(NULL));
        int RandomNumber = rand() % 8;

        if (LastMatch->GetName() != User) {
            if (RandomNumber == 0)
                SendAllChat(User + " slaps " + LastMatch->GetName() + " with a large trout.");
            else if (RandomNumber == 1)
                SendAllChat(User + " slaps " + LastMatch->GetName() + " with a pink Macintosh.");
            else if (RandomNumber == 2)
                SendAllChat(User + " throws a Playstation 3 at " + LastMatch->GetName() + ".");
            else if (RandomNumber == 3)
                SendAllChat(User + " drives a car over " + LastMatch->GetName() + ".");
            else if (RandomNumber == 4)
                SendAllChat(User + " washes " + LastMatch->GetName() + "'s car.  Oh, the irony!");
            else if (RandomNumber == 5)
                SendAllChat(User + " burns " + LastMatch->GetName() + "'s house.");
            else if (RandomNumber >= 6)
                SendAllChat(User + " finds " + LastMatch->GetName() + "'s picture on uglypeople.com.");
        }
        else {
            if (RandomNumber == 0)
                SendAllChat(User + " slaps himself with a large trout.");
            else if (RandomNumber == 1)
                SendAllChat(User + " slaps himself with a pink Macintosh.");
            else if (RandomNumber == 2)
                SendAllChat(User + " throws a Playstation 3 at himself.");
            else if (RandomNumber == 3)
                SendAllChat(User + " drives a car over himself.");
            else if (RandomNumber == 4)
                SendAllChat(User + " steals his cookies. mwahahah!");
            else if (RandomNumber == 5)
                SendAllChat(User + " searches yahoo.com for goatsex + " + User + ". " + UTIL_ToString(rand()) +
                            " hits WEEE!");
            else if (RandomNumber == 6)
                SendAllChat(User + " burns his house.");
            else if (RandomNumber == 7)
                SendAllChat(User + " finds his picture on uglypeople.com.");
        }
    }

        //
        // !IGNORE
        //
    else if (Command == "ignore" && !Payload.empty()) {
        CGamePlayer *LastMatch = NULL;
        uint32_t Matches = GetPlayerFromNamePartial(Payload, &LastMatch);

        if (Matches == 0)
            SendChat(player, m_OHBot->m_Language->FoundNoMatchWithPlayername());
        else if (Matches == 1) {
            player->Ignore(LastMatch->GetName());
            SendChat(player, m_OHBot->m_Language->UserIgnoredPlayer(LastMatch->GetName()));
        }
        else
            SendChat(player, m_OHBot->m_Language->FoundMultiplyMatches());
    }

        //
        // !UNIGNORE
        //
    else if (Command == "unignore" && !Payload.empty()) {
        CGamePlayer *LastMatch = NULL;
        uint32_t Matches = GetPlayerFromNamePartial(Payload, &LastMatch);

        if (Matches == 0)
            SendChat(player, m_OHBot->m_Language->FoundNoMatchWithPlayername());
        else if (Matches == 1) {
            player->UnIgnore(LastMatch->GetName());
            SendChat(player, m_OHBot->m_Language->UserUnIgnoredPlayer(LastMatch->GetName()));
        }
        else
            SendChat(player, m_OHBot->m_Language->FoundMultiplyMatches());
    }

        //
        // !IGNOREALL
        //
    else if (Command == "ignoreall") {
        for (vector<CGamePlayer *>::iterator i = m_Players.begin(); i != m_Players.end(); ++i) {
            player->Ignore((*i)->GetName());
        }
        SendChat(player, m_OHBot->m_Language->UserIgnoringAllPlayersNotify());
    }

        //
        // !UNIGNOREALL
        //
    else if (Command == "unignoreall") {
        for (vector<CGamePlayer *>::iterator i = m_Players.begin(); i != m_Players.end(); ++i) {
            player->UnIgnore((*i)->GetName());
        }
        SendChat(player, m_OHBot->m_Language->UserUnIgnoringAllPlayersNotify());
    }

        //
        // !RULES
        //
    else if (Command == "r" || Command == "rule" || Command == "rules") {
        if (Payload.empty()) {
            SendChat(player, m_OHBot->m_Language->RuleTags() + GetRuleTags());
            SendChat(player, m_OHBot->m_Language->RuleTagNotify());
        }
        else
            SendChat(player, GetRule(Payload));
    }

        //
        // !VOTEMUTE
        //
    else if (Command == "votemute" && m_GameLoaded && m_OHBot->m_VoteMuting) {
        if (m_VoteMuteEventTime == 0 && m_VoteMutePlayer.empty() && m_MuteType == 2 && !Payload.empty()) {
            CGamePlayer *LastMatch = NULL;
            uint32_t Matches = GetPlayerFromNamePartial(Payload, &LastMatch);
            if (Matches == 0)
                SendChat(player, m_OHBot->m_Language->FoundNoMatchWithPlayername());
            else if (Matches == 1) {
                char votePlayerTeam = m_Slots[GetSIDFromPID(player->GetPID())].GetTeam();
                char targetPlayerTeam = m_Slots[GetSIDFromPID(LastMatch->GetPID())].GetTeam();
                string TeamString = targetPlayerTeam == 1 ? "Sentinel" : "Scourge";
                if (votePlayerTeam == targetPlayerTeam) {
                    // same team, we only take care of the allied teammates
                    uint32_t VotesNeeded = 0;
                    for (vector<CGamePlayer *>::iterator i = m_Players.begin(); i != m_Players.end(); ++i) {
                        if (m_Slots[GetSIDFromPID((*i)->GetPID())].GetTeam() == targetPlayerTeam)
                            VotesNeeded += 1;
                    }
                    //remove the target
                    VotesNeeded--;
                    SendAllChat(m_OHBot->m_Language->UserStartedVoteMute(User, LastMatch->GetName()));
                    SendAllChat(m_OHBot->m_Language->UserStartedVoteMuteVotesNeeded(TeamString,
                                                                                    UTIL_ToString(VotesNeeded)));
                    SendAllChat(
                            m_OHBot->m_Language->UserStartedVoteMuteVoteExpire(UTIL_ToString(m_OHBot->m_VoteMuteTime)));
                    m_MuteType = 0;
                } else {
                    // enemy team we need 2 votes from each team to have a success votemute (only allchat is affected)
                    SendAllChat(m_OHBot->m_Language->UserStartedVoteGlobalMute(User, LastMatch->GetName()));
                    SendAllChat(m_OHBot->m_Language->UserStartedVoteGlobalMuteVotesNeeded());
                    SendAllChat(
                            m_OHBot->m_Language->UserStartedVoteMuteVoteExpire(UTIL_ToString(m_OHBot->m_VoteMuteTime)));
                    m_MuteType = 1;
                    m_EnemyVotes = 1;
                }
                SendAllChat(m_OHBot->m_Language->UserStartedVoteNotify());
                m_VoteMuteEventTime = GetTime();
                m_VoteMutePlayer = LastMatch->GetName();
                m_VoteMuteTargetTeam = targetPlayerTeam;
                m_MuteVotes = 1;
            }
        }
        else if (Payload.empty() && !m_VoteMutePlayer.empty()) {
            if (m_MuteType == 0) {
                if (m_Slots[GetSIDFromPID(player->GetPID())].GetTeam() == m_VoteMuteTargetTeam) {
                    m_MuteVotes++;
                    uint32_t VotesNeeded = 0;
                    for (vector<CGamePlayer *>::iterator i = m_Players.begin(); i != m_Players.end(); ++i) {
                        if (m_Slots[GetSIDFromPID((*i)->GetPID())].GetTeam() == m_VoteMuteTargetTeam)
                            VotesNeeded += 1;
                    }
                    //remove the target
                    VotesNeeded--;
                    if (m_MuteVotes >= VotesNeeded) {
                        CGamePlayer *Player = GetPlayerFromName(m_VoteMutePlayer, true);
                        if (Player) {
                            Player->SetMuted(true);
                            SendAllChat(m_OHBot->m_Language->SuccessfullyVoteMutedUser(Player->GetName()));
                        }

                        m_VoteMuteEventTime = 0;
                        m_VoteMutePlayer.clear();
                        m_VoteMuteTargetTeam = 0;
                        m_EnemyVotes = 0;
                        m_MuteVotes = 0;
                        m_MuteType = 2;
                    }
                    else {
                        SendAllChat(m_OHBot->m_Language->UserVotedForMute(User, m_VoteMutePlayer,
                                                                          UTIL_ToString(VotesNeeded - m_MuteVotes)));
                    }
                }
            }
            else if (m_MuteType == 1) {
                if (m_Slots[GetSIDFromPID(player->GetPID())].GetTeam() == m_VoteMuteTargetTeam &&
                    m_MuteVotes - m_EnemyVotes < 2) {
                    m_MuteVotes++;
                }
                else if (m_Slots[GetSIDFromPID(player->GetPID())].GetTeam() != m_VoteMuteTargetTeam &&
                         m_Slots[GetSIDFromPID(player->GetPID())].GetTeam() != 12 && m_EnemyVotes == 1) {
                    m_MuteVotes++;
                    m_EnemyVotes++;
                }
                else if (m_EnemyVotes == 2 || m_MuteVotes - m_EnemyVotes == 2)
                    SendChat(player, m_OHBot->m_Language->UserVotedToModeNoVotesOnTeam());
                else
                    SendChat(player, m_OHBot->m_Language->UserVotedToModeNoVotesOnObserver());

                if (m_MuteVotes == 4) {
                    CGamePlayer *Player = GetPlayerFromName(m_VoteMutePlayer, true);
                    if (Player) {
                        Player->SetGlobalChatMuted(true);
                        SendAllChat(m_OHBot->m_Language->SuccessfullyVoteMutedUser(Player->GetName()));
                    }

                    m_VoteMuteEventTime = 0;
                    m_VoteMutePlayer.clear();
                    m_VoteMuteTargetTeam = 0;
                    m_MuteVotes = 0;
                    m_EnemyVotes = 0;
                    m_MuteType = 2;
                }
                else
                    SendAllChat(m_OHBot->m_Language->UserVotedForMute(User, m_VoteMutePlayer,
                                                                      UTIL_ToString(4 - m_MuteVotes)));

            }
        }
        else
            SendChat(player, m_OHBot->m_Language->ErrorVotingThereIsNoVote());
    }

        // autoending break command
        // !a
        //
    else if (Command == "a" && m_EndGame && m_EndTicks != 0) {
        if (m_Slots[GetSIDFromPID(player->GetPID())].GetTeam() == m_LoosingTeam) {
            if (player->GetVotedForInterruption() == false) {
                player->SetVotedForInterruption(true);
                m_BreakAutoEndVotes++;
                if (m_BreakAutoEndVotes >= m_BreakAutoEndVotesNeeded - 1) {
                    m_EndGame = false;
                    m_EndTicks = 0;
                    m_StartEndTicks = 0;
                    m_BreakAutoEndVotes = 0;
                    m_BreakAutoEndVotesNeeded = 0;
                    m_LoosingTeam = 0;
                    SendAllChat(m_OHBot->m_Language->SuccessfullyInterruptedAutoEnd());
                    for (vector<CGamePlayer *>::iterator i = m_Players.begin(); i != m_Players.end(); ++i) {
                        (*i)->SetVotedForInterruption(false);
                    }
                }
                else
                    SendAllChat(m_OHBot->m_Language->UserVotedForInterruptAutoEnd(User, UTIL_ToString(
                            m_BreakAutoEndVotesNeeded - m_BreakAutoEndVotes)));
            } else {
                SendChat(player, m_OHBot->m_Language->UserAlreadyVotedForIntteruptAutoEnd());
            }
        }
    }

        //
        // !PING
        //
    else if (Command == "ping" && Level < 5) {
        SendChat(player, m_OHBot->m_Language->YourPingIsToday() + " [" +
                         (player->GetNumPings() > 0 ? UTIL_ToString(player->GetPing(m_OHBot->m_LCPings)) +
                                                      m_OHBot->m_Language->Ms() : "N/A") + "]");
    }

        //
        // !VOTE
        //
    else if ((Command == "vote" || Command == "v") && m_OHBot->m_VoteMode &&
             (!m_GameLoaded || !m_GameLoading || !m_CountDownStarted) && !m_Voted) {
        if (m_ForcedMode) {
            SendChat(player, m_OHBot->m_Language->ErrorModeWasAlreadyForced());
            return false;
        }
        if (player->GetVotedMode() != 0) {
            SendChat(player, m_OHBot->m_Language->ErrorVotedAlreadyForMode());
        } else if (Payload.size() != 1 || UTIL_ToUInt32(Payload) < 1 || UTIL_ToUInt32(Payload) > m_ModesToVote.size()) {
            SendChat(player, m_OHBot->m_Language->ErrorInvalidModeWasVoted());
        } else {
            SendAllChat(m_OHBot->m_Language->UserVotedForMode(User, m_ModesToVote[UTIL_ToUInt32(Payload) - 1]));
            player->SetVotedMode(UTIL_ToUInt32(Payload));
            uint32_t c = 0;
            for (vector<CGamePlayer *>::iterator i = m_Players.begin(); i != m_Players.end(); ++i) {
                if ((*i)->GetVotedMode() == UTIL_ToUInt32(Payload)) {
                    c++;
                }
            }
            if (GetNumHumanPlayers() / 2 <= c && m_VotedTimeStart != 0) {
                if (UTIL_ToUInt32(Payload) != 7) {
                    SendAllChat(m_OHBot->m_Language->AbsoluteVoteChoosen(m_ModesToVote[UTIL_ToUInt32(Payload) - 1]));
                    m_HCLCommandString = m_lGameAliasName.find("lod") != string::npos ? m_OHBot->GetLODMode(
                            m_ModesToVote[UTIL_ToUInt32(Payload) - 1]) : m_ModesToVote[UTIL_ToUInt32(Payload) - 1];
                    m_Voted = true;
                    StartCountDownAuto(m_OHBot->m_RequireSpoofChecks);
                    m_LastAutoStartTime = GetTime();
                } else {
                    uint32_t RandomMode = rand() % (m_ModesToVote.size() - 1);
                    SendAllChat(m_OHBot->m_Language->AbsoluteVoteChoosenRandom(m_ModesToVote[RandomMode - 1]));
                    m_HCLCommandString = m_lGameAliasName.find("lod") != string::npos ? m_OHBot->GetLODMode(
                            m_ModesToVote[RandomMode - 1]) : m_ModesToVote[RandomMode - 1];
                    m_Voted = true;
                    StartCountDownAuto(m_OHBot->m_RequireSpoofChecks);
                    m_LastAutoStartTime = GetTime();
                }
            }
        }
    }

        //
        // !VOTEOPTIONS
        //
    else if ((Command == "voteoptions" || Command == "votelist" || Command == "vo") && m_OHBot->m_VoteMode) {
        SendChat(player, m_OHBot->m_Language->PossibleModesToVote());
        string Modes;
        uint32_t c = 1;
        for (vector<string>::iterator k = m_ModesToVote.begin(); k != m_ModesToVote.end(); ++k) {
            Modes += "[" + UTIL_ToString(c) + ": " + *k + "] ";
            if (c == 4) {
                SendChat(player, Modes);
                Modes.clear();
            }
            c++;
        }
        SendChat(player, Modes);
    }

        //
        // !VOTERESULT
        //
    else if ((Command == "voteresult" || Command == "vr") && m_OHBot->m_VoteMode) {
        if (m_ForcedMode) {
            SendChat(player, m_OHBot->m_Language->ModeWasForcedTo(m_ModesToVote[m_ForcedGameMode - 1]));
            return false;
        }
        uint32_t c = 0;
        uint32_t mode1 = 0;
        uint32_t mode2 = 0;
        uint32_t mode3 = 0;
        uint32_t mode4 = 0;
        uint32_t mode5 = 0;
        uint32_t mode6 = 0;
        uint32_t mode7 = 0;
        uint32_t notvoted = 0;
        for (vector<CGamePlayer *>::iterator i = m_Players.begin(); i != m_Players.end(); ++i) {
            if ((*i)->GetVotedMode() != 0) {
                if ((*i)->GetVotedMode() == 1)
                    mode1++;
                else if ((*i)->GetVotedMode() == 2)
                    mode2++;
                else if ((*i)->GetVotedMode() == 3)
                    mode3++;
                else if ((*i)->GetVotedMode() == 4)
                    mode4++;
                else if ((*i)->GetVotedMode() == 5)
                    mode5++;
                else if ((*i)->GetVotedMode() == 6)
                    mode6++;
                else if ((*i)->GetVotedMode() == 7)
                    mode7++;
            } else
                notvoted++;
        }
        SendChat(player, m_OHBot->m_Language->VoteResult());
        string Return = "";
        if (mode1 != 0) {
            Return += "[" + m_ModesToVote[0] + ": " + UTIL_ToString(mode1) + "] ";
        }
        if (mode2 != 0) {
            Return += "[" + m_ModesToVote[1] + ": " + UTIL_ToString(mode2) + "] ";
        }
        if (mode3 != 0) {
            Return += "[" + m_ModesToVote[2] + ": " + UTIL_ToString(mode3) + "] ";
        }
        if (mode4 != 0) {
            Return += "[" + m_ModesToVote[3] + ": " + UTIL_ToString(mode4) + "] ";
        }
        if (Return.size() != 0) {
            SendChat(player, Return);
            Return = "";
        }
        if (mode5 != 0) {
            Return += "[" + m_ModesToVote[4] + ": " + UTIL_ToString(mode5) + "] ";
        }
        if (mode6 != 0) {
            Return += "[" + m_ModesToVote[5] + ": " + UTIL_ToString(mode6) + "] ";
        }
        if (mode7 != 0) {
            Return += "[" + m_ModesToVote[6] + ": " + UTIL_ToString(mode7) + "] ";
        }
        if (Return.size() != 0) {
            SendChat(player, Return);
        }
        if (notvoted != 0) {
            SendChat(player, m_OHBot->m_Language->PlayersWhoDidntVoteForMode(UTIL_ToString(notvoted)));
        }
    }

        //
        // !NOLAG
        //

    else if (Command == "nolag" && GetTime() - player->GetStatsDotASentTime() >= 5) {
        player->SetNoLag(!player->GetNoLag());

        if (player->GetNoLag())
            SendAllChat(m_OHBot->m_Language->UserEnabledNoLag(User));
        else
            SendAllChat(m_OHBot->m_Language->UserDisabledNoLag(User));

        player->SetStatsDotASentTime(GetTime());
    }

        //
        // !REG
        // !REGISTRATION
        //
    else if ((Command == "reg" || Command == "registration" || Command == "verify" || Command == "confirm") &&
             !Payload.empty()) {
        string type = "";
        if (Command == "reg" || Command == "registration")
            type = "r";
        else if (Command == "verify" || Command == "confirm")
            type = "c";

        string Mail;
        string Password;
        stringstream SS;
        SS << Payload;
        SS >> Mail;

        if (!SS.eof()) {
            getline(SS, Password);
            string::size_type Start = Password.find_first_not_of(" ");

            if (Start != string::npos)
                Password = Password.substr(Start);
        }

        if (Mail.find("@") != string::npos || Mail.find(".") != string::npos) {
            if (Password.find(" ") != string::npos)
                SendChat(player, m_OHBot->m_Language->WrongPassRegisterCommand(Password));
            else if (Password.length() > 2) {
                m_PairedRegAdds.push_back(PairedRegAdd(string(),
                                                       m_OHBot->m_DB->ThreadedRegAdd(User, player->GetSpoofedRealm(),
                                                                                     Mail, Password, type)));
            }
            else
                SendChat(player, m_OHBot->m_Language->PassTooShortRegisterCommand(Password));
        }
        else
            SendChat(player, m_OHBot->m_Language->InvalidEmailRegisterCommand(Mail));

        return true;

    }

        //
        // !EXP !LVL !LEVEL
        //
    else if (Command == "exp" || Command == "lvl" || Command == "level") {
        string Name = User;
        uint32_t EXP = player->GetEXP();
        uint32_t Level = 0;
        float Percentage = 0;
        uint32_t ExpToNextLevel = 0;

        if (!Payload.empty()) {
            CGamePlayer *LastMatch = NULL;
            uint32_t Matches = GetPlayerFromNamePartial(Payload, &LastMatch);
            if (Matches == 0) {
                SendChat(player, m_OHBot->m_Language->FoundNoMatchWithPlayername());
                return true;
            }
            else if (Matches == 1) {
                Name = LastMatch->GetName();
                EXP = LastMatch->GetEXP();
            }
            else {
                SendChat(player, m_OHBot->m_Language->FoundMultiplyMatches());
                return true;
            }
        }

        uint32_t calcStart = 0;
        uint32_t calcEnd = 10;
        uint32_t calcInc = 0;
        uint32_t calcLevel = 6;
        uint32_t LevelStart = 0;
        uint32_t LevelEnd = 0;
        uint32_t calcCount = 0;
        do {
            calcCount++;
            if (calcCount % 2 == 0) {
                calcInc = calcInc + calcLevel;
            }
            if ((EXP < calcEnd) && (EXP >= calcStart)) {
                Level = calcCount;
                LevelStart = calcStart;
                LevelEnd = calcEnd;
            }
            calcStart = calcEnd;
            calcEnd = calcEnd + calcInc;
        } while (Level == 0);
        Level--;

        Percentage = (float) (((EXP - LevelStart) * 100) / (LevelEnd - LevelStart));

        ExpToNextLevel = LevelEnd - EXP;

        uint32_t currPerc = 0;
        string visualPerc = "";
        while (currPerc != 100) {
            if (currPerc < Percentage)
                visualPerc += "#";
            else
                visualPerc += "_";

            currPerc = currPerc + 5;
        }

        SendAllChat("[" + Name + "] Level: " + UTIL_ToString(Level) + ", Process: [" + visualPerc + "] " +
                    UTIL_ToString(Percentage, 2) + "% (" + UTIL_ToString(EXP) + "/" + UTIL_ToString(LevelEnd) + ")");
    }

        //
        // !VOTEBALANCE
        //
    else if (Command == "votebalance" && m_GameLoaded && !m_GameLoading) {
        if (!player->GetVotedForBalance()) {
            m_BalanceVotes++;
            SendAllChat(m_OHBot->m_Language->UserVotedForBalance(User));

            if (m_BalanceVotes > (GetNumHumanPlayers() / 2)) {
                m_GameBalance = true;
                SendAllChat(m_OHBot->m_Language->EnabledBalanceForThisGame());
            }
        } else {
            SendChat(player, m_OHBot->m_Language->ErrorVotedAlreadyForBalance());
        }
    }

        //
        // !REPUTATION
        //
    else if (Command == "reputation" || Command == "rep") {
        if (!Payload.empty()) {
            CGamePlayer *LastMatch = NULL;
            uint32_t Matches = GetPlayerFromNamePartial(Payload, &LastMatch);
            if (Matches == 0)
                SendChat(player, m_OHBot->m_Language->FoundNoMatchWithPlayername());
            else if (Matches == 1)
                SendAllChat(m_OHBot->m_Language->UsersReputation(LastMatch->GetName(),
                                                                 UTIL_ToString(LastMatch->GetReputation(), 2)));
            else
                SendChat(player, m_OHBot->m_Language->FoundMultiplyMatches());
        } else {
            SendChat(player, m_OHBot->m_Language->UsersOwnReputation(UTIL_ToString(player->GetReputation(), 2)));
        }
    }


        //
        // !use
        //
    else if (Command == "use" && m_OHBot->m_FunCommands) {
        uint32_t thingsleft = player->GetTheThingAmount();
        if (thingsleft != 0) {
            PlayerUsed(player->GetTheThing(), player->GetTheThingType(), User);
            thingsleft--;
            player->SetTheThingAmount(thingsleft);
        }
        else
            SendChat(player, "You have no things left.");
    }

        //
        // !swaprequest
        //
    else if ((Command == "swaprequest" || Command == "swr") && !Payload.empty() && !m_GameLoaded) {
        if (!player->GetSwapRequested()) {
            CGamePlayer *LastMatch = NULL;
            uint32_t Matches = GetPlayerFromNamePartial(Payload, &LastMatch);
            if (Matches == 0)
                SendChat(player, m_OHBot->m_Language->FoundNoMatchWithPlayername());
            else if (Matches == 1) {
                if (LastMatch->GetSwapTarget() == 255) {
                    SendChat(player, m_OHBot->m_Language->RequestedSwapWithPlayer(LastMatch->GetName()));
                    SendChat(LastMatch, m_OHBot->m_Language->PlayerRequestedSwapWithYou(User));
                    player->SetSwapRequested(true);
                    player->SetSwapTarget(LastMatch->GetPID());
                    LastMatch->SetSwapTarget(player->GetPID());
                } else if (LastMatch->GetSwapTarget() == player->GetPID()) {
                    SendAllChat(m_OHBot->m_Language->PlayersSwapped(User, LastMatch->GetName()));
                    SwapSlots((unsigned char) (GetSIDFromPID(player->GetPID())),
                              (unsigned char) (GetSIDFromPID(LastMatch->GetPID())));
                    player->SetSwapTarget(255);
                    LastMatch->SetSwapTarget(255);
                    LastMatch->SetSwapRequested(false);
                } else {
                    SendChat(player, m_OHBot->m_Language->PlayerIsAlreadySwapping(User));
                }
            }
            else
                SendChat(player, m_OHBot->m_Language->FoundMultiplyMatches());

        }
        else
            SendChat(player, m_OHBot->m_Language->YouAlreadyRequestedASwap());
    }

        //
        // !swapaccept
        //
    else if ((Command == "swapaccept" || Command == "swa") && !m_GameLoaded) {
        if (player->GetSwapTarget() != 255) {
            if (!player->GetSwapRequested()) {
                CGamePlayer *Player = GetPlayerFromPID(player->GetSwapTarget());
                if (Player) {
                    SendAllChat(m_OHBot->m_Language->PlayersSwapped(User, Player->GetName()));
                    SwapSlots((unsigned char) (GetSIDFromPID(player->GetPID())),
                              (unsigned char) (GetSIDFromPID(Player->GetPID())));
                    player->SetSwapTarget(255);
                    Player->SetSwapTarget(255);
                    Player->SetSwapRequested(false);
                }
                else {
                    SendChat(player, m_OHBot->m_Language->ThePlayerAlreadyLeft());
                    player->SetSwapTarget(255);
                }
            }
            else
                SendChat(player, m_OHBot->m_Language->TryingToSwapAcceptWhenRequested());
        }
        else
            SendChat(player, m_OHBot->m_Language->NoOneIsSwappingWithYou());
    }

        //
        // !swapabort
        //
    else if (Command == "swapabort" && player->GetSwapRequested() && !m_GameLoaded) {
        SendChat(player, m_OHBot->m_Language->AbortedTheSwap());
        player->SetSwapRequested(false);
        CGamePlayer *Player = GetPlayerFromPID(player->GetSwapTarget());
        player->SetSwapTarget(255);
        if (Player) {
            Player->SetSwapTarget(255);
            SendChat(Player, m_OHBot->m_Language->UserAbortedSwapWithYou(User));
        }

    }
    else if (Command == "eat") {
        uint32_t Random = rand() % 1;
        if (User == "Juliet" && Random == 1) {
            SendAllChat(
                    "[INFO] Juliet tried to eat his cookies, but he is too chubby to pick it up. He send them now to Mirar.");
            CGamePlayer *LastMatch = NULL;
            uint32_t Matches = GetPlayerFromNamePartial("Mirar", &LastMatch);
            if (Matches == 1) {
                LastMatch->SetCookie(LastMatch->GetCookies() + player->GetCookies());
            }
            player->SetCookie(0);
        }

        if (player->GetCookies() == 0) {
            SendChat(player, "You dont have any cookies to eat");
            return false;
        }
        if (!Payload.empty()) {
            SendAllChat("[INFO] Player " + User + " tried to eat a cookie with " + Payload +
                        " and dropped all his cookies on the floor.");
            player->SetCookie(0);
            return false;
        }
        player->SetCookie(player->GetCookies() - 1);
        SendAllChat("[INFO] Player " + User + " ate a cookie. He has now " + UTIL_ToString(player->GetCookies()) +
                    " cookies left in his jar.");
    }

    else if (Command == "info") {
        if (m_GameBalance)
            SendChat(player, m_OHBot->m_Language->EnabledBalanceForThisGame());
        if (m_OHBot->m_VoteMode) {
            SendChat(player, m_OHBot->m_Language->VoteHasBeenEnabledNotify());
        }
        if (m_OHBot->m_AllowVoteStart) {
            SendChat(player, m_OHBot->m_Language->VoteStartIsEnabledVotesRequired(
                    UTIL_ToString(m_OHBot->m_VoteStartMinPlayers)));
        }

        SendChat(player, m_OHBot->m_Language->WaitingForPlayersBeforeAutoStart(UTIL_ToString(m_AutoStartPlayers),
                                                                               UTIL_ToString(m_AutoStartPlayers -
                                                                                             GetNumHumanPlayers())));

    }
    return HideCommand;
}

void CGame::EventGameStarted() {
    CBaseGame::EventGameStarted();

    // record everything we need to ban each player in case we decide to do so later
    // this is because when a player leaves the game an admin might want to ban that player
    // but since the player has already left the game we don't have access to their information anymore
    // so we create a "potential ban" for each player and only store it in the database if requested to by an admin

    if (m_DBBans.empty()) {
        for (vector<CGamePlayer *>::iterator i = m_Players.begin(); i != m_Players.end(); ++i) {
            m_DBBans.push_back(
                    new CDBBan(0, (*i)->GetJoinedRealm(), (*i)->GetName(), (*i)->GetExternalIPString(), string(),
                               string(), string(), string(), string(), string(), string(), string(), string(),
                               (*i)->GetPenalityLevel()));
        }
    }
}

bool CGame::IsGameDataSaved() {
    return m_CallableGameAdd && m_CallableGameAdd->GetReady();
}

void CGame::SaveGameData() {
    Log->Info("[GAME: " + m_GameName + "] saving game data to database");
    m_CallableGameAdd = m_OHBot->m_DB->ThreadedGameAdd(
            m_OHBot->m_BNETs.size() == 1 ? m_OHBot->m_BNETs[0]->GetServer() : string(), m_DBGame->GetMap(), m_GameName,
            m_OwnerName, m_GameTicks / 1000, m_GameState, m_CreatorName, m_CreatorServer, m_GameType, m_LobbyLog,
            m_GameLog, m_DatabaseID, m_GameLoadedTime - m_CreationTime);
    m_OHBot->m_FinishedGames++;
    m_OHBot->m_CheckForFinishedGames = GetTime();
    DoGameUpdate(true);
}

bool CGame::IsAutoBanned(string name) {
    for (vector<string>::iterator i = m_AutoBans.begin(); i != m_AutoBans.end(); i++) {
        if (*i == name)
            return true;
    }

    return false;
}

bool CGame::CustomVoteKickReason(string reason) {
    transform(reason.begin(), reason.end(), reason.begin(), ::tolower);
    //Votekick reasons: maphack, fountainfarm, feeding, flaming, game ruin
    if (reason.find("maphack") != string::npos || reason.find("fountainfarm") != string::npos ||
        reason.find("feeding") != string::npos || reason.find("flaming") != string::npos ||
        reason.find("gameruin") != string::npos || reason.find("lagging") != string::npos)
        return true;

    return false;
}

string CGame::GetRuleTags() {
    string Tags;
    uint32_t saver = 0;
    for (vector<string>::iterator i = m_OHBot->m_Rules.begin(); i != m_OHBot->m_Rules.end(); i++) {
        string tag;
        stringstream SS;
        SS << *i;
        SS >> tag;
        if (Tags.empty())
            Tags = tag;
        else
            Tags += ", " + tag;
        ++saver;
        if (saver > 10) {
            Log->Warning("There to many rules, stopping after 10.");
            break;
        }
    }
    return Tags;
}

string CGame::GetRule(string tag) {
    transform(tag.begin(), tag.end(), tag.begin(), ::tolower);
    uint32_t saver = 0;
    for (vector<string>::iterator i = m_OHBot->m_Rules.begin(); i != m_OHBot->m_Rules.end(); i++) {
        string rtag;
        string rule;
        stringstream SS;
        SS << *i;
        SS >> rtag;

        if (!SS.fail() && (rtag == tag || rtag.find(tag) != string::npos)) {
            if (SS.eof())
                Log->Warning("[RULE: " + rtag + "] missing input #2 (Rule).");
            else {
                getline(SS, rule);
                string::size_type Start = rule.find_first_not_of(" ");

                if (Start != string::npos)
                    rule = rule.substr(Start);

                return "[" + rtag + "] " + rule;
            }
            return m_OHBot->m_Language->WrongContactBotOwner();
        }
        ++saver;
        if (saver > 10) {
            Log->Warning("There to many rules, stopping after 10.");
            return m_OHBot->m_Language->WrongContactBotOwner();
            break;
        }
    }
    return m_OHBot->m_Language->RuleTagNotify();
}

void CGame::PlayerUsed(string thething, uint32_t thetype, string playername) {
    switch (thetype) {
        case 1:
            SendAllChat(m_OHBot->m_Language->UserUsed1(playername, thething));
            break;
        case 2:
            SendAllChat(m_OHBot->m_Language->UserUsed2(playername, thething));
            break;
        case 3:
            SendAllChat(m_OHBot->m_Language->UserUsed3(playername, thething));
            break;
        case 4:
            SendAllChat(m_OHBot->m_Language->UserUsed4(playername, thething));
            break;
        case 5:
            SendAllChat(m_OHBot->m_Language->UserUsed5(playername, thething));
            break;
        default:
            SendAllChat(m_OHBot->m_Language->UserUsed6(playername, thething));
            break;
    }
}
