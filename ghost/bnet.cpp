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
#include "config.h"
#include "language.h"
#include "commandpacket.h"
#include "ghostdb.h"
#include "bncsutilinterface.h"
#include "bnlsclient.h"
#include "bnetprotocol.h"
#include "bnet.h"
#include "map.h"
#include "packed.h"
#include "savegame.h"
#include "replay.h"
#include "gameprotocol.h"
#include "game_base.h"
#include "includes.h"

#include <QRegExp>
#include <QDir>
#include <QStringList>

//
// CBNET
//

CBNET :: CBNET( CGHost *nGHost, QString nServer, QString nServerAlias, QString nBNLSServer, quint16 nBNLSPort, quint32 nBNLSWardenCookie, QString nCDKeyROC, QString nCDKeyTFT, QString nCountryAbbrev, QString nCountry, quint32 nLocaleID, QString nUserName, QString nUserPassword, QString nFirstChannel, QString nRootAdmin, char nCommandTrigger, bool nHoldFriends, bool nHoldClan, bool nPublicCommands, unsigned char nWar3Version, QByteArray nEXEVersion, QByteArray nEXEVersionHash, QString nPasswordHashType, QString nPVPGNRealmName, quint32 nMaxMessageLength, quint32 nHostCounterID )
	: QObject(NULL)
{
	// todotodo: append path seperator to Warcraft3Path if needed
	m_Retries = 0;
	m_GHost = nGHost;
	m_Socket = new QTcpSocket( );

	QObject::connect(m_Socket, SIGNAL(connected()), this, SLOT(socketConnected()));
	QObject::connect(m_Socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
	QObject::connect(m_Socket, SIGNAL(readyRead()), this, SLOT(socketDataReady()));
	QObject::connect(m_Socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError()));

	m_CallableUpdateTimer.setInterval(200);
	m_CallableUpdateTimer.start();
	m_AdminListUpdateTimer.setInterval(60000);
	m_AdminListUpdateTimer.start();
	m_BanListRefreshTimer.setInterval(3600000);
	m_BanListRefreshTimer.start();
	m_NULLTimer.setInterval(60000);
	QObject::connect(&m_NULLTimer, SIGNAL(timeout()), this, SLOT(timeout_NULL()));

	QObject::connect(&m_CallableUpdateTimer, SIGNAL(timeout()), this, SLOT(EventCallableUpdateTimeout()));
	QObject::connect(&m_AdminListUpdateTimer, SIGNAL(timeout()), this, SLOT(EventUpdateAdminList()));
	QObject::connect(&m_BanListRefreshTimer, SIGNAL(timeout()), this, SLOT(EventRefreshBanList()));

	m_Protocol = new CBNETProtocol( );
	m_BNLSClient = NULL;
	m_BNCSUtil = new CBNCSUtilInterface( nUserName, nUserPassword );
	m_CallableAdminList = m_GHost->m_DB->ThreadedAdminList( nServer );
	m_CallableBanList = m_GHost->m_DB->ThreadedBanList( nServer );
	m_Server = nServer;
	QString LowerServer = m_Server;
	LowerServer = LowerServer.toLower();

	if( !nServerAlias.isEmpty( ) )
		m_ServerAlias = nServerAlias;
	else if( LowerServer == "useast.battle.net" )
		m_ServerAlias = "USEast";
	else if( LowerServer == "uswest.battle.net" )
		m_ServerAlias = "USWest";
	else if( LowerServer == "asia.battle.net" )
		m_ServerAlias = "Asia";
	else if( LowerServer == "europe.battle.net" )
		m_ServerAlias = "Europe";
	else
		m_ServerAlias = m_Server;

	if( nPasswordHashType == "pvpgn" && !nBNLSServer.isEmpty( ) )
	{
		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] pvpgn connection found with a configured BNLS server, ignoring BNLS server" );
		nBNLSServer.clear( );
		nBNLSPort = 0;
		nBNLSWardenCookie = 0;
	}

	m_BNLSServer = nBNLSServer;
	m_BNLSPort = nBNLSPort;
	m_BNLSWardenCookie = nBNLSWardenCookie;
	m_CDKeyROC = nCDKeyROC.toAscii();
	m_CDKeyTFT = nCDKeyTFT.toAscii();

	// remove dashes from CD keys and convert to uppercase

	m_CDKeyROC = m_CDKeyROC.replace('-', "").toLower();
	m_CDKeyTFT = m_CDKeyTFT.replace('-', "").toLower();

	if( m_CDKeyROC.size( ) != 26 )
		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] warning - your ROC CD key is not 26 characters long and is probably invalid" );

	if( m_GHost->m_TFT && m_CDKeyTFT.size( ) != 26 )
		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] warning - your TFT CD key is not 26 characters long and is probably invalid" );

	m_CountryAbbrev = nCountryAbbrev;
	m_Country = nCountry;
	m_LocaleID = nLocaleID;
	m_UserName = nUserName;
	m_UserPassword = nUserPassword;
	m_FirstChannel = nFirstChannel;
	m_RootAdmin = nRootAdmin;
	m_RootAdmin = m_RootAdmin.toLower();
	m_CommandTrigger = nCommandTrigger;
	m_War3Version = nWar3Version;
	m_EXEVersion = nEXEVersion;
	m_EXEVersionHash = nEXEVersionHash;
	m_PasswordHashType = nPasswordHashType;
	m_PVPGNRealmName = nPVPGNRealmName;
	m_MaxMessageLength = nMaxMessageLength;
	m_HostCounterID = nHostCounterID;
	m_LastDisconnectedTime = 0;
	m_LastConnectionAttemptTime = 0;
	m_LastOutPacketSize = 0;
	m_LastAdminRefreshTime = GetTime( );
	m_LastBanRefreshTime = GetTime( );
	m_FirstConnect = true;
	m_LoggedIn = false;
	m_InChat = false;
	m_HoldFriends = nHoldFriends;
	m_HoldClan = nHoldClan;
	m_PublicCommands = nPublicCommands;
}

CBNET :: ~CBNET( )
{
	delete m_Socket;
	delete m_Protocol;
	delete m_BNLSClient;

	while( !m_Packets.isEmpty( ) )
	{
		delete m_Packets.front( );
		m_Packets.dequeue( );
	}

	delete m_BNCSUtil;

	for( QList<CIncomingFriendList *> :: const_iterator i = m_Friends.begin( ); i != m_Friends.end( ); i++ )
		delete *i;

	for( QList<CIncomingClanList *> :: const_iterator i = m_Clans.begin( ); i != m_Clans.end( ); i++ )
		delete *i;

	for( QList<PairedAdminCount> :: const_iterator i = m_PairedAdminCounts.begin( ); i != m_PairedAdminCounts.end( ); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for( QList<PairedAdminAdd> :: const_iterator i = m_PairedAdminAdds.begin( ); i != m_PairedAdminAdds.end( ); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for( QList<PairedAdminRemove> :: const_iterator i = m_PairedAdminRemoves.begin( ); i != m_PairedAdminRemoves.end( ); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for( QList<PairedBanCount> :: const_iterator i = m_PairedBanCounts.begin( ); i != m_PairedBanCounts.end( ); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for( QList<PairedBanAdd> :: const_iterator i = m_PairedBanAdds.begin( ); i != m_PairedBanAdds.end( ); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for( QList<PairedBanRemove> :: const_iterator i = m_PairedBanRemoves.begin( ); i != m_PairedBanRemoves.end( ); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for( QList<PairedGPSCheck> :: const_iterator i = m_PairedGPSChecks.begin( ); i != m_PairedGPSChecks.end( ); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for( QList<PairedDPSCheck> :: const_iterator i = m_PairedDPSChecks.begin( ); i != m_PairedDPSChecks.end( ); i++ )
		m_GHost->m_Callables.push_back( i->second );

	if( m_CallableAdminList )
		m_GHost->m_Callables.push_back( m_CallableAdminList );

	if( m_CallableBanList )
		m_GHost->m_Callables.push_back( m_CallableBanList );

	for( QList<CDBBan *> :: const_iterator i = m_Bans.begin( ); i != m_Bans.end( ); i++ )
		delete *i;
}

void CBNET::socketConnect()
{
	// attempt to connect to battle.net

	m_FirstConnect = false;
	CONSOLE_Print( "[BNET: " + m_ServerAlias + "] connecting to server [" + m_Server + "] on port 6112" );
	m_GHost->EventBNETConnecting( this );

	if( m_ServerIP.isEmpty( ) )
		m_Socket->connectToHost( m_Server, 6112 );

	else
	{
		// use cached server IP address since resolving takes time and is blocking

		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] using cached server IP address " + m_ServerIP );
		m_Socket->connectToHost( m_ServerIP, 6112 );
	}

	if (m_Socket->waitForConnected(15000))
	{
		m_ServerIP = m_Socket->peerAddress().toString();
		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] resolved and cached server IP address " + m_ServerIP );
	}

	else
	{
		// the connection attempt timed out (15 seconds)

		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] connect timed out" );
		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] waiting 90 seconds to reconnect" );
		m_GHost->EventBNETConnectTimedOut( this );
		m_Socket->abort();
		m_Socket->reset();
		m_Socket->deleteLater();
		m_Socket = NULL;
		m_LastDisconnectedTime = GetTime( );
		QTimer::singleShot(90000, this, SLOT(socketConnect()));
		m_NULLTimer.stop();
	}

	m_LastConnectionAttemptTime = GetTime( );
	m_Retries++;
}

void CBNET::timeout_NULL()
{
	m_Socket->write( m_Protocol->SEND_SID_NULL( ) );
}

void CBNET::socketConnected()
{
	// the connection attempt completed

	CONSOLE_Print( "[BNET: " + m_ServerAlias + "] connected" );
	m_GHost->EventBNETConnected( this );
	m_Socket->write( m_Protocol->SEND_PROTOCOL_INITIALIZE_SELECTOR( ) );
	m_Socket->write( m_Protocol->SEND_SID_AUTH_INFO( m_War3Version, m_GHost->m_TFT, m_LocaleID, m_CountryAbbrev, m_Country ) );
	m_NULLTimer.start();

	while( !m_OutPackets.isEmpty( ) )
		m_OutPackets.dequeue( );

	QTimer::singleShot(500, m_GHost, SLOT(EventAutoHost()));
}

void CBNET::socketDisconnected()
{
	// the socket was disconnected

	CONSOLE_Print( "[BNET: " + m_ServerAlias + "] disconnected from battle.net" );
	CONSOLE_Print( "[BNET: " + m_ServerAlias + "] waiting 90 seconds to reconnect" );
	m_GHost->EventBNETDisconnected( this );
	delete m_BNLSClient;
	m_BNLSClient = NULL;
	m_BNCSUtil->Reset( m_UserName, m_UserPassword );
	m_Socket->abort();
	m_Socket->deleteLater();
	m_Socket = NULL;
	m_LastDisconnectedTime = GetTime( );
	m_LoggedIn = false;
	m_InChat = false;
	m_NULLTimer.stop();

	if (m_Retries > 6)
	{
		CONSOLE_Print("[BNET: " + m_ServerAlias + "] giving up after 5 failed retries, waiting for 10 minutes to reconnect" );
		QTimer::singleShot(600000, this, SLOT(socketConnect()));
		m_Retries = 0;
		return;
	}

	QTimer::singleShot(90000, this, SLOT(socketConnect()));

}

void CBNET::sendWardenResponse(const QByteArray & response)
{
	if (m_Socket->state() != QAbstractSocket::ConnectedState)
		return;

	m_Socket->write( m_Protocol->SEND_SID_WARDEN( response ) );
}

void CBNET::socketDataReady()
{
	// the socket is connected and everything appears to be working properly

	ExtractPackets( );
	ProcessPackets( );
}

void CBNET::EnqueuePacket(const QByteArray &pkg)
{
	int ticks = getWaitTicks();
	int pkgs = m_OutPackets.empty();

	m_OutPackets.enqueue(pkg);

	if (pkgs > 0)
		return;

	if (m_LastPacketSent.elapsed() >= ticks)
	{
		SendPacket();
		return;
	}

	QTimer::singleShot(ticks - m_LastPacketSent.elapsed(), this, SLOT(SendPacket()));
}

void CBNET::SendPacket()
{
	if (m_OutPackets.size() == 0)
	{
		DEBUG_Print("Nice, empty query... but actually, this shouldn't happen...");
		return;
	}

	if( m_OutPackets.size( ) > 7 )
		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] packet queue warning - there are " + QString::number( m_OutPackets.size( ) ) + " packets waiting to be sent" );

	m_Socket->write( m_OutPackets.front() );
	m_LastOutPacketSize = m_OutPackets.front( ).size( );
	m_OutPackets.dequeue( );
	m_LastPacketSent.restart();

	if (m_OutPackets.size() > 0)
		QTimer::singleShot(getWaitTicks(), this, SLOT(SendPacket()));
}

void CBNET::socketError()
{
	// the socket has an error

	CONSOLE_Print( "[BNET: " + m_ServerAlias + "] disconnected from battle.net due to socket error: " + m_Socket->errorString() );

	if( m_Socket->error() == QAbstractSocket::RemoteHostClosedError && GetTime( ) - m_LastConnectionAttemptTime <= 15 )
		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] warning - you are probably temporarily IP banned from battle.net" );

	CONSOLE_Print( "[BNET: " + m_ServerAlias + "] waiting 90 seconds to reconnect" );
	m_GHost->EventBNETDisconnected( this );
	delete m_BNLSClient;
	m_BNLSClient = NULL;
	m_BNCSUtil->Reset( m_UserName, m_UserPassword );
	m_Socket->abort();
	m_Socket->deleteLater();
	m_Socket = NULL;
	m_LastDisconnectedTime = GetTime( );
	m_LoggedIn = false;
	m_InChat = false;
	m_NULLTimer.stop();

	QTimer::singleShot(90000, this, SLOT(socketConnect()));
}

QByteArray CBNET :: GetUniqueName( )
{
	return m_Protocol->GetUniqueName( );
}

void CBNET::EventCallableUpdateTimeout()
{

	//
	// update callables
	//

	for( QList<PairedAdminCount> :: iterator i = m_PairedAdminCounts.begin( ); i != m_PairedAdminCounts.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			quint32 Count = i->second->GetResult( );

			if( Count == 0 )
				QueueChatCommand( m_GHost->m_Language->ThereAreNoAdmins( m_Server ), i->first, !i->first.isEmpty( ) );
			else if( Count == 1 )
				QueueChatCommand( m_GHost->m_Language->ThereIsAdmin( m_Server ), i->first, !i->first.isEmpty( ) );
			else
				QueueChatCommand( m_GHost->m_Language->ThereAreAdmins( m_Server, QString::number( Count ) ), i->first, !i->first.isEmpty( ) );

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedAdminCounts.erase( i );
		}
		else
			i++;
	}

	for( QList<PairedAdminAdd> :: iterator i = m_PairedAdminAdds.begin( ); i != m_PairedAdminAdds.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			if( i->second->GetResult( ) )
			{
				AddAdmin( i->second->GetUser( ) );
				QueueChatCommand( m_GHost->m_Language->AddedUserToAdminDatabase( m_Server, i->second->GetUser( ) ), i->first, !i->first.isEmpty( ) );
			}
			else
				QueueChatCommand( m_GHost->m_Language->ErrorAddingUserToAdminDatabase( m_Server, i->second->GetUser( ) ), i->first, !i->first.isEmpty( ) );

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedAdminAdds.erase( i );
		}
		else
			i++;
	}

	for( QList<PairedAdminRemove> :: iterator i = m_PairedAdminRemoves.begin( ); i != m_PairedAdminRemoves.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			if( i->second->GetResult( ) )
			{
				RemoveAdmin( i->second->GetUser( ) );
				QueueChatCommand( m_GHost->m_Language->DeletedUserFromAdminDatabase( m_Server, i->second->GetUser( ) ), i->first, !i->first.isEmpty( ) );
			}
			else
				QueueChatCommand( m_GHost->m_Language->ErrorDeletingUserFromAdminDatabase( m_Server, i->second->GetUser( ) ), i->first, !i->first.isEmpty( ) );

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedAdminRemoves.erase( i );
		}
		else
			i++;
	}

	for( QList<PairedBanCount> :: iterator i = m_PairedBanCounts.begin( ); i != m_PairedBanCounts.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			quint32 Count = i->second->GetResult( );

			if( Count == 0 )
				QueueChatCommand( m_GHost->m_Language->ThereAreNoBannedUsers( m_Server ), i->first, !i->first.isEmpty( ) );
			else if( Count == 1 )
				QueueChatCommand( m_GHost->m_Language->ThereIsBannedUser( m_Server ), i->first, !i->first.isEmpty( ) );
			else
				QueueChatCommand( m_GHost->m_Language->ThereAreBannedUsers( m_Server, QString::number( Count ) ), i->first, !i->first.isEmpty( ) );

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedBanCounts.erase( i );
		}
		else
			i++;
	}

	for( QList<PairedBanAdd> :: iterator i = m_PairedBanAdds.begin( ); i != m_PairedBanAdds.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			if( i->second->GetResult( ) )
			{
				AddBan( i->second->GetUser( ), i->second->GetIP( ), i->second->GetGameName( ), i->second->GetAdmin( ), i->second->GetReason( ) );
				QueueChatCommand( m_GHost->m_Language->BannedUser( i->second->GetServer( ), i->second->GetUser( ) ), i->first, !i->first.isEmpty( ) );
			}
			else
				QueueChatCommand( m_GHost->m_Language->ErrorBanningUser( i->second->GetServer( ), i->second->GetUser( ) ), i->first, !i->first.isEmpty( ) );

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedBanAdds.erase( i );
		}
		else
			i++;
	}

	for( QList<PairedBanRemove> :: iterator i = m_PairedBanRemoves.begin( ); i != m_PairedBanRemoves.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			if( i->second->GetResult( ) )
			{
				RemoveBan( i->second->GetUser( ) );
				QueueChatCommand( m_GHost->m_Language->UnbannedUser( i->second->GetUser( ) ), i->first, !i->first.isEmpty( ) );
			}
			else
				QueueChatCommand( m_GHost->m_Language->ErrorUnbanningUser( i->second->GetUser( ) ), i->first, !i->first.isEmpty( ) );

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedBanRemoves.erase( i );
		}
		else
			i++;
	}

	for( QList<PairedGPSCheck> :: iterator i = m_PairedGPSChecks.begin( ); i != m_PairedGPSChecks.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			CDBGamePlayerSummary *GamePlayerSummary = i->second->GetResult( );

			if( GamePlayerSummary )
				QueueChatCommand( m_GHost->m_Language->HasPlayedGamesWithThisBot( i->second->GetName( ), GamePlayerSummary->GetFirstGameDateTime( ), GamePlayerSummary->GetLastGameDateTime( ), QString::number( GamePlayerSummary->GetTotalGames( ) ), QString::number( (float)GamePlayerSummary->GetAvgLoadingTime( ) / 1000, 'g', 2 ), QString::number( GamePlayerSummary->GetAvgLeftPercent( ) ) ), i->first, !i->first.isEmpty( ) );
			else
				QueueChatCommand( m_GHost->m_Language->HasntPlayedGamesWithThisBot( i->second->GetName( ) ), i->first, !i->first.isEmpty( ) );

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedGPSChecks.erase( i );
		}
		else
			i++;
	}

	for( QList<PairedDPSCheck> :: iterator i = m_PairedDPSChecks.begin( ); i != m_PairedDPSChecks.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			CDBDotAPlayerSummary *DotAPlayerSummary = i->second->GetResult( );

			if( DotAPlayerSummary )
			{
				QString Summary = m_GHost->m_Language->HasPlayedDotAGamesWithThisBot(	i->second->GetName( ),
																						QString::number( DotAPlayerSummary->GetTotalGames( ) ),
																						QString::number( DotAPlayerSummary->GetTotalWins( ) ),
																						QString::number( DotAPlayerSummary->GetTotalLosses( ) ),
																						QString::number( DotAPlayerSummary->GetTotalKills( ) ),
																						QString::number( DotAPlayerSummary->GetTotalDeaths( ) ),
																						QString::number( DotAPlayerSummary->GetTotalCreepKills( ) ),
																						QString::number( DotAPlayerSummary->GetTotalCreepDenies( ) ),
																						QString::number( DotAPlayerSummary->GetTotalAssists( ) ),
																						QString::number( DotAPlayerSummary->GetTotalNeutralKills( ) ),
																						QString::number( DotAPlayerSummary->GetTotalTowerKills( ) ),
																						QString::number( DotAPlayerSummary->GetTotalRaxKills( ) ),
																						QString::number( DotAPlayerSummary->GetTotalCourierKills( ) ),
																						QString::number( DotAPlayerSummary->GetAvgKills( ), 'g', 2 ),
																						QString::number( DotAPlayerSummary->GetAvgDeaths( ), 'g', 2 ),
																						QString::number( DotAPlayerSummary->GetAvgCreepKills( ), 'g', 2 ),
																						QString::number( DotAPlayerSummary->GetAvgCreepDenies( ), 'g', 2 ),
																						QString::number( DotAPlayerSummary->GetAvgAssists( ), 'g', 2 ),
																						QString::number( DotAPlayerSummary->GetAvgNeutralKills( ), 'g', 2 ),
																						QString::number( DotAPlayerSummary->GetAvgTowerKills( ), 'g', 2 ),
																						QString::number( DotAPlayerSummary->GetAvgRaxKills( ), 'g', 2 ),
																						QString::number( DotAPlayerSummary->GetAvgCourierKills( ), 'g', 2 ) );

				QueueChatCommand( Summary, i->first, !i->first.isEmpty( ) );
			}
			else
				QueueChatCommand( m_GHost->m_Language->HasntPlayedDotAGamesWithThisBot( i->second->GetName( ) ), i->first, !i->first.isEmpty( ) );

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedDPSChecks.erase( i );
		}
		else
			i++;
	}

	if( m_CallableAdminList && m_CallableAdminList->GetReady( ) )
	{
		// CONSOLE_Print( "[BNET: " + m_ServerAlias + "] refreshed admin list (" + QString::number( m_Admins.size( ) ) + " -> " + QString::number( m_CallableAdminList->GetResult( ).size( ) ) + " admins)" );
		m_Admins = m_CallableAdminList->GetResult( );
		m_GHost->m_DB->RecoverCallable( m_CallableAdminList );
		delete m_CallableAdminList;
		m_CallableAdminList = NULL;
		m_LastAdminRefreshTime = GetTime( );
	}

	if( m_CallableBanList && m_CallableBanList->GetReady( ) )
	{
		// CONSOLE_Print( "[BNET: " + m_ServerAlias + "] refreshed ban list (" + QString::number( m_Bans.size( ) ) + " -> " + QString::number( m_CallableBanList->GetResult( ).size( ) ) + " bans)" );

		for( QList<CDBBan *> :: const_iterator i = m_Bans.begin( ); i != m_Bans.end( ); i++ )
			delete *i;

		m_Bans = m_CallableBanList->GetResult( );
		m_GHost->m_DB->RecoverCallable( m_CallableBanList );
		delete m_CallableBanList;
		m_CallableBanList = NULL;
		m_LastBanRefreshTime = GetTime( );
	}
}

void CBNET::EventUpdateAdminList()
{
	// refresh the admin list every 5 minutes

	if( !m_CallableAdminList )
		m_CallableAdminList = m_GHost->m_DB->ThreadedAdminList( m_Server );
}

void CBNET::EventRefreshBanList()
{
	// refresh the ban list every 60 minutes

	if( !m_CallableBanList )
		m_CallableBanList = m_GHost->m_DB->ThreadedBanList( m_Server );
}

void CBNET :: ExtractPackets( )
{
	// a packet is at least 4 bytes so loop as long as the buffer contains 4 bytes

	while( m_Socket->bytesAvailable() >= 4 )
	{
		// byte 0 is always 255
		QByteArray header = m_Socket->peek(4);

		if( (unsigned char)header.at(0) != BNET_HEADER_CONSTANT )
		{
			CONSOLE_Print( "[BNET: " + m_ServerAlias + "] error - received invalid packet from battle.net (bad header constant), disconnecting" );
			m_Socket->abort();
			return;
		}

		// bytes 2 and 3 contain the length of the packet

		quint16 Length = Util::extractUInt16(header, 2);

		if( Length < 4 )
		{
			CONSOLE_Print( "[BNET: " + m_ServerAlias + "] error - received invalid packet from battle.net (bad length), disconnecting" );
			m_Socket->abort();
			return;
		}

		if( m_Socket->bytesAvailable() < Length )
			return;

		QByteArray Bytes = m_Socket->read(Length);
		m_Packets.enqueue( new CCommandPacket( BNET_HEADER_CONSTANT, Bytes.at(1), Bytes ) );
	}
}

void CBNET :: ProcessPackets( )
{
	CIncomingGameHost *GameHost = NULL;
	CIncomingChatEvent *ChatEvent = NULL;
	QByteArray WardenData;
	QList<CIncomingFriendList *> Friends;
	QList<CIncomingClanList *> Clans;

	// process all the received packets in the m_Packets queue
	// this normally means sending some kind of response

	while( !m_Packets.isEmpty( ) )
	{
		CCommandPacket *Packet = m_Packets.front( );
		m_Packets.dequeue( );

		if( Packet->GetPacketType( ) == BNET_HEADER_CONSTANT )
		{
			switch( Packet->GetID( ) )
			{
			case CBNETProtocol :: SID_NULL:
				// warning: we do not respond to NULL packets with a NULL packet of our own
				// this is because PVPGN servers are programmed to respond to NULL packets so it will create a vicious cycle of useless traffic
				// official battle.net servers do not respond to NULL packets

				m_Protocol->RECEIVE_SID_NULL( Packet->GetData( ) );
				break;

			case CBNETProtocol :: SID_GETADVLISTEX:
				GameHost = m_Protocol->RECEIVE_SID_GETADVLISTEX( Packet->GetData( ) );

				if( GameHost )
					CONSOLE_Print( "[BNET: " + m_ServerAlias + "] joining game [" + GameHost->GetGameName( ) + "]" );

				delete GameHost;
				GameHost = NULL;
				break;

			case CBNETProtocol :: SID_ENTERCHAT:
				if( m_Protocol->RECEIVE_SID_ENTERCHAT( Packet->GetData( ) ) )
				{
					CONSOLE_Print( "[BNET: " + m_ServerAlias + "] joining channel [" + m_FirstChannel + "]" );
					m_InChat = true;
					m_Socket->write( m_Protocol->SEND_SID_JOINCHANNEL( m_FirstChannel ) );
				}

				break;

			case CBNETProtocol :: SID_CHATEVENT:
				ChatEvent = m_Protocol->RECEIVE_SID_CHATEVENT( Packet->GetData( ) );

				if( ChatEvent )
					ProcessChatEvent( ChatEvent );

				delete ChatEvent;
				ChatEvent = NULL;
				break;

			case CBNETProtocol :: SID_CHECKAD:
				m_Protocol->RECEIVE_SID_CHECKAD( Packet->GetData( ) );
				break;

			case CBNETProtocol :: SID_STARTADVEX3:
				if( m_Protocol->RECEIVE_SID_STARTADVEX3( Packet->GetData( ) ) )
				{
					m_InChat = false;
					m_GHost->EventBNETGameRefreshed( this );
				}
				else
				{
					CONSOLE_Print( "[BNET: " + m_ServerAlias + "] startadvex3 failed" );
					m_GHost->EventBNETGameRefreshFailed( this );
				}

				break;

			case CBNETProtocol :: SID_PING:
				m_Socket->write( m_Protocol->SEND_SID_PING( m_Protocol->RECEIVE_SID_PING( Packet->GetData( ) ) ) );
				break;

			case CBNETProtocol :: SID_AUTH_INFO:
				if( m_Protocol->RECEIVE_SID_AUTH_INFO( Packet->GetData( ) ) )
				{
					if( m_BNCSUtil->HELP_SID_AUTH_CHECK( m_GHost->m_TFT, m_GHost->m_Warcraft3Path, m_CDKeyROC, m_CDKeyTFT,
														 m_Protocol->GetValueStringFormulaString( ), m_Protocol->GetIX86VerFileNameString( ),
														 m_Protocol->GetClientToken( ), m_Protocol->GetServerToken( ) ) )
					{
						// override the exe information generated by bncsutil if specified in the config file
						// apparently this is useful for pvpgn users

						if( m_EXEVersion.size( ) == 4 )
						{
							CONSOLE_Print( "[BNET: " + m_ServerAlias + "] using custom exe version bnet_custom_exeversion = " + QString::number( m_EXEVersion[0] ) + " " + QString::number( m_EXEVersion[1] ) + " " + QString::number( m_EXEVersion[2] ) + " " + QString::number( m_EXEVersion[3] ) );
							m_BNCSUtil->SetEXEVersion( m_EXEVersion );
						}

						if( m_EXEVersionHash.size( ) == 4 )
						{
							CONSOLE_Print( "[BNET: " + m_ServerAlias + "] using custom exe version hash bnet_custom_exeversionhash = " + QString::number( m_EXEVersionHash[0] ) + " " + QString::number( m_EXEVersionHash[1] ) + " " + QString::number( m_EXEVersionHash[2] ) + " " + QString::number( m_EXEVersionHash[3] ) );
							m_BNCSUtil->SetEXEVersionHash( m_EXEVersionHash );
						}

						if( m_GHost->m_TFT )
							CONSOLE_Print( "[BNET: " + m_ServerAlias + "] attempting to auth as Warcraft III: The Frozen Throne" );
						else
							CONSOLE_Print( "[BNET: " + m_ServerAlias + "] attempting to auth as Warcraft III: Reign of Chaos" );

						m_Socket->write( m_Protocol->SEND_SID_AUTH_CHECK( m_GHost->m_TFT, m_Protocol->GetClientToken( ), m_BNCSUtil->GetEXEVersion( ),
																		  m_BNCSUtil->GetEXEVersionHash( ), m_BNCSUtil->GetKeyInfoROC( ), m_BNCSUtil->GetKeyInfoTFT( ),
																		  m_BNCSUtil->GetEXEInfo( ), "GHost" ) );

						// the Warden seed is the first 4 bytes of the ROC key hash
						// initialize the Warden handler

						if( !m_BNLSServer.isEmpty( ) )
						{
							CONSOLE_Print( "[BNET: " + m_ServerAlias + "] creating BNLS client" );
							delete m_BNLSClient;
							m_BNLSClient = new CBNLSClient( m_BNLSServer, m_BNLSPort, m_BNLSWardenCookie );
							m_BNLSClient->QueueWardenSeed( Util::extractUInt32(m_BNCSUtil->GetKeyInfoROC( ), 16) );

							QObject::connect(m_BNLSClient, SIGNAL(newWardenResponse(QByteArray)), this, SLOT(sendWardenResponse(QByteArray)));
						}
					}
					else
					{
						CONSOLE_Print( "[BNET: " + m_ServerAlias + "] logon failed - bncsutil key hash failed (check your Warcraft 3 path and cd keys), disconnecting" );
						m_Socket->abort();
						delete Packet;
						return;
					}
				}

				break;

			case CBNETProtocol :: SID_AUTH_CHECK:
				if( m_Protocol->RECEIVE_SID_AUTH_CHECK( Packet->GetData( ) ) )
				{
					// cd keys accepted

					CONSOLE_Print( "[BNET: " + m_ServerAlias + "] cd keys accepted" );
					m_BNCSUtil->HELP_SID_AUTH_ACCOUNTLOGON( );
					m_Socket->write( m_Protocol->SEND_SID_AUTH_ACCOUNTLOGON( m_BNCSUtil->GetClientKey( ), m_UserName ) );
				}
				else
				{
					// cd keys not accepted

					switch( Util::extractUInt32(m_Protocol->GetKeyState( )) )
					{
					case CBNETProtocol :: KR_ROC_KEY_IN_USE:
						CONSOLE_Print( "[BNET: " + m_ServerAlias + "] logon failed - ROC CD key in use by user [" + m_Protocol->GetKeyStateDescription( ) + "], disconnecting" );
						break;
					case CBNETProtocol :: KR_TFT_KEY_IN_USE:
						CONSOLE_Print( "[BNET: " + m_ServerAlias + "] logon failed - TFT CD key in use by user [" + m_Protocol->GetKeyStateDescription( ) + "], disconnecting" );
						break;
					case CBNETProtocol :: KR_OLD_GAME_VERSION:
						CONSOLE_Print( "[BNET: " + m_ServerAlias + "] logon failed - game version is too old, disconnecting" );
						break;
					case CBNETProtocol :: KR_INVALID_VERSION:
						CONSOLE_Print( "[BNET: " + m_ServerAlias + "] logon failed - game version is invalid, disconnecting" );
						break;
					default:
						CONSOLE_Print( "[BNET: " + m_ServerAlias + "] logon failed - cd keys not accepted ("+QString::number(Util::extractUInt32(m_Protocol->GetKeyState( )))+"), disconnecting" );
						break;
					}

					m_Socket->abort();
					delete Packet;
					return;
				}

				break;

			case CBNETProtocol :: SID_AUTH_ACCOUNTLOGON:
				if( m_Protocol->RECEIVE_SID_AUTH_ACCOUNTLOGON( Packet->GetData( ) ) )
				{
					CONSOLE_Print( "[BNET: " + m_ServerAlias + "] username [" + m_UserName + "] accepted" );

					if( m_PasswordHashType == "pvpgn" )
					{
						// pvpgn logon

						CONSOLE_Print( "[BNET: " + m_ServerAlias + "] using pvpgn logon type (for pvpgn servers only)" );
						m_BNCSUtil->HELP_PvPGNPasswordHash( m_UserPassword.toAscii() );
						m_Socket->write( m_Protocol->SEND_SID_AUTH_ACCOUNTLOGONPROOF( m_BNCSUtil->GetPvPGNPasswordHash( ) ) );
					}
					else
					{
						// battle.net logon

						CONSOLE_Print( "[BNET: " + m_ServerAlias + "] using battle.net logon type (for official battle.net servers only)" );
						m_BNCSUtil->HELP_SID_AUTH_ACCOUNTLOGONPROOF( m_Protocol->GetSalt( ), m_Protocol->GetServerPublicKey( ) );
						m_Socket->write( m_Protocol->SEND_SID_AUTH_ACCOUNTLOGONPROOF( m_BNCSUtil->GetM1( ) ) );
					}
				}
				else
				{
					CONSOLE_Print( "[BNET: " + m_ServerAlias + "] logon failed - invalid username, disconnecting" );
					m_Socket->abort( );
					delete Packet;
					return;
				}

				break;

			case CBNETProtocol :: SID_AUTH_ACCOUNTLOGONPROOF:
				if( m_Protocol->RECEIVE_SID_AUTH_ACCOUNTLOGONPROOF( Packet->GetData( ) ) )
				{
					// logon successful

					CONSOLE_Print( "[BNET: " + m_ServerAlias + "] logon successful" );
					m_LoggedIn = true;
					m_GHost->EventBNETLoggedIn( this );
					m_Socket->write( m_Protocol->SEND_SID_NETGAMEPORT( m_GHost->m_HostPort ) );
					m_Socket->write( m_Protocol->SEND_SID_ENTERCHAT( ) );
					m_Socket->write( m_Protocol->SEND_SID_FRIENDSLIST( ) );
					m_Socket->write( m_Protocol->SEND_SID_CLANMEMBERLIST( ) );
				}
				else
				{
					CONSOLE_Print( "[BNET: " + m_ServerAlias + "] logon failed - invalid password, disconnecting" );

					// try to figure out if the user might be using the wrong logon type since too many people are confused by this

					QString Server = m_Server;
					Server = Server.toLower();

					if( m_PasswordHashType == "pvpgn" && ( Server == "useast.battle.net" || Server == "uswest.battle.net" || Server == "asia.battle.net" || Server == "europe.battle.net" ) )
						CONSOLE_Print( "[BNET: " + m_ServerAlias + "] it looks like you're trying to connect to a battle.net server using a pvpgn logon type, check your config file's \"battle.net custom data\" section" );
					else if( m_PasswordHashType != "pvpgn" && ( Server != "useast.battle.net" && Server != "uswest.battle.net" && Server != "asia.battle.net" && Server != "europe.battle.net" ) )
						CONSOLE_Print( "[BNET: " + m_ServerAlias + "] it looks like you're trying to connect to a pvpgn server using a battle.net logon type, check your config file's \"battle.net custom data\" section" );

					m_Socket->abort( );
					delete Packet;
					return;
				}

				break;

			case CBNETProtocol :: SID_WARDEN:
				WardenData = m_Protocol->RECEIVE_SID_WARDEN( Packet->GetData( ) );

				if( m_BNLSClient )
					m_BNLSClient->QueueWardenRaw( WardenData );
				else
					CONSOLE_Print( "[BNET: " + m_ServerAlias + "] warning - received warden packet but no BNLS server is available, you will be kicked from battle.net soon" );

				break;

			case CBNETProtocol :: SID_FRIENDSLIST:
				Friends = m_Protocol->RECEIVE_SID_FRIENDSLIST( Packet->GetData( ) );

				for( QList<CIncomingFriendList *> :: const_iterator i = m_Friends.begin( ); i != m_Friends.end( ); i++ )
					delete *i;

				m_Friends = Friends;
				break;

			case CBNETProtocol :: SID_CLANMEMBERLIST:
				QList<CIncomingClanList *> Clans = m_Protocol->RECEIVE_SID_CLANMEMBERLIST( Packet->GetData( ) );

				for( QList<CIncomingClanList *> :: const_iterator i = m_Clans.begin( ); i != m_Clans.end( ); i++ )
					delete *i;

				m_Clans = Clans;
				break;
			}
		}

		delete Packet;
	}
}

void CBNET :: ProcessChatEvent( CIncomingChatEvent *chatEvent )
{
	CBNETProtocol :: IncomingChatEvent Event = chatEvent->GetChatEvent( );
	bool Whisper = ( Event == CBNETProtocol :: EID_WHISPER );
	QString User = chatEvent->GetUser( );
	QString Message = chatEvent->GetMessage( );

	if( Event == CBNETProtocol :: EID_WHISPER || Event == CBNETProtocol :: EID_TALK )
	{
		if( Event == CBNETProtocol :: EID_WHISPER )
		{
			CONSOLE_Print( "[WHISPER: " + m_ServerAlias + "] [" + User + "] " + Message );
			m_GHost->EventBNETWhisper( this, User, Message );
		}
		else
		{
			CONSOLE_Print( "[LOCAL: " + m_ServerAlias + "] [" + User + "] " + Message );
			m_GHost->EventBNETChat( this, User, Message );
		}

		// handle spoof checking for current game
		// this case covers whispers - we assume that anyone who sends a whisper to the bot with message "spoofcheck" should be considered spoof checked
		// note that this means you can whisper "spoofcheck" even in a public game to manually spoofcheck if the /whois fails

		if( Event == CBNETProtocol :: EID_WHISPER && m_GHost->m_CurrentGame )
		{
			if( Message == "s" || Message == "sc" || Message == "spoof" || Message == "check" || Message == "spoofcheck" )
				m_GHost->m_CurrentGame->AddToSpoofed( m_Server, User, true );
			else if( Message.indexOf( m_GHost->m_CurrentGame->GetGameName( ) ) != -1 )
			{
				// look for messages like "entered a Warcraft III The Frozen Throne game called XYZ"
				// we don't look for the English part of the text anymore because we want this to work with multiple languages
				// it's a pretty safe bet that anyone whispering the bot with a message containing the game name is a valid spoofcheck

				if( m_PasswordHashType == "pvpgn" && User == m_PVPGNRealmName )
				{
					// the equivalent pvpgn message is: [PvPGN Realm] Your friend abc has entered a Warcraft III Frozen Throne game named "xyz".

					QList<QString> Tokens = UTIL_Tokenize( Message, ' ' );

					if( Tokens.size( ) >= 3 )
						m_GHost->m_CurrentGame->AddToSpoofed( m_Server, Tokens[2], false );
				}
				else
					m_GHost->m_CurrentGame->AddToSpoofed( m_Server, User, false );
			}
		}

		// handle bot commands

		if( Message == "?trigger" && ( IsAdmin( User ) || IsRootAdmin( User ) || ( m_PublicCommands && m_OutPackets.size( ) <= 3 ) ) )
			QueueChatCommand( m_GHost->m_Language->CommandTrigger( QString( 1, m_CommandTrigger ) ), User, Whisper );
		else if( !Message.isEmpty( ) && Message[0] == m_CommandTrigger )
		{
			// extract the command trigger, the command, and the payload
			// e.g. "!say hello world" -> command: "say", payload: "hello world"

			QString Command;
			QString Payload;
			int PayloadStart = Message.indexOf( " " );

			if( PayloadStart != -1 )
			{
				Command = Message.mid( 1, PayloadStart - 1 );
				Payload = Message.mid( PayloadStart + 1 );
			}
			else
				Command = Message.mid( 1 );

			Command = Command.toLower();

			if( IsAdmin( User ) || IsRootAdmin( User ) )
			{
				CONSOLE_Print( "[BNET: " + m_ServerAlias + "] admin [" + User + "] sent command [" + Message + "]" );

				/*****************
				* ADMIN COMMANDS *
				******************/

				//
				// !ADDADMIN
				//

				if( Command == "addadmin" && !Payload.isEmpty( ) )
				{
					if( IsRootAdmin( User ) )
					{
						if( IsAdmin( Payload ) )
							QueueChatCommand( m_GHost->m_Language->UserIsAlreadyAnAdmin( m_Server, Payload ), User, Whisper );
						else
							m_PairedAdminAdds.push_back( PairedAdminAdd( Whisper ? User : QString( ), m_GHost->m_DB->ThreadedAdminAdd( m_Server, Payload ) ) );
					}
					else
						QueueChatCommand( m_GHost->m_Language->YouDontHaveAccessToThatCommand( ), User, Whisper );
				}

				//
				// !ADDBAN
				// !BAN
				//

				if( ( Command == "addban" || Command == "ban" ) && !Payload.isEmpty( ) )
				{
					// extract the victim and the reason
					// e.g. "Varlock leaver after dying" -> victim: "Varlock", reason: "leaver after dying"

					QString Victim;
					QString Reason;
					QTextStream SS(&Payload);
					SS >> Victim;

					if( !SS.atEnd( ) )
					{
						Reason = SS.readLine();
						int Start = Reason.indexOf( QRegExp( "[^ ]" ));

						if( Start != -1 )
							Reason = Reason.mid( Start );
					}

					if( IsBannedName( Victim ) )
						QueueChatCommand( m_GHost->m_Language->UserIsAlreadyBanned( m_Server, Victim ), User, Whisper );
					else
						m_PairedBanAdds.push_back( PairedBanAdd( Whisper ? User : QString( ), m_GHost->m_DB->ThreadedBanAdd( m_Server, Victim, QString( ), QString( ), User, Reason ) ) );
				}

				//
				// !ANNOUNCE
				//

				if( Command == "announce" && m_GHost->m_CurrentGame && !m_GHost->m_CurrentGame->GetCountDownStarted( ) )
				{
					if( Payload.isEmpty( ) || Payload == "off" )
					{
						QueueChatCommand( m_GHost->m_Language->AnnounceMessageDisabled( ), User, Whisper );
						m_GHost->m_CurrentGame->SetAnnounce( 0, QString( ) );
					}
					else
					{
						// extract the interval and the message
						// e.g. "30 hello everyone" -> interval: "30", message: "hello everyone"

						quint32 Interval;
						QString Message;
						QTextStream SS(&Payload);
						SS >> Interval;

						if( SS.status() != QTextStream::Ok || Interval == 0 )
							CONSOLE_Print( "[BNET: " + m_ServerAlias + "] bad input #1 to announce command" );
						else
						{
							if( SS.atEnd( ) )
								CONSOLE_Print( "[BNET: " + m_ServerAlias + "] missing input #2 to announce command" );
							else
							{
								Message = SS.readLine();
								int Start = Message.indexOf( QRegExp( "[^ ]" ));

								if( Start != -1 )
									Message = Message.mid( Start );

								QueueChatCommand( m_GHost->m_Language->AnnounceMessageEnabled( ), User, Whisper );
								m_GHost->m_CurrentGame->SetAnnounce( Interval, Message );
							}
						}
					}
				}

				//
				// !AUTOHOST
				//

				if( Command == "autohost" )
				{
					if( IsRootAdmin( User ) )
					{
						if( Payload.isEmpty( ) || Payload == "off" )
						{
							QueueChatCommand( m_GHost->m_Language->AutoHostDisabled( ), User, Whisper );
							m_GHost->m_AutoHostGameName.clear( );
							m_GHost->m_AutoHostOwner.clear( );
							m_GHost->m_AutoHostServer.clear( );
							m_GHost->m_AutoHostMaximumGames = 0;
							m_GHost->m_AutoHostAutoStartPlayers = 0;
							m_GHost->m_AutoHostMatchMaking = false;
							m_GHost->m_AutoHostMinimumScore = 0.0;
							m_GHost->m_AutoHostMaximumScore = 0.0;
						}
						else
						{
							// extract the maximum games, auto start players, and the game name
							// e.g. "5 10 BattleShips Pro" -> maximum games: "5", auto start players: "10", game name: "BattleShips Pro"

							quint32 MaximumGames;
							quint32 AutoStartPlayers;
							QString GameName;
							QTextStream SS(&Payload);
							SS >> MaximumGames;

							if( SS.status() != QTextStream::Ok || MaximumGames == 0 )
								CONSOLE_Print( "[BNET: " + m_ServerAlias + "] bad input #1 to autohost command" );
							else
							{
								SS >> AutoStartPlayers;

								if( SS.status() != QTextStream::Ok || AutoStartPlayers == 0 )
									CONSOLE_Print( "[BNET: " + m_ServerAlias + "] bad input #2 to autohost command" );
								else
								{
									if( SS.atEnd( ) )
										CONSOLE_Print( "[BNET: " + m_ServerAlias + "] missing input #3 to autohost command" );
									else
									{
										GameName = SS.readLine();
										int Start = GameName.indexOf( QRegExp( "[^ ]" ));

										if( Start != -1 )
											GameName = GameName.mid( Start );

										QueueChatCommand( m_GHost->m_Language->AutoHostEnabled( ), User, Whisper );
										delete m_GHost->m_AutoHostMap;
										m_GHost->m_AutoHostMap = new CMap( *m_GHost->m_Map );
										m_GHost->m_AutoHostGameName = GameName;
										m_GHost->m_AutoHostOwner = User;
										m_GHost->m_AutoHostServer = m_Server;
										m_GHost->m_AutoHostMaximumGames = MaximumGames;
										m_GHost->m_AutoHostAutoStartPlayers = AutoStartPlayers;
										m_GHost->m_AutoHostMatchMaking = false;
										m_GHost->m_AutoHostMinimumScore = 0.0;
										m_GHost->m_AutoHostMaximumScore = 0.0;
									}
								}
							}
						}
					}
					else
						QueueChatCommand( m_GHost->m_Language->YouDontHaveAccessToThatCommand( ), User, Whisper );
				}

				//
				// !AUTOHOSTMM
				//

				if( Command == "autohostmm" )
				{
					if( IsRootAdmin( User ) )
					{
						if( Payload.isEmpty( ) || Payload == "off" )
						{
							QueueChatCommand( m_GHost->m_Language->AutoHostDisabled( ), User, Whisper );
							m_GHost->m_AutoHostGameName.clear( );
							m_GHost->m_AutoHostOwner.clear( );
							m_GHost->m_AutoHostServer.clear( );
							m_GHost->m_AutoHostMaximumGames = 0;
							m_GHost->m_AutoHostAutoStartPlayers = 0;
							m_GHost->m_AutoHostMatchMaking = false;
							m_GHost->m_AutoHostMinimumScore = 0.0;
							m_GHost->m_AutoHostMaximumScore = 0.0;
						}
						else
						{
							// extract the maximum games, auto start players, minimum score, maximum score, and the game name
							// e.g. "5 10 800 1200 BattleShips Pro" -> maximum games: "5", auto start players: "10", minimum score: "800", maximum score: "1200", game name: "BattleShips Pro"

							quint32 MaximumGames;
							quint32 AutoStartPlayers;
							double MinimumScore;
							double MaximumScore;
							QString GameName;
							QTextStream SS(&Payload);
							SS >> MaximumGames;

							if( SS.status() != QTextStream::Ok || MaximumGames == 0 )
								CONSOLE_Print( "[BNET: " + m_ServerAlias + "] bad input #1 to autohostmm command" );
							else
							{
								SS >> AutoStartPlayers;

								if( SS.status() != QTextStream::Ok || AutoStartPlayers == 0 )
									CONSOLE_Print( "[BNET: " + m_ServerAlias + "] bad input #2 to autohostmm command" );
								else
								{
									SS >> MinimumScore;

									if( SS.status() != QTextStream::Ok )
										CONSOLE_Print( "[BNET: " + m_ServerAlias + "] bad input #3 to autohostmm command" );
									else
									{
										SS >> MaximumScore;

										if( SS.status() != QTextStream::Ok )
											CONSOLE_Print( "[BNET: " + m_ServerAlias + "] bad input #4 to autohostmm command" );
										else
										{
											if( SS.atEnd( ) )
												CONSOLE_Print( "[BNET: " + m_ServerAlias + "] missing input #5 to autohostmm command" );
											else
											{
												GameName = SS.readLine();
												int Start = GameName.indexOf( QRegExp( "[^ ]" ));

												if( Start != -1 )
													GameName = GameName.mid( Start );

												QueueChatCommand( m_GHost->m_Language->AutoHostEnabled( ), User, Whisper );
												delete m_GHost->m_AutoHostMap;
												m_GHost->m_AutoHostMap = new CMap( *m_GHost->m_Map );
												m_GHost->m_AutoHostGameName = GameName;
												m_GHost->m_AutoHostOwner = User;
												m_GHost->m_AutoHostServer = m_Server;
												m_GHost->m_AutoHostMaximumGames = MaximumGames;
												m_GHost->m_AutoHostAutoStartPlayers = AutoStartPlayers;
												m_GHost->m_AutoHostMatchMaking = true;
												m_GHost->m_AutoHostMinimumScore = MinimumScore;
												m_GHost->m_AutoHostMaximumScore = MaximumScore;
											}
										}
									}
								}
							}
						}
					}
					else
						QueueChatCommand( m_GHost->m_Language->YouDontHaveAccessToThatCommand( ), User, Whisper );
				}

				//
				// !AUTOSTART
				//

				if( Command == "autostart" && m_GHost->m_CurrentGame && !m_GHost->m_CurrentGame->GetCountDownStarted( ) )
				{
					if( Payload.isEmpty( ) || Payload == "off" )
					{
						QueueChatCommand( m_GHost->m_Language->AutoStartDisabled( ), User, Whisper );
						m_GHost->m_CurrentGame->SetAutoStartPlayers( 0 );
					}
					else
					{
						quint32 AutoStartPlayers = Payload.toUInt();

						if( AutoStartPlayers != 0 )
						{
							QueueChatCommand( m_GHost->m_Language->AutoStartEnabled( QString::number( AutoStartPlayers ) ), User, Whisper );
							m_GHost->m_CurrentGame->SetAutoStartPlayers( AutoStartPlayers );
						}
					}
				}

				//
				// !CHANNEL (change channel)
				//

				if( Command == "channel" && !Payload.isEmpty( ) )
					QueueChatCommand( "/join " + Payload );

				//
				// !CHECKADMIN
				//

				if( Command == "checkadmin" && !Payload.isEmpty( ) )
				{
					if( IsRootAdmin( User ) )
					{
						if( IsAdmin( Payload ) )
							QueueChatCommand( m_GHost->m_Language->UserIsAnAdmin( m_Server, Payload ), User, Whisper );
						else
							QueueChatCommand( m_GHost->m_Language->UserIsNotAnAdmin( m_Server, Payload ), User, Whisper );
					}
					else
						QueueChatCommand( m_GHost->m_Language->YouDontHaveAccessToThatCommand( ), User, Whisper );
				}

				//
				// !CHECKBAN
				//

				if( Command == "checkban" && !Payload.isEmpty( ) )
				{
					CDBBan *Ban = IsBannedName( Payload );

					if( Ban )
						QueueChatCommand( m_GHost->m_Language->UserWasBannedOnByBecause( m_Server, Payload, Ban->GetDate( ), Ban->GetAdmin( ), Ban->GetReason( ) ), User, Whisper );
					else
						QueueChatCommand( m_GHost->m_Language->UserIsNotBanned( m_Server, Payload ), User, Whisper );
				}

				//
				// !CLOSE (close slot)
				//

				if( Command == "close" && !Payload.isEmpty( ) && m_GHost->m_CurrentGame )
				{
					if( !m_GHost->m_CurrentGame->GetLocked( ) )
					{
						// close as many slots as specified, e.g. "5 10" closes slots 5 and 10

						QTextStream SS(&Payload);

						while( !SS.atEnd( ) )
						{
							quint32 SID;
							SS >> SID;

							if( SS.status() != QTextStream::Ok )
							{
								CONSOLE_Print( "[BNET: " + m_ServerAlias + "] bad input to close command" );
								break;
							}
							else
								m_GHost->m_CurrentGame->CloseSlot( (unsigned char)( SID - 1 ), true );
						}
					}
					else
						QueueChatCommand( m_GHost->m_Language->TheGameIsLockedBNET( ), User, Whisper );
				}

				//
				// !CLOSEALL
				//

				if( Command == "closeall" && m_GHost->m_CurrentGame )
				{
					if( !m_GHost->m_CurrentGame->GetLocked( ) )
						m_GHost->m_CurrentGame->CloseAllSlots( );
					else
						QueueChatCommand( m_GHost->m_Language->TheGameIsLockedBNET( ), User, Whisper );
				}

				//
				// !COUNTADMINS
				//

				if( Command == "countadmins" )
				{
					if( IsRootAdmin( User ) )
						m_PairedAdminCounts.push_back( PairedAdminCount( Whisper ? User : QString( ), m_GHost->m_DB->ThreadedAdminCount( m_Server ) ) );
					else
						QueueChatCommand( m_GHost->m_Language->YouDontHaveAccessToThatCommand( ), User, Whisper );
				}

				//
				// !COUNTBANS
				//

				if( Command == "countbans" )
					m_PairedBanCounts.push_back( PairedBanCount( Whisper ? User : QString( ), m_GHost->m_DB->ThreadedBanCount( m_Server ) ) );

				//
				// !DBSTATUS
				//

				if( Command == "dbstatus" )
					QueueChatCommand( m_GHost->m_DB->GetStatus( ), User, Whisper );

				//
				// !DELADMIN
				//

				if( Command == "deladmin" && !Payload.isEmpty( ) )
				{
					if( IsRootAdmin( User ) )
					{
						if( !IsAdmin( Payload ) )
							QueueChatCommand( m_GHost->m_Language->UserIsNotAnAdmin( m_Server, Payload ), User, Whisper );
						else
							m_PairedAdminRemoves.push_back( PairedAdminRemove( Whisper ? User : QString( ), m_GHost->m_DB->ThreadedAdminRemove( m_Server, Payload ) ) );
					}
					else
						QueueChatCommand( m_GHost->m_Language->YouDontHaveAccessToThatCommand( ), User, Whisper );
				}

				//
				// !DELBAN
				// !UNBAN
				//

				if( ( Command == "delban" || Command == "unban" ) && !Payload.isEmpty( ) )
					m_PairedBanRemoves.push_back( PairedBanRemove( Whisper ? User : QString( ), m_GHost->m_DB->ThreadedBanRemove( Payload ) ) );

				//
				// !DISABLE
				//

				if( Command == "disable" )
				{
					if( IsRootAdmin( User ) )
					{
						QueueChatCommand( m_GHost->m_Language->BotDisabled( ), User, Whisper );
						m_GHost->m_Enabled = false;
					}
					else
						QueueChatCommand( m_GHost->m_Language->YouDontHaveAccessToThatCommand( ), User, Whisper );
				}

				//
				// !DOWNLOADS
				//

				if( Command == "downloads" && !Payload.isEmpty( ) )
				{
					quint32 Downloads = Payload.toUInt();

					if( Downloads == 0 )
					{
						QueueChatCommand( m_GHost->m_Language->MapDownloadsDisabled( ), User, Whisper );
						m_GHost->m_AllowDownloads = 0;
					}
					else if( Downloads == 1 )
					{
						QueueChatCommand( m_GHost->m_Language->MapDownloadsEnabled( ), User, Whisper );
						m_GHost->m_AllowDownloads = 1;
					}
					else if( Downloads == 2 )
					{
						QueueChatCommand( m_GHost->m_Language->MapDownloadsConditional( ), User, Whisper );
						m_GHost->m_AllowDownloads = 2;
					}
				}

				//
				// !ENABLE
				//

				if( Command == "enable" )
				{
					if( IsRootAdmin( User ) )
					{
						QueueChatCommand( m_GHost->m_Language->BotEnabled( ), User, Whisper );
						m_GHost->m_Enabled = true;
					}
					else
						QueueChatCommand( m_GHost->m_Language->YouDontHaveAccessToThatCommand( ), User, Whisper );
				}

				//
				// !END
				//

				if( Command == "end" && !Payload.isEmpty( ) )
				{
					// todotodo: what if a game ends just as you're typing this command and the numbering changes?

					int GameNumber = Payload.toUInt() - 1;

					if( GameNumber < m_GHost->m_Games.size( ) )
					{
						// if the game owner is still in the game only allow the root admin to end the game

						if( m_GHost->m_Games[GameNumber]->GetPlayerFromName( m_GHost->m_Games[GameNumber]->GetOwnerName( ), false ) && !IsRootAdmin( User ) )
							QueueChatCommand( m_GHost->m_Language->CantEndGameOwnerIsStillPlaying( m_GHost->m_Games[GameNumber]->GetOwnerName( ) ), User, Whisper );
						else
						{
							QueueChatCommand( m_GHost->m_Language->EndingGame( m_GHost->m_Games[GameNumber]->GetDescription( ) ), User, Whisper );
							CONSOLE_Print( "[GAME: " + m_GHost->m_Games[GameNumber]->GetGameName( ) + "] is over (admin ended game)" );
							m_GHost->m_Games[GameNumber]->StopPlayers( "was disconnected (admin ended game)" );
						}
					}
					else
						QueueChatCommand( m_GHost->m_Language->GameNumberDoesntExist( Payload ), User, Whisper );
				}

				//
				// !ENFORCESG
				//

				if( Command == "enforcesg" && !Payload.isEmpty( ) )
				{
					// only load files in the current directory just to be safe

					if( Payload.indexOf( "/" ) != -1 || Payload.indexOf( "\\" ) != -1 )
						QueueChatCommand( m_GHost->m_Language->UnableToLoadReplaysOutside( ), User, Whisper );
					else
					{
						QString File = m_GHost->m_ReplayPath + Payload + ".w3g";

						if( QFile::exists( File ) )
						{
							QueueChatCommand( m_GHost->m_Language->LoadingReplay( File ), User, Whisper );
							CReplay *Replay = new CReplay( );
							Replay->Load( File, false );
							Replay->ParseReplay( false );
							m_GHost->m_EnforcePlayers = Replay->GetPlayers( );
							delete Replay;
						}
						else
							QueueChatCommand( m_GHost->m_Language->UnableToLoadReplayDoesntExist( File ), User, Whisper );
					}
				}

				//
				// !EXIT
				// !QUIT
				//

				if( Command == "exit" || Command == "quit" )
				{
					if( IsRootAdmin( User ) )
					{
						if( Payload == "nice" )
							m_GHost->m_ExitingNice = true;
						else if( Payload == "force" )
							m_GHost->deleteLater();
						else
						{
							if( m_GHost->m_CurrentGame || !m_GHost->m_Games.isEmpty( ) )
								QueueChatCommand( m_GHost->m_Language->AtLeastOneGameActiveUseForceToShutdown( ), User, Whisper );
							else
								m_GHost->deleteLater();
						}
					}
					else
						QueueChatCommand( m_GHost->m_Language->YouDontHaveAccessToThatCommand( ), User, Whisper );
				}

				//
				// !GETCLAN
				//

				if( Command == "getclan" )
				{
					SendGetClanList( );
					QueueChatCommand( m_GHost->m_Language->UpdatingClanList( ), User, Whisper );
				}

				//
				// !GETFRIENDS
				//

				if( Command == "getfriends" )
				{
					SendGetFriendsList( );
					QueueChatCommand( m_GHost->m_Language->UpdatingFriendsList( ), User, Whisper );
				}

				//
				// !GETGAME
				//

				if( Command == "getgame" && !Payload.isEmpty( ) )
				{
					int GameNumber = Payload.toUInt() - 1;

					if( GameNumber < m_GHost->m_Games.size( ) )
						QueueChatCommand( m_GHost->m_Language->GameNumberIs( Payload, m_GHost->m_Games[GameNumber]->GetDescription( ) ), User, Whisper );
					else
						QueueChatCommand( m_GHost->m_Language->GameNumberDoesntExist( Payload ), User, Whisper );
				}

				//
				// !GETGAMES
				//

				if( Command == "getgames" )
				{
					if( m_GHost->m_CurrentGame )
						QueueChatCommand( m_GHost->m_Language->GameIsInTheLobby( m_GHost->m_CurrentGame->GetDescription( ), QString::number( m_GHost->m_Games.size( ) ), QString::number( m_GHost->m_MaxGames ) ), User, Whisper );
					else
						QueueChatCommand( m_GHost->m_Language->ThereIsNoGameInTheLobby( QString::number( m_GHost->m_Games.size( ) ), QString::number( m_GHost->m_MaxGames ) ), User, Whisper );
				}

				//
				// !HOLD (hold a slot for someone)
				//

				if( Command == "hold" && !Payload.isEmpty( ) && m_GHost->m_CurrentGame )
				{
					// hold as many players as specified, e.g. "Varlock Kilranin" holds players "Varlock" and "Kilranin"

					QTextStream SS(&Payload);

					while( !SS.atEnd( ) )
					{
						QString HoldName;
						SS >> HoldName;

						if( SS.status() != QTextStream::Ok )
						{
							CONSOLE_Print( "[BNET: " + m_ServerAlias + "] bad input to hold command" );
							break;
						}
						else
						{
							QueueChatCommand( m_GHost->m_Language->AddedPlayerToTheHoldList( HoldName ), User, Whisper );
							m_GHost->m_CurrentGame->AddToReserved( HoldName );
						}
					}
				}

				//
				// !HOSTSG
				//

				if( Command == "hostsg" && !Payload.isEmpty( ) )
					m_GHost->CreateGame( m_GHost->m_Map, GAME_PRIVATE, true, Payload, User, User, m_Server, Whisper );

				//
				// !LOAD (load config file)
				//

				if( Command == "load" )
				{
					if( Payload.isEmpty( ) )
						QueueChatCommand( m_GHost->m_Language->CurrentlyLoadedMapCFGIs( m_GHost->m_Map->GetCFGFile( ) ), User, Whisper );
					else
					{
						try
						{
							QDir MapCFGPath( m_GHost->m_MapCFGPath );
							QString Pattern = Payload.toLower();

							if( !MapCFGPath.exists() )
							{
								CONSOLE_Print( "[ADMINGAME] error listing map configs - map config path doesn't exist" );
								QueueChatCommand( m_GHost->m_Language->ErrorListingMapConfigs( ), User, Whisper );
							}
							else
							{
								QStringList files = MapCFGPath.entryList(QStringList("*" + Pattern + "*"), QDir::Files, QDir::Name);
								quint32 Matches = files.size();

								if( Matches == 0 )
									QueueChatCommand( m_GHost->m_Language->NoMapConfigsFound( ), User, Whisper );
								else if (files.contains(Pattern))
								{
									QueueChatCommand( m_GHost->m_Language->LoadingConfigFile( m_GHost->m_MapCFGPath + Pattern ), User, Whisper );
									CConfig MapCFG;
									MapCFG.Read( Pattern );
									m_GHost->m_Map->Load( &MapCFG, m_GHost->m_MapCFGPath + Pattern );
								}
								else if (Matches == 1)
								{
									QString File = files.at(0);
									QueueChatCommand( m_GHost->m_Language->LoadingConfigFile( m_GHost->m_MapCFGPath + File ), User, Whisper );
									CConfig MapCFG;
									MapCFG.Read( m_GHost->m_MapCFGPath + File );
									m_GHost->m_Map->Load( &MapCFG, m_GHost->m_MapCFGPath + File );
								}
								else
									QueueChatCommand( m_GHost->m_Language->FoundMapConfigs( files.join(", ") ), User, Whisper );
							}
						}
						catch( const exception &ex )
						{
							CONSOLE_Print( QString( "[ADMINGAME] error listing map configs - caught exception [" ) + ex.what( ) + "]" );
							QueueChatCommand( m_GHost->m_Language->ErrorListingMapConfigs( ), User, Whisper );
						}
					}
				}

				//
				// !LOADSG
				//

				if( Command == "loadsg" && !Payload.isEmpty( ) )
				{
					// only load files in the current directory just to be safe

					if( Payload.indexOf( "/" ) != -1 || Payload.indexOf( "\\" ) != -1 )
						QueueChatCommand( m_GHost->m_Language->UnableToLoadSaveGamesOutside( ), User, Whisper );
					else
					{
						QString File = m_GHost->m_SaveGamePath + Payload + ".w3z";
						QString FileNoPath = Payload + ".w3z";

						if( QFile::exists( File ) )
						{
							if( m_GHost->m_CurrentGame )
								QueueChatCommand( m_GHost->m_Language->UnableToLoadSaveGameGameInLobby( ), User, Whisper );
							else
							{
								QueueChatCommand( m_GHost->m_Language->LoadingSaveGame( File ), User, Whisper );
								m_GHost->m_SaveGame->Load( File, false );
								m_GHost->m_SaveGame->ParseSaveGame( );
								m_GHost->m_SaveGame->SetFileName( File );
								m_GHost->m_SaveGame->SetFileNameNoPath( FileNoPath );
							}
						}
						else
							QueueChatCommand( m_GHost->m_Language->UnableToLoadSaveGameDoesntExist( File ), User, Whisper );
					}
				}

				//
				// !MAP (load map file)
				//

				if( Command == "map" )
				{
					if( Payload.isEmpty( ) )
						QueueChatCommand( m_GHost->m_Language->CurrentlyLoadedMapCFGIs( m_GHost->m_Map->GetCFGFile( ) ), User, Whisper );
					else
					{
						QString FoundMaps;

						try
						{
							QDir MapPath( m_GHost->m_MapPath );
							QString Pattern = Payload.toLower();

							if( !MapPath.exists() )
							{
								CONSOLE_Print( "[ADMINGAME] error listing maps - map path doesn't exist" );
								QueueChatCommand( m_GHost->m_Language->ErrorListingMaps( ), User, Whisper );
							}
							else
							{
								QStringList files = MapPath.entryList(QStringList("*"+Pattern+"*"), QDir::Files, QDir::Name);
								quint32 Matches = files.size();

								if( Matches == 0 )
									QueueChatCommand( m_GHost->m_Language->NoMapsFound( ), User, Whisper );
								else if (files.contains(Pattern))
								{
									QueueChatCommand( m_GHost->m_Language->LoadingConfigFile( Pattern ), User, Whisper );

									// hackhack: create a config file in memory with the required information to load the map

									CConfig MapCFG;
									MapCFG.Set( "map_path", "Maps\\Download\\" + Pattern );
									MapCFG.Set( "map_localpath", Pattern );
									m_GHost->m_Map->Load( &MapCFG, Pattern );
								}
								else if( Matches == 1 )
								{
									QString File = files.at(0);
									QueueChatCommand( m_GHost->m_Language->LoadingConfigFile( File ), User, Whisper );

									// hackhack: create a config file in memory with the required information to load the map

									CConfig MapCFG;
									MapCFG.Set( "map_path", "Maps\\Download\\" + File );
									MapCFG.Set( "map_localpath", File );
									m_GHost->m_Map->Load( &MapCFG, File );
								}
								else
									QueueChatCommand( m_GHost->m_Language->FoundMaps( files.join(", ") ), User, Whisper );
							}
						}
						catch( const exception &ex )
						{
							CONSOLE_Print( "[BNET: " + m_ServerAlias + "] error listing maps - caught exception [" + ex.what( ) + "]" );
							QueueChatCommand( m_GHost->m_Language->ErrorListingMaps( ), User, Whisper );
						}
					}
				}

				//
				// !OPEN (open slot)
				//

				if( Command == "open" && !Payload.isEmpty( ) && m_GHost->m_CurrentGame )
				{
					if( !m_GHost->m_CurrentGame->GetLocked( ) )
					{
						// open as many slots as specified, e.g. "5 10" opens slots 5 and 10

						QTextStream SS(&Payload);

						while( !SS.atEnd( ) )
						{
							quint32 SID;
							SS >> SID;

							if( SS.status() != QTextStream::Ok )
							{
								CONSOLE_Print( "[BNET: " + m_ServerAlias + "] bad input to open command" );
								break;
							}
							else
								m_GHost->m_CurrentGame->OpenSlot( (unsigned char)( SID - 1 ), true );
						}
					}
					else
						QueueChatCommand( m_GHost->m_Language->TheGameIsLockedBNET( ), User, Whisper );
				}

				//
				// !OPENALL
				//

				if( Command == "openall" && m_GHost->m_CurrentGame )
				{
					if( !m_GHost->m_CurrentGame->GetLocked( ) )
						m_GHost->m_CurrentGame->OpenAllSlots( );
					else
						QueueChatCommand( m_GHost->m_Language->TheGameIsLockedBNET( ), User, Whisper );
				}

				//
				// !PRIV (host private game)
				//

				if( Command == "priv" && !Payload.isEmpty( ) )
					m_GHost->CreateGame( m_GHost->m_Map, GAME_PRIVATE, false, Payload, User, User, m_Server, Whisper );

				//
				// !PRIVBY (host private game by other player)
				//

				if( Command == "privby" && !Payload.isEmpty( ) )
				{
					// extract the owner and the game name
					// e.g. "Varlock dota 6.54b arem ~~~" -> owner: "Varlock", game name: "dota 6.54b arem ~~~"

					QString Owner;
					QString GameName;
					int GameNameStart = Payload.indexOf( " " );

					if( GameNameStart != -1 )
					{
						Owner = Payload.mid( 0, GameNameStart );
						GameName = Payload.mid( GameNameStart + 1 );
						m_GHost->CreateGame( m_GHost->m_Map, GAME_PRIVATE, false, GameName, Owner, User, m_Server, Whisper );
					}
				}

				//
				// !PUB (host public game)
				//

				if( Command == "pub" && !Payload.isEmpty( ) )
					m_GHost->CreateGame( m_GHost->m_Map, GAME_PUBLIC, false, Payload, User, User, m_Server, Whisper );

				//
				// !PUBBY (host public game by other player)
				//

				if( Command == "pubby" && !Payload.isEmpty( ) )
				{
					// extract the owner and the game name
					// e.g. "Varlock dota 6.54b arem ~~~" -> owner: "Varlock", game name: "dota 6.54b arem ~~~"

					QString Owner;
					QString GameName;
					int GameNameStart = Payload.indexOf( " " );

					if( GameNameStart != -1 )
					{
						Owner = Payload.mid( 0, GameNameStart );
						GameName = Payload.mid( GameNameStart + 1 );
						m_GHost->CreateGame( m_GHost->m_Map, GAME_PUBLIC, false, GameName, Owner, User, m_Server, Whisper );
					}
				}

				//
				// !RELOAD
				//

				if( Command == "reload" )
				{
					if( IsRootAdmin( User ) )
					{
						QueueChatCommand( m_GHost->m_Language->ReloadingConfigurationFiles( ), User, Whisper );
						m_GHost->ReloadConfigs( );
					}
					else
						QueueChatCommand( m_GHost->m_Language->YouDontHaveAccessToThatCommand( ), User, Whisper );
				}

				//
				// !SAY
				//

				if( Command == "say" && !Payload.isEmpty( ) )
					QueueChatCommand( Payload );

				//
				// !SAYGAME
				//

				if( Command == "saygame" && !Payload.isEmpty( ) )
				{
					if( IsRootAdmin( User ) )
					{
						// extract the game number and the message
						// e.g. "3 hello everyone" -> game number: "3", message: "hello everyone"

						int GameNumber;
						QString Message;
						QTextStream SS(&Payload);

						SS >> GameNumber;

						if( SS.status() != QTextStream::Ok )
							CONSOLE_Print( "[BNET: " + m_ServerAlias + "] bad input #1 to saygame command" );
						else
						{
							if( SS.atEnd( ) )
								CONSOLE_Print( "[BNET: " + m_ServerAlias + "] missing input #2 to saygame command" );
							else
							{
								Message = SS.readLine();
								int Start = Message.indexOf( QRegExp( "[^ ]" ));

								if( Start != -1 )
									Message = Message.mid( Start );

								if( GameNumber - 1 < m_GHost->m_Games.size( ) )
									m_GHost->m_Games[GameNumber - 1]->SendAllChat( "ADMIN: " + Message );
								else
									QueueChatCommand( m_GHost->m_Language->GameNumberDoesntExist( QString::number( GameNumber ) ), User, Whisper );
							}
						}
					}
					else
						QueueChatCommand( m_GHost->m_Language->YouDontHaveAccessToThatCommand( ), User, Whisper );
				}

				//
				// !SAYGAMES
				//

				if( Command == "saygames" && !Payload.isEmpty( ) )
				{
					if( IsRootAdmin( User ) )
					{
						if( m_GHost->m_CurrentGame )
							m_GHost->m_CurrentGame->SendAllChat( Payload );

						for( QList<CBaseGame *> :: const_iterator i = m_GHost->m_Games.begin( ); i != m_GHost->m_Games.end( ); i++ )
							(*i)->SendAllChat( "ADMIN: " + Payload );
					}
					else
						QueueChatCommand( m_GHost->m_Language->YouDontHaveAccessToThatCommand( ), User, Whisper );
				}

				//
				// !SP
				//

				if( Command == "sp" && m_GHost->m_CurrentGame && !m_GHost->m_CurrentGame->GetCountDownStarted( ) )
				{
					if( !m_GHost->m_CurrentGame->GetLocked( ) )
					{
						m_GHost->m_CurrentGame->SendAllChat( m_GHost->m_Language->ShufflingPlayers( ) );
						m_GHost->m_CurrentGame->ShuffleSlots( );
					}
					else
						QueueChatCommand( m_GHost->m_Language->TheGameIsLockedBNET( ), User, Whisper );
				}

				//
				// !START
				//

				if( Command == "start" && m_GHost->m_CurrentGame && !m_GHost->m_CurrentGame->GetCountDownStarted( ) && m_GHost->m_CurrentGame->GetNumHumanPlayers( ) > 0 )
				{
					if( !m_GHost->m_CurrentGame->GetLocked( ) )
					{
						// if the player sent "!start force" skip the checks and start the countdown
						// otherwise check that the game is ready to start

						if( Payload == "force" )
							m_GHost->m_CurrentGame->StartCountDown( true );
						else
							m_GHost->m_CurrentGame->StartCountDown( false );
					}
					else
						QueueChatCommand( m_GHost->m_Language->TheGameIsLockedBNET( ), User, Whisper );
				}

				//
				// !SWAP (swap slots)
				//

				if( Command == "swap" && !Payload.isEmpty( ) && m_GHost->m_CurrentGame )
				{
					if( !m_GHost->m_CurrentGame->GetLocked( ) )
					{
						quint32 SID1;
						quint32 SID2;
						QTextStream SS(&Payload);

						SS >> SID1;

						if( SS.status() != QTextStream::Ok )
							CONSOLE_Print( "[BNET: " + m_ServerAlias + "] bad input #1 to swap command" );
						else
						{
							if( SS.atEnd( ) )
								CONSOLE_Print( "[BNET: " + m_ServerAlias + "] missing input #2 to swap command" );
							else
							{
								SS >> SID2;

								if( SS.status() != QTextStream::Ok )
									CONSOLE_Print( "[BNET: " + m_ServerAlias + "] bad input #2 to swap command" );
								else
									m_GHost->m_CurrentGame->SwapSlots( (unsigned char)( SID1 - 1 ), (unsigned char)( SID2 - 1 ) );
							}
						}
					}
					else
						QueueChatCommand( m_GHost->m_Language->TheGameIsLockedBNET( ), User, Whisper );
				}

				//
				// !UNHOST
				//

				if( Command == "unhost" )
				{
					if( m_GHost->m_CurrentGame )
					{
						if( m_GHost->m_CurrentGame->GetCountDownStarted( ) )
							QueueChatCommand( m_GHost->m_Language->UnableToUnhostGameCountdownStarted( m_GHost->m_CurrentGame->GetDescription( ) ), User, Whisper );

						// if the game owner is still in the game only allow the root admin to unhost the game

						else if( m_GHost->m_CurrentGame->GetPlayerFromName( m_GHost->m_CurrentGame->GetOwnerName( ), false ) && !IsRootAdmin( User ) )
							QueueChatCommand( m_GHost->m_Language->CantUnhostGameOwnerIsPresent( m_GHost->m_CurrentGame->GetOwnerName( ) ), User, Whisper );
						else
						{
							QueueChatCommand( m_GHost->m_Language->UnhostingGame( m_GHost->m_CurrentGame->GetDescription( ) ), User, Whisper );
							m_GHost->m_CurrentGame->deleteLater();
						}
					}
					else
						QueueChatCommand( m_GHost->m_Language->UnableToUnhostGameNoGameInLobby( ), User, Whisper );
				}

				//
				// !WARDENSTATUS
				//

				if( Command == "wardenstatus" )
				{
					if( m_BNLSClient )
						QueueChatCommand( "WARDEN STATUS --- " + QString::number( m_BNLSClient->GetTotalWardenIn( ) ) + " requests received, " + QString::number( m_BNLSClient->GetTotalWardenOut( ) ) + " responses sent.", User, Whisper );
					else
						QueueChatCommand( "WARDEN STATUS --- Not connected to BNLS server.", User, Whisper );
				}
			}
			else
				CONSOLE_Print( "[BNET: " + m_ServerAlias + "] non-admin [" + User + "] sent command [" + Message + "]" );

			/*********************
			* NON ADMIN COMMANDS *
			*********************/

			// don't respond to non admins if there are more than 3 messages already in the queue
			// this prevents malicious users from filling up the bot's chat queue and crippling the bot
			// in some cases the queue may be full of legitimate messages but we don't really care if the bot ignores one of these commands once in awhile
			// e.g. when several users join a game at the same time and cause multiple /whois messages to be queued at once

			if( IsAdmin( User ) || IsRootAdmin( User ) || ( m_PublicCommands && m_OutPackets.size( ) <= 3 ) )
			{
				//
				// !STATS
				//

				if( Command == "stats" )
				{
					QString StatsUser = User;

					if( !Payload.isEmpty( ) )
						StatsUser = Payload;

					// check for potential abuse

					if( !StatsUser.isEmpty( ) && StatsUser.size( ) < 16 && StatsUser[0] != '/' )
						m_PairedGPSChecks.push_back( PairedGPSCheck( Whisper ? User : QString( ), m_GHost->m_DB->ThreadedGamePlayerSummaryCheck( StatsUser ) ) );
				}

				//
				// !STATSDOTA
				//

				if( Command == "statsdota" )
				{
					QString StatsUser = User;

					if( !Payload.isEmpty( ) )
						StatsUser = Payload;

					// check for potential abuse

					if( !StatsUser.isEmpty( ) && StatsUser.size( ) < 16 && StatsUser[0] != '/' )
						m_PairedDPSChecks.push_back( PairedDPSCheck( Whisper ? User : QString( ), m_GHost->m_DB->ThreadedDotAPlayerSummaryCheck( StatsUser ) ) );
				}

				//
				// !VERSION
				//

				if( Command == "version" )
				{
					if( IsAdmin( User ) || IsRootAdmin( User ) )
						QueueChatCommand( m_GHost->m_Language->VersionAdmin( m_GHost->m_Version ), User, Whisper );
					else
						QueueChatCommand( m_GHost->m_Language->VersionNotAdmin( m_GHost->m_Version ), User, Whisper );
				}
			}
		}
	}
	else if( Event == CBNETProtocol :: EID_CHANNEL )
	{
		// keep track of current channel so we can rejoin it after hosting a game

		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] joined channel [" + Message + "]" );
		m_CurrentChannel = Message;
	}
	else if( Event == CBNETProtocol :: EID_INFO )
	{
		CONSOLE_Print( "[INFO: " + m_ServerAlias + "] " + Message );

		// extract the first word which we hope is the username
		// this is not necessarily true though since info messages also include channel MOTD's and such

		QString UserName;
		int Split = Message.indexOf( " " );

		if( Split != -1 )
			UserName = Message.mid( 0, Split );
		else
			UserName = Message.mid( 0 );

		// handle spoof checking for current game
		// this case covers whois results which are used when hosting a public game (we send out a "/whois [player]" for each player)
		// at all times you can still /w the bot with "spoofcheck" to manually spoof check

		if( m_GHost->m_CurrentGame && m_GHost->m_CurrentGame->GetPlayerFromName( UserName, true ) )
		{
			if( Message.indexOf( "is away" ) != -1 )
				m_GHost->m_CurrentGame->SendAllChat( m_GHost->m_Language->SpoofPossibleIsAway( UserName ) );
			else if( Message.indexOf( "is unavailable" ) != -1 )
				m_GHost->m_CurrentGame->SendAllChat( m_GHost->m_Language->SpoofPossibleIsUnavailable( UserName ) );
			else if( Message.indexOf( "is refusing messages" ) != -1 )
				m_GHost->m_CurrentGame->SendAllChat( m_GHost->m_Language->SpoofPossibleIsRefusingMessages( UserName ) );
			else if( Message.indexOf( "is using Warcraft III The Frozen Throne in the channel" ) != -1 )
				m_GHost->m_CurrentGame->SendAllChat( m_GHost->m_Language->SpoofDetectedIsNotInGame( UserName ) );
			else if( Message.indexOf( "is using Warcraft III The Frozen Throne in channel" ) != -1 )
				m_GHost->m_CurrentGame->SendAllChat( m_GHost->m_Language->SpoofDetectedIsNotInGame( UserName ) );
			else if( Message.indexOf( "is using Warcraft III The Frozen Throne in a private channel" ) != -1 )
				m_GHost->m_CurrentGame->SendAllChat( m_GHost->m_Language->SpoofDetectedIsInPrivateChannel( UserName ) );

			if( Message.indexOf( "is using Warcraft III The Frozen Throne in game" ) != -1 || Message.indexOf( "is using Warcraft III Frozen Throne and is currently in  game" ) != -1 )
			{
				// check both the current game name and the last game name against the /whois response
				// this is because when the game is rehosted, players who joined recently will be in the previous game according to battle.net
				// note: if the game is rehosted more than once it is possible (but unlikely) for a false positive because only two game names are checked

				if( Message.indexOf( m_GHost->m_CurrentGame->GetGameName( ) ) != -1 || Message.indexOf( m_GHost->m_CurrentGame->GetLastGameName( ) ) != -1 )
					m_GHost->m_CurrentGame->AddToSpoofed( m_Server, UserName, false );
				else
					m_GHost->m_CurrentGame->SendAllChat( m_GHost->m_Language->SpoofDetectedIsInAnotherGame( UserName ) );
			}
		}
	}
	else if( Event == CBNETProtocol :: EID_ERROR )
		CONSOLE_Print( "[ERROR: " + m_ServerAlias + "] " + Message );
	else if( Event == CBNETProtocol :: EID_EMOTE )
	{
		CONSOLE_Print( "[EMOTE: " + m_ServerAlias + "] [" + User + "] " + Message );
		m_GHost->EventBNETEmote( this, User, Message );
	}
}

void CBNET :: SendJoinChannel( QString channel )
{
	if( m_LoggedIn && m_InChat )
		m_Socket->write( m_Protocol->SEND_SID_JOINCHANNEL( channel ) );
}

void CBNET :: SendGetFriendsList( )
{
	if( m_LoggedIn )
		m_Socket->write( m_Protocol->SEND_SID_FRIENDSLIST( ) );
}

void CBNET :: SendGetClanList( )
{
	if( m_LoggedIn )
		m_Socket->write( m_Protocol->SEND_SID_CLANMEMBERLIST( ) );
}

void CBNET :: QueueEnterChat( )
{
	if( m_LoggedIn )
		EnqueuePacket( m_Protocol->SEND_SID_ENTERCHAT( ) );
}

void CBNET :: QueueChatCommand( QString chatCommand )
{
	if( chatCommand.isEmpty( ) )
		return;

	if( m_LoggedIn )
	{
		if( m_PasswordHashType == "pvpgn" && chatCommand.size( ) > m_MaxMessageLength )
			chatCommand = chatCommand.mid( 0, m_MaxMessageLength );

		if( chatCommand.size( ) > 255 )
			chatCommand = chatCommand.mid( 0, 255 );

		if( m_OutPackets.size( ) > 10 )
			CONSOLE_Print( "[BNET: " + m_ServerAlias + "] attempted to queue chat command [" + chatCommand + "] but there are too many (" + QString::number( m_OutPackets.size( ) ) + ") packets queued, discarding" );
		else
		{
			CONSOLE_Print( "[QUEUED: " + m_ServerAlias + "] " + chatCommand );
			EnqueuePacket( m_Protocol->SEND_SID_CHATCOMMAND( chatCommand ) );
		}
	}
}

void CBNET :: QueueChatCommand( QString chatCommand, QString user, bool whisper )
{
	if( chatCommand.isEmpty( ) )
		return;

	// if whisper is true send the chat command as a whisper to user, otherwise just queue the chat command

	if( whisper )
		QueueChatCommand( "/w " + user + " " + chatCommand );
	else
		QueueChatCommand( chatCommand );
}

void CBNET :: QueueGameCreate( unsigned char state, QString gameName, QString hostName, CMap *map, CSaveGame *savegame, quint32 hostCounter )
{
	if( m_LoggedIn && map )
	{
		if( !m_CurrentChannel.isEmpty( ) )
			m_FirstChannel = m_CurrentChannel;

		m_InChat = false;

		// a game creation message is just a game refresh message with upTime = 0

		QueueGameRefresh( state, gameName, hostName, map, savegame, 0, hostCounter );
	}
}

void CBNET :: QueueGameRefresh( unsigned char state, QString gameName, QString hostName, CMap *map, CSaveGame *saveGame, quint32 upTime, quint32 hostCounter )
{
	if( hostName.isEmpty( ) )
	{
		QByteArray UniqueName = m_Protocol->GetUniqueName( );
		hostName = UniqueName;
	}

	if( m_LoggedIn && map )
	{
		// construct a fixed host counter which will be used to identify players from this realm
		// the fixed host counter's 4 most significant bits will contain a 4 bit ID (0-15)
		// the rest of the fixed host counter will contain the 28 least significant bits of the actual host counter
		// since we're destroying 4 bits of information here the actual host counter should not be greater than 2^28 which is a reasonable assumption
		// when a player joins a game we can obtain the ID from the received host counter
		// note: LAN broadcasts use an ID of 0, battle.net refreshes use an ID of 1-10, the rest are unused

		quint32 FixedHostCounter = ( hostCounter & 0x0FFFFFFF ) | ( m_HostCounterID << 28 );

		if( saveGame )
		{
			quint32 MapGameType = MAPGAMETYPE_SAVEDGAME;

			// the state should always be private when creating a saved game

			if( state == GAME_PRIVATE )
				MapGameType |= MAPGAMETYPE_PRIVATEGAME;

			// use an invalid map width/height to indicate reconnectable games

			QByteArray MapWidth;
			MapWidth.push_back( 192 );
			MapWidth.push_back( 7 );
			QByteArray MapHeight;
			MapHeight.push_back( 192 );
			MapHeight.push_back( 7 );

			if( m_GHost->m_Reconnect )
				EnqueuePacket( m_Protocol->SEND_SID_STARTADVEX3(
						state,
						Util::fromUInt32( MapGameType),
						map->GetMapGameFlags( ),
						MapWidth,
						MapHeight,
						gameName,
						hostName,
						upTime,
						"Save\\Multiplayer\\" + saveGame->GetFileNameNoPath( ),
						saveGame->GetMagicNumber( ),
						map->GetMapSHA1( ),
						FixedHostCounter ) );
			else
				EnqueuePacket( m_Protocol->SEND_SID_STARTADVEX3(
						state,
						Util::fromUInt32( MapGameType),
						map->GetMapGameFlags( ),
						Util::fromUInt16( (quint16)0),
						Util::fromUInt16( (quint16)0),
						gameName,
						hostName,
						upTime,
						"Save\\Multiplayer\\" + saveGame->GetFileNameNoPath( ),
						saveGame->GetMagicNumber( ),
						map->GetMapSHA1( ),
						FixedHostCounter ) );
		}
		else
		{
			quint32 MapGameType = map->GetMapGameType( );
			MapGameType |= MAPGAMETYPE_UNKNOWN0;

			if( state == GAME_PRIVATE )
				MapGameType |= MAPGAMETYPE_PRIVATEGAME;

			// use an invalid map width/height to indicate reconnectable games

			QByteArray MapWidth;
			MapWidth.push_back( 192 );
			MapWidth.push_back( 7 );
			QByteArray MapHeight;
			MapHeight.push_back( 192 );
			MapHeight.push_back( 7 );

			if( m_GHost->m_Reconnect )
				EnqueuePacket( m_Protocol->SEND_SID_STARTADVEX3( state, Util::fromUInt32( MapGameType), map->GetMapGameFlags( ), MapWidth, MapHeight, gameName, hostName, upTime, map->GetMapPath( ), map->GetMapCRC( ), map->GetMapSHA1( ), FixedHostCounter ) );
			else
				EnqueuePacket( m_Protocol->SEND_SID_STARTADVEX3( state, Util::fromUInt32( MapGameType), map->GetMapGameFlags( ), map->GetMapWidth( ), map->GetMapHeight( ), gameName, hostName, upTime, map->GetMapPath( ), map->GetMapCRC( ), map->GetMapSHA1( ), FixedHostCounter ) );
		}
	}
}

void CBNET :: QueueGameUncreate( )
{
	if( m_LoggedIn )
		EnqueuePacket( m_Protocol->SEND_SID_STOPADV( ) );
}

void CBNET :: UnqueuePackets( unsigned char type )
{
	QQueue<QByteArray> Packets;
	quint32 Unqueued = 0;

	while( !m_OutPackets.isEmpty( ) )
	{
		// todotodo: it's very inefficient to have to copy all these packets while searching the queue

		QByteArray Packet = m_OutPackets.front( );
		m_OutPackets.dequeue( );

		if( Packet.size( ) >= 2 && Packet.at(1) == type )
			Unqueued++;
		else
			Packets.enqueue( Packet );
	}

	m_OutPackets = Packets;

	if( Unqueued > 0 )
		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] unqueued " + QString::number( Unqueued ) + " packets of type " + QString::number( type ) );
}

void CBNET :: UnqueueChatCommand( QString chatCommand )
{
	// hackhack: this is ugly code
	// generate the packet that would be sent for this chat command
	// then search the queue for that exact packet

	QByteArray PacketToUnqueue = m_Protocol->SEND_SID_CHATCOMMAND( chatCommand );
	QQueue<QByteArray> Packets;
	quint32 Unqueued = 0;

	while( !m_OutPackets.isEmpty( ) )
	{
		// todotodo: it's very inefficient to have to copy all these packets while searching the queue

		QByteArray Packet = m_OutPackets.front( );
		m_OutPackets.dequeue( );

		if( Packet == PacketToUnqueue )
			Unqueued++;
		else
			Packets.enqueue( Packet );
	}

	m_OutPackets = Packets;

	if( Unqueued > 0 )
		CONSOLE_Print( "[BNET: " + m_ServerAlias + "] unqueued " + QString::number( Unqueued ) + " chat command packets" );
}

void CBNET :: UnqueueGameRefreshes( )
{
	UnqueuePackets( CBNETProtocol :: SID_STARTADVEX3 );
}

bool CBNET :: IsAdmin( QString name )
{
	name = name.toLower();

	for( QList<QString> :: const_iterator i = m_Admins.begin( ); i != m_Admins.end( ); i++ )
	{
		if( *i == name )
			return true;
	}

	return false;
}

bool CBNET :: IsRootAdmin( QString name )
{
	// m_RootAdmin was already transformed to lower case in the constructor

	name = name.toLower();

	// updated to permit multiple root admins seperated by a space, e.g. "Varlock Kilranin Instinct121"
	// note: this function gets called frequently so it would be better to parse the root admins just once and store them in a list somewhere
	// however, it's hardly worth optimizing at this point since the code's already written

	QTextStream SS(&m_RootAdmin);

	QString s;

	while( !SS.atEnd( ) )
	{
		SS >> s;

		if( name == s )
			return true;
	}

	return false;
}

CDBBan *CBNET :: IsBannedName( QString name )
{
	name = name.toLower();

	// todotodo: optimize this - maybe use a map?

	for( QList<CDBBan *> :: const_iterator i = m_Bans.begin( ); i != m_Bans.end( ); i++ )
	{
		if( (*i)->GetName( ) == name )
			return *i;
	}

	return NULL;
}

CDBBan *CBNET :: IsBannedIP( QString ip )
{
	// todotodo: optimize this - maybe use a map?

	for( QList<CDBBan *> :: const_iterator i = m_Bans.begin( ); i != m_Bans.end( ); i++ )
	{
		if( (*i)->GetIP( ) == ip )
			return *i;
	}

	return NULL;
}

void CBNET :: AddAdmin( QString name )
{
	name = name.toLower();
	m_Admins.push_back( name );
}

void CBNET :: AddBan( QString name, QString ip, QString gamename, QString admin, QString reason )
{
	name = name.toLower();
	m_Bans.push_back( new CDBBan( m_Server, name, ip, "N/A", gamename, admin, reason ) );
}

void CBNET :: RemoveAdmin( QString name )
{
	name = name.toLower();

	for( QList<QString> :: iterator i = m_Admins.begin( ); i != m_Admins.end( ); )
	{
		if( *i == name )
			i = m_Admins.erase( i );
		else
			i++;
	}
}

void CBNET :: RemoveBan( QString name )
{
	name = name.toLower();

	for( QList<CDBBan *> :: iterator i = m_Bans.begin( ); i != m_Bans.end( ); )
	{
		if( (*i)->GetName( ) == name )
			i = m_Bans.erase( i );
		else
			i++;
	}
}

void CBNET :: HoldFriends( CBaseGame *game )
{
	if( game )
	{
		for( QList<CIncomingFriendList *> :: const_iterator i = m_Friends.begin( ); i != m_Friends.end( ); i++ )
			game->AddToReserved( (*i)->GetAccount( ) );
	}
}

void CBNET :: HoldClan( CBaseGame *game )
{
	if( game )
	{
		for( QList<CIncomingClanList *> :: const_iterator i = m_Clans.begin( ); i != m_Clans.end( ); i++ )
			game->AddToReserved( (*i)->GetName( ) );
	}
}
