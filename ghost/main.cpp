#include <signal.h>
#include <QtCore/QCoreApplication>
#include <QTime>
#include <QString>
#include <QFile>
#include <QTextStream>

#include "ghost.h"
#include "util.h"
#include "config.h"

QTime gBasicTime;
QFile gLogFile;
QTextStream gLogStream(&gLogFile);

QString gCFGFile;
uint32_t gLogMethod;
CGHost *gGHost = NULL;

uint32_t GetTime()
{
	return GetTicks( ) / 1000;
}

uint32_t GetTicks()
{
	return gBasicTime.msec();
}

void SignalCatcher2( int s )
{
	CONSOLE_Print( "[!!!] caught signal " + UTIL_ToString( s ) + ", exiting NOW" );

	if( gGHost )
	{
		if( gGHost->m_Exiting )
			exit( 1 );
		else
			gGHost->m_Exiting = true;
	}
	else
		exit( 1 );
}

void SignalCatcher( int s )
{
	// signal( SIGABRT, SignalCatcher2 );
	signal( SIGINT, SignalCatcher2 );

	CONSOLE_Print( "[!!!] caught signal " + UTIL_ToString( s ) + ", exiting nicely" );

	if( gGHost )
		gGHost->m_ExitingNice = true;
	else
		exit( 1 );
}

void CONSOLE_Print( QString message )
{
	cout << message.toStdString() << endl;

	// logging

	if (!gLogFile.fileName().isEmpty())
	{
		if( gLogMethod == 1 )
		{
			gLogFile.open(QFile::WriteOnly | QFile::Append);

			if( gLogFile.isWritable() )
			{
				gLogStream << "[" << QTime::currentTime().toString() << "] " << message << endl;
				gLogFile.close();
			}
		}
		else if( gLogMethod == 2 && gLogFile.isWritable() )
			gLogStream << "[" << QTime::currentTime().toString() << "] " << message << endl;
	}
}

void DEBUG_Print( QString message )
{
	cout << message.toStdString() << endl;
}

void DEBUG_Print( QByteArray b )
{
	cout << "{ ";

	for( unsigned int i = 0; i < b.size( ); i++ )
		cout << hex << (int)b[i] << " ";

	cout << "}" << endl;
}

//
// main
//

int main( int argc, char **argv )
{
	QCoreApplication a(argc, argv);

	gCFGFile = "ghost.cfg";

	if( argc > 1 && argv[1] )
		gCFGFile = argv[1];

	// read config file

	CConfig CFG;
	CFG.Read( "default.cfg" );
	CFG.Read( gCFGFile );
	gLogFile.setFileName(CFG.GetString( "bot_log", QString( ) ));
	gLogMethod = CFG.GetInt( "bot_logmethod", 1 );

	if( !gLogFile.fileName().isEmpty() )
	{
		if( gLogMethod == 1 )
		{
			// log method 1: open, append, and close the log for every message
			// this works well on Linux but poorly on Windows, particularly as the log file grows in size
			// the log file can be edited/moved/deleted while GHost++ is running
		}
		else if( gLogMethod == 2 )
		{
			// log method 2: open the log on startup, flush the log for every message, close the log on shutdown
			// the log file CANNOT be edited/moved/deleted while GHost++ is running

			gLogFile.open(QFile::WriteOnly | QFile::Append);
		}
	}

	CONSOLE_Print( "[GHOST] starting up" );

	if( !gLogFile.fileName().isEmpty() )
	{
		if( gLogMethod == 1 )
			CONSOLE_Print( "[GHOST] using log method 1, logging is enabled and [" + gLogFile.fileName() + "] will not be locked" );
		else if( gLogMethod == 2 )
		{
			if( gLogFile.error() != QFile::NoError )
				CONSOLE_Print( "[GHOST] using log method 2 but unable to open [" + gLogFile.fileName() + "] for appending, logging is disabled" );
			else
				CONSOLE_Print( "[GHOST] using log method 2, logging is enabled and [" + gLogFile.fileName() + "] is now locked" );
		}
	}
	else
		CONSOLE_Print( "[GHOST] no log file specified, logging is disabled" );

	// catch SIGABRT and SIGINT

	// signal( SIGABRT, SignalCatcher );
	signal( SIGINT, SignalCatcher );

#ifndef WIN32
	// disable SIGPIPE since some systems like OS X don't define MSG_NOSIGNAL

	signal( SIGPIPE, SIG_IGN );
#endif

#ifdef WIN32
	// initialize winsock

	CONSOLE_Print( "[GHOST] starting winsock" );
	WSADATA wsadata;

	if( WSAStartup( MAKEWORD( 2, 2 ), &wsadata ) != 0 )
	{
		CONSOLE_Print( "[GHOST] error starting winsock" );
		return 1;
	}

	// increase process priority

	CONSOLE_Print( "[GHOST] setting process priority to \"above normal\"" );
	SetPriorityClass( GetCurrentProcess( ), ABOVE_NORMAL_PRIORITY_CLASS );
#endif

	// initialize ghost

	gGHost = new CGHost( &CFG, gCFGFile );

	int ret = a.exec();

	// shutdown ghost

	CONSOLE_Print( "[GHOST] shutting down" );
	delete gGHost;
	gGHost = NULL;

#ifdef WIN32
	// shutdown winsock

	CONSOLE_Print( "[GHOST] shutting down winsock" );
	WSACleanup( );

	// shutdown timer

	timeEndPeriod( TimerResolution );
#endif

	return ret;
}
