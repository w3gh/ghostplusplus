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

#ifndef BNET_H
#define BNET_H

#include "includes.h"
#include <QTimer>
#include <QStringList>
#include <QTime>

class QTcpSocket;

class CCommandPacket;
class CBNCSUtilInterface;
class CBNETProtocol;
class CBNLSClient;
class CIncomingFriendList;
class CIncomingClanList;
class CIncomingChatEvent;
class CCallableAdminCount;
class CCallableAdminAdd;
class CCallableAdminRemove;
class CCallableAdminList;
class CCallableBanCount;
class CCallableBanAdd;
class CCallableBanRemove;
class CCallableBanList;
class CCallableGamePlayerSummaryCheck;
class CCallableDotAPlayerSummaryCheck;
class CDBBan;
class CGHost;
class CSaveGame;
class CBaseGame;
class CMap;
class CBNETPrivate;

typedef pair<QString,CCallableAdminCount *> PairedAdminCount;
typedef pair<QString,CCallableAdminAdd *> PairedAdminAdd;
typedef pair<QString,CCallableAdminRemove *> PairedAdminRemove;
typedef pair<QString,CCallableBanCount *> PairedBanCount;
typedef pair<QString,CCallableBanAdd *> PairedBanAdd;
typedef pair<QString,CCallableBanRemove *> PairedBanRemove;
typedef pair<QString,CCallableGamePlayerSummaryCheck *> PairedGPSCheck;
typedef pair<QString,CCallableDotAPlayerSummaryCheck *> PairedDPSCheck;

//
// CBNET
//

class CBNET : public QObject
{
	Q_OBJECT
private:
	Q_DECLARE_PRIVATE(CBNET);
signals:
	void SignalBnetCommand( CBNET *bnet, const QString &user, const QString &command, const QString &payload, bool whisper );
public slots:
	void socketConnected();
	void socketDisconnected();
	void socketDataReady();
	void socketConnect();
	void socketError();
	void sendWardenResponse(const QByteArray & response);
	void sendKeepAlivePacket();

	void EventCallableUpdateTimeout();
	void EventUpdateAdminList();
	void EventRefreshBanList();

private:
	QTimer m_KeepAliveTimer;
	QTimer m_CallableUpdateTimer, m_AdminListUpdateTimer, m_BanListRefreshTimer;

public:
	CGHost *m_GHost;

private:
	int m_Retries;
	QTcpSocket *m_Socket;							// the connection to battle.net
	CBNETProtocol *m_Protocol;						// battle.net protocol
	CBNLSClient *m_BNLSClient;						// the BNLS client (for external warden handling)
	QQueue<CCommandPacket *> m_Packets;				// queue of incoming packets
	CBNCSUtilInterface *m_BNCSUtil;					// the interface to the bncsutil library (used for logging into battle.net)
	QQueue<QByteArray> m_OutPackets;					// queue of outgoing packets to be sent (to prevent getting kicked for flooding)
	QList<CIncomingFriendList *> m_Friends;		// vector of friends
	QList<CIncomingClanList *> m_Clans;			// vector of clan members
	QList<PairedAdminCount> m_PairedAdminCounts;	// vector of paired threaded database admin counts in progress
	QList<PairedAdminAdd> m_PairedAdminAdds;		// vector of paired threaded database admin adds in progress
	QList<PairedAdminRemove> m_PairedAdminRemoves;	// vector of paired threaded database admin removes in progress
	QList<PairedBanCount> m_PairedBanCounts;		// vector of paired threaded database ban counts in progress
	QList<PairedBanAdd> m_PairedBanAdds;			// vector of paired threaded database ban adds in progress
	QList<PairedBanRemove> m_PairedBanRemoves;		// vector of paired threaded database ban removes in progress
	QList<PairedGPSCheck> m_PairedGPSChecks;		// vector of paired threaded database game player summary checks in progress
	QList<PairedDPSCheck> m_PairedDPSChecks;		// vector of paired threaded database DotA player summary checks in progress
	CCallableAdminList *m_CallableAdminList;		// threaded database admin list in progress
	CCallableBanList *m_CallableBanList;			// threaded database ban list in progress
	QStringList m_Admins;						// vector of cached admins
	QStringList m_RootAdmins;						// vector of rootadmins
	QList<CDBBan *> m_Bans;						// vector of cached bans
	QString m_Server;								// battle.net server to connect to
	QString m_ServerIP;								// battle.net server to connect to (the IP address so we don't have to resolve it every time we connect)
	QString m_ServerAlias;							// battle.net server alias (short name, e.g. "USEast")
	QString m_BNLSServer;							// BNLS server to connect to (for warden handling)
	quint16 m_BNLSPort;							// BNLS port
	quint32 m_BNLSWardenCookie;					// BNLS warden cookie
	QByteArray m_CDKeyROC;								// ROC CD key
	QByteArray m_CDKeyTFT;								// TFT CD key
	QString m_CountryAbbrev;							// country abbreviation
	QString m_Country;								// country
	quint32 m_LocaleID;							// see: http://msdn.microsoft.com/en-us/library/0h88fahh%28VS.85%29.aspx
	QString m_UserName;								// battle.net username
	QString m_UserPassword;							// battle.net password
	QString m_FirstChannel;							// the first chat channel to join upon entering chat (note: we hijack this to store the last channel when entering a game)
	QString m_CurrentChannel;						// the current chat channel
	QString m_RootAdmin;								// the root admin
	char m_CommandTrigger;							// the character prefix to identify commands
	unsigned char m_War3Version;					// custom warcraft 3 version for PvPGN users
	QByteArray m_EXEVersion;							// custom exe version for PvPGN users
	QByteArray m_EXEVersionHash;						// custom exe version hash for PvPGN users
	QString m_PasswordHashType;						// password hash type for PvPGN users
	QString m_PVPGNRealmName;						// realm name for PvPGN users (for mutual friend spoofchecks)
	int m_MaxMessageLength;					// maximum message length for PvPGN users
	quint32 m_HostCounterID;						// the host counter ID to identify players from this realm
	quint32 m_LastDisconnectedTime;				// GetTime when we were last disconnected from battle.net
	quint32 m_LastConnectionAttemptTime;			// GetTime when we last attempted to connect to battle.net
	quint32 m_LastNullTime;						// GetTime when the last null packet was sent for detecting disconnects
	quint32 m_LastOutPacketSize;
	quint32 m_LastAdminRefreshTime;				// GetTime when the admin list was last refreshed from the database
	quint32 m_LastBanRefreshTime;					// GetTime when the ban list was last refreshed from the database
	bool m_LoggedIn;								// if we've logged into battle.net or not
	bool m_InChat;									// if we've entered chat or not (but we're not necessarily in a chat channel yet)
	bool m_HoldFriends;								// whether to auto hold friends when creating a game or not
	bool m_HoldClan;								// whether to auto hold clan members when creating a game or not
	bool m_PublicCommands;							// whether to allow public commands or not

	inline int getWaitTicks()
	{
		// this formula has changed many times but currently we wait 1 second if the last packet was "small", 3.5 seconds if it was "medium", and 4 seconds if it was "big"
		if( m_LastOutPacketSize < 10 )
			return 1000;

		else if( m_LastOutPacketSize < 100 )
			return 3500;

		return 4000;
	}

	QTime m_LastPacketSent;

	void ResetSocket();

public slots:
	void EnqueuePacket(const QByteArray &pkg);
	void SendPacket();

public:
	CBNET( CGHost *nGHost, const QString &nServer, const QString &nServerAlias, const QString &nBNLSServer, quint16 nBNLSPort, quint32 nBNLSWardenCookie, const QString &nCDKeyROC, const QString &nCDKeyTFT, const QString &nCountryAbbrev, const QString &nCountry, quint32 nLocaleID, const QString &nUserName, const QString &nUserPassword, const QString &nFirstChannel, const QString &nRootAdmin, char nCommandTrigger, bool nHoldFriends, bool nHoldClan, bool nPublicCommands, unsigned char nWar3Version, const QByteArray &nEXEVersion, const QByteArray &nEXEVersionHash, const QString &nPasswordHashType, const QString &nPVPGNRealmName, quint32 nMaxMessageLength, quint32 nHostCounterID );
	virtual ~CBNET( );

	const QString &GetServer( ) const					{ return m_Server; }
	const QString &GetServerAlias( ) const				{ return m_ServerAlias; }
	QString GetCDKeyROC( ) const				{ return m_CDKeyROC; }
	QString GetCDKeyTFT( ) const				{ return m_CDKeyTFT; }
	const QString &GetUserName( ) const				{ return m_UserName; }
	const QString &GetUserPassword( ) const			{ return m_UserPassword; }
	const QString &GetFirstChannel( ) const			{ return m_FirstChannel; }
	const QString &GetCurrentChannel( ) const			{ return m_CurrentChannel; }
	const QString &GetRootAdmin( )	const				{ return m_RootAdmin; }
	char GetCommandTrigger( ) const				{ return m_CommandTrigger; }
	const QByteArray &GetEXEVersion( )	const			{ return m_EXEVersion; }
	const QByteArray &GetEXEVersionHash( )	const		{ return m_EXEVersionHash; }
	const QString &GetPasswordHashType( ) const		{ return m_PasswordHashType; }
	const QString &GetPVPGNRealmName( ) const			{ return m_PVPGNRealmName; }
	quint32 GetHostCounterID( )	const			{ return m_HostCounterID; }
	bool GetLoggedIn( )	const					{ return m_LoggedIn; }
	bool GetInChat( ) const						{ return m_InChat; }
	bool GetHoldFriends( ) const				{ return m_HoldFriends; }
	bool GetHoldClan( )	const					{ return m_HoldClan; }
	bool GetPublicCommands( ) const				{ return m_PublicCommands; }
	quint32 GetOutPacketsQueued( ) const		{ return m_OutPackets.size( ); }
	const QByteArray &GetUniqueName( ) const;

	// processing functions

	void ExtractPackets( );
	void ProcessPackets( );
	void ProcessChatEvent( CIncomingChatEvent *chatEvent );

	// functions to send packets to battle.net

	void SendJoinChannel( const QString &channel );
	void SendGetFriendsList( );
	void SendGetClanList( );
	void QueueEnterChat( );
	void QueueChatCommand( const QString &chatCommand );
	void QueueChatCommand( const QString &chatCommand, const QString &user, bool whisper );
	void QueueGameCreate( unsigned char state, const QString &gameName, const QString &hostName, CMap *map, CSaveGame *saveGame, quint32 hostCounter );
	void QueueGameRefresh( unsigned char state, const QString &gameName, const QString &hostName, CMap *map, CSaveGame *saveGame, quint32 upTime, quint32 hostCounter );
	void QueueGameUncreate( );

	void UnqueuePackets( unsigned char type );
	void UnqueueChatCommand( const QString &chatCommand );
	void UnqueueGameRefreshes( );

	// other functions

	bool IsAdmin( const QString &name ) const;
	bool IsRootAdmin( const QString &name ) const;
	CDBBan *IsBannedName( const QString &name ) const;
	CDBBan *IsBannedIP( const QString &ip ) const;
	void AddAdmin( const QString &name );
	void AddBan( const QString &name, const QString &ip, const QString &gamename, const QString &admin, const QString &reason );
	void RemoveAdmin( const QString &name );
	void RemoveBan( const QString &name );
	void HoldFriends( CBaseGame *game );
	void HoldClan( CBaseGame *game );
};

#endif
