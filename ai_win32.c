/* Copyright 1994-1998 Hiroyuki Ogasawara (COR.) */

#include	"config.h"

unsigned char	_sf_jis7[2],
		_sf_jis8[2],
		_sf_sjis[2],
		_sf_nksjis[2];

memclr( len, ptr )
char	*ptr;
{
	return	memset( ptr, '\0', len );
}

timecpy( dst, src )
unsigned int	*dst, *src;
{
	int	check= 1;
	if( *(char*)&check ){
		*dst= *src;
	}else{
		unsigned char	*d= ((unsigned char*)dst)+4,
				*s= (unsigned char*)src;
		*--d= *s++;
		*--d= *s++;
		*--d= *s++;
		*--d= *s++;
	}
}

DATESTR( date, str )
char	*str;
{
	int	year= (date & 0x0fff0000)>>16,
		month=(date & 0x0000ff00)>>8,
		mday= (date & 0x000000ff);
	sprintf( str, "%02d-%02d-%02d", year-1900, month, mday );
}

TIMESTR( time, str )
char	*str;
{
	int	hour=  (time & 0x00ff0000)>>16,
		minute=(time & 0x0000ff00)>>8,
		second=(time & 0x000000ff);
	sprintf( str, "%02d:%02d:%02d", hour, minute, second );
}

#if WIN32

#include	<time.h>
#include	<sys/types.h>
#include	<sys/stat.h>

#ifdef BFTIME
# include	<sys/timeb.h>
#endif

filesize( fn )
{
	struct stat	st;
	fstat( fn, &st );
	return	st.st_size;
}

unsigned int
filedate( fn )
{
	struct stat	st;
	struct tm	*tp;
	fstat( fn, &st );
	tp= localtime( &st.st_mtime );

	return	((tp->tm_year+1900-1980)<<25)+
		((tp->tm_mon+1)<<21)+
		(tp->tm_mday<<16)+

		(tp->tm_hour<<11)+
		(tp->tm_min<<5)+
		(tp->tm_sec/2);

}

unsigned int
fileperm( fn )
{
	struct stat	st;
	fstat( fn, &st );
	return	st.st_mode & 0777;
}

filesetdate( name, date )
unsigned char	*name;
unsigned int	date;
{
	static int	month[12]= {	0,
					31,
					31+28,
					31+28+31,
					31+28+31+30,
					31+28+31+30+31,
					31+28+31+30+31+30,
					31+28+31+30+31+30+31,
					31+28+31+30+31+30+31+31,
					31+28+31+30+31+30+31+31+30,
					31+28+31+30+31+30+31+31+30+31,
					31+28+31+30+31+30+31+31+30+31+30,
				/*	31+28+31+30+31+30+31+31+30+31+30+31*/};
	unsigned int	year=(date>>25)+1980-1970,
			mon= (date>>21)&15,	/* 1 Å` 12 */
			mday=(date>>16)&31,	/* 1 Å` 31 */
			hour=(date>>11)&31,	/* 0 Å` 23 */
			min= (date>>5)&63,	/* 0 Å` 59 */
			sec= (date&31)*2;	/* 0 Å` 58 */
	time_t		tim[2];
#ifdef SYSV
	extern int	timezone;
	tzset();
#endif
#ifdef BSD_TM
	struct tm	*tp;
	tim[0]= 0;
	tp= localtime( tim );
#endif
#ifdef BFTIME
	struct timeb	tb;
	ftime( &tb );
#endif
	mday= year*365 +((year+2)/4) -((year+70)/100) +((year+370)/400)
						+month[mon-1] +mday -1;
	if( (!(year+2)&3) && mon < 2 )
		mday--;
	tim[0]= tim[1]= mday*24*60*60 +hour*60*60 +min*60 +sec
#ifdef SYSV
				+timezone
#endif
				;
	utime( name, tim );
}

#endif

#if CKNAME
unsigned char *
ckname( name )
unsigned char	*name;
{
	unsigned char	*p= name;
	static unsigned char	new[32];
	for(; *p ; p++ ){
		if( *p & 128 ){
			int	count= 0;
			do{
				sprintf( new, "aish_8bit_name.%03d", count++ );
			}while( access( new, 0 /*F_OK*/ ) >= 0 );
			Merr( " rename -> " );
			Merr( new );
			Merr( " " );
			return	new;
		}
	}
	return	name;
}
#endif

