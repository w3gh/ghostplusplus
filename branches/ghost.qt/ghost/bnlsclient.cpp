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
#include "socket.h"
#include "commandpacket.h"
#include "bnlsprotocol.h"
#include "bnlsclient.h"

//
// CBNLSClient
//

CBNLSClient :: CBNLSClient( QString nServer, uint16_t nPort, uint32_t nWardenCookie )
{
	m_Socket = new CTCPClient( );
	m_Protocol = new CBNLSProtocol( );
	m_WasConnected = false;
	m_Server = nServer;
	m_Port = nPort;
	m_LastNullTime = 0;
	m_WardenCookie = nWardenCookie;
	m_TotalWardenIn = 0;
	m_TotalWardenOut = 0;
}

CBNLSClient :: ~CBNLSClient( )
{
	delete m_Socket;
	delete m_Protocol;

	while( !m_Packets.isEmpty( ) )
	{
		delete m_Packets.front( );
		m_Packets.dequeue( );
	}
}

QByteArray CBNLSClient :: GetWardenResponse( )
{
	QByteArray WardenResponse;

	if( !m_WardenResponses.isEmpty( ) )
	{
		WardenResponse = m_WardenResponses.front( );
		m_WardenResponses.dequeue( );
		m_TotalWardenOut++;
	}

	return WardenResponse;
}

unsigned int CBNLSClient :: SetFD( void *fd, void *send_fd, int *nfds )
{
	if( !m_Socket->HasError( ) && m_Socket->GetConnected( ) )
	{
		m_Socket->SetFD( (fd_set *)fd, (fd_set *)send_fd, nfds );
		return 1;
	}

	return 0;
}

bool CBNLSClient :: Update( void *fd, void *send_fd )
{
	if( m_Socket->HasError( ) )
	{
		CONSOLE_Print( "[BNLSC: " + m_Server + ":" + UTIL_ToString( m_Port ) + ":C" + UTIL_ToString( m_WardenCookie ) + "] disconnected from BNLS server due to socket error" );
		return true;
	}

	if( !m_Socket->GetConnecting( ) && !m_Socket->GetConnected( ) && m_WasConnected )
	{
		CONSOLE_Print( "[BNLSC: " + m_Server + ":" + UTIL_ToString( m_Port ) + ":C" + UTIL_ToString( m_WardenCookie ) + "] disconnected from BNLS server" );
		return true;
	}

	if( m_Socket->GetConnected( ) )
	{
		m_Socket->DoRecv( (fd_set *)fd );
		ExtractPackets( );
		ProcessPackets( );

		if( GetTime( ) - m_LastNullTime >= 50 )
		{
			m_Socket->PutBytes( m_Protocol->SEND_BNLS_NULL( ) );
			m_LastNullTime = GetTime( );
		}

		while( !m_OutPackets.isEmpty( ) )
		{
			m_Socket->PutBytes( m_OutPackets.front( ) );
			m_OutPackets.dequeue( );
		}

		m_Socket->DoSend( (fd_set *)send_fd );
		return false;
	}

	if( m_Socket->GetConnecting( ) && m_Socket->CheckConnect( ) )
	{
		CONSOLE_Print( "[BNLSC: " + m_Server + ":" + UTIL_ToString( m_Port ) + ":C" + UTIL_ToString( m_WardenCookie ) + "] connected" );
		m_WasConnected = true;
		m_LastNullTime = GetTime( );
		return false;
	}

	if( !m_Socket->GetConnecting( ) && !m_Socket->GetConnected( ) && !m_WasConnected )
	{
		CONSOLE_Print( "[BNLSC: " + m_Server + ":" + UTIL_ToString( m_Port ) + ":C" + UTIL_ToString( m_WardenCookie ) + "] connecting to server [" + m_Server + "] on port " + UTIL_ToString( m_Port ) );
		m_Socket->Connect( QString( ), m_Server, m_Port );
		return false;
	}

	return false;
}

void CBNLSClient :: ExtractPackets( )
{
	QString *RecvBuffer = m_Socket->GetBytes( );
	QByteArray Bytes = UTIL_CreateQByteArray( (unsigned char *)RecvBuffer->c_str( ), RecvBuffer->size( ) );

	while( Bytes.size( ) >= 3 )
	{
		uint16_t Length = UTIL_QByteArrayToUInt16( Bytes, false );

		if( Length >= 3 )
		{
			if( Bytes.size( ) >= Length )
			{
				m_Packets.enqueue( new CCommandPacket( 0, Bytes[2], QByteArray( Bytes.begin( ), Bytes.begin( ) + Length ) ) );
				*RecvBuffer = RecvBuffer->substr( Length );
				Bytes = QByteArray( Bytes.begin( ) + Length, Bytes.end( ) );
			}
			else
				return;
		}
		else
		{
			CONSOLE_Print( "[BNLSC: " + m_Server + ":" + UTIL_ToString( m_Port ) + ":C" + UTIL_ToString( m_WardenCookie ) + "] error - received invalid packet from BNLS server (bad length), disconnecting" );
			m_Socket->Disconnect( );
			return;
		}
	}
}

void CBNLSClient :: ProcessPackets( )
{
	while( !m_Packets.isEmpty( ) )
	{
		CCommandPacket *Packet = m_Packets.front( );
		m_Packets.dequeue( );

		if( Packet->GetID( ) == CBNLSProtocol :: BNLS_WARDEN )
		{
			QByteArray WardenResponse = m_Protocol->RECEIVE_BNLS_WARDEN( Packet->GetData( ) );

			if( !WardenResponse.isEmpty( ) )
				m_WardenResponses.enqueue( WardenResponse );
		}

		delete Packet;
	}
}

void CBNLSClient :: QueueWardenSeed( uint32_t seed )
{
	m_OutPackets.enqueue( m_Protocol->SEND_BNLS_WARDEN_SEED( m_WardenCookie, seed ) );
}

void CBNLSClient :: QueueWardenRaw( QByteArray wardenRaw )
{
	m_OutPackets.enqueue( m_Protocol->SEND_BNLS_WARDEN_RAW( m_WardenCookie, wardenRaw ) );
	m_TotalWardenIn++;
}
