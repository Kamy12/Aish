/*	Copyright 1994 H.Ogasawara (COR.)	*/

#include	"config.h"
#include	"microfio.h"

#if HUMAN68K
#  if SYSDOSLIB
#    include	<sys_doslib.h>
#  else
#    include	<doslib.h>
#  endif
#endif
#if UNIX || WIN32
#  include	<sys/types.h>
#  include	<sys/stat.h>
#endif


#define		TRUE	1
#define		FALSE	0
#ifndef NULL
#  define	NULL	0
#endif

#define		AISH	"aish"
#define		TMPNM	"aiSmvTmp.tmp"

#define		BUFSIZE	(1024*32)
#define		RBSIZE	256

unsigned int	fsize,
		line= 500,
		volume= 1,
		volmax= 0,
		volsize= 0,
		mode= 0,
		fcrc16,
		fcrc32,
		os= 0,
		fdate,
		title= 200,
		iopt= FALSE,
#if UNIX
		aopt= TRUE;
#else
		aopt= FALSE;
#endif
unsigned char	fname[NAMELEN],
		sname[NAMELEN],
		tmpnm[PATHLEN],
		buf[BUFSIZE];

static m_file	fm, fmo;

unsigned char	sjis_chk[]= {
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0
	};

void
usage()
{
	Mprint( "\
aishmv 1.13 Multi-Volume ish encoder 1994,95,98 H.Ogasawara (COR.)\r\n\
usage: aishmv -{s78n} [-iado<file>T<file>O<n>t<n>m<n>] <encode file> ..\r\n" );
}


static void
mverr( msg1, msg2 )
char	*msg1, *msg2;
{
	Merr( msg1 );
	Merr( msg2 );
	Merr( "\r\n" );
}

unsigned char *
pp_strcpy( s, p )
unsigned char	*s, *p;
{
	for(; *s++= *p++ ;);
	return	s-1;
}

unsigned char *
getword( str, ptr )
unsigned char	*str, *ptr;
{
	for(; *ptr && (*ptr == ' ' || *ptr == '\t') ; ptr++ );
	for(; *ptr && *ptr != ' ' && *ptr != '\t' ; *str++= *ptr++ );
	*str= '\0';
	return	ptr;
}

#if UNIX
static
fgetdate( fn )
{
	struct stat	st;
	struct tm	*tp;
	fstat( fn, &st );
	return	st.st_mtime;
}

static
fsetdate( name, date )
char	*name;
{
	int	tim[2];
	tim[0]= tim[1]= date;
	utime( name, tim );
}

static void
qexec( ptr )
char	*ptr;
{
	char	*buf[20], **ptrs= buf;
	for(; *ptr ;){
		for(; *ptr == ' ' ; *ptr++= '\0' );
		if( *ptr )
			for( *ptrs++= ptr ; *ptr && *ptr != ' ' ; ptr++ );
	}
	*ptrs++= NULL;
	if( !fork() ){
		execvp( *buf, buf );
		mverr( *buf, ":exec err" );
		exit( 0 );
	}
	wait( 0 );
}

#endif

#if WIN32
static
fgetdate( fn )
{
	struct stat	st;
	struct tm	*tp;
	fstat( fn, &st );
	return	st.st_mtime;
}

static
fsetdate( name, date )
char	*name;
{
	int	tim[2];
	tim[0]= tim[1]= date;
	utime( name, tim );
}

#include	<process.h>

static void
qexec( ptr )
char	*ptr;
{
	char	*buf[20], **ptrs= buf;
	for(; *ptr ;){
		for(; *ptr == ' ' ; *ptr++= '\0' );
		if( *ptr )
			for( *ptrs++= ptr ; *ptr && *ptr != ' ' ; ptr++ );
	}
	*ptrs++= NULL;
	if( spawnvp( _P_WAIT, *buf, buf ) == -1 )
		mverr( *buf, ":exec err" );
}

#endif

#if HUMAN68K
static void
qexec( cmd )
{
	unsigned char	cline[256];
	if( PATHCHK( cmd, cline, NULL ) >= 0 )
		LOADEXEC( cmd, cline, NULL );
	else
		mverr( AISH, ":exec err" );
}
#endif

static
filecrc()
{
	unsigned short	crc16= 0xffff;
	unsigned int	crc32= 0xffffffff;
	int		total= 0, size;
	do{
		total+= size= Mread( &fm, buf, BUFSIZE );
		crc16= (unsigned short)crc_check( crc16, buf, size );
		crc32= (unsigned int)crc_check32( crc32, buf, size );
	}while( size == BUFSIZE );
	fcrc16= (~crc16) & 0xffff;
	fcrc32= ~crc32;
	return	total;
}

static void
calcline()
{
	int	body= 69-3, hline;
	if( mode == '7' )
		body= 63-3;
	else if( mode == 's' )
		body= 73-3;
	hline= line-6 -(line/title);
	volsize= (hline -(iopt ? -2 : (hline/body+1)*2))*body;
	volmax= (fsize+volsize-1)/volsize;
}

static
aish()
{
	unsigned char	cmd[1024], ofname[NAMELEN], ifname[NAMELEN];
	int		last= FALSE;

	calcline();

	if( volmax == 1 ){
		mverr( fname, ":can't split" );
		return	TRUE;
	}
	if( Mcreate( &fmo, tmpnm ) ){
		unsigned int	rsize, size, total= volsize;
		unsigned char	*ptr, *str= cmd;
		do{
			rsize= total > BUFSIZE ? BUFSIZE : total;
			size= Mread( &fm, buf, rsize );
			last= size < rsize;
			if( Mwrite( &fmo, buf, size ) < 0 )
				mverr( tmpnm, ":tmp write err" );
		}while( !last && (total-= rsize) );
#if HUMAN68K
		Mflush( &fmo );
		FILEDATE( fmo.fn, fdate );
#endif
		Mclose( &fmo );
#if UNIX || WIN32
		fsetdate( tmpnm, fdate );
#endif
		ishname( ifname, fname );
		ptr= (unsigned char*)ishname( ofname, *sname ? sname : fname );
		if( *ofname != '-' || ofname[1] ){
			if( aopt )
				for(; *ptr ; ptr++ );
			*ptr++= '.';
			*ptr++= 'i'+(volume/100);
			*ptr++= ((volume/10)%10)+'0';
			*ptr++= (volume%10)+'0';
			*ptr= '\0';
		}

		str= pp_strcpy( str, AISH );
		*str++= ' ';
		*str++= '-';
		*str++= 'O';
		str+= strnum( str, (volume<<16)+(volmax<<8)+(os&255) );
		*str++= ' ';
		*str++= '-';
		*str++= '_';
		str+= strnum( str, fsize );
		*str++= ' ';
		*str++= '-';
		*str++= '/';
		str+= strnum( str, fcrc16 );
		*str++= ' ';
		*str++= '-';
		*str++= '-';
		str+= strnum( str, fcrc32 );
		*str++= ' ';
		*str++= '-';
		*str++= 't';
		str+= strnum( str, title );
		*str++= ' ';
		*str++= '-';
		*str++= 'x';
		str= pp_strcpy( str, ifname );
		*str++= ' ';
		*str++= '-';
		if( iopt )
			*str++= 'i';
		*str++= mode;
		*str++= 'o';
		str= pp_strcpy( str, ofname );
		*str++= ' ';
		str= pp_strcpy( str, tmpnm );

#if 0
		sprintf( cmd, "%s -O%d -_%d -/%d --%d -t%d -x%s -%s%co %s %s",
				AISH, (volume<<16)+(volmax<<8)+(os&255),
				fsize, fcrc16, fcrc32, title, ifname,
				iopt ? "i" : "",
				mode, ofname, tmpnm );
#endif
#if 0
		Mprint( cmd );
		Mprint( "\r\n" );
#endif
		qexec( cmd );
		unlink( tmpnm );
		if( ++volume > 250 ){
			mverr( fname, ":too many volumes" );
			return	TRUE;
		}
	}else{
		mverr( tmpnm, ":create error" );
		return	TRUE;
	}
	return	last;
}

static void
gettemp( ptr )
unsigned char	*ptr;
{
#if HUMAN68K
	if( GETENV( "TEMP", 0, ptr ) >= 0 && *ptr ){
		for(; *ptr ; ptr++ );
		if( ptr[-1] != '/' && ptr[-1] != '\\' )
			*ptr++= '/';
	}
#endif
	p_strcpy( ptr, TMPNM );
}

#if SUPASM
void
main( arg )
unsigned char	*arg;
#else
void
main( argc, argv )
unsigned char	**argv;
#endif
{
	unsigned char	ptr[RBSIZE];
	*sname= *fname= '\0';
	gettemp( tmpnm );
#if SUPASM
	for(; arg= getword( ptr, arg ), *ptr ;)
#else
	for(; --argc && (p_strcpy( ptr, *++argv ),1) ;)
#endif
	{
		if( *ptr == '-' && ptr[1] ){
			unsigned char	*p= ptr+1;
			for(; *p ;) switch( *p++ ){
			case 's':
			case '7':
			case '8':
			case 'n':
				mode= p[-1];
				break;
			case 'm':
				if( *p ){
					line= numstr( p );
				}else{
#if SUPASM
					arg= getword( ptr, arg );
					line= numstr( ptr );
#else
					if( argc >= 2 ){
						argc--;
						line= numstr( *++argv );
					}
#endif
				}
				if( line < 20 )
					line= 20;
				goto	_fbreak;
			case 'i':
				iopt= TRUE;
				break;
			case 'a':
				aopt= TRUE;
				break;
			case 'd':
				aopt= FALSE;
				break;
			case 't':
				if( *p ){
					title= numstr( p );
				}else{
#if SUPASM
					arg= getword( ptr, arg );
					title= numstr( ptr );
#else
					if( argc >= 2 ){
						argc--;
						title= numstr( *++argv );
					}
#endif
				}
				goto	_fbreak;
			case 'O':
				if( *p ){
					os= numstr( p );
				}else{
#if SUPASM
					arg= getword( ptr, arg );
					os= numstr( ptr );
#else
					if( argc >= 2 ){
						argc--;
						os= numstr( *++argv );
					}
#endif
				}
				goto	_fbreak;
			case 'o':
				if( *p ){
					p_strcpy( sname, p );
				}else{
#if SUPASM
					arg= getword( sname, arg );
#else
					if( argc >= 2 ){
						argc--;
						p_strcpy( sname, *++argv );
					}
#endif
				}
				goto	_fbreak;
			case 'T':
				if( *p ){
					p_strcpy( tmpnm, p );
				}else{
#if SUPASM
					arg= getword( tmpnm, arg );
#else
					if( argc >= 2 ){
						argc--;
						p_strcpy( tmpnm, *++argv );
					}
#endif
				}
				goto	_fbreak;
			default:
				usage();
#if SUPASM
				return;
#else
				exit( 1 );
#endif
			}
			_fbreak:;
		}else if( mode ){
			p_strcpy( fname, ptr );
			if( Mopen( &fm, fname ) ){
#if HUMAN68K
				fdate= FILEDATE( fm.fn, 0 );
#endif
#if UNIX || WIN32
				fdate= fgetdate( fm.fn );
#endif
				Merr( "crc check.." );
				fsize= filecrc();
				Mseek( &fm, 0, 0 );
				Merr( "\r\n" );
				for(; !aish() ;);
				Mclose( &fm );
			}else
				mverr( fname, ":open error" );
		}
	}
	if( !mode )
		usage();
#if !SUPASM
	exit( 0 );
#endif
}

