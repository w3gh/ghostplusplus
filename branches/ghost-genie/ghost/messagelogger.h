/*	messagelogger.h
 *
 *	This file is part of Genie
 *	Copyright (C) 2009-2010 Lucas Romero
 *	Created 27.03.10
 *
 *	Genie is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	Genie is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 * 	You should have received a copy of the GNU General Public License
 * 	along with Genie.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LOGGINGCLASS_H
#define LOGGINGCLASS_H

#include <string>
#include <iostream>
using namespace std;

typedef void (*LogMessageHandler)( void *callbackObject, const string &message );

class MessageLogger
{
public:
	MessageLogger( MessageLogger *parentLogger ) : Parent( parentLogger), MessageHandler( NULL ), CallbackObject( NULL ), enabled( true )
	{ /* empty constructor */ }
	MessageLogger( LogMessageHandler msgHandler, void *callbackObject ) : Parent( NULL ), MessageHandler( msgHandler ), CallbackObject( callbackObject ), enabled( true )
	{ /* empty constructor */ }
	
	bool Enabled( ) { return enabled; }
	void SetEnabled( bool value ) { enabled = value; }
	
	void LogInfo( const string &msg )
	{
		WriteLine( msg );
	}
	
	void LogWarning( const string &msg )
	{
		WriteLine( msg );
	}
	
	void LogError( const string &msg )
	{
		WriteLine( msg );
	}
	
	void WriteLine( const string &msg )
	{
		if( !enabled )
			return;
		if( Parent != NULL ) {
			// let the parent handle the message
			Parent->LogInfo( msg );
		}
		else if ( MessageHandler != NULL ) {
			// call function provided by user
			MessageHandler( CallbackObject, msg );
		}
		/*else {
			// got nowhere to log to, print to stdout
			cout << msg << endl;
		}*/
	}
protected:
	MessageLogger *Parent;
	LogMessageHandler MessageHandler;
	void *CallbackObject;
	bool enabled;
};

#endif