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
#include "sha1.h"
#include "csvparser.h"
#include "config.h"
#include "language.h"
#include "ghostdb.h"
#include "ghostdbsqlite.h"
#include "ghostdbmysql.h"
#include "bnet.h"
#include "map.h"
#include "packed.h"
#include "savegame.h"
#include "gameplayer.h"
#include "gameprotocol.h"
#include "gpsprotocol.h"
#include "game_base.h"
#include "game.h"
#include "game_admin.h"

#include <signal.h>
#include <stdlib.h>
#include <QFile>
#include <QTextStream>

#ifdef WIN32
 #include <ws2tcpip.h>		// for WSAIoctl
#endif

#define __STORMLIB_SELF__
#include <stormlib/StormLib.h>

#include <QHostInfo>
#include <QCoreApplication>


//
// CGHost
//

CGHost :: CGHost( CConfig *CFG, QString configFile )
	: m_ConfigFile(configFile)
{
	m_UDPSocket = new QUdpSocket(this);
	m_UDPSocket->setProperty("target", CFG->GetString( "udp_broadcasttarget", QString( ) ) );
	m_UDPSocket->setProperty("dontroute", CFG->GetInt( "udp_dontroute", 0 ) == 0 ? false : true );
	m_ReconnectSocket = NULL;
	m_GPSProtocol = new CGPSProtocol( );
	m_CRC = new CCRC32( );
	m_CRC->Initialize( );
	m_SHA = new CSHA1( );
	m_CurrentGame = NULL;
	QString DBType = CFG->GetString( "db_type", "sqlite3" );

	m_CallableUpdateTimer.setInterval(200);
	QObject::connect(&m_CallableUpdateTimer, SIGNAL(timeout()), this, SLOT(EventCallableUpdateTimeout()));

	m_AutoHostTimer.setSingleShot(true);
	m_AutoHostTimer.setInterval(500);
	QObject::connect(&m_AutoHostTimer, SIGNAL(timeout()), this, SLOT(EventAutoHost()));

	// create connections

	CONSOLE_Print( "[GHOST] opening primary database" );

	if( DBType == "mysql" )
	{
#ifdef GHOST_MYSQL
		m_DB = new CGHostDBMySQL( CFG );
#else
		CONSOLE_Print( "[GHOST] warning - this binary was not compiled with MySQL database support, using SQLite database instead" );
		m_DB = new CGHostDBSQLite( CFG );
#endif
	}
	else
		m_DB = new CGHostDBSQLite( CFG );

	CONSOLE_Print( "[GHOST] opening secondary (local) database" );
	m_DBLocal = new CGHostDBSQLite( CFG );

	if (m_DB->HasError() )
	{
		CONSOLE_Print( "[GHOST] database initialization error - " + m_DB->GetError() );
		deleteLater();
		return;
	}
	if (m_DBLocal->HasError() )
	{
		CONSOLE_Print( "[GHOST] local database initialization error - " + m_DB->GetError() );
		deleteLater();
		return;
	}

	QObject::connect(m_DBLocal, SIGNAL(error(QString)), this, SLOT(EventDatabaseError(QString)));
	QObject::connect(m_DB, SIGNAL(error(QString)), this, SLOT(EventDatabaseError(QString)));

	// get a list of local IP addresses
	// this list is used elsewhere to determine if a player connecting to the bot is local or not

	CONSOLE_Print( "[GHOST] attempting to find local IP addresses" );

	QString HostName = QHostInfo::localHostName();

	CONSOLE_Print( "[GHOST] local hostname is [" + HostName + "]" );

	QHostInfo info = QHostInfo::fromName(HostName);

	for (int i = 0; i < info.addresses().size(); i++)
	{
		CONSOLE_Print( "[GHOST] local IP address #" + QString::number( i + 1 ) + " is [" + info.addresses().at(i).toString() + "]" );
		m_LocalAddresses.push_back( Util::fromUInt32(info.addresses().at(i).toIPv4Address() ) );
	}

	m_Language = NULL;
	m_Exiting = false;
	m_ExitingNice = false;
	m_Enabled = true;
	m_Version = "17.0";
	m_HostCounter = 1;
	m_AutoHostMaximumGames = CFG->GetInt( "autohost_maxgames", 0 );
	m_AutoHostAutoStartPlayers = CFG->GetInt( "autohost_startplayers", 0 );
	m_AutoHostGameName = CFG->GetString( "autohost_gamename", QString( ) );
	m_AutoHostOwner = CFG->GetString( "autohost_owner", QString( ) );
	m_AutoHostMatchMaking = false;
	m_AutoHostMinimumScore = 0.0;
	m_AutoHostMaximumScore = 0.0;
	m_AllGamesFinished = false;
	m_AllGamesFinishedTime = 0;
	m_TFT = CFG->GetInt( "bot_tft", 1 ) == 0 ? false : true;

	if( m_TFT )
		CONSOLE_Print( "[GHOST] acting as Warcraft III: The Frozen Throne" );
	else
		CONSOLE_Print( "[GHOST] acting as Warcraft III: Reign of Chaos" );

	m_HostPort = CFG->GetInt( "bot_hostport", 6112 );
	m_Reconnect = CFG->GetInt( "bot_reconnect", 1 ) == 0 ? false : true;
	m_ReconnectPort = CFG->GetInt( "bot_reconnectport", 6114 );
	m_DefaultMap = CFG->GetString( "bot_defaultmap", "map" );
	m_AdminGameCreate = CFG->GetInt( "admingame_create", 0 ) == 0 ? false : true;
	m_AdminGamePort = CFG->GetInt( "admingame_port", 6113 );
	m_AdminGamePassword = CFG->GetString( "admingame_password", QString( ) );
	m_AdminGameMap = CFG->GetString( "admingame_map", QString( ) );
	m_LANWar3Version = CFG->GetInt( "lan_war3version", 24 );
	m_ReplayWar3Version = CFG->GetInt( "replay_war3version", 24 );
	m_ReplayBuildNumber = CFG->GetInt( "replay_buildnumber", 6059 );
	SetConfigs( CFG );

	if (m_Reconnect)
		CreateReconnectServer();

	// load the battle.net connections
	// we're just loading the config data and creating the CBNET classes here, the connections are established later (in the Update function)

	for( quint32 i = 1; i < 10; i++ )
	{
		QString Prefix;

		if( i == 1 )
			Prefix = "bnet_";
		else
			Prefix = "bnet" + QString::number( i ) + "_";

		QString Server = CFG->GetString( Prefix + "server", QString( ) );
		QString ServerAlias = CFG->GetString( Prefix + "serveralias", QString( ) );
		QString CDKeyROC = CFG->GetString( Prefix + "cdkeyroc", QString( ) );
		QString CDKeyTFT = CFG->GetString( Prefix + "cdkeytft", QString( ) );
		QString CountryAbbrev = CFG->GetString( Prefix + "countryabbrev", "USA" );
		QString Country = CFG->GetString( Prefix + "country", "United States" );
		QString Locale = CFG->GetString( Prefix + "locale", "system" );
		quint32 LocaleID;

		if( Locale == "system" )
		{
#ifdef WIN32
			LocaleID = GetUserDefaultLangID( );
#else
			LocaleID = 1033;
#endif
		}
		else
			LocaleID = Locale.toUInt();

		QString UserName = CFG->GetString( Prefix + "username", QString( ) );
		QString UserPassword = CFG->GetString( Prefix + "password", QString( ) );
		QString FirstChannel = CFG->GetString( Prefix + "firstchannel", "The Void" );
		QString RootAdmin = CFG->GetString( Prefix + "rootadmin", QString( ) );
		QString BNETCommandTrigger = CFG->GetString( Prefix + "commandtrigger", "!" );

		if( BNETCommandTrigger.isEmpty( ) )
			BNETCommandTrigger = "!";

		bool HoldFriends = CFG->GetInt( Prefix + "holdfriends", 1 ) == 0 ? false : true;
		bool HoldClan = CFG->GetInt( Prefix + "holdclan", 1 ) == 0 ? false : true;
		bool PublicCommands = CFG->GetInt( Prefix + "publiccommands", 1 ) == 0 ? false : true;
		QString BNLSServer = CFG->GetString( Prefix + "bnlsserver", QString( ) );
		int BNLSPort = CFG->GetInt( Prefix + "bnlsport", 9367 );
		int BNLSWardenCookie = CFG->GetInt( Prefix + "bnlswardencookie", 0 );
		unsigned char War3Version = CFG->GetInt( Prefix + "custom_war3version", 24 );
		QByteArray EXEVersion = UTIL_ExtractNumbers( CFG->GetString( Prefix + "custom_exeversion", QString( ) ), 4 );
		QByteArray EXEVersionHash = UTIL_ExtractNumbers( CFG->GetString( Prefix + "custom_exeversionhash", QString( ) ), 4 );
		QString PasswordHashType = CFG->GetString( Prefix + "custom_passwordhashtype", QString( ) );
		QString PVPGNRealmName = CFG->GetString( Prefix + "custom_pvpgnrealmname", "PvPGN Realm" );
		quint32 MaxMessageLength = CFG->GetInt( Prefix + "custom_maxmessagelength", 200 );

		if( Server.isEmpty( ) )
			break;

		if( CDKeyROC.isEmpty( ) )
		{
			CONSOLE_Print( "[GHOST] missing " + Prefix + "cdkeyroc, skipping this battle.net connection" );
			continue;
		}

		if( m_TFT && CDKeyTFT.isEmpty( ) )
		{
			CONSOLE_Print( "[GHOST] missing " + Prefix + "cdkeytft, skipping this battle.net connection" );
			continue;
		}

		if( UserName.isEmpty( ) )
		{
			CONSOLE_Print( "[GHOST] missing " + Prefix + "username, skipping this battle.net connection" );
			continue;
		}

		if( UserPassword.isEmpty( ) )
		{
			CONSOLE_Print( "[GHOST] missing " + Prefix + "password, skipping this battle.net connection" );
			continue;
		}

		CONSOLE_Print( "[GHOST] found battle.net connection #" + QString::number( i ) + " for server [" + Server + "]" );

		if( Locale == "system" )
		{
#ifdef WIN32
			CONSOLE_Print( "[GHOST] using system locale of " + QString::number( LocaleID ) );
#else
			CONSOLE_Print( "[GHOST] unable to get system locale, using default locale of 1033" );
#endif
		}

		m_BNETs.push_back( new CBNET( this, Server, ServerAlias, BNLSServer, (quint16)BNLSPort, (quint32)BNLSWardenCookie, CDKeyROC, CDKeyTFT, CountryAbbrev, Country, LocaleID, UserName, UserPassword, FirstChannel, RootAdmin, BNETCommandTrigger.at(0).toAscii(), HoldFriends, HoldClan, PublicCommands, War3Version, EXEVersion, EXEVersionHash, PasswordHashType, PVPGNRealmName, MaxMessageLength, i ) );
	}

	if( m_BNETs.isEmpty( ) )
		CONSOLE_Print( "[GHOST] warning - no battle.net connections found in config file" );

	// extract common.j and blizzard.j from War3Patch.mpq if we can
	// these two files are necessary for calculating "map_crc" when loading maps so we make sure to do it before loading the default map
	// see CMap :: Load for more information

	ExtractScripts( );

	// load the default maps (note: make sure to run ExtractScripts first)

	if( m_DefaultMap.size( ) < 4 || m_DefaultMap.mid( m_DefaultMap.size( ) - 4 ) != ".cfg" )
	{
		m_DefaultMap += ".cfg";
		CONSOLE_Print( "[GHOST] adding \".cfg\" to default map -> new default is [" + m_DefaultMap + "]" );
	}

	CConfig MapCFG;
	MapCFG.Read( m_MapCFGPath + m_DefaultMap );
	m_Map = new CMap( this, &MapCFG, m_MapCFGPath + m_DefaultMap );

	if( !m_AdminGameMap.isEmpty( ) )
	{
		if( m_AdminGameMap.size( ) < 4 || m_AdminGameMap.mid( m_AdminGameMap.size( ) - 4 ) != ".cfg" )
		{
			m_AdminGameMap += ".cfg";
			CONSOLE_Print( "[GHOST] adding \".cfg\" to default admin game map -> new default is [" + m_AdminGameMap + "]" );
		}

		CONSOLE_Print( "[GHOST] trying to load default admin game map" );
		CConfig AdminMapCFG;
		AdminMapCFG.Read( m_MapCFGPath + m_AdminGameMap );
		m_AdminMap = new CMap( this, &AdminMapCFG, m_MapCFGPath + m_AdminGameMap );

		if( !m_AdminMap->GetValid( ) )
		{
			CONSOLE_Print( "[GHOST] default admin game map isn't valid, using hardcoded admin game map instead" );
			delete m_AdminMap;
			m_AdminMap = new CMap( this );
		}
	}
	else
	{
		CONSOLE_Print( "[GHOST] using hardcoded admin game map" );
		m_AdminMap = new CMap( this );
	}

	m_AutoHostMap = new CMap( *m_Map );
	m_SaveGame = new CSaveGame( );

	// load the iptocountry data

	LoadIPToCountryData( );

	// create the admin game

	if( m_AdminGameCreate )
	{
		CONSOLE_Print( "[GHOST] creating admin game" );
		m_AdminGame = new CAdminGame( this, m_AdminMap, NULL, m_AdminGamePort, 0, "GHost++ Admin Game", m_AdminGamePassword );
		QObject::connect(m_AdminGame, SIGNAL(destroyed()), this, SLOT(EventAdminGameDeleted()));

		if( m_AdminGamePort == m_HostPort )
			CONSOLE_Print( "[GHOST] warning - admingame_port and bot_hostport are set to the same value, you won't be able to host any games" );
	}
	else
		m_AdminGame = NULL;

	if( m_BNETs.isEmpty( ) && !m_AdminGame )
		CONSOLE_Print( "[GHOST] warning - no battle.net connections found and no admin game created" );

#ifdef GHOST_MYSQL
	CONSOLE_Print( "[GHOST] GHost++ Version " + m_Version + " (with MySQL support)" );
#else
	CONSOLE_Print( "[GHOST] GHost++ Version " + m_Version + " (without MySQL support)" );
#endif

	if( m_BNETs.isEmpty( ) )
		m_AutoHostTimer.start(0);
	else
	{
		for (QList<CBNET *>::const_iterator i = m_BNETs.begin(); i != m_BNETs.end(); ++i)
			(*i)->socketConnect();
	}
}

void CGHost::EventAdminGameDeleted()
{
	CONSOLE_Print("[GHOST] Admin Game has been deleted, quitting.");
	m_AdminGame = NULL;
	deleteLater();
}

CGHost :: ~CGHost( )
{
	delete m_GPSProtocol;
	delete m_CRC;
	delete m_SHA;

	qDeleteAll( m_BNETs );
	m_BNETs.clear( );

	delete m_CurrentGame;
	delete m_AdminGame;

	qDeleteAll( m_Games );
	m_Games.clear( );

	delete m_DB;
	delete m_DBLocal;

	// warning: we don't delete any entries of m_Callables here because we can't be guaranteed that the associated threads have terminated
	// this is fine if the program is currently exiting because the OS will clean up after us
	// but if you try to recreate the CGHost object within a single session you will probably leak resources!

	if( !m_Callables.isEmpty( ) )
		CONSOLE_Print( "[GHOST] warning - " + QString::number( m_Callables.size( ) ) + " orphaned callables were leaked (this is not an error)" );

	delete m_Language;
	delete m_Map;
	delete m_AdminMap;
	delete m_AutoHostMap;
	delete m_SaveGame;

	DEBUG_Print("[GHOST] good bye");
}

void CGHost::EventGameStarted()
{
	DEBUG_Print("EventGameStarted");
	// move the game to the games in progress vector
	m_Games.push_back( m_CurrentGame );
	m_CurrentGame = NULL;

	// and finally reenter battle.net chat

	for( QList<CBNET *> :: const_iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
	{
		(*i)->QueueGameUncreate( );
		(*i)->QueueEnterChat( );
	}

	m_AutoHostTimer.start();
}

void CGHost::EventIncomingReconnection()
{
	QTcpSocket *con = m_ReconnectSocket->nextPendingConnection();
	if (con == NULL)
		return;

	QObject::connect(con, SIGNAL(readyRead()), this, SLOT(EventReconnectionSocketReadyRead()));
}

void CGHost::EventReconnectionSocketReadyRead()
{
	QTcpSocket* con = (QTcpSocket*)QObject::sender();

	// a packet is at least 4 bytes
	if (con->bytesAvailable() < 4)
		return;

	if( (unsigned char)con->peek(1).at(0) == GPS_HEADER_CONSTANT )
	{
		// bytes 2 and 3 contain the length of the packet

		quint16 Length = Util::extractUInt16( con->peek(4), 2 );

		if( Length < 4 )
		{
			con->write( m_GPSProtocol->SEND_GPSS_REJECT( REJECTGPS_INVALID ) );
			con->deleteLater();
			return;
		}

		if( con->bytesAvailable() < Length )
			return;

		QByteArray Bytes = con->read(Length);
		if( Bytes.at(0) == CGPSProtocol :: GPS_RECONNECT && Length == 13 )
		{
			unsigned char PID = Bytes.at(4);
			quint32 ReconnectKey = Util::extractUInt32( Bytes, 5 );
			quint32 LastPacket = Util::extractUInt32( Bytes, 9 );

			// look for a matching player in a running game

			CGamePlayer *Match = NULL;

			for( QList<CBaseGame *> :: iterator j = m_Games.begin( ); j != m_Games.end( ); j++ )
			{
				if( (*j)->GetGameLoaded( ) )
				{
					CGamePlayer *Player = (*j)->GetPlayerFromPID( PID );

					if( Player && Player->GetGProxy( ) && Player->GetGProxyReconnectKey( ) == ReconnectKey )
					{
						Match = Player;
						break;
					}
				}
			}

			if( Match )
			{
				// reconnect successful!
				Match->EventGProxyReconnect( con, LastPacket );
				return;
			}
			else
			{
				con->write( m_GPSProtocol->SEND_GPSS_REJECT( REJECTGPS_NOTFOUND ) );
				con->deleteLater();
				return;
			}
		}
		else
		{
			con->write( m_GPSProtocol->SEND_GPSS_REJECT( REJECTGPS_INVALID ) );
			con->deleteLater();
			return;
		}
	}
}

void CGHost::EventCallableUpdateTimeout()
{
	// update callables

	for( QList<CBaseCallable *> :: iterator i = m_Callables.begin( ); i != m_Callables.end( ); )
	{
		if( (*i)->GetReady( ) )
		{
			m_DB->RecoverCallable( *i );
			delete *i;
			i = m_Callables.erase( i );
		}
		else
			i++;
	}
}

void CGHost::EventDatabaseError(const QString &error)
{
	// todotodo: do we really want to shutdown if there's a database error? is there any way to recover from this?
	if( QObject::sender() == m_DB )
		CONSOLE_Print( "[GHOST] database error - " + error );

	else if (QObject::sender() == m_DBLocal)
		CONSOLE_Print( "[GHOST] local database error - " + error );

	deleteLater();
}

void CGHost::EventExitNice()
{
	m_ExitingNice = true;

	if( !m_BNETs.isEmpty( ) )
	{
		CONSOLE_Print( "[GHOST] deleting all battle.net connections in preparation for exiting nicely" );

		for( QList<CBNET *> :: const_iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
			delete *i;

		m_BNETs.clear( );
	}

	if( m_CurrentGame )
	{
		CONSOLE_Print( "[GHOST] deleting current game in preparation for exiting nicely" );
		delete m_CurrentGame;
		m_CurrentGame = NULL;
	}

	if( m_AdminGame )
	{
		CONSOLE_Print( "[GHOST] deleting admin game in preparation for exiting nicely" );
		delete m_AdminGame;
		m_AdminGame = NULL;
	}

	if( m_Games.isEmpty( ) )
	{
		if( !m_AllGamesFinished && m_Callables.size( ) > 0 )
		{
			CONSOLE_Print( "[GHOST] all games finished, waiting 60 seconds for threads to finish" );
			CONSOLE_Print( "[GHOST] there are " + QString::number( m_Callables.size( ) ) + " threads in progress" );
			m_AllGamesFinished = true;
			m_AllGamesFinishedTime = GetTime( );
			QTimer::singleShot(60000, this, SLOT(EventWaitForNiceExitTimeout()));
		}
		else if( m_Callables.isEmpty( ) )
		{
			CONSOLE_Print( "[GHOST] all threads finished, exiting nicely" );
			deleteLater();
			QCoreApplication::instance()->quit();
			return;
		}
	}

	// try again in 2 seconds
	QTimer::singleShot(2000, this, SLOT(EventExitNice()));
}

void CGHost::EventWaitForNiceExitTimeout()
{
	CONSOLE_Print( "[GHOST] waited 60 seconds for threads to finish, exiting anyway" );
	CONSOLE_Print( "[GHOST] there are " + QString::number( m_Callables.size( ) ) + " threads still in progress which will be terminated" );
	deleteLater();
}

void CGHost::EventAutoHost()
{
	DEBUG_Print("EventAutoHost");
	// autohost
	if( m_AutoHostGameName.isEmpty( ) || m_AutoHostMaximumGames == 0 || m_AutoHostAutoStartPlayers == 0 )
		return;

	// copy all the checks from CGHost :: CreateGame here because we don't want to spam the chat when there's an error

	if (m_CurrentGame)
		DEBUG_Print("Not autohosting because a game is still open");

	if (m_Games.size( ) >= m_MaxGames)
		DEBUG_Print("Not autohosting because m_Games.size( ) >= m_MaxGames");

	if (m_Games.size( ) >= m_AutoHostMaximumGames)
		DEBUG_Print("Not autohosting because m_Games.size( ) >= m_AutoHostMaximumGames");

	if( m_ExitingNice || !m_Enabled || m_CurrentGame || m_Games.size( ) >= m_MaxGames || m_Games.size( ) >= m_AutoHostMaximumGames )
		return;

	if( !m_AutoHostMap->GetValid( ) )
	{
		CONSOLE_Print( "[GHOST] stopped auto hosting, map config file [" + m_AutoHostMap->GetCFGFile( ) + "] is invalid" );
		m_AutoHostGameName.clear( );
		m_AutoHostOwner.clear( );
		m_AutoHostServer.clear( );
		m_AutoHostMaximumGames = 0;
		m_AutoHostAutoStartPlayers = 0;
		m_AutoHostMatchMaking = false;
		m_AutoHostMinimumScore = 0.0;
		m_AutoHostMaximumScore = 0.0;
		return;
	}

	QString GameName = m_AutoHostGameName + " #" + QString::number( m_HostCounter );

	if( GameName.size( ) >= 31 )
	{
		CONSOLE_Print( "[GHOST] stopped auto hosting, next game name [" + GameName + "] is too long (the maximum is 31 characters)" );
		m_AutoHostGameName.clear( );
		m_AutoHostOwner.clear( );
		m_AutoHostServer.clear( );
		m_AutoHostMaximumGames = 0;
		m_AutoHostAutoStartPlayers = 0;
		m_AutoHostMatchMaking = false;
		m_AutoHostMinimumScore = 0.0;
		m_AutoHostMaximumScore = 0.0;
		return;
	}

	CreateGame( m_AutoHostMap, GAME_PUBLIC, false, GameName, m_AutoHostOwner, m_AutoHostOwner, m_AutoHostServer, false );

	if( !m_CurrentGame )
		return;

	m_LastAutoHostTime.restart();
	m_CurrentGame->SetAutoStartPlayers( m_AutoHostAutoStartPlayers );

	if( !m_AutoHostMatchMaking )
		return;

	if( m_Map->GetMapMatchMakingCategory( ).isEmpty( ) )
	{
		CONSOLE_Print( "[GHOST] autohostmm - map_matchmakingcategory not found, matchmaking disabled" );
		return;
	}

	if( !( m_Map->GetMapOptions( ) & MAPOPT_FIXEDPLAYERSETTINGS ) )
	{
		CONSOLE_Print( "[GHOST] autohostmm - map_matchmakingcategory [" + m_Map->GetMapMatchMakingCategory( ) + "] found but matchmaking can only be used with fixed player settings, matchmaking disabled" );
		return;
	}

	CONSOLE_Print( "[GHOST] autohostmm - map_matchmakingcategory [" + m_Map->GetMapMatchMakingCategory( ) + "] found, matchmaking enabled" );

	m_CurrentGame->SetMatchMaking( true );
	m_CurrentGame->SetMinimumScore( m_AutoHostMinimumScore );
	m_CurrentGame->SetMaximumScore( m_AutoHostMaximumScore );
}

void CGHost::CreateReconnectServer()
{
	// create the GProxy++ reconnect listener
	if( !m_ReconnectSocket )
	{
		m_ReconnectSocket = new QTcpServer( this );
		QObject::connect(m_ReconnectSocket, SIGNAL(newConnection()), this, SLOT(EventIncomingReconnection()));

		if( m_ReconnectSocket->listen( QHostAddress(m_BindAddress), m_ReconnectPort ) )
			CONSOLE_Print( "[GHOST] listening for GProxy++ reconnects on port " + QString::number( m_ReconnectPort ) );
		else
		{
			CONSOLE_Print( "[GHOST] error listening for GProxy++ reconnects on port " + QString::number( m_ReconnectPort ) );
			delete m_ReconnectSocket;
			m_ReconnectSocket = NULL;
			m_Reconnect = false;
		}
	}
	else if( !m_ReconnectSocket->isListening() )
	{
		CONSOLE_Print( "[GHOST] GProxy++ reconnect listener error (" + m_ReconnectSocket->errorString() + ")" );
		delete m_ReconnectSocket;
		m_ReconnectSocket = NULL;
		m_Reconnect = false;
	}
}

void CGHost :: EventBNETConnecting( CBNET *bnet )
{
	if( m_AdminGame )
		m_AdminGame->SendAllChat( m_Language->ConnectingToBNET( bnet->GetServer( ) ) );

	if( m_CurrentGame )
		m_CurrentGame->SendAllChat( m_Language->ConnectingToBNET( bnet->GetServer( ) ) );
}

void CGHost :: EventBNETConnected( CBNET *bnet )
{
	if( m_AdminGame )
		m_AdminGame->SendAllChat( m_Language->ConnectedToBNET( bnet->GetServer( ) ) );

	if( m_CurrentGame )
		m_CurrentGame->SendAllChat( m_Language->ConnectedToBNET( bnet->GetServer( ) ) );
}

void CGHost :: EventBNETDisconnected( CBNET *bnet )
{
	if( m_AdminGame )
		m_AdminGame->SendAllChat( m_Language->DisconnectedFromBNET( bnet->GetServer( ) ) );

	if( m_CurrentGame )
		m_CurrentGame->SendAllChat( m_Language->DisconnectedFromBNET( bnet->GetServer( ) ) );
}

void CGHost :: EventBNETLoggedIn( CBNET *bnet )
{
	if( m_AdminGame )
		m_AdminGame->SendAllChat( m_Language->LoggedInToBNET( bnet->GetServer( ) ) );

	if( m_CurrentGame )
		m_CurrentGame->SendAllChat( m_Language->LoggedInToBNET( bnet->GetServer( ) ) );
}

void CGHost :: EventBNETGameRefreshed( CBNET *bnet )
{
	if( m_AdminGame )
		m_AdminGame->SendAllChat( m_Language->BNETGameHostingSucceeded( bnet->GetServer( ) ) );

	if( m_CurrentGame )
		m_CurrentGame->EventGameRefreshed( bnet->GetServer( ) );
}

void CGHost :: EventBNETGameRefreshFailed( CBNET *bnet )
{
	if( !m_CurrentGame )
		return;

	for( QList<CBNET *> :: const_iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
	{
		(*i)->QueueChatCommand( m_Language->UnableToCreateGameTryAnotherName( bnet->GetServer( ), m_CurrentGame->GetGameName( ) ) );

		if( (*i)->GetServer( ) == m_CurrentGame->GetCreatorServer( ) )
			(*i)->QueueChatCommand( m_Language->UnableToCreateGameTryAnotherName( bnet->GetServer( ), m_CurrentGame->GetGameName( ) ), m_CurrentGame->GetCreatorName( ), true );
	}

	if( m_AdminGame )
		m_AdminGame->SendAllChat( m_Language->BNETGameHostingFailed( bnet->GetServer( ), m_CurrentGame->GetGameName( ) ) );

	m_CurrentGame->SendAllChat( m_Language->UnableToCreateGameTryAnotherName( bnet->GetServer( ), m_CurrentGame->GetGameName( ) ) );

	// we take the easy route and simply close the lobby if a refresh fails
	// it's possible at least one refresh succeeded and therefore the game is still joinable on at least one battle.net (plus on the local network) but we don't keep track of that
	// we only close the game if it has no players since we support game rehosting (via !priv and !pub in the lobby)

	if( m_CurrentGame->GetNumHumanPlayers( ) == 0 )
		m_CurrentGame->deleteLater();

	m_CurrentGame->EventRefreshError();
}

void CGHost :: EventBNETConnectTimedOut( CBNET *bnet )
{
	if( m_AdminGame )
		m_AdminGame->SendAllChat( m_Language->ConnectingToBNETTimedOut( bnet->GetServer( ) ) );

	if( m_CurrentGame )
		m_CurrentGame->SendAllChat( m_Language->ConnectingToBNETTimedOut( bnet->GetServer( ) ) );
}

void CGHost :: EventBNETWhisper( CBNET *bnet, QString user, QString message )
{
	if( m_AdminGame )
	{
		m_AdminGame->SendAdminChat( "[W: " + bnet->GetServerAlias( ) + "] [" + user + "] " + message );

		if( m_CurrentGame )
			m_CurrentGame->SendLocalAdminChat( "[W: " + bnet->GetServerAlias( ) + "] [" + user + "] " + message );

		for( QList<CBaseGame *> :: const_iterator i = m_Games.begin( ); i != m_Games.end( ); i++ )
			(*i)->SendLocalAdminChat( "[W: " + bnet->GetServerAlias( ) + "] [" + user + "] " + message );
	}
}

void CGHost :: EventBNETChat( CBNET *bnet, QString user, QString message )
{
	if( m_AdminGame )
	{
		m_AdminGame->SendAdminChat( "[L: " + bnet->GetServerAlias( ) + "] [" + user + "] " + message );

		if( m_CurrentGame )
			m_CurrentGame->SendLocalAdminChat( "[L: " + bnet->GetServerAlias( ) + "] [" + user + "] " + message );

		for( QList<CBaseGame *> :: const_iterator i = m_Games.begin( ); i != m_Games.end( ); i++ )
			(*i)->SendLocalAdminChat( "[L: " + bnet->GetServerAlias( ) + "] [" + user + "] " + message );
	}
}

void CGHost :: EventBNETEmote( CBNET *bnet, QString user, QString message )
{
	if( m_AdminGame )
	{
		m_AdminGame->SendAdminChat( "[E: " + bnet->GetServerAlias( ) + "] [" + user + "] " + message );

		if( m_CurrentGame )
			m_CurrentGame->SendLocalAdminChat( "[E: " + bnet->GetServerAlias( ) + "] [" + user + "] " + message );

		for( QList<CBaseGame *> :: const_iterator i = m_Games.begin( ); i != m_Games.end( ); i++ )
			(*i)->SendLocalAdminChat( "[E: " + bnet->GetServerAlias( ) + "] [" + user + "] " + message );
	}
}

void CGHost::EventGameDeleted()
{
	CBaseGame *game = (CBaseGame *)QObject::sender();

	if (game == m_CurrentGame)
		m_CurrentGame = NULL;

	m_Games.removeOne(game);

	m_AutoHostTimer.start();
}

void CGHost :: ReloadConfigs( )
{
	CConfig CFG;
	CFG.Read( "default.cfg" );
	CFG.Read( m_ConfigFile );
	SetConfigs( &CFG );
}

void CGHost :: SetConfigs( CConfig *CFG )
{
	// this doesn't set EVERY config value since that would potentially require reconfiguring the battle.net connections
	// it just set the easily reloadable values

	m_LanguageFile = CFG->GetString( "bot_language", "language.cfg" );
	delete m_Language;
	m_Language = new CLanguage( m_LanguageFile );
	m_Warcraft3Path = UTIL_AddPathSeparator( CFG->GetString( "bot_war3path", "C:\\Program Files\\Warcraft III\\" ) ).toAscii();
	m_BindAddress = CFG->GetString( "bot_bindaddress", QString( ) );
	m_ReconnectWaitTime = CFG->GetInt( "bot_reconnectwaittime", 3 );
	m_MaxGames = CFG->GetInt( "bot_maxgames", 5 );
	QString BotCommandTrigger = CFG->GetString( "bot_commandtrigger", "!" );

	if( BotCommandTrigger.isEmpty( ) )
		BotCommandTrigger = "!";

	m_CommandTrigger = BotCommandTrigger[0].toAscii();
	m_MapCFGPath = UTIL_AddPathSeparator( CFG->GetString( "bot_mapcfgpath", QString( ) ) );
	m_SaveGamePath = UTIL_AddPathSeparator( CFG->GetString( "bot_savegamepath", QString( ) ) );
	m_MapPath = UTIL_AddPathSeparator( CFG->GetString( "bot_mappath", QString( ) ) );
	m_SaveReplays = CFG->GetInt( "bot_savereplays", 0 ) == 0 ? false : true;
	m_ReplayPath = UTIL_AddPathSeparator( CFG->GetString( "bot_replaypath", QString( ) ) );
	m_VirtualHostName = CFG->GetString( "bot_virtualhostname", "|cFF4080C0GHost" );
	m_HideIPAddresses = CFG->GetInt( "bot_hideipaddresses", 0 ) == 0 ? false : true;
	m_CheckMultipleIPUsage = CFG->GetInt( "bot_checkmultipleipusage", 1 ) == 0 ? false : true;

	if( m_VirtualHostName.size( ) > 15 )
	{
		m_VirtualHostName = "|cFF4080C0GHost";
		CONSOLE_Print( "[GHOST] warning - bot_virtualhostname is longer than 15 characters, using default virtual host name" );
	}

	m_SpoofChecks = CFG->GetInt( "bot_spoofchecks", 2 );
	m_RequireSpoofChecks = CFG->GetInt( "bot_requirespoofchecks", 0 ) == 0 ? false : true;
	m_ReserveAdmins = CFG->GetInt( "bot_reserveadmins", 1 ) == 0 ? false : true;
	m_RefreshMessages = CFG->GetInt( "bot_refreshmessages", 0 ) == 0 ? false : true;
	m_AutoLock = CFG->GetInt( "bot_autolock", 0 ) == 0 ? false : true;
	m_AutoSave = CFG->GetInt( "bot_autosave", 0 ) == 0 ? false : true;
	m_AllowDownloads = CFG->GetInt( "bot_allowdownloads", 0 );
	m_PingDuringDownloads = CFG->GetInt( "bot_pingduringdownloads", 0 ) == 0 ? false : true;
	m_MaxDownloaders = CFG->GetInt( "bot_maxdownloaders", 3 );
	m_MaxDownloadSpeed = CFG->GetInt( "bot_maxdownloadspeed", 100 );
	m_LCPings = CFG->GetInt( "bot_lcpings", 1 ) == 0 ? false : true;
	m_AutoKickPing = CFG->GetInt( "bot_autokickping", 400 );
	m_BanMethod = CFG->GetInt( "bot_banmethod", 1 );
	m_IPBlackListFile = CFG->GetString( "bot_ipblacklistfile", "ipblacklist.txt" );
	m_LobbyTimeLimit = CFG->GetInt( "bot_lobbytimelimit", 10 );
	m_Latency = CFG->GetInt( "bot_latency", 100 );
	m_SyncLimit = CFG->GetInt( "bot_synclimit", 50 );
	m_VoteKickAllowed = CFG->GetInt( "bot_votekickallowed", 1 ) == 0 ? false : true;
	m_VoteKickPercentage = CFG->GetInt( "bot_votekickpercentage", 100 );

	if( m_VoteKickPercentage > 100 )
	{
		m_VoteKickPercentage = 100;
		CONSOLE_Print( "[GHOST] warning - bot_votekickpercentage is greater than 100, using 100 instead" );
	}

	m_MOTDFile = CFG->GetString( "bot_motdfile", "motd.txt" );
	m_GameLoadedFile = CFG->GetString( "bot_gameloadedfile", "gameloaded.txt" );
	m_GameOverFile = CFG->GetString( "bot_gameoverfile", "gameover.txt" );
	m_LocalAdminMessages = CFG->GetInt( "bot_localadminmessages", 1 ) == 0 ? false : true;
	m_TCPNoDelay = CFG->GetInt( "tcp_nodelay", 0 ) == 0 ? false : true;
	m_MatchMakingMethod = CFG->GetInt( "bot_matchmakingmethod", 1 );
}

void CGHost :: ExtractScripts( )
{
	QString PatchMPQFileName = m_Warcraft3Path + "War3Patch.mpq";
	HANDLE PatchMPQ;

	if( SFileOpenArchive( PatchMPQFileName.toUtf8().data( ), 0, MPQ_OPEN_FORCE_MPQ_V1, &PatchMPQ ) )
	{
		CONSOLE_Print( "[GHOST] loading MPQ file [" + PatchMPQFileName + "]" );
		HANDLE SubFile;

		// common.j

		if( SFileOpenFileEx( PatchMPQ, "Scripts\\common.j", 0, &SubFile ) )
		{
			quint32 FileLength = SFileGetFileSize( SubFile, NULL );

			if( FileLength > 0 && FileLength != 0xFFFFFFFF )
			{
				char *SubFileData = new char[FileLength];
				DWORD BytesRead = 0;

				if( SFileReadFile( SubFile, SubFileData, FileLength, &BytesRead ) )
				{
					CONSOLE_Print( "[GHOST] extracting Scripts\\common.j from MPQ file to [" + m_MapCFGPath + "common.j]" );
					UTIL_FileWrite( m_MapCFGPath + "common.j", QByteArray(SubFileData, BytesRead) );
				}
				else
					CONSOLE_Print( "[GHOST] warning - unable to extract Scripts\\common.j from MPQ file" );

				delete [] SubFileData;
			}

			SFileCloseFile( SubFile );
		}
		else
			CONSOLE_Print( "[GHOST] couldn't find Scripts\\common.j in MPQ file" );

		// blizzard.j

		if( SFileOpenFileEx( PatchMPQ, "Scripts\\blizzard.j", 0, &SubFile ) )
		{
			quint32 FileLength = SFileGetFileSize( SubFile, NULL );

			if( FileLength > 0 && FileLength != 0xFFFFFFFF )
			{
				char *SubFileData = new char[FileLength];
				DWORD BytesRead = 0;

				if( SFileReadFile( SubFile, SubFileData, FileLength, &BytesRead ) )
				{
					CONSOLE_Print( "[GHOST] extracting Scripts\\blizzard.j from MPQ file to [" + m_MapCFGPath + "blizzard.j]" );
					UTIL_FileWrite( m_MapCFGPath + "blizzard.j", QByteArray(SubFileData, BytesRead) );
				}
				else
					CONSOLE_Print( "[GHOST] warning - unable to extract Scripts\\blizzard.j from MPQ file" );

				delete [] SubFileData;
			}

			SFileCloseFile( SubFile );
		}
		else
			CONSOLE_Print( "[GHOST] couldn't find Scripts\\blizzard.j in MPQ file" );

		SFileCloseArchive( PatchMPQ );
	}
	else
		CONSOLE_Print( "[GHOST] warning - unable to load MPQ file [" + PatchMPQFileName + "] - error code " + QString::number( GetLastError( ) ) );
}

void CGHost :: LoadIPToCountryData( )
{
	QFile f("ip-to-country.csv");
	f.open(QFile::ReadOnly);
	QTextStream in(&f);

	if( f.error() != QFile::NoError )
		CONSOLE_Print( "[GHOST] warning - unable to read file [ip-to-country.csv], iptocountry data not loaded" );
	else
	{
		CONSOLE_Print( "[GHOST] started loading [ip-to-country.csv]" );

		// the begin and commit statements are optimizations
		// we're about to insert ~4 MB of data into the database so if we allow the database to treat each insert as a transaction it will take a LONG time
		// todotodo: handle begin/commit failures a bit more gracefully

		if( !m_DBLocal->Begin( ) )
			CONSOLE_Print( "[GHOST] warning - failed to begin local database transaction, iptocountry data not loaded" );
		else
		{
			unsigned char Percent = 0;
			QString Line;
			QString IP1;
			QString IP2;
			QString Country;
			CSVParser parser;

			// get length of file for the progress meter

			quint32 FileLength = f.size();

			while( !in.atEnd( ) )
			{
				Line = in.readLine();

				if( Line.isEmpty( ) )
					continue;

				parser << Line;
				parser >> IP1;
				parser >> IP2;
				parser >> Country;
				m_DBLocal->FromAdd( IP1.toUInt(), IP2.toUInt(), Country );

				// it's probably going to take awhile to load the iptocountry data (~10 seconds on my 3.2 GHz P4 when using SQLite3)
				// so let's print a progress meter just to keep the user from getting worried

				unsigned char NewPercent = (unsigned char)( (float)f.pos() / FileLength * 100 );

				if( NewPercent != Percent && NewPercent % 10 == 0 )
				{
					Percent = NewPercent;
					CONSOLE_Print( "[GHOST] iptocountry data: " + QString::number( Percent ) + "% loaded" );
				}
			}

			if( !m_DBLocal->Commit( ) )
				CONSOLE_Print( "[GHOST] warning - failed to commit local database transaction, iptocountry data not loaded" );
			else
				CONSOLE_Print( "[GHOST] finished loading [ip-to-country.csv]" );
		}

		f.close( );
	}
}

void CGHost :: CreateGame( CMap *map, unsigned char gameState, bool saveGame, QString gameName, QString ownerName, QString creatorName, QString creatorServer, bool whisper )
{
	if( !m_Enabled )
	{
		for( QList<CBNET *> :: const_iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
		{
			if( (*i)->GetServer( ) == creatorServer )
				(*i)->QueueChatCommand( m_Language->UnableToCreateGameDisabled( gameName ), creatorName, whisper );
		}

		if( m_AdminGame )
			m_AdminGame->SendAllChat( m_Language->UnableToCreateGameDisabled( gameName ) );

		return;
	}

	if( gameName.size( ) > 31 )
	{
		for( QList<CBNET *> :: const_iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
		{
			if( (*i)->GetServer( ) == creatorServer )
				(*i)->QueueChatCommand( m_Language->UnableToCreateGameNameTooLong( gameName ), creatorName, whisper );
		}

		if( m_AdminGame )
			m_AdminGame->SendAllChat( m_Language->UnableToCreateGameNameTooLong( gameName ) );

		return;
	}

	if( !map->GetValid( ) )
	{
		for( QList<CBNET *> :: const_iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
		{
			if( (*i)->GetServer( ) == creatorServer )
				(*i)->QueueChatCommand( m_Language->UnableToCreateGameInvalidMap( gameName ), creatorName, whisper );
		}

		if( m_AdminGame )
			m_AdminGame->SendAllChat( m_Language->UnableToCreateGameInvalidMap( gameName ) );

		return;
	}

	if( saveGame )
	{
		if( !m_SaveGame->GetValid( ) )
		{
			for( QList<CBNET *> :: const_iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
			{
				if( (*i)->GetServer( ) == creatorServer )
					(*i)->QueueChatCommand( m_Language->UnableToCreateGameInvalidSaveGame( gameName ), creatorName, whisper );
			}

			if( m_AdminGame )
				m_AdminGame->SendAllChat( m_Language->UnableToCreateGameInvalidSaveGame( gameName ) );

			return;
		}

		QString MapPath1 = m_SaveGame->GetMapPath( ).toLower();
		QString MapPath2 = map->GetMapPath( );
		MapPath2 = MapPath2.toLower();

		if( MapPath1 != MapPath2 )
		{
			CONSOLE_Print( "[GHOST] path mismatch, saved game path is [" + MapPath1 + "] but map path is [" + MapPath2 + "]" );

			for( QList<CBNET *> :: const_iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
			{
				if( (*i)->GetServer( ) == creatorServer )
					(*i)->QueueChatCommand( m_Language->UnableToCreateGameSaveGameMapMismatch( gameName ), creatorName, whisper );
			}

			if( m_AdminGame )
				m_AdminGame->SendAllChat( m_Language->UnableToCreateGameSaveGameMapMismatch( gameName ) );

			return;
		}

		if( m_EnforcePlayers.isEmpty( ) )
		{
			for( QList<CBNET *> :: const_iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
			{
				if( (*i)->GetServer( ) == creatorServer )
					(*i)->QueueChatCommand( m_Language->UnableToCreateGameMustEnforceFirst( gameName ), creatorName, whisper );
			}

			if( m_AdminGame )
				m_AdminGame->SendAllChat( m_Language->UnableToCreateGameMustEnforceFirst( gameName ) );

			return;
		}
	}

	if( m_CurrentGame )
	{
		for( QList<CBNET *> :: const_iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
		{
			if( (*i)->GetServer( ) == creatorServer )
				(*i)->QueueChatCommand( m_Language->UnableToCreateGameAnotherGameInLobby( gameName, m_CurrentGame->GetDescription( ) ), creatorName, whisper );
		}

		if( m_AdminGame )
			m_AdminGame->SendAllChat( m_Language->UnableToCreateGameAnotherGameInLobby( gameName, m_CurrentGame->GetDescription( ) ) );

		return;
	}

	if( m_Games.size( ) >= m_MaxGames )
	{
		for( QList<CBNET *> :: const_iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
		{
			if( (*i)->GetServer( ) == creatorServer )
				(*i)->QueueChatCommand( m_Language->UnableToCreateGameMaxGamesReached( gameName, QString::number( m_MaxGames ) ), creatorName, whisper );
		}

		if( m_AdminGame )
			m_AdminGame->SendAllChat( m_Language->UnableToCreateGameMaxGamesReached( gameName, QString::number( m_MaxGames ) ) );

		return;
	}

	CONSOLE_Print( "[GHOST] creating game [" + gameName + "]" );

	if( saveGame )
		m_CurrentGame = new CGame( this, map, m_SaveGame, m_HostPort, gameState, gameName, ownerName, creatorName, creatorServer );
	else
		m_CurrentGame = new CGame( this, map, NULL, m_HostPort, gameState, gameName, ownerName, creatorName, creatorServer );

	QObject::connect(m_CurrentGame, SIGNAL(startedLoading()), this, SLOT(EventGameStarted()));
	QObject::connect(m_CurrentGame, SIGNAL(destroyed()), this, SLOT(EventGameDeleted()));

	// todotodo: check if listening failed and report the error to the user

	if( m_SaveGame )
	{
		m_CurrentGame->SetEnforcePlayers( m_EnforcePlayers );
		m_EnforcePlayers.clear( );
	}

	for( QList<CBNET *> :: const_iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
	{
		if( whisper && (*i)->GetServer( ) == creatorServer )
		{
			// note that we send this whisper only on the creator server

			if( gameState == GAME_PRIVATE )
				(*i)->QueueChatCommand( m_Language->CreatingPrivateGame( gameName, ownerName ), creatorName, whisper );
			else if( gameState == GAME_PUBLIC )
				(*i)->QueueChatCommand( m_Language->CreatingPublicGame( gameName, ownerName ), creatorName, whisper );
		}
		else
		{
			// note that we send this chat message on all other bnet servers

			if( gameState == GAME_PRIVATE )
				(*i)->QueueChatCommand( m_Language->CreatingPrivateGame( gameName, ownerName ) );
			else if( gameState == GAME_PUBLIC )
				(*i)->QueueChatCommand( m_Language->CreatingPublicGame( gameName, ownerName ) );
		}

		if( saveGame )
			(*i)->QueueGameCreate( gameState, gameName, QString( ), map, m_SaveGame, m_CurrentGame->GetHostCounter( ) );
		else
			(*i)->QueueGameCreate( gameState, gameName, QString( ), map, NULL, m_CurrentGame->GetHostCounter( ) );
	}

	if( m_AdminGame )
	{
		if( gameState == GAME_PRIVATE )
			m_AdminGame->SendAllChat( m_Language->CreatingPrivateGame( gameName, ownerName ) );
		else if( gameState == GAME_PUBLIC )
			m_AdminGame->SendAllChat( m_Language->CreatingPublicGame( gameName, ownerName ) );
	}

	// if we're creating a private game we don't need to send any game refresh messages so we can rejoin the chat immediately
	// unfortunately this doesn't work on PVPGN servers because they consider an enterchat message to be a gameuncreate message when in a game
	// so don't rejoin the chat if we're using PVPGN

	if( gameState == GAME_PRIVATE )
	{
		for( QList<CBNET *> :: const_iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
		{
			if( (*i)->GetPasswordHashType( ) != "pvpgn" )
				(*i)->QueueEnterChat( );
		}
	}

	// hold friends and/or clan members

	for( QList<CBNET *> :: const_iterator i = m_BNETs.begin( ); i != m_BNETs.end( ); i++ )
	{
		if( (*i)->GetHoldFriends( ) )
			(*i)->HoldFriends( m_CurrentGame );

		if( (*i)->GetHoldClan( ) )
			(*i)->HoldClan( m_CurrentGame );
	}
}
