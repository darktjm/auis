/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
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
 *  $
*/

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/util/lib/RCS/fdplumb6.c,v 2.11 1995/11/07 20:17:10 robr Stab74 $";
#endif


 


#include <andrewos.h>    /* types */
#if defined(POSIX_ENV) || defined(M_UNIX)
#include <dirent.h>
#else
#define dirent direct
#endif
#include <fdplumbi.h>

#if SY_OS2
/* Reaching into the DIR struct is non-portable! */
#define dd_fd dd_id
#endif

DIR *dbg_opendir(name)
char *name;
{
    DIR *d;

    d = opendir(name);
    if (d) {
	RegisterOpenFile(d->dd_fd, name, FDLEAK_OPENCODE_OPENDIR);
    }
    return(d);
}


void dbg_closedir(d)
DIR *d;
{
    RegisterCloseFile(d->dd_fd);
    closedir(d);
}

