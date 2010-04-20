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

QByteArray UTIL_CreateBYTEARRAY( unsigned char c )
{
	QByteArray result;
	result.push_back( c );
	return result;
}

QByteArray UTIL_CreateBYTEARRAY( quint16 i, bool reverse )
{
	QByteArray result;

	if (!reverse)
		result.push_back( (unsigned char)i );

	result.push_back( (unsigned char)( i >> 8 ) );

	if (reverse)
		result.push_back( (unsigned char)i );

	return result;
}

QByteArray UTIL_CreateBYTEARRAY( quint32 i, bool reverse )
{
	QByteArray result;

	if( reverse )
	{
		result.push_back( (unsigned char)( i >> 24 ) );
		result.push_back( (unsigned char)( i >> 16 ) );
		result.push_back( (unsigned char)( i >> 8 ) );
		result.push_back( (unsigned char)i );
	}

	else
	{
		result.push_back( (unsigned char)i );
		result.push_back( (unsigned char)( i >> 8 ) );
		result.push_back( (unsigned char)( i >> 16 ) );
		result.push_back( (unsigned char)( i >> 24 ) );
	}

	return result;
}

quint16 UTIL_QByteArrayToUInt16( QByteArray b, bool reverse, unsigned int start )
{
	if( (unsigned int)b.size( ) < start + 2 )
		return 0;

	QByteArray temp = b.mid(start, 2);

	return (quint16)( reverse ? (temp[0] << 8 | temp[1]) : (temp[1] << 8 | temp[0]) );
}

quint32 UTIL_QByteArrayToUInt32( QByteArray b, bool reverse, unsigned int start )
{
	if( (unsigned int)b.size( ) < start + 4 )
		return 0;

	QByteArray temp = b.mid(start, 4);

	if (reverse)
		return (quint32)( temp[0] << 24 | temp[1] << 16 | temp[2] << 8 | temp[3] );

	return (quint32)( temp[3] << 24 | temp[2] << 16 | temp[1] << 8 | temp[0] );
}

QString UTIL_QByteArrayToDecString( QByteArray b )
{
	if( b.isEmpty( ) )
		return QString( );

	QString result = QString::number( b[0] );

	for( QByteArray :: iterator i = b.begin( ) + 1; i != b.end( ); i++ )
		result += " " + QString::number( *i );

	return result;
}

QString UTIL_QByteArrayToHexString( QByteArray b )
{
	return QString::fromAscii(b.toHex());
}

void UTIL_AppendBYTEARRAY( QByteArray &b, const QByteArray &append )
{
	b.append(append);
}

void UTIL_AppendBYTEARRAYFast( QByteArray &b, QByteArray &append )
{
	b.append(append);
}

void UTIL_AppendBYTEARRAY( QByteArray &b, unsigned char *a, int size )
{
	b.append((char*)a, size);
}

void UTIL_AppendBYTEARRAY( QByteArray &b, QString append, bool terminator )
{
	b.append(append);

	if( terminator )
		b.push_back( (char)0 );
}

void UTIL_AppendBYTEARRAYFast( QByteArray &b, QString &append, bool terminator )
{
	b.append(append);

	if( terminator )
		b.push_back( (char)0 );
}

void UTIL_AppendBYTEARRAY( QByteArray &b, quint16 i, bool reverse )
{
	b.append( UTIL_CreateBYTEARRAY( i, reverse ) );
}

void UTIL_AppendBYTEARRAY( QByteArray &b, quint32 i, bool reverse )
{
	b.append( UTIL_CreateBYTEARRAY( i, reverse ) );
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

QByteArray UTIL_ExtractHexNumbers( QString s )
{
	// consider the QString to contain a QByteArray in hex-text form, e.g. "4e 17 b7 e6"

	QByteArray result;
	unsigned int c;
	QTextStream SS(&s);


	while( !SS.atEnd() )
	{
		SS >> hex >> c;

		// todotodo: if c > 255 handle the error instead of truncating

		result.push_back( (unsigned char)c );
	}

	return result;
}

QString UTIL_ToString( unsigned long i )
{
	return QString::number(i);
}

QString UTIL_ToString( unsigned short i )
{
	return QString::number(i);
}

QString UTIL_ToString( unsigned int i )
{
	return QString::number(i);
}

QString UTIL_ToString( long i )
{
	return QString::number(i);
}

QString UTIL_ToString( short i )
{
	return QString::number(i);
}

QString UTIL_ToString( int i )
{
	return QString::number(i);
}

QString UTIL_ToString( float f, int digits )
{
	return QString::number(f, 'g', digits);
}

QString UTIL_ToString( double d, int digits )
{
	return QString::number(d, 'g', digits);
}

QString UTIL_ToHexString( quint32 i )
{
	return QString::number(i);
}

// todotodo: these UTIL_ToXXX functions don't fail gracefully, they just return garbage (in the uint case usually just -1 casted to an unsigned type it looks like)

quint16 UTIL_ToUInt16( QString &s )
{
	return s.toUShort();
}

quint32 UTIL_ToUInt32( QString &s )
{
	return s.toUInt();
}

int16_t UTIL_ToInt16( QString &s )
{
	return s.toShort();
}

int32_t UTIL_ToInt32( QString &s )
{
	return s.toInt();
}

double UTIL_ToDouble( QString &s )
{
	return s.toDouble();
}

QString UTIL_MSToString( quint32 ms )
{
	QString MinString = UTIL_ToString( ( ms / 1000 ) / 60 );
	QString SecString = UTIL_ToString( ( ms / 1000 ) % 60 );

	if( MinString.size( ) == 1 )
		MinString.insert( 0, "0" );

	if( SecString.size( ) == 1 )
		SecString.insert( 0, "0" );

	return MinString + "m" + SecString + "s";
}

bool UTIL_FileExists( QString file )
{
	return QFile::exists(file);
}

QByteArray UTIL_FileRead( QString file, quint32 start, quint32 length )
{
	QFile f(file);
	f.open(QFile::ReadOnly);

	if (f.error() != QFile::NoError)
	{
		CONSOLE_Print( "[UTIL] warning - unable to read file part [" + file + "]" );
		return QByteArray();
	}

	f.seek(start);
	return f.read(length);
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

QString UTIL_AddPathSeperator( QString path )
{
	if( path.isEmpty( ) )
		return path;

#ifdef WIN32
	QString Seperator = "\\";
#else
	QString Seperator = "/";
#endif

	if( path.right(1) == Seperator )
		return path;
	else
		return path + Seperator;
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

QByteArray UTIL_QByteArrayReverse(const QByteArray &b)
{
	QByteArray res;
	QByteArray::const_iterator it;
	for (it = b.end(); it != b.begin(); it--)
		res.push_back(*it);

	return res;
}

QByteArray UTIL_DecodeStatString( QByteArray &data )
{
	unsigned char Mask;
	QByteArray Result;

	for( int i = 0; i < data.size( ); i++ )
	{
		if( ( i % 8 ) == 0 )
			Mask = data[i];
		else
		{
			if( ( Mask & ( 1 << ( i % 8 ) ) ) == 0 )
				Result.push_back( data[i] - 1 );
			else
				Result.push_back( data[i] );
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

bool UTIL_IsLocalIP( QByteArray ip, QVector<QByteArray> &localIPs )
{
	if( ip.size( ) != 4 )
		return false;

	for( QVector<QByteArray> :: iterator i = localIPs.begin( ); i != localIPs.end( ); i++ )
	{
		if( (*i).size( ) != 4 )
			continue;

		if( ip[0] == (*i)[0] && ip[1] == (*i)[1] && ip[2] == (*i)[2] && ip[3] == (*i)[3] )
			return true;
	}

	return false;
}

void UTIL_Replace( QString &Text, QString Key, QString Value )
{
	Text.replace(Key, Value);
}

QVector<QString> UTIL_Tokenize( QString s, char delim )
{
	QVector<QString> Tokens;
	QString Token;

	for( QString :: iterator i = s.begin( ); i != s.end( ); i++ )
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
