/* Copyright 1994.  Carnegie Mellon University. All rights reserved. */

// ATKArgv: a class for manipulating C style argument lists

#include <andrewos.h>
#include <util.h>
#include <ctype.h>

#include "ATKArgv.H"

const int ATKArgv::End=-1;
const int ATKArgv::CountArguments=-1;

int ATKArgv::CountArgs(char **args) {
    int count=0;
    if(args==NULL) return 0;
    while(*args) {
	count++;
	args++;
    }
    return count;
}

void ATKArgv::InsertArguments(char **args, int pos, int count) {
    // if no count was given count the args assuming they are NULL
    // terminated.
    if(count==CountArguments) {
	count=CountArgs(args);
    }
    // if no position was given assume the end of the current list.
    if(pos==End) pos=argc;

    // check whether more space is needed to hold the arguments
    int newsize=argc+count+1;
    if(newsize>=size) {
	if(argv) argv=(char **)realloc(argv, newsize*sizeof(char *));
	else argv=(char **)malloc(newsize*sizeof(char *));
	if(argv==NULL) {
	    size=0;
	    argc=0;
	    error=1;
	    return;
	}
	size=newsize;
    }

    // Shift up any arguments after pos to make space for the new arguments
    if(argc-pos>0) {
	memmove(argv+pos+count, argv+pos, (argc-pos)*sizeof(char *));
    }

    // Add the arguments.
    argc+=count;
    while(count>0 && *args) {
	argv[pos]=strdup(*args);
	if(argv[pos]==NULL) {
	    error=1;
	    return;
	}
	args++;
	pos++;
	count--;
    }
    argv[argc]=NULL;
}

void ATKArgv::DeleteArguments(int pos, int count) {
    // Trying to delete arguments when there are none is an error
    if(argv==NULL) {
	error=1;
	return;
    }
    // If no count was given assume that all arguments at and after pos
    // should be deleted.
    if(count==CountArguments) count=argc-pos;
    // If the region to be deleted exceeds the size of the list there is an
    // error.
    if(pos+count>argc) {
	error=1;
	return;
    }
    // If there are some arguments after the deleted region shift them
    // down.
    if(pos+count<argc) {
	memmove(argv+pos, argv+pos+count, sizeof(char *)*(argc-(pos+count)));
    }
    argc-=count;
    argv[argc]=NULL;
}
char *ATKArgv::SystemString(boolean quote) {
    static char *buf=NULL;
    static int bsize=0;
    int newsize=0;
    char *p, *q;
    if(error) return NULL;
    ResetIterator();
    while((p=NextArgument())) {
	// a simple conservative estimate.
	newsize+=1+quote?strlen(p)*2:strlen(p);
    }
    newsize++;
    if(newsize>bsize) {
	if(buf) buf=(char *)realloc(buf, newsize);
	else buf=(char *)malloc(newsize);
    }
    if(buf==NULL) return NULL;
    ResetIterator();
    q=buf;
    while((p=NextArgument())) {
	if(q!=buf) *q++=' ';
	while(*p) {
	    if(quote && !isalnum(*p)) *q++='\\';
	    *q++=(*p++);
	}
    }
    *q='\0';
    return buf;
}

ATKArgv::ATKArgv(char **args, int count) {
    argv=NULL;
    argc=0;
    error=0;
    size=0;
    iterator=(-1);
    InsertArguments(args, 0, count);
}

ATKArgv::ATKArgv(char *arg)  {
    argv=NULL;
    argc=0;
    error=0;
    size=0;
    iterator=(-1);
    InsertArguments(&arg, 0, 1);
}

ATKArgv::ATKArgv() {
    argv=NULL;
    argc=0;
    error=0;
    size=0;
    iterator=(-1);
}
    

ATKArgv::~ATKArgv() {
    if(argv!=NULL) {
	int i;
	for(i=0;i<argc;i++) {
	    if(argv[i]) free(argv[i]);
	}
    }      
}

