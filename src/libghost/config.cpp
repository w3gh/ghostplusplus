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
#include "config.h"

#include <stdlib.h>

//
// CConfig
//

CConfig :: CConfig( )
{

}

CConfig :: ~CConfig( )
{

}

#include <QFile>
#include <QStringList>
void CConfig :: Read( const QString &file )
{
	QFile f(file);

	if( !f.open(QFile::ReadOnly) )
		CONSOLE_Print( "[CONFIG] warning - unable to read file [" + file + "]" );
	else
	{
		CONSOLE_Print( "[CONFIG] loading file [" + file + "]" );
		QString Line;

		while( !f.atEnd() )
		{
			Line = f.readLine();

			// ignore blank lines and comments

			if( Line.isEmpty( ) || Line[0] == '#' )
				continue;

			// remove newlines and partial newlines to help fix issues with Windows formatted config files on Linux systems

			Line.replace('\r', "").replace('\n', "");

			QStringList parts = Line.split('=');

			if (parts.size() == 2)
				m_CFG[parts.at(0).trimmed()] = parts.at(1).trimmed();
		}

		f.close( );
	}
}

bool CConfig :: Exists( const QString &key ) const
{
	return m_CFG.contains(key);
}

int CConfig :: GetInt( const QString &key, int x ) const
{
	if( !m_CFG.contains( key ) )
		return x;
	else
		return m_CFG[key].toInt();
}

QString CConfig :: GetString( const QString &key, const QString &x ) const
{
	return m_CFG.value(key, x);
}

void CConfig :: Set( const QString &key, const QString &x )
{
	m_CFG[key] = x;
}
