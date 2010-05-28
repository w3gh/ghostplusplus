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

#ifndef BNETPROTOCOL_H
#define BNETPROTOCOL_H

//
// CBNETProtocol
//

#include "includes.h"
#define BNET_HEADER_CONSTANT 255

class CIncomingGameHost;
class CIncomingChatEvent;
class CIncomingFriendList;
class CIncomingClanList;

class CBNETProtocol
{
public:
	enum Protocol {
		SID_NULL					= 0,	// 0x0
		SID_STOPADV					= 2,	// 0x2
		SID_GETADVLISTEX			= 9,	// 0x9
		SID_ENTERCHAT				= 10,	// 0xA
		SID_JOINCHANNEL				= 12,	// 0xC
		SID_CHATCOMMAND				= 14,	// 0xE
		SID_CHATEVENT				= 15,	// 0xF
		SID_CHECKAD					= 21,	// 0x15
		SID_STARTADVEX3				= 28,	// 0x1C
		SID_DISPLAYAD				= 33,	// 0x21
		SID_NOTIFYJOIN				= 34,	// 0x22
		SID_PING					= 37,	// 0x25
		SID_LOGONRESPONSE			= 41,	// 0x29
		SID_NETGAMEPORT				= 69,	// 0x45
		SID_AUTH_INFO				= 80,	// 0x50
		SID_AUTH_CHECK				= 81,	// 0x51
		SID_AUTH_ACCOUNTLOGON		= 83,	// 0x53
		SID_AUTH_ACCOUNTLOGONPROOF	= 84,	// 0x54
		SID_WARDEN					= 94,	// 0x5E
		SID_FRIENDSLIST				= 101,	// 0x65
		SID_FRIENDSUPDATE			= 102,	// 0x66
		SID_CLANMEMBERLIST			= 125,	// 0x7D
		SID_CLANMEMBERSTATUSCHANGE	= 127	// 0x7F
	};

	enum KeyResult {
		KR_GOOD				= 0,
		KR_OLD_GAME_VERSION	= 256,
		KR_INVALID_VERSION	= 257,
		KR_ROC_KEY_IN_USE	= 513,
		KR_TFT_KEY_IN_USE	= 529
	};

	enum IncomingChatEvent {
		EID_SHOWUSER			= 1,	// received when you join a channel (includes users in the channel and their information)
		EID_JOIN				= 2,	// received when someone joins the channel you're currently in
		EID_LEAVE				= 3,	// received when someone leaves the channel you're currently in
		EID_WHISPER				= 4,	// received a whisper message
		EID_TALK				= 5,	// received when someone talks in the channel you're currently in
		EID_BROADCAST			= 6,	// server broadcast
		EID_CHANNEL				= 7,	// received when you join a channel (includes the channel's name, flags)
		EID_USERFLAGS			= 9,	// user flags updates
		EID_WHISPERSENT			= 10,	// sent a whisper message
		EID_CHANNELFULL			= 13,	// channel is full
		EID_CHANNELDOESNOTEXIST	= 14,	// channel does not exist
		EID_CHANNELRESTRICTED	= 15,	// channel is restricted
		EID_INFO				= 18,	// broadcast/information message
		EID_ERROR				= 19,	// error message
		EID_EMOTE				= 23	// emote
	};

private:
	QByteArray m_ClientToken;			// set in constructor
	QByteArray m_LogonType;				// set in RECEIVE_SID_AUTH_INFO
	QByteArray m_ServerToken;			// set in RECEIVE_SID_AUTH_INFO
	QByteArray m_MPQFileTime;			// set in RECEIVE_SID_AUTH_INFO
	QByteArray m_IX86VerFileName;		// set in RECEIVE_SID_AUTH_INFO
	QByteArray m_ValueStringFormula;	// set in RECEIVE_SID_AUTH_INFO
	QByteArray m_KeyState;				// set in RECEIVE_SID_AUTH_CHECK
	QByteArray m_KeyStateDescription;	// set in RECEIVE_SID_AUTH_CHECK
	QByteArray m_Salt;					// set in RECEIVE_SID_AUTH_ACCOUNTLOGON
	QByteArray m_ServerPublicKey;		// set in RECEIVE_SID_AUTH_ACCOUNTLOGON
	QByteArray m_UniqueName;			// set in RECEIVE_SID_ENTERCHAT

public:
	CBNETProtocol( );
	~CBNETProtocol( );

	const QByteArray &GetClientToken( )				{ return m_ClientToken; }
	const QByteArray &GetLogonType( )					{ return m_LogonType; }
	const QByteArray &GetServerToken( )				{ return m_ServerToken; }
	const QByteArray &GetMPQFileTime( )				{ return m_MPQFileTime; }
	const QByteArray &GetIX86VerFileName( )			{ return m_IX86VerFileName; }
	const QByteArray &GetIX86VerFileNameString( )		{ return m_IX86VerFileName; }
	const QByteArray &GetValueStringFormula( )			{ return m_ValueStringFormula; }
	const QByteArray &GetValueStringFormulaString( )	{ return m_ValueStringFormula; }
	const QByteArray &GetKeyState( )					{ return m_KeyState; }
	const QByteArray &GetKeyStateDescription( )		{ return m_KeyStateDescription; }
	const QByteArray &GetSalt( )						{ return m_Salt; }
	const QByteArray &GetServerPublicKey( )			{ return m_ServerPublicKey; }
	const QByteArray &GetUniqueName( )					{ return m_UniqueName; }

	// receive functions

	bool RECEIVE_SID_NULL( const QByteArray &data );
	CIncomingGameHost *RECEIVE_SID_GETADVLISTEX( const QByteArray &data );
	bool RECEIVE_SID_ENTERCHAT( const QByteArray &data );
	CIncomingChatEvent *RECEIVE_SID_CHATEVENT( const QByteArray &data );
	bool RECEIVE_SID_CHECKAD( const QByteArray &data );
	bool RECEIVE_SID_STARTADVEX3( const QByteArray &data );
	QByteArray RECEIVE_SID_PING( const QByteArray &data );
	bool RECEIVE_SID_LOGONRESPONSE( const QByteArray &data );
	bool RECEIVE_SID_AUTH_INFO( const QByteArray &data );
	bool RECEIVE_SID_AUTH_CHECK( const QByteArray &data );
	bool RECEIVE_SID_AUTH_ACCOUNTLOGON( const QByteArray &data );
	bool RECEIVE_SID_AUTH_ACCOUNTLOGONPROOF( const QByteArray &data );
	QByteArray RECEIVE_SID_WARDEN( const QByteArray &data );
	QList<CIncomingFriendList *> RECEIVE_SID_FRIENDSLIST( const QByteArray &data );
	QList<CIncomingClanList *> RECEIVE_SID_CLANMEMBERLIST( const QByteArray &data );
	CIncomingClanList *RECEIVE_SID_CLANMEMBERSTATUSCHANGE( const QByteArray &data );

	// send functions

	QByteArray SEND_PROTOCOL_INITIALIZE_SELECTOR( );
	QByteArray SEND_SID_NULL( );
	QByteArray SEND_SID_STOPADV( );
	QByteArray SEND_SID_GETADVLISTEX( const QString &gameName );
	QByteArray SEND_SID_ENTERCHAT( );
	QByteArray SEND_SID_JOINCHANNEL( const QString &channel );
	QByteArray SEND_SID_CHATCOMMAND( const QString &command );
	QByteArray SEND_SID_CHECKAD( );
	QByteArray SEND_SID_STARTADVEX3( unsigned char state, const QByteArray &mapGameType, const QByteArray &mapFlags, const QByteArray &mapWidth, const QByteArray &mapHeight, const QString &gameName, const QString &hostName, quint32 upTime, const QString &mapPath, const QByteArray &mapCRC, const QByteArray &mapSHA1, quint32 hostCounter );
	QByteArray SEND_SID_NOTIFYJOIN( const QString &gameName );
	QByteArray SEND_SID_PING( const QByteArray &pingValue );
	QByteArray SEND_SID_LOGONRESPONSE( QByteArray clientToken, QByteArray serverToken, QByteArray passwordHash, QString accountName );
	QByteArray SEND_SID_NETGAMEPORT( quint16 serverPort );
	QByteArray SEND_SID_AUTH_INFO( unsigned char ver, bool TFT, quint32 localeID, QString countryAbbrev, QString country );
	QByteArray SEND_SID_AUTH_CHECK( bool TFT, QByteArray clientToken, QByteArray exeVersion, QByteArray exeVersionHash, QByteArray keyInfoROC, QByteArray keyInfoTFT, QString exeInfo, QString keyOwnerName );
	QByteArray SEND_SID_AUTH_ACCOUNTLOGON( QByteArray clientPublicKey, QString accountName );
	QByteArray SEND_SID_AUTH_ACCOUNTLOGONPROOF( QByteArray clientPasswordProof );
	QByteArray SEND_SID_WARDEN( QByteArray wardenResponse );
	QByteArray SEND_SID_FRIENDSLIST( );
	QByteArray SEND_SID_CLANMEMBERLIST( );

	// other functions

private:
	bool AssignLength( QByteArray &content ) const;
	bool ValidateLength( const QByteArray &content ) const;
};

//
// CIncomingGameHost
//

class CIncomingGameHost
{
private:
	QByteArray m_IP;
	quint16 m_Port;
	QString m_GameName;
	QByteArray m_HostCounter;

public:
	CIncomingGameHost( QByteArray &nIP, quint16 nPort, QString nGameName, QByteArray &nHostCounter );
	~CIncomingGameHost( );

	QByteArray GetIP( )			{ return m_IP; }
	QString GetIPString( );
	quint16 GetPort( )			{ return m_Port; }
	QString GetGameName( )		{ return m_GameName; }
	QByteArray GetHostCounter( )	{ return m_HostCounter; }
};

//
// CIncomingChatEvent
//

class CIncomingChatEvent
{
private:
	CBNETProtocol :: IncomingChatEvent m_ChatEvent;
	quint32 m_Ping;
	QString m_User;
	QString m_Message;

public:
	CIncomingChatEvent( CBNETProtocol :: IncomingChatEvent nChatEvent, quint32 nPing, QString nUser, QString nMessage );
	~CIncomingChatEvent( );

	CBNETProtocol :: IncomingChatEvent GetChatEvent( )	{ return m_ChatEvent; }
	quint32 GetPing( )									{ return m_Ping; }
	QString GetUser( )									{ return m_User; }
	QString GetMessage( )								{ return m_Message; }
};

//
// CIncomingFriendList
//

class CIncomingFriendList
{
private:
	QString m_Account;
	unsigned char m_Status;
	unsigned char m_Area;
	QString m_Location;

public:
	CIncomingFriendList( QString nAccount, unsigned char nStatus, unsigned char nArea, QString nLocation );
	~CIncomingFriendList( );

	QString GetAccount( )		{ return m_Account; }
	unsigned char GetStatus( )	{ return m_Status; }
	unsigned char GetArea( )	{ return m_Area; }
	QString GetLocation( )		{ return m_Location; }
	QString GetDescription( );

private:
	QString ExtractStatus( unsigned char status );
	QString ExtractArea( unsigned char area );
	QString ExtractLocation( QString location );
};

//
// CIncomingClanList
//

class CIncomingClanList
{
private:
	QString m_Name;
	unsigned char m_Rank;
	unsigned char m_Status;

public:
	CIncomingClanList( QString nName, unsigned char nRank, unsigned char nStatus );
	~CIncomingClanList( );

	QString GetName( )			{ return m_Name; }
	QString GetRank( );
	QString GetStatus( );
	QString GetDescription( );
};

#endif
