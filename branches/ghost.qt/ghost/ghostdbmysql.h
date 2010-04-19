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
	uint16_t m_Port;
	uint32_t m_BotID;
	QQueue<void *> m_IdleConnections;
	uint32_t m_NumConnections;
	uint32_t m_OutstandingCallables;

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
	virtual CCallableGameAdd *ThreadedGameAdd( QString server, QString map, QString gamename, QString ownername, uint32_t duration, uint32_t gamestate, QString creatorname, QString creatorserver );
	virtual CCallableGamePlayerAdd *ThreadedGamePlayerAdd( uint32_t gameid, QString name, QString ip, uint32_t spoofed, QString spoofedrealm, uint32_t reserved, uint32_t loadingtime, uint32_t left, QString leftreason, uint32_t team, uint32_t colour );
	virtual CCallableGamePlayerSummaryCheck *ThreadedGamePlayerSummaryCheck( QString name );
	virtual CCallableDotAGameAdd *ThreadedDotAGameAdd( uint32_t gameid, uint32_t winner, uint32_t min, uint32_t sec );
	virtual CCallableDotAPlayerAdd *ThreadedDotAPlayerAdd( uint32_t gameid, uint32_t colour, uint32_t kills, uint32_t deaths, uint32_t creepkills, uint32_t creepdenies, uint32_t assists, uint32_t gold, uint32_t neutralkills, QString item1, QString item2, QString item3, QString item4, QString item5, QString item6, QString hero, uint32_t newcolour, uint32_t towerkills, uint32_t raxkills, uint32_t courierkills );
	virtual CCallableDotAPlayerSummaryCheck *ThreadedDotAPlayerSummaryCheck( QString name );
	virtual CCallableDownloadAdd *ThreadedDownloadAdd( QString map, uint32_t mapsize, QString name, QString ip, uint32_t spoofed, QString spoofedrealm, uint32_t downloadtime );
	virtual CCallableScoreCheck *ThreadedScoreCheck( QString category, QString name, QString server );
	virtual CCallableW3MMDPlayerAdd *ThreadedW3MMDPlayerAdd( QString category, uint32_t gameid, uint32_t pid, QString name, QString flag, uint32_t leaver, uint32_t practicing );
	virtual CCallableW3MMDVarAdd *ThreadedW3MMDVarAdd( uint32_t gameid, QMap<VarP,int32_t> var_ints );
	virtual CCallableW3MMDVarAdd *ThreadedW3MMDVarAdd( uint32_t gameid, QMap<VarP,double> var_reals );
	virtual CCallableW3MMDVarAdd *ThreadedW3MMDVarAdd( uint32_t gameid, QMap<VarP,string> var_strings );

	// other database functions

	virtual void *GetIdleConnection( );
};

//
// global helper functions
//

uint32_t MySQLAdminCount( void *conn, QString *error, uint32_t botid, QString server );
bool MySQLAdminCheck( void *conn, QString *error, uint32_t botid, QString server, QString user );
bool MySQLAdminAdd( void *conn, QString *error, uint32_t botid, QString server, QString user );
bool MySQLAdminRemove( void *conn, QString *error, uint32_t botid, QString server, QString user );
QVector<string> MySQLAdminList( void *conn, QString *error, uint32_t botid, QString server );
uint32_t MySQLBanCount( void *conn, QString *error, uint32_t botid, QString server );
CDBBan *MySQLBanCheck( void *conn, QString *error, uint32_t botid, QString server, QString user, QString ip );
bool MySQLBanAdd( void *conn, QString *error, uint32_t botid, QString server, QString user, QString ip, QString gamename, QString admin, QString reason );
bool MySQLBanRemove( void *conn, QString *error, uint32_t botid, QString server, QString user );
bool MySQLBanRemove( void *conn, QString *error, uint32_t botid, QString user );
QVector<CDBBan *> MySQLBanList( void *conn, QString *error, uint32_t botid, QString server );
uint32_t MySQLGameAdd( void *conn, QString *error, uint32_t botid, QString server, QString map, QString gamename, QString ownername, uint32_t duration, uint32_t gamestate, QString creatorname, QString creatorserver );
uint32_t MySQLGamePlayerAdd( void *conn, QString *error, uint32_t botid, uint32_t gameid, QString name, QString ip, uint32_t spoofed, QString spoofedrealm, uint32_t reserved, uint32_t loadingtime, uint32_t left, QString leftreason, uint32_t team, uint32_t colour );
CDBGamePlayerSummary *MySQLGamePlayerSummaryCheck( void *conn, QString *error, uint32_t botid, QString name );
uint32_t MySQLDotAGameAdd( void *conn, QString *error, uint32_t botid, uint32_t gameid, uint32_t winner, uint32_t min, uint32_t sec );
uint32_t MySQLDotAPlayerAdd( void *conn, QString *error, uint32_t botid, uint32_t gameid, uint32_t colour, uint32_t kills, uint32_t deaths, uint32_t creepkills, uint32_t creepdenies, uint32_t assists, uint32_t gold, uint32_t neutralkills, QString item1, QString item2, QString item3, QString item4, QString item5, QString item6, QString hero, uint32_t newcolour, uint32_t towerkills, uint32_t raxkills, uint32_t courierkills );
CDBDotAPlayerSummary *MySQLDotAPlayerSummaryCheck( void *conn, QString *error, uint32_t botid, QString name );
bool MySQLDownloadAdd( void *conn, QString *error, uint32_t botid, QString map, uint32_t mapsize, QString name, QString ip, uint32_t spoofed, QString spoofedrealm, uint32_t downloadtime );
double MySQLScoreCheck( void *conn, QString *error, uint32_t botid, QString category, QString name, QString server );
uint32_t MySQLW3MMDPlayerAdd( void *conn, QString *error, uint32_t botid, QString category, uint32_t gameid, uint32_t pid, QString name, QString flag, uint32_t leaver, uint32_t practicing );
bool MySQLW3MMDVarAdd( void *conn, QString *error, uint32_t botid, uint32_t gameid, QMap<VarP,int32_t> var_ints );
bool MySQLW3MMDVarAdd( void *conn, QString *error, uint32_t botid, uint32_t gameid, QMap<VarP,double> var_reals );
bool MySQLW3MMDVarAdd( void *conn, QString *error, uint32_t botid, uint32_t gameid, QMap<VarP,string> var_strings );

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
	uint16_t m_SQLPort;
	uint32_t m_SQLBotID;

public:
	CMySQLCallable( void *nConnection, uint32_t nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), m_Connection( nConnection ), m_SQLBotID( nSQLBotID ), m_SQLServer( nSQLServer ), m_SQLDatabase( nSQLDatabase ), m_SQLUser( nSQLUser ), m_SQLPassword( nSQLPassword ), m_SQLPort( nSQLPort ) { }
	virtual ~CMySQLCallable( ) { }

	virtual void *GetConnection( )	{ return m_Connection; }

	virtual void Init( );
	virtual void Close( );
};

class CMySQLCallableAdminCount : public CCallableAdminCount, public CMySQLCallable
{
public:
	CMySQLCallableAdminCount( QString nServer, void *nConnection, uint32_t nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableAdminCount( nServer ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableAdminCount( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableAdminCheck : public CCallableAdminCheck, public CMySQLCallable
{
public:
	CMySQLCallableAdminCheck( QString nServer, QString nUser, void *nConnection, uint32_t nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableAdminCheck( nServer, nUser ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableAdminCheck( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableAdminAdd : public CCallableAdminAdd, public CMySQLCallable
{
public:
	CMySQLCallableAdminAdd( QString nServer, QString nUser, void *nConnection, uint32_t nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableAdminAdd( nServer, nUser ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableAdminAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableAdminRemove : public CCallableAdminRemove, public CMySQLCallable
{
public:
	CMySQLCallableAdminRemove( QString nServer, QString nUser, void *nConnection, uint32_t nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableAdminRemove( nServer, nUser ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableAdminRemove( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableAdminList : public CCallableAdminList, public CMySQLCallable
{
public:
	CMySQLCallableAdminList( QString nServer, void *nConnection, uint32_t nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableAdminList( nServer ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableAdminList( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableBanCount : public CCallableBanCount, public CMySQLCallable
{
public:
	CMySQLCallableBanCount( QString nServer, void *nConnection, uint32_t nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableBanCount( nServer ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableBanCount( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableBanCheck : public CCallableBanCheck, public CMySQLCallable
{
public:
	CMySQLCallableBanCheck( QString nServer, QString nUser, QString nIP, void *nConnection, uint32_t nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableBanCheck( nServer, nUser, nIP ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableBanCheck( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableBanAdd : public CCallableBanAdd, public CMySQLCallable
{
public:
	CMySQLCallableBanAdd( QString nServer, QString nUser, QString nIP, QString nGameName, QString nAdmin, QString nReason, void *nConnection, uint32_t nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableBanAdd( nServer, nUser, nIP, nGameName, nAdmin, nReason ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableBanAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableBanRemove : public CCallableBanRemove, public CMySQLCallable
{
public:
	CMySQLCallableBanRemove( QString nServer, QString nUser, void *nConnection, uint32_t nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableBanRemove( nServer, nUser ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableBanRemove( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableBanList : public CCallableBanList, public CMySQLCallable
{
public:
	CMySQLCallableBanList( QString nServer, void *nConnection, uint32_t nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableBanList( nServer ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableBanList( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableGameAdd : public CCallableGameAdd, public CMySQLCallable
{
public:
	CMySQLCallableGameAdd( QString nServer, QString nMap, QString nGameName, QString nOwnerName, uint32_t nDuration, uint32_t nGameState, QString nCreatorName, QString nCreatorServer, void *nConnection, uint32_t nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableGameAdd( nServer, nMap, nGameName, nOwnerName, nDuration, nGameState, nCreatorName, nCreatorServer ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableGameAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableGamePlayerAdd : public CCallableGamePlayerAdd, public CMySQLCallable
{
public:
	CMySQLCallableGamePlayerAdd( uint32_t nGameID, QString nName, QString nIP, uint32_t nSpoofed, QString nSpoofedRealm, uint32_t nReserved, uint32_t nLoadingTime, uint32_t nLeft, QString nLeftReason, uint32_t nTeam, uint32_t nColour, void *nConnection, uint32_t nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableGamePlayerAdd( nGameID, nName, nIP, nSpoofed, nSpoofedRealm, nReserved, nLoadingTime, nLeft, nLeftReason, nTeam, nColour ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableGamePlayerAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableGamePlayerSummaryCheck : public CCallableGamePlayerSummaryCheck, public CMySQLCallable
{
public:
	CMySQLCallableGamePlayerSummaryCheck( QString nName, void *nConnection, uint32_t nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableGamePlayerSummaryCheck( nName ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableGamePlayerSummaryCheck( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableDotAGameAdd : public CCallableDotAGameAdd, public CMySQLCallable
{
public:
	CMySQLCallableDotAGameAdd( uint32_t nGameID, uint32_t nWinner, uint32_t nMin, uint32_t nSec, void *nConnection, uint32_t nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableDotAGameAdd( nGameID, nWinner, nMin, nSec ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableDotAGameAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableDotAPlayerAdd : public CCallableDotAPlayerAdd, public CMySQLCallable
{
public:
	CMySQLCallableDotAPlayerAdd( uint32_t nGameID, uint32_t nColour, uint32_t nKills, uint32_t nDeaths, uint32_t nCreepKills, uint32_t nCreepDenies, uint32_t nAssists, uint32_t nGold, uint32_t nNeutralKills, QString nItem1, QString nItem2, QString nItem3, QString nItem4, QString nItem5, QString nItem6, QString nHero, uint32_t nNewColour, uint32_t nTowerKills, uint32_t nRaxKills, uint32_t nCourierKills, void *nConnection, uint32_t nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableDotAPlayerAdd( nGameID, nColour, nKills, nDeaths, nCreepKills, nCreepDenies, nAssists, nGold, nNeutralKills, nItem1, nItem2, nItem3, nItem4, nItem5, nItem6, nHero, nNewColour, nTowerKills, nRaxKills, nCourierKills ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableDotAPlayerAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableDotAPlayerSummaryCheck : public CCallableDotAPlayerSummaryCheck, public CMySQLCallable
{
public:
	CMySQLCallableDotAPlayerSummaryCheck( QString nName, void *nConnection, uint32_t nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableDotAPlayerSummaryCheck( nName ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableDotAPlayerSummaryCheck( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableDownloadAdd : public CCallableDownloadAdd, public CMySQLCallable
{
public:
	CMySQLCallableDownloadAdd( QString nMap, uint32_t nMapSize, QString nName, QString nIP, uint32_t nSpoofed, QString nSpoofedRealm, uint32_t nDownloadTime, void *nConnection, uint32_t nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableDownloadAdd( nMap, nMapSize, nName, nIP, nSpoofed, nSpoofedRealm, nDownloadTime ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableDownloadAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableScoreCheck : public CCallableScoreCheck, public CMySQLCallable
{
public:
	CMySQLCallableScoreCheck( QString nCategory, QString nName, QString nServer, void *nConnection, uint32_t nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableScoreCheck( nCategory, nName, nServer ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableScoreCheck( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableW3MMDPlayerAdd : public CCallableW3MMDPlayerAdd, public CMySQLCallable
{
public:
	CMySQLCallableW3MMDPlayerAdd( QString nCategory, uint32_t nGameID, uint32_t nPID, QString nName, QString nFlag, uint32_t nLeaver, uint32_t nPracticing, void *nConnection, uint32_t nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableW3MMDPlayerAdd( nCategory, nGameID, nPID, nName, nFlag, nLeaver, nPracticing ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableW3MMDPlayerAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

class CMySQLCallableW3MMDVarAdd : public CCallableW3MMDVarAdd, public CMySQLCallable
{
public:
	CMySQLCallableW3MMDVarAdd( uint32_t nGameID, QMap<VarP,int32_t> nVarInts, void *nConnection, uint32_t nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableW3MMDVarAdd( nGameID, nVarInts ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	CMySQLCallableW3MMDVarAdd( uint32_t nGameID, QMap<VarP,double> nVarReals, void *nConnection, uint32_t nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableW3MMDVarAdd( nGameID, nVarReals ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	CMySQLCallableW3MMDVarAdd( uint32_t nGameID, QMap<VarP,string> nVarStrings, void *nConnection, uint32_t nSQLBotID, QString nSQLServer, QString nSQLDatabase, QString nSQLUser, QString nSQLPassword, uint16_t nSQLPort ) : CBaseCallable( ), CCallableW3MMDVarAdd( nGameID, nVarStrings ), CMySQLCallable( nConnection, nSQLBotID, nSQLServer, nSQLDatabase, nSQLUser, nSQLPassword, nSQLPort ) { }
	virtual ~CMySQLCallableW3MMDVarAdd( ) { }

	virtual void operator( )( );
	virtual void Init( ) { CMySQLCallable :: Init( ); }
	virtual void Close( ) { CMySQLCallable :: Close( ); }
};

#endif

#endif
