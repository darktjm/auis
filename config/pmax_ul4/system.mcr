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
#include <pmax_ul4/system.h>
        SYSTEM_H_FILE = pmax_ul4/system.h
#undef In_Imake

/* These next two lines help configure the embedded machine-dependent
    directories overhead/class/machdep, atk/console/stats, and
    atk/console/stats/common. */
        SYS_IDENT = pmax_ul4
        SYS_OS_ARCH = dec_mips
        SYS_CONFDIR= pmax_ul4
/* Get parent inclusions */
#include <allsys.mcr>

DYNLINKPROG =ANDREWDIR=$(BASEDIR);export ANDREWDIR;$(BASEDIR)/etc/adynlink $(CPPC) $(ATKDYNDEBUG)
AEXELINK=ANDREWDIR=$(BASEDIR);export ANDREWDIR;$(BASEDIR)/etc/aexelink
PREDYNFLAGS = -Wl,-r
/* Now for the system-dependent information. */
/* (currently none) */

/* MIPS' compiler seems to have the standard set of PCC bugs dealing with 
 *     void...
 * The -G 0 is to prevent dynamically loadable modules from having global
 *     area sections.
 * The -Wl,D,1000000 switch is to move the data area down so that
 *     dynamically loaded routines are addressible within the 28 bit jump
 *     offset limit of the MIPS architecture.
 */
        CC = gcc
        CCNOG0 = gcc
 /*       STD_DEFINES =  "-Wl,-D,1000000" */
STD_DEFINES = 
        MIPS_GS_FILE = $(DESTDIR)/etc/mips_glspace.c
        MIPS_GSIZE_SCRIPT = $(DESTDIR)/etc/mips_gsize.awk
COMPILERFLAGS="-Wl,-D,1000000"
ATKLIBFLAGS = -d '$(BASEDIR)/etc/alibexp'
ATKTMPDYNEXTM=-
POSTDYNFLAGS= -L$(BASEDIR)/lib/dynlink $(CPPSTDLIB) $(GCCLIB) -lc
MKDYNMAIN = $(BASEDIR)/etc/mkdynmain -ctor -main

/* Get site-specific inclusions */
#include <site.mcr>
