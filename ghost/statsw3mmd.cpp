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
#include "ghostdb.h"
#include "gameprotocol.h"
#include "game_base.h"
#include "stats.h"
#include "statsw3mmd.h"

#include <QByteArray>

//
// CStatsW3MMD
//

CStatsW3MMD :: CStatsW3MMD( CBaseGame *nGame, QString nCategory ) : CStats( nGame )
{
	CONSOLE_Print( "[STATSW3MMD] using Warcraft 3 Map Meta Data stats parser version 1" );
	CONSOLE_Print( "[STATSW3MMD] using map_statsw3mmdcategory [" + nCategory + "]" );
	m_Category = nCategory;
	m_NextValueID = 0;
	m_NextCheckID = 0;
}

CStatsW3MMD :: ~CStatsW3MMD( )
{

}

bool CStatsW3MMD :: ProcessAction( CIncomingAction *Action )
{
	unsigned int i = 0;
	QByteArray *ActionData = Action->GetAction( );
	QByteArray MissionKey;
	QByteArray Key;
	QByteArray Value;

	while( (unsigned int)ActionData->size( ) >= i + 9 )
	{
		if( (*ActionData).at(i) == 'k' &&
			(*ActionData).at(i + 1) == 'M' &&
			(*ActionData).at(i + 2) == 'M' &&
			(*ActionData).at(i + 3) == 'D' &&
			(*ActionData).at(i + 4) == '.' &&
			(*ActionData).at(i + 5) == 'D' &&
			(*ActionData).at(i + 6) == 'a' &&
			(*ActionData).at(i + 7) == 't' &&
			(*ActionData).at(i + 8) == 0x00 )
		{
			if( (unsigned int)ActionData->size( ) >= i + 10 )
			{
				MissionKey = UTIL_ExtractCString( *ActionData, i + 9 );

				if( (unsigned int)ActionData->size( ) >= i + 11 + MissionKey.size( ) )
				{
					Key = UTIL_ExtractCString( *ActionData, i + 10 + MissionKey.size( ) );

					if( (unsigned int)ActionData->size( ) >= i + 15 + MissionKey.size( ) + Key.size( ) )
					{
						Value = ActionData->mid(i + 11 + MissionKey.size( ) + Key.size( ), 4 );
						//quint32 ValueInt = UTIL_QByteArrayToUInt32( Value, false );

						// CONSOLE_Print( "[STATSW3MMD] DEBUG: mkey [" + MissionKey + "], key [" + Key + "], value [" + QString::number(ValueInt) + "]" );

						if( MissionKey.size( ) > 4 && MissionKey.mid( 0, 4 ) == "val:" )
						{
							//QByteArray ValueIDString = MissionKey.mid( 4 );
							//quint32 ValueID = UTIL_ToUInt32( ValueIDString );
							QList<QByteArray> Tokens = TokenizeKey( Key );

							if( !Tokens.isEmpty( ) )
							{
								if( Tokens[0] == "init" && Tokens.size( ) >= 2 )
								{
									if( Tokens[1] == "version" && Tokens.size( ) == 4 )
									{
										// Tokens[2] = minimum
										// Tokens[3] = current

										CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] map is using Warcraft 3 Map Meta Data library version [" + Tokens[3] + "]" );

										if( Tokens[2].toUInt() > 1 )
											CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] warning - parser version 1 is not compatible with this map, minimum version [" + Tokens[2] + "]" );
									}
									else if( Tokens[1] == "pid" && Tokens.size( ) == 4 )
									{
										// Tokens[2] = pid
										// Tokens[3] = name

										quint32 PID = Tokens[2].toUInt();

										if( m_PIDToName.find( PID ) != m_PIDToName.end( ) )
											CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] overwriting previous name [" + m_PIDToName[PID] + "] with new name [" + Tokens[3] + "] for PID [" + Tokens[2] + "]" );

										m_PIDToName[PID] = Tokens[3];
									}
								}
								else if( Tokens[0] == "DefVarP" && Tokens.size( ) == 5 )
								{
									// Tokens[1] = name
									// Tokens[2] = value type
									// Tokens[3] = goal type (ignored here)
									// Tokens[4] = suggestion (ignored here)

									if( m_DefVarPs.find( Tokens[1] ) != m_DefVarPs.end( ) )
										CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] duplicate DefVarP [" + Key + "] found, ignoring" );
									else
									{
										if( Tokens[2] == "int" || Tokens[2] == "real" || Tokens[2] == "QString" )
											m_DefVarPs[Tokens[1]] = Tokens[2];
										else
											CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] unknown DefVarP [" + Key + "] found, ignoring" );
									}

								}
								else if( Tokens[0] == "VarP" && Tokens.size( ) == 5 )
								{
									// Tokens[1] = pid
									// Tokens[2] = name
									// Tokens[3] = operation
									// Tokens[4] = value

									if( m_DefVarPs.find( Tokens[2] ) == m_DefVarPs.end( ) )
										CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] VarP [" + Key + "] found without a corresponding DefVarP, ignoring" );
									else
									{
										QString ValueType = m_DefVarPs[Tokens[2]];

										if( ValueType == "int" )
										{
											VarP VP = VarP( Tokens[1].toUInt(), Tokens[2] );

											if( Tokens[3] == "=" )
													m_VarPInts[VP] = Tokens[4].toInt();
											else if( Tokens[3] == "+=" )
											{
												if( m_VarPInts.find( VP ) != m_VarPInts.end( ) )
													m_VarPInts[VP] += Tokens[4].toInt();
												else
												{
													// CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] int VarP [" + Key + "] found with relative operation [+=] without a previously assigned value, ignoring" );
													m_VarPInts[VP] = Tokens[4].toInt();
												}
											}
											else if( Tokens[3] == "-=" )
											{
												if( m_VarPInts.find( VP ) != m_VarPInts.end( ) )
													m_VarPInts[VP] -= Tokens[4].toInt();
												else
												{
													// CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] int VarP [" + Key + "] found with relative operation [-=] without a previously assigned value, ignoring" );
													m_VarPInts[VP] = -Tokens[4].toInt();
												}
											}
											else
												CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] unknown int VarP [" + Key + "] operation [" + Tokens[3] + "] found, ignoring" );
										}
										else if( ValueType == "real" )
										{
											VarP VP = VarP( Tokens[1].toUInt(), Tokens[2] );

											if( Tokens[3] == "=" )
												m_VarPReals[VP] = Tokens[4].toDouble();
											else if( Tokens[3] == "+=" )
											{
												if( m_VarPReals.find( VP ) != m_VarPReals.end( ) )
													m_VarPReals[VP] += Tokens[4].toDouble();
												else
												{
													// CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] real VarP [" + Key + "] found with relative operation [+=] without a previously assigned value, ignoring" );
													m_VarPReals[VP] = Tokens[4].toDouble();
												}
											}
											else if( Tokens[3] == "-=" )
											{
												if( m_VarPReals.find( VP ) != m_VarPReals.end( ) )
													m_VarPReals[VP] -= Tokens[4].toDouble();
												else
												{
													// CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] real VarP [" + Key + "] found with relative operation [-=] without a previously assigned value, ignoring" );
													m_VarPReals[VP] = -Tokens[4].toDouble();
												}
											}
											else
												CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] unknown real VarP [" + Key + "] operation [" + Tokens[3] + "] found, ignoring" );
										}
										else
										{
											VarP VP = VarP( Tokens[1].toUInt(), Tokens[2]);

											if( Tokens[3] == "=" )
												m_VarPStrings[VP] = Tokens[4];
											else
												CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] unknown QString VarP [" + Key + "] operation [" + Tokens[3] + "] found, ignoring" );
										}
									}
								}
								else if( Tokens[0] == "FlagP" && Tokens.size( ) == 3 )
								{
									// Tokens[1] = pid
									// Tokens[2] = flag

									if( Tokens[2] == "winner" || Tokens[2] == "loser" || Tokens[2] == "drawer" || Tokens[2] == "leaver" || Tokens[2] == "practicing" )
									{
										quint32 PID = Tokens[1].toUInt();

										if( Tokens[2] == "leaver" )
											m_FlagsLeaver[PID] = true;
										else if( Tokens[2] == "practicing" )
											m_FlagsPracticing[PID] = true;
										else
										{
											if( m_Flags.find( PID ) != m_Flags.end( ) )
												CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] overwriting previous flag [" + m_Flags[PID] + "] with new flag [" + Tokens[2] + "] for PID [" + Tokens[1] + "]" );

											m_Flags[PID] = Tokens[2];
										}
									}
									else
										CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] unknown flag [" + Tokens[2] + "] found, ignoring" );
								}
								else if( Tokens[0] == "DefEvent" && Tokens.size( ) >= 4 )
								{
									// Tokens[1] = name
									// Tokens[2] = # of arguments (n)
									// Tokens[3..n+3] = arguments
									// Tokens[n+3] = format

									if( m_DefEvents.find( Tokens[1] ) != m_DefEvents.end( ) )
										CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] duplicate DefEvent [" + Key + "] found, ignoring" );
									else
									{
										quint32 Arguments = Tokens[2].toUInt();

										if( (unsigned int)Tokens.size( ) == Arguments + 4 )
											m_DefEvents[Tokens[1]] = Tokens.mid(3);
									}
								}
								else if( Tokens[0] == "Event" && Tokens.size( ) >= 2 )
								{
									// Tokens[1] = name
									// Tokens[2..n+2] = arguments (where n is the # of arguments in the corresponding DefEvent)

									if( m_DefEvents.find( Tokens[1] ) == m_DefEvents.end( ) )
										CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] Event [" + Key + "] found without a corresponding DefEvent, ignoring" );
									else
									{
										QList<QByteArray> DefEvent = m_DefEvents[Tokens[1]];

										if( !DefEvent.isEmpty( ) )
										{
											QString Format = DefEvent[DefEvent.size( ) - 1];

											if( Tokens.size( ) - 2 != DefEvent.size( ) - 1 )
												CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] Event [" + Key + "] found with " + QString::number( Tokens.size( ) - 2 ) + " arguments but expected " + QString::number(DefEvent.size() - 1 ) + " arguments, ignoring" );
											else
											{
												// replace the markers in the format QString with the arguments

												for( int i = 0; i < Tokens.size( ) - 2; i++ )
												{
													// check if the marker is a PID marker

													if( DefEvent[i].mid( 0, 4 ) == "pid:" )
													{
														// replace it with the player's name rather than their PID

														quint32 PID = Tokens[i + 2].toUInt();

														if( m_PIDToName.find( PID ) == m_PIDToName.end( ) )
															Format.replace( "{" + QString::number(i) + "}", "PID:" + Tokens[i + 2] );
														else
															Format.replace( "{" + QString::number(i) + "}", m_PIDToName[PID] );
													}
													else
														Format.replace( "{" + QString::number(i) + "}", Tokens[i + 2] );
												}

												CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] " + Format );
											}
										}
									}

									// CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] event [" + Key + "]" );
								}
								else if( Tokens[0] == "Blank" )
								{
									// ignore
								}
								else if( Tokens[0] == "Custom" )
								{
									CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] custom [" + Key + "]" );
								}
								else
									CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] unknown message type [" + Tokens[0] + "] found, ignoring" );
							}

							m_NextValueID++;
						}
						else if( MissionKey.size( ) > 4 && MissionKey.mid( 0, 4 ) == "chk:" )
						{
							QString CheckIDString = MissionKey.mid( 4 );
							//quint32 CheckID = UTIL_ToUInt32( CheckIDString );

							// todotodo: cheat detection

							m_NextCheckID++;
						}
						else
							CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] unknown mission key [" + MissionKey + "] found, ignoring" );

						i += 15 + MissionKey.size( ) + Key.size( );
					}
					else
						i++;
				}
				else
					i++;
			}
			else
				i++;
		}
		else
			i++;
	}

	return false;
}

void CStatsW3MMD :: Save( CGHost *GHost, CGHostDB *DB, quint32 GameID )
{
	CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] received " + QString::number(m_NextValueID) + "/" + QString::number(m_NextCheckID) + " value/check messages" );

	if( DB->Begin( ) )
	{
		// todotodo: there's no reason to create a new callable for each entry in this map
		// rewrite ThreadedW3MMDPlayerAdd to act more like ThreadedW3MMDVarAdd

		for( QMap<quint32,QByteArray> :: const_iterator i = m_PIDToName.begin( ); i != m_PIDToName.end( ); i++ )
		{
			QString Flags = m_Flags[i.key()];
			quint32 Leaver = 0;
			quint32 Practicing = 0;

			if( m_FlagsLeaver.find( i.key() ) != m_FlagsLeaver.end( ) && m_FlagsLeaver[i.key()] )
			{
				Leaver = 1;

				if( !Flags.isEmpty( ) )
					Flags += "/";

				Flags += "leaver";
			}

			if( m_FlagsPracticing.find( i.key() ) != m_FlagsPracticing.end( ) && m_FlagsPracticing[i.key()] )
			{
				Practicing = 1;

				if( !Flags.isEmpty( ) )
					Flags += "/";

				Flags += "practicing";
			}

			CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] recorded flags [" + Flags + "] for player [" + i.value() + "] with PID [" + QString::number( i.key() ) + "]" );
			GHost->m_Callables.push_back( DB->ThreadedW3MMDPlayerAdd( m_Category, GameID, i.key(), i.value(), m_Flags[i.key()], Leaver, Practicing ) );
		}

		if( !m_VarPInts.isEmpty( ) )
			GHost->m_Callables.push_back( DB->ThreadedW3MMDVarAdd( GameID, m_VarPInts ) );

		if( !m_VarPReals.isEmpty( ) )
			GHost->m_Callables.push_back( DB->ThreadedW3MMDVarAdd( GameID, m_VarPReals ) );

		if( !m_VarPStrings.isEmpty( ) )
			GHost->m_Callables.push_back( DB->ThreadedW3MMDVarAdd( GameID, m_VarPStrings ) );

		if( DB->Commit( ) )
			CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] saving data" );
		else
			CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] unable to commit database transaction, data not saved" );
	}
	else
		CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] unable to begin database transaction, data not saved" );
}

QList<QByteArray> CStatsW3MMD :: TokenizeKey( const QByteArray& key )
{
	QList<QByteArray> Tokens;
	QByteArray Token;
	bool Escaping = false;

	for( QByteArray::const_iterator i = key.begin();
		i != key.end();
		i++ )
	{
		if( Escaping )
		{
			if( *i == ' ' )
				Token += ' ';
			else if( *i == '\\' )
				Token += '\\';
			else
			{
				CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] error tokenizing key [" + key + "], invalid escape sequence found, ignoring" );
				return QList<QByteArray>( );
			}

			Escaping = false;
		}
		else
		{
			if( *i == ' ' )
			{
				if( Token.isEmpty( ) )
				{
					CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] error tokenizing key [" + key + "], empty token found, ignoring" );
					return QList<QByteArray>( );
				}

				Tokens.push_back( Token );
				Token.clear( );
			}
			else if( *i == '\\' )
				Escaping = true;
			else
				Token += *i;
		}
	}

	if( Token.isEmpty( ) )
	{
		CONSOLE_Print( "[STATSW3MMD: " + m_Game->GetGameName( ) + "] error tokenizing key [" + key + "], empty token found, ignoring" );
		return QList<QByteArray>( );
	}

	Tokens.push_back( Token );
	return Tokens;
}
