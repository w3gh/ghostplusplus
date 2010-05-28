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
#include "ghostdb.h"
#include "bnet.h"
#include "map.h"
#include "packed.h"
#include "savegame.h"
#include "replay.h"
#include "gameplayer.h"
#include "gameprotocol.h"
#include "game_base.h"

#include <cmath>
#include <QString>
#include <time.h>
#include <QTextStream>
#include <QFile>
#include <QStringList>
#include <QRegExp>
#include <QUdpSocket>
#include <QTcpServer>
#include <QTcpSocket>

#include "next_combination.h"

//
// CBaseGame
//

quint32 CBaseGame::m_GlobalHostCounter = 1;

CBaseGame :: CBaseGame( CGHost *nGHost, CMap *nMap, CSaveGame *nSaveGame, quint16 nHostPort, unsigned char nGameState, QString nGameName, QString nOwnerName, QString nCreatorName, QString nCreatorServer )
{
	m_GHost = nGHost;
	m_Socket = new QTcpServer(this);
	QObject::connect(m_Socket, SIGNAL(newConnection()), this, SLOT(EventNewConnection()));

	m_BroadcastTimer.start(3000);
	QObject::connect(&m_BroadcastTimer, SIGNAL(timeout()), this, SLOT(EventBroadcastTimeout()));

	m_RefreshTimer.start(3000);
	QObject::connect(&m_RefreshTimer, SIGNAL(timeout()), this, SLOT(EventRefreshTimeout()));

	m_MapDataTimer.start(100);
	QObject::connect(&m_MapDataTimer, SIGNAL(timeout()), this, SLOT(EventMapDataTimeout()));

	m_CountdownTimer.setInterval(500);
	QObject::connect(&m_CountdownTimer, SIGNAL(timeout()), this, SLOT(EventCountdownTimeout()));

	m_AutostartTimer.start(10000);
	QObject::connect(&m_AutostartTimer, SIGNAL(timeout()), this, SLOT(EventAutostartTimeout()));

	m_GameOverTimer.setInterval(60000);
	QObject::connect(&m_GameOverTimer, SIGNAL(timeout()), this, SLOT(EventGameOverTimeout()));

	m_VotekickTimer.setInterval(60000);
	QObject::connect(&m_VotekickTimer, SIGNAL(timeout()), this, SLOT(EventVotekickTimeout()));

	m_SlotInfoTimer.setInterval(500);
	m_SlotInfoTimer.setSingleShot(true);
	QObject::connect(&m_SlotInfoTimer, SIGNAL(timeout()), this, SLOT(SendAllSlotInfo()));

	m_CallableUpdateTimer.setInterval(200);
	QObject::connect(&m_CallableUpdateTimer, SIGNAL(timeout()), this, SLOT(EventCallableUpdateTimeout()));

	m_DownloadCounterTimer.start(1000);
	QObject::connect(&m_DownloadCounterTimer, SIGNAL(timeout()), this, SLOT(EventResetDownloadCounter()));

	QObject::connect(&m_AnnounceTimer, SIGNAL(timeout()), this, SLOT(EventAnnounceTimeout()));

	m_DropLaggerTimer.setInterval(60000);
	m_DropLaggerTimer.setSingleShot(true);
	QObject::connect(&m_DropLaggerTimer, SIGNAL(timeout()), this, SLOT(EventDropLaggerTimeout()));

	m_ResetLagscreenTimer.setInterval(60000);
	QObject::connect(&m_ResetLagscreenTimer, SIGNAL(timeout()), this, SLOT(EventResetLagscreenTimeout()));

	m_LoadInGameTimer.setInterval(30000);
	QObject::connect(&m_LoadInGameTimer, SIGNAL(timeout()), this, SLOT(EventLoadInGameTimeout()));

	m_LobbyTimeoutTimer.setInterval(m_GHost->m_LobbyTimeLimit * 60000);
	m_LobbyTimeoutTimer.setSingleShot(true);
	QObject::connect(&m_LobbyTimeoutTimer, SIGNAL(timeout()), this, SLOT(EventLobbyTimeout()));

	m_Latency = m_GHost->m_Latency;
	m_SendActionTimer.setInterval(m_Latency);
	QObject::connect(&m_SendActionTimer, SIGNAL(timeout()), this, SLOT(EventSendActions()));

	QObject::connect(this, SIGNAL(finishedLoading()), &m_SendActionTimer, SLOT(start()));
	QObject::connect(this, SIGNAL(startedLoading()), this, SLOT(EventGameStarted()));
	QObject::connect(this, SIGNAL(finishedLoading()), this, SLOT(EventGameLoaded()));



	m_RequestedLatency = 0;
	m_Protocol = new CGameProtocol( m_GHost );
	m_Map = new CMap( *nMap );
	m_SaveGame = nSaveGame;

	if( m_GHost->m_SaveReplays && !m_SaveGame )
		m_Replay = new CReplay( );
	else
		m_Replay = NULL;

	m_Saving = false;
	m_HostPort = nHostPort;
	m_GameState = nGameState;
	m_VirtualHostPID = 255;
	m_FakePlayerPID = 255;

	// wait time of 1 minute  = 0 empty actions required
	// wait time of 2 minutes = 1 empty action required
	// etc...

	if( m_GHost->m_ReconnectWaitTime == 0 )
		m_GProxyEmptyActions = 0;
	else
	{
		m_GProxyEmptyActions = m_GHost->m_ReconnectWaitTime - 1;

		// clamp to 9 empty actions (10 minutes)

		if( m_GProxyEmptyActions > 9 )
			m_GProxyEmptyActions = 9;
	}

	m_GameName = nGameName;
	m_LastGameName = nGameName;
	m_VirtualHostName = m_GHost->m_VirtualHostName;
	m_OwnerName = nOwnerName;
	m_CreatorName = nCreatorName;
	m_CreatorServer = nCreatorServer;
	m_HCLCommandString = m_Map->GetMapDefaultHCL( );
	m_RandomSeed = GetTicks( );
	m_HostCounter = GetNewHostCounter( );
	m_SyncLimit = m_GHost->m_SyncLimit;
	m_SyncCounter = 0;
	m_GameTicks = 0;
	m_CreationTime = GetTime( );
	m_LastDownloadTicks = GetTime( );
	m_DownloadCounter = 0;
	m_AutoStartPlayers = 0;
	m_CountDownCounter = 0;
	m_StartedLoadingTicks = 0;
	m_StartPlayers = 0;
	m_LastActionSentTicks = 0;
	m_StartedLaggingTime = 0;
	m_LastPlayerLeaveTicks = 0;
	m_MinimumScore = 0.0;
	m_MaximumScore = 0.0;
	m_Locked = false;
	m_RefreshMessages = m_GHost->m_RefreshMessages;
	m_RefreshError = false;
	m_RefreshRehosted = false;
	m_MuteAll = false;
	m_MuteLobby = false;
	m_CountDownStarted = false;
	m_GameLoading = false;
	m_GameLoaded = false;
	m_LoadInGame = m_Map->GetMapLoadInGame( );
	m_Lagging = false;
	m_AutoSave = m_GHost->m_AutoSave;
	m_MatchMaking = false;
	m_LocalAdminMessages = m_GHost->m_LocalAdminMessages;

	if( m_SaveGame )
	{
		m_EnforceSlots = m_SaveGame->GetSlots( );
		m_Slots = m_SaveGame->GetSlots( );

		// the savegame slots contain player entries
		// we really just want the open/closed/computer entries
		// so open all the player slots

		for( QList<CGameSlot> :: iterator i = m_Slots.begin( ); i != m_Slots.end( ); i++ )
		{
			if( (*i).GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && (*i).GetComputer( ) == 0 )
			{
				(*i).SetPID( 0 );
				(*i).SetDownloadStatus( 255 );
				(*i).SetSlotStatus( SLOTSTATUS_OPEN );
			}
		}
	}
	else
		m_Slots = m_Map->GetSlots( );

	if( !m_GHost->m_IPBlackListFile.isEmpty( ) )
	{
		QFile f(m_GHost->m_IPBlackListFile);

		if( !f.open(QFile::ReadOnly) )
			CONSOLE_Print( "[GAME: " + m_GameName + "] error loading IP blacklist file [" + m_GHost->m_IPBlackListFile + "]" );
		else
		{
			QTextStream in(&f);
			CONSOLE_Print( "[GAME: " + m_GameName + "] loading IP blacklist file [" + m_GHost->m_IPBlackListFile + "]" );
			QString Line;

			while( !in.atEnd( ) )
			{
				Line = in.readLine();

				// ignore blank lines and comments

				if( Line.isEmpty( ) || Line[0] == '#' )
					continue;

				// remove newlines and partial newlines to help fix issues with Windows formatted files on Linux systems

				Line.replace(QRegExp("[ \r\n]"), "");

				// ignore lines that don't look like IP addresses

				if( Line.indexOf( QRegExp("[^0-9\\.]") ) != -1 )
					continue;

				m_IPBlackList.insert( Line );
			}

			f.close( );

			CONSOLE_Print( "[GAME: " + m_GameName + "] loaded " + QString::number( m_IPBlackList.size( ) ) + " lines from IP blacklist file" );
		}
	}

	CreateVirtualHost( );

	// start listening for connections

	if( !m_GHost->m_BindAddress.isEmpty( ) )
		CONSOLE_Print( "[GAME: " + m_GameName + "] attempting to bind to address [" + m_GHost->m_BindAddress + "]" );
	else
		CONSOLE_Print( "[GAME: " + m_GameName + "] attempting to bind to all available addresses" );

	QHostAddress hostAddr = m_GHost->m_BindAddress == "" ? QHostAddress::Any : QHostAddress(m_GHost->m_BindAddress);
	if( m_Socket->listen( hostAddr, m_HostPort ) )
		CONSOLE_Print( "[GAME: " + m_GameName + "] listening on " + hostAddr.toString() + ":" + QString::number( m_Socket->serverPort() ) );
	else
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] error listening on " + hostAddr.toString() + ":" + QString::number( m_HostPort ) );
		deleteLater();
	}
}

CBaseGame :: ~CBaseGame( )
{
	for( QList<CBNET *> :: const_iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
	{
		(*i)->QueueChatCommand( m_GHost->GetLanguage( )->GameIsOver( GetDescription( ) ) );

		if( (*i)->GetServer( ) == GetCreatorServer( ) )
			(*i)->QueueChatCommand( m_GHost->GetLanguage( )->GameIsOver( GetDescription( ) ), GetCreatorName( ), true );
	}

	// save replay
	// todotodo: put this in a thread

	if( m_Replay && ( m_GameLoading || m_GameLoaded ) )
	{
		time_t Now = time( NULL );
		char Time[17];
		memset( Time, 0, sizeof( char ) * 17 );
		strftime( Time, sizeof( char ) * 17, "%Y-%m-%d %H-%M", localtime( &Now ) );
		QString MinString = QString::number( ( m_GameTicks / 1000 ) / 60 );
		QString SecString = QString::number( ( m_GameTicks / 1000 ) % 60 );

		if( MinString.size( ) == 1 )
			MinString.insert( 0, "0" );

		if( SecString.size( ) == 1 )
			SecString.insert( 0, "0" );

		m_Replay->BuildReplay( m_GameName, m_StatString, m_GHost->m_ReplayWar3Version, m_GHost->m_ReplayBuildNumber );
		m_Replay->Save( m_GHost->m_TFT, m_GHost->m_ReplayPath + UTIL_FileSafeName( "GHost++ " + QString( Time ) + " " + m_GameName + " (" + MinString + "m" + SecString + "s).w3g" ) );
	}

	delete m_Socket;
	delete m_Protocol;
	delete m_Map;
	delete m_Replay;

	for( QList<CPotentialPlayer *> :: const_iterator i = m_Potentials.begin( ); i != m_Potentials.end( ); i++ )
		delete *i;

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		delete *i;

	for( QList<CCallableScoreCheck *> :: const_iterator i = m_ScoreChecks.begin( ); i != m_ScoreChecks.end( ); i++ )
		m_GHost->m_Callables.push_back( *i );

	while( !m_Actions.isEmpty( ) )
	{
		delete m_Actions.front( );
		m_Actions.dequeue( );
	}
}

quint32 CBaseGame :: GetSlotsOccupied( ) const
{
	quint32 NumSlotsOccupied = 0;

	for( QList<CGameSlot> :: const_iterator i = m_Slots.begin( ); i != m_Slots.end( ); i++ )
	{
		if( (*i).GetSlotStatus( ) == SLOTSTATUS_OCCUPIED )
			NumSlotsOccupied++;
	}

	return NumSlotsOccupied;
}

quint32 CBaseGame :: GetSlotsOpen( ) const
{
	quint32 NumSlotsOpen = 0;

	for( QList<CGameSlot> :: const_iterator i = m_Slots.begin( ); i != m_Slots.end( ); i++ )
	{
		if( (*i).GetSlotStatus( ) == SLOTSTATUS_OPEN )
			NumSlotsOpen++;
	}

	return NumSlotsOpen;
}

quint32 CBaseGame :: GetNumPlayers( ) const
{
	quint32 NumPlayers = GetNumHumanPlayers( );

	if( m_FakePlayerPID != 255 )
		NumPlayers++;

	return NumPlayers;
}

quint32 CBaseGame :: GetNumHumanPlayers( ) const
{
	quint32 NumHumanPlayers = 0;

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( !(*i)->GetLeftMessageSent( ) )
			NumHumanPlayers++;
	}

	return NumHumanPlayers;
}

QString CBaseGame :: GetDescription( ) const
{
	QString Description = m_GameName + " : " + m_OwnerName + " : " + QString::number( GetNumHumanPlayers( ) ) + "/" + QString::number( m_GameLoading || m_GameLoaded ? m_StartPlayers : m_Slots.size( ) );

	if( m_GameLoading || m_GameLoaded )
		Description += " : " + QString::number( ( m_GameTicks / 1000 ) / 60 ) + "m";
	else
		Description += " : " + QString::number( ( GetTime( ) - m_CreationTime ) / 60 ) + "m";

	return Description;
}

void CBaseGame :: SetAnnounce( quint32 interval, const QString &message )
{
	m_AnnounceTimer.start(1000 * interval);
	m_AnnounceMessage = message;
}

void CBaseGame::CheckGameLoaded()
{
	bool FinishedLoading = true;

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		FinishedLoading = (*i)->GetFinishedLoading( );

		if( !FinishedLoading )
			break;
	}

	if( FinishedLoading )
	{
		m_LastActionSentTicks = GetTicks( );
		m_GameLoading = false;
		m_GameLoaded = true;
		emit finishedLoading();
		return;
	}
}

void CBaseGame::EventLoadInGameTimeout()
{
	// reset the "lag" screen (the load-in-game screen) every 30 seconds

	bool UsingGProxy = false;

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( (*i)->GetGProxy( ) )
			UsingGProxy = true;
	}

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( (*i)->GetFinishedLoading( ) )
		{
			// stop the lag screen

			for( QList<CGamePlayer *> :: iterator j = m_Players.begin( ); j != m_Players.end( ); j++ )
			{
				if( !(*j)->GetFinishedLoading( ) )
					Send( *i, m_Protocol->SEND_W3GS_STOP_LAG( *j, true ) );
			}

			// send an empty update
			// this resets the lag screen timer but creates a rather annoying problem
			// in order to prevent a desync we must make sure every player receives the exact same "desyncable game data" (updates and player leaves) in the exact same order
			// unfortunately we cannot send updates to players who are still loading the map, so we buffer the updates to those players (see the else clause a few lines down for the code)
			// in addition to this we must ensure any player leave messages are sent in the exact same position relative to these updates so those must be buffered too

			if( UsingGProxy && !(*i)->GetGProxy( ) )
			{
				// we must send empty actions to non-GProxy++ players
				// GProxy++ will insert these itself so we don't need to send them to GProxy++ players
				// empty actions are used to extend the time a player can use when reconnecting

				for( unsigned char j = 0; j < m_GProxyEmptyActions; j++ )
					Send( *i, m_Protocol->SEND_W3GS_INCOMING_ACTION( QQueue<CIncomingAction *>( ), 0 ) );
			}

			Send( *i, m_Protocol->SEND_W3GS_INCOMING_ACTION( QQueue<CIncomingAction *>( ), 0 ) );

			// start the lag screen

			Send( *i, m_Protocol->SEND_W3GS_START_LAG( m_Players, true ) );
		}
		else
		{
			// buffer the empty update since the player is still loading the map

			if( UsingGProxy && !(*i)->GetGProxy( ) )
			{
				// we must send empty actions to non-GProxy++ players
				// GProxy++ will insert these itself so we don't need to send them to GProxy++ players
				// empty actions are used to extend the time a player can use when reconnecting

				for( unsigned char j = 0; j < m_GProxyEmptyActions; j++ )
					(*i)->AddLoadInGameData( m_Protocol->SEND_W3GS_INCOMING_ACTION( QQueue<CIncomingAction *>( ), 0 ) );
			}

			(*i)->AddLoadInGameData( m_Protocol->SEND_W3GS_INCOMING_ACTION( QQueue<CIncomingAction *>( ), 0 ) );
		}
	}

	// add actions to replay

	if( m_Replay )
	{
		if( UsingGProxy )
		{
			for( unsigned char i = 0; i < m_GProxyEmptyActions; i++ )
				m_Replay->AddTimeSlot( 0, QQueue<CIncomingAction *>( ) );
		}

		m_Replay->AddTimeSlot( 0, QQueue<CIncomingAction *>( ) );
	}

	// Warcraft III doesn't seem to respond to empty actions

	/* if( UsingGProxy )
		m_SyncCounter += m_GProxyEmptyActions;

	m_SyncCounter++; */
}

void CBaseGame::EventSendActions()
{
	if (m_Lagging)
	{
		m_SendActionTimer.stop();
		return;
	}

	// send actions every m_Latency milliseconds
	// actions are at the heart of every Warcraft 3 game but luckily we don't need to know their contents to relay them
	// we queue player actions in EventPlayerAction then just resend them in batches to all players here

	SendAllActions( );
}

void CBaseGame::EventBroadcastTimeout()
{
	// we broadcast the game to the local network every 3 seconds so we hijack this timer for our nefarious purposes
	// however we only want to broadcast if the countdown hasn't started
	// see the !sendlan code later in this file for some more information about how this works
	// todotodo: should we send a game cancel message somewhere? we'll need to implement a host counter for it to work

	if (m_GameLoading || m_GameLoaded)
	{
		m_BroadcastTimer.stop();
		return;
	}

	// construct a fixed host counter which will be used to identify players from this "realm" (i.e. LAN)
	// the fixed host counter's 4 most significant bits will contain a 4 bit ID (0-15)
	// the rest of the fixed host counter will contain the 28 least significant bits of the actual host counter
	// since we're destroying 4 bits of information here the actual host counter should not be greater than 2^28 which is a reasonable assumption
	// when a player joins a game we can obtain the ID from the received host counter
	// note: LAN broadcasts use an ID of 0, battle.net refreshes use an ID of 1-10, the rest are unused

	quint32 FixedHostCounter = m_HostCounter & 0x0FFFFFFF;

	if( m_SaveGame )
	{
		// note: the PrivateGame flag is not set when broadcasting to LAN (as you might expect)

		quint32 MapGameType = MAPGAMETYPE_SAVEDGAME;
		QByteArray MapWidth;
		MapWidth.push_back( (char)0 );
		MapWidth.push_back( (char)0 );
		QByteArray MapHeight;
		MapHeight.push_back( (char)0 );
		MapHeight.push_back( (char)0 );
		m_GHost->SendUdpBroadcast(
			m_Protocol->SEND_W3GS_GAMEINFO(
						m_GHost->m_TFT,
						m_GHost->m_LANWar3Version,
						Util::fromUInt32( MapGameType),
						m_Map->GetMapGameFlags( ),
						MapWidth,
						MapHeight,
						m_GameName,
						"Varlock",
						GetTime( ) - m_CreationTime,
						"Save\\Multiplayer\\" + m_SaveGame->GetFileNameNoPath( ),
						m_SaveGame->GetMagicNumber( ),
						12,
						12,
						m_HostPort,
						FixedHostCounter ),
			6112);
	}
	else
	{
		// note: the PrivateGame flag is not set when broadcasting to LAN (as you might expect)
		// note: we do not use m_Map->GetMapGameType because none of the filters are set when broadcasting to LAN (also as you might expect)

		quint32 MapGameType = MAPGAMETYPE_UNKNOWN0;
		m_GHost->SendUdpBroadcast(
				m_Protocol->SEND_W3GS_GAMEINFO(
						m_GHost->m_TFT,
						m_GHost->m_LANWar3Version,
						Util::fromUInt32( MapGameType),
						m_Map->GetMapGameFlags( ),
						m_Map->GetMapWidth( ),
						m_Map->GetMapHeight( ),
						m_GameName,
						"Varlock",
						GetTime( ) - m_CreationTime,
						m_Map->GetMapPath( ),
						m_Map->GetMapCRC( ),
						12,
						12,
						m_HostPort,
						FixedHostCounter ),
				 6112 );
	}
}

void CBaseGame::EventTryAutoRehost()
{
	// there's a slim chance that this isn't actually an autohosted game since there is no explicit autohost flag
	// however, if autohosting is enabled and this game is public and this game is set to autostart, it's probably autohosted
	// so rehost it using the current autohost game name

	m_HostCounter = GetNewHostCounter( );
	QString GameName = m_GHost->m_AutoHostGameName + " #" + QString::number( m_HostCounter );
	CONSOLE_Print( "[GAME: " + m_GameName + "] automatically trying to rehost as public game [" + GameName + "] due to refresh failure" );
	m_LastGameName = m_GameName;
	m_GameName = GameName;

	m_RefreshError = false;

	for( QList<CBNET *> :: const_iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
	{
		(*i)->QueueGameUncreate( );
		(*i)->QueueEnterChat( );

		// the game creation message will be sent on the next refresh
	}

	m_CreationTime = GetTime( );
}

void CBaseGame::EventRefreshError()
{
	if (m_GameLoading || m_GameLoaded)
	{
		m_RefreshTimer.stop();
		return;
	}

	if (m_CountDownStarted)
		return;
	DEBUG_Print("EventRefreshError");

	// auto rehost if there was a refresh error in autohosted games
	m_RefreshError = true;

	if(m_GameState == GAME_PUBLIC &&
	   !m_GHost->m_AutoHostGameName.isEmpty( ) &&
	   m_GHost->m_AutoHostMaximumGames != 0 &&
	   m_GHost->m_AutoHostAutoStartPlayers != 0 &&
	   m_AutoStartPlayers != 0 )
		QTimer::singleShot(5000, this, SLOT(EventTryAutoRehost()));
}

void CBaseGame::EventRefreshTimeout()
{
	if (m_GameLoading || m_GameLoaded)
	{
		m_RefreshTimer.stop();
		return;
	}

	// refresh every 3 seconds
	if( !m_RefreshError && m_GameState == GAME_PUBLIC && GetSlotsOpen( ) > 0)
	{
		// send a game refresh packet to each battle.net connection

		bool Refreshed = false;

		for( QList<CBNET *> :: const_iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
		{
			// don't queue a game refresh message if the queue contains more than 1 packet because they're very low priority

			if( (*i)->GetOutPacketsQueued( ) <= 1 )
			{
				(*i)->QueueGameRefresh( m_GameState, m_GameName, QString( ), m_Map, m_SaveGame, GetTime( ) - m_CreationTime, m_HostCounter );
				Refreshed = true;
			}
		}

		// only print the "game refreshed" message if we actually refreshed on at least one battle.net server

		if( m_RefreshMessages && Refreshed )
			SendAllChat( m_GHost->GetLanguage( )->GameRefreshed( ) );
	}
}

void CBaseGame::EventResetDownloadCounter()
{
	// 1 Hz
	if (m_GameLoading || m_GameLoaded )
	{
		m_DownloadCounterTimer.stop();
		return;
	}

	m_DownloadCounter = 0;
}

void CBaseGame::EventAnnounceTimeout()
{
	// announce every m_AnnounceInterval seconds

	if( m_AnnounceMessage.isEmpty( ) )
	{
		m_AnnounceTimer.stop();
		return;
	}

	if (m_CountDownStarted)
		return;

	SendAllChat( m_AnnounceMessage );
}

void CBaseGame::EventMapDataTimeout()
{
	if (m_GameLoading || m_GameLoaded )
	{
		m_MapDataTimer.stop();
		return;
	}

	quint32 Downloaders = 0;

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( (*i)->GetDownloadStarted( ) && !(*i)->GetDownloadFinished( ) )
		{
			Downloaders++;

			if( m_GHost->m_MaxDownloaders > 0 && Downloaders > m_GHost->m_MaxDownloaders )
				break;

			// send up to 100 pieces of the map at once so that the download goes faster
			// if we wait for each MAPPART packet to be acknowledged by the client it'll take a long time to download
			// this is because we would have to wait the round trip time (the ping time) between sending every 1442 bytes of map data
			// doing it this way allows us to send at least 140 KB in each round trip interval which is much more reasonable
			// the theoretical throughput is [140 KB * 1000 / ping] in KB/sec so someone with 100 ping (round trip ping, not LC ping) could download at 1400 KB/sec
			// note: this creates a queue of map data which clogs up the connection when the client is on a slower connection (e.g. dialup)
			// in this case any changes to the lobby are delayed by the amount of time it takes to send the queued data (i.e. 140 KB, which could be 30 seconds or more)
			// for example, players joining and leaving, slot changes, chat messages would all appear to happen much later for the low bandwidth player
			// note: the throughput is also limited by the number of times this code is executed each second
			// e.g. if we send the maximum amount (140 KB) 10 times per second the theoretical throughput is 1400 KB/sec
			// therefore the maximum throughput is 1400 KB/sec regardless of ping and this value slowly diminishes as the player's ping increases
			// in addition to this, the throughput is limited by the configuration value bot_maxdownloadspeed
			// in summary: the actual throughput is MIN( 140 * 1000 / ping, 1400, bot_maxdownloadspeed ) in KB/sec assuming only one player is downloading the map

			quint32 MapSize = Util::extractUInt32(m_Map->GetMapSize( ));

			while( (*i)->GetLastMapPartSent( ) < (*i)->GetLastMapPartAcked( ) + 1442 * 100 && (*i)->GetLastMapPartSent( ) < MapSize )
			{
				if( (*i)->GetLastMapPartSent( ) == 0 )
				{
					// overwrite the "started download ticks" since this is the first time we've sent any map data to the player
					// prior to this we've only determined if the player needs to download the map but it's possible we could have delayed sending any data due to download limits

					(*i)->SetStartedDownloadingTicks( GetTicks( ) );
				}

				// limit the download speed if we're sending too much data
				// the download counter is the # of map bytes downloaded in the last second (it's reset once per second)

				if( m_GHost->m_MaxDownloadSpeed > 0 && m_DownloadCounter > m_GHost->m_MaxDownloadSpeed * 1024 )
					break;

				Send( *i, m_Protocol->SEND_W3GS_MAPPART( GetHostPID( ), (*i)->GetPID( ), (*i)->GetLastMapPartSent( ), m_Map->GetMapData( ) ) );
				(*i)->SetLastMapPartSent( (*i)->GetLastMapPartSent( ) + 1442 );
				m_DownloadCounter += 1442;
			}
		}
	}
}

void CBaseGame::EventCountdownTimeout()
{
	if( m_CountDownCounter <= 0 )
	{
		emit startedLoading();
		m_CountdownTimer.stop();
		return;
	}

	SendAllChat( QString::number( m_CountDownCounter ) + ". . ." );
	m_CountDownCounter--;
}

void CBaseGame::EventAutostartTimeout()
{
	if (m_GameLoading || m_GameLoaded)
	{
		m_AutostartTimer.stop();
		return;
	}

	// try to auto start every 10 seconds

	if( m_CountDownStarted || m_AutoStartPlayers == 0 )
		return;

	StartCountDownAuto( m_GHost->m_RequireSpoofChecks );
}

void CBaseGame::EventGameOverTimeout()
{
	if( m_Players.size() == 0 )
		return;

	CONSOLE_Print( "[GAME: " + m_GameName + "] is over (gameover timer finished)" );
	StopPlayers( "was disconnected (gameover timer finished)" );
	deleteLater();
}

void CBaseGame::EventVotekickTimeout()
{
	// expire the votekick

	if( m_KickVotePlayer.isEmpty( )  )
		return;

	CONSOLE_Print( "[GAME: " + m_GameName + "] votekick against player [" + m_KickVotePlayer + "] expired" );
	SendAllChat( m_GHost->GetLanguage( )->VoteKickExpired( m_KickVotePlayer ) );
	m_KickVotePlayer.clear( );
}

void CBaseGame::EventLobbyTimeout()
{
	if( m_GameLoading || m_GameLoaded || m_AutoStartPlayers != 0)
		return;

	// check if we've hit the time limit

	CONSOLE_Print( "[GAME: " + m_GameName + "] is over (lobby time limit hit)" );
	deleteLater();
}

void CBaseGame::EventNewConnection()
{
	QTcpSocket *NewSocket = m_Socket->nextPendingConnection();

	if( !NewSocket )
		return;

	DEBUG_Print("New connection incomming");

	// check the IP blacklist
	if( m_IPBlackList.find( NewSocket->localAddress().toString() ) != m_IPBlackList.end( ) )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] rejected connection from [" + NewSocket->localAddress().toString() + "] due to blacklist" );
		NewSocket->deleteLater();
		return;
	}

	if( m_GHost->m_TCPNoDelay )
		NewSocket->setSocketOption(QAbstractSocket::LowDelayOption, true);

	new CPotentialPlayer( m_Protocol, this, NewSocket );
}

void CBaseGame :: Send( CGamePlayer *player, const QByteArray &data )
{
	if( player )
		player->Send( data );
}

void CBaseGame :: Send( unsigned char PID, const QByteArray &data )
{
	Send( GetPlayerFromPID( PID ), data );
}

void CBaseGame :: Send( const QByteArray &PIDs, const QByteArray &data )
{
	for( int i = 0; i < PIDs.size( ); i++ )
		Send( PIDs[i], data );
}

void CBaseGame :: SendAll( const QByteArray &data )
{
	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		(*i)->Send( data );
}

void CBaseGame :: SendChat( unsigned char fromPID, CGamePlayer *player, const QString &message )
{
	// send a private message to one player - it'll be marked [Private] in Warcraft 3

	if( player )
	{
		if( !m_GameLoading && !m_GameLoaded )
		{
			// limit text length to 254
			Send( player, m_Protocol->SEND_W3GS_CHAT_FROM_HOST( fromPID, QByteArray(1, player->GetPID( ) ), 16, QByteArray( ), message.mid( 0, 254 ) ) );
		}
		else
		{
			unsigned char ExtraFlags[] = { 3, 0, 0, 0 };

			// based on my limited testing it seems that the extra flags' first byte contains 3 plus the recipient's colour to denote a private message

			unsigned char SID = GetSIDFromPID( player->GetPID( ) );

			if( SID < m_Slots.size( ) )
				ExtraFlags[0] = 3 + m_Slots[SID].GetColour( );
			
			// limit text length to 127
			Send( player, m_Protocol->SEND_W3GS_CHAT_FROM_HOST( fromPID, QByteArray( 1, player->GetPID( ) ), 32, QByteArray( (char*)ExtraFlags, 4 ), message.mid( 0, 127 ) ) );
		}
	}
}

void CBaseGame :: SendChat( unsigned char fromPID, unsigned char toPID, const QString &message )
{
	SendChat( fromPID, GetPlayerFromPID( toPID ), message );
}

void CBaseGame :: SendChat( CGamePlayer *player, const QString &message )
{
	SendChat( GetHostPID( ), player, message );
}

void CBaseGame :: SendChat( unsigned char toPID, const QString &message )
{
	SendChat( GetHostPID( ), toPID, message );
}

void CBaseGame :: SendAllChat( unsigned char fromPID, const QString &message )
{
	// send a public message to all players - it'll be marked [All] in Warcraft 3

	if( GetNumHumanPlayers( ) > 0 )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] [Local]: " + message );

		if( !m_GameLoading && !m_GameLoaded )
		{
			// limit text length to 254
			SendAll( m_Protocol->SEND_W3GS_CHAT_FROM_HOST( fromPID, GetPIDs( ), 16, QByteArray( ), message.mid( 0, 254 ) ) );
		}
		else
		{
			// limit text length to 127
			SendAll( m_Protocol->SEND_W3GS_CHAT_FROM_HOST( fromPID, GetPIDs( ), 32, Util::fromUInt32( 0), message.mid( 0, 127 ) ) );

			if( m_Replay )
				m_Replay->AddChatMessage( fromPID, 32, 0, message );
		}
	}
}

void CBaseGame :: SendAllChat( const QString &message )
{
	SendAllChat( GetHostPID( ), message );
}

void CBaseGame :: SendLocalAdminChat( const QString &message )
{
	if( !m_LocalAdminMessages )
		return;

	// send a message to LAN/local players who are admins
	// at the time of this writing it is only possible for the game owner to meet this criteria because being an admin requires spoof checking
	// this is mainly used for relaying battle.net whispers, chat messages, and emotes to these players

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( (*i)->GetSpoofed( ) && IsOwner( (*i)->GetName( ) ) && ( UTIL_IsLanIP( (*i)->GetExternalIP( ) ) || UTIL_IsLocalIP( (*i)->GetExternalIP( ), m_GHost->m_LocalAddresses ) ) )
		{
			if( m_VirtualHostPID != 255 )
				SendChat( m_VirtualHostPID, *i, message );
			else
			{
				// make the chat message originate from the recipient since it's not going to be logged to the replay

				SendChat( (*i)->GetPID( ), *i, message );
			}
		}
	}
}

void CBaseGame :: SendAllSlotInfo( )
{
	if( !m_GameLoading && !m_GameLoaded )
		SendAll( m_Protocol->SEND_W3GS_SLOTINFO( m_Slots, m_RandomSeed, m_Map->GetMapLayoutStyle( ), m_Map->GetMapNumPlayers( ) ) );
}

void CBaseGame :: SendVirtualHostPlayerInfo( CGamePlayer *player )
{
	if( m_VirtualHostPID == 255 )
		return;

	QByteArray IP;
	IP.push_back( (char)0 );
	IP.push_back( (char)0 );
	IP.push_back( (char)0 );
	IP.push_back( (char)0 );
	Send( player, m_Protocol->SEND_W3GS_PLAYERINFO( m_VirtualHostPID, m_VirtualHostName, IP, IP ) );
}

void CBaseGame :: SendFakePlayerInfo( CGamePlayer *player )
{
	if( m_FakePlayerPID == 255 )
		return;

	QByteArray IP;
	IP.push_back( (char)0 );
	IP.push_back( (char)0 );
	IP.push_back( (char)0 );
	IP.push_back( (char)0 );
	Send( player, m_Protocol->SEND_W3GS_PLAYERINFO( m_FakePlayerPID, "FakePlayer", IP, IP ) );
}

void CBaseGame :: SendAllActions( )
{
	bool UsingGProxy = false;

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( (*i)->GetGProxy( ) )
			UsingGProxy = true;
	}

	if (m_RequestedLatency != 0)
	{
		m_Latency = m_RequestedLatency;
		m_SendActionTimer.setInterval(m_RequestedLatency);
		m_SendActionTimer.start();
		m_RequestedLatency = 0;
	}

	m_GameTicks += m_Latency;

	if( UsingGProxy )
	{
		// we must send empty actions to non-GProxy++ players
		// GProxy++ will insert these itself so we don't need to send them to GProxy++ players
		// empty actions are used to extend the time a player can use when reconnecting

		for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			if( !(*i)->GetGProxy( ) )
			{
				for( unsigned char j = 0; j < m_GProxyEmptyActions; j++ )
					Send( *i, m_Protocol->SEND_W3GS_INCOMING_ACTION( QQueue<CIncomingAction *>( ), 0 ) );
			}
		}

		if( m_Replay )
		{
			for( unsigned char i = 0; i < m_GProxyEmptyActions; i++ )
				m_Replay->AddTimeSlot( 0, QQueue<CIncomingAction *>( ) );
		}
	}

	// Warcraft III doesn't seem to respond to empty actions

	/* if( UsingGProxy )
		m_SyncCounter += m_GProxyEmptyActions; */

	m_SyncCounter++;
	CheckPlayersStartedLaggging();

	// we aren't allowed to send more than 1460 bytes in a single packet but it's possible we might have more than that many bytes waiting in the queue

	if( !m_Actions.isEmpty( ) )
	{
		// we use a "sub actions queue" which we keep adding actions to until we reach the size limit
		// start by adding one action to the sub actions queue

		QQueue<CIncomingAction *> SubActions;
		CIncomingAction *Action = m_Actions.front( );
		m_Actions.dequeue( );
		SubActions.enqueue( Action );
		quint32 SubActionsLength = Action->GetLength( );

		while( !m_Actions.isEmpty( ) )
		{
			Action = m_Actions.front( );
			m_Actions.dequeue( );

			// check if adding the next action to the sub actions queue would put us over the limit (1452 because the INCOMING_ACTION and INCOMING_ACTION2 packets use an extra 8 bytes)

			if( SubActionsLength + Action->GetLength( ) > 1452 )
			{
				// we'd be over the limit if we added the next action to the sub actions queue
				// so send everything already in the queue and then clear it out
				// the W3GS_INCOMING_ACTION2 packet handles the overflow but it must be sent *before* the corresponding W3GS_INCOMING_ACTION packet

				SendAll( m_Protocol->SEND_W3GS_INCOMING_ACTION2( SubActions ) );

				if( m_Replay )
					m_Replay->AddTimeSlot2( SubActions );

				while( !SubActions.isEmpty( ) )
				{
					delete SubActions.front( );
					SubActions.dequeue( );
				}

				SubActionsLength = 0;
			}

			SubActions.enqueue( Action );
			SubActionsLength += Action->GetLength( );
		}

		SendAll( m_Protocol->SEND_W3GS_INCOMING_ACTION( SubActions, m_Latency ) );

		if( m_Replay )
			m_Replay->AddTimeSlot( m_Latency, SubActions );

		while( !SubActions.isEmpty( ) )
		{
			delete SubActions.front( );
			SubActions.dequeue( );
		}
	}
	else
	{
		SendAll( m_Protocol->SEND_W3GS_INCOMING_ACTION( m_Actions, m_Latency ) );

		if( m_Replay )
			m_Replay->AddTimeSlot( m_Latency, m_Actions );
	}

	if( GetTicks() - m_LastActionSentTicks > 1.5 * m_Latency )
	{
		// something is going terribly wrong - GHost++ is probably starved of resources
		// print a message because even though this will take more resources it should provide some information to the administrator for future reference
		// other solutions - dynamically modify the latency, request higher priority, terminate other games, ???

		CONSOLE_Print( "[GAME: " + m_GameName + "] warning - the latency is " + QString::number( m_Latency ) + "ms but GHost needed " + QString::number(GetTicks() - m_LastActionSentTicks)  + "ms	, your machine is probably overloaded" );
	}

	m_LastActionSentTicks = GetTicks( );
}

void CBaseGame :: SendWelcomeMessage( CGamePlayer *player )
{
	// read from motd.txt if available (thanks to zeeg for this addition)

	QFile f(m_GHost->m_MOTDFile);

	if( !f.open(QFile::ReadOnly) )
	{
		// default welcome message

		if( m_HCLCommandString.isEmpty( ) )
			SendChat( player, " " );

		SendChat( player, " " );
		SendChat( player, " " );
		SendChat( player, " " );
		SendChat( player, "GHost++                                         http://www.codelain.com/" );
		SendChat( player, "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-" );
		SendChat( player, "     Game Name:                 " + m_GameName );

		if( !m_HCLCommandString.isEmpty( ) )
			SendChat( player, "     HCL Command String:  " + m_HCLCommandString );
	}
	else
	{
		// custom welcome message
		// don't print more than 8 lines
		QStringList data = QString::fromUtf8(f.readAll()).replace('\r', "").split('\n');

		for (int i = 0; i < data.size() && i < 8; i++)
			SendChat( player, data.at(i) );
	}
}

void CBaseGame :: SendEndMessage( )
{
	// read from gameover.txt if available

	QFile f(m_GHost->m_GameOverFile);

	if( f.open(QFile::ReadOnly) )
	{
		// don't print more than 8 lines
		QStringList data = QString::fromUtf8(f.readAll()).replace('\r', "").split('\n');

		for (int i = 0; i < data.size() && i < 8; i++)
			SendAllChat( data.at(i) );
	}
}

void CBaseGame::CheckPlayersStartedLaggging()
{
	// check if anyone has started lagging
	// we consider a player to have started lagging if they're more than m_SyncLimit keepalives behind

	if( m_Lagging )
		return;


	bool UsingGProxy = false;

	m_WaitTime = 60;

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( (*i)->GetGProxy( ) )
		{
			if( UsingGProxy )
				m_WaitTime = ( m_GProxyEmptyActions + 1 ) * 60;

			break;
		}
	}

	QString LaggingString;
	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( m_SyncCounter - (*i)->GetSyncCounter( ) > m_SyncLimit )
		{
			(*i)->SetLagging( true );
			(*i)->SetStartedLaggingTicks( GetTicks( ) );
			m_Lagging = true;
			m_StartedLaggingTime = GetTime( );

			m_DropLaggerTimer.start(m_WaitTime*1000);

			if( LaggingString.isEmpty( ) )
				LaggingString = (*i)->GetName( );
			else
				LaggingString += ", " + (*i)->GetName( );
		}
	}

	if( m_Lagging )
	{
		// start the lag screen

		CONSOLE_Print( "[GAME: " + m_GameName + "] started lagging on [" + LaggingString + "]" );
		SendAll( m_Protocol->SEND_W3GS_START_LAG( m_Players ) );

		// reset everyone's drop vote

		for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
			(*i)->SetDropVote( false );

		m_ResetLagscreenTimer.start();
		m_SendActionTimer.stop();
	}
}

void CBaseGame::EventDropLaggerTimeout()
{
	StopLaggers( m_GHost->GetLanguage( )->WasAutomaticallyDroppedAfterSeconds( QString::number(m_WaitTime) ) );
}

void CBaseGame::EventResetLagscreenTimeout()
{
	if (!m_Lagging)
	{
		m_ResetLagscreenTimer.stop();
		return;
	}

	bool UsingGProxy = false;

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( (*i)->GetGProxy( ) )
			UsingGProxy = true;
	}

	// we cannot allow the lag screen to stay up for more than ~65 seconds because Warcraft III disconnects if it doesn't receive an action packet at least this often
	// one (easy) solution is to simply drop all the laggers if they lag for more than 60 seconds
	// another solution is to reset the lag screen the same way we reset it when using load-in-game
	// this is required in order to give GProxy++ clients more time to reconnect

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		// stop the lag screen

		for( QList<CGamePlayer *> :: iterator j = m_Players.begin( ); j != m_Players.end( ); j++ )
		{
			if( (*j)->GetLagging( ) )
				Send( *i, m_Protocol->SEND_W3GS_STOP_LAG( *j ) );
		}

		// send an empty update
		// this resets the lag screen timer

		if( UsingGProxy && !(*i)->GetGProxy( ) )
		{
			// we must send additional empty actions to non-GProxy++ players
			// GProxy++ will insert these itself so we don't need to send them to GProxy++ players
			// empty actions are used to extend the time a player can use when reconnecting

			for( unsigned char j = 0; j < m_GProxyEmptyActions; j++ )
				Send( *i, m_Protocol->SEND_W3GS_INCOMING_ACTION( QQueue<CIncomingAction *>( ), 0 ) );
		}

		Send( *i, m_Protocol->SEND_W3GS_INCOMING_ACTION( QQueue<CIncomingAction *>( ), 0 ) );

		// start the lag screen

		Send( *i, m_Protocol->SEND_W3GS_START_LAG( m_Players ) );
	}

	// add actions to replay

	if( m_Replay )
	{
		if( UsingGProxy )
		{
			for( unsigned char i = 0; i < m_GProxyEmptyActions; i++ )
				m_Replay->AddTimeSlot( 0, QQueue<CIncomingAction *>( ) );
		}

		m_Replay->AddTimeSlot( 0, QQueue<CIncomingAction *>( ) );
	}

	// Warcraft III doesn't seem to respond to empty actions

	/* if( UsingGProxy )
		m_SyncCounter += m_GProxyEmptyActions;

	m_SyncCounter++; */
}

void CBaseGame::EventPlayerStoppedLagging()
{
	if( !m_Lagging )
		return;

	CGamePlayer *player = (CGamePlayer*)QObject::sender();
	if (player == NULL)
		return;

	// we consider a player to have stopped lagging if they're less than half m_SyncLimit keepalives behind

	CONSOLE_Print( "[GAME: " + m_GameName + "] stopped lagging on [" + player->GetName( ) + "]" );
	SendAll( m_Protocol->SEND_W3GS_STOP_LAG( player ) );
	player->SetStartedLaggingTicks( 0 );

	// check if everyone has stopped lagging

	bool Lagging = false;

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( (*i)->GetLagging( ) )
			Lagging = true;
	}

	m_Lagging = Lagging;

	if (!m_Lagging)
	{
		m_DropLaggerTimer.stop();

		// we continue to send actions
		m_SendActionTimer.start();
	}
}

void CBaseGame :: EventPlayerDeleted()
{
	CGamePlayer *player = (CGamePlayer*)QObject::sender();
	CONSOLE_Print( "[GAME: " + m_GameName + "] deleting player [" + player->GetName( ) + "]: " + player->GetLeftReason( ) );

	if( !m_Players.removeOne(player) )
		return;

	// create the virtual host player if there is room
	if( !m_GameLoading && !m_GameLoaded && GetNumPlayers( ) < 12 )
		CreateVirtualHost( );

	bool res = false;
	// check if there's another player with reserved status in the game
	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		if( (*i)->GetReserved( ) )
			res = true;

	if (res)
		m_LobbyTimeoutTimer.start();

	// unlock the game if the owner leaves
	if( m_Locked && player->GetName() == m_OwnerName )
	{
		SendAllChat( m_GHost->GetLanguage( )->GameUnlocked( ) );
		m_Locked = false;
	}

	// start the gameover timer if there's only one player left
	if( ( m_GameLoading || m_GameLoaded ) && m_Players.size( ) == 1 && m_FakePlayerPID == 255 && !m_GameOverTimer.isActive() )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] gameover timer started (one player left)" );
		m_GameOverTimer.start();
	}

	// end the game if there aren't any players left
	if( m_Players.isEmpty( ) && (m_GameLoading || m_GameLoaded) )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] is over (no players left)" );
		SaveGameData( );
	}

	// remove any queued spoofcheck messages for this player

	if( player->GetWhoisSent( ) && !player->GetJoinedRealm( ).isEmpty( ) && player->GetSpoofedRealm( ).isEmpty( ) )
	{
		for( QList<CBNET *> :: const_iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
		{
			if( (*i)->GetServer( ) == player->GetJoinedRealm( ) )
			{
				// hackhack: there must be a better way to do this

				if( (*i)->GetPasswordHashType( ) == "pvpgn" )
					(*i)->UnqueueChatCommand( "/whereis " + player->GetName( ) );
				else
					(*i)->UnqueueChatCommand( "/whois " + player->GetName( ) );

				(*i)->UnqueueChatCommand( "/w " + player->GetName( ) + " " + m_GHost->GetLanguage( )->SpoofCheckByReplying( ) );
			}
		}
	}

	m_LastPlayerLeaveTicks = GetTicks( );

	// in some cases we're forced to send the left message early so don't send it again

	if( player->GetLeftMessageSent( ) )
		return;

	if( m_GameLoaded )
		SendAllChat( player->GetName( ) + " " + player->GetLeftReason( ) + "." );

	if( player->GetLagging( ) )
		SendAll( m_Protocol->SEND_W3GS_STOP_LAG( player ) );

	// autosave

	if( m_GameLoaded && player->GetLeftCode( ) == PLAYERLEAVE_DISCONNECT && m_AutoSave )
	{
		QString SaveGameName = UTIL_FileSafeName( "GHost++ AutoSave " + m_GameName + " (" + player->GetName( ) + ").w3z" );
		CONSOLE_Print( "[GAME: " + m_GameName + "] auto saving [" + SaveGameName + "] before player drop, shortened send interval = " + QString::number( GetTicks( ) - m_LastActionSentTicks ) );
		QByteArray CRC;
		QByteArray Action;
		Action.push_back( 6 );
		Action.append(SaveGameName.toUtf8());
		m_Actions.enqueue( new CIncomingAction( player->GetPID( ), CRC, Action ) );

		// todotodo: with the new latency system there needs to be a way to send a 0-time action

		SendAllActions( );
	}

	if( m_GameLoading && m_LoadInGame )
	{
		// we must buffer player leave messages when using "load in game" to prevent desyncs
		// this ensures the player leave messages are correctly interleaved with the empty updates sent to each player

		for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			if( (*i)->GetFinishedLoading( ) )
			{
				if( !player->GetFinishedLoading( ) )
					Send( *i, m_Protocol->SEND_W3GS_STOP_LAG( player ) );

				Send( *i, m_Protocol->SEND_W3GS_PLAYERLEAVE_OTHERS( player->GetPID( ), player->GetLeftCode( ) ) );
			}
			else
				(*i)->AddLoadInGameData( m_Protocol->SEND_W3GS_PLAYERLEAVE_OTHERS( player->GetPID( ), player->GetLeftCode( ) ) );
		}
	}
	else
	{
		// tell everyone about the player leaving

		SendAll( m_Protocol->SEND_W3GS_PLAYERLEAVE_OTHERS( player->GetPID( ), player->GetLeftCode( ) ) );
	}

	// set the replay's host PID and name to the last player to leave the game
	// this will get overwritten as each player leaves the game so it will eventually be set to the last player

	if( m_Replay && ( m_GameLoading || m_GameLoaded ) )
	{
		m_Replay->SetHostPID( player->GetPID( ) );
		m_Replay->SetHostName( player->GetName( ) );

		// add leave message to replay

		if( m_GameLoading && !m_LoadInGame )
			m_Replay->AddLeaveGameDuringLoading( 1, player->GetPID( ), player->GetLeftCode( ) );
		else
			m_Replay->AddLeaveGame( 1, player->GetPID( ), player->GetLeftCode( ) );
	}

	// abort the countdown if there was one in progress

	if( m_CountDownStarted && !m_GameLoading && !m_GameLoaded )
	{
		SendAllChat( m_GHost->GetLanguage( )->CountDownAborted( ) );
		m_CountDownStarted = false;
	}

	// abort the votekick

	if( !m_KickVotePlayer.isEmpty( ) )
		SendAllChat( m_GHost->GetLanguage( )->VoteKickCancelled( m_KickVotePlayer ) );

	m_KickVotePlayer.clear( );
	m_VotekickTimer.stop();
}

void CBaseGame :: EventPlayerDisconnectTimedOut( CGamePlayer *player )
{
	if( player->GetGProxy( ) && m_GameLoaded )
	{
		if( !player->GetGProxyDisconnectNoticeSent( ) )
		{
			SendAllChat( player->GetName( ) + " " + m_GHost->GetLanguage( )->HasLostConnectionTimedOutGProxy( ) + "." );
			player->SetGProxyDisconnectNoticeSent( true );
		}

		player->m_SendGProxyMessageTimer.start();
		return;
	}

	// not only do we not do any timeouts if the game is lagging, we allow for an additional grace period of 5 seconds
	// this is because Warcraft 3 stops sending packets during the lag screen
	// so when the lag screen finishes we would immediately disconnect everyone if we didn't give them some extra time

	QTimer::singleShot(5000, this, SLOT(EventPlayerLaggedOut(player)));
}

void CBaseGame::EventPlayerLaggedOut(CGamePlayer *player)
{
	player->SetLeftReason( m_GHost->GetLanguage( )->HasLostConnectionTimedOut( ) );
	player->SetLeftCode( PLAYERLEAVE_DISCONNECT );

	if( !m_GameLoading && !m_GameLoaded )
		OpenSlot( GetSIDFromPID( player->GetPID( ) ), false );
}

void CBaseGame :: EventPlayerDisconnectPlayerError( CGamePlayer *player )
{
	// at the time of this comment there's only one player error and that's when we receive a bad packet from the player
	// since TCP has checks and balances for data corruption the chances of this are pretty slim

	player->SetLeftReason( m_GHost->GetLanguage( )->HasLostConnectionPlayerError( player->GetErrorString( ) ) );
	player->SetLeftCode( PLAYERLEAVE_DISCONNECT );

	if( !m_GameLoading && !m_GameLoaded )
		OpenSlot( GetSIDFromPID( player->GetPID( ) ), false );
}

void CBaseGame :: EventPlayerDisconnectSocketError( CGamePlayer *player )
{
	if( player->GetGProxy( ) && m_GameLoaded )
	{
		if( !player->GetGProxyDisconnectNoticeSent( ) )
		{
			SendAllChat( player->GetName( ) + " " + m_GHost->GetLanguage( )->HasLostConnectionSocketErrorGProxy( player->GetSocket( )->errorString() ) + "." );
			player->SetGProxyDisconnectNoticeSent( true );
		}

		if( GetTime( ) - player->GetLastGProxyWaitNoticeSentTime( ) >= 20 )
		{
			quint32 TimeRemaining = ( m_GProxyEmptyActions + 1 ) * 60 - ( GetTime( ) - m_StartedLaggingTime );

			if( TimeRemaining > ( (quint32)m_GProxyEmptyActions + 1 ) * 60 )
				TimeRemaining = ( m_GProxyEmptyActions + 1 ) * 60;

			SendAllChat( player->GetPID( ), m_GHost->GetLanguage( )->WaitForReconnectSecondsRemain( QString::number( TimeRemaining ) ) );
			player->SetLastGProxyWaitNoticeSentTime( GetTime( ) );
		}

		return;
	}

	player->SetLeftReason( m_GHost->GetLanguage( )->HasLostConnectionSocketError( player->GetSocket( )->errorString( ) ) );
	player->SetLeftCode( PLAYERLEAVE_DISCONNECT );

	if( !m_GameLoading && !m_GameLoaded )
		OpenSlot( GetSIDFromPID( player->GetPID( ) ), false );
}

void CBaseGame :: EventPlayerDisconnectConnectionClosed( CGamePlayer *player )
{
	if( player->GetGProxy( ) && m_GameLoaded )
	{
		if( !player->GetGProxyDisconnectNoticeSent( ) )
		{
			SendAllChat( player->GetName( ) + " " + m_GHost->GetLanguage( )->HasLostConnectionClosedByRemoteHostGProxy( ) + "." );
			player->SetGProxyDisconnectNoticeSent( true );
		}

		if( GetTime( ) - player->GetLastGProxyWaitNoticeSentTime( ) >= 20 )
		{
			quint32 TimeRemaining = ( m_GProxyEmptyActions + 1 ) * 60 - ( GetTime( ) - m_StartedLaggingTime );

			if( TimeRemaining > ( (quint32)m_GProxyEmptyActions + 1 ) * 60 )
				TimeRemaining = ( m_GProxyEmptyActions + 1 ) * 60;

			SendAllChat( player->GetPID( ), m_GHost->GetLanguage( )->WaitForReconnectSecondsRemain( QString::number( TimeRemaining ) ) );
			player->SetLastGProxyWaitNoticeSentTime( GetTime( ) );
		}

		return;
	}

	player->SetLeftReason( m_GHost->GetLanguage( )->HasLostConnectionClosedByRemoteHost( ) );
	player->SetLeftCode( PLAYERLEAVE_DISCONNECT );

	if( !m_GameLoading && !m_GameLoaded )
		OpenSlot( GetSIDFromPID( player->GetPID( ) ), false );
}

void CBaseGame :: EventPlayerJoined( CPotentialPlayer *potential, CIncomingJoinPlayer *joinPlayer )
{
	// check if the new player's name is empty or too long

	if( joinPlayer->GetName( ).isEmpty( ) || joinPlayer->GetName( ).size( ) > 15 )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] is trying to join the game with an invalid name of length " + QString::number( joinPlayer->GetName( ).size( ) ) );
		potential->Send( m_Protocol->SEND_W3GS_REJECTJOIN( REJECTJOIN_FULL ) );
		potential->deleteLater();
		return;
	}

	// check if the new player's name is the same as the virtual host name

	if( joinPlayer->GetName( ) == m_VirtualHostName )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] is trying to join the game with the virtual host name" );
		potential->Send( m_Protocol->SEND_W3GS_REJECTJOIN( REJECTJOIN_FULL ) );
		potential->deleteLater();
		return;
	}

	// check if the new player's name is already taken

	if( GetPlayerFromName( joinPlayer->GetName( ), false ) )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] is trying to join the game but that name is already taken" );
		// SendAllChat( m_GHost->GetLanguage( )->TryingToJoinTheGameButTaken( joinPlayer->GetName( ) ) );
		potential->Send( m_Protocol->SEND_W3GS_REJECTJOIN( REJECTJOIN_FULL ) );
		potential->deleteLater();
		return;
	}

	// identify their joined realm
	// this is only possible because when we send a game refresh via LAN or battle.net we encode an ID value in the 4 most significant bits of the host counter
	// the client sends the host counter when it joins so we can extract the ID value here
	// note: this is not a replacement for spoof checking since it doesn't verify the player's name and it can be spoofed anyway

	quint32 HostCounterID = joinPlayer->GetHostCounter( ) >> 28;
	QString JoinedRealm;

	// we use an ID value of 0 to denote joining via LAN

	if( HostCounterID != 0 )
	{
		for( QList<CBNET *> :: const_iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
		{
			if( (*i)->GetHostCounterID( ) == HostCounterID )
				JoinedRealm = (*i)->GetServer( );
		}
	}

	// check if the new player's name is banned but only if bot_banmethod is not 0
	// this is because if bot_banmethod is 0 and we announce the ban here it's possible for the player to be rejected later because the game is full
	// this would allow the player to spam the chat by attempting to join the game multiple times in a row

	if( m_GHost->m_BanMethod != 0 )
	{
		for( QList<CBNET *> :: const_iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
		{
			if( (*i)->GetServer( ) == JoinedRealm )
			{
				CDBBan *Ban = (*i)->IsBannedName( joinPlayer->GetName( ) );

				if( Ban )
				{
					if( m_GHost->m_BanMethod == 1 || m_GHost->m_BanMethod == 3 )
					{
						CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] is trying to join the game but is banned by name" );

						if( m_IgnoredNames.find( joinPlayer->GetName( ) ) == m_IgnoredNames.end( ) )
						{
							SendAllChat( m_GHost->GetLanguage( )->TryingToJoinTheGameButBannedByName( joinPlayer->GetName( ) ) );
							SendAllChat( m_GHost->GetLanguage( )->UserWasBannedOnByBecause( Ban->GetServer( ), Ban->GetName( ), Ban->GetDate( ), Ban->GetAdmin( ), Ban->GetReason( ) ) );
							m_IgnoredNames.insert( joinPlayer->GetName( ) );
						}

						// let banned players "join" the game with an arbitrary PID then immediately close the connection
						// this causes them to be kicked back to the chat channel on battle.net

						QList<CGameSlot> Slots = m_Map->GetSlots( );
						potential->Send( m_Protocol->SEND_W3GS_SLOTINFOJOIN( 1, Util::fromUInt16(potential->GetSocket( )->localPort()), potential->GetExternalIP( ), Slots, 0, m_Map->GetMapLayoutStyle( ), m_Map->GetMapNumPlayers( ) ) );
						potential->deleteLater();
						return;
					}

					break;
				}
			}

			CDBBan *Ban = (*i)->IsBannedIP( potential->GetExternalIPString( ) );

			if( Ban )
			{
				if( m_GHost->m_BanMethod == 2 || m_GHost->m_BanMethod == 3 )
				{
					CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] is trying to join the game but is banned by IP address" );

					if( m_IgnoredNames.find( joinPlayer->GetName( ) ) == m_IgnoredNames.end( ) )
					{
						SendAllChat( m_GHost->GetLanguage( )->TryingToJoinTheGameButBannedByIP( joinPlayer->GetName( ), potential->GetExternalIPString( ), Ban->GetName( ) ) );
						SendAllChat( m_GHost->GetLanguage( )->UserWasBannedOnByBecause( Ban->GetServer( ), Ban->GetName( ), Ban->GetDate( ), Ban->GetAdmin( ), Ban->GetReason( ) ) );
						m_IgnoredNames.insert( joinPlayer->GetName( ) );
					}

					// let banned players "join" the game with an arbitrary PID then immediately close the connection
					// this causes them to be kicked back to the chat channel on battle.net

					QList<CGameSlot> Slots = m_Map->GetSlots( );
					potential->Send( m_Protocol->SEND_W3GS_SLOTINFOJOIN( 1, Util::fromUInt16((quint16)potential->GetSocket( )->localPort()), potential->GetExternalIP( ), Slots, 0, m_Map->GetMapLayoutStyle( ), m_Map->GetMapNumPlayers( ) ) );
					potential->deleteLater();
					return;
				}

				break;
			}
		}
	}

	if( m_MatchMaking && m_AutoStartPlayers != 0 && !m_Map->GetMapMatchMakingCategory( ).isEmpty( ) && m_Map->GetMapOptions( ) & MAPOPT_FIXEDPLAYERSETTINGS )
	{
		// matchmaking is enabled
		// start a database query to determine the player's score
		// when the query is complete we will call EventPlayerJoinedWithScore

		m_ScoreChecks.push_back( m_GHost->m_DB->ThreadedScoreCheck( m_Map->GetMapMatchMakingCategory( ), joinPlayer->GetName( ), JoinedRealm ) );
		return;
	}

	// check if the player is an admin or root admin on any connected realm for determining reserved status
	// we can't just use the spoof checked realm like in EventPlayerBotCommand because the player hasn't spoof checked yet

	bool AnyAdminCheck = false;

	for( QList<CBNET *> :: const_iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
	{
		if( (*i)->IsAdmin( joinPlayer->GetName( ) ) || (*i)->IsRootAdmin( joinPlayer->GetName( ) ) )
		{
			AnyAdminCheck = true;
			break;
		}
	}

	bool Reserved = IsReserved( joinPlayer->GetName( ) ) || ( m_GHost->m_ReserveAdmins && AnyAdminCheck ) || IsOwner( joinPlayer->GetName( ) );

	// stop some timers
	if (Reserved)
		m_LobbyTimeoutTimer.stop();

	// try to find a slot

	unsigned char SID = 255;
	unsigned char EnforcePID = 255;
	unsigned char EnforceSID = 0;
	CGameSlot EnforceSlot( 255, 0, 0, 0, 0, 0, 0 );

	if( m_SaveGame )
	{
		// in a saved game we enforce the player layout and the slot layout
		// unfortunately we don't know how to extract the player layout from the saved game so we use the data from a replay instead
		// the !enforcesg command defines the player layout by parsing a replay

		for( QList<PIDPlayer> :: const_iterator i = m_EnforcePlayers.begin( ); i != m_EnforcePlayers.end( ); i++ )
		{
			if( (*i).second == joinPlayer->GetName( ) )
				EnforcePID = (*i).first;
		}

		for( QList<CGameSlot> :: iterator i = m_EnforceSlots.begin( ); i != m_EnforceSlots.end( ); i++ )
		{
			if( (*i).GetPID( ) == EnforcePID )
			{
				EnforceSlot = *i;
				break;
			}

			EnforceSID++;
		}

		if( EnforcePID == 255 || EnforceSlot.GetPID( ) == 255 || EnforceSID >= m_Slots.size( ) )
		{
			CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] is trying to join the game but isn't in the enforced list" );
			potential->Send( m_Protocol->SEND_W3GS_REJECTJOIN( REJECTJOIN_FULL ) );
			potential->deleteLater();
			return;
		}

		SID = EnforceSID;
	}
	else
	{
		// try to find an empty slot

		SID = GetEmptySlot( false );

		if( SID == 255 && Reserved )
		{
			// a reserved player is trying to join the game but it's full, try to find a reserved slot

			SID = GetEmptySlot( true );

			if( SID != 255 )
			{
				CGamePlayer *KickedPlayer = GetPlayerFromSID( SID );

				if( KickedPlayer )
				{
					KickedPlayer->SetLeftReason( m_GHost->GetLanguage( )->WasKickedForReservedPlayer( joinPlayer->GetName( ) ) );
					KickedPlayer->SetLeftCode( PLAYERLEAVE_LOBBY );

					// send a playerleave message immediately since it won't normally get sent until the player is deleted which is after we send a playerjoin message
					// we don't need to call OpenSlot here because we're about to overwrite the slot data anyway

					SendAll( m_Protocol->SEND_W3GS_PLAYERLEAVE_OTHERS( KickedPlayer->GetPID( ), KickedPlayer->GetLeftCode( ) ) );
					KickedPlayer->SetLeftMessageSent( true );
					KickedPlayer->deleteLater();
				}
			}
		}

		if( SID == 255 && IsOwner( joinPlayer->GetName( ) ) )
		{
			// the owner player is trying to join the game but it's full and we couldn't even find a reserved slot, kick the player in the lowest numbered slot
			// updated this to try to find a player slot so that we don't end up kicking a computer

			SID = 0;

			for( unsigned char i = 0; i < m_Slots.size( ); i++ )
			{
				if( m_Slots[i].GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && m_Slots[i].GetComputer( ) == 0 )
				{
					SID = i;
					break;
				}
			}

			CGamePlayer *KickedPlayer = GetPlayerFromSID( SID );

			if( KickedPlayer )
			{
				KickedPlayer->SetLeftReason( m_GHost->GetLanguage( )->WasKickedForOwnerPlayer( joinPlayer->GetName( ) ) );
				KickedPlayer->SetLeftCode( PLAYERLEAVE_LOBBY );

				// send a playerleave message immediately since it won't normally get sent until the player is deleted which is after we send a playerjoin message
				// we don't need to call OpenSlot here because we're about to overwrite the slot data anyway

				SendAll( m_Protocol->SEND_W3GS_PLAYERLEAVE_OTHERS( KickedPlayer->GetPID( ), KickedPlayer->GetLeftCode( ) ) );
				KickedPlayer->SetLeftMessageSent( true );
				KickedPlayer->deleteLater();
			}
		}
	}

	if( SID >= m_Slots.size( ) )
	{
		potential->Send( m_Protocol->SEND_W3GS_REJECTJOIN( REJECTJOIN_FULL ) );
		potential->deleteLater();
		return;
	}

	// check if the new player's name is banned but only if bot_banmethod is 0
	// this is because if bot_banmethod is 0 we need to wait to announce the ban until now because they could have been rejected because the game was full
	// this would have allowed the player to spam the chat by attempting to join the game multiple times in a row

	if( m_GHost->m_BanMethod == 0 )
	{
		for( QList<CBNET *> :: const_iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
		{
			if( (*i)->GetServer( ) == JoinedRealm )
			{
				CDBBan *Ban = (*i)->IsBannedName( joinPlayer->GetName( ) );

				if( Ban )
				{
					CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] is using a banned name" );
					SendAllChat( m_GHost->GetLanguage( )->HasBannedName( joinPlayer->GetName( ) ) );
					SendAllChat( m_GHost->GetLanguage( )->UserWasBannedOnByBecause( Ban->GetServer( ), Ban->GetName( ), Ban->GetDate( ), Ban->GetAdmin( ), Ban->GetReason( ) ) );
					break;
				}
			}

			CDBBan *Ban = (*i)->IsBannedIP( potential->GetExternalIPString( ) );

			if( Ban )
			{
				CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] is using a banned IP address" );
				SendAllChat( m_GHost->GetLanguage( )->HasBannedIP( joinPlayer->GetName( ), potential->GetExternalIPString( ), Ban->GetName( ) ) );
				SendAllChat( m_GHost->GetLanguage( )->UserWasBannedOnByBecause( Ban->GetServer( ), Ban->GetName( ), Ban->GetDate( ), Ban->GetAdmin( ), Ban->GetReason( ) ) );
				break;
			}
		}
	}

	// we have a slot for the new player
	// make room for them by deleting the virtual host player if we have to

	if( GetNumPlayers( ) >= 11 || EnforcePID == m_VirtualHostPID )
		DeleteVirtualHost( );

	// turning the CPotentialPlayer into a CGamePlayer is a bit of a pain because we have to be careful not to close the socket
	// this problem is solved by setting the socket to NULL before deletion and handling the NULL case in the destructor
	// we also have to be careful to not modify the m_Potentials vector since we're currently looping through it

	CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] joined the game" );
	CGamePlayer *Player = new CGamePlayer( potential, m_SaveGame ? EnforcePID : GetNewPID( ), JoinedRealm, joinPlayer->GetName( ), joinPlayer->GetInternalIP( ), Reserved );

	// consider LAN players to have already spoof checked since they can't
	// since so many people have trouble with this feature we now use the JoinedRealm to determine LAN status

	if( JoinedRealm.isEmpty( ) )
		Player->SetSpoofed( true );

	Player->SetWhoisShouldBeSent( m_GHost->m_SpoofChecks == 1 || ( m_GHost->m_SpoofChecks == 2 && AnyAdminCheck ) );
	m_Players.push_back( Player );
	potential->SetSocket( NULL );
	potential->deleteLater();

	if( m_SaveGame )
		m_Slots[SID] = EnforceSlot;
	else
	{
		if( m_Map->GetMapOptions( ) & MAPOPT_FIXEDPLAYERSETTINGS )
			m_Slots[SID] = CGameSlot( Player->GetPID( ), 255, SLOTSTATUS_OCCUPIED, 0, m_Slots[SID].GetTeam( ), m_Slots[SID].GetColour( ), m_Slots[SID].GetRace( ) );
		else
		{
			if( m_Map->GetMapFlags( ) & MAPFLAG_RANDOMRACES )
				m_Slots[SID] = CGameSlot( Player->GetPID( ), 255, SLOTSTATUS_OCCUPIED, 0, 12, 12, SLOTRACE_RANDOM );
			else
				m_Slots[SID] = CGameSlot( Player->GetPID( ), 255, SLOTSTATUS_OCCUPIED, 0, 12, 12, SLOTRACE_RANDOM | SLOTRACE_SELECTABLE );

			// try to pick a team and colour
			// make sure there aren't too many other players already

			unsigned char NumOtherPlayers = 0;

			for( unsigned char i = 0; i < m_Slots.size( ); i++ )
			{
				if( m_Slots[i].GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && m_Slots[i].GetTeam( ) != 12 )
					NumOtherPlayers++;
			}

			if( NumOtherPlayers < m_Map->GetMapNumPlayers( ) )
			{
				if( SID < m_Map->GetMapNumPlayers( ) )
					m_Slots[SID].SetTeam( SID );
				else
					m_Slots[SID].SetTeam( 0 );

				m_Slots[SID].SetColour( GetNewColour( ) );
			}
		}
	}

	// send slot info to the new player
	// the SLOTINFOJOIN packet also tells the client their assigned PID and that the join was successful

	Player->Send( m_Protocol->SEND_W3GS_SLOTINFOJOIN( Player->GetPID( ), Util::fromUInt16(Player->GetSocket( )->localPort()), Player->GetExternalIP( ), m_Slots, m_RandomSeed, m_Map->GetMapLayoutStyle( ), m_Map->GetMapNumPlayers( ) ) );

	// send virtual host info and fake player info (if present) to the new player

	SendVirtualHostPlayerInfo( Player );
	SendFakePlayerInfo( Player );

	QByteArray BlankIP;
	BlankIP.push_back( (char)0 );
	BlankIP.push_back( (char)0 );
	BlankIP.push_back( (char)0 );
	BlankIP.push_back( (char)0 );

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( !(*i)->GetLeftMessageSent( ) && *i != Player )
		{
			// send info about the new player to every other player

			if( (*i)->GetSocket( ) )
			{
				if( m_GHost->m_HideIPAddresses )
					(*i)->Send( m_Protocol->SEND_W3GS_PLAYERINFO( Player->GetPID( ), Player->GetName( ), BlankIP, BlankIP ) );
				else
					(*i)->Send( m_Protocol->SEND_W3GS_PLAYERINFO( Player->GetPID( ), Player->GetName( ), Player->GetExternalIP( ), Player->GetInternalIP( ) ) );
			}

			// send info about every other player to the new player

			if( m_GHost->m_HideIPAddresses )
				Player->Send( m_Protocol->SEND_W3GS_PLAYERINFO( (*i)->GetPID( ), (*i)->GetName( ), BlankIP, BlankIP ) );
			else
				Player->Send( m_Protocol->SEND_W3GS_PLAYERINFO( (*i)->GetPID( ), (*i)->GetName( ), (*i)->GetExternalIP( ), (*i)->GetInternalIP( ) ) );
		}
	}

	// send a map check packet to the new player

	Player->Send( m_Protocol->SEND_W3GS_MAPCHECK( m_Map->GetMapPath( ), m_Map->GetMapSize( ), m_Map->GetMapInfo( ), m_Map->GetMapCRC( ), m_Map->GetMapSHA1( ) ) );

	// send slot info to everyone, so the new player gets this info twice but everyone else still needs to know the new slot layout

	SendAllSlotInfo( );

	// send a welcome message

	SendWelcomeMessage( Player );

	// if spoof checks are required and we won't automatically spoof check this player then tell them how to spoof check
	// e.g. if automatic spoof checks are disabled, or if automatic spoof checks are done on admins only and this player isn't an admin

	if( m_GHost->m_RequireSpoofChecks && !Player->GetWhoisShouldBeSent( ) )
	{
		for( QList<CBNET *> :: const_iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
		{
			// note: the following (commented out) line of code will crash because calling GetUniqueName( ) twice will result in two different return values
			// and unfortunately iterators are not valid if compared against different containers
			// this comment shall serve as warning to not make this mistake again since it has now been made twice before in GHost++
			// QString( (*i)->GetUniqueName( ).begin( ), (*i)->GetUniqueName( ).end( ) )

			QByteArray UniqueName = (*i)->GetUniqueName( );

			if( (*i)->GetServer( ) == JoinedRealm )
				SendChat( Player, m_GHost->GetLanguage( )->SpoofCheckByWhispering( UniqueName ) );
		}
	}

	// check for multiple IP usage

	if( m_GHost->m_CheckMultipleIPUsage )
	{
		QString Others;

		for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			if( Player != *i && Player->GetExternalIPString( ) == (*i)->GetExternalIPString( ) )
			{
				if( Others.isEmpty( ) )
					Others = (*i)->GetName( );
				else
					Others += ", " + (*i)->GetName( );
			}
		}

		if( !Others.isEmpty( ) )
			SendAllChat( m_GHost->GetLanguage( )->MultipleIPAddressUsageDetected( joinPlayer->GetName( ), Others ) );
	}

	// abort the countdown if there was one in progress

	if( m_CountDownStarted && !m_GameLoading && !m_GameLoaded )
	{
		SendAllChat( m_GHost->GetLanguage( )->CountDownAborted( ) );
		m_CountDownStarted = false;
	}

	// auto lock the game

	if( m_GHost->m_AutoLock && !m_Locked && IsOwner( joinPlayer->GetName( ) ) )
	{
		SendAllChat( m_GHost->GetLanguage( )->GameLocked( ) );
		m_Locked = true;
	}
}

void CBaseGame :: EventPlayerJoinedWithScore( CPotentialPlayer *potential, CIncomingJoinPlayer *joinPlayer, double score )
{
	// this function is only called when matchmaking is enabled
	// EventPlayerJoined will be called first in all cases
	// if matchmaking is enabled EventPlayerJoined will start a database query to retrieve the player's score and keep the connection open while we wait
	// when the database query is complete EventPlayerJoinedWithScore will be called

	// check if the new player's name is the same as the virtual host name

	if( joinPlayer->GetName( ) == m_VirtualHostName )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] is trying to join the game with the virtual host name" );
		potential->Send( m_Protocol->SEND_W3GS_REJECTJOIN( REJECTJOIN_FULL ) );
		potential->deleteLater();
		return;
	}

	// check if the new player's name is already taken

	if( GetPlayerFromName( joinPlayer->GetName( ), false ) )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] is trying to join the game but that name is already taken" );
		// SendAllChat( m_GHost->GetLanguage( )->TryingToJoinTheGameButTaken( joinPlayer->GetName( ) ) );
		potential->Send( m_Protocol->SEND_W3GS_REJECTJOIN( REJECTJOIN_FULL ) );
		potential->deleteLater();
		return;
	}

	// check if the new player's score is within the limits

	if( score > -99999.0 && ( score < m_MinimumScore || score > m_MaximumScore ) )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] is trying to join the game but has a rating [" + QString::number( score, 'g', 2 ) + "] outside the limits [" + QString::number( m_MinimumScore, 'g', 2 ) + "] to [" + QString::number( m_MaximumScore, 'g', 2 ) + "]" );
		potential->Send( m_Protocol->SEND_W3GS_REJECTJOIN( REJECTJOIN_FULL ) );
		potential->deleteLater();
		return;
	}

	// try to find an empty slot

	unsigned char SID = GetEmptySlot( false );

	// check if the player is an admin or root admin on any connected realm for determining reserved status
	// we can't just use the spoof checked realm like in EventPlayerBotCommand because the player hasn't spoof checked yet

	bool AnyAdminCheck = false;

	for( QList<CBNET *> :: const_iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
	{
		if( (*i)->IsAdmin( joinPlayer->GetName( ) ) || (*i)->IsRootAdmin( joinPlayer->GetName( ) ) )
		{
			AnyAdminCheck = true;
			break;
		}
	}

	bool Reserved = IsReserved( joinPlayer->GetName( ) ) || ( m_GHost->m_ReserveAdmins && AnyAdminCheck ) || IsOwner( joinPlayer->GetName( ) );

	// stop some timers
	if (Reserved)
		m_LobbyTimeoutTimer.stop();

	if( SID == 255 )
	{
		// no empty slot found, time to do some matchmaking!
		// note: the database code uses a score of -100000 to denote "no score"

		if( m_GHost->m_MatchMakingMethod == 0 )
		{
			// method 0: don't do any matchmaking
			// that was easy!
		}
		else if( m_GHost->m_MatchMakingMethod == 1 )
		{
			// method 1: furthest score method
			// calculate the average score of all players in the game
			// then kick the player with the score furthest from that average (or a player without a score)
			// this ensures that the players' scores will tend to converge as players join the game

			double AverageScore = 0.0;
			quint32 PlayersScored = 0;

			if( score > -99999.0 )
			{
				AverageScore = score;
				PlayersScored = 1;
			}

			for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
			{
				if( (*i)->GetScore( ) > -99999.0 )
				{
					AverageScore += (*i)->GetScore( );
					PlayersScored++;
				}
			}

			if( PlayersScored > 0 )
				AverageScore /= PlayersScored;

			// calculate the furthest player from the average

			CGamePlayer *FurthestPlayer = NULL;

			for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
			{
				if( !FurthestPlayer || (*i)->GetScore( ) < -99999.0 || abs( (*i)->GetScore( ) - AverageScore ) > abs( FurthestPlayer->GetScore( ) - AverageScore ) )
					FurthestPlayer = *i;
			}

			if( !FurthestPlayer )
			{
				// this should be impossible

				CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] is trying to join the game but no furthest player was found (this should be impossible)" );
				potential->Send( m_Protocol->SEND_W3GS_REJECTJOIN( REJECTJOIN_FULL ) );
				potential->deleteLater();
				return;
			}

			// kick the new player if they have the furthest score

			if( score < -99999.0 || abs( score - AverageScore ) > abs( FurthestPlayer->GetScore( ) - AverageScore ) )
			{
				if( score < -99999.0 )
					CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] is trying to join the game but has the furthest rating [N/A] from the average [" + QString::number( AverageScore, 'g', 2 ) + "]" );
				else
					CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] is trying to join the game but has the furthest rating [" + QString::number( score, 'g', 2 ) + "] from the average [" + QString::number( AverageScore, 'g', 2 ) + "]" );

				potential->Send( m_Protocol->SEND_W3GS_REJECTJOIN( REJECTJOIN_FULL ) );
				potential->deleteLater();
				return;
			}

			// kick the furthest player

			SID = GetSIDFromPID( FurthestPlayer->GetPID( ) );

			if( FurthestPlayer->GetScore( ) < -99999.0 )
				FurthestPlayer->SetLeftReason( m_GHost->GetLanguage( )->WasKickedForHavingFurthestScore( "N/A", QString::number( AverageScore, 'g', 2 ) ) );
			else
				FurthestPlayer->SetLeftReason( m_GHost->GetLanguage( )->WasKickedForHavingFurthestScore( QString::number( FurthestPlayer->GetScore( ), 'g', 2 ), QString::number( AverageScore, 'g', 2 ) ) );

			FurthestPlayer->SetLeftCode( PLAYERLEAVE_LOBBY );

			// send a playerleave message immediately since it won't normally get sent until the player is deleted which is after we send a playerjoin message
			// we don't need to call OpenSlot here because we're about to overwrite the slot data anyway

			SendAll( m_Protocol->SEND_W3GS_PLAYERLEAVE_OTHERS( FurthestPlayer->GetPID( ), FurthestPlayer->GetLeftCode( ) ) );
			FurthestPlayer->SetLeftMessageSent( true );
			FurthestPlayer->deleteLater();

			if( FurthestPlayer->GetScore( ) < -99999.0 )
				SendAllChat( m_GHost->GetLanguage( )->PlayerWasKickedForFurthestScore( FurthestPlayer->GetName( ), "N/A", QString::number( AverageScore, 'g', 2 ) ) );
			else
				SendAllChat( m_GHost->GetLanguage( )->PlayerWasKickedForFurthestScore( FurthestPlayer->GetName( ), QString::number( FurthestPlayer->GetScore( ), 'g', 2 ), QString::number( AverageScore, 'g', 2 ) ) );
		}
		else if( m_GHost->m_MatchMakingMethod == 2 )
		{
			// method 2: lowest score method
			// kick the player with the lowest score (or a player without a score)

			CGamePlayer *LowestPlayer = NULL;

			for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
			{
				if( !LowestPlayer || (*i)->GetScore( ) < -99999.0 || (*i)->GetScore( ) < LowestPlayer->GetScore( ) )
					LowestPlayer = *i;
			}

			if( !LowestPlayer )
			{
				// this should be impossible

				CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] is trying to join the game but no lowest player was found (this should be impossible)" );
				potential->Send( m_Protocol->SEND_W3GS_REJECTJOIN( REJECTJOIN_FULL ) );
				potential->deleteLater();
				return;
			}

			// kick the new player if they have the lowest score

			if( score < -99999.0 || score < LowestPlayer->GetScore( ) )
			{
				if( score < -99999.0 )
					CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] is trying to join the game but has the lowest rating [N/A]" );
				else
					CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] is trying to join the game but has the lowest rating [" + QString::number( score, 'g', 2 ) + "]" );

				potential->Send( m_Protocol->SEND_W3GS_REJECTJOIN( REJECTJOIN_FULL ) );
				potential->deleteLater();
				return;
			}

			// kick the lowest player

			SID = GetSIDFromPID( LowestPlayer->GetPID( ) );

			if( LowestPlayer->GetScore( ) < -99999.0 )
				LowestPlayer->SetLeftReason( m_GHost->GetLanguage( )->WasKickedForHavingLowestScore( "N/A" ) );
			else
				LowestPlayer->SetLeftReason( m_GHost->GetLanguage( )->WasKickedForHavingLowestScore( QString::number(  LowestPlayer->GetScore( ), 'g', 2 ) ) );

			LowestPlayer->SetLeftCode( PLAYERLEAVE_LOBBY );

			// send a playerleave message immediately since it won't normally get sent until the player is deleted which is after we send a playerjoin message
			// we don't need to call OpenSlot here because we're about to overwrite the slot data anyway

			SendAll( m_Protocol->SEND_W3GS_PLAYERLEAVE_OTHERS( LowestPlayer->GetPID( ), LowestPlayer->GetLeftCode( ) ) );
			LowestPlayer->SetLeftMessageSent( true );
			LowestPlayer->deleteLater();

			if( LowestPlayer->GetScore( ) < -99999.0 )
				SendAllChat( m_GHost->GetLanguage( )->PlayerWasKickedForLowestScore( LowestPlayer->GetName( ), "N/A" ) );
			else
				SendAllChat( m_GHost->GetLanguage( )->PlayerWasKickedForLowestScore( LowestPlayer->GetName( ), QString::number( LowestPlayer->GetScore( ), 'g', 2 ) ) );
		}
	}

	if( SID >= m_Slots.size( ) )
	{
		potential->Send( m_Protocol->SEND_W3GS_REJECTJOIN( REJECTJOIN_FULL ) );
		potential->deleteLater();
		return;
	}

	// we have a slot for the new player
	// make room for them by deleting the virtual host player if we have to

	if( GetNumPlayers( ) >= 11 )
		DeleteVirtualHost( );

	// identify their joined realm
	// this is only possible because when we send a game refresh via LAN or battle.net we encode an ID value in the 4 most significant bits of the host counter
	// the client sends the host counter when it joins so we can extract the ID value here
	// note: this is not a replacement for spoof checking since it doesn't verify the player's name and it can be spoofed anyway

	quint32 HostCounterID = joinPlayer->GetHostCounter( ) >> 28;
	QString JoinedRealm;

	// we use an ID value of 0 to denote joining via LAN

	if( HostCounterID != 0 )
	{
		for( QList<CBNET *> :: const_iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
		{
			if( (*i)->GetHostCounterID( ) == HostCounterID )
				JoinedRealm = (*i)->GetServer( );
		}
	}

	// turning the CPotentialPlayer into a CGamePlayer is a bit of a pain because we have to be careful not to close the socket
	// this problem is solved by setting the socket to NULL before deletion and handling the NULL case in the destructor
	// we also have to be careful to not modify the m_Potentials vector since we're currently looping through it

	CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + joinPlayer->GetName( ) + "|" + potential->GetExternalIPString( ) + "] joined the game" );
	CGamePlayer *Player = new CGamePlayer( potential, GetNewPID( ), JoinedRealm, joinPlayer->GetName( ), joinPlayer->GetInternalIP( ), false );

	// consider LAN players to have already spoof checked since they can't
	// since so many people have trouble with this feature we now use the JoinedRealm to determine LAN status

	if( JoinedRealm.isEmpty( ) )
		Player->SetSpoofed( true );

	Player->SetWhoisShouldBeSent( m_GHost->m_SpoofChecks == 1 || ( m_GHost->m_SpoofChecks == 2 && AnyAdminCheck ) );
	Player->SetScore( score );
	m_Players.push_back( Player );
	potential->SetSocket( NULL );
	potential->deleteLater();
	m_Slots[SID] = CGameSlot( Player->GetPID( ), 255, SLOTSTATUS_OCCUPIED, 0, m_Slots[SID].GetTeam( ), m_Slots[SID].GetColour( ), m_Slots[SID].GetRace( ) );

	// send slot info to the new player
	// the SLOTINFOJOIN packet also tells the client their assigned PID and that the join was successful

	Player->Send( m_Protocol->SEND_W3GS_SLOTINFOJOIN( Player->GetPID( ), Util::fromUInt16((quint16)Player->GetSocket( )->localPort()), Player->GetExternalIP( ), m_Slots, m_RandomSeed, m_Map->GetMapLayoutStyle( ), m_Map->GetMapNumPlayers( ) ) );

	// send virtual host info and fake player info (if present) to the new player

	SendVirtualHostPlayerInfo( Player );
	SendFakePlayerInfo( Player );

	QByteArray BlankIP;
	BlankIP.push_back( (char)0 );
	BlankIP.push_back( (char)0 );
	BlankIP.push_back( (char)0 );
	BlankIP.push_back( (char)0 );

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( !(*i)->GetLeftMessageSent( ) && *i != Player )
		{
			// send info about the new player to every other player

			if( (*i)->GetSocket( ) )
			{
				if( m_GHost->m_HideIPAddresses )
					(*i)->Send( m_Protocol->SEND_W3GS_PLAYERINFO( Player->GetPID( ), Player->GetName( ), BlankIP, BlankIP ) );
				else
					(*i)->Send( m_Protocol->SEND_W3GS_PLAYERINFO( Player->GetPID( ), Player->GetName( ), Player->GetExternalIP( ), Player->GetInternalIP( ) ) );
			}

			// send info about every other player to the new player

			if( m_GHost->m_HideIPAddresses )
				Player->Send( m_Protocol->SEND_W3GS_PLAYERINFO( (*i)->GetPID( ), (*i)->GetName( ), BlankIP, BlankIP ) );
			else
				Player->Send( m_Protocol->SEND_W3GS_PLAYERINFO( (*i)->GetPID( ), (*i)->GetName( ), (*i)->GetExternalIP( ), (*i)->GetInternalIP( ) ) );
		}
	}

	// send a map check packet to the new player

	Player->Send( m_Protocol->SEND_W3GS_MAPCHECK( m_Map->GetMapPath( ), m_Map->GetMapSize( ), m_Map->GetMapInfo( ), m_Map->GetMapCRC( ), m_Map->GetMapSHA1( ) ) );

	// send slot info to everyone, so the new player gets this info twice but everyone else still needs to know the new slot layout

	SendAllSlotInfo( );

	// send a welcome message

	SendWelcomeMessage( Player );

	// if spoof checks are required and we won't automatically spoof check this player then tell them how to spoof check
	// e.g. if automatic spoof checks are disabled, or if automatic spoof checks are done on admins only and this player isn't an admin

	if( m_GHost->m_RequireSpoofChecks && !Player->GetWhoisShouldBeSent( ) )
	{
		for( QList<CBNET *> :: const_iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
		{
			// note: the following (commented out) line of code will crash because calling GetUniqueName( ) twice will result in two different return values
			// and unfortunately iterators are not valid if compared against different containers
			// this comment shall serve as warning to not make this mistake again since it has now been made twice before in GHost++
			// QString( (*i)->GetUniqueName( ).begin( ), (*i)->GetUniqueName( ).end( ) )

			QByteArray UniqueName = (*i)->GetUniqueName( );

			if( (*i)->GetServer( ) == JoinedRealm )
				SendChat( Player, m_GHost->GetLanguage( )->SpoofCheckByWhispering( UniqueName.end( )) );
		}
	}

	if( score < -99999.0 )
		SendAllChat( m_GHost->GetLanguage( )->PlayerHasScore( joinPlayer->GetName( ), "N/A" ) );
	else
		SendAllChat( m_GHost->GetLanguage( )->PlayerHasScore( joinPlayer->GetName( ), QString::number( score, 'g', 2 ) ) );

	quint32 PlayersScored = 0;
	quint32 PlayersNotScored = 0;
	double AverageScore = 0.0;
	double MinScore = 0.0;
	double MaxScore = 0.0;
	bool Found = false;

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( !(*i)->GetLeftMessageSent( ) )
		{
			if( (*i)->GetScore( ) < -99999.0 )
				PlayersNotScored++;
			else
			{
				PlayersScored++;
				AverageScore += (*i)->GetScore( );

				if( !Found || (*i)->GetScore( ) < MinScore )
					MinScore = (*i)->GetScore( );

				if( !Found || (*i)->GetScore( ) > MaxScore )
					MaxScore = (*i)->GetScore( );

				Found = true;
			}
		}
	}

	double Spread = MaxScore - MinScore;
	SendAllChat( m_GHost->GetLanguage( )->RatedPlayersSpread( QString::number( PlayersScored ), QString::number( PlayersScored + PlayersNotScored ), QString::number( (quint32)Spread ) ) );

	// check for multiple IP usage

	if( m_GHost->m_CheckMultipleIPUsage )
	{
		QString Others;

		for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			if( Player != *i && Player->GetExternalIPString( ) == (*i)->GetExternalIPString( ) )
			{
				if( Others.isEmpty( ) )
					Others = (*i)->GetName( );
				else
					Others += ", " + (*i)->GetName( );
			}
		}

		if( !Others.isEmpty( ) )
			SendAllChat( m_GHost->GetLanguage( )->MultipleIPAddressUsageDetected( joinPlayer->GetName( ), Others ) );
	}

	// abort the countdown if there was one in progress

	if( m_CountDownStarted && !m_GameLoading && !m_GameLoaded )
	{
		SendAllChat( m_GHost->GetLanguage( )->CountDownAborted( ) );
		m_CountDownStarted = false;
	}

	// auto lock the game

	if( m_GHost->m_AutoLock && !m_Locked && IsOwner( joinPlayer->GetName( ) ) )
	{
		SendAllChat( m_GHost->GetLanguage( )->GameLocked( ) );
		m_Locked = true;
	}

	// balance the slots

	if( m_AutoStartPlayers != 0 && GetNumHumanPlayers( ) == m_AutoStartPlayers )
		BalanceSlots( );
}

void CBaseGame :: EventPlayerLeft( CGamePlayer *player, quint32 reason )
{
	// this function is only called when a player leave packet is received, not when there's a socket error, kick, etc...
	DEBUG_Print("EventPlayerLeft");


	if( reason == PLAYERLEAVE_GPROXY )
		player->SetLeftReason( m_GHost->GetLanguage( )->WasUnrecoverablyDroppedFromGProxy( ) );
	else
		player->SetLeftReason( m_GHost->GetLanguage( )->HasLeftVoluntarily( ) );

	player->SetLeftCode( PLAYERLEAVE_LOST );
	player->deleteLater();

	if( !m_GameLoading && !m_GameLoaded )
		OpenSlot( GetSIDFromPID( player->GetPID( ) ), false );
}

void CBaseGame :: EventPlayerLoaded()
{
	CGamePlayer *player = (CGamePlayer*)QObject::sender();
	CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + player->GetName( ) + "] finished loading in " + QString::number( (float)( player->GetFinishedLoadingTicks( ) - m_StartedLoadingTicks ) / 1000, 'g' ) + " seconds" );

	if( m_LoadInGame )
	{
		// send any buffered data to the player now
		// see the Update function for more information about why we do this
		// this includes player loaded messages, game updates, and player leave messages

		QQueue<QByteArray> *LoadInGameData = player->GetLoadInGameData( );

		while( !LoadInGameData->empty( ) )
			Send( player, LoadInGameData->dequeue( ) );

		// start the lag screen for the new player

		bool FinishedLoading = true;

		for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			FinishedLoading = (*i)->GetFinishedLoading( );

			if( !FinishedLoading )
				break;
		}

		if( !FinishedLoading )
			Send( player, m_Protocol->SEND_W3GS_START_LAG( m_Players, true ) );

		// remove the new player from previously loaded players' lag screens

		for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			if( *i != player && (*i)->GetFinishedLoading( ) )
				Send( *i, m_Protocol->SEND_W3GS_STOP_LAG( player ) );
		}

		// send a chat message to previously loaded players

		for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			if( *i != player && (*i)->GetFinishedLoading( ) )
				SendChat( *i, m_GHost->GetLanguage( )->PlayerFinishedLoading( player->GetName( ) ) );
		}

		if( !FinishedLoading )
			SendChat( player, m_GHost->GetLanguage( )->PleaseWaitPlayersStillLoading( ) );
	}
	else
		SendAll( m_Protocol->SEND_W3GS_GAMELOADED_OTHERS( player->GetPID( ) ) );

	CheckGameLoaded();
}

void CBaseGame :: EventPlayerAction( CGamePlayer *player, CIncomingAction *action )
{
	m_Actions.enqueue( action );

	// check for players saving the game and notify everyone

	if( !action->GetAction( ).isEmpty( ) && action->GetAction( ).at(0) == 6 )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + player->GetName( ) + "] is saving the game" );
		SendAllChat( m_GHost->GetLanguage( )->PlayerIsSavingTheGame( player->GetName( ) ) );
	}
}

void CBaseGame :: EventPlayerKeepAlive( CGamePlayer */*player*/, quint32 /*checkSum*/ )
{
	// check for desyncs
	// however, it's possible that not every player has sent a checksum for this frame yet
	// first we verify that we have enough checksums to work with otherwise we won't know exactly who desynced

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( (*i)->GetCheckSums( )->empty( ) )
			return;
	}

	// now we check for desyncs since we know that every player has at least one checksum waiting

	bool FoundPlayer = false;
	quint32 FirstCheckSum = 0;

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		FoundPlayer = true;
		FirstCheckSum = (*i)->GetCheckSums( )->front( );
		break;
	}

	if( !FoundPlayer )
		return;

	bool AddToReplay = true;

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( (*i)->GetCheckSums( )->front( ) != FirstCheckSum )
		{
			CONSOLE_Print( "[GAME: " + m_GameName + "] desync detected" );
			SendAllChat( m_GHost->GetLanguage( )->DesyncDetected( ) );

			// try to figure out who desynced
			// this is complicated by the fact that we don't know what the correct game state is so we let the players vote
			// put the players into bins based on their game state

			QMap<quint32, QList<unsigned char> > Bins;

			for( QList<CGamePlayer *> :: iterator j = m_Players.begin( ); j != m_Players.end( ); j++ )
				Bins[(*j)->GetCheckSums( )->front( )].push_back( (*j)->GetPID( ) );

			quint32 StateNumber = 1;
			QMap<quint32, QList<unsigned char> > :: iterator LargestBin = Bins.begin( );
			bool Tied = false;

			for( QMap<quint32, QList<unsigned char> > :: iterator j = Bins.begin( ); j != Bins.end( ); j++ )
			{
				if( j.value().size( ) > LargestBin.value().size( ) )
				{
					LargestBin = j;
					Tied = false;
				}
				else if( j != LargestBin && j.value().size( ) == LargestBin.value().size( ) )
					Tied = true;

				QString Players;

				for( QList<unsigned char> :: iterator k = j.value().begin( ); k != j.value().end( ); k++ )
				{
					CGamePlayer *Player = GetPlayerFromPID( *k );

					if( Player )
					{
						if( Players.isEmpty( ) )
							Players = Player->GetName( );
						else
							Players += ", " + Player->GetName( );
					}
				}

				SendAllChat( m_GHost->GetLanguage( )->PlayersInGameState( QString::number( StateNumber ), Players ) );
				StateNumber++;
			}

			FirstCheckSum = LargestBin.key();

			if( Tied )
			{
				// there is a tie, which is unfortunate
				// the most common way for this to happen is with a desync in a 1v1 situation
				// this is not really unsolvable since the game shouldn't continue anyway so we just kick both players
				// in a 2v2 or higher the chance of this happening is very slim
				// however, we still kick every player because it's not fair to pick one or another group
				// todotodo: it would be possible to split the game at this point and create a "new" game for each game state

				CONSOLE_Print( "[GAME: " + m_GameName + "] can't kick desynced players because there is a tie, kicking all players instead" );
				StopPlayers( m_GHost->GetLanguage( )->WasDroppedDesync( ) );
				AddToReplay = false;
			}
			else
			{
				CONSOLE_Print( "[GAME: " + m_GameName + "] kicking desynced players" );

				for( QMap<quint32, QList<unsigned char> > :: iterator j = Bins.begin( ); j != Bins.end( ); j++ )
				{
					// kick players who are NOT in the largest bin
					// examples: suppose there are 10 players
					// the most common case will be 9v1 (e.g. one player desynced and the others were unaffected) and this will kick the single outlier
					// another (very unlikely) possibility is 8v1v1 or 8v2 and this will kick both of the outliers, regardless of whether their game states match

					if( j.key() != LargestBin.key() )
					{
						for( QList<unsigned char> :: iterator k = j.value().begin( ); k != j.value().end( ); k++ )
						{
							CGamePlayer *Player = GetPlayerFromPID( *k );

							if( Player )
							{
								Player->SetLeftReason( m_GHost->GetLanguage( )->WasDroppedDesync( ) );
								Player->SetLeftCode( PLAYERLEAVE_LOST );
								Player->deleteLater();
							}
						}
					}
				}
			}

			// don't continue looking for desyncs, we already found one!

			break;
		}
	}

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		(*i)->GetCheckSums( )->dequeue( );

	// add checksum to replay

	/* if( m_Replay && AddToReplay )
		m_Replay->AddCheckSum( FirstCheckSum ); */
}

void CBaseGame :: EventPlayerChatToHost( CGamePlayer *player, CIncomingChatPlayer *chatPlayer )
{
	if( chatPlayer->GetFromPID( ) == player->GetPID( ) )
	{
		if( chatPlayer->GetType( ) == CIncomingChatPlayer :: CTH_MESSAGE || chatPlayer->GetType( ) == CIncomingChatPlayer :: CTH_MESSAGEEXTRA )
		{
			// relay the chat message to other players

			bool Relay = !player->GetMuted( );
			QByteArray ExtraFlags = chatPlayer->GetExtraFlags( );

			// calculate timestamp

			QString MinString = QString::number( ( m_GameTicks / 1000 ) / 60 );
			QString SecString = QString::number( ( m_GameTicks / 1000 ) % 60 );

			if( MinString.size( ) == 1 )
				MinString.insert( 0, "0" );

			if( SecString.size( ) == 1 )
				SecString.insert( 0, "0" );

			if( !ExtraFlags.isEmpty( ) )
			{
				if( ExtraFlags.at(0) == 0 )
				{
					// this is an ingame [All] message, print it to the console

					CONSOLE_Print( "[GAME: " + m_GameName + "] (" + MinString + ":" + SecString + ") [All] [" + player->GetName( ) + "]: " + chatPlayer->GetMessage( ) );

					// don't relay ingame messages targeted for all players if we're currently muting all
					// note that commands will still be processed even when muting all because we only stop relaying the messages, the rest of the function is unaffected

					if( m_MuteAll )
						Relay = false;
				}
				else if( ExtraFlags.at(0) == 2 )
				{
					// this is an ingame [Obs/Ref] message, print it to the console

					CONSOLE_Print( "[GAME: " + m_GameName + "] (" + MinString + ":" + SecString + ") [Obs/Ref] [" + player->GetName( ) + "]: " + chatPlayer->GetMessage( ) );
				}

				if( Relay )
				{
					// add chat message to replay
					// this includes allied chat and private chat from both teams as long as it was relayed

					if( m_Replay )
						m_Replay->AddChatMessage( chatPlayer->GetFromPID( ), chatPlayer->GetFlag( ), Util::extractUInt32(chatPlayer->GetExtraFlags( )), chatPlayer->GetMessage( ) );
				}
			}
			else
			{
				// this is a lobby message, print it to the console

				CONSOLE_Print( "[GAME: " + m_GameName + "] [Lobby] [" + player->GetName( ) + "]: " + chatPlayer->GetMessage( ) );

				if( m_MuteLobby )
					Relay = false;
			}

			// handle bot commands

			QString Message = chatPlayer->GetMessage( );

			if( Message == "?trigger" )
				SendChat( player, m_GHost->GetLanguage( )->CommandTrigger( QString( 1, m_GHost->m_CommandTrigger ) ) );
			else if( !Message.isEmpty( ) && Message[0] == m_GHost->m_CommandTrigger )
			{
				// extract the command trigger, the command, and the payload
				// e.g. "!say hello world" -> command: "say", payload: "hello world"

				QString Command;
				QString Payload;
				int PayloadStart = Message.indexOf( ' ' );

				if( PayloadStart != -1 )
				{
					Command = Message.mid( 1, PayloadStart - 1 );
					Payload = Message.mid( PayloadStart + 1 );
				}
				else
					Command = Message.mid( 1 );

				Command = Command.toLower();

				// don't allow EventPlayerBotCommand to veto a previous instruction to set Relay to false
				// so if Relay is already false (e.g. because the player is muted) then it cannot be forced back to true here

				if( EventPlayerBotCommand( player, Command, Payload ) )
					Relay = false;
			}

			if( Relay )
				Send( chatPlayer->GetToPIDs( ), m_Protocol->SEND_W3GS_CHAT_FROM_HOST( chatPlayer->GetFromPID( ), chatPlayer->GetToPIDs( ), chatPlayer->GetFlag( ), chatPlayer->GetExtraFlags( ), chatPlayer->GetMessage( ) ) );
		}
		else if( chatPlayer->GetType( ) == CIncomingChatPlayer :: CTH_TEAMCHANGE && !m_CountDownStarted )
			EventPlayerChangeTeam( player, chatPlayer->GetByte( ) );
		else if( chatPlayer->GetType( ) == CIncomingChatPlayer :: CTH_COLOURCHANGE && !m_CountDownStarted )
			EventPlayerChangeColour( player, chatPlayer->GetByte( ) );
		else if( chatPlayer->GetType( ) == CIncomingChatPlayer :: CTH_RACECHANGE && !m_CountDownStarted )
			EventPlayerChangeRace( player, chatPlayer->GetByte( ) );
		else if( chatPlayer->GetType( ) == CIncomingChatPlayer :: CTH_HANDICAPCHANGE && !m_CountDownStarted )
			EventPlayerChangeHandicap( player, chatPlayer->GetByte( ) );
	}
}

bool CBaseGame :: EventPlayerBotCommand( CGamePlayer *player, const QString &command, const QString &payload )
{
	// return true if the command itself should be hidden from other players
	emit SignalPlayerCommand( this, player, command, payload );
	return false;
}

void CBaseGame :: EventPlayerChangeTeam( CGamePlayer *player, unsigned char team )
{
	// player is requesting a team change

	if( m_SaveGame )
		return;

	if( m_Map->GetMapOptions( ) & MAPOPT_FIXEDPLAYERSETTINGS )
	{
		unsigned char oldSID = GetSIDFromPID( player->GetPID( ) );
		unsigned char newSID = GetEmptySlot( team, player->GetPID( ) );
		SwapSlots( oldSID, newSID );
	}
	else
	{
		if( team > 12 )
			return;

		if( team == 12 )
		{
			if( m_Map->GetMapObservers( ) != MAPOBS_ALLOWED && m_Map->GetMapObservers( ) != MAPOBS_REFEREES )
				return;
		}
		else
		{
			if( team >= m_Map->GetMapNumPlayers( ) )
				return;

			// make sure there aren't too many other players already

			unsigned char NumOtherPlayers = 0;

			for( unsigned char i = 0; i < m_Slots.size( ); i++ )
			{
				if( m_Slots[i].GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && m_Slots[i].GetTeam( ) != 12 && m_Slots[i].GetPID( ) != player->GetPID( ) )
					NumOtherPlayers++;
			}

			if( NumOtherPlayers >= m_Map->GetMapNumPlayers( ) )
				return;
		}

		unsigned char SID = GetSIDFromPID( player->GetPID( ) );

		if( SID < m_Slots.size( ) )
		{
			m_Slots[SID].SetTeam( team );

			if( team == 12 )
			{
				// if they're joining the observer team give them the observer colour

				m_Slots[SID].SetColour( 12 );
			}
			else if( m_Slots[SID].GetColour( ) == 12 )
			{
				// if they're joining a regular team give them an unused colour

				m_Slots[SID].SetColour( GetNewColour( ) );
			}

			SendAllSlotInfo( );
		}
	}
}

void CBaseGame :: EventPlayerChangeColour( CGamePlayer *player, unsigned char colour )
{
	// player is requesting a colour change

	if( m_SaveGame )
		return;

	if( m_Map->GetMapOptions( ) & MAPOPT_FIXEDPLAYERSETTINGS )
		return;

	if( colour > 11 )
		return;

	unsigned char SID = GetSIDFromPID( player->GetPID( ) );

	if( SID < m_Slots.size( ) )
	{
		// make sure the player isn't an observer

		if( m_Slots[SID].GetTeam( ) == 12 )
			return;

		ColourSlot( SID, colour );
	}
}

void CBaseGame :: EventPlayerChangeRace( CGamePlayer *player, unsigned char race )
{
	// player is requesting a race change

	if( m_SaveGame )
		return;

	if( m_Map->GetMapOptions( ) & MAPOPT_FIXEDPLAYERSETTINGS )
		return;

	if( m_Map->GetMapFlags( ) & MAPFLAG_RANDOMRACES )
		return;

	if( race != SLOTRACE_HUMAN && race != SLOTRACE_ORC && race != SLOTRACE_NIGHTELF && race != SLOTRACE_UNDEAD && race != SLOTRACE_RANDOM )
		return;

	unsigned char SID = GetSIDFromPID( player->GetPID( ) );

	if( SID < m_Slots.size( ) )
	{
		m_Slots[SID].SetRace( race | SLOTRACE_SELECTABLE );
		SendAllSlotInfo( );
	}
}

void CBaseGame :: EventPlayerChangeHandicap( CGamePlayer *player, unsigned char handicap )
{
	// player is requesting a handicap change

	if( m_SaveGame )
		return;

	if( m_Map->GetMapOptions( ) & MAPOPT_FIXEDPLAYERSETTINGS )
		return;

	if( handicap != 50 && handicap != 60 && handicap != 70 && handicap != 80 && handicap != 90 && handicap != 100 )
		return;

	unsigned char SID = GetSIDFromPID( player->GetPID( ) );

	if( SID < m_Slots.size( ) )
	{
		m_Slots[SID].SetHandicap( handicap );
		SendAllSlotInfo( );
	}
}

void CBaseGame :: EventPlayerDropRequest( CGamePlayer *player )
{
	// todotodo: check that we've waited the full 45 seconds

	if( m_Lagging )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] player [" + player->GetName( ) + "] voted to drop laggers" );
		SendAllChat( m_GHost->GetLanguage( )->PlayerVotedToDropLaggers( player->GetName( ) ) );

		// check if at least half the players voted to drop

		quint32 Votes = 0;

		for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			if( (*i)->GetDropVote( ) )
				Votes++;
		}

		if( (float)Votes / m_Players.size( ) > 0.49 )
			StopLaggers( m_GHost->GetLanguage( )->LaggedOutDroppedByVote( ) );
	}
}

void CBaseGame :: EventPlayerMapSize( CGamePlayer *player, CIncomingMapSize *mapSize )
{
	if( m_GameLoading || m_GameLoaded )
		return;

	// todotodo: the variable names here are confusing due to extremely poor design on my part

	quint32 MapSize = Util::extractUInt32(m_Map->GetMapSize( ));

	if( mapSize->GetSizeFlag( ) != 1 || mapSize->GetMapSize( ) != MapSize )
	{
		// the player doesn't have the map

		if( m_GHost->m_AllowDownloads != 0 )
		{
			const QByteArray &MapData = m_Map->GetMapData( );

			if( !MapData.isEmpty( ) )
			{
				if( m_GHost->m_AllowDownloads == 1 || ( m_GHost->m_AllowDownloads == 2 && player->GetDownloadAllowed( ) ) )
				{
					if( !player->GetDownloadStarted( ) && mapSize->GetSizeFlag( ) == 1 )
					{
						// inform the client that we are willing to send the map

						CONSOLE_Print( "[GAME: " + m_GameName + "] map download started for player [" + player->GetName( ) + "]" );
						Send( player, m_Protocol->SEND_W3GS_STARTDOWNLOAD( GetHostPID( ) ) );
						player->SetDownloadStarted( true );
						player->SetStartedDownloadingTicks( GetTicks( ) );
					}
					else
						player->SetLastMapPartAcked( mapSize->GetMapSize( ) );
				}
			}
			else
			{
				player->SetLeftReason( "doesn't have the map and there is no local copy of the map to send" );
				player->SetLeftCode( PLAYERLEAVE_LOBBY );
				player->deleteLater();
				OpenSlot( GetSIDFromPID( player->GetPID( ) ), false );
			}
		}
		else
		{
			player->SetLeftReason( "doesn't have the map and map downloads are disabled" );
			player->SetLeftCode( PLAYERLEAVE_LOBBY );
			player->deleteLater();
			OpenSlot( GetSIDFromPID( player->GetPID( ) ), false );
		}
	}
	else
	{
		if( player->GetDownloadStarted( ) )
		{
			// calculate download rate

			float Seconds = (float)( GetTicks( ) - player->GetStartedDownloadingTicks( ) ) / 1000;
			float Rate = (float)MapSize / 1024 / Seconds;
			CONSOLE_Print( "[GAME: " + m_GameName + "] map download finished for player [" + player->GetName( ) + "] in " + QString::number( Seconds, 'g', 1 ) + " seconds" );
			SendAllChat( m_GHost->GetLanguage( )->PlayerDownloadedTheMap( player->GetName( ), QString::number( Seconds, 'g', 1 ), QString::number( Rate, 'g', 1 ) ) );
			player->SetDownloadFinished( true );
			player->SetFinishedDownloadingTime( GetTime( ) );

			// add to database

			m_GHost->m_Callables.push_back( m_GHost->m_DB->ThreadedDownloadAdd( m_Map->GetMapPath( ), MapSize, player->GetName( ), player->GetExternalIPString( ), player->GetSpoofed( ) ? 1 : 0, player->GetSpoofedRealm( ), GetTicks( ) - player->GetStartedDownloadingTicks( ) ) );
		}
	}

	unsigned char NewDownloadStatus = (unsigned char)( (float)mapSize->GetMapSize( ) / MapSize * 100 );
	unsigned char SID = GetSIDFromPID( player->GetPID( ) );

	if( NewDownloadStatus > 100 )
		NewDownloadStatus = 100;

	if( SID < m_Slots.size( ) )
	{
		// only send the slot info if the download status changed

		if( m_Slots[SID].GetDownloadStatus( ) != NewDownloadStatus )
		{
			m_Slots[SID].SetDownloadStatus( NewDownloadStatus );

			// we don't actually send the new slot info here
			// this is an optimization because it's possible for a player to download a map very quickly
			// if we send a new slot update for every percentage change in their download status it adds up to a lot of data
			// instead, we mark the slot info as "out of date" and update it only once in awhile (once per second when this comment was made)

			if (!m_SlotInfoTimer.isActive())
				m_SlotInfoTimer.start();
		}
	}
}

void CBaseGame::EventCallableUpdateTimeout()
{
	// update callables

	for( QList<CCallableScoreCheck *> :: iterator i = m_ScoreChecks.begin( ); i != m_ScoreChecks.end( ); )
	{
		if( (*i)->GetReady( ) )
		{
			double Score = (*i)->GetResult( );

			for( QList<CPotentialPlayer *> :: iterator j = m_Potentials.begin( ); j != m_Potentials.end( ); j++ )
			{
				if( (*j)->GetJoinPlayer( ) && (*j)->GetJoinPlayer( )->GetName( ) == (*i)->GetName( ) )
					EventPlayerJoinedWithScore( *j, (*j)->GetJoinPlayer( ), Score );
			}

			m_GHost->m_DB->RecoverCallable( *i );
			delete *i;
			i = m_ScoreChecks.erase( i );
		}
		else
			i++;
	}
}

void CBaseGame :: EventPlayerPongToHost( CGamePlayer *player, quint32 /*pong*/ )
{
	// autokick players with excessive pings but only if they're not reserved and we've received at least 3 pings from them
	// also don't kick anyone if the game is loading or loaded - this could happen because we send pings during loading but we stop sending them after the game is loaded
	// see the Update function for where we send pings

	if( !m_GameLoading && !m_GameLoaded && !player->GetReserved( ) && player->GetNumPings( ) >= 3 && player->GetPing( m_GHost->m_LCPings ) > m_GHost->m_AutoKickPing )
	{
		// send a chat message because we don't normally do so when a player leaves the lobby

		SendAllChat( m_GHost->GetLanguage( )->AutokickingPlayerForExcessivePing( player->GetName( ), QString::number( player->GetPing( m_GHost->m_LCPings ) ) ) );
		player->SetLeftReason( "was autokicked for excessive ping of " + QString::number( player->GetPing( m_GHost->m_LCPings ) ) );
		player->SetLeftCode( PLAYERLEAVE_LOBBY );
		player->deleteLater();
		OpenSlot( GetSIDFromPID( player->GetPID( ) ), false );
	}
}

void CBaseGame :: EventGameRefreshed( const QString &/*server*/ )
{
	if( m_RefreshRehosted )
	{
		// we're not actually guaranteed this refresh was for the rehosted game and not the previous one
		// but since we unqueue game refreshes when rehosting, the only way this can happen is due to network delay
		// it's a risk we're willing to take but can result in a false positive here

		SendAllChat( m_GHost->GetLanguage( )->RehostWasSuccessful( ) );
		m_RefreshRehosted = false;
	}
}

void CBaseGame :: EventGameStarted( )
{
	CONSOLE_Print( "[GAME: " + m_GameName + "] started loading with " + QString::number( GetNumHumanPlayers( ) ) + " players" );

	// encode the HCL command QString in the slot handicaps
	// here's how it works:
	//  the user inputs a command QString to be sent to the map
	//  it is almost impossible to send a message from the bot to the map so we encode the command QString in the slot handicaps
	//  this works because there are only 6 valid handicaps but Warcraft III allows the bot to set up to 256 handicaps
	//  we encode the original (unmodified) handicaps in the new handicaps and use the remaining space to store a short message
	//  only occupied slots deliver their handicaps to the map and we can send one character (from a list) per handicap
	//  when the map finishes loading, assuming it's designed to use the HCL system, it checks if anyone has an invalid handicap
	//  if so, it decodes the message from the handicaps and restores the original handicaps using the encoded values
	//  the meaning of the message is specific to each map and the bot doesn't need to understand it
	//  e.g. you could send game modes, # of rounds, level to start on, anything you want as long as it fits in the limited space available
	//  note: if you attempt to use the HCL system on a map that does not support HCL the bot will drastically modify the handicaps
	//  since the map won't automatically restore the original handicaps in this case your game will be ruined

	if( !m_HCLCommandString.isEmpty( ) )
	{
		if( m_HCLCommandString.size( ) <= (int)GetSlotsOccupied( ) )
		{
			QString HCLChars = "abcdefghijklmnopqrstuvwxyz0123456789 -=,.";

			if( m_HCLCommandString.indexOf( QRegExp( "[^" + HCLChars + "]" ) ) == -1 )
			{
				unsigned char EncodingMap[256];
				unsigned char j = 0;

				for( quint32 i = 0; i < 256; i++ )
				{
					// the following 7 handicap values are forbidden

					if( j == 0 || j == 50 || j == 60 || j == 70 || j == 80 || j == 90 || j == 100 )
						j++;

					EncodingMap[i] = j++;
				}

				unsigned char CurrentSlot = 0;

				for( QString :: iterator si = m_HCLCommandString.begin( ); si != m_HCLCommandString.end( ); si++ )
				{
					while( m_Slots[CurrentSlot].GetSlotStatus( ) != SLOTSTATUS_OCCUPIED )
						CurrentSlot++;

					unsigned char HandicapIndex = ( m_Slots[CurrentSlot].GetHandicap( ) - 50 ) / 10;
					unsigned char CharIndex = HCLChars.indexOf( *si );
					m_Slots[CurrentSlot++].SetHandicap( EncodingMap[HandicapIndex + CharIndex * 6] );
				}

				SendAllSlotInfo( );
				CONSOLE_Print( "[GAME: " + m_GameName + "] successfully encoded HCL command QString [" + m_HCLCommandString + "]" );
			}
			else
				CONSOLE_Print( "[GAME: " + m_GameName + "] encoding HCL command QString [" + m_HCLCommandString + "] failed because it contains invalid characters" );
		}
		else
			CONSOLE_Print( "[GAME: " + m_GameName + "] encoding HCL command QString [" + m_HCLCommandString + "] failed because there aren't enough occupied slots" );
	}

	// send a final slot info update if necessary
	// this typically won't happen because we prevent the !start command from completing while someone is downloading the map
	// however, if someone uses !start force while a player is downloading the map this could trigger
	// this is because we only permit slot info updates to be flagged when it's just a change in download status, all others are sent immediately
	// it might not be necessary but let's clean up the mess anyway

	//if( m_SlotInfoChanged )
		SendAllSlotInfo( );

	m_StartedLoadingTicks = GetTicks( );
	m_GameLoading = true;

	// since we use a fake countdown to deal with leavers during countdown the COUNTDOWN_START and COUNTDOWN_END packets are sent in quick succession
	// send a start countdown packet

	SendAll( m_Protocol->SEND_W3GS_COUNTDOWN_START( ) );

	// remove the virtual host player

	DeleteVirtualHost( );

	// send an end countdown packet

	SendAll( m_Protocol->SEND_W3GS_COUNTDOWN_END( ) );

	// send a game loaded packet for the fake player (if present)

	if( m_FakePlayerPID != 255 )
		SendAll( m_Protocol->SEND_W3GS_GAMELOADED_OTHERS( m_FakePlayerPID ) );

	// record the starting number of players

	m_StartPlayers = GetNumHumanPlayers( );

	// close the listening socket

	delete m_Socket;
	m_Socket = NULL;

	// delete any potential players that are still hanging around

	for( QList<CPotentialPlayer *> :: const_iterator i = m_Potentials.begin( ); i != m_Potentials.end( ); i++ )
		delete *i;

	m_Potentials.clear( );

	// set initial values for replay

	if( m_Replay )
	{
		for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
			m_Replay->AddPlayer( (*i)->GetPID( ), (*i)->GetName( ) );

		if( m_FakePlayerPID != 255 )
			m_Replay->AddPlayer( m_FakePlayerPID, "FakePlayer" );

		m_Replay->SetSlots( m_Slots );
		m_Replay->SetRandomSeed( m_RandomSeed );
		m_Replay->SetSelectMode( m_Map->GetMapLayoutStyle( ) );
		m_Replay->SetStartSpotCount( m_Map->GetMapNumPlayers( ) );

		if( m_SaveGame )
		{
			quint32 MapGameType = MAPGAMETYPE_SAVEDGAME;

			if( m_GameState == GAME_PRIVATE )
				MapGameType |= MAPGAMETYPE_PRIVATEGAME;

			m_Replay->SetMapGameType( MapGameType );
		}
		else
		{
			quint32 MapGameType = m_Map->GetMapGameType( );
			MapGameType |= MAPGAMETYPE_UNKNOWN0;

			if( m_GameState == GAME_PRIVATE )
				MapGameType |= MAPGAMETYPE_PRIVATEGAME;

			m_Replay->SetMapGameType( MapGameType );
		}

		if( !m_Players.isEmpty( ) )
		{
			// this might not be necessary since we're going to overwrite the replay's host PID and name everytime a player leaves

			m_Replay->SetHostPID( m_Players[0]->GetPID( ) );
			m_Replay->SetHostName( m_Players[0]->GetName( ) );
		}
	}

	// build a stat QString for use when saving the replay
	// we have to build this now because the map data is going to be deleted

	QByteArray StatString;
	StatString.append(m_Map->GetMapGameFlags( ));
	StatString.push_back( (char)0 );
	StatString.append(m_Map->GetMapWidth( ));
	StatString.append(m_Map->GetMapHeight( ));
	StatString.append(m_Map->GetMapCRC( ));
	StatString.append(m_Map->GetMapPath( ).toUtf8());
	StatString.append("GHost++");
	StatString.push_back( (char)0 );
	StatString.append(m_Map->GetMapSHA1( ));		// note: in replays generated by Warcraft III it stores 20 zeros for the SHA1 instead of the real thing
	StatString = UTIL_EncodeStatString( StatString );
	m_StatString = StatString;

	// delete the map data

	delete m_Map;
	m_Map = NULL;

	if( m_LoadInGame )
	{
		m_LoadInGameTimer.start();

		// buffer all the player loaded messages
		// this ensures that every player receives the same set of player loaded messages in the same order, even if someone leaves during loading
		// if someone leaves during loading we buffer the leave message to ensure it gets sent in the correct position but the player loaded message wouldn't get sent if we didn't buffer it now

		for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			for( QList<CGamePlayer *> :: iterator j = m_Players.begin( ); j != m_Players.end( ); j++ )
				(*j)->AddLoadInGameData( m_Protocol->SEND_W3GS_GAMELOADED_OTHERS( (*i)->GetPID( ) ) );
		}
	}
}

void CBaseGame :: EventGameLoaded( )
{
	CONSOLE_Print( "[GAME: " + m_GameName + "] finished loading with " + QString::number( GetNumHumanPlayers( ) ) + " players" );

	// send shortest, longest, and personal load times to each player

	CGamePlayer *Shortest = NULL;
	CGamePlayer *Longest = NULL;
	m_LoadInGameTimer.stop();

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( !Shortest || (*i)->GetFinishedLoadingTicks( ) < Shortest->GetFinishedLoadingTicks( ) )
			Shortest = *i;

		if( !Longest || (*i)->GetFinishedLoadingTicks( ) > Longest->GetFinishedLoadingTicks( ) )
			Longest = *i;
	}

	if( Shortest && Longest )
	{
		SendAllChat( m_GHost->GetLanguage( )->ShortestLoadByPlayer( Shortest->GetName( ), QString::number( (float)( Shortest->GetFinishedLoadingTicks( ) - m_StartedLoadingTicks ) / 1000, 'g', 2 ) ) );
		SendAllChat( m_GHost->GetLanguage( )->LongestLoadByPlayer( Longest->GetName( ), QString::number( (float)( Longest->GetFinishedLoadingTicks( ) - m_StartedLoadingTicks ) / 1000, 'g', 2 ) ) );
	}

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		SendChat( *i, m_GHost->GetLanguage( )->YourLoadingTimeWas( QString::number( (float)( (*i)->GetFinishedLoadingTicks( ) - m_StartedLoadingTicks ) / 1000, 'g', 2 ) ) );

	// read from gameloaded.txt if available

	QFile f(m_GHost->m_GameLoadedFile);

	if( f.open(QFile::ReadOnly) )
	{
		// don't print more than 8 lines

		QStringList data = QString::fromUtf8(f.readAll()).replace('\r', "").split('\n');
		for (int i = 0; i < 8 && i < data.size(); i++)
			SendAllChat(data.at(i));
	}
}

unsigned char CBaseGame :: GetSIDFromPID( unsigned char PID )
{
	if( m_Slots.size( ) > 255 )
		return 255;

	for( unsigned char i = 0; i < m_Slots.size( ); i++ )
	{
		if( m_Slots[i].GetPID( ) == PID )
			return i;
	}

	return 255;
}

CGamePlayer *CBaseGame :: GetPlayerFromPID( unsigned char PID )
{
	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( !(*i)->GetLeftMessageSent( ) && (*i)->GetPID( ) == PID )
			return *i;
	}

	return NULL;
}

CGamePlayer *CBaseGame :: GetPlayerFromSID( unsigned char SID )
{
	if( SID < m_Slots.size( ) )
		return GetPlayerFromPID( m_Slots[SID].GetPID( ) );

	return NULL;
}

CGamePlayer *CBaseGame :: GetPlayerFromName( const QString &name, bool sensitive )
{
	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( !(*i)->GetLeftMessageSent( ) )
		{
			if( name.compare( (*i)->GetName( ), sensitive ? Qt::CaseSensitive : Qt::CaseInsensitive ) == 0 )
			   return *i;
		}
	}

	return NULL;
}

quint32 CBaseGame :: GetPlayerFromNamePartial( const QString &name, CGamePlayer **player )
{
	quint32 Matches = 0;
	*player = NULL;

	// try to match each player with the passed QString (e.g. "Varlock" would be matched with "lock")

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( !(*i)->GetLeftMessageSent( ) )
		{
			QString TestName = (*i)->GetName( );

			if( TestName.contains( name, Qt::CaseInsensitive ) )
			{
				Matches++;
				*player = *i;

				// if the name matches exactly stop any further matching

				if( TestName.compare( name, Qt::CaseInsensitive ) == 0 )
				{
					Matches = 1;
					break;
				}
			}
		}
	}

	return Matches;
}

CGamePlayer *CBaseGame :: GetPlayerFromColour( unsigned char colour )
{
	for( unsigned char i = 0; i < m_Slots.size( ); i++ )
	{
		if( m_Slots[i].GetColour( ) == colour )
			return GetPlayerFromSID( i );
	}

	return NULL;
}

unsigned char CBaseGame :: GetNewPID( )
{
	// find an unused PID for a new player to use

	for( unsigned char TestPID = 1; TestPID < 255; TestPID++ )
	{
		if( TestPID == m_VirtualHostPID || TestPID == m_FakePlayerPID )
			continue;

		bool InUse = false;

		for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			if( !(*i)->GetLeftMessageSent( ) && (*i)->GetPID( ) == TestPID )
			{
				InUse = true;
				break;
			}
		}

		if( !InUse )
			return TestPID;
	}

	// this should never happen

	return 255;
}

unsigned char CBaseGame :: GetNewColour( )
{
	// find an unused colour for a player to use

	for( unsigned char TestColour = 0; TestColour < 12; TestColour++ )
	{
		bool InUse = false;

		for( unsigned char i = 0; i < m_Slots.size( ); i++ )
		{
			if( m_Slots[i].GetColour( ) == TestColour )
			{
				InUse = true;
				break;
			}
		}

		if( !InUse )
			return TestColour;
	}

	// this should never happen

	return 12;
}

QByteArray CBaseGame :: GetPIDs( )
{
	QByteArray result;

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( !(*i)->GetLeftMessageSent( ) )
			result.push_back( (*i)->GetPID( ) );
	}

	return result;
}

QByteArray CBaseGame :: GetPIDs( unsigned char excludePID )
{
	QByteArray result;

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( !(*i)->GetLeftMessageSent( ) && (*i)->GetPID( ) != excludePID )
			result.push_back( (*i)->GetPID( ) );
	}

	return result;
}

unsigned char CBaseGame :: GetHostPID( )
{
	// return the player to be considered the host (it can be any player) - mainly used for sending text messages from the bot
	// try to find the virtual host player first

	if( m_VirtualHostPID != 255 )
		return m_VirtualHostPID;

	// try to find the fakeplayer next

	if( m_FakePlayerPID != 255 )
		return m_FakePlayerPID;

	// try to find the owner player next

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( !(*i)->GetLeftMessageSent( ) && IsOwner( (*i)->GetName( ) ) )
			return (*i)->GetPID( );
	}

	// okay then, just use the first available player

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( !(*i)->GetLeftMessageSent( ) )
			return (*i)->GetPID( );
	}

	return 255;
}

unsigned char CBaseGame :: GetEmptySlot( bool reserved )
{
	if( m_Slots.size( ) > 255 )
		return 255;

	if( m_SaveGame )
	{
		// unfortunately we don't know which slot each player was assigned in the savegame
		// but we do know which slots were occupied and which weren't so let's at least force players to use previously occupied slots

		QList<CGameSlot> SaveGameSlots = m_SaveGame->GetSlots( );

		for( unsigned char i = 0; i < m_Slots.size( ); i++ )
		{
			if( m_Slots[i].GetSlotStatus( ) == SLOTSTATUS_OPEN && SaveGameSlots[i].GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && SaveGameSlots[i].GetComputer( ) == 0 )
				return i;
		}

		// don't bother with reserved slots in savegames
	}
	else
	{
		// look for an empty slot for a new player to occupy
		// if reserved is true then we're willing to use closed or occupied slots as long as it wouldn't displace a player with a reserved slot

		for( unsigned char i = 0; i < m_Slots.size( ); i++ )
		{
			if( m_Slots[i].GetSlotStatus( ) == SLOTSTATUS_OPEN )
				return i;
		}

		if( reserved )
		{
			// no empty slots, but since player is reserved give them a closed slot

			for( unsigned char i = 0; i < m_Slots.size( ); i++ )
			{
				if( m_Slots[i].GetSlotStatus( ) == SLOTSTATUS_CLOSED )
					return i;
			}

			// no closed slots either, give them an occupied slot but not one occupied by another reserved player
			// first look for a player who is downloading the map and has the least amount downloaded so far

			unsigned char LeastDownloaded = 100;
			unsigned char LeastSID = 255;

			for( unsigned char i = 0; i < m_Slots.size( ); i++ )
			{
				CGamePlayer *Player = GetPlayerFromSID( i );

				if( Player && !Player->GetReserved( ) && m_Slots[i].GetDownloadStatus( ) < LeastDownloaded )
				{
					LeastDownloaded = m_Slots[i].GetDownloadStatus( );
					LeastSID = i;
				}
			}

			if( LeastSID != 255 )
				return LeastSID;

			// nobody who isn't reserved is downloading the map, just choose the first player who isn't reserved

			for( unsigned char i = 0; i < m_Slots.size( ); i++ )
			{
				CGamePlayer *Player = GetPlayerFromSID( i );

				if( Player && !Player->GetReserved( ) )
					return i;
			}
		}
	}

	return 255;
}

unsigned char CBaseGame :: GetEmptySlot( unsigned char team, unsigned char PID )
{
	if( m_Slots.size( ) > 255 )
		return 255;

	// find an empty slot based on player's current slot

	unsigned char StartSlot = GetSIDFromPID( PID );

	if( StartSlot < m_Slots.size( ) )
	{
		if( m_Slots[StartSlot].GetTeam( ) != team )
		{
			// player is trying to move to another team so start looking from the first slot on that team
			// we actually just start looking from the very first slot since the next few loops will check the team for us

			StartSlot = 0;
		}

		if( m_SaveGame )
		{
			QList<CGameSlot> SaveGameSlots = m_SaveGame->GetSlots( );

			for( unsigned char i = StartSlot; i < m_Slots.size( ); i++ )
			{
				if( m_Slots[i].GetSlotStatus( ) == SLOTSTATUS_OPEN && m_Slots[i].GetTeam( ) == team && SaveGameSlots[i].GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && SaveGameSlots[i].GetComputer( ) == 0 )
					return i;
			}

			for( unsigned char i = 0; i < StartSlot; i++ )
			{
				if( m_Slots[i].GetSlotStatus( ) == SLOTSTATUS_OPEN && m_Slots[i].GetTeam( ) == team && SaveGameSlots[i].GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && SaveGameSlots[i].GetComputer( ) == 0 )
					return i;
			}
		}
		else
		{
			// find an empty slot on the correct team starting from StartSlot

			for( unsigned char i = StartSlot; i < m_Slots.size( ); i++ )
			{
				if( m_Slots[i].GetSlotStatus( ) == SLOTSTATUS_OPEN && m_Slots[i].GetTeam( ) == team )
					return i;
			}

			// didn't find an empty slot, but we could have missed one with SID < StartSlot
			// e.g. in the DotA case where I am in slot 4 (yellow), slot 5 (orange) is occupied, and slot 1 (blue) is open and I am trying to move to another slot

			for( unsigned char i = 0; i < StartSlot; i++ )
			{
				if( m_Slots[i].GetSlotStatus( ) == SLOTSTATUS_OPEN && m_Slots[i].GetTeam( ) == team )
					return i;
			}
		}
	}

	return 255;
}

void CBaseGame :: SwapSlots( unsigned char SID1, unsigned char SID2 )
{
	if( SID1 < m_Slots.size( ) && SID2 < m_Slots.size( ) && SID1 != SID2 )
	{
		CGameSlot Slot1 = m_Slots[SID1];
		CGameSlot Slot2 = m_Slots[SID2];

		if( m_Map->GetMapOptions( ) & MAPOPT_FIXEDPLAYERSETTINGS )
		{
			// don't swap the team, colour, or race

			m_Slots[SID1] = CGameSlot( Slot2.GetPID( ), Slot2.GetDownloadStatus( ), Slot2.GetSlotStatus( ), Slot2.GetComputer( ), Slot1.GetTeam( ), Slot1.GetColour( ), Slot1.GetRace( ), Slot2.GetComputerType( ), Slot2.GetHandicap( ) );
			m_Slots[SID2] = CGameSlot( Slot1.GetPID( ), Slot1.GetDownloadStatus( ), Slot1.GetSlotStatus( ), Slot1.GetComputer( ), Slot2.GetTeam( ), Slot2.GetColour( ), Slot2.GetRace( ), Slot1.GetComputerType( ), Slot1.GetHandicap( ) );
		}
		else
		{
			// swap everything

			m_Slots[SID1] = Slot2;
			m_Slots[SID2] = Slot1;
		}

		SendAllSlotInfo( );
	}
}

void CBaseGame :: OpenSlot( unsigned char SID, bool kick )
{
	if( SID < m_Slots.size( ) )
	{
		if( kick )
		{
			CGamePlayer *Player = GetPlayerFromSID( SID );

			if( Player )
			{
				Player->SetLeftReason( "was kicked when opening a slot" );
				Player->SetLeftCode( PLAYERLEAVE_LOBBY );
				Player->deleteLater();
			}
		}

		CGameSlot Slot = m_Slots[SID];
		m_Slots[SID] = CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, Slot.GetTeam( ), Slot.GetColour( ), Slot.GetRace( ) );
		SendAllSlotInfo( );
	}
}

void CBaseGame :: CloseSlot( unsigned char SID, bool kick )
{
	if( SID < m_Slots.size( ) )
	{
		if( kick )
		{
			CGamePlayer *Player = GetPlayerFromSID( SID );

			if( Player )
			{
				Player->SetLeftReason( "was kicked when closing a slot" );
				Player->SetLeftCode( PLAYERLEAVE_LOBBY );
				Player->deleteLater();
			}
		}

		CGameSlot Slot = m_Slots[SID];
		m_Slots[SID] = CGameSlot( 0, 255, SLOTSTATUS_CLOSED, 0, Slot.GetTeam( ), Slot.GetColour( ), Slot.GetRace( ) );
		SendAllSlotInfo( );
	}
}

void CBaseGame :: ComputerSlot( unsigned char SID, unsigned char skill, bool kick )
{
	if( SID < m_Slots.size( ) && skill < 3 )
	{
		if( kick )
		{
			CGamePlayer *Player = GetPlayerFromSID( SID );

			if( Player )
			{
				Player->SetLeftReason( "was kicked when creating a computer in a slot" );
				Player->SetLeftCode( PLAYERLEAVE_LOBBY );
				Player->deleteLater();
			}
		}

		CGameSlot Slot = m_Slots[SID];
		m_Slots[SID] = CGameSlot( 0, 100, SLOTSTATUS_OCCUPIED, 1, Slot.GetTeam( ), Slot.GetColour( ), Slot.GetRace( ), skill );
		SendAllSlotInfo( );
	}
}

void CBaseGame :: ColourSlot( unsigned char SID, unsigned char colour )
{
	if( SID < m_Slots.size( ) && colour < 12 )
	{
		// make sure the requested colour isn't already taken

		bool Taken = false;
		unsigned char TakenSID = 0;

		for( unsigned char i = 0; i < m_Slots.size( ); i++ )
		{
			if( m_Slots[i].GetColour( ) == colour )
			{
				TakenSID = i;
				Taken = true;
			}
		}

		if( Taken && m_Slots[TakenSID].GetSlotStatus( ) != SLOTSTATUS_OCCUPIED )
		{
			// the requested colour is currently "taken" by an unused (open or closed) slot
			// but we allow the colour to persist within a slot so if we only update the existing player's colour the unused slot will have the same colour
			// this isn't really a problem except that if someone then joins the game they'll receive the unused slot's colour resulting in a duplicate
			// one way to solve this (which we do here) is to swap the player's current colour into the unused slot

			m_Slots[TakenSID].SetColour( m_Slots[SID].GetColour( ) );
			m_Slots[SID].SetColour( colour );
			SendAllSlotInfo( );
		}
		else if( !Taken )
		{
			// the requested colour isn't used by ANY slot

			m_Slots[SID].SetColour( colour );
			SendAllSlotInfo( );
		}
	}
}

void CBaseGame :: OpenAllSlots( )
{
	bool Changed = false;

	for( QList<CGameSlot> :: iterator i = m_Slots.begin( ); i != m_Slots.end( ); i++ )
	{
		if( (*i).GetSlotStatus( ) == SLOTSTATUS_CLOSED )
		{
			(*i).SetSlotStatus( SLOTSTATUS_OPEN );
			Changed = true;
		}
	}

	if( Changed )
		SendAllSlotInfo( );
}

void CBaseGame :: CloseAllSlots( )
{
	bool Changed = false;

	for( QList<CGameSlot> :: iterator i = m_Slots.begin( ); i != m_Slots.end( ); i++ )
	{
		if( (*i).GetSlotStatus( ) == SLOTSTATUS_OPEN )
		{
			(*i).SetSlotStatus( SLOTSTATUS_CLOSED );
			Changed = true;
		}
	}

	if( Changed )
		SendAllSlotInfo( );
}

void CBaseGame :: ShuffleSlots( )
{
	// we only want to shuffle the player slots
	// that means we need to prevent this function from shuffling the open/closed/computer slots too
	// so we start by copying the player slots to a temporary vector

	QList<CGameSlot> PlayerSlots;

	for( QList<CGameSlot> :: iterator i = m_Slots.begin( ); i != m_Slots.end( ); i++ )
	{
		if( (*i).GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && (*i).GetComputer( ) == 0 && (*i).GetTeam( ) != 12 )
			PlayerSlots.push_back( *i );
	}

	// now we shuffle PlayerSlots

	if( m_Map->GetMapOptions( ) & MAPOPT_FIXEDPLAYERSETTINGS )
	{
		// rather than rolling our own probably broken shuffle algorithm we use random_shuffle because it's guaranteed to do it properly
		// so in order to let random_shuffle do all the work we need a vector to operate on
		// unfortunately we can't just use PlayerSlots because the team/colour/race shouldn't be modified
		// so make a vector we can use

		QList<unsigned char> SIDs;

		for( unsigned char i = 0; i < PlayerSlots.size( ); i++ )
			SIDs.push_back( i );

		random_shuffle( SIDs.begin( ), SIDs.end( ) );

		// now put the PlayerSlots vector in the same order as the SIDs vector

		QList<CGameSlot> Slots;

		// as usual don't modify the team/colour/race

		for( unsigned char i = 0; i < SIDs.size( ); i++ )
			Slots.push_back( CGameSlot( PlayerSlots[SIDs[i]].GetPID( ), PlayerSlots[SIDs[i]].GetDownloadStatus( ), PlayerSlots[SIDs[i]].GetSlotStatus( ), PlayerSlots[SIDs[i]].GetComputer( ), PlayerSlots[i].GetTeam( ), PlayerSlots[i].GetColour( ), PlayerSlots[i].GetRace( ) ) );

		PlayerSlots = Slots;
	}
	else
	{
		// regular game
		// it's easy when we're allowed to swap the team/colour/race!

		random_shuffle( PlayerSlots.begin( ), PlayerSlots.end( ) );
	}

	// now we put m_Slots back together again

	QList<CGameSlot> :: iterator CurrentPlayer = PlayerSlots.begin( );
	QList<CGameSlot> Slots;

	for( QList<CGameSlot> :: iterator i = m_Slots.begin( ); i != m_Slots.end( ); i++ )
	{
		if( (*i).GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && (*i).GetComputer( ) == 0 && (*i).GetTeam( ) != 12 )
		{
			Slots.push_back( *CurrentPlayer );
			CurrentPlayer++;
		}
		else
			Slots.push_back( *i );
	}

	m_Slots = Slots;

	// and finally tell everyone about the new slot configuration

	SendAllSlotInfo( );
}

QList<unsigned char> CBaseGame :: BalanceSlotsRecursive( QList<unsigned char> PlayerIDs, unsigned char *TeamSizes, double *PlayerScores, unsigned char StartTeam )
{
	// take a brute force approach to finding the best balance by iterating through every possible combination of players
	// 1.) since the number of teams is arbitrary this algorithm must be recursive
	// 2.) on the first recursion step every possible combination of players into two "teams" is checked, where the first team is the correct size and the second team contains everyone else
	// 3.) on the next recursion step every possible combination of the remaining players into two more "teams" is checked, continuing until all the actual teams are accounted for
	// 4.) for every possible combination, check the largest difference in total scores between any two actual teams
	// 5.) minimize this value by choosing the combination of players with the smallest difference

	QList<unsigned char> BestOrdering = PlayerIDs;
	double BestDifference = -1.0;

	for( unsigned char i = StartTeam; i < 12; i++ )
	{
		if( TeamSizes[i] > 0 )
		{
			unsigned char Mid = TeamSizes[i];

			// the base case where only one actual team worth of players was passed to this function is handled by the behaviour of next_combination
			// in this case PlayerIDs.begin( ) + Mid will actually be equal to PlayerIDs.end( ) and next_combination will return false

			while( next_combination( PlayerIDs.begin( ), PlayerIDs.begin( ) + Mid, PlayerIDs.end( ) ) )
			{
				// we're splitting the players into every possible combination of two "teams" based on the midpoint Mid
				// the first (left) team contains the correct number of players but the second (right) "team" might or might not
				// for example, it could contain one, two, or more actual teams worth of players
				// so recurse using the second "team" as the full set of players to perform the balancing on

				QList<unsigned char> BestSubOrdering = BalanceSlotsRecursive( PlayerIDs.mid(Mid), TeamSizes, PlayerScores, i + 1 );

				// BestSubOrdering now contains the best ordering of all the remaining players (the "right team") given this particular combination of players into two "teams"
				// in order to calculate the largest difference in total scores we need to recombine the subordering with the first team

				QList<unsigned char> TestOrdering = PlayerIDs.mid(0, Mid);
				for (int k = 0; k < BestSubOrdering.size(); k++)
					TestOrdering.push_back(BestSubOrdering.at(k));

				// now calculate the team scores for all the teams that we know about (e.g. on subsequent recursion steps this will NOT be every possible team)

				QList<unsigned char> :: iterator CurrentPID = TestOrdering.begin( );
				double TeamScores[12];

				for( unsigned char j = StartTeam; j < 12; j++ )
				{
					TeamScores[j] = 0.0;

					for( unsigned char k = 0; k < TeamSizes[j]; k++ )
					{
						TeamScores[j] += PlayerScores[*CurrentPID];
						CurrentPID++;
					}
				}

				// find the largest difference in total scores between any two teams

				double LargestDifference = 0.0;

				for( unsigned char j = StartTeam; j < 12; j++ )
				{
					if( TeamSizes[j] > 0 )
					{
						for( unsigned char k = j + 1; k < 12; k++ )
						{
							if( TeamSizes[k] > 0 )
							{
								double Difference = abs( TeamScores[j] - TeamScores[k] );

								if( Difference > LargestDifference )
									LargestDifference = Difference;
							}
						}
					}
				}

				// and minimize it

				if( BestDifference < 0.0 || LargestDifference < BestDifference )
				{
					BestOrdering = TestOrdering;
					BestDifference = LargestDifference;
				}
			}
		}
	}

	return BestOrdering;
}

void CBaseGame :: BalanceSlots( )
{
	if( !( m_Map->GetMapOptions( ) & MAPOPT_FIXEDPLAYERSETTINGS ) )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] error balancing slots - can't balance slots without fixed player settings" );
		return;
	}

	// setup the necessary variables for the balancing algorithm
	// use an array of 13 elements for 12 players because GHost++ allocates PID's from 1-12 (i.e. excluding 0) and we use the PID to index the array

	QList<unsigned char> PlayerIDs;
	unsigned char TeamSizes[12];
	double PlayerScores[13];
	memset( TeamSizes, 0, sizeof( unsigned char ) * 12 );

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		unsigned char PID = (*i)->GetPID( );

		if( PID < 13 )
		{
			unsigned char SID = GetSIDFromPID( PID );

			if( SID < m_Slots.size( ) )
			{
				unsigned char Team = m_Slots[SID].GetTeam( );

				if( Team < 12 )
				{
					// we are forced to use a default score because there's no way to balance the teams otherwise

					double Score = (*i)->GetScore( );

					if( Score < -99999.0 )
						Score = m_Map->GetMapDefaultPlayerScore( );

					PlayerIDs.push_back( PID );
					TeamSizes[Team]++;
					PlayerScores[PID] = Score;
				}
			}
		}
	}

	sort( PlayerIDs.begin( ), PlayerIDs.end( ) );

	// balancing the teams is a variation of the bin packing problem which is NP
	// we can have up to 12 players and/or teams so the scope of the problem is sometimes small enough to process quickly
	// let's try to figure out roughly how much work this is going to take
	// examples:
	//  2 teams of 4 =     70 ~    5ms *** ok
	//  2 teams of 5 =    252 ~    5ms *** ok
	//  2 teams of 6 =    924 ~   20ms *** ok
	//  3 teams of 2 =     90 ~    5ms *** ok
	//  3 teams of 3 =   1680 ~   25ms *** ok
	//  3 teams of 4 =  34650 ~  250ms *** will cause a lag spike
	//  4 teams of 2 =   2520 ~   30ms *** ok
	//  4 teams of 3 = 369600 ~ 3500ms *** unacceptable

	quint32 AlgorithmCost = 0;
	quint32 PlayersLeft = PlayerIDs.size( );

	for( unsigned char i = 0; i < 12; i++ )
	{
		if( TeamSizes[i] > 0 )
		{
			if( AlgorithmCost == 0 )
				AlgorithmCost = nCr( PlayersLeft, TeamSizes[i] );
			else
				AlgorithmCost *= nCr( PlayersLeft, TeamSizes[i] );

			PlayersLeft -= TeamSizes[i];
		}
	}

	if( AlgorithmCost > 40000 )
	{
		// the cost is too high, don't run the algorithm
		// a possible alternative: stop after enough iterations and/or time has passed

		CONSOLE_Print( "[GAME: " + m_GameName + "] shuffling slots instead of balancing - the algorithm is too slow (with a cost of " + QString::number( AlgorithmCost ) + ") for this team configuration" );
		SendAllChat( m_GHost->GetLanguage( )->ShufflingPlayers( ) );
		ShuffleSlots( );
		return;
	}

	quint32 StartTicks = GetTicks( );
	QList<unsigned char> BestOrdering = BalanceSlotsRecursive( PlayerIDs, TeamSizes, PlayerScores, 0 );
	quint32 EndTicks = GetTicks( );

	// the BestOrdering assumes the teams are in slot order although this may not be the case
	// so put the players on the correct teams regardless of slot order

	QList<unsigned char> :: iterator CurrentPID = BestOrdering.begin( );

	for( unsigned char i = 0; i < 12; i++ )
	{
		unsigned char CurrentSlot = 0;

		for( unsigned char j = 0; j < TeamSizes[i]; j++ )
		{
			while( CurrentSlot < m_Slots.size( ) && m_Slots[CurrentSlot].GetTeam( ) != i )
				CurrentSlot++;

			// put the CurrentPID player on team i by swapping them into CurrentSlot

			unsigned char SID1 = CurrentSlot;
			unsigned char SID2 = GetSIDFromPID( *CurrentPID );

			if( SID1 < m_Slots.size( ) && SID2 < m_Slots.size( ) )
			{
				CGameSlot Slot1 = m_Slots[SID1];
				CGameSlot Slot2 = m_Slots[SID2];
				m_Slots[SID1] = CGameSlot( Slot2.GetPID( ), Slot2.GetDownloadStatus( ), Slot2.GetSlotStatus( ), Slot2.GetComputer( ), Slot1.GetTeam( ), Slot1.GetColour( ), Slot1.GetRace( ) );
				m_Slots[SID2] = CGameSlot( Slot1.GetPID( ), Slot1.GetDownloadStatus( ), Slot1.GetSlotStatus( ), Slot1.GetComputer( ), Slot2.GetTeam( ), Slot2.GetColour( ), Slot2.GetRace( ) );
			}
			else
			{
				CONSOLE_Print( "[GAME: " + m_GameName + "] shuffling slots instead of balancing - the balancing algorithm tried to do an invalid swap (this shouldn't happen)" );
				SendAllChat( m_GHost->GetLanguage( )->ShufflingPlayers( ) );
				ShuffleSlots( );
				return;
			}

			CurrentPID++;
			CurrentSlot++;
		}
	}

	CONSOLE_Print( "[GAME: " + m_GameName + "] balancing slots completed in " + QString::number( EndTicks - StartTicks ) + "ms (with a cost of " + QString::number( AlgorithmCost ) + ")" );
	SendAllChat( m_GHost->GetLanguage( )->BalancingSlotsCompleted( ) );
	SendAllSlotInfo( );

	for( unsigned char i = 0; i < 12; i++ )
	{
		bool TeamHasPlayers = false;
		double TeamScore = 0.0;

		for( QList<CGamePlayer *> :: iterator j = m_Players.begin( ); j != m_Players.end( ); j++ )
		{
			unsigned char SID = GetSIDFromPID( (*j)->GetPID( ) );

			if( SID < m_Slots.size( ) && m_Slots[SID].GetTeam( ) == i )
			{
				TeamHasPlayers = true;
				double Score = (*j)->GetScore( );

				if( Score < -99999.0 )
					Score = m_Map->GetMapDefaultPlayerScore( );

				TeamScore += Score;
			}
		}

		if( TeamHasPlayers )
			SendAllChat( m_GHost->GetLanguage( )->TeamCombinedScore( QString::number( i + 1 ), QString::number( TeamScore, 'g', 2 ) ) );
	}
}

void CBaseGame :: AddToSpoofed( const QString &server, const QString &name, bool sendMessage )
{
	CGamePlayer *Player = GetPlayerFromName( name, true );

	if( Player )
	{
		Player->SetSpoofedRealm( server );
		Player->SetSpoofed( true );

		if( sendMessage )
			SendAllChat( m_GHost->GetLanguage( )->SpoofCheckAcceptedFor( server, name ) );
	}
}

void CBaseGame :: AddToReserved( const QString &name )
{
	QString nameLower = name.toLower();

	// check that the user is not already reserved

	if ( m_Reserved.contains( nameLower ) )
		return;

	m_Reserved.push_back( nameLower );

	// upgrade the user if they're already in the game

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( name.compare( (*i)->GetName( ), Qt::CaseInsensitive ) == 0 )
			(*i)->SetReserved( true );
	}
}

bool CBaseGame :: IsOwner( const QString &name )
{
	QString OwnerLower = m_OwnerName.toLower();
	return name.toLower() == OwnerLower;
}

bool CBaseGame :: IsReserved( const QString &name )
{
	if( m_Reserved.contains( name, Qt::CaseInsensitive ) ) {
		return true;
	}
	
	return false;
}

bool CBaseGame :: IsDownloading( )
{
	// returns true if at least one player is downloading the map

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( (*i)->GetDownloadStarted( ) && !(*i)->GetDownloadFinished( ) )
			return true;
	}

	return false;
}

bool CBaseGame :: IsGameDataSaved( )
{
	return true;
}

void CBaseGame :: SaveGameData( )
{

}

void CBaseGame :: StartCountDown( bool force )
{
	if( !m_CountDownStarted )
	{
		if( force )
		{
			m_CountDownStarted = true;
			m_CountdownTimer.start();
			m_CountDownCounter = 5;
		}
		else
		{
			// check if the HCL command QString is short enough

			if( m_HCLCommandString.size( ) > (int)GetSlotsOccupied( ) )
			{
				SendAllChat( m_GHost->GetLanguage( )->TheHCLIsTooLongUseForceToStart( ) );
				return;
			}

			// check if everyone has the map

			QString StillDownloading;

			for( QList<CGameSlot> :: iterator i = m_Slots.begin( ); i != m_Slots.end( ); i++ )
			{
				if( (*i).GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && (*i).GetComputer( ) == 0 && (*i).GetDownloadStatus( ) != 100 )
				{
					CGamePlayer *Player = GetPlayerFromPID( (*i).GetPID( ) );

					if( Player )
					{
						if( StillDownloading.isEmpty( ) )
							StillDownloading = Player->GetName( );
						else
							StillDownloading += ", " + Player->GetName( );
					}
				}
			}

			if( !StillDownloading.isEmpty( ) )
				SendAllChat( m_GHost->GetLanguage( )->PlayersStillDownloading( StillDownloading ) );

			// check if everyone is spoof checked

			QString NotSpoofChecked;

			if( m_GHost->m_RequireSpoofChecks )
			{
				for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
				{
					if( !(*i)->GetSpoofed( ) )
					{
						if( NotSpoofChecked.isEmpty( ) )
							NotSpoofChecked = (*i)->GetName( );
						else
							NotSpoofChecked += ", " + (*i)->GetName( );
					}
				}

				if( !NotSpoofChecked.isEmpty( ) )
					SendAllChat( m_GHost->GetLanguage( )->PlayersNotYetSpoofChecked( NotSpoofChecked ) );
			}

			// check if everyone has been pinged enough (3 times) that the autokicker would have kicked them by now
			// see function EventPlayerPongToHost for the autokicker code

			QString NotPinged;

			for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
			{
				if( !(*i)->GetReserved( ) && (*i)->GetNumPings( ) < 3 )
				{
					if( NotPinged.isEmpty( ) )
						NotPinged = (*i)->GetName( );
					else
						NotPinged += ", " + (*i)->GetName( );
				}
			}

			if( !NotPinged.isEmpty( ) )
				SendAllChat( m_GHost->GetLanguage( )->PlayersNotYetPinged( NotPinged ) );

			// if no problems found start the game

			if( StillDownloading.isEmpty( ) && NotSpoofChecked.isEmpty( ) && NotPinged.isEmpty( ) )
			{
				m_CountDownStarted = true;
				m_CountDownCounter = 5;
				m_CountdownTimer.start();
			}
		}
	}
}

void CBaseGame :: StartCountDownAuto( bool requireSpoofChecks )
{
	if( m_CountDownStarted )
		return;

	// check if enough players are present

	if( GetNumHumanPlayers( ) < m_AutoStartPlayers )
	{
		SendAllChat( m_GHost->GetLanguage( )->WaitingForPlayersBeforeAutoStart( QString::number( m_AutoStartPlayers ), QString::number( m_AutoStartPlayers - GetNumHumanPlayers( ) ) ) );
		return;
	}

	// check if everyone has the map

	QString StillDownloading;

	for( QList<CGameSlot> :: iterator i = m_Slots.begin( ); i != m_Slots.end( ); i++ )
	{
		if( (*i).GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && (*i).GetComputer( ) == 0 && (*i).GetDownloadStatus( ) != 100 )
		{
			CGamePlayer *Player = GetPlayerFromPID( (*i).GetPID( ) );

			if( Player )
			{
				if( StillDownloading.isEmpty( ) )
					StillDownloading = Player->GetName( );
				else
					StillDownloading += ", " + Player->GetName( );
			}
		}
	}

	if( !StillDownloading.isEmpty( ) )
	{
		SendAllChat( m_GHost->GetLanguage( )->PlayersStillDownloading( StillDownloading ) );
		return;
	}

	// check if everyone is spoof checked

	QString NotSpoofChecked;

	if( requireSpoofChecks )
	{
		for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			if( !(*i)->GetSpoofed( ) )
			{
				if( NotSpoofChecked.isEmpty( ) )
					NotSpoofChecked = (*i)->GetName( );
				else
					NotSpoofChecked += ", " + (*i)->GetName( );
			}
		}

		if( !NotSpoofChecked.isEmpty( ) )
			SendAllChat( m_GHost->GetLanguage( )->PlayersNotYetSpoofChecked( NotSpoofChecked ) );
	}

	// check if everyone has been pinged enough (3 times) that the autokicker would have kicked them by now
	// see function EventPlayerPongToHost for the autokicker code

	QString NotPinged;

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( !(*i)->GetReserved( ) && (*i)->GetNumPings( ) < 3 )
		{
			if( NotPinged.isEmpty( ) )
				NotPinged = (*i)->GetName( );
			else
				NotPinged += ", " + (*i)->GetName( );
		}
	}

	if( !NotPinged.isEmpty( ) )
	{
		SendAllChat( m_GHost->GetLanguage( )->PlayersNotYetPingedAutoStart( NotPinged ) );
		return;
	}

	// if no problems found start the game

	if( StillDownloading.isEmpty( ) && NotSpoofChecked.isEmpty( ) && NotPinged.isEmpty( ) )
	{
		m_CountDownStarted = true;
		m_CountdownTimer.start();
		m_CountDownCounter = 10;
	}
}

void CBaseGame :: StopPlayers( const QString &reason )
{
	// disconnect every player and set their left reason to the passed QString
	// we use this function when we want the code in the Update function to run before the destructor (e.g. saving players to the database)
	// therefore calling this function when m_GameLoading || m_GameLoaded is roughly equivalent to setting m_Exiting = true
	// the only difference is whether the code in the Update function is executed or not

	for( QList<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); )
	{
		CGamePlayer *p = *i;
		i = m_Players.erase(i);

		p->SetLeftReason( reason );
		p->SetLeftCode( PLAYERLEAVE_LOST );
		p->deleteLater();
	}
}

void CBaseGame :: StopLaggers( const QString &reason )
{
	for( QList<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( (*i)->GetLagging( ) )
		{
			CGamePlayer *p = *i;
			i = m_Players.erase(i);

			p->SetLeftReason( reason );
			p->SetLeftCode( PLAYERLEAVE_LOST );
			p->deleteLater();
		}
	}
}

void CBaseGame :: CreateVirtualHost( )
{
	if( m_VirtualHostPID != 255 )
		return;

	m_VirtualHostPID = GetNewPID( );
	QByteArray IP;
	IP.push_back( (char)0 );
	IP.push_back( (char)0 );
	IP.push_back( (char)0 );
	IP.push_back( (char)0 );
	SendAll( m_Protocol->SEND_W3GS_PLAYERINFO( m_VirtualHostPID, m_VirtualHostName, IP, IP ) );
}

void CBaseGame :: DeleteVirtualHost( )
{
	if( m_VirtualHostPID == 255 )
		return;

	SendAll( m_Protocol->SEND_W3GS_PLAYERLEAVE_OTHERS( m_VirtualHostPID, PLAYERLEAVE_LOBBY ) );
	m_VirtualHostPID = 255;
}

void CBaseGame :: CreateFakePlayer( )
{
	if( m_FakePlayerPID != 255 )
		return;

	unsigned char SID = GetEmptySlot( false );

	if( SID < m_Slots.size( ) )
	{
		if( GetNumPlayers( ) >= 11 )
			DeleteVirtualHost( );

		m_FakePlayerPID = GetNewPID( );
		QByteArray IP;
		IP.push_back( (char)0 );
		IP.push_back( (char)0 );
		IP.push_back( (char)0 );
		IP.push_back( (char)0 );
		SendAll( m_Protocol->SEND_W3GS_PLAYERINFO( m_FakePlayerPID, "FakePlayer", IP, IP ) );
		m_Slots[SID] = CGameSlot( m_FakePlayerPID, 100, SLOTSTATUS_OCCUPIED, 0, m_Slots[SID].GetTeam( ), m_Slots[SID].GetColour( ), m_Slots[SID].GetRace( ) );
		SendAllSlotInfo( );
	}
}

void CBaseGame :: DeleteFakePlayer( )
{
	if( m_FakePlayerPID == 255 )
		return;

	for( unsigned char i = 0; i < m_Slots.size( ); i++ )
	{
		if( m_Slots[i].GetPID( ) == m_FakePlayerPID )
			m_Slots[i] = CGameSlot( 0, 255, SLOTSTATUS_OPEN, 0, m_Slots[i].GetTeam( ), m_Slots[i].GetColour( ), m_Slots[i].GetRace( ) );
	}

	SendAll( m_Protocol->SEND_W3GS_PLAYERLEAVE_OTHERS( m_FakePlayerPID, PLAYERLEAVE_LOBBY ) );
	SendAllSlotInfo( );
	m_FakePlayerPID = 255;
}
