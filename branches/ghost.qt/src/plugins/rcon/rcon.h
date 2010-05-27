/*
 *  commands.h
 *  ghost
 *
 *  Created by Lucas on 08.05.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef RCON_H
#define RCON_H

#include <QObject>
#include "interfaces.h"
#include <QHostAddress>
//#include "ghost.h"

QT_FORWARD_DECLARE_CLASS(QUdpSocket)
class CConfig;
class CGHost;
class CRemoteConsole;

class CRemoteConsoleClient
{
public:
	CRemoteConsoleClient( const CRemoteConsole &rcon, const QHostAddress &address, quint16 port, qint32 banTime, bool authed );
	
	//virtual void Ping( )                                            { m_LastPinged = 000; }
	//virtual void Pong( )                                            { m_LastReceived = 000; }
	
	virtual void Ban( int seconds )                                 { m_BannedUntil = 000 + seconds; }
	virtual bool IsBanned( ) const                                       { return false; }
	virtual void UnBan( )                                           { m_BannedUntil = 0; }
	
	virtual bool IsAuthed( ) const                                       { return m_Authed; }
	virtual void SetAuthed( bool authed )							{ m_Authed = authed; }
	
	virtual const time_t &GetLastPinged( ) const                                { return m_LastPinged; }
	virtual const time_t &GetLastReceived( ) const                              { return m_LastReceived; }
	
	virtual const QHostAddress &GetAddress( ) const					{ return m_Address; }
	virtual const quint16 &GetPort( ) const							{ return m_Port; }
	//virtual bool hasAddress(const QHostAddress &a, const quint16 &port)		{ return m_Address == a && m_Port == port; }
	//virtual void Send( const QString &message )                             {  }
private:
	const CRemoteConsole &m_RCon;
	QHostAddress m_Address;
	quint16 m_Port;
	time_t m_LastReceived;
	time_t m_LastPinged;
	time_t m_BannedUntil;
	bool m_Authed;
};

class CRemoteConsole : public QObject, public ILogPlugin
{
	Q_OBJECT
	Q_INTERFACES(IGHostPlugin)
	Q_INTERFACES(ILogPlugin)
public:
	virtual void LogInfo( const QString &message );
	virtual void LogWarning( const QString &message );
	virtual void LogError( const QString &message );
private slots:
	void readPendingDatagrams();
public:
	CRemoteConsole( );
	virtual ~CRemoteConsole( );
	virtual QString GetName() const;
	virtual void Send( const QString &message );	// output a message to all authed clients
	virtual void PluginLoaded( CGHost *ghost, CConfig *cfg );
	//virtual void ConsoleOutput( const QString &message );
private:
	CGHost *m_GHost;																						// our parent
	QUdpSocket *m_UDPSocket;																				// the socket we use to communicate
	QString m_Password;																						// the password users need to authenticate themselves with
	bool m_AnonymousBroadcast;																				// should everyone requesting a LAN broadcast packet be granted their wish?
	bool m_AnonymousAdminGame;																				// should the admin game also be broadcasted to anonymous requesters?
	int m_KeepAliveTime;																					// time that has to pass before we start asking the client if he is still there
	int m_KeepAliveInterval;																				// time to wait between ping packets when asking if client is still there
	int m_Timeout;																							// time in which the client has to repond to our ping(s)
	QList<CRemoteConsoleClient *> m_KnownClients;															// all our console clients (includes authed but banned clients)
	void Send( const QString &message, CRemoteConsoleClient *client );												// output a message only to a specific client
	static void SplitLine( const QString &input, QString &command, QString &payload, QString &target );			// splits a line received by the rcon socket up into pieces
	void ProcessPacket( const QByteArray &data, const QHostAddress &sender, const quint16 &senderPort );
	//virtual void ProcessInput( string cmd, string payload, string target, CRemoteConsoleClient *sender );	// does the actual action based on rcon input
};

#endif
