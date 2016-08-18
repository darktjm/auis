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
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/ams/libs/ms/RCS/logging.c,v 2.11 1992/12/15 21:20:00 rr2b Stab74 $";
#endif

#include <ms.h>
#include <stdio.h>
#include <sys/stat.h>
#include <mail.h>

extern int IsLoggingMailStats;
extern char MAILLOGSTATFILE[];
extern char home[], Me[];

ConsiderLoggingRead(FileName)
char *FileName;
{
    char LineBuf[1000], *s;
    int size = 0;
    FILE *fp, *logfp;
    struct stat statbuf;

    if (!IsLoggingMailStats) return;
    logfp = fopen(MAILLOGSTATFILE, "a");
    if (!logfp) return;
#ifdef M_UNIX
    chmod(MAILLOGSTATFILE, 0600);
#else
    fchmod(fileno(logfp), 0600);
#endif

    fp = fopen(FileName, "r");
    if (!fp &&
	(RetryBodyFileName(FileName) < 0 || !(fp = fopen(FileName, "r")))) {
	fclose(logfp);
	return;
    }
    if (fstat(fileno(fp), &statbuf)) {
	fclose(logfp);
	return;
    }
    fputc('\n', logfp);
    while (fgets(LineBuf, sizeof(LineBuf), fp) && LineBuf[0] != '\n') {
	fputs(LineBuf, logfp);
	size += strlen(LineBuf);
    }
    fclose(fp);
    s = strrchr(FileName, '/');
    if (s) *s = '\0';
    fprintf(logfp, "X-StatTrace: %s READ %d bytes %s %s ; %s", Me, (int)(statbuf.st_size - size), strncmp(FileName, home, strlen(home)) ? "BBOARD" : "MAIL", FileName, arpadate());
    if (s) *s = '/';
    fclose(logfp);
}
