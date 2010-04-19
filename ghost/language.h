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

//
// CLanguage
//

class CLanguage
{
private:
	CConfig *m_CFG;

public:
	CLanguage( QString nCFGFile );
	~CLanguage( );

	QString UnableToCreateGameTryAnotherName( QString server, QString gamename );
	QString UserIsAlreadyAnAdmin( QString server, QString user );
	QString AddedUserToAdminDatabase( QString server, QString user );
	QString ErrorAddingUserToAdminDatabase( QString server, QString user );
	QString YouDontHaveAccessToThatCommand( );
	QString UserIsAlreadyBanned( QString server, QString victim );
	QString BannedUser( QString server, QString victim );
	QString ErrorBanningUser( QString server, QString victim );
	QString UserIsAnAdmin( QString server, QString user );
	QString UserIsNotAnAdmin( QString server, QString user );
	QString UserWasBannedOnByBecause( QString server, QString victim, QString date, QString admin, QString reason );
	QString UserIsNotBanned( QString server, QString victim );
	QString ThereAreNoAdmins( QString server );
	QString ThereIsAdmin( QString server );
	QString ThereAreAdmins( QString server, QString count );
	QString ThereAreNoBannedUsers( QString server );
	QString ThereIsBannedUser( QString server );
	QString ThereAreBannedUsers( QString server, QString count );
	QString YouCantDeleteTheRootAdmin( );
	QString DeletedUserFromAdminDatabase( QString server, QString user );
	QString ErrorDeletingUserFromAdminDatabase( QString server, QString user );
	QString UnbannedUser( QString victim );
	QString ErrorUnbanningUser( QString victim );
	QString GameNumberIs( QString number, QString description );
	QString GameNumberDoesntExist( QString number );
	QString GameIsInTheLobby( QString description, QString current, QString max );
	QString ThereIsNoGameInTheLobby( QString current, QString max );
	QString UnableToLoadConfigFilesOutside( );
	QString LoadingConfigFile( QString file );
	QString UnableToLoadConfigFileDoesntExist( QString file );
	QString CreatingPrivateGame( QString gamename, QString user );
	QString CreatingPublicGame( QString gamename, QString user );
	QString UnableToUnhostGameCountdownStarted( QString description );
	QString UnhostingGame( QString description );
	QString UnableToUnhostGameNoGameInLobby( );
	QString VersionAdmin( QString version );
	QString VersionNotAdmin( QString version );
	QString UnableToCreateGameAnotherGameInLobby( QString gamename, QString description );
	QString UnableToCreateGameMaxGamesReached( QString gamename, QString max );
	QString GameIsOver( QString description );
	QString SpoofCheckByReplying( );
	QString GameRefreshed( );
	QString SpoofPossibleIsAway( QString user );
	QString SpoofPossibleIsUnavailable( QString user );
	QString SpoofPossibleIsRefusingMessages( QString user );
	QString SpoofDetectedIsNotInGame( QString user );
	QString SpoofDetectedIsInPrivateChannel( QString user );
	QString SpoofDetectedIsInAnotherGame( QString user );
	QString CountDownAborted( );
	QString TryingToJoinTheGameButBanned( QString victim );
	QString UnableToBanNoMatchesFound( QString victim );
	QString PlayerWasBannedByPlayer( QString server, QString victim, QString user );
	QString UnableToBanFoundMoreThanOneMatch( QString victim );
	QString AddedPlayerToTheHoldList( QString user );
	QString UnableToKickNoMatchesFound( QString victim );
	QString UnableToKickFoundMoreThanOneMatch( QString victim );
	QString SettingLatencyToMinimum( QString min );
	QString SettingLatencyToMaximum( QString max );
	QString SettingLatencyTo( QString latency );
	QString KickingPlayersWithPingsGreaterThan( QString total, QString ping );
	QString HasPlayedGamesWithThisBot( QString user, QString firstgame, QString lastgame, QString totalgames, QString avgloadingtime, QString avgstay );
	QString HasntPlayedGamesWithThisBot( QString user );
	QString AutokickingPlayerForExcessivePing( QString victim, QString ping );
	QString SpoofCheckAcceptedFor( QString server, QString user );
	QString PlayersNotYetSpoofChecked( QString notspoofchecked );
	QString ManuallySpoofCheckByWhispering( QString hostname );
	QString SpoofCheckByWhispering( QString hostname );
	QString EveryoneHasBeenSpoofChecked( );
	QString PlayersNotYetPinged( QString notpinged );
	QString EveryoneHasBeenPinged( );
	QString ShortestLoadByPlayer( QString user, QString loadingtime );
	QString LongestLoadByPlayer( QString user, QString loadingtime );
	QString YourLoadingTimeWas( QString loadingtime );
	QString HasPlayedDotAGamesWithThisBot( QString user, QString totalgames, QString totalwins, QString totallosses, QString totalkills, QString totaldeaths, QString totalcreepkills, QString totalcreepdenies, QString totalassists, QString totalneutralkills, QString totaltowerkills, QString totalraxkills, QString totalcourierkills, QString avgkills, QString avgdeaths, QString avgcreepkills, QString avgcreepdenies, QString avgassists, QString avgneutralkills, QString avgtowerkills, QString avgraxkills, QString avgcourierkills );
	QString HasntPlayedDotAGamesWithThisBot( QString user );
	QString WasKickedForReservedPlayer( QString reserved );
	QString WasKickedForOwnerPlayer( QString owner );
	QString WasKickedByPlayer( QString user );
	QString HasLostConnectionPlayerError( QString error );
	QString HasLostConnectionSocketError( QString error );
	QString HasLostConnectionClosedByRemoteHost( );
	QString HasLeftVoluntarily( );
	QString EndingGame( QString description );
	QString HasLostConnectionTimedOut( );
	QString GlobalChatMuted( );
	QString GlobalChatUnmuted( );
	QString ShufflingPlayers( );
	QString UnableToLoadConfigFileGameInLobby( );
	QString PlayersStillDownloading( QString stilldownloading );
	QString RefreshMessagesEnabled( );
	QString RefreshMessagesDisabled( );
	QString AtLeastOneGameActiveUseForceToShutdown( );
	QString CurrentlyLoadedMapCFGIs( QString mapcfg );
	QString LaggedOutDroppedByAdmin( );
	QString LaggedOutDroppedByVote( );
	QString PlayerVotedToDropLaggers( QString user );
	QString LatencyIs( QString latency );
	QString SyncLimitIs( QString synclimit );
	QString SettingSyncLimitToMinimum( QString min );
	QString SettingSyncLimitToMaximum( QString max );
	QString SettingSyncLimitTo( QString synclimit );
	QString UnableToCreateGameNotLoggedIn( QString gamename );
	QString AdminLoggedIn( );
	QString AdminInvalidPassword( QString attempt );
	QString ConnectingToBNET( QString server );
	QString ConnectedToBNET( QString server );
	QString DisconnectedFromBNET( QString server );
	QString LoggedInToBNET( QString server );
	QString BNETGameHostingSucceeded( QString server );
	QString BNETGameHostingFailed( QString server, QString gamename );
	QString ConnectingToBNETTimedOut( QString server );
	QString PlayerDownloadedTheMap( QString user, QString seconds, QString rate );
	QString UnableToCreateGameNameTooLong( QString gamename );
	QString SettingGameOwnerTo( QString owner );
	QString TheGameIsLocked( );
	QString GameLocked( );
	QString GameUnlocked( );
	QString UnableToStartDownloadNoMatchesFound( QString victim );
	QString UnableToStartDownloadFoundMoreThanOneMatch( QString victim );
	QString UnableToSetGameOwner( QString owner );
	QString UnableToCheckPlayerNoMatchesFound( QString victim );
	QString CheckedPlayer( QString victim, QString ping, QString from, QString admin, QString owner, QString spoofed, QString spoofedrealm, QString reserved );
	QString UnableToCheckPlayerFoundMoreThanOneMatch( QString victim );
	QString TheGameIsLockedBNET( );
	QString UnableToCreateGameDisabled( QString gamename );
	QString BotDisabled( );
	QString BotEnabled( );
	QString UnableToCreateGameInvalidMap( QString gamename );
	QString WaitingForPlayersBeforeAutoStart( QString players, QString playersleft );
	QString AutoStartDisabled( );
	QString AutoStartEnabled( QString players );
	QString AnnounceMessageEnabled( );
	QString AnnounceMessageDisabled( );
	QString AutoHostEnabled( );
	QString AutoHostDisabled( );
	QString UnableToLoadSaveGamesOutside( );
	QString UnableToLoadSaveGameGameInLobby( );
	QString LoadingSaveGame( QString file );
	QString UnableToLoadSaveGameDoesntExist( QString file );
	QString UnableToCreateGameInvalidSaveGame( QString gamename );
	QString UnableToCreateGameSaveGameMapMismatch( QString gamename );
	QString AutoSaveEnabled( );
	QString AutoSaveDisabled( );
	QString DesyncDetected( );
	QString UnableToMuteNoMatchesFound( QString victim );
	QString MutedPlayer( QString victim, QString user );
	QString UnmutedPlayer( QString victim, QString user );
	QString UnableToMuteFoundMoreThanOneMatch( QString victim );
	QString PlayerIsSavingTheGame( QString player );
	QString UpdatingClanList( );
	QString UpdatingFriendsList( );
	QString MultipleIPAddressUsageDetected( QString player, QString others );
	QString UnableToVoteKickAlreadyInProgress( );
	QString UnableToVoteKickNotEnoughPlayers( );
	QString UnableToVoteKickNoMatchesFound( QString victim );
	QString UnableToVoteKickPlayerIsReserved( QString victim );
	QString StartedVoteKick( QString victim, QString user, QString votesneeded );
	QString UnableToVoteKickFoundMoreThanOneMatch( QString victim );
	QString VoteKickPassed( QString victim );
	QString ErrorVoteKickingPlayer( QString victim );
	QString VoteKickAcceptedNeedMoreVotes( QString victim, QString user, QString votes );
	QString VoteKickCancelled( QString victim );
	QString VoteKickExpired( QString victim );
	QString WasKickedByVote( );
	QString TypeYesToVote( QString commandtrigger );
	QString PlayersNotYetPingedAutoStart( QString notpinged );
	QString WasKickedForNotSpoofChecking( );
	QString WasKickedForHavingFurthestScore( QString score, QString average );
	QString PlayerHasScore( QString player, QString score );
	QString RatedPlayersSpread( QString rated, QString total, QString spread );
	QString ErrorListingMaps( );
	QString FoundMaps( QString maps );
	QString NoMapsFound( );
	QString ErrorListingMapConfigs( );
	QString FoundMapConfigs( QString mapconfigs );
	QString NoMapConfigsFound( );
	QString PlayerFinishedLoading( QString user );
	QString PleaseWaitPlayersStillLoading( );
	QString MapDownloadsDisabled( );
	QString MapDownloadsEnabled( );
	QString MapDownloadsConditional( );
	QString SettingHCL( QString HCL );
	QString UnableToSetHCLInvalid( );
	QString UnableToSetHCLTooLong( );
	QString TheHCLIs( QString HCL );
	QString TheHCLIsTooLongUseForceToStart( );
	QString ClearingHCL( );
	QString TryingToRehostAsPrivateGame( QString gamename );
	QString TryingToRehostAsPublicGame( QString gamename );
	QString RehostWasSuccessful( );
	QString TryingToJoinTheGameButBannedByName( QString victim );
	QString TryingToJoinTheGameButBannedByIP( QString victim, QString ip, QString bannedname );
	QString HasBannedName( QString victim );
	QString HasBannedIP( QString victim, QString ip, QString bannedname );
	QString PlayersInGameState( QString number, QString players );
	QString ValidServers( QString servers );
	QString TeamCombinedScore( QString team, QString score );
	QString BalancingSlotsCompleted( );
	QString PlayerWasKickedForFurthestScore( QString name, QString score, QString average );
	QString LocalAdminMessagesEnabled( );
	QString LocalAdminMessagesDisabled( );
	QString WasDroppedDesync( );
	QString WasKickedForHavingLowestScore( QString score );
	QString PlayerWasKickedForLowestScore( QString name, QString score );
	QString ReloadingConfigurationFiles( );
	QString CountDownAbortedSomeoneLeftRecently( );
	QString UnableToCreateGameMustEnforceFirst( QString gamename );
	QString UnableToLoadReplaysOutside( );
	QString LoadingReplay( QString file );
	QString UnableToLoadReplayDoesntExist( QString file );
	QString CommandTrigger( QString trigger );
	QString CantEndGameOwnerIsStillPlaying( QString owner );
	QString CantUnhostGameOwnerIsPresent( QString owner );
	QString WasAutomaticallyDroppedAfterSeconds( QString seconds );
	QString HasLostConnectionTimedOutGProxy( );
	QString HasLostConnectionSocketErrorGProxy( QString error );
	QString HasLostConnectionClosedByRemoteHostGProxy( );
	QString WaitForReconnectSecondsRemain( QString seconds );
	QString WasUnrecoverablyDroppedFromGProxy( );
	QString PlayerReconnectedWithGProxy( QString name );
};

#endif
