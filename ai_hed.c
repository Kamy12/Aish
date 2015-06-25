/*	Copyright 1993 H.Ogasawara (COR.)	*/

#include	"ai.h"
#include	"microfio.h"

struct headline {
	unsigned char	sequence,	/* 0 only */
			volume,		/* 0:single volume, 1..250:volume */
			fs[4],		/* file size */
			line[2],	/* bytes of line */
			block[2],	/* lines in block */
			orgbit,		/* 8:jis8,16:jis7|sjis|sjisn */
			ishbit,		/* 8:jis8,13:jis7,15:sjis,14,sjisn */
			node[8], ext[3], /* file name (MS-DOS format) */
			janle,		/* zero */
			os,		/* os type */
			exttype,	/* 0:old_style 1:check sum */
/* 0:time stamp, 1:difference time, 2:CRC16, 3:CRC32, 4:CRCvol16 */
			flag,		/* bit flag */
			date[4],	/* time & date (MS-DOS format) */
			tsec[3],	/* difference time */
			crc16[2],	/* total crc16 */
			crc32[4],	/* total crc32 */
/* end of volume check */
			vbyte[4],	/* volume size (byte) */
			vcrc16[2],	/* volume crc16 */
			reserve[15],
			dummy[17];	/* jis8 headder */
};

static unsigned char	ishbit2z[]= {
	0,0,0,0,0,0,0,0,zJIS8,0,0,0,0,zJIS7,zSJISN,zSJIS };

extern unsigned int	decline_7(), decline_8(), decline_s(), decline_n(),
			encline_7(), encline_8(), encline_s(), encline_n();
extern unsigned char	_sf_jis7[], _sf_jis8[], _sf_sjis[], _sf_nksjis[];

T_Z	ztype[]= {
	{ decline_7, encline_7, _sf_jis7,  {'j','i','s','7',0}, 63, 13, 0x9d },
	{ decline_8, encline_8, _sf_jis8,  {'j','i','s','8',0}, 69,  8, 0x1a },
	{ decline_s, encline_s, _sf_sjis,  {'s','j','i','s',0}, 73, 15, 0x04 },
	{ decline_n, encline_n, _sf_nksjis,{'n','k','-','s','j','i','s',0}, 69, 14, 0x1a }
};


#if UUENCODE
unsigned int
readuuhead( hp, ptr, slen )
T_HD		*hp;
unsigned char	*ptr;
unsigned int	slen;
{
	unsigned int	mode= hp->mode;
	memclr( sizeof(T_HD)-NAMES, hp );
	hp->mode= mode;
	ptr= (unsigned char*)getword( hp->fname, ptr );
	if( *ptr ){
		ptr= (unsigned char*)getword( hp->fname, ptr );
		hp->perm= num7str( hp->fname );
		if( hp->perm >= 0 && *ptr ){
			ptr= (unsigned char*)getword( hp->fname, ptr );
			if( !(hp->mode & mLIST) ){
				Merr( hp->fname );
				Merrch( ' ' );
			}else{
				Mprint( hp->fname );
				Mprint( " line:" );
			}
			hp->ztype= zUUE;
			hp->uulength= -1;
			hp->fsize= 0;
			hp->nextline= 0;
			return	TRUE;
		}
	}
	return	FALSE;
}
#endif

void
Numerr( i )
{
	char	buf[20];
	strnum( buf, i );
	Merr( buf );
}

void
Numput( i )
{
	char	buf[20];
	strnum( buf, i );
	Mprint( buf );
}

static unsigned int
readhead( hp, ptr, slen )
T_HD		*hp;
unsigned char	*ptr;
unsigned int	slen;
{
	struct headline	hd;
	unsigned int	len= *ptr == '!' ? decline_7( &hd, ptr, slen ) :
						decline_8( &hd, ptr, slen );
	if( len > 60 && crc_check( 0xffff, &hd, len ) == 0x1d0f ){
		unsigned int	i;
		unsigned char	*s, *p;
/*		memclr( sizeof(T_HD), hp );*/
		timecpy( &hp->fsize, hd.fs );
		if( hp->vol= hd.volume ){
			timecpy( &hp->volsize, hd.vbyte );
			hp->fsize= (hp->volsize*hd.volume > hp->fsize ?
				hp->fsize-(hp->volsize*(hd.volume-1)) : hp->volsize);
		}
		hp->byte= hd.line[0]+(hd.line[1]<<8);
		hp->tcrc= (hd.crc16[0]<<8)+hd.crc16[1];
		hp->body= hp->byte-3;
		hp->blockbyte= hp->body*hp->body;
		hp->date= hp->perm= 0;
		if( hd.flag & 1 )
			timecpy( &hp->date, hd.date );
		hp->initflag= TRUE;
		hp->exec= ztype[hp->ztype= ishbit2z[hd.ishbit]].decexec;
		hp->block= hp->fsize/hp->blockbyte +1;
		hp->fsize+= 2;
		hp->dline= hp->fsize/hp->body+1;
		for( i= 0, s= hp->fname, p= hd.node ; *p != ' ' && i<8 ; i++ )
			*s++= *p++;
		if( *hd.ext != ' ' )
			for( *s++= '.', i= 0, p= hd.ext ; *p!=' '&&i<3 ; i++ )
				*s++= *p++;
		*s= '\0';

		if( !(hp->mode & mADJNAME) ){
			unsigned int	ic;
			for( ic= 0, s= hp->fname, p= hp->adjname ;
				*s && *s == *p && ic<8 ; s++, p++, ic++ );
			if( (!*s || *s == '.' || ic == 8) && *p )
				p_strcpy( hp->fname, hp->adjname );
		}

	/* debug */
		{
			if( !(hp->mode & mLIST) ){
				Merr( hp->fname );
				Merrch( '(' );
				Numerr( hp->fsize-2 );
				if( hd.volume ){
					Merrch( '/' );
					Numerr( hd.volume );
				}
				Merrch( ')' );
			}else{
				char	buf[28];
				Mprint( hp->fname );
				Mputchar( ' ' );
				Mputchar( '(' );
				Numput( hp->fsize-2 );
				if( hd.volume ){
					Mputchar( '/' );
					Numput( hd.volume );
				}
				Mputchar( ')' );
				Mputchar( ' ' );
				DATESTR(
					((hp->date & 0xfe000000)>>9)+
					((hp->date & 0x01e00000)>>13)+
					((hp->date & 0x001f0000)>>16)+
					0x27bc0000
						, buf );
				Mprint( buf );
				Mputchar( ' ' );
				TIMESTR(
					((hp->date & 0x0000f800)<<5)+
					((hp->date & 0x000007e0)<<3)+
					((hp->date<<1) & 0x3e)
						, buf );
				Mprint( buf );
				Mputchar( ' ' );
				Mprint( ztype[hp->ztype].name );
				Mprint( " os:" );
				Numput( hd.os );
#if 0
				if( hd.flag & 12 ){
					unsigned	i,j;
					i= (hd.crc16[0]<<8)+hd.crc16[1];
					timecpy( &j, hd.crc32 );
					if( filecrc( hp->fname, i, j ) ){
						Mprint( "<crcOK!>" );
					}
				}
#endif
				Mprint( " line:" );
			}
		}
		return	TRUE;
	}
	return	FALSE;
}


unsigned int
checkhead( hp, ptr, slen )
T_HD		*hp;
unsigned char	*ptr;
unsigned int	slen;
{
	if( (*ptr == '!' || *ptr == '#' ) && slen >= 76 && slen <= 81 ){
		return	readhead( hp, ptr, slen );
#if UUENCODE
	}else if( !(hp->mode & mUUENC) &&
				*ptr == 'b' && !p_strmatch( ptr, "begin " ) ){
		return	readuuhead( hp, ptr, slen );
#endif
	}else if( *ptr == '<' && ptr[1] == '<' && ptr[2] == '<' &&
		ptr[3] == ' ' && slen >= 18 ){
		unsigned char	*str= hp->adjname;
		int	i= 30;
		for( ptr+= 4 ; *ptr && *ptr != ' ' && i-- ; *str++= *ptr++ );
		*str= '\0';
	}
	return	FALSE;
}


writehead( hp, hd )
T_HD	*hp;
struct headline	*hd;
{
	unsigned char	*s, *p, *ptr, buf[30];
	unsigned int	i, extflag= FALSE;
	memclr( sizeof(struct headline), hd );
	if( hp->vol ){
		int	mv= (hp->os>>8)&0xff;
		timecpy( hd->fs, &hp->volsize );
		if( hp->vol > 1 && hp->vol == mv ){
			mv= (hp->volsize-hp->fsize)/(hp->vol-1);
			timecpy( hd->vbyte, &mv );
		}else
			timecpy( hd->vbyte, &hp->fsize );
		timecpy( hd->crc32, &hp->tcrc32 );
		hd->crc16[0]= hp->tcrc>>8;
		hd->crc16[1]= hp->tcrc;
		hd->flag= 13;	/* use:time stamp,crc16,crc32 */
	}else{
		timecpy( hd->fs, &hp->fsize );
		hd->flag= 1;	/* use:time stamp */
	}
	hd->line[0]= hp->byte;
	hd->line[1]= hp->byte>>8;
	hd->block[0]= hp->byte-1;
	hd->block[1]= (hp->byte-1)>>8;
	hd->ishbit= ztype[hp->ztype].code;
	hd->orgbit= hp->ztype == zJIS8 ? 8 : 16;
	ptr= (unsigned char*)ishname( buf, hp->fname );
	if( *ptr == '.' )
		extflag= TRUE;
	for( *ptr++='\0', s=hd->node, p=buf,i=8 ; *p && i ; i--, *s++= *p++ );
	for(; i-- ; *s++= ' ' );
	i= 3;
	if( extflag )
		for(; *ptr && i ; i--, *s++= *ptr++ );
	for(; i-- ; *s++= ' ' );
	hd->os= hp->os;
	hd->volume= hp->vol;
	hd->exttype= 1;	/* 1:new_style */
	timecpy( hd->date, &hp->date );
}
