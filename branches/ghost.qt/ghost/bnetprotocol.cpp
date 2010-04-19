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
#include "bnetprotocol.h"

CBNETProtocol :: CBNETProtocol( )
{
	unsigned char ClientToken[] = { 220, 1, 203, 7 };
	m_ClientToken = UTIL_CreateQByteArray( ClientToken, 4 );
}

CBNETProtocol :: ~CBNETProtocol( )
{

}

///////////////////////
// RECEIVE FUNCTIONS //
///////////////////////

bool CBNETProtocol :: RECEIVE_SID_NULL( QByteArray data )
{
	// DEBUG_Print( "RECEIVED SID_NULL" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length

	return ValidateLength( data );
}

CIncomingGameHost *CBNETProtocol :: RECEIVE_SID_GETADVLISTEX( QByteArray data )
{
	// DEBUG_Print( "RECEIVED SID_GETADVLISTEX" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> GamesFound
	// if( GamesFound > 0 )
	//		10 bytes			-> ???
	//		2 bytes				-> Port
	//		4 bytes				-> IP
	//		null term QString	-> GameName
	//		2 bytes				-> ???
	//		8 bytes				-> HostCounter

	if( ValidateLength( data ) && data.size( ) >= 8 )
	{
		QByteArray GamesFound = data.mid(4, 4);

		if( UTIL_QByteArrayToUInt32( GamesFound, false ) > 0 && data.size( ) >= 25 )
		{
			QByteArray Port = data.mid(18, 2);
			QByteArray IP = data.mid(20, 4);
			QByteArray GameName = UTIL_ExtractCString( data, 24 );

			if( data.size( ) >= GameName.size( ) + 35 )
			{
				QByteArray HostCounter;
				HostCounter.push_back( UTIL_ExtractHex( data, GameName.size( ) + 27, true ) );
				HostCounter.push_back( UTIL_ExtractHex( data, GameName.size( ) + 29, true ) );
				HostCounter.push_back( UTIL_ExtractHex( data, GameName.size( ) + 31, true ) );
				HostCounter.push_back( UTIL_ExtractHex( data, GameName.size( ) + 33, true ) );
				return new CIncomingGameHost(	IP,
												UTIL_QByteArrayToUInt16( Port, false ),
												GameName,
												HostCounter );
			}
		}
	}

	return NULL;
}

bool CBNETProtocol :: RECEIVE_SID_ENTERCHAT( QByteArray data )
{
	// DEBUG_Print( "RECEIVED SID_ENTERCHAT" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// null terminated QString	-> UniqueName

	if( ValidateLength( data ) && data.size( ) >= 5 )
	{
		m_UniqueName = UTIL_ExtractCString( data, 4 );
		return true;
	}

	return false;
}

CIncomingChatEvent *CBNETProtocol :: RECEIVE_SID_CHATEVENT( QByteArray data )
{
	// DEBUG_Print( "RECEIVED SID_CHATEVENT" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> EventID
	// 4 bytes					-> ???
	// 4 bytes					-> Ping
	// 12 bytes					-> ???
	// null terminated QString	-> User
	// null terminated QString	-> Message

	if( ValidateLength( data ) && data.size( ) >= 29 )
	{
		QByteArray EventID = data.mid(4, 8 - 4);
		QByteArray Ping = data.mid(12, 16 - 12);
		QByteArray User = UTIL_ExtractCString( data, 28 );
		QByteArray Message = UTIL_ExtractCString( data, User.size( ) + 29 );

		switch( UTIL_QByteArrayToUInt32( EventID, false ) )
		{
		case CBNETProtocol :: EID_SHOWUSER:
		case CBNETProtocol :: EID_JOIN:
		case CBNETProtocol :: EID_LEAVE:
		case CBNETProtocol :: EID_WHISPER:
		case CBNETProtocol :: EID_TALK:
		case CBNETProtocol :: EID_BROADCAST:
		case CBNETProtocol :: EID_CHANNEL:
		case CBNETProtocol :: EID_USERFLAGS:
		case CBNETProtocol :: EID_WHISPERSENT:
		case CBNETProtocol :: EID_CHANNELFULL:
		case CBNETProtocol :: EID_CHANNELDOESNOTEXIST:
		case CBNETProtocol :: EID_CHANNELRESTRICTED:
		case CBNETProtocol :: EID_INFO:
		case CBNETProtocol :: EID_ERROR:
		case CBNETProtocol :: EID_EMOTE:
			return new CIncomingChatEvent(	(CBNETProtocol :: IncomingChatEvent)UTIL_QByteArrayToUInt32( EventID, false ),
												UTIL_QByteArrayToUInt32( Ping, false ),
												User,
												Message );
		}

	}

	return NULL;
}

bool CBNETProtocol :: RECEIVE_SID_CHECKAD( QByteArray data )
{
	// DEBUG_Print( "RECEIVED SID_CHECKAD" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length

	return ValidateLength( data );
}

bool CBNETProtocol :: RECEIVE_SID_STARTADVEX3( QByteArray data )
{
	// DEBUG_Print( "RECEIVED SID_STARTADVEX3" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> Status

	if( ValidateLength( data ) && data.size( ) >= 8 )
	{
		QByteArray Status = data.mid(4, 8 - 4);

		if( UTIL_QByteArrayToUInt32( Status, false ) == 0 )
			return true;
	}

	return false;
}

QByteArray CBNETProtocol :: RECEIVE_SID_PING( QByteArray data )
{
	// DEBUG_Print( "RECEIVED SID_PING" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> Ping

	if( ValidateLength( data ) && data.size( ) >= 8 )
		return data.mid(4, 8 - 4);

	return QByteArray( );
}

bool CBNETProtocol :: RECEIVE_SID_LOGONRESPONSE( QByteArray data )
{
	// DEBUG_Print( "RECEIVED SID_LOGONRESPONSE" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> Status

	if( ValidateLength( data ) && data.size( ) >= 8 )
	{
		QByteArray Status = data.mid(4, 8 - 4);

		if( UTIL_QByteArrayToUInt32( Status, false ) == 1 )
			return true;
	}

	return false;
}

bool CBNETProtocol :: RECEIVE_SID_AUTH_INFO( QByteArray data )
{
	// DEBUG_Print( "RECEIVED SID_AUTH_INFO" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> LogonType
	// 4 bytes					-> ServerToken
	// 4 bytes					-> ???
	// 8 bytes					-> MPQFileTime
	// null terminated QString	-> IX86VerFileName
	// null terminated QString	-> ValueStringFormula

	if( ValidateLength( data ) && data.size( ) >= 25 )
	{
		m_LogonType = data.mid(4, 8 - 4);
		m_ServerToken = data.mid(8, 12 - 8);
		m_MPQFileTime = data.mid(16, 24 - 16);
		m_IX86VerFileName = UTIL_ExtractCString( data, 24 );
		m_ValueStringFormula = UTIL_ExtractCString( data, m_IX86VerFileName.size( ) + 25 );
		return true;
	}

	return false;
}

bool CBNETProtocol :: RECEIVE_SID_AUTH_CHECK( QByteArray data )
{
	// DEBUG_Print( "RECEIVED SID_AUTH_CHECK" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> KeyState
	// null terminated QString	-> KeyStateDescription

	if( ValidateLength( data ) && data.size( ) >= 9 )
	{
		m_KeyState = data.mid(4, 8 - 4);
		m_KeyStateDescription = UTIL_ExtractCString( data, 8 );

		if( UTIL_QByteArrayToUInt32( m_KeyState, false ) == KR_GOOD )
			return true;
	}

	return false;
}

bool CBNETProtocol :: RECEIVE_SID_AUTH_ACCOUNTLOGON( QByteArray data )
{
	// DEBUG_Print( "RECEIVED SID_AUTH_ACCOUNTLOGON" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> Status
	// if( Status == 0 )
	//		32 bytes			-> Salt
	//		32 bytes			-> ServerPublicKey

	if( ValidateLength( data ) && data.size( ) >= 8 )
	{
		QByteArray status = data.mid(4, 8 - 4);

		if( UTIL_QByteArrayToUInt32( status, false ) == 0 && data.size( ) >= 72 )
		{
			m_Salt = data.mid(8, 40 - 8);
			m_ServerPublicKey = data.mid(40, 72 - 40);
			return true;
		}
	}

	return false;
}

bool CBNETProtocol :: RECEIVE_SID_AUTH_ACCOUNTLOGONPROOF( QByteArray data )
{
	// DEBUG_Print( "RECEIVED SID_AUTH_ACCOUNTLOGONPROOF" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> Status

	if( ValidateLength( data ) && data.size( ) >= 8 )
	{
		QByteArray Status = data.mid(4, 8 - 4);

		if( UTIL_QByteArrayToUInt32( Status, false ) == 0 )
			return true;
	}

	return false;
}

QByteArray CBNETProtocol :: RECEIVE_SID_WARDEN( QByteArray data )
{
	// DEBUG_Print( "RECEIVED SID_WARDEN" );
	// DEBUG_PRINT( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// n bytes					-> Data

	if( ValidateLength( data ) && data.size( ) >= 4 )
		return data.mid(4);

	return QByteArray( );
}

QVector<CIncomingFriendList *> CBNETProtocol :: RECEIVE_SID_FRIENDSLIST( QByteArray data )
{
	// DEBUG_Print( "RECEIVED SID_FRIENDSLIST" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 1 byte					-> Total
	// for( 1 .. Total )
	//		null term QString	-> Account
	//		1 byte				-> Status
	//		1 byte				-> Area
	//		4 bytes				-> ???
	//		null term QString	-> Location

	QVector<CIncomingFriendList *> Friends;

	if( ValidateLength( data ) && data.size( ) >= 5 )
	{
		unsigned int i = 5;
		unsigned char Total = data[4];

		while( Total > 0 )
		{
			Total--;

			if( data.size( ) < i + 1 )
				break;

			QByteArray Account = UTIL_ExtractCString( data, i );
			i += Account.size( ) + 1;

			if( data.size( ) < i + 7 )
				break;

			unsigned char Status = data[i];
			unsigned char Area = data[i + 1];
			i += 6;
			QByteArray Location = UTIL_ExtractCString( data, i );
			i += Location.size( ) + 1;
			Friends.push_back( new CIncomingFriendList(	Account,
														Status,
														Area,
														Location ) );
		}
	}

	return Friends;
}

QVector<CIncomingClanList *> CBNETProtocol :: RECEIVE_SID_CLANMEMBERLIST( QByteArray data )
{
	// DEBUG_Print( "RECEIVED SID_CLANMEMBERLIST" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// 4 bytes					-> ???
	// 1 byte					-> Total
	// for( 1 .. Total )
	//		null term QString	-> Name
	//		1 byte				-> Rank
	//		1 byte				-> Status
	//		null term QString	-> Location

	QVector<CIncomingClanList *> ClanList;

	if( ValidateLength( data ) && data.size( ) >= 9 )
	{
		unsigned int i = 9;
		unsigned char Total = data[8];

		while( Total > 0 )
		{
			Total--;

			if( data.size( ) < i + 1 )
				break;

			QByteArray Name = UTIL_ExtractCString( data, i );
			i += Name.size( ) + 1;

			if( data.size( ) < i + 3 )
				break;

			unsigned char Rank = data[i];
			unsigned char Status = data[i + 1];
			i += 2;

			// in the original VB source the location QString is read but discarded, so that's what I do here

			QByteArray Location = UTIL_ExtractCString( data, i );
			i += Location.size( ) + 1;
			ClanList.push_back( new CIncomingClanList(	Name,
														Rank,
														Status ) );
		}
	}

	return ClanList;
}

CIncomingClanList *CBNETProtocol :: RECEIVE_SID_CLANMEMBERSTATUSCHANGE( QByteArray data )
{
	// DEBUG_Print( "RECEIVED SID_CLANMEMBERSTATUSCHANGE" );
	// DEBUG_Print( data );

	// 2 bytes					-> Header
	// 2 bytes					-> Length
	// null terminated QString	-> Name
	// 1 byte					-> Rank
	// 1 byte					-> Status
	// null terminated QString	-> Location

	if( ValidateLength( data ) && data.size( ) >= 5 )
	{
		QByteArray Name = UTIL_ExtractCString( data, 4 );

		if( data.size( ) >= Name.size( ) + 7 )
		{
			unsigned char Rank = data[Name.size( ) + 5];
			unsigned char Status = data[Name.size( ) + 6];

			// in the original VB source the location QString is read but discarded, so that's what I do here

			QByteArray Location = UTIL_ExtractCString( data, Name.size( ) + 7 );
			return new CIncomingClanList(	Name,
											Rank,
											Status );
		}
	}

	return NULL;
}

////////////////////
// SEND FUNCTIONS //
////////////////////

QByteArray CBNETProtocol :: SEND_PROTOCOL_INITIALIZE_SELECTOR( )
{
	QByteArray packet;
	packet.push_back( 1 );
	// DEBUG_Print( "SENT PROTOCOL_INITIALIZE_SELECTOR" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CBNETProtocol :: SEND_SID_NULL( )
{
	QByteArray packet;
	packet.push_back( BNET_HEADER_CONSTANT );	// BNET header constant
	packet.push_back( SID_NULL );				// SID_NULL
	packet.push_back( (char)0 );						// packet length will be assigned later
	packet.push_back( (char)0 );						// packet length will be assigned later
	AssignLength( packet );
	// DEBUG_Print( "SENT SID_NULL" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CBNETProtocol :: SEND_SID_STOPADV( )
{
	QByteArray packet;
	packet.push_back( BNET_HEADER_CONSTANT );	// BNET header constant
	packet.push_back( SID_STOPADV );			// SID_STOPADV
	packet.push_back( (char)0 );						// packet length will be assigned later
	packet.push_back( (char)0 );						// packet length will be assigned later
	AssignLength( packet );
	// DEBUG_Print( "SENT SID_STOPADV" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CBNETProtocol :: SEND_SID_GETADVLISTEX( QString gameName )
{
	unsigned char MapFilter1[]	= { 255, 3, 0, 0 };
	unsigned char MapFilter2[]	= { 255, 3, 0, 0 };
	unsigned char MapFilter3[]	= {   0, 0, 0, 0 };
	unsigned char NumGames[]	= {   1, 0, 0, 0 };

	QByteArray packet;
	packet.push_back( BNET_HEADER_CONSTANT );			// BNET header constant
	packet.push_back( SID_GETADVLISTEX );				// SID_GETADVLISTEX
	packet.push_back( (char)0 );								// packet length will be assigned later
	packet.push_back( (char)0 );								// packet length will be assigned later
	UTIL_AppendQByteArray( packet, MapFilter1, 4 );		// Map Filter
	UTIL_AppendQByteArray( packet, MapFilter2, 4 );		// Map Filter
	UTIL_AppendQByteArray( packet, MapFilter3, 4 );		// Map Filter
	UTIL_AppendQByteArray( packet, NumGames, 4 );		// maximum number of games to list
	UTIL_AppendQByteArrayFast( packet, gameName );		// Game Name
	packet.push_back( (char)0 );								// Game Password is NULL
	packet.push_back( (char)0 );								// Game Stats is NULL
	AssignLength( packet );
	// DEBUG_Print( "SENT SID_GETADVLISTEX" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CBNETProtocol :: SEND_SID_ENTERCHAT( )
{
	QByteArray packet;
	packet.push_back( BNET_HEADER_CONSTANT );	// BNET header constant
	packet.push_back( SID_ENTERCHAT );			// SID_ENTERCHAT
	packet.push_back( (char)0 );						// packet length will be assigned later
	packet.push_back( (char)0 );						// packet length will be assigned later
	packet.push_back( (char)0 );						// Account Name is NULL on Warcraft III/The Frozen Throne
	packet.push_back( (char)0 );						// Stat String is NULL on CDKEY'd products
	AssignLength( packet );
	// DEBUG_Print( "SENT SID_ENTERCHAT" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CBNETProtocol :: SEND_SID_JOINCHANNEL( QString channel )
{
	unsigned char NoCreateJoin[]	= { 2, 0, 0, 0 };
	unsigned char FirstJoin[]		= { 1, 0, 0, 0 };

	QByteArray packet;
	packet.push_back( BNET_HEADER_CONSTANT );				// BNET header constant
	packet.push_back( SID_JOINCHANNEL );					// SID_JOINCHANNEL
	packet.push_back( (char)0 );									// packet length will be assigned later
	packet.push_back( (char)0 );									// packet length will be assigned later

	if( channel.size( ) > 0 )
		UTIL_AppendQByteArray( packet, NoCreateJoin, 4 );	// flags for no create join
	else
		UTIL_AppendQByteArray( packet, FirstJoin, 4 );		// flags for first join

	UTIL_AppendQByteArrayFast( packet, channel );
	AssignLength( packet );
	// DEBUG_Print( "SENT SID_JOINCHANNEL" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CBNETProtocol :: SEND_SID_CHATCOMMAND( QString command )
{
	QByteArray packet;
	packet.push_back( BNET_HEADER_CONSTANT );		// BNET header constant
	packet.push_back( SID_CHATCOMMAND );			// SID_CHATCOMMAND
	packet.push_back( (char)0 );							// packet length will be assigned later
	packet.push_back( (char)0 );							// packet length will be assigned later
	UTIL_AppendQByteArrayFast( packet, command );	// Message
	AssignLength( packet );
	// DEBUG_Print( "SENT SID_CHATCOMMAND" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CBNETProtocol :: SEND_SID_CHECKAD( )
{
	unsigned char Zeros[] = { 0, 0, 0, 0 };

	QByteArray packet;
	packet.push_back( BNET_HEADER_CONSTANT );	// BNET header constant
	packet.push_back( SID_CHECKAD );			// SID_CHECKAD
	packet.push_back( (char)0 );						// packet length will be assigned later
	packet.push_back( (char)0 );						// packet length will be assigned later
	UTIL_AppendQByteArray( packet, Zeros, 4 );	// ???
	UTIL_AppendQByteArray( packet, Zeros, 4 );	// ???
	UTIL_AppendQByteArray( packet, Zeros, 4 );	// ???
	UTIL_AppendQByteArray( packet, Zeros, 4 );	// ???
	AssignLength( packet );
	// DEBUG_Print( "SENT SID_CHECKAD" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CBNETProtocol :: SEND_SID_STARTADVEX3( unsigned char state, QByteArray mapGameType, QByteArray mapFlags, QByteArray mapWidth, QByteArray mapHeight, QString gameName, QString hostName, uint32_t upTime, QString mapPath, QByteArray mapCRC, QByteArray mapSHA1, uint32_t hostCounter )
{
	// todotodo: sort out how GameType works, the documentation is horrendous

/*

Game type tag: (read W3GS_GAMEINFO for this field)
 0x00000001 - Custom
 0x00000009 - Blizzard/Ladder
Map author: (mask 0x00006000) can be combined
*0x00002000 - Blizzard
 0x00004000 - Custom
Battle type: (mask 0x00018000) cant be combined
 0x00000000 - Battle
*0x00010000 - Scenario
Map size: (mask 0x000E0000) can be combined with 2 nearest values
 0x00020000 - Small
 0x00040000 - Medium
*0x00080000 - Huge
Observers: (mask 0x00700000) cant be combined
 0x00100000 - Allowed observers
 0x00200000 - Observers on defeat
*0x00400000 - No observers
Flags:
 0x00000800 - Private game flag (not used in game list)

*/

	unsigned char Unknown[]		= { 255,  3,  0,  0 };
	unsigned char CustomGame[]	= {   0,  0,  0,  0 };

	QString HostCounterString = UTIL_ToHexString( hostCounter );

	QString fi;
	fi.fill('0', 8 - HostCounterString.size( ) );

	if( HostCounterString.size( ) < 8 )
		HostCounterString += fi;

	HostCounterString = UTIL_QByteArrayReverse( HostCounterString.toUtf8() );

	QByteArray packet;

	// make the stat QString

	QByteArray StatString;
	UTIL_AppendQByteArrayFast( StatString, mapFlags );
	StatString.push_back( (char)0 );
	UTIL_AppendQByteArrayFast( StatString, mapWidth );
	UTIL_AppendQByteArrayFast( StatString, mapHeight );
	UTIL_AppendQByteArrayFast( StatString, mapCRC );
	UTIL_AppendQByteArrayFast( StatString, mapPath );
	UTIL_AppendQByteArrayFast( StatString, hostName );
	StatString.push_back( (char)0 );
	UTIL_AppendQByteArrayFast( StatString, mapSHA1 );
	StatString = UTIL_EncodeStatString( StatString );

	if( mapGameType.size( ) == 4 && mapFlags.size( ) == 4 && mapWidth.size( ) == 2 && mapHeight.size( ) == 2 && !gameName.isEmpty( ) && !hostName.isEmpty( ) && !mapPath.isEmpty( ) && mapCRC.size( ) == 4 && mapSHA1.size( ) == 20 && StatString.size( ) < 128 && HostCounterString.size( ) == 8 )
	{
		// make the rest of the packet

		packet.push_back( BNET_HEADER_CONSTANT );						// BNET header constant
		packet.push_back( SID_STARTADVEX3 );							// SID_STARTADVEX3
		packet.push_back( (char)0 );											// packet length will be assigned later
		packet.push_back( (char)0 );											// packet length will be assigned later
		packet.push_back( state );										// State (16 = public, 17 = private, 18 = close)
		packet.push_back( (char)0 );											// State continued...
		packet.push_back( (char)0 );											// State continued...
		packet.push_back( (char)0 );											// State continued...
		UTIL_AppendQByteArray( packet, upTime, false );					// time since creation
		UTIL_AppendQByteArrayFast( packet, mapGameType );				// Game Type, Parameter
		UTIL_AppendQByteArray( packet, Unknown, 4 );						// ???
		UTIL_AppendQByteArray( packet, CustomGame, 4 );					// Custom Game
		UTIL_AppendQByteArrayFast( packet, gameName );					// Game Name
		packet.push_back( (char)0 );											// Game Password is NULL
		packet.push_back( 98 );											// Slots Free (ascii 98 = char 'b' = 11 slots free) - note: do not reduce this as this is the # of PID's Warcraft III will allocate
		UTIL_AppendQByteArrayFast( packet, HostCounterString, false );	// Host Counter
		UTIL_AppendQByteArrayFast( packet, StatString );					// Stat String
		packet.push_back( (char)0 );											// Stat String null terminator (the stat QString is encoded to remove all even numbers i.e. zeros)
		AssignLength( packet );
	}
	else
		CONSOLE_Print( "[BNETPROTO] invalid parameters passed to SEND_SID_STARTADVEX3" );

	// DEBUG_Print( "SENT SID_STARTADVEX3" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CBNETProtocol :: SEND_SID_NOTIFYJOIN( QString gameName )
{
	unsigned char ProductID[]		= {  0, 0, 0, 0 };
	unsigned char ProductVersion[]	= { 14, 0, 0, 0 };	// Warcraft III is 14

	QByteArray packet;
	packet.push_back( BNET_HEADER_CONSTANT );			// BNET header constant
	packet.push_back( SID_NOTIFYJOIN );					// SID_NOTIFYJOIN
	packet.push_back( (char)0 );								// packet length will be assigned later
	packet.push_back( (char)0 );								// packet length will be assigned later
	UTIL_AppendQByteArray( packet, ProductID, 4 );		// Product ID
	UTIL_AppendQByteArray( packet, ProductVersion, 4 );	// Product Version
	UTIL_AppendQByteArrayFast( packet, gameName );		// Game Name
	packet.push_back( (char)0 );								// Game Password is NULL
	AssignLength( packet );
	// DEBUG_Print( "SENT SID_NOTIFYJOIN" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CBNETProtocol :: SEND_SID_PING( QByteArray pingValue )
{
	QByteArray packet;

	if( pingValue.size( ) == 4 )
	{
		packet.push_back( BNET_HEADER_CONSTANT );		// BNET header constant
		packet.push_back( SID_PING );					// SID_PING
		packet.push_back( (char)0 );							// packet length will be assigned later
		packet.push_back( (char)0 );							// packet length will be assigned later
		UTIL_AppendQByteArrayFast( packet, pingValue );	// Ping Value
		AssignLength( packet );
	}
	else
		CONSOLE_Print( "[BNETPROTO] invalid parameters passed to SEND_SID_PING" );

	// DEBUG_Print( "SENT SID_PING" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CBNETProtocol :: SEND_SID_LOGONRESPONSE( QByteArray clientToken, QByteArray serverToken, QByteArray passwordHash, QString accountName )
{
	// todotodo: check that the passed QByteArray sizes are correct (don't know what they should be right now so I can't do this today)

	QByteArray packet;
	packet.push_back( BNET_HEADER_CONSTANT );			// BNET header constant
	packet.push_back( SID_LOGONRESPONSE );				// SID_LOGONRESPONSE
	packet.push_back( (char)0 );								// packet length will be assigned later
	packet.push_back( (char)0 );								// packet length will be assigned later
	UTIL_AppendQByteArrayFast( packet, clientToken );	// Client Token
	UTIL_AppendQByteArrayFast( packet, serverToken );	// Server Token
	UTIL_AppendQByteArrayFast( packet, passwordHash );	// Password Hash
	UTIL_AppendQByteArrayFast( packet, accountName );	// Account Name
	AssignLength( packet );
	// DEBUG_Print( "SENT SID_LOGONRESPONSE" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CBNETProtocol :: SEND_SID_NETGAMEPORT( uint16_t serverPort )
{
	QByteArray packet;
	packet.push_back( BNET_HEADER_CONSTANT );			// BNET header constant
	packet.push_back( SID_NETGAMEPORT );				// SID_NETGAMEPORT
	packet.push_back( (char)0 );								// packet length will be assigned later
	packet.push_back( (char)0 );								// packet length will be assigned later
	UTIL_AppendQByteArray( packet, serverPort, false );	// local game server port
	AssignLength( packet );
	// DEBUG_Print( "SENT SID_NETGAMEPORT" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CBNETProtocol :: SEND_SID_AUTH_INFO( unsigned char ver, bool TFT, uint32_t localeID, QString countryAbbrev, QString country )
{
	unsigned char ProtocolID[]		= {   0,   0,   0,   0 };
	unsigned char PlatformID[]		= {  54,  56,  88,  73 };	// "IX86"
	unsigned char ProductID_ROC[]	= {  51,  82,  65,  87 };	// "WAR3"
	unsigned char ProductID_TFT[]	= {  80,  88,  51,  87 };	// "W3XP"
	unsigned char Version[]			= { ver,   0,   0,   0 };
	unsigned char Language[]		= {  83,  85, 110, 101 };	// "enUS"
	unsigned char LocalIP[]			= { 127,   0,   0,   1 };
	unsigned char TimeZoneBias[]	= {  44,   1,   0,   0 };	// 300 minutes (GMT -0500)

	QByteArray packet;
	packet.push_back( BNET_HEADER_CONSTANT );				// BNET header constant
	packet.push_back( SID_AUTH_INFO );						// SID_AUTH_INFO
	packet.push_back( (char)0 );									// packet length will be assigned later
	packet.push_back( (char)0 );									// packet length will be assigned later
	UTIL_AppendQByteArray( packet, ProtocolID, 4 );			// Protocol ID
	UTIL_AppendQByteArray( packet, PlatformID, 4 );			// Platform ID

	if( TFT )
		UTIL_AppendQByteArray( packet, ProductID_TFT, 4 );	// Product ID (TFT)
	else
		UTIL_AppendQByteArray( packet, ProductID_ROC, 4 );	// Product ID (ROC)

	UTIL_AppendQByteArray( packet, Version, 4 );				// Version
	UTIL_AppendQByteArray( packet, Language, 4 );			// Language (hardcoded as enUS to ensure battle.net sends the bot messages in English)
	UTIL_AppendQByteArray( packet, LocalIP, 4 );				// Local IP for NAT compatibility
	UTIL_AppendQByteArray( packet, TimeZoneBias, 4 );		// Time Zone Bias
	UTIL_AppendQByteArray( packet, localeID, false );		// Locale ID
	UTIL_AppendQByteArray( packet, localeID, false );		// Language ID (copying the locale ID should be sufficient since we don't care about sublanguages)
	UTIL_AppendQByteArrayFast( packet, countryAbbrev );		// Country Abbreviation
	UTIL_AppendQByteArrayFast( packet, country );			// Country
	AssignLength( packet );
	// DEBUG_Print( "SENT SID_AUTH_INFO" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CBNETProtocol :: SEND_SID_AUTH_CHECK( bool TFT, QByteArray clientToken, QByteArray exeVersion, QByteArray exeVersionHash, QByteArray keyInfoROC, QByteArray keyInfoTFT, QString exeInfo, QString keyOwnerName )
{
	uint32_t NumKeys = 0;

	if( TFT )
		NumKeys = 2;
	else
		NumKeys = 1;

	QByteArray packet;

	if( clientToken.size( ) == 4 && exeVersion.size( ) == 4 && exeVersionHash.size( ) == 4 && keyInfoROC.size( ) == 36 && ( !TFT || keyInfoTFT.size( ) == 36 ) )
	{
		packet.push_back( BNET_HEADER_CONSTANT );			// BNET header constant
		packet.push_back( SID_AUTH_CHECK );					// SID_AUTH_CHECK
		packet.push_back( (char)0 );								// packet length will be assigned later
		packet.push_back( (char)0 );								// packet length will be assigned later
		UTIL_AppendQByteArrayFast( packet, clientToken );	// Client Token
		UTIL_AppendQByteArrayFast( packet, exeVersion );		// EXE Version
		UTIL_AppendQByteArrayFast( packet, exeVersionHash );	// EXE Version Hash
		UTIL_AppendQByteArray( packet, NumKeys, false );		// number of keys in this packet
		UTIL_AppendQByteArray( packet, (uint32_t)0, false );	// boolean Using Spawn (32 bit)
		UTIL_AppendQByteArrayFast( packet, keyInfoROC );		// ROC Key Info

		if( TFT )
			UTIL_AppendQByteArrayFast( packet, keyInfoTFT );	// TFT Key Info

		UTIL_AppendQByteArrayFast( packet, exeInfo );		// EXE Info
		UTIL_AppendQByteArrayFast( packet, keyOwnerName );	// CD Key Owner Name
		AssignLength( packet );
	}
	else
		CONSOLE_Print( "[BNETPROTO] invalid parameters passed to SEND_SID_AUTH_CHECK" );

	// DEBUG_Print( "SENT SID_AUTH_CHECK" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CBNETProtocol :: SEND_SID_AUTH_ACCOUNTLOGON( QByteArray clientPublicKey, QString accountName )
{
	QByteArray packet;

	if( clientPublicKey.size( ) == 32 )
	{
		packet.push_back( BNET_HEADER_CONSTANT );				// BNET header constant
		packet.push_back( SID_AUTH_ACCOUNTLOGON );				// SID_AUTH_ACCOUNTLOGON
		packet.push_back( (char)0 );									// packet length will be assigned later
		packet.push_back( (char)0 );									// packet length will be assigned later
		UTIL_AppendQByteArrayFast( packet, clientPublicKey );	// Client Key
		UTIL_AppendQByteArrayFast( packet, accountName );		// Account Name
		AssignLength( packet );
	}
	else
		CONSOLE_Print( "[BNETPROTO] invalid parameters passed to SEND_SID_AUTH_ACCOUNTLOGON" );

	// DEBUG_Print( "SENT SID_AUTH_ACCOUNTLOGON" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CBNETProtocol :: SEND_SID_AUTH_ACCOUNTLOGONPROOF( QByteArray clientPasswordProof )
{
	QByteArray packet;

	if( clientPasswordProof.size( ) == 20 )
	{
		packet.push_back( BNET_HEADER_CONSTANT );					// BNET header constant
		packet.push_back( SID_AUTH_ACCOUNTLOGONPROOF );				// SID_AUTH_ACCOUNTLOGONPROOF
		packet.push_back( (char)0 );										// packet length will be assigned later
		packet.push_back( (char)0 );										// packet length will be assigned later
		UTIL_AppendQByteArrayFast( packet, clientPasswordProof );	// Client Password Proof
		AssignLength( packet );
	}
	else
		CONSOLE_Print( "[BNETPROTO] invalid parameters passed to SEND_SID_AUTH_ACCOUNTLOGON" );

	// DEBUG_Print( "SENT SID_AUTH_ACCOUNTLOGONPROOF" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CBNETProtocol :: SEND_SID_WARDEN( QByteArray wardenResponse )
{
	QByteArray packet;
	packet.push_back( BNET_HEADER_CONSTANT );			// BNET header constant
	packet.push_back( SID_WARDEN );						// SID_WARDEN
	packet.push_back( (char)0 );								// packet length will be assigned later
	packet.push_back( (char)0 );								// packet length will be assigned later
	UTIL_AppendQByteArrayFast( packet, wardenResponse );	// warden response
	AssignLength( packet );
	// DEBUG_Print( "SENT SID_WARDEN" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CBNETProtocol :: SEND_SID_FRIENDSLIST( )
{
	QByteArray packet;
	packet.push_back( BNET_HEADER_CONSTANT );	// BNET header constant
	packet.push_back( SID_FRIENDSLIST );		// SID_FRIENDSLIST
	packet.push_back( (char)0 );						// packet length will be assigned later
	packet.push_back( (char)0 );						// packet length will be assigned later
	AssignLength( packet );
	// DEBUG_Print( "SENT SID_FRIENDSLIST" );
	// DEBUG_Print( packet );
	return packet;
}

QByteArray CBNETProtocol :: SEND_SID_CLANMEMBERLIST( )
{
	unsigned char Cookie[] = { 0, 0, 0, 0 };

	QByteArray packet;
	packet.push_back( BNET_HEADER_CONSTANT );	// BNET header constant
	packet.push_back( SID_CLANMEMBERLIST );		// SID_CLANMEMBERLIST
	packet.push_back( (char)0 );						// packet length will be assigned later
	packet.push_back( (char)0 );						// packet length will be assigned later
	UTIL_AppendQByteArray( packet, Cookie, 4 );	// cookie
	AssignLength( packet );
	// DEBUG_Print( "SENT SID_CLANMEMBERLIST" );
	// DEBUG_Print( packet );
	return packet;
}

/////////////////////
// OTHER FUNCTIONS //
/////////////////////

bool CBNETProtocol :: AssignLength( QByteArray &content )
{
	// insert the actual length of the content array into bytes 3 and 4 (indices 2 and 3)

	QByteArray LengthBytes;

	if( content.size( ) >= 4 && content.size( ) <= 65535 )
	{
		LengthBytes = UTIL_CreateQByteArray( (uint16_t)content.size( ), false );
		content[2] = LengthBytes[0];
		content[3] = LengthBytes[1];
		return true;
	}

	return false;
}

bool CBNETProtocol :: ValidateLength( QByteArray &content )
{
	// verify that bytes 3 and 4 (indices 2 and 3) of the content array describe the length

	uint16_t Length;
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

//
// CIncomingGameHost
//

CIncomingGameHost :: CIncomingGameHost( QByteArray &nIP, uint16_t nPort, QString nGameName, QByteArray &nHostCounter )
{
	m_IP = nIP;
	m_Port = nPort;
	m_GameName = nGameName;
	m_HostCounter = nHostCounter;
}

CIncomingGameHost :: ~CIncomingGameHost( )
{

}

QString CIncomingGameHost :: GetIPString( )
{
	QString Result;

	if( m_IP.size( ) >= 4 )
	{
		for( unsigned int i = 0; i < 4; i++ )
		{
			Result += UTIL_ToString( (unsigned int)m_IP[i] );

			if( i < 3 )
				Result += ".";
		}
	}

	return Result;
}

//
// CIncomingChatEvent
//

CIncomingChatEvent :: CIncomingChatEvent( CBNETProtocol :: IncomingChatEvent nChatEvent, uint32_t nPing, QString nUser, QString nMessage )
{
	m_ChatEvent = nChatEvent;
	m_Ping = nPing;
	m_User = nUser;
	m_Message = nMessage;
}

CIncomingChatEvent :: ~CIncomingChatEvent( )
{

}

//
// CIncomingFriendList
//

CIncomingFriendList :: CIncomingFriendList( QString nAccount, unsigned char nStatus, unsigned char nArea, QString nLocation )
{
	m_Account = nAccount;
	m_Status = nStatus;
	m_Area = nArea;
	m_Location = nLocation;
}

CIncomingFriendList :: ~CIncomingFriendList( )
{

}

QString CIncomingFriendList :: GetDescription( )
{
	QString Description;
	Description += GetAccount( ) + "\n";
	Description += ExtractStatus( GetStatus( ) ) + "\n";
	Description += ExtractArea( GetArea( ) ) + "\n";
	Description += ExtractLocation( GetLocation( ) ) + "\n\n";
	return Description;
}

QString CIncomingFriendList :: ExtractStatus( unsigned char status )
{
	QString Result;

	if( status & 1 )
		Result += "<Mutual>";

	if( status & 2 )
		Result += "<DND>";

	if( status & 4 )
		Result += "<Away>";

	if( Result.isEmpty( ) )
		Result = "<None>";

	return Result;
}

QString CIncomingFriendList :: ExtractArea( unsigned char area )
{
	switch( area )
	{
	case 0: return "<Offline>";
	case 1: return "<No Channel>";
	case 2: return "<In Channel>";
	case 3: return "<Public Game>";
	case 4: return "<Private Game>";
	case 5: return "<Private Game>";
	}

	return "<Unknown>";
}

QString CIncomingFriendList :: ExtractLocation( QString location )
{
	QString Result;

	if( location.mid( 0, 4 ) == "PX3W" )
		Result = location.mid( 4 );

	if( Result.isEmpty( ) )
		Result = ".";

	return Result;
}

//
// CIncomingClanList
//

CIncomingClanList :: CIncomingClanList( QString nName, unsigned char nRank, unsigned char nStatus )
{
	m_Name = nName;
	m_Rank = nRank;
	m_Status = nStatus;
}

CIncomingClanList :: ~CIncomingClanList( )
{

}

QString CIncomingClanList :: GetRank( )
{
	switch( m_Rank )
	{
	case 0: return "Recruit";
	case 1: return "Peon";
	case 2: return "Grunt";
	case 3: return "Shaman";
	case 4: return "Chieftain";
	}

	return "Rank Unknown";
}

QString CIncomingClanList :: GetStatus( )
{
	if( m_Status == 0 )
		return "Offline";
	else
		return "Online";
}

QString CIncomingClanList :: GetDescription( )
{
	QString Description;
	Description += GetName( ) + "\n";
	Description += GetStatus( ) + "\n";
	Description += GetRank( ) + "\n\n";
	return Description;
}
