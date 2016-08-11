/* Copyright IBM Corporation 1988,1991 - All Rights Reserved */

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
#include <hp700_90/system.h>
        SYSTEM_H_FILE = hp700_90/system.h
#undef In_Imake

/* These next three lines help configure the embedded machine-dependent
    directories overhead/class/machdep, atk/console/stats, and
    atk/console/stats/common. */
        SYS_IDENT = hp700
        SYS_OS_ARCH = hp_pa_risc_90
       SYS_CONFDIR = hp700_90

/* Get parent inclusions */
#include <allsys.mcr>

DYNLDFLAGS=$(RESOLVLIB) $(SYS_LIBRARIES) \
	$(SYSAUX_LIBRARIES) \
	$(LOCAL_SHARED_LIB_PATH) \
	$(SHARED_LIB_PATH) \
	$(DYN_LINK_LIB) 

NORELS=$(CPPSTDLIB)
CPPDYNFLAGS=-Wl,-c,$(BASEDIR)/lib/atkhp/libClist $(CPPSTDLIB) -L$(BASEDIR)/lib/atkhp 
POSTDYNFLAGS = -L$(BASEDIR)/lib/atkhp/dyn   $(GCCLIB) -lc
CPPLDFLAGS= $(CPPSTDLIB) -L$(BASEDIR)/lib/atkhp $(LDFLAGS)
ATKLIBLDARGS=$(SYS_LIBRARIES) \
        $(SYSAUX_LIBRARIES) \
        $(GCCLIB)  -lc \
        $(LOCAL_SHARED_LIB_PATH) \
        $(SHARED_LIB_PATH)   \
	$(DYN_LINK_LIB) 

/* Redefine for HP-UX 8.0 */
        XLIBDIR = /usr/lib/X11R5
        XINCDIR = /usr/include/X11R5
        XFC = /usr/bin/X11/bdftopcf
        XBINDIR = /usr/bin/X11
        XUTILDIR = /usr/bin/X11

/* Now for the system-dependent information. */
  
        MALLOCALIGNMENT = 8
        CDEBUGFLAGS = 
        SHLIBLDFLAGS= -b -E

#ifdef USE_SHARED_LIBS
ATKLIBFLAGS=-s
PICFLAG= +Z
ECOMPFLAGS= -Wl,+s -Wl,+b,\: -Wl,-E
CPPSTDLIB = /usr/lib/libC.a
#ifdef HAVE_DYNAMIC_LOADING
PREDYNFLAGS= -b
TMPDYNEXT=$(DYNEXT)
MKDYNMAIN = $(BASEDIR)/etc/mkdynmain - ATK_EntryFunction

ATKDYNEXTM=sl
ATKTMPDYNEXTM=sl
ATKLIBEXTM=sl
ATKDYNPREFIXM=lib
#endif
#else
ATKLIBFLAGS=
PICFLAG=
ECOMPFLAGS=
CPPSTDLIB = /usr/lib/libC.a
#endif

LINKPREFIX = $(BASEDIR)/etc/relativize $(LOCALNORELS) $(NORELS) ---
        COMPILERFLAGS =$(ECOMPFLAGS) -Aa -D_HPUX_SOURCE -Dhpux -Dunix $(PICFLAG)

        SYS_LIBRARIES = $(DESTDIR)/lib/libatkos.a $(DESTDIR)/lib/libossup.a
DYN_LINK_LIB = -ldld

DYNMATHLIB=
LIBMATHLIB=
MATHLIB = -L/lib -lm

CC=cc
CPPC = /usr/bin/CC  -D__STDC__ +T
CDEBUGFLAGS=
YACC=yacc

/* Get site-specific inclusions */

#include <site.mcr>

