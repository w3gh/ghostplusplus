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

#include "ghost.h"
#include "util.h"
#include "config.h"
#include "ghostdb.h"
#include "ghostdbmysql.h"

#include <signal.h>

#ifdef WIN32
 #include <winsock.h>
#endif

#include <mysql/mysql.h>
#include <boost/thread.hpp>

//
// CGHostDBMySQL
//

CGHostDBMySQL :: CGHostDBMySQL( CConfig *CFG ) : CGHostDB( CFG )
{
	m_Server = CFG->GetString( "db_mysql_server", QString( ) );
	m_Database = CFG->GetString( "db_mysql_database", "ghost" );
	m_User = CFG->GetString( "db_mysql_user", QString( ) );
	m_Password = CFG->GetString( "db_mysql_password", QString( ) );
	m_Port = CFG->GetInt( "db_mysql_port", 0 );
	m_BotID = CFG->GetInt( "db_mysql_botid", 0 );
	m_NumConnections = 1;
	m_OutstandingCallables = 0;

	mysql_library_init( 0, NULL, NULL );

	// create the first connection

	CONSOLE_Print( "[MYSQL] connecting to database server" );
	MYSQL *Connection = NULL;

	if( !( Connection = mysql_init( NULL ) ) )
	{
		CONSOLE_Print( QString( "[MYSQL] " ) + mysql_error( Connection ) );
		m_HasError = true;
		m_Error = "error initializing MySQL connection";
		return;
	}

	my_bool Reconnect = true;
	mysql_options( Connection, MYSQL_OPT_RECONNECT, &Reconnect );

	if( !( mysql_real_connect( Connection, m_Server.c_str( ), m_User.c_str( ), m_Password.c_str( ), m_Database.c_str( ), m_Port, NULL, 0 ) ) )
	{
		CONSOLE_Print( QString( "[MYSQL] " ) + mysql_error( Connection ) );
		m_HasError = true;
		m_Error = "error connecting to MySQL server";
		return;
	}

	m_IdleConnections.push( Connection );
}

CGHostDBMySQL :: ~CGHostDBMySQL( )
{
	CONSOLE_Print( "[MYSQL] closing " + UTIL_ToString( m_IdleConnections.size( ) ) + "/" + UTIL_ToString( m_NumConnections ) + " idle MySQL connections" );

	while( !m_IdleConnections.empty( ) )
	{
		mysql_close( (MYSQL *)m_IdleConnections.front( ) );
		m_IdleConnections.pop( );
	}

	if( m_OutstandingCallables > 0 )
		CONSOLE_Print( "[MYSQL] " + UTIL_ToString( m_OutstandingCallables ) + " outstanding callables were never recovered" );

	mysql_library_end( );
}

QString CGHostDBMySQL :: GetStatus( )
{
	return "DB STATUS --- Connections: " + UTIL_ToString( m_IdleConnections.size( ) ) + "/" + UTIL_ToString( m_NumConnections ) + " idle. Outstanding callables: " + UTIL_ToString( m_OutstandingCallables ) + ".";
}

void CGHostDBMySQL :: RecoverCallable( CBaseCallable *callable )
{
	CMySQLCallable *MySQLCallable = dynamic_cast<CMySQLCallable *>( callable );

	if( MySQLCallable )
	{
		if( m_IdleConnections.size( ) > 30 )
		{
			mysql_close( (MYSQL *)MySQLCallable->GetConnection( ) );
			m_NumConnections--;
		}
		else
			m_IdleConnections.push( MySQLCallable->GetConnection( ) );

		if( m_OutstandingCallables == 0 )
			CONSOLE_Print( "[MYSQL] recovered a mysql callable with zero outstanding" );
		else
			m_OutstandingCallables--;

		if( !MySQLCallable->GetError( ).empty( ) )
			CONSOLE_Print( "[MYSQL] error --- " + MySQLCallable->GetError( ) );
	}
	else
		CONSOLE_Print( "[MYSQL] tried to recover a non-mysql callable" );
}

void CGHostDBMySQL :: CreateThread( CBaseCallable *callable )
{
	try
	{
		boost :: thread Thread( boost :: ref( *callable ) );
	}
	catch( boost :: thread_resource_error tre )
	{
		CONSOLE_Print( "[MYSQL] error spawning thread on attempt #1 [" + QString( tre.what( ) ) + "], pausing execution and trying again in 50ms" );
		MILLISLEEP( 50 );

		try
		{
			boost :: thread Thread( boost :: ref( *callable ) );
		}
		catch( boost :: thread_resource_error tre2 )
		{
			CONSOLE_Print( "[MYSQL] error spawning thread on attempt #2 [" + QString( tre2.what( ) ) + "], giving up" );
			callable->SetReady( true );
		}
	}
}

CCallableAdminCount *CGHostDBMySQL :: ThreadedAdminCount( QString server )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableAdminCount *Callable = new CMySQLCallableAdminCount( server, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableAdminCheck *CGHostDBMySQL :: ThreadedAdminCheck( QString server, QString user )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableAdminCheck *Callable = new CMySQLCallableAdminCheck( server, user, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableAdminAdd *CGHostDBMySQL :: ThreadedAdminAdd( QString server, QString user )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableAdminAdd *Callable = new CMySQLCallableAdminAdd( server, user, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableAdminRemove *CGHostDBMySQL :: ThreadedAdminRemove( QString server, QString user )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableAdminRemove *Callable = new CMySQLCallableAdminRemove( server, user, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableAdminList *CGHostDBMySQL :: ThreadedAdminList( QString server )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableAdminList *Callable = new CMySQLCallableAdminList( server, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableBanCount *CGHostDBMySQL :: ThreadedBanCount( QString server )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableBanCount *Callable = new CMySQLCallableBanCount( server, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableBanCheck *CGHostDBMySQL :: ThreadedBanCheck( QString server, QString user, QString ip )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableBanCheck *Callable = new CMySQLCallableBanCheck( server, user, ip, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableBanAdd *CGHostDBMySQL :: ThreadedBanAdd( QString server, QString user, QString ip, QString gamename, QString admin, QString reason )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableBanAdd *Callable = new CMySQLCallableBanAdd( server, user, ip, gamename, admin, reason, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableBanRemove *CGHostDBMySQL :: ThreadedBanRemove( QString server, QString user )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableBanRemove *Callable = new CMySQLCallableBanRemove( server, user, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableBanRemove *CGHostDBMySQL :: ThreadedBanRemove( QString user )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableBanRemove *Callable = new CMySQLCallableBanRemove( QString( ), user, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableBanList *CGHostDBMySQL :: ThreadedBanList( QString server )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableBanList *Callable = new CMySQLCallableBanList( server, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableGameAdd *CGHostDBMySQL :: ThreadedGameAdd( QString server, QString map, QString gamename, QString ownername, uint32_t duration, uint32_t gamestate, QString creatorname, QString creatorserver )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableGameAdd *Callable = new CMySQLCallableGameAdd( server, map, gamename, ownername, duration, gamestate, creatorname, creatorserver, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableGamePlayerAdd *CGHostDBMySQL :: ThreadedGamePlayerAdd( uint32_t gameid, QString name, QString ip, uint32_t spoofed, QString spoofedrealm, uint32_t reserved, uint32_t loadingtime, uint32_t left, QString leftreason, uint32_t team, uint32_t colour )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableGamePlayerAdd *Callable = new CMySQLCallableGamePlayerAdd( gameid, name, ip, spoofed, spoofedrealm, reserved, loadingtime, left, leftreason, team, colour, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableGamePlayerSummaryCheck *CGHostDBMySQL :: ThreadedGamePlayerSummaryCheck( QString name )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableGamePlayerSummaryCheck *Callable = new CMySQLCallableGamePlayerSummaryCheck( name, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableDotAGameAdd *CGHostDBMySQL :: ThreadedDotAGameAdd( uint32_t gameid, uint32_t winner, uint32_t min, uint32_t sec )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableDotAGameAdd *Callable = new CMySQLCallableDotAGameAdd( gameid, winner, min, sec, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableDotAPlayerAdd *CGHostDBMySQL :: ThreadedDotAPlayerAdd( uint32_t gameid, uint32_t colour, uint32_t kills, uint32_t deaths, uint32_t creepkills, uint32_t creepdenies, uint32_t assists, uint32_t gold, uint32_t neutralkills, QString item1, QString item2, QString item3, QString item4, QString item5, QString item6, QString hero, uint32_t newcolour, uint32_t towerkills, uint32_t raxkills, uint32_t courierkills )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableDotAPlayerAdd *Callable = new CMySQLCallableDotAPlayerAdd( gameid, colour, kills, deaths, creepkills, creepdenies, assists, gold, neutralkills, item1, item2, item3, item4, item5, item6, hero, newcolour, towerkills, raxkills, courierkills, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableDotAPlayerSummaryCheck *CGHostDBMySQL :: ThreadedDotAPlayerSummaryCheck( QString name )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableDotAPlayerSummaryCheck *Callable = new CMySQLCallableDotAPlayerSummaryCheck( name, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableDownloadAdd *CGHostDBMySQL :: ThreadedDownloadAdd( QString map, uint32_t mapsize, QString name, QString ip, uint32_t spoofed, QString spoofedrealm, uint32_t downloadtime )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableDownloadAdd *Callable = new CMySQLCallableDownloadAdd( map, mapsize, name, ip, spoofed, spoofedrealm, downloadtime, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableScoreCheck *CGHostDBMySQL :: ThreadedScoreCheck( QString category, QString name, QString server )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableScoreCheck *Callable = new CMySQLCallableScoreCheck( category, name, server, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableW3MMDPlayerAdd *CGHostDBMySQL :: ThreadedW3MMDPlayerAdd( QString category, uint32_t gameid, uint32_t pid, QString name, QString flag, uint32_t leaver, uint32_t practicing )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableW3MMDPlayerAdd *Callable = new CMySQLCallableW3MMDPlayerAdd( category, gameid, pid, name, flag, leaver, practicing, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableW3MMDVarAdd *CGHostDBMySQL :: ThreadedW3MMDVarAdd( uint32_t gameid, map<VarP,int32_t> var_ints )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableW3MMDVarAdd *Callable = new CMySQLCallableW3MMDVarAdd( gameid, var_ints, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableW3MMDVarAdd *CGHostDBMySQL :: ThreadedW3MMDVarAdd( uint32_t gameid, map<VarP,double> var_reals )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableW3MMDVarAdd *Callable = new CMySQLCallableW3MMDVarAdd( gameid, var_reals, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

CCallableW3MMDVarAdd *CGHostDBMySQL :: ThreadedW3MMDVarAdd( uint32_t gameid, map<VarP,QString> var_strings )
{
	void *Connection = GetIdleConnection( );

	if( !Connection )
		m_NumConnections++;

	CCallableW3MMDVarAdd *Callable = new CMySQLCallableW3MMDVarAdd( gameid, var_strings, Connection, m_BotID, m_Server, m_Database, m_User, m_Password, m_Port );
	CreateThread( Callable );
	m_OutstandingCallables++;
	return Callable;
}

void *CGHostDBMySQL :: GetIdleConnection( )
{
	void *Connection = NULL;

	if( !m_IdleConnections.empty( ) )
	{
		Connection = m_IdleConnections.front( );
		m_IdleConnections.pop( );
	}

	return Connection;
}

//
// unprototyped global helper functions
//

QString MySQLEscapeString( void *conn, QString str )
{
	char *to = new char[str.size( ) * 2 + 1];
	unsigned long size = mysql_real_escape_QString( (MYSQL *)conn, to, str.c_str( ), str.size( ) );
	QString result( to, size );
	delete [] to;
	return result;
}

vector<QString> MySQLFetchRow( MYSQL_RES *res )
{
	vector<QString> Result;

	MYSQL_ROW Row = mysql_fetch_row( res );

	if( Row )
	{
		unsigned long *Lengths;
		Lengths = mysql_fetch_lengths( res );

		for( unsigned int i = 0; i < mysql_num_fields( res ); i++ )
		{
			if( Row[i] )
				Result.push_back( QString( Row[i], Lengths[i] ) );
			else
				Result.push_back( QString( ) );
		}
	}

	return Result;
}

//
// global helper functions
//

uint32_t MySQLAdminCount( void *conn, QString *error, uint32_t botid, QString server )
{
	QString EscServer = MySQLEscapeString( conn, server );
	uint32_t Count = 0;
	QString Query = "SELECT COUNT(*) FROM admins WHERE server='" + EscServer + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
	{
		MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

		if( Result )
		{
			vector<QString> Row = MySQLFetchRow( Result );

			if( Row.size( ) == 1 )
				Count = UTIL_ToUInt32( Row[0] );
			else
				*error = "error counting admins [" + server + "] - row doesn't have 1 column";

			mysql_free_result( Result );
		}
		else
			*error = mysql_error( (MYSQL *)conn );
	}

	return Count;
}

bool MySQLAdminCheck( void *conn, QString *error, uint32_t botid, QString server, QString user )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	QString EscServer = MySQLEscapeString( conn, server );
	QString EscUser = MySQLEscapeString( conn, user );
	bool IsAdmin = false;
	QString Query = "SELECT * FROM admins WHERE server='" + EscServer + "' AND name='" + EscUser + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
	{
		MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

		if( Result )
		{
			vector<QString> Row = MySQLFetchRow( Result );

			if( !Row.empty( ) )
				IsAdmin = true;

			mysql_free_result( Result );
		}
		else
			*error = mysql_error( (MYSQL *)conn );
	}

	return IsAdmin;
}

bool MySQLAdminAdd( void *conn, QString *error, uint32_t botid, QString server, QString user )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	QString EscServer = MySQLEscapeString( conn, server );
	QString EscUser = MySQLEscapeString( conn, user );
	bool Success = false;
	QString Query = "INSERT INTO admins ( botid, server, name ) VALUES ( " + UTIL_ToString( botid ) + ", '" + EscServer + "', '" + EscUser + "' )";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}

bool MySQLAdminRemove( void *conn, QString *error, uint32_t botid, QString server, QString user )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	QString EscServer = MySQLEscapeString( conn, server );
	QString EscUser = MySQLEscapeString( conn, user );
	bool Success = false;
	QString Query = "DELETE FROM admins WHERE server='" + EscServer + "' AND name='" + EscUser + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}

vector<QString> MySQLAdminList( void *conn, QString *error, uint32_t botid, QString server )
{
	QString EscServer = MySQLEscapeString( conn, server );
	vector<QString> AdminList;
	QString Query = "SELECT name FROM admins WHERE server='" + EscServer + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
	{
		MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

		if( Result )
		{
			vector<QString> Row = MySQLFetchRow( Result );

			while( !Row.empty( ) )
			{
				AdminList.push_back( Row[0] );
				Row = MySQLFetchRow( Result );
			}

			mysql_free_result( Result );
		}
		else
			*error = mysql_error( (MYSQL *)conn );
	}

	return AdminList;
}

uint32_t MySQLBanCount( void *conn, QString *error, uint32_t botid, QString server )
{
	QString EscServer = MySQLEscapeString( conn, server );
	uint32_t Count = 0;
	QString Query = "SELECT COUNT(*) FROM bans WHERE server='" + EscServer + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
	{
		MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

		if( Result )
		{
			vector<QString> Row = MySQLFetchRow( Result );

			if( Row.size( ) == 1 )
				Count = UTIL_ToUInt32( Row[0] );
			else
				*error = "error counting bans [" + server + "] - row doesn't have 1 column";

			mysql_free_result( Result );
		}
		else
			*error = mysql_error( (MYSQL *)conn );
	}

	return Count;
}

CDBBan *MySQLBanCheck( void *conn, QString *error, uint32_t botid, QString server, QString user, QString ip )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	QString EscServer = MySQLEscapeString( conn, server );
	QString EscUser = MySQLEscapeString( conn, user );
	QString EscIP = MySQLEscapeString( conn, ip );
	CDBBan *Ban = NULL;
	QString Query;

	if( ip.empty( ) )
		Query = "SELECT name, ip, DATE(date), gamename, admin, reason FROM bans WHERE server='" + EscServer + "' AND name='" + EscUser + "'";
	else
		Query = "SELECT name, ip, DATE(date), gamename, admin, reason FROM bans WHERE (server='" + EscServer + "' AND name='" + EscUser + "') OR ip='" + EscIP + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
	{
		MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

		if( Result )
		{
			vector<QString> Row = MySQLFetchRow( Result );

			if( Row.size( ) == 6 )
				Ban = new CDBBan( server, Row[0], Row[1], Row[2], Row[3], Row[4], Row[5] );
			/* else
				*error = "error checking ban [" + server + " : " + user + "] - row doesn't have 6 columns"; */

			mysql_free_result( Result );
		}
		else
			*error = mysql_error( (MYSQL *)conn );
	}

	return Ban;
}

bool MySQLBanAdd( void *conn, QString *error, uint32_t botid, QString server, QString user, QString ip, QString gamename, QString admin, QString reason )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	QString EscServer = MySQLEscapeString( conn, server );
	QString EscUser = MySQLEscapeString( conn, user );
	QString EscIP = MySQLEscapeString( conn, ip );
	QString EscGameName = MySQLEscapeString( conn, gamename );
	QString EscAdmin = MySQLEscapeString( conn, admin );
	QString EscReason = MySQLEscapeString( conn, reason );
	bool Success = false;
	QString Query = "INSERT INTO bans ( botid, server, name, ip, date, gamename, admin, reason ) VALUES ( " + UTIL_ToString( botid ) + ", '" + EscServer + "', '" + EscUser + "', '" + EscIP + "', CURDATE( ), '" + EscGameName + "', '" + EscAdmin + "', '" + EscReason + "' )";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}

bool MySQLBanRemove( void *conn, QString *error, uint32_t botid, QString server, QString user )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	QString EscServer = MySQLEscapeString( conn, server );
	QString EscUser = MySQLEscapeString( conn, user );
	bool Success = false;
	QString Query = "DELETE FROM bans WHERE server='" + EscServer + "' AND name='" + EscUser + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}

bool MySQLBanRemove( void *conn, QString *error, uint32_t botid, QString user )
{
	transform( user.begin( ), user.end( ), user.begin( ), (int(*)(int))tolower );
	QString EscUser = MySQLEscapeString( conn, user );
	bool Success = false;
	QString Query = "DELETE FROM bans WHERE name='" + EscUser + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}

vector<CDBBan *> MySQLBanList( void *conn, QString *error, uint32_t botid, QString server )
{
	QString EscServer = MySQLEscapeString( conn, server );
	vector<CDBBan *> BanList;
	QString Query = "SELECT name, ip, DATE(date), gamename, admin, reason FROM bans WHERE server='" + EscServer + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
	{
		MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

		if( Result )
		{
			vector<QString> Row = MySQLFetchRow( Result );

			while( Row.size( ) == 6 )
			{
				BanList.push_back( new CDBBan( server, Row[0], Row[1], Row[2], Row[3], Row[4], Row[5] ) );
				Row = MySQLFetchRow( Result );
			}

			mysql_free_result( Result );
		}
		else
			*error = mysql_error( (MYSQL *)conn );
	}

	return BanList;
}

uint32_t MySQLGameAdd( void *conn, QString *error, uint32_t botid, QString server, QString map, QString gamename, QString ownername, uint32_t duration, uint32_t gamestate, QString creatorname, QString creatorserver )
{
	uint32_t RowID = 0;
	QString EscServer = MySQLEscapeString( conn, server );
	QString EscMap = MySQLEscapeString( conn, map );
	QString EscGameName = MySQLEscapeString( conn, gamename );
	QString EscOwnerName = MySQLEscapeString( conn, ownername );
	QString EscCreatorName = MySQLEscapeString( conn, creatorname );
	QString EscCreatorServer = MySQLEscapeString( conn, creatorserver );
	QString Query = "INSERT INTO games ( botid, server, map, datetime, gamename, ownername, duration, gamestate, creatorname, creatorserver ) VALUES ( " + UTIL_ToString( botid ) + ", '" + EscServer + "', '" + EscMap + "', NOW( ), '" + EscGameName + "', '" + EscOwnerName + "', " + UTIL_ToString( duration ) + ", " + UTIL_ToString( gamestate ) + ", '" + EscCreatorName + "', '" + EscCreatorServer + "' )";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		RowID = mysql_insert_id( (MYSQL *)conn );

	return RowID;
}

uint32_t MySQLGamePlayerAdd( void *conn, QString *error, uint32_t botid, uint32_t gameid, QString name, QString ip, uint32_t spoofed, QString spoofedrealm, uint32_t reserved, uint32_t loadingtime, uint32_t left, QString leftreason, uint32_t team, uint32_t colour )
{
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	uint32_t RowID = 0;
	QString EscName = MySQLEscapeString( conn, name );
	QString EscIP = MySQLEscapeString( conn, ip );
	QString EscSpoofedRealm = MySQLEscapeString( conn, spoofedrealm );
	QString EscLeftReason = MySQLEscapeString( conn, leftreason );
	QString Query = "INSERT INTO gameplayers ( botid, gameid, name, ip, spoofed, reserved, loadingtime, `left`, leftreason, team, colour, spoofedrealm ) VALUES ( " + UTIL_ToString( botid ) + ", " + UTIL_ToString( gameid ) + ", '" + EscName + "', '" + EscIP + "', " + UTIL_ToString( spoofed ) + ", " + UTIL_ToString( reserved ) + ", " + UTIL_ToString( loadingtime ) + ", " + UTIL_ToString( left ) + ", '" + EscLeftReason + "', " + UTIL_ToString( team ) + ", " + UTIL_ToString( colour ) + ", '" + EscSpoofedRealm + "' )";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		RowID = mysql_insert_id( (MYSQL *)conn );

	return RowID;
}

CDBGamePlayerSummary *MySQLGamePlayerSummaryCheck( void *conn, QString *error, uint32_t botid, QString name )
{
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	QString EscName = MySQLEscapeString( conn, name );
	CDBGamePlayerSummary *GamePlayerSummary = NULL;
	QString Query = "SELECT MIN(DATE(datetime)), MAX(DATE(datetime)), COUNT(*), MIN(loadingtime), AVG(loadingtime), MAX(loadingtime), MIN(`left`/duration)*100, AVG(`left`/duration)*100, MAX(`left`/duration)*100, MIN(duration), AVG(duration), MAX(duration) FROM gameplayers LEFT JOIN games ON games.id=gameid WHERE LOWER(name)='" + EscName + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
	{
		MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

		if( Result )
		{
			vector<QString> Row = MySQLFetchRow( Result );

			if( Row.size( ) == 12 )
			{
				QString FirstGameDateTime = Row[0];
				QString LastGameDateTime = Row[1];
				uint32_t TotalGames = UTIL_ToUInt32( Row[2] );
				uint32_t MinLoadingTime = UTIL_ToUInt32( Row[3] );
				uint32_t AvgLoadingTime = UTIL_ToUInt32( Row[4] );
				uint32_t MaxLoadingTime = UTIL_ToUInt32( Row[5] );
				uint32_t MinLeftPercent = UTIL_ToUInt32( Row[6] );
				uint32_t AvgLeftPercent = UTIL_ToUInt32( Row[7] );
				uint32_t MaxLeftPercent = UTIL_ToUInt32( Row[8] );
				uint32_t MinDuration = UTIL_ToUInt32( Row[9] );
				uint32_t AvgDuration = UTIL_ToUInt32( Row[10] );
				uint32_t MaxDuration = UTIL_ToUInt32( Row[11] );
				GamePlayerSummary = new CDBGamePlayerSummary( QString( ), name, FirstGameDateTime, LastGameDateTime, TotalGames, MinLoadingTime, AvgLoadingTime, MaxLoadingTime, MinLeftPercent, AvgLeftPercent, MaxLeftPercent, MinDuration, AvgDuration, MaxDuration );
			}
			else
				*error = "error checking gameplayersummary [" + name + "] - row doesn't have 12 columns";

			mysql_free_result( Result );
		}
		else
			*error = mysql_error( (MYSQL *)conn );
	}

	return GamePlayerSummary;
}

uint32_t MySQLDotAGameAdd( void *conn, QString *error, uint32_t botid, uint32_t gameid, uint32_t winner, uint32_t min, uint32_t sec )
{
	uint32_t RowID = 0;
	QString Query = "INSERT INTO dotagames ( botid, gameid, winner, min, sec ) VALUES ( " + UTIL_ToString( botid ) + ", " + UTIL_ToString( gameid ) + ", " + UTIL_ToString( winner ) + ", " + UTIL_ToString( min ) + ", " + UTIL_ToString( sec ) + " )";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		RowID = mysql_insert_id( (MYSQL *)conn );

	return RowID;
}

uint32_t MySQLDotAPlayerAdd( void *conn, QString *error, uint32_t botid, uint32_t gameid, uint32_t colour, uint32_t kills, uint32_t deaths, uint32_t creepkills, uint32_t creepdenies, uint32_t assists, uint32_t gold, uint32_t neutralkills, QString item1, QString item2, QString item3, QString item4, QString item5, QString item6, QString hero, uint32_t newcolour, uint32_t towerkills, uint32_t raxkills, uint32_t courierkills )
{
	uint32_t RowID = 0;
	QString EscItem1 = MySQLEscapeString( conn, item1 );
	QString EscItem2 = MySQLEscapeString( conn, item2 );
	QString EscItem3 = MySQLEscapeString( conn, item3 );
	QString EscItem4 = MySQLEscapeString( conn, item4 );
	QString EscItem5 = MySQLEscapeString( conn, item5 );
	QString EscItem6 = MySQLEscapeString( conn, item6 );
	QString EscHero = MySQLEscapeString( conn, hero );
	QString Query = "INSERT INTO dotaplayers ( botid, gameid, colour, kills, deaths, creepkills, creepdenies, assists, gold, neutralkills, item1, item2, item3, item4, item5, item6, hero, newcolour, towerkills, raxkills, courierkills ) VALUES ( " + UTIL_ToString( botid ) + ", " + UTIL_ToString( gameid ) + ", " + UTIL_ToString( colour ) + ", " + UTIL_ToString( kills ) + ", " + UTIL_ToString( deaths ) + ", " + UTIL_ToString( creepkills ) + ", " + UTIL_ToString( creepdenies ) + ", " + UTIL_ToString( assists ) + ", " + UTIL_ToString( gold ) + ", " + UTIL_ToString( neutralkills ) + ", '" + EscItem1 + "', '" + EscItem2 + "', '" + EscItem3 + "', '" + EscItem4 + "', '" + EscItem5 + "', '" + EscItem6 + "', '" + EscHero + "', " + UTIL_ToString( newcolour ) + ", " + UTIL_ToString( towerkills ) + ", " + UTIL_ToString( raxkills ) + ", " + UTIL_ToString( courierkills ) + " )";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		RowID = mysql_insert_id( (MYSQL *)conn );

	return RowID;
}

CDBDotAPlayerSummary *MySQLDotAPlayerSummaryCheck( void *conn, QString *error, uint32_t botid, QString name )
{
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	QString EscName = MySQLEscapeString( conn, name );
	CDBDotAPlayerSummary *DotAPlayerSummary = NULL;
	QString Query = "SELECT COUNT(dotaplayers.id), SUM(kills), SUM(deaths), SUM(creepkills), SUM(creepdenies), SUM(assists), SUM(neutralkills), SUM(towerkills), SUM(raxkills), SUM(courierkills) FROM gameplayers LEFT JOIN games ON games.id=gameplayers.gameid LEFT JOIN dotaplayers ON dotaplayers.gameid=games.id AND dotaplayers.colour=gameplayers.colour WHERE LOWER(name)='" + EscName + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
	{
		MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

		if( Result )
		{
			vector<QString> Row = MySQLFetchRow( Result );

			if( Row.size( ) == 10 )
			{
				uint32_t TotalGames = UTIL_ToUInt32( Row[0] );

				if( TotalGames > 0 )
				{
					uint32_t TotalWins = 0;
					uint32_t TotalLosses = 0;
					uint32_t TotalKills = UTIL_ToUInt32( Row[1] );
					uint32_t TotalDeaths = UTIL_ToUInt32( Row[2] );
					uint32_t TotalCreepKills = UTIL_ToUInt32( Row[3] );
					uint32_t TotalCreepDenies = UTIL_ToUInt32( Row[4] );
					uint32_t TotalAssists = UTIL_ToUInt32( Row[5] );
					uint32_t TotalNeutralKills = UTIL_ToUInt32( Row[6] );
					uint32_t TotalTowerKills = UTIL_ToUInt32( Row[7] );
					uint32_t TotalRaxKills = UTIL_ToUInt32( Row[8] );
					uint32_t TotalCourierKills = UTIL_ToUInt32( Row[9] );

					// calculate total wins

					QString Query2 = "SELECT COUNT(*) FROM gameplayers LEFT JOIN games ON games.id=gameplayers.gameid LEFT JOIN dotaplayers ON dotaplayers.gameid=games.id AND dotaplayers.colour=gameplayers.colour LEFT JOIN dotagames ON games.id=dotagames.gameid WHERE name='" + EscName + "' AND ((winner=1 AND dotaplayers.newcolour>=1 AND dotaplayers.newcolour<=5) OR (winner=2 AND dotaplayers.newcolour>=7 AND dotaplayers.newcolour<=11))";

					if( mysql_real_query( (MYSQL *)conn, Query2.c_str( ), Query2.size( ) ) != 0 )
						*error = mysql_error( (MYSQL *)conn );
					else
					{
						MYSQL_RES *Result2 = mysql_store_result( (MYSQL *)conn );

						if( Result2 )
						{
							vector<QString> Row2 = MySQLFetchRow( Result2 );

							if( Row2.size( ) == 1 )
								TotalWins = UTIL_ToUInt32( Row2[0] );
							else
								*error = "error checking dotaplayersummary wins [" + name + "] - row doesn't have 1 column";

							mysql_free_result( Result2 );
						}
						else
							*error = mysql_error( (MYSQL *)conn );
					}

					// calculate total losses

					QString Query3 = "SELECT COUNT(*) FROM gameplayers LEFT JOIN games ON games.id=gameplayers.gameid LEFT JOIN dotaplayers ON dotaplayers.gameid=games.id AND dotaplayers.colour=gameplayers.colour LEFT JOIN dotagames ON games.id=dotagames.gameid WHERE name='" + EscName + "' AND ((winner=2 AND dotaplayers.newcolour>=1 AND dotaplayers.newcolour<=5) OR (winner=1 AND dotaplayers.newcolour>=7 AND dotaplayers.newcolour<=11))";

					if( mysql_real_query( (MYSQL *)conn, Query3.c_str( ), Query3.size( ) ) != 0 )
						*error = mysql_error( (MYSQL *)conn );
					else
					{
						MYSQL_RES *Result3 = mysql_store_result( (MYSQL *)conn );

						if( Result3 )
						{
							vector<QString> Row3 = MySQLFetchRow( Result3 );

							if( Row3.size( ) == 1 )
								TotalLosses = UTIL_ToUInt32( Row3[0] );
							else
								*error = "error checking dotaplayersummary losses [" + name + "] - row doesn't have 1 column";

							mysql_free_result( Result3 );
						}
						else
							*error = mysql_error( (MYSQL *)conn );
					}

					// done

					DotAPlayerSummary = new CDBDotAPlayerSummary( QString( ), name, TotalGames, TotalWins, TotalLosses, TotalKills, TotalDeaths, TotalCreepKills, TotalCreepDenies, TotalAssists, TotalNeutralKills, TotalTowerKills, TotalRaxKills, TotalCourierKills );
				}
			}
			else
				*error = "error checking dotaplayersummary [" + name + "] - row doesn't have 10 columns";

			mysql_free_result( Result );
		}
		else
			*error = mysql_error( (MYSQL *)conn );
	}

	return DotAPlayerSummary;
}

bool MySQLDownloadAdd( void *conn, QString *error, uint32_t botid, QString map, uint32_t mapsize, QString name, QString ip, uint32_t spoofed, QString spoofedrealm, uint32_t downloadtime )
{
	bool Success = false;
	QString EscMap = MySQLEscapeString( conn, map );
	QString EscName = MySQLEscapeString( conn, name );
	QString EscIP = MySQLEscapeString( conn, ip );
	QString EscSpoofedRealm = MySQLEscapeString( conn, spoofedrealm );
	QString Query = "INSERT INTO downloads ( botid, map, mapsize, datetime, name, ip, spoofed, spoofedrealm, downloadtime ) VALUES ( " + UTIL_ToString( botid ) + ", '" + EscMap + "', " + UTIL_ToString( mapsize ) + ", NOW( ), '" + EscName + "', '" + EscIP + "', " + UTIL_ToString( spoofed ) + ", '" + EscSpoofedRealm + "', " + UTIL_ToString( downloadtime ) + " )";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}

double MySQLScoreCheck( void *conn, QString *error, uint32_t botid, QString category, QString name, QString server )
{
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	QString EscCategory = MySQLEscapeString( conn, category );
	QString EscName = MySQLEscapeString( conn, name );
	QString EscServer = MySQLEscapeString( conn, server );
	double Score = -100000.0;
	QString Query = "SELECT score FROM scores WHERE category='" + EscCategory + "' AND name='" + EscName + "' AND server='" + EscServer + "'";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
	{
		MYSQL_RES *Result = mysql_store_result( (MYSQL *)conn );

		if( Result )
		{
			vector<QString> Row = MySQLFetchRow( Result );

			if( Row.size( ) == 1 )
				Score = UTIL_ToDouble( Row[0] );
			/* else
				*error = "error checking score [" + category + " : " + name + " : " + server + "] - row doesn't have 1 column"; */

			mysql_free_result( Result );
		}
		else
			*error = mysql_error( (MYSQL *)conn );
	}

	return Score;
}

uint32_t MySQLW3MMDPlayerAdd( void *conn, QString *error, uint32_t botid, QString category, uint32_t gameid, uint32_t pid, QString name, QString flag, uint32_t leaver, uint32_t practicing )
{
	transform( name.begin( ), name.end( ), name.begin( ), (int(*)(int))tolower );
	uint32_t RowID = 0;
	QString EscCategory = MySQLEscapeString( conn, category );
	QString EscName = MySQLEscapeString( conn, name );
	QString EscFlag = MySQLEscapeString( conn, flag );
	QString Query = "INSERT INTO w3mmdplayers ( botid, category, gameid, pid, name, flag, leaver, practicing ) VALUES ( " + UTIL_ToString( botid ) + ", '" + EscCategory + "', " + UTIL_ToString( gameid ) + ", " + UTIL_ToString( pid ) + ", '" + EscName + "', '" + EscFlag + "', " + UTIL_ToString( leaver ) + ", " + UTIL_ToString( practicing ) + " )";

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		RowID = mysql_insert_id( (MYSQL *)conn );

	return RowID;
}

bool MySQLW3MMDVarAdd( void *conn, QString *error, uint32_t botid, uint32_t gameid, map<VarP,int32_t> var_ints )
{
	if( var_ints.empty( ) )
		return false;

	bool Success = false;
	QString Query;

	for( map<VarP,int32_t> :: iterator i = var_ints.begin( ); i != var_ints.end( ); i++ )
	{
		QString EscVarName = MySQLEscapeString( conn, i->first.second );

		if( Query.empty( ) )
			Query = "INSERT INTO w3mmdvars ( botid, gameid, pid, varname, value_int ) VALUES ( " + UTIL_ToString( botid ) + ", " + UTIL_ToString( gameid ) + ", " + UTIL_ToString( i->first.first ) + ", '" + EscVarName + "', " + UTIL_ToString( i->second ) + " )";
		else
			Query += ", ( " + UTIL_ToString( botid ) + ", " + UTIL_ToString( gameid ) + ", " + UTIL_ToString( i->first.first ) + ", '" + EscVarName + "', " + UTIL_ToString( i->second ) + " )";
	}

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}

bool MySQLW3MMDVarAdd( void *conn, QString *error, uint32_t botid, uint32_t gameid, map<VarP,double> var_reals )
{
	if( var_reals.empty( ) )
		return false;

	bool Success = false;
	QString Query;

	for( map<VarP,double> :: iterator i = var_reals.begin( ); i != var_reals.end( ); i++ )
	{
		QString EscVarName = MySQLEscapeString( conn, i->first.second );

		if( Query.empty( ) )
			Query = "INSERT INTO w3mmdvars ( botid, gameid, pid, varname, value_real ) VALUES ( " + UTIL_ToString( botid ) + ", " + UTIL_ToString( gameid ) + ", " + UTIL_ToString( i->first.first ) + ", '" + EscVarName + "', " + UTIL_ToString( i->second, 10 ) + " )";
		else
			Query += ", ( " + UTIL_ToString( botid ) + ", " + UTIL_ToString( gameid ) + ", " + UTIL_ToString( i->first.first ) + ", '" + EscVarName + "', " + UTIL_ToString( i->second, 10 ) + " )";
	}

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}

bool MySQLW3MMDVarAdd( void *conn, QString *error, uint32_t botid, uint32_t gameid, map<VarP,QString> var_strings )
{
	if( var_strings.empty( ) )
		return false;

	bool Success = false;
	QString Query;

	for( map<VarP,QString> :: iterator i = var_strings.begin( ); i != var_strings.end( ); i++ )
	{
		QString EscVarName = MySQLEscapeString( conn, i->first.second );
		QString EscValueString = MySQLEscapeString( conn, i->second );

		if( Query.empty( ) )
			Query = "INSERT INTO w3mmdvars ( botid, gameid, pid, varname, value_QString ) VALUES ( " + UTIL_ToString( botid ) + ", " + UTIL_ToString( gameid ) + ", " + UTIL_ToString( i->first.first ) + ", '" + EscVarName + "', '" + EscValueString + "' )";
		else
			Query += ", ( " + UTIL_ToString( botid ) + ", " + UTIL_ToString( gameid ) + ", " + UTIL_ToString( i->first.first ) + ", '" + EscVarName + "', '" + EscValueString + "' )";
	}

	if( mysql_real_query( (MYSQL *)conn, Query.c_str( ), Query.size( ) ) != 0 )
		*error = mysql_error( (MYSQL *)conn );
	else
		Success = true;

	return Success;
}

//
// MySQL Callables
//

void CMySQLCallable :: Init( )
{
	CBaseCallable :: Init( );

#ifndef WIN32
	// disable SIGPIPE since this is (or should be) a new thread and it doesn't inherit the spawning thread's signal handlers
	// MySQL should automatically disable SIGPIPE when we initialize it but we do so anyway here

	signal( SIGPIPE, SIG_IGN );
#endif

	mysql_thread_init( );

	if( !m_Connection )
	{
		if( !( m_Connection = mysql_init( NULL ) ) )
			m_Error = mysql_error( (MYSQL *)m_Connection );

		my_bool Reconnect = true;
		mysql_options( (MYSQL *)m_Connection, MYSQL_OPT_RECONNECT, &Reconnect );

		if( !( mysql_real_connect( (MYSQL *)m_Connection, m_SQLServer.c_str( ), m_SQLUser.c_str( ), m_SQLPassword.c_str( ), m_SQLDatabase.c_str( ), m_SQLPort, NULL, 0 ) ) )
			m_Error = mysql_error( (MYSQL *)m_Connection );
	}
	else if( mysql_ping( (MYSQL *)m_Connection ) != 0 )
		m_Error = mysql_error( (MYSQL *)m_Connection );
}

void CMySQLCallable :: Close( )
{
	mysql_thread_end( );

	CBaseCallable :: Close( );
}

void CMySQLCallableAdminCount :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLAdminCount( m_Connection, &m_Error, m_SQLBotID, m_Server );

	Close( );
}

void CMySQLCallableAdminCheck :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLAdminCheck( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User );

	Close( );
}

void CMySQLCallableAdminAdd :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLAdminAdd( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User );

	Close( );
}

void CMySQLCallableAdminRemove :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLAdminRemove( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User );

	Close( );
}

void CMySQLCallableAdminList :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLAdminList( m_Connection, &m_Error, m_SQLBotID, m_Server );

	Close( );
}

void CMySQLCallableBanCount :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLBanCount( m_Connection, &m_Error, m_SQLBotID, m_Server );

	Close( );
}

void CMySQLCallableBanCheck :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLBanCheck( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User, m_IP );

	Close( );
}

void CMySQLCallableBanAdd :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLBanAdd( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User, m_IP, m_GameName, m_Admin, m_Reason );

	Close( );
}

void CMySQLCallableBanRemove :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
	{
		if( m_Server.empty( ) )
			m_Result = MySQLBanRemove( m_Connection, &m_Error, m_SQLBotID, m_User );
		else
			m_Result = MySQLBanRemove( m_Connection, &m_Error, m_SQLBotID, m_Server, m_User );
	}

	Close( );
}

void CMySQLCallableBanList :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLBanList( m_Connection, &m_Error, m_SQLBotID, m_Server );

	Close( );
}

void CMySQLCallableGameAdd :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLGameAdd( m_Connection, &m_Error, m_SQLBotID, m_Server, m_Map, m_GameName, m_OwnerName, m_Duration, m_GameState, m_CreatorName, m_CreatorServer );

	Close( );
}

void CMySQLCallableGamePlayerAdd :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLGamePlayerAdd( m_Connection, &m_Error, m_SQLBotID, m_GameID, m_Name, m_IP, m_Spoofed, m_SpoofedRealm, m_Reserved, m_LoadingTime, m_Left, m_LeftReason, m_Team, m_Colour );

	Close( );
}

void CMySQLCallableGamePlayerSummaryCheck :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLGamePlayerSummaryCheck( m_Connection, &m_Error, m_SQLBotID, m_Name );

	Close( );
}

void CMySQLCallableDotAGameAdd :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLDotAGameAdd( m_Connection, &m_Error, m_SQLBotID, m_GameID, m_Winner, m_Min, m_Sec );

	Close( );
}

void CMySQLCallableDotAPlayerAdd :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLDotAPlayerAdd( m_Connection, &m_Error, m_SQLBotID, m_GameID, m_Colour, m_Kills, m_Deaths, m_CreepKills, m_CreepDenies, m_Assists, m_Gold, m_NeutralKills, m_Item1, m_Item2, m_Item3, m_Item4, m_Item5, m_Item6, m_Hero, m_NewColour, m_TowerKills, m_RaxKills, m_CourierKills );

	Close( );
}

void CMySQLCallableDotAPlayerSummaryCheck :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLDotAPlayerSummaryCheck( m_Connection, &m_Error, m_SQLBotID, m_Name );

	Close( );
}

void CMySQLCallableDownloadAdd :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLDownloadAdd( m_Connection, &m_Error, m_SQLBotID, m_Map, m_MapSize, m_Name, m_IP, m_Spoofed, m_SpoofedRealm, m_DownloadTime );

	Close( );
}

void CMySQLCallableScoreCheck :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLScoreCheck( m_Connection, &m_Error, m_SQLBotID, m_Category, m_Name, m_Server );

	Close( );
}

void CMySQLCallableW3MMDPlayerAdd :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
		m_Result = MySQLW3MMDPlayerAdd( m_Connection, &m_Error, m_SQLBotID, m_Category, m_GameID, m_PID, m_Name, m_Flag, m_Leaver, m_Practicing );

	Close( );
}

void CMySQLCallableW3MMDVarAdd :: operator( )( )
{
	Init( );

	if( m_Error.empty( ) )
	{
		if( m_ValueType == VALUETYPE_INT )
			m_Result = MySQLW3MMDVarAdd( m_Connection, &m_Error, m_SQLBotID, m_GameID, m_VarInts );
		else if( m_ValueType == VALUETYPE_REAL )
			m_Result = MySQLW3MMDVarAdd( m_Connection, &m_Error, m_SQLBotID, m_GameID, m_VarReals );
		else
			m_Result = MySQLW3MMDVarAdd( m_Connection, &m_Error, m_SQLBotID, m_GameID, m_VarStrings );
	}

	Close( );
}

#endif
