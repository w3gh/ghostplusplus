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

   CODE PORTED FROM THE ORIGINAL GHOST PROJECT: http://ghost.pwner.org/

*/

#include "ghost.h"
#include "util.h"
#include "config.h"
#include "language.h"

//
// CLanguage
//

CLanguage :: CLanguage( QString nCFGFile )
{
	m_CFG = new CConfig( );
	m_CFG->Read( nCFGFile );
}

CLanguage :: ~CLanguage( )
{
	delete m_CFG;
}

QString CLanguage :: UnableToCreateGameTryAnotherName( QString server, QString gamename )
{
	QString Out = m_CFG->GetString( "lang_0001", "lang_0001" );
	Out.replace("$SERVER$", server);
	Out.replace("$GAMENAME$", gamename);
	return Out;
}

QString CLanguage :: UserIsAlreadyAnAdmin( QString server, QString user )
{
	QString Out = m_CFG->GetString( "lang_0002", "lang_0002" );
	Out.replace("$SERVER$", server);
	Out.replace("$USER$", user);
	return Out;
}

QString CLanguage :: AddedUserToAdminDatabase( QString server, QString user )
{
	QString Out = m_CFG->GetString( "lang_0003", "lang_0003" );
	Out.replace("$SERVER$", server);
	Out.replace("$USER$", user);
	return Out;
}

QString CLanguage :: ErrorAddingUserToAdminDatabase( QString server, QString user )
{
	QString Out = m_CFG->GetString( "lang_0004", "lang_0004" );
	Out.replace("$SERVER$", server);
	Out.replace("$USER$", user);
	return Out;
}

QString CLanguage :: YouDontHaveAccessToThatCommand( )
{
	return m_CFG->GetString( "lang_0005", "lang_0005" );
}

QString CLanguage :: UserIsAlreadyBanned( QString server, QString victim )
{
	QString Out = m_CFG->GetString( "lang_0006", "lang_0006" );
	Out.replace("$SERVER$", server);
	Out.replace("$VICTIM$", victim);
	return Out;
}

QString CLanguage :: BannedUser( QString server, QString victim )
{
	QString Out = m_CFG->GetString( "lang_0007", "lang_0007" );
	Out.replace("$SERVER$", server);
	Out.replace("$VICTIM$", victim);
	return Out;
}

QString CLanguage :: ErrorBanningUser( QString server, QString victim )
{
	QString Out = m_CFG->GetString( "lang_0008", "lang_0008" );
	Out.replace("$SERVER$", server);
	Out.replace("$VICTIM$", victim);
	return Out;
}

QString CLanguage :: UserIsAnAdmin( QString server, QString user )
{
	QString Out = m_CFG->GetString( "lang_0009", "lang_0009" );
	Out.replace("$SERVER$", server);
	Out.replace("$USER$", user);
	return Out;
}

QString CLanguage :: UserIsNotAnAdmin( QString server, QString user )
{
	QString Out = m_CFG->GetString( "lang_0010", "lang_0010" );
	Out.replace("$SERVER$", server);
	Out.replace("$USER$", user);
	return Out;
}

QString CLanguage :: UserWasBannedOnByBecause( QString server, QString victim, QString date, QString admin, QString reason )
{
	QString Out = m_CFG->GetString( "lang_0011", "lang_0011" );
	Out.replace("$SERVER$", server);
	Out.replace("$VICTIM$", victim);
	Out.replace("$DATE$", date);
	Out.replace("$ADMIN$", admin);
	Out.replace("$REASON$", reason);
	return Out;
}

QString CLanguage :: UserIsNotBanned( QString server, QString victim )
{
	QString Out = m_CFG->GetString( "lang_0012", "lang_0012" );
	Out.replace("$SERVER$", server);
	Out.replace("$VICTIM$", victim);
	return Out;
}

QString CLanguage :: ThereAreNoAdmins( QString server )
{
	QString Out = m_CFG->GetString( "lang_0013", "lang_0013" );
	Out.replace("$SERVER$", server);
	return Out;
}

QString CLanguage :: ThereIsAdmin( QString server )
{
	QString Out = m_CFG->GetString( "lang_0014", "lang_0014" );
	Out.replace("$SERVER$", server);
	return Out;
}

QString CLanguage :: ThereAreAdmins( QString server, QString count )
{
	QString Out = m_CFG->GetString( "lang_0015", "lang_0015" );
	Out.replace("$SERVER$", server);
	Out.replace("$COUNT$", count);
	return Out;
}

QString CLanguage :: ThereAreNoBannedUsers( QString server )
{
	QString Out = m_CFG->GetString( "lang_0016", "lang_0016" );
	Out.replace("$SERVER$", server);
	return Out;
}

QString CLanguage :: ThereIsBannedUser( QString server )
{
	QString Out = m_CFG->GetString( "lang_0017", "lang_0017" );
	Out.replace("$SERVER$", server);
	return Out;
}

QString CLanguage :: ThereAreBannedUsers( QString server, QString count )
{
	QString Out = m_CFG->GetString( "lang_0018", "lang_0018" );
	Out.replace("$SERVER$", server);
	Out.replace("$COUNT$", count);
	return Out;
}

QString CLanguage :: YouCantDeleteTheRootAdmin( )
{
	return m_CFG->GetString( "lang_0019", "lang_0019" );
}

QString CLanguage :: DeletedUserFromAdminDatabase( QString server, QString user )
{
	QString Out = m_CFG->GetString( "lang_0020", "lang_0020" );
	Out.replace("$SERVER$", server);
	Out.replace("$USER$", user);
	return Out;
}

QString CLanguage :: ErrorDeletingUserFromAdminDatabase( QString server, QString user )
{
	QString Out = m_CFG->GetString( "lang_0021", "lang_0021" );
	Out.replace("$SERVER$", server);
	Out.replace("$USER$", user);
	return Out;
}

QString CLanguage :: UnbannedUser( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0022", "lang_0022" );
	Out.replace("$VICTIM$", victim);
	return Out;
}

QString CLanguage :: ErrorUnbanningUser( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0023", "lang_0023" );
	Out.replace("$VICTIM$", victim);
	return Out;
}

QString CLanguage :: GameNumberIs( QString number, QString description )
{
	QString Out = m_CFG->GetString( "lang_0024", "lang_0024" );
	Out.replace("$NUMBER$", number);
	Out.replace("$DESCRIPTION$", description);
	return Out;
}

QString CLanguage :: GameNumberDoesntExist( QString number )
{
	QString Out = m_CFG->GetString( "lang_0025", "lang_0025" );
	Out.replace("$NUMBER$", number);
	return Out;
}

QString CLanguage :: GameIsInTheLobby( QString description, QString current, QString max )
{
	QString Out = m_CFG->GetString( "lang_0026", "lang_0026" );
	Out.replace("$DESCRIPTION$", description);
	Out.replace("$CURRENT$", current);
	Out.replace("$MAX$", max);
	return Out;
}

QString CLanguage :: ThereIsNoGameInTheLobby( QString current, QString max )
{
	QString Out = m_CFG->GetString( "lang_0027", "lang_0027" );
	Out.replace("$CURRENT$", current);
	Out.replace("$MAX$", max);
	return Out;
}

QString CLanguage :: UnableToLoadConfigFilesOutside( )
{
	return m_CFG->GetString( "lang_0028", "lang_0028" );
}

QString CLanguage :: LoadingConfigFile( QString file )
{
	QString Out = m_CFG->GetString( "lang_0029", "lang_0029" );
	Out.replace("$FILE$", file);
	return Out;
}

QString CLanguage :: UnableToLoadConfigFileDoesntExist( QString file )
{
	QString Out = m_CFG->GetString( "lang_0030", "lang_0030" );
	Out.replace("$FILE$", file);
	return Out;
}

QString CLanguage :: CreatingPrivateGame( QString gamename, QString user )
{
	QString Out = m_CFG->GetString( "lang_0031", "lang_0031" );
	Out.replace("$GAMENAME$", gamename);
	Out.replace("$USER$", user);
	return Out;
}

QString CLanguage :: CreatingPublicGame( QString gamename, QString user )
{
	QString Out = m_CFG->GetString( "lang_0032", "lang_0032" );
	Out.replace("$GAMENAME$", gamename);
	Out.replace("$USER$", user);
	return Out;
}

QString CLanguage :: UnableToUnhostGameCountdownStarted( QString description )
{
	QString Out = m_CFG->GetString( "lang_0033", "lang_0033" );
	Out.replace("$DESCRIPTION$", description);
	return Out;
}

QString CLanguage :: UnhostingGame( QString description )
{
	QString Out = m_CFG->GetString( "lang_0034", "lang_0034" );
	Out.replace("$DESCRIPTION$", description);
	return Out;
}

QString CLanguage :: UnableToUnhostGameNoGameInLobby( )
{
	return m_CFG->GetString( "lang_0035", "lang_0035" );
}

QString CLanguage :: VersionAdmin( QString version )
{
	QString Out = m_CFG->GetString( "lang_0036", "lang_0036" );
	Out.replace("$VERSION$", version);
	return Out;
}

QString CLanguage :: VersionNotAdmin( QString version )
{
	QString Out = m_CFG->GetString( "lang_0037", "lang_0037" );
	Out.replace("$VERSION$", version);
	return Out;
}

QString CLanguage :: UnableToCreateGameAnotherGameInLobby( QString gamename, QString description )
{
	QString Out = m_CFG->GetString( "lang_0038", "lang_0038" );
	Out.replace("$GAMENAME$", gamename);
	Out.replace("$DESCRIPTION$", description);
	return Out;
}

QString CLanguage :: UnableToCreateGameMaxGamesReached( QString gamename, QString max )
{
	QString Out = m_CFG->GetString( "lang_0039", "lang_0039" );
	Out.replace("$GAMENAME$", gamename);
	Out.replace("$MAX$", max);
	return Out;
}

QString CLanguage :: GameIsOver( QString description )
{
	QString Out = m_CFG->GetString( "lang_0040", "lang_0040" );
	Out.replace("$DESCRIPTION$", description);
	return Out;
}

QString CLanguage :: SpoofCheckByReplying( )
{
	return m_CFG->GetString( "lang_0041", "lang_0041" );
}

QString CLanguage :: GameRefreshed( )
{
	return m_CFG->GetString( "lang_0042", "lang_0042" );
}

QString CLanguage :: SpoofPossibleIsAway( QString user )
{
	QString Out = m_CFG->GetString( "lang_0043", "lang_0043" );
	Out.replace("$USER$", user);
	return Out;
}

QString CLanguage :: SpoofPossibleIsUnavailable( QString user )
{
	QString Out = m_CFG->GetString( "lang_0044", "lang_0044" );
	Out.replace("$USER$", user);
	return Out;
}

QString CLanguage :: SpoofPossibleIsRefusingMessages( QString user )
{
	QString Out = m_CFG->GetString( "lang_0045", "lang_0045" );
	Out.replace("$USER$", user);
	return Out;
}

QString CLanguage :: SpoofDetectedIsNotInGame( QString user )
{
	QString Out = m_CFG->GetString( "lang_0046", "lang_0046" );
	Out.replace("$USER$", user);
	return Out;
}

QString CLanguage :: SpoofDetectedIsInPrivateChannel( QString user )
{
	QString Out = m_CFG->GetString( "lang_0047", "lang_0047" );
	Out.replace("$USER$", user);
	return Out;
}

QString CLanguage :: SpoofDetectedIsInAnotherGame( QString user )
{
	QString Out = m_CFG->GetString( "lang_0048", "lang_0048" );
	Out.replace("$USER$", user);
	return Out;
}

QString CLanguage :: CountDownAborted( )
{
	return m_CFG->GetString( "lang_0049", "lang_0049" );
}

QString CLanguage :: TryingToJoinTheGameButBanned( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0050", "lang_0050" );
	Out.replace("$VICTIM$", victim);
	return Out;
}

QString CLanguage :: UnableToBanNoMatchesFound( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0051", "lang_0051" );
	Out.replace("$VICTIM$", victim);
	return Out;
}

QString CLanguage :: PlayerWasBannedByPlayer( QString server, QString victim, QString user )
{
	QString Out = m_CFG->GetString( "lang_0052", "lang_0052" );
	Out.replace("$SERVER$", server);
	Out.replace("$VICTIM$", victim);
	Out.replace("$USER$", user);
	return Out;
}

QString CLanguage :: UnableToBanFoundMoreThanOneMatch( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0053", "lang_0053" );
	Out.replace("$VICTIM$", victim);
	return Out;
}

QString CLanguage :: AddedPlayerToTheHoldList( QString user )
{
	QString Out = m_CFG->GetString( "lang_0054", "lang_0054" );
	Out.replace("$USER$", user);
	return Out;
}

QString CLanguage :: UnableToKickNoMatchesFound( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0055", "lang_0055" );
	Out.replace("$VICTIM$", victim);
	return Out;
}

QString CLanguage :: UnableToKickFoundMoreThanOneMatch( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0056", "lang_0056" );
	Out.replace("$VICTIM$", victim);
	return Out;
}

QString CLanguage :: SettingLatencyToMinimum( QString min )
{
	QString Out = m_CFG->GetString( "lang_0057", "lang_0057" );
	Out.replace("$MIN$", min);
	return Out;
}

QString CLanguage :: SettingLatencyToMaximum( QString max )
{
	QString Out = m_CFG->GetString( "lang_0058", "lang_0058" );
	Out.replace("$MAX$", max);
	return Out;
}

QString CLanguage :: SettingLatencyTo( QString latency )
{
	QString Out = m_CFG->GetString( "lang_0059", "lang_0059" );
	Out.replace("$LATENCY$", latency);
	return Out;
}

QString CLanguage :: KickingPlayersWithPingsGreaterThan( QString total, QString ping )
{
	QString Out = m_CFG->GetString( "lang_0060", "lang_0060" );
	Out.replace("$TOTAL$", total);
	Out.replace("$PING$", ping);
	return Out;
}

QString CLanguage :: HasPlayedGamesWithThisBot( QString user, QString firstgame, QString lastgame, QString totalgames, QString avgloadingtime, QString avgstay )
{
	QString Out = m_CFG->GetString( "lang_0061", "lang_0061" );
	Out.replace("$USER$", user);
	Out.replace("$FIRSTGAME$", firstgame);
	Out.replace("$LASTGAME$", lastgame);
	Out.replace("$TOTALGAMES$", totalgames);
	Out.replace("$AVGLOADINGTIME$", avgloadingtime);
	Out.replace("$AVGSTAY$", avgstay);
	return Out;
}

QString CLanguage :: HasntPlayedGamesWithThisBot( QString user )
{
	QString Out = m_CFG->GetString( "lang_0062", "lang_0062" );
	Out.replace("$USER$", user);
	return Out;
}

QString CLanguage :: AutokickingPlayerForExcessivePing( QString victim, QString ping )
{
	QString Out = m_CFG->GetString( "lang_0063", "lang_0063" );
	Out.replace("$VICTIM$", victim);
	Out.replace("$PING$", ping);
	return Out;
}

QString CLanguage :: SpoofCheckAcceptedFor( QString server, QString user )
{
	QString Out = m_CFG->GetString( "lang_0064", "lang_0064" );
	Out.replace("$SERVER$", server);
	Out.replace("$USER$", user);
	return Out;
}

QString CLanguage :: PlayersNotYetSpoofChecked( QString notspoofchecked )
{
	QString Out = m_CFG->GetString( "lang_0065", "lang_0065" );
	Out.replace("$NOTSPOOFCHECKED$", notspoofchecked);
	return Out;
}

QString CLanguage :: ManuallySpoofCheckByWhispering( QString hostname )
{
	QString Out = m_CFG->GetString( "lang_0066", "lang_0066" );
	Out.replace("$HOSTNAME$", hostname);
	return Out;
}

QString CLanguage :: SpoofCheckByWhispering( QString hostname )
{
	QString Out = m_CFG->GetString( "lang_0067", "lang_0067" );
	Out.replace("$HOSTNAME$", hostname);
	return Out;
}

QString CLanguage :: EveryoneHasBeenSpoofChecked( )
{
	return m_CFG->GetString( "lang_0068", "lang_0068" );
}

QString CLanguage :: PlayersNotYetPinged( QString notpinged )
{
	QString Out = m_CFG->GetString( "lang_0069", "lang_0069" );
	Out.replace("$NOTPINGED$", notpinged);
	return Out;
}

QString CLanguage :: EveryoneHasBeenPinged( )
{
	return m_CFG->GetString( "lang_0070", "lang_0070" );
}

QString CLanguage :: ShortestLoadByPlayer( QString user, QString loadingtime )
{
	QString Out = m_CFG->GetString( "lang_0071", "lang_0071" );
	Out.replace("$USER$", user);
	Out.replace("$LOADINGTIME$", loadingtime);
	return Out;
}

QString CLanguage :: LongestLoadByPlayer( QString user, QString loadingtime )
{
	QString Out = m_CFG->GetString( "lang_0072", "lang_0072" );
	Out.replace("$USER$", user);
	Out.replace("$LOADINGTIME$", loadingtime);
	return Out;
}

QString CLanguage :: YourLoadingTimeWas( QString loadingtime )
{
	QString Out = m_CFG->GetString( "lang_0073", "lang_0073" );
	Out.replace("$LOADINGTIME$", loadingtime);
	return Out;
}

QString CLanguage :: HasPlayedDotAGamesWithThisBot( QString user, QString totalgames, QString totalwins, QString totallosses, QString totalkills, QString totaldeaths, QString totalcreepkills, QString totalcreepdenies, QString totalassists, QString totalneutralkills, QString totaltowerkills, QString totalraxkills, QString totalcourierkills, QString avgkills, QString avgdeaths, QString avgcreepkills, QString avgcreepdenies, QString avgassists, QString avgneutralkills, QString avgtowerkills, QString avgraxkills, QString avgcourierkills )
{
	QString Out = m_CFG->GetString( "lang_0074", "lang_0074" );
	Out.replace("$USER$", user);
	Out.replace("$TOTALGAMES$", totalgames);
	Out.replace("$TOTALWINS$", totalwins);
	Out.replace("$TOTALLOSSES$", totallosses);
	Out.replace("$TOTALKILLS$", totalkills);
	Out.replace("$TOTALDEATHS$", totaldeaths);
	Out.replace("$TOTALCREEPKILLS$", totalcreepkills);
	Out.replace("$TOTALCREEPDENIES$", totalcreepdenies);
	Out.replace("$TOTALASSISTS$", totalassists);
	Out.replace("$TOTALNEUTRALKILLS$", totalneutralkills);
	Out.replace("$TOTALTOWERKILLS$", totaltowerkills);
	Out.replace("$TOTALRAXKILLS$", totalraxkills);
	Out.replace("$TOTALCOURIERKILLS$", totalcourierkills);
	Out.replace("$AVGKILLS$", avgkills);
	Out.replace("$AVGDEATHS$", avgdeaths);
	Out.replace("$AVGCREEPKILLS$", avgcreepkills);
	Out.replace("$AVGCREEPDENIES$", avgcreepdenies);
	Out.replace("$AVGASSISTS$", avgassists);
	Out.replace("$AVGNEUTRALKILLS$", avgneutralkills);
	Out.replace("$AVGTOWERKILLS$", avgtowerkills);
	Out.replace("$AVGRAXKILLS$", avgraxkills);
	Out.replace("$AVGCOURIERKILLS$", avgcourierkills);
	return Out;
}

QString CLanguage :: HasntPlayedDotAGamesWithThisBot( QString user )
{
	QString Out = m_CFG->GetString( "lang_0075", "lang_0075" );
	Out.replace("$USER$", user);
	return Out;
}

QString CLanguage :: WasKickedForReservedPlayer( QString reserved )
{
	QString Out = m_CFG->GetString( "lang_0076", "lang_0076" );
	Out.replace("$RESERVED$", reserved);
	return Out;
}

QString CLanguage :: WasKickedForOwnerPlayer( QString owner )
{
	QString Out = m_CFG->GetString( "lang_0077", "lang_0077" );
	Out.replace("$OWNER$", owner);
	return Out;
}

QString CLanguage :: WasKickedByPlayer( QString user )
{
	QString Out = m_CFG->GetString( "lang_0078", "lang_0078" );
	Out.replace("$USER$", user);
	return Out;
}

QString CLanguage :: HasLostConnectionPlayerError( QString error )
{
	QString Out = m_CFG->GetString( "lang_0079", "lang_0079" );
	Out.replace("$ERROR$", error);
	return Out;
}

QString CLanguage :: HasLostConnectionSocketError( QString error )
{
	QString Out = m_CFG->GetString( "lang_0080", "lang_0080" );
	Out.replace("$ERROR$", error);
	return Out;
}

QString CLanguage :: HasLostConnectionClosedByRemoteHost( )
{
	return m_CFG->GetString( "lang_0081", "lang_0081" );
}

QString CLanguage :: HasLeftVoluntarily( )
{
	return m_CFG->GetString( "lang_0082", "lang_0082" );
}

QString CLanguage :: EndingGame( QString description )
{
	QString Out = m_CFG->GetString( "lang_0083", "lang_0083" );
	Out.replace("$DESCRIPTION$", description);
	return Out;
}

QString CLanguage :: HasLostConnectionTimedOut( )
{
	return m_CFG->GetString( "lang_0084", "lang_0084" );
}

QString CLanguage :: GlobalChatMuted( )
{
	return m_CFG->GetString( "lang_0085", "lang_0085" );
}

QString CLanguage :: GlobalChatUnmuted( )
{
	return m_CFG->GetString( "lang_0086", "lang_0086" );
}

QString CLanguage :: ShufflingPlayers( )
{
	return m_CFG->GetString( "lang_0087", "lang_0087" );
}

QString CLanguage :: UnableToLoadConfigFileGameInLobby( )
{
	return m_CFG->GetString( "lang_0088", "lang_0088" );
}

QString CLanguage :: PlayersStillDownloading( QString stilldownloading )
{
	QString Out = m_CFG->GetString( "lang_0089", "lang_0089" );
	Out.replace("$STILLDOWNLOADING$", stilldownloading);
	return Out;
}

QString CLanguage :: RefreshMessagesEnabled( )
{
	return m_CFG->GetString( "lang_0090", "lang_0090" );
}

QString CLanguage :: RefreshMessagesDisabled( )
{
	return m_CFG->GetString( "lang_0091", "lang_0091" );
}

QString CLanguage :: AtLeastOneGameActiveUseForceToShutdown( )
{
	return m_CFG->GetString( "lang_0092", "lang_0092" );
}

QString CLanguage :: CurrentlyLoadedMapCFGIs( QString mapcfg )
{
	QString Out = m_CFG->GetString( "lang_0093", "lang_0093" );
	Out.replace("$MAPCFG$", mapcfg);
	return Out;
}

QString CLanguage :: LaggedOutDroppedByAdmin( )
{
	return m_CFG->GetString( "lang_0094", "lang_0094" );
}

QString CLanguage :: LaggedOutDroppedByVote( )
{
	return m_CFG->GetString( "lang_0095", "lang_0095" );
}

QString CLanguage :: PlayerVotedToDropLaggers( QString user )
{
	QString Out = m_CFG->GetString( "lang_0096", "lang_0096" );
	Out.replace("$USER$", user);
	return Out;
}

QString CLanguage :: LatencyIs( QString latency )
{
	QString Out = m_CFG->GetString( "lang_0097", "lang_0097" );
	Out.replace("$LATENCY$", latency);
	return Out;
}

QString CLanguage :: SyncLimitIs( QString synclimit )
{
	QString Out = m_CFG->GetString( "lang_0098", "lang_0098" );
	Out.replace("$SYNCLIMIT$", synclimit);
	return Out;
}

QString CLanguage :: SettingSyncLimitToMinimum( QString min )
{
	QString Out = m_CFG->GetString( "lang_0099", "lang_0099" );
	Out.replace("$MIN$", min);
	return Out;
}

QString CLanguage :: SettingSyncLimitToMaximum( QString max )
{
	QString Out = m_CFG->GetString( "lang_0100", "lang_0100" );
	Out.replace("$MAX$", max);
	return Out;
}

QString CLanguage :: SettingSyncLimitTo( QString synclimit )
{
	QString Out = m_CFG->GetString( "lang_0101", "lang_0101" );
	Out.replace("$SYNCLIMIT$", synclimit);
	return Out;
}

QString CLanguage :: UnableToCreateGameNotLoggedIn( QString gamename )
{
	QString Out = m_CFG->GetString( "lang_0102", "lang_0102" );
	Out.replace("$GAMENAME$", gamename);
	return Out;
}

QString CLanguage :: AdminLoggedIn( )
{
	return m_CFG->GetString( "lang_0103", "lang_0103" );
}

QString CLanguage :: AdminInvalidPassword( QString attempt )
{
	QString Out = m_CFG->GetString( "lang_0104", "lang_0104" );
	Out.replace("$ATTEMPT$", attempt);
	return Out;
}

QString CLanguage :: ConnectingToBNET( QString server )
{
	QString Out = m_CFG->GetString( "lang_0105", "lang_0105" );
	Out.replace("$SERVER$", server);
	return Out;
}

QString CLanguage :: ConnectedToBNET( QString server )
{
	QString Out = m_CFG->GetString( "lang_0106", "lang_0106" );
	Out.replace("$SERVER$", server);
	return Out;
}

QString CLanguage :: DisconnectedFromBNET( QString server )
{
	QString Out = m_CFG->GetString( "lang_0107", "lang_0107" );
	Out.replace("$SERVER$", server);
	return Out;
}

QString CLanguage :: LoggedInToBNET( QString server )
{
	QString Out = m_CFG->GetString( "lang_0108", "lang_0108" );
	Out.replace("$SERVER$", server);
	return Out;
}

QString CLanguage :: BNETGameHostingSucceeded( QString server )
{
	QString Out = m_CFG->GetString( "lang_0109", "lang_0109" );
	Out.replace("$SERVER$", server);
	return Out;
}

QString CLanguage :: BNETGameHostingFailed( QString server, QString gamename )
{
	QString Out = m_CFG->GetString( "lang_0110", "lang_0110" );
	Out.replace("$SERVER$", server);
	Out.replace("$GAMENAME$", gamename);
	return Out;
}

QString CLanguage :: ConnectingToBNETTimedOut( QString server )
{
	QString Out = m_CFG->GetString( "lang_0111", "lang_0111" );
	Out.replace("$SERVER$", server);
	return Out;
}

QString CLanguage :: PlayerDownloadedTheMap( QString user, QString seconds, QString rate )
{
	QString Out = m_CFG->GetString( "lang_0112", "lang_0112" );
	Out.replace("$USER$", user);
	Out.replace("$SECONDS$", seconds);
	Out.replace("$RATE$", rate);
	return Out;
}

QString CLanguage :: UnableToCreateGameNameTooLong( QString gamename )
{
	QString Out = m_CFG->GetString( "lang_0113", "lang_0113" );
	Out.replace("$GAMENAME$", gamename);
	return Out;
}

QString CLanguage :: SettingGameOwnerTo( QString owner )
{
	QString Out = m_CFG->GetString( "lang_0114", "lang_0114" );
	Out.replace("$OWNER$", owner);
	return Out;
}

QString CLanguage :: TheGameIsLocked( )
{
	return m_CFG->GetString( "lang_0115", "lang_0115" );
}

QString CLanguage :: GameLocked( )
{
	return m_CFG->GetString( "lang_0116", "lang_0116" );
}

QString CLanguage :: GameUnlocked( )
{
	return m_CFG->GetString( "lang_0117", "lang_0117" );
}

QString CLanguage :: UnableToStartDownloadNoMatchesFound( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0118", "lang_0118" );
	Out.replace("$VICTIM$", victim);
	return Out;
}

QString CLanguage :: UnableToStartDownloadFoundMoreThanOneMatch( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0119", "lang_0119" );
	Out.replace("$VICTIM$", victim);
	return Out;
}

QString CLanguage :: UnableToSetGameOwner( QString owner )
{
	QString Out = m_CFG->GetString( "lang_0120", "lang_0120" );
	Out.replace("$OWNER$", owner);
	return Out;
}

QString CLanguage :: UnableToCheckPlayerNoMatchesFound( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0121", "lang_0121" );
	Out.replace("$VICTIM$", victim);
	return Out;
}

QString CLanguage :: CheckedPlayer( QString victim, QString ping, QString from, QString admin, QString owner, QString spoofed, QString spoofedrealm, QString reserved )
{
	QString Out = m_CFG->GetString( "lang_0122", "lang_0122" );
	Out.replace("$VICTIM$", victim);
	Out.replace("$PING$", ping);
	Out.replace("$FROM$", from);
	Out.replace("$ADMIN$", admin);
	Out.replace("$OWNER$", owner);
	Out.replace("$SPOOFED$", spoofed);
	Out.replace("$SPOOFEDREALM$", spoofedrealm);
	Out.replace("$RESERVED$", reserved);
	return Out;
}

QString CLanguage :: UnableToCheckPlayerFoundMoreThanOneMatch( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0123", "lang_0123" );
	Out.replace("$VICTIM$", victim);
	return Out;
}

QString CLanguage :: TheGameIsLockedBNET( )
{
	return m_CFG->GetString( "lang_0124", "lang_0124" );
}

QString CLanguage :: UnableToCreateGameDisabled( QString gamename )
{
	QString Out = m_CFG->GetString( "lang_0125", "lang_0125" );
	Out.replace("$GAMENAME$", gamename);
	return Out;
}

QString CLanguage :: BotDisabled( )
{
	return m_CFG->GetString( "lang_0126", "lang_0126" );
}

QString CLanguage :: BotEnabled( )
{
	return m_CFG->GetString( "lang_0127", "lang_0127" );
}

QString CLanguage :: UnableToCreateGameInvalidMap( QString gamename )
{
	QString Out = m_CFG->GetString( "lang_0128", "lang_0128" );
	Out.replace("$GAMENAME$", gamename);
	return Out;
}

QString CLanguage :: WaitingForPlayersBeforeAutoStart( QString players, QString playersleft )
{
	QString Out = m_CFG->GetString( "lang_0129", "lang_0129" );
	Out.replace("$PLAYERS$", players);
	Out.replace("$PLAYERSLEFT$", playersleft);
	return Out;
}

QString CLanguage :: AutoStartDisabled( )
{
	return m_CFG->GetString( "lang_0130", "lang_0130" );
}

QString CLanguage :: AutoStartEnabled( QString players )
{
	QString Out = m_CFG->GetString( "lang_0131", "lang_0131" );
	Out.replace("$PLAYERS$", players);
	return Out;
}

QString CLanguage :: AnnounceMessageEnabled( )
{
	return m_CFG->GetString( "lang_0132", "lang_0132" );
}

QString CLanguage :: AnnounceMessageDisabled( )
{
	return m_CFG->GetString( "lang_0133", "lang_0133" );
}

QString CLanguage :: AutoHostEnabled( )
{
	return m_CFG->GetString( "lang_0134", "lang_0134" );
}

QString CLanguage :: AutoHostDisabled( )
{
	return m_CFG->GetString( "lang_0135", "lang_0135" );
}

QString CLanguage :: UnableToLoadSaveGamesOutside( )
{
	return m_CFG->GetString( "lang_0136", "lang_0136" );
}

QString CLanguage :: UnableToLoadSaveGameGameInLobby( )
{
	return m_CFG->GetString( "lang_0137", "lang_0137" );
}

QString CLanguage :: LoadingSaveGame( QString file )
{
	QString Out = m_CFG->GetString( "lang_0138", "lang_0138" );
	Out.replace("$FILE$", file);
	return Out;
}

QString CLanguage :: UnableToLoadSaveGameDoesntExist( QString file )
{
	QString Out = m_CFG->GetString( "lang_0139", "lang_0139" );
	Out.replace("$FILE$", file);
	return Out;
}

QString CLanguage :: UnableToCreateGameInvalidSaveGame( QString gamename )
{
	QString Out = m_CFG->GetString( "lang_0140", "lang_0140" );
	Out.replace("$GAMENAME$", gamename);
	return Out;
}

QString CLanguage :: UnableToCreateGameSaveGameMapMismatch( QString gamename )
{
	QString Out = m_CFG->GetString( "lang_0141", "lang_0141" );
	Out.replace("$GAMENAME$", gamename);
	return Out;
}

QString CLanguage :: AutoSaveEnabled( )
{
	return m_CFG->GetString( "lang_0142", "lang_0142" );
}

QString CLanguage :: AutoSaveDisabled( )
{
	return m_CFG->GetString( "lang_0143", "lang_0143" );
}

QString CLanguage :: DesyncDetected( )
{
	return m_CFG->GetString( "lang_0144", "lang_0144" );
}

QString CLanguage :: UnableToMuteNoMatchesFound( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0145", "lang_0145" );
	Out.replace("$VICTIM$", victim);
	return Out;
}

QString CLanguage :: MutedPlayer( QString victim, QString user )
{
	QString Out = m_CFG->GetString( "lang_0146", "lang_0146" );
	Out.replace("$VICTIM$", victim);
	Out.replace("$USER$", user);
	return Out;
}

QString CLanguage :: UnmutedPlayer( QString victim, QString user )
{
	QString Out = m_CFG->GetString( "lang_0147", "lang_0147" );
	Out.replace("$VICTIM$", victim);
	Out.replace("$USER$", user);
	return Out;
}

QString CLanguage :: UnableToMuteFoundMoreThanOneMatch( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0148", "lang_0148" );
	Out.replace("$VICTIM$", victim);
	return Out;
}

QString CLanguage :: PlayerIsSavingTheGame( QString player )
{
	QString Out = m_CFG->GetString( "lang_0149", "lang_0149" );
	Out.replace("$PLAYER$", player);
	return Out;
}

QString CLanguage :: UpdatingClanList( )
{
	return m_CFG->GetString( "lang_0150", "lang_0150" );
}

QString CLanguage :: UpdatingFriendsList( )
{
	return m_CFG->GetString( "lang_0151", "lang_0151" );
}

QString CLanguage :: MultipleIPAddressUsageDetected( QString player, QString others )
{
	QString Out = m_CFG->GetString( "lang_0152", "lang_0152" );
	Out.replace("$PLAYER$", player);
	Out.replace("$OTHERS$", others);
	return Out;
}

QString CLanguage :: UnableToVoteKickAlreadyInProgress( )
{
	return m_CFG->GetString( "lang_0153", "lang_0153" );
}

QString CLanguage :: UnableToVoteKickNotEnoughPlayers( )
{
	return m_CFG->GetString( "lang_0154", "lang_0154" );
}

QString CLanguage :: UnableToVoteKickNoMatchesFound( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0155", "lang_0155" );
	Out.replace("$VICTIM$", victim);
	return Out;
}

QString CLanguage :: UnableToVoteKickPlayerIsReserved( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0156", "lang_0156" );
	Out.replace("$VICTIM$", victim);
	return Out;
}

QString CLanguage :: StartedVoteKick( QString victim, QString user, QString votesneeded )
{
	QString Out = m_CFG->GetString( "lang_0157", "lang_0157" );
	Out.replace("$VICTIM$", victim);
	Out.replace("$USER$", user);
	Out.replace("$VOTESNEEDED$", votesneeded);
	return Out;
}

QString CLanguage :: UnableToVoteKickFoundMoreThanOneMatch( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0158", "lang_0158" );
	Out.replace("$VICTIM$", victim);
	return Out;
}

QString CLanguage :: VoteKickPassed( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0159", "lang_0159" );
	Out.replace("$VICTIM$", victim);
	return Out;
}

QString CLanguage :: ErrorVoteKickingPlayer( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0160", "lang_0160" );
	Out.replace("$VICTIM$", victim);
	return Out;
}

QString CLanguage :: VoteKickAcceptedNeedMoreVotes( QString victim, QString user, QString votes )
{
	QString Out = m_CFG->GetString( "lang_0161", "lang_0161" );
	Out.replace("$VICTIM$", victim);
	Out.replace("$USER$", user);
	Out.replace("$VOTES$", votes);
	return Out;
}

QString CLanguage :: VoteKickCancelled( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0162", "lang_0162" );
	Out.replace("$VICTIM$", victim);
	return Out;
}

QString CLanguage :: VoteKickExpired( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0163", "lang_0163" );
	Out.replace("$VICTIM$", victim);
	return Out;
}

QString CLanguage :: WasKickedByVote( )
{
	return m_CFG->GetString( "lang_0164", "lang_0164" );
}

QString CLanguage :: TypeYesToVote( QString commandtrigger )
{
	QString Out = m_CFG->GetString( "lang_0165", "lang_0165" );
	Out.replace("$COMMANDTRIGGER$", commandtrigger);
	return Out;
}

QString CLanguage :: PlayersNotYetPingedAutoStart( QString notpinged )
{
	QString Out = m_CFG->GetString( "lang_0166", "lang_0166" );
	Out.replace("$NOTPINGED$", notpinged);
	return Out;
}

QString CLanguage :: WasKickedForNotSpoofChecking( )
{
	return m_CFG->GetString( "lang_0167", "lang_0167" );
}

QString CLanguage :: WasKickedForHavingFurthestScore( QString score, QString average )
{
	QString Out = m_CFG->GetString( "lang_0168", "lang_0168" );
	Out.replace("$SCORE$", score);
	Out.replace("$AVERAGE$", average);
	return Out;
}

QString CLanguage :: PlayerHasScore( QString player, QString score )
{
	QString Out = m_CFG->GetString( "lang_0169", "lang_0169" );
	Out.replace("$PLAYER$", player);
	Out.replace("$SCORE$", score);
	return Out;
}

QString CLanguage :: RatedPlayersSpread( QString rated, QString total, QString spread )
{
	QString Out = m_CFG->GetString( "lang_0170", "lang_0170" );
	Out.replace("$RATED$", rated);
	Out.replace("$TOTAL$", total);
	Out.replace("$SPREAD$", spread);
	return Out;
}

QString CLanguage :: ErrorListingMaps( )
{
	return m_CFG->GetString( "lang_0171", "lang_0171" );
}

QString CLanguage :: FoundMaps( QString maps )
{
	QString Out = m_CFG->GetString( "lang_0172", "lang_0172" );
	Out.replace("$MAPS$", maps);
	return Out;
}

QString CLanguage :: NoMapsFound( )
{
	return m_CFG->GetString( "lang_0173", "lang_0173" );
}

QString CLanguage :: ErrorListingMapConfigs( )
{
	return m_CFG->GetString( "lang_0174", "lang_0174" );
}

QString CLanguage :: FoundMapConfigs( QString mapconfigs )
{
	QString Out = m_CFG->GetString( "lang_0175", "lang_0175" );
	Out.replace("$MAPCONFIGS$", mapconfigs);
	return Out;
}

QString CLanguage :: NoMapConfigsFound( )
{
	return m_CFG->GetString( "lang_0176", "lang_0176" );
}

QString CLanguage :: PlayerFinishedLoading( QString user )
{
	QString Out = m_CFG->GetString( "lang_0177", "lang_0177" );
	Out.replace("$USER$", user);
	return Out;
}

QString CLanguage :: PleaseWaitPlayersStillLoading( )
{
	return m_CFG->GetString( "lang_0178", "lang_0178" );
}

QString CLanguage :: MapDownloadsDisabled( )
{
	return m_CFG->GetString( "lang_0179", "lang_0179" );
}

QString CLanguage :: MapDownloadsEnabled( )
{
	return m_CFG->GetString( "lang_0180", "lang_0180" );
}

QString CLanguage :: MapDownloadsConditional( )
{
	return m_CFG->GetString( "lang_0181", "lang_0181" );
}

QString CLanguage :: SettingHCL( QString HCL )
{
	QString Out = m_CFG->GetString( "lang_0182", "lang_0182" );
	Out.replace("$HCL$", HCL);
	return Out;
}

QString CLanguage :: UnableToSetHCLInvalid( )
{
	return m_CFG->GetString( "lang_0183", "lang_0183" );
}

QString CLanguage :: UnableToSetHCLTooLong( )
{
	return m_CFG->GetString( "lang_0184", "lang_0184" );
}

QString CLanguage :: TheHCLIs( QString HCL )
{
	QString Out = m_CFG->GetString( "lang_0185", "lang_0185" );
	Out.replace("$HCL$", HCL);
	return Out;
}

QString CLanguage :: TheHCLIsTooLongUseForceToStart( )
{
	return m_CFG->GetString( "lang_0186", "lang_0186" );
}

QString CLanguage :: ClearingHCL( )
{
	return m_CFG->GetString( "lang_0187", "lang_0187" );
}

QString CLanguage :: TryingToRehostAsPrivateGame( QString gamename )
{
	QString Out = m_CFG->GetString( "lang_0188", "lang_0188" );
	Out.replace("$GAMENAME$", gamename);
	return Out;
}

QString CLanguage :: TryingToRehostAsPublicGame( QString gamename )
{
	QString Out = m_CFG->GetString( "lang_0189", "lang_0189" );
	Out.replace("$GAMENAME$", gamename);
	return Out;
}

QString CLanguage :: RehostWasSuccessful( )
{
	return m_CFG->GetString( "lang_0190", "lang_0190" );
}

QString CLanguage :: TryingToJoinTheGameButBannedByName( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0191", "lang_0191" );
	Out.replace("$VICTIM$", victim);
	return Out;
}

QString CLanguage :: TryingToJoinTheGameButBannedByIP( QString victim, QString ip, QString bannedname )
{
	QString Out = m_CFG->GetString( "lang_0192", "lang_0192" );
	Out.replace("$VICTIM$", victim);
	Out.replace("$IP$", ip);
	Out.replace("$BANNEDNAME$", bannedname);
	return Out;
}

QString CLanguage :: HasBannedName( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0193", "lang_0193" );
	Out.replace("$VICTIM$", victim);
	return Out;
}

QString CLanguage :: HasBannedIP( QString victim, QString ip, QString bannedname )
{
	QString Out = m_CFG->GetString( "lang_0194", "lang_0194" );
	Out.replace("$VICTIM$", victim);
	Out.replace("$IP$", ip);
	Out.replace("$BANNEDNAME$", bannedname);
	return Out;
}

QString CLanguage :: PlayersInGameState( QString number, QString players )
{
	QString Out = m_CFG->GetString( "lang_0195", "lang_0195" );
	Out.replace("$NUMBER$", number);
	Out.replace("$PLAYERS$", players);
	return Out;
}

QString CLanguage :: ValidServers( QString servers )
{
	QString Out = m_CFG->GetString( "lang_0196", "lang_0196" );
	Out.replace("$SERVERS$", servers);
	return Out;
}

QString CLanguage :: TeamCombinedScore( QString team, QString score )
{
	QString Out = m_CFG->GetString( "lang_0197", "lang_0197" );
	Out.replace("$TEAM$", team);
	Out.replace("$SCORE$", score);
	return Out;
}

QString CLanguage :: BalancingSlotsCompleted( )
{
	return m_CFG->GetString( "lang_0198", "lang_0198" );
}

QString CLanguage :: PlayerWasKickedForFurthestScore( QString name, QString score, QString average )
{
	QString Out = m_CFG->GetString( "lang_0199", "lang_0199" );
	Out.replace("$NAME$", name);
	Out.replace("$SCORE$", score);
	Out.replace("$AVERAGE$", average);
	return Out;
}

QString CLanguage :: LocalAdminMessagesEnabled( )
{
	return m_CFG->GetString( "lang_0200", "lang_0200" );
}

QString CLanguage :: LocalAdminMessagesDisabled( )
{
	return m_CFG->GetString( "lang_0201", "lang_0201" );
}

QString CLanguage :: WasDroppedDesync( )
{
	return m_CFG->GetString( "lang_0202", "lang_0202" );
}

QString CLanguage :: WasKickedForHavingLowestScore( QString score )
{
	QString Out = m_CFG->GetString( "lang_0203", "lang_0203" );
	Out.replace("$SCORE$", score);
	return Out;
}

QString CLanguage :: PlayerWasKickedForLowestScore( QString name, QString score )
{
	QString Out = m_CFG->GetString( "lang_0204", "lang_0204" );
	Out.replace("$NAME$", name);
	Out.replace("$SCORE$", score);
	return Out;
}

QString CLanguage :: ReloadingConfigurationFiles( )
{
	return m_CFG->GetString( "lang_0205", "lang_0205" );
}

QString CLanguage :: CountDownAbortedSomeoneLeftRecently( )
{
	return m_CFG->GetString( "lang_0206", "lang_0206" );
}

QString CLanguage :: UnableToCreateGameMustEnforceFirst( QString gamename )
{
	QString Out = m_CFG->GetString( "lang_0207", "lang_0207" );
	Out.replace("$GAMENAME$", gamename);
	return Out;
}

QString CLanguage :: UnableToLoadReplaysOutside( )
{
	return m_CFG->GetString( "lang_0208", "lang_0208" );
}

QString CLanguage :: LoadingReplay( QString file )
{
	QString Out = m_CFG->GetString( "lang_0209", "lang_0209" );
	Out.replace("$FILE$", file);
	return Out;
}

QString CLanguage :: UnableToLoadReplayDoesntExist( QString file )
{
	QString Out = m_CFG->GetString( "lang_0210", "lang_0210" );
	Out.replace("$FILE$", file);
	return Out;
}

QString CLanguage :: CommandTrigger( QString trigger )
{
	QString Out = m_CFG->GetString( "lang_0211", "lang_0211" );
	Out.replace("$TRIGGER$", trigger);
	return Out;
}

QString CLanguage :: CantEndGameOwnerIsStillPlaying( QString owner )
{
	QString Out = m_CFG->GetString( "lang_0212", "lang_0212" );
	Out.replace("$OWNER$", owner);
	return Out;
}

QString CLanguage :: CantUnhostGameOwnerIsPresent( QString owner )
{
	QString Out = m_CFG->GetString( "lang_0213", "lang_0213" );
	Out.replace("$OWNER$", owner);
	return Out;
}

QString CLanguage :: WasAutomaticallyDroppedAfterSeconds( QString seconds )
{
	QString Out = m_CFG->GetString( "lang_0214", "lang_0214" );
	Out.replace("$SECONDS$", seconds);
	return Out;
}

QString CLanguage :: HasLostConnectionTimedOutGProxy( )
{
	return m_CFG->GetString( "lang_0215", "lang_0215" );
}

QString CLanguage :: HasLostConnectionSocketErrorGProxy( QString error )
{
	QString Out = m_CFG->GetString( "lang_0216", "lang_0216" );
	Out.replace("$ERROR$", error);
	return Out;
}

QString CLanguage :: HasLostConnectionClosedByRemoteHostGProxy( )
{
	return m_CFG->GetString( "lang_0217", "lang_0217" );
}

QString CLanguage :: WaitForReconnectSecondsRemain( QString seconds )
{
	QString Out = m_CFG->GetString( "lang_0218", "lang_0218" );
	Out.replace("$SECONDS$", seconds);
	return Out;
}

QString CLanguage :: WasUnrecoverablyDroppedFromGProxy( )
{
	return m_CFG->GetString( "lang_0219", "lang_0219" );
}

QString CLanguage :: PlayerReconnectedWithGProxy( QString name )
{
	QString Out = m_CFG->GetString( "lang_0220", "lang_0220" );
	Out.replace("$NAME$", name);
	return Out;
}
