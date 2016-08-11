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

#define ATK_IN_RELATIVIZE 1

#include <andrewos.h>
#include <string.h>

/* Define this function here so that this can be compiled before everything else. */
char *NewString(char *str)
{
    char *result;
    if(str==NULL) return NULL;
    result=(char *)malloc(strlen(str)+1);
    if(result==NULL) return result;
    strcpy(result, str);
    return result;
}
struct dontlist {
    char *path;
    struct dontlist *next;
} *dont=NULL, **lastdont=(&dont);

struct dirlist {
    char *path;
    struct dirlist *next;
} *dirs=NULL, **last=(&dirs);

int AddToDontList(char *path)
{
    struct dontlist *s=dont;
    while(s) {
	if(strcmp(s->path, path)==0) break;
	else s=s->next;
    }
    if(s==NULL) {
	struct dontlist *result=(struct dontlist *)malloc(sizeof(struct dontlist));
	result->path=NewString(path);
	result->next=NULL;
	*lastdont=result;
	lastdont=(&result->next);
	return 1;
    }
    return 0;
}

boolean Dont(char *path)
{
    struct dontlist *s=dont;
    while(s) {
	char *p=strrchr(s->path, '/');
	/* don't do it if it is an exact match. */
	if(strcmp(s->path, path)==0) return TRUE;
	if(p) {
	     /* if the don't path ended in a / don't do any libs in that directory, but do do libs in subdirs. */
	    int len = p-s->path+1;
	    if(p[1]=='\0' && strncmp(s->path, path, len)==0 && strchr(path+len, '/')==NULL) return TRUE;
	} else {
	     /* compare just the last component of the path name to the don't list path */
	    p=strrchr(path, '/');
	    if(p && strcmp(s->path, p+1)==0) return TRUE;
	}
	s=s->next;
    }
    return FALSE;
}	

int AddToDirList(char *path)
{
    struct dirlist *s=dirs;
    while(s) {
	if(strcmp(s->path, path)==0) break;
	else s=s->next;
    }
    if(s==NULL) {
	struct dirlist *result=(struct dirlist *)malloc(sizeof(struct dirlist));
	result->path=NewString(path);
	result->next=NULL;
	*last=result;
	last=(&result->next);
	return 1;
    }
    return 0;
}

char **newargv=NULL;
int size=0;
int newargc=0;

void AddArg(char *arg)
{
    if(newargc>=size) {
	size+=10;
	if(newargv) newargv=(char **)realloc(newargv, size*sizeof(char *));
	else newargv=(char **)malloc(size*sizeof(char *));
    }
    newargv[newargc]=NewString(arg);
    newargc++;
}

char *libexts[]={
    "a",
#ifdef ATKLIBEXT
    ATKLIBEXT,
#endif
    NULL
};

void ProcessArg(char *arg, char *p, char *q)
{
    struct dirlist *s=dirs;
    char **libext=libexts;
    if(arg[0]=='-') {
	if(arg[1]=='L') {
	    if(AddToDirList(arg+2)) AddArg(arg);
	    return;
	}
    }
    if(q) {
    char *start=q+1;
    int len=strlen(start);
    while(*libext) {
	int rlen=strlen(*libext);
	if(len>rlen && start[len-rlen-1]=='.' && strcmp(start+len-rlen, *libext)==0 && strncmp(start, "lib", 3)==0 && !Dont(arg)) {
	    char *n;
	    start[len-rlen-1]='\0';
	    *q='\0';
	    while(s) {
		if(strcmp(s->path, arg)==0) {
		    n=(char *)malloc(strlen(start+3)+3);
		    strcpy(n, "-l");
		    strcat(n, start+3);
		    AddArg(n);
		    free(n);
		    return;
		}
		s=s->next;
	    }
	    n=(char *)malloc(strlen(arg)+3);
	    strcpy(n, "-L");
	    if(arg[0]) strcat(n, arg);
	    else strcat(n, "/");
	    if(AddToDirList(arg)) AddArg(n);
	    n=(char *)realloc(n, strlen(start+3)+3);
	    strcpy(n, "-l");
	    strcat(n, start+3);
	    AddArg(n);
	    free(n);
	    return;
	}
	libext++;
    }
    }
    AddArg(arg);
}

static void usage() {
    fprintf(stderr, "usage: relativize [directory/ or pathname or libname.a, as many as needed ---] command arguments\n");
    exit(-1);
}

int main(int argc, char **argv)
{
    int i;
    if(argc<2) usage();
    for(i=1;i<=argc-1 && argv[i][0]!='-';i++) {
	AddToDontList(argv[i]);
    }
#ifdef ATK_RELATIVIZE_OPTIONS
    ATK_RELATIVIZE_OPTIONS(argc, argv, i);
#endif
    if(i>=argc) usage();
    if(i<argc) if(argv[i][0]=='-' && argv[i][1]=='-' && argv[i][2]=='-') i++;
    if(i>=argc) usage();
#ifdef ATK_RELATIVIZE_START_HOOK
    ATK_RELATIVIZE_START_HOOK(argc, argv, i);
#endif
    for(;i<=argc-1;i++) {
	char *arg=NewString(argv[i]);
	char *p=strrchr(arg, '.');
	char *q=strrchr(arg, '/');
#ifndef ATK_RELATIVIZE_FUNCTION
	ProcessArg(arg, p, q);
#else
	ATK_RELATIVIZE_FUNCTION(arg, p, q);
#endif
	free(arg);
    }
#ifdef ATK_RELATIVIZE_FINISH_HOOK
    ATK_RELATIVIZE_FINISH_HOOK(argc, argv, i);
#endif
    for(i=0;i<newargc;i++) printf("%s%s",newargv[i],i<newargc-1?" ":"");
    printf("\n");
    AddArg(NULL);
    fflush(stdout);
    fflush(stderr);
    execvp(newargv[0], newargv);
    exit(-127);
}
	
