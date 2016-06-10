/**
 * Copyright [2016] [m-unkel]
 * Mail: info@unive.de
 * URL: https://github.com/m-unkel/OHSystem
 *
 * This is modified from OHSYSTEM https://github.com/OHSystem
 *
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
 *
 */

#include "../ohbot.h"
#include "../utils/util.h"
#include "../config.h"
#include "../language.h"
#include "../network/socket.h"
#include "../network/commandpacket.h"
#include "../database/ghostdb.h"
#include "../utils/bncsutilinterface.h"
#include "../bnls/bnlsclient.h"
#include "bnetprotocol.h"
#include "bnet.h"
#include "../command.h"
#include "../map.h"
#include "../utils/packed.h"
#include "../savegame.h"
#include "../replay.h"
#include "../game/gameprotocol.h"
#include "../game/game_base.h"
#include "../game/gameplayer.h"

#include <boost/filesystem.hpp>

using namespace boost::filesystem;

//
// CBNET
//

CBNET::CBNET(COHBot *nOHBot, string nServer, string nServerAlias, string nBNLSServer, uint16_t nBNLSPort,
             uint32_t nBNLSWardenCookie, string nCDKeyROC, string nCDKeyTFT, string nCountryAbbrev, string nCountry,
             uint32_t nLocaleID, string nUserName, string nUserPassword, string nFirstChannel, char nCommandTrigger,
             bool nHoldFriends, bool nHoldClan, bool nPublicCommands, unsigned char nWar3Version, BYTEARRAY nEXEVersion,
             BYTEARRAY nEXEVersionHash, string nPasswordHashType, string nPVPGNRealmName, uint32_t nMaxMessageLength,
             uint32_t nHostCounterID, uint8_t nup) {
    // todotodo: append path seperator to Warcraft3Path if needed
    UpTime = nup;
    m_OHBot = nOHBot;
    m_Socket = new CTCPClient();
    m_Protocol = new CBNETProtocol();
    m_BNLSClient = NULL;
    m_BNCSUtil = new CBNCSUtilInterface(nUserName, nUserPassword);
    m_CallablePList = m_OHBot->m_DB->ThreadedPList(nServer);
    m_CallableBanList = m_OHBot->m_DB->ThreadedBanList(nServer);
    m_CallableTBRemove = m_OHBot->m_DB->ThreadedTBRemove(nServer);
    m_Exiting = false;
    m_Server = nServer;
    string LowerServer = m_Server;
    m_AdminLog = vector<string>();
    transform(LowerServer.begin(), LowerServer.end(), LowerServer.begin(), ::tolower);
    m_OHBot->m_CheckForFinishedGames = GetTime();
    LastUpdateTime = GetTime();
    if (!nServerAlias.empty())
        m_ServerAlias = nServerAlias;
    else if (LowerServer == "useast.battle.net")
        m_ServerAlias = "USEast";
    else if (LowerServer == "uswest.battle.net")
        m_ServerAlias = "USWest";
    else if (LowerServer == "asia.battle.net")
        m_ServerAlias = "Asia";
    else if (LowerServer == "europe.battle.net")
        m_ServerAlias = "Europe";
    else
        m_ServerAlias = m_Server;

    m_FakeRealm = false;
    if (m_ServerAlias == "WC3Connect" || m_ServerAlias == "Garena") {
        m_FakeRealm = true;
    }

    if (nPasswordHashType == "pvpgn" && !nBNLSServer.empty()) {
        CONSOLE_Print("[BNET: " + m_ServerAlias +
                      "] pvpgn connection found with a configured BNLS server, ignoring BNLS server");
        nBNLSServer.clear();
        nBNLSPort = 0;
        nBNLSWardenCookie = 0;
    }

    m_BNLSServer = nBNLSServer;
    m_BNLSPort = nBNLSPort;
    m_BNLSWardenCookie = nBNLSWardenCookie;
    m_CDKeyROC = nCDKeyROC;
    m_CDKeyTFT = nCDKeyTFT;

    // remove dashes and spaces from CD keys and convert to uppercase

    m_CDKeyROC.erase(remove(m_CDKeyROC.begin(), m_CDKeyROC.end(), '-'), m_CDKeyROC.end());
    m_CDKeyTFT.erase(remove(m_CDKeyTFT.begin(), m_CDKeyTFT.end(), '-'), m_CDKeyTFT.end());
    m_CDKeyROC.erase(remove(m_CDKeyROC.begin(), m_CDKeyROC.end(), ' '), m_CDKeyROC.end());
    m_CDKeyTFT.erase(remove(m_CDKeyTFT.begin(), m_CDKeyTFT.end(), ' '), m_CDKeyTFT.end());
    transform(m_CDKeyROC.begin(), m_CDKeyROC.end(), m_CDKeyROC.begin(), (int (*)(int)) toupper);
    transform(m_CDKeyTFT.begin(), m_CDKeyTFT.end(), m_CDKeyTFT.begin(), (int (*)(int)) toupper);

    if (m_CDKeyROC.size() != 26 && !m_FakeRealm)
        CONSOLE_Print("[BNET: " + m_ServerAlias +
                      "] warning - your ROC CD key is not 26 characters long and is probably invalid");

    if (m_OHBot->m_TFT && m_CDKeyTFT.size() != 26 && !m_FakeRealm)
        CONSOLE_Print("[BNET: " + m_ServerAlias +
                      "] warning - your TFT CD key is not 26 characters long and is probably invalid");

    m_CountryAbbrev = nCountryAbbrev;
    m_Country = nCountry;
    m_LocaleID = nLocaleID;
    m_UserName = nUserName;
    m_UserPassword = nUserPassword;
    m_FirstChannel = nFirstChannel;
    m_CommandTrigger = nCommandTrigger;
    m_War3Version = nWar3Version;
    m_EXEVersion = nEXEVersion;
    m_EXEVersionHash = nEXEVersionHash;
    m_PasswordHashType = nPasswordHashType;
    m_PVPGNRealmName = nPVPGNRealmName;
    m_MaxMessageLength = nMaxMessageLength;
    m_HostCounterID = nHostCounterID;
    m_LastDisconnectedTime = 0;
    m_LastConnectionAttemptTime = 0;
    m_LastNullTime = 0;
    m_LastOutPacketTicks = 0;
    m_LastOutPacketSize = 0;
    m_FrequencyDelayTimes = 0;
    m_LastAdminRefreshTime = GetTime();
    m_LastBanRefreshTime = GetTime();
    m_FirstConnect = true;
    m_WaitingToConnect = true;
    m_LoggedIn = false;
    m_InChat = false;
    m_HoldFriends = nHoldFriends;
    m_HoldClan = nHoldClan;
    m_PublicCommands = nPublicCommands;
    m_LastLogUpdateTime = GetTime();
}

CBNET::~CBNET() {
    delete m_Socket;
    delete m_Protocol;
    delete m_BNLSClient;

    while (!m_Packets.empty()) {
        delete m_Packets.front();
        m_Packets.pop();
    }

    delete m_BNCSUtil;

    for (vector<CIncomingFriendList *>::iterator i = m_Friends.begin(); i != m_Friends.end(); ++i)
        delete *i;

    for (vector<CIncomingClanList *>::iterator i = m_Clans.begin(); i != m_Clans.end(); ++i)
        delete *i;

    for (vector<PairedRegAdd>::iterator i = m_PairedRegAdds.begin(); i != m_PairedRegAdds.end(); ++i)
        m_OHBot->m_Callables.push_back(i->second);

    for (vector<PairedSS>::iterator i = m_PairedSSs.begin(); i != m_PairedSSs.end(); ++i)
        m_OHBot->m_Callables.push_back(i->second);

    for (vector<Pairedpm>::iterator i = m_Pairedpms.begin(); i != m_Pairedpms.end(); ++i)
        m_OHBot->m_Callables.push_back(i->second);

    for (vector<PairedPassCheck>::iterator i = m_PairedPassChecks.begin(); i != m_PairedPassChecks.end(); ++i)
        m_OHBot->m_Callables.push_back(i->second);

    for (vector<Pairedpenp>::iterator i = m_Pairedpenps.begin(); i != m_Pairedpenps.end(); ++i)
        m_OHBot->m_Callables.push_back(i->second);

    for (vector<PairedPUp>::iterator i = m_PairedPUps.begin(); i != m_PairedPUps.end(); ++i)
        m_OHBot->m_Callables.push_back(i->second);

    for (vector<PairedBanCheck2>::iterator i = m_PairedBanCheck2s.begin(); i != m_PairedBanCheck2s.end(); ++i)
        m_OHBot->m_Callables.push_back(i->second);

    for (vector<PairedBanCount>::iterator i = m_PairedBanCounts.begin(); i != m_PairedBanCounts.end(); ++i)
        m_OHBot->m_Callables.push_back(i->second);

    for (vector<PairedBanAdd>::iterator i = m_PairedBanAdds.begin(); i != m_PairedBanAdds.end(); ++i)
        m_OHBot->m_Callables.push_back(i->second);

    for (vector<PairedBanRemove>::iterator i = m_PairedBanRemoves.begin(); i != m_PairedBanRemoves.end(); ++i)
        m_OHBot->m_Callables.push_back(i->second);

    for (vector<PairedGSCheck>::iterator i = m_PairedGSChecks.begin(); i != m_PairedGSChecks.end(); ++i)
        m_OHBot->m_Callables.push_back(i->second);

    for (vector<PairedRankCheck>::iterator i = m_PairedRankChecks.begin(); i != m_PairedRankChecks.end(); ++i)
        m_OHBot->m_Callables.push_back(i->second);

    for (vector<PairedStreakCheck>::iterator i = m_PairedStreakChecks.begin(); i != m_PairedStreakChecks.end(); ++i)
        m_OHBot->m_Callables.push_back(i->second);

    for (vector<PairedINCheck>::iterator i = m_PairedINChecks.begin(); i != m_PairedINChecks.end(); ++i)
        m_OHBot->m_Callables.push_back(i->second);

    for (vector<PairedSCheck>::iterator i = m_PairedSChecks.begin(); i != m_PairedSChecks.end(); ++i)
        m_OHBot->m_Callables.push_back(i->second);

    for (vector<PairedGameUpdate>::iterator i = m_PairedGameUpdates.begin(); i != m_PairedGameUpdates.end(); ++i)
        m_OHBot->m_Callables.push_back(i->second);

    for (vector<BotStatusUpdate>::iterator i = m_BotStatusUpdate.begin(); i != m_BotStatusUpdate.end(); ++i)
        m_OHBot->m_Callables.push_back(i->second);

    for (vector<BotStatusCreate>::iterator i = m_BotStatusCreate.begin(); i != m_BotStatusCreate.end(); ++i)
        m_OHBot->m_Callables.push_back(i->second);

    if (m_CallablePList)
        m_OHBot->m_Callables.push_back(m_CallablePList);

    if (m_CallableBanList)
        m_OHBot->m_Callables.push_back(m_CallableBanList);

    if (m_CallableTBRemove)
        m_OHBot->m_Callables.push_back(m_CallableTBRemove);

    for (vector<CDBBan *>::iterator i = m_Bans.begin(); i != m_Bans.end(); ++i)
        delete *i;
}

BYTEARRAY CBNET::GetUniqueName() {
    return m_Protocol->GetUniqueName();
}

unsigned int CBNET::SetFD(void *fd, void *send_fd, int *nfds) {
    unsigned int NumFDs = 0;

    if (!m_Socket->HasError() && m_Socket->GetConnected()) {
        m_Socket->SetFD((fd_set *) fd, (fd_set *) send_fd, nfds);
        ++NumFDs;

        if (m_BNLSClient)
            NumFDs += m_BNLSClient->SetFD(fd, send_fd, nfds);
    }

    return NumFDs;
}

bool CBNET::Update(void *fd, void *send_fd) {
    //
    // update callables
    //

    for (vector<PairedRegAdd>::iterator i = m_PairedRegAdds.begin(); i != m_PairedRegAdds.end();) {
        if (i->second->GetReady()) {
            uint32_t Result = i->second->GetResult();
            m_LastRegisterProcess = GetTime();
            if (Result == 1)
                QueueChatCommand(m_OHBot->m_Language->SuccessfullyRegistered(), i->first, true);
            else if (Result == 2)
                QueueChatCommand(m_OHBot->m_Language->SuccessfullyRegistered(), i->first, true);
            else if (Result == 3)
                QueueChatCommand(m_OHBot->m_Language->WrongPassword(), i->first, true);
            else if (Result == 4)
                QueueChatCommand(m_OHBot->m_Language->WrongEMail(), i->first, true);
            else if (Result == 5)
                QueueChatCommand(m_OHBot->m_Language->NameAlreadyUsed(), i->first, true);
            else if (Result == 6)
                QueueChatCommand(m_OHBot->m_Language->NoAccountToConfirm(), i->first, true);
            else
                QueueChatCommand(m_OHBot->m_Language->WrongContactBotOwner(), i->first, true);


            m_OHBot->m_DB->RecoverCallable(i->second);
            delete i->second;
            i = m_PairedRegAdds.erase(i);
        }
        else
            ++i;
    }

    for (vector<PairedSS>::iterator i = m_PairedSSs.begin(); i != m_PairedSSs.end();) {
        if (i->second->GetReady()) {
            string Result = i->second->GetResult();
            if (i->second->GetType() == "betcheck")
                QueueChatCommand(m_OHBot->m_Language->BetPoints(i->second->GetUser(), Result), i->first,
                                 !i->first.empty());

            else if (i->second->GetType() == "aliascheck") {
                if (Result != "failed")
                    QueueChatCommand(Result, i->first, !i->first.empty());
                else
                    QueueChatCommand(m_OHBot->m_Language->NoRecordFoundForUser(i->second->GetUser()), i->first,
                                     !i->first.empty());
            }
            else if (i->second->GetType() == "rpp") {
                if (Result != "failed")
                    QueueChatCommand(Result, i->first, !i->first.empty());
                else
                    QueueChatCommand(m_OHBot->m_Language->WrongContactBotOwner(), i->first, !i->first.empty());
            }
            else if (i->second->GetType() == "top") {
                if (Result != "failed")
                    QueueChatCommand(Result, i->first, !i->first.empty());
                else
                    QueueChatCommand(m_OHBot->m_Language->WrongContactBotOwner(), i->first, !i->first.empty());
            }
            else if (i->second->GetType() == "forcedgproxy") {
                if (Result != "failed")
                    QueueChatCommand(Result, i->first, !i->first.empty());
                else
                    QueueChatCommand(m_OHBot->m_Language->WrongContactBotOwner(), i->first, !i->first.empty());
            } else if (i->second->GetType() == "gamelist") {
                if (Result != "failed") {
                    string msg = Result;
                    do {
                        if (msg.length() > 200) {
                            QueueChatCommand(msg.substr(0, 200), i->first, !i->first.empty());
                            msg = msg.substr(200);
                        } else {
                            QueueChatCommand(msg, i->first, !i->first.empty());
                            msg = "";
                        }
                    } while (msg.length() != 0);
                } else
                    QueueChatCommand(m_OHBot->m_Language->WrongContactBotOwner(), i->first, !i->first.empty());
            }

            m_OHBot->m_DB->RecoverCallable(i->second);
            delete i->second;
            i = m_PairedSSs.erase(i);
        }
        else
            ++i;
    }

    for (vector<Pairedpm>::iterator i = m_Pairedpms.begin(); i != m_Pairedpms.end();) {
        if (i->second->GetReady()) {
            uint32_t Result = i->second->GetResult();

            if (Result == -1)
                CONSOLE_Print(m_OHBot->m_Language->SuccessfullyStoredMessage());
            else if (Result > 0) {
                QueueChatCommand(m_OHBot->m_Language->NewMessages(i->first, UTIL_ToString(Result)), i->first, true);
            }

            m_OHBot->m_DB->RecoverCallable(i->second);
            delete i->second;
            i = m_Pairedpms.erase(i);
        }
        else
            ++i;
    }

    for (vector<PairedPassCheck>::iterator i = m_PairedPassChecks.begin(); i != m_PairedPassChecks.end();) {
        if (i->second->GetReady()) {
            uint32_t Result = i->second->GetResult();
            if (Result == 1) {
                if (m_OHBot->m_CurrentGame) {
                    for (vector<CGamePlayer *>::iterator k = m_OHBot->m_CurrentGame->m_Players.begin();
                         k != m_OHBot->m_CurrentGame->m_Players.end(); ++k) {
                        CGamePlayer *Player = m_OHBot->m_CurrentGame->GetPlayerFromName((*k)->GetName(), true);
                        if (Player) {
                            if (Player->GetName() == i->first) {
                                CONSOLE_Print(m_OHBot->m_Language->SuccessfullyTypedPassword(Player->GetName()));
                                Player->SetPasswordProt(false);
                                Player->SetSpoofed(true);
                            }
                        }
                    }
                }
            }

            m_OHBot->m_DB->RecoverCallable(i->second);
            delete i->second;
            i = m_PairedPassChecks.erase(i);
        }
        else
            ++i;
    }

    for (vector<Pairedpenp>::iterator i = m_Pairedpenps.begin(); i != m_Pairedpenps.end();) {
        if (i->second->GetReady()) {
            uint32_t Result = i->second->GetResult();
            if (i->second->GetType() == "check") {
                if (Result != 0)
                    QueueChatCommand(
                            m_OHBot->m_Language->UserGotPenalityPoints(i->second->GetName(), UTIL_ToString(Result)),
                            i->first, !i->first.empty());
                else
                    QueueChatCommand(m_OHBot->m_Language->UserGotNoPenalityPoints(i->second->GetName()), i->first,
                                     !i->first.empty());
            }
            else if (i->second->GetType() == "checkall") {
                if (Result != 0)
                    QueueChatCommand(
                            m_OHBot->m_Language->UserGotPenalityPoints(i->second->GetName(), UTIL_ToString(Result)),
                            i->first, !i->first.empty());
                else
                    QueueChatCommand(m_OHBot->m_Language->UserGotNoPenalityPoints(i->second->GetName()), i->first,
                                     !i->first.empty());
            }
            else if (i->second->GetType() == "add") {
                if (Result == 1)
                    QueueChatCommand(
                            m_OHBot->m_Language->AddedPenalityPoints(i->second->GetName(), i->second->GetAdmin(),
                                                                     UTIL_ToString(i->second->GetAmount())), i->first,
                            !i->first.empty());
                else if (Result == 2)
                    QueueChatCommand(m_OHBot->m_Language->BannedUserForReachingTooManyPPoints(i->second->GetName()),
                                     i->first, !i->first.empty());
                else
                    CONSOLE_Print(m_OHBot->m_Language->FailedToAddPPoint());
            }
            else
                CONSOLE_Print(m_OHBot->m_Language->WrongContactBotOwner());

            m_OHBot->m_DB->RecoverCallable(i->second);
            delete i->second;
            i = m_Pairedpenps.erase(i);
        }
        else
            ++i;
    }

    for (vector<PairedBanCount>::iterator i = m_PairedBanCounts.begin(); i != m_PairedBanCounts.end();) {
        if (i->second->GetReady()) {
            uint32_t Count = i->second->GetResult();

            if (Count == 0)
                QueueChatCommand(m_OHBot->m_Language->ThereAreNoBannedUsers(m_Server), i->first, !i->first.empty());
            else if (Count == 1)
                QueueChatCommand(m_OHBot->m_Language->ThereIsBannedUser(m_Server), i->first, !i->first.empty());
            else
                QueueChatCommand(m_OHBot->m_Language->ThereAreBannedUsers(m_Server, UTIL_ToString(Count)), i->first,
                                 !i->first.empty());

            m_OHBot->m_DB->RecoverCallable(i->second);
            delete i->second;
            i = m_PairedBanCounts.erase(i);
        }
        else
            ++i;
    }

    for (vector<PairedBanCheck2>::iterator i = m_PairedBanCheck2s.begin(); i != m_PairedBanCheck2s.end();) {
        if (i->second->GetReady()) {
            string Result = i->second->GetResult();
            if (Result == "norec")
                QueueChatCommand(m_OHBot->m_Language->NoRecordFoundForUser(i->first), i->first, !i->first.empty());
            else if (Result == "fail")
                QueueChatCommand(m_OHBot->m_Language->NotIPBanned(), i->first, !i->first.empty());
            else
                QueueChatCommand(m_OHBot->m_Language->FoundIPBans(Result), i->first, !i->first.empty());

            m_OHBot->m_DB->RecoverCallable(i->second);
            delete i->second;
            i = m_PairedBanCheck2s.erase(i);
        }
        else
            ++i;
    }

    for (vector<PairedBanAdd>::iterator i = m_PairedBanAdds.begin(); i != m_PairedBanAdds.end();) {
        if (i->second->GetReady()) {
            uint32_t Result = i->second->GetResult();
            if (Result == 1)
                QueueChatCommand(m_OHBot->m_Language->ErrorBanningUserAlreadyPermBanned(i->second->GetUser()), i->first,
                                 !i->first.empty());
            else if (Result == 2)
                QueueChatCommand(m_OHBot->m_Language->ErrorBanningUserAlreadyLongerBanned(i->second->GetUser()),
                                 i->first, !i->first.empty());
            else if (Result >= 3 && Result <= 5) {
                AddBan(i->second->GetUser(), i->second->GetIP(), i->second->GetGameName(), i->second->GetAdmin(),
                       i->second->GetReason());
                if (Result == 3)
                    QueueChatCommand(
                            m_OHBot->m_Language->SuccessfullyBannedUser(i->second->GetUser(), i->second->GetServer()),
                            i->first, !i->first.empty());
                else if (Result == 4)
                    QueueChatCommand(m_OHBot->m_Language->SuccessfullyUpdatedBannedUser(i->second->GetUser(),
                                                                                        i->second->GetServer()),
                                     i->first, !i->first.empty());
                else if (Result == 5)
                    QueueChatCommand(m_OHBot->m_Language->SuccessfullyPermBannedUser(i->second->GetUser(),
                                                                                     i->second->GetServer()), i->first,
                                     !i->first.empty());
            }
            else
                QueueChatCommand(m_OHBot->m_Language->WrongContactBotOwner(), i->first, !i->first.empty());

            m_OHBot->m_DB->RecoverCallable(i->second);
            delete i->second;
            i = m_PairedBanAdds.erase(i);
        }
        else
            ++i;
    }

    for (vector<PairedPUp>::iterator i = m_PairedPUps.begin(); i != m_PairedPUps.end();) {
        if (i->second->GetReady()) {
            bool Result = i->second->GetResult();
            if (Result && m_OHBot->m_RanksLoaded)
                QueueChatCommand(m_OHBot->m_Language->ChangedRankOfUser(i->second->GetName(),
                                                                        GetLevelName(i->second->GetLevel()),
                                                                        i->second->GetRealm()));
            else if (Result && !m_OHBot->m_RanksLoaded)
                QueueChatCommand(m_OHBot->m_Language->RanksNotLoaded());
            else
                QueueChatCommand(m_OHBot->m_Language->NoRecordFoundForUser(i->second->GetName()));

            m_OHBot->m_DB->RecoverCallable(i->second);
            delete i->second;
            i = m_PairedPUps.erase(i);
        }
        else
            ++i;
    }

    for (vector<PairedBanRemove>::iterator i = m_PairedBanRemoves.begin(); i != m_PairedBanRemoves.end();) {
        if (i->second->GetReady()) {
            if (i->second->GetResult()) {
                RemoveBan(i->second->GetUser());
                QueueChatCommand(m_OHBot->m_Language->UnbannedUser(i->second->GetUser()), i->first, !i->first.empty());
            }
            else
                QueueChatCommand(m_OHBot->m_Language->ErrorUnbanningUser(i->second->GetUser()), i->first,
                                 !i->first.empty());

            m_OHBot->m_DB->RecoverCallable(i->second);
            delete i->second;
            i = m_PairedBanRemoves.erase(i);
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
                    string Streak = UTIL_ToString(StatsPlayerSummary->GetStreak());
                    if (StatsPlayerSummary->GetStreak() < 0)
                        Streak = "-" + Streak;

                    QueueChatCommand(m_OHBot->m_Language->HasPlayedGamesWithThisBot(i->second->GetName(),
                                                                                    UTIL_ToString(
                                                                                            StatsPlayerSummary->GetScore(),
                                                                                            0),
                                                                                    UTIL_ToString(
                                                                                            StatsPlayerSummary->GetGames()),
                                                                                    UTIL_ToString(
                                                                                            StatsPlayerSummary->GetWinPerc(),
                                                                                            2),
                                                                                    Streak,
                                                                                    m_OHBot->GetMonthInWords(Month),
                                                                                    Year
                    ));
                }
                else {
                    if (i->first != i->second->GetName())
                        QueueChatCommand(m_OHBot->m_Language->UserHasAHiddenAcc(StatsPlayerSummary->GetPlayer()));
                    else {
                        string Streak = UTIL_ToString(StatsPlayerSummary->GetStreak());
                        if (StatsPlayerSummary->GetStreak() < 0)
                            Streak = "-" + UTIL_ToString(StatsPlayerSummary->GetStreak());

                        QueueChatCommand(m_OHBot->m_Language->HasPlayedGamesWithThisBot(i->second->GetName(),
                                                                                        UTIL_ToString(
                                                                                                StatsPlayerSummary->GetScore(),
                                                                                                0),
                                                                                        UTIL_ToString(
                                                                                                StatsPlayerSummary->GetGames()),
                                                                                        UTIL_ToString(
                                                                                                StatsPlayerSummary->GetWinPerc(),
                                                                                                2),
                                                                                        Streak,
                                                                                        m_OHBot->GetMonthInWords(Month),
                                                                                        Year
                        ), i->first, true);
                    }
                }
            }
            else
                QueueChatCommand(m_OHBot->m_Language->HasntPlayedGamesWithThisBot(i->second->GetName(),
                                                                                  m_OHBot->GetMonthInWords(Month),
                                                                                  Year), i->first, !i->first.empty());

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
                    if (m_OHBot->m_RanksLoaded)
                        QueueChatCommand(m_OHBot->m_Language->RankOfUser(i->second->GetName(), Time,
                                                                         m_OHBot->GetAliasName(i->second->GetAlias()),
                                                                         StatsPlayerSummary->GetRank(),
                                                                         UTIL_ToString(IsLevel(i->second->GetName())),
                                                                         GetLevelName(IsLevel(i->second->GetName()))),
                                         i->first, !i->first.empty());
                    else
                        QueueChatCommand(m_OHBot->m_Language->RankOfUserWithoutLevel(i->second->GetName(), Time,
                                                                                     m_OHBot->GetAliasName(
                                                                                             i->second->GetAlias()),
                                                                                     StatsPlayerSummary->GetRank()),
                                         i->first, !i->first.empty());
                } else {
                    if (i->first != i->second->GetName())
                        QueueChatCommand(m_OHBot->m_Language->UserHasAHiddenAcc(StatsPlayerSummary->GetPlayer()));
                    else {
                        if (m_OHBot->m_RanksLoaded)
                            QueueChatCommand(m_OHBot->m_Language->RankOfUser(i->second->GetName(), Time,
                                                                             m_OHBot->GetAliasName(
                                                                                     i->second->GetAlias()),
                                                                             StatsPlayerSummary->GetRank(),
                                                                             UTIL_ToString(
                                                                                     IsLevel(i->second->GetName())),
                                                                             GetLevelName(
                                                                                     IsLevel(i->second->GetName()))),
                                             i->first, true);
                        else
                            QueueChatCommand(m_OHBot->m_Language->RankOfUserWithoutLevel(i->second->GetName(), Time,
                                                                                         m_OHBot->GetAliasName(
                                                                                                 i->second->GetAlias()),
                                                                                         StatsPlayerSummary->GetRank()),
                                             i->first, true);
                    }
                }
            }
            else
                QueueChatCommand(m_OHBot->m_Language->HasntPlayedAliasGamesWithThisBot(i->second->GetName(),
                                                                                       m_OHBot->GetMonthInWords(Month),
                                                                                       Year,
                                                                                       m_OHBot->GetAliasName(
                                                                                               i->second->GetAlias())),
                                 i->first, !i->first.empty());

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
                if (!StatsPlayerSummary->GetHidden())
                    QueueChatCommand(m_OHBot->m_Language->StreakOfUser(StatsPlayerSummary->GetPlayer(), Time,
                                                                       m_OHBot->GetAliasName(i->second->GetAlias()),
                                                                       (StatsPlayerSummary->GetStreak() != 0
                                                                        ? UTIL_ToString(StatsPlayerSummary->GetStreak())
                                                                        : "-" + UTIL_ToString(
                                                                                       StatsPlayerSummary->GetLosingStreak())),
                                                                       UTIL_ToString(
                                                                               StatsPlayerSummary->GetMaxStreak()),
                                                                       UTIL_ToString(
                                                                               StatsPlayerSummary->GetMaxLosingStreak())));
                else {
                    if (i->first != i->second->GetName())
                        QueueChatCommand(m_OHBot->m_Language->UserHasAHiddenAcc(StatsPlayerSummary->GetPlayer()));
                    else
                        QueueChatCommand(m_OHBot->m_Language->StreakOfUser(StatsPlayerSummary->GetPlayer(), Time,
                                                                           m_OHBot->GetAliasName(i->second->GetAlias()),
                                                                           (StatsPlayerSummary->GetStreak() != 0
                                                                            ? UTIL_ToString(
                                                                                           StatsPlayerSummary->GetStreak())
                                                                            : "-" + UTIL_ToString(
                                                                                           StatsPlayerSummary->GetLosingStreak())),
                                                                           UTIL_ToString(
                                                                                   StatsPlayerSummary->GetMaxStreak()),
                                                                           UTIL_ToString(
                                                                                   StatsPlayerSummary->GetMaxLosingStreak())),
                                         i->first, true);
                }
            }
            else
                QueueChatCommand(m_OHBot->m_Language->HasntPlayedAliasGamesWithThisBot(i->second->GetName(),
                                                                                       m_OHBot->GetMonthInWords(Month),
                                                                                       Year,
                                                                                       m_OHBot->GetAliasName(
                                                                                               i->second->GetAlias())),
                                 i->first, !i->first.empty());

            m_OHBot->m_DB->RecoverCallable(i->second);
            delete i->second;
            i = m_PairedStreakChecks.erase(i);
        }
        else
            ++i;
    }

    for (vector<PairedINCheck>::iterator i = m_PairedINChecks.begin(); i != m_PairedINChecks.end();) {
        if (i->second->GetReady()) {
            CDBInboxSummary *InboxSummary = i->second->GetResult();

            if (InboxSummary)
                QueueChatCommand("[" + InboxSummary->GetUser() + "] " + InboxSummary->GetMessage(), i->first, true);
            else
                QueueChatCommand(m_OHBot->m_Language->ErrorInboxEmpty(), i->first, true);

            m_OHBot->m_DB->RecoverCallable(i->second);
            delete i->second;
            i = m_PairedINChecks.erase(i);
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
                    Summary = m_OHBot->m_Language->HasPlayedAliasGamesWithThisBot2(i->second->GetName(),
                                                                                   UTIL_ToString(
                                                                                           StatsPlayerSummary->GetGames()),
                                                                                   UTIL_ToString(
                                                                                           StatsPlayerSummary->GetWins()),
                                                                                   UTIL_ToString(
                                                                                           StatsPlayerSummary->GetLosses()),
                                                                                   UTIL_ToString(
                                                                                           StatsPlayerSummary->GetDraw()),
                                                                                   UTIL_ToString(
                                                                                           StatsPlayerSummary->GetKills()),
                                                                                   UTIL_ToString(
                                                                                           StatsPlayerSummary->GetDeaths()),
                                                                                   UTIL_ToString(
                                                                                           StatsPlayerSummary->GetCreeps()),
                                                                                   UTIL_ToString(
                                                                                           StatsPlayerSummary->GetDenies()),
                                                                                   UTIL_ToString(
                                                                                           StatsPlayerSummary->GetAssists()),
                                                                                   UTIL_ToString(
                                                                                           StatsPlayerSummary->GetNeutrals()),
                                                                                   UTIL_ToString(
                                                                                           StatsPlayerSummary->GetTowers()),
                                                                                   UTIL_ToString(
                                                                                           StatsPlayerSummary->GetRax()),
                                                                                   UTIL_ToString(
                                                                                           StatsPlayerSummary->GetAvgKills(),
                                                                                           1),
                                                                                   UTIL_ToString(
                                                                                           StatsPlayerSummary->GetAvgDeaths(),
                                                                                           1),
                                                                                   UTIL_ToString(
                                                                                           StatsPlayerSummary->GetAvgCreeps(),
                                                                                           1),
                                                                                   UTIL_ToString(
                                                                                           StatsPlayerSummary->GetAvgDenies(),
                                                                                           1),
                                                                                   UTIL_ToString(
                                                                                           StatsPlayerSummary->GetAvgAssists(),
                                                                                           1),
                                                                                   UTIL_ToString(
                                                                                           StatsPlayerSummary->GetAvgNeutrals(),
                                                                                           1),
                                                                                   UTIL_ToString(
                                                                                           StatsPlayerSummary->GetAvgTowers(),
                                                                                           1),
                                                                                   UTIL_ToString(
                                                                                           StatsPlayerSummary->GetAvgRax(),
                                                                                           1),
                                                                                   m_OHBot->GetMonthInWords(Month),
                                                                                   Year,
                                                                                   m_OHBot->GetAliasName(
                                                                                           i->second->GetAlias())
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
                    QueueChatCommand(Summary, i->first, !i->first.empty());
                } else {
                    if (i->first != i->second->GetName())
                        QueueChatCommand(m_OHBot->m_Language->UserHasAHiddenAcc(StatsPlayerSummary->GetPlayer()));
                    else
                        QueueChatCommand(Summary, i->first, true);
                }
            }
            else
                QueueChatCommand(m_OHBot->m_Language->HasntPlayedAliasGamesWithThisBot(i->second->GetName(),
                                                                                       m_OHBot->GetMonthInWords(Month),
                                                                                       Year,
                                                                                       m_OHBot->GetAliasName(
                                                                                               i->second->GetAlias())),
                                 i->first, !i->first.empty());

            m_OHBot->m_DB->RecoverCallable(i->second);
            delete i->second;
            i = m_PairedSChecks.erase(i);
        }
        else
            ++i;
    }

    for (vector<PairedGameUpdate>::iterator i = m_PairedGameUpdates.begin(); i != m_PairedGameUpdates.end();) {
        if (i->second->GetReady()) {
            string response = i->second->GetResult();
            QueueChatCommand(response, i->first, !i->first.empty());

            m_OHBot->m_DB->RecoverCallable(i->second);
            delete i->second;
            i = m_PairedGameUpdates.erase(i);
        }
        else
            ++i;
    }

    for (vector<BotStatusUpdate>::iterator i = m_BotStatusUpdate.begin(); i != m_BotStatusUpdate.end();) {
        if (i->second->GetReady()) {
            m_OHBot->m_DB->RecoverCallable(i->second);
            delete i->second;
            i = m_BotStatusUpdate.erase(i);
        }
        else
            ++i;
    }

    for (vector<BotStatusCreate>::iterator i = m_BotStatusCreate.begin(); i != m_BotStatusCreate.end();) {
        if (i->second->GetReady()) {
            m_OHBot->m_DB->RecoverCallable(i->second);
            delete i->second;
            i = m_BotStatusCreate.erase(i);
        }
        else
            ++i;
    }

    if (GetTime() - m_LastLogUpdateTime >= 1800) {
        m_OHBot->m_Callables.push_back(m_OHBot->m_DB->ThreadedStoreLog(0, string(), m_AdminLog));
        m_AdminLog = vector<string>();
        m_LastLogUpdateTime = GetTime();
    }

    // refresh the permission list every 5 minutes

    if (!m_CallablePList && GetTime() - m_LastAdminRefreshTime >= 300) {
        m_CallablePList = m_OHBot->m_DB->ThreadedPList(m_Server);
        m_LastAdminRefreshTime = GetTime();
    }

    // checking for finished games
    if (GetTime() - m_OHBot->m_CheckForFinishedGames >= 120 && m_OHBot->m_StatsUpdate) {
#ifdef WIN32
        system("stats.exe");
#else
                                                                                                                                uint32_t result = system("./stats");
	if(!result) { CONSOLE_Print("Error. Didn't found stats binary file. Couldn't update stats."); }
#endif
        m_OHBot->m_CheckForFinishedGames = GetTime();
//              m_OHBot->m_FinishedGames--;
    }

    if (m_CallablePList && m_CallablePList->GetReady()) {
        m_OHBot->LoadDatas();
        m_Permissions = m_CallablePList->GetResult();
        m_OHBot->m_DB->RecoverCallable(m_CallablePList);
        delete m_CallablePList;
        m_CallablePList = NULL;
        m_LastAdminRefreshTime = GetTime();
    }

    // remove temp bans every 5 min
    // refresh the ban list every 5 minutes

    if (!m_CallableBanList && GetTime() - m_LastBanRefreshTime >= 300) {
        m_CallableBanList = m_OHBot->m_DB->ThreadedBanList(m_Server);
        m_CallableTBRemove = m_OHBot->m_DB->ThreadedTBRemove(m_Server);
    }

    if (m_CallableBanList && m_CallableBanList->GetReady() && m_CallableTBRemove && m_CallableTBRemove->GetReady()) {
        DEBUG_Print("[BNET: " + m_ServerAlias + "] refreshed ban list (" + UTIL_ToString(m_Bans.size()) + " -> " +
                    UTIL_ToString(m_CallableBanList->GetResult().size()) + " bans)");

        for (vector<CDBBan *>::iterator i = m_Bans.begin(); i != m_Bans.end(); ++i)
            delete *i;

        m_Bans = m_CallableBanList->GetResult();
        m_OHBot->m_DB->RecoverCallable(m_CallableBanList);
        delete m_CallableBanList;
        m_CallableBanList = NULL;
        m_OHBot->m_DB->RecoverCallable(m_CallableTBRemove);
        delete m_CallableTBRemove;
        m_CallableTBRemove = NULL;
        m_LastBanRefreshTime = GetTime();
    }

    // we return at the end of each if statement so we don't have to deal with errors related to the order of the if statements
    // that means it might take a few ms longer to complete a task involving multiple steps (in this case, reconnecting) due to blocking or sleeping
    // but it's not a big deal at all, maybe 100ms in the worst possible case (based on a 50ms blocking time)

    if (m_Socket->HasError() && !m_FakeRealm) {
        // the socket has an error

        CONSOLE_Print("[BNET: " + m_ServerAlias + "] disconnected from battle.net due to socket error");

        if (m_Socket->GetError() == ECONNRESET && GetTime() - m_LastConnectionAttemptTime <= 15) {
            CONSOLE_Print("[BNET: " + m_ServerAlias +
                          "] warning - you are probably using an IP temporarily banned from battle.net");
        }

        CONSOLE_Print("[BNET: " + m_ServerAlias + "] waiting 90 seconds to reconnect");
        m_OHBot->EventBNETDisconnected(this);
        delete m_BNLSClient;
        m_BNLSClient = NULL;
        m_BNCSUtil->Reset(m_UserName, m_UserPassword);
        m_Socket->Reset();
        m_LastDisconnectedTime = GetTime();
        m_LoggedIn = false;
        m_InChat = false;
        m_WaitingToConnect = true;
        return m_Exiting;
    }

    if (!m_Socket->GetConnecting() && !m_Socket->GetConnected() && !m_WaitingToConnect && !m_FakeRealm) {
        // the socket was disconnected

        CONSOLE_Print("[BNET: " + m_ServerAlias + "] disconnected from battle.net");
        CONSOLE_Print("[BNET: " + m_ServerAlias + "] waiting 90 seconds to reconnect");
        m_OHBot->EventBNETDisconnected(this);
        delete m_BNLSClient;
        m_BNLSClient = NULL;
        m_BNCSUtil->Reset(m_UserName, m_UserPassword);
        m_Socket->Reset();
        m_LastDisconnectedTime = GetTime();
        m_LoggedIn = false;
        m_InChat = false;
        m_WaitingToConnect = true;
        return m_Exiting;
    }

    if (m_Socket->GetConnected()) {
        // the socket is connected and everything appears to be working properly

        if (GetTime() - LastUpdateTime >= 10) {
            LastUpdateTime = GetTime();
        }

        m_Socket->DoRecv((fd_set *) fd);
        ExtractPackets();
        ProcessPackets();

        // update the BNLS client

        if (m_BNLSClient) {
            if (m_BNLSClient->Update(fd, send_fd)) {
                CONSOLE_Print("[BNET: " + m_ServerAlias + "] deleting BNLS client");
                delete m_BNLSClient;
                m_BNLSClient = NULL;
            }
            else {
                BYTEARRAY WardenResponse = m_BNLSClient->GetWardenResponse();

                if (!WardenResponse.empty())
                    m_Socket->PutBytes(m_Protocol->SEND_SID_WARDEN(WardenResponse));
            }
        }

        // check if at least one packet is waiting to be sent and if we've waited long enough to prevent flooding
        // this formula has changed many times but currently we wait 1 second if the last packet was "small", 3.5 seconds if it was "medium", and 4 seconds if it was "big"

        uint32_t WaitTicks = 0;

        if (m_LastOutPacketSize < 10)
            WaitTicks = 1300;
        else if (m_LastOutPacketSize < 30)
            WaitTicks = 3400;
        else if (m_LastOutPacketSize < 50)
            WaitTicks = 3600;
        else if (m_LastOutPacketSize < 100)
            WaitTicks = 3900;
        else
            WaitTicks = 5500;

        // add on frequency delay

        WaitTicks += m_FrequencyDelayTimes * 60;

        if (!m_OutPackets.empty() && GetTicks() - m_LastOutPacketTicks >= WaitTicks) {
            if (m_OutPackets.size() > 7)
                CONSOLE_Print("[BNET: " + m_ServerAlias + "] packet queue warning - there are " +
                              UTIL_ToString(m_OutPackets.size()) + " packets waiting to be sent");

            m_Socket->PutBytes(m_OutPackets.front());
            m_LastOutPacketSize = m_OutPackets.front().size();
            m_OutPackets.pop();

            // reset frequency delay (or increment it)

            if (m_FrequencyDelayTimes >= 100 || GetTicks() > m_LastOutPacketTicks + WaitTicks + 500)
                m_FrequencyDelayTimes = 0;
            else
                m_FrequencyDelayTimes++;

            m_LastOutPacketTicks = GetTicks();
        }

        // send a null packet every 60 seconds to detect disconnects

        if (GetTime() - m_LastNullTime >= 60 && GetTicks() - m_LastOutPacketTicks >= 60000) {
            m_Socket->PutBytes(m_Protocol->SEND_SID_NULL());
            m_LastNullTime = GetTime();
        }

        m_Socket->DoSend((fd_set *) send_fd);
        return m_Exiting;
    }

    if (m_Socket->GetConnecting() && !m_FakeRealm) {

        if (!m_OHBot->isCreated) {
            m_BotStatusCreate.push_back(BotStatusCreate(string(), m_OHBot->m_DB->ThreadedBotStatusCreate(m_UserName,
                                                                                                         m_OHBot->m_AutoHostGameName,
                                                                                                         m_OHBot->m_BindAddress,
                                                                                                         m_OHBot->m_HostPort,
                                                                                                         m_CDKeyROC,
                                                                                                         m_CDKeyTFT)));
            m_OHBot->isCreated = true;
        }
        // we are currently attempting to connect to battle.net

        if (m_Socket->CheckConnect()) {
            // the connection attempt completed

            CONSOLE_Print("[BNET: " + m_ServerAlias + "] connected");
            m_OHBot->EventBNETConnected(this);
            m_Socket->PutBytes(m_Protocol->SEND_PROTOCOL_INITIALIZE_SELECTOR());
            m_Socket->PutBytes(
                    m_Protocol->SEND_SID_AUTH_INFO(m_War3Version, m_OHBot->m_TFT, m_LocaleID, m_CountryAbbrev,
                                                   m_Country));
            m_Socket->DoSend((fd_set *) send_fd);
            m_LastNullTime = GetTime();
            m_LastOutPacketTicks = GetTicks();

            while (!m_OutPackets.empty())
                m_OutPackets.pop();

            return m_Exiting;
        }
        else if (GetTime() - m_LastConnectionAttemptTime >= 15) {
            // the connection attempt timed out (15 seconds)

            CONSOLE_Print("[BNET: " + m_ServerAlias + "] connect timed out");
            CONSOLE_Print("[BNET: " + m_ServerAlias + "] waiting 90 seconds to reconnect");

            m_OHBot->EventBNETConnectTimedOut(this);
            m_Socket->Reset();
            m_LastDisconnectedTime = GetTime();
            m_WaitingToConnect = true;
            return m_Exiting;
        }
    }

    if (!m_Socket->GetConnecting() && !m_Socket->GetConnected() &&
        (m_FirstConnect || GetTime() - m_LastDisconnectedTime >= 90) && !m_FakeRealm) {
        // attempt to connect to battle.net

        m_FirstConnect = false;
        CONSOLE_Print("[BNET: " + m_ServerAlias + "] connecting to server [" + m_Server + "] on port 6112");
        m_OHBot->EventBNETConnecting(this);

        if (!m_OHBot->m_BindAddress.empty())
            CONSOLE_Print(
                    "[BNET: " + m_ServerAlias + "] attempting to bind to address [" + m_OHBot->m_BindAddress + "]");

        if (m_ServerIP.empty()) {
            m_Socket->Connect(m_OHBot->m_BindAddress, m_Server, 6112);

            if (!m_Socket->HasError()) {
                m_ServerIP = m_Socket->GetIPString();
                CONSOLE_Print("[BNET: " + m_ServerAlias + "] resolved and cached server IP address " + m_ServerIP);
            }
        }
        else {
            // use cached server IP address since resolving takes time and is blocking

            CONSOLE_Print("[BNET: " + m_ServerAlias + "] using cached server IP address " + m_ServerIP);
            m_Socket->Connect(m_OHBot->m_BindAddress, m_ServerIP, 6112);
        }

        m_WaitingToConnect = false;
        m_LastConnectionAttemptTime = GetTime();
        return m_Exiting;
    }

    return m_Exiting;
}

void CBNET::ExtractPackets() {
    // extract as many packets as possible from the socket's receive buffer and put them in the m_Packets queue

    string *RecvBuffer = m_Socket->GetBytes();
    BYTEARRAY Bytes = UTIL_CreateByteArray((unsigned char *) RecvBuffer->c_str(), RecvBuffer->size());

    // a packet is at least 4 bytes so loop as long as the buffer contains 4 bytes

    while (Bytes.size() >= 4) {
        // byte 0 is always 255

        if (Bytes[0] == BNET_HEADER_CONSTANT) {
            // bytes 2 and 3 contain the length of the packet

            uint16_t Length = UTIL_ByteArrayToUInt16(Bytes, false, 2);

            if (Length >= 4) {
                if (Bytes.size() >= Length) {
                    m_Packets.push(new CCommandPacket(BNET_HEADER_CONSTANT, Bytes[1],
                                                      BYTEARRAY(Bytes.begin(), Bytes.begin() + Length)));
                    *RecvBuffer = RecvBuffer->substr(Length);
                    Bytes = BYTEARRAY(Bytes.begin() + Length, Bytes.end());
                }
                else
                    return;
            }
            else {
                CONSOLE_Print("[BNET: " + m_ServerAlias +
                              "] error - received invalid packet from battle.net (bad length), disconnecting");
                m_Socket->Disconnect();
                return;
            }
        }
        else {
            CONSOLE_Print("[BNET: " + m_ServerAlias +
                          "] error - received invalid packet from battle.net (bad header constant), disconnecting");
            m_Socket->Disconnect();
            return;
        }
    }
}

void CBNET::ProcessPackets() {
    CIncomingGameHost *GameHost = NULL;
    CIncomingChatEvent *ChatEvent = NULL;
    BYTEARRAY WardenData;
    vector<CIncomingFriendList *> Friends;

    // process all the received packets in the m_Packets queue
    // this normally means sending some kind of response

    while (!m_Packets.empty()) {
        CCommandPacket *Packet = m_Packets.front();
        m_Packets.pop();

        if (Packet->GetPacketType() == BNET_HEADER_CONSTANT) {
            switch (Packet->GetID()) {
                case CBNETProtocol::SID_NULL:
                    // warning: we do not respond to NULL packets with a NULL packet of our own
                    // this is because PVPGN servers are programmed to respond to NULL packets so it will create a vicious cycle of useless traffic
                    // official battle.net servers do not respond to NULL packets

                    m_Protocol->RECEIVE_SID_NULL(Packet->GetData());
                    break;

                case CBNETProtocol::SID_GETADVLISTEX:
                    GameHost = m_Protocol->RECEIVE_SID_GETADVLISTEX(Packet->GetData());

                    if (GameHost)
                        CONSOLE_Print("[BNET: " + m_ServerAlias + "] joining game [" + GameHost->GetGameName() + "]");

                    delete GameHost;
                    GameHost = NULL;
                    break;

                case CBNETProtocol::SID_ENTERCHAT:
                    if (m_Protocol->RECEIVE_SID_ENTERCHAT(Packet->GetData())) {
                        CONSOLE_Print("[BNET: " + m_ServerAlias + "] joining channel [" + m_FirstChannel + "]");
                        m_InChat = true;
                        m_Socket->PutBytes(m_Protocol->SEND_SID_JOINCHANNEL(m_FirstChannel));
                    }

                    break;

                case CBNETProtocol::SID_CHATEVENT:
                    ChatEvent = m_Protocol->RECEIVE_SID_CHATEVENT(Packet->GetData());

                    if (ChatEvent)
                        ProcessChatEvent(ChatEvent);

                    delete ChatEvent;
                    ChatEvent = NULL;
                    break;

                case CBNETProtocol::SID_CHECKAD:
                    m_Protocol->RECEIVE_SID_CHECKAD(Packet->GetData());
                    break;

                case CBNETProtocol::SID_STARTADVEX3:
                    if (m_Protocol->RECEIVE_SID_STARTADVEX3(Packet->GetData())) {
                        m_InChat = false;
                        m_OHBot->EventBNETGameRefreshed(this);
                    }
                    else {
                        CONSOLE_Print("[BNET: " + m_ServerAlias + "] startadvex3 failed");
                        m_OHBot->EventBNETGameRefreshFailed(this);
                    }

                    break;

                case CBNETProtocol::SID_PING:
                    m_Socket->PutBytes(m_Protocol->SEND_SID_PING(m_Protocol->RECEIVE_SID_PING(Packet->GetData())));
                    break;

                case CBNETProtocol::SID_AUTH_INFO:
                    if (m_Protocol->RECEIVE_SID_AUTH_INFO(Packet->GetData())) {
                        if (m_BNCSUtil->HELP_SID_AUTH_CHECK(m_OHBot->m_TFT, m_OHBot->m_Warcraft3Path, m_CDKeyROC,
                                                            m_CDKeyTFT, m_Protocol->GetValueStringFormulaString(),
                                                            m_Protocol->GetIX86VerFileNameString(),
                                                            m_Protocol->GetClientToken(),
                                                            m_Protocol->GetServerToken())) {
                            // override the exe information generated by bncsutil if specified in the config file
                            // apparently this is useful for pvpgn users

                            if (m_EXEVersion.size() == 4) {
                                CONSOLE_Print("[BNET: " + m_ServerAlias +
                                              "] using custom exe version bnet_custom_exeversion = " +
                                              UTIL_ToString(m_EXEVersion[0]) + " " + UTIL_ToString(m_EXEVersion[1]) +
                                              " " + UTIL_ToString(m_EXEVersion[2]) + " " +
                                              UTIL_ToString(m_EXEVersion[3]));
                                m_BNCSUtil->SetEXEVersion(m_EXEVersion);
                            }

                            if (m_EXEVersionHash.size() == 4) {
                                CONSOLE_Print("[BNET: " + m_ServerAlias +
                                              "] using custom exe version hash bnet_custom_exeversionhash = " +
                                              UTIL_ToString(m_EXEVersionHash[0]) + " " +
                                              UTIL_ToString(m_EXEVersionHash[1]) + " " +
                                              UTIL_ToString(m_EXEVersionHash[2]) + " " +
                                              UTIL_ToString(m_EXEVersionHash[3]));
                                m_BNCSUtil->SetEXEVersionHash(m_EXEVersionHash);
                            }

                            if (m_OHBot->m_TFT)
                                CONSOLE_Print("[BNET: " + m_ServerAlias +
                                              "] attempting to auth as Warcraft III: The Frozen Throne");
                            else
                                CONSOLE_Print("[BNET: " + m_ServerAlias +
                                              "] attempting to auth as Warcraft III: Reign of Chaos");

                            m_Socket->PutBytes(
                                    m_Protocol->SEND_SID_AUTH_CHECK(m_OHBot->m_TFT, m_Protocol->GetClientToken(),
                                                                    m_BNCSUtil->GetEXEVersion(),
                                                                    m_BNCSUtil->GetEXEVersionHash(),
                                                                    m_BNCSUtil->GetKeyInfoROC(),
                                                                    m_BNCSUtil->GetKeyInfoTFT(),
                                                                    m_BNCSUtil->GetEXEInfo(), "GHost"));

                            // the Warden seed is the first 4 bytes of the ROC key hash
                            // initialize the Warden handler

                            if (!m_BNLSServer.empty()) {
                                CONSOLE_Print("[BNET: " + m_ServerAlias + "] creating BNLS client");
                                delete m_BNLSClient;
                                m_BNLSClient = new CBNLSClient(m_BNLSServer, m_BNLSPort, m_BNLSWardenCookie);
                                m_BNLSClient->QueueWardenSeed(
                                        UTIL_ByteArrayToUInt32(m_BNCSUtil->GetKeyInfoROC(), false, 16));
                            }
                        }
                        else {
                            CONSOLE_Print("[BNET: " + m_ServerAlias +
                                          "] logon failed - bncsutil key hash failed (check your Warcraft 3 path and cd keys), disconnecting");
                            m_Socket->Disconnect();
                            delete Packet;
                            return;
                        }
                    }

                    break;

                case CBNETProtocol::SID_AUTH_CHECK:
                    if (m_Protocol->RECEIVE_SID_AUTH_CHECK(Packet->GetData())) {
                        // cd keys accepted

                        CONSOLE_Print("[BNET: " + m_ServerAlias + "] cd keys accepted");
                        m_BNCSUtil->HELP_SID_AUTH_ACCOUNTLOGON();
                        m_Socket->PutBytes(
                                m_Protocol->SEND_SID_AUTH_ACCOUNTLOGON(m_BNCSUtil->GetClientKey(), m_UserName));
                    }
                    else {
                        // cd keys not accepted

                        switch (UTIL_ByteArrayToUInt32(m_Protocol->GetKeyState(), false)) {
                            case CBNETProtocol::KR_ROC_KEY_IN_USE:
                                CONSOLE_Print(
                                        "[BNET: " + m_ServerAlias + "] logon failed - ROC CD key in use by user [" +
                                        m_Protocol->GetKeyStateDescription() + "], disconnecting");
                                break;
                            case CBNETProtocol::KR_TFT_KEY_IN_USE:
                                CONSOLE_Print(
                                        "[BNET: " + m_ServerAlias + "] logon failed - TFT CD key in use by user [" +
                                        m_Protocol->GetKeyStateDescription() + "], disconnecting");
                                break;
                            case CBNETProtocol::KR_OLD_GAME_VERSION:
                                CONSOLE_Print("[BNET: " + m_ServerAlias +
                                              "] logon failed - game version is too old, disconnecting");
                                break;
                            case CBNETProtocol::KR_INVALID_VERSION:
                                CONSOLE_Print("[BNET: " + m_ServerAlias +
                                              "] logon failed - game version is invalid, disconnecting");
                                break;
                            default:
                                CONSOLE_Print("[BNET: " + m_ServerAlias +
                                              "] logon failed - cd keys not accepted, disconnecting");
                                break;
                        }

                        m_Socket->Disconnect();
                        delete Packet;
                        return;
                    }

                    break;

                case CBNETProtocol::SID_AUTH_ACCOUNTLOGON:
                    if (m_Protocol->RECEIVE_SID_AUTH_ACCOUNTLOGON(Packet->GetData())) {
                        CONSOLE_Print("[BNET: " + m_ServerAlias + "] username [" + m_UserName + "] accepted");

                        if (m_PasswordHashType == "pvpgn") {
                            // pvpgn logon

                            CONSOLE_Print(
                                    "[BNET: " + m_ServerAlias + "] using pvpgn logon type (for pvpgn servers only)");
                            m_BNCSUtil->HELP_PvPGNPasswordHash(m_UserPassword);
                            m_Socket->PutBytes(
                                    m_Protocol->SEND_SID_AUTH_ACCOUNTLOGONPROOF(m_BNCSUtil->GetPvPGNPasswordHash()));
                        }
                        else {
                            // battle.net logon

                            CONSOLE_Print("[BNET: " + m_ServerAlias +
                                          "] using battle.net logon type (for official battle.net servers only)");
                            m_BNCSUtil->HELP_SID_AUTH_ACCOUNTLOGONPROOF(m_Protocol->GetSalt(),
                                                                        m_Protocol->GetServerPublicKey());
                            m_Socket->PutBytes(m_Protocol->SEND_SID_AUTH_ACCOUNTLOGONPROOF(m_BNCSUtil->GetM1()));
                        }
                    }
                    else {
                        CONSOLE_Print("[BNET: " + m_ServerAlias + "] logon failed - invalid username, disconnecting");
                        m_Socket->Disconnect();
                        delete Packet;
                        return;
                    }

                    break;

                case CBNETProtocol::SID_AUTH_ACCOUNTLOGONPROOF:
                    if (m_Protocol->RECEIVE_SID_AUTH_ACCOUNTLOGONPROOF(Packet->GetData())) {
                        // logon successful

                        CONSOLE_Print("[BNET: " + m_ServerAlias + "] logon successful");

                        m_LoggedIn = true;
                        m_OHBot->EventBNETLoggedIn(this);
                        m_Socket->PutBytes(m_Protocol->SEND_SID_NETGAMEPORT(m_OHBot->m_HostPort));
                        m_Socket->PutBytes(m_Protocol->SEND_SID_ENTERCHAT());
                        m_Socket->PutBytes(m_Protocol->SEND_SID_FRIENDSLIST());
                        m_Socket->PutBytes(m_Protocol->SEND_SID_CLANMEMBERLIST());
                    }
                    else {
                        CONSOLE_Print("[BNET: " + m_ServerAlias + "] logon failed - invalid password, disconnecting");

                        // try to figure out if the user might be using the wrong logon type since too many people are confused by this

                        string Server = m_Server;
                        transform(Server.begin(), Server.end(), Server.begin(), ::tolower);

                        if (m_PasswordHashType == "pvpgn" &&
                            (Server == "useast.battle.net" || Server == "uswest.battle.net" ||
                             Server == "asia.battle.net" || Server == "europe.battle.net"))
                            CONSOLE_Print("[BNET: " + m_ServerAlias +
                                          "] it looks like you're trying to connect to a battle.net server using a pvpgn logon type, check your config file's \"battle.net custom data\" section");
                        else if (m_PasswordHashType != "pvpgn" &&
                                 (Server != "useast.battle.net" && Server != "uswest.battle.net" &&
                                  Server != "asia.battle.net" && Server != "europe.battle.net"))
                            CONSOLE_Print("[BNET: " + m_ServerAlias +
                                          "] it looks like you're trying to connect to a pvpgn server using a battle.net logon type, check your config file's \"battle.net custom data\" section");

                        m_Socket->Disconnect();
                        delete Packet;
                        return;
                    }

                    break;

                case CBNETProtocol::SID_WARDEN:
                    WardenData = m_Protocol->RECEIVE_SID_WARDEN(Packet->GetData());

                    if (m_BNLSClient)
                        m_BNLSClient->QueueWardenRaw(WardenData);
                    else
                        CONSOLE_Print("[BNET: " + m_ServerAlias +
                                      "] warning - received warden packet but no BNLS server is available, you will be kicked from battle.net soon");

                    break;

                case CBNETProtocol::SID_FRIENDSLIST:
                    Friends = m_Protocol->RECEIVE_SID_FRIENDSLIST(Packet->GetData());

                    for (vector<CIncomingFriendList *>::iterator i = m_Friends.begin(); i != m_Friends.end(); ++i)
                        delete *i;

                    m_Friends = Friends;
                    break;

                case CBNETProtocol::SID_CLANMEMBERLIST:
                    vector<CIncomingClanList *> Clans = m_Protocol->RECEIVE_SID_CLANMEMBERLIST(Packet->GetData());

                    for (vector<CIncomingClanList *>::iterator i = m_Clans.begin(); i != m_Clans.end(); ++i)
                        delete *i;

                    m_Clans = Clans;
                    break;
            }
        }

        delete Packet;
    }
}

void CBNET::ProcessChatEvent(CIncomingChatEvent *chatEvent) {
    CBNETProtocol::IncomingChatEvent Event = chatEvent->GetChatEvent();
    bool Whisper = (Event == CBNETProtocol::EID_WHISPER);
    string User = chatEvent->GetUser();
    string Message = chatEvent->GetMessage();

    if (Event == CBNETProtocol::EID_WHISPER || Event == CBNETProtocol::EID_TALK) {
        if (Event == CBNETProtocol::EID_WHISPER) {
            CONSOLE_Print("[WHISPER: " + m_ServerAlias + "] [" + User + "] " + Message);
            m_OHBot->EventBNETWhisper(this, User, Message);
        }
        else {
            CONSOLE_Print("[LOCAL: " + m_ServerAlias + "] [" + User + "] " + Message);
            m_OHBot->EventBNETChat(this, User, Message);
        }

        // handle spoof checking for current game
        // this case covers whispers - we assume that anyone who sends a whisper to the bot with message "spoofcheck" should be considered spoof checked
        // note that this means you can whisper "spoofcheck" even in a public game to manually spoofcheck if the /whois fails

        boost::mutex::scoped_lock lock(m_OHBot->m_GamesMutex);

        if (Event == CBNETProtocol::EID_WHISPER && m_OHBot->m_CurrentGame) {
            if (Message == "s" || Message == "sc" || Message == "spoof" || Message == "check" ||
                Message == "spoofcheck")
                m_OHBot->m_CurrentGame->AddToSpoofed(m_Server, User, true);
            else if (Message.find(m_OHBot->m_CurrentGame->GetGameName()) != string::npos) {
                // look for messages like "entered a Warcraft III The Frozen Throne game called XYZ"
                // we don't look for the English part of the text anymore because we want this to work with multiple languages
                // it's a pretty safe bet that anyone whispering the bot with a message containing the game name is a valid spoofcheck

                if (m_PasswordHashType == "pvpgn" && User == m_PVPGNRealmName) {
                    // the equivalent pvpgn message is: [PvPGN Realm] Your friend abc has entered a Warcraft III Frozen Throne game named "xyz".

                    vector<string> Tokens = UTIL_Tokenize(Message, ' ');

                    if (Tokens.size() >= 3)
                        m_OHBot->m_CurrentGame->AddToSpoofed(m_Server, Tokens[2], false);
                }
                else
                    m_OHBot->m_CurrentGame->AddToSpoofed(m_Server, User, false);
            }
        }

        lock.unlock();

        // handle bot commands

        if (Message == "?trigger" && (IsLevel(User) >= 9 || (m_PublicCommands && m_OutPackets.size() <= 3)))
            QueueChatCommand(m_OHBot->m_Language->CommandTrigger(string(1, m_CommandTrigger)), User, Whisper);
        else if (!Message.empty() && Message[0] == m_CommandTrigger) {
            BotCommand(Message, User, Whisper, false);
        }
        else if (!Message.empty() && Message.substr(0, 1) == "/" && Message.find("pvpgn") != string::npos &&
                 m_OHBot->m_PVPGNMode) {
            PVPGNCommand(Message);
        }
    }
    else if (Event == CBNETProtocol::EID_CHANNEL) {
        // keep track of current channel so we can rejoin it after hosting a game
        CONSOLE_Print("[BNET: " + m_ServerAlias + "] joined channel [" + Message + "]");
        m_CurrentChannel = Message;
    }
    else if (Event == CBNETProtocol::EID_INFO) {
        CONSOLE_Print("[INFO: " + m_ServerAlias + "] " + Message);

        // extract the first word which we hope is the username
        // this is not necessarily true though since info messages also include channel MOTD's and such

        string UserName;
        string::size_type Split = Message.find(" ");

        if (Split != string::npos)
            UserName = Message.substr(0, Split);
        else
            UserName = Message.substr(0);

        // handle spoof checking for current game
        // this case covers whois results which are used when hosting a public game (we send out a "/whois [player]" for each player)
        // at all times you can still /w the bot with "spoofcheck" to manually spoof check

        boost::mutex::scoped_lock lock(m_OHBot->m_GamesMutex);

        if (m_OHBot->m_CurrentGame && m_OHBot->m_CurrentGame->GetPlayerFromName(UserName, true)) {
            if (Message.find("is away") != string::npos)
                m_OHBot->m_CurrentGame->SendAllChat(m_OHBot->m_Language->SpoofPossibleIsAway(UserName));
            else if (Message.find("is unavailable") != string::npos)
                m_OHBot->m_CurrentGame->SendAllChat(m_OHBot->m_Language->SpoofPossibleIsUnavailable(UserName));
            else if (Message.find("is refusing messages") != string::npos)
                m_OHBot->m_CurrentGame->SendAllChat(m_OHBot->m_Language->SpoofPossibleIsRefusingMessages(UserName));
            else if (Message.find("is using Warcraft III The Frozen Throne in the channel") != string::npos)
                m_OHBot->m_CurrentGame->SendAllChat(m_OHBot->m_Language->SpoofDetectedIsNotInGame(UserName));
            else if (Message.find("is using Warcraft III The Frozen Throne in channel") != string::npos)
                m_OHBot->m_CurrentGame->SendAllChat(m_OHBot->m_Language->SpoofDetectedIsNotInGame(UserName));
            else if (Message.find("is using Warcraft III The Frozen Throne in a private channel") != string::npos)
                m_OHBot->m_CurrentGame->SendAllChat(m_OHBot->m_Language->SpoofDetectedIsInPrivateChannel(UserName));

            string LMessage = Message;
            transform(LMessage.begin(), LMessage.end(), LMessage.begin(), ::tolower);

            if (Message.find("is using Warcraft III The Frozen Throne in game") != string::npos ||
                Message.find("is using Warcraft III Frozen Throne and is currently in  game") != string::npos) {

                // check both the current game name and the last game name against the /whois response
                // this is because when the game is rehosted, players who joined recently will be in the previous game according to battle.net
                // note: if the game is rehosted more than once it is possible (but unlikely) for a false positive because only two game names are checked

                if (Message.find(m_OHBot->m_CurrentGame->GetGameName()) != string::npos ||
                    Message.find(m_OHBot->m_CurrentGame->GetLastGameName()) != string::npos ||
                    LMessage.find(m_OHBot->m_SpoofPattern) != string::npos)
                    m_OHBot->m_CurrentGame->AddToSpoofed(m_Server, UserName, false);
                else
                    m_OHBot->m_CurrentGame->SendAllChat(m_OHBot->m_Language->SpoofDetectedIsInAnotherGame(UserName));
            }
        }

        lock.unlock();
    }

    else if (Event == CBNETProtocol::EID_JOIN) {
        if (m_OHBot->m_MessageSystem)
            m_Pairedpms.push_back(Pairedpm(User, m_OHBot->m_DB->Threadedpm(User, string(), 0, string(), "join")));
        CONSOLE_Print("[BNET: " + m_ServerAlias + "] user [" + User + "] joined channel " + m_CurrentChannel);
    }
    else if (Event == CBNETProtocol::EID_ERROR)
        CONSOLE_Print("[ERROR: " + m_ServerAlias + "] " + Message);
    else if (Event == CBNETProtocol::EID_EMOTE) {
        CONSOLE_Print("[EMOTE: " + m_ServerAlias + "] [" + User + "] " + Message);
        m_OHBot->EventBNETEmote(this, User, Message);
    }
}

void CBNET::BotCommand(string Message, string User, bool Whisper, bool ForceRoot) {

    // extract the command trigger, the command, and the payload
    // e.g. "!say hello world" -> command: "say", payload: "hello world"

    bool cmdSuccess = false;
    string Command;
    string Payload;
    string::size_type PayloadStart = Message.find(" ");

    if (PayloadStart != string::npos) {
        Command = Message.substr(1, PayloadStart - 1);
        Payload = Message.substr(PayloadStart + 1);
    }
    else
        Command = Message.substr(1);

    transform(Command.begin(), Command.end(), Command.begin(), ::tolower);

    // Test if Bot is in PVPGN Mode (^= accept commands only via pvpgn)
    if (!ForceRoot && m_OHBot->m_PVPGNMode && Command != "version") {
        CONSOLE_Print("[PVPGN-Mode] User used command, but bot is running in pvpgn-mode.");
        return;
    }

    if ((IsLevel(User) >= 5 || ForceRoot) && m_OHBot->m_RanksLoaded) {
        string level = GetLevelName(IsLevel(User));
        CONSOLE_Print(
                "[BNET] " + level + " [" + User + "] sent command [" + Command + "] with payload [" + Payload + "]");

        //save admin log
        m_AdminLog.push_back(User + " cl\t" + UTIL_ToString(IsLevel(User)) + "\t" + Command + "\t" + Payload);

        //
        // !VOUCH
        //
        if (Command == "vouch" && IsLevel(User) >= 6 && !Payload.empty()) {
            string Name;
            string Realm;
            stringstream SS;
            SS << Payload;
            SS >> Name;
            if (Name.length() <= 3) {
                QueueChatCommand(m_OHBot->m_Language->InvalidName());
                return;
            }
            SS >> Realm;
            if (SS.fail() || Realm.empty())
                Realm = m_Server;

            m_PairedPUps.push_back(
                    PairedPUp(Whisper ? User : string(), m_OHBot->m_DB->ThreadedPUp(Name, 1, Realm, User)));
        }

        //
        // !SETPERMISSION
        //
        if ((Command == "setp" || Command == "sep" || Command == "setpermission") && IsLevel(User) >= 9) {
            string Name;
            string NewLevel;
            string Realm = "";
            stringstream SS;
            SS << Payload;
            SS >> Name;

            if (Name.length() <= 3) {
                QueueChatCommand(m_OHBot->m_Language->InvalidName());
                return;
            }

            SS >> NewLevel;

            if (SS.fail() || NewLevel.empty()) {
                QueueChatCommand(m_OHBot->m_Language->WrongInputUseALevel());
                return;
            }
            else {
                if (!NewLevel.find_first_not_of("1234567890") == string::npos) {
                    QueueChatCommand(m_OHBot->m_Language->WrongInputInvalidLevel());
                    return;
                }
                SS >> Realm;
                if (SS.fail() || Realm.empty())
                    Realm = m_Server;

                m_PairedPUps.push_back(PairedPUp(Whisper ? User : string(),
                                                 m_OHBot->m_DB->ThreadedPUp(Name, UTIL_ToUInt32(NewLevel), Realm,
                                                                            User)));
            }
        }

            //
            // !UPDATE ALL STATS
            //
        else if (Command == "update" && IsLevel(User) == 10) {
#ifdef WIN32
            system("stats.exe");
#else
                                                                                                                                    uint32_t result = system("./stats");
	if(!result) { CONSOLE_Print("Error. Didn't found stats binary file. Couldn't update stats."); }
#endif
        }

            //
            // !PERMISSION
            //
        else if ((Command == "perm" || Command == "permission") && IsLevel(User) >= 9) {
            string StatsUser = User;
            if (!Payload.empty())
                StatsUser = Payload;

            QueueChatCommand(m_OHBot->m_Language->UserLevelCheck(StatsUser, UTIL_ToString(IsLevel(StatsUser))));
        }

            //
            // !CheckPP
            //
        else if (Command == "pp" || Command == "checkpp") {
            string StatsUser = User;
            if (!Payload.empty())
                StatsUser = Payload;

            m_Pairedpenps.push_back(Pairedpenp(string(), m_OHBot->m_DB->Threadedpenp(StatsUser, "", "", 0, "check")));
        }

            //
            // !PPADD !PUNISH
            //
        else if ((Command == "ppadd" || Command == "punish") && m_OHBot->m_ChannelBotOnly) {
            string Victim;
            string Amount;
            string Reason;
            stringstream SS;
            SS << Payload;
            SS >> Victim;

            if (SS.fail() || Victim.empty())
                CONSOLE_Print("[PP] bad input #1 to the !TEMPBAN command");
            else if (Victim.size() < 3)
                QueueChatCommand(m_OHBot->m_Language->InvalidNameTooShort());
            else if (IsLevel(User) != 10 && ((IsLevel(User) == 9 && IsLevel(Victim) >= 6) ||
                                             ((IsLevel(User) == 5 || IsLevel(User) == 6 || IsLevel(User) == 7) &&
                                              IsLevel(Victim) >= 2)))
                QueueChatCommand(m_OHBot->m_Language->NoPermissionToExecCommand(), User, Whisper);
            else {
                SS >> Amount;

                if (SS.fail() || Amount == "0")
                    CONSOLE_Print("[PP] bad input #2 to !TEMPBAN command");
                else if ((UTIL_ToUInt32(Amount) > 3 && IsLevel(User) < 8) ||
                         UTIL_ToUInt32(Amount) > 10 && IsLevel(User) <= 10)
                    QueueChatCommand(m_OHBot->m_Language->TooMuchPPoints());
                else {
                    SS >> Reason;

                    if (!SS.eof()) {
                        getline(SS, Reason);
                        string::size_type Start = Reason.find_first_not_of(" ");

                        if (Start != string::npos)
                            Reason = Reason.substr(Start);
                    }
                    if (!Reason.empty()) {
                        m_Pairedpenps.push_back(Pairedpenp(string(), m_OHBot->m_DB->Threadedpenp(Victim, Reason, User,
                                                                                                 UTIL_ToUInt32(Amount),
                                                                                                 "add")));
                    }
                    else
                        QueueChatCommand(m_OHBot->m_Language->ErrorMissingReason());
                }
            }
        }

            //
            // !RPP      !REMOVEPP
            //
        else if ((Command == "rpp" || Command == "removepp") && IsLevel(User) >= 9 && m_OHBot->m_ChannelBotOnly) {
            string SUser;
            string Amount;
            string Reason;
            stringstream SS;
            SS << Payload;
            SS >> SUser;
            if (SS.fail() || SUser.empty())
                QueueChatCommand(m_OHBot->m_Language->ErrorBadInput(), User, Whisper);
            else {
                SS >> Amount;
                if (SS.fail() || Amount.empty())
                    QueueChatCommand(m_OHBot->m_Language->ErrorBadInput(), User, Whisper);
                else {
                    SS >> Reason;

                    if (!SS.eof()) {
                        getline(SS, Reason);
                        string::size_type Start = Reason.find_first_not_of(" ");

                        if (Start != string::npos)
                            Reason = Reason.substr(Start);
                    }
                    if (!Reason.empty())
                        m_PairedSSs.push_back(PairedSS(Whisper ? User : string(),
                                                       m_OHBot->m_DB->ThreadedStatsSystem(SUser, Reason,
                                                                                          UTIL_ToUInt32(Amount),
                                                                                          "rpp")));
                    else
                        m_PairedSSs.push_back(PairedSS(Whisper ? User : string(),
                                                       m_OHBot->m_DB->ThreadedStatsSystem(SUser, "",
                                                                                          UTIL_ToUInt32(Amount),
                                                                                          "rpp")));
                }
            }
        }

            //
            // !FORCEGPROXY
            //
        else if (Command == "forcegproxy") {
            if (Payload.empty()) {
                QueueChatCommand(m_OHBot->m_Language->NoUserDefined(), User, Whisper);
            } else {
                m_PairedSSs.push_back(PairedSS(Whisper ? User : string(),
                                               m_OHBot->m_DB->ThreadedStatsSystem(Payload, User, 0, "forcegproxy")));
            }
        }




        /*****************
        * ADMIN COMMANDS *
        *****************/

            //
            // !ADDBAN
            // !BAN
            //

        else if ((Command == "addban" || Command == "ban") && !Payload.empty() && m_OHBot->m_ChannelBotOnly) {
            if (IsLevel(User) >= 7) {
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
                if (!Reason.empty()) {
                    //if( IsBannedName( Victim ) )
                    //      QueueChatCommand( m_OHBot->m_Language->UserIsAlreadyBanned( m_Server, Victim ), User, Whisper );
                    //else
                    m_PairedBanAdds.push_back(PairedBanAdd(Whisper ? User : string(),
                                                           m_OHBot->m_DB->ThreadedBanAdd(m_Server, Victim, string(),
                                                                                         string(), User, Reason, 0,
                                                                                         "")));
                }
                else
                    QueueChatCommand(m_OHBot->m_Language->ErrorMissingReason(), User, Whisper);
            }
            else
                QueueChatCommand(m_OHBot->m_Language->YouDontHaveAccessToThatCommand(), User, Whisper);
        }

            //
            // !IRB
            // !IPRANGEADD
            // !IPRANGEBAN
            // PLEASE USE THIS BAN WITH CAUTION, IF YOU ARE NOT SURE WITH SUBNET's OR SIMILAIR THINGS, DON'T USE IT!
            //
        else if ((Command == "irb" || Command == "iprangeadd" || Command == "iprangeban") && !Payload.empty() &&
                 IsLevel(User) >= 9 && m_OHBot->m_ChannelBotOnly) {
            string VictimIP;
            string Reason;
            stringstream SS;
            SS << Payload;
            SS >> VictimIP;

            if (!SS.eof()) {
                getline(SS, Reason);
                string::size_type Start = Reason.find_first_not_of(" ");

                if (Start != string::npos)
                    Reason = Reason.substr(Start);
            }

            if (IsBannedName(VictimIP))
                QueueChatCommand(m_OHBot->m_Language->UserIsAlreadyBanned(m_Server, VictimIP), User, Whisper);
            else {
                m_PairedBanAdds.push_back(PairedBanAdd(Whisper ? User : string(),
                                                       m_OHBot->m_DB->ThreadedBanAdd(m_Server, "IPRange",
                                                                                     ":" + VictimIP, string(), User,
                                                                                     Reason, 0, "")));
                QueueChatCommand(m_OHBot->m_Language->BannedIPRange(VictimIP, m_Server), User, Whisper);
            }
        }

            //
            // !TEMPBAN
            // !TBAN
            //
        else if ((Command == "tempban" || Command == "tban") && !Payload.empty() && m_OHBot->m_ChannelBotOnly) {
            // extract the victim and the reason
            // e.g. "Varlock leaver after dying" -> victim: "Varlock", reason: "leaver after dying"

            string Victim;
            string Reason;

            uint32_t Amount;
            uint32_t BanTime;
            string Suffix;

            stringstream SS;
            SS << Payload;
            SS >> Victim;

            if (SS.fail() || Victim.empty())
                CONSOLE_Print("[TEMPBAN] bad input #1 to the !TEMPBAN command");
            else if (Victim.size() < 3)
                QueueChatCommand(m_OHBot->m_Language->InvalidNameTooShort());
            else if (IsLevel(User) != 10 && ((IsLevel(User) == 9 && IsLevel(Victim) >= 6) ||
                                             ((IsLevel(User) == 5 || IsLevel(User) == 6 || IsLevel(User) == 7) &&
                                              IsLevel(Victim) >= 2)))
                QueueChatCommand(m_OHBot->m_Language->NoPermissionToExecCommand(), User, Whisper);
            else {
                SS >> Amount;

                if (SS.fail() || Amount == 0)
                    CONSOLE_Print("[TEMPBAN] bad input #2 to !TEMPBAN command");
                else {
                    SS >> Suffix;

                    if (SS.fail() || Suffix.empty())
                        CONSOLE_Print("[TEMPBAN] bad input #3 to the autohost command");
                    else {
                        transform(Suffix.begin(), Suffix.end(), Suffix.begin(), ::tolower);

                        // handle suffix
                        // valid suffix is: hour, h, week, w, day, d, month, m

                        bool ValidSuffix = false;
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
                            if ((IsLevel(User) == 5 || IsLevel(User) == 6) &&
                                ((Suffix == "hour" || Suffix == "hours" || Suffix == "h") ||
                                 ((Suffix == "days" || Suffix == "d" || Suffix == "day") && Amount <= 5)) ||
                                IsLevel(User) >= 7) {
                                if (!SS.eof()) {
                                    getline(SS, Reason);
                                    string::size_type Start = Reason.find_first_not_of(" ");

                                    if (Start != string::npos)
                                        Reason = Reason.substr(Start);
                                }

                                if (IsBannedName(Victim))
                                    QueueChatCommand(m_OHBot->m_Language->UserIsAlreadyBanned(m_Server, Victim), User,
                                                     Whisper);
                                else {
                                    if (!Reason.empty()) {
                                        m_PairedBanAdds.push_back(PairedBanAdd(Whisper ? User : string(),
                                                                               m_OHBot->m_DB->ThreadedBanAdd(m_Server,
                                                                                                             Victim,
                                                                                                             string(),
                                                                                                             string(),
                                                                                                             User,
                                                                                                             Reason,
                                                                                                             BanTime,
                                                                                                             "")));
                                        //QueueChatCommand( "Temporary ban: " + Victim + " for " + UTIL_ToString(Amount) + " " + Suffix + " with reason: " + Reason, User, Whisper);
                                    }
                                    else
                                        QueueChatCommand(m_OHBot->m_Language->ErrorMissingReason(), User, Whisper);
                                }
                            }
                            else
                                QueueChatCommand(m_OHBot->m_Language->NoPermissionToExecCommand(), User, Whisper);

                        }
                        else {
                            QueueChatCommand(m_OHBot->m_Language->ErrorBanningWrongSuffix(Suffix), User, Whisper);
                        }
                    }
                }
            }
        }
/*
        //
        // !ANNOUNCE
        //

        else if( Command == "announce" && m_OHBot->m_CurrentGame && !m_OHBot->m_CurrentGame->GetCountDownStarted( ) )
        {
            if( Payload.empty( ) || Payload == "off" )
            {
                QueueChatCommand( m_OHBot->m_Language->AnnounceMessageDisabled( ), User, Whisper );
                m_OHBot->m_CurrentGame->SetAnnounce( 0, string( ) );
            }
            else
            {
                // extract the interval and the message
                // e.g. "30 hello everyone" -> interval: "30", message: "hello everyone"

                uint32_t Interval;
                string Message;
                stringstream SS;
                SS << Payload;
                SS >> Interval;

                if( SS.fail( ) || Interval == 0 )
                    CONSOLE_Print( "[BNET: " + m_ServerAlias + "] bad input #1 to the announce command" );
                else
                {
                    if( SS.eof( ) )
                        CONSOLE_Print( "[BNET: " + m_ServerAlias + "] missing input #2 to the announce command" );
                    else
                    {
                        getline( SS, Message );
                        string :: size_type Start = Message.find_first_not_of( " " );

                        if( Start != string :: npos )
                            Message = Message.substr( Start );

                        QueueChatCommand( m_OHBot->m_Language->AnnounceMessageEnabled( ), User, Whisper );
                        m_OHBot->m_CurrentGame->SetAnnounce( Interval, Message );
                    }
                }
            }
        }
*/
            //
            // !AUTOHOST
            //

        else if (Command == "autohost" && !m_OHBot->m_ChannelBotOnly) {
            if (IsLevel(User) == 10 || ForceRoot) {
                if (Payload.empty() || Payload == "off") {
                    QueueChatCommand(m_OHBot->m_Language->AutoHostDisabled(), User, Whisper);
                    m_OHBot->m_AutoHostGameName.clear();
                    m_OHBot->m_AutoHostOwner.clear();
                    m_OHBot->m_AutoHostServer.clear();
                    m_OHBot->m_AutoHostMaximumGames = 0;
                    m_OHBot->m_AutoHostAutoStartPlayers = 0;
                    m_OHBot->m_AutoHostGameType = 3;
                    m_OHBot->m_LastAutoHostTime = GetTime();
                    m_OHBot->m_AutoHostMatchMaking = false;
                    m_OHBot->m_AutoHostMinimumScore = 0.0;
                    m_OHBot->m_AutoHostMaximumScore = 0.0;

                    cmdSuccess = true;
                }
                else {
                    // extract the maximum games, auto start players, and the game name
                    // e.g. "5 10 BattleShips Pro" -> maximum games: "5", auto start players: "10", game name: "BattleShips Pro"

                    uint32_t MaximumGames;
                    uint32_t AutoStartPlayers;
                    uint32_t GameType;
                    string GameName;
                    stringstream SS;
                    SS << Payload;
                    SS >> MaximumGames;

                    if (SS.fail() || MaximumGames == 0) {
                        CONSOLE_Print("[BNET: " + m_ServerAlias + "] bad input #1 to the autohost command");
                        QueueChatCommand("Bad input #1 (maximum number of games). Allowed: > 0", User, true);
                    } else {
                        SS >> AutoStartPlayers;

                        if (SS.fail() || AutoStartPlayers < 1) {
                            CONSOLE_Print("[BNET: " + m_ServerAlias + "] bad input #2 to the autohost command");
                            QueueChatCommand("Bad input #2 (minimum playerscount to start). Allowed: > 0", User, true);
                        } else {
                            SS >> GameType;
                            if (SS.fail() || GameType < 3 || GameType > 5) {
                                CONSOLE_Print("[BNET: " + m_ServerAlias + "] bad input #3 to the autohost command");
                                QueueChatCommand("Bad input #3 (GameType). Allowed: 3 = Standard, 4 = VIPGame, 5 = AdminGame", User, true);
                            } else {
                                if (SS.eof()) {
                                    CONSOLE_Print("[BNET: " + m_ServerAlias + "] missing input #4 to the autohost command");
                                    QueueChatCommand("Bad input #4 (hosted game name). Allowed: Any text.", User, true);
                                } else {
                                    getline(SS, GameName);
                                    string::size_type Start = GameName.find_first_not_of(" ");

                                    if (Start != string::npos)
                                        GameName = GameName.substr(Start);

                                    QueueChatCommand(m_OHBot->m_Language->AutoHostEnabled(), User, Whisper);
                                    delete m_OHBot->m_AutoHostMap;
                                    m_OHBot->m_AutoHostMap = new CMap(*m_OHBot->m_Map);
                                    if (GameType == 4)
                                        m_OHBot->m_AutoHostGameName = "[V]" + GameName;
                                    else if (GameType == 5)
                                        m_OHBot->m_AutoHostGameName = "[R]" + GameName;
                                    else
                                        m_OHBot->m_AutoHostGameName = GameName;
                                    m_OHBot->m_AutoHostOwner = User;
                                    m_OHBot->m_AutoHostServer = m_Server;
                                    m_OHBot->m_AutoHostGameType = GameType;
                                    m_OHBot->m_AutoHostMaximumGames = MaximumGames;
                                    m_OHBot->m_AutoHostAutoStartPlayers = AutoStartPlayers;
                                    m_OHBot->m_LastAutoHostTime = GetTime();
                                    m_OHBot->m_AutoHostMatchMaking = false;
                                    m_OHBot->m_AutoHostMinimumScore = 0.0;
                                    m_OHBot->m_AutoHostMaximumScore = 0.0;

                                    cmdSuccess = true;
                                }
                            }
                        }
                    }
                }

                if( !cmdSuccess ){
                    QueueChatCommand("Usage: !autohost MaximumGames AutoStartPlayers GameType GameName (e.g. !autohost 1 4 3 My Hosted Map)", User, true);
                }
            }
            else
                QueueChatCommand(m_OHBot->m_Language->YouDontHaveAccessToThatCommand(), User, Whisper);
        }

            //
            // !AUTOHOSTMM
            //

        else if (Command == "autohostmm" && !m_OHBot->m_ChannelBotOnly) {
            if (IsLevel(User) == 10 || ForceRoot) {
                if (Payload.empty() || Payload == "off") {
                    QueueChatCommand(m_OHBot->m_Language->AutoHostDisabled(), User, Whisper);
                    m_OHBot->m_AutoHostGameName.clear();
                    m_OHBot->m_AutoHostOwner.clear();
                    m_OHBot->m_AutoHostServer.clear();
                    m_OHBot->m_AutoHostMaximumGames = 0;
                    m_OHBot->m_AutoHostAutoStartPlayers = 0;
                    m_OHBot->m_LastAutoHostTime = GetTime();
                    m_OHBot->m_AutoHostMatchMaking = false;
                    m_OHBot->m_AutoHostMinimumScore = 0.0;
                    m_OHBot->m_AutoHostMaximumScore = 0.0;
                    m_OHBot->m_AutoHostGameType = 3;

                    cmdSuccess = true;
                }
                else {
                    // extract the maximum games, auto start players, minimum score, maximum score, and the game name
                    // e.g. "5 10 800 1200 BattleShips Pro" -> maximum games: "5", auto start players: "10", minimum score: "800", maximum score: "1200", game name: "BattleShips Pro"

                    uint32_t MaximumGames;
                    uint32_t AutoStartPlayers;
                    uint32_t GameType;
                    double MinimumScore;
                    double MaximumScore;
                    string GameName;
                    stringstream SS;
                    SS << Payload;
                    SS >> MaximumGames;

                    if (SS.fail() || MaximumGames == 0) {
                        CONSOLE_Print("[BNET: " + m_ServerAlias + "] bad input #1 to the autohostmm command");
                        QueueChatCommand("Bad input #1 (maximum number of games). Allowed: > 0", User, true);
                    } else {
                        SS >> AutoStartPlayers;

                        if (SS.fail() || AutoStartPlayers == 0) {
                            CONSOLE_Print("[BNET: " + m_ServerAlias + "] bad input #2 to the autohostmm command");
                            QueueChatCommand("Bad input #2 (minimum playerscount to start). Allowed: > 0", User, true);
                        } else {
                            SS >> GameType;
                            if (SS.fail() || GameType < 3 || GameType > 5) {
                                CONSOLE_Print("[BNET: " + m_ServerAlias + "] bad input #3 to the autohostmm command");
                                QueueChatCommand("Bad input #3 (GameType). Allowed: 3 = Standard, 4 = VIPGame, 5 = AdminGame", User, true);
                            } else {
                                SS >> MinimumScore;

                                if (SS.fail()) {
                                    CONSOLE_Print("[BNET: " + m_ServerAlias + "] bad input #4 to the autohostmm command");
                                    QueueChatCommand("Bad input #4 (minimum player score). Allowed: > 0", User, true);
                                } else {
                                    SS >> MaximumScore;

                                    if (SS.fail()) {
                                        CONSOLE_Print("[BNET: " + m_ServerAlias + "] bad input #5 to the autohostmm command");
                                        QueueChatCommand("Bad input #5 (maximum player score). Allowed: > 0", User, true);
                                    } else {
                                        if (SS.eof()) {
                                            CONSOLE_Print("[BNET: " + m_ServerAlias + "] missing input #6 to the autohostmm command");
                                            QueueChatCommand("Bad input #6 (hosted game name). Allowed: Any text.", User, true);
                                        } else {
                                            getline(SS, GameName);
                                            string::size_type Start = GameName.find_first_not_of(" ");

                                            if (Start != string::npos)
                                                GameName = GameName.substr(Start);

                                            QueueChatCommand(m_OHBot->m_Language->AutoHostEnabled(), User, Whisper);
                                            delete m_OHBot->m_AutoHostMap;
                                            m_OHBot->m_AutoHostMap = new CMap(*m_OHBot->m_Map);
                                            m_OHBot->m_AutoHostGameName = GameName;
                                            m_OHBot->m_AutoHostOwner = User;
                                            m_OHBot->m_AutoHostServer = m_Server;
                                            m_OHBot->m_AutoHostMaximumGames = MaximumGames;
                                            m_OHBot->m_AutoHostAutoStartPlayers = AutoStartPlayers;
                                            m_OHBot->m_LastAutoHostTime = GetTime();
                                            m_OHBot->m_AutoHostMatchMaking = true;
                                            m_OHBot->m_AutoHostMinimumScore = MinimumScore;
                                            m_OHBot->m_AutoHostMaximumScore = MaximumScore;
                                            m_OHBot->m_AutoHostGameType = GameType;

                                            cmdSuccess = true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                if( !cmdSuccess ){
                    QueueChatCommand("Usage: !autohostmm MaximumGames AutoStartPlayers GameType MinimumScore MaximumScore GameName (e.g. !autohostmm 1 4 3 50 400 My Hosted Map)", User, true);
                }

            }
            else
                QueueChatCommand(m_OHBot->m_Language->YouDontHaveAccessToThatCommand(), User, Whisper);
        }
/*
        //
        // !AUTOSTART
        //

        else if( Command == "autostart" && m_OHBot->m_CurrentGame && !m_OHBot->m_CurrentGame->GetCountDownStarted( ) && IsLevel( User ) >= 9 )
        {
            if( Payload.empty( ) || Payload == "off" )
            {
                QueueChatCommand( m_OHBot->m_Language->AutoStartDisabled( ), User, Whisper );
                m_OHBot->m_CurrentGame->SetAutoStartPlayers( 0 );
            }
            else
            {
                uint32_t AutoStartPlayers = UTIL_ToUInt32( Payload );

                if( AutoStartPlayers != 0 )
                {
                    QueueChatCommand( m_OHBot->m_Language->AutoStartEnabled( UTIL_ToString( AutoStartPlayers ) ), User, Whisper );
                    m_OHBot->m_CurrentGame->SetAutoStartPlayers( AutoStartPlayers );
                }
            }
        }
*/
            //
            // !CHANNEL (change channel)
            //

        else if (Command == "channel" && !Payload.empty() && IsLevel(User) >= 9)
            QueueChatCommand("/join " + Payload);

            //
            // !CHECKBAN
            //

        else if (Command == "checkban" && !Payload.empty()) {
            CDBBan *Ban = IsBannedName(Payload);

            if (Ban) {
                string Remaining;
                if (Ban->GetMonths() != "")
                    Remaining += Ban->GetMonths() + "w ";
                if (Ban->GetDays() != "")
                    Remaining += Ban->GetDays() + "d ";
                if (Ban->GetHours() != "")
                    Remaining += Ban->GetHours() + "h ";
                if (Ban->GetMinutes() != "")
                    Remaining += Ban->GetMinutes() + "m";

                if (!Remaining.empty())
                    QueueChatCommand(
                            m_OHBot->m_Language->CheckBanTempUser(Ban->GetName(), Ban->GetDate(), Ban->GetExpire(),
                                                                  Remaining, Ban->GetReason()));
                else
                    QueueChatCommand(
                            m_OHBot->m_Language->CheckBanPermUser(Ban->GetName(), Ban->GetDate(), Ban->GetReason()),
                            User, Whisper);
            }
            else
                QueueChatCommand(m_OHBot->m_Language->UserIsNotBanned(m_Server, Payload), User, Whisper);
        }

            //
            // !CHECKIPBAN
            //

        else if (Command == "checkipban" && !Payload.empty()) {
            CDBBan *Ban = IsBannedName(Payload);
            if (Ban) {
                QueueChatCommand(m_OHBot->m_Language->CheckBanNameBan(), User, Whisper);
                return;
            }
            else {
                m_PairedBanCheck2s.push_back(PairedBanCheck2(Whisper ? User : string(),
                                                             m_OHBot->m_DB->ThreadedBanCheck2(m_Server, Payload,
                                                                                              "check")));
            }
        }
/*
        //
        // !CLOSE (close slot)
        //

        else if( Command == "close" && !Payload.empty( ) && m_OHBot->m_CurrentGame && IsLevel( User ) >= 7 )
        {
            if( !m_OHBot->m_CurrentGame->GetLocked( ) )
            {
                // close as many slots as specified, e.g. "5 10" closes slots 5 and 10

                stringstream SS;
                SS << Payload;

                while( !SS.eof( ) )
                {
                    uint32_t SID;
                    SS >> SID;

                    if( SS.fail( ) )
                    {
                        CONSOLE_Print( "[BNET: " + m_ServerAlias + "] bad input to the close command" );
                        break;
                    }
                    else
                        m_OHBot->m_CurrentGame->CloseSlot( (unsigned char)( SID - 1 ), true );
                }
            }
            else
                QueueChatCommand( m_OHBot->m_Language->TheGameIsLockedBNET( ), User, Whisper );
        }

        //
        // !CLOSEALL
        //

        else if( Command == "closeall" && m_OHBot->m_CurrentGame && IsLevel( User ) >= 9 )
        {
            if( !m_OHBot->m_CurrentGame->GetLocked( ) )
                m_OHBot->m_CurrentGame->CloseAllSlots( );
            else
                QueueChatCommand( m_OHBot->m_Language->TheGameIsLockedBNET( ), User, Whisper );
        }
*/
            //
            // !COUNTBANS
            //

        else if (Command == "countbans" && IsLevel(User) >= 9)
            m_PairedBanCounts.push_back(
                    PairedBanCount(Whisper ? User : string(), m_OHBot->m_DB->ThreadedBanCount(m_Server)));

            //
            // !DBSTATUS
            //

        else if (Command == "dbstatus" && IsLevel(User) >= 9)
            QueueChatCommand(m_OHBot->m_DB->GetStatus(), User, Whisper);

            //
            // !DELBAN
            // !UNBAN
            //

        else if ((Command == "delban" || Command == "unban") && !Payload.empty() && IsLevel(User) >= 7)
            m_PairedBanRemoves.push_back(
                    PairedBanRemove(Whisper ? User : string(), m_OHBot->m_DB->ThreadedBanRemove(Payload)));

            //
            // !DISABLE
            //

        else if (Command == "disable") {
            if (IsLevel(User) == 10 || ForceRoot) {
                QueueChatCommand(m_OHBot->m_Language->BotDisabled(), User, Whisper);
                m_OHBot->m_Enabled = false;
            }
            else
                QueueChatCommand(m_OHBot->m_Language->YouDontHaveAccessToThatCommand(), User, Whisper);
        }

            //
            // !DOWNLOADS
            //

        else if (Command == "downloads" && !Payload.empty()) {
            uint32_t Downloads = UTIL_ToUInt32(Payload);

            if (Downloads == 0) {
                QueueChatCommand(m_OHBot->m_Language->MapDownloadsDisabled(), User, Whisper);
                m_OHBot->m_AllowDownloads = 0;
            }
            else if (Downloads == 1) {
                QueueChatCommand(m_OHBot->m_Language->MapDownloadsEnabled(), User, Whisper);
                m_OHBot->m_AllowDownloads = 1;
            }
            else if (Downloads == 2) {
                QueueChatCommand(m_OHBot->m_Language->MapDownloadsConditional(), User, Whisper);
                m_OHBot->m_AllowDownloads = 2;
            }
        }

            //
            // !ENABLE
            //

        else if (Command == "enable") {
            if (IsLevel(User) == 10 || ForceRoot) {
                QueueChatCommand(m_OHBot->m_Language->BotEnabled(), User, Whisper);
                m_OHBot->m_Enabled = true;
            }
            else
                QueueChatCommand(m_OHBot->m_Language->YouDontHaveAccessToThatCommand(), User, Whisper);
        }

            //
            // !END
            //

        else if (Command == "end" && !Payload.empty() && IsLevel(User) >= 7) {
            // todotodo: what if a game ends just as you're typing this command and the numbering changes?

            uint32_t GameNumber = UTIL_ToUInt32(Payload) - 1;

            if (GameNumber < m_OHBot->m_Games.size()) {
                // if the game owner is still in the game only allow the root admin to end the game

                if (m_OHBot->m_Games[GameNumber]->GetPlayerFromName(m_OHBot->m_Games[GameNumber]->GetOwnerName(),
                                                                    false) && IsLevel(User) != 10)
                    QueueChatCommand(m_OHBot->m_Language->CantEndGameOwnerIsStillPlaying(
                            m_OHBot->m_Games[GameNumber]->GetOwnerName()), User, Whisper);
                else {
                    QueueChatCommand(m_OHBot->m_Language->EndingGame(m_OHBot->m_Games[GameNumber]->GetDescription()),
                                     User, Whisper);
                    CONSOLE_Print(
                            "[GAME: " + m_OHBot->m_Games[GameNumber]->GetGameName() + "] is over (admin ended game)");
                    m_OHBot->m_Games[GameNumber]->StopPlayers("was disconnected (admin ended game)");
                }
            }
            else
                QueueChatCommand(m_OHBot->m_Language->GameNumberDoesntExist(Payload), User, Whisper);
        }

            //
            // !ENFORCESG
            //

        else if (Command == "enforcesg" && !Payload.empty()) {
            // only load files in the current directory just to be safe

            if (Payload.find("/") != string::npos || Payload.find("\\") != string::npos)
                QueueChatCommand(m_OHBot->m_Language->UnableToLoadReplaysOutside(), User, Whisper);
            else {
                string File = m_OHBot->m_ReplayPath + Payload + ".w3g";

                if (UTIL_FileExists(File)) {
                    QueueChatCommand(m_OHBot->m_Language->LoadingReplay(File), User, Whisper);
                    CReplay *Replay = new CReplay();
                    Replay->Load(File, false);
                    Replay->ParseReplay(false);
                    m_OHBot->m_EnforcePlayers = Replay->GetPlayers();
                    delete Replay;
                }
                else
                    QueueChatCommand(m_OHBot->m_Language->UnableToLoadReplayDoesntExist(File), User, Whisper);
            }
        }

            //
            // !GETCLAN
            //

        else if (Command == "getclan") {
            SendGetClanList();
            QueueChatCommand(m_OHBot->m_Language->UpdatingClanList(), User, Whisper);
        }

            //
            // !GETFRIENDS
            //

        else if (Command == "getfriends") {
            SendGetFriendsList();
            QueueChatCommand(m_OHBot->m_Language->UpdatingFriendsList(), User, Whisper);
        }

            //
            // !GETGAME
            //

        else if (Command == "getgame" && !Payload.empty() && IsLevel(User) >= 9) {
            uint32_t GameNumber = UTIL_ToUInt32(Payload) - 1;

            if (GameNumber < m_OHBot->m_Games.size())
                QueueChatCommand(
                        m_OHBot->m_Language->GameNumberIs(Payload, m_OHBot->m_Games[GameNumber]->GetDescription()),
                        User, Whisper);
            else
                QueueChatCommand(m_OHBot->m_Language->GameNumberDoesntExist(Payload), User, Whisper);
        }

            //
            // !LOAD (load config file)
            //

        else if (Command == "load" && IsLevel(User) >= 8 && !m_OHBot->m_ChannelBotOnly) {
            if (Payload.empty())
                QueueChatCommand(m_OHBot->m_Language->CurrentlyLoadedMapCFGIs(m_OHBot->m_Map->GetCFGFile()), User,
                                 Whisper);
            else {
                string FoundMapConfigs;

                try {
                    path MapCFGPath(m_OHBot->m_MapCFGPath);
                    string Pattern = Payload;
                    transform(Pattern.begin(), Pattern.end(), Pattern.begin(), ::tolower);

                    if (!exists(MapCFGPath)) {
                        CONSOLE_Print("[BNET: " + m_ServerAlias +
                                      "] error listing map configs - map config path doesn't exist");
                        QueueChatCommand(m_OHBot->m_Language->ErrorListingMapConfigs(), User, Whisper);
                    }
                    else {
                        directory_iterator EndIterator;
                        path LastMatch;
                        uint32_t Matches = 0;

                        for (directory_iterator i(MapCFGPath); i != EndIterator; ++i) {
                            string FileName = i->path().filename().string();
                            string Stem = i->path().stem().string();
                            transform(FileName.begin(), FileName.end(), FileName.begin(), ::tolower);
                            transform(Stem.begin(), Stem.end(), Stem.begin(), ::tolower);

                            if (!is_directory(i->status()) && i->path().extension() == ".cfg" &&
                                FileName.find(Pattern) != string::npos) {
                                LastMatch = i->path();
                                ++Matches;

                                if (FoundMapConfigs.empty())
                                    FoundMapConfigs = i->path().filename().string();
                                else
                                    FoundMapConfigs += ", " + i->path().filename().string();

                                // if the pattern matches the filename exactly, with or without extension, stop any further matching

                                if (FileName == Pattern || Stem == Pattern) {
                                    Matches = 1;
                                    break;
                                }
                            }
                        }

                        if (Matches == 0)

                            QueueChatCommand(m_OHBot->m_Language->NoMapConfigsFound(), User, Whisper);
                        else if (Matches == 1) {
                            string File = LastMatch.filename().string();
                            QueueChatCommand(m_OHBot->m_Language->LoadingConfigFile(m_OHBot->m_MapCFGPath + File), User,
                                             Whisper);
                            CConfig MapCFG;
                            MapCFG.Read(LastMatch.string());
                            m_OHBot->m_Map->Load(&MapCFG, m_OHBot->m_MapCFGPath + File);
                        }
                        else
                            QueueChatCommand(m_OHBot->m_Language->FoundMapConfigs(FoundMapConfigs), User, Whisper);
                    }
                }
                catch (const exception &ex) {
                    CONSOLE_Print(
                            "[BNET: " + m_ServerAlias + "] error listing map configs - caught exception [" + ex.what() +
                            "]");
                    QueueChatCommand(m_OHBot->m_Language->ErrorListingMapConfigs(), User, Whisper);
                }
            }
        }

            //
            // !MAP (load map file)
            //

        else if (Command == "map" && IsLevel(User) >= 8 && !m_OHBot->m_ChannelBotOnly) {
            if (Payload.empty())
                QueueChatCommand(m_OHBot->m_Language->CurrentlyLoadedMapCFGIs(m_OHBot->m_Map->GetCFGFile()), User,
                                 Whisper);
            else {
                string FoundMaps;

                try {
                    path MapPath(m_OHBot->m_MapPath);
                    string Pattern = Payload;
                    transform(Pattern.begin(), Pattern.end(), Pattern.begin(), ::tolower);

                    if (!exists(MapPath)) {
                        CONSOLE_Print("[BNET: " + m_ServerAlias + "] error listing maps - map path doesn't exist");
                        QueueChatCommand(m_OHBot->m_Language->ErrorListingMaps(), User, Whisper);
                    }
                    else {
                        directory_iterator EndIterator;
                        path LastMatch;
                        uint32_t Matches = 0;

                        for (directory_iterator i(MapPath); i != EndIterator; ++i) {
                            string FileName = i->path().filename().string();
                            string Stem = i->path().stem().string();
                            transform(FileName.begin(), FileName.end(), FileName.begin(), ::tolower);
                            transform(Stem.begin(), Stem.end(), Stem.begin(), ::tolower);

                            if (!is_directory(i->status()) && FileName.find(Pattern) != string::npos) {
                                LastMatch = i->path();
                                ++Matches;

                                if (FoundMaps.empty())
                                    FoundMaps = i->path().filename().string();
                                else
                                    FoundMaps += ", " + i->path().filename().string();

                                // if the pattern matches the filename exactly, with or without extension, stop any further matching

                                if (FileName == Pattern || Stem == Pattern) {
                                    Matches = 1;
                                    break;
                                }
                            }
                        }

                        if (Matches == 0)
                            QueueChatCommand(m_OHBot->m_Language->NoMapsFound(), User, Whisper);
                        else if (Matches == 1) {
                            string File = LastMatch.filename().string();
                            QueueChatCommand(m_OHBot->m_Language->LoadingConfigFile(File), User, Whisper);

                            // hackhack: create a config file in memory with the required information to load the map

                            CConfig MapCFG;
                            MapCFG.Set("map_path", "Maps\\Download\\" + File);
                            MapCFG.Set("map_localpath", File);
                            m_OHBot->m_Map->Load(&MapCFG, File);
                        }
                        else
                            QueueChatCommand(m_OHBot->m_Language->FoundMaps(FoundMaps), User, Whisper);
                    }
                }
                catch (const exception &ex) {
                    CONSOLE_Print( "[BNET: " + m_ServerAlias + "] error listing maps - caught exception [" + ex.what() + "]");
                    QueueChatCommand(m_OHBot->m_Language->ErrorListingMaps(), User, Whisper);
                }
            }
        }

            //
            // !PUB host a single public game
            //

        else if (Command == "pub" && !m_OHBot->m_ChannelBotOnly) {
            if (IsLevel(User) == 10 || ForceRoot) {

                // extract the game name
                // e.g. game name: "BattleShips Pro"

                string GameName;
                stringstream SS;
                SS << Payload;

                if (SS.eof())
                    CONSOLE_Print("[BNET: " + m_ServerAlias + "] missing input #1 (GameName) to the pub command");
                else {
                    getline(SS, GameName);
                    string::size_type Start = GameName.find_first_not_of(" ");

                    if (Start != string::npos)
                        GameName = GameName.substr(Start);

                    // QueueChatCommand( m_OHBot->m_Language->AutoHostEnabled( ), User, Whisper );
                    m_OHBot->CreateGame(m_OHBot->m_Map, GAME_PUBLIC, false, GameName, User, User, m_Server, 1, Whisper,
                                        1);
                }
            }
            else
                QueueChatCommand(m_OHBot->m_Language->YouDontHaveAccessToThatCommand(), User, Whisper);
        }

            //
            // !PUBBY host a single public game
            // @TODO Implementation
            //

            //
            // !PRIV host a single private game
            // @TODO Test Implementation
            //

        else if (Command == "priv" && !m_OHBot->m_ChannelBotOnly) {
            if (IsLevel(User) == 10 || ForceRoot) {

                // extract the game name
                // e.g. game name: "BattleShips Pro"

                string GameName;
                stringstream SS;
                SS << Payload;

                if (SS.eof())
                    CONSOLE_Print("[BNET: " + m_ServerAlias + "] missing input #1 (GameName) to the pub command");
                else {
                    getline(SS, GameName);
                    string::size_type Start = GameName.find_first_not_of(" ");

                    if (Start != string::npos)
                        GameName = GameName.substr(Start);

                    // QueueChatCommand( m_OHBot->m_Language->AutoHostEnabled( ), User, Whisper );
                    m_OHBot->CreateGame(m_OHBot->m_Map, GAME_PRIVATE, false, GameName, User, User, m_Server, 1, Whisper,
                                        1);
                }
            }
            else
                QueueChatCommand(m_OHBot->m_Language->YouDontHaveAccessToThatCommand(), User, Whisper);
        }

            //
            // !VIP Reg Needed
            //

        else if (Command == "vipreg" && IsLevel(User) >= 8) {
            if (Payload == "on") {
                m_OHBot->m_RegVIPGames = 1;
                QueueChatCommand(m_OHBot->m_Language->ChangedStatusForVIPGames(Payload), User, Whisper);
            }
            else if (Payload == "off") {
                m_OHBot->m_RegVIPGames = 0;
                QueueChatCommand(m_OHBot->m_Language->ChangedStatusForVIPGames(Payload), User, Whisper);
            }
            else
                QueueChatCommand(m_OHBot->m_Language->WrongOptionUserOnOff(), User, Whisper);
        }

            //
            // !VIPMINGAMES
            //
        else if (Command == "vipmingames" && IsLevel(User) >= 8) {
            if (Payload.empty()) {
                m_OHBot->m_MinVIPGames = 50;
                QueueChatCommand(
                        m_OHBot->m_Language->ChangedMinPlayedGames("VIP", UTIL_ToString(m_OHBot->m_MinVIPGames)), User,
                        Whisper);
            }
            else {
                m_OHBot->m_MinVIPGames = UTIL_ToUInt32(Payload);
                QueueChatCommand(
                        m_OHBot->m_Language->ChangedMinPlayedGames("VIP", UTIL_ToString(m_OHBot->m_MinVIPGames)), User,
                        Whisper);
            }
        }

            //
            // !HIGHMINGAMES
            //
        else if (Command == "highmingames" && IsLevel(User) >= 8) {
            if (Payload.empty()) {
                m_OHBot->m_MinLimit = 25;
                QueueChatCommand(m_OHBot->m_Language->ChangedMinPlayedGames("HIGH", UTIL_ToString(m_OHBot->m_MinLimit)),
                                 User, Whisper);
            }
            else {
                m_OHBot->m_MinLimit = UTIL_ToUInt32(Payload);
                QueueChatCommand(m_OHBot->m_Language->ChangedMinPlayedGames("HIGH", UTIL_ToString(m_OHBot->m_MinLimit)),
                                 User, Whisper);
            }
        }

            //
            // !RELOAD
            //

        else if (Command == "reload") {
            if (IsLevel(User) >= 9 || ForceRoot) {
                QueueChatCommand(m_OHBot->m_Language->ReloadingConfigurationFiles(), User, Whisper);
                m_OHBot->ReloadConfigs();
            }
            else
                QueueChatCommand(m_OHBot->m_Language->YouDontHaveAccessToThatCommand(), User, Whisper);
        }

            //
            // !SAY
            //

        else if (Command == "say" && !Payload.empty()) {
            if (IsLevel(User) == 10 || ForceRoot) {
                QueueChatCommand(Payload);
            }
            else
                QueueChatCommand(m_OHBot->m_Language->YouDontHaveAccessToThatCommand(), User, Whisper);
        }

            //
            // !SAYGAME
            //

        else if (Command == "saygame" && !Payload.empty()) {
            if (IsLevel(User) == 10 || ForceRoot) {
                // extract the game number and the message
                // e.g. "3 hello everyone" -> game number: "3", message: "hello everyone"

                uint32_t GameNumber;
                string SayMessage;
                stringstream SS;
                SS << Payload;
                SS >> GameNumber;

                if (SS.fail())
                    CONSOLE_Print("[BNET: " + m_ServerAlias + "] bad input #1 (GameNumber) to the saygame command");
                else {
                    if (SS.eof())
                        CONSOLE_Print(
                                "[BNET: " + m_ServerAlias + "] missing input #2 (Message) to the saygame command");
                    else {
                        getline(SS, SayMessage);
                        string::size_type Start = SayMessage.find_first_not_of(" ");

                        if (Start != string::npos)
                            SayMessage = SayMessage.substr(Start);

                        if (GameNumber - 1 < m_OHBot->m_Games.size())
                            m_OHBot->m_Games[GameNumber - 1]->SendAllChat("ADMIN: " + SayMessage);
                        else
                            QueueChatCommand(m_OHBot->m_Language->GameNumberDoesntExist(UTIL_ToString(GameNumber)),
                                             User, Whisper);
                    }
                }
            }
            else
                QueueChatCommand(m_OHBot->m_Language->YouDontHaveAccessToThatCommand(), User, Whisper);
        }

            //
            // !WARDENSTATUS
            //

        else if (Command == "wardenstatus" && IsLevel(User) >= 9) {
            if (m_BNLSClient)
                QueueChatCommand("WARDEN STATUS --- " + UTIL_ToString(m_BNLSClient->GetTotalWardenIn()) +
                                 " requests received, " + UTIL_ToString(m_BNLSClient->GetTotalWardenOut()) +
                                 " responses sent.", User, Whisper);
            else
                QueueChatCommand("WARDEN STATUS --- Not connected to a BNLS server.", User, Whisper);
        }
    }
    else if (!m_OHBot->m_RanksLoaded)
        CONSOLE_Print(m_OHBot->m_Language->RanksNotLoaded());
    else
        CONSOLE_Print("[BNET: " + m_ServerAlias + "] non-admin [" + User + "] sent command [" + Message + "]");

    /*********************
    * NON ADMIN COMMANDS *
    *********************/

    // don't respond to non admins if there are more than 3 messages already in the queue
    // this prevents malicious users from filling up the bot's chat queue and crippling the bot
    // in some cases the queue may be full of legitimate messages but we don't really care if the bot ignores one of these commands once in awhile
    // e.g. when several users join a game at the same time and cause multiple /whois messages to be queued at once

    if (IsLevel(User) >= 9 || (m_PublicCommands && m_OutPackets.size() <= 3)) {
        //
        // !GAMES
        //

        if (Command == "games" || Command == "g") {
            m_PairedSSs.push_back(PairedSS(Whisper ? User : string(),
                                           m_OHBot->m_DB->ThreadedStatsSystem(Payload, "", 0, "gamelist")));
        }

        //
        // !CHECKBAN
        //
        if (Command == "checkban" && Payload.empty()) {
            CDBBan *Ban = IsBannedName(User);

            if (Ban) {
                string Remaining;
                if (Ban->GetMonths() != "")
                    Remaining += Ban->GetMonths() + "w ";
                if (Ban->GetDays() != "")
                    Remaining += Ban->GetDays() + "d ";
                if (Ban->GetHours() != "")
                    Remaining += Ban->GetHours() + "h ";
                if (Ban->GetMinutes() != "")
                    Remaining += Ban->GetMinutes() + "m";

                QueueChatCommand(m_OHBot->m_Language->CheckBanTempUser(Ban->GetName(), Ban->GetDate(), Ban->GetExpire(),
                                                                       Remaining, Ban->GetReason()));
            }
            else
                QueueChatCommand(m_OHBot->m_Language->UserIsNotBanned(m_Server, User), User, Whisper);
        }

            //
            // !CHECKIPBAN
            //
        else if (Command == "checkipban" && Payload.empty()) {
            CDBBan *Ban = IsBannedName(User);
            if (Ban) {
                QueueChatCommand(m_OHBot->m_Language->CheckBanNameBan(), User, Whisper);
                return;
            }
            else {
                m_PairedBanCheck2s.push_back(PairedBanCheck2(Whisper ? User : string(),
                                                             m_OHBot->m_DB->ThreadedBanCheck2(m_Server, User,
                                                                                              "check")));
            }
        }

            //
            // !RCONPW
            // This command is normally used in a relation to a modified GProxy.
            // GProxy will be added soon to the SVN
            //
        else if (Command == "rconpw" && m_OHBot->m_AccountProtection) {
            m_PairedPassChecks.push_back(PairedPassCheck(User, m_OHBot->m_DB->ThreadedPassCheck(User, Payload, 0)));
        }

            //
            // !STATS
            //

        else if (Command == "stats") {
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
                m_PairedGSChecks.push_back(PairedGSCheck(Whisper ? User : string(),
                                                         m_OHBot->m_DB->ThreadedStatsPlayerSummaryCheck(StatsUser,
                                                                                                        Month, Year,
                                                                                                        0)));
        }

            //
            // !RANKDOTA
            // !RANKLOD
            // !RANKIMBA
            // !RD
            // !RL
            // !RI
            //
        else if (Command.substr(0, 4) == "rank" || (Command == "rd" || Command == "rl" || Command == "ri")) {
            uint32_t m_StatsAlias = 0;
            if (Command.size() > 4)
                m_StatsAlias = m_OHBot->GetStatsAliasNumber(Command.substr(4, Command.size() - 4));
            else if (Command.size() < 4)
                m_StatsAlias = m_OHBot->GetStatsAliasNumber(Command.substr(1, Command.size() - 1));

            if (m_StatsAlias == 0)
                return;

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
                m_PairedRankChecks.push_back(PairedRankCheck(Whisper ? User : string(),
                                                             m_OHBot->m_DB->ThreadedStatsPlayerSummaryCheck(StatsUser,
                                                                                                            Month, Year,
                                                                                                            m_StatsAlias)));
        }

            //
            // !STATSDOTA
            // !STATSLOD
            // !STATSIMBA
            // !SD
            // !SL
            // !SI
            //
        else if ((Command.substr(0, 5) == "stats" && Command.size() > 5) ||
                 (Command == "sl" || Command == "sd" || Command == "si")) {
            uint32_t m_StatsAlias = 0;
            if (Command.size() > 5)
                m_StatsAlias = m_OHBot->GetStatsAliasNumber(Command.substr(5, Command.size() - 5));
            else if (Command.size() < 5)
                m_StatsAlias = m_OHBot->GetStatsAliasNumber(Command.substr(1, Command.size() - 1));

            if (m_StatsAlias == 0)
                return;

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
                m_PairedSChecks.push_back(PairedSCheck(Whisper ? User : string(),
                                                       m_OHBot->m_DB->ThreadedStatsPlayerSummaryCheck(StatsUser, Month,
                                                                                                      Year,
                                                                                                      m_StatsAlias)));
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
            if (Command.size() > 6)
                m_StatsAlias = m_OHBot->GetStatsAliasNumber(Command.substr(6, Command.size() - 6));
            else if (Command.size() < 6)
                m_StatsAlias = m_OHBot->GetStatsAliasNumber(Command.substr(2, Command.size() - 2));

            if (m_StatsAlias == 0)
                return;

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
                m_PairedStreakChecks.push_back(PairedStreakCheck(Whisper ? User : string(),
                                                                 m_OHBot->m_DB->ThreadedStatsPlayerSummaryCheck(
                                                                         StatsUser, Month, Year, m_StatsAlias)));
        }

            //
            // !VERSION
            //

        else if (Command == "version") {
            if (IsLevel(User) >= 9 || ForceRoot)
                QueueChatCommand(m_OHBot->m_Language->VersionAdmin(m_OHBot->m_Version), User, Whisper);
            else
                QueueChatCommand(m_OHBot->m_Language->VersionNotAdmin(m_OHBot->m_Version), User, Whisper);
        }

        /************************
        ** GRIEF-CODE COMMANDS **
        ************************/

            //
            // !PM
            //
        else if ((Command == "pm" || Command == "mail") && !Payload.empty() && m_OHBot->m_MessageSystem) {
            CDBBan *Ban = IsBannedName(User);

            string UserTo;
            string PMMessage;
            stringstream SS;
            SS << Payload;
            SS >> UserTo;
            if (!SS.eof()) {
                getline(SS, PMMessage);
                string::size_type Start = PMMessage.find_first_not_of(" ");

                if (Start != string::npos)
                    PMMessage = PMMessage.substr(Start);
            }
            else {
                QueueChatCommand(m_OHBot->m_Language->ErrorWrongInputForMessage(), User, true);
                return;
            }
            if (UserTo.length() >= 3) {
                if (PMMessage.length() > 3) {
                    QueueChatCommand(m_OHBot->m_Language->SuccessfullyStoredMessage(), User, true);
                    m_Pairedpms.push_back(Pairedpm(User, m_OHBot->m_DB->Threadedpm(User, UserTo, 0, PMMessage, "add")));
                }
                else
                    QueueChatCommand(m_OHBot->m_Language->ErrorMessageTooShort());
            }
            else
                QueueChatCommand(m_OHBot->m_Language->InvalidNameTooShort());
        }

            //
            // !INBOX
            //
        else if ((Command == "i" || Command == "inbox") && Payload.empty() && m_OHBot->m_MessageSystem) {
            m_PairedINChecks.push_back(PairedINCheck(User, m_OHBot->m_DB->ThreadedInboxSummaryCheck(User)));
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

            if (GetTime() - m_LastRegisterProcess <= 180 && (Command == "reg" || Command == "registration"))
                QueueChatCommand(m_OHBot->m_Language->ErrorRegisterAbusePrevention());
            else {
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
                else {
                    QueueChatCommand(m_OHBot->m_Language->WrongRegisterCommand(Command), User, true);
                    return;
                }
                if (Whisper) {
                    if (Mail.find("@") != string::npos || Mail.find(".") != string::npos) {
                        if (Password.find(" ") != string::npos)
                            QueueChatCommand(m_OHBot->m_Language->WrongPassRegisterCommand(Password), User, true);
                        else if (Password.length() > 2) {
                            m_PairedRegAdds.push_back(PairedRegAdd(User,
                                                                   m_OHBot->m_DB->ThreadedRegAdd(User, m_Server, Mail,
                                                                                                 Password, type)));
                        }
                        else
                            QueueChatCommand(m_OHBot->m_Language->PassTooShortRegisterCommand(Password), User, true);
                    }
                    else
                        QueueChatCommand(m_OHBot->m_Language->InvalidEmailRegisterCommand(Mail), User, true);
                }
                else
                    QueueChatCommand(m_OHBot->m_Language->ErrorWhispRegister(), User, true);
            }

        }

            //
            // !POINTS      !P
            //
        else if ((Command == "points" || Command == "p") && m_OHBot->m_BetSystem) {
            string StatsUser = User;

            if (!Payload.empty())
                StatsUser = Payload;

            // check for potential abuse

            if (!StatsUser.empty() && StatsUser.size() < 16 && StatsUser[0] != '/')
                m_PairedSSs.push_back(PairedSS(Whisper ? User : string(),
                                               m_OHBot->m_DB->ThreadedStatsSystem(StatsUser, "betsystem", 0,
                                                                                  "betcheck")));
        }

            //
            // !ALIAS      !AL
            //
        else if (Command == "alias" || Command == "al") {
            string StatsUser = User;
            if (IsLevel(User) >= 9) {
                if (!Payload.empty())
                    StatsUser = Payload;
            }

            // check for potential abuse

            if (!StatsUser.empty() && StatsUser.size() < 16 && StatsUser[0] != '/')
                m_PairedSSs.push_back(PairedSS(Whisper ? User : string(),
                                               m_OHBot->m_DB->ThreadedStatsSystem(StatsUser, "", 0, "aliascheck")));
        }

            //
            // !TOP      !TOP10
            //
        else if (Command == "top" || Command == "top10") {
            m_PairedSSs.push_back(
                    PairedSS(Whisper ? User : string(), m_OHBot->m_DB->ThreadedStatsSystem("", "", 0, "top")));
        }

            //
            // !GPROXYLIST
            //
        else if (Command == "gproxylist") {
            if (!Payload.empty()) {
                //return value is reserved :-D
                if (!m_OHBot->FindHackFiles(Payload)) {
                    //found hack file
                    m_PairedBanAdds.push_back(PairedBanAdd(Whisper ? User : string(),
                                                           m_OHBot->m_DB->ThreadedBanAdd(m_Server, User, string(),
                                                                                         string(), User,
                                                                                         "Suspicius File(s): " +
                                                                                         Payload, 0, "")));
                }
            }
        }
    }
}

void CBNET::SendJoinChannel(string channel) {
    if (m_LoggedIn && m_InChat)
        m_Socket->PutBytes(m_Protocol->SEND_SID_JOINCHANNEL(channel));
}

void CBNET::SendGetFriendsList() {
    if (m_LoggedIn)
        m_Socket->PutBytes(m_Protocol->SEND_SID_FRIENDSLIST());
}

void CBNET::SendGetClanList() {
    if (m_LoggedIn)
        m_Socket->PutBytes(m_Protocol->SEND_SID_CLANMEMBERLIST());
}

void CBNET::QueueEnterChat() {
    if (m_LoggedIn)
        m_OutPackets.push(m_Protocol->SEND_SID_ENTERCHAT());
}

void CBNET::QueueChatCommand(string chatCommand) {
    if (chatCommand.empty())
        return;

    if (m_LoggedIn) {
        if (m_PasswordHashType == "pvpgn" && chatCommand.size() > m_MaxMessageLength)
            chatCommand = chatCommand.substr(0, m_MaxMessageLength);

        if (chatCommand.size() > 255)
            chatCommand = chatCommand.substr(0, 255);

        if (m_OutPackets.size() > 10)
            CONSOLE_Print("[BNET: " + m_ServerAlias + "] attempted to queue a chat command [" + chatCommand + "] but there are too many (" + UTIL_ToString(m_OutPackets.size()) +
                          ") packets queued, discarding");
        else {
            CONSOLE_Print("[QUEUED: " + m_ServerAlias + "] " + chatCommand);
            m_OutPackets.push(m_Protocol->SEND_SID_CHATCOMMAND(chatCommand));
        }
    }
}

void CBNET::QueueChatCommand(string chatCommand, string user, bool whisper) {
    if (chatCommand.empty())
        return;

    // if whisper is true send the chat command as a whisper to user, otherwise just queue the chat command

    if (whisper)
        QueueChatCommand("/w " + user + " " + chatCommand);
    else
        QueueChatCommand(chatCommand);
}

void CBNET::QueueGameCreate(unsigned char state, string gameName, string hostName, CMap *map, CSaveGame *savegame,
                            uint32_t hostCounter) {
    if (m_LoggedIn && map) {
        if (!m_CurrentChannel.empty())
            m_FirstChannel = m_CurrentChannel;

        m_InChat = false;

        // a game creation message is just a game refresh message with upTime = 0

        QueueGameRefresh(state, gameName, hostName, map, savegame, 0, hostCounter);
    }
}

void CBNET::QueueGameRefresh(unsigned char state, string gameName, string hostName, CMap *map, CSaveGame *saveGame,
                             uint32_t upTime, uint32_t hostCounter) {
    if (hostName.empty()) {
        BYTEARRAY UniqueName = m_Protocol->GetUniqueName();
        hostName = string(UniqueName.begin(), UniqueName.end());
    }

    if (m_LoggedIn && map) {
        // construct a fixed host counter which will be used to identify players from this realm
        // the fixed host counter's 4 most significant bits will contain a 4 bit ID (0-15)
        // the rest of the fixed host counter will contain the 28 least significant bits of the actual host counter
        // since we're destroying 4 bits of information here the actual host counter should not be greater than 2^28 which is a reasonable assumption
        // when a player joins a game we can obtain the ID from the received host counter
        // note: LAN broadcasts use an ID of 0, battle.net refreshes use an ID of 1-10, the rest are unused

        uint32_t FixedHostCounter = (hostCounter & 0x0FFFFFFF) | (m_HostCounterID << 28);

        if (saveGame) {
            uint32_t MapGameType = MAPGAMETYPE_SAVEDGAME;

            // the state should always be private when creating a saved game

            if (state == GAME_PRIVATE)
                MapGameType |= MAPGAMETYPE_PRIVATEGAME;

            // use an invalid map width/height to indicate reconnectable games

            BYTEARRAY MapWidth;
            MapWidth.push_back(192);
            MapWidth.push_back(7);
            BYTEARRAY MapHeight;
            MapHeight.push_back(192);
            MapHeight.push_back(7);

            if (m_OHBot->HasMode( MODE_GPROXY ))
                m_OutPackets.push(m_Protocol->SEND_SID_STARTADVEX3(state, UTIL_CreateByteArray(MapGameType, false), map->GetMapGameFlags(), MapWidth, MapHeight,
                                                                   gameName, hostName, upTime, "Save\\Multiplayer\\" + saveGame->GetFileNameNoPath(),
                                                                   saveGame->GetMagicNumber(), map->GetMapSHA1(), FixedHostCounter));
            else
                m_OutPackets.push(m_Protocol->SEND_SID_STARTADVEX3(state, UTIL_CreateByteArray(MapGameType, false), map->GetMapGameFlags(),
                                                                   UTIL_CreateByteArray((uint16_t) 0, false), UTIL_CreateByteArray((uint16_t) 0, false), gameName,
                                                                   hostName, upTime, "Save\\Multiplayer\\" + saveGame->GetFileNameNoPath(),
                                                                   saveGame->GetMagicNumber(), map->GetMapSHA1(), FixedHostCounter));
        }
        else {
            uint32_t MapGameType = map->GetMapGameType();
            MapGameType |= MAPGAMETYPE_UNKNOWN0;
            //Apply overwrite if not equal to 0
            MapGameType = (m_OHBot->m_MapGameType == 0) ? MapGameType : m_OHBot->m_MapGameType;

            if (state == GAME_PRIVATE)
                MapGameType |= MAPGAMETYPE_PRIVATEGAME;

            // use an invalid map width/height to indicate reconnectable games

            BYTEARRAY MapWidth;
            MapWidth.push_back(192);
            MapWidth.push_back(7);
            BYTEARRAY MapHeight;
            MapHeight.push_back(192);
            MapHeight.push_back(7);

            //4294901762, 4294901760, 4294901776, 4294901778, 4294901777, 4294901766, 4294901767, 4294901779, 4294901764, 4294901765, 4294901763, 4399106, 4399107, 4399110, 4399111, 6, 4399111, 4901779, 4294901779, 01322020

            int RandomNumber = (rand() % (21 - 1)) + 1;
            if (RandomNumber == 1)
                MapGameType = 01322020;
            if (RandomNumber == 2)
                MapGameType = 4294901762;
            if (RandomNumber == 2)
                MapGameType = 4294901760;
            if (RandomNumber == 4)
                MapGameType = 4294901776;
            if (RandomNumber == 5)
                MapGameType = 4294901778;
            if (RandomNumber == 6)
                MapGameType = 4294901777;
            if (RandomNumber == 7)
                MapGameType = 4294901766;
            if (RandomNumber == 8)
                MapGameType = 4294901767;
            if (RandomNumber == 9)
                MapGameType = 4294901779;
            if (RandomNumber == 10)
                MapGameType = 4294901762;
            if (RandomNumber == 11)
                MapGameType = 4294901764;
            if (RandomNumber == 12)
                MapGameType = 4294901765;
            if (RandomNumber == 13)
                MapGameType = 4294901763;
            if (RandomNumber == 14)
                MapGameType = 4399106;
            if (RandomNumber == 15)
                MapGameType = 4399107;
            if (RandomNumber == 16)
                MapGameType = 4399110;
            if (RandomNumber == 17)
                MapGameType = 4399111;
            if (RandomNumber == 18)
                MapGameType = 6;
            if (RandomNumber == 19)
                MapGameType = 4399111;
            if (RandomNumber == 20)
                MapGameType = 4901779;
            if (RandomNumber == 21)
                MapGameType = 4294901779;

            //CONSOLE_Print("[INFO] Using now MapGameType: "+UTIL_ToString(MapGameType));

            if (m_OHBot->HasMode( MODE_GPROXY))
                m_OutPackets.push(m_Protocol->SEND_SID_STARTADVEX3(state, UTIL_CreateByteArray(MapGameType, false), map->GetMapGameFlags(), MapWidth, MapHeight,
                                                                   gameName, hostName, upTime, map->GetMapPath(), map->GetMapCRC(), map->GetMapSHA1(), FixedHostCounter));
            else
                m_OutPackets.push(m_Protocol->SEND_SID_STARTADVEX3(state, UTIL_CreateByteArray(MapGameType, false), map->GetMapGameFlags(), map->GetMapWidth(),
                                                                   map->GetMapHeight(), gameName, hostName, upTime, map->GetMapPath(), map->GetMapCRC(),
                                                                   map->GetMapSHA1(), FixedHostCounter));
        }
    }
}

void CBNET::QueueGameUncreate() {
    if (m_LoggedIn)
        m_OutPackets.push(m_Protocol->SEND_SID_STOPADV());
}

void CBNET::UnqueuePackets(unsigned char type) {
    queue<BYTEARRAY> Packets;
    uint32_t Unqueued = 0;

    while (!m_OutPackets.empty()) {
        // todotodo: it's very inefficient to have to copy all these packets while searching the queue

        BYTEARRAY Packet = m_OutPackets.front();
        m_OutPackets.pop();

        if (Packet.size() >= 2 && Packet[1] == type)
            ++Unqueued;
        else
            Packets.push(Packet);
    }

    m_OutPackets = Packets;

    if (Unqueued > 0)
        CONSOLE_Print("[BNET: " + m_ServerAlias + "] unqueued " + UTIL_ToString(Unqueued) + " packets of type " +
                      UTIL_ToString(type));
}

void CBNET::UnqueueChatCommand(string chatCommand) {
    // hackhack: this is ugly code
    // generate the packet that would be sent for this chat command
    // then search the queue for that exact packet

    BYTEARRAY PacketToUnqueue = m_Protocol->SEND_SID_CHATCOMMAND(chatCommand);
    queue<BYTEARRAY> Packets;
    uint32_t Unqueued = 0;

    while (!m_OutPackets.empty()) {
        // todotodo: it's very inefficient to have to copy all these packets while searching the queue

        BYTEARRAY Packet = m_OutPackets.front();
        m_OutPackets.pop();

        if (Packet == PacketToUnqueue)
            ++Unqueued;
        else
            Packets.push(Packet);
    }

    m_OutPackets = Packets;

    if (Unqueued > 0)
        CONSOLE_Print("[BNET: " + m_ServerAlias + "] unqueued " + UTIL_ToString(Unqueued) + " chat command packets");
}

void CBNET::UnqueueGameRefreshes() {
    UnqueuePackets(CBNETProtocol::SID_STARTADVEX3);
}

uint32_t CBNET::IsLevel(string name) {
    transform(name.begin(), name.end(), name.begin(), ::tolower);

    for (vector<permission>::iterator i = m_Permissions.begin(); i != m_Permissions.end(); ++i) {
        if (i->player == name)
            return i->level;
    }

    return 0;
}

string CBNET::GetLevelName(uint32_t level) {

    if (level < 0 || level > 10)
        return m_OHBot->m_Language->Unknown();
    else
        return m_OHBot->m_Ranks[level];
}

CDBBan *CBNET::IsBannedName(string name) {
    transform(name.begin(), name.end(), name.begin(), ::tolower);

    // todotodo: optimize this - maybe use a map?

    for (vector<CDBBan *>::iterator i = m_Bans.begin(); i != m_Bans.end(); ++i) {
        if ((*i)->GetName() == name)
            return *i;
    }

    return NULL;
}

CDBBan *CBNET::IsBannedIP(string ip) {
    transform(ip.begin(), ip.end(), ip.begin(), ::tolower); //transform in case it's a hostname
    for (vector<CDBBan *>::iterator i = m_Bans.begin(); i != m_Bans.end(); ++i) {
        if ((*i)->GetIP()[0] == ':') {
            string BanIP = (*i)->GetIP().substr(1);
            size_t len = BanIP.length();

            if (ip.length() >= len && ip.substr(0, len) == BanIP) {
                return *i;
            }
            else if (BanIP.length() >= 3 && BanIP[0] == 'h' && ip.length() >= 3 && ip[0] == 'h' &&
                     ip.substr(1).find(BanIP.substr(1)) != string::npos) {

                return *i;
            }
        }

        if ((*i)->GetIP() == ip) {
            return *i;
        }
    }

    return NULL;
}

void CBNET::AddBan(string name, string ip, string gamename, string admin, string reason) {
    transform(name.begin(), name.end(), name.begin(), ::tolower);
    m_Bans.push_back(
            new CDBBan(0, m_Server, name, ip, "NA", gamename, admin, reason, string(), string(), string(), string(),
                       string(), 0));
}

void CBNET::RemoveBan(string name) {
    transform(name.begin(), name.end(), name.begin(), ::tolower);

    for (vector<CDBBan *>::iterator i = m_Bans.begin(); i != m_Bans.end();) {
        if ((*i)->GetName() == name)
            i = m_Bans.erase(i);
        else
            ++i;
    }
}

void CBNET::HoldFriends(CBaseGame *game) {
    if (game) {
        for (vector<CIncomingFriendList *>::iterator i = m_Friends.begin(); i != m_Friends.end(); ++i)
            game->AddToReserved((*i)->GetAccount(), 255, 1);
    }
}

void CBNET::HoldClan(CBaseGame *game) {
    if (game) {
        for (vector<CIncomingClanList *>::iterator i = m_Clans.begin(); i != m_Clans.end(); ++i)
            game->AddToReserved((*i)->GetName(), 255, 1);
    }
}

void CBNET::PVPGNCommand(string Command) {
    string cmd;
    string args;
    string::size_type argss = Command.find(" ");

    if (argss != string::npos) {
        cmd = Command.substr(1, argss - 1);
        args = Command.substr(argss + 1);
    }
    else
        cmd = Command.substr(1);

    transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

    if (cmd == "init") {
        m_OHBot->m_PVPGNMode = !m_OHBot->m_PVPGNMode;
        return;
    }
    string usr;
    stringstream SS;
    SS << args;
    SS >> usr;
    SS >> args;

    if (cmd == "host" || cmd == "chost") { }
    else if (cmd == "unhost") {
    }
    else if (cmd == "ping") { }
    else if (cmd == "swap") { }
    else if (cmd == "open") { }
    else if (cmd == "close") { }
    else if (cmd == "start") { }
    else if (cmd == "abort" || cmd == "a") { }
    else if (cmd == "pub") { } // host? or chost?
    else if (cmd == "priv") { }
}