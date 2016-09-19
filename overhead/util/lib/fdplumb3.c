/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include <stdio.h>
#include <fdplumbi.h>
#include <util.h>
 

FILE *dbg_popen(const char *path, const char *type)
{
    FILE *fp;
    extern FILE *popen();

    fp = popen(path, type);
    if (fp) RegisterOpenFile(fileno(fp), path, FDLEAK_OPENCODE_POPEN);
    return(fp);
}

int dbg_pclose(FILE *fp)
{
    RegisterCloseFile(fileno(fp));
    return(pclose(fp));
}

