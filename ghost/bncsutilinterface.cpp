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
#include "bncsutilinterface.h"

#include <bncsutil/bncsutil.h>
#include <QFile>

//
// CBNCSUtilInterface
//

CBNCSUtilInterface :: CBNCSUtilInterface( QString userName, QString userPassword )
{
	// m_nls = (void *)nls_init( userName.c_str( ), userPassword.c_str( ) );
	m_NLS = new NLS( userName.toStdString(), userPassword.toStdString() );
}

CBNCSUtilInterface :: ~CBNCSUtilInterface( )
{
	// nls_free( (nls_t *)m_nls );
	delete (NLS *)m_NLS;
}

void CBNCSUtilInterface :: Reset( QString userName, QString userPassword )
{
	// nls_free( (nls_t *)m_nls );
	// m_nls = (void *)nls_init( userName.c_str( ), userPassword.c_str( ) );
	delete (NLS *)m_NLS;
	m_NLS = new NLS( userName.toStdString(), userPassword.toStdString() );
}

bool CBNCSUtilInterface :: HELP_SID_AUTH_CHECK( bool TFT, const QByteArray& war3Path, const QByteArray & keyROC,
												const QByteArray & keyTFT, const QByteArray & valueStringFormula, const QByteArray & mpqFileName,
												const QByteArray & clientToken, const QByteArray & serverToken )
{
	// set m_EXEVersion, m_EXEVersionHash, m_EXEInfo, m_InfoROC, m_InfoTFT

	QByteArray FileWar3EXE = war3Path + "war3.exe";
	QByteArray FileStormDLL = war3Path + "Storm.dll";

	if( !QFile::exists( FileStormDLL ) )
		FileStormDLL = war3Path + "storm.dll";

	QByteArray FileGameDLL = war3Path + "game.dll";
	bool ExistsWar3EXE = QFile::exists( FileWar3EXE );
	bool ExistsStormDLL = QFile::exists( FileStormDLL );
	bool ExistsGameDLL = QFile::exists( FileGameDLL );

	if( ExistsWar3EXE && ExistsStormDLL && ExistsGameDLL )
	{
		// todotodo: check getExeInfo return value to ensure 1024 bytes was enough
		char buf[1024];
		quint32 EXEVersion;
		getExeInfo( FileWar3EXE.data( ), (char *)&buf, 1024, (quint32 *)&EXEVersion, BNCSUTIL_PLATFORM_X86 );
		m_EXEInfo = buf;
		m_EXEVersion = Util::fromUInt32( EXEVersion);
		quint32 EXEVersionHash;
		checkRevisionFlat( valueStringFormula.data( ), FileWar3EXE.data( ), FileStormDLL.data( ), FileGameDLL.data( ), extractMPQNumber( mpqFileName.data( ) ), (unsigned long *)&EXEVersionHash );
		m_EXEVersionHash = Util::fromUInt32( EXEVersionHash);
		m_KeyInfoROC = CreateKeyInfo( keyROC, Util::extractUInt32(clientToken), Util::extractUInt32(serverToken) );

		if( TFT )
			m_KeyInfoTFT = CreateKeyInfo( keyTFT, Util::extractUInt32(clientToken), Util::extractUInt32(serverToken) );

		if( m_KeyInfoROC.size( ) == 36 && ( !TFT || m_KeyInfoTFT.size( ) == 36 ) )
			return true;
		else
		{
			if( m_KeyInfoROC.size( ) != 36 )
				CONSOLE_Print( "[BNCSUI] unable to create ROC key info - invalid ROC key" );

			if( TFT && m_KeyInfoTFT.size( ) != 36 )
				CONSOLE_Print( "[BNCSUI] unable to create TFT key info - invalid TFT key" );
		}
	}
	else
	{
		if( !ExistsWar3EXE )
			CONSOLE_Print( "[BNCSUI] unable to open [" + FileWar3EXE + "]" );

		if( !ExistsStormDLL )
			CONSOLE_Print( "[BNCSUI] unable to open [" + FileStormDLL + "]" );

		if( !ExistsGameDLL )
			CONSOLE_Print( "[BNCSUI] unable to open [" + FileGameDLL + "]" );
	}

	return false;
}

bool CBNCSUtilInterface :: HELP_SID_AUTH_ACCOUNTLOGON( )
{
	// set m_ClientKey

	char buf[32];
	// nls_get_A( (nls_t *)m_nls, buf );
	( (NLS *)m_NLS )->getPublicKey( buf );
	m_ClientKey = QByteArray( (char *)buf, 32 );
	return true;
}

bool CBNCSUtilInterface :: HELP_SID_AUTH_ACCOUNTLOGONPROOF( const QByteArray & salt, const QByteArray & serverKey )
{
	// set m_M1

	char buf[20];
	// nls_get_M1( (nls_t *)m_nls, buf, QString( serverKey.begin( ), serverKey.end( ) ).data( ), QString( salt.begin( ), salt.end( ) ).data( ) );
	( (NLS *)m_NLS )->getClientSessionKey( buf, salt.data(), serverKey.data() );
	m_M1 = QByteArray( (char *)buf, 20 );
	return true;
}

bool CBNCSUtilInterface :: HELP_PvPGNPasswordHash( const QByteArray &userPassword )
{
	// set m_PvPGNPasswordHash

	char buf[20];
	hashPassword( userPassword.data( ), buf );
	m_PvPGNPasswordHash = QByteArray( (char *)buf, 20 );
	return true;
}

QByteArray CBNCSUtilInterface :: CreateKeyInfo( const QByteArray & key, quint32 clientToken, quint32 serverToken )
{
	unsigned char Zeros[] = { 0, 0, 0, 0 };
	QByteArray KeyInfo;
	CDKeyDecoder Decoder( key.data(), key.size( ) );

	if( Decoder.isKeyValid( ) )
	{
		KeyInfo.append(Util::fromUInt32( key.size( )));
		KeyInfo.append(Util::fromUInt32( Decoder.getProduct( )));
		KeyInfo.append(Util::fromUInt32( Decoder.getVal1( )));
		KeyInfo.append(QByteArray( (char*)Zeros, 4 ) );
		size_t Length = Decoder.calculateHash( clientToken, serverToken );
		char *buf = new char[Length];
		Length = Decoder.getHash( buf );
		KeyInfo.append(QByteArray( buf, Length ) );
		delete [] buf;
	}

	return KeyInfo;
}
