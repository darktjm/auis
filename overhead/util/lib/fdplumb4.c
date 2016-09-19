/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include <stdio.h>
#include <fdplumbi.h>
#include <util.h>
 

FILE *dbg_qopen(const char *path, const char * const argv[], const char *mode)
{
    FILE *fp;

    fp = (FILE *) qopen(path, argv, mode);
    if (fp) RegisterOpenFile(fileno(fp), path, FDLEAK_OPENCODE_QOPEN);
    return(fp);
}

FILE *dbg_topen(const char *path, const char * const argv[], const char *mode, int *pgrp)
{
    FILE *fp;

    fp = topen(path, argv, mode, pgrp);
    if (fp) RegisterOpenFile(fileno(fp), path, FDLEAK_OPENCODE_TOPEN);
    return(fp);
}

int dbg_qclose(FILE *fp)
{
    RegisterCloseFile(fileno(fp));
    return(qclose(fp));
}

int dbg_tclose(FILE *fp, int seconds, int *timedout)
{
    RegisterCloseFile(fileno(fp));
    return(tclose(fp, seconds, timedout));
}

