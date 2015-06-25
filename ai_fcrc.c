/* Copyright 1994 H.Ogasawara (COR.) */

#include	"config.h"
#include	"microfio.h"

#define		BUFSIZE	(1024*4)

extern m_file	fmo;

#if CRC32
filecrc( name, c16, c32 )
char	*name;
unsigned short	c16;
unsigned int	c32;
#else
filecrc( name, c16 )
char	*name;
unsigned short	c16;
#endif
{
	unsigned char	buf[BUFSIZE];
	unsigned short	crc16= 0xffff;
#if CRC32
	unsigned int	crc32= 0xffffffff;
#endif
	if( Mopen( &fmo, name ) ){
		int	size;
		do{
			size= Mread( &fmo, buf, BUFSIZE );
			crc16= (unsigned short)crc_check( crc16, buf, size );
#if CRC32
			crc32= (unsigned int)crc_check32( crc32, buf, size );
#endif
		}while( size == BUFSIZE );
		Mclose( &fmo );
	}else{
		Merr( "file not found\r\n" );
		return	0;
	}
#if CRC32
#  if ANSIC
	if( c16 != (unsigned short)~crc16 || c32 != ~crc32 )
#  else
	if( c16 != ((~crc16)&0xffff) || c32 != ~crc32 )
#  endif
#else
#  if ANSIC
	if( c16 != (unsigned short)~crc16 )
#  else
	if( c16 != ((~crc16)&0xffff) )
#  endif
#endif
		return	0;
	return	1;
}
