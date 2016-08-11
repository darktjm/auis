/* $Id$ */

/*
	$Disclaimer: 
Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, provided 
that the above copyright notice appear in all copies and that both that 
copyright notice and this permission notice appear in supporting 
documentation, and that the name of IBM not be used in advertising or 
publicity pertaining to distribution of the software without specific, 
written prior permission. 
                        
THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

 $
*/

SHELL=/bin/sh
 
/* Copyright IBM Corporation 1988,1991 - All Rights Reserved */
/* For full copyright information see:'andrew/config/COPYRITE' */
 
#define In_Imake 1
/* The next two lines need to be kept in sync */
#include <rs_aix3/system.h>
SYSTEM_H_FILE = rs_aix3/system.h
#undef In_Imake
 
/* These next two lines help configure the embedded machine-dependent
    directories overhead/class/machdep, atk/console/stats, and atk/console/stats/common. */
SYS_IDENT = rs_aix3
SYS_OS_ARCH = aix_3
 
SYS_CONFDIR = rs_aix3

/* Get parent inclusions */
#include <allsys.mcr>
 
/* Now for the system-dependent information. */
STD_DEFINES = -DAIX -D_POSIX_SOURCE -D_ALL_SOURCE
STD_CPPC_DEFINES = -DAIX -D_POSIX_SOURCE -D_ALL_SOURCE
/* Don't trust optimization for 3.1 */
CDEBUGFLAGS =

RANLIB = echo ranlib is not needed this system

/* BSDLIB = -lbsd */

ATKLIBFLAGS=-s
DASHL=-L
SHARED_LIB_PATH = $(DASHL)DEFAULT_ANDREWDIR_ENV/lib $(DASHL)DEFAULT_ANDREWDIR_ENV/lib/atk

PREDYNFLAGS=

#ifdef GNU_ENV
CC = gcc
CPPC=g++
#if SY_AIX4
COMPILERFLAGS=-mcpu=common
#endif
#else
/* Installed exports file for users of libclass.a */
CC = cc
#endif

#ifdef IBMCSET_ENV
CPPC = xlC -qlanglvl=extended
ATKLIBLDARGS = -L/usr/lpp/xlC/lib -lC -lc
#endif

SYS_LIBRARIES = $(BASEDIR)/lib/libossup.a $(BASEDIR)/lib/libatkos.a

LINKPREFIX=$(BASEDIR)/etc/relativize $(LOCALNORELS) $(NORELS) --- 
 
#ifndef IBMCSET_ENV
AEXELINK=$(BASEDIR)/etc/aexelink 
#endif

#define ConstructMFLAGS 1
 
/* Get site-specific inclusions */
 
#include <site.mcr>
 
