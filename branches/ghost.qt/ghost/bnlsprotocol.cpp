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

#include "ghost.h"
#include "util.h"
#include "bnlsprotocol.h"

CBNLSProtocol :: CBNLSProtocol( )
{

}

CBNLSProtocol :: ~CBNLSProtocol( )
{

}

///////////////////////
// RECEIVE FUNCTIONS //
///////////////////////

QByteArray CBNLSProtocol :: RECEIVE_BNLS_WARDEN( QByteArray data )
{
	// 2 bytes					-> Length
	// 1 byte					-> ID
	// (BYTE)					-> Usage
	// (DWORD)					-> Cookie
	// (BYTE)					-> Result
	// (WORD)					-> Length of data
	// (VOID)					-> Data

	if( ValidateLength( data ) && data.size( ) >= 11 )
	{
		//unsigned char Usage = data[3];
		//quint32 Cookie = UTIL_QByteArrayToUInt32( data, false, 4 );
		unsigned char Result = data[8];
		//quint16 Length = UTIL_QByteArrayToUInt16( data, false, 10 );

		if( Result == 0x00 )
			return data.mid(11);
		else
			CONSOLE_Print( "[BNLSPROTO] received error code " + QString::number( data[8] ) );
	}

	return QByteArray( );
}

////////////////////
// SEND FUNCTIONS //
////////////////////

QByteArray CBNLSProtocol :: SEND_BNLS_NULL( )
{
	QByteArray packet;
	packet.push_back( (char)0 );							// packet length will be assigned later
	packet.push_back( (char)0 );							// packet length will be assigned later
	packet.push_back( BNLS_NULL );					// BNLS_NULL
	AssignLength( packet );
	return packet;
}

QByteArray CBNLSProtocol :: SEND_BNLS_WARDEN_SEED( quint32 cookie, quint32 seed )
{
	char Client[] = {  80,  88,  51,  87 };	// "W3XP"

	QByteArray packet;
	packet.push_back( (char)0 );								// packet length will be assigned later
	packet.push_back( (char)0 );								// packet length will be assigned later
	packet.push_back( BNLS_WARDEN );					// BNLS_WARDEN
	packet.push_back( (char)0 );								// BNLS_WARDEN_SEED
	packet.append(Util::fromUInt32(cookie));		// cookie
	packet.append(QByteArray(Client, 4));			// Client
	packet.append(Util::fromUInt16(4));	// length of seed
	packet.append(Util::fromUInt32(seed));		// seed
	packet.push_back( (char)0 );								// username is blank
	packet.append(Util::fromUInt16(0));	// password length
														// password
	AssignLength( packet );
	return packet;
}

QByteArray CBNLSProtocol :: SEND_BNLS_WARDEN_RAW( quint32 cookie, QByteArray raw )
{
	QByteArray packet;
	packet.push_back( (char)0 );											// packet length will be assigned later
	packet.push_back( (char)0 );											// packet length will be assigned later
	packet.push_back( BNLS_WARDEN );								// BNLS_WARDEN
	packet.push_back( 1 );											// BNLS_WARDEN_RAW
	packet.append(Util::fromUInt32(cookie));					// cookie
	packet.append(Util::fromUInt16(raw.size( )));	// raw length
	packet.append(raw);							// raw
	AssignLength( packet );
	return packet;
}

QByteArray CBNLSProtocol :: SEND_BNLS_WARDEN_RUNMODULE( quint32 /*cookie*/ )
{
	return QByteArray( );
}

/////////////////////
// OTHER FUNCTIONS //
/////////////////////

bool CBNLSProtocol :: AssignLength( QByteArray &content )
{
	// insert the actual length of the content array into bytes 1 and 2 (indices 0 and 1)

	QByteArray LengthBytes;

	if( content.size( ) >= 2 && content.size( ) <= 65535 )
	{
		LengthBytes = Util::fromUInt16(content.size( ));
		content[0] = LengthBytes[0];
		content[1] = LengthBytes[1];
		return true;
	}

	return false;
}

bool CBNLSProtocol :: ValidateLength( QByteArray &content )
{
	// verify that bytes 1 and 2 (indices 0 and 1) of the content array describe the length

	quint16 Length;
	QByteArray LengthBytes;

	if( content.size( ) >= 2 && content.size( ) <= 65535 )
	{
		LengthBytes.push_back( content[0] );
		LengthBytes.push_back( content[1] );
		Length = Util::extractUInt16( LengthBytes );

		if( Length == content.size( ) )
			return true;
	}

	return false;
}
