/*
 *  loggingclass.h
 *  Genie
 *
 *  Created by Lucas on 27.03.10.
 *  Copyright 2010 Lucas Romero. All rights reserved.
 *
 */

#ifndef LOGGINGCLASS_H
#define LOGGINGCLASS_H

#include <string>
#include <iostream>
using namespace std;

typedef void (*LogMessageHandler)( const string &message );

class MessageLogger
{
public:
	MessageLogger( MessageLogger *parentLogger ) : parent( parentLogger), messageHandler( NULL )
	{ /* empty constructor */ }
	MessageLogger( LogMessageHandler msgHandler ) : parent( NULL ), messageHandler( msgHandler )
	{ /* empty constructor */ }
	
	void LogInfo( const string &msg )
	{
		WriteLine( msg );
	}
	
	void WriteLine( const string &msg )
	{
		if( parent != NULL ) {
			// let the parent handle the message
			parent->LogInfo( msg );
		}
		else if ( messageHandler != NULL ) {
			// call function provided by user
			messageHandler( msg );
		}
		else {
			// got nowhere to log to, print to stdout
			cout << msg << endl;
		}
	}
protected:
	MessageLogger *parent;
	LogMessageHandler messageHandler;
};

#endif