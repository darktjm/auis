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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/overhead/dynlink/rs_aix3/RCS/doload.c,v 1.3 1996/03/28 15:57:43 robr Stab74 $";
#endif

/* 
 *	doload.c - AIX.V3 dynamic loader for class system
 *      Written by Zalman Stern
 *	Mon Mar 19 19:46:57 1990
 */
#include <andrewos.h>
#include <sys/types.h>
#include <sys/ldr.h>
#include <stddef.h> /* For NULL and the like. */
#include <errno.h>

extern int			    errno, sys_nerr;
extern char			   *sys_errlist[];

/* This should be removed, but the machine independent code declares
 * doload_Extension external expecting it to be defined in the machdep
 * code. I suppose one might want to set this to something other than ".do"
 * (like "") to conform to some AIX convention but for now we will do it
 * the way it has always been done.
 */
char doload_extension[] = ".do";

int doload_trace=0;		/* nonzero if debugging */
/* Dummy function for debugging.
 * Set a breakpoint here to catch *after* a file has been loaded.
 * Then you can set breakpoints on the newly loaded code.
 */
void doload_done() {};

static void CheckPath(char *path, boolean fload) {
    int (*mfunc)(int argc, char **argv);
    static struct inited {
	struct inited *next;
	char path[1];
    } *first=NULL;
    struct inited *i=first;
    while(i) {
	if(strcmp(i->path,path)==0) return;
	i=i->next;
    }
    i=(struct inited *)malloc(strlen(path)+sizeof(struct inited));
    if(i==NULL) return;
    strcpy(i->path,path);
    i->next=first;
    first=i;
    if(fload) {
	mfunc=(int (*)(int, char **))load(path, 1, NULL);
	if(mfunc==NULL) {
	    perror("doload WARNING");
	    return;
	}
	if(mfunc(0,NULL)!=0) {
	    fprintf(stderr, "doload WARNING: couldn't init %s.\n", path);
	    return;
	}
    }
}

static void InitInitialLoads()  {
    struct ld_info *li;
    char *info=(char *)malloc(25600);
    int lqrc;
    if(info) {
	lqrc=loadquery(L_GETINFO, info, 25600);
    } else lqrc=(-1);
    if(lqrc<0) {
	perror("WARNING: loadquery or malloc failed");
	return;
    }
    li=(struct ld_info *)info;
    do {
	char *p=strrchr(li->ldinfo_filename, '.');
	if(p) {
	    if(strcmp(p,".+")==0) CheckPath(li->ldinfo_filename, FALSE);
	}
	if(li->ldinfo_next) li=(struct ld_info *)(((char *)li)+li->ldinfo_next);
	else break;
    } while(li);
    free(info);
}

/* doload: Load a dynamic object.
 *
 * Basically, this just calls the AIX dynamic loader and relies on the
 * system to do the right thing. This will search for libraries in the normal
 * system manner. Note that if the path parameter is not an absolute path,
 * this will search for the module along $PATH. I don't think this ever
 * happens though. There are too many casts here, but its a proof of
 * concept. Hack it 'til it glows.
 */
char *doload(inFD, name, bp, lenP, path) /* return pointer to entry point, */
				/* or NULL if error */
/* UNUSED */ int inFD;			/* open fd for package file. */
/* UNUSED */ char *name;			/* name of package being loaded */
char **bp;			/* base address of package */
long *lenP;			/* size of text segment */
char *path;			/* Pathname of package being loaded */
				/* Path is used by the MACH loader, not this one */
{

    char *entry;
    char *dummy;
    int lqrc;
/* In theory, one could provide the correct values for bp and len using
 * the loadquery system call, but doindex is going to free the memory
 * associated with a dynamic object before loading another dynamic object
 * to prevent memory bloat.
 *
 * Therefore this routine fakes something for doindex to free. Of course
 * the correct way to do this is to add a routine to the doload interface to
 * unload a dynamic object. (See the commented out doload_unload below.)
 * If doindex and all the doload files for other machines are ever fixed,
 * this code can be eliminated.
 *
 * Note that the loss of correct bp and len info is not to big a deal since
 * it is mostly just used for debugging which AIX handles in a much cleaner
 * manner.
 */
    static boolean inited=FALSE;
    if(!inited) {
	InitInitialLoads();
	inited=TRUE;
    }
    dummy = (char *) malloc(1);
    if (dummy == NULL)
        return NULL;
    *bp = dummy;
    *lenP = 1;

	
    entry = (char *) load(path, 1, NULL);
   
    if (entry == NULL){
	int pid;
        char *load_errs[1024];

	printf("While attempting to load %s:\n", path);

	if(errno > 0 && errno <= sys_nerr) 
	    printf("Error on load syscall: '%s'\n", sys_errlist[errno]);
	else
	    printf("Unknown error on load syscall: errno = '%d'\n", errno);
	pid=fork();
	if(pid==0) {
	    load_errs[0] = "execerror";
	    load_errs[1] = path;
	    if (lqrc = loadquery(L_GETMESSAGES, &load_errs[2], sizeof(load_errs) - 2 * sizeof(load_errs[0])) == 0)
		execvp("/etc/execerror", load_errs);
	    else if(errno > 0 && errno <= sys_nerr) 
		printf("Loadquery returned '%d': '%s'\n", lqrc, sys_errlist[errno]);
	    else
		printf("Loadquery returned '%d', errno = '%d'\n", lqrc, errno);
	} else if(pid<0) {
	    perror("doload fork");
	}
    }
    {
	struct ld_info *li;
	char *info=(char *)malloc(25600);
	if(info) {
	    lqrc=loadquery(L_GETINFO, info, 25600);
	} else lqrc=(-1);
	if(lqrc<0) {
	    perror("WARNING: loadquery or malloc failed");
	    return NULL;
	}
	li=(struct ld_info *)info;
	do {
	    char *p=strrchr(li->ldinfo_filename, '.');
	    if(p) {
		if(strcmp(p,".+")==0) CheckPath(li->ldinfo_filename, TRUE);
	    }
	    if(li->ldinfo_next) li=(struct ld_info *)(((char *)li)+li->ldinfo_next);
	    else break;
	} while(li);
	free(info);
    }
    /* Bind undefined symbols in the newly loaded object with ourselves. */
    if (loadbind(0, doload, entry) == -1)
	perror("WARNING: loadbind  failed");	/* Shouldn't ever fail... */
    if (doload_trace) {
	printf("doload: just loaded %s\n", path);
	doload_done();
    }
    CheckPath(path, FALSE);
    return entry;
}

#if 0
/* doload_unload: Free the resources associated with a dynamicly loaded */
 * object.
 * The programmer is responsible for making sure that no pointers into the
 * storage associated with the dynamic object are held. On most systems,
 * this routine will simply do a "free(base);". This routine is provided
 * for doindex to call. It cannot safely be used in most situations.
 */ 
doload_free(name, base)
    char *name;
    char *base; /* Not a string. */
{
    unload(name);
}
#endif
