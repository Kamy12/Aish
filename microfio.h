/*	Copyright 1993 H.Ogasawara (COR.)	*/

#if HUMANFIO

#define	Mprint(a)	PRINT(a)
#define	Mputchar(a)	PUTCHAR(a)
#define	unlink(a)	DELETE(a)
#if MVOLENC
# define Mseek(a,b,c)	SEEK((a)->fn,b,c)
#else
# define Merr(a)	MerrAsm(a)
# define Merrch(a)	MerrchAsm(a)
#endif
#define	MFBUFSIZE	4096

typedef	struct {
		int	fn;
		unsigned char	*ptr;
		unsigned char	*limit;
		unsigned char	buf[MFBUFSIZE];
	} m_file;

#else

#include	<stdio.h>
typedef	struct {
		FILE	*fp;
		int	fn;
	} m_file;

#endif

#define	UFLUSH()

