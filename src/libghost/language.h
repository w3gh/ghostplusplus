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

#ifndef LANGUAGE_H
#define LANGUAGE_H

#include <QString>

class CConfig;

//
// CLanguage
//

class CLanguage
{
private:
	CConfig *m_CFG;

public:
	CLanguage( const QString &nCFGFile );
	~CLanguage( );

	QString UnableToCreateGameTryAnotherName( const QString &server, const QString &gamename ) const;
	QString UserIsAlreadyAnAdmin( const QString &server, const QString &user ) const;
	QString AddedUserToAdminDatabase( const QString &server, const QString &user ) const;
	QString ErrorAddingUserToAdminDatabase( const QString &server, const QString &user ) const;
	QString YouDontHaveAccessToThatCommand( ) const;
	QString UserIsAlreadyBanned( const QString &server, const QString &victim ) const;
	QString BannedUser( const QString &server, const QString &victim ) const;
	QString ErrorBanningUser( const QString &server, const QString &victim ) const;
	QString UserIsAnAdmin( const QString &server, const QString &user ) const;
	QString UserIsNotAnAdmin( const QString &server, const QString &user ) const;
	QString UserWasBannedOnByBecause( const QString &server, const QString &victim, const QString &date, const QString &admin, const QString &reason ) const;
	QString UserIsNotBanned( const QString &server, const QString &victim ) const;
	QString ThereAreNoAdmins( const QString &server ) const;
	QString ThereIsAdmin( const QString &server ) const;
	QString ThereAreAdmins( const QString &server, const QString &count ) const;
	QString ThereAreNoBannedUsers( const QString &server ) const;
	QString ThereIsBannedUser( const QString &server ) const;
	QString ThereAreBannedUsers( const QString &server, const QString &count ) const;
	QString YouCantDeleteTheRootAdmin( ) const;
	QString DeletedUserFromAdminDatabase( const QString &server, const QString &user ) const;
	QString ErrorDeletingUserFromAdminDatabase( const QString &server, const QString &user ) const;
	QString UnbannedUser( const QString &victim ) const;
	QString ErrorUnbanningUser( const QString &victim ) const;
	QString GameNumberIs( const QString &number, const QString &description ) const;
	QString GameNumberDoesntExist( const QString &number ) const;
	QString GameIsInTheLobby( const QString &description, const QString &current, const QString &max ) const;
	QString ThereIsNoGameInTheLobby( const QString &current, const QString &max ) const;
	QString UnableToLoadConfigFilesOutside( ) const;
	QString LoadingConfigFile( const QString &file ) const;
	QString UnableToLoadConfigFileDoesntExist( const QString &file ) const;
	QString CreatingPrivateGame( const QString &gamename, const QString &user ) const;
	QString CreatingPublicGame( const QString &gamename, const QString &user ) const;
	QString UnableToUnhostGameCountdownStarted( const QString &description ) const;
	QString UnhostingGame( const QString &description ) const;
	QString UnableToUnhostGameNoGameInLobby( ) const;
	QString VersionAdmin( const QString &version ) const;
	QString VersionNotAdmin( const QString &version ) const;
	QString UnableToCreateGameAnotherGameInLobby( const QString &gamename, const QString &description ) const;
	QString UnableToCreateGameMaxGamesReached( const QString &gamename, const QString &max ) const;
	QString GameIsOver( const QString &description ) const;
	QString SpoofCheckByReplying( ) const;
	QString GameRefreshed( ) const;
	QString SpoofPossibleIsAway( const QString &user ) const;
	QString SpoofPossibleIsUnavailable( const QString &user ) const;
	QString SpoofPossibleIsRefusingMessages( const QString &user ) const;
	QString SpoofDetectedIsNotInGame( const QString &user ) const;
	QString SpoofDetectedIsInPrivateChannel( const QString &user ) const;
	QString SpoofDetectedIsInAnotherGame( const QString &user ) const;
	QString CountDownAborted( ) const;
	QString TryingToJoinTheGameButBanned( const QString &victim ) const;
	QString UnableToBanNoMatchesFound( const QString &victim ) const;
	QString PlayerWasBannedByPlayer( const QString &server, const QString &victim, const QString &user ) const;
	QString UnableToBanFoundMoreThanOneMatch( const QString &victim ) const;
	QString AddedPlayerToTheHoldList( const QString &user ) const;
	QString UnableToKickNoMatchesFound( const QString &victim ) const;
	QString UnableToKickFoundMoreThanOneMatch( const QString &victim ) const;
	QString SettingLatencyToMinimum( const QString &min ) const;
	QString SettingLatencyToMaximum( const QString &max ) const;
	QString SettingLatencyTo( const QString &latency ) const;
	QString KickingPlayersWithPingsGreaterThan( const QString &total, const QString &ping ) const;
	QString HasPlayedGamesWithThisBot( const QString &user, const QString &firstgame, const QString &lastgame, const QString &totalgames, const QString &avgloadingtime, const QString &avgstay ) const;
	QString HasntPlayedGamesWithThisBot( const QString &user ) const;
	QString AutokickingPlayerForExcessivePing( const QString &victim, const QString &ping ) const;
	QString SpoofCheckAcceptedFor( const QString &server, const QString &user ) const;
	QString PlayersNotYetSpoofChecked( const QString &notspoofchecked ) const;
	QString ManuallySpoofCheckByWhispering( const QString &hostname ) const;
	QString SpoofCheckByWhispering( const QString &hostname ) const;
	QString EveryoneHasBeenSpoofChecked( ) const;
	QString PlayersNotYetPinged( const QString &notpinged ) const;
	QString EveryoneHasBeenPinged( ) const;
	QString ShortestLoadByPlayer( const QString &user, const QString &loadingtime ) const;
	QString LongestLoadByPlayer( const QString &user, const QString &loadingtime ) const;
	QString YourLoadingTimeWas( const QString &loadingtime ) const;
	QString HasPlayedDotAGamesWithThisBot( const QString &user, const QString &totalgames, const QString &totalwins, const QString &totallosses, const QString &totalkills, const QString &totaldeaths, const QString &totalcreepkills, const QString &totalcreepdenies, const QString &totalassists, const QString &totalneutralkills, const QString &totaltowerkills, const QString &totalraxkills, const QString &totalcourierkills, const QString &avgkills, const QString &avgdeaths, const QString &avgcreepkills, const QString &avgcreepdenies, const QString &avgassists, const QString &avgneutralkills, const QString &avgtowerkills, const QString &avgraxkills, const QString &avgcourierkills ) const;
	QString HasntPlayedDotAGamesWithThisBot( const QString &user ) const;
	QString WasKickedForReservedPlayer( const QString &reserved ) const;
	QString WasKickedForOwnerPlayer( const QString &owner ) const;
	QString WasKickedByPlayer( const QString &user ) const;
	QString HasLostConnectionPlayerError( const QString &error ) const;
	QString HasLostConnectionSocketError( const QString &error ) const;
	QString HasLostConnectionClosedByRemoteHost( ) const;
	QString HasLeftVoluntarily( ) const;
	QString EndingGame( const QString &description ) const;
	QString HasLostConnectionTimedOut( ) const;
	QString GlobalChatMuted( ) const;
	QString GlobalChatUnmuted( ) const;
	QString ShufflingPlayers( ) const;
	QString UnableToLoadConfigFileGameInLobby( ) const;
	QString PlayersStillDownloading( const QString &stilldownloading ) const;
	QString RefreshMessagesEnabled( ) const;
	QString RefreshMessagesDisabled( ) const;
	QString AtLeastOneGameActiveUseForceToShutdown( ) const;
	QString CurrentlyLoadedMapCFGIs( const QString &mapcfg ) const;
	QString LaggedOutDroppedByAdmin( ) const;
	QString LaggedOutDroppedByVote( ) const;
	QString PlayerVotedToDropLaggers( const QString &user ) const;
	QString LatencyIs( const QString &latency ) const;
	QString SyncLimitIs( const QString &synclimit ) const;
	QString SettingSyncLimitToMinimum( const QString &min ) const;
	QString SettingSyncLimitToMaximum( const QString &max ) const;
	QString SettingSyncLimitTo( const QString &synclimit ) const;
	QString UnableToCreateGameNotLoggedIn( const QString &gamename ) const;
	QString AdminLoggedIn( ) const;
	QString AdminInvalidPassword( const QString &attempt ) const;
	QString ConnectingToBNET( const QString &server ) const;
	QString ConnectedToBNET( const QString &server ) const;
	QString DisconnectedFromBNET( const QString &server ) const;
	QString LoggedInToBNET( const QString &server ) const;
	QString BNETGameHostingSucceeded( const QString &server ) const;
	QString BNETGameHostingFailed( const QString &server, const QString &gamename ) const;
	QString ConnectingToBNETTimedOut( const QString &server ) const;
	QString PlayerDownloadedTheMap( const QString &user, const QString &seconds, const QString &rate ) const;
	QString UnableToCreateGameNameTooLong( const QString &gamename ) const;
	QString SettingGameOwnerTo( const QString &owner ) const;
	QString TheGameIsLocked( ) const;
	QString GameLocked( ) const;
	QString GameUnlocked( ) const;
	QString UnableToStartDownloadNoMatchesFound( const QString &victim ) const;
	QString UnableToStartDownloadFoundMoreThanOneMatch( const QString &victim ) const;
	QString UnableToSetGameOwner( const QString &owner ) const;
	QString UnableToCheckPlayerNoMatchesFound( const QString &victim ) const;
	QString CheckedPlayer( const QString &victim, const QString &ping, const QString &from, const QString &admin, const QString &owner, const QString &spoofed, const QString &spoofedrealm, const QString &reserved ) const;
	QString UnableToCheckPlayerFoundMoreThanOneMatch( const QString &victim ) const;
	QString TheGameIsLockedBNET( ) const;
	QString UnableToCreateGameDisabled( const QString &gamename ) const;
	QString BotDisabled( ) const;
	QString BotEnabled( ) const;
	QString UnableToCreateGameInvalidMap( const QString &gamename ) const;
	QString WaitingForPlayersBeforeAutoStart( const QString &players, const QString &playersleft ) const;
	QString AutoStartDisabled( ) const;
	QString AutoStartEnabled( const QString &players ) const;
	QString AnnounceMessageEnabled( ) const;
	QString AnnounceMessageDisabled( ) const;
	QString AutoHostEnabled( ) const;
	QString AutoHostDisabled( ) const;
	QString UnableToLoadSaveGamesOutside( ) const;
	QString UnableToLoadSaveGameGameInLobby( ) const;
	QString LoadingSaveGame( const QString &file ) const;
	QString UnableToLoadSaveGameDoesntExist( const QString &file ) const;
	QString UnableToCreateGameInvalidSaveGame( const QString &gamename ) const;
	QString UnableToCreateGameSaveGameMapMismatch( const QString &gamename ) const;
	QString AutoSaveEnabled( ) const;
	QString AutoSaveDisabled( ) const;
	QString DesyncDetected( ) const;
	QString UnableToMuteNoMatchesFound( const QString &victim ) const;
	QString MutedPlayer( const QString &victim, const QString &user ) const;
	QString UnmutedPlayer( const QString &victim, const QString &user ) const;
	QString UnableToMuteFoundMoreThanOneMatch( const QString &victim ) const;
	QString PlayerIsSavingTheGame( const QString &player ) const;
	QString UpdatingClanList( ) const;
	QString UpdatingFriendsList( ) const;
	QString MultipleIPAddressUsageDetected( const QString &player, const QString &others ) const;
	QString UnableToVoteKickAlreadyInProgress( ) const;
	QString UnableToVoteKickNotEnoughPlayers( ) const;
	QString UnableToVoteKickNoMatchesFound( const QString &victim ) const;
	QString UnableToVoteKickPlayerIsReserved( const QString &victim ) const;
	QString StartedVoteKick( const QString &victim, const QString &user, const QString &votesneeded ) const;
	QString UnableToVoteKickFoundMoreThanOneMatch( const QString &victim ) const;
	QString VoteKickPassed( const QString &victim ) const;
	QString ErrorVoteKickingPlayer( const QString &victim ) const;
	QString VoteKickAcceptedNeedMoreVotes( const QString &victim, const QString &user, const QString &votes ) const;
	QString VoteKickCancelled( const QString &victim ) const;
	QString VoteKickExpired( const QString &victim ) const;
	QString WasKickedByVote( ) const;
	QString TypeYesToVote( const QString &commandtrigger ) const;
	QString PlayersNotYetPingedAutoStart( const QString &notpinged ) const;
	QString WasKickedForNotSpoofChecking( ) const;
	QString WasKickedForHavingFurthestScore( const QString &score, const QString &average ) const;
	QString PlayerHasScore( const QString &player, const QString &score ) const;
	QString RatedPlayersSpread( const QString &rated, const QString &total, const QString &spread ) const;
	QString ErrorListingMaps( ) const;
	QString FoundMaps( const QString &maps ) const;
	QString NoMapsFound( ) const;
	QString ErrorListingMapConfigs( ) const;
	QString FoundMapConfigs( const QString &mapconfigs ) const;
	QString NoMapConfigsFound( ) const;
	QString PlayerFinishedLoading( const QString &user ) const;
	QString PleaseWaitPlayersStillLoading( ) const;
	QString MapDownloadsDisabled( ) const;
	QString MapDownloadsEnabled( ) const;
	QString MapDownloadsConditional( ) const;
	QString SettingHCL( const QString &HCL ) const;
	QString UnableToSetHCLInvalid( ) const;
	QString UnableToSetHCLTooLong( ) const;
	QString TheHCLIs( const QString &HCL ) const;
	QString TheHCLIsTooLongUseForceToStart( ) const;
	QString ClearingHCL( ) const;
	QString TryingToRehostAsPrivateGame( const QString &gamename ) const;
	QString TryingToRehostAsPublicGame( const QString &gamename ) const;
	QString RehostWasSuccessful( ) const;
	QString TryingToJoinTheGameButBannedByName( const QString &victim ) const;
	QString TryingToJoinTheGameButBannedByIP( const QString &victim, const QString &ip, const QString &bannedname ) const;
	QString HasBannedName( const QString &victim ) const;
	QString HasBannedIP( const QString &victim, const QString &ip, const QString &bannedname ) const;
	QString PlayersInGameState( const QString &number, const QString &players ) const;
	QString ValidServers( const QString &servers ) const;
	QString TeamCombinedScore( const QString &team, const QString &score ) const;
	QString BalancingSlotsCompleted( ) const;
	QString PlayerWasKickedForFurthestScore( const QString &name, const QString &score, const QString &average ) const;
	QString LocalAdminMessagesEnabled( ) const;
	QString LocalAdminMessagesDisabled( ) const;
	QString WasDroppedDesync( ) const;
	QString WasKickedForHavingLowestScore( const QString &score ) const;
	QString PlayerWasKickedForLowestScore( const QString &name, const QString &score ) const;
	QString ReloadingConfigurationFiles( ) const;
	QString CountDownAbortedSomeoneLeftRecently( ) const;
	QString UnableToCreateGameMustEnforceFirst( const QString &gamename ) const;
	QString UnableToLoadReplaysOutside( ) const;
	QString LoadingReplay( const QString &file ) const;
	QString UnableToLoadReplayDoesntExist( const QString &file ) const;
	QString CommandTrigger( const QString &trigger ) const;
	QString CantEndGameOwnerIsStillPlaying( const QString &owner ) const;
	QString CantUnhostGameOwnerIsPresent( const QString &owner ) const;
	QString WasAutomaticallyDroppedAfterSeconds( const QString &seconds ) const;
	QString HasLostConnectionTimedOutGProxy( ) const;
	QString HasLostConnectionSocketErrorGProxy( const QString &error ) const;
	QString HasLostConnectionClosedByRemoteHostGProxy( ) const;
	QString WaitForReconnectSecondsRemain( const QString &seconds ) const;
	QString WasUnrecoverablyDroppedFromGProxy( ) const;
	QString PlayerReconnectedWithGProxy( const QString &name ) const;
};

#endif
