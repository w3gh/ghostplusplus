/*

   Copyright 2010 Trevor Hogan

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

	   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

#include "ghost.h"
#include "util.h"
#include "gpsprotocol.h"

//
// CGPSProtocol
//

CGPSProtocol :: CGPSProtocol( )
{

}

CGPSProtocol :: ~CGPSProtocol( )
{

}

///////////////////////
// RECEIVE FUNCTIONS //
///////////////////////

////////////////////
// SEND FUNCTIONS //
////////////////////

QByteArray CGPSProtocol :: SEND_GPSC_INIT( quint32 version ) const
{
	QByteArray packet;
	packet.push_back( GPS_HEADER_CONSTANT );
	packet.push_back( GPS_INIT );
	packet.push_back( (char)0 );
	packet.push_back( (char)0 );
	packet.append(Util::fromUInt32(version));
	AssignLength( packet );
	return packet;
}

QByteArray CGPSProtocol :: SEND_GPSC_RECONNECT( unsigned char PID, quint32 reconnectKey, quint32 lastPacket ) const
{
	QByteArray packet;
	packet.push_back( GPS_HEADER_CONSTANT );
	packet.push_back( GPS_RECONNECT );
	packet.push_back( (char)0 );
	packet.push_back( (char)0 );
	packet.push_back( PID );
	packet.append(Util::fromUInt32(reconnectKey));
	packet.append(Util::fromUInt32(lastPacket));
	AssignLength( packet );
	return packet;
}

QByteArray CGPSProtocol :: SEND_GPSC_ACK( quint32 lastPacket ) const
{
	QByteArray packet;
	packet.push_back( GPS_HEADER_CONSTANT );
	packet.push_back( GPS_ACK );
	packet.push_back( (char)0 );
	packet.push_back( (char)0 );
	packet.append(Util::fromUInt32(lastPacket));
	AssignLength( packet );
	return packet;
}

QByteArray CGPSProtocol :: SEND_GPSS_INIT( quint16 reconnectPort, unsigned char PID, quint32 reconnectKey, unsigned char numEmptyActions ) const
{
	QByteArray packet;
	packet.push_back( GPS_HEADER_CONSTANT );
	packet.push_back( GPS_INIT );
	packet.push_back( (char)0 );
	packet.push_back( (char)0 );
	packet.append(Util::fromUInt16(reconnectPort));
	packet.push_back( PID );
	packet.append(Util::fromUInt32(reconnectKey));
	packet.push_back( numEmptyActions );
	AssignLength( packet );
	return packet;
}

QByteArray CGPSProtocol :: SEND_GPSS_RECONNECT( quint32 lastPacket ) const
{
	QByteArray packet;
	packet.push_back( GPS_HEADER_CONSTANT );
	packet.push_back( GPS_RECONNECT );
	packet.push_back( (char)0 );
	packet.push_back( (char)0 );
	packet.append(Util::fromUInt32(lastPacket));
	AssignLength( packet );
	return packet;
}

QByteArray CGPSProtocol :: SEND_GPSS_ACK( quint32 lastPacket ) const
{
	QByteArray packet;
	packet.push_back( GPS_HEADER_CONSTANT );
	packet.push_back( GPS_ACK );
	packet.push_back( (char)0 );
	packet.push_back( (char)0 );
	packet.append(Util::fromUInt32(lastPacket));
	AssignLength( packet );
	return packet;
}

QByteArray CGPSProtocol :: SEND_GPSS_REJECT( quint32 reason ) const
{
	QByteArray packet;
	packet.push_back( GPS_HEADER_CONSTANT );
	packet.push_back( GPS_REJECT );
	packet.push_back( (char)0 );
	packet.push_back( (char)0 );
	packet.append(Util::fromUInt32(reason));
	AssignLength( packet );
	return packet;
}

/////////////////////
// OTHER FUNCTIONS //
/////////////////////

bool CGPSProtocol :: AssignLength( QByteArray &content ) const
{
	// insert the actual length of the content array into bytes 3 and 4 (indices 2 and 3)

	QByteArray LengthBytes;

	if( content.size( ) >= 4 && content.size( ) <= 65535 )
	{
		LengthBytes = Util::fromUInt16(content.size( ));
		content[2] = LengthBytes[0];
		content[3] = LengthBytes[1];
		return true;
	}

	return false;
}

bool CGPSProtocol :: ValidateLength( QByteArray &content ) const
{
	// verify that bytes 3 and 4 (indices 2 and 3) of the content array describe the length

	quint16 Length;
	QByteArray LengthBytes;

	if( content.size( ) >= 4 && content.size( ) <= 65535 )
	{
		LengthBytes.push_back( content[2] );
		LengthBytes.push_back( content[3] );
		Length = LengthBytes.toUShort();

		if( Length == content.size( ) )
			return true;
	}

	return false;
}
