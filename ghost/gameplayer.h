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

#ifndef GAMEPLAYER_H
#define GAMEPLAYER_H

class CCommandPacket;
class CGameProtocol;
class CGame;
class CIncomingJoinPlayer;

#include "includes.h"
class CBaseGame;
//
// CPotentialPlayer
//

#include <QTimer>
#include <QTcpSocket>

class CPotentialPlayer
	: public QObject
{
	Q_OBJECT

signals:
	void aboutToDelete();

public slots:
	void EventDataReady();
	virtual void EventConnectionError(QAbstractSocket::SocketError);
	virtual void EventConnectionClosed();
	virtual void deleteLater();

public:
	CGameProtocol *m_Protocol;
	CBaseGame *m_Game;
	QTimer m_TimeoutTimer, m_SendGProxyMessageTimer;

protected:
	// note: we permit m_Socket to be NULL in this class to allow for the virtual host player which doesn't really exist
	// it also allows us to convert CPotentialPlayers to CGamePlayers without the CPotentialPlayer's destructor closing the socket

	QTcpSocket *m_Socket;
	QQueue<CCommandPacket *> m_Packets;
	bool m_Error;
	QString m_ErrorString;
	CIncomingJoinPlayer *m_IncomingJoinPlayer;

public:
	CPotentialPlayer( CGameProtocol *nProtocol, CBaseGame *nGame, QTcpSocket *nSocket );
	virtual ~CPotentialPlayer( );

	virtual QTcpSocket *GetSocket( )				{ return m_Socket; }
	virtual QByteArray GetExternalIP( );
	virtual QString GetExternalIPString( );
	virtual QQueue<CCommandPacket *> GetPackets( )	{ return m_Packets; }
	virtual bool GetError( )						{ return m_Error; }
	virtual QString GetErrorString( )				{ return m_ErrorString; }
	virtual CIncomingJoinPlayer *GetJoinPlayer( )	{ return m_IncomingJoinPlayer; }

	virtual void SetSocket( QTcpSocket *nSocket )	{ m_Socket = nSocket; }

	// processing functions

	virtual void ExtractPackets( );
	virtual void ProcessPackets( );

	// other functions

	virtual void Send( const QByteArray &data );
};


//
// CGamePlayer
//
class CGamePlayer : public CPotentialPlayer
{
	Q_OBJECT

public slots:
	virtual void EventConnectionError(QAbstractSocket::SocketError);
	virtual void EventConnectionClosed();
	void EventConnectionTimeout();
	void EventWhoisTimeout();
	void EventACKTimeout();
	void EventPingTimeout();
	void EventSendGProxyMessage();
	void EventSpoofCheckTimeout();

signals:
	void finishedLoading();
	void stoppedLagging();

private:
	QTimer m_ACKTimer;
	QTimer m_PingTimer;
	void init();

private:
	unsigned char m_PID;
	QString m_Name;								// the player's name
	QByteArray m_InternalIP;						// the player's internal IP address as reported by the player when connecting
	QList<quint32> m_Pings;					// store the last few (20) pings received so we can take an average
	QQueue<quint32> m_CheckSums;				// the last few checksums the player has sent (for detecting desyncs)
	QString m_LeftReason;						// the reason the player left the game
	QString m_SpoofedRealm;						// the realm the player last spoof checked on
	QString m_JoinedRealm;						// the realm the player joined on (probable, can be spoofed)
	quint32 m_TotalPacketsSent;
	quint32 m_TotalPacketsReceived;
	quint32 m_LeftCode;						// the code to be sent in W3GS_PLAYERLEAVE_OTHERS for why this player left the game
	quint32 m_LoginAttempts;					// the number of attempts to login (used with CAdminGame only)
	quint32 m_SyncCounter;						// the number of keepalive packets received from this player
	quint32 m_JoinTime;						// GetTime when the player joined the game (used to delay sending the /whois a few seconds to allow for some lag)
	quint32 m_LastMapPartSent;					// the last mappart sent to the player (for sending more than one part at a time)
	quint32 m_LastMapPartAcked;				// the last mappart acknowledged by the player
	quint32 m_StartedDownloadingTicks;			// GetTicks when the player started downloading the map
	quint32 m_FinishedDownloadingTime;			// GetTime when the player finished downloading the map
	quint32 m_FinishedLoadingTicks;			// GetTicks when the player finished loading the game
	quint32 m_StartedLaggingTicks;				// GetTicks when the player started lagging
	quint32 m_StatsSentTime;					// GetTime when we sent this player's stats to the chat (to prevent players from spamming !stats)
	quint32 m_StatsDotASentTime;				// GetTime when we sent this player's dota stats to the chat (to prevent players from spamming !statsdota)
	quint32 m_LastGProxyWaitNoticeSentTime;
	QQueue<QByteArray> m_LoadInGameData;			// queued data to be sent when the player finishes loading when using "load in game"
	double m_Score;								// the player's generic "score" for the matchmaking algorithm
	bool m_LoggedIn;							// if the player has logged in or not (used with CAdminGame only)
	bool m_Spoofed;								// if the player has spoof checked or not
	bool m_Reserved;							// if the player is reserved (VIP) or not
	bool m_WhoisShouldBeSent;					// if a battle.net /whois should be sent for this player or not
	bool m_WhoisSent;							// if we've sent a battle.net /whois for this player yet (for spoof checking)
	bool m_DownloadAllowed;						// if we're allowed to download the map or not (used with permission based map downloads)
	bool m_DownloadStarted;						// if we've started downloading the map or not
	bool m_DownloadFinished;					// if we've finished downloading the map or not
	bool m_FinishedLoading;						// if the player has finished loading or not
	bool m_Lagging;								// if the player is lagging or not (on the lag screen)
	bool m_DropVote;							// if the player voted to drop the laggers or not (on the lag screen)
	bool m_KickVote;							// if the player voted to kick a player or not
	bool m_Muted;								// if the player is muted or not
	bool m_LeftMessageSent;						// if the playerleave message has been sent or not
	bool m_GProxy;								// if the player is using GProxy++
	bool m_GProxyDisconnectNoticeSent;			// if a disconnection notice has been sent or not when using GProxy++
	QQueue<QByteArray> m_GProxyBuffer;
	quint32 m_GProxyReconnectKey;
	quint32 m_LastGProxyAckTime;

public:
	CGamePlayer( CGameProtocol *nProtocol, CBaseGame *nGame, QTcpSocket *nSocket, unsigned char nPID, QString nJoinedRealm, QString nName, QByteArray nInternalIP, bool nReserved );
	CGamePlayer( CPotentialPlayer *potential, unsigned char nPID, QString nJoinedRealm, QString nName, QByteArray nInternalIP, bool nReserved );
	virtual ~CGamePlayer( );

	unsigned char GetPID( )						{ return m_PID; }
	QString GetName( )							{ return m_Name; }
	QByteArray GetInternalIP( )					{ return m_InternalIP; }
	unsigned int GetNumPings( )					{ return m_Pings.size( ); }
	unsigned int GetNumCheckSums( )				{ return m_CheckSums.size( ); }
	QQueue<quint32> *GetCheckSums( )			{ return &m_CheckSums; }
	QString GetLeftReason( )						{ return m_LeftReason; }
	QString GetSpoofedRealm( )					{ return m_SpoofedRealm; }
	QString GetJoinedRealm( )					{ return m_JoinedRealm; }
	quint32 GetLeftCode( )						{ return m_LeftCode; }
	quint32 GetLoginAttempts( )				{ return m_LoginAttempts; }
	quint32 GetSyncCounter( )					{ return m_SyncCounter; }
	quint32 GetJoinTime( )						{ return m_JoinTime; }
	quint32 GetLastMapPartSent( )				{ return m_LastMapPartSent; }
	quint32 GetLastMapPartAcked( )				{ return m_LastMapPartAcked; }
	quint32 GetStartedDownloadingTicks( )		{ return m_StartedDownloadingTicks; }
	quint32 GetFinishedDownloadingTime( )		{ return m_FinishedDownloadingTime; }
	quint32 GetFinishedLoadingTicks( )			{ return m_FinishedLoadingTicks; }
	quint32 GetStartedLaggingTicks( )			{ return m_StartedLaggingTicks; }
	quint32 GetStatsSentTime( )				{ return m_StatsSentTime; }
	quint32 GetStatsDotASentTime( )			{ return m_StatsDotASentTime; }
	quint32 GetLastGProxyWaitNoticeSentTime( )	{ return m_LastGProxyWaitNoticeSentTime; }
	QQueue<QByteArray> *GetLoadInGameData( )		{ return &m_LoadInGameData; }
	double GetScore( )							{ return m_Score; }
	bool GetLoggedIn( )							{ return m_LoggedIn; }
	bool GetSpoofed( )							{ return m_Spoofed; }
	bool GetReserved( )							{ return m_Reserved; }
	bool GetWhoisShouldBeSent( )				{ return m_WhoisShouldBeSent; }
	bool GetWhoisSent( )						{ return m_WhoisSent; }
	bool GetDownloadAllowed( )					{ return m_DownloadAllowed; }
	bool GetDownloadStarted( )					{ return m_DownloadStarted; }
	bool GetDownloadFinished( )					{ return m_DownloadFinished; }
	bool GetFinishedLoading( )					{ return m_FinishedLoading; }
	bool GetLagging( )							{ return m_Lagging; }
	bool GetDropVote( )							{ return m_DropVote; }
	bool GetKickVote( )							{ return m_KickVote; }
	bool GetMuted( )							{ return m_Muted; }
	bool GetLeftMessageSent( )					{ return m_LeftMessageSent; }
	bool GetGProxy( )							{ return m_GProxy; }
	bool GetGProxyDisconnectNoticeSent( )		{ return m_GProxyDisconnectNoticeSent; }
	quint32 GetGProxyReconnectKey( )			{ return m_GProxyReconnectKey; }

	void SetLeftReason( QString nLeftReason )										{ m_LeftReason = nLeftReason; }
	void SetSpoofedRealm( QString nSpoofedRealm )									{ m_SpoofedRealm = nSpoofedRealm; }
	void SetLeftCode( quint32 nLeftCode )											{ m_LeftCode = nLeftCode; }
	void SetLoginAttempts( quint32 nLoginAttempts )								{ m_LoginAttempts = nLoginAttempts; }
	void SetSyncCounter( quint32 nSyncCounter )									{ m_SyncCounter = nSyncCounter; }
	void SetLastMapPartSent( quint32 nLastMapPartSent )							{ m_LastMapPartSent = nLastMapPartSent; }
	void SetLastMapPartAcked( quint32 nLastMapPartAcked )							{ m_LastMapPartAcked = nLastMapPartAcked; }
	void SetStartedDownloadingTicks( quint32 nStartedDownloadingTicks )			{ m_StartedDownloadingTicks = nStartedDownloadingTicks; }
	void SetFinishedDownloadingTime( quint32 nFinishedDownloadingTime )			{ m_FinishedDownloadingTime = nFinishedDownloadingTime; }
	void SetStartedLaggingTicks( quint32 nStartedLaggingTicks )					{ m_StartedLaggingTicks = nStartedLaggingTicks; }
	void SetStatsSentTime( quint32 nStatsSentTime )								{ m_StatsSentTime = nStatsSentTime; }
	void SetStatsDotASentTime( quint32 nStatsDotASentTime )						{ m_StatsDotASentTime = nStatsDotASentTime; }
	void SetLastGProxyWaitNoticeSentTime( quint32 nLastGProxyWaitNoticeSentTime )	{ m_LastGProxyWaitNoticeSentTime = nLastGProxyWaitNoticeSentTime; }
	void SetScore( double nScore )													{ m_Score = nScore; }
	void SetLoggedIn( bool nLoggedIn )												{ m_LoggedIn = nLoggedIn; }
	void SetSpoofed( bool nSpoofed )												{ m_Spoofed = nSpoofed; }
	void SetReserved( bool nReserved )												{ m_Reserved = nReserved; }
	void SetWhoisShouldBeSent( bool nWhoisShouldBeSent )							{ m_WhoisShouldBeSent = nWhoisShouldBeSent; }
	void SetDownloadAllowed( bool nDownloadAllowed )								{ m_DownloadAllowed = nDownloadAllowed; }
	void SetDownloadStarted( bool nDownloadStarted )								{ m_DownloadStarted = nDownloadStarted; }
	void SetDownloadFinished( bool nDownloadFinished )								{ m_DownloadFinished = nDownloadFinished; }
	void SetLagging( bool nLagging )												{ m_Lagging = nLagging; }
	void SetDropVote( bool nDropVote )												{ m_DropVote = nDropVote; }
	void SetKickVote( bool nKickVote )												{ m_KickVote = nKickVote; }
	void SetMuted( bool nMuted )													{ m_Muted = nMuted; }
	void SetLeftMessageSent( bool nLeftMessageSent )								{ m_LeftMessageSent = nLeftMessageSent; }
	void SetGProxyDisconnectNoticeSent( bool nGProxyDisconnectNoticeSent )			{ m_GProxyDisconnectNoticeSent = nGProxyDisconnectNoticeSent; }

	QString GetNameTerminated( );
	quint32 GetPing( bool LCPing );

	void AddLoadInGameData( const QByteArray &nLoadInGameData )							{ m_LoadInGameData.enqueue( nLoadInGameData ); }

	// processing functions

	virtual void ExtractPackets( );
	virtual void ProcessPackets( );

	// other functions

	virtual void Send( const QByteArray &data );
	virtual void EventGProxyReconnect( QTcpSocket *NewSocket, quint32 LastPacket );
};

#endif
