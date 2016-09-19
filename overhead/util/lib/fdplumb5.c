/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include <stdio.h>
#include <fdplumbi.h>
#include <util.h>


int dbg_t2open(const char *name, char * const argv[], FILE **r, FILE **w)
{
    int code;

    code = t2open(name, argv, r, w);
    if (!code) {
	RegisterOpenFile(fileno(*w), "t2-r", FDLEAK_OPENCODE_T2OPEN);
	RegisterOpenFile(fileno(*r), "t2-w", FDLEAK_OPENCODE_T2OPEN);
    }
    return(code);
}


int dbg_t2close(FILE *fp, int seconds, int *timedout)
{
    RegisterCloseFile(fileno(fp));
    return(t2close(fp, seconds, timedout));
}


