/* Copyright IBM Corporation 1988,1989 - All Rights Reserved */

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
#include <sun4_mach/system.h>
        SYSTEM_H_FILE = sun4_mach/system.h
#undef In_Imake

/* These next two lines help configure the embedded machine-dependent
    directories overhead/class/machdep, atk/console/stats, and
    atk/console/stats/common. */
        SYS_IDENT = sun4_mach
        SYS_OS_ARCH = sun_sparc_mach

/* Get parent inclusions */
#include <allsys.mcr>

#ifdef OPENWINDOWS_ENV
        CONVERTFONT = /usr/openwin/bin/convertfont
        BLDFAMILY = /usr/openwin/bin/bldfamily
#endif /*OPENWINDOWS_ENV */

/* Now for the system-dependent information. */

        MALLOCALIGNMENT = 8
        COMPILERFLAGS = -Dsun -Dsun4 -DMACH -Dsparc


/* Get site-specific inclusions */
#include <site.mcr>
