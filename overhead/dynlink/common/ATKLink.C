/* Copyright 1994 Carnegie Mellon University All rights reserved. */

#include "ATKLink.H"

int ATKLink::FirstObjectFile() {
    int i=0;
    ResetIterator();
    char *p;
    while((p=NextArgument())) {
	int len=strlen(p);
	if(p[len-1]=='o' && p[len-2]=='.') return i;
	i++;
    }
    return i;
}

int ATKLink::LastObjectFile() {
    int i=0, lastpos=0;
    ResetIterator();
    char *p;
    while((p=NextArgument())) {
	int len=strlen(p);
	if(p[len-1]=='o' && p[len-2]=='.') return lastpos=i;
	i++;
    }
    return i;
}


