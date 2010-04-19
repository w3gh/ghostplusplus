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

//
// CGHostDB
//

CGHostDB :: CGHostDB( CConfig *CFG )
{
	m_HasError = false;
}

CGHostDB :: ~CGHostDB( )
{

}

void CGHostDB :: RecoverCallable( CBaseCallable *callable )
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

uint32_t CGHostDB :: AdminCount( QString server )
{
	return 0;
}

bool CGHostDB :: AdminCheck( QString server, QString user )
{
	return false;
}

bool CGHostDB :: AdminAdd( QString server, QString user )
{
	return false;
}

bool CGHostDB :: AdminRemove( QString server, QString user )
{
	return false;
}

QVector<QString> CGHostDB :: AdminList( QString server )
{
	return QVector<QString>( );
}

uint32_t CGHostDB :: BanCount( QString server )
{
	return 0;
}

CDBBan *CGHostDB :: BanCheck( QString server, QString user, QString ip )
{
	return NULL;
}

bool CGHostDB :: BanAdd( QString server, QString user, QString ip, QString gamename, QString admin, QString reason )
{
	return false;
}

bool CGHostDB :: BanRemove( QString server, QString user )
{
	return false;
}

bool CGHostDB :: BanRemove( QString user )
{
	return false;
}

QVector<CDBBan *> CGHostDB :: BanList( QString server )
{
	return QVector<CDBBan *>( );
}

uint32_t CGHostDB :: GameAdd( QString server, QString map, QString gamename, QString ownername, uint32_t duration, uint32_t gamestate, QString creatorname, QString creatorserver )
{
	return 0;
}

uint32_t CGHostDB :: GamePlayerAdd( uint32_t gameid, QString name, QString ip, uint32_t spoofed, QString spoofedrealm, uint32_t reserved, uint32_t loadingtime, uint32_t left, QString leftreason, uint32_t team, uint32_t colour )
{
	return 0;
}

uint32_t CGHostDB :: GamePlayerCount( QString name )
{
	return 0;
}

CDBGamePlayerSummary *CGHostDB :: GamePlayerSummaryCheck( QString name )
{
	return NULL;
}

uint32_t CGHostDB :: DotAGameAdd( uint32_t gameid, uint32_t winner, uint32_t min, uint32_t sec )
{
	return 0;
}

uint32_t CGHostDB :: DotAPlayerAdd( uint32_t gameid, uint32_t colour, uint32_t kills, uint32_t deaths, uint32_t creepkills, uint32_t creepdenies, uint32_t assists, uint32_t gold, uint32_t neutralkills, QString item1, QString item2, QString item3, QString item4, QString item5, QString item6, QString hero, uint32_t newcolour, uint32_t towerkills, uint32_t raxkills, uint32_t courierkills )
{
	return 0;
}

uint32_t CGHostDB :: DotAPlayerCount( QString name )
{
	return 0;
}

CDBDotAPlayerSummary *CGHostDB :: DotAPlayerSummaryCheck( QString name )
{
	return NULL;
}

QString CGHostDB :: FromCheck( uint32_t ip )
{
	return "??";
}

bool CGHostDB :: FromAdd( uint32_t ip1, uint32_t ip2, QString country )
{
	return false;
}

bool CGHostDB :: DownloadAdd( QString map, uint32_t mapsize, QString name, QString ip, uint32_t spoofed, QString spoofedrealm, uint32_t downloadtime )
{
	return false;
}

uint32_t CGHostDB :: W3MMDPlayerAdd( QString category, uint32_t gameid, uint32_t pid, QString name, QString flag, uint32_t leaver, uint32_t practicing )
{
	return 0;
}

bool CGHostDB :: W3MMDVarAdd( uint32_t gameid, QMap<VarP,int32_t> var_ints )
{
	return false;
}

bool CGHostDB :: W3MMDVarAdd( uint32_t gameid, QMap<VarP,double> var_reals )
{
	return false;
}

bool CGHostDB :: W3MMDVarAdd( uint32_t gameid, QMap<VarP,QString> var_strings )
{
	return false;
}

void CGHostDB :: CreateThread( CBaseCallable *callable )
{
	callable->SetReady( true );
}

CCallableAdminCount *CGHostDB :: ThreadedAdminCount( QString server )
{
	return NULL;
}

CCallableAdminCheck *CGHostDB :: ThreadedAdminCheck( QString server, QString user )
{
	return NULL;
}

CCallableAdminAdd *CGHostDB :: ThreadedAdminAdd( QString server, QString user )
{
	return NULL;
}

CCallableAdminRemove *CGHostDB :: ThreadedAdminRemove( QString server, QString user )
{
	return NULL;
}

CCallableAdminList *CGHostDB :: ThreadedAdminList( QString server )
{
	return NULL;
}

CCallableBanCount *CGHostDB :: ThreadedBanCount( QString server )
{
	return NULL;
}

CCallableBanCheck *CGHostDB :: ThreadedBanCheck( QString server, QString user, QString ip )
{
	return NULL;
}

CCallableBanAdd *CGHostDB :: ThreadedBanAdd( QString server, QString user, QString ip, QString gamename, QString admin, QString reason )
{
	return NULL;
}

CCallableBanRemove *CGHostDB :: ThreadedBanRemove( QString server, QString user )
{
	return NULL;
}

CCallableBanRemove *CGHostDB :: ThreadedBanRemove( QString user )
{
	return NULL;
}

CCallableBanList *CGHostDB :: ThreadedBanList( QString server )
{
	return NULL;
}

CCallableGameAdd *CGHostDB :: ThreadedGameAdd( QString server, QString map, QString gamename, QString ownername, uint32_t duration, uint32_t gamestate, QString creatorname, QString creatorserver )
{
	return NULL;
}

CCallableGamePlayerAdd *CGHostDB :: ThreadedGamePlayerAdd( uint32_t gameid, QString name, QString ip, uint32_t spoofed, QString spoofedrealm, uint32_t reserved, uint32_t loadingtime, uint32_t left, QString leftreason, uint32_t team, uint32_t colour )
{
	return NULL;
}

CCallableGamePlayerSummaryCheck *CGHostDB :: ThreadedGamePlayerSummaryCheck( QString name )
{
	return NULL;
}

CCallableDotAGameAdd *CGHostDB :: ThreadedDotAGameAdd( uint32_t gameid, uint32_t winner, uint32_t min, uint32_t sec )
{
	return NULL;
}

CCallableDotAPlayerAdd *CGHostDB :: ThreadedDotAPlayerAdd( uint32_t gameid, uint32_t colour, uint32_t kills, uint32_t deaths, uint32_t creepkills, uint32_t creepdenies, uint32_t assists, uint32_t gold, uint32_t neutralkills, QString item1, QString item2, QString item3, QString item4, QString item5, QString item6, QString hero, uint32_t newcolour, uint32_t towerkills, uint32_t raxkills, uint32_t courierkills )
{
	return NULL;
}

CCallableDotAPlayerSummaryCheck *CGHostDB :: ThreadedDotAPlayerSummaryCheck( QString name )
{
	return NULL;
}

CCallableDownloadAdd *CGHostDB :: ThreadedDownloadAdd( QString map, uint32_t mapsize, QString name, QString ip, uint32_t spoofed, QString spoofedrealm, uint32_t downloadtime )
{
	return NULL;
}

CCallableScoreCheck *CGHostDB :: ThreadedScoreCheck( QString category, QString name, QString server )
{
	return NULL;
}

CCallableW3MMDPlayerAdd *CGHostDB :: ThreadedW3MMDPlayerAdd( QString category, uint32_t gameid, uint32_t pid, QString name, QString flag, uint32_t leaver, uint32_t practicing )
{
	return NULL;
}

CCallableW3MMDVarAdd *CGHostDB :: ThreadedW3MMDVarAdd( uint32_t gameid, QMap<VarP,int32_t> var_ints )
{
	return NULL;
}

CCallableW3MMDVarAdd *CGHostDB :: ThreadedW3MMDVarAdd( uint32_t gameid, QMap<VarP,double> var_reals )
{
	return NULL;
}

CCallableW3MMDVarAdd *CGHostDB :: ThreadedW3MMDVarAdd( uint32_t gameid, QMap<VarP,QString> var_strings )
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

CDBGame :: CDBGame( uint32_t nID, QString nServer, QString nMap, QString nDateTime, QString nGameName, QString nOwnerName, uint32_t nDuration )
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

CDBGamePlayer :: CDBGamePlayer( uint32_t nID, uint32_t nGameID, QString nName, QString nIP, uint32_t nSpoofed, QString nSpoofedRealm, uint32_t nReserved, uint32_t nLoadingTime, uint32_t nLeft, QString nLeftReason, uint32_t nTeam, uint32_t nColour )
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

CDBGamePlayerSummary :: CDBGamePlayerSummary( QString nServer, QString nName, QString nFirstGameDateTime, QString nLastGameDateTime, uint32_t nTotalGames, uint32_t nMinLoadingTime, uint32_t nAvgLoadingTime, uint32_t nMaxLoadingTime, uint32_t nMinLeftPercent, uint32_t nAvgLeftPercent, uint32_t nMaxLeftPercent, uint32_t nMinDuration, uint32_t nAvgDuration, uint32_t nMaxDuration )
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

CDBDotAGame :: CDBDotAGame( uint32_t nID, uint32_t nGameID, uint32_t nWinner, uint32_t nMin, uint32_t nSec )
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

CDBDotAPlayer :: CDBDotAPlayer( uint32_t nID, uint32_t nGameID, uint32_t nColour, uint32_t nKills, uint32_t nDeaths, uint32_t nCreepKills, uint32_t nCreepDenies, uint32_t nAssists, uint32_t nGold, uint32_t nNeutralKills, QString nItem1, QString nItem2, QString nItem3, QString nItem4, QString nItem5, QString nItem6, QString nHero, uint32_t nNewColour, uint32_t nTowerKills, uint32_t nRaxKills, uint32_t nCourierKills )
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

CDBDotAPlayerSummary :: CDBDotAPlayerSummary( QString nServer, QString nName, uint32_t nTotalGames, uint32_t nTotalWins, uint32_t nTotalLosses, uint32_t nTotalKills, uint32_t nTotalDeaths, uint32_t nTotalCreepKills, uint32_t nTotalCreepDenies, uint32_t nTotalAssists, uint32_t nTotalNeutralKills, uint32_t nTotalTowerKills, uint32_t nTotalRaxKills, uint32_t nTotalCourierKills )
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
