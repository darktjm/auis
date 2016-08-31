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

/* convenient stuff for dealing with subprocesses */
#include <andrewos.h>		/* sys/time.h index rindex */
#include <util.h>
#include <ctype.h>
#include <truth.h>

/* trash second arg */
char *argvtostr(const char * const *argv,char *buf,unsigned int len)
{
    if(argv!=NULL && *argv){
	const char * const *p=argv;
	int skip = 0;

	if(strcmp(p[0],"/bin/sh")==0 && strcmp(p[1],"-c")==0 && strcmp(p[2],"exec ")==0) {
	    p += 3;
	    skip = 5;
	}

	if(strlen(*p)+1>len)
	    return NULL;
	strcpy(buf,(*p++) + skip);

	while(*p!=NULL){
	    if(strlen(*p)+2>len)
		return NULL;
	    strcat(buf," ");
	    strcat(buf,*p++);
	}
    }else
	return NULL;

    return buf;
}

static int contains(const char *str,const char *set)
{
    const char *p,*q;

    for(p=str; *p!='\0'; p++)
	for(q=set; *q!='\0'; q++)
	    if(*p==*q)
		return TRUE;

    return FALSE;
}

#define CSHCHARS "$;'\"`~<>*?|()"
#define SHCHARS "$;'\"`<>*?|()"

/* trashes first two args */
const char **strtoargv(char *str,const char **argv,int len)
{
    const char **p=argv;
    char *s;
    const char *shell=getenv("SHELL");
    short csh=FALSE;

    if(shell!=NULL){
	const char *leaf=strrchr(shell,'/');
	if(leaf==NULL)
	    leaf=shell;
	else
	    leaf++;
	csh=(strcmp(leaf,"csh")==0);
	if(!contains(str,(csh ? CSHCHARS : SHCHARS)))
	    shell=NULL;
    }
	    
    if(shell!=NULL){
	if(len<5)
	    return NULL;
	*p++=shell;
	if(csh)
	    *p++="-f";
	*p++="-c";
	*p++=str;
	/* put 'exec ' in */
	for(s=str+strlen(str); s>=str; s--)
	    *(s+5)= *s;
	memmove(str,"exec ",5);
	*p=NULL;
	return argv;
    }

    /* no special shell characters */
    while(*str && --len>0){
	while(*str && isspace(*str))
	    *str++='\0';
	if(*str){
	    *p++=str;
	    do
		str++;
	    while(*str && !isspace(*str));
	}
    }
    *p=NULL;

    return argv;
}

/* trash second arg */
char *statustostr(WAIT_STATUS_TYPE *status,char *buf,int len)
{
#if !defined(POSIX_ENV) && defined(M_UNIX)
    snprintf(buf,len,"status = %d.",*status);
    if(*status==0) return NULL;
#else /* hpux */
#if POSIX_ENV
    if(WIFSTOPPED(*status))
	strncpy(buf,"stopped.",len);
    else if (WIFSIGNALED(*status))
	snprintf(buf,len,"killed by signal %d.",WTERMSIG(*status));
    else if (WIFEXITED(*status)) {
	if (WEXITSTATUS(*status) == 0) {
	    strncpy(buf, "exited.",len);
	    return NULL;
	} else
	snprintf(buf,len,"exited with exit status of %d.", WEXITSTATUS(*status));
    }
#else
    if(WIFSTOPPED(*status))
	strncpy(buf,"stopped.",len);
    else if(status->w_termsig!=0){
	snprintf(buf,"killed by signal %d",status->w_termsig);
	if(status->w_coredump)
	    strcat(buf," (core dumped)."); /* not worth trying to add len */
	else
	    strcat(buf,"."); /* not worth trying to add len */
    }else if(status->w_retcode==0){
	strncpy(buf,"exited.",len);
	return NULL; /* successful */
    }else
	sprintf(buf,"exited with exit status of %d.",status->w_retcode);
#endif /* POSIX_ENV */
#endif /* hpux */
    return buf;
}
