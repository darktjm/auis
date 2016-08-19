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
#include <ix86_Linux/system.h>
        SYSTEM_H_FILE = ix86_Linux/system.h
#undef In_Imake
SYS_CONFDIR = ix86_Linux

/* These next three lines help configure the embedded machine-dependent
    directories overhead/class/machdep, atk/console/stats, and
    atk/console/stats/common. */
        SYS_IDENT = ix86_Linux
        SYS_OS_ARCH = ix86_Linux

/* Get parent inclusions */
#include <allsys.mcr>

OSSUPPORT= ossupport
        XLIBDIR = /usr/lib
        XINCDIR = /usr/include
        XFC = /usr/bin/bdftopcf
        XBINDIR = /usr/bin
        XUTILDIR = /usr/bin
	XMKFONTDIR = $(XUTILDIR)/mkfontdir
/* Now for the system-dependent information. */
  
        MALLOCALIGNMENT = 8
        CDEBUGFLAGS = 
XMAKEDEPEND=$(XUTILDIR)/gccmakedep

#ifndef USE_ELF
DYNLINKPROG = CLASSPATH=$(LOCALCLASSPATH)$(BASEDIR)/lib/atk;export CLASSPATH;$(BASEDIR)/etc/adynlink $(CPPC)
PREDYNFLAGS= -Wl,-e,_main,-r,-x -nostdlib
POSTDYNFLAGS= -lg++ -lc -lgcc -lc
AEXELINK=$(BASEDIR)/etc/aexelink
TMPDYNEXT=-
SYS_DYNDIR=ix86_LinuxAout
#else /* USE_ELF */
DYNLINKPROG = $(LINKPREFIX) gcc -shared
POSTDYNFLAGS=
PREDYNFLAGS=
DYN_LINK_LIB=-ldl
MKDYNMAIN=$(BASEDIR)/etc/mkdynmain -ctor
ATKLIBEXTM =so
ATKDYNEXTM=so
ATKDYNPREFIXM=lib
ATKTMPDYNEXTM=so
#endif /* USE_ELF */

#ifndef USE_ELF
ATKLIBFLAGS = -d '$(BASEDIR)/etc/alibexp'
#else /* USE_ELF */
ATKLIBFLAGS = -s
#endif /* USE_ELF */
PICFLAG=
ECOMPFLAGS=

/* jpeg & tiff are too old; use included versions */
/* also, libutil seems to be getting picked up somewhere else */
NORELS=libjpeg.a libtiff.a libutil.a
LINKPREFIX = $(BASEDIR)/etc/relativize $(LOCALNORELS) $(NORELS) ---

        SYS_LIBRARIES = $(DESTDIR)/lib/libatkos.a

CPPC = g++

LEXLIB = -lfl
LEX = flex
SHARED_LIB_PATH=-Wl,-R,DEFAULT_ANDREWDIR_ENV/lib/atk:$(DESTDIR)/lib/atk:$(BASEDIR)/lib/atk
ATKLIBLDARGS=$(CPPSTDLIB) $(GCCLIB) $(SHARED_LIB_PATH)
#define ConstructMFLAGS 1
/* Get site-specific inclusions */
#include <site.mcr>
