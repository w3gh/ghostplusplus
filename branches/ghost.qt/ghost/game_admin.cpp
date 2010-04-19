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
#include "socket.h"
#include "ghostdb.h"
#include "bnet.h"
#include "map.h"
#include "packed.h"
#include "savegame.h"
#include "replay.h"
#include "gameplayer.h"
#include "gameprotocol.h"
#include "game_base.h"
#include "game_admin.h"

#include <QString>

#include <boost/filesystem.hpp>

using namespace boost :: filesystem;

//
// CAdminGame
//

CAdminGame :: CAdminGame( CGHost *nGHost, CMap *nMap, CSaveGame *nSaveGame, uint16_t nHostPort, unsigned char nGameState, QString nGameName, QString nPassword ) : CBaseGame( nGHost, nMap, nSaveGame, nHostPort, nGameState, nGameName, QString( ), QString( ), QString( ) )
{
	m_VirtualHostName = "|cFFC04040Admin";
	m_MuteLobby = true;
	m_Password = nPassword;
}

CAdminGame :: ~CAdminGame( )
{
	for( QVector<PairedAdminCount> :: iterator i = m_PairedAdminCounts.begin( ); i != m_PairedAdminCounts.end( ); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for( QVector<PairedAdminAdd> :: iterator i = m_PairedAdminAdds.begin( ); i != m_PairedAdminAdds.end( ); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for( QVector<PairedAdminRemove> :: iterator i = m_PairedAdminRemoves.begin( ); i != m_PairedAdminRemoves.end( ); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for( QVector<PairedBanCount> :: iterator i = m_PairedBanCounts.begin( ); i != m_PairedBanCounts.end( ); i++ )
		m_GHost->m_Callables.push_back( i->second );

	/*

	for( QVector<PairedBanAdd> :: iterator i = m_PairedBanAdds.begin( ); i != m_PairedBanAdds.end( ); i++ )
		m_GHost->m_Callables.push_back( i->second );

	*/

	for( QVector<PairedBanRemove> :: iterator i = m_PairedBanRemoves.begin( ); i != m_PairedBanRemoves.end( ); i++ )
		m_GHost->m_Callables.push_back( i->second );
}

bool CAdminGame :: Update( void *fd, void *send_fd )
{
	//
	// update callables
	//

	for( QVector<PairedAdminCount> :: iterator i = m_PairedAdminCounts.begin( ); i != m_PairedAdminCounts.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			CGamePlayer *Player = GetPlayerFromName( i->first, true );

			if( Player )
			{
				uint32_t Count = i->second->GetResult( );

				if( Count == 0 )
					SendChat( Player, m_GHost->m_Language->ThereAreNoAdmins( i->second->GetServer( ) ) );
				else if( Count == 1 )
					SendChat( Player, m_GHost->m_Language->ThereIsAdmin( i->second->GetServer( ) ) );
				else
					SendChat( Player, m_GHost->m_Language->ThereAreAdmins( i->second->GetServer( ), UTIL_ToString( Count ) ) );
			}

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedAdminCounts.erase( i );
		}
		else
			i++;
	}

	for( QVector<PairedAdminAdd> :: iterator i = m_PairedAdminAdds.begin( ); i != m_PairedAdminAdds.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			if( i->second->GetResult( ) )
			{
				for( QVector<CBNET *> :: iterator j = m_GHost->m_BNETs.begin( ); j != m_GHost->m_BNETs.end( ); j++ )
				{
					if( (*j)->GetServer( ) == i->second->GetServer( ) )
						(*j)->AddAdmin( i->second->GetUser( ) );
				}
			}

			CGamePlayer *Player = GetPlayerFromName( i->first, true );

			if( Player )
			{
				if( i->second->GetResult( ) )
					SendChat( Player, m_GHost->m_Language->AddedUserToAdminDatabase( i->second->GetServer( ), i->second->GetUser( ) ) );
				else
					SendChat( Player, m_GHost->m_Language->ErrorAddingUserToAdminDatabase( i->second->GetServer( ), i->second->GetUser( ) ) );
			}

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedAdminAdds.erase( i );
		}
		else
			i++;
	}

	for( QVector<PairedAdminRemove> :: iterator i = m_PairedAdminRemoves.begin( ); i != m_PairedAdminRemoves.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			if( i->second->GetResult( ) )
			{
				for( QVector<CBNET *> :: iterator j = m_GHost->m_BNETs.begin( ); j != m_GHost->m_BNETs.end( ); j++ )
				{
					if( (*j)->GetServer( ) == i->second->GetServer( ) )
						(*j)->RemoveAdmin( i->second->GetUser( ) );
				}
			}

			CGamePlayer *Player = GetPlayerFromName( i->first, true );

			if( Player )
			{
				if( i->second->GetResult( ) )
					SendChat( Player, m_GHost->m_Language->DeletedUserFromAdminDatabase( i->second->GetServer( ), i->second->GetUser( ) ) );
				else
					SendChat( Player, m_GHost->m_Language->ErrorDeletingUserFromAdminDatabase( i->second->GetServer( ), i->second->GetUser( ) ) );
			}

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedAdminRemoves.erase( i );
		}
		else
			i++;
	}

	for( QVector<PairedBanCount> :: iterator i = m_PairedBanCounts.begin( ); i != m_PairedBanCounts.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			CGamePlayer *Player = GetPlayerFromName( i->first, true );

			if( Player )
			{
				uint32_t Count = i->second->GetResult( );

				if( Count == 0 )
					SendChat( Player, m_GHost->m_Language->ThereAreNoBannedUsers( i->second->GetServer( ) ) );
				else if( Count == 1 )
					SendChat( Player, m_GHost->m_Language->ThereIsBannedUser( i->second->GetServer( ) ) );
				else
					SendChat( Player, m_GHost->m_Language->ThereAreBannedUsers( i->second->GetServer( ), UTIL_ToString( Count ) ) );
			}

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedBanCounts.erase( i );
		}
		else
			i++;
	}

	/*

	for( QVector<PairedBanAdd> :: iterator i = m_PairedBanAdds.begin( ); i != m_PairedBanAdds.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			if( i->second->GetResult( ) )
			{
				for( QVector<CBNET *> :: iterator j = m_GHost->m_BNETs.begin( ); j != m_GHost->m_BNETs.end( ); j++ )
				{
					if( (*j)->GetServer( ) == i->second->GetServer( ) )
						(*j)->AddBan( i->second->GetUser( ), i->second->GetIP( ), i->second->GetGameName( ), i->second->GetAdmin( ), i->second->GetReason( ) );
				}
			}

			CGamePlayer *Player = GetPlayerFromName( i->first, true );

			if( Player )
			{
				if( i->second->GetResult( ) )
					SendChat( Player, m_GHost->m_Language->BannedUser( i->second->GetServer( ), i->second->GetUser( ) ) );
				else
					SendChat( Player, m_GHost->m_Language->ErrorBanningUser( i->second->GetServer( ), i->second->GetUser( ) ) );
			}

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedBanAdds.erase( i );
		}
		else
			i++;
	}

	*/

	for( QVector<PairedBanRemove> :: iterator i = m_PairedBanRemoves.begin( ); i != m_PairedBanRemoves.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			if( i->second->GetResult( ) )
			{
				for( QVector<CBNET *> :: iterator j = m_GHost->m_BNETs.begin( ); j != m_GHost->m_BNETs.end( ); j++ )
				{
					if( (*j)->GetServer( ) == i->second->GetServer( ) )
						(*j)->RemoveBan( i->second->GetUser( ) );
				}
			}

			CGamePlayer *Player = GetPlayerFromName( i->first, true );

			if( Player )
			{
				if( i->second->GetResult( ) )
					SendChat( Player, m_GHost->m_Language->UnbannedUser( i->second->GetUser( ) ) );
				else
					SendChat( Player, m_GHost->m_Language->ErrorUnbanningUser( i->second->GetUser( ) ) );
			}

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedBanRemoves.erase( i );
		}
		else
			i++;
	}

	// reset the last reserved seen timer since the admin game should never be considered abandoned

	m_LastReservedSeen = GetTime( );
	return CBaseGame :: Update( fd, send_fd );
}

void CAdminGame :: SendAdminChat( QString message )
{
	for( QVector<CGamePlayer *> :: iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
	{
		if( (*i)->GetLoggedIn( ) )
			SendChat( *i, message );
	}
}

void CAdminGame :: SendWelcomeMessage( CGamePlayer *player )
{
	SendChat( player, "GHost++ Admin Game                     http://www.codelain.com/" );
	SendChat( player, "-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-" );
	SendChat( player, "Commands: addadmin, autohost, autohostmm, checkadmin" );
	SendChat( player, "Commands: checkban, countadmins, countbans, deladmin" );
	SendChat( player, "Commands: delban, disable, downloads, enable, end, enforcesg" );
	SendChat( player, "Commands: exit, getgame, getgames, hostsg, load, loadsg" );
	SendChat( player, "Commands: map, password, priv, privby, pub, pubby, quit" );
	SendChat( player, "Commands: reload, say, saygame, saygames, unban, unhost, w" );
}

void CAdminGame :: EventPlayerJoined( CPotentialPlayer *potential, CIncomingJoinPlayer *joinPlayer )
{
	uint32_t Time = GetTime( );

	for( QVector<TempBan> :: iterator i = m_TempBans.begin( ); i != m_TempBans.end( ); )
	{
		// remove old tempbans (after 5 seconds)

		if( Time - (*i).second >= 5 )
			i = m_TempBans.erase( i );
		else
		{
			if( (*i).first == potential->GetExternalIPString( ) )
			{
				// tempbanned, goodbye

				potential->GetSocket( )->PutBytes( m_Protocol->SEND_W3GS_REJECTJOIN( REJECTJOIN_WRONGPASSWORD ) );
				potential->SetDeleteMe( true );
				CONSOLE_Print( "[ADMINGAME] player [" + joinPlayer->GetName( ) + "] at ip [" + (*i).first + "] is trying to join the game but is tempbanned" );
				return;
			}

			i++;
		}
	}

	CBaseGame :: EventPlayerJoined( potential, joinPlayer );
}

bool CAdminGame :: EventPlayerBotCommand( CGamePlayer *player, QString command, QString payload )
{
	CBaseGame :: EventPlayerBotCommand( player, command, payload );

	// todotodo: don't be lazy

	QString User = player->GetName( );
	QString Command = command;
	QString Payload = payload;

	if( player->GetLoggedIn( ) )
	{
		CONSOLE_Print( "[ADMINGAME] admin [" + User + "] sent command [" + Command + "] with payload [" + Payload + "]" );

		/*****************
		* ADMIN COMMANDS *
		******************/

		//
		// !ADDADMIN
		//

		if( Command == "addadmin" && !Payload.isEmpty( ) )
		{
			// extract the name and the server
			// e.g. "Varlock useast.battle.net" -> name: "Varlock", server: "useast.battle.net"

			QString Name;
			QString Server;
			stringstream SS;
			SS << Payload;
			SS >> Name;

			if( SS.eof( ) )
			{
				if( m_GHost->m_BNETs.size( ) == 1 )
					Server = m_GHost->m_BNETs[0]->GetServer( );
				else
					CONSOLE_Print( "[ADMINGAME] missing input #2 to addadmin command" );
			}
			else
				SS >> Server;

			if( !Server.isEmpty( ) )
			{
				QString Servers;
				bool FoundServer = false;

				for( QVector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
				{
					if( Servers.isEmpty( ) )
						Servers = (*i)->GetServer( );
					else
						Servers += ", " + (*i)->GetServer( );

					if( (*i)->GetServer( ) == Server )
					{
						FoundServer = true;

						if( (*i)->IsAdmin( Name ) )
							SendChat( player, m_GHost->m_Language->UserIsAlreadyAnAdmin( Server, Name ) );
						else
							m_PairedAdminAdds.push_back( PairedAdminAdd( player->GetName( ), m_GHost->m_DB->ThreadedAdminAdd( Server, Name ) ) );

						break;
					}
				}

				if( !FoundServer )
					SendChat( player, m_GHost->m_Language->ValidServers( Servers ) );
			}
		}

		//
		// !AUTOHOST
		//

		if( Command == "autohost" )
		{
			if( Payload.isEmpty( ) || Payload == "off" )
			{
				SendChat( player, m_GHost->m_Language->AutoHostDisabled( ) );
				m_GHost->m_AutoHostGameName.clear( );
				m_GHost->m_AutoHostOwner.clear( );
				m_GHost->m_AutoHostServer.clear( );
				m_GHost->m_AutoHostMaximumGames = 0;
				m_GHost->m_AutoHostAutoStartPlayers = 0;
				m_GHost->m_LastAutoHostTime = GetTime( );
				m_GHost->m_AutoHostMatchMaking = false;
				m_GHost->m_AutoHostMinimumScore = 0.0;
				m_GHost->m_AutoHostMaximumScore = 0.0;
			}
			else
			{
				// extract the maximum games, auto start players, and the game name
				// e.g. "5 10 BattleShips Pro" -> maximum games: "5", auto start players: "10", game name: "BattleShips Pro"

				uint32_t MaximumGames;
				uint32_t AutoStartPlayers;
				QString GameName;
				stringstream SS;
				SS << Payload;
				SS >> MaximumGames;

				if( SS.fail( ) || MaximumGames == 0 )
					CONSOLE_Print( "[ADMINGAME] bad input #1 to autohost command" );
				else
				{
					SS >> AutoStartPlayers;

					if( SS.fail( ) || AutoStartPlayers == 0 )
						CONSOLE_Print( "[ADMINGAME] bad input #2 to autohost command" );
					else
					{
						if( SS.eof( ) )
							CONSOLE_Print( "[ADMINGAME] missing input #3 to autohost command" );
						else
						{
							getline( SS, GameName );
							QString :: size_type Start = GameName.find_first_not_of( " " );

							if( Start != QString :: npos )
								GameName = GameName.mid( Start );

							SendChat( player, m_GHost->m_Language->AutoHostEnabled( ) );
							delete m_GHost->m_AutoHostMap;
							m_GHost->m_AutoHostMap = new CMap( *m_GHost->m_Map );
							m_GHost->m_AutoHostGameName = GameName;
							m_GHost->m_AutoHostOwner = User;
							m_GHost->m_AutoHostServer.clear( );
							m_GHost->m_AutoHostMaximumGames = MaximumGames;
							m_GHost->m_AutoHostAutoStartPlayers = AutoStartPlayers;
							m_GHost->m_LastAutoHostTime = GetTime( );
							m_GHost->m_AutoHostMatchMaking = false;
							m_GHost->m_AutoHostMinimumScore = 0.0;
							m_GHost->m_AutoHostMaximumScore = 0.0;
						}
					}
				}
			}
		}

		//
		// !AUTOHOSTMM
		//

		if( Command == "autohostmm" )
		{
			if( Payload.isEmpty( ) || Payload == "off" )
			{
				SendChat( player, m_GHost->m_Language->AutoHostDisabled( ) );
				m_GHost->m_AutoHostGameName.clear( );
				m_GHost->m_AutoHostOwner.clear( );
				m_GHost->m_AutoHostServer.clear( );
				m_GHost->m_AutoHostMaximumGames = 0;
				m_GHost->m_AutoHostAutoStartPlayers = 0;
				m_GHost->m_LastAutoHostTime = GetTime( );
				m_GHost->m_AutoHostMatchMaking = false;
				m_GHost->m_AutoHostMinimumScore = 0.0;
				m_GHost->m_AutoHostMaximumScore = 0.0;
			}
			else
			{
				// extract the maximum games, auto start players, and the game name
				// e.g. "5 10 800 1200 BattleShips Pro" -> maximum games: "5", auto start players: "10", minimum score: "800", maximum score: "1200", game name: "BattleShips Pro"

				uint32_t MaximumGames;
				uint32_t AutoStartPlayers;
				double MinimumScore;
				double MaximumScore;
				QString GameName;
				stringstream SS;
				SS << Payload;
				SS >> MaximumGames;

				if( SS.fail( ) || MaximumGames == 0 )
					CONSOLE_Print( "[ADMINGAME] bad input #1 to autohostmm command" );
				else
				{
					SS >> AutoStartPlayers;

					if( SS.fail( ) || AutoStartPlayers == 0 )
						CONSOLE_Print( "[ADMINGAME] bad input #2 to autohostmm command" );
					else
					{
						SS >> MinimumScore;

						if( SS.fail( ) )
							CONSOLE_Print( "[ADMINGAME] bad input #3 to autohostmm command" );
						else
						{
							SS >> MaximumScore;

							if( SS.fail( ) )
								CONSOLE_Print( "[ADMINGAME] bad input #4 to autohostmm command" );
							else
							{
								if( SS.eof( ) )
									CONSOLE_Print( "[ADMINGAME] missing input #5 to autohostmm command" );
								else
								{
									getline( SS, GameName );
									QString :: size_type Start = GameName.find_first_not_of( " " );

									if( Start != QString :: npos )
										GameName = GameName.mid( Start );

									SendChat( player, m_GHost->m_Language->AutoHostEnabled( ) );
									delete m_GHost->m_AutoHostMap;
									m_GHost->m_AutoHostMap = new CMap( *m_GHost->m_Map );
									m_GHost->m_AutoHostGameName = GameName;
									m_GHost->m_AutoHostOwner = User;
									m_GHost->m_AutoHostServer.clear( );
									m_GHost->m_AutoHostMaximumGames = MaximumGames;
									m_GHost->m_AutoHostAutoStartPlayers = AutoStartPlayers;
									m_GHost->m_LastAutoHostTime = GetTime( );
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

		//
		// !CHECKADMIN
		//

		if( Command == "checkadmin" && !Payload.isEmpty( ) )
		{
			// extract the name and the server
			// e.g. "Varlock useast.battle.net" -> name: "Varlock", server: "useast.battle.net"

			QString Name;
			QString Server;
			stringstream SS;
			SS << Payload;
			SS >> Name;

			if( SS.eof( ) )
			{
				if( m_GHost->m_BNETs.size( ) == 1 )
					Server = m_GHost->m_BNETs[0]->GetServer( );
				else
					CONSOLE_Print( "[ADMINGAME] missing input #2 to checkadmin command" );
			}
			else
				SS >> Server;

			if( !Server.isEmpty( ) )
			{
				QString Servers;
				bool FoundServer = false;

				for( QVector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
				{
					if( Servers.isEmpty( ) )
						Servers = (*i)->GetServer( );
					else
						Servers += ", " + (*i)->GetServer( );

					if( (*i)->GetServer( ) == Server )
					{
						FoundServer = true;

						if( (*i)->IsAdmin( Name ) )
							SendChat( player, m_GHost->m_Language->UserIsAnAdmin( Server, Name ) );
						else
							SendChat( player, m_GHost->m_Language->UserIsNotAnAdmin( Server, Name ) );

						break;
					}
				}

				if( !FoundServer )
					SendChat( player, m_GHost->m_Language->ValidServers( Servers ) );
			}
		}

		//
		// !CHECKBAN
		//

		if( Command == "checkban" && !Payload.isEmpty( ) )
		{
			// extract the name and the server
			// e.g. "Varlock useast.battle.net" -> name: "Varlock", server: "useast.battle.net"

			QString Name;
			QString Server;
			stringstream SS;
			SS << Payload;
			SS >> Name;

			if( SS.eof( ) )
			{
				if( m_GHost->m_BNETs.size( ) == 1 )
					Server = m_GHost->m_BNETs[0]->GetServer( );
				else
					CONSOLE_Print( "[ADMINGAME] missing input #2 to checkban command" );
			}
			else
				SS >> Server;

			if( !Server.isEmpty( ) )
			{
				QString Servers;
				bool FoundServer = false;

				for( QVector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
				{
					if( Servers.isEmpty( ) )
						Servers = (*i)->GetServer( );
					else
						Servers += ", " + (*i)->GetServer( );

					if( (*i)->GetServer( ) == Server )
					{
						FoundServer = true;
						CDBBan *Ban = (*i)->IsBannedName( Name );

						if( Ban )
							SendChat( player, m_GHost->m_Language->UserWasBannedOnByBecause( Server, Name, Ban->GetDate( ), Ban->GetAdmin( ), Ban->GetReason( ) ) );
						else
							SendChat( player, m_GHost->m_Language->UserIsNotBanned( Server, Name ) );

						break;
					}
				}

				if( !FoundServer )
					SendChat( player, m_GHost->m_Language->ValidServers( Servers ) );
			}
		}

		//
		// !COUNTADMINS
		//

		if( Command == "countadmins" )
		{
			QString Server = Payload;

			if( Server.isEmpty( ) && m_GHost->m_BNETs.size( ) == 1 )
				Server = m_GHost->m_BNETs[0]->GetServer( );

			if( !Server.isEmpty( ) )
				m_PairedAdminCounts.push_back( PairedAdminCount( player->GetName( ), m_GHost->m_DB->ThreadedAdminCount( Server ) ) );
		}

		//
		// !COUNTBANS
		//

		if( Command == "countbans" )
		{
			QString Server = Payload;

			if( Server.isEmpty( ) && m_GHost->m_BNETs.size( ) == 1 )
				Server = m_GHost->m_BNETs[0]->GetServer( );

			if( !Server.isEmpty( ) )
				m_PairedBanCounts.push_back( PairedBanCount( player->GetName( ), m_GHost->m_DB->ThreadedBanCount( Server ) ) );
		}

		//
		// !DELADMIN
		//

		if( Command == "deladmin" && !Payload.isEmpty( ) )
		{
			// extract the name and the server
			// e.g. "Varlock useast.battle.net" -> name: "Varlock", server: "useast.battle.net"

			QString Name;
			QString Server;
			stringstream SS;
			SS << Payload;
			SS >> Name;

			if( SS.eof( ) )
			{
				if( m_GHost->m_BNETs.size( ) == 1 )
					Server = m_GHost->m_BNETs[0]->GetServer( );
				else
					CONSOLE_Print( "[ADMINGAME] missing input #2 to deladmin command" );
			}
			else
				SS >> Server;

			if( !Server.isEmpty( ) )
			{
				QString Servers;
				bool FoundServer = false;

				for( QVector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
				{
					if( Servers.isEmpty( ) )
						Servers = (*i)->GetServer( );
					else
						Servers += ", " + (*i)->GetServer( );

					if( (*i)->GetServer( ) == Server )
					{
						FoundServer = true;

						if( !(*i)->IsAdmin( Name ) )
							SendChat( player, m_GHost->m_Language->UserIsNotAnAdmin( Server, Name ) );
						else
							m_PairedAdminRemoves.push_back( PairedAdminRemove( player->GetName( ), m_GHost->m_DB->ThreadedAdminRemove( Server, Name ) ) );

						break;
					}
				}

				if( !FoundServer )
					SendChat( player, m_GHost->m_Language->ValidServers( Servers ) );
			}
		}

		//
		// !DELBAN
		// !UNBAN
		//

		if( ( Command == "delban" || Command == "unban" ) && !Payload.isEmpty( ) )
			m_PairedBanRemoves.push_back( PairedBanRemove( player->GetName( ), m_GHost->m_DB->ThreadedBanRemove( Payload ) ) );

		//
		// !DISABLE
		//

		if( Command == "disable" )
		{
			SendChat( player, m_GHost->m_Language->BotDisabled( ) );
			m_GHost->m_Enabled = false;
		}

		//
		// !DOWNLOADS
		//

		if( Command == "downloads" && !Payload.isEmpty( ) )
		{
			uint32_t Downloads = UTIL_ToUInt32( Payload );

			if( Downloads == 0 )
			{
				SendChat( player, m_GHost->m_Language->MapDownloadsDisabled( ) );
				m_GHost->m_AllowDownloads = 0;
			}
			else if( Downloads == 1 )
			{
				SendChat( player, m_GHost->m_Language->MapDownloadsEnabled( ) );
				m_GHost->m_AllowDownloads = 1;
			}
			else if( Downloads == 2 )
			{
				SendChat( player, m_GHost->m_Language->MapDownloadsConditional( ) );
				m_GHost->m_AllowDownloads = 2;
			}
		}

		//
		// !ENABLE
		//

		if( Command == "enable" )
		{
			SendChat( player, m_GHost->m_Language->BotEnabled( ) );
			m_GHost->m_Enabled = true;
		}

		//
		// !END
		//

		if( Command == "end" && !Payload.isEmpty( ) )
		{
			// todotodo: what if a game ends just as you're typing this command and the numbering changes?

			uint32_t GameNumber = UTIL_ToUInt32( Payload ) - 1;

			if( GameNumber < m_GHost->m_Games.size( ) )
			{
				SendChat( player, m_GHost->m_Language->EndingGame( m_GHost->m_Games[GameNumber]->GetDescription( ) ) );
				CONSOLE_Print( "[GAME: " + m_GHost->m_Games[GameNumber]->GetGameName( ) + "] is over (admin ended game)" );
				m_GHost->m_Games[GameNumber]->StopPlayers( "was disconnected (admin ended game)" );
			}
			else
				SendChat( player, m_GHost->m_Language->GameNumberDoesntExist( Payload ) );
		}

		//
		// !ENFORCESG
		//

		if( Command == "enforcesg" && !Payload.isEmpty( ) )
		{
			// only load files in the current directory just to be safe

			if( Payload.find( "/" ) != QString :: npos || Payload.find( "\\" ) != QString :: npos )
				SendChat( player, m_GHost->m_Language->UnableToLoadReplaysOutside( ) );
			else
			{
				QString File = m_GHost->m_ReplayPath + Payload + ".w3g";

				if( UTIL_FileExists( File ) )
				{
					SendChat( player, m_GHost->m_Language->LoadingReplay( File ) );
					CReplay *Replay = new CReplay( );
					Replay->Load( File, false );
					Replay->ParseReplay( false );
					m_GHost->m_EnforcePlayers = Replay->GetPlayers( );
					delete Replay;
				}
				else
					SendChat( player, m_GHost->m_Language->UnableToLoadReplayDoesntExist( File ) );
			}
		}

		//
		// !EXIT
		// !QUIT
		//

		if( Command == "exit" || Command == "quit" )
		{
			if( Payload == "nice" )
				m_GHost->m_ExitingNice = true;
			else if( Payload == "force" )
				m_Exiting = true;
			else
			{
				if( m_GHost->m_CurrentGame || !m_GHost->m_Games.isEmpty( ) )
					SendChat( player, m_GHost->m_Language->AtLeastOneGameActiveUseForceToShutdown( ) );
				else
					m_Exiting = true;
			}
		}

		//
		// !GETGAME
		//

		if( Command == "getgame" && !Payload.isEmpty( ) )
		{
			uint32_t GameNumber = UTIL_ToUInt32( Payload ) - 1;

			if( GameNumber < m_GHost->m_Games.size( ) )
				SendChat( player, m_GHost->m_Language->GameNumberIs( Payload, m_GHost->m_Games[GameNumber]->GetDescription( ) ) );
			else
				SendChat( player, m_GHost->m_Language->GameNumberDoesntExist( Payload ) );
		}

		//
		// !GETGAMES
		//

		if( Command == "getgames" )
		{
			if( m_GHost->m_CurrentGame )
				SendChat( player, m_GHost->m_Language->GameIsInTheLobby( m_GHost->m_CurrentGame->GetDescription( ), UTIL_ToString( m_GHost->m_Games.size( ) ), UTIL_ToString( m_GHost->m_MaxGames ) ) );
			else
				SendChat( player, m_GHost->m_Language->ThereIsNoGameInTheLobby( UTIL_ToString( m_GHost->m_Games.size( ) ), UTIL_ToString( m_GHost->m_MaxGames ) ) );
		}

		//
		// !HOSTSG
		//

		if( Command == "hostsg" && !Payload.isEmpty( ) )
			m_GHost->CreateGame( m_GHost->m_Map, GAME_PRIVATE, true, Payload, User, User, QString( ), false );

		//
		// !LOAD (load config file)
		//

		if( Command == "load" )
		{
			if( Payload.isEmpty( ) )
				SendChat( player, m_GHost->m_Language->CurrentlyLoadedMapCFGIs( m_GHost->m_Map->GetCFGFile( ) ) );
			else
			{
				QString FoundMapConfigs;

				try
				{
					path MapCFGPath( m_GHost->m_MapCFGPath );
					QString Pattern = Payload;
					transform( Pattern.begin( ), Pattern.end( ), Pattern.begin( ), (int(*)(int))tolower );

					if( !exists( MapCFGPath ) )
					{
						CONSOLE_Print( "[ADMINGAME] error listing map configs - map config path doesn't exist" );
						SendChat( player, m_GHost->m_Language->ErrorListingMapConfigs( ) );
					}
					else
					{
						directory_iterator EndIterator;
						path LastMatch;
						uint32_t Matches = 0;

						for( directory_iterator i( MapCFGPath ); i != EndIterator; i++ )
						{
							QString FileName = i->filename( );
							QString Stem = i->path( ).stem( );
							transform( FileName.begin( ), FileName.end( ), FileName.begin( ), (int(*)(int))tolower );
							transform( Stem.begin( ), Stem.end( ), Stem.begin( ), (int(*)(int))tolower );

							if( !is_directory( i->status( ) ) && i->path( ).extension( ) == ".cfg" && FileName.find( Pattern ) != QString :: npos )
							{
								LastMatch = i->path( );
								Matches++;

								if( FoundMapConfigs.isEmpty( ) )
									FoundMapConfigs = i->filename( );
								else
									FoundMapConfigs += ", " + i->filename( );

								// if the pattern matches the filename exactly, with or without extension, stop any further matching

								if( FileName == Pattern || Stem == Pattern )
								{
									Matches = 1;
									break;
								}
							}
						}

						if( Matches == 0 )
							SendChat( player, m_GHost->m_Language->NoMapConfigsFound( ) );
						else if( Matches == 1 )
						{
							QString File = LastMatch.filename( );
							SendChat( player, m_GHost->m_Language->LoadingConfigFile( m_GHost->m_MapCFGPath + File ) );
							CConfig MapCFG;
							MapCFG.Read( LastMatch.QString( ) );
							m_GHost->m_Map->Load( &MapCFG, m_GHost->m_MapCFGPath + File );
						}
						else
							SendChat( player, m_GHost->m_Language->FoundMapConfigs( FoundMapConfigs ) );
					}
				}
				catch( const exception &ex )
				{
					CONSOLE_Print( QString( "[ADMINGAME] error listing map configs - caught exception [" ) + ex.what( ) + "]" );
					SendChat( player, m_GHost->m_Language->ErrorListingMapConfigs( ) );
				}
			}
		}

		//
		// !LOADSG
		//

		if( Command == "loadsg" && !Payload.isEmpty( ) )
		{
			// only load files in the current directory just to be safe

			if( Payload.find( "/" ) != QString :: npos || Payload.find( "\\" ) != QString :: npos )
				SendChat( player, m_GHost->m_Language->UnableToLoadSaveGamesOutside( ) );
			else
			{
				QString File = m_GHost->m_SaveGamePath + Payload + ".w3z";
				QString FileNoPath = Payload + ".w3z";

				if( UTIL_FileExists( File ) )
				{
					if( m_GHost->m_CurrentGame )
						SendChat( player, m_GHost->m_Language->UnableToLoadSaveGameGameInLobby( ) );
					else
					{
						SendChat( player, m_GHost->m_Language->LoadingSaveGame( File ) );
						m_GHost->m_SaveGame->Load( File, false );
						m_GHost->m_SaveGame->ParseSaveGame( );
						m_GHost->m_SaveGame->SetFileName( File );
						m_GHost->m_SaveGame->SetFileNameNoPath( FileNoPath );
					}
				}
				else
					SendChat( player, m_GHost->m_Language->UnableToLoadSaveGameDoesntExist( File ) );
			}
		}

		//
		// !MAP (load map file)
		//

		if( Command == "map" )
		{
			if( Payload.isEmpty( ) )
				SendChat( player, m_GHost->m_Language->CurrentlyLoadedMapCFGIs( m_GHost->m_Map->GetCFGFile( ) ) );
			else
			{
				QString FoundMaps;

				try
				{
					path MapPath( m_GHost->m_MapPath );
					QString Pattern = Payload;
					transform( Pattern.begin( ), Pattern.end( ), Pattern.begin( ), (int(*)(int))tolower );

					if( !exists( MapPath ) )
					{
						CONSOLE_Print( "[ADMINGAME] error listing maps - map path doesn't exist" );
						SendChat( player, m_GHost->m_Language->ErrorListingMaps( ) );
					}
					else
					{
						directory_iterator EndIterator;
						path LastMatch;
						uint32_t Matches = 0;

						for( directory_iterator i( MapPath ); i != EndIterator; i++ )
						{
							QString FileName = i->filename( );
							QString Stem = i->path( ).stem( );
							transform( FileName.begin( ), FileName.end( ), FileName.begin( ), (int(*)(int))tolower );
							transform( Stem.begin( ), Stem.end( ), Stem.begin( ), (int(*)(int))tolower );

							if( !is_directory( i->status( ) ) && FileName.find( Pattern ) != QString :: npos )
							{
								LastMatch = i->path( );
								Matches++;

								if( FoundMaps.isEmpty( ) )
									FoundMaps = i->filename( );
								else
									FoundMaps += ", " + i->filename( );

								// if the pattern matches the filename exactly, with or without extension, stop any further matching

								if( FileName == Pattern || Stem == Pattern )
								{
									Matches = 1;
									break;
								}
							}
						}

						if( Matches == 0 )
							SendChat( player, m_GHost->m_Language->NoMapsFound( ) );
						else if( Matches == 1 )
						{
							QString File = LastMatch.filename( );
							SendChat( player, m_GHost->m_Language->LoadingConfigFile( File ) );

							// hackhack: create a config file in memory with the required information to load the map

							CConfig MapCFG;
							MapCFG.Set( "map_path", "Maps\\Download\\" + File );
							MapCFG.Set( "map_localpath", File );
							m_GHost->m_Map->Load( &MapCFG, File );
						}
						else
							SendChat( player, m_GHost->m_Language->FoundMaps( FoundMaps ) );
					}
				}
				catch( const exception &ex )
				{
					CONSOLE_Print( QString( "[ADMINGAME] error listing maps - caught exception [" ) + ex.what( ) + "]" );
					SendChat( player, m_GHost->m_Language->ErrorListingMaps( ) );
				}
			}
		}

		//
		// !PRIV (host private game)
		//

		if( Command == "priv" && !Payload.isEmpty( ) )
			m_GHost->CreateGame( m_GHost->m_Map, GAME_PRIVATE, false, Payload, User, User, QString( ), false );

		//
		// !PRIVBY (host private game by other player)
		//

		if( Command == "privby" && !Payload.isEmpty( ) )
		{
			// extract the owner and the game name
			// e.g. "Varlock dota 6.54b arem ~~~" -> owner: "Varlock", game name: "dota 6.54b arem ~~~"

			QString Owner;
			QString GameName;
			QString :: size_type GameNameStart = Payload.find( " " );

			if( GameNameStart != QString :: npos )
			{
				Owner = Payload.mid( 0, GameNameStart );
				GameName = Payload.mid( GameNameStart + 1 );
				m_GHost->CreateGame( m_GHost->m_Map, GAME_PRIVATE, false, GameName, Owner, User, QString( ), false );
			}
		}

		//
		// !PUB (host public game)
		//

		if( Command == "pub" && !Payload.isEmpty( ) )
			m_GHost->CreateGame( m_GHost->m_Map, GAME_PUBLIC, false, Payload, User, User, QString( ), false );

		//
		// !PUBBY (host public game by other player)
		//

		if( Command == "pubby" && !Payload.isEmpty( ) )
		{
			// extract the owner and the game name
			// e.g. "Varlock dota 6.54b arem ~~~" -> owner: "Varlock", game name: "dota 6.54b arem ~~~"

			QString Owner;
			QString GameName;
			QString :: size_type GameNameStart = Payload.find( " " );

			if( GameNameStart != QString :: npos )
			{
				Owner = Payload.mid( 0, GameNameStart );
				GameName = Payload.mid( GameNameStart + 1 );
				m_GHost->CreateGame( m_GHost->m_Map, GAME_PUBLIC, false, GameName, Owner, User, QString( ), false );
			}
		}

		//
		// !RELOAD
		//

		if( Command == "reload" )
		{
			SendChat( player, m_GHost->m_Language->ReloadingConfigurationFiles( ) );
			m_GHost->ReloadConfigs( );
		}

		//
		// !SAY
		//

		if( Command == "say" && !Payload.isEmpty( ) )
		{
			for( QVector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
				(*i)->QueueChatCommand( Payload );
		}

		//
		// !SAYGAME
		//

		if( Command == "saygame" && !Payload.isEmpty( ) )
		{
			// extract the game number and the message
			// e.g. "3 hello everyone" -> game number: "3", message: "hello everyone"

			uint32_t GameNumber;
			QString Message;
			stringstream SS;
			SS << Payload;
			SS >> GameNumber;

			if( SS.fail( ) )
				CONSOLE_Print( "[ADMINGAME] bad input #1 to saygame command" );
			else
			{
				if( SS.eof( ) )
					CONSOLE_Print( "[ADMINGAME] missing input #2 to saygame command" );
				else
				{
					getline( SS, Message );
					QString :: size_type Start = Message.find_first_not_of( " " );

					if( Start != QString :: npos )
						Message = Message.mid( Start );

					if( GameNumber - 1 < m_GHost->m_Games.size( ) )
						m_GHost->m_Games[GameNumber - 1]->SendAllChat( "ADMIN: " + Message );
					else
						SendChat( player, m_GHost->m_Language->GameNumberDoesntExist( UTIL_ToString( GameNumber ) ) );
				}
			}
		}

		//
		// !SAYGAMES
		//

		if( Command == "saygames" && !Payload.isEmpty( ) )
		{
			if( m_GHost->m_CurrentGame )
				m_GHost->m_CurrentGame->SendAllChat( Payload );

			for( QVector<CBaseGame *> :: iterator i = m_GHost->m_Games.begin( ); i != m_GHost->m_Games.end( ); i++ )
				(*i)->SendAllChat( "ADMIN: " + Payload );
		}

		//
		// !UNHOST
		//

		if( Command == "unhost" )
		{
			if( m_GHost->m_CurrentGame )
			{
				if( m_GHost->m_CurrentGame->GetCountDownStarted( ) )
					SendChat( player, m_GHost->m_Language->UnableToUnhostGameCountdownStarted( m_GHost->m_CurrentGame->GetDescription( ) ) );
				else
				{
					SendChat( player, m_GHost->m_Language->UnhostingGame( m_GHost->m_CurrentGame->GetDescription( ) ) );
					m_GHost->m_CurrentGame->SetExiting( true );
				}
			}
			else
				SendChat( player, m_GHost->m_Language->UnableToUnhostGameNoGameInLobby( ) );
		}

		//
		// !W
		//

		if( Command == "w" && !Payload.isEmpty( ) )
		{
			// extract the name and the message
			// e.g. "Varlock hello there!" -> name: "Varlock", message: "hello there!"

			QString Name;
			QString Message;
			QString :: size_type MessageStart = Payload.find( " " );

			if( MessageStart != QString :: npos )
			{
				Name = Payload.mid( 0, MessageStart );
				Message = Payload.mid( MessageStart + 1 );

				for( QVector<CBNET *> :: iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
					(*i)->QueueChatCommand( Message, Name, true );
			}
		}
	}
	else
		CONSOLE_Print( "[ADMINGAME] user [" + User + "] sent command [" + Command + "] with payload [" + Payload + "]" );

	/*********************
	* NON ADMIN COMMANDS *
	*********************/

	//
	// !PASSWORD
	//

	if( Command == "password" && !player->GetLoggedIn( ) )
	{
		if( !m_Password.isEmpty( ) && Payload == m_Password )
		{
			CONSOLE_Print( "[ADMINGAME] user [" + User + "] logged in" );
			SendChat( player, m_GHost->m_Language->AdminLoggedIn( ) );
			player->SetLoggedIn( true );
		}
		else
		{
			uint32_t LoginAttempts = player->GetLoginAttempts( ) + 1;
			player->SetLoginAttempts( LoginAttempts );
			CONSOLE_Print( "[ADMINGAME] user [" + User + "] login attempt failed" );
			SendChat( player, m_GHost->m_Language->AdminInvalidPassword( UTIL_ToString( LoginAttempts ) ) );

			if( LoginAttempts >= 1 )
			{
				player->SetDeleteMe( true );
				player->SetLeftReason( "was kicked for too many failed login attempts" );
				player->SetLeftCode( PLAYERLEAVE_LOBBY );
				OpenSlot( GetSIDFromPID( player->GetPID( ) ), false );

				// tempban for 5 seconds to prevent bruteforcing

				m_TempBans.push_back( TempBan( player->GetExternalIPString( ), GetTime( ) ) );
			}
		}
	}

	// always hide chat commands from other players in the admin game
	// note: this is actually redundant because we've already set m_MuteLobby = true so this has no effect
	// if you actually wanted to relay chat commands you would have to set m_MuteLobby = false AND return false here

	return true;
}
