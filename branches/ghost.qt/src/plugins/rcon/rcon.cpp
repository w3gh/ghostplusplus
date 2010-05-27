/*
 *      rcon.cpp
 *
 *      Copyright 2008-2010 Lucas Romero
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#include "ghost.h"
#include "config.h"
#include "bnet.h"
#include "game.h"
#include "language.h"
#include <QTextStream>
#include <QString>
#include <QUdpSocket>
#include "rcon.h"
#include "config.h"
#include "gameplayer.h"
#include "bnetprotocol.h"
#include "util.h"
#include "game_base.h"
#include "gameprotocol.h"

CRemoteConsole :: CRemoteConsole( )
{
	CONSOLE_Print( "[RCON] Remote Console 1.5 loading" );
	m_UDPSocket = new QUdpSocket(this);
	connect(m_UDPSocket, SIGNAL(readyRead()),
			this, SLOT(readPendingDatagrams()));
	// lookup "localhost" for later use
	//bool lookup_success = (inet_aton("localhost", &localhost) == 1);
}

CRemoteConsole :: ~CRemoteConsole( )
{
	for( QList<CRemoteConsoleClient *> :: iterator i = m_KnownClients.begin( ); i != m_KnownClients.end( ); i++ )
	{
		delete *i;
	}
}

QString CRemoteConsole :: GetName() const
{
	return "Remote Console";
}

void CRemoteConsole :: PluginLoaded( CGHost *ghost, CConfig *cfg )
{
	m_GHost = ghost;
	QString bindto = cfg->GetString( "rcon_bindto", "255.255.255.255" );
	qint32 port = cfg->GetInt( "rcon_port", 6969 );
	m_Password = cfg->GetString( "rcon_password", "lol" );
	m_Timeout = cfg->GetInt( "rcon_timeout", 10 );
	m_KeepAliveTime = cfg->GetInt( "rcon_keepalivetime", 120 );
	m_KeepAliveInterval = cfg->GetInt( "rcon_keepaliveinterval", 2 );
	m_AnonymousBroadcast = cfg->GetInt( "rcon_anonymousbroadcast", 0 ) == 0 ? false : true;
	m_AnonymousAdminGame = cfg->GetInt( "rcon_anonymousadmingame", 0 ) == 0 ? false : true;
	
	QHostAddress bindAddr( bindto );

	if( m_Password.isEmpty( ) )
	{
		bindAddr = QHostAddress::LocalHost;
		CONSOLE_Print("[RCON] WARNING: No rcon_password set in the .cfg file! Only local commands allowed!");
	}
	// we are all set, lets create our socket


	// TODO: bind correctly
	if( !m_UDPSocket->bind( bindAddr, port ) )
	{
		CONSOLE_Print( "[RCON] ERROR: Could not bind to [" + bindto + "] on port [" + QString :: number( port ) + "]" );
	}
	else
	{
		CONSOLE_Print( "[RCON] Listening at [" + bindto + "] on port [" + QString :: number( port ) + "]" );
		//disabled for now
		/*if( m_AnonymousBroadcast )
			CONSOLE_Print( "[RCON] Anonymous broadcasting [enabled]" );
		else
			CONSOLE_Print( "[RCON] Anonymous broadcasting [disabled]" );
		if( m_AnonymousAdminGame )
			CONSOLE_Print( "[RCON] Anonymous Admin game [enabled]" );
		else
			CONSOLE_Print( "[RCON] Anonymous Admin game [disabled]" );*/
	}
}

void CRemoteConsole :: readPendingDatagrams()
{
	while (m_UDPSocket->hasPendingDatagrams()) {
		QByteArray datagram;
		datagram.resize(m_UDPSocket->pendingDatagramSize());
		QHostAddress sender;
		quint16 senderPort;
		
		m_UDPSocket->readDatagram(datagram.data(), datagram.size(),
								&sender, &senderPort);
		ProcessPacket( datagram, sender, senderPort );
		// echo
		m_UDPSocket->writeDatagram(datagram.data(), sender, senderPort);
	}
}

void CRemoteConsole :: ProcessPacket( const QByteArray &data, const QHostAddress &sender, const quint16 &senderPort )
{
	QString message( data );
	message = message.trimmed();
	
	if (!message.isEmpty())
	{
		QString cmd;
		QString payload;
		QString target;
		SplitLine( message, cmd, payload, target );
		
		CRemoteConsoleClient *client = NULL;
		for( QList<CRemoteConsoleClient *> :: iterator i = m_KnownClients.begin( ); i != m_KnownClients.end( ); i++ )
		{
			if( (*i)->GetAddress( ) == sender && (*i)->GetPort() == senderPort )
			{
				client = *i;
				// update LastReceived time
				//client->Pong( );
				break;
			}
		}
	}
	
	for( QList<CRemoteConsoleClient *> :: iterator i = m_KnownClients.begin( ); i != m_KnownClients.end( ); )
	{
		//(*i)->AutoBroadcast( m_GHost->m_CurrentGame, m_GHost->m_AdminGame );
		
		/*if( GetTime( ) > (*i)->GetLastReceived( ) + m_KeepAliveTime + m_Timeout && (*i)->IsAuthed( ) )
		{
			// user has not responded in time, de-auth him
			(*i)->SetAuthed( false );
			(*i)->Send( "[RCON] Your session timed out" );
			CONSOLE_Print("[RCON] User [" + (*i)->GetIPString( ) + "] timed out");
		}
		else if( GetTime( ) > (*i)->GetLastReceived( ) + m_KeepAliveTime
				&& GetTime( ) > (*i)->GetLastPinged( ) + m_KeepAliveInterval )
		{
			// user has not sent an command in quite some time, send him a keep-alive packet (containing the word "PING")
			(*i)->Ping( );
		}
		
		if ( !(*i)->IsAuthed( ) && !(*i)->IsBanned( ) )
		{
			delete *i;
			i = m_KnownClients.erase( i );
		}
		else
			i++;*/
	}
}

void CRemoteConsole :: LogInfo( const QString &message )
{
	Send( message );
}

void CRemoteConsole :: LogWarning( const QString &message )
{
	Send( message );
}

void CRemoteConsole :: LogError( const QString &message )
{
	Send( message );
}

/*bool CRemoteConsole :: Update( void *fd )
{
	if( !m_Socket )
		return true;
	sockaddr_in recvAddr;
	string recvString;
	m_Socket->RecvFrom( (fd_set *)fd, &recvAddr, &recvString);
	
	if( !recvString.empty( ) )
	{
		// erase newline if found
		if ( recvString[recvString.size()-1] == '\n' )
			recvString.erase( recvString.size()-1, 1 );
		string cmd, payload, target = "";
		SplitLine( recvString, cmd, payload, target );
		CRemoteConsoleClient *client = NULL;
		for( vector<CRemoteConsoleClient *> :: iterator i = m_KnownClients.begin( ); i != m_KnownClients.end( ); i++ )
		{
			if( AreEqual( (*i)->GetEndPoint( ), &recvAddr , true) )
			{
				client = *i;
				// update LastReceived time
				client->Pong( );
				break;
			}
		}
		
		if( !client )
		{
			// client is not known yet
			if( recvAddr.sin_addr.s_addr == localhost.s_addr || ( !m_Password.empty( ) && cmd == "rcon_password" && payload == m_Password ) )
			{
				// client has authed himself
				client = new CRemoteConsoleClient( m_Socket, recvAddr, -1, true );
				client->Send( "[RCON] You are now authed" );
				CONSOLE_Print("[RCON] User [" + client->GetIPString( ) + "] is now authorized");
				m_KnownClients.push_back( client );
			}
			else if( cmd == "rcon_broadcast" && payload.empty( ) )
			{
				bool authorized = false;
				if( !authorized )
					for( vector<CRemoteConsoleClient *> :: iterator i = m_KnownClients.begin( ); i != m_KnownClients.end( ); i++ )
					{
						// only respond if IP is already connected and authed as rcon user
						if( AreEqual( (*i)->GetEndPoint( ), &recvAddr, false ) && (*i)->IsAuthed( ) )
						{
							authorized = true;
							break;
						}
					}
				if( authorized || m_AnonymousBroadcast )
				{
					if( m_GHost->m_CurrentGame )
						m_GHost->m_CurrentGame->SendGame( m_Socket, recvAddr );
					
					if( m_GHost->m_AdminGame && ( authorized || m_AnonymousAdminGame ) )
						m_GHost->m_AdminGame->SendGame( m_Socket, recvAddr );
				}
				else
				{
					// ban client for 5 seconds for not being authorized
					client = new CRemoteConsoleClient( m_Socket, recvAddr, 5 , false);
					m_KnownClients.push_back( client );
					CONSOLE_Print("[RCON] User [" + client->GetIPString( ) + "] is not allowed to receive broadcasts");
				}
			}
			else
			{
				// ban client for 5 seconds for sending an unkown command or the wrong password
				client = new CRemoteConsoleClient( m_Socket, recvAddr, 5, false );
				m_KnownClients.push_back( client );
				CONSOLE_Print("[RCON] User [" + client->GetIPString( ) + "] failed to authenticate");
			}
		}
		else if( client->IsBanned( ) )
		{
			// we have seen this user before, but he is banned
			CONSOLE_Print("[RCON] Banned user [" + client->GetIPString( ) + "] tried to execute command [" + recvString + "]");
		}
		else if( !client->IsAuthed( ) )
		{
			// we have seen this user before, but he hasn't provided the right login credentials yet
			if( !m_Password.empty( ) && cmd == "rcon_password" && payload == m_Password )
			{
				client->Send( "[RCON] You are now authed" );
				CONSOLE_Print("[RCON] User [" + client->GetIPString( ) + "] is now authorized");
				client->SetAuthed( true );
			}
			else
			{
				// ban client for 5 seconds for sending an unkown command or the wrong password
				client->Ban( 5 );
				CONSOLE_Print("[RCON] User [" + client->GetIPString( ) + "] failed to authenticate");
			}
		}
		else if( cmd == "rcon_password" )
		{
			// client is already authed and tries to auth again, just tell him he is now authed
			client->Send( "[RCON] You are now authed" );
		}
		else if( cmd == "rcon_sendlobbychat" )
		{
			if ( m_GHost->m_CurrentGame && !m_GHost->m_CurrentGame->GetGameLoading( ) && !m_GHost->m_CurrentGame->GetGameLoaded( ) )
				m_GHost->m_CurrentGame->SendAllChat( payload );
		}
		else if( cmd == "rcon_kick" )
		{
			if ( m_GHost->m_CurrentGame && !m_GHost->m_CurrentGame->GetGameLoading( ) && !m_GHost->m_CurrentGame->GetGameLoaded( ) )
			{
				CGamePlayer *player = m_GHost->m_CurrentGame->GetPlayerFromName( payload, true );
				if( player )
				{
					player->SetDeleteMe( true );
					player->SetLeftReason( m_GHost->m_Language->WasKickedByPlayer( "Remote Console" ) );
					player->SetLeftCode( PLAYERLEAVE_LOBBY );
					m_GHost->m_CurrentGame->OpenSlot( m_GHost->m_CurrentGame->GetSIDFromPID( player->GetPID( ) ), false );
				}
			}
		}
		// disabled for now
		//else if( cmd == "rcon_gamedetails" )
		//{
		//	if( m_GHost->m_CurrentGame && !m_GHost->m_CurrentGame->GetGameLoading( ) && !m_GHost->m_CurrentGame->GetGameLoaded( ) )
		//		client->Send( "[RCON] Game details: " + m_GHost->m_CurrentGame->GetGameDetails( ) );
		//	else
		//		client->Send( "[RCON] Game details: no game in lobby" );
		//}
		//else
		if( !cmd.empty( ) )
		{
			// this is a legitimate user, do what he says
			ProcessInput( cmd, payload, target, client );
		}
	}
	
	for( vector<CRemoteConsoleClient *> :: iterator i = m_KnownClients.begin( ); i != m_KnownClients.end( ); )
	{
		//(*i)->AutoBroadcast( m_GHost->m_CurrentGame, m_GHost->m_AdminGame );
		
		if( GetTime( ) > (*i)->GetLastReceived( ) + m_KeepAliveTime + m_Timeout && (*i)->IsAuthed( ) )
		{
			// user has not responded in time, de-auth him
			(*i)->SetAuthed( false );
			(*i)->Send( "[RCON] Your session timed out" );
			CONSOLE_Print("[RCON] User [" + (*i)->GetIPString( ) + "] timed out");
		}
		else if( GetTime( ) > (*i)->GetLastReceived( ) + m_KeepAliveTime
				&& GetTime( ) > (*i)->GetLastPinged( ) + m_KeepAliveInterval )
		{
			// user has not sent an command in quite some time, send him a keep-alive packet (containing the word "PING")
			(*i)->Ping( );
		}
		
		if ( !(*i)->IsAuthed( ) && !(*i)->IsBanned( ) )
		{
			delete *i;
			i = m_KnownClients.erase( i );
		}
		else
			i++;
	}
	
	return false;
}*/

void CRemoteConsole :: SplitLine( const QString &input, QString &command, QString &payload, QString &target )
{
	int pos;
	QString Message = input;
	// has the user specified a specific target the command should be sent to?
	// looks for "<someaddress>" at the beginning of the received command,
	// sets the target accordingly and strips it from the command
	if ( input.indexOf("<") == 0 && (pos=input.indexOf(">")) != -1 )
	{
		target = input.mid(1, pos - 1);
		Message = input.mid( pos+1 );
		//input.erase(0, pos + 1);
	}
	// we expect commands not to start with the command trigger because this is a commandsocket,
	// we only except commands and therefore know we received one and not some chatting
	// this way the user sending the command does not have to have knowledge of the commandtrigger
	// set in GHost's config file
	
	
	int PayloadStart = Message.indexOf( " " );
	
	if( PayloadStart != -1 )
	{
		command = Message.mid( 0, PayloadStart );
		payload = Message.mid( PayloadStart + 1 );
	}
	else
		command = Message;
}

/*void CRemoteConsole :: ProcessInput( string cmd, string payload, string target, CRemoteConsoleClient *sender )
{
	// default server to relay the message to
	bool relayed = false;
	
	// loop through all connections to find the server the command should be issued on
	for( vector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
	{
		// is this the right one or should we just send it to the first in list?
		if ( target == (*i)->GetServer( ) || target.empty() )
		{
			// don't be so verbose!
			//CONSOLE_Print("[RCON] Relaying command [" + cmd + "] with payload [" + payload + "] to server [" + (*i)->GetServer( ) + "]");
			// spoof a whisper from the rootadmin belonging to this connection
			string msg = (*i)->GetCommandTrigger( ) + cmd;
			if( !payload.empty( ) )
				msg += " " + payload;
			CIncomingChatEvent chatCommand( CBNETProtocol::EID_WHISPER, 0, (*i)->GetRootAdmin( ), msg);
			(*i)->ProcessChatEvent( &chatCommand );
			relayed = true;
			break;
		}
	}
	if (!relayed)
		CONSOLE_Print("[RCON] Could not relay cmd [" + cmd + "] with payload [" + payload + "] to server [" + target + "]: server unknown");
}*/

void CRemoteConsole :: Send( const QString &message )
{
	QByteArray datagram;
	QDataStream out( &datagram, QIODevice::WriteOnly );
	out << message;
	for( QList<CRemoteConsoleClient *> :: iterator i = m_KnownClients.begin( ); i != m_KnownClients.end( ); i++ )
	{
		// relay message to all unbanned clients
		if( !(*i)->IsBanned( ) && (*i)->IsAuthed( ) )
			m_UDPSocket->writeDatagram( datagram, (*i)->GetAddress( ), (*i)->GetPort( ) );
	}
}

/*bool CRemoteConsole :: AreEqual( const struct sockaddr_in *a, const struct sockaddr_in *b, bool checkports )
{
	return a->sin_family == b->sin_family &&
	a->sin_addr.s_addr == b->sin_addr.s_addr &&
	( !checkports || a->sin_port == b->sin_port );
}*/

CRemoteConsoleClient :: CRemoteConsoleClient( const CRemoteConsole &rcon, const QHostAddress &address, quint16 port, int banTime, bool authed ) :
m_RCon( rcon ),
m_Address( address ),
m_Port( port ),
m_LastReceived( 0 ),
m_LastPinged( 0 ),
m_Authed( authed )
{
	Ban( banTime );
}

Q_EXPORT_PLUGIN2(cmd_builtin, CRemoteConsole)
