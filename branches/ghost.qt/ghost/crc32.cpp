#include "ghost.h"
#include "crc32.h"

void CCRC32 :: Initialize( )
{
	for( int iCodes = 0; iCodes <= 0xFF; iCodes++ )
	{
		ulTable[iCodes] = Reflect( iCodes, 8 ) << 24;

		for( int iPos = 0; iPos < 8; iPos++ )
			ulTable[iCodes] = ( ulTable[iCodes] << 1 ) ^ ( ulTable[iCodes] & (1 << 31) ? CRC32_POLYNOMIAL : 0 );

		ulTable[iCodes] = Reflect( ulTable[iCodes], 32 );
	}
}

quint32 CCRC32 :: Reflect( quint32 ulReflect, char cChar )
{
	quint32 ulValue = 0;

	for( int iPos = 1; iPos < ( cChar + 1 ); iPos++ )
	{
		if( ulReflect & 1 )
			ulValue |= 1 << ( cChar - iPos );

		ulReflect >>= 1;
	}

	return ulValue;
}

quint32 CCRC32 :: FullCRC( QString data )
{
	quint32 ulCRC = 0xFFFFFFFF;
	PartialCRC( &ulCRC, (const unsigned char*)data.toStdString().c_str(), data.toStdString().size() );
	return ulCRC ^ 0xFFFFFFFF;
}

void CCRC32 :: PartialCRC( quint32 *ulInCRC, const unsigned char *sData, quint32 ulLength )
{
	while( ulLength-- )
		*ulInCRC = ( *ulInCRC >> 8 ) ^ ulTable[( *ulInCRC & 0xFF ) ^ *sData++];
}
