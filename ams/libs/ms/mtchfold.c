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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/ams/libs/ms/RCS/mtchfold.c,v 1.17 1993/06/15 03:56:02 rr2b Stab74 $";
#endif

#include <stdio.h>
#include <ms.h>
#include <mailconf.h>

long MS_MatchFolderName(pat, filename)
char *pat, *filename;
{
    int i, patlen;
    long errsave;
    FILE *wfp, *rfp;
    char SubsMapFile[1+MAXPATHLEN], LineBuf[3*MAXPATHLEN], *s;

    if (!pat) pat = "";
    GenTempName(filename);
    wfp = fopen(filename, "w");
    if (!wfp) {
	AMS_RETURN_ERRCODE(errno, EIN_FOPEN, EVIA_MATCHFOLDERNAME);
    }
    if (*pat == '/') { /* Searching in absolute path  -- might not even be on mspath. */
	char dirname[1+MAXPATHLEN];
	DIR *dirp;
	DIRENT_TYPE *dirent;
	int len;

	strcpy(dirname, pat);
	s = strrchr(dirname, '/');
	if (s) {
	    *s++ = '\0';
	} else {
	    strcpy(dirname, "/");
	    s = pat;
	}
	if ((dirp = opendir(dirname)) == NULL) {
	    errsave = errno;
	    fclose(wfp);
	    unlink(filename);
	    AMS_RETURN_ERRCODE(errsave, EIN_OPENDIR, EVIA_MATCHFOLDERNAME);
	}
	len = strlen(s);
	while ((dirent = readdir(dirp)) != NULL) {
	    if (!strncmp(dirent->d_name, s, len)) {
		fprintf(wfp, "%s/%s\n", dirname, dirent->d_name);
	    }
	}
	closedir(dirp);
    } else { /* Searching through mspath preference */
	for (s = pat; *s; ++s) {
	    if (*s == '/') *s = '.';
	}
	patlen = strlen(pat);
	for (i=0; i<MS_NumDirsInSearchPath; ++i) {
	    sprintf(SubsMapFile, "%s/%s", SearchPathElements[i].Path, AMS_SUBSCRIPTIONMAPFILE);
	    if (!(rfp = fopen(SubsMapFile, "r"))) {
		NonfatalBizarreError("Unreadable directory on mspath, ignoring!");
		continue;
	    }
	    while(fgets(LineBuf, sizeof(LineBuf), rfp) != NULL) {
		s = strchr(LineBuf, ':');
		if (s) *s = '\0';
		if ((strlen(LineBuf) >= patlen) && !strncmp(LineBuf, pat, patlen)) {
		    fputs(LineBuf, wfp);
		    fputs("\n", wfp);
		}
	    }
	    fclose(rfp);
	}
    }
    if (vfclose(wfp)) {
	errsave = errno;
	unlink(filename);
	AMS_RETURN_ERRCODE(errsave, EIN_VFCLOSE, EVIA_MATCHFOLDERNAME);
    }
    return(0);
}
