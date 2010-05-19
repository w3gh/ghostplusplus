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

#ifndef GAME_ADMIN_H
#define GAME_ADMIN_H

//
// CAdminGame
//

class CCallableAdminCount;
class CCallableAdminAdd;
class CCallableAdminRemove;
class CCallableBanCount;
// class CCallableBanAdd;
class CCallableBanRemove;

#include "includes.h"
#include "game_base.h"
#include <QTimer>

typedef pair<QString,CCallableAdminCount *> PairedAdminCount;
typedef pair<QString,CCallableAdminAdd *> PairedAdminAdd;
typedef pair<QString,CCallableAdminRemove *> PairedAdminRemove;
typedef pair<QString,CCallableBanCount *> PairedBanCount;
// typedef pair<QString,CCallableBanAdd *> PairedBanAdd;
typedef pair<QString,CCallableBanRemove *> PairedBanRemove;

typedef pair<QString,quint32> TempBan;

class CAdminGame : public CBaseGame
{
	Q_OBJECT

public slots:
	void EventCallableUpdateTimeout();

protected:
	QString m_Password;
	QList<TempBan> m_TempBans;
	QList<PairedAdminCount> m_PairedAdminCounts;	// vector of paired threaded database admin counts in progress
	QList<PairedAdminAdd> m_PairedAdminAdds;		// vector of paired threaded database admin adds in progress
	QList<PairedAdminRemove> m_PairedAdminRemoves;	// vector of paired threaded database admin removes in progress
	QList<PairedBanCount> m_PairedBanCounts;		// vector of paired threaded database ban counts in progress
	// QList<PairedBanAdd> m_PairedBanAdds;		// vector of paired threaded database ban adds in progress
	QList<PairedBanRemove> m_PairedBanRemoves;		// vector of paired threaded database ban removes in progress

public:
	CAdminGame( CGHost *nGHost, CMap *nMap, CSaveGame *nSaveGame, quint16 nHostPort, unsigned char nGameState, QString nGameName, QString nPassword );
	virtual ~CAdminGame( );

	virtual void SendAdminChat( QString message );
	virtual void SendWelcomeMessage( CGamePlayer *player );
	virtual void EventPlayerJoined( CPotentialPlayer *potential, CIncomingJoinPlayer *joinPlayer );
	virtual bool EventPlayerBotCommand( CGamePlayer *player, QString command, QString payload );
};

#endif
