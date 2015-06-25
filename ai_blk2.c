/* 1993,94 H.Ogasawara (COR.) */

#include	"ai.h"
#include	"microfio.h"

extern m_file	fm, fmo;

static INLINE
crc_set( str, len )
unsigned char	*str;
{
	unsigned short	crc= ~crc_check( 0xffff, str, len );
	str[len  ]= crc>>8;
	str[len+1]= crc;
}

#if !CSUMASM
static void INLINE
sum_check( hp, pos, ptr, sumh, sumv )
T_HD	*hp;
unsigned char	*ptr, *sumh, *sumv;
{
	int	i, j, len= hp->byte-2;
	for( i= 1 ; i< len ; i++ )
		sumv[i] -= ptr[i];
	for( j= (len- pos+1)%len, i= 0 ; i< len ; i++, j= (j+1) % len )
		sumh[j+1] -= ptr[i];
/*
	pos:   1  2  3  4  5  6  7
	len:                        8
	j:     7  6  5  4  3  2  1  0
	j+1:   8  7  6  5  4  3  2  1
	%len:  1  8  7  6  5  4  3  2
	%len:  2  1  8  7  6  5  4  3
	%len:  3  2  1  8  7  6  5  4
	%len:  4  3  2  1  8  7  6  5
	%len:  5  4  3  2  1  8  7  6
	%len:  6  5  4  3  2  1  8  7
	%len:  7  6  5  4  3  2  1  8
	%len:  8  7  6  5  4  3  2  1

*/
}
#endif

static void
enc_comment( hp )
T_HD	*hp;
{
	char	buf[20];
	static unsigned	oline;
	if( hp->cline == oline )
		return;
	Mputs( &fmo, "--- " );
	Mputs( &fmo, hp->fname );
	if( hp->vol ){
		Mputs( &fmo, " " );
		strnum( buf, hp->vol );
		Mputs( &fmo, buf );
		Mputs( &fmo, "/" );
		strnum( buf, hp->os>>8 );
		Mputs( &fmo, buf );
	}
	Mputs( &fmo, " (" );
	strnum( buf, hp->cline );
	Mputs( &fmo, buf );
	Mputs( &fmo, "/" );
	strnum( buf, hp->dline );
	Mputs( &fmo, buf );
	Mputs( &fmo, ") ---\r\n" );
	oline= hp->cline;
}

static void
print_line( hp, str )
T_HD	*hp;
unsigned char	*str;
{
	Mputs( &fmo, str );
	Mputs( &fmo, "\r\n" );
	hp->cline++;
	if( hp->cline == hp->nextline ){
		hp->cline++;
		enc_comment( hp );
		hp->nextline+= hp->stepline;
	}
}

unsigned char	sumh[DBUF],
		sumv[DBUF];
encblock( hp )
T_HD	*hp;
{
	static unsigned int	pos;
	static unsigned short	crc;
	unsigned int		ret= TRUE;
	unsigned char		str[DBUF],
				buf[DBUF];
#if UUENCODE
	if( hp->ztype == zUUE )
		return	uuencode( hp );
#endif
	if( hp->initflag ){
		unsigned char	*ptr, *nam;
		if( !Mopen( &fm, hp->fname ) ){
			Merr( "open error\r\n" );
			return	FALSE;
		}
		ptr= (unsigned char*)ishname( str, hp->fname );
		p_strcpy( hp->fname, str );
		if( hp->mode & mOUTPUT ){
			p_strcpy( str, hp->outname );
		}else{
			if( hp->mode & mAPPEND )
				for(; *ptr ; ptr++ );
			*ptr++= '.';
			*ptr++= 'i';
			*ptr++= 's';
			*ptr++= 'h';
			*ptr++= '\0';
		}
		Merr( str );
		Merrch( ':' );
		if( !Mcreate( &fmo, str ) ){
			Mclose( &fm );
			Merr( "create error\r\n" );
			return	FALSE;
		}
#if HUMAN68K
		hp->fsize= SEEK( fm.fn, 0, 2 );
		SEEK( fm.fn, 0, 0 );
		hp->date= FILEDATE( fm.fn, 0 );
#endif
#if UNIX || WIN32
		hp->fsize= filesize( fm.fn );
		hp->date= filedate( fm.fn );
#endif
		if( *hp->skipname )
			p_strcpy( hp->fname, hp->skipname );
		Mputs( &fmo, "<<< " );
		Mputs( &fmo, hp->fname );
		Mputs( &fmo, " " );
		if( hp->vol ){
			strnum( str, hp->vol );
			Mputs( &fmo, str );
			Mputs( &fmo, "/" );
			strnum( str, hp->os>>8 );
			Mputs( &fmo, str );
			Mputs( &fmo, " " );
		}
		strnum( str, hp->fsize );
		Mputs( &fmo, str );
		Mputs( &fmo, " byte (" );
		{
			T_Z	*zp= ztype+hp->ztype;
			hp->byte= zp->byte;
			hp->exec= zp->encexec;
			hp->sf_tbl= zp->sf_tbl;
			Mputs( &fmo, zp->name );
		}
		hp->body= hp->byte-3;
/*
<<< adepo023.lzh 304023 byte 00/00/00 00:00 (sjis) [9879 lines] aish 0.93 >>>
<<< adepo023.lzh 304023 byte (sjis) [9879 lines] aish 0.93 >>>
<<< adepo023.lzh 4/10 304023 byte (sjis) [9879 lines] aish 0.93 >>>
*/
		Mputs( &fmo, ") [" );
		hp->cline= 1;
		hp->dline= hp->fsize/hp->body+1+5+
				(hp->mode & mNOTSUM ? -2 :
				 (hp->fsize/(hp->body*hp->body)+1)*2);
		hp->nextline= hp->stepline-1;
		if( hp->dline != hp->stepline )
			hp->dline+= hp->dline/hp->stepline;
		strnum( str, hp->dline );
		Mputs( &fmo, str );
		Mputs( &fmo, " lines] aish 1.12 >>>\r\n" ); /* <=== Ver !!*/
		writehead( hp, buf );
		crc_set( buf, 63-2      /*jis7*/ );
		encline_7( str, buf, 63 /*jis7*/ );
		print_line( hp, str );
		if( !(hp->mode & mNOTSUM) ){
			print_line( hp, str );
			print_line( hp, str );
		}
		crc= 0xffff;
		pos= 1;
		hp->initflag= FALSE;
	}else{
		static unsigned int	lastblock= FALSE;
		buf[0]= pos;
		if( lastblock ){
			buf[1]= crc;
			memclr( hp->byte-2, buf+2 );
			lastblock= FALSE;
			ret= EOISH;
		}else{
			unsigned int	byte= Mread( &fm, buf+1, hp->body );
			crc= crc_check( crc, buf+1, byte );
			if( byte < hp->body ){
				int	len;
				buf[byte+1]= (crc=~crc)>>8;
				if( (len= hp->body-byte) >= 2 ){
					buf[byte+2]= crc;
					memclr( len, buf+byte+3 );
					ret= EOISH;
				}else
					lastblock= TRUE;
			}
		}
		if( pos == 1 ){
			memclr( DBUF, sumh );
			memclr( DBUF, sumv );
		}
#if !CSUMASM
		sum_check( hp, pos, buf, sumh, sumv );
#else
		sum_check( pos, hp->body, buf, hp->sf_tbl );
#endif
		crc_set( buf, hp->byte-2 );
		(*hp->exec)( str, buf, hp->byte );
		print_line( hp, str );
		if( pos == hp->body || ret == EOISH ){
			if( !(hp->mode & mNOTSUM) ){
				sumv[0]= hp->byte-2;
				crc_set( sumv, hp->byte-2 );
				(*hp->exec)( str, sumv, hp->byte );
				print_line( hp, str );
#if !CSUMASM
				sum_check( hp, hp->byte-2, sumv, sumh, sumv );
#else
				sum_check2( hp->byte-2, hp->body, hp->sf_tbl );
#endif
				sumh[0]= hp->byte;
				if( ++pos != hp->byte-2 ){
					for(; pos < hp->byte-2 ; pos++ ){
						unsigned int	i=
							((hp->byte-2)-(pos-1))%
							(hp->byte-2)+1;
						sumh[i]=(sumh[i]&0xff)-
								(pos&0xff);
					}
				}
				crc_set( sumh, hp->byte-2 );
				(*hp->exec)( str, sumh, hp->byte );
				print_line( hp, str );
			}
			pos= 0;
			Merrch( 'o' );
			if( ret == EOISH ){
				hp->cline++;
				enc_comment( hp );
				Mflush( &fmo );
				Mclose( &fmo );
				Merr( "\r\n" );
				Mclose( &fm );
			}
		}
		pos++;
	}
	return	ret;
}

