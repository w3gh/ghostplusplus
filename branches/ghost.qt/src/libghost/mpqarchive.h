#ifndef MPQARCHIVE_H
#define MPQARCHIVE_H

#include <QFile>
class MPQFile;

class MPQArchive : public QFile
{
	Q_OBJECT

public:
	MPQArchive(const QString & filename);
	virtual ~MPQArchive();

	bool open();
	MPQFile* getFile(const QByteArray &filename);

	enum Error
	{
		NO_ERROR,
		FILE_CORRUPT,
		INVALID_HEADER,
		FILE_IS_AVI,
		CANNOT_OPEN_FILE,
		BLOCK_TABLE_CORRUPT,
		NO_FILE_LIST,
	} m_Error;

	class Header
	{
	public:
		bool readFrom(MPQArchive* parent);

		// The ID_MPQ ('MPQ\x1A') signature
		QByteArray m_ID;

		// Size of the archive header
		quint32 m_HeaderSize;

		// Size of MPQ archive
		// This field is deprecated in the Burning Crusade MoPaQ format, and the size of the archive
		// is calculated as the size from the beginning of the archive to the end of the hash table,
		// block table, or extended block table (whichever is largest).
		quint32 m_ArchiveSize;

		// 0 = Original format
		// 1 = Extended format (The Burning Crusade and newer)
		quint16 m_FormatVersion;

		// Power of two exponent specifying the number of 512-byte disk sectors in each logical sector
		// in the archive. The size of each logical sector in the archive is 512 * 2^SectorSizeShift.
		// Bugs in the Storm library dictate that this should always be 3 (4096 byte sectors).
		quint16 m_BlockSize;

		// Offset to the beginning of the hash table, relative to the beginning of the archive.
		quint32 m_HashTablePos;

		// Offset to the beginning of the block table, relative to the beginning of the archive.
		quint32 m_BlockTablePos;

		// Number of entries in the hash table. Must be a power of two, and must be less than 2^16 for
		// the original MoPaQ format, or less than 2^20 for the Burning Crusade format.
		quint32 m_HashTableSize;

		// Number of entries in the block table
		quint32 m_BlockTableSize;
	};

	class Shunt
	{
	public:
		bool readFrom(MPQArchive* parent);

		// The ID_MPQ_SHUNT ('MPQ\x1B') signature
		QByteArray m_ID;

		// Position of the MPQ header, relative to the begin of the shunt
		qint32 m_HeaderPos;
	};

	class FileNode
	{
	public:
		bool readFrom(MPQArchive* parent);

		qint32 m_References;
		QByteArray m_Filename;
		int m_Size;
	};

	class HashEntry
	{
	public:
		bool readFrom(MPQArchive* parent, quint32 &Seed1, quint32& Seed2);

		// The hash of the file path, using method A.
		quint32 m_Name1;

		// The hash of the file path, using method B.
		quint32 m_Name2;

		// The language of the file. This is a Windows LANGID data type, and uses the same values.
		// 0 indicates the default language (American English), or that the file is language-neutral.
		quint16 m_Locale;

		// The platform the file is used for. 0 indicates the default platform.
		// No other values have been observed.
		quint16 m_Platform;

		// If the hash table entry is valid, this is the index into the block table of the file.
		// Otherwise, one of the following two values:
		//  - FFFFFFFFh: Hash table entry is empty, and has always been empty.
		//               Terminates searches for a given file.
		//  - FFFFFFFEh: Hash table entry is empty, but was valid at some point (a deleted file).
		//               Does not terminate searches for a given file.
		quint32 m_BlockIndex;
	};

	class BlockEntry
	{
	public:
		bool readFrom(MPQArchive* parent, quint32 &Seed1, quint32& Seed2);

		bool exists();
		bool valid();
		bool compressed();
		bool encrypted();
		bool fixSeed();
		bool singleUnit();
		bool hasExtra();
		bool usesImplode();

		MPQArchive *m_Parent;
		QByteArray readAll();

		// Offset of the beginning of the block, relative to the beginning of the archive.
		quint32 m_FilePos;

		// Compressed file size
		quint32 m_CSize;

		// Only valid if the block is a file; otherwise meaningless, and should be 0.
		// If the file is compressed, this is the size of the uncompressed file data.
		quint32 m_FSize;

		// Flags for the file. See MPQ_FILE_XXXX constants
		quint32 m_Flags;
	};

	int m_HeaderOffset;			// offset where the header was found
	int m_BlockSize;          // Size of file block
	quint32 getHash(const char *data, int offset = 0x000);
	MPQFile *m_ListFile;
	quint32 m_CryptBuffer[0x500];

private:
	HashEntry *GetHashEntry(const char * file, quint32 locale = 0xFFFFFFFF);

	int m_ShuntPos;             // MPQShunt offset (only valid if a shunt is present)
	int m_HashTablePos;         // Hash table offset (relative to the begin of the file)
	int m_BlockTablePos;        // Block table offset (relative to the begin of the file)
	int m_ExtBlockTablePos;     // Ext. block table offset (relative to the begin of the file)
	int m_MpqSize;              // Size of MPQ archive

	Header		m_Header;        // MPQ header
	HashEntry *m_HashTable;
	BlockEntry *m_BlockTable;
	FileNode *m_FileList;

};

#endif // MPGARCHIVE_H
