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

#include <andrewos.h>

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/ams/libs/ms/RCS/ckmiss.c,v 2.8 1992/12/15 21:18:15 rr2b Stab74 $";
#endif

#include <mailconf.h>
#include <ms.h>
#include <stdio.h>
#include <sys/stat.h>

/* This routine checks to see if a nonexistent folder has been replaced */

MS_CheckMissingFolder(OldName, NewName)
char *OldName; /* IN */
char *NewName; /* OUT */
{
    FILE *fp;
    struct stat statbuf;
    char LineBuf[500], *s, *t;
    int len;

    NewName[0] = '\0';
    len = strlen(OldName);
    if (len<=0) {
	AMS_RETURN_ERRCODE(EINVAL, EIN_PARAMCHECK, EVIA_CHECKMISSINGFOLDER);
    }
    fp = fopen(GlobalChangeSubsFile, "r");
    if (!fp) {
	if (errno == ENOENT) return(0); /* NO problem... */
	AMS_RETURN_ERRCODE(errno, EIN_FOPEN, EVIA_CHECKMISSINGFOLDER);
    }
    if (fstat(fileno(fp), &statbuf)) {
	fclose(fp);
	AMS_RETURN_ERRCODE(errno, EIN_STAT, EVIA_CHECKMISSINGFOLDER);
    }
    while (fgets(LineBuf,sizeof(LineBuf) -1, fp) != NULL) {
	if (LineBuf[len] == ' ' && !strncmp(LineBuf, OldName, len)) {
	    /* Found it!  Hoo-hah! */
	    s = LineBuf + len + 1;
	    t = strchr(s, ' ');
	    if (t) *t = '\0'; /* rest of line is comment */
	    t = strchr(s, '\t');
	    if (t) *t = '\0'; /* rest of line is comment */
	    t = strchr(s, '\n');
	    if (t) *t = '\0'; /* rest of line is comment */
	    strncpy(NewName, s, MAXPATHLEN);
	    fclose(fp);
	    return(0);
	}
    }
    fclose(fp);
    return(0);
}
