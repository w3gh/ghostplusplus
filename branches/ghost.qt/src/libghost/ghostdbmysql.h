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

#ifdef GHOST_MYSQL

#ifndef GHOSTDBMYSQL_H
#define GHOSTDBMYSQL_H

/**************
 *** SCHEMA ***
 **************

CREATE TABLE admins (
	id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	botid INT NOT NULL,
	name VARCHAR(15) NOT NULL,
	server VARCHAR(100) NOT NULL
)

CREATE TABLE bans (
	id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	botid INT NOT NULL,
	server VARCHAR(100) NOT NULL,
	name VARCHAR(15) NOT NULL,
	ip VARCHAR(15) NOT NULL,
	date DATETIME NOT NULL,
	gamename VARCHAR(31) NOT NULL,
	admin VARCHAR(15) NOT NULL,
	reason VARCHAR(255) NOT NULL
)

CREATE TABLE games (
	id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	botid INT NOT NULL,
	server VARCHAR(100) NOT NULL,
	map VARCHAR(100) NOT NULL,
	datetime DATETIME NOT NULL,
	gamename VARCHAR(31) NOT NULL,
	ownername VARCHAR(15) NOT NULL,
	duration INT NOT NULL,
	gamestate INT NOT NULL,
	creatorname VARCHAR(15) NOT NULL,
	creatorserver VARCHAR(100) NOT NULL
)

CREATE TABLE gameplayers (
	id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	botid INT NOT NULL,
	gameid INT NOT NULL,
	name VARCHAR(15) NOT NULL,
	ip VARCHAR(15) NOT NULL,
	spoofed INT NOT NULL,
	reserved INT NOT NULL,
	loadingtime INT NOT NULL,
	`left` INT NOT NULL,
	leftreason VARCHAR(100) NOT NULL,
	team INT NOT NULL,
	colour INT NOT NULL,
	spoofedrealm VARCHAR(100) NOT NULL,
	INDEX( gameid )
)

CREATE TABLE dotagames (
	id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	botid INT NOT NULL,
	gameid INT NOT NULL,
	winner INT NOT NULL,
	min INT NOT NULL,
	sec INT NOT NULL
)

CREATE TABLE dotaplayers (
	id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	botid INT NOT NULL,
	gameid INT NOT NULL,
	colour INT NOT NULL,
	kills INT NOT NULL,
	deaths INT NOT NULL,
	creepkills INT NOT NULL,
	creepdenies INT NOT NULL,
	assists INT NOT NULL,
	gold INT NOT NULL,
	neutralkills INT NOT NULL,
	item1 CHAR(4) NOT NULL,
	item2 CHAR(4) NOT NULL,
	item3 CHAR(4) NOT NULL,
	item4 CHAR(4) NOT NULL,
	item5 CHAR(4) NOT NULL,
	item6 CHAR(4) NOT NULL,
	hero CHAR(4) NOT NULL,
	newcolour INT NOT NULL,
	towerkills INT NOT NULL,
	raxkills INT NOT NULL,
	courierkills INT NOT NULL,
	INDEX( gameid, colour )
)

CREATE TABLE downloads (
	id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	botid INT NOT NULL,
	map VARCHAR(100) NOT NULL,
	mapsize INT NOT NULL,
	datetime DATETIME NOT NULL,
	name VARCHAR(15) NOT NULL,
	ip VARCHAR(15) NOT NULL,
	spoofed INT NOT NULL,
	spoofedrealm VARCHAR(100) NOT NULL,
	downloadtime INT NOT NULL
)

CREATE TABLE scores (
	id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	category VARCHAR(25) NOT NULL,
	name VARCHAR(15) NOT NULL,
	server VARCHAR(100) NOT NULL,
	score REAL NOT NULL
)

CREATE TABLE w3mmdplayers (
	id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	botid INT NOT NULL,
	category VARCHAR(25) NOT NULL,
	gameid INT NOT NULL,
	pid INT NOT NULL,
	name VARCHAR(15) NOT NULL,
	flag VARCHAR(32) NOT NULL,
	leaver INT NOT NULL,
	practicing INT NOT NULL
)

CREATE TABLE w3mmdvars (
	id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
	botid INT NOT NULL,
	gameid INT NOT NULL,
	pid INT NOT NULL,
	varname VARCHAR(25) NOT NULL,
	value_int INT DEFAULT NULL,
	value_real REAL DEFAULT NULL,
	value_string VARCHAR(100) DEFAULT NULL
)

 **************
 *** SCHEMA ***
 **************/

//
// CGHostDBMySQL
//

class CGHostDBMySQL : public CGHostDB
{
private:
	QString m_Server;
	QString m_Database;
	QString m_User;
	QString m_Password;
	quint16 m_Port;
	quint32 m_BotID;
	QQueue<void *> m_IdleConnections;
	quint32 m_NumConnections;
	quint32 m_OutstandingCallables;

public:
	CGHostDBMySQL( CConfig *CFG );
	virtual ~CGHostDBMySQL( );

	virtual QString GetStatus( );

	virtual void RecoverCallable( CBaseCallable *callable );

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
	virtual CCallableW3MMDVarAdd *ThreadedW3MMDVarAdd( quint32 gameid, QMap<VarP,int32_t> var_ints );
	virtual CCallableW3MMDVarAdd *ThreadedW3MMDVarAdd( quint32 gameid, QMap<VarP,double> var_reals );
	virtual CCallableW3MMDVarAdd *ThreadedW3MMDVarAdd( quint32 gameid, QMap<VarP,string> var_strings );

	// other database functions

	virtual void *GetIdleConnection( );
};

//
// global helper functions
//

quint32 MySQLAdminCount( void *conn, QString *error, quint32 botid, QString server );
bool MySQLAdminCheck( void *conn, QString *error, quint32 botid, QString server, QString user );
bool MySQLAdminAdd( void *conn, QString *error, quint32 botid, QString server, QString user );
bool MySQLAdminRemove( void *conn, QString *error, quint32 botid, QString server, QString user );
QList<string> MySQLAdminList( void *conn, QString *error, quint32 botid, QString server );
quint32 MySQLBanCount( void *conn, QString *error, quint32 botid, QString server );
CDBBan *MySQLBanCheck( void *conn, QString *error, quint32 botid, QString server, QString user, QString ip );
bool MySQLBanAdd( void *conn, QString *error, quint32 botid, QString server, QString user, QString ip, QString gamename, QString admin, QString reason );
bool MySQLBanRemove( void *conn, QString *error, quint32 botid, QString server, QString user );
bool MySQLBanRemove( void *conn, QString *error, quint32 botid, QString user );
QList<CDBBan *> MySQLBanList( void *conn, QString *error, quint32 botid, QString server );
quint32 MySQLGameAdd( void *conn, QString *error, quint32 botid, QString server, QString map, QString gamename, QString ownername, quint32 duration, quint32 gamestate, QString creatorname, QString creatorserver );
quint32 MySQLGamePlayerAdd( void *conn, QString *error, quint32 botid, quint32 gameid, QString name, QString ip, quint32 spoofed, QString spoofedrealm, quint32 reserved, quint32 loadingtime, quint32 left, QString leftreason, quint32 team, quint32 colour );
CDBGamePlayerSummary *MySQLGamePlayerSummaryCheck( void *conn, QString *error, quint32 botid, QString name );
quint32 MySQLDotAGameAdd( void *conn, QString *error, quint32 botid, quint32 gameid, quint32 winner, quint32 min, quint32 sec );
quint32 MySQLDotAPlayerAdd( void *conn, QString *error, quint32 botid, quint32 gameid, quint32 colour, quint32 kills, quint32 deaths, quint32 creepkills, quint32 creepdenies, quint32 assists, quint32 gold, quint32 neutralkills, QString item1, QString item2, QString item3, QString item4, QString item5, QString item6, QString hero, quint32 newcolour, quint32 towerkills, quint32 raxkills, quint32 courierkills );
CDBDotAPlayerSummary *MySQLDotAPlayerSummaryCheck( void *conn, QString *error, quint32 botid, QString name );
bool MySQLDownloadAdd( void *conn, QString *error, quint32 botid, QString map, quint32 mapsize, QString name, QString ip, quint32 spoofed, QString spoofedrealm, quint32 downloadtime );
double MySQLScoreCheck( void *conn, QString *error, quint32 botid, QString category, QString name, QString server );
quint32 MySQLW3MMDPlayerAdd( void *conn, QString *error, quint32 botid, QString category, quint32 gameid, quint32 pid, QString name, QString flag, quint32 leaver, quint32 practicing );
bool MySQLW3MMDVarAdd( void *conn, QString *error, quint32 botid, quint32 gameid, QMap<VarP,int32_t> var_ints );
bool MySQLW3MMDVarAdd( void *conn, QString *error, quint32 botid, quint32 gameid, QMap<VarP,double> var_reals );
bool MySQLW3MMDVarAdd( void *conn, QString *error, quint32 botid, quint32 gameid, QMap<VarP,string> var_strings );

//
// MySQL Callables
//

class CMySQLCallable : virtual public CBaseCallable
{
protected:
	void *m_Connection;
	QString m_SQLServer;
	QString m_SQLDatabase;
	QString m_SQLUser;
	QString m_SQLPassword;
	quint16 m_SQLPort;
	quint32 m_SQLBotID;

public:
	CMySQLCallable( void *nConnection, quint32 nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, quint16 nSQLPort ) : CBaseCallable( ), m_Connection( nConnection ), m_SQLBotID( nSQLBotID ), m_SQLServer( nSQLServer ), m_SQLDatabase( nSQLDatabase ), m_SQLUser( nSQLUser ), m_SQLPassword( nSQLPassword ), m_SQLPort( nSQLPort ) { }
	virtual ~CMySQLCallable( ) { }

	virtual void *GetConnection( )	{ return m_Connection; }

	virtual void Init( );
	virtual void Close( );
};

class CMySQLCallableAdminCount : public CCallableAdminCount, public CMySQLCallable
{
public:
	CMySQLCallableAdminCount( QString nServer, void *nConnection, quint32 nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, quint16 nSQLPort ) : CBaseCallable( ), CCallableAdminCount( nServer ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableAdminCount( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableAdminCheck : public CCallableAdminCheck, public CMySQLCallable
{
public:
	CMySQLCallableAdminCheck( QString nServer, QString nUser, void *nConnection, quint32 nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, quint16 nSQLPort ) : CBaseCallable( ), CCallableAdminCheck( nServer, nUser ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableAdminCheck( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableAdminAdd : public CCallableAdminAdd, public CMySQLCallable
{
public:
	CMySQLCallableAdminAdd( QString nServer, QString nUser, void *nConnection, quint32 nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, quint16 nSQLPort ) : CBaseCallable( ), CCallableAdminAdd( nServer, nUser ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableAdminAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableAdminRemove : public CCallableAdminRemove, public CMySQLCallable
{
public:
	CMySQLCallableAdminRemove( QString nServer, QString nUser, void *nConnection, quint32 nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, quint16 nSQLPort ) : CBaseCallable( ), CCallableAdminRemove( nServer, nUser ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableAdminRemove( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableAdminList : public CCallableAdminList, public CMySQLCallable
{
public:
	CMySQLCallableAdminList( QString nServer, void *nConnection, quint32 nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, quint16 nSQLPort ) : CBaseCallable( ), CCallableAdminList( nServer ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableAdminList( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableBanCount : public CCallableBanCount, public CMySQLCallable
{
public:
	CMySQLCallableBanCount( QString nServer, void *nConnection, quint32 nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, quint16 nSQLPort ) : CBaseCallable( ), CCallableBanCount( nServer ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableBanCount( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableBanCheck : public CCallableBanCheck, public CMySQLCallable
{
public:
	CMySQLCallableBanCheck( QString nServer, QString nUser, QString nIP, void *nConnection, quint32 nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, quint16 nSQLPort ) : CBaseCallable( ), CCallableBanCheck( nServer, nUser, nIP ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableBanCheck( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableBanAdd : public CCallableBanAdd, public CMySQLCallable
{
public:
	CMySQLCallableBanAdd( QString nServer, QString nUser, QString nIP, QString nGameName, QString nAdmin, QString nReason, void *nConnection, quint32 nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, quint16 nSQLPort ) : CBaseCallable( ), CCallableBanAdd( nServer, nUser, nIP, nGameName, nAdmin, nReason ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableBanAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableBanRemove : public CCallableBanRemove, public CMySQLCallable
{
public:
	CMySQLCallableBanRemove( QString nServer, QString nUser, void *nConnection, quint32 nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, quint16 nSQLPort ) : CBaseCallable( ), CCallableBanRemove( nServer, nUser ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableBanRemove( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableBanList : public CCallableBanList, public CMySQLCallable
{
public:
	CMySQLCallableBanList( QString nServer, void *nConnection, quint32 nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, quint16 nSQLPort ) : CBaseCallable( ), CCallableBanList( nServer ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableBanList( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableGameAdd : public CCallableGameAdd, public CMySQLCallable
{
public:
	CMySQLCallableGameAdd( QString nServer, QString nMap, QString nGameName, QString nOwnerName, quint32 nDuration, quint32 nGameState, QString nCreatorName, QString nCreatorServer, void *nConnection, quint32 nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, quint16 nSQLPort ) : CBaseCallable( ), CCallableGameAdd( nServer, nMap, nGameName, nOwnerName, nDuration, nGameState, nCreatorName, nCreatorServer ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableGameAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableGamePlayerAdd : public CCallableGamePlayerAdd, public CMySQLCallable
{
public:
	CMySQLCallableGamePlayerAdd( quint32 nGameID, QString nName, QString nIP, quint32 nSpoofed, QString nSpoofedRealm, quint32 nReserved, quint32 nLoadingTime, quint32 nLeft, QString nLeftReason, quint32 nTeam, quint32 nColour, void *nConnection, quint32 nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, quint16 nSQLPort ) : CBaseCallable( ), CCallableGamePlayerAdd( nGameID, nName, nIP, nSpoofed, nSpoofedRealm, nReserved, nLoadingTime, nLeft, nLeftReason, nTeam, nColour ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableGamePlayerAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableGamePlayerSummaryCheck : public CCallableGamePlayerSummaryCheck, public CMySQLCallable
{
public:
	CMySQLCallableGamePlayerSummaryCheck( QString nName, void *nConnection, quint32 nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, quint16 nSQLPort ) : CBaseCallable( ), CCallableGamePlayerSummaryCheck( nName ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableGamePlayerSummaryCheck( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableDotAGameAdd : public CCallableDotAGameAdd, public CMySQLCallable
{
public:
	CMySQLCallableDotAGameAdd( quint32 nGameID, quint32 nWinner, quint32 nMin, quint32 nSec, void *nConnection, quint32 nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, quint16 nSQLPort ) : CBaseCallable( ), CCallableDotAGameAdd( nGameID, nWinner, nMin, nSec ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableDotAGameAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableDotAPlayerAdd : public CCallableDotAPlayerAdd, public CMySQLCallable
{
public:
	CMySQLCallableDotAPlayerAdd( quint32 nGameID, quint32 nColour, quint32 nKills, quint32 nDeaths, quint32 nCreepKills, quint32 nCreepDenies, quint32 nAssists, quint32 nGold, quint32 nNeutralKills, QString nItem1, QString nItem2, QString nItem3, QString nItem4, QString nItem5, QString nItem6, QString nHero, quint32 nNewColour, quint32 nTowerKills, quint32 nRaxKills, quint32 nCourierKills, void *nConnection, quint32 nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, quint16 nSQLPort ) : CBaseCallable( ), CCallableDotAPlayerAdd( nGameID, nColour, nKills, nDeaths, nCreepKills, nCreepDenies, nAssists, nGold, nNeutralKills, nItem1, nItem2, nItem3, nItem4, nItem5, nItem6, nHero, nNewColour, nTowerKills, nRaxKills, nCourierKills ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableDotAPlayerAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableDotAPlayerSummaryCheck : public CCallableDotAPlayerSummaryCheck, public CMySQLCallable
{
public:
	CMySQLCallableDotAPlayerSummaryCheck( QString nName, void *nConnection, quint32 nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, quint16 nSQLPort ) : CBaseCallable( ), CCallableDotAPlayerSummaryCheck( nName ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableDotAPlayerSummaryCheck( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableDownloadAdd : public CCallableDownloadAdd, public CMySQLCallable
{
public:
	CMySQLCallableDownloadAdd( QString nMap, quint32 nMapSize, QString nName, QString nIP, quint32 nSpoofed, QString nSpoofedRealm, quint32 nDownloadTime, void *nConnection, quint32 nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, quint16 nSQLPort ) : CBaseCallable( ), CCallableDownloadAdd( nMap, nMapSize, nName, nIP, nSpoofed, nSpoofedRealm, nDownloadTime ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableDownloadAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableScoreCheck : public CCallableScoreCheck, public CMySQLCallable
{
public:
	CMySQLCallableScoreCheck( QString nCategory, QString nName, QString nServer, void *nConnection, quint32 nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, quint16 nSQLPort ) : CBaseCallable( ), CCallableScoreCheck( nCategory, nName, nServer ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableScoreCheck( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableW3MMDPlayerAdd : public CCallableW3MMDPlayerAdd, public CMySQLCallable
{
public:
	CMySQLCallableW3MMDPlayerAdd( QString nCategory, quint32 nGameID, quint32 nPID, QString nName, QString nFlag, quint32 nLeaver, quint32 nPracticing, void *nConnection, quint32 nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, quint16 nSQLPort ) : CBaseCallable( ), CCallableW3MMDPlayerAdd( nCategory, nGameID, nPID, nName, nFlag, nLeaver, nPracticing ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableW3MMDPlayerAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableW3MMDVarAdd : public CCallableW3MMDVarAdd, public CMySQLCallable
{
public:
	CMySQLCallableW3MMDVarAdd( quint32 nGameID, QMap<VarP,int32_t> nVarInts, void *nConnection, quint32 nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, quint16 nSQLPort ) : CBaseCallable( ), CCallableW3MMDVarAdd( nGameID, nVarInts ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	CMySQLCallableW3MMDVarAdd( quint32 nGameID, QMap<VarP,double> nVarReals, void *nConnection, quint32 nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, quint16 nSQLPort ) : CBaseCallable( ), CCallableW3MMDVarAdd( nGameID, nVarReals ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	CMySQLCallableW3MMDVarAdd( quint32 nGameID, QMap<VarP,string> nVarStrings, void *nConnection, quint32 nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, quint16 nSQLPort ) : CBaseCallable( ), CCallableW3MMDVarAdd( nGameID, nVarStrings ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableW3MMDVarAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

#endif

#endif
