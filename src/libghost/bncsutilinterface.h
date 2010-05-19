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

#ifndef BNCSUTIL_INTERFACE_H
#define BNCSUTIL_INTERFACE_H

//
// CBNCSUtilInterface
//
#include "includes.h"

class CBNCSUtilInterface
{
private:
	void *m_NLS;
	QByteArray m_EXEVersion;			// set in HELP_SID_AUTH_CHECK
	QByteArray m_EXEVersionHash;		// set in HELP_SID_AUTH_CHECK
	QString m_EXEInfo;				// set in HELP_SID_AUTH_CHECK
	QByteArray m_KeyInfoROC;			// set in HELP_SID_AUTH_CHECK
	QByteArray m_KeyInfoTFT;			// set in HELP_SID_AUTH_CHECK
	QByteArray m_ClientKey;			// set in HELP_SID_AUTH_ACCOUNTLOGON
	QByteArray m_M1;					// set in HELP_SID_AUTH_ACCOUNTLOGONPROOF
	QByteArray m_PvPGNPasswordHash;	// set in HELP_PvPGNPasswordHash

public:
	CBNCSUtilInterface( QString userName, QString userPassword );
	~CBNCSUtilInterface( );

	QByteArray GetEXEVersion( )								{ return m_EXEVersion; }
	QByteArray GetEXEVersionHash( )							{ return m_EXEVersionHash; }
	QString GetEXEInfo( )									{ return m_EXEInfo; }
	QByteArray GetKeyInfoROC( )								{ return m_KeyInfoROC; }
	QByteArray GetKeyInfoTFT( )								{ return m_KeyInfoTFT; }
	QByteArray GetClientKey( )								{ return m_ClientKey; }
	QByteArray GetM1( )										{ return m_M1; }
	QByteArray GetPvPGNPasswordHash( )						{ return m_PvPGNPasswordHash; }

	void SetEXEVersion( QByteArray &nEXEVersion )			{ m_EXEVersion = nEXEVersion; }
	void SetEXEVersionHash( QByteArray &nEXEVersionHash )	{ m_EXEVersionHash = nEXEVersionHash; }

	void Reset( QString userName, QString userPassword );

	bool HELP_SID_AUTH_CHECK( bool TFT, const QByteArray & war3Path, const QByteArray & keyROC, const QByteArray & keyTFT,
							  const QByteArray & valueStringFormula, const QByteArray & mpqFileName, const QByteArray & clientToken, const QByteArray & serverToken );
	bool HELP_SID_AUTH_ACCOUNTLOGON( );
	bool HELP_SID_AUTH_ACCOUNTLOGONPROOF( const QByteArray & salt, const QByteArray & serverKey );
	bool HELP_PvPGNPasswordHash( const QByteArray & userPassword );

private:
	QByteArray CreateKeyInfo( const QByteArray & key, quint32 clientToken, quint32 serverToken );
};

#endif
