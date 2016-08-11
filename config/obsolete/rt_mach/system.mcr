/* Copyright IBM Corporation 1988,1991 - All Rights Reserved */
/* For full copyright information see:'andrew/config/COPYRITE' */
/* $Disclaimer: 
* Permission to use, copy, modify, and distribute this software and its 
* documentation for any purpose and without fee is hereby granted, provided 
* that the above copyright notice appear in all copies and that both that 
* copyright notice and this permission notice appear in supporting 
* documentation, and that the name of IBM not be used in advertising or 
* publicity pertaining to distribution of the software without specific, 
* written prior permission. 
*                         
* THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
* TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
* HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
* DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
* DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
* OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
* WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
* 
*  $ */

#define In_Imake 1
/* The next two lines need to be kept in sync */
#include <rt_mach/system.h>
        SYSTEM_H_FILE = rt_mach/system.h
#undef In_Imake

/* These next two lines help configure the embedded machine-dependent
    directories overhead/class/machdep, atk/console/stats, and
    atk/console/stats/common. */
        SYS_IDENT = rt_mach
        SYS_OS_ARCH = mach_rt

/* Get parent inclusions */
#include <allsys.mcr>


/* Now for the system-dependent information. */
        CC = hc


/* Get site-specific inclusions */
#include <site.mcr>
