/* 1993,94 H.Ogasawara (COR.) */

#include	"ai.h"
#include	"microfio.h"

extern m_file	fmo;

#if !ERRCASM
unsigned char	errline[4];
#endif

static
ecc( hp, err, decbuf )
T_HD	*hp;
unsigned char	**decbuf;
{
	unsigned int	sum, i, j, k, x;
	extern unsigned char	errline[];
	unsigned int	e1= errline[0], e2= errline[1];
	if( err == 1 ){
		if( decbuf[hp->byte-2][0] ){
			for( j= 1 ; j< hp->byte-2 ; j++ ){
				for( i= 1, sum= 0 ; i< hp->byte-1 ; i++ )
					sum-= decbuf[i][j];
				decbuf[e1][j]= sum;
			}
			return	TRUE;
		}
		e2= hp->byte-2;
		decbuf[e2][0]= e2;
		memclr( hp->byte-2-1, decbuf[e2]+1 );
	}
	if( !decbuf[hp->byte][0] )
		return	FALSE;
/*	sum=	hp->code == CODE7 ? 0x9d :
		hp->code == CODES ? 0x04 : 0x1a ;*/
	sum= ztype[hp->ztype].sum;
	for( j= 1 ; j< hp->byte-2 ; sum-= decbuf[hp->byte][j++] );
	decbuf[hp->byte][0]= sum;
	memclr( hp->byte, decbuf[hp->byte-1] );
	for( x= k= 0 ; k < hp->body ; k++ ){
		if( x <= e1 )
			j= hp->byte-1-e1+x;
		else
			j= x-e1+1;
		for( i= 1, sum= 0 ; i<= hp->byte ; j++, i++ )
			sum -= decbuf[i][j=j%(hp->byte-2)];
		decbuf[e2][x= (x+e2-e1)%(hp->byte-2)]= sum;
		for( i= 1, sum= 0 ; i< hp->byte-1 ; sum -= decbuf[i++][x] );
		decbuf[e1][x]= sum;
	}
	return	TRUE;
}

static
crc_ad( hp, decbuf )
T_HD	*hp;
unsigned char	**decbuf;
{
	unsigned int	i, err, tic;
	static unsigned short	crc= 0xffff;
	static unsigned short	ccrc;
#if ERRCASM
	err= errcheck( hp->body, decbuf, &hp->dline );
#else
	tic= hp->dline < hp->body ? hp->dline : hp->body;
	for( hp->dline-= tic, i= 1, err= 0 ; i< hp->byte-2 ; i++ ){
		if( !decbuf[i][0] ){
			decbuf[i][0]= i;
			memclr( hp->byte-2-1, decbuf[i]+1 );
			if( i <= tic ){
				if( err < 2 )
					errline[err]= i;
				err++;
			}
		}
	}
#endif
	if( err ){
		Merrch( 'e' ); UFLUSH();
		if( (err >= 3 || !ecc( hp, err, decbuf )) ){
			Merr( " too many error\r\n" );
			crc= 0xffff;
			return	FALSE;
		}
	}else
		Merrch( 'o' ); UFLUSH();
	for( i= 1 ; i< hp->byte-2 ; i++ ){
		hp->fsize-= (tic= hp->fsize<hp->body ? hp->fsize : hp->body);
		if( hp->fsize >= 2 ){
			crc= crc_check( crc, decbuf[i]+1, tic );
			if( !Mwrite( &fmo, decbuf[i]+1, tic ) ){
				Merr( " write error\r\n" );
				crc= 0xffff;
				return	FALSE;
			}
		}else if( hp->fsize == 1 ){
			crc= crc_check( crc, decbuf[i]+1, tic-1 );
			if( !Mwrite( &fmo, decbuf[i]+1, tic-1 ) ){
				Merr( " write error\r\n" );
				crc= 0xffff;
				return	FALSE;
			}
			ccrc= decbuf[i][tic]<<8;
		}else{
			if( tic >= 2 ){
				if( tic > 2 ){
					crc= crc_check(crc,decbuf[i]+1,tic-2);
					if( !Mwrite( &fmo, decbuf[i]+1, tic-2 ) ){
						Merr( " write error\r\n" );
						return	FALSE;
					}
				}
				ccrc= (decbuf[i][tic-1]<<8)+decbuf[i][tic];
			}else{
				ccrc += decbuf[i][1];
			}
			break;
		}
	}
	for( i= 1 ; i< hp->byte ; *decbuf[i++]= '\0' );
	if( hp->fsize == 0 ){
#if ANSIC
		if( (unsigned short)~crc != ccrc )
#else
		if( (~crc & 0xffff) != ccrc )
#endif
		{
			Merr( "CRC error\r\n" );
			crc= 0xffff;
			return	FALSE;
		}
		Merr( "\r\n" );
		crc= 0xffff;
		return	EOISH;
	}
	return	TRUE;
}

/*
ret val:TRUE  continue
	FALSE error stop
	EOISH complete
	ESKIP skipline
*/
decblock( hp, ptr, slen )
T_HD		*hp;
unsigned char	*ptr;
unsigned int	slen;
{
	static unsigned int	pos= 0;
	unsigned int		len, row;
	static unsigned char	decbuf[DBUF][DBUF],
				*dpbuf[DBUF];
#if UUENCODE
	if( hp->ztype == zUUE )
		return	uudecode( hp, ptr, slen );
#endif
	if( hp->initflag ){
		if( hp->initflag == 0xff ){
			return	crc_ad( hp, dpbuf );
		}else{
			unsigned int	i;
			for( i= 0 ; i< DBUF ; dpbuf[i]= decbuf[i], i++ );
		}
		hp->initflag= FALSE;
	}
	if( slen < 60 ||
		(len= (*hp->exec)( dpbuf[0], ptr, slen )) < hp->byte ||
			crc_check( 0xffff, dpbuf[0], len ) != 0x1d0f ||
				(row= *dpbuf[0]) > hp->byte )
		return	ESKIP;
	if( row < pos ){
		unsigned int	ret= crc_ad( hp, dpbuf );
		unsigned char	*p= dpbuf[0];
		dpbuf[0]= dpbuf[row];
		dpbuf[pos= row]= p;
/*		*dpbuf[row]= row;*/
		return	ret;
	}else{
		unsigned char	*p= dpbuf[0];
		dpbuf[0]= dpbuf[row];
		dpbuf[pos= row]= p;
		if( row >= hp->byte ){
			pos= 0;
			return	crc_ad( hp, dpbuf );
		}
	}
	return	TRUE;
}

