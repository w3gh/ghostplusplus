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

#ifndef UTIL_H
#define UTIL_H

#include <QString>
#include <QByteArray>
#include <QList>
#include <QtEndian>

class Util
{
public:
	static quint16 extractUInt16(const QByteArray& data, int offset = 0);
	static quint32 extractUInt32(const QByteArray& data, int offset = 0);

	static QByteArray fromUInt16(const quint16 &value);
	static QByteArray fromUInt32(const quint32 &value);

	static QByteArray reverse(const QByteArray &b);
};

// byte arrays

QString UTIL_QByteArrayToDecString( QByteArray b );
QByteArray UTIL_ExtractCString( QByteArray &b, unsigned int start );
unsigned char UTIL_ExtractHex( QByteArray &b, unsigned int start, bool reverse );
QByteArray UTIL_ExtractNumbers( QString s, unsigned int count );

// files

QByteArray UTIL_FileRead( QString file );
bool UTIL_FileWrite( QString file, const QByteArray &data );
QString UTIL_FileSafeName( QString fileName );
QString UTIL_AddPathSeparator( QString path );

// stat strings

QByteArray UTIL_EncodeStatString( QByteArray &data );

// other

bool UTIL_IsLanIP( QByteArray ip );
bool UTIL_IsLocalIP( QByteArray ip, QList<QByteArray> &localIPs );
QList<QString> UTIL_Tokenize( QString s, char delim );

// math

quint32 UTIL_Factorial( quint32 x );

#define nCr(n, r) (UTIL_Factorial(n) / UTIL_Factorial((n)-(r)) / UTIL_Factorial(r))
#define nPr(n, r) (UTIL_Factorial(n) / UTIL_Factorial((n)-(r)))

#endif
