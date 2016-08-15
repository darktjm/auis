/* Copyright 1993 Carnegie Mellon University All rights reserved.
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
#define IN_ATKOS_LIB 1
#include <system.h>
/* systems needing this should have the following in system.h:
 #define NEED_ANSI_TMPFILES 1
 #include <atkos.h>
... see atkos.h to see which #defines can be overridden in the system.h file.

 Also, in system.mcr SUPLIBS should include $(BASEDIR)/lib/libatkos.a possibly in addition to
 libossupport.a
P_tmpdir can be defined as something else if needed.
 Not sure how to define TMP_MAX correctly..., the value in atkos.h is a guess.
 */

#ifndef NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/ossupport/RCS/oscommon.c,v 1.19 1995/01/16 04:29:02 rr2b Stab74 $";
#endif

#include "atkos.h"

long atkos_dummy_variable_for_ar=99;

#ifdef NEED_ATKINIFINI

void ATK_DoFini() {
    extern void (*ATK_FiniList[])();
    void (**p)()=ATK_FiniList;
    while(*p) {
	(*p)();
	p++;
    }
}

void ATK_DoIniFini() {
    extern void (*ATK_IniList[])();
    void (**p)()=ATK_IniList;
    while(*p) {
	(*p)();
	p++;
    }
    atexit((void (*)())ATK_DoFini);
}

#endif
#ifdef NEED_ANSI_TMPFILES

#ifndef DONT_DECLARE_MKTEMP
extern char *mktemp();
#endif

char *Andrew_tmpnam(buf)
char *buf;
{
    static char lbuf[L_tmpnam];
    char *ptr=buf;
    if(ptr==NULL) ptr=lbuf;
    strcpy(ptr, P_tmpdir);
    sprintf(ptr+strlen(ptr), "And%dXXXXXX", (unsigned short)getpid());
    ptr=mktemp(ptr);
    if(ptr==NULL || *ptr=='\0' || (*ptr=='/' && ptr[1]=='\0')) return NULL;
    else return ptr;
}

FILE *Andrew_tmpfile()
{
    static char buf[L_tmpnam];
    char *file=Andrew_tmpnam(buf);
    FILE *fp=NULL;
    if(file==NULL) {
	fprintf(stderr, "warning couldn't get name for temporary file.\n");
	return NULL;
    }
    fp=fopen(file, "w+");
    unlink(buf);
    return fp;
}
#endif


/* sigh, this is against policy, but AFS needs actual functions for the following,
 so the rename the function and use a #define trick won't help here -rr2b */
#ifdef NEED_UTIMES
#include <utime.h>
int utimes(file, tvp)
char *file;
struct timeval *tvp;
{
	struct utimbuf times;
	times.actime = tvp[0].tv_sec;
	times.modtime = tvp[1].tv_sec;
	return utime(file,&times);
}
#endif /* NEED_UTIMES */
 
#ifdef NEED_RANDOM_MAPPED_TO_RAND48
#undef random
#undef srandom
long random()
{
    return lrand48();
}

void srandom(seed)
int seed;
{
    srand48(seed);
}
#endif /* NEED_RANDOM_MAPPED_TO_RAND48 */

#ifdef NEED_INSREMQUE
struct qelem {
    struct qelem *next;
    struct qelem *prev;
    char q_data[1];
};

void insque(elq, pred)
struct qelem *elq, *pred;
{
    if(pred->next) pred->next->prev = elq;
    elq->next=pred->next;
    elq->prev=pred;
    pred->next=elq;
}

void remque(elq)
struct qelem *elq;
{
    if(elq->prev) elq->prev->next=elq->next;
    if(elq->next) elq->next->prev=elq->prev;
}
#endif /* NEED_INSREMQUE */

#ifdef NEED_LOCKF
 /* This provides the atk_lockf and atk_unlockf functions that
   translate lockf requests into the appropriate fcntl calls. )Rob Earhart (re00@andrew) */

#include <fcntl.h>

int Andrew_lockf(fd)
int fd;
{
    struct flock f_rec;

    f_rec.l_type = F_WRLCK;
    f_rec.l_whence = 0;
    f_rec.l_start = 0;
    f_rec.l_len = 0;
    return((fcntl(fd, F_SETLK, &f_rec) !=-1)?0:-1);
}

int Andrew_unlockf(fd)
int fd; {
    struct flock f_rec;

    f_rec.l_type = F_UNLCK;
    f_rec.l_whence = 0;
    f_rec.l_start = 0;
    f_rec.l_len = 0;
    return((fcntl(fd, F_SETLK, &f_rec) != -1)?0:-1);
}
#endif
