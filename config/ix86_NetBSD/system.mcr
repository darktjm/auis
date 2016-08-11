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
#include <ix86_NetBSD/system.h>
        SYSTEM_H_FILE = ix86_NetBSD/system.h
#undef In_Imake
SYS_CONFDIR = ix86_NetBSD

/* These next three lines help configure the embedded machine-dependent
    directories overhead/class/machdep, atk/console/stats, and
    atk/console/stats/common. */
        SYS_IDENT = ix86_NetBSD
        SYS_OS_ARCH = ix86_NetBSD

/* Get parent inclusions */
#include <allsys.mcr>

OSSUPPORT= ossupport
        XLIBDIR = /usr/X11R6/lib
        XINCDIR = /usr/X11R6/include
        XFC = /usr/X11R6/bin/bdftopcf
        XBINDIR = /usr/X11R6/bin
        XUTILDIR = /usr/X11R6/bin
        XMKFONTDIR = $(XUTILDIR)/mkfontdir
/* Now for the system-dependent information. */
  
        MALLOCALIGNMENT = 8
        CDEBUGFLAGS = 
XMAKEDEPEND=$(XUTILDIR)/gccmakedep

DYNLINKPROG = $(LINKPREFIX) ld -Bshareable 
POSTDYNFLAGS=
PREDYNFLAGS=
DYN_LINK_LIB=
MKDYNMAIN=$(BASEDIR)/etc/mkdynmain -ctor
ATKLIBEXTM =so.1.0
ATKDYNEXTM=so.1.0
ATKDYNPREFIXM=lib
ATKTMPDYNEXTM=so.1.0

ATKLIBFLAGS = -s
COMPILERFLAGS=-fpic

LINKPREFIX = $(BASEDIR)/etc/relativize $(LOCALNORELS) $(NORELS) ---

        SYS_LIBRARIES = $(DESTDIR)/lib/libatkos.a

CPPC = g++

LEXLIB = -lfl
LEX = flex
BUILTINSHLIBPATH=DEFAULT_ANDREWDIR_ENV/lib/atk:$(DESTDIR)/lib/atk:$(BASEDIR)/lib/atk
SHARED_LIB_PATH=-Wl,-R,$(BUILTINSHLIBPATH)
ATKLIBLDARGS=$(CPPSTDLIB) $(GCCLIB) -R $(BUILTINSHLIBPATH) 
DYNLDFLAGS=$(ALLAFSOBJFILE) $(RESOLVLIB) $(SYS_LIBRARIES) \
        $(SYSAUX_LIBRARIES) \
        $(LOCAL_SHARED_LIB_PATH) \
        -R $(BUILTINSHLIBPATH) \
        $(DYN_LINK_LIB)
#define PreDynFlags(dobj,extraclasses,objs,libs,syslibs) $(PREDYNFLAGS)

EXTRAWWWCONF=;echo \\#undef HAVE_TIMEZONE>>config.h
BSDTTYLIB=-lcompat

/* Get site-specific inclusions */
#include <site.mcr>
