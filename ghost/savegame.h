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

#ifndef SAVEGAME_H
#define SAVEGAME_H

#include "gameslot.h"

//
// CSaveGame
//

class CSaveGame : public CPacked
{
private:
	QString m_FileName;
	QString m_FileNameNoPath;
	QString m_MapPath;
	QString m_GameName;
	unsigned char m_NumSlots;
	QVector<CGameSlot> m_Slots;
	uint32_t m_RandomSeed;
	QByteArray m_MagicNumber;

public:
	CSaveGame( );
	virtual ~CSaveGame( );

	QString GetFileName( )				{ return m_FileName; }
	QString GetFileNameNoPath( )			{ return m_FileNameNoPath; }
	QString GetMapPath( )				{ return m_MapPath; }
	QString GetGameName( )				{ return m_GameName; }
	unsigned char GetNumSlots( )		{ return m_NumSlots; }
	QVector<CGameSlot> GetSlots( )		{ return m_Slots; }
	uint32_t GetRandomSeed( )			{ return m_RandomSeed; }
	QByteArray GetMagicNumber( )			{ return m_MagicNumber; }

	void SetFileName( QString nFileName )				{ m_FileName = nFileName; }
	void SetFileNameNoPath( QString nFileNameNoPath )	{ m_FileNameNoPath = nFileNameNoPath; }

	void ParseSaveGame( );
};

#endif
