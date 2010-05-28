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

#ifndef GAMEPROTOCOL_H
#define GAMEPROTOCOL_H

//
// CGameProtocol
//

#define W3GS_HEADER_CONSTANT		247

#define GAME_NONE					0		// this case isn't part of the protocol, it's for internal use only
#define GAME_FULL					2
#define GAME_PUBLIC					16
#define GAME_PRIVATE				17

#define GAMETYPE_CUSTOM				1
#define GAMETYPE_BLIZZARD			9

#define PLAYERLEAVE_DISCONNECT		1
#define PLAYERLEAVE_LOST			7
#define PLAYERLEAVE_LOSTBUILDINGS	8
#define PLAYERLEAVE_WON				9
#define PLAYERLEAVE_DRAW			10
#define PLAYERLEAVE_OBSERVER		11
#define PLAYERLEAVE_LOBBY			13
#define PLAYERLEAVE_GPROXY			100

#define REJECTJOIN_FULL				9
#define REJECTJOIN_STARTED			10
#define REJECTJOIN_WRONGPASSWORD	27

#include <QByteArray>
#include "includes.h"
#include "gameslot.h"

class CGHost;
class CGamePlayer;
class CIncomingJoinPlayer;
class CIncomingAction;
class CIncomingChatPlayer;
class CIncomingMapSize;

class CGameProtocol
{
public:
	CGHost *m_GHost;

	enum Protocol {
		W3GS_PING_FROM_HOST		= 1,	// 0x01
		W3GS_SLOTINFOJOIN		= 4,	// 0x04
		W3GS_REJECTJOIN			= 5,	// 0x05
		W3GS_PLAYERINFO			= 6,	// 0x06
		W3GS_PLAYERLEAVE_OTHERS	= 7,	// 0x07
		W3GS_GAMELOADED_OTHERS	= 8,	// 0x08
		W3GS_SLOTINFO			= 9,	// 0x09
		W3GS_COUNTDOWN_START	= 10,	// 0x0A
		W3GS_COUNTDOWN_END		= 11,	// 0x0B
		W3GS_INCOMING_ACTION	= 12,	// 0x0C
		W3GS_CHAT_FROM_HOST		= 15,	// 0x0F
		W3GS_START_LAG			= 16,	// 0x10
		W3GS_STOP_LAG			= 17,	// 0x11
		W3GS_HOST_KICK_PLAYER	= 28,	// 0x1C
		W3GS_REQJOIN			= 30,	// 0x1E
		W3GS_LEAVEGAME			= 33,	// 0x21
		W3GS_GAMELOADED_SELF	= 35,	// 0x23
		W3GS_OUTGOING_ACTION	= 38,	// 0x26
		W3GS_OUTGOING_KEEPALIVE	= 39,	// 0x27
		W3GS_CHAT_TO_HOST		= 40,	// 0x28
		W3GS_DROPREQ			= 41,	// 0x29
		W3GS_SEARCHGAME			= 47,	// 0x2F (UDP/LAN)
		W3GS_GAMEINFO			= 48,	// 0x30 (UDP/LAN)
		W3GS_CREATEGAME			= 49,	// 0x31 (UDP/LAN)
		W3GS_REFRESHGAME		= 50,	// 0x32 (UDP/LAN)
		W3GS_DECREATEGAME		= 51,	// 0x33 (UDP/LAN)
		W3GS_CHAT_OTHERS		= 52,	// 0x34
		W3GS_PING_FROM_OTHERS	= 53,	// 0x35
		W3GS_PONG_TO_OTHERS		= 54,	// 0x36
		W3GS_MAPCHECK			= 61,	// 0x3D
		W3GS_STARTDOWNLOAD		= 63,	// 0x3F
		W3GS_MAPSIZE			= 66,	// 0x42
		W3GS_MAPPART			= 67,	// 0x43
		W3GS_MAPPARTOK			= 68,	// 0x44
		W3GS_MAPPARTNOTOK		= 69,	// 0x45 - just a guess, received this packet after forgetting to send a crc in W3GS_MAPPART (f7 45 0a 00 01 02 01 00 00 00)
		W3GS_PONG_TO_HOST		= 70,	// 0x46
		W3GS_INCOMING_ACTION2	= 72	// 0x48 - received this packet when there are too many actions to fit in W3GS_INCOMING_ACTION
	};

	CGameProtocol( CGHost *nGHost );
	~CGameProtocol( );

	// receive functions

	CIncomingJoinPlayer *RECEIVE_W3GS_REQJOIN( const QByteArray &data );
	quint32 RECEIVE_W3GS_LEAVEGAME( const QByteArray &data );
	bool RECEIVE_W3GS_GAMELOADED_SELF( const QByteArray &data );
	CIncomingAction *RECEIVE_W3GS_OUTGOING_ACTION( const QByteArray &data, unsigned char PID );
	quint32 RECEIVE_W3GS_OUTGOING_KEEPALIVE( const QByteArray &data );
	CIncomingChatPlayer *RECEIVE_W3GS_CHAT_TO_HOST( const QByteArray &data );
	bool RECEIVE_W3GS_SEARCHGAME( const QByteArray &data, unsigned char war3Version );
	CIncomingMapSize *RECEIVE_W3GS_MAPSIZE( const QByteArray &data, const QByteArray &mapSize );
	quint32 RECEIVE_W3GS_MAPPARTOK( const QByteArray &data );
	quint32 RECEIVE_W3GS_PONG_TO_HOST( const QByteArray &data );

	// send functions

	QByteArray SEND_W3GS_PING_FROM_HOST( );
	QByteArray SEND_W3GS_SLOTINFOJOIN( unsigned char PID, const QByteArray & port, const QByteArray & externalIP, QList<CGameSlot> &lslots, quint32 randomSeed, unsigned char layoutStyle, unsigned char playerSlots );
	QByteArray SEND_W3GS_REJECTJOIN( quint32 reason );
	QByteArray SEND_W3GS_PLAYERINFO( unsigned char PID, const QString &name, const QByteArray & externalIP, const QByteArray & internalIP );
	QByteArray SEND_W3GS_PLAYERLEAVE_OTHERS( unsigned char PID, quint32 leftCode );
	QByteArray SEND_W3GS_GAMELOADED_OTHERS( unsigned char PID );
	QByteArray SEND_W3GS_SLOTINFO( const QList<CGameSlot> &lslots, quint32 randomSeed, unsigned char layoutStyle, unsigned char playerSlots );
	QByteArray SEND_W3GS_COUNTDOWN_START( );
	QByteArray SEND_W3GS_COUNTDOWN_END( );
	QByteArray SEND_W3GS_INCOMING_ACTION( const QList<CIncomingAction *> &actions, quint16 sendInterval );
	QByteArray SEND_W3GS_CHAT_FROM_HOST( unsigned char fromPID, const QByteArray & toPIDs, unsigned char flag, const QByteArray & flagExtra, const QString & message );
	QByteArray SEND_W3GS_START_LAG( const QList<CGamePlayer *> &players, bool loadInGame = false );
	QByteArray SEND_W3GS_STOP_LAG( CGamePlayer *player, bool loadInGame = false );
	QByteArray SEND_W3GS_SEARCHGAME( bool TFT, unsigned char war3Version );
	QByteArray SEND_W3GS_GAMEINFO( bool TFT, unsigned char war3Version, const QByteArray & mapGameType, const QByteArray & mapFlags, const QByteArray & mapWidth, const QByteArray & mapHeight, const QString & gameName, const QString & hostName, quint32 upTime, const QString & mapPath, const QByteArray & mapCRC, quint32 slotsTotal, quint32 slotsOpen, quint16 port, quint32 hostCounter );
	QByteArray SEND_W3GS_CREATEGAME( bool TFT, unsigned char war3Version );
	QByteArray SEND_W3GS_REFRESHGAME( quint32 players, quint32 playerSlots );
	QByteArray SEND_W3GS_DECREATEGAME( );
	QByteArray SEND_W3GS_MAPCHECK( const QString & mapPath, const QByteArray & mapSize, const QByteArray & mapInfo, const QByteArray & mapCRC, const QByteArray & mapSHA1 );
	QByteArray SEND_W3GS_STARTDOWNLOAD( unsigned char fromPID );
	QByteArray SEND_W3GS_MAPPART( unsigned char fromPID, unsigned char toPID, quint32 start, const QByteArray &mapData );
	QByteArray SEND_W3GS_INCOMING_ACTION2( const QList<CIncomingAction *> &actions );

	// other functions

private:
	bool AssignLength( QByteArray &content ) const;
	bool ValidateLength( const QByteArray &content ) const;
	QByteArray EncodeSlotInfo( const QList<CGameSlot> &slots, quint32 randomSeed, unsigned char layoutStyle, unsigned char playerSlots );
};

//
// CIncomingJoinPlayer
//

class CIncomingJoinPlayer
{
private:
	quint32 m_HostCounter;
	QString m_Name;
	QByteArray m_InternalIP;

public:
	CIncomingJoinPlayer( quint32 nHostCounter, const QString &nName, const QByteArray &nInternalIP );
	~CIncomingJoinPlayer( );

	quint32 GetHostCounter( ) const	{ return m_HostCounter; }
	const QString &GetName( ) const		{ return m_Name; }
	const QByteArray &GetInternalIP( )	const { return m_InternalIP; }
};

//
// CIncomingAction
//

class CIncomingAction
{
private:
	unsigned char m_PID;
	QByteArray m_CRC;
	QByteArray m_Action;

public:
	CIncomingAction( unsigned char nPID, const QByteArray &nCRC, const QByteArray &nAction );
	~CIncomingAction( );

	unsigned char GetPID( ) const	{ return m_PID; }
	const QByteArray &GetCRC( )	const	{ return m_CRC; }
	const QByteArray &GetAction( ) const	{ return m_Action; }
	quint32 GetLength( ) const	{ return m_Action.size( ) + 3; }
};

//
// CIncomingChatPlayer
//

class CIncomingChatPlayer
{
public:
	enum ChatToHostType
	{
		CTH_MESSAGE			= 0,	// a chat message
		CTH_MESSAGEEXTRA	= 1,	// a chat message with extra flags
		CTH_TEAMCHANGE		= 2,	// a team change request
		CTH_COLOURCHANGE	= 3,	// a colour change request
		CTH_RACECHANGE		= 4,	// a race change request
		CTH_HANDICAPCHANGE	= 5		// a handicap change request
	};

private:
	ChatToHostType m_Type;
	unsigned char m_FromPID;
	QByteArray m_ToPIDs;
	unsigned char m_Flag;
	QString m_Message;
	unsigned char m_Byte;
	QByteArray m_ExtraFlags;

public:
	CIncomingChatPlayer( unsigned char nFromPID, const QByteArray &nToPIDs, unsigned char nFlag, const QString &nMessage );
	CIncomingChatPlayer( unsigned char nFromPID, const QByteArray &nToPIDs, unsigned char nFlag, const QString &nMessage, const QByteArray &nExtraFlags );
	CIncomingChatPlayer( unsigned char nFromPID, const QByteArray &nToPIDs, unsigned char nFlag, unsigned char nByte );
	~CIncomingChatPlayer( );

	ChatToHostType GetType( ) const		{ return m_Type; }
	unsigned char GetFromPID( ) const		{ return m_FromPID; }
	const QByteArray &GetToPIDs( ) const			{ return m_ToPIDs; }
	unsigned char GetFlag( ) const		{ return m_Flag; }
	const QString &GetMessage( ) const			{ return m_Message; }
	unsigned char GetByte( ) const		{ return m_Byte; }
	const QByteArray &GetExtraFlags( ) const		{ return m_ExtraFlags; }
};

class CIncomingMapSize
{
private:
	unsigned char m_SizeFlag;
	quint32 m_MapSize;

public:
	CIncomingMapSize( unsigned char nSizeFlag, quint32 nMapSize );
	~CIncomingMapSize( );

	unsigned char GetSizeFlag( )	{ return m_SizeFlag; }
	quint32 GetMapSize( )			{ return m_MapSize; }
};

#endif
