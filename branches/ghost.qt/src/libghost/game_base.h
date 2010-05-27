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

#ifndef GAME_BASE_H
#define GAME_BASE_H

#include "includes.h"
#include "gameslot.h"

//
// CBaseGame
//

class CTCPServer;
class CGameProtocol;
class CPotentialPlayer;
class CGamePlayer;
class CMap;
class CSaveGame;
class CReplay;
class CIncomingJoinPlayer;
class CIncomingAction;
class CIncomingChatPlayer;
class CIncomingMapSize;
class CCallableScoreCheck;
class CGHost;
QT_FORWARD_DECLARE_CLASS(QTcpServer)

#include <QTimer>
#include <QSet>
#include <QStringList>

class CBaseGame : public QObject
{
	Q_OBJECT

signals:
	void startedLoading();
	void finishedLoading();
	void SignalPlayerCommand( CBaseGame *game, CGamePlayer *player, const QString &command, const QString &payload );

public:
	CGHost *m_GHost;

public slots:
	virtual void EventGameStarted( );
	virtual void EventGameLoaded( );
	void EventNewConnection();
	void EventBroadcastTimeout();
	void EventRefreshTimeout();
	void EventRefreshError();
	void EventTryAutoRehost();
	void EventMapDataTimeout();
	void EventCountdownTimeout();
	void EventAutostartTimeout();
	void EventGameOverTimeout();
	void EventVotekickTimeout();
	virtual void EventCallableUpdateTimeout();
	void EventResetDownloadCounter();
	void EventAnnounceTimeout();
	void EventLobbyTimeout();
	void EventPlayerStoppedLagging();
	void CheckPlayersStartedLaggging();
	void EventLoadInGameTimeout();
	void EventResetLagscreenTimeout();
	void EventDropLaggerTimeout();
	void EventPlayerLaggedOut(CGamePlayer *player);
	void EventSendActions();
	virtual void SendAllSlotInfo();
	virtual void EventPlayerLoaded();
	virtual void EventPlayerDeleted();

protected:
	QTimer m_BroadcastTimer, m_RefreshTimer, m_MapDataTimer, m_CountdownTimer, m_AutostartTimer, m_GameOverTimer, m_VotekickTimer,
		m_SlotInfoTimer, m_DownloadCounterTimer, m_CallableUpdateTimer, m_AnnounceTimer, m_LobbyTimeoutTimer,
		m_DropLaggerTimer, m_LoadInGameTimer, m_ResetLagscreenTimer, m_SendActionTimer;

	void CheckGameLoaded();

protected:
	QTcpServer *m_Socket;							// listening socket
	CGameProtocol *m_Protocol;						// game protocol
	QList<CGameSlot> m_Slots;						// vector of slots
	QList<CPotentialPlayer *> m_Potentials;		// vector of potential players (connections that haven't sent a W3GS_REQJOIN packet yet)
	QList<CGamePlayer *> m_Players;				// vector of players
	QList<CCallableScoreCheck *> m_ScoreChecks;
	QQueue<CIncomingAction *> m_Actions;				// queue of actions to be sent
	QStringList m_Reserved;						// vector of player names with reserved slots (from the !hold command)
	QSet<QString> m_IgnoredNames;						// set of player names to NOT print ban messages for when joining because they've already been printed
	QSet<QString> m_IPBlackList;						// set of IP addresses to blacklist from joining (todotodo: convert to uint32's for efficiency)
	QList<CGameSlot> m_EnforceSlots;				// vector of slots to force players to use (used with saved games)
	QList<PIDPlayer> m_EnforcePlayers;				// vector of pids to force players to use (used with saved games)
	CMap *m_Map;									// map data
	CSaveGame *m_SaveGame;							// savegame data (this is a pointer to global data)
	CReplay *m_Replay;								// replay
	bool m_Saving;									// if we're currently saving game data to the database
	quint16 m_HostPort;							// the port to host games on
	unsigned char m_GameState;						// game state, public or private
	unsigned char m_VirtualHostPID;					// virtual host's PID
	unsigned char m_FakePlayerPID;					// the fake player's PID (if present)
	unsigned char m_GProxyEmptyActions;
	QString m_GameName;								// game name
	QString m_LastGameName;							// last game name (the previous game name before it was rehosted)
	QString m_VirtualHostName;						// virtual host's name
	QString m_OwnerName;								// name of the player who owns this game (should be considered an admin)
	QString m_CreatorName;							// name of the player who created this game
	QString m_CreatorServer;							// battle.net server the player who created this game was on
	QString m_AnnounceMessage;						// a message to be sent every m_AnnounceInterval seconds
	QString m_StatString;							// the stat QString when the game started (used when saving replays)
	QString m_KickVotePlayer;						// the player to be kicked with the currently running kick vote
	QString m_HCLCommandString;						// the "HostBot Command Library" command QString, used to pass a limited amount of data to specially designed maps
	quint32 m_WaitTime;
	quint32 m_RandomSeed;							// the random seed sent to the Warcraft III clients
	quint32 m_HostCounter;							// a unique game number
	quint32 m_Latency;								// the number of ms to wait between sending action packets (we queue any received during this time)
	quint32 m_RequestedLatency;
	quint32 m_SyncLimit;							// the maximum number of packets a player can fall out of sync before starting the lag screen
	quint32 m_SyncCounter;							// the number of actions sent so far (for determining if anyone is lagging)
	quint32 m_GameTicks;							// ingame ticks
	quint32 m_CreationTime;						// GetTime when the game was created
	quint32 m_LastDownloadTicks;					// GetTicks when the last map download cycle was performed
	quint32 m_DownloadCounter;						// # of map bytes downloaded in the last second
	quint32 m_AutoStartPlayers;					// auto start the game when there are this many players or more
	quint32 m_CountDownCounter;					// the countdown is finished when this reaches zero
	quint32 m_StartedLoadingTicks;					// GetTicks when the game started loading
	quint32 m_StartPlayers;						// number of players when the game started
	quint32 m_LastActionSentTicks;					// GetTicks when the last action packet was sent
	quint32 m_StartedLaggingTime;					// GetTime when the last lag screen started
	quint32 m_LastPlayerLeaveTicks;				// GetTicks when the most recent player left the game
	static quint32 m_GlobalHostCounter;					// the current host counter (a unique number to identify a game, incremented each time a game is created)
	double m_MinimumScore;							// the minimum allowed score for matchmaking mode
	double m_MaximumScore;							// the maximum allowed score for matchmaking mode
	bool m_Locked;									// if the game owner is the only one allowed to run game commands or not
	bool m_RefreshMessages;							// if we should display "game refreshed..." messages or not
	bool m_RefreshError;							// if there was an error refreshing the game
	bool m_RefreshRehosted;							// if we just rehosted and are waiting for confirmation that it was successful
	bool m_MuteAll;									// if we should stop forwarding ingame chat messages targeted for all players or not
	bool m_MuteLobby;								// if we should stop forwarding lobby chat messages
	bool m_CountDownStarted;						// if the game start countdown has started or not
	bool m_GameLoading;								// if the game is currently loading or not
	bool m_GameLoaded;								// if the game has loaded or not
	bool m_LoadInGame;								// if the load-in-game feature is enabled or not
	bool m_Lagging;									// if the lag screen is active or not
	bool m_AutoSave;								// if we should auto save the game before someone disconnects
	bool m_MatchMaking;								// if matchmaking mode is enabled
	bool m_LocalAdminMessages;						// if local admin messages should be relayed or not

	quint32 GetNewHostCounter( ) { return m_GlobalHostCounter++; }

public:
	CBaseGame( CGHost *nGHost, CMap *nMap, CSaveGame *nSaveGame, quint16 nHostPort, unsigned char nGameState, QString nGameName, QString nOwnerName, QString nCreatorName, QString nCreatorServer );
	virtual ~CBaseGame( );

	static quint32 GetCurrentHostCounter( ) { return m_GlobalHostCounter; }


	virtual const QList<CGameSlot> &GetEnforceSlots( )	{ return m_EnforceSlots; }
	virtual const QList<PIDPlayer> &GetEnforcePlayers( )	{ return m_EnforcePlayers; }
	virtual CSaveGame *GetSaveGame( ) const				{ return m_SaveGame; }
	virtual quint16 GetHostPort( ) const					{ return m_HostPort; }
	virtual unsigned char GetGameState( ) const			{ return m_GameState; }
	virtual unsigned char GetGProxyEmptyActions( ) const	{ return m_GProxyEmptyActions; }
	virtual const QString &GetGameName( ) const					{ return m_GameName; }
	virtual const QString &GetLastGameName( ) const				{ return m_LastGameName; }
	virtual const QString &GetVirtualHostName( ) const			{ return m_VirtualHostName; }
	virtual const QString &GetOwnerName( ) const					{ return m_OwnerName; }
	virtual const QString &GetCreatorName( ) const				{ return m_CreatorName; }
	virtual const QString &GetCreatorServer( ) const				{ return m_CreatorServer; }
	virtual quint32 GetHostCounter( ) const				{ return m_HostCounter; }
	virtual quint32 GetStartedLaggingTime( ) const		{ return m_StartedLaggingTime; }
	virtual quint32 GetAutoStartPlayers( ) const			{ return m_AutoStartPlayers; }
	virtual bool GetLocked( ) const						{ return m_Locked; }
	virtual bool GetRefreshMessages( ) const				{ return m_RefreshMessages; }
	virtual bool GetCountDownStarted( ) const				{ return m_CountDownStarted; }
	virtual bool GetGameLoading( ) const					{ return m_GameLoading; }
	virtual bool GetGameLoaded( ) const					{ return m_GameLoaded; }
	virtual bool GetLagging( ) const						{ return m_Lagging; }

	virtual void SetEnforceSlots( QList<CGameSlot> nEnforceSlots )		{ m_EnforceSlots = nEnforceSlots; }
	virtual void SetEnforcePlayers( QList<PIDPlayer> nEnforcePlayers )	{ m_EnforcePlayers = nEnforcePlayers; }
	virtual void SetAutoStartPlayers( quint32 nAutoStartPlayers )		{ m_AutoStartPlayers = nAutoStartPlayers; }
	virtual void SetMinimumScore( double nMinimumScore )				{ m_MinimumScore = nMinimumScore; }
	virtual void SetMaximumScore( double nMaximumScore )				{ m_MaximumScore = nMaximumScore; }
	virtual void SetMatchMaking( bool nMatchMaking )					{ m_MatchMaking = nMatchMaking; }

	virtual quint32 GetSlotsOccupied( ) const;
	virtual quint32 GetSlotsOpen( ) const;
	virtual quint32 GetNumPlayers( ) const;
	virtual quint32 GetNumHumanPlayers( ) const;
	virtual QString GetDescription( ) const;
	quint32 GetSyncCounter() { return m_SyncCounter; }
	quint32 GetSyncLimit() { return m_SyncLimit; }

	virtual void SetAnnounce( quint32 interval, const QString &message );

	// generic functions to send packets to players

	virtual void Send( CGamePlayer *player, const QByteArray &data );
	virtual void Send( unsigned char PID, const QByteArray &data );
	virtual void Send( const QByteArray &PIDs, const QByteArray &data );
	virtual void SendAll( const QByteArray &data );

	// functions to send packets to players

	virtual void SendChat( unsigned char fromPID, CGamePlayer *player, const QString &message );
	virtual void SendChat( unsigned char fromPID, unsigned char toPID, const QString &message );
	virtual void SendChat( CGamePlayer *player, const QString &message );
	virtual void SendChat( unsigned char toPID, const QString &message );
	virtual void SendAllChat( unsigned char fromPID, const QString &message );
	virtual void SendAllChat( const QString &message );
	virtual void SendLocalAdminChat( const QString &message );
	virtual void SendVirtualHostPlayerInfo( CGamePlayer *player );
	virtual void SendFakePlayerInfo( CGamePlayer *player );
	virtual void SendAllActions( );
	virtual void SendWelcomeMessage( CGamePlayer *player );
	virtual void SendEndMessage( );

	// events
	// note: these are only called while iterating through the m_Potentials or m_Players vectors
	// therefore you can't modify those vectors and must use the player's m_DeleteMe member to flag for deletion

	virtual void EventPlayerDisconnectTimedOut( CGamePlayer *player );
	virtual void EventPlayerDisconnectPlayerError( CGamePlayer *player );
	virtual void EventPlayerDisconnectSocketError( CGamePlayer *player );
	virtual void EventPlayerDisconnectConnectionClosed( CGamePlayer *player );
	virtual void EventPlayerJoined( CPotentialPlayer *potential, CIncomingJoinPlayer *joinPlayer );
	virtual void EventPlayerJoinedWithScore( CPotentialPlayer *potential, CIncomingJoinPlayer *joinPlayer, double score );
	virtual void EventPlayerLeft( CGamePlayer *player, quint32 reason );
	virtual void EventPlayerAction( CGamePlayer *player, CIncomingAction *action );
	virtual void EventPlayerKeepAlive( CGamePlayer *player, quint32 checkSum );
	virtual void EventPlayerChatToHost( CGamePlayer *player, CIncomingChatPlayer *chatPlayer );
	virtual bool EventPlayerBotCommand( CGamePlayer *player, const QString &command, const QString &payload );
	virtual void EventPlayerChangeTeam( CGamePlayer *player, unsigned char team );
	virtual void EventPlayerChangeColour( CGamePlayer *player, unsigned char colour );
	virtual void EventPlayerChangeRace( CGamePlayer *player, unsigned char race );
	virtual void EventPlayerChangeHandicap( CGamePlayer *player, unsigned char handicap );
	virtual void EventPlayerDropRequest( CGamePlayer *player );
	virtual void EventPlayerMapSize( CGamePlayer *player, CIncomingMapSize *mapSize );
	virtual void EventPlayerPongToHost( CGamePlayer *player, quint32 pong );

	// these events are called outside of any iterations

	virtual void EventGameRefreshed( const QString &server );

	// other functions

	virtual unsigned char GetSIDFromPID( unsigned char PID );
	virtual CGamePlayer *GetPlayerFromPID( unsigned char PID );
	virtual CGamePlayer *GetPlayerFromSID( unsigned char SID );
	virtual CGamePlayer *GetPlayerFromName( const QString &name, bool sensitive );
	virtual quint32 GetPlayerFromNamePartial( const QString &name, CGamePlayer **player );
	virtual CGamePlayer *GetPlayerFromColour( unsigned char colour );
	virtual unsigned char GetNewPID( );
	virtual unsigned char GetNewColour( );
	virtual QByteArray GetPIDs( );
	virtual QByteArray GetPIDs( unsigned char excludePID );
	virtual unsigned char GetHostPID( );
	virtual unsigned char GetEmptySlot( bool reserved );
	virtual unsigned char GetEmptySlot( unsigned char team, unsigned char PID );
	virtual void SwapSlots( unsigned char SID1, unsigned char SID2 );
	virtual void OpenSlot( unsigned char SID, bool kick );
	virtual void CloseSlot( unsigned char SID, bool kick );
	virtual void ComputerSlot( unsigned char SID, unsigned char skill, bool kick );
	virtual void ColourSlot( unsigned char SID, unsigned char colour );
	virtual void OpenAllSlots( );
	virtual void CloseAllSlots( );
	virtual void ShuffleSlots( );
	virtual QList<unsigned char> BalanceSlotsRecursive( QList<unsigned char> PlayerIDs, unsigned char *TeamSizes, double *PlayerScores, unsigned char StartTeam );
	virtual void BalanceSlots( );
	virtual void AddToSpoofed( const QString &server, const QString &name, bool sendMessage );
	virtual void AddToReserved( const QString &name );
	virtual bool IsOwner( const QString &name );
	virtual bool IsReserved( const QString &name );
	virtual bool IsDownloading( );
	virtual bool IsGameDataSaved( );
	virtual void SaveGameData( );
	virtual void StartCountDown( bool force );
	virtual void StartCountDownAuto( bool requireSpoofChecks );
	virtual void StopPlayers( const QString &reason );
	virtual void StopLaggers( const QString &reason );
	virtual void CreateVirtualHost( );
	virtual void DeleteVirtualHost( );
	virtual void CreateFakePlayer( );
	virtual void DeleteFakePlayer( );
};

#endif
