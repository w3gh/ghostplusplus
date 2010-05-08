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

	QString UnableToCreateGameTryAnotherName( const QString &server, const QString &gamename );
	QString UserIsAlreadyAnAdmin( const QString &server, const QString &user );
	QString AddedUserToAdminDatabase( const QString &server, const QString &user );
	QString ErrorAddingUserToAdminDatabase( const QString &server, const QString &user );
	QString YouDontHaveAccessToThatCommand( );
	QString UserIsAlreadyBanned( const QString &server, const QString &victim );
	QString BannedUser( const QString &server, const QString &victim );
	QString ErrorBanningUser( const QString &server, const QString &victim );
	QString UserIsAnAdmin( const QString &server, const QString &user );
	QString UserIsNotAnAdmin( const QString &server, const QString &user );
	QString UserWasBannedOnByBecause( const QString &server, const QString &victim, const QString &date, const QString &admin, const QString &reason );
	QString UserIsNotBanned( const QString &server, const QString &victim );
	QString ThereAreNoAdmins( const QString &server );
	QString ThereIsAdmin( const QString &server );
	QString ThereAreAdmins( const QString &server, const QString &count );
	QString ThereAreNoBannedUsers( const QString &server );
	QString ThereIsBannedUser( const QString &server );
	QString ThereAreBannedUsers( const QString &server, const QString &count );
	QString YouCantDeleteTheRootAdmin( );
	QString DeletedUserFromAdminDatabase( const QString &server, const QString &user );
	QString ErrorDeletingUserFromAdminDatabase( const QString &server, const QString &user );
	QString UnbannedUser( const QString &victim );
	QString ErrorUnbanningUser( const QString &victim );
	QString GameNumberIs( const QString &number, const QString &description );
	QString GameNumberDoesntExist( const QString &number );
	QString GameIsInTheLobby( const QString &description, const QString &current, const QString &max );
	QString ThereIsNoGameInTheLobby( const QString &current, const QString &max );
	QString UnableToLoadConfigFilesOutside( );
	QString LoadingConfigFile( const QString &file );
	QString UnableToLoadConfigFileDoesntExist( const QString &file );
	QString CreatingPrivateGame( const QString &gamename, const QString &user );
	QString CreatingPublicGame( const QString &gamename, const QString &user );
	QString UnableToUnhostGameCountdownStarted( const QString &description );
	QString UnhostingGame( const QString &description );
	QString UnableToUnhostGameNoGameInLobby( );
	QString VersionAdmin( const QString &version );
	QString VersionNotAdmin( const QString &version );
	QString UnableToCreateGameAnotherGameInLobby( const QString &gamename, const QString &description );
	QString UnableToCreateGameMaxGamesReached( const QString &gamename, const QString &max );
	QString GameIsOver( const QString &description );
	QString SpoofCheckByReplying( );
	QString GameRefreshed( );
	QString SpoofPossibleIsAway( const QString &user );
	QString SpoofPossibleIsUnavailable( const QString &user );
	QString SpoofPossibleIsRefusingMessages( const QString &user );
	QString SpoofDetectedIsNotInGame( const QString &user );
	QString SpoofDetectedIsInPrivateChannel( const QString &user );
	QString SpoofDetectedIsInAnotherGame( const QString &user );
	QString CountDownAborted( );
	QString TryingToJoinTheGameButBanned( const QString &victim );
	QString UnableToBanNoMatchesFound( const QString &victim );
	QString PlayerWasBannedByPlayer( const QString &server, const QString &victim, const QString &user );
	QString UnableToBanFoundMoreThanOneMatch( const QString &victim );
	QString AddedPlayerToTheHoldList( const QString &user );
	QString UnableToKickNoMatchesFound( const QString &victim );
	QString UnableToKickFoundMoreThanOneMatch( const QString &victim );
	QString SettingLatencyToMinimum( const QString &min );
	QString SettingLatencyToMaximum( const QString &max );
	QString SettingLatencyTo( const QString &latency );
	QString KickingPlayersWithPingsGreaterThan( const QString &total, const QString &ping );
	QString HasPlayedGamesWithThisBot( const QString &user, const QString &firstgame, const QString &lastgame, const QString &totalgames, const QString &avgloadingtime, const QString &avgstay );
	QString HasntPlayedGamesWithThisBot( const QString &user );
	QString AutokickingPlayerForExcessivePing( const QString &victim, const QString &ping );
	QString SpoofCheckAcceptedFor( const QString &server, const QString &user );
	QString PlayersNotYetSpoofChecked( const QString &notspoofchecked );
	QString ManuallySpoofCheckByWhispering( const QString &hostname );
	QString SpoofCheckByWhispering( const QString &hostname );
	QString EveryoneHasBeenSpoofChecked( );
	QString PlayersNotYetPinged( const QString &notpinged );
	QString EveryoneHasBeenPinged( );
	QString ShortestLoadByPlayer( const QString &user, const QString &loadingtime );
	QString LongestLoadByPlayer( const QString &user, const QString &loadingtime );
	QString YourLoadingTimeWas( const QString &loadingtime );
	QString HasPlayedDotAGamesWithThisBot( const QString &user, const QString &totalgames, const QString &totalwins, const QString &totallosses, const QString &totalkills, const QString &totaldeaths, const QString &totalcreepkills, const QString &totalcreepdenies, const QString &totalassists, const QString &totalneutralkills, const QString &totaltowerkills, const QString &totalraxkills, const QString &totalcourierkills, const QString &avgkills, const QString &avgdeaths, const QString &avgcreepkills, const QString &avgcreepdenies, const QString &avgassists, const QString &avgneutralkills, const QString &avgtowerkills, const QString &avgraxkills, const QString &avgcourierkills );
	QString HasntPlayedDotAGamesWithThisBot( const QString &user );
	QString WasKickedForReservedPlayer( const QString &reserved );
	QString WasKickedForOwnerPlayer( const QString &owner );
	QString WasKickedByPlayer( const QString &user );
	QString HasLostConnectionPlayerError( const QString &error );
	QString HasLostConnectionSocketError( const QString &error );
	QString HasLostConnectionClosedByRemoteHost( );
	QString HasLeftVoluntarily( );
	QString EndingGame( const QString &description );
	QString HasLostConnectionTimedOut( );
	QString GlobalChatMuted( );
	QString GlobalChatUnmuted( );
	QString ShufflingPlayers( );
	QString UnableToLoadConfigFileGameInLobby( );
	QString PlayersStillDownloading( const QString &stilldownloading );
	QString RefreshMessagesEnabled( );
	QString RefreshMessagesDisabled( );
	QString AtLeastOneGameActiveUseForceToShutdown( );
	QString CurrentlyLoadedMapCFGIs( const QString &mapcfg );
	QString LaggedOutDroppedByAdmin( );
	QString LaggedOutDroppedByVote( );
	QString PlayerVotedToDropLaggers( const QString &user );
	QString LatencyIs( const QString &latency );
	QString SyncLimitIs( const QString &synclimit );
	QString SettingSyncLimitToMinimum( const QString &min );
	QString SettingSyncLimitToMaximum( const QString &max );
	QString SettingSyncLimitTo( const QString &synclimit );
	QString UnableToCreateGameNotLoggedIn( const QString &gamename );
	QString AdminLoggedIn( );
	QString AdminInvalidPassword( const QString &attempt );
	QString ConnectingToBNET( const QString &server );
	QString ConnectedToBNET( const QString &server );
	QString DisconnectedFromBNET( const QString &server );
	QString LoggedInToBNET( const QString &server );
	QString BNETGameHostingSucceeded( const QString &server );
	QString BNETGameHostingFailed( const QString &server, const QString &gamename );
	QString ConnectingToBNETTimedOut( const QString &server );
	QString PlayerDownloadedTheMap( const QString &user, const QString &seconds, const QString &rate );
	QString UnableToCreateGameNameTooLong( const QString &gamename );
	QString SettingGameOwnerTo( const QString &owner );
	QString TheGameIsLocked( );
	QString GameLocked( );
	QString GameUnlocked( );
	QString UnableToStartDownloadNoMatchesFound( const QString &victim );
	QString UnableToStartDownloadFoundMoreThanOneMatch( const QString &victim );
	QString UnableToSetGameOwner( const QString &owner );
	QString UnableToCheckPlayerNoMatchesFound( const QString &victim );
	QString CheckedPlayer( const QString &victim, const QString &ping, const QString &from, const QString &admin, const QString &owner, const QString &spoofed, const QString &spoofedrealm, const QString &reserved );
	QString UnableToCheckPlayerFoundMoreThanOneMatch( const QString &victim );
	QString TheGameIsLockedBNET( );
	QString UnableToCreateGameDisabled( const QString &gamename );
	QString BotDisabled( );
	QString BotEnabled( );
	QString UnableToCreateGameInvalidMap( const QString &gamename );
	QString WaitingForPlayersBeforeAutoStart( const QString &players, const QString &playersleft );
	QString AutoStartDisabled( );
	QString AutoStartEnabled( const QString &players );
	QString AnnounceMessageEnabled( );
	QString AnnounceMessageDisabled( );
	QString AutoHostEnabled( );
	QString AutoHostDisabled( );
	QString UnableToLoadSaveGamesOutside( );
	QString UnableToLoadSaveGameGameInLobby( );
	QString LoadingSaveGame( const QString &file );
	QString UnableToLoadSaveGameDoesntExist( const QString &file );
	QString UnableToCreateGameInvalidSaveGame( const QString &gamename );
	QString UnableToCreateGameSaveGameMapMismatch( const QString &gamename );
	QString AutoSaveEnabled( );
	QString AutoSaveDisabled( );
	QString DesyncDetected( );
	QString UnableToMuteNoMatchesFound( const QString &victim );
	QString MutedPlayer( const QString &victim, const QString &user );
	QString UnmutedPlayer( const QString &victim, const QString &user );
	QString UnableToMuteFoundMoreThanOneMatch( const QString &victim );
	QString PlayerIsSavingTheGame( const QString &player );
	QString UpdatingClanList( );
	QString UpdatingFriendsList( );
	QString MultipleIPAddressUsageDetected( const QString &player, const QString &others );
	QString UnableToVoteKickAlreadyInProgress( );
	QString UnableToVoteKickNotEnoughPlayers( );
	QString UnableToVoteKickNoMatchesFound( const QString &victim );
	QString UnableToVoteKickPlayerIsReserved( const QString &victim );
	QString StartedVoteKick( const QString &victim, const QString &user, const QString &votesneeded );
	QString UnableToVoteKickFoundMoreThanOneMatch( const QString &victim );
	QString VoteKickPassed( const QString &victim );
	QString ErrorVoteKickingPlayer( const QString &victim );
	QString VoteKickAcceptedNeedMoreVotes( const QString &victim, const QString &user, const QString &votes );
	QString VoteKickCancelled( const QString &victim );
	QString VoteKickExpired( const QString &victim );
	QString WasKickedByVote( );
	QString TypeYesToVote( const QString &commandtrigger );
	QString PlayersNotYetPingedAutoStart( const QString &notpinged );
	QString WasKickedForNotSpoofChecking( );
	QString WasKickedForHavingFurthestScore( const QString &score, const QString &average );
	QString PlayerHasScore( const QString &player, const QString &score );
	QString RatedPlayersSpread( const QString &rated, const QString &total, const QString &spread );
	QString ErrorListingMaps( );
	QString FoundMaps( const QString &maps );
	QString NoMapsFound( );
	QString ErrorListingMapConfigs( );
	QString FoundMapConfigs( const QString &mapconfigs );
	QString NoMapConfigsFound( );
	QString PlayerFinishedLoading( const QString &user );
	QString PleaseWaitPlayersStillLoading( );
	QString MapDownloadsDisabled( );
	QString MapDownloadsEnabled( );
	QString MapDownloadsConditional( );
	QString SettingHCL( const QString &HCL );
	QString UnableToSetHCLInvalid( );
	QString UnableToSetHCLTooLong( );
	QString TheHCLIs( const QString &HCL );
	QString TheHCLIsTooLongUseForceToStart( );
	QString ClearingHCL( );
	QString TryingToRehostAsPrivateGame( const QString &gamename );
	QString TryingToRehostAsPublicGame( const QString &gamename );
	QString RehostWasSuccessful( );
	QString TryingToJoinTheGameButBannedByName( const QString &victim );
	QString TryingToJoinTheGameButBannedByIP( const QString &victim, const QString &ip, const QString &bannedname );
	QString HasBannedName( const QString &victim );
	QString HasBannedIP( const QString &victim, const QString &ip, const QString &bannedname );
	QString PlayersInGameState( const QString &number, const QString &players );
	QString ValidServers( const QString &servers );
	QString TeamCombinedScore( const QString &team, const QString &score );
	QString BalancingSlotsCompleted( );
	QString PlayerWasKickedForFurthestScore( const QString &name, const QString &score, const QString &average );
	QString LocalAdminMessagesEnabled( );
	QString LocalAdminMessagesDisabled( );
	QString WasDroppedDesync( );
	QString WasKickedForHavingLowestScore( const QString &score );
	QString PlayerWasKickedForLowestScore( const QString &name, const QString &score );
	QString ReloadingConfigurationFiles( );
	QString CountDownAbortedSomeoneLeftRecently( );
	QString UnableToCreateGameMustEnforceFirst( const QString &gamename );
	QString UnableToLoadReplaysOutside( );
	QString LoadingReplay( const QString &file );
	QString UnableToLoadReplayDoesntExist( const QString &file );
	QString CommandTrigger( const QString &trigger );
	QString CantEndGameOwnerIsStillPlaying( const QString &owner );
	QString CantUnhostGameOwnerIsPresent( const QString &owner );
	QString WasAutomaticallyDroppedAfterSeconds( const QString &seconds );
	QString HasLostConnectionTimedOutGProxy( );
	QString HasLostConnectionSocketErrorGProxy( const QString &error );
	QString HasLostConnectionClosedByRemoteHostGProxy( );
	QString WaitForReconnectSecondsRemain( const QString &seconds );
	QString WasUnrecoverablyDroppedFromGProxy( );
	QString PlayerReconnectedWithGProxy( const QString &name );
};

#endif
