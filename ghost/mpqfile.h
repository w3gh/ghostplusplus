#ifndef MPQFILE_H
#define MPQFILE_H

#include "mpqarchive.h"

class MPQFile
{
public:
	MPQFile(MPQArchive* archive, const QString &filename, MPQArchive::BlockEntry *block, MPQArchive::HashEntry *hash);

	enum Error
	{
		NO_ERROR,
		POS_TABLE_CORRUPT,
		SEED_NOT_FOUND,
		DECRYPTION_FAILED,
		FILE_CORRUPTED,
	} m_Error;

	bool read();
	bool readBlock(quint32 from, quint32 to, quint32 seed);
	void decryptBlock(quint32 *data, quint32 length, quint32 seed);

	MPQArchive* m_Parent;
	MPQArchive::BlockEntry *m_BlockEntry;
	MPQArchive::HashEntry *m_HashEntry;

	quint32 m_Seed1;
	quint32 m_PositionFromBegin;
	quint32 m_PositionFromHeader;
	quint32 m_Size;

	int m_BlockPosTableSize;
	qint32 *m_BlockPosTable;

	int m_Blocks;

	QByteArray m_Content;

	QString m_Name;

/*    DWORD        * pdwBlockPos;         // Position of each file block (only for compressed files)
	DWORD          nBlocks;             // Number of blocks in the file (incl. the last incomplete one)
	BOOL           bBlockPosLoaded;     // TRUE if block positions loaded
	BYTE         * pbFileBuffer;        // Decompressed file (for single unit files, size is the uncompressed file size)

	TMPQCRC32    * pCrc32;              // Pointer to CRC32 (NULL if none)
	TMPQFileTime * pFileTime;           // Pointer to file's FILETIME (NULL if none)
	TMPQMD5      * pMd5;                // Pointer to file's MD5 (NULL if none)

	DWORD          dwHashIndex;         // Index to Hash table
	DWORD          dwBlockIndex;        // Index to Block table
	char           szFileName[1];       // File name (variable length)	*/
};

#endif // MPQFILE_H
