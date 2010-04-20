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
	m_Socket = new QTcpSocket( );

	QObject::connect(m_Socket, SIGNAL(connected()), this, SLOT(socketConnected()));
	QObject::connect(m_Socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
	QObject::connect(m_Socket, SIGNAL(readyRead()), this, SLOT(socketDataReady()));
	QObject::connect(m_Socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError()));
	QObject::connect(&m_NULLTimer, SIGNAL(timeout()), this, SLOT(timeout_NULL()));

	m_NULLTimer.setInterval(50000);

	m_Protocol = new CBNLSProtocol( );
	m_WasConnected = false;
	m_Server = nServer;
	m_Port = nPort;
	m_LastNullTime = 0;
	m_WardenCookie = nWardenCookie;
	m_TotalWardenIn = 0;
	m_TotalWardenOut = 0;
	m_Retries = 0;
}

void CBNLSClient::socketConnect()
{
	CONSOLE_Print( "[BNLSC: " + m_Server + ":" + UTIL_ToString( m_Port ) + ":C" + UTIL_ToString( m_WardenCookie ) + "] connecting to server [" + m_Server + "] on port " + UTIL_ToString( m_Port ) );
	m_Socket->connectToHost( QHostAddress(m_Server), m_Port );
	m_Retries++;
}

void CBNLSClient::socketConnected()
{
	CONSOLE_Print( "[BNLSC: " + m_Server + ":" + UTIL_ToString( m_Port ) + ":C" + UTIL_ToString( m_WardenCookie ) + "] connected" );
	m_WasConnected = true;
	m_NULLTimer.start();
}

void CBNLSClient::socketDisconnected()
{
	CONSOLE_Print( "[BNLSC: " + m_Server + ":" + UTIL_ToString( m_Port ) + ":C" + UTIL_ToString( m_WardenCookie ) + "] disconnected from BNLS server" );
	m_Socket->deleteLater();
	m_Socket = new QTcpSocket();

	if (m_Retries > 6)
	{
		CONSOLE_Print("[BNLSC: " + m_Server + ":" + UTIL_ToString( m_Port ) + ":C" + UTIL_ToString( m_WardenCookie ) + "] giving up after 5 failed retries." );
		deleteLater();
		return;
	}

	QTimer::singleShot(5000, this, SLOT(socketConnect()));

}

void CBNLSClient::timeout_NULL()
{
	m_Socket->write( m_Protocol->SEND_BNLS_NULL( ) );
}

void CBNLSClient::socketDataReady()
{
	ExtractPackets( );
	ProcessPackets( );

	while( !m_OutPackets.isEmpty( ) )
		m_Socket->write( m_OutPackets.dequeue( ) );
}

void CBNLSClient::socketError()
{
	CONSOLE_Print( "[BNLSC: " + m_Server + ":" + UTIL_ToString( m_Port ) + ":C" + UTIL_ToString( m_WardenCookie ) + "] disconnected from BNLS server due to socket error" );
	m_Socket->deleteLater();
	m_Socket = NULL;
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

void CBNLSClient :: ExtractPackets( )
{
	while( m_Socket->bytesAvailable() >= 3 )
	{
		uint16_t Length = UTIL_QByteArrayToUInt16( m_Socket->peek(2), false );

		if( Length < 3 )
		{
			CONSOLE_Print( "[BNLSC: " + m_Server + ":" + UTIL_ToString( m_Port ) + ":C" + UTIL_ToString( m_WardenCookie ) + "] error - received invalid packet from BNLS server (bad length), disconnecting" );
			m_Socket->abort();
			m_Socket->deleteLater();
			return;
		}

		if( m_Socket->bytesAvailable() < Length )
			return;

		QByteArray Bytes = m_Socket->read(Length);
		m_Packets.enqueue( new CCommandPacket( 0, Bytes.at(2), Bytes ) );
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
				emit newWardenResponse(WardenResponse);
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
