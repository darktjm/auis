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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/util/lib/RCS/fdplumb4.c,v 2.7 1992/12/15 21:09:01 rr2b Stab74 $";
#endif


 

#include <stdio.h>
#include <fdplumbi.h>

FILE *dbg_qopen(path, argv, mode)
char *path, *argv[], *mode;
{
    FILE *fp;

    fp = (FILE *) qopen(path, argv, mode);
    if (fp) RegisterOpenFile(fileno(fp), path, FDLEAK_OPENCODE_QOPEN);
    return(fp);
}

FILE *dbg_topen(path, argv, mode, pgrp)
char *path, *argv[], *mode;
int *pgrp;
{
    FILE *fp;
    extern FILE *topen();

    fp = topen(path, argv, mode, pgrp);
    if (fp) RegisterOpenFile(fileno(fp), path, FDLEAK_OPENCODE_TOPEN);
    return(fp);
}

dbg_qclose(fp)
FILE *fp;
{
    RegisterCloseFile(fileno(fp));
    return(qclose(fp));
}

dbg_tclose(fp, seconds, timedout)
FILE *fp;
int seconds, *timedout;
{
    RegisterCloseFile(fileno(fp));
    return(tclose(fp, seconds, timedout));
}

