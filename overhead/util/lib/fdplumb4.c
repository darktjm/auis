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

#include <andrewos.h>
#include <stdio.h>
#include <fdplumbi.h>
#include <util.h>
 

FILE *dbg_qopen(const char *path, const char * const argv[], const char *mode)
{
    FILE *fp;

    fp = (FILE *) qopen(path, argv, mode);
    if (fp) RegisterOpenFile(fileno(fp), path, FDLEAK_OPENCODE_QOPEN);
    return(fp);
}

FILE *dbg_topen(const char *path, const char * const argv[], const char *mode, int *pgrp)
{
    FILE *fp;

    fp = topen(path, argv, mode, pgrp);
    if (fp) RegisterOpenFile(fileno(fp), path, FDLEAK_OPENCODE_TOPEN);
    return(fp);
}

int dbg_qclose(FILE *fp)
{
    RegisterCloseFile(fileno(fp));
    return(qclose(fp));
}

int dbg_tclose(FILE *fp, int seconds, int *timedout)
{
    RegisterCloseFile(fileno(fp));
    return(tclose(fp, seconds, timedout));
}

