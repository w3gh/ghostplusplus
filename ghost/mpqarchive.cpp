// rewritten using StormLib:
/*****************************************************************************/
/* SFileOpenArchive.cpp                       Copyright Ladislav Zezula 1999 */
/*                                                                           */
/* Author : Ladislav Zezula                                                  */
/* E-mail : ladik@zezula.net                                                 */
/* WWW    : www.zezula.net                                                   */
/*---------------------------------------------------------------------------*/
/*                       Archive functions of Storm.dll                      */
/*---------------------------------------------------------------------------*/
/*   Date    Ver   Who  Comment                                              */
/* --------  ----  ---  -------                                              */
/* xx.xx.xx  1.00  Lad  The first version of SFileOpenArchive.cpp            */
/* 19.11.03  1.01  Dan  Big endian handling                                  */
/*****************************************************************************/

#include <QtEndian>
#include "mpqarchive.h"
#include "mpqfile.h"
#include "includes.h"

QByteArray m_ID;


bool MPQArchive::Header::readFrom(MPQArchive *parent)
{
	char buffer[32];
	if (parent->read(buffer, 32)  != 32)
		return false;
DEBUG_Print(QByteArray(buffer, 32).toHex());
	//<- 4b -> <- 4b -> <- 4b -> <- 4b -> <- 4b -> <- 4b -> <- 4b -> <- 4b ->
	//4d50511a 20000000 98a46c00 00000f00 c8256c00 c8656c00 00040000 ed030000

	m_ID = QByteArray(buffer, 4);
	m_HeaderSize	 = qFromLittleEndian<quint32>((unsigned char*)buffer + 4);
	m_ArchiveSize	 = qFromLittleEndian<quint32>((unsigned char*)buffer + 8);
	m_FormatVersion	 = qFromLittleEndian<quint16>((unsigned char*)buffer + 12);
	m_BlockSize		 = qFromLittleEndian<quint16>((unsigned char*)buffer + 14);
	m_HashTablePos	 = qFromLittleEndian<quint32>((unsigned char*)buffer + 16);
	m_BlockTablePos	 = qFromLittleEndian<quint32>((unsigned char*)buffer + 20);
	m_HashTableSize	 = qFromLittleEndian<quint32>((unsigned char*)buffer + 24);
	m_BlockTableSize = qFromLittleEndian<quint32>((unsigned char*)buffer + 28);

	return true;
}

bool MPQArchive::Shunt::readFrom(MPQArchive *parent)
{
	char buffer[12];
	if (parent->read(buffer, 12)  != 12)
		return false;

	m_ID = QByteArray(buffer, 4);
	m_HeaderPos = qFromLittleEndian<qint32>((unsigned char*)buffer + 8);

	return true;
}

bool MPQArchive::HashEntry::readFrom(MPQArchive *parent, quint32 &Seed1, quint32 &Seed2)
{
	char buffer[16];
	if (parent->read(buffer, 16)  != 16)
		return false;

	unsigned char buffer_out[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0 };

	for (int i = 0; i < 4; i++)
	{
		quint32 p = qFromLittleEndian<quint32>((unsigned char*)buffer + i * 4);
		Seed2 += parent->m_CryptBuffer[0x400 + (Seed1 & 0xFF)];
		quint32 ch = p ^ (Seed1 + Seed2);

		Seed1  = ((~Seed1 << 0x15) + 0x11111111) | (Seed1 >> 0x0B);
		Seed2  = ch + Seed2 + (Seed2 << 5) + 3;
		qToLittleEndian<quint32>(ch, buffer_out + i * 4);
	}

	m_Name1		 = qFromLittleEndian<quint32>(buffer_out +  0);
	m_Name2		 = qFromLittleEndian<quint32>(buffer_out +  4);
	m_Locale	 = qFromLittleEndian<quint16>(buffer_out +  8);
	m_Platform	 = qFromLittleEndian<quint16>(buffer_out + 10);
	m_BlockIndex = qFromLittleEndian<quint32>(buffer_out + 12);

	return true;
}

bool  MPQArchive::BlockEntry::readFrom(MPQArchive *parent, quint32 &Seed1, quint32 &Seed2)
{
	m_Parent = parent;
	char buffer[16];
	if (parent->read(buffer, 16)  != 16)
		return false;

	unsigned char buffer_out[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0 };

	for (int i = 0; i < 4; i++)
	{
		quint32 p = qFromLittleEndian<quint32>((unsigned char*)buffer + i * 4);
		Seed2 += parent->m_CryptBuffer[0x400 + (Seed1 & 0xFF)];
		quint32 ch = p ^ (Seed1 + Seed2);

		Seed1  = ((~Seed1 << 0x15) + 0x11111111) | (Seed1 >> 0x0B);
		Seed2  = ch + Seed2 + (Seed2 << 5) + 3;
		qToLittleEndian<quint32>(ch, buffer_out + i * 4);
	}

	m_FilePos	= qFromLittleEndian<quint32>(buffer_out +  0);
	m_CSize		= qFromLittleEndian<quint32>(buffer_out +  4);
	m_FSize		= qFromLittleEndian<quint32>(buffer_out +  8);
	m_Flags		= qFromLittleEndian<quint32>(buffer_out + 12);
	return true;
}

bool MPQArchive::BlockEntry::encrypted()
{
	return (m_Flags & 0x00010000);
}

bool MPQArchive::BlockEntry::fixSeed()
{
	return (m_Flags & 0x00020000);
}

bool MPQArchive::BlockEntry::compressed()
{
	return (m_Flags & 0x0000FF00);
}

bool MPQArchive::BlockEntry::exists()
{
	return (m_Flags & 0x80000000);
}

bool MPQArchive::BlockEntry::singleUnit()
{
	return (m_Flags & 0x01000000);
}

bool MPQArchive::BlockEntry::hasExtra()
{
	return (m_Flags & 0x04000000);
}

bool MPQArchive::BlockEntry::usesImplode()
{
	return (m_Flags & 0x00000100);
}

bool MPQArchive::BlockEntry::valid()
{
	return !(m_Flags & ~( 0x00000100 | 0x00000200 | 0x00010000 | 0x00020000 | 0x01000000 | 0x02000000 | 0x04000000 | 0x80000000 ));
}

QByteArray MPQArchive::BlockEntry::readAll()
{
	//m_Parent->seek(m_FilePos + m_Parent->m_HeaderOffset);
	//return m_Parent->read(m_CSize);
	if (compressed())
		return "SINGLEUNIT";

	return "-";
}

bool MPQArchive::FileNode::readFrom(MPQArchive *parent)
{
	char buffer[16];
	if (parent->read(buffer, 16)  != 16)
		return false;

	qint32 m_References;
	QByteArray m_Filename;
	int m_Size;
	return true;
}


MPQArchive::MPQArchive(const QString &file)
	: QFile(file)
{
	m_Error = NO_ERROR;
	m_HashTable = NULL;

	m_FileList = NULL;
	m_BlockTable = NULL;
	m_HashTable = NULL;
	m_ListFile = NULL;

	quint32 dwSeed = 0x00100001;

	// Initialize the decryption buffer.
	for (int index1 = 0; index1 < 0x100; index1++)
	{
		for (int index2 = index1, i = 0; i < 5; i++, index2 += 0x100)
		{
			quint32 temp1, temp2;

			dwSeed = (dwSeed * 125 + 3) % 0x2AAAAB;
			temp1  = (dwSeed & 0xFFFF) << 0x10;

			dwSeed = (dwSeed * 125 + 3) % 0x2AAAAB;
			temp2  = (dwSeed & 0xFFFF);

			m_CryptBuffer[index2] = (temp1 | temp2);
		}
	}
}

quint32 MPQArchive::getHash(const char *data, int offset)
{
	quint32 Seed1 = 0x7FED7FED;
	quint32 Seed2 = 0xEEEEEEEE;

	while(*data != 0)
	{
		quint32 ch = toupper(*data++);

		Seed1 = m_CryptBuffer[offset + ch] ^ (Seed1 + Seed2);
		Seed2 = ch + Seed1 + Seed2 + (Seed2 << 5) + 3;
	}

	return Seed1;
}

bool MPQArchive::open()
{
	if (!QFile::open(ReadOnly))
	{
		m_Error = CANNOT_OPEN_FILE;
		return false;
	}

	m_HeaderOffset = 0;

	for(;;)
	{
		if (!m_Header.readFrom(this))
		{
			m_Error = FILE_CORRUPT;
			return false;
		}

		// Special check : Some MPQs are actually AVI files, only with
		// changed extension.
		if (m_Header.m_ID == "FFIR" ||
			m_Header.m_ID == " IVA" ||
			m_Header.m_ID == "TSIL")
		{
			m_Error = FILE_IS_AVI;
			return false;
		}
/*
/ note: Ignore the MPQ shunt completely if the caller wants to open the MPQ as V1.0

		// If there is the MPQ shunt signature, process it
		if (m_Header.m_ID == "MPQ\x1B")
		{

			//ha->ShuntPos = MpqPos;

			if (!m_CurrentShunt.readFrom(this))
				return false;

			// Set the MPQ pos and repeat the search
			if (!seek(m_CurrentShunt.m_HeaderPos))
				return false;

			continue;
		}
*/
		// There must be MPQ header signature
		if (m_Header.m_ID != "MPQ\x1A")
		{
			m_HeaderOffset += 0x200;
			if (!seek(m_HeaderOffset))
			{
				m_Error = FILE_CORRUPT;
				return false;
			}

			continue;
		}

		// If valid signature has been found, break the loop
		if(m_Header.m_FormatVersion == 0)
			break;

		m_Error = INVALID_HEADER;
		return false;
	}

	m_BlockSize = 0x200 << m_Header.m_BlockSize;
	DEBUG_Print("Block size is " + QString::number(m_Header.m_BlockSize) + " bzw. " + QString::number(m_BlockSize));

	// read hash table
	if (m_Header.m_HashTableSize > 0)
	{
		m_HashTable = new HashEntry[m_Header.m_HashTableSize];
		seek(m_Header.m_HashTablePos + m_HeaderOffset);

		// Decrypt hash table on the fly
		const char *key = "(HASH TABLE)";
		quint32 Seed1 = getHash(key, 0x300);//0x7FED7FED;
		quint32 Seed2 = 0xEEEEEEEE;

		// Prepare seeds
		/*for (uint i = 0; i < strlen(key); i++)
		{
			Seed1 = m_CryptBuffer[0x300 + key[i]] ^ (Seed1 + Seed2);
			Seed2 = key[i] + Seed1 + Seed2 + (Seed2 << 5) + 3;
		}

		Seed1 = getHash(key, 0x300);
		Seed2 = 0xEEEEEEEE;
		*/

		for (uint i = 0; i < m_Header.m_HashTableSize; i++)
		if (!m_HashTable[i].readFrom(this, Seed1, Seed2))
		{
			m_Error = FILE_CORRUPT;
			return false;
		}

		DEBUG_Print("Read "+ QString::number(m_Header.m_HashTableSize) + " hash entries");
	}

	// read block table
	if (m_Header.m_BlockTableSize > 0)
	{
		m_BlockTable = new BlockEntry[m_Header.m_BlockTableSize];
		seek(m_Header.m_BlockTablePos + m_HeaderOffset);

		// Decrypt hash table on the fly
		const char *key = "(BLOCK TABLE)";
		quint32 Seed1 = getHash(key, 0x300);//0x7FED7FED;
		quint32 Seed2 = 0xEEEEEEEE;

		// Prepare seeds
		/*for (uint i = 0; i < strlen(key); i++)
		{
			Seed1 = m_CryptBuffer[0x300 + key[i]] ^ (Seed1 + Seed2);
			Seed2 = key[i] + Seed1 + Seed2 + (Seed2 << 5) + 3;
		}

		Seed2 = 0xEEEEEEEE;
		*/

		for (uint i = 0; i < m_Header.m_BlockTableSize; i++)
		if (!m_BlockTable[i].readFrom(this, Seed1, Seed2))
		{
			m_Error = FILE_CORRUPT;
			return false;
		}

		DEBUG_Print("Read "+ QString::number(m_Header.m_BlockTableSize) + " block entries");
	}

	// verify the block table
	BlockEntry *pBlockEnd = m_BlockTable + m_Header.m_BlockTableSize + 1;
	BlockEntry *pBlock    = m_BlockTable;

	// we will check if all sizes in the block table is correct.
	for(; pBlock < pBlockEnd; pBlock++)
	if(pBlock->m_Flags & 0x80000000 /*MPQ_FILE_EXISTS*/ && pBlock->m_FilePos > size())
	{
		m_Error = BLOCK_TABLE_CORRUPT;
		return false;
	}

	// Add the internal listfile
	const char* listfilename = "(listfile)";
	HashEntry *pHash = GetHashEntry(listfilename);

	if (pHash == NULL || pHash->m_BlockIndex > m_Header.m_BlockTableSize)
	{
		m_Error = NO_FILE_LIST;
		return false;
	}

	// Get block and test if the file was not already deleted.
	pBlock = m_BlockTable + pHash->m_BlockIndex;

	if (!pBlock->exists() || !pBlock->valid())
	{
		m_Error = NO_FILE_LIST;
		return false;
	}

	DEBUG_Print("file list @block " + QString::number(pHash->m_BlockIndex));

	m_ListFile = new MPQFile(this, listfilename, pBlock, pHash);

	 // If the caller didn't specified otherwise,
	// load the "(attributes)" file
	// Ignore the result here. Attrobutes are not necessary,
	// if they are not there, we will just ignore them
//	SAttrFileLoad(ha);

	return true;
}

MPQFile* MPQArchive::getFile(const QByteArray &filename)
{
	HashEntry *pHash = GetHashEntry(filename.data());

	if (pHash == NULL || pHash->m_BlockIndex > m_Header.m_BlockTableSize)
	{
		m_Error = NO_FILE_LIST;
		return false;
	}

	// Get block and test if the file was not already deleted.
	BlockEntry *pBlock = m_BlockTable + pHash->m_BlockIndex;

	if (!pBlock->exists() || !pBlock->valid())
	{
		m_Error = NO_FILE_LIST;
		return false;
	}

	DEBUG_Print("file @block " + QString::number(pHash->m_BlockIndex));

	return new MPQFile(this, filename.data(), pBlock, pHash);
}

MPQArchive::~MPQArchive()
{
	delete[] m_CryptBuffer;
	delete[] m_BlockTable;
	delete[] m_HashTable;
	delete[] m_FileList;
	delete m_ListFile;
}

MPQArchive::HashEntry *MPQArchive::GetHashEntry(const char * file, quint32 locale)
{
	HashEntry *pHashEnd = m_HashTable + m_Header.m_HashTableSize;
	HashEntry *pHash0;                      // File hash entry (start)
	HashEntry *pHash;                       // File hash entry (current)
	HashEntry *pHashNeutral = NULL;

	// Decrypt name and block index
	quint32 index = getHash(file) & (m_Header.m_HashTableSize - 1);
	quint32 name1 = getHash(file, 0x100);
	quint32 name2 = getHash(file, 0x200);
	cout << file << " " << index << " " << m_Header.m_HashTableSize << " " << name2 << endl;

	pHash = pHash0 = m_HashTable + index;

	// Look for hash index
	while(pHash->m_BlockIndex != 0xFFFFFFFF /*HASH_ENTRY_FREE*/)
	{
		if(pHash->m_Name1 == name1 &&
		   pHash->m_Name2 == name2 &&
		   pHash->m_BlockIndex != 0xFFFFFFFE /*HASH_ENTRY_DELETED*/)
		{
			if (locale == 0xFFFFFFFF)
				return pHash;

			else
			{
				if (pHash->m_Locale == 0)
					pHashNeutral = pHash;

				if (pHash->m_Locale == locale)
					return pHash;
			}
		}

		// Move to the next hash entry
		if(++pHash >= pHashEnd)
			pHash = m_HashTable;
		if(pHash == pHash0)
			break;
	}

	// File was not found
	return pHashNeutral;
}

