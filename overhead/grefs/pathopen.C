/* Copyright 1992 Carnegie Mellon University All rights reserved. */

#include <andrewos.h>
#include <util.h>
#ifndef PATHSEP
#ifdef DOS_STYLE_PATHNAMES
#define PATHSEP ';'
#else
#define PATHSEP ':'
#endif
#endif

#include "pathopen.h"

char pathopen_realpath[MAXPATHLEN+1];
FILE *pathopen(const char  *path, const char  *fpath, const char  *mode)
{
    const char *p=path;
    FILE *fp=NULL;
    while (*p && fp==NULL) {
	int len;
	const char *q=strchr(p, PATHSEP);
	if(q==NULL) len=strlen(p);
	else len = q-p;
	if(len+strlen(fpath)+2<MAXPATHLEN) {
	    strncpy(pathopen_realpath, p, len);
	    strcpy(pathopen_realpath+len, "/");
	    strcat(pathopen_realpath+len, fpath);
	    fp=fopen(pathopen_realpath, mode);
	}
	p+=len;
	if(*p) p++;
    }
    if(fp==NULL) {
	return NULL;
    }
    return fp;
}
