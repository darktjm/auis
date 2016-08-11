

all::  config/site.h config/site.mcr
	-mv Makefile Makefile,
	-rm -f Makefile
	imake  -I. -I./config -Timake.tmpl  -f Amakefile 
	-rm -f Makefile,

Makefile::
	@echo
	@echo USE  'make' WITHOUT ANY TARGET TO MAKE 'Makefile'
	@echo
	@echo Ignore the following message:
Makefile:: "Makefile".yet

Makefiles:: all
	make Makefiles
depend:: all
	make depend
doc:: all
	make doc
Doc:: all
	make Doc
aliases:: all
	make aliases
Aliases:: all
	make Aliases
install:: all
	make install
Install:: all
	make Install
dependInstall:: all
	make dependInstall
world:: all
	make world 
World:: all
	make World
tidy:: all
	make tidy
Tidy:: all
	make Tidy
clean:: all
	make clean
Clean:: all
	make Clean
