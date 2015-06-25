# aish/koish 1993,94,95 by H.Ogasawara (COR.)

# install directory
DSTDIR	= /usr/local/bin

#### *** UNIX
CC	= gcc
LK	= $(CC)
CFLAGS	= -O -DUNIX=1 -DUUENCODE=1 -DMIMEDEC=1
#CFLAGS	= -O -DUNIX=1 -DUUENCODE=1
SYSOBJ	= ai_unix.o
SYSCHK	= syschk.h
####
#### ***  Human68k
#CC	= gcc -m68000
#LK	= lk
#AS	= as
#CFLAGS	= -O -fcombine-regs -fall-bsr -DHUMAN68K=1 -DUUENCODE=1 -DMIMEDEC=1
#STARTUP = startup.o
#AIOBJAS = ai_sjis.o ai_sub.o
#LIB	= gnulib.a iocslib.l
#MLIB	= gnulib.a iocslib.l doslib.l
####
#### *** Human68k all C
#CC	= gcc -m68000
#LK	= lk
#CFLAGS	= -O -fcombine-regs -fall-bsr -DHUMAN68K2=1 -DUUENCODE=1
#SYSOBJ	= ai_unix.o
#LIB	= gnulib.a clib.l doslib.l iocslib.l floatfnc.l
#MLIB	= $(LIB)
####
#### *** Human options
GCC_OPTION=LFIOAM
SILK	=-x -z -l
HAS	=*-u -w
#### *****************
####
MVFLAG	= -DMVOLENC=1
AIOBJ	= ai_crc.o ai_blk2.o ai_fio.o ai_fcrc.o ai_7.o ai_8.o ai_s.o ai_n.o \
	ai_hed.o ai_ish.o ai_blk.o ai_nam.o aish.o ai_uue.o mimedec.o
OBJ	= $(STARTUP) $(AIOBJ) $(AIOBJAS) $(SYSOBJ)
MOBJ	= $(STARTUP) aishmv.o ai_crc32.o ai_crcmv.o ai_fiomv.o ai_nam.o \
	$(SYSOBJ)
KOBJ	= koish.o $(AIOBJ)
KLIB	= clib.a gnulib.a
PROG	= aish
MPROG	= aishmv
KPROG	= koishenc.win
####

all:	$(SYSCHK) $(PROG) $(MPROG)

$(PROG): $(OBJ)
	$(LK) $(OBJ) -o $(PROG) $(LIB)

$(MPROG): $(MOBJ)
	$(LK) $(MOBJ) -o $(MPROG) $(MLIB)

$(KPROG): $(KOBJ)
	$(LK) $(KOBJ) -o $(KPROG) $(KLIB)

aish.o:		aish.c ai.h microfio.h config.h
ai_ish.o:	ai_ish.c ai.h microfio.h config.h
ai_fio.o:	ai_fio.c microfio.h config.h
ai_fcrc.o:	ai_fcrc.c microfio.h config.h
ai_hed.o:	ai_hed.c ai.h microfio.h config.h
ai_blk.o:	ai_blk.c ai.h microfio.h config.h
ai_blk2.o:	ai_blk2.c ai.h microfio.h config.h
ai_s.o:		ai_s.c config.h
ai_crc.o:	ai_crc.c config.h
ai_nam.o:	ai_nam.c config.h
ai_uue.o:	ai_uue.c ai.h config.h microfio.h
koish.o:	koish.c ai.h microfio.h config.h
aishmv.o:	aishmv.c config.h microfio.h
	$(CC) $(CFLAGS) $(MVFLAG) -c aishmv.c
ai_crc32.o:	ai_crc32.c config.h
	$(CC) $(CFLAGS) $(MVFLAG) -c ai_crc32.c
##--NEWC-b
ai_crcmv.o:	ai_crc.c config.h
	$(CC) $(CFLAGS) $(MVFLAG) -c ai_crc.c -o $@
ai_fiomv.o:	ai_fio.c config.h microfio.h
	$(CC) $(CFLAGS) $(MVFLAG) -c ai_fio.c -o $@
##--NEWC-e
##--OLDC-b
#ai_crcmv.o:	ai_crc.c config.h
#	ln ai_crc.c ai_crcmv.c
#	$(CC) $(CFLAGS) $(MVFLAG) -c ai_crcmv.c
#	rm ai_crcmv.c
#ai_fiomv.o:	ai_fio.c config.h microfio.h
#	ln ai_fio.c ai_fiomv.c
#	$(CC) $(CFLAGS) $(MVFLAG) -c ai_fiomv.c
#	rm ai_fiomv.c
##--OLDC-e

syschk.h:
	@sh -c "(if [ -r /usr/include/strings.h ] ;\
	then echo '#define BZ_BSD' > $@;else echo '#define BZ_SYSV' > $@;fi)"
	@sh -c "(if grep tm_gmtoff /usr/include/time.h > /dev/null ;\
		then echo '#define BSD_TM' >> $@;\
	elif grep tz_minuteswest /usr/include/time.h > /dev/null ;\
		then echo '#define BSD' >> $@;\
	elif grep 'timezone;' /usr/include/time.h > /dev/null ;\
		then echo '#define SYSV' >> $@;\
	else echo '#define BFTIME' >> $@;fi)"

#### archives
lha:
	lha a d:/aish.lzh aish.x aishmv.x aish.doc
TARFILE= README.doc aish.doc Makefile ishcat \
	ai_7.c ai_8.c ai_blk.c ai_blk2.c ai_crc.c ai_fio.c ai_hed.c \
	ai_ish.c ai_n.c ai_nam.c ai_s.c ai_unix.c aish.c ai_crc32.c \
	ai_fcrc.c ai_uue.c ai.h config.h microfio.h aishmv.c mimedec.c
TARFILE2= startup.s ai_sjis.s ai_sub.s
tar:
	tar -cvf d:/aish.tar $(TARFILE)
lhh:
	lha a d:/ai.lzh $(TARFILE)
lhh2:
	lha f d:/ai.lzh $(TARFILE)
pack:
	lha a d:/ai.lzh $(TARFILE) *.s
#	lha a d:/ai $(TARFILE) $(TARFILE2)
#	rm -f aishpack.tar.gz
#	tar -cvf aishpack.tar $(TARFILE) $(TARFILE2)
#	gzip -9 aishpack.tar
####
clean:
	rm -f *.o $(SYSCHK) $(PROG) $(MPROG) core *~
install: $(PROG) $(MPROG)
	install -c -s $(PROG) $(DSTDIR)
	install -c -s $(MPROG) $(DSTDIR)
	install -c -m 0755 ishcat $(DSTDIR)
oldc:
	sed '/^##--OLDC-b/,/^##--OLDC-e/s/^#//' Makefile | \
	sed '/^##--NEWC-b/,/^##--NEWC-e/s/.*/#&/' > aisholdc.mk
	make -f aisholdc.mk
	rm -f aisholdc.mk
####
%.o::%.c
	$(CC) $(CFLAGS) -c $<
%.o::%.s
	$(AS) $<
####
