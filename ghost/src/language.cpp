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

#include "ohbot.h"
#include "utils/util.h"
#include "config.h"
#include "language.h"

//
// CLanguage
//

CLanguage :: CLanguage( string nCFGFile )
{
    m_CFG = new CConfig( );
    m_CFG->Read( nCFGFile );
}

CLanguage :: ~CLanguage( )
{
    delete m_CFG;
}


string CLanguage :: GetMonthInWords( string month ) {
    if(month=="1")
        return "January";
    else if(month=="2")
        return "February";
    else if(month=="3")
        return "March";
    else if(month=="4")
        return "April";
    else if(month=="5")
        return "May";
    else if(month=="6")
        return "June";
    else if(month=="7")
        return "July";
    else if(month=="8")
        return "August";
    else if(month=="9")
        return "September";
    else if(month=="10")
        return "October";
    else if(month=="11")
        return "November";
    else if(month=="12")
        return "December";
    else
        return "unknown";
}

string CLanguage :: UnableToCreateGameTryAnotherName( string server, string gamename )
{
    string Out = m_CFG->GetString( "lang_0001", "lang_0001" );
    UTIL_Replace( Out, "$SERVER$", server );
    UTIL_Replace( Out, "$GAMENAME$", gamename );
    return Out;
}

string CLanguage :: UserIsAlreadyAnAdmin( string server, string user )
{
    string Out = m_CFG->GetString( "lang_0002", "lang_0002" );
    UTIL_Replace( Out, "$SERVER$", server );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: AddedUserToAdminDatabase( string server, string user )
{
    string Out = m_CFG->GetString( "lang_0003", "lang_0003" );
    UTIL_Replace( Out, "$SERVER$", server );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: ErrorAddingUserToAdminDatabase( string server, string user )
{
    string Out = m_CFG->GetString( "lang_0004", "lang_0004" );
    UTIL_Replace( Out, "$SERVER$", server );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: YouDontHaveAccessToThatCommand( )
{
    return m_CFG->GetString( "lang_0005", "lang_0005" );
}

string CLanguage :: UserIsAlreadyBanned( string server, string victim )
{
    string Out = m_CFG->GetString( "lang_0006", "lang_0006" );
    UTIL_Replace( Out, "$SERVER$", server );
    UTIL_Replace( Out, "$VICTIM$", victim );
    return Out;
}

string CLanguage :: BannedUser( string server, string victim )
{
    string Out = m_CFG->GetString( "lang_0007", "lang_0007" );
    UTIL_Replace( Out, "$SERVER$", server );
    UTIL_Replace( Out, "$VICTIM$", victim );
    return Out;
}

string CLanguage :: ErrorBanningUser( string server, string victim )
{
    string Out = m_CFG->GetString( "lang_0008", "lang_0008" );
    UTIL_Replace( Out, "$SERVER$", server );
    UTIL_Replace( Out, "$VICTIM$", victim );
    return Out;
}

string CLanguage :: UserIsAnAdmin( string server, string user )
{
    string Out = m_CFG->GetString( "lang_0009", "lang_0009" );
    UTIL_Replace( Out, "$SERVER$", server );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: UserIsNotAnAdmin( string server, string user )
{
    string Out = m_CFG->GetString( "lang_0010", "lang_0010" );
    UTIL_Replace( Out, "$SERVER$", server );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: UserWasBannedOnByBecause( string server, string victim, string date, string admin, string reason, string expire, string remain )
{
    string Out = m_CFG->GetString( "lang_0011", "lang_0011" );
    UTIL_Replace( Out, "$SERVER$", server );
    UTIL_Replace( Out, "$VICTIM$", victim );
    UTIL_Replace( Out, "$DATE$", date );
    UTIL_Replace( Out, "$ADMIN$", admin );
    UTIL_Replace( Out, "$REASON$", reason );
    UTIL_Replace( Out, "$EXPIRE$", expire );
    UTIL_Replace( Out, "$REMAIN$", remain );
    return Out;
}

string CLanguage :: UserIsNotBanned( string server, string victim )
{
    string Out = m_CFG->GetString( "lang_0012", "lang_0012" );
    UTIL_Replace( Out, "$SERVER$", server );
    UTIL_Replace( Out, "$VICTIM$", victim );
    return Out;
}

string CLanguage :: ThereAreNoAdmins( string server )
{
    string Out = m_CFG->GetString( "lang_0013", "lang_0013" );
    UTIL_Replace( Out, "$SERVER$", server );
    return Out;
}

string CLanguage :: ThereIsAdmin( string server )
{
    string Out = m_CFG->GetString( "lang_0014", "lang_0014" );
    UTIL_Replace( Out, "$SERVER$", server );
    return Out;
}

string CLanguage :: ThereAreAdmins( string server, string count )
{
    string Out = m_CFG->GetString( "lang_0015", "lang_0015" );
    UTIL_Replace( Out, "$SERVER$", server );
    UTIL_Replace( Out, "$COUNT$", count );
    return Out;
}

string CLanguage :: ThereAreNoBannedUsers( string server )
{
    string Out = m_CFG->GetString( "lang_0016", "lang_0016" );
    UTIL_Replace( Out, "$SERVER$", server );
    return Out;
}

string CLanguage :: ThereIsBannedUser( string server )
{
    string Out = m_CFG->GetString( "lang_0017", "lang_0017" );
    UTIL_Replace( Out, "$SERVER$", server );
    return Out;
}

string CLanguage :: ThereAreBannedUsers( string server, string count )
{
    string Out = m_CFG->GetString( "lang_0018", "lang_0018" );
    UTIL_Replace( Out, "$SERVER$", server );
    UTIL_Replace( Out, "$COUNT$", count );
    return Out;
}

string CLanguage :: YouCantDeleteTheRootAdmin( )
{
    return m_CFG->GetString( "lang_0019", "lang_0019" );
}

string CLanguage :: DeletedUserFromAdminDatabase( string server, string user )
{
    string Out = m_CFG->GetString( "lang_0020", "lang_0020" );
    UTIL_Replace( Out, "$SERVER$", server );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: ErrorDeletingUserFromAdminDatabase( string server, string user )
{
    string Out = m_CFG->GetString( "lang_0021", "lang_0021" );
    UTIL_Replace( Out, "$SERVER$", server );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: UnbannedUser( string victim )
{
    string Out = m_CFG->GetString( "lang_0022", "lang_0022" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    return Out;
}

string CLanguage :: ErrorUnbanningUser( string victim )
{
    string Out = m_CFG->GetString( "lang_0023", "lang_0023" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    return Out;
}

string CLanguage :: GameNumberIs( string number, string description )
{
    string Out = m_CFG->GetString( "lang_0024", "lang_0024" );
    UTIL_Replace( Out, "$NUMBER$", number );
    UTIL_Replace( Out, "$DESCRIPTION$", description );
    return Out;
}

string CLanguage :: GameNumberDoesntExist( string number )
{
    string Out = m_CFG->GetString( "lang_0025", "lang_0025" );
    UTIL_Replace( Out, "$NUMBER$", number );
    return Out;
}

string CLanguage :: GameIsInTheLobby( string description, string current, string max )
{
    string Out = m_CFG->GetString( "lang_0026", "lang_0026" );
    UTIL_Replace( Out, "$DESCRIPTION$", description );
    UTIL_Replace( Out, "$CURRENT$", current );
    UTIL_Replace( Out, "$MAX$", max );
    return Out;
}

string CLanguage :: ThereIsNoGameInTheLobby( string current, string max )
{
    string Out = m_CFG->GetString( "lang_0027", "lang_0027" );
    UTIL_Replace( Out, "$CURRENT$", current );
    UTIL_Replace( Out, "$MAX$", max );
    return Out;
}

string CLanguage :: UnableToLoadConfigFilesOutside( )
{
    return m_CFG->GetString( "lang_0028", "lang_0028" );
}

string CLanguage :: LoadingConfigFile( string file )
{
    string Out = m_CFG->GetString( "lang_0029", "lang_0029" );
    UTIL_Replace( Out, "$FILE$", file );
    return Out;
}

string CLanguage :: UnableToLoadConfigFileDoesntExist( string file )
{
    string Out = m_CFG->GetString( "lang_0030", "lang_0030" );
    UTIL_Replace( Out, "$FILE$", file );
    return Out;
}

string CLanguage :: CreatingPrivateGame( string gamename, string user )
{
    string Out = m_CFG->GetString( "lang_0031", "lang_0031" );
    UTIL_Replace( Out, "$GAMENAME$", gamename );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: CreatingPublicGame( string gamename, string user )
{
    string Out = m_CFG->GetString( "lang_0032", "lang_0032" );
    UTIL_Replace( Out, "$GAMENAME$", gamename );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: UnableToUnhostGameCountdownStarted( string description )
{
    string Out = m_CFG->GetString( "lang_0033", "lang_0033" );
    UTIL_Replace( Out, "$DESCRIPTION$", description );
    return Out;
}

string CLanguage :: UnhostingGame( string description )
{
    string Out = m_CFG->GetString( "lang_0034", "lang_0034" );
    UTIL_Replace( Out, "$DESCRIPTION$", description );
    return Out;
}

string CLanguage :: UnableToUnhostGameNoGameInLobby( )
{
    return m_CFG->GetString( "lang_0035", "lang_0035" );
}

string CLanguage :: VersionAdmin( string version )
{
    string Out = m_CFG->GetString( "lang_0036", "lang_0036" );
    UTIL_Replace( Out, "$VERSION$", version );
    return Out;
}

string CLanguage :: VersionNotAdmin( string version )
{
    string Out = m_CFG->GetString( "lang_0037", "lang_0037" );
    UTIL_Replace( Out, "$VERSION$", version );
    return Out;
}

string CLanguage :: UnableToCreateGameAnotherGameInLobby( string gamename, string description )
{
    string Out = m_CFG->GetString( "lang_0038", "lang_0038" );
    UTIL_Replace( Out, "$GAMENAME$", gamename );
    UTIL_Replace( Out, "$DESCRIPTION$", description );
    return Out;
}

string CLanguage :: UnableToCreateGameMaxGamesReached( string gamename, string max )
{
    string Out = m_CFG->GetString( "lang_0039", "lang_0039" );
    UTIL_Replace( Out, "$GAMENAME$", gamename );
    UTIL_Replace( Out, "$MAX$", max );
    return Out;
}

string CLanguage :: GameIsOver( string description )
{
    string Out = m_CFG->GetString( "lang_0040", "lang_0040" );
    UTIL_Replace( Out, "$DESCRIPTION$", description );
    return Out;
}

string CLanguage :: SpoofCheckByReplying( )
{
    return m_CFG->GetString( "lang_0041", "lang_0041" );
}

string CLanguage :: GameRefreshed( )
{
    return m_CFG->GetString( "lang_0042", "lang_0042" );
}

string CLanguage :: SpoofPossibleIsAway( string user )
{
    string Out = m_CFG->GetString( "lang_0043", "lang_0043" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: SpoofPossibleIsUnavailable( string user )
{
    string Out = m_CFG->GetString( "lang_0044", "lang_0044" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: SpoofPossibleIsRefusingMessages( string user )
{
    string Out = m_CFG->GetString( "lang_0045", "lang_0045" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: SpoofDetectedIsNotInGame( string user )
{
    string Out = m_CFG->GetString( "lang_0046", "lang_0046" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: SpoofDetectedIsInPrivateChannel( string user )
{
    string Out = m_CFG->GetString( "lang_0047", "lang_0047" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: SpoofDetectedIsInAnotherGame( string user )
{
    string Out = m_CFG->GetString( "lang_0048", "lang_0048" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: CountDownAborted( )
{
    return m_CFG->GetString( "lang_0049", "lang_0049" );
}

string CLanguage :: TryingToJoinTheGameButBanned( string victim )
{
    string Out = m_CFG->GetString( "lang_0050", "lang_0050" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    return Out;
}

string CLanguage :: UnableToBanNoMatchesFound( string victim )
{
    string Out = m_CFG->GetString( "lang_0051", "lang_0051" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    return Out;
}

string CLanguage :: PlayerWasBannedByPlayer( string server, string victim, string user )
{
    string Out = m_CFG->GetString( "lang_0052", "lang_0052" );
    UTIL_Replace( Out, "$SERVER$", server );
    UTIL_Replace( Out, "$VICTIM$", victim );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: UnableToBanFoundMoreThanOneMatch( string victim )
{
    string Out = m_CFG->GetString( "lang_0053", "lang_0053" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    return Out;
}

string CLanguage :: AddedPlayerToTheHoldList( string user )
{
    string Out = m_CFG->GetString( "lang_0054", "lang_0054" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: UnableToKickNoMatchesFound( string victim )
{
    string Out = m_CFG->GetString( "lang_0055", "lang_0055" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    return Out;
}

string CLanguage :: UnableToKickFoundMoreThanOneMatch( string victim )
{
    string Out = m_CFG->GetString( "lang_0056", "lang_0056" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    return Out;
}

string CLanguage :: SettingLatencyToMinimum( string min )
{
    string Out = m_CFG->GetString( "lang_0057", "lang_0057" );
    UTIL_Replace( Out, "$MIN$", min );
    return Out;
}

string CLanguage :: SettingLatencyToMaximum( string max )
{
    string Out = m_CFG->GetString( "lang_0058", "lang_0058" );
    UTIL_Replace( Out, "$MAX$", max );
    return Out;
}

string CLanguage :: SettingLatencyTo( string latency )
{
    string Out = m_CFG->GetString( "lang_0059", "lang_0059" );
    UTIL_Replace( Out, "$LATENCY$", latency );
    return Out;
}

string CLanguage :: KickingPlayersWithPingsGreaterThan( string total, string ping )
{
    string Out = m_CFG->GetString( "lang_0060", "lang_0060" );
    UTIL_Replace( Out, "$TOTAL$", total );
    UTIL_Replace( Out, "$PING$", ping );
    return Out;
}

string CLanguage :: HasPlayedGamesWithThisBot( string user, string score, string games, string winperc, string streak, string month, string year )
{
    string Out = m_CFG->GetString( "lang_0061", "lang_0061" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$SCORE$", score );
    UTIL_Replace( Out, "$GAMES$", games );
    UTIL_Replace( Out, "$WINPERC$", winperc );
    UTIL_Replace( Out, "$STREAK$", streak );
    if( month == "unknown" && year == "0")
        UTIL_Replace( Out, "$TIME$", "overall");
    else
        UTIL_Replace( Out, "$TIME$", month+", "+year);
    return Out;
}

string CLanguage :: HasntPlayedGamesWithThisBot( string user, string month, string year )
{
    string Out = m_CFG->GetString( "lang_0062", "lang_0062" );
    UTIL_Replace( Out, "$USER$", user );
    if( month == "unknown" && year == "0")
        UTIL_Replace( Out, "$TIME$", "overall");
    else
        UTIL_Replace( Out, "$TIME$", month+", "+year);
    return Out;
}

string CLanguage :: AutokickingPlayerForExcessivePing( string victim, string ping )
{
    string Out = m_CFG->GetString( "lang_0063", "lang_0063" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    UTIL_Replace( Out, "$PING$", ping );
    return Out;
}

string CLanguage :: SpoofCheckAcceptedFor( string server, string user )
{
    string Out = m_CFG->GetString( "lang_0064", "lang_0064" );
    UTIL_Replace( Out, "$SERVER$", server );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: PlayersNotYetSpoofChecked( string notspoofchecked )
{
    string Out = m_CFG->GetString( "lang_0065", "lang_0065" );
    UTIL_Replace( Out, "$NOTSPOOFCHECKED$", notspoofchecked );
    return Out;
}

string CLanguage :: ManuallySpoofCheckByWhispering( string hostname )
{
    string Out = m_CFG->GetString( "lang_0066", "lang_0066" );
    UTIL_Replace( Out, "$HOSTNAME$", hostname );
    return Out;
}

string CLanguage :: SpoofCheckByWhispering( string hostname )
{
    string Out = m_CFG->GetString( "lang_0067", "lang_0067" );
    UTIL_Replace( Out, "$HOSTNAME$", hostname );
    return Out;
}

string CLanguage :: EveryoneHasBeenSpoofChecked( )
{
    return m_CFG->GetString( "lang_0068", "lang_0068" );
}

string CLanguage :: PlayersNotYetPinged( string notpinged )
{
    string Out = m_CFG->GetString( "lang_0069", "lang_0069" );
    UTIL_Replace( Out, "$NOTPINGED$", notpinged );
    return Out;
}

string CLanguage :: EveryoneHasBeenPinged( )
{
    return m_CFG->GetString( "lang_0070", "lang_0070" );
}

string CLanguage :: ShortestLoadByPlayer( string user, string loadingtime )
{
    string Out = m_CFG->GetString( "lang_0071", "lang_0071" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$LOADINGTIME$", loadingtime );
    return Out;
}

string CLanguage :: LongestLoadByPlayer( string user, string loadingtime )
{
    string Out = m_CFG->GetString( "lang_0072", "lang_0072" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$LOADINGTIME$", loadingtime );
    return Out;
}

string CLanguage :: YourLoadingTimeWas( string loadingtime )
{
    string Out = m_CFG->GetString( "lang_0073", "lang_0073" );
    UTIL_Replace( Out, "$LOADINGTIME$", loadingtime );
    return Out;
}

string CLanguage :: HasPlayedAliasGamesWithThisBot( string totalgames, string totalwins, string totallosses, string draws, string totalkills, string totaldeaths, string totalcreepkills, string totalcreepdenies, string totalassists, string totalneutralkills, string totaltowerkills, string totalraxkills, string avgkills, string avgdeaths, string avgcreepkills, string avgcreepdenies, string avgassists, string avgneutralkills, string avgtowerkills, string avgraxkills)
{
    string Out = m_CFG->GetString( "lang_0074", "lang_0074" );
    UTIL_Replace( Out, "$TOTALGAMES$", totalgames );
    UTIL_Replace( Out, "$TOTALWINS$", totalwins );
    UTIL_Replace( Out, "$TOTALLOSSES$", totallosses );
    UTIL_Replace( Out, "$TOTALDRAWS$", draws );
    UTIL_Replace( Out, "$TOTALKILLS$", totalkills );
    UTIL_Replace( Out, "$TOTALDEATHS$", totaldeaths );
    UTIL_Replace( Out, "$TOTALCREEPKILLS$", totalcreepkills );
    UTIL_Replace( Out, "$TOTALCREEPDENIES$", totalcreepdenies );
    UTIL_Replace( Out, "$TOTALASSISTS$", totalassists );
    UTIL_Replace( Out, "$TOTALNEUTRALKILLS$", totalneutralkills );
    UTIL_Replace( Out, "$TOTALTOWERKILLS$", totaltowerkills );
    UTIL_Replace( Out, "$TOTALRAXKILLS$", totalraxkills );
    UTIL_Replace( Out, "$AVGKILLS$", avgkills );
    UTIL_Replace( Out, "$AVGDEATHS$", avgdeaths );
    UTIL_Replace( Out, "$AVGCREEPKILLS$", avgcreepkills );
    UTIL_Replace( Out, "$AVGCREEPDENIES$", avgcreepdenies );
    UTIL_Replace( Out, "$AVGASSISTS$", avgassists );
    UTIL_Replace( Out, "$AVGNEUTRALKILLS$", avgneutralkills );
    UTIL_Replace( Out, "$AVGTOWERKILLS$", avgtowerkills );
    UTIL_Replace( Out, "$AVGRAXKILLS$", avgraxkills );
    return Out;
}

/**
 * for bnet only
*/
string CLanguage :: HasPlayedAliasGamesWithThisBot2( string user, string totalgames, string totalwins, string totallosses, string draws, string totalkills, string totaldeaths, string totalcreepkills, string totalcreepdenies, string totalassists, string totalneutralkills, string totaltowerkills, string totalraxkills, string avgkills, string avgdeaths, string avgcreepkills, string avgcreepdenies, string avgassists, string avgneutralkills, string avgtowerkills, string avgraxkills, string month, string year, string alias)
{
    string Out = m_CFG->GetString( "lang_0074a", "lang_0074a" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$GAMEALIAS$", alias);
    UTIL_Replace( Out, "$TOTALGAMES$", totalgames );
    UTIL_Replace( Out, "$TOTALGAMES$", totalgames );
    UTIL_Replace( Out, "$TOTALWINS$", totalwins );
    UTIL_Replace( Out, "$TOTALLOSSES$", totallosses );
    UTIL_Replace( Out, "$TOTALDRAWS$", draws );
    UTIL_Replace( Out, "$TOTALKILLS$", totalkills );
    UTIL_Replace( Out, "$TOTALDEATHS$", totaldeaths );
    UTIL_Replace( Out, "$TOTALCREEPKILLS$", totalcreepkills );
    UTIL_Replace( Out, "$TOTALCREEPDENIES$", totalcreepdenies );
    UTIL_Replace( Out, "$TOTALASSISTS$", totalassists );
    UTIL_Replace( Out, "$TOTALNEUTRALKILLS$", totalneutralkills );
    UTIL_Replace( Out, "$TOTALTOWERKILLS$", totaltowerkills );
    UTIL_Replace( Out, "$TOTALRAXKILLS$", totalraxkills );
    UTIL_Replace( Out, "$AVGKILLS$", avgkills );
    UTIL_Replace( Out, "$AVGDEATHS$", avgdeaths );
    UTIL_Replace( Out, "$AVGCREEPKILLS$", avgcreepkills );
    UTIL_Replace( Out, "$AVGCREEPDENIES$", avgcreepdenies );
    UTIL_Replace( Out, "$AVGASSISTS$", avgassists );
    UTIL_Replace( Out, "$AVGNEUTRALKILLS$", avgneutralkills );
    UTIL_Replace( Out, "$AVGTOWERKILLS$", avgtowerkills );
    UTIL_Replace( Out, "$AVGRAXKILLS$", avgraxkills );
    if( month == "unknown" && year == "0")
        UTIL_Replace( Out, "$TIME$", "overall");
    else
        UTIL_Replace( Out, "$TIME$", month+", "+year);
    return Out;
}

string CLanguage :: HasntPlayedAliasGamesWithThisBot( string user, string month, string year, string gamealias )
{
    string Out = m_CFG->GetString( "lang_0075", "lang_0075" );
    UTIL_Replace( Out, "$GAMEALIAS$", gamealias );
    UTIL_Replace( Out, "$USER$", user );
    if( month == "unknown" && year == "0")
        UTIL_Replace( Out, "$TIME$", "overall");
    else
        UTIL_Replace( Out, "$TIME$", month+", "+year);
    return Out;
}

string CLanguage :: WasKickedForReservedPlayer( string reserved )
{
    string Out = m_CFG->GetString( "lang_0076", "lang_0076" );
    UTIL_Replace( Out, "$RESERVED$", reserved );
    return Out;
}

string CLanguage :: WasKickedForOwnerPlayer( string owner )
{
    string Out = m_CFG->GetString( "lang_0077", "lang_0077" );
    UTIL_Replace( Out, "$OWNER$", owner );
    return Out;
}

string CLanguage :: WasKickedByPlayer( string user )
{
    string Out = m_CFG->GetString( "lang_0078", "lang_0078" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: HasLostConnectionPlayerError( string error )
{
    string Out = m_CFG->GetString( "lang_0079", "lang_0079" );
    UTIL_Replace( Out, "$ERROR$", error );
    return Out;
}

string CLanguage :: HasLostConnectionSocketError( string error )
{
    string Out = m_CFG->GetString( "lang_0080", "lang_0080" );
    UTIL_Replace( Out, "$ERROR$", error );
    return Out;
}

string CLanguage :: HasLostConnectionClosedByRemoteHost( )
{
    return m_CFG->GetString( "lang_0081", "lang_0081" );
}

string CLanguage :: HasLeftVoluntarily( )
{
    return m_CFG->GetString( "lang_0082", "lang_0082" );
}

string CLanguage :: EndingGame( string description )
{
    string Out = m_CFG->GetString( "lang_0083", "lang_0083" );
    UTIL_Replace( Out, "$DESCRIPTION$", description );
    return Out;
}

string CLanguage :: HasLostConnectionTimedOut( )
{
    return m_CFG->GetString( "lang_0084", "lang_0084" );
}

string CLanguage :: GlobalChatMuted( )
{
    return m_CFG->GetString( "lang_0085", "lang_0085" );
}

string CLanguage :: GlobalChatUnmuted( )
{
    return m_CFG->GetString( "lang_0086", "lang_0086" );
}

string CLanguage :: ShufflingPlayers( )
{
    return m_CFG->GetString( "lang_0087", "lang_0087" );
}

string CLanguage :: UnableToLoadConfigFileGameInLobby( )
{
    return m_CFG->GetString( "lang_0088", "lang_0088" );
}

string CLanguage :: PlayersStillDownloading( string stilldownloading )
{
    string Out = m_CFG->GetString( "lang_0089", "lang_0089" );
    UTIL_Replace( Out, "$STILLDOWNLOADING$", stilldownloading );
    return Out;
}

string CLanguage :: RefreshMessagesEnabled( )
{
    return m_CFG->GetString( "lang_0090", "lang_0090" );
}

string CLanguage :: RefreshMessagesDisabled( )
{
    return m_CFG->GetString( "lang_0091", "lang_0091" );
}

string CLanguage :: AtLeastOneGameActiveUseForceToShutdown( )
{
    return m_CFG->GetString( "lang_0092", "lang_0092" );
}

string CLanguage :: CurrentlyLoadedMapCFGIs( string mapcfg )
{
    string Out = m_CFG->GetString( "lang_0093", "lang_0093" );
    UTIL_Replace( Out, "$MAPCFG$", mapcfg );
    return Out;
}

string CLanguage :: LaggedOutDroppedByAdmin( )
{
    return m_CFG->GetString( "lang_0094", "lang_0094" );
}

string CLanguage :: LaggedOutDroppedByVote( )
{
    return m_CFG->GetString( "lang_0095", "lang_0095" );
}

string CLanguage :: PlayerVotedToDropLaggers( string user )
{
    string Out = m_CFG->GetString( "lang_0096", "lang_0096" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: LatencyIs( string latency )
{
    string Out = m_CFG->GetString( "lang_0097", "lang_0097" );
    UTIL_Replace( Out, "$LATENCY$", latency );
    return Out;
}

string CLanguage :: SyncLimitIs( string synclimit )
{
    string Out = m_CFG->GetString( "lang_0098", "lang_0098" );
    UTIL_Replace( Out, "$SYNCLIMIT$", synclimit );
    return Out;
}

string CLanguage :: SettingSyncLimitToMinimum( string min )
{
    string Out = m_CFG->GetString( "lang_0099", "lang_0099" );
    UTIL_Replace( Out, "$MIN$", min );
    return Out;
}

string CLanguage :: SettingSyncLimitToMaximum( string max )
{
    string Out = m_CFG->GetString( "lang_0100", "lang_0100" );
    UTIL_Replace( Out, "$MAX$", max );
    return Out;
}

string CLanguage :: SettingSyncLimitTo( string synclimit )
{
    string Out = m_CFG->GetString( "lang_0101", "lang_0101" );
    UTIL_Replace( Out, "$SYNCLIMIT$", synclimit );
    return Out;
}

string CLanguage :: UnableToCreateGameNotLoggedIn( string gamename )
{
    string Out = m_CFG->GetString( "lang_0102", "lang_0102" );
    UTIL_Replace( Out, "$GAMENAME$", gamename );
    return Out;
}

string CLanguage :: AdminLoggedIn( )
{
    return m_CFG->GetString( "lang_0103", "lang_0103" );
}

string CLanguage :: AdminInvalidPassword( string attempt )
{
    string Out = m_CFG->GetString( "lang_0104", "lang_0104" );
    UTIL_Replace( Out, "$ATTEMPT$", attempt );
    return Out;
}

string CLanguage :: ConnectingToBNET( string server )
{
    string Out = m_CFG->GetString( "lang_0105", "lang_0105" );
    UTIL_Replace( Out, "$SERVER$", server );
    return Out;
}

string CLanguage :: ConnectedToBNET( string server )
{
    string Out = m_CFG->GetString( "lang_0106", "lang_0106" );
    UTIL_Replace( Out, "$SERVER$", server );
    return Out;
}

string CLanguage :: DisconnectedFromBNET( string server )
{
    string Out = m_CFG->GetString( "lang_0107", "lang_0107" );
    UTIL_Replace( Out, "$SERVER$", server );
    return Out;
}

string CLanguage :: LoggedInToBNET( string server )
{
    string Out = m_CFG->GetString( "lang_0108", "lang_0108" );
    UTIL_Replace( Out, "$SERVER$", server );
    return Out;
}

string CLanguage :: BNETGameHostingSucceeded( string server )
{
    string Out = m_CFG->GetString( "lang_0109", "lang_0109" );
    UTIL_Replace( Out, "$SERVER$", server );
    return Out;
}

string CLanguage :: BNETGameHostingFailed( string server, string gamename )
{
    string Out = m_CFG->GetString( "lang_0110", "lang_0110" );
    UTIL_Replace( Out, "$SERVER$", server );
    UTIL_Replace( Out, "$GAMENAME$", gamename );
    return Out;
}

string CLanguage :: ConnectingToBNETTimedOut( string server )
{
    string Out = m_CFG->GetString( "lang_0111", "lang_0111" );
    UTIL_Replace( Out, "$SERVER$", server );
    return Out;
}

string CLanguage :: PlayerDownloadedTheMap( string user, string seconds, string rate )
{
    string Out = m_CFG->GetString( "lang_0112", "lang_0112" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$SECONDS$", seconds );
    UTIL_Replace( Out, "$RATE$", rate );
    return Out;
}

string CLanguage :: UnableToCreateGameNameTooLong( string gamename )
{
    string Out = m_CFG->GetString( "lang_0113", "lang_0113" );
    UTIL_Replace( Out, "$GAMENAME$", gamename );
    return Out;
}

string CLanguage :: SettingGameOwnerTo( string owner )
{
    string Out = m_CFG->GetString( "lang_0114", "lang_0114" );
    UTIL_Replace( Out, "$OWNER$", owner );
    return Out;
}

string CLanguage :: TheGameIsLocked( )
{
    return m_CFG->GetString( "lang_0115", "lang_0115" );
}

string CLanguage :: GameLocked( )
{
    return m_CFG->GetString( "lang_0116", "lang_0116" );
}

string CLanguage :: GameUnlocked( )
{
    return m_CFG->GetString( "lang_0117", "lang_0117" );
}

string CLanguage :: UnableToStartDownloadNoMatchesFound( string victim )
{
    string Out = m_CFG->GetString( "lang_0118", "lang_0118" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    return Out;
}

string CLanguage :: UnableToStartDownloadFoundMoreThanOneMatch( string victim )
{
    string Out = m_CFG->GetString( "lang_0119", "lang_0119" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    return Out;
}

string CLanguage :: UnableToSetGameOwner( string owner )
{
    string Out = m_CFG->GetString( "lang_0120", "lang_0120" );
    UTIL_Replace( Out, "$OWNER$", owner );
    return Out;
}

string CLanguage :: UnableToCheckPlayerNoMatchesFound( string victim )
{
    string Out = m_CFG->GetString( "lang_0121", "lang_0121" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    return Out;
}

string CLanguage :: CheckedPlayer( string victim, string ping, string from, string admin, string owner, string spoofed, string spoofedrealm, string reserved )
{
    string Out = m_CFG->GetString( "lang_0122", "lang_0122" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    UTIL_Replace( Out, "$PING$", ping );
    UTIL_Replace( Out, "$FROM$", from );
    UTIL_Replace( Out, "$ADMIN$", admin );
    UTIL_Replace( Out, "$OWNER$", owner );
    UTIL_Replace( Out, "$SPOOFED$", spoofed );
    UTIL_Replace( Out, "$SPOOFEDREALM$", spoofedrealm );
    UTIL_Replace( Out, "$RESERVED$", reserved );
    return Out;
}

string CLanguage :: UnableToCheckPlayerFoundMoreThanOneMatch( string victim )
{
    string Out = m_CFG->GetString( "lang_0123", "lang_0123" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    return Out;
}

string CLanguage :: TheGameIsLockedBNET( )
{
    return m_CFG->GetString( "lang_0124", "lang_0124" );
}

string CLanguage :: UnableToCreateGameDisabled( string gamename )
{
    string Out = m_CFG->GetString( "lang_0125", "lang_0125" );
    UTIL_Replace( Out, "$GAMENAME$", gamename );
    return Out;
}

string CLanguage :: BotDisabled( )
{
    return m_CFG->GetString( "lang_0126", "lang_0126" );
}

string CLanguage :: BotEnabled( )
{
    return m_CFG->GetString( "lang_0127", "lang_0127" );
}

string CLanguage :: UnableToCreateGameInvalidMap( string gamename )
{
    string Out = m_CFG->GetString( "lang_0128", "lang_0128" );
    UTIL_Replace( Out, "$GAMENAME$", gamename );
    return Out;
}

string CLanguage :: WaitingForPlayersBeforeAutoStart( string players, string playersleft )
{
    string Out = m_CFG->GetString( "lang_0129", "lang_0129" );
    UTIL_Replace( Out, "$PLAYERS$", players );
    UTIL_Replace( Out, "$PLAYERSLEFT$", playersleft );
    return Out;
}

string CLanguage :: AutoStartDisabled( )
{
    return m_CFG->GetString( "lang_0130", "lang_0130" );
}

string CLanguage :: AutoStartEnabled( string players )
{
    string Out = m_CFG->GetString( "lang_0131", "lang_0131" );
    UTIL_Replace( Out, "$PLAYERS$", players );
    return Out;
}

string CLanguage :: AnnounceMessageEnabled( )
{
    return m_CFG->GetString( "lang_0132", "lang_0132" );
}

string CLanguage :: AnnounceMessageDisabled( )
{
    return m_CFG->GetString( "lang_0133", "lang_0133" );
}

string CLanguage :: AutoHostEnabled( )
{
    return m_CFG->GetString( "lang_0134", "lang_0134" );
}

string CLanguage :: AutoHostDisabled( )
{
    return m_CFG->GetString( "lang_0135", "lang_0135" );
}

string CLanguage :: UnableToLoadSaveGamesOutside( )
{
    return m_CFG->GetString( "lang_0136", "lang_0136" );
}

string CLanguage :: UnableToLoadSaveGameGameInLobby( )
{
    return m_CFG->GetString( "lang_0137", "lang_0137" );
}

string CLanguage :: LoadingSaveGame( string file )
{
    string Out = m_CFG->GetString( "lang_0138", "lang_0138" );
    UTIL_Replace( Out, "$FILE$", file );
    return Out;
}

string CLanguage :: UnableToLoadSaveGameDoesntExist( string file )
{
    string Out = m_CFG->GetString( "lang_0139", "lang_0139" );
    UTIL_Replace( Out, "$FILE$", file );
    return Out;
}

string CLanguage :: UnableToCreateGameInvalidSaveGame( string gamename )
{
    string Out = m_CFG->GetString( "lang_0140", "lang_0140" );
    UTIL_Replace( Out, "$GAMENAME$", gamename );
    return Out;
}

string CLanguage :: UnableToCreateGameSaveGameMapMismatch( string gamename )
{
    string Out = m_CFG->GetString( "lang_0141", "lang_0141" );
    UTIL_Replace( Out, "$GAMENAME$", gamename );
    return Out;
}

string CLanguage :: AutoSaveEnabled( )
{
    return m_CFG->GetString( "lang_0142", "lang_0142" );
}

string CLanguage :: AutoSaveDisabled( )
{
    return m_CFG->GetString( "lang_0143", "lang_0143" );
}

string CLanguage :: DesyncDetected( )
{
    return m_CFG->GetString( "lang_0144", "lang_0144" );
}

string CLanguage :: UnableToMuteNoMatchesFound( string victim )
{
    string Out = m_CFG->GetString( "lang_0145", "lang_0145" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    return Out;
}

string CLanguage :: MutedPlayer( string victim, string user )
{
    string Out = m_CFG->GetString( "lang_0146", "lang_0146" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: UnmutedPlayer( string victim, string user )
{
    string Out = m_CFG->GetString( "lang_0147", "lang_0147" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: UnableToMuteFoundMoreThanOneMatch( string victim )
{
    string Out = m_CFG->GetString( "lang_0148", "lang_0148" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    return Out;
}

string CLanguage :: PlayerIsSavingTheGame( string player )
{
    string Out = m_CFG->GetString( "lang_0149", "lang_0149" );
    UTIL_Replace( Out, "$PLAYER$", player );
    return Out;
}

string CLanguage :: UpdatingClanList( )
{
    return m_CFG->GetString( "lang_0150", "lang_0150" );
}

string CLanguage :: UpdatingFriendsList( )
{
    return m_CFG->GetString( "lang_0151", "lang_0151" );
}

string CLanguage :: MultipleIPAddressUsageDetected( string player, string others )
{
    string Out = m_CFG->GetString( "lang_0152", "lang_0152" );
    UTIL_Replace( Out, "$PLAYER$", player );
    UTIL_Replace( Out, "$OTHERS$", others );
    return Out;
}

string CLanguage :: UnableToVoteKickAlreadyInProgress( )
{
    return m_CFG->GetString( "lang_0153", "lang_0153" );
}

string CLanguage :: UnableToVoteKickNotEnoughPlayers( )
{
    return m_CFG->GetString( "lang_0154", "lang_0154" );
}

string CLanguage :: UnableToVoteKickNoMatchesFound( string victim )
{
    string Out = m_CFG->GetString( "lang_0155", "lang_0155" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    return Out;
}

string CLanguage :: UnableToVoteKickPlayerIsReserved( string victim )
{
    string Out = m_CFG->GetString( "lang_0156", "lang_0156" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    return Out;
}

string CLanguage :: StartedVoteKick( string victim, string user, string votesneeded )
{
    string Out = m_CFG->GetString( "lang_0157", "lang_0157" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$VOTESNEEDED$", votesneeded );
    return Out;
}

string CLanguage :: UnableToVoteKickFoundMoreThanOneMatch( string victim )
{
    string Out = m_CFG->GetString( "lang_0158", "lang_0158" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    return Out;
}

string CLanguage :: VoteKickPassed( string victim )
{
    string Out = m_CFG->GetString( "lang_0159", "lang_0159" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    return Out;
}

string CLanguage :: ErrorVoteKickingPlayer( string victim )
{
    string Out = m_CFG->GetString( "lang_0160", "lang_0160" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    return Out;
}

string CLanguage :: VoteKickAcceptedNeedMoreVotes( string victim, string user, string votes )
{
    string Out = m_CFG->GetString( "lang_0161", "lang_0161" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$VOTES$", votes );
    return Out;
}

string CLanguage :: VoteKickCancelled( string victim )
{
    string Out = m_CFG->GetString( "lang_0162", "lang_0162" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    return Out;
}

string CLanguage :: VoteKickExpired( string victim )
{
    string Out = m_CFG->GetString( "lang_0163", "lang_0163" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    return Out;
}

string CLanguage :: WasKickedByVote( )
{
    return m_CFG->GetString( "lang_0164", "lang_0164" );
}

string CLanguage :: TypeYesToVote( string commandtrigger )
{
    string Out = m_CFG->GetString( "lang_0165", "lang_0165" );
    UTIL_Replace( Out, "$COMMANDTRIGGER$", commandtrigger );
    return Out;
}

string CLanguage :: PlayersNotYetPingedAutoStart( string notpinged )
{
    string Out = m_CFG->GetString( "lang_0166", "lang_0166" );
    UTIL_Replace( Out, "$NOTPINGED$", notpinged );
    return Out;
}

string CLanguage :: WasKickedForNotSpoofChecking( )
{
    return m_CFG->GetString( "lang_0167", "lang_0167" );
}

string CLanguage :: WasKickedForHavingFurthestScore( string score, string average )
{
    string Out = m_CFG->GetString( "lang_0168", "lang_0168" );
    UTIL_Replace( Out, "$SCORE$", score );
    UTIL_Replace( Out, "$AVERAGE$", average );
    return Out;
}

string CLanguage :: PlayerHasScore( string player, string score )
{
    string Out = m_CFG->GetString( "lang_0169", "lang_0169" );
    UTIL_Replace( Out, "$PLAYER$", player );
    UTIL_Replace( Out, "$SCORE$", score );
    return Out;
}

string CLanguage :: RatedPlayersSpread( string rated, string total, string spread )
{
    string Out = m_CFG->GetString( "lang_0170", "lang_0170" );
    UTIL_Replace( Out, "$RATED$", rated );
    UTIL_Replace( Out, "$TOTAL$", total );
    UTIL_Replace( Out, "$SPREAD$", spread );
    return Out;
}

string CLanguage :: ErrorListingMaps( )
{
    return m_CFG->GetString( "lang_0171", "lang_0171" );
}

string CLanguage :: FoundMaps( string maps )
{
    string Out = m_CFG->GetString( "lang_0172", "lang_0172" );
    UTIL_Replace( Out, "$MAPS$", maps );
    return Out;
}

string CLanguage :: NoMapsFound( )
{
    return m_CFG->GetString( "lang_0173", "lang_0173" );
}

string CLanguage :: ErrorListingMapConfigs( )
{
    return m_CFG->GetString( "lang_0174", "lang_0174" );
}

string CLanguage :: FoundMapConfigs( string mapconfigs )
{
    string Out = m_CFG->GetString( "lang_0175", "lang_0175" );
    UTIL_Replace( Out, "$MAPCONFIGS$", mapconfigs );
    return Out;
}

string CLanguage :: NoMapConfigsFound( )
{
    return m_CFG->GetString( "lang_0176", "lang_0176" );
}

string CLanguage :: PlayerFinishedLoading( string user )
{
    string Out = m_CFG->GetString( "lang_0177", "lang_0177" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: PleaseWaitPlayersStillLoading( )
{
    return m_CFG->GetString( "lang_0178", "lang_0178" );
}

string CLanguage :: MapDownloadsDisabled( )
{
    return m_CFG->GetString( "lang_0179", "lang_0179" );
}

string CLanguage :: MapDownloadsEnabled( )
{
    return m_CFG->GetString( "lang_0180", "lang_0180" );
}

string CLanguage :: MapDownloadsConditional( )
{
    return m_CFG->GetString( "lang_0181", "lang_0181" );
}

string CLanguage :: SettingHCL( string HCL )
{
    string Out = m_CFG->GetString( "lang_0182", "lang_0182" );
    UTIL_Replace( Out, "$HCL$", HCL );
    return Out;
}

string CLanguage :: UnableToSetHCLInvalid( )
{
    return m_CFG->GetString( "lang_0183", "lang_0183" );
}

string CLanguage :: UnableToSetHCLTooLong( )
{
    return m_CFG->GetString( "lang_0184", "lang_0184" );
}

string CLanguage :: TheHCLIs( string HCL )
{
    string Out = m_CFG->GetString( "lang_0185", "lang_0185" );
    UTIL_Replace( Out, "$HCL$", HCL );
    return Out;
}

string CLanguage :: TheHCLIsTooLongUseForceToStart( )
{
    return m_CFG->GetString( "lang_0186", "lang_0186" );
}

string CLanguage :: ClearingHCL( )
{
    return m_CFG->GetString( "lang_0187", "lang_0187" );
}

string CLanguage :: TryingToRehostAsPrivateGame( string gamename )
{
    string Out = m_CFG->GetString( "lang_0188", "lang_0188" );
    UTIL_Replace( Out, "$GAMENAME$", gamename );
    return Out;
}

string CLanguage :: TryingToRehostAsPublicGame( string gamename )
{
    string Out = m_CFG->GetString( "lang_0189", "lang_0189" );
    UTIL_Replace( Out, "$GAMENAME$", gamename );
    return Out;
}

string CLanguage :: RehostWasSuccessful( )
{
    return m_CFG->GetString( "lang_0190", "lang_0190" );
}

string CLanguage :: TryingToJoinTheGameButBannedByName( string victim )
{
    string Out = m_CFG->GetString( "lang_0191", "lang_0191" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    return Out;
}

string CLanguage :: TryingToJoinTheGameButBannedByIP( string victim, string ip, string bannedname )
{
    string Out = m_CFG->GetString( "lang_0192", "lang_0192" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    UTIL_Replace( Out, "$IP$", ip );
    UTIL_Replace( Out, "$BANNEDNAME$", bannedname );
    return Out;
}

string CLanguage :: HasBannedName( string victim )
{
    string Out = m_CFG->GetString( "lang_0193", "lang_0193" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    return Out;
}

string CLanguage :: HasBannedIP( string victim, string ip, string bannedname )
{
    string Out = m_CFG->GetString( "lang_0194", "lang_0194" );
    UTIL_Replace( Out, "$VICTIM$", victim );
    UTIL_Replace( Out, "$IP$", ip );
    UTIL_Replace( Out, "$BANNEDNAME$", bannedname );
    return Out;
}

string CLanguage :: PlayersInGameState( string number, string players )
{
    string Out = m_CFG->GetString( "lang_0195", "lang_0195" );
    UTIL_Replace( Out, "$NUMBER$", number );
    UTIL_Replace( Out, "$PLAYERS$", players );
    return Out;
}

string CLanguage :: ValidServers( string servers )
{
    string Out = m_CFG->GetString( "lang_0196", "lang_0196" );
    UTIL_Replace( Out, "$SERVERS$", servers );
    return Out;
}

string CLanguage :: TeamCombinedScore( string team, string score )
{
    string Out = m_CFG->GetString( "lang_0197", "lang_0197" );
    UTIL_Replace( Out, "$TEAM$", team );
    UTIL_Replace( Out, "$SCORE$", score );
    return Out;
}

string CLanguage :: BalancingSlotsCompleted( )
{
    return m_CFG->GetString( "lang_0198", "lang_0198" );
}

string CLanguage :: PlayerWasKickedForFurthestScore( string name, string score, string average )
{
    string Out = m_CFG->GetString( "lang_0199", "lang_0199" );
    UTIL_Replace( Out, "$NAME$", name );
    UTIL_Replace( Out, "$SCORE$", score );
    UTIL_Replace( Out, "$AVERAGE$", average );
    return Out;
}

string CLanguage :: LocalAdminMessagesEnabled( )
{
    return m_CFG->GetString( "lang_0200", "lang_0200" );
}

string CLanguage :: LocalAdminMessagesDisabled( )
{
    return m_CFG->GetString( "lang_0201", "lang_0201" );
}

string CLanguage :: WasDroppedDesync( )
{
    return m_CFG->GetString( "lang_0202", "lang_0202" );
}

string CLanguage :: WasKickedForHavingLowestScore( string score )
{
    string Out = m_CFG->GetString( "lang_0203", "lang_0203" );
    UTIL_Replace( Out, "$SCORE$", score );
    return Out;
}

string CLanguage :: PlayerWasKickedForLowestScore( string name, string score )
{
    string Out = m_CFG->GetString( "lang_0204", "lang_0204" );
    UTIL_Replace( Out, "$NAME$", name );
    UTIL_Replace( Out, "$SCORE$", score );
    return Out;
}

string CLanguage :: ReloadingConfigurationFiles( )
{
    return m_CFG->GetString( "lang_0205", "lang_0205" );
}

string CLanguage :: CountDownAbortedSomeoneLeftRecently( )
{
    return m_CFG->GetString( "lang_0206", "lang_0206" );
}

string CLanguage :: UnableToCreateGameMustEnforceFirst( string gamename )
{
    string Out = m_CFG->GetString( "lang_0207", "lang_0207" );
    UTIL_Replace( Out, "$GAMENAME$", gamename );
    return Out;
}

string CLanguage :: UnableToLoadReplaysOutside( )
{
    return m_CFG->GetString( "lang_0208", "lang_0208" );
}

string CLanguage :: LoadingReplay( string file )
{
    string Out = m_CFG->GetString( "lang_0209", "lang_0209" );
    UTIL_Replace( Out, "$FILE$", file );
    return Out;
}

string CLanguage :: UnableToLoadReplayDoesntExist( string file )
{
    string Out = m_CFG->GetString( "lang_0210", "lang_0210" );
    UTIL_Replace( Out, "$FILE$", file );
    return Out;
}

string CLanguage :: CommandTrigger( string trigger )
{
    string Out = m_CFG->GetString( "lang_0211", "lang_0211" );
    UTIL_Replace( Out, "$TRIGGER$", trigger );
    return Out;
}

string CLanguage :: CantEndGameOwnerIsStillPlaying( string owner )
{
    string Out = m_CFG->GetString( "lang_0212", "lang_0212" );
    UTIL_Replace( Out, "$OWNER$", owner );
    return Out;
}

string CLanguage :: CantUnhostGameOwnerIsPresent( string owner )
{
    string Out = m_CFG->GetString( "lang_0213", "lang_0213" );
    UTIL_Replace( Out, "$OWNER$", owner );
    return Out;
}

string CLanguage :: WasAutomaticallyDroppedAfterSeconds( string seconds )
{
    string Out = m_CFG->GetString( "lang_0214", "lang_0214" );
    UTIL_Replace( Out, "$SECONDS$", seconds );
    return Out;
}

string CLanguage :: HasLostConnectionTimedOutGProxy( )
{
    return m_CFG->GetString( "lang_0215", "lang_0215" );
}

string CLanguage :: HasLostConnectionSocketErrorGProxy( string error )
{
    string Out = m_CFG->GetString( "lang_0216", "lang_0216" );
    UTIL_Replace( Out, "$ERROR$", error );
    return Out;
}

string CLanguage :: HasLostConnectionClosedByRemoteHostGProxy( )
{
    return m_CFG->GetString( "lang_0217", "lang_0217" );
}

string CLanguage :: WaitForReconnectSecondsRemain( string seconds )
{
    string Out = m_CFG->GetString( "lang_0218", "lang_0218" );
    UTIL_Replace( Out, "$SECONDS$", seconds );
    return Out;
}

string CLanguage :: WasUnrecoverablyDroppedFromGProxy( )
{
    return m_CFG->GetString( "lang_0219", "lang_0219" );
}

string CLanguage :: PlayerReconnectedWithGProxy( string name )
{
    string Out = m_CFG->GetString( "lang_0220", "lang_0220" );
    UTIL_Replace( Out, "$NAME$", name );
    return Out;
}

string CLanguage :: WrongContactBotOwner( )
{
    string Out = m_CFG->GetString("oh_900", "oh_900");

    return Out;
}

string CLanguage :: RanksNotLoaded( )
{
    string Out = m_CFG->GetString("oh_901", "oh_901");

    return Out;
}

string CLanguage :: SuccessfullyRegistered( )
{
    string Out = m_CFG->GetString("oh_001", "oh_001");

    return Out;
}

string CLanguage :: SuccessfullyConfirmedAccount( )
{
    string Out = m_CFG->GetString("oh_002", "oh_002");

    return Out;
}

string CLanguage :: WrongPassword()
{
    string Out = m_CFG->GetString("oh_003", "oh_003");

    return Out;
}

string CLanguage :: WrongEMail()
{
    string Out = m_CFG->GetString("oh_004", "oh_004");

    return Out;
}

string CLanguage :: NameAlreadyUsed()
{
    string Out = m_CFG->GetString("oh_005", "oh_005");

    return Out;
}

string CLanguage :: NoAccountToConfirm()
{
    string Out = m_CFG->GetString("oh_006", "oh_006");

    return Out;
}

string CLanguage :: BetPoints( string user, string points )
{
    string Out = m_CFG->GetString( "oh_007", "oh_007" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace(Out, "$POINTS$", points);
    return Out;
}

string CLanguage :: SuccessfullyResetedStats( string user )
{
    string Out = m_CFG->GetString( "oh_008", "oh_008" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: NoRecordFoundForUser( string user )
{
    string Out = m_CFG->GetString( "oh_009", "oh_009" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: SuccessfullyStoredMessage()
{
    string Out = m_CFG->GetString("oh_010", "oh_010");

    return Out;
}

string CLanguage :: NewMessages( string user, string amount )
{
    string Out = m_CFG->GetString( "oh_011", "oh_011" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace(Out, "$AMOUNT$", amount);
    return Out;
}

string CLanguage :: ErrorInboxEmpty()
{
    string Out = m_CFG->GetString("oh_011a", "oh_011a");

    return Out;
}

string CLanguage :: SuccessfullyTypedPassword( string user )
{
    string Out = m_CFG->GetString( "oh_012", "oh_012" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: UserGotPenalityPoints( string user, string amount )
{
    string Out = m_CFG->GetString( "oh_013", "oh_013" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace(Out, "$AMOUNT$", amount);
    return Out;
}

string CLanguage :: UserGotNoPenalityPoints( string user )
{
    string Out = m_CFG->GetString( "oh_014", "oh_014" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: AddedPenalityPoints( string user, string admin, string amount )
{
    string Out = m_CFG->GetString( "oh_015", "oh_015" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$ADMIN$", admin );
    UTIL_Replace(Out, "$AMOUNT$", amount);
    return Out;
}

string CLanguage :: BannedUserForReachingTooManyPPoints( string user )
{
    string Out = m_CFG->GetString( "oh_016", "oh_016" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: FailedToAddPPoint()
{
    string Out = m_CFG->GetString("oh_017", "oh_017");

    return Out;
}

string CLanguage :: NotIPBanned()
{
    string Out = m_CFG->GetString("oh_018", "oh_018");

    return Out;
}

string CLanguage :: FoundIPBans( string result )
{
    string Out = m_CFG->GetString( "oh_019", "oh_019" );
    UTIL_Replace( Out, "$RESULT$", result );
    return Out;
}

string CLanguage :: ErrorBanningUserAlreadyPermBanned( string user )
{
    string Out = m_CFG->GetString( "oh_020", "oh_020" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: ErrorBanningUserAlreadyLongerBanned( string user )
{
    string Out = m_CFG->GetString( "oh_021", "oh_021" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: SuccessfullyBannedUser( string user, string server )
{
    string Out = m_CFG->GetString( "oh_022", "oh_022" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$SERVER$", server );
    return Out;
}
string CLanguage :: SuccessfullyUpdatedBannedUser( string user, string server )
{
    string Out = m_CFG->GetString( "oh_023", "oh_023" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$SERVER$", server );
    return Out;
}
string CLanguage :: SuccessfullyPermBannedUser( string user, string server )
{
    string Out = m_CFG->GetString( "oh_024", "oh_024" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$SERVER$", server );
    return Out;
}

string CLanguage :: ChangedRankOfUser( string user, string levelname, string server )
{
    string Out = m_CFG->GetString( "oh_025", "oh_025" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$LEVELNAME$", levelname );
    UTIL_Replace(Out, "$SERVER$", server);
    return Out;
}

string CLanguage :: UserHasAHiddenAcc( string user )
{
    string Out = m_CFG->GetString( "oh_026", "oh_026" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: RankOfUser( string user, string time, string alias, string rank, string level, string levelname )
{
    string Out = m_CFG->GetString( "oh_027", "oh_027" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace(Out, "$TIME$", time);
    UTIL_Replace(Out, "$ALIAS$", alias);
    UTIL_Replace(Out, "$RANK$", rank);
    UTIL_Replace(Out, "$LEVEL$", level);
    UTIL_Replace( Out, "$LEVELNAME$", levelname );
    return Out;
}

string CLanguage :: RankOfUserWithoutLevel( string user, string time, string alias, string rank)
{
    string Out = m_CFG->GetString( "oh_027a", "oh_027a" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace(Out, "$TIME$", time);
    UTIL_Replace(Out, "$ALIAS$", alias);
    UTIL_Replace(Out, "$RANK$", rank);
    return Out;
}

string CLanguage :: StreakOfUser( string user, string time, string alias, string streak, string maxstreak, string maxlstreak)
{
    string Out = m_CFG->GetString( "oh_028", "oh_028" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace(Out, "$TIME$", time);
    UTIL_Replace(Out, "$ALIAS$", alias);
    UTIL_Replace(Out, "$STREAK$", streak);
    UTIL_Replace(Out, "$MAXSTREAK$", maxstreak);
    UTIL_Replace(Out, "$MAXLSTREAK$", maxlstreak);
    return Out;
}

string CLanguage :: UserMutedByRCON( string user, string admin )
{
    string Out = m_CFG->GetString( "oh_029", "oh_029" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$ADMIN$", admin );
    return Out;
}
string CLanguage :: UserUnMutedByRCON( string user, string admin )
{
    string Out = m_CFG->GetString( "oh_030", "oh_030" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$ADMIN$", admin );
    return Out;
}

string CLanguage :: UserKickedByRCON( string user, string admin )
{
    string Out = m_CFG->GetString( "oh_031", "oh_031" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$ADMIN$", admin );
    return Out;
}

string CLanguage :: InvalidName( )
{
    string Out = m_CFG->GetString( "oh_032", "oh_032" );
    return Out;
}
string CLanguage :: WrongInputUseALevel( )
{
    string Out = m_CFG->GetString( "oh_033", "oh_033" );
    return Out;
}
string CLanguage :: WrongInputInvalidLevel( )
{
    string Out = m_CFG->GetString( "oh_034", "oh_034" );
    return Out;
}

string CLanguage :: UserLevelCheck( string user, string level )
{
    string Out = m_CFG->GetString( "oh_035", "oh_035" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$LEVEL$", level );
    return Out;
}

string CLanguage :: InvalidNameTooShort( )
{
    string Out = m_CFG->GetString( "oh_036", "oh_036" );
    return Out;
}

string CLanguage :: NoPermissionToExecCommand( )
{
    string Out = m_CFG->GetString( "oh_037", "oh_037" );
    return Out;
}

string CLanguage ::CommandUsageInfo(string command) {
    string Out = m_CFG->GetString( "usage_" + command , "Usage: !" + command + " ( usage_" + command + ")" );
    return Out;
}

string CLanguage :: TooMuchPPoints( )
{
    string Out = m_CFG->GetString( "oh_038", "oh_038" );
    return Out;
}

string CLanguage :: ErrorMissingReason( )
{
    string Out = m_CFG->GetString( "oh_039", "oh_039" );
    return Out;
}

string CLanguage :: ErrorBadInput( )
{
    string Out = m_CFG->GetString( "oh_040", "oh_040" );
    return Out;
}

string CLanguage :: BannedIPRange( string ip, string server )
{
    string Out = m_CFG->GetString( "oh_041", "oh_041" );
    UTIL_Replace( Out, "$IP$", ip );
    UTIL_Replace( Out, "$SERVER$", server);
    return Out;
}

string CLanguage :: ErrorBanningWrongSuffix( string suffix )
{
    string Out = m_CFG->GetString( "oh_042", "oh_042" );
    UTIL_Replace( Out, "$SUFFIX$", suffix );
    return Out;
}

string CLanguage :: CheckBanTempUser( string user, string date, string expire, string remain, string reason )
{
    string Out = m_CFG->GetString( "oh_043", "oh_043" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$DATE$", date );
    UTIL_Replace( Out, "$EXPIRE$", expire );
    UTIL_Replace( Out, "$REMAIN$", remain );
    UTIL_Replace( Out, "$REASON$", reason );
    return Out;
}

string CLanguage :: CheckBanPermUser( string user, string date, string reason )
{
    string Out = m_CFG->GetString( "oh_044", "oh_044" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$DATE$", date );
    UTIL_Replace( Out, "$REASON$", reason );
    return Out;
}

string CLanguage :: CheckBanNameBan( )
{
    string Out = m_CFG->GetString( "oh_045", "oh_045" );
    return Out;
}

string CLanguage :: ChangedStatusForVIPGames( string type )
{
    string Out = m_CFG->GetString( "oh_046", "oh_046" );
    UTIL_Replace( Out, "$TYPE$", type );
    return Out;
}

string CLanguage :: WrongOptionUserOnOff( )
{
    string Out = m_CFG->GetString( "oh_047", "oh_047" );
    return Out;
}

string CLanguage :: ChangedMinPlayedGames( string type, string limit )
{
    string Out = m_CFG->GetString( "oh_048", "oh_048" );
    UTIL_Replace( Out, "$TYPE$", type );
    UTIL_Replace( Out, "$LIMIT$", limit );
    return Out;
}

string CLanguage :: ErrorWrongInputForSayPlayer( )
{
    string Out = m_CFG->GetString( "oh_049", "oh_049" );
    return Out;
}

string CLanguage :: ErrorWrongInputForMessage( )
{
    string Out = m_CFG->GetString( "oh_050", "oh_050" );
    return Out;
}

string CLanguage :: ErrorMessageTooShort( )
{
    string Out = m_CFG->GetString( "oh_051", "oh_051" );
    return Out;
}

string CLanguage :: ErrorRegisterAbusePrevention( )
{
    string Out = m_CFG->GetString( "oh_052", "oh_052" );
    return Out;
}

string CLanguage :: WrongRegisterCommand( string command )
{
    string Out = m_CFG->GetString( "oh_053", "oh_053" );
    UTIL_Replace( Out, "$COMMAND$", command);
    return Out;
}

string CLanguage :: WrongPassRegisterCommand( string pass )
{
    string Out = m_CFG->GetString( "oh_054", "oh_054" );
    UTIL_Replace( Out, "$PASS$", pass);
    return Out;
}

string CLanguage :: PassTooShortRegisterCommand( string pass )
{
    string Out = m_CFG->GetString( "oh_055", "oh_055" );
    UTIL_Replace( Out, "$PASS$", pass);
    return Out;
}

string CLanguage :: InvalidEmailRegisterCommand( string mail)
{
    string Out = m_CFG->GetString( "oh_056", "oh_056" );
    UTIL_Replace( Out, "$MAIL$", mail);
    return Out;
}

string CLanguage :: ErrorWhispRegister( )
{
    string Out = m_CFG->GetString( "oh_057", "oh_057" );
    return Out;
}

string CLanguage :: NoMapAliasRecordFound( )
{
    string Out = m_CFG->GetString( "oh_058", "oh_058" );
    return Out;
}

string CLanguage :: PassProtAcc( )
{
    string Out = m_CFG->GetString( "oh_059", "oh_059" );
    return Out;
}

string CLanguage :: RemovedPassProtAcc( )
{
    string Out = m_CFG->GetString( "oh_060", "oh_060" );
    return Out;
}

string CLanguage :: NoPassProtAcc( )
{
    string Out = m_CFG->GetString( "oh_061", "oh_061" );
    return Out;
}

string CLanguage :: CurrentPoints( string user, string points)
{
    string Out = m_CFG->GetString( "oh_062", "oh_062" );
    UTIL_Replace( Out, "$USER$", user);
    UTIL_Replace( Out, "$POINTS$", points);
    return Out;
}

string CLanguage :: ErrorAlreadyBet( )
{
    string Out = m_CFG->GetString( "oh_063", "oh_063" );
    return Out;
}

string CLanguage :: UserBet( string user, string amount)
{
    string Out = m_CFG->GetString( "oh_064", "oh_064" );
    UTIL_Replace( Out, "$USER$", user);
    UTIL_Replace( Out, "$AMOUNT$", amount);
    return Out;
}

string CLanguage :: BetATooHighAmount( string result)
{
    string Out = m_CFG->GetString( "oh_065", "oh_065" );
    UTIL_Replace( Out, "$RESULT$", result);
    return Out;
}

string CLanguage :: RemovingPlayerNotifyFF( string team)
{
    string Out = m_CFG->GetString( "oh_066", "oh_066" );
    UTIL_Replace( Out, "$TEAM$", team);
    return Out;
}

string CLanguage :: WaitForProperStatsSafe( )
{
    string Out = m_CFG->GetString( "oh_067", "oh_067" );
    return Out;
}

string CLanguage :: GameWillEndInTen( )
{
    string Out = m_CFG->GetString( "oh_068", "oh_068" );
    return Out;
}

string CLanguage :: WinnerSetTo( string team)
{
    string Out = m_CFG->GetString( "oh_069", "oh_069" );
    UTIL_Replace( Out, "$TEAM$", team);
    return Out;
}

string CLanguage :: AutoEndSpreadInterruptNotify( string seconds, string votes )
{
    string Out = m_CFG->GetString( "oh_070", "oh_070" );
    UTIL_Replace( Out, "$SECONDS$", seconds);
    UTIL_Replace( Out, "$VOTES$", votes);
    return Out;
}

string CLanguage :: VoteModeExpired( )
{
    string Out = m_CFG->GetString( "oh_071", "oh_071" );
    return Out;
}

string CLanguage :: AbsoluteVoteChoosen( string mode )
{
    string Out = m_CFG->GetString( "oh_072", "oh_072" );
    UTIL_Replace( Out, "$MODE$", mode );
    return Out;
}

string CLanguage :: AbsoluteVoteChoosenRandom( string mode )
{
    string Out = m_CFG->GetString( "oh_073", "oh_073" );
    UTIL_Replace( Out, "$MODE$", mode );
    return Out;
}

string CLanguage :: TopVoteChoosenRandom( string mode )
{
    string Out = m_CFG->GetString( "oh_074", "oh_074" );
    UTIL_Replace( Out, "$MODE$", mode );
    return Out;
}

string CLanguage :: UserMayBanned( string user )
{
    string Out = m_CFG->GetString( "oh_075", "oh_075" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: AutoEndHighSpread( string spread )
{
    string Out = m_CFG->GetString( "oh_076", "oh_076" );
    UTIL_Replace( Out, "$SPREAD$", spread );
    return Out;
}

string CLanguage :: AutoEndTooFewPlayers( )
{
    string Out = m_CFG->GetString( "oh_077", "oh_077" );
    return Out;
}

string CLanguage :: AutoEndToDraw( )
{
    string Out = m_CFG->GetString( "oh_078", "oh_078" );
    return Out;
}

string CLanguage :: AutoEndOneTeamRemain( )
{
    string Out = m_CFG->GetString( "oh_079", "oh_079" );
    return Out;
}

string CLanguage :: AutoEndSpreadNotify( string team, string votes )
{
    string Out = m_CFG->GetString( "oh_080", "oh_080" );
    UTIL_Replace( Out, "$TEAM$", team);
    UTIL_Replace( Out, "$VOTES$", votes);
    return Out;
}

string CLanguage :: AutoEndEarlyDrawOne( )
{
    string Out = m_CFG->GetString( "oh_081", "oh_081" );
    return Out;
}
string CLanguage :: AutoEndEarlyDrawTwo( )
{
    string Out = m_CFG->GetString( "oh_082", "oh_082" );
    return Out;
}
string CLanguage :: AutoEndEarlyDrawThree( )
{
    string Out = m_CFG->GetString( "oh_083", "oh_083" );
    return Out;
}

string CLanguage :: FoundNoMatchWithPlayername( )
{
    string Out = m_CFG->GetString( "oh_084", "oh_084" );
    return Out;
}

string CLanguage :: SuccessfullyLetPlayerInsult( )
{
    string Out = m_CFG->GetString( "oh_085", "oh_085" );
    return Out;
}

string CLanguage :: FoundMultiplyMatches( )
{
    string Out = m_CFG->GetString( "oh_086", "oh_086" );
    return Out;
}

string CLanguage :: MessageTooLongChooseAMoreShorten( )
{
    string Out = m_CFG->GetString( "oh_087", "oh_087" );
    return Out;
}

string CLanguage :: DisallowedOnlyCountryCheckNotify( )
{
    string Out = m_CFG->GetString( "oh_088", "oh_088" );
    return Out;
}

string CLanguage :: DisallowedUnallowedCountryCheckNotify( )
{
    string Out = m_CFG->GetString( "oh_089", "oh_089" );
    return Out;
}

string CLanguage :: ErrorCountryDenyListIsntCleared( )
{
    string Out = m_CFG->GetString( "oh_090", "oh_090" );
    return Out;
}

string CLanguage :: ErrorCountryOnlyListIsntCleared( )
{
    string Out = m_CFG->GetString( "oh_091", "oh_091" );
    return Out;
}

string CLanguage :: EnabledDeniedOrLimitedCountries( string type, string countries )
{
    string Out = m_CFG->GetString( "oh_092", "oh_092" );
    UTIL_Replace( Out, "$TYPE$", type);
    UTIL_Replace( Out, "$COUNTRIES$", countries);
    return Out;
}

string CLanguage :: CountryKickExceptionPlayer( string user, string cc, string type, string levelname)
{
    string Out = m_CFG->GetString( "oh_093", "oh_093" );
    UTIL_Replace( Out, "$USER$", user);
    UTIL_Replace( Out, "$CC$", cc);
    UTIL_Replace( Out, "$TYPE$", type);
    UTIL_Replace( Out, "$LEVELNAME$", levelname);
    return Out;
}

string CLanguage :: KickedPlayerForDeniedCountry( string user)
{
    string Out = m_CFG->GetString( "oh_094", "oh_094" );
    UTIL_Replace( Out, "$USER$", user);
    return Out;
}

string CLanguage :: DeniedGarenaKickNotify( )
{
    string Out = m_CFG->GetString( "oh_095", "oh_095" );
    return Out;
}

string CLanguage :: KickedPlayerForDeniedGarena( string user)
{
    string Out = m_CFG->GetString( "oh_096", "oh_096" );
    UTIL_Replace( Out, "$USER$", user);
    return Out;
}

string CLanguage :: DeniedUser( string user)
{
    string Out = m_CFG->GetString( "oh_097", "oh_097" );
    UTIL_Replace( Out, "$USER$", user);
    return Out;
}

string CLanguage :: GaveTheThing( string player, string user )
{
    string Out = m_CFG->GetString( "oh_098", "oh_098" );
    UTIL_Replace( Out, "$USER$", user);
    UTIL_Replace( Out, "$PLAYER$", player);
    return Out;
}

string CLanguage :: MutedAllChatOfUser( string user, string admin)
{
    string Out = m_CFG->GetString( "oh_099", "oh_099" );
    UTIL_Replace( Out, "$USER$", user);
    UTIL_Replace( Out, "$ADMIN$", admin);
    return Out;
}

string CLanguage :: UnMutedAllChatOfUser( string user, string admin)
{
    string Out = m_CFG->GetString( "oh_100", "oh_100" );
    UTIL_Replace( Out, "$USER$", user);
    UTIL_Replace( Out, "$ADMIN$", admin);
    return Out;
}

string CLanguage :: SetWinnerByUser( string user, string team)
{
    string Out = m_CFG->GetString( "oh_101", "oh_101" );
    UTIL_Replace( Out, "$USER$", user);
    UTIL_Replace( Out, "$TEAM$", team);
    return Out;
}

string CLanguage :: UserAlreadyVotedForFF( )
{
    string Out = m_CFG->GetString( "oh_102", "oh_102" );
    return Out;
}

string CLanguage :: UserAlreadyVotedForDraw( )
{
    string Out = m_CFG->GetString( "oh_103", "oh_103" );
    return Out;
}

string CLanguage :: DisplayIgnoredPlayersForUser( string ignored )
{
    string Out = m_CFG->GetString( "oh_104", "oh_104" );
    UTIL_Replace( Out, "$IGNORED$", ignored);
    return Out;
}

string CLanguage :: AlreadyMarkedAsAFK( )
{
    string Out = m_CFG->GetString( "oh_105", "oh_105" );
    return Out;
}

string CLanguage :: AlreadyMarkedAsFeeder( )
{
    string Out = m_CFG->GetString( "oh_106", "oh_106" );
    return Out;
}

string CLanguage :: AlreadyMarkedAsFullFeeder( )
{
    string Out = m_CFG->GetString( "oh_107", "oh_107" );
    return Out;
}

string CLanguage :: AlreadyMarkedXTimesWithHighPing( string times)
{
    string Out = m_CFG->GetString( "oh_108", "oh_108" );
    UTIL_Replace( Out, "$TIMES$", times);
    return Out;
}

string CLanguage :: DidNotFoundAlias( string alias )
{
    string Out = m_CFG->GetString( "oh_109", "oh_109" );
    UTIL_Replace( Out, "$ALIAS$", alias);
    return Out;
}

string CLanguage :: UsingDefaultAlias( string alias )
{
    string Out = m_CFG->GetString( "oh_110", "oh_110" );
    UTIL_Replace( Out, "$ALIAS$", alias);
    return Out;
}

string CLanguage :: CurrentVoteKickProcessUser( string user )
{
    string Out = m_CFG->GetString( "oh_111", "oh_111" );
    UTIL_Replace( Out, "$USER$", user);
    return Out;
}

string CLanguage :: CurrentVoteKickRunTime( string runtime, string timeleft )
{
    string Out = m_CFG->GetString( "oh_112", "oh_112" );
    UTIL_Replace( Out, "$RUNTIME$", runtime);
    UTIL_Replace( Out, "$TIMELEFT$", timeleft);
    return Out;
}

string CLanguage :: NotifyForAbusiveVotekick( )
{
    string Out = m_CFG->GetString( "oh_113", "oh_113" );
    return Out;
}

string CLanguage :: NotifyForCustomVotekickReason( )
{
    string Out = m_CFG->GetString( "oh_114", "oh_114" );
    return Out;
}

string CLanguage :: VoteKickedUserWhoWasntMarkedAsFeeder( string user )
{
    string Out = m_CFG->GetString( "oh_115", "oh_115" );
    UTIL_Replace( Out, "$USER$", user);
    return Out;
}

string CLanguage :: VoteKickFeederAbuseReason( )
{
    string Out = m_CFG->GetString( "oh_116", "oh_116" );
    return Out;
}

string CLanguage :: VoteKickedUserWhoWasMarkedAsFeederLevelOne( string user )
{
    string Out = m_CFG->GetString( "oh_117", "oh_117" );
    UTIL_Replace( Out, "$USER$", user);
    return Out;
}

string CLanguage :: ReminederForVotekickAFeederLevelOne( )
{
    string Out = m_CFG->GetString( "oh_118", "oh_118" );
    return Out;
}

string CLanguage :: VoteKickNotPossiblePlayerIsInAnotherTeam( string user )
{
    string Out = m_CFG->GetString( "oh_119", "oh_119" );
    UTIL_Replace( Out, "$USER$", user);
    return Out;
}

string CLanguage :: UnableToVotekickYourself( )
{
    string Out = m_CFG->GetString( "oh_120", "oh_120" );
    return Out;
}

string CLanguage :: UnableToVoteStartMissingPlayers( string players )
{
    string Out = m_CFG->GetString( "oh_121", "oh_121" );
    UTIL_Replace( Out, "$PLAYERS$", players);
    return Out;
}

string CLanguage :: UnableToVoteStartMissingVotesd( string votes )
{
    string Out = m_CFG->GetString( "oh_122", "oh_122" );
    UTIL_Replace( Out, "$VOTES$", votes);
    return Out;
}

string CLanguage :: SuccessfullyVoteStarted( )
{
    string Out = m_CFG->GetString( "oh_123", "oh_123" );
    return Out;
}

string CLanguage :: UnableToVoteStartOwnerInGame( )
{
    string Out = m_CFG->GetString( "oh_124", "oh_124" );
    return Out;
}

string CLanguage :: UnableToVoteKickNotUsersTeam( )
{
    string Out = m_CFG->GetString( "oh_125", "oh_125" );
    return Out;
}

string CLanguage :: UserVotedForDraw( string user, string votes )
{
    string Out = m_CFG->GetString( "oh_126", "oh_126" );
    UTIL_Replace( Out, "$USER$", user);
    UTIL_Replace( Out, "$VOTES$", votes);
    return Out;
}

string CLanguage :: UndrawNotify( )
{
    string Out = m_CFG->GetString( "oh_127", "oh_127" );
    return Out;
}

string CLanguage :: UserRecalledDrawVote( string user )
{
    string Out = m_CFG->GetString( "oh_128", "oh_128" );
    UTIL_Replace( Out, "$USER$", user);
    return Out;
}

string CLanguage :: ObserverTriesToDraw( )
{
    string Out = m_CFG->GetString( "oh_129", "oh_129" );
    return Out;
}

string CLanguage :: RemainFFTime( string minff, string remain )
{
    string Out = m_CFG->GetString( "oh_130", "oh_130" );
    UTIL_Replace( Out, "$MINFF$", minff );
    UTIL_Replace( Out, "$REMAIN$", remain );
    return Out;
}

string CLanguage :: TeamForfeited( string team )
{
    string Out = m_CFG->GetString( "oh_131", "oh_131" );
    UTIL_Replace( Out, "$TEAM$", team);
    return Out;
}

string CLanguage :: StayToSafeStats( )
{
    string Out = m_CFG->GetString( "oh_132", "oh_132" );
    return Out;
}

string CLanguage :: UserForfeitedGame( string user )
{
    string Out = m_CFG->GetString( "oh_133", "oh_133" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: UserForfeitedGameNotify( string team, string votes, string votesneeded )
{
    string Out = m_CFG->GetString( "oh_134", "oh_134" );
    UTIL_Replace( Out, "$TEAM$", team );
    UTIL_Replace( Out, "$VOTES$", votes );
    UTIL_Replace( Out, "$VOTESNEEDED$", votesneeded );
    return Out;
}

string CLanguage :: NoOneHasForfeitedYet( )
{
    string Out = m_CFG->GetString( "oh_135", "oh_135" );
    return Out;
}

string CLanguage :: ErrorForfeitingNoTwoTeamMap( )
{
    string Out = m_CFG->GetString( "oh_136", "oh_136" );
    return Out;
}

string CLanguage :: UserWillObserveGame( string user )
{
    string Out = m_CFG->GetString( "oh_137", "oh_137" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: ErrorObserverGameAlreadyObserver( )
{
    string Out = m_CFG->GetString( "oh_138", "oh_138" );
    return Out;
}

string CLanguage :: UserWillNoLongerObserveGame( string user )
{
    string Out = m_CFG->GetString( "oh_139", "oh_139" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: ErrorRemovingPassProtectionInvalidOption( )
{
    string Out = m_CFG->GetString( "oh_140", "oh_140" );
    return Out;
}

string CLanguage :: ErrorBetInvalidAmount( )
{
    string Out = m_CFG->GetString( "oh_141", "oh_141" );
    return Out;
}

string CLanguage :: ErrorBetInvalidLogicalAmount( )
{
    string Out = m_CFG->GetString( "oh_142", "oh_142" );
    return Out;
}

string CLanguage :: ErrorBetAlreadyTooLate( )
{
    string Out = m_CFG->GetString( "oh_143", "oh_143" );
    return Out;
}

string CLanguage :: UserRequestedToPauseGame( string user )
{
    string Out = m_CFG->GetString( "oh_144", "oh_144" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: GamePauseNotify( )
{
    string Out = m_CFG->GetString( "oh_145", "oh_145" );
    return Out;
}

string CLanguage :: ErrorPausingAlreadyRequested( )
{
    string Out = m_CFG->GetString( "oh_146", "oh_146" );
    return Out;
}

string CLanguage :: UserRequestedToUnPauseGame( string user )
{
    string Out = m_CFG->GetString( "oh_147", "oh_147" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: ErrorUnPausingGameIsntPaused( )
{
    string Out = m_CFG->GetString( "oh_148", "oh_148" );
    return Out;
}

string CLanguage :: ErrorOneTeamHasNoPlayers( )
{
    string Out = m_CFG->GetString( "oh_149", "oh_149" );
    return Out;
}

string CLanguage :: ErrorNotATwoTeamMap( )
{
    string Out = m_CFG->GetString( "oh_150", "oh_150" );
    return Out;
}

string CLanguage :: UserAteACookie( string user )
{
    string Out = m_CFG->GetString( "oh_151", "oh_151" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: UserRemovedCookieNotify( string user )
{
    string Out = m_CFG->GetString( "oh_152", "oh_152" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: ErrorEatACookieNoCookieAvaible( )
{
    string Out = m_CFG->GetString( "oh_153", "oh_153" );
    return Out;
}

string CLanguage :: UserIgnoredPlayer( string user )
{
    string Out = m_CFG->GetString( "oh_154", "oh_154" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: UserUnIgnoredPlayer( string user )
{
    string Out = m_CFG->GetString( "oh_155", "oh_155" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: UserIgnoringAllPlayersNotify( )
{
    string Out = m_CFG->GetString( "oh_156", "oh_156" );
    return Out;
}

string CLanguage :: UserUnIgnoringAllPlayersNotify( )
{
    string Out = m_CFG->GetString( "oh_157", "oh_157" );
    return Out;
}

string CLanguage :: RuleTagNotify( )
{
    string Out = m_CFG->GetString( "oh_158", "oh_158" );
    return Out;
}

string CLanguage :: UserStartedVoteMute( string user, string target )
{
    string Out = m_CFG->GetString( "oh_159", "oh_159" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$TARGET$", target );
    return Out;
}

string CLanguage :: UserStartedVoteMuteVotesNeeded( string team, string votes )
{
    string Out = m_CFG->GetString( "oh_160", "oh_160" );
    UTIL_Replace( Out, "$TEAM$", team);
    UTIL_Replace( Out, "$VOTES$", votes );
    return Out;
}

string CLanguage :: UserStartedVoteMuteVoteExpire( string expire )
{
    string Out = m_CFG->GetString( "oh_161", "oh_161" );
    UTIL_Replace( Out, "$EXPIRE$", expire );
    return Out;
}

string CLanguage :: UserStartedVoteGlobalMute( string user, string target )
{
    string Out = m_CFG->GetString( "oh_162", "oh_162" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$TARGET$", target );
    return Out;
}

string CLanguage :: UserStartedVoteGlobalMuteVotesNeeded( )
{
    string Out = m_CFG->GetString( "oh_163", "oh_163" );
    return Out;
}

string CLanguage :: UserStartedVoteNotify( )
{
    string Out = m_CFG->GetString( "oh_164", "oh_164" );
    return Out;
}

string CLanguage :: SuccessfullyVoteMutedUser( string user )
{
    string Out = m_CFG->GetString( "oh_165", "oh_165" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: UserVotedForMute( string user, string target, string votes )
{
    string Out = m_CFG->GetString( "oh_166", "oh_166" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$TARGET$", target );
    UTIL_Replace( Out, "$VOTES$", votes );
    return Out;
}

string CLanguage :: UserVotedToModeNoVotesOnTeam( )
{
    string Out = m_CFG->GetString( "oh_167", "oh_167" );
    return Out;
}

string CLanguage :: UserVotedToModeNoVotesOnObserver( )
{
    string Out = m_CFG->GetString( "oh_168", "oh_168" );
    return Out;
}

string CLanguage :: ErrorVotingThereIsNoVote( )
{
    string Out = m_CFG->GetString( "oh_169", "oh_169" );
    return Out;
}

string CLanguage :: SuccessfullyInterruptedAutoEnd( )
{
    string Out = m_CFG->GetString( "oh_170", "oh_170" );
    return Out;
}

string CLanguage :: UserVotedForInterruptAutoEnd( string user, string votes )
{
    string Out = m_CFG->GetString( "oh_171", "oh_171" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$VOTES$", votes );
    return Out;
}

string CLanguage :: UserAlreadyVotedForIntteruptAutoEnd( )
{
    string Out = m_CFG->GetString( "oh_172", "oh_172" );
    return Out;
}

string CLanguage :: ErrorVotedAlreadyForMode( )
{
    string Out = m_CFG->GetString( "oh_173", "oh_173" );
    return Out;
}

string CLanguage :: ErrorInvalidModeWasVoted( )
{
    string Out = m_CFG->GetString( "oh_174", "oh_174" );
    return Out;
}

string CLanguage :: UserVotedForMode( string user, string mode )
{
    string Out = m_CFG->GetString( "oh_175", "oh_175" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$MODE$", mode );
    return Out;
}

string CLanguage :: PlayersWhoDidntVoteForMode( string players )
{
    string Out = m_CFG->GetString( "oh_176", "oh_176" );
    UTIL_Replace( Out, "$PLAYERS$", players );
    return Out;
}

string CLanguage :: NoUserDefined( )
{
    string Out = m_CFG->GetString( "oh_177", "oh_177" );
    return Out;
}

string CLanguage :: UserForcedMode( string user, string mode )
{
    string Out = m_CFG->GetString( "oh_178", "oh_178" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$MODE$", mode );
    return Out;
}

string CLanguage :: UserRefilledCookieChar( string user, string player )
{
    string Out = m_CFG->GetString( "oh_179", "oh_179" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$PLAYER$", player);
    return Out;
}

string CLanguage :: EnabledAutobalance( )
{
    string Out = m_CFG->GetString( "oh_180", "oh_180" );
    return Out;
}

string CLanguage :: DisabledAutobalance( )
{
    string Out = m_CFG->GetString( "oh_181", "oh_181" );
    return Out;
}

string CLanguage :: UserEnabledNoLag( string user )
{
    string Out = m_CFG->GetString( "oh_182", "oh_182" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: UserDisabledNoLag( string user )
{
    string Out = m_CFG->GetString( "oh_183", "oh_183" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: UserVotedForBalance( string user )
{
    string Out = m_CFG->GetString( "oh_184", "oh_184" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}
string CLanguage :: EnabledBalanceForThisGame( )
{
    string Out = m_CFG->GetString( "oh_185", "oh_185" );
    return Out;
}

string CLanguage :: ErrorVotedAlreadyForBalance( )
{
    string Out = m_CFG->GetString( "oh_186", "oh_186" );
    return Out;
}

string CLanguage :: UsersReputation( string user, string reputation )
{
    string Out = m_CFG->GetString( "oh_187", "oh_187" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$REPUTATION$", reputation);
    return Out;
}

string CLanguage :: UsersOwnReputation( string reputation )
{
    string Out = m_CFG->GetString( "oh_188", "oh_188" );
    UTIL_Replace( Out, "$REPUTATION$", reputation);
    return Out;
}

string CLanguage :: WelcomeUserCreateUniqueId( string user )
{
    string Out = m_CFG->GetString( "oh_189", "oh_189" );
    UTIL_Replace( Out, "$USER$", user);
    return Out;
}

string CLanguage :: DomainOnJoinNotify( )
{
    string Out = m_CFG->GetString( "oh_190", "oh_190" );
    return Out;
}

string CLanguage :: WelcomeBackUser( string user )
{
    string Out = m_CFG->GetString( "oh_191", "oh_191" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: CreatedUniqueIdForNewUser( string id )
{
    string Out = m_CFG->GetString( "oh_192", "oh_192" );
    UTIL_Replace( Out, "$ID$", id);
    return Out;
}

string CLanguage :: PotentialBanAvoider( string user, string count )
{
    string Out = m_CFG->GetString( "oh_193", "oh_193" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$COUNT$", count);
    return Out;
}

string CLanguage :: KickedUserForBeingAFK( string user )
{
    string Out = m_CFG->GetString( "oh_194", "oh_194" );
    UTIL_Replace( Out, "$USER$", user);
    return Out;
}

string CLanguage :: WarnedUserForBeingAFK( string user )
{
    string Out = m_CFG->GetString( "oh_195", "oh_195" );
    UTIL_Replace( Out, "$USER$", user);
    return Out;
}

string CLanguage :: KickedUserForBeingAfk( string user )
{
    string Out = m_CFG->GetString( "oh_196", "oh_196" );
    UTIL_Replace( Out, "$USER$", user);
    return Out;
}

string CLanguage :: ErrorForAPMAFKSystem( )
{
    string Out = m_CFG->GetString( "oh_197", "oh_197" );
    return Out;
}

string CLanguage :: UserHasBeenMarkedAPMAFK( string user, string apm )
{
    string Out = m_CFG->GetString( "oh_198", "oh_198" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$APM$", apm);
    return Out;
}

string CLanguage :: UserWarningAPMAFK( string min, string warns )
{
    string Out = m_CFG->GetString( "oh_199", "oh_199" );
    UTIL_Replace( Out, "$MIN$", min );
    UTIL_Replace( Out, "$WARN$", warns);
    return Out;
}

string CLanguage :: UserKickedAPMAFK( string user, string apm, string times, string min )
{
    string Out = m_CFG->GetString( "oh_200", "oh_200" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$APM$", apm);
    UTIL_Replace( Out, "$TIMES$", times );
    UTIL_Replace( Out, "$MIN$", min);
    return Out;
}

string CLanguage :: VoteStartExpired( )
{
    string Out = m_CFG->GetString( "oh_201", "oh_201" );
    return Out;
}

string CLanguage :: VoteMuteExpired( string user )
{
    string Out = m_CFG->GetString( "oh_202", "oh_202" );
    UTIL_Replace( Out, "$USER$", user);
    return Out;
}

string CLanguage :: GameWillBePausedInTicks( string ticks )
{
    string Out = m_CFG->GetString( "oh_203", "oh_203" );
    UTIL_Replace( Out, "$TICKS$", ticks);
    return Out;
}

string CLanguage :: GameWillBeResumedInTicks( string ticks )
{
    string Out = m_CFG->GetString( "oh_204", "oh_204" );
    UTIL_Replace( Out, "$TICKS$", ticks);
    return Out;
}

string CLanguage :: GameWillBeUnpauseInTenSeconds( )
{
    string Out = m_CFG->GetString( "oh_205", "oh_205" );
    return Out;
}

string CLanguage :: UserHaveAHighPingMark( string ping, string times)
{
    string Out = m_CFG->GetString( "oh_206", "oh_206" );
    UTIL_Replace( Out, "$PING$", ping );
    UTIL_Replace( Out, "$TIMES$", times);
    return Out;
}

string CLanguage :: UserCanLeaveBecauseHighPing( )
{
    string Out = m_CFG->GetString( "oh_207", "oh_207" );
    return Out;
}

string CLanguage :: UserCanLeaveBecausePing( string user )
{
    string Out = m_CFG->GetString( "oh_208", "oh_208" );
    UTIL_Replace( Out, "$USER$", user);
    return Out;
}

string CLanguage :: UserTriedToPause( string user )
{
    string Out = m_CFG->GetString( "oh_209", "oh_209" );
    UTIL_Replace( Out, "$USER$", user);
    return Out;
}

string CLanguage :: UserTriedToPauseWarnng( )
{
    string Out = m_CFG->GetString( "oh_210", "oh_210" );
    return Out;
}

string CLanguage :: UserGorPunishedForPausing( string user )
{
    string Out = m_CFG->GetString( "oh_211", "oh_211" );
    UTIL_Replace( Out, "$USER$", user);
    return Out;
}

string CLanguage :: UserGorPunishedForSaving( string user )
{
    string Out = m_CFG->GetString( "oh_212", "oh_212" );
    UTIL_Replace( Out, "$USER$", user);
    return Out;
}

string CLanguage :: VoteHasBeenEnabledNotify( )
{
    string Out = m_CFG->GetString( "oh_213", "oh_213" );
    return Out;
}

string CLanguage :: VoteStartIsEnabledVotesRequired( string votes )
{
    string Out = m_CFG->GetString( "oh_214", "oh_214" );
    UTIL_Replace( Out, "$VOTES$", votes);
    return Out;
}

string CLanguage :: UserWasKickedForJoiningFromGarena( string user )
{
    string Out = m_CFG->GetString( "oh_215", "oh_215" );
    UTIL_Replace( Out, "$USER$", user);
    return Out;
}

string CLanguage :: UserJoinedFromGarena( string rank, string user, string realm, string room )
{
    string Out = m_CFG->GetString( "oh_216", "oh_216" );
    UTIL_Replace( Out, "$RANK$", rank);
    UTIL_Replace( Out, "$USER$", user);
    UTIL_Replace( Out, "$REALM$", realm);
    UTIL_Replace( Out, "$ROOM$", room);
    return Out;
}

string CLanguage :: UserJoined( string rank, string user, string realm )
{
    string Out = m_CFG->GetString( "oh_217", "oh_217" );
    UTIL_Replace( Out, "$RANK$", rank);
    UTIL_Replace( Out, "$USER$", user);
    UTIL_Replace( Out, "$REALM$", realm);
    return Out;
}

string CLanguage :: UserJoinedWithHighLeaveRate( string user, string perc)
{
    string Out = m_CFG->GetString( "oh_218", "oh_218" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$PERC$", perc);
    return Out;
}

string CLanguage :: PreventUserFromTransferResources( string user )
{
    string Out = m_CFG->GetString( "oh_219", "oh_219" );
    UTIL_Replace( Out, "$USER$", user);
    return Out;
}

string CLanguage :: Balance( string sent, string scou)
{
    string Out = m_CFG->GetString( "oh_220", "oh_220" );
    UTIL_Replace( Out, "$SENT$", sent );
    UTIL_Replace( Out, "$SCOU$", scou);
    return Out;
}

string CLanguage :: SpreadWithLockedPlayers( string spread, string locked)
{
    string Out = m_CFG->GetString( "oh_221", "oh_221" );
    UTIL_Replace( Out, "$SPREAD$", spread );
    UTIL_Replace( Out, "$LOCKED$", locked);
    return Out;
}


string CLanguage :: WinChance( string sent, string scou)
{
    string Out = m_CFG->GetString( "oh_222", "oh_222" );
    UTIL_Replace( Out, "$SENT$", sent );
    UTIL_Replace( Out, "$SCOU$", scou);
    return Out;
}

string CLanguage :: YouHaventVotedYet( )
{
    string Out = m_CFG->GetString( "oh_223", "oh_223" );
    return Out;
}

string CLanguage :: PlayersNotVerifiedYet( )
{
    string Out = m_CFG->GetString( "oh_224", "oh_224" );
    return Out;
}

string CLanguage :: PlayersNotVotedYet( )
{
    string Out = m_CFG->GetString( "oh_225", "oh_225" );
    return Out;
}

string CLanguage :: TimeLeftBeforeRandomMode( string left )
{
    string Out = m_CFG->GetString( "oh_226", "oh_226" );
    UTIL_Replace( Out, "$LEFT$", left);
    return Out;
}

string CLanguage :: TimeLeftBeforeTopVotedMode( string left )
{
    string Out = m_CFG->GetString( "oh_227", "oh_227" );
    UTIL_Replace( Out, "$LEFT$", left);
    return Out;
}

string CLanguage :: VoteModeHasStarted( )
{
    string Out = m_CFG->GetString( "oh_228", "oh_228" );
    return Out;
}

string CLanguage :: UserUsed1( string user, string thing)
{
    string Out = m_CFG->GetString( "oh_229", "oh_229" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$THING$", thing);
    return Out;
}

string CLanguage :: UserUsed2( string user, string thing)
{
    string Out = m_CFG->GetString( "oh_230", "oh_230" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$THING$", thing);
    return Out;
}

string CLanguage :: UserUsed3( string user, string thing)
{
    string Out = m_CFG->GetString( "oh_231", "oh_231" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$THING$", thing);
    return Out;
}

string CLanguage :: UserUsed4( string user, string thing)
{
    string Out = m_CFG->GetString( "oh_232", "oh_232" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$THING$", thing);
    return Out;
}

string CLanguage :: UserUsed5( string user, string thing)
{
    string Out = m_CFG->GetString( "oh_233", "oh_233" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$THING$", thing);
    return Out;
}

string CLanguage :: UserUsed6( string user, string thing)
{
    string Out = m_CFG->GetString( "oh_234", "oh_234" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$THING$", thing);
    return Out;
}

string CLanguage :: UserWasKickedForSlowDownloadRate( string user )
{
    string Out = m_CFG->GetString( "oh_235", "oh_235" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: UserWasAutomaticallyUnmuted( )
{
    string Out = m_CFG->GetString( "oh_236", "oh_236" );
    return Out;
}

string CLanguage :: SpamWarning( )
{
    string Out = m_CFG->GetString( "oh_237", "oh_237" );
    return Out;
}

string CLanguage :: UserWasMutedForReason( string user, string reason)
{
    string Out = m_CFG->GetString( "oh_238", "oh_238" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$REASON$", reason);
    return Out;
}

string CLanguage :: SpamWarning2( )
{
    string Out = m_CFG->GetString( "oh_239", "oh_239" );
    return Out;
}

string CLanguage :: UserIgnoerNotify( string user )
{
    string Out = m_CFG->GetString( "oh_240", "oh_240" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: FlameWarn( )
{
    string Out = m_CFG->GetString( "oh_241", "oh_241" );
    return Out;
}

string CLanguage :: FlameWarn2( )
{
    string Out = m_CFG->GetString( "oh_242", "oh_242" );
    return Out;
}

string CLanguage :: FlameWarn3( )
{
    string Out = m_CFG->GetString( "oh_243", "oh_243" );
    return Out;
}

string CLanguage :: RequestedSwapWithPlayer( string user )
{
    string Out = m_CFG->GetString( "oh_244", "oh_244" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}
string CLanguage :: PlayerRequestedSwapWithYou( string user )
{
    string Out = m_CFG->GetString( "oh_245", "oh_245" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}
string CLanguage :: PlayersSwapped( string user, string player )
{
    string Out = m_CFG->GetString( "oh_246", "oh_246" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$PLAYER$", player);
    return Out;
}
string CLanguage :: PlayerIsAlreadySwapping( string user )
{
    string Out = m_CFG->GetString( "oh_247", "oh_247" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}
string CLanguage :: YouAlreadyRequestedASwap( )
{
    string Out = m_CFG->GetString( "oh_248", "oh_248" );
    return Out;
}
string CLanguage :: ThePlayerAlreadyLeft( )
{
    string Out = m_CFG->GetString( "oh_249", "oh_249" );
    return Out;
}
string CLanguage :: TryingToSwapAcceptWhenRequested( )
{
    string Out = m_CFG->GetString( "oh_250", "oh_250" );
    return Out;
}

string CLanguage :: NoOneIsSwappingWithYou( )
{
    string Out = m_CFG->GetString( "oh_251", "oh_251" );
    return Out;
}

string CLanguage :: AbortedTheSwap( )
{
    string Out = m_CFG->GetString( "oh_252", "oh_252" );
    return Out;
}

string CLanguage :: UserAbortedSwapWithYou( string user )
{
    string Out = m_CFG->GetString( "oh_253", "oh_253" );
    UTIL_Replace( Out, "$USER$", user );
    return Out;
}

string CLanguage :: DisconnectedAt( )
{
    string Out = m_CFG->GetString( "oh_254", "oh_254" );
    return Out;
}

string CLanguage :: LeftAt( )
{
    string Out = m_CFG->GetString( "oh_255", "oh_255" );
    return Out;
}

string CLanguage :: SuccessfullyUsedCachedPass( )
{
    string Out = m_CFG->GetString( "oh_256", "oh_256" );
    return Out;
}

string CLanguage :: UseDrawToDrawGame( )
{
    string Out = m_CFG->GetString( "oh_257", "oh_257" );
    return Out;
}

string CLanguage :: Unknown( )
{
    string Out = m_CFG->GetString( "oh_258", "oh_258" );
    return Out;
}

string CLanguage :: LevelWasChanged( string level, string user )
{
    string Out = m_CFG->GetString( "oh_259", "oh_259" );
    UTIL_Replace( Out, "$USER$", user );
    UTIL_Replace( Out, "$LEVEL$", level );
    return Out;
}

string CLanguage :: CustomVoteKickReasons( )
{
    string Out = m_CFG->GetString( "oh_260", "oh_260" );
    return Out;
}

string CLanguage :: YourUniqueId( string id)
{
    string Out = m_CFG->GetString( "oh_261", "oh_261" );
    UTIL_Replace( Out, "$ID$", id );
    return Out;
}

string CLanguage :: RuleTags( )
{
    string Out = m_CFG->GetString( "oh_262", "oh_262" );
    return Out;
}

string CLanguage :: YourPingIsToday( )
{
    string Out = m_CFG->GetString( "oh_263", "oh_263" );
    return Out;
}

string CLanguage :: PossibleModesToVote( )
{
    string Out = m_CFG->GetString( "oh_264", "oh_264" );
    return Out;
}

string CLanguage :: VoteResult( )
{
    string Out = m_CFG->GetString( "oh_265", "oh_265" );
    return Out;
}

string CLanguage :: BanReasonIs( )
{
    string Out = m_CFG->GetString( "oh_266", "oh_266" );
    return Out;
}

string CLanguage :: UnableToJoin( )
{
    string Out = m_CFG->GetString( "oh_267", "oh_267" );
    return Out;
}

string CLanguage :: YouAreBanned( )
{
    string Out = m_CFG->GetString( "oh_268", "oh_268" );
    return Out;
}

string CLanguage :: BannedAt( uint32_t id, string name, string server )
{
    string Out = m_CFG->GetString( "oh_269", "oh_269" );
    UTIL_Replace( Out, "$ID$", UTIL_ToString(id) );
    UTIL_Replace( Out, "$NAME$", name );
    UTIL_Replace( Out, "$SERVER$", server );
    return Out;
}
string CLanguage :: BanDateAndRemain( string date, string remain )
{
    string Out = m_CFG->GetString( "oh_270", "oh_270" );
    UTIL_Replace( Out, "$DATE$", date );
    UTIL_Replace( Out, "$REMAIN$", remain );
    return Out;
}
string CLanguage :: BanDateWithoutRemain( string date )
{
    string Out = m_CFG->GetString( "oh_271", "oh_271" );
    UTIL_Replace( Out, "$DATE$", date );
    return Out;
}
string CLanguage :: BanGameName( string game )
{
    string Out = m_CFG->GetString( "oh_272", "oh_272" );
    UTIL_Replace( Out, "$GAMENAME$", game );
    return Out;
}
string CLanguage :: BanReason( string reason )
{
    string Out = m_CFG->GetString( "oh_273", "oh_273" );
    UTIL_Replace( Out, "$REASON$", reason );
    return Out;
}
string CLanguage :: YouCanAlsoUseGproxy( )
{
    string Out = m_CFG->GetString( "oh_274", "oh_274" );
    return Out;
}
string CLanguage :: JoinedInAGameWithAMinAmountOfGames( )
{
    string Out = m_CFG->GetString( "oh_275", "oh_275" );
    return Out;
}
string CLanguage :: YouRequireAtLeastGames( string min )
{
    string Out = m_CFG->GetString( "oh_276", "oh_276" );
        UTIL_Replace( Out, "$MIN$", min );
    return Out;
}
string CLanguage :: JoinedInAGameWithAMinAmountOfScore( )
{
    string Out = m_CFG->GetString( "oh_277", "oh_277" );
    return Out;
}
string CLanguage :: YouRequireToBeRegisteredOnStatsPage( )
{
    string Out = m_CFG->GetString( "oh_278", "oh_278" );
    return Out;
}
string CLanguage :: YouRequireToBeSafelisted( )
{
    string Out = m_CFG->GetString( "oh_279", "oh_279" );
    return Out;
}
string CLanguage :: GetMoreInformationOn( string site )
{
    string Out = m_CFG->GetString( "oh_280", "oh_280" );
        UTIL_Replace( Out, "$SITE$", site );
    return Out;
}
string CLanguage :: YouAreBannedFromLobby( )
{
    string Out = m_CFG->GetString( "oh_281", "oh_281" );
    return Out;
}
string CLanguage :: YouJoinedFromProxy( )
{
    string Out = m_CFG->GetString( "oh_282", "oh_282" );
    return Out;
}
string CLanguage :: YouJoinedFromABannedCountry( )
{
    string Out = m_CFG->GetString( "oh_283", "oh_283" );
    return Out;
}
string CLanguage :: YouAreForcedToUseGproxy( )
{
    string Out = m_CFG->GetString( "oh_284", "oh_284" );
    return Out;
}
string CLanguage :: YouWillBeKickedInAFewSeconds( )
{
    string Out = m_CFG->GetString( "oh_285", "oh_285" );
    return Out;
}
string CLanguage :: ReplayPrefix()
{
    string Out = m_CFG->GetString( "oh_286", "oh_286" );
    return Out;
}
string CLanguage :: YouHaveBeenDetectedAsFountainFarmer()
{
    string Out = m_CFG->GetString( "oh_287", "oh_287" );
    return Out;
}
string CLanguage :: LostConnectionPleaseTryLater()
{
    string Out = m_CFG->GetString( "oh_288", "oh_288" );
    return Out;
}
string CLanguage :: Month()
{
    string Out = m_CFG->GetString( "oh_289", "oh_289" );
    return Out;
}
string CLanguage :: Day()
{
    string Out = m_CFG->GetString( "oh_290", "oh_290" );
    return Out;
}

string CLanguage :: Hour()
{
    string Out = m_CFG->GetString( "oh_291", "oh_291" );
    return Out;
}

string CLanguage :: Minute()
{
    string Out = m_CFG->GetString( "oh_292", "oh_292" );
    return Out;
}
string CLanguage :: Announcement()
{
    string Out = m_CFG->GetString( "oh_293", "oh_293" );
    return Out;
}
string CLanguage::ErrorModeWasAlreadyForced()
{
	string Out = m_CFG->GetString("oh_294", "oh_294");
	return Out;
}
string CLanguage::ModeWasForcedTo(string mode)
{
	string Out = m_CFG->GetString("oh_295", "oh_295");
	UTIL_Replace(Out, "$MODE$", mode);
	return Out;
}
string CLanguage :: Ms()
{
       string Out = m_CFG->GetString( "oh_296", "oh_296" );
       return Out;
}
string CLanguage :: Country()
{
    string Out = m_CFG->GetString( "oh_297", "oh_297" );
    return Out;
}
string CLanguage :: Status()
{
    string Out = m_CFG->GetString( "oh_298", "oh_298" );
    return Out;
}
string CLanguage :: SpoofChecked()
{
    string Out = m_CFG->GetString( "oh_299", "oh_299" );
    return Out;
}
string CLanguage :: Yes()
{
    string Out = m_CFG->GetString( "oh_300", "oh_300" );
    return Out;
}
string CLanguage :: No()
{
    string Out = m_CFG->GetString( "oh_301", "oh_301" );
    return Out;
}
string CLanguage :: Realm()
{
    string Out = m_CFG->GetString( "oh_302", "oh_302" );
    return Out;
}
string CLanguage :: WelcomeMessage()
{
    string Out = m_CFG->GetString( "oh_303", "oh_303" );
    return Out;
}
