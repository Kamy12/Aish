/*	Copyright 1993,94 H.Ogasawara (COR.)	*/

#include	"ai.h"
#include	"microfio.h"

extern m_file	fm;
extern T_HD	hp;

#define	ishNOP		0
#define	ishDECODE	1
#define	ishENCODE	2
#define	ishLIST		3
#define	RBSIZE		256

static void
usage()
{
	Mprint( "\
aish v1.13 ish/uuencode converter 1993,94,95,98 H.Ogasawara (COR.)\r\n\
usage: aish [-pfadhAlUo<file>x<file>] <decode text> ..\r\n\
       aish -{s78nu} [-iadco<file>x<file>O<n>t<n>] <encode file> ..\r\n\
       aish -m <encode file> ..   (MIME decode)\
\r\n"
		);
}

unsigned char *
getword( str, ptr )
unsigned char	*str, *ptr;
{
	for(; *ptr == ' ' || *ptr == '\t' ; ptr++ );
	for(; *ptr && *ptr != ' ' && *ptr != '\t' ; *str++= *ptr++ );
	*str= '\0';
	return	ptr;
}

#if PREREAD
static int	PreFlag= FALSE;
static unsigned char	PreBuf[RBSIZE];
static
MMgets( fm, ptr, len )
m_file	*fm;
unsigned char	*ptr;
{
	if( PreFlag ){
		PreFlag= FALSE;
		return	(unsigned char*)p_strcpy( ptr, PreBuf )-ptr;
	}
	return	Mgets( fm, ptr, len );
}
unsigned char *
PreRead()
{
	Mgets( &fm, PreBuf, RBSIZE-8 );
	PreFlag= TRUE;
	return	PreBuf;
}
#endif

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
	int		encode= ishNOP;
	memclr( sizeof(T_HD), &hp );
	hp.stepline= 200;
#if UNIX
	hp.mode= mAPPEND;
#else
	hp.mode= 0;
#endif
#if SUPASM
	for(; arg= getword( ptr, arg ), *ptr ;)
#else
	for(; --argc && (p_strcpy( ptr, *++argv ),1) ;)
#endif
	{
		if( encode == ishNOP )
			encode= ishDECODE;
		if( *ptr == '-' && ptr[1] ){
			unsigned char	*p= ptr+1;
			for(; *p ; p++ ) switch( *p ){
			case 'l':
				hp.mode|= mLIST;
				encode= ishLIST;
				break;
			case 'r':
				encode= ishDECODE;
				break;
			case '7':
				hp.ztype= zJIS7;
				encode= ishENCODE;
				break;
			case '8':
				hp.ztype= zJIS8;
				encode= ishENCODE;
				break;
			case 's':
				hp.ztype= zSJIS;
				encode= ishENCODE;
				break;
			case 'n':
				hp.ztype= zSJISN;
				encode= ishENCODE;
				break;
			case 'u':
				hp.ztype= zUUE;
				encode= ishENCODE;
				break;
			case 't':
				if( p[1] ){
					hp.stepline= numstr( p+1 );
				}else{
#if SUPASM
					arg= getword( ptr, arg );
					hp.stepline= numstr( ptr );
#else
					if( argc >= 2 ){
						argc--;
						hp.stepline= numstr( *++argv );
					}
#endif
				}
				if( hp.stepline <= 2 )
					hp.stepline= -1;
				goto	_forbreak;
			case '_':
				hp.volsize= numstr( p+1 );
				goto	_forbreak;
			case '/':
				hp.tcrc= numstr( p+1 );
				goto	_forbreak;
			case '-':
				hp.tcrc32= numstr( p+1 );
				goto	_forbreak;
#if NOERR
			case '+':
				{
				extern noerrflag;
				noerrflag= TRUE;
				}
				break;
#endif
			case 'O':
				if( p[1] ){
					int	i= numstr( p+1 );
					hp.os= i;
					hp.vol= i>>16;
				}else{
					int	i;
#if SUPASM
					arg= getword( ptr, arg );
					i= numstr( ptr );
#else
					if( argc >= 2 ){
						argc--;
						i= numstr( *++argv );
					}
					hp.os= i;
					hp.vol= i>>16;
#endif
				}
				goto	_forbreak;
			case 'p':
				hp.mode|= mPRTOUT;
				break;
			case 'i':
				hp.mode|= mNOTSUM;
				break;
			case 'A':
				hp.mode|= mADJNAME;
				break;
			case 'a':
				hp.mode|= mAPPEND;
				break;
			case 'd':
				hp.mode&= ~mAPPEND;
				break;
			case 'U':
			case 'c':
				hp.mode|= mUUENC;	/* mUUCHK */
				break;
			case 'f':
				hp.mode|= mOWRITE;
				break;
			case 'h':
				hp.mode|= mFHEAD;
				break;
			case 'm':
				hp.mode|= mMIME;
				break;
			case 'x':
				if( p[1] )
					p_strcpy( hp.skipname, p+1 );
				else
#if SUPASM
					arg= getword( hp.skipname, arg );
#else
					if( argc >= 2 ){
						argc--;
						p_strcpy( hp.skipname, *++argv );
					}
#endif
				if( *hp.skipname )
					hp.mode|= mSKIP;
				goto	_forbreak;
			case 'o':
				hp.mode|= mOUTPUT;
				if( p[1] )
					p_strcpy( hp.outname, p+1 );
				else
#if SUPASM
					arg= getword( hp.outname, arg );
#else
					if( argc >= 2 ){
						argc--;
						p_strcpy( hp.outname,*++argv);
					}
#endif
				if( *hp.outname )
					goto	_forbreak;
			default:
				usage();
#if SUPASM
				return;
#else
				exit( 1 );
#endif
			}
		  _forbreak:;
		}else if( encode == ishDECODE ){
			if( Mopen( &fm, ptr ) ){
				unsigned int	len, nodec= TRUE;
#if MIMEDEC
				if( hp.mode & mMIME ){
					MimeDec( &fm );
				}else
#endif
				{
				for(; len= MMgets( &fm, ptr, RBSIZE-8 ) ;){
					if( (hp.mode & mPRTOUT) && nodec ){
						Mprint( ptr ),
						Mprint( "\r\n" );
					}
					if( !--len )
						continue;
					nodec= ai_decode( ptr, len );
				}
				if( !nodec )
					ai_decode_lastchance();
				}
				Mclose( &fm );
			}else
				Merr( "open error\r\n" );
		}else if( encode == ishLIST ){
			if( Mopen( &fm, ptr ) ){
				unsigned int	len, line;
				for( line= 1 ; len= Mgets( &fm, ptr, RBSIZE-8 ) ; line++ ){
					if( checkhead( &hp, ptr, len-1 ) ){
						Numput( line );
						Mprint( "\r\n" );
					}
				}
				Mclose( &fm );
			}else
				Merr( "open error\r\n" );
		}else{
			p_strcpy( hp.fname, ptr );
			hp.initflag= TRUE;
			while( encblock( &hp ) == TRUE );
		}
	}
	if( encode == ishNOP )
		usage();
#if !SUPASM
	exit( 0 );
#endif
}
