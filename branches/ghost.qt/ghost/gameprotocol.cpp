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
#include "crc32.h"
#include "gameplayer.h"
#include "gameprotocol.h"
#include "game_base.h"

#include <QByteArray>
//
// CGameProtocol
//

CGameProtocol :: CGameProtocol( CGHost *nGHost )
{
	m_GHost = nGHost;
}

CGameProtocol :: ~CGameProtocol( )
{

}

///////////////////////
// RECEIVE FUNCTIONS //
///////////////////////

CIncomingJoinPlayer *CGameProtocol :: RECEIVE_W3GS_REQJOIN( QByteArray data )
{
	// DEBUG_Print( "RECEIVED W3GS_REQJOIN" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> Host Counter (Game ID)
	// 4 bytes					-> Entry Key (used in LAN)
	// 1 byte					-> ???
	// 2 bytes					-> Listen Port
	// 4 bytes					-> Peer Key
	// null terminated QString	-> Name
	// 4 bytes					-> ???
	// 2 bytes					-> InternalPort (???)
	// 4 bytes					-> InternalIP

	if( ValidateLength( data ) && data.size( ) >= 20 )
	{
		quint32 HostCounter = UTIL_QByteArrayToUInt32( data, false, 4 );
		QByteArray Name = UTIL_ExtractCString( data, 19 );

		if( !Name.isEmpty( ) && data.size( ) >= Name.size( ) + 30 )
		{
			QByteArray InternalIP = data.mid(Name.size() + 26, 4);
			return new CIncomingJoinPlayer( HostCounter, Name, InternalIP );
		}
	}

	return NULL;
}

quint32 CGameProtocol :: RECEIVE_W3GS_LEAVEGAME( QByteArray data )
{
	// DEBUG_Print( "RECEIVED W3GS_LEAVEGAME" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> Reason

	if( ValidateLength( data ) && data.size( ) >= 8 )
		return UTIL_QByteArrayToUInt32( data, false, 4 );

	return 0;
}

bool CGameProtocol :: RECEIVE_W3GS_GAMELOADED_SELF( QByteArray data )
{
	// DEBUG_Print( "RECEIVED W3GS_GAMELOADED_SELF" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length

	if( ValidateLength( data ) )
		return true;

	return false;
}

CIncomingAction *CGameProtocol :: RECEIVE_W3GS_OUTGOING_ACTION( QByteArray data, unsigned char PID )
{
	// DEBUG_Print( "RECEIVED W3GS_OUTGOING_ACTION" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> CRC
	// remainder of packet		-> Action

	if( PID != 255 && ValidateLength( data ) && data.size( ) >= 8 )
	{
		QByteArray CRC = data.mid(4, 4);
		QByteArray Action = data.mid(8);
		return new CIncomingAction( PID, CRC, Action );
	}

	return NULL;
}

quint32 CGameProtocol :: RECEIVE_W3GS_OUTGOING_KEEPALIVE( QByteArray data )
{
	// DEBUG_Print( "RECEIVED W3GS_OUTGOING_KEEPALIVE" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 1 byte					-> ???
	// 4 bytes					-> CheckSum??? (used in replays)

	if( ValidateLength( data ) && data.size( ) == 9 )
		return UTIL_QByteArrayToUInt32( data, false, 5 );

	return 0;
}

CIncomingChatPlayer *CGameProtocol :: RECEIVE_W3GS_CHAT_TO_HOST( QByteArray data )
{
	// DEBUG_Print( "RECEIVED W3GS_CHAT_TO_HOST" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 1 byte					-> Total
	// for( 1 .. Total )
	//		1 byte				-> ToPID
	// 1 byte					-> FromPID
	// 1 byte					-> Flag
	// if( Flag == 16 )
	//		null term QString	-> Message
	// elseif( Flag == 17 )
	//		1 byte				-> Team
	// elseif( Flag == 18 )
	//		1 byte				-> Colour
	// elseif( Flag == 19 )
	//		1 byte				-> Race
	// elseif( Flag == 20 )
	//		1 byte				-> Handicap
	// elseif( Flag == 32 )
	//		4 bytes				-> ExtraFlags
	//		null term QString	-> Message

	if( ValidateLength( data ) )
	{
		unsigned int i = 5;
		unsigned char Total = data[4];

		if( Total > 0 && (unsigned int)data.size( ) >= i + Total )
		{
			QByteArray ToPIDs = data.mid(i, Total );
			i += Total;
			unsigned char FromPID = data[i];
			unsigned char Flag = data[i + 1];
			i += 2;

			if( Flag == 16 && (unsigned int)data.size( ) >= i + 1 )
			{
				// chat message

				QByteArray Message = UTIL_ExtractCString( data, i );
				return new CIncomingChatPlayer( FromPID, ToPIDs, Flag, QString::fromUtf8(Message) );
			}
			else if( ( Flag >= 17 && Flag <= 20 ) && (unsigned int)data.size( ) >= i + 1 )
			{
				// team/colour/race/handicap change request

				unsigned char Byte = data[i];
				return new CIncomingChatPlayer( FromPID, ToPIDs, Flag, Byte );
			}
			else if( Flag == 32 && (unsigned int)data.size( ) >= i + 5 )
			{
				// chat message with extra flags

				QByteArray ExtraFlags = data.mid(i, 4);
				QByteArray Message = UTIL_ExtractCString( data, i + 4 );
				return new CIncomingChatPlayer( FromPID, ToPIDs, Flag, QString::fromUtf8(Message), ExtraFlags );
			}
		}
	}

	return NULL;
}

bool CGameProtocol :: RECEIVE_W3GS_SEARCHGAME( QByteArray data, unsigned char war3Version )
{
	quint32 ProductID	= 1462982736;	// "W3XP"
	quint32 Version	= war3Version;

	// DEBUG_Print( "RECEIVED W3GS_SEARCHGAME" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> ProductID
	// 4 bytes					-> Version
	// 4 bytes					-> ??? (Zero)

	if( ValidateLength( data ) && data.size( ) >= 16 )
	{
		if( UTIL_QByteArrayToUInt32( data, false, 4 ) == ProductID )
		{
			if( UTIL_QByteArrayToUInt32( data, false, 8 ) == Version )
			{
				if( UTIL_QByteArrayToUInt32( data, false, 12 ) == 0 )
					return true;
			}
		}
	}

	return false;
}

CIncomingMapSize *CGameProtocol :: RECEIVE_W3GS_MAPSIZE( QByteArray data, QByteArray /*mapSize*/ )
{
	// DEBUG_Print( "RECEIVED W3GS_MAPSIZE" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> ???
	// 1 byte					-> SizeFlag (1 = have map, 3 = continue download)
	// 4 bytes					-> MapSize

	if( ValidateLength( data ) && data.size( ) >= 13 )
		return new CIncomingMapSize( data[8], UTIL_QByteArrayToUInt32( data, false, 9 ) );

	return NULL;
}

quint32 CGameProtocol :: RECEIVE_W3GS_MAPPARTOK( QByteArray data )
{
	// DEBUG_Print( "RECEIVED W3GS_MAPPARTOK" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 1 byte					-> SenderPID
	// 1 byte					-> ReceiverPID
	// 4 bytes					-> ???
	// 4 bytes					-> MapSize

	if( ValidateLength( data ) && data.size( ) >= 14 )
		return UTIL_QByteArrayToUInt32( data, false, 10 );

	return 0;
}

quint32 CGameProtocol :: RECEIVE_W3GS_PONG_TO_HOST( QByteArray data )
{
	// DEBUG_Print( "RECEIVED W3GS_PONG_TO_HOST" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> Pong

	// the pong value is just a copy of whatever was sent in SEND_W3GS_PING_FROM_HOST which was GetTicks( ) at the time of sending
	// so as long as we trust that the client isn't trying to fake us out and mess with the pong value we can find the round trip time by simple subtraction
	// (the subtraction is done elsewhere because the very first pong value seems to be 1 and we want to discard that one)

	if( ValidateLength( data ) && data.size( ) >= 8 )
		return UTIL_QByteArrayToUInt32( data, false, 4 );

	return 1;
}

////////////////////
// SEND FUNCTIONS //
////////////////////

QByteArray CGameProtocol :: SEND_W3GS_PING_FROM_HOST( )
{
	QByteArray packet;
	packet.push_back( W3GS_HEADER_CONSTANT );				// W3GS header constant
	packet.push_back( W3GS_PING_FROM_HOST );				// W3GS_PING_FROM_HOST
	packet.push_back( (char)0 );									// packet length will be assigned later
	packet.push_back( (char)0 );									// packet length will be assigned later
	UTIL_AppendBYTEARRAY( packet, GetTicks( ), false );		// ping value
	AssignLength( packet );
	// DEBUG_Print( "SENT W3GS_PING_FROM_HOST" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CGameProtocol :: SEND_W3GS_SLOTINFOJOIN( unsigned char PID, QByteArray port, QByteArray externalIP, QVector<CGameSlot> &lslots, quint32 randomSeed, unsigned char layoutStyle, unsigned char playerSlots )
{
	unsigned char Zeros[] = { 0, 0, 0, 0 };

	QByteArray SlotInfo = EncodeSlotInfo( lslots, randomSeed, layoutStyle, playerSlots );
	QByteArray packet;

	if( port.size( ) == 2 && externalIP.size( ) == 4 )
	{
		packet.push_back( W3GS_HEADER_CONSTANT );									// W3GS header constant
		packet.push_back( W3GS_SLOTINFOJOIN );										// W3GS_SLOTINFOJOIN
		packet.push_back( (char)0 );														// packet length will be assigned later
		packet.push_back( (char)0 );														// packet length will be assigned later
		UTIL_AppendBYTEARRAY( packet, (quint16)SlotInfo.size( ), false );			// SlotInfo length
		UTIL_AppendBYTEARRAYFast( packet, SlotInfo );								// SlotInfo
		packet.push_back( PID );													// PID
		packet.push_back( 2 );														// AF_INET
		packet.push_back( (char)0 );														// AF_INET continued...
		UTIL_AppendBYTEARRAY( packet, port );										// port
		UTIL_AppendBYTEARRAYFast( packet, externalIP );								// external IP
		UTIL_AppendBYTEARRAY( packet, Zeros, 4 );									// ???
		UTIL_AppendBYTEARRAY( packet, Zeros, 4 );									// ???
		AssignLength( packet );
	}
	else
		CONSOLE_Print( "[GAMEPROTO] invalid parameters passed to SEND_W3GS_SLOTINFOJOIN (" +
					   QString::number(port.size()) + ": " + port.toHex() + ", " +
					   QString::number(externalIP.size()) + ": " + externalIP.toHex() + ")");

	// DEBUG_Print( "SENT W3GS_SLOTINFOJOIN" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CGameProtocol :: SEND_W3GS_REJECTJOIN( quint32 reason )
{
	QByteArray packet;
	packet.push_back( W3GS_HEADER_CONSTANT );				// W3GS header constant
	packet.push_back( W3GS_REJECTJOIN );					// W3GS_REJECTJOIN
	packet.push_back( (char)0 );									// packet length will be assigned later
	packet.push_back( (char)0 );									// packet length will be assigned later
	UTIL_AppendBYTEARRAY( packet, reason, false );			// reason
	AssignLength( packet );
	// DEBUG_Print( "SENT W3GS_REJECTJOIN" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CGameProtocol :: SEND_W3GS_PLAYERINFO( unsigned char PID, QString name, QByteArray externalIP, QByteArray internalIP )
{
	unsigned char PlayerJoinCounter[]	= { 2, 0, 0, 0 };
	unsigned char Zeros[]				= { 0, 0, 0, 0 };

	QByteArray packet;

	if( !name.isEmpty( ) && name.size( ) <= 15 && externalIP.size( ) == 4 && internalIP.size( ) == 4 )
	{
		packet.push_back( W3GS_HEADER_CONSTANT );							// W3GS header constant
		packet.push_back( W3GS_PLAYERINFO );								// W3GS_PLAYERINFO
		packet.push_back( (char)0 );												// packet length will be assigned later
		packet.push_back( (char)0 );												// packet length will be assigned later
		UTIL_AppendBYTEARRAY( packet, PlayerJoinCounter, 4 );				// player join counter
		packet.push_back( PID );											// PID
		UTIL_AppendBYTEARRAYFast( packet, name );							// player name
		packet.push_back( 1 );												// ???
		packet.push_back( (char)0 );												// ???
		packet.push_back( 2 );												// AF_INET
		packet.push_back( (char)0 );												// AF_INET continued...
		packet.push_back( (char)0 );												// port
		packet.push_back( (char)0 );												// port continued...
		UTIL_AppendBYTEARRAYFast( packet, externalIP );						// external IP
		UTIL_AppendBYTEARRAY( packet, Zeros, 4 );							// ???
		UTIL_AppendBYTEARRAY( packet, Zeros, 4 );							// ???
		packet.push_back( 2 );												// AF_INET
		packet.push_back( (char)0 );												// AF_INET continued...
		packet.push_back( (char)0 );												// port
		packet.push_back( (char)0 );												// port continued...
		UTIL_AppendBYTEARRAYFast( packet, internalIP );						// internal IP
		UTIL_AppendBYTEARRAY( packet, Zeros, 4 );							// ???
		UTIL_AppendBYTEARRAY( packet, Zeros, 4 );							// ???
		AssignLength( packet );
	}
	else
		CONSOLE_Print( "[GAMEPROTO] invalid parameters passed to SEND_W3GS_PLAYERINFO" );

	// DEBUG_Print( "SENT W3GS_PLAYERINFO" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CGameProtocol :: SEND_W3GS_PLAYERLEAVE_OTHERS( unsigned char PID, quint32 leftCode )
{
	QByteArray packet;

	if( PID != 255 )
	{
		packet.push_back( W3GS_HEADER_CONSTANT );			// W3GS header constant
		packet.push_back( W3GS_PLAYERLEAVE_OTHERS );		// W3GS_PLAYERLEAVE_OTHERS
		packet.push_back( (char)0 );								// packet length will be assigned later
		packet.push_back( (char)0 );								// packet length will be assigned later
		packet.push_back( PID );							// PID
		UTIL_AppendBYTEARRAY( packet, leftCode, false );	// left code (see PLAYERLEAVE_ constants in gameprotocol.h)
		AssignLength( packet );
	}
	else
		CONSOLE_Print( "[GAMEPROTO] invalid parameters passed to SEND_W3GS_PLAYERLEAVE_OTHERS" );

	// DEBUG_Print( "SENT W3GS_PLAYERLEAVE_OTHERS" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CGameProtocol :: SEND_W3GS_GAMELOADED_OTHERS( unsigned char PID )
{
	QByteArray packet;

	if( PID != 255 )
	{
		packet.push_back( W3GS_HEADER_CONSTANT );		// W3GS header constant
		packet.push_back( W3GS_GAMELOADED_OTHERS );		// W3GS_GAMELOADED_OTHERS
		packet.push_back( (char)0 );							// packet length will be assigned later
		packet.push_back( (char)0 );							// packet length will be assigned later
		packet.push_back( PID );						// PID
		AssignLength( packet );
	}
	else
		CONSOLE_Print( "[GAMEPROTO] invalid parameters passed to SEND_W3GS_GAMELOADED_OTHERS" );

	// DEBUG_Print( "SENT W3GS_GAMELOADED_OTHERS" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CGameProtocol :: SEND_W3GS_SLOTINFO( QVector<CGameSlot> &lslots, quint32 randomSeed, unsigned char layoutStyle, unsigned char playerSlots )
{
	QByteArray SlotInfo = EncodeSlotInfo( lslots, randomSeed, layoutStyle, playerSlots );
	QByteArray packet;

	// f709 1900 1300 0c00      0000 0000 0000 0000 0000 0000 0000 0000 0c
	// f709 1c00 1600 0c01 ff02 0000 0000 0000 0000 0000 0000 0000 0000 000c
	packet.push_back( W3GS_HEADER_CONSTANT );									// W3GS header constant
	packet.push_back( W3GS_SLOTINFO );											// W3GS_SLOTINFO
	packet.push_back( (char)0 );														// packet length will be assigned later
	packet.push_back( (char)0 );														// packet length will be assigned later
	UTIL_AppendBYTEARRAY( packet, (quint16)SlotInfo.size( ), false );			// SlotInfo length
	UTIL_AppendBYTEARRAYFast( packet, SlotInfo );								// SlotInfo
	AssignLength( packet );
	// DEBUG_Print( "SENT W3GS_SLOTINFO" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CGameProtocol :: SEND_W3GS_COUNTDOWN_START( )
{
	QByteArray packet;
	packet.push_back( W3GS_HEADER_CONSTANT );		// W3GS header constant
	packet.push_back( W3GS_COUNTDOWN_START );		// W3GS_COUNTDOWN_START
	packet.push_back( (char)0 );							// packet length will be assigned later
	packet.push_back( (char)0 );							// packet length will be assigned later
	AssignLength( packet );
	// DEBUG_Print( "SENT W3GS_COUNTDOWN_START" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CGameProtocol :: SEND_W3GS_COUNTDOWN_END( )
{
	QByteArray packet;
	packet.push_back( W3GS_HEADER_CONSTANT );		// W3GS header constant
	packet.push_back( W3GS_COUNTDOWN_END );			// W3GS_COUNTDOWN_END
	packet.push_back( (char)0 );							// packet length will be assigned later
	packet.push_back( (char)0 );							// packet length will be assigned later
	AssignLength( packet );
	// DEBUG_Print( "SENT W3GS_COUNTDOWN_END" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CGameProtocol :: SEND_W3GS_INCOMING_ACTION( QQueue<CIncomingAction *> actions, quint16 sendInterval )
{
	QByteArray packet;
	packet.push_back( W3GS_HEADER_CONSTANT );				// W3GS header constant
	packet.push_back( W3GS_INCOMING_ACTION );				// W3GS_INCOMING_ACTION
	packet.push_back( (char)0 );									// packet length will be assigned later
	packet.push_back( (char)0 );									// packet length will be assigned later
	UTIL_AppendBYTEARRAY( packet, sendInterval, false );	// send interval

	// create subpacket

	if( !actions.isEmpty( ) )
	{
		QByteArray subpacket;

		while( !actions.isEmpty( ) )
		{
			CIncomingAction *Action = actions.dequeue( );

			subpacket.push_back( Action->GetPID( ) );
			UTIL_AppendBYTEARRAY( subpacket, (quint16)Action->GetAction( )->size( ), false );
			UTIL_AppendBYTEARRAYFast( subpacket, *Action->GetAction( ) );
		}

		// calculate crc (we only care about the first 2 bytes though)

		QByteArray crc32 = UTIL_CreateBYTEARRAY( m_GHost->m_CRC->FullCRC( subpacket ), false );
		crc32.resize( 2 );
		DEBUG_Print("Calculating CRC for " + subpacket.toHex() + ":  " + crc32.toHex());

		// finish subpacket

		UTIL_AppendBYTEARRAYFast( packet, crc32 );			// crc
		UTIL_AppendBYTEARRAYFast( packet, subpacket );		// subpacket
	}

	AssignLength( packet );
	// DEBUG_Print( "SENT W3GS_INCOMING_ACTION" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CGameProtocol :: SEND_W3GS_CHAT_FROM_HOST( unsigned char fromPID, QByteArray toPIDs, unsigned char flag, QByteArray flagExtra, QString message )
{
	QByteArray packet;

	if( !toPIDs.isEmpty( ) && !message.isEmpty( ) && message.size( ) < 255 )
	{
		packet.push_back( W3GS_HEADER_CONSTANT );		// W3GS header constant
		packet.push_back( W3GS_CHAT_FROM_HOST );		// W3GS_CHAT_FROM_HOST
		packet.push_back( (char)0 );							// packet length will be assigned later
		packet.push_back( (char)0 );							// packet length will be assigned later
		packet.push_back( toPIDs.size( ) );				// number of receivers
		UTIL_AppendBYTEARRAYFast( packet, toPIDs );		// receivers
		packet.push_back( fromPID );					// sender
		packet.push_back( flag );						// flag
		UTIL_AppendBYTEARRAYFast( packet, flagExtra );	// extra flag
		UTIL_AppendBYTEARRAYFast( packet, message );	// message
		AssignLength( packet );
	}
	else
		CONSOLE_Print( "[GAMEPROTO] invalid parameters passed to SEND_W3GS_CHAT_FROM_HOST" );

	// DEBUG_Print( "SENT W3GS_CHAT_FROM_HOST" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CGameProtocol :: SEND_W3GS_START_LAG( QVector<CGamePlayer *> players, bool loadInGame )
{
	QByteArray packet;

	unsigned char NumLaggers = 0;

	for( QVector<CGamePlayer *> :: iterator i = players.begin( ); i != players.end( ); i++ )
	{
		if( loadInGame )
		{
			if( !(*i)->GetFinishedLoading( ) )
				NumLaggers++;
		}
		else
		{
			if( (*i)->GetLagging( ) )
				NumLaggers++;
		}
	}

	if( NumLaggers > 0 )
	{
		packet.push_back( W3GS_HEADER_CONSTANT );	// W3GS header constant
		packet.push_back( W3GS_START_LAG );			// W3GS_START_LAG
		packet.push_back( (char)0 );						// packet length will be assigned later
		packet.push_back( (char)0 );						// packet length will be assigned later
		packet.push_back( NumLaggers );

		for( QVector<CGamePlayer *> :: iterator i = players.begin( ); i != players.end( ); i++ )
		{
			if( loadInGame )
			{
				if( !(*i)->GetFinishedLoading( ) )
				{
					packet.push_back( (*i)->GetPID( ) );
					UTIL_AppendBYTEARRAY( packet, (quint32)0, false );
				}
			}
			else
			{
				if( (*i)->GetLagging( ) )
				{
					packet.push_back( (*i)->GetPID( ) );
					UTIL_AppendBYTEARRAY( packet, GetTicks( ) - (*i)->GetStartedLaggingTicks( ), false );
				}
			}
		}

		AssignLength( packet );
	}
	else
		CONSOLE_Print( "[GAMEPROTO] no laggers passed to SEND_W3GS_START_LAG" );

	// DEBUG_Print( "SENT W3GS_START_LAG" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CGameProtocol :: SEND_W3GS_STOP_LAG( CGamePlayer *player, bool loadInGame )
{
	QByteArray packet;
	packet.push_back( W3GS_HEADER_CONSTANT );	// W3GS header constant
	packet.push_back( W3GS_STOP_LAG );			// W3GS_STOP_LAG
	packet.push_back( (char)0 );						// packet length will be assigned later
	packet.push_back( (char)0 );						// packet length will be assigned later
	packet.push_back( player->GetPID( ) );

	if( loadInGame )
		UTIL_AppendBYTEARRAY( packet, (quint32)0, false );
	else
		UTIL_AppendBYTEARRAY( packet, GetTicks( ) - player->GetStartedLaggingTicks( ), false );

	AssignLength( packet );
	// DEBUG_Print( "SENT W3GS_STOP_LAG" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CGameProtocol :: SEND_W3GS_SEARCHGAME( bool TFT, unsigned char war3Version )
{
	unsigned char ProductID_ROC[]	= {          51, 82, 65, 87 };	// "WAR3"
	unsigned char ProductID_TFT[]	= {          80, 88, 51, 87 };	// "W3XP"
	unsigned char Version[]			= { war3Version,  0,  0,  0 };
	unsigned char Unknown[]			= {           0,  0,  0,  0 };

	QByteArray packet;
	packet.push_back( W3GS_HEADER_CONSTANT );				// W3GS header constant
	packet.push_back( W3GS_SEARCHGAME );					// W3GS_SEARCHGAME
	packet.push_back( (char)0 );									// packet length will be assigned later
	packet.push_back( (char)0 );									// packet length will be assigned later

	if( TFT )
		UTIL_AppendBYTEARRAY( packet, ProductID_TFT, 4 );	// Product ID (TFT)
	else
		UTIL_AppendBYTEARRAY( packet, ProductID_ROC, 4 );	// Product ID (ROC)

	UTIL_AppendBYTEARRAY( packet, Version, 4 );				// Version
	UTIL_AppendBYTEARRAY( packet, Unknown, 4 );				// ???
	AssignLength( packet );
	// DEBUG_Print( "SENT W3GS_SEARCHGAME" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CGameProtocol :: SEND_W3GS_GAMEINFO( bool TFT, unsigned char war3Version, QByteArray mapGameType, QByteArray mapFlags, QByteArray mapWidth, QByteArray mapHeight, QString gameName, QString hostName, quint32 upTime, QString mapPath, QByteArray mapCRC, quint32 slotsTotal, quint32 slotsOpen, quint16 port, quint32 hostCounter )
{
	unsigned char ProductID_ROC[]	= {          51, 82, 65, 87 };	// "WAR3"
	unsigned char ProductID_TFT[]	= {          80, 88, 51, 87 };	// "W3XP"
	unsigned char Version[]			= { war3Version,  0,  0,  0 };
	unsigned char Unknown1[]		= {           1,  2,  3,  4 };
	unsigned char Unknown2[]		= {           1,  0,  0,  0 };

	QByteArray packet;

	if( mapGameType.size( ) == 4 && mapFlags.size( ) == 4 && mapWidth.size( ) == 2 && mapHeight.size( ) == 2 && !gameName.isEmpty( ) && !hostName.isEmpty( ) && !mapPath.isEmpty( ) && mapCRC.size( ) == 4 )
	{
		// make the stat QString

		QByteArray StatString;
		UTIL_AppendBYTEARRAYFast( StatString, mapFlags );
		StatString.push_back( (char)0 );
		UTIL_AppendBYTEARRAYFast( StatString, mapWidth );
		UTIL_AppendBYTEARRAYFast( StatString, mapHeight );
		UTIL_AppendBYTEARRAYFast( StatString, mapCRC );
		UTIL_AppendBYTEARRAYFast( StatString, mapPath );
		UTIL_AppendBYTEARRAYFast( StatString, hostName );
		StatString.push_back( (char)0 );
		StatString = UTIL_EncodeStatString( StatString );

		// make the rest of the packet

		packet.push_back( W3GS_HEADER_CONSTANT );						// W3GS header constant
		packet.push_back( W3GS_GAMEINFO );								// W3GS_GAMEINFO
		packet.push_back( (char)0 );											// packet length will be assigned later
		packet.push_back( (char)0 );											// packet length will be assigned later

		if( TFT )
			UTIL_AppendBYTEARRAY( packet, ProductID_TFT, 4 );			// Product ID (TFT)
		else
			UTIL_AppendBYTEARRAY( packet, ProductID_ROC, 4 );			// Product ID (ROC)

		UTIL_AppendBYTEARRAY( packet, Version, 4 );						// Version
		UTIL_AppendBYTEARRAY( packet, hostCounter, false );				// Host Counter
		UTIL_AppendBYTEARRAY( packet, Unknown1, 4 );					// ??? (this varies wildly even between two identical games created one after another)
		UTIL_AppendBYTEARRAYFast( packet, gameName );					// Game Name
		packet.push_back( (char)0 );											// ??? (maybe game password)
		UTIL_AppendBYTEARRAYFast( packet, StatString );					// Stat String
		packet.push_back( (char)0 );											// Stat String null terminator (the stat QString is encoded to remove all even numbers i.e. zeros)
		UTIL_AppendBYTEARRAY( packet, slotsTotal, false );				// Slots Total
		UTIL_AppendBYTEARRAYFast( packet, mapGameType );				// Game Type
		UTIL_AppendBYTEARRAY( packet, Unknown2, 4 );					// ???
		UTIL_AppendBYTEARRAY( packet, slotsOpen, false );				// Slots Open
		UTIL_AppendBYTEARRAY( packet, upTime, false );					// time since creation
		UTIL_AppendBYTEARRAY( packet, port, false );					// port
		AssignLength( packet );
	}
	else
		CONSOLE_Print( "[GAMEPROTO] invalid parameters passed to SEND_W3GS_GAMEINFO" );

	// DEBUG_Print( "SENT W3GS_GAMEINFO" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CGameProtocol :: SEND_W3GS_CREATEGAME( bool TFT, unsigned char war3Version )
{
	unsigned char ProductID_ROC[]	= {          51, 82, 65, 87 };	// "WAR3"
	unsigned char ProductID_TFT[]	= {          80, 88, 51, 87 };	// "W3XP"
	unsigned char Version[]			= { war3Version,  0,  0,  0 };
	unsigned char HostCounter[]		= {           1,  0,  0,  0 };

	QByteArray packet;
	packet.push_back( W3GS_HEADER_CONSTANT );				// W3GS header constant
	packet.push_back( W3GS_CREATEGAME );					// W3GS_CREATEGAME
	packet.push_back( (char)0 );									// packet length will be assigned later
	packet.push_back( (char)0 );									// packet length will be assigned later

	if( TFT )
		UTIL_AppendBYTEARRAY( packet, ProductID_TFT, 4 );	// Product ID (TFT)
	else
		UTIL_AppendBYTEARRAY( packet, ProductID_ROC, 4 );	// Product ID (ROC)

	UTIL_AppendBYTEARRAY( packet, Version, 4 );				// Version
	UTIL_AppendBYTEARRAY( packet, HostCounter, 4 );			// Host Counter
	AssignLength( packet );
	// DEBUG_Print( "SENT W3GS_CREATEGAME" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CGameProtocol :: SEND_W3GS_REFRESHGAME( quint32 players, quint32 playerSlots )
{
	unsigned char HostCounter[]	= { 1, 0, 0, 0 };

	QByteArray packet;
	packet.push_back( W3GS_HEADER_CONSTANT );			// W3GS header constant
	packet.push_back( W3GS_REFRESHGAME );				// W3GS_REFRESHGAME
	packet.push_back( (char)0 );								// packet length will be assigned later
	packet.push_back( (char)0 );								// packet length will be assigned later
	UTIL_AppendBYTEARRAY( packet, HostCounter, 4 );		// Host Counter
	UTIL_AppendBYTEARRAY( packet, players, false );		// Players
	UTIL_AppendBYTEARRAY( packet, playerSlots, false );	// Player Slots
	AssignLength( packet );
	// DEBUG_Print( "SENT W3GS_REFRESHGAME" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CGameProtocol :: SEND_W3GS_DECREATEGAME( )
{
	unsigned char HostCounter[]	= { 1, 0, 0, 0 };

	QByteArray packet;
	packet.push_back( W3GS_HEADER_CONSTANT );			// W3GS header constant
	packet.push_back( W3GS_DECREATEGAME );				// W3GS_DECREATEGAME
	packet.push_back( (char)0 );								// packet length will be assigned later
	packet.push_back( (char)0 );								// packet length will be assigned later
	UTIL_AppendBYTEARRAY( packet, HostCounter, 4 );		// Host Counter
	AssignLength( packet );
	// DEBUG_Print( "SENT W3GS_DECREATEGAME" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CGameProtocol :: SEND_W3GS_MAPCHECK( QString mapPath, QByteArray mapSize, QByteArray mapInfo, QByteArray mapCRC, QByteArray mapSHA1 )
{
	unsigned char Unknown[] = { 1, 0, 0, 0 };

	QByteArray packet;

	if( !mapPath.isEmpty( ) && mapSize.size( ) == 4 && mapInfo.size( ) == 4 && mapCRC.size( ) == 4 && mapSHA1.size( ) == 20 )
	{
		packet.push_back( W3GS_HEADER_CONSTANT );		// W3GS header constant
		packet.push_back( W3GS_MAPCHECK );				// W3GS_MAPCHECK
		packet.push_back( (char)0 );							// packet length will be assigned later
		packet.push_back( (char)0 );							// packet length will be assigned later
		UTIL_AppendBYTEARRAY( packet, Unknown, 4 );		// ???
		UTIL_AppendBYTEARRAYFast( packet, mapPath );	// map path
		UTIL_AppendBYTEARRAYFast( packet, mapSize );	// map size
		UTIL_AppendBYTEARRAYFast( packet, mapInfo );	// map info
		UTIL_AppendBYTEARRAYFast( packet, mapCRC );		// map crc
		UTIL_AppendBYTEARRAYFast( packet, mapSHA1 );	// map sha1
		AssignLength( packet );
	}
	else
		CONSOLE_Print( "[GAMEPROTO] invalid parameters passed to SEND_W3GS_MAPCHECK" );

	// DEBUG_Print( "SENT W3GS_MAPCHECK" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CGameProtocol :: SEND_W3GS_STARTDOWNLOAD( unsigned char fromPID )
{
	unsigned char Unknown[] = { 1, 0, 0, 0 };

	QByteArray packet;
	packet.push_back( W3GS_HEADER_CONSTANT );				// W3GS header constant
	packet.push_back( W3GS_STARTDOWNLOAD );					// W3GS_STARTDOWNLOAD
	packet.push_back( (char)0 );									// packet length will be assigned later
	packet.push_back( (char)0 );									// packet length will be assigned later
	UTIL_AppendBYTEARRAY( packet, Unknown, 4 );				// ???
	packet.push_back( fromPID );							// from PID
	AssignLength( packet );
	// DEBUG_Print( "SENT W3GS_STARTDOWNLOAD" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CGameProtocol :: SEND_W3GS_MAPPART( unsigned char fromPID, unsigned char toPID, quint32 start, QByteArray *mapData )
{
	unsigned char Unknown[] = { 1, 0, 0, 0 };

	QByteArray packet;

	if( start < (unsigned int)mapData->size( ) )
	{
		packet.push_back( W3GS_HEADER_CONSTANT );				// W3GS header constant
		packet.push_back( W3GS_MAPPART );						// W3GS_MAPPART
		packet.push_back( (char)0 );									// packet length will be assigned later
		packet.push_back( (char)0 );									// packet length will be assigned later
		packet.push_back( toPID );								// to PID
		packet.push_back( fromPID );							// from PID
		UTIL_AppendBYTEARRAY( packet, Unknown, 4 );				// ???
		UTIL_AppendBYTEARRAY( packet, start, false );			// start position

		// calculate end position (don't send more than 1442 map bytes in one packet)

		quint32 End = start + 1442;

		if( End > (unsigned int)mapData->size( ) )
			End = mapData->size( );

		// calculate crc

		QByteArray crc32 = UTIL_CreateBYTEARRAY( m_GHost->m_CRC->FullCRC( mapData->mid(start, End - start) ), false );
		UTIL_AppendBYTEARRAYFast( packet, crc32 );

		// map data

		packet.append( mapData->mid(start, End - start) );
		AssignLength( packet );
	}
	else
		CONSOLE_Print( "[GAMEPROTO] invalid parameters passed to SEND_W3GS_MAPPART" );

	// DEBUG_Print( "SENT W3GS_MAPPART" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CGameProtocol :: SEND_W3GS_INCOMING_ACTION2( QQueue<CIncomingAction *> actions )
{
	QByteArray packet;
	packet.push_back( W3GS_HEADER_CONSTANT );				// W3GS header constant
	packet.push_back( W3GS_INCOMING_ACTION2 );				// W3GS_INCOMING_ACTION2
	packet.push_back( (char)0 );									// packet length will be assigned later
	packet.push_back( (char)0 );									// packet length will be assigned later
	packet.push_back( (char)0 );									// ??? (send interval?)
	packet.push_back( (char)0 );									// ??? (send interval?)

	// create subpacket

	if( !actions.isEmpty( ) )
	{
		QByteArray subpacket;

		while( !actions.isEmpty( ) )
		{
			CIncomingAction *Action = actions.front( );
			actions.dequeue( );
			subpacket.push_back( Action->GetPID( ) );
			UTIL_AppendBYTEARRAY( subpacket, (quint16)Action->GetAction( )->size( ), false );
			UTIL_AppendBYTEARRAYFast( subpacket, *Action->GetAction( ) );
		}

		// calculate crc (we only care about the first 2 bytes though)

		QByteArray crc32 = UTIL_CreateBYTEARRAY( m_GHost->m_CRC->FullCRC(subpacket), false );
		crc32.resize( 2 );

		// finish subpacket

		UTIL_AppendBYTEARRAYFast( packet, crc32 );			// crc
		UTIL_AppendBYTEARRAYFast( packet, subpacket );		// subpacket
	}

	AssignLength( packet );
	// DEBUG_Print( "SENT W3GS_INCOMING_ACTION2" );
	// DEBUG_Print( packet );
	return packet;
}

/////////////////////
// OTHER FUNCTIONS //
/////////////////////

bool CGameProtocol :: AssignLength( QByteArray &content )
{
	// insert the actual length of the content array into bytes 3 and 4 (indices 2 and 3)

	QByteArray LengthBytes;

	if( content.size( ) >= 4 && content.size( ) <= 65535 )
	{
		LengthBytes = UTIL_CreateBYTEARRAY( (quint16)content.size( ), false );
		content[2] = LengthBytes[0];
		content[3] = LengthBytes[1];
		return true;
	}

	return false;
}

bool CGameProtocol :: ValidateLength( QByteArray &content )
{
	// verify that bytes 3 and 4 (indices 2 and 3) of the content array describe the length

	quint16 Length;
	QByteArray LengthBytes;

	if( content.size( ) >= 4 && content.size( ) <= 65535 )
	{
		LengthBytes.push_back( content[2] );
		LengthBytes.push_back( content[3] );
		Length = UTIL_QByteArrayToUInt16( LengthBytes, false );

		if( Length == content.size( ) )
			return true;
	}

	return false;
}

QByteArray CGameProtocol :: EncodeSlotInfo( QVector<CGameSlot> &lslots, quint32 randomSeed, unsigned char layoutStyle, unsigned char playerSlots )
{
	QByteArray SlotInfo;
	SlotInfo.push_back( (unsigned char)lslots.size( ) );		// number of slots

	for( int i = 0; i < lslots.size( ); i++ )
		UTIL_AppendBYTEARRAY( SlotInfo, lslots[i].GetQByteArray( ) );

	UTIL_AppendBYTEARRAY( SlotInfo, randomSeed, false );	// random seed
	SlotInfo.push_back( layoutStyle );						// LayoutStyle (0 = melee, 1 = custom forces, 3 = custom forces + fixed player settings)
	SlotInfo.push_back( playerSlots );						// number of player slots (non observer)
	return SlotInfo;
}

//
// CIncomingJoinPlayer
//

CIncomingJoinPlayer :: CIncomingJoinPlayer( quint32 nHostCounter, QString nName, QByteArray &nInternalIP )
{
	m_HostCounter = nHostCounter;
	m_Name = nName;
	m_InternalIP = nInternalIP;
}

CIncomingJoinPlayer :: ~CIncomingJoinPlayer( )
{

}

//
// CIncomingAction
//

CIncomingAction :: CIncomingAction( unsigned char nPID, QByteArray &nCRC, QByteArray &nAction )
{
	m_PID = nPID;
	m_CRC = nCRC;
	m_Action = nAction;
}

CIncomingAction :: ~CIncomingAction( )
{

}

//
// CIncomingChatPlayer
//

CIncomingChatPlayer :: CIncomingChatPlayer( unsigned char nFromPID, QByteArray &nToPIDs, unsigned char nFlag, QString nMessage )
{
	m_Type = CTH_MESSAGE;
	m_FromPID = nFromPID;
	m_ToPIDs = nToPIDs;
	m_Flag = nFlag;
	m_Message = nMessage;
}

CIncomingChatPlayer :: CIncomingChatPlayer( unsigned char nFromPID, QByteArray &nToPIDs, unsigned char nFlag, QString nMessage, QByteArray &nExtraFlags )
{
	m_Type = CTH_MESSAGEEXTRA;
	m_FromPID = nFromPID;
	m_ToPIDs = nToPIDs;
	m_Flag = nFlag;
	m_Message = nMessage;
	m_ExtraFlags = nExtraFlags;
}

CIncomingChatPlayer :: CIncomingChatPlayer( unsigned char nFromPID, QByteArray &nToPIDs, unsigned char nFlag, unsigned char nByte )
{
	if( nFlag == 17 )
		m_Type = CTH_TEAMCHANGE;
	else if( nFlag == 18 )
		m_Type = CTH_COLOURCHANGE;
	else if( nFlag == 19 )
		m_Type = CTH_RACECHANGE;
	else if( nFlag == 20 )
		m_Type = CTH_HANDICAPCHANGE;

	m_FromPID = nFromPID;
	m_ToPIDs = nToPIDs;
	m_Flag = nFlag;
	m_Byte = nByte;
}

CIncomingChatPlayer :: ~CIncomingChatPlayer( )
{

}

//
// CIncomingMapSize
//

CIncomingMapSize :: CIncomingMapSize( unsigned char nSizeFlag, quint32 nMapSize )
{
	m_SizeFlag = nSizeFlag;
	m_MapSize = nMapSize;
}

CIncomingMapSize :: ~CIncomingMapSize( )
{

}
