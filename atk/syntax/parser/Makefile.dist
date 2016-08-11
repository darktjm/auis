# parser - generate a C++ object declaration from a grammar file
#		(requires AUIS Bison)

# targets:  
#	all - builds libparser.a
#	install - installs four files noted below
#	testparser - builds a rudimentary test
#	clean - deletes non-source files	

#  	To configure for your system 
#
#	Specify the installation tree,
DESTDIR = /usr/local
# 	the C++ compiler,
CXX = g++	
#	and the location of the AUIS Bison
BISON = bison
#	Add compiler flags here,
COMPILERFLAGS =
#	specify debug mode (often -O or -g),
CDEBUGFLAGS = 
#	specify additional includes,
INCLUDES = -I. 
#	add any defines desired,
DEFINES =
#	and give flags for CXX when used for ld.
LOADFLAGS =
#	flag -DYYDEBUG if parse debugging wanted
YYFLAG =

        CPPFLAGS = $(COMPILERFLAGS) $(CDEBUGFLAGS) $(INCLUDES) $(DEFINES)  $(YYFLAG)

.SUFFIXES:
.SUFFIXES:  .o .C 

.C.o:
	rm -f $@
	$(CXX) $(CPPFLAGS) -c $*.C


all: libparser.a

libparser.a: parser.o
	ar r $@ $?
	ranlib $@


INSTALLFILES =  ${DESTDIR}/bin/mkparserclass ${DESTDIR}/include/parser.H ${DESTDIR}/lib/libparser.a ${DESTDIR}/doc/prsclass.doc  ${DESTDIR}/help/atkparse.doc

install:: ${INSTALLFILES}
${DESTDIR}/bin/mkparserclass:: mkparserclass
	rm -f $@
	cp  $? $@
	chmod a+x $@
${DESTDIR}/include/parser.H:: parser.H
	rm -f $@
	cp  $? $@
${DESTDIR}/lib/libparser.a:: libparser.a
	rm -f $@
	cp  $? $@
	ranlib $@
${DESTDIR}/doc/prsclass.doc:: prsclass.asc
	rm -f $@
	cp  $? $@
${DESTDIR}/help/atkparse.doc:: parse.asc
	rm -f $@
	cp  $? $@
install:: 
	chmod a-w ${INSTALLFILES}
	touch install

#
#rudimentary test:  make testparser;  ./testparser
#
foo.H foo.C: foo.gra
	${BISON} -b foo -k foo.gra
	./mkparserclass foo
TESTOBJS =  foo.o testparser.o parser.o
testparser.o: foo.H
testparser: ${TESTOBJS} Makefile mkparserclass
	$(CXX) $(CDEBUGFLAGS) ${LOADFLAGS} -o testparser ${TESTOBJS}


clean: 
	rm -f \#* ,* *~ *.CKP *.BAK *.bas errs core
	rm -f *.ln *.o *.dog tags TAGS make.log
	rm -f *.do *.eh *.ih *.a *.fwm *.snf *.pcf *.rtx *.fb
	rm -f install.time install.doc
	rm -f testparser
	rm -f foo.C foo.H foo.act *.tab.* *.output
