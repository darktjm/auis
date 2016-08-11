/*
 *      Copyright BellSouth 1991 - All Rights Reserved
 *      For full copyright information see:'andrew/config/COPYRITE.bls'.
 */

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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/dynlink/hp700_90/RCS/doload.c,v 1.1 1994/06/06 17:12:53 rr2b Stab74 $";
#endif


/* 
 *	doload.c - dynamic loader for class system
 *          for HP 9000/800's running HPUX 7.0
 *      Written by David Anderson
 *	Thu 7 March 1991
 *
 *      Loosely based on Zalman's RS6000 version.
 */

#include <andrewos.h>
#include <stdio.h>
#include <sys/param.h> /* For MAXPATHLEN */
#include <stddef.h> /* For NULL and the like. */
#include <errno.h>
#include <dl.h>

#define TRUE 1
#define FALSE 0

/* This should be removed, but the machine independent code declares
 * doload_Extension external expecting it to be defined in the machdep
 * code. I suppose one might want to set this to something other than ".do"
 * but for now we will do it the way it has always been done.
 */
char doload_extension[] = ".do";

int doload_trace=0;		/* nonzero if debugging */
extern int errno;

/* doload: Load a dynamic object.
 *
 * Basically this just uses HP's dynamic loader and relies on it
 * to do the right thing.
 */

char *doload(inFD, name, bp, lenP, path) /* return pointer to entry point, */
                                         /* or NULL if error */
/* UNUSED */ int inFD;			 /* open fd for package */
char *name;  		/* name of package being loaded */
char **bp;              /* base address of package */
long *lenP;		/* size of text segment */
char *path;		/* Pathname of package being loaded */
{
    shl_t *status;
    int status2;
    long i;
    void (*entry)();
    char *dummy;
    char entryname[256];
    char libpath[MAXPATHLEN];

/* In theory, one might be able to provide the correct values for bp and len,
 * but doindex is going to free the memory associated with a dynamic object
 * before loading another dynamic object to prevent memory bloat.
 *
 * Therefore this routine fakes something for doindex to free. Of course
 * the correct way to do this is to add a routine to the doload interface to
 * unload a dynamic object. (See the commented out doload_unload below.)
 * If doindex and all the doload files for other machines are ever fixed,
 * this code can be eliminated.
 *
 * Note that the loss of correct bp and len info is not to big a deal since
 * it is mostly just used for debugging.
 */
    dummy = (char *) malloc(1);
    if (dummy == NULL)
        return NULL;
    *bp = dummy;
    *lenP = 1;

    status = cxxshl_load(path, BIND_DEFERRED, 0L);
    if (status == 0) {
	fprintf(stderr, "\ndoload: error loading %s from %s\n", name, path);
	fprintf(stderr, "          errno %d\n", errno);
        return NULL;
    }

    status2 = shl_findsym(&status, "ATK_EntryFunction", TYPE_PROCEDURE, &entry);
    if (status2 == -1) {
        fprintf(stderr, "doload: ATK_EntryFunction is undefined\n");
        return NULL;
    }

    if (doload_trace > 0) {
	fprintf(stderr, "doload: loaded %s from %s, entry: %lx\n",
		name, path, entry);
    }

    return entry;
}
