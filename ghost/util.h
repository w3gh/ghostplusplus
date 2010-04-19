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

// byte arrays

QByteArray UTIL_CreateQByteArray( unsigned char *a, int size );
QByteArray UTIL_CreateQByteArray( unsigned char c );
QByteArray UTIL_CreateQByteArray( uint16_t i, bool reverse );
QByteArray UTIL_CreateQByteArray( uint32_t i, bool reverse );
uint16_t UTIL_QByteArrayToUInt16( QByteArray b, bool reverse, unsigned int start = 0 );
uint32_t UTIL_QByteArrayToUInt32( QByteArray b, bool reverse, unsigned int start = 0 );
QString UTIL_QByteArrayToDecString( QByteArray b );
QString UTIL_QByteArrayToHexString( QByteArray b );
void UTIL_AppendQByteArray( QByteArray &b, QByteArray append );
void UTIL_AppendQByteArrayFast( QByteArray &b, QByteArray &append );
void UTIL_AppendQByteArray( QByteArray &b, unsigned char *a, int size );
void UTIL_AppendQByteArray( QByteArray &b, QString append, bool terminator = true );
void UTIL_AppendQByteArrayFast( QByteArray &b, QString &append, bool terminator = true );
void UTIL_AppendQByteArray( QByteArray &b, uint16_t i, bool reverse );
void UTIL_AppendQByteArray( QByteArray &b, uint32_t i, bool reverse );
QByteArray UTIL_ExtractCString( QByteArray &b, unsigned int start );
unsigned char UTIL_ExtractHex( QByteArray &b, unsigned int start, bool reverse );
QByteArray UTIL_ExtractNumbers( QString s, unsigned int count );
QByteArray UTIL_ExtractHexNumbers( QString s );
QByteArray UTIL_QByteArrayReverse(const QByteArray &b);

// conversions

QString UTIL_ToString( unsigned long i );
QString UTIL_ToString( unsigned short i );
QString UTIL_ToString( unsigned int i );
QString UTIL_ToString( long i );
QString UTIL_ToString( short i );
QString UTIL_ToString( int i );
QString UTIL_ToString( float f, int digits );
QString UTIL_ToString( double d, int digits );
QString UTIL_ToHexString( uint32_t i );
uint16_t UTIL_ToUInt16( QString &s );
uint32_t UTIL_ToUInt32( QString &s );
int16_t UTIL_ToInt16( QString &s );
int32_t UTIL_ToInt32( QString &s );
double UTIL_ToDouble( QString &s );
QString UTIL_MSToString( uint32_t ms );

// files

bool UTIL_FileExists( QString file );
QByteArray UTIL_FileRead( QString file, uint32_t start, uint32_t length );
QByteArray UTIL_FileRead( QString file );
bool UTIL_FileWrite( QString file, unsigned char *data, uint32_t length );
QString UTIL_FileSafeName( QString fileName );
QString UTIL_AddPathSeperator( QString path );

// stat strings

QByteArray UTIL_EncodeStatString( QByteArray &data );
QByteArray UTIL_DecodeStatString( QByteArray &data );

// other

bool UTIL_IsLanIP( QByteArray ip );
bool UTIL_IsLocalIP( QByteArray ip, vector<QByteArray> &localIPs );
void UTIL_Replace( QString &Text, QString Key, QString Value );
vector<QString> UTIL_Tokenize( QString s, char delim );

// math

uint32_t UTIL_Factorial( uint32_t x );

#define nCr(n, r) (UTIL_Factorial(n) / UTIL_Factorial((n)-(r)) / UTIL_Factorial(r))
#define nPr(n, r) (UTIL_Factorial(n) / UTIL_Factorial((n)-(r)))

#endif
