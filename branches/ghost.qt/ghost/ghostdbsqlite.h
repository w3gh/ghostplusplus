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

#ifndef GHOSTDBSQLITE_H
#define GHOSTDBSQLITE_H

/**************
 *** SCHEMA ***
 **************

CREATE TABLE admins (
	id INTEGER PRIMARY KEY,
	name TEXT NOT NULL,
	server TEXT NOT NULL DEFAULT ""
)

CREATE TABLE bans (
	id INTEGER PRIMARY KEY,
	server TEXT NOT NULL,
	name TEXT NOT NULL,
	ip TEXT,
	date TEXT NOT NULL,
	gamename TEXT,
	admin TEXT NOT NULL,
	reason TEXT
)

CREATE TABLE games (
	id INTEGER PRIMARY KEY,
	server TEXT NOT NULL,
	map TEXT NOT NULL,
	datetime TEXT NOT NULL,
	gamename TEXT NOT NULL,
	ownername TEXT NOT NULL,
	duration INTEGER NOT NULL,
	gamestate INTEGER NOT NULL DEFAULT 0,
	creatorname TEXT NOT NULL DEFAULT "",
	creatorserver TEXT NOT NULL DEFAULT ""
)

CREATE TABLE gameplayers (
	id INTEGER PRIMARY KEY,
	gameid INTEGER NOT NULL,
	name TEXT NOT NULL,
	ip TEXT NOT NULL,
	spoofed INTEGER NOT NULL,
	reserved INTEGER NOT NULL,
	loadingtime INTEGER NOT NULL,
	left INTEGER NOT NULL,
	leftreason TEXT NOT NULL,
	team INTEGER NOT NULL,
	colour INTEGER NOT NULL,
	spoofedrealm TEXT NOT NULL DEFAULT ""
)

CREATE TABLE dotagames (
	id INTEGER PRIMARY KEY,
	gameid INTEGER NOT NULL,
	winner INTEGER NOT NULL,
	min INTEGER NOT NULL DEFAULT 0,
	sec INTEGER NOT NULL DEFAULT 0
)

CREATE TABLE dotaplayers (
	id INTEGER PRIMARY KEY,
	gameid INTEGER NOT NULL,
	colour INTEGER NOT NULL,
	kills INTEGER NOT NULL,
	deaths INTEGER NOT NULL,
	creepkills INTEGER NOT NULL,
	creepdenies INTEGER NOT NULL,
	assists INTEGER NOT NULL,
	gold INTEGER NOT NULL,
	neutralkills INTEGER NOT NULL,
	item1 TEXT NOT NULL,
	item2 TEXT NOT NULL,
	item3 TEXT NOT NULL,
	item4 TEXT NOT NULL,
	item5 TEXT NOT NULL,
	item6 TEXT NOT NULL,
	hero TEXT NOT NULL DEFAULT "",
	newcolour NOT NULL DEFAULT 0,
	towerkills NOT NULL DEFAULT 0,
	raxkills NOT NULL DEFAULT 0,
	courierkills NOT NULL DEFAULT 0
)

CREATE TABLE config (
	name TEXT NOT NULL PRIMARY KEY,
	value TEXT NOT NULL
)

CREATE TABLE downloads (
	id INTEGER PRIMARY KEY,
	map TEXT NOT NULL,
	mapsize INTEGER NOT NULL,
	datetime TEXT NOT NULL,
	name TEXT NOT NULL,
	ip TEXT NOT NULL,
	spoofed INTEGER NOT NULL,
	spoofedrealm TEXT NOT NULL,
	downloadtime INTEGER NOT NULL
)

CREATE TABLE w3mmdplayers (
	id INTEGER PRIMARY KEY,
	category TEXT NOT NULL,
	gameid INTEGER NOT NULL,
	pid INTEGER NOT NULL,
	name TEXT NOT NULL,
	flag TEXT NOT NULL,
	leaver INTEGER NOT NULL,
	practicing INTEGER NOT NULL
)

CREATE TABLE w3mmdvars (
	id INTEGER PRIMARY KEY,
	gameid INTEGER NOT NULL,
	pid INTEGER NOT NULL,
	varname TEXT NOT NULL,
	value_int INTEGER DEFAULT NULL,
	value_real REAL DEFAULT NULL,
	value_string TEXT DEFAULT NULL
)

CREATE TEMPORARY TABLE iptocountry (
	ip1 INTEGER NOT NULL,
	ip2 INTEGER NOT NULL,
	country TEXT NOT NULL,
	PRIMARY KEY ( ip1, ip2 )
)

CREATE INDEX idx_gameid ON gameplayers ( gameid )
CREATE INDEX idx_gameid_colour ON dotaplayers ( gameid, colour )

 **************
 *** SCHEMA ***
 **************/

//
// CSQLITE3 (wrapper class)
//

class CSQLITE3
{
private:
	void *m_DB;
	bool m_Ready;
	QVector<QString> m_Row;

public:
	CSQLITE3( QString filename );
	~CSQLITE3( );

	bool GetReady( )			{ return m_Ready; }
	QVector<QString> *GetRow( )	{ return &m_Row; }
	QString GetError( );

	int Prepare( QString query, void **Statement );
	int Step( void *Statement );
	int Finalize( void *Statement );
	int Reset( void *Statement );
	int ClearBindings( void *Statement );
	int Exec( QString query );
	quint32 LastRowID( );
};

//
// CGHostDBSQLite
//

#include "includes.h"

class CGHostDBSQLite : public CGHostDB
{
private:
	QString m_File;
	CSQLITE3 *m_DB;

	// we keep some prepared statements in memory rather than recreating them each function call
	// this is an optimization because preparing statements takes time
	// however it only pays off if you're going to be using the statement extremely often

	void *FromAddStmt;

public:
	CGHostDBSQLite( CConfig *CFG );
	virtual ~CGHostDBSQLite( );

	virtual void Upgrade1_2( );
	virtual void Upgrade2_3( );
	virtual void Upgrade3_4( );
	virtual void Upgrade4_5( );
	virtual void Upgrade5_6( );
	virtual void Upgrade6_7( );
	virtual void Upgrade7_8( );

	virtual bool Begin( );
	virtual bool Commit( );
	virtual quint32 AdminCount( QString server );
	virtual bool AdminCheck( QString server, QString user );
	virtual bool AdminAdd( QString server, QString user );
	virtual bool AdminRemove( QString server, QString user );
	virtual QVector<QString> AdminList( QString server );
	virtual quint32 BanCount( QString server );
	virtual CDBBan *BanCheck( QString server, QString user, QString ip );
	virtual bool BanAdd( QString server, QString user, QString ip, QString gamename, QString admin, QString reason );
	virtual bool BanRemove( QString server, QString user );
	virtual bool BanRemove( QString user );
	virtual QVector<CDBBan *> BanList( QString server );
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
	virtual bool W3MMDVarAdd( quint32 gameid, QMap<VarP,int32_t> var_ints );
	virtual bool W3MMDVarAdd( quint32 gameid, QMap<VarP,double> var_reals );
	virtual bool W3MMDVarAdd( quint32 gameid, QMap<VarP,QString> var_strings );

	// threaded database functions
	// note: these are not actually implemented with threads at the moment, they WILL block until the query is complete
	// todotodo: implement threads here

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
	virtual CCallableW3MMDPlayerAdd *ThreadedW3MMDPlayerAdd( QString category, quint32 gameid, quint32 pid, QString name, QString flag, quint32 leaver, quint32 practicing );
	virtual CCallableW3MMDVarAdd *ThreadedW3MMDVarAdd( quint32 gameid, QMap<VarP,int32_t> var_ints );
	virtual CCallableW3MMDVarAdd *ThreadedW3MMDVarAdd( quint32 gameid, QMap<VarP,double> var_reals );
	virtual CCallableW3MMDVarAdd *ThreadedW3MMDVarAdd( quint32 gameid, QMap<VarP,QString> var_strings );
};

#endif
