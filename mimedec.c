/*	1995 H.Ogasawara (COR.)		*/

#if MIMEDEC

#include	"ai.h"
#include	"microfio.h"

#define		CHARSET		"ISO-2022-JP"
#define		ENCODING	"Base64"
#define		ENCODINGCH	'B'

#define		IOBUF		256
#define		STRBUF		256

#define		PutLine(a)	Mprint( a )
#define		PutChar(a)	Mputchar( a )

#define		cMULTIPART	0x10

static char	content_name[60],
		content_type[60],
		encoding_name[60],
		charset[60]= CHARSET,
		file_outname[255],
		file_boundary[255];
static	int	TmpNumber= 0;

static unsigned char	ibuf[IOBUF],
			obuf[IOBUF];

static unsigned char	pade[]= {
	'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
	'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',
	'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
	'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/',
};
static unsigned char	padd[]= {
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,62, 0, 0, 0,63,
	52,53,54,55,56,57,58,59,60,61, 0, 0, 0,99, 0, 0,
	 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
	15,16,17,18,19,20,21,22,23,24,25, 0, 0, 0, 0, 0,
	 0,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
	41,42,43,44,45,46,47,48,49,50,51, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};


static unsigned char *
decbenc( str, ptr )
unsigned char	*str, *ptr;
{
	for(; *ptr && *ptr != '\n' && *ptr != '\r' ; ptr+= 4 ){
		unsigned int	i= (padd[*ptr]<<18)+(padd[ptr[1]]<<12)+
					(padd[ptr[2]]<<6)+(padd[ptr[3]]);
		*str++= (i>>16)&255;
		if( ptr[2] != '=' )
			*str++= (i>>8) &255;
		if( ptr[3] != '=' )
			*str++= i & 255;
	}
	return	str;
}

static
ISalnum( ch )
unsigned char	ch;
{
	return	(ch >= '0' && ch <= '9')||
		(ch >= 'A' && ch <= 'Z')||
		(ch >= 'a' && ch <= 'z') || ch == '-' || ch == '_';
}

static
strcmpi( str, ptr )
unsigned char	*str, *ptr;
{
	for(; *str && (*str|0x20) == (*ptr|0x20) ; str++, ptr++ );
	return	(*str|0x20)-(*ptr|0x20);
}

static unsigned char *
get_tkn( str, ptr, ch )
unsigned char	*str, *ptr, ch;
{
	for(; *ptr && *ptr != ch ;)
		*str++= *ptr++;
	*str= '\0';
	if( *ptr == ch )
		ptr++;
	return	ptr;
}

static unsigned char *
get_tkn2( str, ptr, ch )
unsigned char	*str, *ptr, ch;
{
	if( *ptr == ch )
		ptr++;
	for(; *ptr && *ptr != ch ;)
		*str++= *ptr++;
	*str= '\0';
	if( *ptr == ch )
		ptr++;
	return	ptr;
}


static unsigned char *
get_word( str, ptr )
unsigned char	*str, *ptr;
{
	for(; *ptr && !ISalnum( *ptr ) ; ptr++ );
	for(; *ptr && ISalnum(*ptr) ;)
		*str++= *ptr++;
	*str= '\0';
	return	ptr;
}

static void
content_init()
{
	*content_type= '\0';
	*content_name= '\0';
	*encoding_name= '\0';
	*file_outname= '\0';
	*file_boundary= '\0';
}

static void
content_decode( ptr )
unsigned char	*ptr;
{
	unsigned char	buf[STRBUF];
	content_init();
	ptr= get_word( content_type, ptr );
	if( *ptr == '/' )
		ptr= get_word( content_name, ptr+1 );
	ptr= get_word( buf, ptr );
	if( *buf ){
		if( !strcmpi( buf, "charset" ) ){
			if( *ptr == '=' )
				ptr= get_tkn2( charset, ptr+1, '"' );
		}else if( !strcmpi( buf, "name" ) ){
			if( *ptr == '=' )
				ptr= get_tkn2( file_outname, ptr+1, '"' );
		}else if( !strcmpi( buf, "boundary" ) ){
			if( *ptr == '=' )
				ptr= get_tkn2( file_boundary, ptr+1, '"' );
		}
	}
}

line_decode( ptr )
unsigned char	*ptr;
{
	unsigned char	chset[STRBUF],
			encode[STRBUF];
	for(; *ptr ; ptr++ ){
		if( *ptr == '=' && ptr[1] == '?' ){
			unsigned char	*p= get_tkn( chset,  ptr+2, '?' );
			if( *p && !strcmpi( chset, charset ) ){
				ptr= get_tkn( encode, p, '?' );
				ptr= get_tkn( ibuf, ptr, '?' );
				p= decbenc( obuf, ibuf );
				*p= '\0';
				PutLine( obuf );
			}
		}else
			PutChar( *ptr );
	}
}

static void
skip_header( fm )
m_file	*fm;
{
	for(; Mgets( fm, ibuf, IOBUF ) ;){
		PutLine( ibuf );
		PutLine( "\r\n" );
		if( !*ibuf )
			return;
	}
}

static void
bin_decode( fm )
m_file	*fm;
{
	unsigned char	name[STRBUF], *s;
	m_file	fo;
	skip_header( fm );
	if( !*file_outname ){
		s= (void*)p_strcpy( name, content_type );
		s[-1]= '_';
		s= (void*)p_strcpy( s, content_name );
		s[-1]= '.';
		strnum( s, TmpNumber++ );
	}else{
		p_strcpy( name, file_outname );
	}
	if( Mcreate( &fo, name ) ){
		for(; Mgets( fm, ibuf, IOBUF ) ;){
			if( *ibuf != '-' || ibuf[1] != '-' ){
				unsigned char	*p= decbenc( obuf, ibuf );
				Mwrite( &fo, obuf, p-obuf );
			}else{
				PutLine( ibuf );
				PutLine( "\r\n" );
				break;
			}
		}
		Mflush( &fo );
		Mclose( &fo );
	}
}

MimeDec( fm )
m_file	*fm;
{
	content_init();
	for(; Mgets( fm, ibuf, IOBUF ) ;){
		if( !p_strmatch( ibuf, "Content-Type: " ) ){
			PutLine( ibuf );
			PutLine( "\r\n" );
			content_decode( ibuf+14 );
		}else if( !p_strmatch( ibuf, "Content-Transfer-Encoding: " ) ){
			PutLine( ibuf );
			PutLine( "\r\n" );
			get_word( encoding_name, ibuf+27 );
			/*encoding_decode( ibuf+27 );*/
			if( !strcmpi( encoding_name, ENCODING ) )
				bin_decode( fm );
		}else{
			line_decode( ibuf );
			PutLine( "\r\n" );
		}
	}
}

#endif
