/*
 *      rcon.h
 *
 *      Copyright 2008-2009 Lucas Romero
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */
#ifndef RCON_H
#define RCON_H

#include "ghost.h"
#include "socket.h"
#include "util.h"
#ifdef WIN32
 #include <winsock.h>
#else
 #include <netinet/in.h>
#endif

using namespace std;
class CUDPServer;
class CConfig;
class CGHost;

class CRemoteConsoleClient
{
	public:
		CRemoteConsoleClient( CUDPServer *socket, sockaddr_in endPoint, int banTime, bool authed );

		virtual void Ping( )							{ Send( "PING" ); m_LastPinged = GetTime( ); }
		virtual void Pong( )							{ m_LastReceived = GetTime( ); /*m_PingSent = false;*/ }

		virtual void Ban( int seconds )					{ m_BannedUntil = GetTime( ) + seconds; }
		virtual bool IsBanned( )						{ return GetTime() < m_BannedUntil; }
		virtual void UnBan( )							{ m_BannedUntil = 0; }
		
		virtual bool IsAuthed( )						{ return m_Authed; }
		virtual void SetAuthed( bool authed )			{ m_Authed = authed; }
		
		virtual time_t GetLastPinged( )					{ return m_LastPinged; }
		virtual time_t GetLastReceived( )				{ return m_LastReceived; }
		
		virtual const sockaddr_in* GetEndPoint( )		{ return &m_EndPoint; }
		virtual string GetIPString( )					{ return string( inet_ntoa( m_EndPoint.sin_addr ) ); }
		virtual void Send( string message )				{ m_Socket->SendTo( m_EndPoint, message ); }
	private:
		CUDPServer *m_Socket;
		sockaddr_in m_EndPoint;
		time_t m_LastReceived;
		time_t m_LastPinged;
		time_t m_BannedUntil;
		bool m_Authed;
};

class CRemoteConsole
{
	public:
		CRemoteConsole( CGHost *ghost, CConfig *cfg );
		virtual ~CRemoteConsole( );
		virtual unsigned int SetFD( void *fd, void *send_fd, int *nfds );
		virtual bool Update( void *fd );
		virtual void Send( string message );																	// output a message to all authed clients
	private:
		CGHost *m_GHost;																						// our parent
		in_addr localhost; 
		CUDPServer *m_Socket;																					// the socket we use to communicate
		string m_Password;																						// the password users need to authenticate themselves with
		bool m_AnonymousBroadcast;																				// should everyone requesting a LAN broadcast packet be granted their wish?
		bool m_AnonymousAdminGame;																				// should the admin game also be broadcasted to anonymous requesters?
		int m_KeepAliveTime;																					// time that has to pass before we start asking the client if he is still there
		int m_KeepAliveInterval;																				// time to wait between ping packets when asking if client is still there
		int m_Timeout;																							// time in which the client has to repond to our ping(s)
		vector<CRemoteConsoleClient *> m_KnownClients;															// all our console clients (includes authed but banned clients)
		void Send( string message, CRemoteConsoleClient *client );												// output a message only to a specific client
		static bool AreEqual( const sockaddr_in *a, const sockaddr_in *b, bool checkports );					// checks if two ip endpoints are the same
		static void SplitLine( const string input, string &command, string &payload, string &target );			// splits a line received by the rcon socket up into pieces
		virtual void ProcessInput( string cmd, string payload, string target, CRemoteConsoleClient *sender );	// does the actual action based on rcon input
};

#endif