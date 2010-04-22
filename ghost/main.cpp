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
quint32 gLogMethod;
CGHost *gGHost = NULL;

quint32 GetTime()
{
	return GetTicks( ) / 1000;
}

quint32 GetTicks()
{
	return gBasicTime.elapsed();
}

void SignalCatcher2( int s )
{
	CONSOLE_Print( "[!!!] caught signal " + QString::number( s ) + ", exiting NOW" );

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

	CONSOLE_Print( "[!!!] caught signal " + QString::number( s ) + ", exiting nicely" );

	if( gGHost )
		gGHost->EventExitNice();
	else
		exit( 1 );
}

void CONSOLE_Print( QString message )
{
	cout << message.toStdString() << endl;

	// logging

	/*if (!gLogFile.fileName().isEmpty())
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
	}*/
}

void DEBUG_Print( QString message )
{
	cout << message.toStdString() << endl;
}

//
// main
//

#include "crc32.h"
int main( int argc, char **argv )
{
	QCoreApplication a(argc, argv);

	/*DEBUG_Print(Util::fromUInt32(1111).toHex());
	return 0;*/

/*
	QByteArray data = QByteArray::fromHex("0224001b019a0700009a070000160101009a0700009a0700001a19303030559a0700009a070000");
	CCRC32 *crc = new CCRC32();
	crc->Initialize();
	DEBUG_Print("Expected: 868c6fa1");
	QByteArray crc32 = UTIL_CreateBYTEARRAY( crc->FullCRC( data ), false );
	// expected: bb76fe69
	DEBUG_Print(crc32.toHex());
	crc32.resize( 2 );
	DEBUG_Print(crc32.toHex());

	quint16 crc16 = qChecksum("ABCDEF", 6);
	DEBUG_Print(UTIL_CreateBYTEARRAY(crc16, false).toHex()); // 5e6f
	return 0;*/

	gCFGFile = "ghost.cfg";

	if( argc > 1 && argv[1] )
		gCFGFile = argv[1];

	// read config file

	gBasicTime.start();
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
