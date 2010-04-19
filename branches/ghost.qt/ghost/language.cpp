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
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$GAMENAME$", gamename );
	return Out;
}

QString CLanguage :: UserIsAlreadyAnAdmin( QString server, QString user )
{
	QString Out = m_CFG->GetString( "lang_0002", "lang_0002" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

QString CLanguage :: AddedUserToAdminDatabase( QString server, QString user )
{
	QString Out = m_CFG->GetString( "lang_0003", "lang_0003" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

QString CLanguage :: ErrorAddingUserToAdminDatabase( QString server, QString user )
{
	QString Out = m_CFG->GetString( "lang_0004", "lang_0004" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

QString CLanguage :: YouDontHaveAccessToThatCommand( )
{
	return m_CFG->GetString( "lang_0005", "lang_0005" );
}

QString CLanguage :: UserIsAlreadyBanned( QString server, QString victim )
{
	QString Out = m_CFG->GetString( "lang_0006", "lang_0006" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

QString CLanguage :: BannedUser( QString server, QString victim )
{
	QString Out = m_CFG->GetString( "lang_0007", "lang_0007" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

QString CLanguage :: ErrorBanningUser( QString server, QString victim )
{
	QString Out = m_CFG->GetString( "lang_0008", "lang_0008" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

QString CLanguage :: UserIsAnAdmin( QString server, QString user )
{
	QString Out = m_CFG->GetString( "lang_0009", "lang_0009" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

QString CLanguage :: UserIsNotAnAdmin( QString server, QString user )
{
	QString Out = m_CFG->GetString( "lang_0010", "lang_0010" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

QString CLanguage :: UserWasBannedOnByBecause( QString server, QString victim, QString date, QString admin, QString reason )
{
	QString Out = m_CFG->GetString( "lang_0011", "lang_0011" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$VICTIM$", victim );
	UTIL_Replace( Out, "$DATE$", date );
	UTIL_Replace( Out, "$ADMIN$", admin );
	UTIL_Replace( Out, "$REASON$", reason );
	return Out;
}

QString CLanguage :: UserIsNotBanned( QString server, QString victim )
{
	QString Out = m_CFG->GetString( "lang_0012", "lang_0012" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

QString CLanguage :: ThereAreNoAdmins( QString server )
{
	QString Out = m_CFG->GetString( "lang_0013", "lang_0013" );
	UTIL_Replace( Out, "$SERVER$", server );
	return Out;
}

QString CLanguage :: ThereIsAdmin( QString server )
{
	QString Out = m_CFG->GetString( "lang_0014", "lang_0014" );
	UTIL_Replace( Out, "$SERVER$", server );
	return Out;
}

QString CLanguage :: ThereAreAdmins( QString server, QString count )
{
	QString Out = m_CFG->GetString( "lang_0015", "lang_0015" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$COUNT$", count );
	return Out;
}

QString CLanguage :: ThereAreNoBannedUsers( QString server )
{
	QString Out = m_CFG->GetString( "lang_0016", "lang_0016" );
	UTIL_Replace( Out, "$SERVER$", server );
	return Out;
}

QString CLanguage :: ThereIsBannedUser( QString server )
{
	QString Out = m_CFG->GetString( "lang_0017", "lang_0017" );
	UTIL_Replace( Out, "$SERVER$", server );
	return Out;
}

QString CLanguage :: ThereAreBannedUsers( QString server, QString count )
{
	QString Out = m_CFG->GetString( "lang_0018", "lang_0018" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$COUNT$", count );
	return Out;
}

QString CLanguage :: YouCantDeleteTheRootAdmin( )
{
	return m_CFG->GetString( "lang_0019", "lang_0019" );
}

QString CLanguage :: DeletedUserFromAdminDatabase( QString server, QString user )
{
	QString Out = m_CFG->GetString( "lang_0020", "lang_0020" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

QString CLanguage :: ErrorDeletingUserFromAdminDatabase( QString server, QString user )
{
	QString Out = m_CFG->GetString( "lang_0021", "lang_0021" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

QString CLanguage :: UnbannedUser( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0022", "lang_0022" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

QString CLanguage :: ErrorUnbanningUser( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0023", "lang_0023" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

QString CLanguage :: GameNumberIs( QString number, QString description )
{
	QString Out = m_CFG->GetString( "lang_0024", "lang_0024" );
	UTIL_Replace( Out, "$NUMBER$", number );
	UTIL_Replace( Out, "$DESCRIPTION$", description );
	return Out;
}

QString CLanguage :: GameNumberDoesntExist( QString number )
{
	QString Out = m_CFG->GetString( "lang_0025", "lang_0025" );
	UTIL_Replace( Out, "$NUMBER$", number );
	return Out;
}

QString CLanguage :: GameIsInTheLobby( QString description, QString current, QString max )
{
	QString Out = m_CFG->GetString( "lang_0026", "lang_0026" );
	UTIL_Replace( Out, "$DESCRIPTION$", description );
	UTIL_Replace( Out, "$CURRENT$", current );
	UTIL_Replace( Out, "$MAX$", max );
	return Out;
}

QString CLanguage :: ThereIsNoGameInTheLobby( QString current, QString max )
{
	QString Out = m_CFG->GetString( "lang_0027", "lang_0027" );
	UTIL_Replace( Out, "$CURRENT$", current );
	UTIL_Replace( Out, "$MAX$", max );
	return Out;
}

QString CLanguage :: UnableToLoadConfigFilesOutside( )
{
	return m_CFG->GetString( "lang_0028", "lang_0028" );
}

QString CLanguage :: LoadingConfigFile( QString file )
{
	QString Out = m_CFG->GetString( "lang_0029", "lang_0029" );
	UTIL_Replace( Out, "$FILE$", file );
	return Out;
}

QString CLanguage :: UnableToLoadConfigFileDoesntExist( QString file )
{
	QString Out = m_CFG->GetString( "lang_0030", "lang_0030" );
	UTIL_Replace( Out, "$FILE$", file );
	return Out;
}

QString CLanguage :: CreatingPrivateGame( QString gamename, QString user )
{
	QString Out = m_CFG->GetString( "lang_0031", "lang_0031" );
	UTIL_Replace( Out, "$GAMENAME$", gamename );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

QString CLanguage :: CreatingPublicGame( QString gamename, QString user )
{
	QString Out = m_CFG->GetString( "lang_0032", "lang_0032" );
	UTIL_Replace( Out, "$GAMENAME$", gamename );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

QString CLanguage :: UnableToUnhostGameCountdownStarted( QString description )
{
	QString Out = m_CFG->GetString( "lang_0033", "lang_0033" );
	UTIL_Replace( Out, "$DESCRIPTION$", description );
	return Out;
}

QString CLanguage :: UnhostingGame( QString description )
{
	QString Out = m_CFG->GetString( "lang_0034", "lang_0034" );
	UTIL_Replace( Out, "$DESCRIPTION$", description );
	return Out;
}

QString CLanguage :: UnableToUnhostGameNoGameInLobby( )
{
	return m_CFG->GetString( "lang_0035", "lang_0035" );
}

QString CLanguage :: VersionAdmin( QString version )
{
	QString Out = m_CFG->GetString( "lang_0036", "lang_0036" );
	UTIL_Replace( Out, "$VERSION$", version );
	return Out;
}

QString CLanguage :: VersionNotAdmin( QString version )
{
	QString Out = m_CFG->GetString( "lang_0037", "lang_0037" );
	UTIL_Replace( Out, "$VERSION$", version );
	return Out;
}

QString CLanguage :: UnableToCreateGameAnotherGameInLobby( QString gamename, QString description )
{
	QString Out = m_CFG->GetString( "lang_0038", "lang_0038" );
	UTIL_Replace( Out, "$GAMENAME$", gamename );
	UTIL_Replace( Out, "$DESCRIPTION$", description );
	return Out;
}

QString CLanguage :: UnableToCreateGameMaxGamesReached( QString gamename, QString max )
{
	QString Out = m_CFG->GetString( "lang_0039", "lang_0039" );
	UTIL_Replace( Out, "$GAMENAME$", gamename );
	UTIL_Replace( Out, "$MAX$", max );
	return Out;
}

QString CLanguage :: GameIsOver( QString description )
{
	QString Out = m_CFG->GetString( "lang_0040", "lang_0040" );
	UTIL_Replace( Out, "$DESCRIPTION$", description );
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
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

QString CLanguage :: SpoofPossibleIsUnavailable( QString user )
{
	QString Out = m_CFG->GetString( "lang_0044", "lang_0044" );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

QString CLanguage :: SpoofPossibleIsRefusingMessages( QString user )
{
	QString Out = m_CFG->GetString( "lang_0045", "lang_0045" );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

QString CLanguage :: SpoofDetectedIsNotInGame( QString user )
{
	QString Out = m_CFG->GetString( "lang_0046", "lang_0046" );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

QString CLanguage :: SpoofDetectedIsInPrivateChannel( QString user )
{
	QString Out = m_CFG->GetString( "lang_0047", "lang_0047" );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

QString CLanguage :: SpoofDetectedIsInAnotherGame( QString user )
{
	QString Out = m_CFG->GetString( "lang_0048", "lang_0048" );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

QString CLanguage :: CountDownAborted( )
{
	return m_CFG->GetString( "lang_0049", "lang_0049" );
}

QString CLanguage :: TryingToJoinTheGameButBanned( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0050", "lang_0050" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

QString CLanguage :: UnableToBanNoMatchesFound( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0051", "lang_0051" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

QString CLanguage :: PlayerWasBannedByPlayer( QString server, QString victim, QString user )
{
	QString Out = m_CFG->GetString( "lang_0052", "lang_0052" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$VICTIM$", victim );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

QString CLanguage :: UnableToBanFoundMoreThanOneMatch( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0053", "lang_0053" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

QString CLanguage :: AddedPlayerToTheHoldList( QString user )
{
	QString Out = m_CFG->GetString( "lang_0054", "lang_0054" );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

QString CLanguage :: UnableToKickNoMatchesFound( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0055", "lang_0055" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

QString CLanguage :: UnableToKickFoundMoreThanOneMatch( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0056", "lang_0056" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

QString CLanguage :: SettingLatencyToMinimum( QString min )
{
	QString Out = m_CFG->GetString( "lang_0057", "lang_0057" );
	UTIL_Replace( Out, "$MIN$", min );
	return Out;
}

QString CLanguage :: SettingLatencyToMaximum( QString max )
{
	QString Out = m_CFG->GetString( "lang_0058", "lang_0058" );
	UTIL_Replace( Out, "$MAX$", max );
	return Out;
}

QString CLanguage :: SettingLatencyTo( QString latency )
{
	QString Out = m_CFG->GetString( "lang_0059", "lang_0059" );
	UTIL_Replace( Out, "$LATENCY$", latency );
	return Out;
}

QString CLanguage :: KickingPlayersWithPingsGreaterThan( QString total, QString ping )
{
	QString Out = m_CFG->GetString( "lang_0060", "lang_0060" );
	UTIL_Replace( Out, "$TOTAL$", total );
	UTIL_Replace( Out, "$PING$", ping );
	return Out;
}

QString CLanguage :: HasPlayedGamesWithThisBot( QString user, QString firstgame, QString lastgame, QString totalgames, QString avgloadingtime, QString avgstay )
{
	QString Out = m_CFG->GetString( "lang_0061", "lang_0061" );
	UTIL_Replace( Out, "$USER$", user );
	UTIL_Replace( Out, "$FIRSTGAME$", firstgame );
	UTIL_Replace( Out, "$LASTGAME$", lastgame );
	UTIL_Replace( Out, "$TOTALGAMES$", totalgames );
	UTIL_Replace( Out, "$AVGLOADINGTIME$", avgloadingtime );
	UTIL_Replace( Out, "$AVGSTAY$", avgstay );
	return Out;
}

QString CLanguage :: HasntPlayedGamesWithThisBot( QString user )
{
	QString Out = m_CFG->GetString( "lang_0062", "lang_0062" );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

QString CLanguage :: AutokickingPlayerForExcessivePing( QString victim, QString ping )
{
	QString Out = m_CFG->GetString( "lang_0063", "lang_0063" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	UTIL_Replace( Out, "$PING$", ping );
	return Out;
}

QString CLanguage :: SpoofCheckAcceptedFor( QString server, QString user )
{
	QString Out = m_CFG->GetString( "lang_0064", "lang_0064" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

QString CLanguage :: PlayersNotYetSpoofChecked( QString notspoofchecked )
{
	QString Out = m_CFG->GetString( "lang_0065", "lang_0065" );
	UTIL_Replace( Out, "$NOTSPOOFCHECKED$", notspoofchecked );
	return Out;
}

QString CLanguage :: ManuallySpoofCheckByWhispering( QString hostname )
{
	QString Out = m_CFG->GetString( "lang_0066", "lang_0066" );
	UTIL_Replace( Out, "$HOSTNAME$", hostname );
	return Out;
}

QString CLanguage :: SpoofCheckByWhispering( QString hostname )
{
	QString Out = m_CFG->GetString( "lang_0067", "lang_0067" );
	UTIL_Replace( Out, "$HOSTNAME$", hostname );
	return Out;
}

QString CLanguage :: EveryoneHasBeenSpoofChecked( )
{
	return m_CFG->GetString( "lang_0068", "lang_0068" );
}

QString CLanguage :: PlayersNotYetPinged( QString notpinged )
{
	QString Out = m_CFG->GetString( "lang_0069", "lang_0069" );
	UTIL_Replace( Out, "$NOTPINGED$", notpinged );
	return Out;
}

QString CLanguage :: EveryoneHasBeenPinged( )
{
	return m_CFG->GetString( "lang_0070", "lang_0070" );
}

QString CLanguage :: ShortestLoadByPlayer( QString user, QString loadingtime )
{
	QString Out = m_CFG->GetString( "lang_0071", "lang_0071" );
	UTIL_Replace( Out, "$USER$", user );
	UTIL_Replace( Out, "$LOADINGTIME$", loadingtime );
	return Out;
}

QString CLanguage :: LongestLoadByPlayer( QString user, QString loadingtime )
{
	QString Out = m_CFG->GetString( "lang_0072", "lang_0072" );
	UTIL_Replace( Out, "$USER$", user );
	UTIL_Replace( Out, "$LOADINGTIME$", loadingtime );
	return Out;
}

QString CLanguage :: YourLoadingTimeWas( QString loadingtime )
{
	QString Out = m_CFG->GetString( "lang_0073", "lang_0073" );
	UTIL_Replace( Out, "$LOADINGTIME$", loadingtime );
	return Out;
}

QString CLanguage :: HasPlayedDotAGamesWithThisBot( QString user, QString totalgames, QString totalwins, QString totallosses, QString totalkills, QString totaldeaths, QString totalcreepkills, QString totalcreepdenies, QString totalassists, QString totalneutralkills, QString totaltowerkills, QString totalraxkills, QString totalcourierkills, QString avgkills, QString avgdeaths, QString avgcreepkills, QString avgcreepdenies, QString avgassists, QString avgneutralkills, QString avgtowerkills, QString avgraxkills, QString avgcourierkills )
{
	QString Out = m_CFG->GetString( "lang_0074", "lang_0074" );
	UTIL_Replace( Out, "$USER$", user );
	UTIL_Replace( Out, "$TOTALGAMES$", totalgames );
	UTIL_Replace( Out, "$TOTALWINS$", totalwins );
	UTIL_Replace( Out, "$TOTALLOSSES$", totallosses );
	UTIL_Replace( Out, "$TOTALKILLS$", totalkills );
	UTIL_Replace( Out, "$TOTALDEATHS$", totaldeaths );
	UTIL_Replace( Out, "$TOTALCREEPKILLS$", totalcreepkills );
	UTIL_Replace( Out, "$TOTALCREEPDENIES$", totalcreepdenies );
	UTIL_Replace( Out, "$TOTALASSISTS$", totalassists );
	UTIL_Replace( Out, "$TOTALNEUTRALKILLS$", totalneutralkills );
	UTIL_Replace( Out, "$TOTALTOWERKILLS$", totaltowerkills );
	UTIL_Replace( Out, "$TOTALRAXKILLS$", totalraxkills );
	UTIL_Replace( Out, "$TOTALCOURIERKILLS$", totalcourierkills );
	UTIL_Replace( Out, "$AVGKILLS$", avgkills );
	UTIL_Replace( Out, "$AVGDEATHS$", avgdeaths );
	UTIL_Replace( Out, "$AVGCREEPKILLS$", avgcreepkills );
	UTIL_Replace( Out, "$AVGCREEPDENIES$", avgcreepdenies );
	UTIL_Replace( Out, "$AVGASSISTS$", avgassists );
	UTIL_Replace( Out, "$AVGNEUTRALKILLS$", avgneutralkills );
	UTIL_Replace( Out, "$AVGTOWERKILLS$", avgtowerkills );
	UTIL_Replace( Out, "$AVGRAXKILLS$", avgraxkills );
	UTIL_Replace( Out, "$AVGCOURIERKILLS$", avgcourierkills );
	return Out;
}

QString CLanguage :: HasntPlayedDotAGamesWithThisBot( QString user )
{
	QString Out = m_CFG->GetString( "lang_0075", "lang_0075" );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

QString CLanguage :: WasKickedForReservedPlayer( QString reserved )
{
	QString Out = m_CFG->GetString( "lang_0076", "lang_0076" );
	UTIL_Replace( Out, "$RESERVED$", reserved );
	return Out;
}

QString CLanguage :: WasKickedForOwnerPlayer( QString owner )
{
	QString Out = m_CFG->GetString( "lang_0077", "lang_0077" );
	UTIL_Replace( Out, "$OWNER$", owner );
	return Out;
}

QString CLanguage :: WasKickedByPlayer( QString user )
{
	QString Out = m_CFG->GetString( "lang_0078", "lang_0078" );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

QString CLanguage :: HasLostConnectionPlayerError( QString error )
{
	QString Out = m_CFG->GetString( "lang_0079", "lang_0079" );
	UTIL_Replace( Out, "$ERROR$", error );
	return Out;
}

QString CLanguage :: HasLostConnectionSocketError( QString error )
{
	QString Out = m_CFG->GetString( "lang_0080", "lang_0080" );
	UTIL_Replace( Out, "$ERROR$", error );
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
	UTIL_Replace( Out, "$DESCRIPTION$", description );
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
	UTIL_Replace( Out, "$STILLDOWNLOADING$", stilldownloading );
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
	UTIL_Replace( Out, "$MAPCFG$", mapcfg );
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
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

QString CLanguage :: LatencyIs( QString latency )
{
	QString Out = m_CFG->GetString( "lang_0097", "lang_0097" );
	UTIL_Replace( Out, "$LATENCY$", latency );
	return Out;
}

QString CLanguage :: SyncLimitIs( QString synclimit )
{
	QString Out = m_CFG->GetString( "lang_0098", "lang_0098" );
	UTIL_Replace( Out, "$SYNCLIMIT$", synclimit );
	return Out;
}

QString CLanguage :: SettingSyncLimitToMinimum( QString min )
{
	QString Out = m_CFG->GetString( "lang_0099", "lang_0099" );
	UTIL_Replace( Out, "$MIN$", min );
	return Out;
}

QString CLanguage :: SettingSyncLimitToMaximum( QString max )
{
	QString Out = m_CFG->GetString( "lang_0100", "lang_0100" );
	UTIL_Replace( Out, "$MAX$", max );
	return Out;
}

QString CLanguage :: SettingSyncLimitTo( QString synclimit )
{
	QString Out = m_CFG->GetString( "lang_0101", "lang_0101" );
	UTIL_Replace( Out, "$SYNCLIMIT$", synclimit );
	return Out;
}

QString CLanguage :: UnableToCreateGameNotLoggedIn( QString gamename )
{
	QString Out = m_CFG->GetString( "lang_0102", "lang_0102" );
	UTIL_Replace( Out, "$GAMENAME$", gamename );
	return Out;
}

QString CLanguage :: AdminLoggedIn( )
{
	return m_CFG->GetString( "lang_0103", "lang_0103" );
}

QString CLanguage :: AdminInvalidPassword( QString attempt )
{
	QString Out = m_CFG->GetString( "lang_0104", "lang_0104" );
	UTIL_Replace( Out, "$ATTEMPT$", attempt );
	return Out;
}

QString CLanguage :: ConnectingToBNET( QString server )
{
	QString Out = m_CFG->GetString( "lang_0105", "lang_0105" );
	UTIL_Replace( Out, "$SERVER$", server );
	return Out;
}

QString CLanguage :: ConnectedToBNET( QString server )
{
	QString Out = m_CFG->GetString( "lang_0106", "lang_0106" );
	UTIL_Replace( Out, "$SERVER$", server );
	return Out;
}

QString CLanguage :: DisconnectedFromBNET( QString server )
{
	QString Out = m_CFG->GetString( "lang_0107", "lang_0107" );
	UTIL_Replace( Out, "$SERVER$", server );
	return Out;
}

QString CLanguage :: LoggedInToBNET( QString server )
{
	QString Out = m_CFG->GetString( "lang_0108", "lang_0108" );
	UTIL_Replace( Out, "$SERVER$", server );
	return Out;
}

QString CLanguage :: BNETGameHostingSucceeded( QString server )
{
	QString Out = m_CFG->GetString( "lang_0109", "lang_0109" );
	UTIL_Replace( Out, "$SERVER$", server );
	return Out;
}

QString CLanguage :: BNETGameHostingFailed( QString server, QString gamename )
{
	QString Out = m_CFG->GetString( "lang_0110", "lang_0110" );
	UTIL_Replace( Out, "$SERVER$", server );
	UTIL_Replace( Out, "$GAMENAME$", gamename );
	return Out;
}

QString CLanguage :: ConnectingToBNETTimedOut( QString server )
{
	QString Out = m_CFG->GetString( "lang_0111", "lang_0111" );
	UTIL_Replace( Out, "$SERVER$", server );
	return Out;
}

QString CLanguage :: PlayerDownloadedTheMap( QString user, QString seconds, QString rate )
{
	QString Out = m_CFG->GetString( "lang_0112", "lang_0112" );
	UTIL_Replace( Out, "$USER$", user );
	UTIL_Replace( Out, "$SECONDS$", seconds );
	UTIL_Replace( Out, "$RATE$", rate );
	return Out;
}

QString CLanguage :: UnableToCreateGameNameTooLong( QString gamename )
{
	QString Out = m_CFG->GetString( "lang_0113", "lang_0113" );
	UTIL_Replace( Out, "$GAMENAME$", gamename );
	return Out;
}

QString CLanguage :: SettingGameOwnerTo( QString owner )
{
	QString Out = m_CFG->GetString( "lang_0114", "lang_0114" );
	UTIL_Replace( Out, "$OWNER$", owner );
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
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

QString CLanguage :: UnableToStartDownloadFoundMoreThanOneMatch( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0119", "lang_0119" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

QString CLanguage :: UnableToSetGameOwner( QString owner )
{
	QString Out = m_CFG->GetString( "lang_0120", "lang_0120" );
	UTIL_Replace( Out, "$OWNER$", owner );
	return Out;
}

QString CLanguage :: UnableToCheckPlayerNoMatchesFound( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0121", "lang_0121" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

QString CLanguage :: CheckedPlayer( QString victim, QString ping, QString from, QString admin, QString owner, QString spoofed, QString spoofedrealm, QString reserved )
{
	QString Out = m_CFG->GetString( "lang_0122", "lang_0122" );
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

QString CLanguage :: UnableToCheckPlayerFoundMoreThanOneMatch( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0123", "lang_0123" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

QString CLanguage :: TheGameIsLockedBNET( )
{
	return m_CFG->GetString( "lang_0124", "lang_0124" );
}

QString CLanguage :: UnableToCreateGameDisabled( QString gamename )
{
	QString Out = m_CFG->GetString( "lang_0125", "lang_0125" );
	UTIL_Replace( Out, "$GAMENAME$", gamename );
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
	UTIL_Replace( Out, "$GAMENAME$", gamename );
	return Out;
}

QString CLanguage :: WaitingForPlayersBeforeAutoStart( QString players, QString playersleft )
{
	QString Out = m_CFG->GetString( "lang_0129", "lang_0129" );
	UTIL_Replace( Out, "$PLAYERS$", players );
	UTIL_Replace( Out, "$PLAYERSLEFT$", playersleft );
	return Out;
}

QString CLanguage :: AutoStartDisabled( )
{
	return m_CFG->GetString( "lang_0130", "lang_0130" );
}

QString CLanguage :: AutoStartEnabled( QString players )
{
	QString Out = m_CFG->GetString( "lang_0131", "lang_0131" );
	UTIL_Replace( Out, "$PLAYERS$", players );
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
	UTIL_Replace( Out, "$FILE$", file );
	return Out;
}

QString CLanguage :: UnableToLoadSaveGameDoesntExist( QString file )
{
	QString Out = m_CFG->GetString( "lang_0139", "lang_0139" );
	UTIL_Replace( Out, "$FILE$", file );
	return Out;
}

QString CLanguage :: UnableToCreateGameInvalidSaveGame( QString gamename )
{
	QString Out = m_CFG->GetString( "lang_0140", "lang_0140" );
	UTIL_Replace( Out, "$GAMENAME$", gamename );
	return Out;
}

QString CLanguage :: UnableToCreateGameSaveGameMapMismatch( QString gamename )
{
	QString Out = m_CFG->GetString( "lang_0141", "lang_0141" );
	UTIL_Replace( Out, "$GAMENAME$", gamename );
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
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

QString CLanguage :: MutedPlayer( QString victim, QString user )
{
	QString Out = m_CFG->GetString( "lang_0146", "lang_0146" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

QString CLanguage :: UnmutedPlayer( QString victim, QString user )
{
	QString Out = m_CFG->GetString( "lang_0147", "lang_0147" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	UTIL_Replace( Out, "$USER$", user );
	return Out;
}

QString CLanguage :: UnableToMuteFoundMoreThanOneMatch( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0148", "lang_0148" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

QString CLanguage :: PlayerIsSavingTheGame( QString player )
{
	QString Out = m_CFG->GetString( "lang_0149", "lang_0149" );
	UTIL_Replace( Out, "$PLAYER$", player );
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
	UTIL_Replace( Out, "$PLAYER$", player );
	UTIL_Replace( Out, "$OTHERS$", others );
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
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

QString CLanguage :: UnableToVoteKickPlayerIsReserved( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0156", "lang_0156" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

QString CLanguage :: StartedVoteKick( QString victim, QString user, QString votesneeded )
{
	QString Out = m_CFG->GetString( "lang_0157", "lang_0157" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	UTIL_Replace( Out, "$USER$", user );
	UTIL_Replace( Out, "$VOTESNEEDED$", votesneeded );
	return Out;
}

QString CLanguage :: UnableToVoteKickFoundMoreThanOneMatch( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0158", "lang_0158" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

QString CLanguage :: VoteKickPassed( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0159", "lang_0159" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

QString CLanguage :: ErrorVoteKickingPlayer( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0160", "lang_0160" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

QString CLanguage :: VoteKickAcceptedNeedMoreVotes( QString victim, QString user, QString votes )
{
	QString Out = m_CFG->GetString( "lang_0161", "lang_0161" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	UTIL_Replace( Out, "$USER$", user );
	UTIL_Replace( Out, "$VOTES$", votes );
	return Out;
}

QString CLanguage :: VoteKickCancelled( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0162", "lang_0162" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

QString CLanguage :: VoteKickExpired( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0163", "lang_0163" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

QString CLanguage :: WasKickedByVote( )
{
	return m_CFG->GetString( "lang_0164", "lang_0164" );
}

QString CLanguage :: TypeYesToVote( QString commandtrigger )
{
	QString Out = m_CFG->GetString( "lang_0165", "lang_0165" );
	UTIL_Replace( Out, "$COMMANDTRIGGER$", commandtrigger );
	return Out;
}

QString CLanguage :: PlayersNotYetPingedAutoStart( QString notpinged )
{
	QString Out = m_CFG->GetString( "lang_0166", "lang_0166" );
	UTIL_Replace( Out, "$NOTPINGED$", notpinged );
	return Out;
}

QString CLanguage :: WasKickedForNotSpoofChecking( )
{
	return m_CFG->GetString( "lang_0167", "lang_0167" );
}

QString CLanguage :: WasKickedForHavingFurthestScore( QString score, QString average )
{
	QString Out = m_CFG->GetString( "lang_0168", "lang_0168" );
	UTIL_Replace( Out, "$SCORE$", score );
	UTIL_Replace( Out, "$AVERAGE$", average );
	return Out;
}

QString CLanguage :: PlayerHasScore( QString player, QString score )
{
	QString Out = m_CFG->GetString( "lang_0169", "lang_0169" );
	UTIL_Replace( Out, "$PLAYER$", player );
	UTIL_Replace( Out, "$SCORE$", score );
	return Out;
}

QString CLanguage :: RatedPlayersSpread( QString rated, QString total, QString spread )
{
	QString Out = m_CFG->GetString( "lang_0170", "lang_0170" );
	UTIL_Replace( Out, "$RATED$", rated );
	UTIL_Replace( Out, "$TOTAL$", total );
	UTIL_Replace( Out, "$SPREAD$", spread );
	return Out;
}

QString CLanguage :: ErrorListingMaps( )
{
	return m_CFG->GetString( "lang_0171", "lang_0171" );
}

QString CLanguage :: FoundMaps( QString maps )
{
	QString Out = m_CFG->GetString( "lang_0172", "lang_0172" );
	UTIL_Replace( Out, "$MAPS$", maps );
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
	UTIL_Replace( Out, "$MAPCONFIGS$", mapconfigs );
	return Out;
}

QString CLanguage :: NoMapConfigsFound( )
{
	return m_CFG->GetString( "lang_0176", "lang_0176" );
}

QString CLanguage :: PlayerFinishedLoading( QString user )
{
	QString Out = m_CFG->GetString( "lang_0177", "lang_0177" );
	UTIL_Replace( Out, "$USER$", user );
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
	UTIL_Replace( Out, "$HCL$", HCL );
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
	UTIL_Replace( Out, "$HCL$", HCL );
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
	UTIL_Replace( Out, "$GAMENAME$", gamename );
	return Out;
}

QString CLanguage :: TryingToRehostAsPublicGame( QString gamename )
{
	QString Out = m_CFG->GetString( "lang_0189", "lang_0189" );
	UTIL_Replace( Out, "$GAMENAME$", gamename );
	return Out;
}

QString CLanguage :: RehostWasSuccessful( )
{
	return m_CFG->GetString( "lang_0190", "lang_0190" );
}

QString CLanguage :: TryingToJoinTheGameButBannedByName( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0191", "lang_0191" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

QString CLanguage :: TryingToJoinTheGameButBannedByIP( QString victim, QString ip, QString bannedname )
{
	QString Out = m_CFG->GetString( "lang_0192", "lang_0192" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	UTIL_Replace( Out, "$IP$", ip );
	UTIL_Replace( Out, "$BANNEDNAME$", bannedname );
	return Out;
}

QString CLanguage :: HasBannedName( QString victim )
{
	QString Out = m_CFG->GetString( "lang_0193", "lang_0193" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	return Out;
}

QString CLanguage :: HasBannedIP( QString victim, QString ip, QString bannedname )
{
	QString Out = m_CFG->GetString( "lang_0194", "lang_0194" );
	UTIL_Replace( Out, "$VICTIM$", victim );
	UTIL_Replace( Out, "$IP$", ip );
	UTIL_Replace( Out, "$BANNEDNAME$", bannedname );
	return Out;
}

QString CLanguage :: PlayersInGameState( QString number, QString players )
{
	QString Out = m_CFG->GetString( "lang_0195", "lang_0195" );
	UTIL_Replace( Out, "$NUMBER$", number );
	UTIL_Replace( Out, "$PLAYERS$", players );
	return Out;
}

QString CLanguage :: ValidServers( QString servers )
{
	QString Out = m_CFG->GetString( "lang_0196", "lang_0196" );
	UTIL_Replace( Out, "$SERVERS$", servers );
	return Out;
}

QString CLanguage :: TeamCombinedScore( QString team, QString score )
{
	QString Out = m_CFG->GetString( "lang_0197", "lang_0197" );
	UTIL_Replace( Out, "$TEAM$", team );
	UTIL_Replace( Out, "$SCORE$", score );
	return Out;
}

QString CLanguage :: BalancingSlotsCompleted( )
{
	return m_CFG->GetString( "lang_0198", "lang_0198" );
}

QString CLanguage :: PlayerWasKickedForFurthestScore( QString name, QString score, QString average )
{
	QString Out = m_CFG->GetString( "lang_0199", "lang_0199" );
	UTIL_Replace( Out, "$NAME$", name );
	UTIL_Replace( Out, "$SCORE$", score );
	UTIL_Replace( Out, "$AVERAGE$", average );
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
	UTIL_Replace( Out, "$SCORE$", score );
	return Out;
}

QString CLanguage :: PlayerWasKickedForLowestScore( QString name, QString score )
{
	QString Out = m_CFG->GetString( "lang_0204", "lang_0204" );
	UTIL_Replace( Out, "$NAME$", name );
	UTIL_Replace( Out, "$SCORE$", score );
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
	UTIL_Replace( Out, "$GAMENAME$", gamename );
	return Out;
}

QString CLanguage :: UnableToLoadReplaysOutside( )
{
	return m_CFG->GetString( "lang_0208", "lang_0208" );
}

QString CLanguage :: LoadingReplay( QString file )
{
	QString Out = m_CFG->GetString( "lang_0209", "lang_0209" );
	UTIL_Replace( Out, "$FILE$", file );
	return Out;
}

QString CLanguage :: UnableToLoadReplayDoesntExist( QString file )
{
	QString Out = m_CFG->GetString( "lang_0210", "lang_0210" );
	UTIL_Replace( Out, "$FILE$", file );
	return Out;
}

QString CLanguage :: CommandTrigger( QString trigger )
{
	QString Out = m_CFG->GetString( "lang_0211", "lang_0211" );
	UTIL_Replace( Out, "$TRIGGER$", trigger );
	return Out;
}

QString CLanguage :: CantEndGameOwnerIsStillPlaying( QString owner )
{
	QString Out = m_CFG->GetString( "lang_0212", "lang_0212" );
	UTIL_Replace( Out, "$OWNER$", owner );
	return Out;
}

QString CLanguage :: CantUnhostGameOwnerIsPresent( QString owner )
{
	QString Out = m_CFG->GetString( "lang_0213", "lang_0213" );
	UTIL_Replace( Out, "$OWNER$", owner );
	return Out;
}

QString CLanguage :: WasAutomaticallyDroppedAfterSeconds( QString seconds )
{
	QString Out = m_CFG->GetString( "lang_0214", "lang_0214" );
	UTIL_Replace( Out, "$SECONDS$", seconds );
	return Out;
}

QString CLanguage :: HasLostConnectionTimedOutGProxy( )
{
	return m_CFG->GetString( "lang_0215", "lang_0215" );
}

QString CLanguage :: HasLostConnectionSocketErrorGProxy( QString error )
{
	QString Out = m_CFG->GetString( "lang_0216", "lang_0216" );
	UTIL_Replace( Out, "$ERROR$", error );
	return Out;
}

QString CLanguage :: HasLostConnectionClosedByRemoteHostGProxy( )
{
	return m_CFG->GetString( "lang_0217", "lang_0217" );
}

QString CLanguage :: WaitForReconnectSecondsRemain( QString seconds )
{
	QString Out = m_CFG->GetString( "lang_0218", "lang_0218" );
	UTIL_Replace( Out, "$SECONDS$", seconds );
	return Out;
}

QString CLanguage :: WasUnrecoverablyDroppedFromGProxy( )
{
	return m_CFG->GetString( "lang_0219", "lang_0219" );
}

QString CLanguage :: PlayerReconnectedWithGProxy( QString name )
{
	QString Out = m_CFG->GetString( "lang_0220", "lang_0220" );
	UTIL_Replace( Out, "$NAME$", name );
	return Out;
}
