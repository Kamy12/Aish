/*	Copyright 1993,94 H.Ogasawara (COR.)	*/

#include	"config.h"

#if PREREAD
unsigned char *
#else
void
#endif
p_strcpy( str, ptr )
unsigned char	*str, *ptr;
{
	while( *str++= *ptr++ );
#if PREREAD
	return	str;
#endif
}

p_strcmp( str, ptr )
char	*str, *ptr;
{
	for(; *str && *str == *ptr ; str++, ptr++ );
	return	*str-*ptr;
}

#if UUENCODE
p_strmatch( str, ptr )
char	*str, *ptr;
{
	for(; *ptr && *str == *ptr ; str++, ptr++ );
	return	*ptr;
}

num7str( ptr )
unsigned char	*ptr;
{
	int	i= 0, perm= 0;
	for( i= 0 ; i < 3 && *ptr >= '0' && *ptr <= '7' ; i++, ptr++ ){
		perm<<= 3;
		perm+= *ptr & 7;
	}
	if( !i || *ptr )
		return	-1;
	return	perm;
}
#endif

strnum( str, i )
unsigned int	i;
unsigned char	*str;
{
	unsigned char	buf[24], *ptr= buf+24;
	*--ptr= '\0';
	do{
		*--ptr= i%10+'0';
	}while( i/= 10 );
	for(; *str++= *ptr++ ; i++ );
	return	i;
}

numstr( ptr )
unsigned char	*ptr;
{
	unsigned int	i= 0;
	for(; *ptr ; ptr++ ){
		i*= 10;
		i+= *ptr & 15;
	}
	return	i;
}

unsigned char *
ishname( str, ptr )
unsigned char	*str, *ptr;
{
	extern unsigned char	sjis_chk[];
	unsigned int	kanji= 0;
	unsigned char	*lastpos;
	for( lastpos= ptr ; *ptr ; ptr++ ){
		if( kanji ){
			kanji= 0;
		}else{
			kanji= sjis_chk[*ptr];
			if( *ptr == '/' || *ptr == '\\' || *ptr == ':' )
				lastpos= ptr+1;
		}
	}
	for( ptr= 0 ; *lastpos ; *str++= *lastpos++ ){
		if( *lastpos == '.' )
			ptr= str;
	}
	*str= '\0';
	return	ptr ? ptr : str;
}

