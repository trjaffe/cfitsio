# Generated automatically from Makefile.in by configure.
#
# Makefile for cfitsio library:
#       libcfits.a
#
#       JDD
#       NASA GSFC
#       March, 1996
#

SHELL = /bin/sh
RANLIB=echo ranlib
CC=cc


SOURCES = 	cfileio.c convert.c fitscore.c getcol.c \
		getkey.c modkey.c putcol.c putkey.c \
		utilproc.c 

all:		stand_alone clean

stand_alone:	libcfitsio.a

libcfitsio.a:	psplit
		@mkdir split.$$$$; \
		cp ${SOURCES} split.$$$$; \
		cp *.h split.$$$$; \
		cd split.$$$$ ; \
		../psplit ${SOURCES} ; \
		rm -f ${SOURCES} ; \
		echo "cfitsio: compiling split c files..."; \
		${CC} -c ${CFLAGS} *.c; \
		ar rv libcfitsio.a *.o; \
		${RANLIB} libcfitsio.a; \
		mv libcfitsio.a .. ;

install:	libcfitsio.a
		mv libcfitsio.a ${FTOOLS_LIB}
		cp *.h ../include/

clean:
		rm -f *.o ; \
		rm -rf split.*

distclean:	clean
		rm -f psplit ; \
		rm -f Makefile config.*

psplit :
		${CC} ${CFLAGS} -o psplit psplit.c

.c.o:
		${CC} ${CFLAGS} -c -o $@ $*.c
