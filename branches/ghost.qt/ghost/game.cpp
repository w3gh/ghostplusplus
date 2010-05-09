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
#include "ghostdb.h"
#include "bnet.h"
#include "map.h"
#include "packed.h"
#include "savegame.h"
#include "gameplayer.h"
#include "gameprotocol.h"
#include "game_base.h"
#include "game.h"
#include "stats.h"
#include "statsdota.h"
#include "statsw3mmd.h"

#include <cmath>
#include <QString>
#include <QtAlgorithms>
#include <time.h>

#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QStringList>
#include <QUdpSocket>

//
// sorting classes
//

class CGamePlayerSortAscByPing
{
public:
	bool operator( ) ( CGamePlayer *Player1, CGamePlayer *Player2 ) const
	{
		return Player1->GetPing( false ) < Player2->GetPing( false );
	}
};

class CGamePlayerSortDescByPing
{
public:
	bool operator( ) ( CGamePlayer *Player1, CGamePlayer *Player2 ) const
	{
		return Player1->GetPing( false ) > Player2->GetPing( false );
	}
};

//
// CGame
//

CGame :: CGame( CGHost *nGHost, CMap *nMap, CSaveGame *nSaveGame, quint16 nHostPort, unsigned char nGameState, QString nGameName, QString nOwnerName, QString nCreatorName, QString nCreatorServer ) : CBaseGame( nGHost, nMap, nSaveGame, nHostPort, nGameState, nGameName, nOwnerName, nCreatorName, nCreatorServer )
{
	m_DBBanLast = NULL;
	m_DBGame = new CDBGame( 0, QString( ), m_Map->GetMapPath( ), QString( ), QString( ), QString( ), 0 );

	if( m_Map->GetMapType( ) == "w3mmd" )
		m_Stats = new CStatsW3MMD( this, m_Map->GetMapStatsW3MMDCategory( ) );
	else if( m_Map->GetMapType( ) == "dota" )
		m_Stats = new CStatsDOTA( this );
	else
		m_Stats = NULL;

	m_CallableGameAdd = NULL;
}

void CGame::EventGameDataSaved()
{
	if( m_CallableGameAdd->GetResult( ) > 0 )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] saving player/stats data to database" );

		// store the CDBGamePlayers in the database

		for( QList<CDBGamePlayer *> :: const_iterator i = m_DBGamePlayers.begin( ); i != m_DBGamePlayers.end( ); i++ )
			m_GHost->m_Callables.push_back( m_GHost->m_DB->ThreadedGamePlayerAdd( m_CallableGameAdd->GetResult( ), (*i)->GetName( ), (*i)->GetIP( ), (*i)->GetSpoofed( ), (*i)->GetSpoofedRealm( ), (*i)->GetReserved( ), (*i)->GetLoadingTime( ), (*i)->GetLeft( ), (*i)->GetLeftReason( ), (*i)->GetTeam( ), (*i)->GetColour( ) ) );

		// store the stats in the database

		if( m_Stats )
			m_Stats->Save( m_GHost, m_GHost->m_DB, m_CallableGameAdd->GetResult( ) );
	}
	else
		CONSOLE_Print( "[GAME: " + m_GameName + "] unable to save player/stats data to database" );

	m_GHost->m_DB->RecoverCallable( m_CallableGameAdd );
	delete m_CallableGameAdd;
	m_CallableGameAdd = NULL;
	deleteLater();
}

CGame :: ~CGame( )
{
	if (m_CallableGameAdd != NULL)
	{
		delete m_CallableGameAdd;
		m_CallableGameAdd = NULL;
	}

	for( QList<PairedBanCheck> :: const_iterator i = m_PairedBanChecks.begin( ); i != m_PairedBanChecks.end( ); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for( QList<PairedBanAdd> :: const_iterator i = m_PairedBanAdds.begin( ); i != m_PairedBanAdds.end( ); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for( QList<PairedGPSCheck> :: const_iterator i = m_PairedGPSChecks.begin( ); i != m_PairedGPSChecks.end( ); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for( QList<PairedDPSCheck> :: const_iterator i = m_PairedDPSChecks.begin( ); i != m_PairedDPSChecks.end( ); i++ )
		m_GHost->m_Callables.push_back( i->second );

	for( QList<CDBBan *> :: const_iterator i = m_DBBans.begin( ); i != m_DBBans.end( ); i++ )
		delete *i;

	delete m_DBGame;

	for( QList<CDBGamePlayer *> :: const_iterator i = m_DBGamePlayers.begin( ); i != m_DBGamePlayers.end( ); i++ )
		delete *i;

	delete m_Stats;

	// it's a "bad thing" if m_CallableGameAdd is non NULL here
	// it means the game is being deleted after m_CallableGameAdd was created (the first step to saving the game data) but before the associated thread terminated
	// rather than failing horribly we choose to allow the thread to complete in the orphaned callables list but step 2 will never be completed
	// so this will create a game entry in the database without any gameplayers and/or DotA stats

	if( m_CallableGameAdd )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] game is being deleted before all game data was saved, game data has been lost" );
		m_GHost->m_Callables.push_back( m_CallableGameAdd );
	}
}

void CGame::EventCallableUpdateTimeout()
{
	// update callables

	for( QList<PairedBanCheck> :: iterator i = m_PairedBanChecks.begin( ); i != m_PairedBanChecks.end( ); )
	{
		if( i->second->GetReady( ) )
		{
			CDBBan *Ban = i->second->GetResult( );

			if( Ban )
				SendAllChat( m_GHost->m_Language->UserWasBannedOnByBecause( i->second->GetServer( ), i->second->GetUser( ), Ban->GetDate( ), Ban->GetAdmin( ), Ban->GetReason( ) ) );
			else
				SendAllChat( m_GHost->m_Language->UserIsNotBanned( i->second->GetServer( ), i->second->GetUser( ) ) );

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedBanChecks.erase( i );
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
				for( QList<CBNET *> :: iterator j = m_GHost->m_BNETs.begin( ); j != m_GHost->m_BNETs.end( ); j++ )
				{
					if( (*j)->GetServer( ) == i->second->GetServer( ) )
						(*j)->AddBan( i->second->GetUser( ), i->second->GetIP( ), i->second->GetGameName( ), i->second->GetAdmin( ), i->second->GetReason( ) );
				}

				SendAllChat( m_GHost->m_Language->PlayerWasBannedByPlayer( i->second->GetServer( ), i->second->GetUser( ), i->first ) );
			}

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedBanAdds.erase( i );
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
			{
				if( i->first.isEmpty( ) )
					SendAllChat( m_GHost->m_Language->HasPlayedGamesWithThisBot( i->second->GetName( ), GamePlayerSummary->GetFirstGameDateTime( ), GamePlayerSummary->GetLastGameDateTime( ), QString::number( GamePlayerSummary->GetTotalGames( ) ), QString::number( (float)GamePlayerSummary->GetAvgLoadingTime( ) / 1000, 'g', 2 ), QString::number( GamePlayerSummary->GetAvgLeftPercent( ) ) ) );
				else
				{
					CGamePlayer *Player = GetPlayerFromName( i->first, true );

					if( Player )
						SendChat( Player, m_GHost->m_Language->HasPlayedGamesWithThisBot( i->second->GetName( ), GamePlayerSummary->GetFirstGameDateTime( ), GamePlayerSummary->GetLastGameDateTime( ), QString::number( GamePlayerSummary->GetTotalGames( ) ), QString::number( (float)GamePlayerSummary->GetAvgLoadingTime( ) / 1000, 'g', 2 ), QString::number( GamePlayerSummary->GetAvgLeftPercent( ) ) ) );
				}
			}
			else
			{
				if( i->first.isEmpty( ) )
					SendAllChat( m_GHost->m_Language->HasntPlayedGamesWithThisBot( i->second->GetName( ) ) );
				else
				{
					CGamePlayer *Player = GetPlayerFromName( i->first, true );

					if( Player )
						SendChat( Player, m_GHost->m_Language->HasntPlayedGamesWithThisBot( i->second->GetName( ) ) );
				}
			}

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

				if( i->first.isEmpty( ) )
					SendAllChat( Summary );
				else
				{
					CGamePlayer *Player = GetPlayerFromName( i->first, true );

					if( Player )
						SendChat( Player, Summary );
				}
			}
			else
			{
				if( i->first.isEmpty( ) )
					SendAllChat( m_GHost->m_Language->HasntPlayedDotAGamesWithThisBot( i->second->GetName( ) ) );
				else
				{
					CGamePlayer *Player = GetPlayerFromName( i->first, true );

					if( Player )
						SendChat( Player, m_GHost->m_Language->HasntPlayedDotAGamesWithThisBot( i->second->GetName( ) ) );
				}
			}

			m_GHost->m_DB->RecoverCallable( i->second );
			delete i->second;
			i = m_PairedDPSChecks.erase( i );
		}
		else
			i++;
	}

	return CBaseGame::EventCallableUpdateTimeout();
}

void CGame :: EventPlayerDeleted()
{
	CGamePlayer *player = (CGamePlayer*)QObject::sender();
	CBaseGame :: EventPlayerDeleted();

	// record everything we need to know about the player for storing in the database later
	// since we haven't stored the game yet (it's not over yet!) we can't link the gameplayer to the game
	// see the destructor for where these CDBGamePlayers are stored in the database
	// we could have inserted an incomplete record on creation and updated it later but this makes for a cleaner interface

	if( m_GameLoading || m_GameLoaded )
	{
		// todotodo: since we store players that crash during loading it's possible that the stats classes could have no information on them
		// that could result in a DBGamePlayer without a corresponding DBDotAPlayer - just be aware of the possibility

		unsigned char SID = GetSIDFromPID( player->GetPID( ) );
		unsigned char Team = 255;
		unsigned char Colour = 255;

		if( SID < m_Slots.size( ) )
		{
			Team = m_Slots[SID].GetTeam( );
			Colour = m_Slots[SID].GetColour( );
		}

		m_DBGamePlayers.push_back( new CDBGamePlayer( 0, 0, player->GetName( ), player->GetExternalIPString( ), player->GetSpoofed( ) ? 1 : 0, player->GetSpoofedRealm( ), player->GetReserved( ) ? 1 : 0, player->GetFinishedLoading( ) ? player->GetFinishedLoadingTicks( ) - m_StartedLoadingTicks : 0, m_GameTicks / 1000, player->GetLeftReason( ), Team, Colour ) );

		// also keep track of the last player to leave for the !banlast command

		for( QList<CDBBan *> :: const_iterator i = m_DBBans.begin( ); i != m_DBBans.end( ); i++ )
		{
			if( (*i)->GetName( ) == player->GetName( ) )
				m_DBBanLast = *i;
		}
	}
}

void CGame :: EventPlayerAction( CGamePlayer *player, CIncomingAction *action )
{
	CBaseGame :: EventPlayerAction( player, action );

	// give the stats class a chance to process the action

	if( m_Stats && m_Stats->ProcessAction( action ) && !m_GameOverTimer.isActive() )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] gameover timer started (stats class reported game over)" );
		SendEndMessage( );
		m_GameOverTimer.start();
	}
}

bool CGame :: EventPlayerBotCommand( CGamePlayer *player, QString command, QString payload )
{
	bool HideCommand = CBaseGame :: EventPlayerBotCommand( player, command, payload );

	// todotodo: don't be lazy

	QString User = player->GetName( );
	QString Command = command;
	QString Payload = payload;

	bool AdminCheck = false;

	for( QList<CBNET *> :: const_iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
	{
		if( (*i)->GetServer( ) == player->GetSpoofedRealm( ) && (*i)->IsAdmin( User ) )
		{
			AdminCheck = true;
			break;
		}
	}

	bool RootAdminCheck = false;

	for( QList<CBNET *> :: const_iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
	{
		if( (*i)->GetServer( ) == player->GetSpoofedRealm( ) && (*i)->IsRootAdmin( User ) )
		{
			RootAdminCheck = true;
			break;
		}
	}

	if( player->GetSpoofed( ) && ( AdminCheck || RootAdminCheck || IsOwner( User ) ) )
	{
		CONSOLE_Print( "[GAME: " + m_GameName + "] admin [" + User + "] sent command [" + Command + "] with payload [" + Payload + "]" );

		if( !m_Locked || RootAdminCheck || IsOwner( User ) )
		{
			/*****************
			* ADMIN COMMANDS *
			******************/

			//
			// !ABORT (abort countdown)
			// !A
			//

			// we use "!a" as an alias for abort because you don't have much time to abort the countdown so it's useful for the abort command to be easy to type

			if( ( Command == "abort" || Command == "a" ) && m_CountDownStarted && !m_GameLoading && !m_GameLoaded )
			{
				SendAllChat( m_GHost->m_Language->CountDownAborted( ) );
				m_CountdownTimer.stop();
				m_CountDownStarted = false;
			}

			//
			// !ADDBAN
			// !BAN
			//

			if( ( Command == "addban" || Command == "ban" ) && !Payload.isEmpty( ) && !m_GHost->m_BNETs.isEmpty( ) )
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

				if( m_GameLoaded )
				{
					QString VictimLower = Victim.toLower();
					quint32 Matches = 0;
					CDBBan *LastMatch = NULL;

					// try to match each player with the passed QString (e.g. "Varlock" would be matched with "lock")
					// we use the m_DBBans vector for this in case the player already left and thus isn't in the m_Players vector anymore

					for( QList<CDBBan *> :: const_iterator i = m_DBBans.begin( ); i != m_DBBans.end( ); i++ )
					{
						QString TestName = (*i)->GetName( ).toLower();

						if( TestName.indexOf( VictimLower ) != -1 )
						{
							Matches++;
							LastMatch = *i;

							// if the name matches exactly stop any further matching

							if( TestName == VictimLower )
							{
								Matches = 1;
								break;
							}
						}
					}

					if( Matches == 0 )
						SendAllChat( m_GHost->m_Language->UnableToBanNoMatchesFound( Victim ) );
					else if( Matches == 1 )
						m_PairedBanAdds.push_back( PairedBanAdd( User, m_GHost->m_DB->ThreadedBanAdd( LastMatch->GetServer( ), LastMatch->GetName( ), LastMatch->GetIP( ), m_GameName, User, Reason ) ) );
					else
						SendAllChat( m_GHost->m_Language->UnableToBanFoundMoreThanOneMatch( Victim ) );
				}
				else
				{
					CGamePlayer *LastMatch = NULL;
					quint32 Matches = GetPlayerFromNamePartial( Victim, &LastMatch );

					if( Matches == 0 )
						SendAllChat( m_GHost->m_Language->UnableToBanNoMatchesFound( Victim ) );
					else if( Matches == 1 )
						m_PairedBanAdds.push_back( PairedBanAdd( User, m_GHost->m_DB->ThreadedBanAdd( LastMatch->GetJoinedRealm( ), LastMatch->GetName( ), LastMatch->GetExternalIPString( ), m_GameName, User, Reason ) ) );
					else
						SendAllChat( m_GHost->m_Language->UnableToBanFoundMoreThanOneMatch( Victim ) );
				}
			}

			//
			// !ANNOUNCE
			//

			if( Command == "announce" && !m_CountDownStarted )
			{
				if( Payload.isEmpty( ) || Payload == "off" )
				{
					SendAllChat( m_GHost->m_Language->AnnounceMessageDisabled( ) );
					SetAnnounce( 0, QString( ) );
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
						CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #1 to announce command" );
					else
					{
						if( SS.atEnd( ) )
							CONSOLE_Print( "[GAME: " + m_GameName + "] missing input #2 to announce command" );
						else
						{
							Message = SS.readLine();
							int Start = Message.indexOf( QRegExp( "[^ ]" ));

							if( Start != -1 )
								Message = Message.mid( Start );

							SendAllChat( m_GHost->m_Language->AnnounceMessageEnabled( ) );
							SetAnnounce( Interval, Message );
						}
					}
				}
			}

			//
			// !AUTOSAVE
			//

			if( Command == "autosave" )
			{
				if( Payload == "on" )
				{
					SendAllChat( m_GHost->m_Language->AutoSaveEnabled( ) );
					m_AutoSave = true;
				}
				else if( Payload == "off" )
				{
					SendAllChat( m_GHost->m_Language->AutoSaveDisabled( ) );
					m_AutoSave = false;
				}
			}

			//
			// !AUTOSTART
			//

			if( Command == "autostart" && !m_CountDownStarted )
			{
				if( Payload.isEmpty( ) || Payload == "off" )
				{
					SendAllChat( m_GHost->m_Language->AutoStartDisabled( ) );
					m_AutoStartPlayers = 0;
				}
				else
				{
					quint32 AutoStartPlayers = Payload.toUInt();

					if( AutoStartPlayers != 0 )
					{
						SendAllChat( m_GHost->m_Language->AutoStartEnabled( QString::number( AutoStartPlayers ) ) );
						m_AutoStartPlayers = AutoStartPlayers;
					}
				}
			}

			//
			// !BANLAST
			//

			if( Command == "banlast" && m_GameLoaded && !m_GHost->m_BNETs.isEmpty( ) && m_DBBanLast )
				m_PairedBanAdds.push_back( PairedBanAdd( User, m_GHost->m_DB->ThreadedBanAdd( m_DBBanLast->GetServer( ), m_DBBanLast->GetName( ), m_DBBanLast->GetIP( ), m_GameName, User, Payload ) ) );

			//
			// !CHECK
			//

			if( Command == "check" )
			{
				if( !Payload.isEmpty( ) )
				{
					CGamePlayer *LastMatch = NULL;
					quint32 Matches = GetPlayerFromNamePartial( Payload, &LastMatch );

					if( Matches == 0 )
						SendAllChat( m_GHost->m_Language->UnableToCheckPlayerNoMatchesFound( Payload ) );
					else if( Matches == 1 )
					{
						bool LastMatchAdminCheck = false;

						for( QList<CBNET *> :: const_iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
						{
							if( (*i)->GetServer( ) == LastMatch->GetSpoofedRealm( ) && (*i)->IsAdmin( LastMatch->GetName( ) ) )
							{
								LastMatchAdminCheck = true;
								break;
							}
						}

						bool LastMatchRootAdminCheck = false;

						for( QList<CBNET *> :: const_iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
						{
							if( (*i)->GetServer( ) == LastMatch->GetSpoofedRealm( ) && (*i)->IsRootAdmin( LastMatch->GetName( ) ) )
							{
								LastMatchRootAdminCheck = true;
								break;
							}
						}

						SendAllChat( m_GHost->m_Language->CheckedPlayer( LastMatch->GetName( ), LastMatch->GetNumPings( ) > 0 ? QString::number( LastMatch->GetPing( m_GHost->m_LCPings ) ) + "ms" : "N/A", m_GHost->m_DBLocal->FromCheck( Util::extractUInt32( Util::reverse(LastMatch->GetExternalIP( )) ) ), LastMatchAdminCheck || LastMatchRootAdminCheck ? "Yes" : "No", IsOwner( LastMatch->GetName( ) ) ? "Yes" : "No", LastMatch->GetSpoofed( ) ? "Yes" : "No", LastMatch->GetSpoofedRealm( ).isEmpty( ) ? "N/A" : LastMatch->GetSpoofedRealm( ), LastMatch->GetReserved( ) ? "Yes" : "No" ) );
					}
					else
						SendAllChat( m_GHost->m_Language->UnableToCheckPlayerFoundMoreThanOneMatch( Payload ) );
				}
				else
					SendAllChat( m_GHost->m_Language->CheckedPlayer( User, player->GetNumPings( ) > 0 ? QString::number( player->GetPing( m_GHost->m_LCPings ) ) + "ms" : "N/A", m_GHost->m_DBLocal->FromCheck( Util::extractUInt32(Util::reverse(player->GetExternalIP( ))) ), AdminCheck || RootAdminCheck ? "Yes" : "No", IsOwner( User ) ? "Yes" : "No", player->GetSpoofed( ) ? "Yes" : "No", player->GetSpoofedRealm( ).isEmpty( ) ? "N/A" : player->GetSpoofedRealm( ), player->GetReserved( ) ? "Yes" : "No" ) );
			}

			//
			// !CHECKBAN
			//

			if( Command == "checkban" && !Payload.isEmpty( ) && !m_GHost->m_BNETs.isEmpty( ) )
			{
				for( QList<CBNET *> :: const_iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
					m_PairedBanChecks.push_back( PairedBanCheck( User, m_GHost->m_DB->ThreadedBanCheck( (*i)->GetServer( ), Payload, QString( ) ) ) );
			}

			//
			// !CLEARHCL
			//

			if( Command == "clearhcl" && !m_CountDownStarted )
			{
				m_HCLCommandString.clear( );
				SendAllChat( m_GHost->m_Language->ClearingHCL( ) );
			}

			//
			// !CLOSE (close slot)
			//

			if( Command == "close" && !Payload.isEmpty( ) && !m_GameLoading && !m_GameLoaded )
			{
				// close as many slots as specified, e.g. "5 10" closes slots 5 and 10

				QTextStream SS(&Payload);


				while( !SS.atEnd( ) )
				{
					quint32 SID;
					SS >> SID;

					if( SS.status() != QTextStream::Ok )
					{
						CONSOLE_Print( "[GAME: " + m_GameName + "] bad input to close command" );
						break;
					}
					else
						CloseSlot( (unsigned char)( SID - 1 ), true );
				}
			}

			//
			// !CLOSEALL
			//

			if( Command == "closeall" && !m_GameLoading && !m_GameLoaded )
				CloseAllSlots( );

			//
			// !COMP (computer slot)
			//

			if( Command == "comp" && !Payload.isEmpty( ) && !m_GameLoading && !m_GameLoaded && !m_SaveGame )
			{
				// extract the slot and the skill
				// e.g. "1 2" -> slot: "1", skill: "2"

				quint32 Slot;
				quint32 Skill = 1;
				QTextStream SS(&Payload);

				SS >> Slot;

				if( SS.status() != QTextStream::Ok )
					CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #1 to comp command" );
				else
				{
					if( !SS.atEnd( ) )
						SS >> Skill;

					if( SS.status() != QTextStream::Ok )
						CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #2 to comp command" );
					else
						ComputerSlot( (unsigned char)( Slot - 1 ), (unsigned char)Skill, true );
				}
			}

			//
			// !COMPCOLOUR (computer colour change)
			//

			if( Command == "compcolour" && !Payload.isEmpty( ) && !m_GameLoading && !m_GameLoaded && !m_SaveGame )
			{
				// extract the slot and the colour
				// e.g. "1 2" -> slot: "1", colour: "2"

				quint32 Slot;
				quint32 Colour;
				QTextStream SS(&Payload);

				SS >> Slot;

				if( SS.status() != QTextStream::Ok )
					CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #1 to compcolour command" );
				else
				{
					if( SS.atEnd( ) )
						CONSOLE_Print( "[GAME: " + m_GameName + "] missing input #2 to compcolour command" );
					else
					{
						SS >> Colour;

						if( SS.status() != QTextStream::Ok )
							CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #2 to compcolour command" );
						else
						{
							unsigned char SID = (unsigned char)( Slot - 1 );

							if( !( m_Map->GetMapOptions( ) & MAPOPT_FIXEDPLAYERSETTINGS ) && Colour < 12 && SID < m_Slots.size( ) )
							{
								if( m_Slots[SID].GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && m_Slots[SID].GetComputer( ) == 1 )
									ColourSlot( SID, Colour );
							}
						}
					}
				}
			}

			//
			// !COMPHANDICAP (computer handicap change)
			//

			if( Command == "comphandicap" && !Payload.isEmpty( ) && !m_GameLoading && !m_GameLoaded && !m_SaveGame )
			{
				// extract the slot and the handicap
				// e.g. "1 50" -> slot: "1", handicap: "50"

				quint32 Slot;
				quint32 Handicap;
				QTextStream SS(&Payload);

				SS >> Slot;

				if( SS.status() != QTextStream::Ok )
					CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #1 to comphandicap command" );
				else
				{
					if( SS.atEnd( ) )
						CONSOLE_Print( "[GAME: " + m_GameName + "] missing input #2 to comphandicap command" );
					else
					{
						SS >> Handicap;

						if( SS.status() != QTextStream::Ok )
							CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #2 to comphandicap command" );
						else
						{
							unsigned char SID = (unsigned char)( Slot - 1 );

							if( !( m_Map->GetMapOptions( ) & MAPOPT_FIXEDPLAYERSETTINGS ) && ( Handicap == 50 || Handicap == 60 || Handicap == 70 || Handicap == 80 || Handicap == 90 || Handicap == 100 ) && SID < m_Slots.size( ) )
							{
								if( m_Slots[SID].GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && m_Slots[SID].GetComputer( ) == 1 )
								{
									m_Slots[SID].SetHandicap( (unsigned char)Handicap );
									SendAllSlotInfo( );
								}
							}
						}
					}
				}
			}

			//
			// !COMPRACE (computer race change)
			//

			if( Command == "comprace" && !Payload.isEmpty( ) && !m_GameLoading && !m_GameLoaded && !m_SaveGame )
			{
				// extract the slot and the race
				// e.g. "1 human" -> slot: "1", race: "human"

				quint32 Slot;
				QString Race;
				QTextStream SS(&Payload);

				SS >> Slot;

				if( SS.status() != QTextStream::Ok )
					CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #1 to comprace command" );
				else
				{
					if( SS.atEnd( ) )
						CONSOLE_Print( "[GAME: " + m_GameName + "] missing input #2 to comprace command" );
					else
					{
						Race = SS.readLine().toLower();
						int Start = Race.indexOf( QRegExp( "[^ ]" ));

						if( Start != -1 )
							Race = Race.mid( Start );

						unsigned char SID = (unsigned char)( Slot - 1 );

						if( !( m_Map->GetMapOptions( ) & MAPOPT_FIXEDPLAYERSETTINGS ) && !( m_Map->GetMapFlags( ) & MAPFLAG_RANDOMRACES ) && SID < m_Slots.size( ) )
						{
							if( m_Slots[SID].GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && m_Slots[SID].GetComputer( ) == 1 )
							{
								if( Race == "human" )
								{
									m_Slots[SID].SetRace( SLOTRACE_HUMAN | SLOTRACE_SELECTABLE );
									SendAllSlotInfo( );
								}
								else if( Race == "orc" )
								{
									m_Slots[SID].SetRace( SLOTRACE_ORC | SLOTRACE_SELECTABLE );
									SendAllSlotInfo( );
								}
								else if( Race == "night elf" )
								{
									m_Slots[SID].SetRace( SLOTRACE_NIGHTELF | SLOTRACE_SELECTABLE );
									SendAllSlotInfo( );
								}
								else if( Race == "undead" )
								{
									m_Slots[SID].SetRace( SLOTRACE_UNDEAD | SLOTRACE_SELECTABLE );
									SendAllSlotInfo( );
								}
								else if( Race == "random" )
								{
									m_Slots[SID].SetRace( SLOTRACE_RANDOM | SLOTRACE_SELECTABLE );
									SendAllSlotInfo( );
								}
								else
									CONSOLE_Print( "[GAME: " + m_GameName + "] unknown race [" + Race + "] sent to comprace command" );
							}
						}
					}
				}
			}

			//
			// !COMPTEAM (computer team change)
			//

			if( Command == "compteam" && !Payload.isEmpty( ) && !m_GameLoading && !m_GameLoaded && !m_SaveGame )
			{
				// extract the slot and the team
				// e.g. "1 2" -> slot: "1", team: "2"

				quint32 Slot;
				quint32 Team;
				QTextStream SS(&Payload);

				SS >> Slot;

				if( SS.status() != QTextStream::Ok )
					CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #1 to compteam command" );
				else
				{
					if( SS.atEnd( ) )
						CONSOLE_Print( "[GAME: " + m_GameName + "] missing input #2 to compteam command" );
					else
					{
						SS >> Team;

						if( SS.status() != QTextStream::Ok )
							CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #2 to compteam command" );
						else
						{
							unsigned char SID = (unsigned char)( Slot - 1 );

							if( !( m_Map->GetMapOptions( ) & MAPOPT_FIXEDPLAYERSETTINGS ) && Team < 12 && SID < m_Slots.size( ) )
							{
								if( m_Slots[SID].GetSlotStatus( ) == SLOTSTATUS_OCCUPIED && m_Slots[SID].GetComputer( ) == 1 )
								{
									m_Slots[SID].SetTeam( (unsigned char)( Team - 1 ) );
									SendAllSlotInfo( );
								}
							}
						}
					}
				}
			}

			//
			// !DBSTATUS
			//

			if( Command == "dbstatus" )
				SendAllChat( m_GHost->m_DB->GetStatus( ) );

			//
			// !DOWNLOAD
			// !DL
			//

			if( ( Command == "download" || Command == "dl" ) && !Payload.isEmpty( ) && !m_GameLoading && !m_GameLoaded )
			{
				CGamePlayer *LastMatch = NULL;
				quint32 Matches = GetPlayerFromNamePartial( Payload, &LastMatch );

				if( Matches == 0 )
					SendAllChat( m_GHost->m_Language->UnableToStartDownloadNoMatchesFound( Payload ) );
				else if( Matches == 1 )
				{
					if( !LastMatch->GetDownloadStarted( ) && !LastMatch->GetDownloadFinished( ) )
					{
						unsigned char SID = GetSIDFromPID( LastMatch->GetPID( ) );

						if( SID < m_Slots.size( ) && m_Slots[SID].GetDownloadStatus( ) != 100 )
						{
							// inform the client that we are willing to send the map

							CONSOLE_Print( "[GAME: " + m_GameName + "] map download started for player [" + LastMatch->GetName( ) + "]" );
							Send( LastMatch, m_Protocol->SEND_W3GS_STARTDOWNLOAD( GetHostPID( ) ) );
							LastMatch->SetDownloadAllowed( true );
							LastMatch->SetDownloadStarted( true );
							LastMatch->SetStartedDownloadingTicks( GetTicks( ) );
						}
					}
				}
				else
					SendAllChat( m_GHost->m_Language->UnableToStartDownloadFoundMoreThanOneMatch( Payload ) );
			}

			//
			// !DROP
			//

			if( Command == "drop" && m_GameLoaded )
				StopLaggers( "lagged out (dropped by admin)" );

			//
			// !END
			//

			if( Command == "end" && m_GameLoaded )
			{
				CONSOLE_Print( "[GAME: " + m_GameName + "] is over (admin ended game)" );
				StopPlayers( "was disconnected (admin ended game)" );
				deleteLater();
			}

			//
			// !FAKEPLAYER
			//

			if( Command == "fakeplayer" && !m_CountDownStarted )
			{
				if( m_FakePlayerPID == 255 )
					CreateFakePlayer( );
				else
					DeleteFakePlayer( );
			}

			//
			// !FPPAUSE
			//

			if( Command == "fppause" && m_FakePlayerPID != 255 && m_GameLoaded )
			{
				QByteArray CRC;
				QByteArray Action;
				Action.push_back( 1 );
				m_Actions.enqueue( new CIncomingAction( m_FakePlayerPID, CRC, Action ) );
			}

			//
			// !FPRESUME
			//

			if( Command == "fpresume" && m_FakePlayerPID != 255 && m_GameLoaded )
			{
				QByteArray CRC;
				QByteArray Action;
				Action.push_back( 2 );
				m_Actions.enqueue( new CIncomingAction( m_FakePlayerPID, CRC, Action ) );
			}

			//
			// !FROM
			//

			if( Command == "from" )
			{
				QString Froms;

				for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
				{
					// we reverse the byte order on the IP because it's stored in network byte order

					Froms += (*i)->GetNameTerminated( );
					Froms += ": (";
					Froms += m_GHost->m_DBLocal->FromCheck( Util::extractUInt32(Util::reverse((*i)->GetExternalIP( ))) );
					Froms += ")";

					if( i != m_Players.end( ) - 1 )
						Froms += ", ";

					if( ( m_GameLoading || m_GameLoaded ) && Froms.size( ) > 100 )
					{
						// cut the text into multiple lines ingame

						SendAllChat( Froms );
						Froms.clear( );
					}
				}

				if( !Froms.isEmpty( ) )
					SendAllChat( Froms );
			}

			//
			// !HCL
			//

			if( Command == "hcl" && !m_CountDownStarted )
			{
				if( !Payload.isEmpty( ) )
				{
					if( Payload.size( ) <= m_Slots.size( ) )
					{
						QString HCLChars = "abcdefghijklmnopqrstuvwxyz0123456789 -=,.";

						if( Payload.indexOf( QRegExp("[^" + HCLChars + "]") ) == -1 )
						{
							m_HCLCommandString = Payload;
							SendAllChat( m_GHost->m_Language->SettingHCL( m_HCLCommandString ) );
						}
						else
							SendAllChat( m_GHost->m_Language->UnableToSetHCLInvalid( ) );
					}
					else
						SendAllChat( m_GHost->m_Language->UnableToSetHCLTooLong( ) );
				}
				else
					SendAllChat( m_GHost->m_Language->TheHCLIs( m_HCLCommandString ) );
			}

			//
			// !HOLD (hold a slot for someone)
			//

			if( Command == "hold" && !Payload.isEmpty( ) && !m_GameLoading && !m_GameLoaded )
			{
				// hold as many players as specified, e.g. "Varlock Kilranin" holds players "Varlock" and "Kilranin"

				QTextStream SS(&Payload);


				while( !SS.atEnd( ) )
				{
					QString HoldName;
					SS >> HoldName;

					if( SS.status() != QTextStream::Ok )
					{
						CONSOLE_Print( "[GAME: " + m_GameName + "] bad input to hold command" );
						break;
					}
					else
					{
						SendAllChat( m_GHost->m_Language->AddedPlayerToTheHoldList( HoldName ) );
						AddToReserved( HoldName );
					}
				}
			}

			//
			// !KICK (kick a player)
			//

			if( Command == "kick" && !Payload.isEmpty( ) )
			{
				CGamePlayer *LastMatch = NULL;
				quint32 Matches = GetPlayerFromNamePartial( Payload, &LastMatch );

				if( Matches == 0 )
					SendAllChat( m_GHost->m_Language->UnableToKickNoMatchesFound( Payload ) );
				else if( Matches == 1 )
				{
					LastMatch->SetLeftReason( m_GHost->m_Language->WasKickedByPlayer( User ) );

					if( !m_GameLoading && !m_GameLoaded )
						LastMatch->SetLeftCode( PLAYERLEAVE_LOBBY );
					else
						LastMatch->SetLeftCode( PLAYERLEAVE_LOST );

					LastMatch->deleteLater();
					if( !m_GameLoading && !m_GameLoaded )
						OpenSlot( GetSIDFromPID( LastMatch->GetPID( ) ), false );
				}
				else
					SendAllChat( m_GHost->m_Language->UnableToKickFoundMoreThanOneMatch( Payload ) );
			}

			//
			// !LATENCY (set game latency)
			//

			if( Command == "latency" )
			{
				if( Payload.isEmpty( ) )
					SendAllChat( m_GHost->m_Language->LatencyIs( QString::number( m_Latency ) ) );
				else
				{
					m_RequestedLatency = Payload.toUInt();

					quint32 min = 20, max = 2000;

					if( m_RequestedLatency <= min )
					{
						m_RequestedLatency = min;
						SendAllChat( m_GHost->m_Language->SettingLatencyToMinimum( QString::number(m_RequestedLatency) ) );
					}
					else if( m_RequestedLatency >= max )
					{
						m_RequestedLatency = max;
						SendAllChat( m_GHost->m_Language->SettingLatencyToMaximum( QString::number(m_RequestedLatency) ) );
					}
					else
						SendAllChat( m_GHost->m_Language->SettingLatencyTo( QString::number( m_RequestedLatency ) ) );
				}
			}

			//
			// !LOCK
			//

			if( Command == "lock" && ( RootAdminCheck || IsOwner( User ) ) )
			{
				SendAllChat( m_GHost->m_Language->GameLocked( ) );
				m_Locked = true;
			}

			//
			// !MESSAGES
			//

			if( Command == "messages" )
			{
				if( Payload == "on" )
				{
					SendAllChat( m_GHost->m_Language->LocalAdminMessagesEnabled( ) );
					m_LocalAdminMessages = true;
				}
				else if( Payload == "off" )
				{
					SendAllChat( m_GHost->m_Language->LocalAdminMessagesDisabled( ) );
					m_LocalAdminMessages = false;
				}
			}

			//
			// !MUTE
			//

			if( Command == "mute" )
			{
				CGamePlayer *LastMatch = NULL;
				quint32 Matches = GetPlayerFromNamePartial( Payload, &LastMatch );

				if( Matches == 0 )
					SendAllChat( m_GHost->m_Language->UnableToMuteNoMatchesFound( Payload ) );
				else if( Matches == 1 )
				{
					SendAllChat( m_GHost->m_Language->MutedPlayer( LastMatch->GetName( ), User ) );
					LastMatch->SetMuted( true );
				}
				else
					SendAllChat( m_GHost->m_Language->UnableToMuteFoundMoreThanOneMatch( Payload ) );
			}

			//
			// !MUTEALL
			//

			if( Command == "muteall" && m_GameLoaded )
			{
				SendAllChat( m_GHost->m_Language->GlobalChatMuted( ) );
				m_MuteAll = true;
			}

			//
			// !OPEN (open slot)
			//

			if( Command == "open" && !Payload.isEmpty( ) && !m_GameLoading && !m_GameLoaded )
			{
				// open as many slots as specified, e.g. "5 10" opens slots 5 and 10

				QTextStream SS(&Payload);


				while( !SS.atEnd( ) )
				{
					quint32 SID;
					SS >> SID;

					if( SS.status() != QTextStream::Ok )
					{
						CONSOLE_Print( "[GAME: " + m_GameName + "] bad input to open command" );
						break;
					}
					else
						OpenSlot( (unsigned char)( SID - 1 ), true );
				}
			}

			//
			// !OPENALL
			//

			if( Command == "openall" && !m_GameLoading && !m_GameLoaded )
				OpenAllSlots( );

			//
			// !OWNER (set game owner)
			//

			if( Command == "owner" )
			{
				if( RootAdminCheck || IsOwner( User ) || !GetPlayerFromName( m_OwnerName, false ) )
				{
					if( !Payload.isEmpty( ) )
					{
						SendAllChat( m_GHost->m_Language->SettingGameOwnerTo( Payload ) );
						m_OwnerName = Payload;
					}
					else
					{
						SendAllChat( m_GHost->m_Language->SettingGameOwnerTo( User ) );
						m_OwnerName = User;
					}
				}
				else
					SendAllChat( m_GHost->m_Language->UnableToSetGameOwner( m_OwnerName ) );
			}

			//
			// !PING
			//

			if( Command == "ping" )
			{
				// kick players with ping higher than payload if payload isn't empty
				// we only do this if the game hasn't started since we don't want to kick players from a game in progress

				quint32 Kicked = 0;
				quint32 KickPing = 0;

				if( !m_GameLoading && !m_GameLoaded && !Payload.isEmpty( ) )
					KickPing = Payload.toUInt();

				// copy the m_Players vector so we can sort by descending ping so it's easier to find players with high pings

				QList<CGamePlayer *> SortedPlayers = m_Players;
				qSort(SortedPlayers);
				QString Pings;

				for( QList<CGamePlayer *> :: const_iterator i = SortedPlayers.begin( ); i != SortedPlayers.end( ); i++ )
				{
					Pings += (*i)->GetNameTerminated( );
					Pings += ": ";

					if( (*i)->GetNumPings( ) > 0 )
					{
						Pings += QString::number( (*i)->GetPing( m_GHost->m_LCPings ) );

						if( !m_GameLoading && !m_GameLoaded && !(*i)->GetReserved( ) && KickPing > 0 && (*i)->GetPing( m_GHost->m_LCPings ) > KickPing )
						{
							(*i)->SetLeftReason( "was kicked for excessive ping " + QString::number( (*i)->GetPing( m_GHost->m_LCPings ) ) + " > " + QString::number( KickPing ) );
							(*i)->SetLeftCode( PLAYERLEAVE_LOBBY );
							(*i)->deleteLater();
							OpenSlot( GetSIDFromPID( (*i)->GetPID( ) ), false );
							Kicked++;
						}

						Pings += "ms";
					}
					else
						Pings += "N/A";

					if( i != SortedPlayers.end( ) - 1 )
						Pings += ", ";

					if( ( m_GameLoading || m_GameLoaded ) && Pings.size( ) > 100 )
					{
						// cut the text into multiple lines ingame

						SendAllChat( Pings );
						Pings.clear( );
					}
				}

				if( !Pings.isEmpty( ) )
					SendAllChat( Pings );

				if( Kicked > 0 )
					SendAllChat( m_GHost->m_Language->KickingPlayersWithPingsGreaterThan( QString::number( Kicked ), QString::number( KickPing ) ) );
			}

			//
			// !PRIV (rehost as private game)
			//

			if( Command == "priv" && !Payload.isEmpty( ) && !m_CountDownStarted && !m_SaveGame )
			{
				CONSOLE_Print( "[GAME: " + m_GameName + "] trying to rehost as private game [" + Payload + "]" );
				SendAllChat( m_GHost->m_Language->TryingToRehostAsPrivateGame( Payload ) );
				m_GameState = GAME_PRIVATE;
				m_LastGameName = m_GameName;
				m_GameName = Payload;
				m_HostCounter = m_GHost->m_HostCounter++;
				m_RefreshError = false;
				m_RefreshRehosted = true;

				for( QList<CBNET *> :: const_iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
				{
					// unqueue any existing game refreshes because we're going to assume the next successful game refresh indicates that the rehost worked
					// this ignores the fact that it's possible a game refresh was just sent and no response has been received yet
					// we assume this won't happen very often since the only downside is a potential false positive

					(*i)->UnqueueGameRefreshes( );
					(*i)->QueueGameUncreate( );
					(*i)->QueueEnterChat( );

					// we need to send the game creation message now because private games are not refreshed

					(*i)->QueueGameCreate( m_GameState, m_GameName, QString( ), m_Map, NULL, m_HostCounter );

					if( (*i)->GetPasswordHashType( ) != "pvpgn" )
						(*i)->QueueEnterChat( );
				}

				m_CreationTime = GetTime( );
			}

			//
			// !PUB (rehost as public game)
			//

			if( Command == "pub" && !Payload.isEmpty( ) && !m_CountDownStarted && !m_SaveGame )
			{
				CONSOLE_Print( "[GAME: " + m_GameName + "] trying to rehost as public game [" + Payload + "]" );
				SendAllChat( m_GHost->m_Language->TryingToRehostAsPublicGame( Payload ) );
				m_GameState = GAME_PUBLIC;
				m_LastGameName = m_GameName;
				m_GameName = Payload;
				m_HostCounter = m_GHost->m_HostCounter++;
				m_RefreshError = false;
				m_RefreshRehosted = true;

				for( QList<CBNET *> :: const_iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
				{
					// unqueue any existing game refreshes because we're going to assume the next successful game refresh indicates that the rehost worked
					// this ignores the fact that it's possible a game refresh was just sent and no response has been received yet
					// we assume this won't happen very often since the only downside is a potential false positive

					(*i)->UnqueueGameRefreshes( );
					(*i)->QueueGameUncreate( );
					(*i)->QueueEnterChat( );

					// the game creation message will be sent on the next refresh
				}

				m_CreationTime = GetTime( );
			}

			//
			// !REFRESH (turn on or off refresh messages)
			//

			if( Command == "refresh" && !m_CountDownStarted )
			{
				if( Payload == "on" )
				{
					SendAllChat( m_GHost->m_Language->RefreshMessagesEnabled( ) );
					m_RefreshMessages = true;
				}
				else if( Payload == "off" )
				{
					SendAllChat( m_GHost->m_Language->RefreshMessagesDisabled( ) );
					m_RefreshMessages = false;
				}
			}

			//
			// !SAY
			//

			if( Command == "say" && !Payload.isEmpty( ) )
			{
				for( QList<CBNET *> :: const_iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
					(*i)->QueueChatCommand( Payload );

				HideCommand = true;
			}

			//
			// !SENDLAN
			//

			if( Command == "sendlan" && !Payload.isEmpty( ) && !m_CountDownStarted )
			{
				// extract the ip and the port
				// e.g. "1.2.3.4 6112" -> ip: "1.2.3.4", port: "6112"

				QString IP;
				quint32 Port = 6112;
				QTextStream SS(&Payload);

				SS >> IP;

				if( !SS.atEnd( ) )
					SS >> Port;

				if( SS.status() != QTextStream::Ok )
					CONSOLE_Print( "[GAME: " + m_GameName + "] bad inputs to sendlan command" );
				else
				{
					// we send 12 for SlotsTotal because this determines how many PID's Warcraft 3 allocates
					// we need to make sure Warcraft 3 allocates at least SlotsTotal + 1 but at most 12 PID's
					// this is because we need an extra PID for the virtual host player (but we always delete the virtual host player when the 12th person joins)
					// however, we can't send 13 for SlotsTotal because this causes Warcraft 3 to crash when sharing control of units
					// nor can we send SlotsTotal because then Warcraft 3 crashes when playing maps with less than 12 PID's (because of the virtual host player taking an extra PID)
					// we also send 12 for SlotsOpen because Warcraft 3 assumes there's always at least one player in the game (the host)
					// so if we try to send accurate numbers it'll always be off by one and results in Warcraft 3 assuming the game is full when it still needs one more player
					// the easiest solution is to simply send 12 for both so the game will always show up as (1/12) players

					if( m_SaveGame )
					{
						// note: the PrivateGame flag is not set when broadcasting to LAN (as you might expect)

						quint32 MapGameType = MAPGAMETYPE_SAVEDGAME;
						QByteArray MapWidth;
						MapWidth.push_back( (char)0 );
						MapWidth.push_back( (char)0 );
						QByteArray MapHeight;
						MapHeight.push_back( (char)0 );
						MapHeight.push_back( (char)0 );
						m_GHost->m_UDPSocket->writeDatagram( m_Protocol->SEND_W3GS_GAMEINFO( m_GHost->m_TFT, m_GHost->m_LANWar3Version, Util::fromUInt32( MapGameType), m_Map->GetMapGameFlags( ), MapWidth, MapHeight, m_GameName, "Varlock", GetTime( ) - m_CreationTime, "Save\\Multiplayer\\" + m_SaveGame->GetFileNameNoPath( ), m_SaveGame->GetMagicNumber( ), 12, 12, m_HostPort, m_HostCounter ),
								QHostAddress(IP), Port);
					}
					else
					{
						// note: the PrivateGame flag is not set when broadcasting to LAN (as you might expect)
						// note: we do not use m_Map->GetMapGameType because none of the filters are set when broadcasting to LAN (also as you might expect)

						quint32 MapGameType = MAPGAMETYPE_UNKNOWN0;
						m_GHost->m_UDPSocket->writeDatagram( m_Protocol->SEND_W3GS_GAMEINFO( m_GHost->m_TFT, m_GHost->m_LANWar3Version, Util::fromUInt32( MapGameType), m_Map->GetMapGameFlags( ), m_Map->GetMapWidth( ), m_Map->GetMapHeight( ), m_GameName, "Varlock", GetTime( ) - m_CreationTime, m_Map->GetMapPath( ), m_Map->GetMapCRC( ), 12, 12, m_HostPort, m_HostCounter ),
								QHostAddress(IP), Port);
					}
				}
			}

			//
			// !SP
			//

			if( Command == "sp" && !m_CountDownStarted )
			{
				SendAllChat( m_GHost->m_Language->ShufflingPlayers( ) );
				ShuffleSlots( );
			}

			//
			// !START
			//

			if( Command == "start" && !m_CountDownStarted )
			{
				// if the player sent "!start force" skip the checks and start the countdown
				// otherwise check that the game is ready to start

				if( Payload == "force" )
					StartCountDown( true );
				else
				{
					if( GetTicks( ) - m_LastPlayerLeaveTicks >= 2000 )
						StartCountDown( false );
					else
						SendAllChat( m_GHost->m_Language->CountDownAbortedSomeoneLeftRecently( ) );
				}
			}

			//
			// !SWAP (swap slots)
			//

			if( Command == "swap" && !Payload.isEmpty( ) && !m_GameLoading && !m_GameLoaded )
			{
				quint32 SID1;
				quint32 SID2;
				QTextStream SS(&Payload);

				SS >> SID1;

				if( SS.status() != QTextStream::Ok )
					CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #1 to swap command" );
				else
				{
					if( SS.atEnd( ) )
						CONSOLE_Print( "[GAME: " + m_GameName + "] missing input #2 to swap command" );
					else
					{
						SS >> SID2;

						if( SS.status() != QTextStream::Ok )
							CONSOLE_Print( "[GAME: " + m_GameName + "] bad input #2 to swap command" );
						else
							SwapSlots( (unsigned char)( SID1 - 1 ), (unsigned char)( SID2 - 1 ) );
					}
				}
			}

			//
			// !SYNCLIMIT
			//

			if( Command == "synclimit" )
			{
				if( Payload.isEmpty( ) )
					SendAllChat( m_GHost->m_Language->SyncLimitIs( QString::number( m_SyncLimit ) ) );
				else
				{
					m_SyncLimit = Payload.toUInt();

					if( m_SyncLimit <= 10 )
					{
						m_SyncLimit = 10;
						SendAllChat( m_GHost->m_Language->SettingSyncLimitToMinimum( "10" ) );
					}
					else if( m_SyncLimit >= 10000 )
					{
						m_SyncLimit = 10000;
						SendAllChat( m_GHost->m_Language->SettingSyncLimitToMaximum( "10000" ) );
					}
					else
						SendAllChat( m_GHost->m_Language->SettingSyncLimitTo( QString::number( m_SyncLimit ) ) );
				}
			}

			//
			// !UNHOST
			//

			if( Command == "unhost" && !m_CountDownStarted )
				deleteLater();

			//
			// !UNLOCK
			//

			if( Command == "unlock" && ( RootAdminCheck || IsOwner( User ) ) )
			{
				SendAllChat( m_GHost->m_Language->GameUnlocked( ) );
				m_Locked = false;
			}

			//
			// !UNMUTE
			//

			if( Command == "unmute" )
			{
				CGamePlayer *LastMatch = NULL;
				quint32 Matches = GetPlayerFromNamePartial( Payload, &LastMatch );

				if( Matches == 0 )
					SendAllChat( m_GHost->m_Language->UnableToMuteNoMatchesFound( Payload ) );
				else if( Matches == 1 )
				{
					SendAllChat( m_GHost->m_Language->UnmutedPlayer( LastMatch->GetName( ), User ) );
					LastMatch->SetMuted( false );
				}
				else
					SendAllChat( m_GHost->m_Language->UnableToMuteFoundMoreThanOneMatch( Payload ) );
			}

			//
			// !UNMUTEALL
			//

			if( Command == "unmuteall" && m_GameLoaded )
			{
				SendAllChat( m_GHost->m_Language->GlobalChatUnmuted( ) );
				m_MuteAll = false;
			}

			//
			// !VIRTUALHOST
			//

			if( Command == "virtualhost" && !Payload.isEmpty( ) && Payload.size( ) <= 15 && !m_CountDownStarted )
			{
				DeleteVirtualHost( );
				m_VirtualHostName = Payload;
				CreateVirtualHost();
			}

			//
			// !VOTECANCEL
			//

			if( Command == "votecancel" && !m_KickVotePlayer.isEmpty( ) )
			{
				SendAllChat( m_GHost->m_Language->VoteKickCancelled( m_KickVotePlayer ) );
				m_KickVotePlayer.clear( );
				m_VotekickTimer.stop();
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
				int MessageStart = Payload.indexOf( " " );

				if( MessageStart != -1 )
				{
					Name = Payload.mid( 0, MessageStart );
					Message = Payload.mid( MessageStart + 1 );

					for( QList<CBNET *> :: const_iterator i = m_GHost->m_BNETs.begin( ); i != m_GHost->m_BNETs.end( ); i++ )
						(*i)->QueueChatCommand( Message, Name, true );
				}

				HideCommand = true;
			}
		}
		else
		{
			CONSOLE_Print( "[GAME: " + m_GameName + "] admin command ignored, the game is locked" );
			SendChat( player, m_GHost->m_Language->TheGameIsLocked( ) );
		}
	}
	else
	{
		if( !player->GetSpoofed( ) )
			CONSOLE_Print( "[GAME: " + m_GameName + "] non-spoofchecked user [" + User + "] sent command [" + Command + "] with payload [" + Payload + "]" );
		else
			CONSOLE_Print( "[GAME: " + m_GameName + "] non-admin [" + User + "] sent command [" + Command + "] with payload [" + Payload + "]" );
	}

	/*********************
	* NON ADMIN COMMANDS *
	*********************/

	//
	// !CHECKME
	//

	if( Command == "checkme" )
		SendChat( player, m_GHost->m_Language->CheckedPlayer( User, player->GetNumPings( ) > 0 ? QString::number( player->GetPing( m_GHost->m_LCPings ) ) + "ms" : "N/A", m_GHost->m_DBLocal->FromCheck( Util::extractUInt32(Util::reverse(player->GetExternalIP( ))) ), AdminCheck || RootAdminCheck ? "Yes" : "No", IsOwner( User ) ? "Yes" : "No", player->GetSpoofed( ) ? "Yes" : "No", player->GetSpoofedRealm( ).isEmpty( ) ? "N/A" : player->GetSpoofedRealm( ), player->GetReserved( ) ? "Yes" : "No" ) );

	//
	// !STATS
	//

	if( Command == "stats" && GetTime( ) - player->GetStatsSentTime( ) >= 5 )
	{
		QString StatsUser = User;

		if( !Payload.isEmpty( ) )
			StatsUser = Payload;

		if( player->GetSpoofed( ) && ( AdminCheck || RootAdminCheck || IsOwner( User ) ) )
			m_PairedGPSChecks.push_back( PairedGPSCheck( QString( ), m_GHost->m_DB->ThreadedGamePlayerSummaryCheck( StatsUser ) ) );
		else
			m_PairedGPSChecks.push_back( PairedGPSCheck( User, m_GHost->m_DB->ThreadedGamePlayerSummaryCheck( StatsUser ) ) );

		player->SetStatsSentTime( GetTime( ) );
	}

	//
	// !STATSDOTA
	//

	if( Command == "statsdota" && GetTime( ) - player->GetStatsDotASentTime( ) >= 5 )
	{
		QString StatsUser = User;

		if( !Payload.isEmpty( ) )
			StatsUser = Payload;

		if( player->GetSpoofed( ) && ( AdminCheck || RootAdminCheck || IsOwner( User ) ) )
			m_PairedDPSChecks.push_back( PairedDPSCheck( QString( ), m_GHost->m_DB->ThreadedDotAPlayerSummaryCheck( StatsUser ) ) );
		else
			m_PairedDPSChecks.push_back( PairedDPSCheck( User, m_GHost->m_DB->ThreadedDotAPlayerSummaryCheck( StatsUser ) ) );

		player->SetStatsDotASentTime( GetTime( ) );
	}

	//
	// !VERSION
	//

	if( Command == "version" )
	{
		if( player->GetSpoofed( ) && ( AdminCheck || RootAdminCheck || IsOwner( User ) ) )
			SendChat( player, m_GHost->m_Language->VersionAdmin( m_GHost->m_Version ) );
		else
			SendChat( player, m_GHost->m_Language->VersionNotAdmin( m_GHost->m_Version ) );
	}

	//
	// !VOTEKICK
	//

	if( Command == "votekick" && m_GHost->m_VoteKickAllowed && !Payload.isEmpty( ) )
	{
		if( !m_KickVotePlayer.isEmpty( ) )
			SendChat( player, m_GHost->m_Language->UnableToVoteKickAlreadyInProgress( ) );
		else if( m_Players.size( ) == 2 )
			SendChat( player, m_GHost->m_Language->UnableToVoteKickNotEnoughPlayers( ) );
		else
		{
			CGamePlayer *LastMatch = NULL;
			quint32 Matches = GetPlayerFromNamePartial( Payload, &LastMatch );

			if( Matches == 0 )
				SendChat( player, m_GHost->m_Language->UnableToVoteKickNoMatchesFound( Payload ) );
			else if( Matches == 1 )
			{
				if( LastMatch->GetReserved( ) )
					SendChat( player, m_GHost->m_Language->UnableToVoteKickPlayerIsReserved( LastMatch->GetName( ) ) );
				else
				{
					m_KickVotePlayer = LastMatch->GetName( );
					m_VotekickTimer.start();

					for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
						(*i)->SetKickVote( false );

					player->SetKickVote( true );
					CONSOLE_Print( "[GAME: " + m_GameName + "] votekick against player [" + m_KickVotePlayer + "] started by player [" + User + "]" );
					SendAllChat( m_GHost->m_Language->StartedVoteKick( LastMatch->GetName( ), User, QString::number( (quint32)ceil( ( GetNumHumanPlayers( ) - 1 ) * (float)m_GHost->m_VoteKickPercentage / 100 ) - 1 ) ) );
					SendAllChat( m_GHost->m_Language->TypeYesToVote( QString( 1, m_GHost->m_CommandTrigger ) ) );
				}
			}
			else
				SendChat( player, m_GHost->m_Language->UnableToVoteKickFoundMoreThanOneMatch( Payload ) );
		}
	}

	//
	// !YES
	//

	if( Command == "yes" && !m_KickVotePlayer.isEmpty( ) && player->GetName( ) != m_KickVotePlayer && !player->GetKickVote( ) )
	{
		player->SetKickVote( true );
		quint32 VotesNeeded = (quint32)ceil( ( GetNumHumanPlayers( ) - 1 ) * (float)m_GHost->m_VoteKickPercentage / 100 );
		quint32 Votes = 0;

		for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		{
			if( (*i)->GetKickVote( ) )
				Votes++;
		}

		if( Votes >= VotesNeeded )
		{
			CGamePlayer *Victim = GetPlayerFromName( m_KickVotePlayer, true );

			if( Victim )
			{
				Victim->SetLeftReason( m_GHost->m_Language->WasKickedByVote( ) );

				if( !m_GameLoading && !m_GameLoaded )
					Victim->SetLeftCode( PLAYERLEAVE_LOBBY );
				else
					Victim->SetLeftCode( PLAYERLEAVE_LOST );

				Victim->deleteLater();

				if( !m_GameLoading && !m_GameLoaded )
					OpenSlot( GetSIDFromPID( Victim->GetPID( ) ), false );

				CONSOLE_Print( "[GAME: " + m_GameName + "] votekick against player [" + m_KickVotePlayer + "] passed with " + QString::number( Votes ) + "/" + QString::number( GetNumHumanPlayers( ) ) + " votes" );
				SendAllChat( m_GHost->m_Language->VoteKickPassed( m_KickVotePlayer ) );
			}
			else
				SendAllChat( m_GHost->m_Language->ErrorVoteKickingPlayer( m_KickVotePlayer ) );

			m_KickVotePlayer.clear( );
			m_VotekickTimer.stop();
		}
		else
			SendAllChat( m_GHost->m_Language->VoteKickAcceptedNeedMoreVotes( m_KickVotePlayer, User, QString::number( VotesNeeded - Votes ) ) );
	}

	return HideCommand;
}

void CGame :: EventGameStarted( )
{
	CBaseGame :: EventGameStarted( );

	// record everything we need to ban each player in case we decide to do so later
	// this is because when a player leaves the game an admin might want to ban that player
	// but since the player has already left the game we don't have access to their information anymore
	// so we create a "potential ban" for each player and only store it in the database if requested to by an admin

	for( QList<CGamePlayer *> :: const_iterator i = m_Players.begin( ); i != m_Players.end( ); i++ )
		m_DBBans.push_back( new CDBBan( (*i)->GetJoinedRealm( ), (*i)->GetName( ), (*i)->GetExternalIPString( ), QString( ), QString( ), QString( ), QString( ) ) );
}

bool CGame :: IsGameDataSaved( )
{
	return m_CallableGameAdd && m_CallableGameAdd->GetReady( );
}

void CGame :: SaveGameData( )
{
	CONSOLE_Print( "[GAME: " + m_GameName + "] saving game data to database" );
	m_CallableGameAdd = m_GHost->m_DB->ThreadedGameAdd( m_GHost->m_BNETs.size( ) == 1 ? m_GHost->m_BNETs[0]->GetServer( ) : QString( ), m_DBGame->GetMap( ), m_GameName, m_OwnerName, m_GameTicks / 1000, m_GameState, m_CreatorName, m_CreatorServer );

	if (!m_CallableGameAdd->GetReady())
		QObject::connect(m_CallableGameAdd, SIGNAL(finished()), this, SLOT(EventGameDataSaved()));

	else
		EventGameDataSaved();
}
