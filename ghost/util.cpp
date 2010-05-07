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

#include <QFile>
#include <QRegExp>
#include <QtEndian>
#include <QDir>

quint16 Util::extractUInt16(const QByteArray& data, int offset)
{
	return qFromLittleEndian<quint16>((uchar*)data.mid(offset, 2).data());
}

quint32 Util::extractUInt32(const QByteArray& data, int offset)
{
	return qFromLittleEndian<quint32>((uchar*)data.mid(offset, 4).data());
}

QByteArray Util::fromUInt16(const quint16 &value)
{
	uchar dest[2];
	qToLittleEndian<quint16>(value, dest);
	return QByteArray((char*)dest, 2);
}

QByteArray Util::fromUInt32(const quint32 &value)
{
	uchar dest[4];
	qToLittleEndian<quint32>(value, dest);
	return QByteArray((char*)dest, 4);
}

QByteArray Util::reverse(const QByteArray &b)
{
	QByteArray res;
	QByteArray::const_iterator it;
	for (it = b.end() - 1; it == b.begin(); it--)
		res.push_back(*it);

	return res;
}

QString UTIL_QByteArrayToDecString( QByteArray b )
{
	if( b.isEmpty( ) )
		return QString( );

	QString result = QString::number( (unsigned char)b[0] );

	for( QByteArray :: const_iterator i = b.begin( ) + 1; i != b.end( ); i++ )
		result += " " + QString::number( (unsigned char)*i );

	return result;
}

QByteArray UTIL_ExtractCString( QByteArray &b, unsigned int start )
{
	// start searching the byte array at position 'start' for the first null value
	// if found, return the subarray from 'start' to the null value but not including the null value

	QString s(b.mid(start).data());
	return s.toUtf8();
}

unsigned char UTIL_ExtractHex( QByteArray &b, unsigned int start, bool reverse )
{
	// consider the byte array to contain a 2 character ASCII encoded hex value at b[start] and b[start + 1] e.g. "FF"
	// extract it as a single decoded byte

	if (reverse)
		return (unsigned char)(b.mid(start + 1, 1) + b.mid(start, 1)).toUShort(NULL, 16);

	return (unsigned char)b.mid(start, 2).toUShort(NULL, 16);
}

QByteArray UTIL_ExtractNumbers( QString s, unsigned int count )
{
	// consider the QString to contain a QByteArray in dec-text form, e.g. "52 99 128 1"

	QByteArray result;
	unsigned int c;
	QTextStream SS(&s);

	for( unsigned int i = 0; i < count; i++ )
	{
		if( SS.atEnd() )
			break;

		SS >> c;

		// todotodo: if c > 255 handle the error instead of truncating

		result.push_back( (unsigned char)c );
	}

	return result;
}

QByteArray UTIL_FileRead( QString file )
{
	QFile f(file);
	f.open(QFile::ReadOnly);

	if (f.error() != QFile::NoError)
	{
		CONSOLE_Print( "[UTIL] warning - unable to read file [" + file + "]" );
		return QByteArray();
	}

	return f.readAll();
}

bool UTIL_FileWrite( QString file, const QByteArray &data )
{
	QFile f(file);
	f.open(QFile::Truncate | QFile::WriteOnly);

	if (f.error() != QFile::NoError || !f.isWritable())
	{
		CONSOLE_Print( "[UTIL] warning - unable to write file [" + file + "]" );
		return false;
	}

	f.write(data);
	return true;
}

QString UTIL_FileSafeName( QString fileName )
{
	return fileName.replace(QRegExp("\\\\\\/\\:\\*\\?\\<\\>\\|"), "_");
}

QString UTIL_AddPathSeparator( QString path )
{
	if( path.isEmpty( ) )
		return path;

	if( path.right(1) == QDir::separator() )
		return path;
	else
		return path + QDir::separator();
}

QByteArray UTIL_EncodeStatString( QByteArray &data )
{
	unsigned char Mask = 1;
	QByteArray Result;

	for( int i = 0; i < data.size( ); i++ )
	{
		if( ( data[i] % 2 ) == 0 )
			Result.push_back( data[i] + 1 );
		else
		{
			Result.push_back( data[i] );
			Mask |= 1 << ( ( i % 7 ) + 1 );
		}

		if( i % 7 == 6 || i == data.size( ) - 1 )
		{
			Result.insert( Result.size() - 1 - ( i % 7 ), Mask);
			Mask = 1;
		}
	}

	return Result;
}

bool UTIL_IsLanIP( QByteArray ip )
{
	if( ip.size( ) != 4 )
		return false;

	// thanks to LuCasn for this function

	// 127.0.0.1
	if( ip.at(0) == 127 && ip.at(1) == 0 && ip.at(2) == 0 && ip.at(3) == 1 )
		return true;

	// 10.x.x.x
	if( ip.at(0) == 10 )
		return true;

	// 172.16.0.0-172.31.255.255
	if( (unsigned char)ip.at(0) == 172 && ip.at(1) >= 16 && ip.at(1) <= 31 )
		return true;

	// 192.168.x.x
	if( (unsigned char)ip.at(0) == 192 && (unsigned char)ip.at(1) == 168 )
		return true;

	// RFC 3330 and RFC 3927 automatic address range
	if( (unsigned char)ip.at(0) == 169 && (unsigned char)ip.at(1) == 254 )
		return true;

	return false;
}

bool UTIL_IsLocalIP( QByteArray ip, QList<QByteArray> &localIPs )
{
	if( ip.size( ) != 4 )
		return false;

	for( QList<QByteArray> :: const_iterator i = localIPs.begin( ); i != localIPs.end( ); i++ )
	{
		if( (*i).size( ) != 4 )
			continue;

		if( ip[0] == (*i)[0] && ip[1] == (*i)[1] && ip[2] == (*i)[2] && ip[3] == (*i)[3] )
			return true;
	}

	return false;
}

QList<QString> UTIL_Tokenize( QString s, char delim )
{
	QList<QString> Tokens;
	QString Token;

	for( QString :: const_iterator i = s.begin( ); i != s.end( ); i++ )
	{
		if( *i == delim )
		{
			if( Token.isEmpty( ) )
				continue;

			Tokens.push_back( Token );
			Token.clear( );
		}
		else
			Token += *i;
	}

	if( !Token.isEmpty( ) )
		Tokens.push_back( Token );

	return Tokens;
}

quint32 UTIL_Factorial( quint32 x )
{
	quint32 Factorial = 1;

	for( quint32 i = 2; i <= x; i++ )
		Factorial *= i;

	return Factorial;
}

