/* Copyright 1992 Carnegie Mellon University All rights reserved.
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
