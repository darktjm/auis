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
#include <sun4_41/system.h>
        SYSTEM_H_FILE = sun4_41/system.h
#undef In_Imake

/* These next three lines help configure the embedded machine-dependent
    directories overhead/class/machdep, atk/console/stats, and
    atk/console/stats/common. */
        SYS_IDENT = sun4_41
        SYS_OS_ARCH = sun_sparc_41
       SYS_CONFDIR = sun4_41

/* Get parent inclusions */
#include <allsys.mcr>


        XLIBDIR = /usr/local/lib
        XINCDIR = /usr/include
        XFC = /usr/local/bin/bdftopcf
        XBINDIR = /usr/local/bin
        XUTILDIR = /usr/local/bin

/* Now for the system-dependent information. */
  
        MALLOCALIGNMENT = 8
        CDEBUGFLAGS = 

MKDYNMAIN=$(BASEDIR)/etc/mkdynmain - ATK_EntryFunction
ATKLIBFLAGS = -d '$(BASEDIR)/etc/alibexp'
DYNLINKPROG =ANDREWDIR=$(BASEDIR);export ANDREWDIR;$(BASEDIR)/etc/adynlink $(GCCLIBDIR)/ld
ECOMPFLAGS=
GCCLIBDIR = `$(CC) -v  2>&1 | sed -n -e 's!Reading specs from \\(.*\\)gcc-lib/\\(.*\\)specs!\\1gcc-lib/\\2!p'`
#ifdef USE_SHARED_LIBS
ATKLIBEXTM =so
ATKLIBFLAGS=-s
PICFLAG=-fPIC
#endif

POSTDYNFLAGS= $(CPPSTDLIB)
LINKPREFIX = $(BASEDIR)/etc/relativize $(LOCALNORELS) $(NORELS) ---
        COMPILERFLAGS =$(ECOMPFLAGS) $(PICFLAG)

SYS_LIBRARIES=$(BASEDIR)/lib/libatkos.a
DYN_LINK_LIB = -ldl

CPPC = g++
CDEBUGFLAGS=
YACC=yacc
DYNMATHLIB=
/* Get site-specific inclusions */

#include <site.mcr>

