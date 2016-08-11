/* Copyright IBM Corporation 1988,1991 - All Rights Reserved */
MFLAGS=
/*
	$Disclaimer: 
# Permission to use, copy, modify, and distribute this software and its 
# documentation for any purpose and without fee is hereby granted, provided 
# that the above copyright notice appear in all copies and that both that 
# copyright notice and this permission notice appear in supporting 
# documentation, and that the name of IBM not be used in advertising or 
# publicity pertaining to distribution of the software without specific, 
# written prior permission. 
#                         
# THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
# TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
# HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
# DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
# DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
# OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
# WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
# 
#  $
*/

/* For full copyright information see:'andrew/config/COPYRITE' */

#define In_Imake 1
/* The next two lines need to be kept in sync */
#include <sun4_5x/system.h>
        SYSTEM_H_FILE = sun4_5x/system.h
#undef In_Imake
SYS_CONFDIR = sun4_5x

/* These next three lines help configure the embedded machine-dependent
    directories overhead/class/machdep, atk/console/stats, and
    atk/console/stats/common. */
        SYS_IDENT = sun4_5x
        SYS_OS_ARCH = sun4_5x

/* Get parent inclusions */
#include <allsys.mcr>

OSSUPPORT= ossupport
        XLIBDIR = /usr/openwin/lib
        XINCDIR = /usr/openwin/include
        XFC = /usr/openwin/bin/bdftopcf
        XBINDIR = /usr/openwin/bin
        XUTILDIR = /usr/openwin/bin
	XMKFONTDIR = $(XUTILDIR)/mkfontdir
/* Now for the system-dependent information. */
  
        MALLOCALIGNMENT = 8
        CDEBUGFLAGS = 

DYNLINKPROG = $(LINKPREFIX) gcc -shared
POSTDYNFLAGS=
PREDYNFLAGS=
DYN_LINK_LIB= -ldl
MKDYNMAIN=$(BASEDIR)/etc/mkdynmain -ctor
ATKLIBEXTM =so
ATKDYNEXTM=so
ATKDYNPREFIXM=lib
ATKTMPDYNEXTM=so

ATKLIBFLAGS = -s
PICFLAG=-fPIC
RANLIB=true
AR = /usr/ccs/bin/ar cq
NM = /usr/ccs/bin/nm
YACC = /usr/ccs/bin/yacc
LEX = /usr/ccs/bin/lex
MAKE = /usr/ccs/bin/make
CPP = /usr/ccs/lib/cpp
MAKEFLAGS = S
RESOLVLIB = -lresolv
NETLIBS	= -lsocket -lnsl -lelf
XMAKEDEPEND=$(BASEDIR)/etc/gccmakedep
COMPILERFLAGS= $(ECOMPFLAGS) $(PICFLAG)
LINKPREFIX = $(BASEDIR)/etc/relativize $(LOCALNORELS) $(NORELS) ---

        SYS_LIBRARIES = $(NETLIBS) $(BASEDIR)/lib/libatkos.a $(BASEDIR)/lib/libossup.a 
AFSUTILS=
ATKLIBLIBS= $(NETLIBS) $(BASEDIR)/lib/libatkos.a $(BASEDIR)/lib/libossup.a
REALDESLIB = $(AFSBASEDIR)/lib/libafs_des.a
CPPC = g++
CC= gcc
DYNLDFLAGS = $(RESOLVLIB) $(NETLIBS) $(BASEDIR)/lib/libatkos.a $(BASEDIR)/lib/libossup.a  \
	$(SYSAUX_LIBRARIES) \
	$(SHARED_LIB_PATH) \
	$(DYN_LINK_LIB)
SHARED_LIB_PATH=-Wl,-R,DEFAULT_ANDREWDIR_ENV/lib/atk:$(DESTDIR)/lib/atk:$(BASEDIR)/lib/atk
ATKLIBLDARGS=
/* Get site-specific inclusions */
#include <site.mcr>
