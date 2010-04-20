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
#include "ghostdb.h"

#include <QObject>

//
// CGHostDB
//

CGHostDB :: CGHostDB( CConfig */*CFG*/ )
{
	m_HasError = false;
}

CGHostDB :: ~CGHostDB( )
{

}

void CGHostDB::SetError(const QString &sError)
{
	m_Error = sError;
	m_HasError = true;
	emit error(m_Error);
}

void CGHostDB :: RecoverCallable( CBaseCallable */*callable*/ )
{

}

bool CGHostDB :: Begin( )
{
	return true;
}

bool CGHostDB :: Commit( )
{
	return true;
}

quint32 CGHostDB :: AdminCount( QString /*server*/ )
{
	return 0;
}

bool CGHostDB :: AdminCheck( QString /*server*/, QString /*user*/ )
{
	return false;
}

bool CGHostDB :: AdminAdd( QString /*server*/, QString /*user*/ )
{
	return false;
}

bool CGHostDB :: AdminRemove( QString /*server*/, QString /*user*/ )
{
	return false;
}

QVector<QString> CGHostDB :: AdminList( QString /*server*/ )
{
	return QVector<QString>( );
}

quint32 CGHostDB :: BanCount( QString /*server*/ )
{
	return 0;
}

CDBBan *CGHostDB :: BanCheck( QString /*server*/, QString /*user*/, QString /*ip*/ )
{
	return NULL;
}

bool CGHostDB :: BanAdd( QString /*server*/, QString /*user*/, QString /*ip*/, QString /*gamename*/, QString /*admin*/, QString /*reason*/ )
{
	return false;
}

bool CGHostDB :: BanRemove( QString /*server*/, QString /*user*/ )
{
	return false;
}

bool CGHostDB :: BanRemove( QString /*user*/ )
{
	return false;
}

QVector<CDBBan *> CGHostDB :: BanList( QString /*server*/ )
{
	return QVector<CDBBan *>( );
}

quint32 CGHostDB :: GameAdd( QString /*server*/, QString /*map*/, QString /*gamename*/, QString /*ownername*/, quint32 /*duration*/, quint32 /*gamestate*/, QString /*creatorname*/, QString /*creatorserver*/ )
{
	return 0;
}

quint32 CGHostDB :: GamePlayerAdd( quint32 /*gameid*/, QString /*name*/, QString /*ip*/, quint32 /*spoofed*/, QString /*spoofedrealm*/, quint32 /*reserved*/, quint32 /*loadingtime*/, quint32 /*left*/, QString /*leftreason*/, quint32 /*team*/, quint32 /*colour*/ )
{
	return 0;
}

quint32 CGHostDB :: GamePlayerCount( QString /*name*/ )
{
	return 0;
}

CDBGamePlayerSummary *CGHostDB :: GamePlayerSummaryCheck( QString /*name*/ )
{
	return NULL;
}

quint32 CGHostDB :: DotAGameAdd( quint32 /*gameid*/, quint32 /*winner*/, quint32 /*min*/, quint32 /*sec*/ )
{
	return 0;
}

quint32 CGHostDB :: DotAPlayerAdd( quint32 /*gameid*/, quint32 /*colour*/, quint32 /*kills*/, quint32 /*deaths*/, quint32 /*creepkills*/, quint32 /*creepdenies*/, quint32 /*assists*/, quint32 /*gold*/, quint32 /*neutralkills*/, QString /*item1*/, QString /*item2*/, QString /*item3*/, QString /*item4*/, QString /*item5*/, QString /*item6*/, QString /*hero*/, quint32 /*newcolour*/, quint32 /*towerkills*/, quint32 /*raxkills*/, quint32 /*courierkills*/ )
{
	return 0;
}

quint32 CGHostDB :: DotAPlayerCount( QString /*name*/ )
{
	return 0;
}

CDBDotAPlayerSummary *CGHostDB :: DotAPlayerSummaryCheck( QString /*name*/ )
{
	return NULL;
}

QString CGHostDB :: FromCheck( quint32 /*ip*/ )
{
	return "??";
}

bool CGHostDB :: FromAdd( quint32 /*ip1*/, quint32 /*ip2*/, QString /*country*/ )
{
	return false;
}

bool CGHostDB :: DownloadAdd( QString /*map*/, quint32 /*mapsize*/, QString /*name*/, QString /*ip*/, quint32 /*spoofed*/, QString /*spoofedrealm*/, quint32 /*downloadtime*/ )
{
	return false;
}

quint32 CGHostDB :: W3MMDPlayerAdd( QString /*category*/, quint32 /*gameid*/, quint32 /*pid*/, QString /*name*/, QString /*flag*/, quint32 /*leaver*/, quint32 /*practicing*/ )
{
	return 0;
}

bool CGHostDB :: W3MMDVarAdd( quint32 /*gameid*/, QMap<VarP,int32_t> /*var_ints*/ )
{
	return false;
}

bool CGHostDB :: W3MMDVarAdd( quint32 /*gameid*/, QMap<VarP,double> /*var_reals*/ )
{
	return false;
}

bool CGHostDB :: W3MMDVarAdd( quint32 /*gameid*/, QMap<VarP,QString> /*var_strings*/ )
{
	return false;
}

void CGHostDB :: CreateThread( CBaseCallable *callable )
{
	callable->SetReady( true );
}

CCallableAdminCount *CGHostDB :: ThreadedAdminCount( QString /*server*/ )
{
	return NULL;
}

CCallableAdminCheck *CGHostDB :: ThreadedAdminCheck( QString /*server*/, QString /*user*/ )
{
	return NULL;
}

CCallableAdminAdd *CGHostDB :: ThreadedAdminAdd( QString /*server*/, QString /*user*/ )
{
	return NULL;
}

CCallableAdminRemove *CGHostDB :: ThreadedAdminRemove( QString /*server*/, QString /*user*/ )
{
	return NULL;
}

CCallableAdminList *CGHostDB :: ThreadedAdminList( QString /*server*/ )
{
	return NULL;
}

CCallableBanCount *CGHostDB :: ThreadedBanCount( QString /*server*/ )
{
	return NULL;
}

CCallableBanCheck *CGHostDB :: ThreadedBanCheck( QString /*server*/, QString /*user*/, QString /*ip*/ )
{
	return NULL;
}

CCallableBanAdd *CGHostDB :: ThreadedBanAdd( QString /*server*/, QString /*user*/, QString /*ip*/, QString /*gamename*/, QString /*admin*/, QString /*reason*/ )
{
	return NULL;
}

CCallableBanRemove *CGHostDB :: ThreadedBanRemove( QString /*server*/, QString /*user*/ )
{
	return NULL;
}

CCallableBanRemove *CGHostDB :: ThreadedBanRemove( QString /*user*/ )
{
	return NULL;
}

CCallableBanList *CGHostDB :: ThreadedBanList( QString /*server*/ )
{
	return NULL;
}

CCallableGameAdd *CGHostDB :: ThreadedGameAdd( QString /*server*/, QString /*map*/, QString /*gamename*/, QString /*ownername*/, quint32 /*duration*/, quint32 /*gamestate*/, QString /*creatorname*/, QString /*creatorserver*/ )
{
	return NULL;
}

CCallableGamePlayerAdd *CGHostDB :: ThreadedGamePlayerAdd( quint32 /*gameid*/, QString /*name*/, QString /*ip*/, quint32 /*spoofed*/, QString /*spoofedrealm*/, quint32 /*reserved*/, quint32 /*loadingtime*/, quint32 /*left*/, QString /*leftreason*/, quint32 /*team*/, quint32 /*colour*/ )
{
	return NULL;
}

CCallableGamePlayerSummaryCheck *CGHostDB :: ThreadedGamePlayerSummaryCheck( QString /*name*/ )
{
	return NULL;
}

CCallableDotAGameAdd *CGHostDB :: ThreadedDotAGameAdd( quint32 /*gameid*/, quint32 /*winner*/, quint32 /*min*/, quint32 /*sec*/ )
{
	return NULL;
}

CCallableDotAPlayerAdd *CGHostDB :: ThreadedDotAPlayerAdd( quint32 /*gameid*/, quint32 /*colour*/, quint32 /*kills*/, quint32 /*deaths*/, quint32 /*creepkills*/, quint32 /*creepdenies*/, quint32 /*assists*/, quint32 /*gold*/, quint32 /*neutralkills*/, QString /*item1*/, QString /*item2*/, QString /*item3*/, QString /*item4*/, QString /*item5*/, QString /*item6*/, QString /*hero*/, quint32 /*newcolour*/, quint32 /*towerkills*/, quint32 /*raxkills*/, quint32 /*courierkills*/ )
{
	return NULL;
}

CCallableDotAPlayerSummaryCheck *CGHostDB :: ThreadedDotAPlayerSummaryCheck( QString /*name*/ )
{
	return NULL;
}

CCallableDownloadAdd *CGHostDB :: ThreadedDownloadAdd( QString /*map*/, quint32 /*mapsize*/, QString /*name*/, QString /*ip*/, quint32 /*spoofed*/, QString /*spoofedrealm*/, quint32 /*downloadtime*/ )
{
	return NULL;
}

CCallableScoreCheck *CGHostDB :: ThreadedScoreCheck( QString /*category*/, QString /*name*/, QString /*server*/ )
{
	return NULL;
}

CCallableW3MMDPlayerAdd *CGHostDB :: ThreadedW3MMDPlayerAdd( QString /*category*/, quint32 /*gameid*/, quint32 /*pid*/, QString /*name*/, QString /*flag*/, quint32 /*leaver*/, quint32 /*practicing*/ )
{
	return NULL;
}

CCallableW3MMDVarAdd *CGHostDB :: ThreadedW3MMDVarAdd( quint32 /*gameid*/, QMap<VarP,int32_t> /*var_ints*/ )
{
	return NULL;
}

CCallableW3MMDVarAdd *CGHostDB :: ThreadedW3MMDVarAdd( quint32 /*gameid*/, QMap<VarP,double> /*var_reals*/ )
{
	return NULL;
}

CCallableW3MMDVarAdd *CGHostDB :: ThreadedW3MMDVarAdd( quint32 /*gameid*/, QMap<VarP,QString> /*var_strings*/ )
{
	return NULL;
}

//
// Callables
//

void CBaseCallable :: Init( )
{
	m_StartTicks = GetTicks( );
}

void CBaseCallable :: Close( )
{
	m_EndTicks = GetTicks( );
	m_Ready = true;
}

CCallableAdminCount :: ~CCallableAdminCount( )
{

}

CCallableAdminCheck :: ~CCallableAdminCheck( )
{

}

CCallableAdminAdd :: ~CCallableAdminAdd( )
{

}

CCallableAdminRemove :: ~CCallableAdminRemove( )
{

}

CCallableAdminList :: ~CCallableAdminList( )
{

}

CCallableBanCount :: ~CCallableBanCount( )
{

}

CCallableBanCheck :: ~CCallableBanCheck( )
{
	delete m_Result;
}

CCallableBanAdd :: ~CCallableBanAdd( )
{

}

CCallableBanRemove :: ~CCallableBanRemove( )
{

}

CCallableBanList :: ~CCallableBanList( )
{
	// don't delete anything in m_Result here, it's the caller's responsibility
}

CCallableGameAdd :: ~CCallableGameAdd( )
{

}

CCallableGamePlayerAdd :: ~CCallableGamePlayerAdd( )
{

}

CCallableGamePlayerSummaryCheck :: ~CCallableGamePlayerSummaryCheck( )
{
	delete m_Result;
}

CCallableDotAGameAdd :: ~CCallableDotAGameAdd( )
{

}

CCallableDotAPlayerAdd :: ~CCallableDotAPlayerAdd( )
{

}

CCallableDotAPlayerSummaryCheck :: ~CCallableDotAPlayerSummaryCheck( )
{
	delete m_Result;
}

CCallableDownloadAdd :: ~CCallableDownloadAdd( )
{

}

CCallableScoreCheck :: ~CCallableScoreCheck( )
{

}

CCallableW3MMDPlayerAdd :: ~CCallableW3MMDPlayerAdd( )
{

}

CCallableW3MMDVarAdd :: ~CCallableW3MMDVarAdd( )
{

}

//
// CDBBan
//

CDBBan :: CDBBan( QString nServer, QString nName, QString nIP, QString nDate, QString nGameName, QString nAdmin, QString nReason )
{
	m_Server = nServer;
	m_Name = nName;
	m_IP = nIP;
	m_Date = nDate;
	m_GameName = nGameName;
	m_Admin = nAdmin;
	m_Reason = nReason;
}

CDBBan :: ~CDBBan( )
{

}

//
// CDBGame
//

CDBGame :: CDBGame( quint32 nID, QString nServer, QString nMap, QString nDateTime, QString nGameName, QString nOwnerName, quint32 nDuration )
{
	m_ID = nID;
	m_Server = nServer;
	m_Map = nMap;
	m_DateTime = nDateTime;
	m_GameName = nGameName;
	m_OwnerName = nOwnerName;
	m_Duration = nDuration;
}

CDBGame :: ~CDBGame( )
{

}

//
// CDBGamePlayer
//

CDBGamePlayer :: CDBGamePlayer( quint32 nID, quint32 nGameID, QString nName, QString nIP, quint32 nSpoofed, QString nSpoofedRealm, quint32 nReserved, quint32 nLoadingTime, quint32 nLeft, QString nLeftReason, quint32 nTeam, quint32 nColour )
{
	m_ID = nID;
	m_GameID = nGameID;
	m_Name = nName;
	m_IP = nIP;
	m_Spoofed = nSpoofed;
	m_SpoofedRealm = nSpoofedRealm;
	m_Reserved = nReserved;
	m_LoadingTime = nLoadingTime;
	m_Left = nLeft;
	m_LeftReason = nLeftReason;
	m_Team = nTeam;
	m_Colour = nColour;
}

CDBGamePlayer :: ~CDBGamePlayer( )
{

}

//
// CDBGamePlayerSummary
//

CDBGamePlayerSummary :: CDBGamePlayerSummary( QString nServer, QString nName, QString nFirstGameDateTime, QString nLastGameDateTime, quint32 nTotalGames, quint32 nMinLoadingTime, quint32 nAvgLoadingTime, quint32 nMaxLoadingTime, quint32 nMinLeftPercent, quint32 nAvgLeftPercent, quint32 nMaxLeftPercent, quint32 nMinDuration, quint32 nAvgDuration, quint32 nMaxDuration )
{
	m_Server = nServer;
	m_Name = nName;
	m_FirstGameDateTime = nFirstGameDateTime;
	m_LastGameDateTime = nLastGameDateTime;
	m_TotalGames = nTotalGames;
	m_MinLoadingTime = nMinLoadingTime;
	m_AvgLoadingTime = nAvgLoadingTime;
	m_MaxLoadingTime = nMaxLoadingTime;
	m_MinLeftPercent = nMinLeftPercent;
	m_AvgLeftPercent = nAvgLeftPercent;
	m_MaxLeftPercent = nMaxLeftPercent;
	m_MinDuration = nMinDuration;
	m_AvgDuration = nAvgDuration;
	m_MaxDuration = nMaxDuration;
}

CDBGamePlayerSummary :: ~CDBGamePlayerSummary( )
{

}

//
// CDBDotAGame
//

CDBDotAGame :: CDBDotAGame( quint32 nID, quint32 nGameID, quint32 nWinner, quint32 nMin, quint32 nSec )
{
	m_ID = nID;
	m_GameID = nGameID;
	m_Winner = nWinner;
	m_Min = nMin;
	m_Sec = nSec;
}

CDBDotAGame :: ~CDBDotAGame( )
{

}

//
// CDBDotAPlayer
//

CDBDotAPlayer :: CDBDotAPlayer( )
{
	m_ID = 0;
	m_GameID = 0;
	m_Colour = 0;
	m_Kills = 0;
	m_Deaths = 0;
	m_CreepKills = 0;
	m_CreepDenies = 0;
	m_Assists = 0;
	m_Gold = 0;
	m_NeutralKills = 0;
	m_NewColour = 0;
	m_TowerKills = 0;
	m_RaxKills = 0;
	m_CourierKills = 0;
}

CDBDotAPlayer :: CDBDotAPlayer( quint32 nID, quint32 nGameID, quint32 nColour, quint32 nKills, quint32 nDeaths, quint32 nCreepKills, quint32 nCreepDenies, quint32 nAssists, quint32 nGold, quint32 nNeutralKills, QString nItem1, QString nItem2, QString nItem3, QString nItem4, QString nItem5, QString nItem6, QString nHero, quint32 nNewColour, quint32 nTowerKills, quint32 nRaxKills, quint32 nCourierKills )
{
	m_ID = nID;
	m_GameID = nGameID;
	m_Colour = nColour;
	m_Kills = nKills;
	m_Deaths = nDeaths;
	m_CreepKills = nCreepKills;
	m_CreepDenies = nCreepDenies;
	m_Assists = nAssists;
	m_Gold = nGold;
	m_NeutralKills = nNeutralKills;
	m_Items[0] = nItem1;
	m_Items[1] = nItem2;
	m_Items[2] = nItem3;
	m_Items[3] = nItem4;
	m_Items[4] = nItem5;
	m_Items[5] = nItem6;
	m_Hero = nHero;
	m_NewColour = nNewColour;
	m_TowerKills = nTowerKills;
	m_RaxKills = nRaxKills;
	m_CourierKills = nCourierKills;
}

CDBDotAPlayer :: ~CDBDotAPlayer( )
{

}

QString CDBDotAPlayer :: GetItem( unsigned int i )
{
	if( i < 6 )
		return m_Items[i];

	return QString( );
}

void CDBDotAPlayer :: SetItem( unsigned int i, QString item )
{
	if( i < 6 )
		m_Items[i] = item;
}

//
// CDBDotAPlayerSummary
//

CDBDotAPlayerSummary :: CDBDotAPlayerSummary( QString nServer, QString nName, quint32 nTotalGames, quint32 nTotalWins, quint32 nTotalLosses, quint32 nTotalKills, quint32 nTotalDeaths, quint32 nTotalCreepKills, quint32 nTotalCreepDenies, quint32 nTotalAssists, quint32 nTotalNeutralKills, quint32 nTotalTowerKills, quint32 nTotalRaxKills, quint32 nTotalCourierKills )
{
	m_Server = nServer;
	m_Name = nName;
	m_TotalGames = nTotalGames;
	m_TotalWins = nTotalWins;
	m_TotalLosses = nTotalLosses;
	m_TotalKills = nTotalKills;
	m_TotalDeaths = nTotalDeaths;
	m_TotalCreepKills = nTotalCreepKills;
	m_TotalCreepDenies = nTotalCreepDenies;
	m_TotalAssists = nTotalAssists;
	m_TotalNeutralKills = nTotalNeutralKills;
	m_TotalTowerKills = nTotalTowerKills;
	m_TotalRaxKills = nTotalRaxKills;
	m_TotalCourierKills = nTotalCourierKills;
}

CDBDotAPlayerSummary :: ~CDBDotAPlayerSummary( )
{

}
