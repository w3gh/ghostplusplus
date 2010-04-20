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

#ifndef GAME_H
#define GAME_H

//
// CGame
//

#include "includes.h"

class CDBBan;
class CDBGame;
class CDBGamePlayer;
class CStats;
class CCallableBanCheck;
class CCallableBanAdd;
class CCallableGameAdd;
class CCallableGamePlayerSummaryCheck;
class CCallableDotAPlayerSummaryCheck;

typedef pair<QString,CCallableBanCheck *> PairedBanCheck;
typedef pair<QString,CCallableBanAdd *> PairedBanAdd;
typedef pair<QString,CCallableGamePlayerSummaryCheck *> PairedGPSCheck;
typedef pair<QString,CCallableDotAPlayerSummaryCheck *> PairedDPSCheck;

#include <QObject>
#include "game_base.h"

class CGame : public CBaseGame
{
	Q_OBJECT

public slots:
	virtual void EventPlayerDeleted();
	void EventCallableUpdateTimeout();

protected:
	CDBBan *m_DBBanLast;						// last ban for the !banlast command - this is a pointer to one of the items in m_DBBans
	QVector<CDBBan *> m_DBBans;					// vector of potential ban data for the database (see the Update function for more info, it's not as straightforward as you might think)
	CDBGame *m_DBGame;							// potential game data for the database
	QVector<CDBGamePlayer *> m_DBGamePlayers;	// vector of potential gameplayer data for the database
	CStats *m_Stats;							// class to keep track of game stats such as kills/deaths/assists in dota
	CCallableGameAdd *m_CallableGameAdd;		// threaded database game addition in progress
	QVector<PairedBanCheck> m_PairedBanChecks;	// vector of paired threaded database ban checks in progress
	QVector<PairedBanAdd> m_PairedBanAdds;		// vector of paired threaded database ban adds in progress
	QVector<PairedGPSCheck> m_PairedGPSChecks;	// vector of paired threaded database game player summary checks in progress
	QVector<PairedDPSCheck> m_PairedDPSChecks;	// vector of paired threaded database DotA player summary checks in progress

public:
	CGame( CGHost *nGHost, CMap *nMap, CSaveGame *nSaveGame, uint16_t nHostPort, unsigned char nGameState, QString nGameName, QString nOwnerName, QString nCreatorName, QString nCreatorServer );
	virtual ~CGame( );

	virtual void EventPlayerAction( CGamePlayer *player, CIncomingAction *action );
	virtual bool EventPlayerBotCommand( CGamePlayer *player, QString command, QString payload );
	virtual void EventGameStarted( );
	virtual bool IsGameDataSaved( );
	virtual void SaveGameData( );
};

#endif
