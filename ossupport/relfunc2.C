/* Copyright 1996 Carnegie Mellon University All rights reserved. */
#define IN_ATKOS_LIB 1
#include <system.h>
#include <relativize.H>
#include <atkos.h>

static struct nmlist {
    char *name;
    struct nmlist *next;
} *first=NULL;

static void EnterName(char *libname) {
    struct nmlist *result=new nmlist;
    result->next=first;
    result->name=strdup(libname);
    first=result;
}

static boolean CheckName(char *libname) {
    struct nmlist *f=first;
    while(f) {
	if(strcmp(f->name,libname)==0) return TRUE;
	f=f->next;
    }
    return FALSE;
}

static int lastwasout=0;
void ATKMinimizeLibs(char *arg, char *dot, char *slash) {
    if(!Dont(arg) && !lastwasout) {
	if(dot && strcmp(dot+1, "so")==0) {
	    if(CheckName(arg)) return;
	    EnterName(arg);
	    char filename[MAXPATHLEN];
	    *dot='\0';
	    if(slash) {
		*slash='\0';
		strcpy(filename, arg);
		strcat(filename, "/");
		if(strncmp(slash+1, "lib", 3)==0) strcat(filename, slash+4);
		*slash='/';
	    } else {
		if(strncmp(arg, "lib", 3)==0) strcpy(filename, arg+3);
	    }
	    *dot='.';
	    strcat(filename, ".lt");
	    FILE *fp=fopen(filename, "r");
	    if(fp!=NULL) {
		char libname[MAXPATHLEN+1], *p;
		int c=0;
		while(c!=EOF && (c=fgetc(fp))!=EOF) {
		    p=libname;
		    do {
			if(isspace(c)) break;
			*p++=c;
		    } while ((c=fgetc(fp))!=EOF);
		    *p='\0';
		    char *q=strrchr(libname, '.');
		    if(q && strcmp(q+1, "so")==0) {
			EnterName(libname);
		    }
		}
		fclose(fp);
	    }
	}
    }
    if(strcmp(arg,"-o")==0) lastwasout=1;
    else lastwasout=0;
    ProcessArg(arg, dot, slash);
}
