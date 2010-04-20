#ifndef CRC32_H
#define CRC32_H

#define CRC32_POLYNOMIAL 0x04c11db7

class CCRC32
{
public:
	void Initialize( );
	quint32 FullCRC( const QByteArray& data );
	void PartialCRC( quint32 *ulInCRC, const unsigned char *sData, quint32 ulLength );

private:
	quint32 Reflect( quint32 ulReflect, char cChar );
	quint32 ulTable[256];
};

#endif
