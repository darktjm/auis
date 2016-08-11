/* $Id$ */


/* Copyright IBM Corporation 1994 - All Rights Reserved */
/* For full copyright information see:'andrew/config/COPYRITE' */
 
#define In_Imake 1
/* The next two lines need to be kept in sync */
#include <ix86_OS2/system.h>
SYSTEM_H_FILE = ix86_OS2/system.h
#undef In_Imake
 
/* These next two lines help configure the embedded machine-dependent
    directories overhead/class/machdep, atk/console/stats, and atk/console/stats/common. */
SYS_IDENT = ix86_OS2
SYS_OS_ARCH = OS2
SYS_CONFDIR = ix86_OS2

/* Get parent inclusions */
#include <allsys.mcr>

/* Defaults for EMX and IBM TCP/IP directories. */
/*allow environment variables EMXDIR or TCPIPDIR to override*/
ifndef EMXDIR
  EMXDIR = c:/emx
endif
ifndef TCPIPDIR
  TCPIPDIR = c:/tcpip
endif

XLIBDIR = $(TCPIPDIR)/lib
XLIB = $(XLIBDIR)/xlibi.lib
 
/* Now for the system-dependent information. */
STD_DEFINES = -DOS2
STD_CPPC_DEFINES = -DOS2 -DUSE_GCCPRAGMA
INCLUDES =  -I${BASEDIR}/include -I$(BASEDIR)/include/atk -I${TCPIPDIR}/include
COMPILERFLAGS = -Zomf -Zcrtdll -Zstack 0x150
CDEBUGFLAGS =

SYS_LIBRARIES = $(BASEDIR)/lib/libossup.a -lsocket -lbsd
ATKLIBLDARGS = $(SYS_LIBRARIES)
ATKLIBFLAGS = -s

XFC = bdftopcf
XMKFONTDIR = mkfontdr
DEPENDSCRIPT = $(TOP)/config/depend
XMAKEDEPEND = makedepend

/* File extension for executables. */
EXEEXT = .exe
DYNEXT = dll
TMPDYNEXT = out
DYNPOSTPROCESS = $(BASEDIR)/etc/mkatkdyn

INSTALL = install
LN = cp -p
AR = emxomfar clq
RANLIB = emxomfar s

GENSTATL_EXTRA = -s

CC = gcc
CPPC = gcc
LEX = flex
YACC = bison -y
LEXLIB = -lfl
IMAKE = imake -DOS2
XFC = bdftopcf
XMKFONTDIR = mkfontdr

RESOLVLIB =
RESINC =

ATKALIASES = $(BASEDIR)/lib/atkaliases

/* Get site-specific inclusions */
 
#include <site.mcr>


