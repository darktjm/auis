/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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

/* titles.c */

#include <stdio.h>
#include <util.h>

static char *BeginInitialArgv = NULL, *EndInitialArgv;

void SetInitialArgs(int argc, char **argv, char **envp)
{
    int Ix;

    BeginInitialArgv = argv[0];
    Ix = 0;
    if (envp != NULL) {
	for (Ix = 0; Ix < 60; ++Ix) if (envp[Ix] == NULL) break;
    }
    if (Ix > 0) {
	EndInitialArgv = envp[Ix-1] + strlen(envp[Ix-1]);
    } else {
	EndInitialArgv = argv[argc-1] + strlen(argv[argc-1]);
    }
}

#ifndef ANSI_COMPILER
#include <varargs.h>

/*VARARGS1*/
void SetProcTitle(va_alist)
va_dcl
{/* Set the process title. */
    char *cp, *str;
    char Title[1500];
    va_list ap;

    va_start(ap);
    str = va_arg(ap, char *);
    if (BeginInitialArgv == NULL) return;   /* must call SetInitialArgs first */
    sprintf(Title, str, ap);
    strncpy(BeginInitialArgv, Title, EndInitialArgv - BeginInitialArgv - 1);
    EndInitialArgv[-1] = '\0';
    cp = &BeginInitialArgv[strlen(BeginInitialArgv)];
    while (cp < EndInitialArgv) *cp++ = ' ';
    va_end(ap);
}
#else
#include <stdarg.h>

void SetProcTitle(const char *str, ...)
{/* Set the process title. */
    char *cp;
    char Title[1500];
    va_list ap;

    va_start(ap, str);
    if (BeginInitialArgv == NULL) return;   /* must call SetInitialArgs first */
    vsprintf(Title, str, ap);
    strncpy(BeginInitialArgv, Title, EndInitialArgv - BeginInitialArgv - 1);
    EndInitialArgv[-1] = '\0';
    cp = &BeginInitialArgv[strlen(BeginInitialArgv)];
    while (cp < EndInitialArgv) *cp++ = ' ';
    va_end(ap);
}
#endif

#ifdef TESTINGONLYTESTING
main(argc, argv)
int argc; char **argv;
{
    char Str[500];
    char *cp;

    SetInitialArgs(argc, argv, NULL);
    for (;;) {
	fputs("Title for process: ", stdout); fflush(stdout);
	cp = fgets(Str, sizeof(Str), stdin);
	if (cp == NULL) break;
	for (cp = &Str[strlen(Str)-1]; cp >= Str; --cp) if (*cp == '\n') *cp = '\0';
	SetProcTitle("%s", Str);
    }
    exit(0);
}
#endif /* TESTINGONLYTESTING */
