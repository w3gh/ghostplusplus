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

#ifndef PACKED_H
#define PACKED_H

//
// CPacked
//

class CCRC32;

class CPacked
{
public:
	CCRC32 *m_CRC;

protected:
	bool m_Valid;
	QByteArray m_Compressed;
	QByteArray m_Decompressed;
	quint32 m_HeaderSize;
	quint32 m_CompressedSize;
	quint32 m_HeaderVersion;
	quint32 m_DecompressedSize;
	quint32 m_NumBlocks;
	quint32 m_War3Identifier;
	quint32 m_War3Version;
	quint16 m_BuildNumber;
	quint16 m_Flags;
	quint32 m_ReplayLength;

public:
	CPacked( );
	virtual ~CPacked( );

	virtual bool GetValid( )				{ return m_Valid; }
	virtual quint32 GetHeaderSize( )		{ return m_HeaderSize; }
	virtual quint32 GetCompressedSize( )	{ return m_CompressedSize; }
	virtual quint32 GetHeaderVersion( )	{ return m_HeaderVersion; }
	virtual quint32 GetDecompressedSize( )	{ return m_DecompressedSize; }
	virtual quint32 GetNumBlocks( )		{ return m_NumBlocks; }
	virtual quint32 GetWar3Identifier( )	{ return m_War3Identifier; }
	virtual quint32 GetWar3Version( )		{ return m_War3Version; }
	virtual quint16 GetBuildNumber( )		{ return m_BuildNumber; }
	virtual quint16 GetFlags( )			{ return m_Flags; }
	virtual quint32 GetReplayLength( )		{ return m_ReplayLength; }

	virtual void SetWar3Version( quint32 nWar3Version )			{ m_War3Version = nWar3Version; }
	virtual void SetBuildNumber( quint16 nBuildNumber )			{ m_BuildNumber = nBuildNumber; }
	virtual void SetFlags( quint16 nFlags )						{ m_Flags = nFlags; }
	virtual void SetReplayLength( quint32 nReplayLength )			{ m_ReplayLength = nReplayLength; }

	virtual void Load( QString fileName, bool allBlocks );
	virtual bool Save( bool TFT, QString fileName );
	virtual bool Extract( QString inFileName, QString outFileName );
	virtual bool Pack( bool TFT, QString inFileName, QString outFileName );
	virtual void Decompress( bool allBlocks );
	virtual void Compress( bool TFT );
};

#endif
