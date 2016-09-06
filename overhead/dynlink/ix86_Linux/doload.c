/* Copyright 1994 Carnegie Mellon University All rights reserved.
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

/* 
 *	doload.c - dynamic loader for SunOS 4.1.3
 */

#include <andrewos.h>
#include <errno.h>
#include <dlfcn.h>
#include "doload.h"

#if 0
static struct dlist {
    long *dtors;
    struct dlist *next;
} *dtorschain=NULL;

static void dodtors() {
    while(dtorschain) {
	long *dtors=dtorschain->dtors;
	dtors++;
	while(*dtors) {
	    ((void (*)())(*dtors))();
	    dtors++;
	}
	dtorschain=dtorschain->next;
    }
}
#endif

int doload_trace=0;		/* nonzero if debugging */

static int dummyfunc(int argc, char **argv) {
    return 0;
}

/* doload: Load a dynamic object.
 *
 * Basically this just uses Sun's dynamic loader and relies on it
 * to do the right thing.
 */

char *doload( /* return pointer to entry point, */
                                         /* or NULL if error */
/* UNUSED */ int inFD,			 /* open fd for package */
char *name,  		/* name of package being loaded */
char **bp,              /* base address of package */
long *lenP,		/* size of text segment */
char *path)		/* Pathname of package being loaded */
{
    void *status;
    void (*entry)();
    
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
/* Then again, these days, doindex (??) isn't even called, so this just leaks */
    *bp = NULL;
    *lenP = 0;

    status = dlopen(path, RTLD_LAZY);
    if (status == 0) {
	fprintf(stderr, "\ndoload: error loading %s from %s\n", name, path);
	fprintf(stderr, "          errno %d\n", errno);
        return NULL;
    }
#if 0 
    long *ctors;
    long *dtors;
    ctors = (long *)dlsym(status, "__CTOR_LIST__");
    if(ctors==NULL) {
	
    } else {
	ctors++;
	while(*ctors) {
	    ((void (*)())*ctors)();
	    ctors++;
	}
    }
    
    dtors = (long *)dlsym(status, "__DTOR_LIST__");
    if(dtors==NULL) {
	
    } else {
	if(*dtors) {
	    struct dlist *n=(struct dlist *)malloc(sizeof(struct dlist));
	    if(n) {
		n->dtors=dtors;
		n->next=dtorschain;
		if(dtorschain==NULL) {
		    on_exit(dodtors, 0);
		}
		dtorschain=n;
	    }
	}
    }
#endif
#if 0
entry = (void (*)())dlsym(status, "main");
    if (entry == NULL) {
        fprintf(stderr, "doload: ATK_EntryFunction is undefined\n");
        return NULL;
    }
#endif
    if (doload_trace > 0) {
	fprintf(stderr, "doload: loaded %s from %s, entry: %p\n",
		name, path, entry);
    }

    return (char *)dummyfunc;
}
