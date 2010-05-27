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

#ifndef GHOSTDB_H
#define GHOSTDB_H

//
// CGHostDB
//


class CBaseCallable;
class CCallableAdminCount;
class CCallableAdminCheck;
class CCallableAdminAdd;
class CCallableAdminRemove;
class CCallableAdminList;
class CCallableBanCount;
class CCallableBanCheck;
class CCallableBanAdd;
class CCallableBanRemove;
class CCallableBanList;
class CCallableGameAdd;
class CCallableGamePlayerAdd;
class CCallableGamePlayerSummaryCheck;
class CCallableDotAGameAdd;
class CCallableDotAPlayerAdd;
class CCallableDotAPlayerSummaryCheck;
class CCallableDownloadAdd;
class CCallableScoreCheck;
class CCallableW3MMDPlayerAdd;
class CCallableW3MMDVarAdd;
class CDBBan;
class CDBGame;
class CDBGamePlayer;
class CDBGamePlayerSummary;
class CDBDotAPlayerSummary;
class CConfig;

#include "includes.h"
#include <QPair>

typedef QPair<quint32,QByteArray> VarP;

class CGHostDB : public QObject
{
	Q_OBJECT

signals:
	void error(const QString& message);

private:
	bool m_HasError;
	QString m_Error;

protected:
	void SetError(const QString& error);

public:
	CGHostDB( CConfig *CFG );
	virtual ~CGHostDB( );

	bool HasError( )			{ return m_HasError; }
	QString GetError( )			{ return m_Error; }
	virtual QString GetStatus( )	{ return "DB STATUS --- OK"; }

	virtual void RecoverCallable( CBaseCallable *callable );

	// standard (non-threaded) database functions

	virtual bool Begin( );
	virtual bool Commit( );
	virtual quint32 AdminCount( QString server );
	virtual bool AdminCheck( QString server, QString user );
	virtual bool AdminAdd( QString server, QString user );
	virtual bool AdminRemove( QString server, QString user );
	virtual QList<QString> AdminList( QString server );
	virtual quint32 BanCount( QString server );
	virtual CDBBan *BanCheck( QString server, QString user, QString ip );
	virtual bool BanAdd( QString server, QString user, QString ip, QString gamename, QString admin, QString reason );
	virtual bool BanRemove( QString server, QString user );
	virtual bool BanRemove( QString user );
	virtual QList<CDBBan *> BanList( QString server );
	virtual quint32 GameAdd( QString server, QString map, QString gamename, QString ownername, quint32 duration, quint32 gamestate, QString creatorname, QString creatorserver );
	virtual quint32 GamePlayerAdd( quint32 gameid, QString name, QString ip, quint32 spoofed, QString spoofedrealm, quint32 reserved, quint32 loadingtime, quint32 left, QString leftreason, quint32 team, quint32 colour );
	virtual quint32 GamePlayerCount( QString name );
	virtual CDBGamePlayerSummary *GamePlayerSummaryCheck( QString name );
	virtual quint32 DotAGameAdd( quint32 gameid, quint32 winner, quint32 min, quint32 sec );
	virtual quint32 DotAPlayerAdd( quint32 gameid, quint32 colour, quint32 kills, quint32 deaths, quint32 creepkills, quint32 creepdenies, quint32 assists, quint32 gold, quint32 neutralkills, QString item1, QString item2, QString item3, QString item4, QString item5, QString item6, QString hero, quint32 newcolour, quint32 towerkills, quint32 raxkills, quint32 courierkills );
	virtual quint32 DotAPlayerCount( QString name );
	virtual CDBDotAPlayerSummary *DotAPlayerSummaryCheck( QString name );
	virtual QString FromCheck( quint32 ip );
	virtual bool FromAdd( quint32 ip1, quint32 ip2, QString country );
	virtual bool DownloadAdd( QString map, quint32 mapsize, QString name, QString ip, quint32 spoofed, QString spoofedrealm, quint32 downloadtime );
	virtual quint32 W3MMDPlayerAdd( QString category, quint32 gameid, quint32 pid, QString name, QString flag, quint32 leaver, quint32 practicing );
	virtual bool W3MMDVarAdd( quint32 gameid, QMap<VarP,quint32> var_ints );
	virtual bool W3MMDVarAdd( quint32 gameid, QMap<VarP,double> var_reals );
	virtual bool W3MMDVarAdd( quint32 gameid, QMap<VarP,QString> var_strings );

	// threaded database functions

	virtual void CreateThread( CBaseCallable *callable );
	virtual CCallableAdminCount *ThreadedAdminCount( QString server );
	virtual CCallableAdminCheck *ThreadedAdminCheck( QString server, QString user );
	virtual CCallableAdminAdd *ThreadedAdminAdd( QString server, QString user );
	virtual CCallableAdminRemove *ThreadedAdminRemove( QString server, QString user );
	virtual CCallableAdminList *ThreadedAdminList( QString server );
	virtual CCallableBanCount *ThreadedBanCount( QString server );
	virtual CCallableBanCheck *ThreadedBanCheck( QString server, QString user, QString ip );
	virtual CCallableBanAdd *ThreadedBanAdd( QString server, QString user, QString ip, QString gamename, QString admin, QString reason );
	virtual CCallableBanRemove *ThreadedBanRemove( QString server, QString user );
	virtual CCallableBanRemove *ThreadedBanRemove( QString user );
	virtual CCallableBanList *ThreadedBanList( QString server );
	virtual CCallableGameAdd *ThreadedGameAdd( QString server, QString map, QString gamename, QString ownername, quint32 duration, quint32 gamestate, QString creatorname, QString creatorserver );
	virtual CCallableGamePlayerAdd *ThreadedGamePlayerAdd( quint32 gameid, QString name, QString ip, quint32 spoofed, QString spoofedrealm, quint32 reserved, quint32 loadingtime, quint32 left, QString leftreason, quint32 team, quint32 colour );
	virtual CCallableGamePlayerSummaryCheck *ThreadedGamePlayerSummaryCheck( QString name );
	virtual CCallableDotAGameAdd *ThreadedDotAGameAdd( quint32 gameid, quint32 winner, quint32 min, quint32 sec );
	virtual CCallableDotAPlayerAdd *ThreadedDotAPlayerAdd( quint32 gameid, quint32 colour, quint32 kills, quint32 deaths, quint32 creepkills, quint32 creepdenies, quint32 assists, quint32 gold, quint32 neutralkills, QString item1, QString item2, QString item3, QString item4, QString item5, QString item6, QString hero, quint32 newcolour, quint32 towerkills, quint32 raxkills, quint32 courierkills );
	virtual CCallableDotAPlayerSummaryCheck *ThreadedDotAPlayerSummaryCheck( QString name );
	virtual CCallableDownloadAdd *ThreadedDownloadAdd( QString map, quint32 mapsize, QString name, QString ip, quint32 spoofed, QString spoofedrealm, quint32 downloadtime );
	virtual CCallableScoreCheck *ThreadedScoreCheck( QString category, QString name, QString server );
	virtual CCallableW3MMDPlayerAdd *ThreadedW3MMDPlayerAdd( QString category, quint32 gameid, quint32 pid, QString name, QString flag, quint32 leaver, quint32 practicing );
	virtual CCallableW3MMDVarAdd *ThreadedW3MMDVarAdd( quint32 gameid, QMap<VarP,quint32> var_ints );
	virtual CCallableW3MMDVarAdd *ThreadedW3MMDVarAdd( quint32 gameid, QMap<VarP,double> var_reals );
	virtual CCallableW3MMDVarAdd *ThreadedW3MMDVarAdd( quint32 gameid, QMap<VarP,QByteArray> var_strings );
};

//
// Callables
//

// life cycle of a callable:
//  - the callable is created in one of the database's ThreadedXXX functions
//  - initially the callable is NOT ready (i.e. m_Ready = false)
//  - the ThreadedXXX function normally creates a thread to perform some query and (potentially) store some result in the callable
//  - at the time of this writing all threads are immediately detached, the code does not join any threads (the callable's "readiness" is used for this purpose instead)
//  - when the thread completes it will set m_Ready = true
//  - DO NOT DO *ANYTHING* TO THE CALLABLE UNTIL IT'S READY OR YOU WILL CREATE A CONCURRENCY MESS
//  - THE ONLY SAFE FUNCTION IN THE CALLABLE IS GetReady
//  - when the callable is ready you may access the callable's result which will have been set within the (now terminated) thread

// example usage:
//  - normally you will call a ThreadedXXX function, store the callable in a vector, and periodically check if the callable is ready
//  - when the callable is ready you will consume the result then you will pass the callable back to the database via the RecoverCallable function
//  - the RecoverCallable function allows the database to recover some of the callable's resources to be reused later (e.g. MySQL connections)
//  - note that this will NOT free the callable's memory, you must do that yourself after calling the RecoverCallable function
//  - be careful not to leak any callables, it's NOT safe to delete a callable even if you decide that you don't want the result anymore
//  - you should deliver any to-be-orphaned callables to the main vector in CGHost so they can be properly deleted when ready even if you don't care about the result anymore
//  - e.g. if a player does a stats check immediately before a game is deleted you can't just delete the callable on game deletion unless it's ready

class CBaseCallable : public QObject
{
	Q_OBJECT

signals:
	void finished();

protected:
	QString m_Error;

private:
	volatile bool m_Ready;
	quint32 m_StartTicks;
	quint32 m_EndTicks;

public:
	CBaseCallable( ) : m_Error( ), m_Ready( false ), m_StartTicks( 0 ), m_EndTicks( 0 ) { }
	virtual ~CBaseCallable( ) { }

	virtual void operator( )( ) { }

	virtual void Init( );
	virtual void Close( );

	virtual const QString &GetError( ) const		{ return m_Error; }
	virtual bool GetReady( ) const			{ return m_Ready; }
	virtual void SetReady( bool nReady )	{ m_Ready = nReady; if (nReady) emit finished(); }
	virtual quint32 GetElapsed( ) const		{ return m_Ready ? m_EndTicks - m_StartTicks : 0; }
};

class CCallableAdminCount : public CBaseCallable
{
protected:
	QString m_Server;
	quint32 m_Result;

public:
	CCallableAdminCount( QString nServer ) : CBaseCallable( ), m_Server( nServer ), m_Result( 0 ) { }
	virtual ~CCallableAdminCount( );

	virtual const QString &GetServer( ) const	{ return m_Server; }
	virtual quint32 GetResult( ) const			{ return m_Result; }
	virtual void SetResult( quint32 nResult )	{ m_Result = nResult; }
};

class CCallableAdminCheck : public CBaseCallable
{
protected:
	QString m_Server;
	QString m_User;
	bool m_Result;

public:
	CCallableAdminCheck( QString nServer, QString nUser ) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_Result( false ) { }
	virtual ~CCallableAdminCheck( );

	virtual QString GetServer( )				{ return m_Server; }
	virtual QString GetUser( )				{ return m_User; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableAdminAdd : public CBaseCallable
{
protected:
	QString m_Server;
	QString m_User;
	bool m_Result;

public:
	CCallableAdminAdd( QString nServer, QString nUser ) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_Result( false ) { }
	virtual ~CCallableAdminAdd( );

	virtual QString GetServer( )				{ return m_Server; }
	virtual QString GetUser( )				{ return m_User; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableAdminRemove : public CBaseCallable
{
protected:
	QString m_Server;
	QString m_User;
	bool m_Result;

public:
	CCallableAdminRemove( QString nServer, QString nUser ) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_Result( false ) { }
	virtual ~CCallableAdminRemove( );

	virtual QString GetServer( )				{ return m_Server; }
	virtual QString GetUser( )				{ return m_User; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableAdminList : public CBaseCallable
{
protected:
	QString m_Server;
	QList<QString> m_Result;

public:
	CCallableAdminList( QString nServer ) : CBaseCallable( ), m_Server( nServer ) { }
	virtual ~CCallableAdminList( );

	virtual QList<QString> GetResult( )					{ return m_Result; }
	virtual void SetResult( QList<QString> nResult )	{ m_Result = nResult; }
};

class CCallableBanCount : public CBaseCallable
{
protected:
	QString m_Server;
	quint32 m_Result;

public:
	CCallableBanCount( QString nServer ) : CBaseCallable( ), m_Server( nServer ), m_Result( 0 ) { }
	virtual ~CCallableBanCount( );

	virtual QString GetServer( )					{ return m_Server; }
	virtual quint32 GetResult( )				{ return m_Result; }
	virtual void SetResult( quint32 nResult )	{ m_Result = nResult; }
};

class CCallableBanCheck : public CBaseCallable
{
protected:
	QString m_Server;
	QString m_User;
	QString m_IP;
	CDBBan *m_Result;

public:
	CCallableBanCheck( QString nServer, QString nUser, QString nIP ) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_IP( nIP ), m_Result( NULL ) { }
	virtual ~CCallableBanCheck( );

	virtual QString GetServer( )					{ return m_Server; }
	virtual QString GetUser( )					{ return m_User; }
	virtual QString GetIP( )						{ return m_IP; }
	virtual CDBBan *GetResult( )				{ return m_Result; }
	virtual void SetResult( CDBBan *nResult )	{ m_Result = nResult; }
};

class CCallableBanAdd : public CBaseCallable
{
protected:
	QString m_Server;
	QString m_User;
	QString m_IP;
	QString m_GameName;
	QString m_Admin;
	QString m_Reason;
	bool m_Result;

public:
	CCallableBanAdd( QString nServer, QString nUser, QString nIP, QString nGameName, QString nAdmin, QString nReason ) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_IP( nIP ), m_GameName( nGameName ), m_Admin( nAdmin ), m_Reason( nReason ), m_Result( false ) { }
	virtual ~CCallableBanAdd( );

	virtual QString GetServer( )				{ return m_Server; }
	virtual QString GetUser( )				{ return m_User; }
	virtual QString GetIP( )					{ return m_IP; }
	virtual QString GetGameName( )			{ return m_GameName; }
	virtual QString GetAdmin( )				{ return m_Admin; }
	virtual QString GetReason( )				{ return m_Reason; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableBanRemove : public CBaseCallable
{
protected:
	QString m_Server;
	QString m_User;
	bool m_Result;

public:
	CCallableBanRemove( QString nServer, QString nUser ) : CBaseCallable( ), m_Server( nServer ), m_User( nUser ), m_Result( false ) { }
	virtual ~CCallableBanRemove( );

	virtual QString GetServer( )				{ return m_Server; }
	virtual QString GetUser( )				{ return m_User; }
	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableBanList : public CBaseCallable
{
protected:
	QString m_Server;
	QList<CDBBan *> m_Result;

public:
	CCallableBanList( QString nServer ) : CBaseCallable( ), m_Server( nServer ) { }
	virtual ~CCallableBanList( );

	virtual QList<CDBBan *> GetResult( )				{ return m_Result; }
	virtual void SetResult( QList<CDBBan *> nResult )	{ m_Result = nResult; }
};

class CCallableGameAdd : public CBaseCallable
{
protected:
	QString m_Server;
	QString m_Map;
	QString m_GameName;
	QString m_OwnerName;
	quint32 m_Duration;
	quint32 m_GameState;
	QString m_CreatorName;
	QString m_CreatorServer;
	quint32 m_Result;

public:
	CCallableGameAdd( QString nServer, QString nMap, QString nGameName, QString nOwnerName, quint32 nDuration, quint32 nGameState, QString nCreatorName, QString nCreatorServer ) : CBaseCallable( ), m_Server( nServer ), m_Map( nMap ), m_GameName( nGameName ), m_OwnerName( nOwnerName ), m_Duration( nDuration ), m_GameState( nGameState ), m_CreatorName( nCreatorName ), m_CreatorServer( nCreatorServer ), m_Result( 0 ) { }
	virtual ~CCallableGameAdd( );

	virtual quint32 GetResult( )				{ return m_Result; }
	virtual void SetResult( quint32 nResult )	{ m_Result = nResult; }
};

class CCallableGamePlayerAdd : public CBaseCallable
{
protected:
	quint32 m_GameID;
	QString m_Name;
	QString m_IP;
	quint32 m_Spoofed;
	QString m_SpoofedRealm;
	quint32 m_Reserved;
	quint32 m_LoadingTime;
	quint32 m_Left;
	QString m_LeftReason;
	quint32 m_Team;
	quint32 m_Colour;
	quint32 m_Result;

public:
	CCallableGamePlayerAdd( quint32 nGameID, QString nName, QString nIP, quint32 nSpoofed, QString nSpoofedRealm, quint32 nReserved, quint32 nLoadingTime, quint32 nLeft, QString nLeftReason, quint32 nTeam, quint32 nColour ) : CBaseCallable( ), m_GameID( nGameID ), m_Name( nName ), m_IP( nIP ), m_Spoofed( nSpoofed ), m_SpoofedRealm( nSpoofedRealm ), m_Reserved( nReserved ), m_LoadingTime( nLoadingTime ), m_Left( nLeft ), m_LeftReason( nLeftReason ), m_Team( nTeam ), m_Colour( nColour ), m_Result( 0 ) { }
	virtual ~CCallableGamePlayerAdd( );

	virtual quint32 GetResult( )				{ return m_Result; }
	virtual void SetResult( quint32 nResult )	{ m_Result = nResult; }
};

class CCallableGamePlayerSummaryCheck : public CBaseCallable
{
protected:
	QString m_Name;
	CDBGamePlayerSummary *m_Result;

public:
	CCallableGamePlayerSummaryCheck( QString nName ) : CBaseCallable( ), m_Name( nName ), m_Result( NULL ) { }
	virtual ~CCallableGamePlayerSummaryCheck( );

	virtual QString GetName( )								{ return m_Name; }
	virtual CDBGamePlayerSummary *GetResult( )				{ return m_Result; }
	virtual void SetResult( CDBGamePlayerSummary *nResult )	{ m_Result = nResult; }
};

class CCallableDotAGameAdd : public CBaseCallable
{
protected:
	quint32 m_GameID;
	quint32 m_Winner;
	quint32 m_Min;
	quint32 m_Sec;
	quint32 m_Result;

public:
	CCallableDotAGameAdd( quint32 nGameID, quint32 nWinner, quint32 nMin, quint32 nSec ) : CBaseCallable( ), m_GameID( nGameID ), m_Winner( nWinner ), m_Min( nMin ), m_Sec( nSec ), m_Result( 0 ) { }
	virtual ~CCallableDotAGameAdd( );

	virtual quint32 GetResult( )				{ return m_Result; }
	virtual void SetResult( quint32 nResult )	{ m_Result = nResult; }
};

class CCallableDotAPlayerAdd : public CBaseCallable
{
protected:
	quint32 m_GameID;
	quint32 m_Colour;
	quint32 m_Kills;
	quint32 m_Deaths;
	quint32 m_CreepKills;
	quint32 m_CreepDenies;
	quint32 m_Assists;
	quint32 m_Gold;
	quint32 m_NeutralKills;
	QString m_Item1;
	QString m_Item2;
	QString m_Item3;
	QString m_Item4;
	QString m_Item5;
	QString m_Item6;
	QString m_Hero;
	quint32 m_NewColour;
	quint32 m_TowerKills;
	quint32 m_RaxKills;
	quint32 m_CourierKills;
	quint32 m_Result;

public:
	CCallableDotAPlayerAdd( quint32 nGameID, quint32 nColour, quint32 nKills, quint32 nDeaths, quint32 nCreepKills, quint32 nCreepDenies, quint32 nAssists, quint32 nGold, quint32 nNeutralKills, QString nItem1, QString nItem2, QString nItem3, QString nItem4, QString nItem5, QString nItem6, QString nHero, quint32 nNewColour, quint32 nTowerKills, quint32 nRaxKills, quint32 nCourierKills ) : CBaseCallable( ), m_GameID( nGameID ), m_Colour( nColour ), m_Kills( nKills ), m_Deaths( nDeaths ), m_CreepKills( nCreepKills ), m_CreepDenies( nCreepDenies ), m_Assists( nAssists ), m_Gold( nGold ), m_NeutralKills( nNeutralKills ), m_Item1( nItem1 ), m_Item2( nItem2 ), m_Item3( nItem3 ), m_Item4( nItem4 ), m_Item5( nItem5 ), m_Item6( nItem6 ), m_Hero( nHero ), m_NewColour( nNewColour ), m_TowerKills( nTowerKills ), m_RaxKills( nRaxKills ), m_CourierKills( nCourierKills ), m_Result( 0 ) { }
	virtual ~CCallableDotAPlayerAdd( );

	virtual quint32 GetResult( )				{ return m_Result; }
	virtual void SetResult( quint32 nResult )	{ m_Result = nResult; }
};

class CCallableDotAPlayerSummaryCheck : public CBaseCallable
{
protected:
	QString m_Name;
	CDBDotAPlayerSummary *m_Result;

public:
	CCallableDotAPlayerSummaryCheck( QString nName ) : CBaseCallable( ), m_Name( nName ), m_Result( NULL ) { }
	virtual ~CCallableDotAPlayerSummaryCheck( );

	virtual QString GetName( )								{ return m_Name; }
	virtual CDBDotAPlayerSummary *GetResult( )				{ return m_Result; }
	virtual void SetResult( CDBDotAPlayerSummary *nResult )	{ m_Result = nResult; }
};

class CCallableDownloadAdd : public CBaseCallable
{
protected:
	QString m_Map;
	quint32 m_MapSize;
	QString m_Name;
	QString m_IP;
	quint32 m_Spoofed;
	QString m_SpoofedRealm;
	quint32 m_DownloadTime;
	bool m_Result;

public:
	CCallableDownloadAdd( QString nMap, quint32 nMapSize, QString nName, QString nIP, quint32 nSpoofed, QString nSpoofedRealm, quint32 nDownloadTime ) : CBaseCallable( ), m_Map( nMap ), m_MapSize( nMapSize ), m_Name( nName ), m_IP( nIP ), m_Spoofed( nSpoofed ), m_SpoofedRealm( nSpoofedRealm ), m_DownloadTime( nDownloadTime ), m_Result( false ) { }
	virtual ~CCallableDownloadAdd( );

	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

class CCallableScoreCheck : public CBaseCallable
{
protected:
	QString m_Category;
	QString m_Name;
	QString m_Server;
	double m_Result;

public:
	CCallableScoreCheck( QString nCategory, QString nName, QString nServer ) : CBaseCallable( ), m_Category( nCategory ), m_Name( nName ), m_Server( nServer ), m_Result( 0.0 ) { }
	virtual ~CCallableScoreCheck( );

	virtual QString GetName( )					{ return m_Name; }
	virtual double GetResult( )					{ return m_Result; }
	virtual void SetResult( double nResult )	{ m_Result = nResult; }
};

class CCallableW3MMDPlayerAdd : public CBaseCallable
{
protected:
	QString m_Category;
	quint32 m_GameID;
	quint32 m_PID;
	QString m_Name;
	QString m_Flag;
	quint32 m_Leaver;
	quint32 m_Practicing;
	quint32 m_Result;

public:
	CCallableW3MMDPlayerAdd( QString nCategory, quint32 nGameID, quint32 nPID, QString nName, QString nFlag, quint32 nLeaver, quint32 nPracticing ) : CBaseCallable( ), m_Category( nCategory ), m_GameID( nGameID ), m_PID( nPID ), m_Name( nName ), m_Flag( nFlag ), m_Leaver( nLeaver ), m_Practicing( nPracticing ), m_Result( 0 ) { }
	virtual ~CCallableW3MMDPlayerAdd( );

	virtual quint32 GetResult( )				{ return m_Result; }
	virtual void SetResult( quint32 nResult )	{ m_Result = nResult; }
};

class CCallableW3MMDVarAdd : public CBaseCallable
{
protected:
	quint32 m_GameID;
	QMap<VarP,quint32> m_VarInts;
	QMap<VarP,double> m_VarReals;
	QMap<VarP,QString> m_VarStrings;

	enum ValueType {
		VALUETYPE_INT = 1,
		VALUETYPE_REAL = 2,
		VALUETYPE_STRING = 3
	};

	ValueType m_ValueType;
	bool m_Result;

public:
	CCallableW3MMDVarAdd( quint32 nGameID, QMap<VarP,quint32> nVarInts ) : CBaseCallable( ), m_GameID( nGameID ), m_VarInts( nVarInts ), m_ValueType( VALUETYPE_INT ), m_Result( false ) { }
	CCallableW3MMDVarAdd( quint32 nGameID, QMap<VarP,double> nVarReals ) : CBaseCallable( ), m_GameID( nGameID ), m_VarReals( nVarReals ), m_ValueType( VALUETYPE_REAL ), m_Result( false ) { }
	CCallableW3MMDVarAdd( quint32 nGameID, QMap<VarP,QString> nVarStrings ) : CBaseCallable( ), m_GameID( nGameID ), m_VarStrings( nVarStrings ), m_ValueType( VALUETYPE_STRING ), m_Result( false ) { }
	virtual ~CCallableW3MMDVarAdd( );

	virtual bool GetResult( )				{ return m_Result; }
	virtual void SetResult( bool nResult )	{ m_Result = nResult; }
};

//
// CDBBan
//

class CDBBan
{
private:
	QString m_Server;
	QString m_Name;
	QString m_IP;
	QString m_Date;
	QString m_GameName;
	QString m_Admin;
	QString m_Reason;

public:
	CDBBan( QString nServer, QString nName, QString nIP, QString nDate, QString nGameName, QString nAdmin, QString nReason );
	~CDBBan( );

	QString GetServer( )		{ return m_Server; }
	QString GetName( )		{ return m_Name; }
	QString GetIP( )			{ return m_IP; }
	QString GetDate( )		{ return m_Date; }
	QString GetGameName( )	{ return m_GameName; }
	QString GetAdmin( )		{ return m_Admin; }
	QString GetReason( )		{ return m_Reason; }
};

//
// CDBGame
//

class CDBGame
{
private:
	quint32 m_ID;
	QString m_Server;
	QString m_Map;
	QString m_DateTime;
	QString m_GameName;
	QString m_OwnerName;
	quint32 m_Duration;

public:
	CDBGame( quint32 nID, QString nServer, QString nMap, QString nDateTime, QString nGameName, QString nOwnerName, quint32 nDuration );
	~CDBGame( );

	quint32 GetID( )		{ return m_ID; }
	QString GetServer( )		{ return m_Server; }
	QString GetMap( )		{ return m_Map; }
	QString GetDateTime( )	{ return m_DateTime; }
	QString GetGameName( )	{ return m_GameName; }
	QString GetOwnerName( )	{ return m_OwnerName; }
	quint32 GetDuration( )	{ return m_Duration; }

	void SetDuration( quint32 nDuration )	{ m_Duration = nDuration; }
};

//
// CDBGamePlayer
//

class CDBGamePlayer
{
private:
	quint32 m_ID;
	quint32 m_GameID;
	QString m_Name;
	QString m_IP;
	quint32 m_Spoofed;
	QString m_SpoofedRealm;
	quint32 m_Reserved;
	quint32 m_LoadingTime;
	quint32 m_Left;
	QString m_LeftReason;
	quint32 m_Team;
	quint32 m_Colour;

public:
	CDBGamePlayer( quint32 nID, quint32 nGameID, QString nName, QString nIP, quint32 nSpoofed, QString nSpoofedRealm, quint32 nReserved, quint32 nLoadingTime, quint32 nLeft, QString nLeftReason, quint32 nTeam, quint32 nColour );
	~CDBGamePlayer( );

	quint32 GetID( )			{ return m_ID; }
	quint32 GetGameID( )		{ return m_GameID; }
	QString GetName( )			{ return m_Name; }
	QString GetIP( )				{ return m_IP; }
	quint32 GetSpoofed( )		{ return m_Spoofed; }
	QString GetSpoofedRealm( )	{ return m_SpoofedRealm; }
	quint32 GetReserved( )		{ return m_Reserved; }
	quint32 GetLoadingTime( )	{ return m_LoadingTime; }
	quint32 GetLeft( )			{ return m_Left; }
	QString GetLeftReason( )		{ return m_LeftReason; }
	quint32 GetTeam( )			{ return m_Team; }
	quint32 GetColour( )		{ return m_Colour; }

	void SetLoadingTime( quint32 nLoadingTime )	{ m_LoadingTime = nLoadingTime; }
	void SetLeft( quint32 nLeft )					{ m_Left = nLeft; }
	void SetLeftReason( QString nLeftReason )		{ m_LeftReason = nLeftReason; }
};

//
// CDBGamePlayerSummary
//

class CDBGamePlayerSummary
{
private:
	QString m_Server;
	QString m_Name;
	QString m_FirstGameDateTime;		// datetime of first game played
	QString m_LastGameDateTime;		// datetime of last game played
	quint32 m_TotalGames;			// total number of games played
	quint32 m_MinLoadingTime;		// minimum loading time in milliseconds (this could be skewed because different maps have different load times)
	quint32 m_AvgLoadingTime;		// average loading time in milliseconds (this could be skewed because different maps have different load times)
	quint32 m_MaxLoadingTime;		// maximum loading time in milliseconds (this could be skewed because different maps have different load times)
	quint32 m_MinLeftPercent;		// minimum time at which the player left the game expressed as a percentage of the game duration (0-100)
	quint32 m_AvgLeftPercent;		// average time at which the player left the game expressed as a percentage of the game duration (0-100)
	quint32 m_MaxLeftPercent;		// maximum time at which the player left the game expressed as a percentage of the game duration (0-100)
	quint32 m_MinDuration;			// minimum game duration in seconds
	quint32 m_AvgDuration;			// average game duration in seconds
	quint32 m_MaxDuration;			// maximum game duration in seconds

public:
	CDBGamePlayerSummary( QString nServer, QString nName, QString nFirstGameDateTime, QString nLastGameDateTime, quint32 nTotalGames, quint32 nMinLoadingTime, quint32 nAvgLoadingTime, quint32 nMaxLoadingTime, quint32 nMinLeftPercent, quint32 nAvgLeftPercent, quint32 nMaxLeftPercent, quint32 nMinDuration, quint32 nAvgDuration, quint32 nMaxDuration );
	~CDBGamePlayerSummary( );

	QString GetServer( )					{ return m_Server; }
	QString GetName( )					{ return m_Name; }
	QString GetFirstGameDateTime( )		{ return m_FirstGameDateTime; }
	QString GetLastGameDateTime( )		{ return m_LastGameDateTime; }
	quint32 GetTotalGames( )			{ return m_TotalGames; }
	quint32 GetMinLoadingTime( )		{ return m_MinLoadingTime; }
	quint32 GetAvgLoadingTime( )		{ return m_AvgLoadingTime; }
	quint32 GetMaxLoadingTime( )		{ return m_MaxLoadingTime; }
	quint32 GetMinLeftPercent( )		{ return m_MinLeftPercent; }
	quint32 GetAvgLeftPercent( )		{ return m_AvgLeftPercent; }
	quint32 GetMaxLeftPercent( )		{ return m_MaxLeftPercent; }
	quint32 GetMinDuration( )			{ return m_MinDuration; }
	quint32 GetAvgDuration( )			{ return m_AvgDuration; }
	quint32 GetMaxDuration( )			{ return m_MaxDuration; }
};

//
// CDBDotAGame
//

class CDBDotAGame
{
private:
	quint32 m_ID;
	quint32 m_GameID;
	quint32 m_Winner;
	quint32 m_Min;
	quint32 m_Sec;

public:
	CDBDotAGame( quint32 nID, quint32 nGameID, quint32 nWinner, quint32 nMin, quint32 nSec );
	~CDBDotAGame( );

	quint32 GetID( )		{ return m_ID; }
	quint32 GetGameID( )	{ return m_GameID; }
	quint32 GetWinner( )	{ return m_Winner; }
	quint32 GetMin( )		{ return m_Min; }
	quint32 GetSec( )		{ return m_Sec; }
};

//
// CDBDotAPlayer
//

class CDBDotAPlayer
{
private:
	quint32 m_ID;
	quint32 m_GameID;
	quint32 m_Colour;
	quint32 m_Kills;
	quint32 m_Deaths;
	quint32 m_CreepKills;
	quint32 m_CreepDenies;
	quint32 m_Assists;
	quint32 m_Gold;
	quint32 m_NeutralKills;
	QString m_Items[6];
	QString m_Hero;
	quint32 m_NewColour;
	quint32 m_TowerKills;
	quint32 m_RaxKills;
	quint32 m_CourierKills;

public:
	CDBDotAPlayer( );
	CDBDotAPlayer( quint32 nID, quint32 nGameID, quint32 nColour, quint32 nKills, quint32 nDeaths, quint32 nCreepKills, quint32 nCreepDenies, quint32 nAssists, quint32 nGold, quint32 nNeutralKills, QString nItem1, QString nItem2, QString nItem3, QString nItem4, QString nItem5, QString nItem6, QString nHero, quint32 nNewColour, quint32 nTowerKills, quint32 nRaxKills, quint32 nCourierKills );
	~CDBDotAPlayer( );

	quint32 GetID( )			{ return m_ID; }
	quint32 GetGameID( )		{ return m_GameID; }
	quint32 GetColour( )		{ return m_Colour; }
	quint32 GetKills( )		{ return m_Kills; }
	quint32 GetDeaths( )		{ return m_Deaths; }
	quint32 GetCreepKills( )	{ return m_CreepKills; }
	quint32 GetCreepDenies( )	{ return m_CreepDenies; }
	quint32 GetAssists( )		{ return m_Assists; }
	quint32 GetGold( )			{ return m_Gold; }
	quint32 GetNeutralKills( )	{ return m_NeutralKills; }
	QString GetItem( unsigned int i );
	QString GetHero( )			{ return m_Hero; }
	quint32 GetNewColour( )	{ return m_NewColour; }
	quint32 GetTowerKills( )	{ return m_TowerKills; }
	quint32 GetRaxKills( )		{ return m_RaxKills; }
	quint32 GetCourierKills( )	{ return m_CourierKills; }

	void SetColour( quint32 nColour )				{ m_Colour = nColour; }
	void SetKills( quint32 nKills )				{ m_Kills = nKills; }
	void SetDeaths( quint32 nDeaths )				{ m_Deaths = nDeaths; }
	void SetCreepKills( quint32 nCreepKills )		{ m_CreepKills = nCreepKills; }
	void SetCreepDenies( quint32 nCreepDenies )	{ m_CreepDenies = nCreepDenies; }
	void SetAssists( quint32 nAssists )			{ m_Assists = nAssists; }
	void SetGold( quint32 nGold )					{ m_Gold = nGold; }
	void SetNeutralKills( quint32 nNeutralKills )	{ m_NeutralKills = nNeutralKills; }
	void SetItem( unsigned int i, QString item );
	void SetHero( QString nHero )					{ m_Hero = nHero; }
	void SetNewColour( quint32 nNewColour )		{ m_NewColour = nNewColour; }
	void SetTowerKills( quint32 nTowerKills )		{ m_TowerKills = nTowerKills; }
	void SetRaxKills( quint32 nRaxKills )			{ m_RaxKills = nRaxKills; }
	void SetCourierKills( quint32 nCourierKills )	{ m_CourierKills = nCourierKills; }
};

//
// CDBDotAPlayerSummary
//

class CDBDotAPlayerSummary
{
private:
	QString m_Server;
	QString m_Name;
	quint32 m_TotalGames;			// total number of dota games played
	quint32 m_TotalWins;			// total number of dota games won
	quint32 m_TotalLosses;			// total number of dota games lost
	quint32 m_TotalKills;			// total number of hero kills
	quint32 m_TotalDeaths;			// total number of deaths
	quint32 m_TotalCreepKills;		// total number of creep kills
	quint32 m_TotalCreepDenies;	// total number of creep denies
	quint32 m_TotalAssists;		// total number of assists
	quint32 m_TotalNeutralKills;	// total number of neutral kills
	quint32 m_TotalTowerKills;		// total number of tower kills
	quint32 m_TotalRaxKills;		// total number of rax kills
	quint32 m_TotalCourierKills;	// total number of courier kills

public:
	CDBDotAPlayerSummary( QString nServer, QString nName, quint32 nTotalGames, quint32 nTotalWins, quint32 nTotalLosses, quint32 nTotalKills, quint32 nTotalDeaths, quint32 nTotalCreepKills, quint32 nTotalCreepDenies, quint32 nTotalAssists, quint32 nTotalNeutralKills, quint32 nTotalTowerKills, quint32 nTotalRaxKills, quint32 nTotalCourierKills );
	~CDBDotAPlayerSummary( );

	QString GetServer( )					{ return m_Server; }
	QString GetName( )					{ return m_Name; }
	quint32 GetTotalGames( )			{ return m_TotalGames; }
	quint32 GetTotalWins( )			{ return m_TotalWins; }
	quint32 GetTotalLosses( )			{ return m_TotalLosses; }
	quint32 GetTotalKills( )			{ return m_TotalKills; }
	quint32 GetTotalDeaths( )			{ return m_TotalDeaths; }
	quint32 GetTotalCreepKills( )		{ return m_TotalCreepKills; }
	quint32 GetTotalCreepDenies( )		{ return m_TotalCreepDenies; }
	quint32 GetTotalAssists( )			{ return m_TotalAssists; }
	quint32 GetTotalNeutralKills( )	{ return m_TotalNeutralKills; }
	quint32 GetTotalTowerKills( )		{ return m_TotalTowerKills; }
	quint32 GetTotalRaxKills( )		{ return m_TotalRaxKills; }
	quint32 GetTotalCourierKills( )	{ return m_TotalCourierKills; }

	float GetAvgKills( )				{ return m_TotalGames > 0 ? (float)m_TotalKills / m_TotalGames : 0; }
	float GetAvgDeaths( )				{ return m_TotalGames > 0 ? (float)m_TotalDeaths / m_TotalGames : 0; }
	float GetAvgCreepKills( )			{ return m_TotalGames > 0 ? (float)m_TotalCreepKills / m_TotalGames : 0; }
	float GetAvgCreepDenies( )			{ return m_TotalGames > 0 ? (float)m_TotalCreepDenies / m_TotalGames : 0; }
	float GetAvgAssists( )				{ return m_TotalGames > 0 ? (float)m_TotalAssists / m_TotalGames : 0; }
	float GetAvgNeutralKills( )			{ return m_TotalGames > 0 ? (float)m_TotalNeutralKills / m_TotalGames : 0; }
	float GetAvgTowerKills( )			{ return m_TotalGames > 0 ? (float)m_TotalTowerKills / m_TotalGames : 0; }
	float GetAvgRaxKills( )				{ return m_TotalGames > 0 ? (float)m_TotalRaxKills / m_TotalGames : 0; }
	float GetAvgCourierKills( )			{ return m_TotalGames > 0 ? (float)m_TotalCourierKills / m_TotalGames : 0; }
};

#endif
