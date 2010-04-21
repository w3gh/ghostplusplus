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
#include "language.h"
#include "commandpacket.h"
#include "bnet.h"
#include "map.h"
#include "gameplayer.h"
#include "gameprotocol.h"
#include "gpsprotocol.h"
#include "game_base.h"

//
// CPotentialPlayer
//

#include <QAbstractSocket>

CPotentialPlayer :: CPotentialPlayer( CGameProtocol *nProtocol, CBaseGame *nGame, QTcpSocket *nSocket )
	: QObject(NULL)
{
	m_Protocol = nProtocol;
	m_Game = nGame;
	m_Socket = nSocket;
	m_Socket->setParent(this);

	m_Error = false;
	m_IncomingJoinPlayer = NULL;

	QObject::connect(nSocket, SIGNAL(readyRead()), this, SLOT(EventDataReady()));
	QObject::connect(nSocket, SIGNAL(disconnected()), this, SLOT(EventConnectionClosed()));
	QObject::connect(nSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(EventConnectionError(QAbstractSocket::SocketError)));
}

void CPotentialPlayer::deleteLater()
{
	emit aboutToDelete();
	QObject::deleteLater();
}

CPotentialPlayer :: ~CPotentialPlayer( )
{
	while( !m_Packets.isEmpty( ) )
		delete m_Packets.dequeue( );

	delete m_IncomingJoinPlayer;
	m_IncomingJoinPlayer = NULL;
}

void CPotentialPlayer::EventDataReady()
{
	m_TimeoutTimer.start();

	ExtractPackets( );
	ProcessPackets( );
}

void CPotentialPlayer::EventConnectionError(QAbstractSocket::SocketError /*error*/)
{
	DEBUG_Print("CPotentialPlayer::EventConnectionError()");
	deleteLater();
}

void CPotentialPlayer::EventConnectionClosed()
{
	DEBUG_Print("CPotentialPlayer::EventConnectionClosed()");
	deleteLater();
}

void CGamePlayer::EventPingTimeout()
{
	Send(m_Protocol->SEND_W3GS_PING_FROM_HOST( ) );
}

void CGamePlayer::EventACKTimeout()
{
	DEBUG_Print("EventACKTimeout()");
	// GProxy++ acks

	Send( m_Game->m_GHost->m_GPSProtocol->SEND_GPSS_ACK( m_TotalPacketsReceived ) );
}

void CGamePlayer::EventWhoisTimeout()
{
	// wait 4 seconds after joining before sending the /whois or /w
	// if we send the /whois too early battle.net may not have caught up with where the player is and return erroneous results

	if( m_WhoisShouldBeSent && !m_Spoofed && !m_WhoisSent && !m_JoinedRealm.isEmpty( ) )
	{
		// todotodo: we could get kicked from battle.net for sending a command with invalid characters, do some basic checking

		for( QVector<CBNET *> :: iterator i = m_Game->m_GHost->m_BNETs.begin( ); i != m_Game->m_GHost->m_BNETs.end( ); i++ )
		{
			if( (*i)->GetServer( ) == m_JoinedRealm )
			{
				if( m_Game->GetGameState( ) == GAME_PUBLIC )
				{
					if( (*i)->GetPasswordHashType( ) == "pvpgn" )
						(*i)->QueueChatCommand( "/whereis " + m_Name );
					else
						(*i)->QueueChatCommand( "/whois " + m_Name );
				}
				else if( m_Game->GetGameState( ) == GAME_PRIVATE )
					(*i)->QueueChatCommand( m_Game->m_GHost->m_Language->SpoofCheckByReplying( ), m_Name, true );
			}
		}

		m_WhoisSent = true;
	}

}

void CGamePlayer::EventConnectionError(QAbstractSocket::SocketError /*error*/)
{
	m_Game->EventPlayerDisconnectSocketError( this );
	deleteLater();
}

void CGamePlayer::EventConnectionTimeout()
{
	// check for socket timeouts
	// if we don't receive anything from a player for 30 seconds we can assume they've dropped
	// this works because in the lobby we send pings every 5 seconds and expect a response to each one
	// and in the game the Warcraft 3 client sends keepalives frequently (at least once per second it looks like)
	m_Game->EventPlayerDisconnectTimedOut( this );
	deleteLater();
}

void CGamePlayer::EventConnectionClosed()
{
	if (m_Socket->error() != QAbstractSocket::UnknownSocketError)
	{
		DEBUG_Print("Error is " + QString::number((int)m_Socket->error()));
		return;
	}

	m_Game->EventPlayerDisconnectConnectionClosed( this );
	deleteLater();
}

QByteArray CPotentialPlayer :: GetExternalIP( )
{
	if( m_Socket )
		return UTIL_CreateBYTEARRAY((quint32)m_Socket->localAddress().toIPv4Address(), false);

	unsigned char Zeros[] = { 0, 0, 0, 0 };
	return QByteArray( (char*)Zeros, 4 );
}

QString CPotentialPlayer :: GetExternalIPString( )
{
	if( m_Socket )
		return m_Socket->localAddress().toString();

	return QString( );
}

void CPotentialPlayer :: ExtractPackets( )
{
	if( !m_Socket || !m_Socket->isValid() )
		return;

	// extract as many packets as possible from the socket's receive buffer and put them in the m_Packets queue
	// a packet is at least 4 bytes so loop as long as the buffer contains 4 bytes

	while( m_Socket->bytesAvailable() >= 4 )
	{
		unsigned char header = m_Socket->peek(1).at(0);
		if (header != W3GS_HEADER_CONSTANT && header != GPS_HEADER_CONSTANT )
		{
			m_Error = true;
			m_ErrorString = "received invalid packet from player (bad header constant)";
			m_Socket->abort();
			m_Socket->deleteLater();
			m_Socket = NULL;
			return;
		}

		// bytes 2 and 3 contain the length of the packet

		quint16 Length = UTIL_QByteArrayToUInt16( m_Socket->peek(4), false, 2 );

		if( Length < 4 )
		{
			m_Error = true;
			m_ErrorString = "received invalid packet from player (bad length)";
			m_Socket->abort();
			m_Socket->deleteLater();
			m_Socket = NULL;
			return;
		}

		if( m_Socket->bytesAvailable() < Length )
			return;

		QByteArray Bytes = m_Socket->read(Length);
		m_Packets.enqueue( new CCommandPacket( header, Bytes.at(1), Bytes ) );
	}
}

void CPotentialPlayer :: ProcessPackets( )
{
	if( !m_Socket || !m_Socket->isValid())
		return;

	// process all the received packets in the m_Packets queue

	while( !m_Packets.isEmpty( ) )
	{
		CCommandPacket *Packet = m_Packets.front( );
		m_Packets.dequeue( );

		if( Packet->GetPacketType( ) == W3GS_HEADER_CONSTANT )
		{
			// the only packet we care about as a potential player is W3GS_REQJOIN, ignore everything else

			switch( Packet->GetID( ) )
			{
			case CGameProtocol :: W3GS_REQJOIN:
				delete m_IncomingJoinPlayer;
				m_IncomingJoinPlayer = m_Protocol->RECEIVE_W3GS_REQJOIN( Packet->GetData( ) );

				if( m_IncomingJoinPlayer )
					m_Game->EventPlayerJoined( this, m_IncomingJoinPlayer );

				// don't continue looping because there may be more packets waiting and this parent class doesn't handle them
				// EventPlayerJoined creates the new player, NULLs the socket, and sets the delete flag on this object so it'll be deleted shortly
				// any unprocessed packets will be copied to the new CGamePlayer in the constructor or discarded if we get deleted because the game is full

				delete Packet;
				return;
			}
		}

		delete Packet;
	}
}

void CPotentialPlayer :: Send( QByteArray data )
{
	if( m_Socket && m_Socket->isValid() )
		m_Socket->write(data);
}

//
// CGamePlayer
//
#include <QTimer>
CGamePlayer :: CGamePlayer( CGameProtocol *nProtocol, CBaseGame *nGame, QTcpSocket *nSocket, unsigned char nPID, QString nJoinedRealm, QString nName, QByteArray nInternalIP, bool nReserved )
	: CPotentialPlayer( nProtocol, nGame, nSocket )
{
	m_PID = nPID;
	m_Name = nName;
	m_InternalIP = nInternalIP;
	m_JoinedRealm = nJoinedRealm;
	m_TotalPacketsSent = 0;
	m_TotalPacketsReceived = 0;
	m_LeftCode = PLAYERLEAVE_LOBBY;
	m_LoginAttempts = 0;
	m_SyncCounter = 0;
	m_JoinTime = GetTime( );
	m_LastMapPartSent = 0;
	m_LastMapPartAcked = 0;
	m_StartedDownloadingTicks = 0;
	m_FinishedDownloadingTime = 0;
	m_FinishedLoadingTicks = 0;
	m_StartedLaggingTicks = 0;
	m_StatsSentTime = 0;
	m_StatsDotASentTime = 0;
	m_LastGProxyWaitNoticeSentTime = 0;
	m_Score = -100000.0;
	m_LoggedIn = false;
	m_Spoofed = false;
	m_Reserved = nReserved;
	m_WhoisShouldBeSent = false;
	m_WhoisSent = false;
	m_DownloadAllowed = false;
	m_DownloadStarted = false;
	m_DownloadFinished = false;
	m_FinishedLoading = false;
	m_Lagging = false;
	m_DropVote = false;
	m_KickVote = false;
	m_Muted = false;
	m_LeftMessageSent = false;
	m_GProxy = false;
	m_GProxyDisconnectNoticeSent = false;
	m_GProxyReconnectKey = GetTicks( );
	m_LastGProxyAckTime = 0;
}

CGamePlayer :: CGamePlayer( CPotentialPlayer *potential, unsigned char nPID, QString nJoinedRealm, QString nName, QByteArray nInternalIP, bool nReserved )
	: CPotentialPlayer( potential->m_Protocol, potential->m_Game, potential->GetSocket( ) )
{
	// todotodo: properly copy queued packets to the new player, this just discards them
	// this isn't a big problem because official Warcraft III clients don't send any packets after the join request until they receive a response

	QTcpSocket *s = potential->GetSocket();

	s->disconnect(potential, SLOT(EventConnectionClosed()));
	s->disconnect(potential, SLOT(EventConnectionError(QAbstractSocket::SocketError)));
	s->disconnect(potential, SLOT(EventDataReady()));

	// m_Packets = potential->GetPackets( );
	m_PID = nPID;
	m_Name = nName;
	m_InternalIP = nInternalIP;
	m_JoinedRealm = nJoinedRealm;
	m_TotalPacketsSent = 0;

	// hackhack: we initialize this to 1 because the CPotentialPlayer must have received a W3GS_REQJOIN before this class was created
	// to fix this we could move the packet counters to CPotentialPlayer and copy them here
	// note: we must make sure we never send a packet to a CPotentialPlayer otherwise the send counter will be incorrect too! what a mess this is...
	// that said, the packet counters are only used for managing GProxy++ reconnections

	m_TotalPacketsReceived = 1;
	m_LeftCode = PLAYERLEAVE_LOBBY;
	m_LoginAttempts = 0;
	m_SyncCounter = 0;
	m_JoinTime = GetTime( );
	m_LastMapPartSent = 0;
	m_LastMapPartAcked = 0;
	m_StartedDownloadingTicks = 0;
	m_FinishedDownloadingTime = 0;
	m_FinishedLoadingTicks = 0;
	m_StartedLaggingTicks = 0;
	m_StatsSentTime = 0;
	m_StatsDotASentTime = 0;
	m_LastGProxyWaitNoticeSentTime = 0;
	m_Score = -100000.0;
	m_LoggedIn = false;
	m_Spoofed = false;
	m_Reserved = nReserved;
	m_WhoisShouldBeSent = false;
	m_WhoisSent = false;
	m_DownloadAllowed = false;
	m_DownloadStarted = false;
	m_DownloadFinished = false;
	m_FinishedLoading = false;
	m_Lagging = false;
	m_DropVote = false;
	m_KickVote = false;
	m_Muted = false;
	m_LeftMessageSent = false;
	m_GProxy = false;
	m_GProxyDisconnectNoticeSent = false;
	m_GProxyReconnectKey = GetTicks( );
	m_LastGProxyAckTime = 0;
	init();
}

void CGamePlayer::EventSpoofCheckTimeout()
{
	// kick players who don't spoof check within 20 seconds when spoof checks are required and the game is autohosted

	if (GetSpoofed())
		return;

	if (m_Game->GetCountDownStarted() || !m_Game->m_GHost->m_RequireSpoofChecks ||
		m_Game->GetGameState() != GAME_PUBLIC || !m_Game->m_GHost->m_AutoHostGameName.isEmpty() ||
		m_Game->m_GHost->m_AutoHostMaximumGames == 0 || m_Game->m_GHost->m_AutoHostAutoStartPlayers == 0 ||
		m_Game->GetAutoStartPlayers() == 0)
		return;

	deleteLater();
	SetLeftReason( m_Game->m_GHost->m_Language->WasKickedForNotSpoofChecking( ) );
	SetLeftCode( PLAYERLEAVE_LOBBY );
	m_Game->OpenSlot( m_Game->GetSIDFromPID( GetPID( ) ), false );
}

void CGamePlayer::init()
{
	QObject::connect(&m_TimeoutTimer, SIGNAL(timeout()), this, SLOT(EventConnectionTimeout()));
	QObject::connect(&m_ACKTimer, SIGNAL(timeout()), this, SLOT(EventACKTimeout()));
	QObject::connect(&m_PingTimer, SIGNAL(timeout()), this, SLOT(EventPingTimeout()));
	QTimer::singleShot(4000, this, SLOT(EventWhoisTimeout()));

	QObject::connect(this, SIGNAL(finishedLoading()), m_Game, SLOT(EventPlayerLoaded()));
	QObject::connect(this, SIGNAL(aboutToDelete()), m_Game, SLOT(EventPlayerDeleted()));
	m_SendGProxyMessageTimer.setInterval(20000);
	QObject::connect(&m_SendGProxyMessageTimer, SIGNAL(timeout()), this, SLOT(EventSendGProxyMessage()));

	QTimer::singleShot(20000, this, SLOT(EventSpoofCheckTimeout()));

	if (m_GProxy)
		m_ACKTimer.start(10000);

	m_PingTimer.start(5000);
	m_TimeoutTimer.start(30000);
	m_TimeoutTimer.setSingleShot(true);
}

CGamePlayer :: ~CGamePlayer( )
{
}

void CGamePlayer::EventSendGProxyMessage()
{
	quint32 TimeRemaining = ( m_Game->GetGProxyEmptyActions() + 1 ) * 60 - ( GetTime( ) - m_Game->GetStartedLaggingTime() );

	if( TimeRemaining > ( (quint32)m_Game->GetGProxyEmptyActions() + 1 ) * 60 )
		TimeRemaining = ( m_Game->GetGProxyEmptyActions() + 1 ) * 60;

	m_Game->SendAllChat( GetPID( ), m_Game->m_GHost->m_Language->WaitForReconnectSecondsRemain( UTIL_ToString( TimeRemaining ) ) );
	SetLastGProxyWaitNoticeSentTime( GetTime( ) );
}

QString CGamePlayer :: GetNameTerminated( )
{
	// if the player's name contains an unterminated colour code add the colour terminator to the end of their name
	// this is useful because it allows you to print the player's name in a longer message which doesn't colour all the subsequent text

	QString LowerName = m_Name.toLower();
	int Start = LowerName.indexOf( "|c" );
	int End = LowerName.indexOf( "|r" );

	if( Start != -1 && ( End == -1 || End < Start ) )
		return m_Name + "|r";
	else
		return m_Name;
}

quint32 CGamePlayer :: GetPing( bool LCPing )
{
	// just average all the pings in the vector, nothing fancy

	if( m_Pings.isEmpty( ) )
		return 0;

	quint32 AvgPing = 0;

	for( int i = 0; i < m_Pings.size( ); i++ )
		AvgPing += m_Pings[i];

	AvgPing /= m_Pings.size( );

	if( LCPing )
		return AvgPing / 2;
	else
		return AvgPing;
}

void CGamePlayer :: ExtractPackets( )
{
	if( !m_Socket || !m_Socket->isValid() )
		return;

	// extract as many packets as possible from the socket's receive buffer and put them in the m_Packets queue
	// a packet is at least 4 bytes so loop as long as the buffer contains 4 bytes

	while( m_Socket->bytesAvailable() >= 4 )
	{
		unsigned char header = m_Socket->peek(1).at(0);
		if (header != W3GS_HEADER_CONSTANT && header != GPS_HEADER_CONSTANT )
		{
			m_Error = true;
			m_ErrorString = "received invalid packet from player (bad header constant)";
			m_Game->EventPlayerDisconnectPlayerError( this );
			m_Socket->abort();
			m_Socket->deleteLater();
			m_Socket = NULL;
			deleteLater();
			return;
		}

		// bytes 2 and 3 contain the length of the packet

		quint16 Length = UTIL_QByteArrayToUInt16( m_Socket->peek(4), false, 2 );

		if( Length < 4 )
		{
			m_Error = true;
			m_ErrorString = "received invalid packet from player (bad length)";
			m_Socket->abort();
			m_Socket->deleteLater();
			m_Socket = NULL;
			return;
		}

		if( m_Socket->bytesAvailable() < Length )
			return;

		QByteArray Bytes = m_Socket->read(Length);
		m_Packets.enqueue( new CCommandPacket( header, Bytes.at(1), Bytes ) );

		if( header == W3GS_HEADER_CONSTANT )
			m_TotalPacketsReceived++;
	}
}

void CGamePlayer :: ProcessPackets( )
{
	if( !m_Socket || !m_Socket->isValid() )
		return;

	CIncomingAction *Action = NULL;
	CIncomingChatPlayer *ChatPlayer = NULL;
	CIncomingMapSize *MapSize = NULL;
	quint32 CheckSum = 0;
	quint32 Pong = 0;

	// process all the received packets in the m_Packets queue

	while( !m_Packets.isEmpty( ) )
	{
		CCommandPacket *Packet = m_Packets.front( );
		m_Packets.dequeue( );

		if( Packet->GetPacketType( ) == W3GS_HEADER_CONSTANT )
		{
			switch( Packet->GetID( ) )
			{
			case CGameProtocol :: W3GS_LEAVEGAME:
				m_Game->EventPlayerLeft( this, m_Protocol->RECEIVE_W3GS_LEAVEGAME( Packet->GetData( ) ) );
				break;

			case CGameProtocol :: W3GS_GAMELOADED_SELF:
				if( m_Protocol->RECEIVE_W3GS_GAMELOADED_SELF( Packet->GetData( ) ) )
				{
					if( !m_FinishedLoading )
					{
						m_FinishedLoading = true;
						m_FinishedLoadingTicks = GetTicks( );
						emit finishedLoading();
					}
					else
					{
						// we received two W3GS_GAMELOADED_SELF packets from this player!
					}
				}

				break;

			case CGameProtocol :: W3GS_OUTGOING_ACTION:
				Action = m_Protocol->RECEIVE_W3GS_OUTGOING_ACTION( Packet->GetData( ), m_PID );

				if( Action )
					m_Game->EventPlayerAction( this, Action );

				// don't delete Action here because the game is going to store it in a queue and delete it later

				break;

			case CGameProtocol :: W3GS_OUTGOING_KEEPALIVE:
				CheckSum = m_Protocol->RECEIVE_W3GS_OUTGOING_KEEPALIVE( Packet->GetData( ) );
				m_CheckSums.enqueue( CheckSum );
				m_SyncCounter++;
				m_Game->EventPlayerKeepAlive( this, CheckSum );
				break;

			case CGameProtocol :: W3GS_CHAT_TO_HOST:
				ChatPlayer = m_Protocol->RECEIVE_W3GS_CHAT_TO_HOST( Packet->GetData( ) );

				if( ChatPlayer )
					m_Game->EventPlayerChatToHost( this, ChatPlayer );

				delete ChatPlayer;
				ChatPlayer = NULL;
				break;

			case CGameProtocol :: W3GS_DROPREQ:
				// todotodo: no idea what's in this packet

				if( !m_DropVote )
				{
					m_DropVote = true;
					m_Game->EventPlayerDropRequest( this );
				}

				break;

			case CGameProtocol :: W3GS_MAPSIZE:
				MapSize = m_Protocol->RECEIVE_W3GS_MAPSIZE( Packet->GetData( ), m_Game->m_GHost->m_Map->GetMapSize( ) );

				if( MapSize )
					m_Game->EventPlayerMapSize( this, MapSize );

				delete MapSize;
				MapSize = NULL;
				break;

			case CGameProtocol :: W3GS_PONG_TO_HOST:
				Pong = m_Protocol->RECEIVE_W3GS_PONG_TO_HOST( Packet->GetData( ) );

				// we discard pong values of 1
				// the client sends one of these when connecting plus we return 1 on error to kill two birds with one stone

				if( Pong != 1 )
				{
					// we also discard pong values when we're downloading because they're almost certainly inaccurate
					// this statement also gives the player a 5 second grace period after downloading the map to allow queued (i.e. delayed) ping packets to be ignored

					if( !m_DownloadStarted || ( m_DownloadFinished && GetTime( ) - m_FinishedDownloadingTime >= 5 ) )
					{
						// we also discard pong values when anyone else is downloading if we're configured to

						if( m_Game->m_GHost->m_PingDuringDownloads || !m_Game->IsDownloading( ) )
						{
							m_Pings.push_back( GetTicks( ) - Pong );

							if( m_Pings.size( ) > 20 )
								m_Pings.erase( m_Pings.begin( ) );
						}
					}
				}

				m_Game->EventPlayerPongToHost( this, Pong );
				break;
			}
		}
		else if( Packet->GetPacketType( ) == GPS_HEADER_CONSTANT )
		{
			QByteArray Data = Packet->GetData( );

			if( Packet->GetID( ) == CGPSProtocol :: GPS_INIT )
			{
				if( m_Game->m_GHost->m_Reconnect )
				{
					m_GProxy = true;
					m_Socket->write( m_Game->m_GHost->m_GPSProtocol->SEND_GPSS_INIT( m_Game->m_GHost->m_ReconnectPort, m_PID, m_GProxyReconnectKey, m_Game->GetGProxyEmptyActions( ) ) );
					CONSOLE_Print( "[GAME: " + m_Game->GetGameName( ) + "] player [" + m_Name + "] is using GProxy++" );
				}
				else
				{
					// todotodo: send notice that we're not permitting reconnects
					// note: GProxy++ should never send a GPS_INIT message if bot_reconnect = 0 because we don't advertise the game with invalid map dimensions
					// but it would be nice to cover this case anyway
				}
			}
			else if( Packet->GetID( ) == CGPSProtocol :: GPS_RECONNECT )
			{
				// this is handled in ghost.cpp
			}
			else if( Packet->GetID( ) == CGPSProtocol :: GPS_ACK && Data.size( ) == 8 )
			{
				int LastPacket = UTIL_QByteArrayToUInt32( Data, false, 4 );
				int PacketsAlreadyUnqueued = m_TotalPacketsSent - m_GProxyBuffer.size( );

				if( LastPacket > PacketsAlreadyUnqueued )
				{
					int PacketsToUnqueue = LastPacket - PacketsAlreadyUnqueued;

					if( PacketsToUnqueue > m_GProxyBuffer.size( ) )
						PacketsToUnqueue = m_GProxyBuffer.size( );

					while( PacketsToUnqueue > 0 )
					{
						m_GProxyBuffer.dequeue( );
						PacketsToUnqueue--;
					}
				}
			}
		}

		delete Packet;
	}
}

void CGamePlayer :: Send( QByteArray data )
{
	// must start counting packet total from beginning of connection
	// but we can avoid buffering packets until we know the client is using GProxy++ since that'll be determined before the game starts
	// this prevents us from buffering packets for non-GProxy++ clients
	//if (data.toHex() != "f70c06006400")
	//	DEBUG_Print("Sending " + data.toHex());

	m_TotalPacketsSent++;

	if( m_GProxy && m_Game->GetGameLoaded( ) )
		m_GProxyBuffer.enqueue( data );

	CPotentialPlayer :: Send( data );
}

void CGamePlayer :: EventGProxyReconnect( QTcpSocket *NewSocket, quint32 LastPacket )
{
	m_Socket->deleteLater();
	m_Socket = NewSocket;

	QObject::connect(m_Socket, SIGNAL(readyRead()), this, SLOT(EventDataReady()));
	QObject::connect(m_Socket, SIGNAL(disconnected()), this, SLOT(EventConnectionClosed()));
	QObject::connect(m_Socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(EventConnectionError(QAbstractSocket::SocketError)));

	m_Socket->write( m_Game->m_GHost->m_GPSProtocol->SEND_GPSS_RECONNECT( m_TotalPacketsReceived ) );

	quint32 PacketsAlreadyUnqueued = m_TotalPacketsSent - m_GProxyBuffer.size( );

	if( LastPacket > PacketsAlreadyUnqueued )
	{
		int PacketsToUnqueue = LastPacket - PacketsAlreadyUnqueued;

		if( PacketsToUnqueue > m_GProxyBuffer.size( ) )
			PacketsToUnqueue = m_GProxyBuffer.size( );

		while( PacketsToUnqueue > 0 )
		{
			m_GProxyBuffer.dequeue( );
			PacketsToUnqueue--;
		}
	}

	// send remaining packets from buffer, preserve buffer

	QQueue<QByteArray> TempBuffer;

	while( !m_GProxyBuffer.isEmpty( ) )
	{
		m_Socket->write( m_GProxyBuffer.front( ) );
		TempBuffer.enqueue( m_GProxyBuffer.front( ) );
		m_GProxyBuffer.dequeue( );
	}

	m_GProxyBuffer = TempBuffer;
	m_GProxyDisconnectNoticeSent = false;
	m_Game->SendAllChat( m_Game->m_GHost->m_Language->PlayerReconnectedWithGProxy( m_Name ) );
}
