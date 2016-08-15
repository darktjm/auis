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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/ams/libs/shr/RCS/tildes.c,v 2.10 1992/12/15 21:22:34 rr2b Stab74 $";
#endif

#include <stdio.h>
#include <pwd.h>
#include <andyenv.h>
#include <util.h>
#ifdef WHITEPAGES_ENV
#include <wp.h>
#endif /* #ifdef WHITEPAGES_ENV */

#if !POSIX_ENV
extern char *malloc();
#endif

/* The purpose of this routine is simply to minimize the number of times
	we call the getpwnam routine, because that routine seems to be
	both very slow AND seems to encourage fragmentation of memory
	by mallocing huge areas of memory for the password file.

   The basic idea is very simple:  We keep a cache of every name we've
	looked up before, and we don't go to the password file if we can
	get it from the cache.  Currently the cache is searched linearly;
	so far I have not known anyone to use more than 5 or 6 different
	tilde references in a session.  The real problem is that the
	same ones get made over and over, especially by the bboard daemon.
*/

static struct udircache {
    char *user, *ucell, *udir;
    struct udircache *next;
} *Udc = NULL;

char *
FindUserDir(user, cellname)
char *user, *cellname;
{
    struct udircache *u;
    struct passwd *pw;

    for (u = Udc; u; u = u->next) {
	if (strcmp(u->user, user) == 0 && ULstrcmp(cellname, u->ucell) == 0) break;
    }
    if (u) {
	return(u->udir);
    }

    pw=getcpwnam(user, cellname);
    if (!pw) {
	return((char *) NULL);
    }
    u = (struct udircache *) malloc(sizeof(struct udircache));
    if (!u) return((char *) -1);
    u->user = NewString(user);
    u->udir = NewString(pw->pw_dir);
    if (!u->user || !u->udir) {
	if (u->user != NULL) free(u->user);
	if (u->udir != NULL) free(u->udir);
	free(u);
	return((char *) -1);
    }
    u->ucell = NewString(cellname);
    if (u->ucell == NULL) {
	free(u->user); free(u->udir); free(u);
	return((char *) -1);
    }
    u->next = Udc;
    Udc = u;
    return(u->udir);
}
