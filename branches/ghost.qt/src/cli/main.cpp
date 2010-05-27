#include <signal.h>
#include <QtCore/QCoreApplication>
#include <QTime>
#include <QString>
#include <QFile>
#include <QTextStream>

#include "ghost.h"
#include "util.h"
#include "config.h"
#ifdef WIN32
#include "windows.h"
#endif

QFile gLogFile;
QTextStream gLogStream(&gLogFile);

QString gCFGFile;
quint32 gLogMethod;
CGHost *gGHost = NULL;

void SignalCatcher2( int s )
{
	CONSOLE_Print( "[!!!] caught signal " + QString::number( s ) + ", exiting NOW" );
	
	if( gGHost )
	{
		if( gGHost->GetExiting( ) )
			exit( 1 );
		else
			gGHost->Exit( );
	}
	else
		exit( 1 );
}

void SignalCatcher( int s )
{
	// signal( SIGABRT, SignalCatcher2 );
	signal( SIGINT, SignalCatcher2 );
	
	CONSOLE_Print( "[!!!] caught signal " + QString::number( s ) + ", exiting nicely" );
	
	if( gGHost )
		gGHost->EventExitNice();
	else
		exit( 1 );
}

//
// main
//

#include "mpqarchive.h"
#include "mpqfile.h"

int main( int argc, char **argv )
{
	QCoreApplication a(argc, argv);

	/*MPQArchive arch("/mnt/disk/Programme/Warcraft III/War3Patch.mpq");
	if (!arch.open() && arch.m_Error != MPQArchive::NO_FILE_LIST)
	{
		DEBUG_Print("Failed to open, error " + QString::number(arch.m_Error));
		exit(1);
	}*/



	/*if (!arch.m_ListFile->read())
	{
		DEBUG_Print("Failed to read file list, error " + QString::number(arch.m_ListFile->m_Error));
	}*/


	/*DEBUG_Print("Successfully open MPQ file");

	MPQFile* common_j = arch.getFile("Scripts\\common.j");

	if (!common_j->read())
	{
		DEBUG_Print("Failed to read common.j, error " + QString::number(common_j->m_Error));
	}
	exit(0);*/

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
	// increase process priority

	CONSOLE_Print( "[GHOST] setting process priority to \"above normal\"" );
	SetPriorityClass( GetCurrentProcess( ), ABOVE_NORMAL_PRIORITY_CLASS );
#endif

	// initialize ghost

	gGHost = new CGHost( &CFG, gCFGFile );

	QObject::connect(gGHost, SIGNAL(destroyed()), &a, SLOT(deleteLater()));

	return a.exec();
}
