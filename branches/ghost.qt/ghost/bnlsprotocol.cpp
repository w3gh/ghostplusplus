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
		unsigned char Usage = data[3];
		uint32_t Cookie = UTIL_QByteArrayToUInt32( data, false, 4 );
		unsigned char Result = data[8];
		uint16_t Length = UTIL_QByteArrayToUInt16( data, false, 10 );

		if( Result == 0x00 )
			return QByteArray( data.begin( ) + 11, data.end( ) );
		else
			CONSOLE_Print( "[BNLSPROTO] received error code " + UTIL_ToString( data[8] ) );
	}

	return QByteArray( );
}

////////////////////
// SEND FUNCTIONS //
////////////////////

QByteArray CBNLSProtocol :: SEND_BNLS_NULL( )
{
	QByteArray packet;
	packet.push_back( 0 );							// packet length will be assigned later
	packet.push_back( 0 );							// packet length will be assigned later
	packet.push_back( BNLS_NULL );					// BNLS_NULL
	AssignLength( packet );
	return packet;
}

QByteArray CBNLSProtocol :: SEND_BNLS_WARDEN_SEED( uint32_t cookie, uint32_t seed )
{
	unsigned char Client[] = {  80,  88,  51,  87 };	// "W3XP"

	QByteArray packet;
	packet.push_back( 0 );								// packet length will be assigned later
	packet.push_back( 0 );								// packet length will be assigned later
	packet.push_back( BNLS_WARDEN );					// BNLS_WARDEN
	packet.push_back( 0 );								// BNLS_WARDEN_SEED
	UTIL_AppendQByteArray( packet, cookie, false );		// cookie
	UTIL_AppendQByteArray( packet, Client, 4 );			// Client
	UTIL_AppendQByteArray( packet, (uint16_t)4, false );	// length of seed
	UTIL_AppendQByteArray( packet, seed, false );		// seed
	packet.push_back( 0 );								// username is blank
	UTIL_AppendQByteArray( packet, (uint16_t)0, false );	// password length
														// password
	AssignLength( packet );
	return packet;
}

QByteArray CBNLSProtocol :: SEND_BNLS_WARDEN_RAW( uint32_t cookie, QByteArray raw )
{
	QByteArray packet;
	packet.push_back( 0 );											// packet length will be assigned later
	packet.push_back( 0 );											// packet length will be assigned later
	packet.push_back( BNLS_WARDEN );								// BNLS_WARDEN
	packet.push_back( 1 );											// BNLS_WARDEN_RAW
	UTIL_AppendQByteArray( packet, cookie, false );					// cookie
	UTIL_AppendQByteArray( packet, (uint16_t)raw.size( ), false );	// raw length
	UTIL_AppendQByteArray( packet, raw );							// raw
	AssignLength( packet );
	return packet;
}

QByteArray CBNLSProtocol :: SEND_BNLS_WARDEN_RUNMODULE( uint32_t cookie )
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
		LengthBytes = UTIL_CreateQByteArray( (uint16_t)content.size( ), false );
		content[0] = LengthBytes[0];
		content[1] = LengthBytes[1];
		return true;
	}

	return false;
}

bool CBNLSProtocol :: ValidateLength( QByteArray &content )
{
	// verify that bytes 1 and 2 (indices 0 and 1) of the content array describe the length

	uint16_t Length;
	QByteArray LengthBytes;

	if( content.size( ) >= 2 && content.size( ) <= 65535 )
	{
		LengthBytes.push_back( content[0] );
		LengthBytes.push_back( content[1] );
		Length = UTIL_QByteArrayToUInt16( LengthBytes, false );

		if( Length == content.size( ) )
			return true;
	}

	return false;
}
