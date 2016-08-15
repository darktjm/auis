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

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/ams/libs/ms/RCS/ckmail.c,v 2.11 1993/06/15 03:56:02 rr2b Stab74 $";
#endif

#include <ms.h>

extern char home[], *GetPersonalMailbox();

MS_DoIHaveMail(count)
int *count;
{
    DIR *dirp;
    DIRENT_TYPE *dirent;
    char *Mailbox;

    *count = 0;
    Mailbox = GetPersonalMailbox();
    if ((dirp = opendir(Mailbox)) == NULL) {
	AMS_RETURN_ERRCODE(errno, EIN_OPENDIR, EVIA_DOIHAVEMAIL);
    }
    while ((dirent = readdir(dirp)) != NULL) {
	if (dirent->d_name[0] != '.') {
	    ++*count;
	}
    }
    closedir(dirp);
    return(0);
}
