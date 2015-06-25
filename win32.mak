# aish/koish 1993,94,95 by H.Ogasawara (COR.)

#### *** Win32
CC	= cl
LK	= cl
CFLAGS	= -O -DWIN32=1 -DUUENCODE=1 -DMIMEDEC=1
SYSOBJ	= ai_win32.obj
####
#### *****************
####
MVFLAG	= -DMVOLENC=1
AIOBJ	= ai_crc.obj ai_blk2.obj ai_fio.obj ai_fcrc.obj ai_7.obj ai_8.obj \
	ai_s.obj ai_n.obj \
	ai_hed.obj ai_ish.obj ai_blk.obj ai_nam.obj aish.obj ai_uue.obj \
	mimedec.obj
OBJ	= $(STARTUP) $(AIOBJ) $(AIOBJAS) $(SYSOBJ)
MOBJ	= $(STARTUP) aishmv.obj ai_crc32.obj ai_crcmv.obj ai_fiomv.obj \
	ai_nam.obj \
	$(SYSOBJ)
PROG	= aish
MPROG	= aishmv
####

all:	$(SYSCHK) $(PROG) $(MPROG)

$(PROG): $(OBJ)
	$(LK) $(OBJ) -o $(PROG)

$(MPROG): $(MOBJ)
	$(LK) $(MOBJ) -o $(MPROG)


aish.obj:		aish.c ai.h microfio.h config.h
ai_ish.obj:	ai_ish.c ai.h microfio.h config.h
ai_fio.obj:	ai_fio.c microfio.h config.h
ai_fcrc.obj:	ai_fcrc.c microfio.h config.h
ai_hed.obj:	ai_hed.c ai.h microfio.h config.h
ai_blk.obj:	ai_blk.c ai.h microfio.h config.h
ai_blk2.obj:	ai_blk2.c ai.h microfio.h config.h
ai_s.obj:		ai_s.c config.h
ai_crc.obj:	ai_crc.c config.h
ai_nam.obj:	ai_nam.c config.h
ai_uue.obj:	ai_uue.c ai.h config.h microfio.h
koish.obj:	koish.c ai.h microfio.h config.h
aishmv.obj:	aishmv.c config.h microfio.h
	$(CC) $(CFLAGS) $(MVFLAG) -c aishmv.c
ai_crc32.obj:	ai_crc32.c config.h
	$(CC) $(CFLAGS) $(MVFLAG) -c ai_crc32.c

ai_crcmv.c: ai_crc.c
	copy ai_crc.c ai_crcmv.c
ai_fiomv.c: ai_fio.c
	copy ai_fio.c ai_fiomv.c

ai_crcmv.obj:	ai_crcmv.c config.h
	$(CC) -o ai_crcmv.obj $(CFLAGS) $(MVFLAG) -c ai_crcmv.c
ai_fiomv.obj:	ai_fiomv.c config.h microfio.h
	$(CC) -o ai_crcmv.obj $(CFLAGS) $(MVFLAG) -c ai_fiomv.c

