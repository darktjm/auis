# cparser
#		AUIS parser

#  To configure for your system, specify the installation tree:
DESTDIR = /usr/local

BISON=${DESTDIR}/bin/bison

# To provide for debugging symbol tables remove # on next line
#CDEBUGFLAGS = -g

# To debug parsing, remove the # from the next line.
#YYFLAG= -DYYDEBUG

# Memcopy
# The code uses bcopy().  If your system instead supplies memcpy,
# you need to remove the # from the next line.
#MEMFLAG = -DUSEMEMCPY 

CFLAGS = -Dvoid=int  -I.   \
		${CDEBUGFLAGS} ${YYFLAG}  ${MEMFLAG}


libcparser.a: cparser.o
	ar r libcparser.a cparser.o
	ranlib libcparser.a


INSTALLFILES =  ${DESTDIR}/bin/mkparser ${DESTDIR}/include/cparser.h ${DESTDIR}/lib/libcparser.a ${DESTDIR}/doc/parser.doc

install::  ${INSTALLFILES}
${DESTDIR}/bin/mkparser :: mkparser
	rm -f $@
	cp  $? $@
	chmod a+x $@
${DESTDIR}/include/cparser.h :: cparser.h
	rm -f $@
	cp  $? $@
${DESTDIR}/lib/libcparser.a :: libcparser.a
	rm -f $@
	cp  $? $@
	ranlib $@
${DESTDIR}/doc/parser.doc :: parser.asc
	rm -f $@
	cp  $? $@
install:: 
	chmod a-w ${INSTALLFILES}
	touch install


TESTOBJS =  foo.o testparse.o cparser.o
foo.h foo.c: foo.y
	${BISON} -k foo.y
	./mkparser foo
testparse: foo.h ${TESTOBJS}
	cc -o testparse ${TESTOBJS}


clean: 
	rm -f \#* ,* *~ *.CKP *.BAK *.bas errs core
	rm -f *.ln *.o *.dog tags TAGS make.log
	rm -f *.do *.eh *.ih *.a *.fwm *.snf *.pcf *.rtx *.fb
	rm -f install.time install.doc
	rm -f testparse
	rm -f foo.c foo.h foo.act foo.tab.c
