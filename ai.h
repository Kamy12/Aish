/*	Copyright 1993,94 H.Ogasawara (COR.)	*/

#include	"config.h"

#if HUMAN68K
#  if SYSDOSLIB
#    include	<sys_doslib.h>
#  else
#    include	<doslib.h>
#  endif
#endif

#define	NAMES	(PATHLEN*2+NAMELEN*2)

typedef struct {
		unsigned int	(*exec)();
		unsigned int	fsize,		/* file size counter */
				byte,		/* byte length of line */
				body,		/* data length of line */
				blockbyte,	/* block size */
				dline,
				cline,
				nextline,
				stepline,
				initflag,
				vol,
				volsize,
				mode,
				date,
				tcrc32,
				ztype,
				uulength;
		int		block;		/* total block counter */
		unsigned short	tcrc;
		short		perm,
				os;
		unsigned char	*sf_tbl;
		unsigned char	fname[PATHLEN],
				adjname[NAMELEN],
				outname[PATHLEN],
				skipname[NAMELEN];
	} T_HD;

typedef struct {
		unsigned int	(*decexec)(),
				(*encexec)();
		unsigned char	*sf_tbl,
				name[8],
				byte, code, sum;
	}	T_Z;

extern T_Z	ztype[];

#define	FALSE	0
#define	TRUE	1
#define	EOISH	2
#if ERRCONT
# define ESKIP	3
#else
# define ESKIP	TRUE
#endif
#define	UURETRY	5

#define	zJIS7	0
#define	zJIS8	1
#define	zSJIS	2
#define	zSJISN	3
#define	zUUE	4

#define	DBUF	80

/* flags */
#define	mNOTSUM		1
#define	mPRTOUT		2
#define	mADJNAME	4
#define	mOUTPUT		8
#define	mAPPEND		16
#define	mLIST		32
#define	mSKIP		64
#define	mUUENC		128
#define	mUUCHK		128
#define	mNOERR		256
#define	mOWRITE		512
#define	mFHEAD		1024
#define	mMIME		2048

