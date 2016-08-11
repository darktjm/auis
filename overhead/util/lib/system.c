/* Copyright 1992 Carnegie Mellon University, All rights reserved.
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

/* $Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/util/lib/RCS/system.c,v 1.3 1992/12/14 20:21:35 rr2b Stab74 $ */

#ifndef NORCSID
static char system_c_rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/util/lib/RCS/system.c,v 1.3 1992/12/14 20:21:35 rr2b Stab74 $";
#endif

#include <andrewos.h>

/* work around a bug in system where if the parent process catches sigchld system returns a bogus exit status. */
#ifdef hpux
#include <stdlib.h>
#include <errno.h>
extern int errno;
int os_system(cmd)
char *cmd;
{
    int result;
    errno=0;
    result=system(cmd);
    if(result==-1 && errno==EINTR) return 0;
    return result;
}
#else
int os_system(cmd)
char *cmd;
{
    return system(cmd);
}
#endif

