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

#include <andrewos.h> /* sys/file.h */

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/ams/libs/ms/RCS/storfile.c,v 2.8 1993/07/02 23:01:53 rr2b Stab74 $";
#endif

#include <stdio.h>
#include <ms.h>

MS_StorePartialFile(FileName, startpos, len, mode, Truncate, WhatToStore)
char *FileName, *WhatToStore;
int startpos, len, mode, Truncate;
{
    int fd, errsave;

    debug(1, ("MS_StorePartialFile %s pos %d len %d mode %d\n", FileName, startpos, len, mode));
    fd = open(FileName, O_RDWR | O_CREAT, mode);
    if (fd<0) {
	AMS_RETURN_ERRCODE(errno, EIN_OPEN, EVIA_STOREPARTIALFILE);
    }
    if (startpos>0 && lseek(fd, startpos, SEEK_SET) < 0) {
	errsave = errno;
	vclose(fd);
	if ((startpos == 0) && Truncate) unlink(FileName);
	AMS_RETURN_ERRCODE(errsave, EIN_LSEEK, EVIA_STOREPARTIALFILE);
    }
    if (writeall(fd, WhatToStore, len) < 0) {
	errsave = errno;
	vclose(fd); /* BOGUS -- should somehow ABORT the Vice store */
	if ((startpos == 0) && Truncate) unlink(FileName);
	AMS_RETURN_ERRCODE(errsave, EIN_WRITE, EVIA_STOREPARTIALFILE);
    }
    if (Truncate && ftruncate(fd, startpos + len)) {
	errsave = errno;
	vclose(fd); /* BOGUS -- should somehow ABORT the Vice store */
	if ((startpos == 0) && Truncate) unlink(FileName);
	AMS_RETURN_ERRCODE(errsave, EIN_FTRUNCATE, EVIA_STOREPARTIALFILE);
    }
    if (vclose(fd)) {
	errsave = errno;
	if ((startpos == 0) && Truncate) unlink(FileName);
	AMS_RETURN_ERRCODE(errsave, EIN_VCLOSE, EVIA_STOREPARTIALFILE);
    }
    return(0);
}
