/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>    /* types */
#if defined(POSIX_ENV) || defined(M_UNIX)
#include <dirent.h>
#else
#define dirent direct
#endif
#include <fdplumbi.h>
#include <util.h>

#if SY_OS2
/* Reaching into the DIR struct is non-portable! */
#define dd_fd dd_id
#endif

DIR *dbg_opendir(const char *name)
{
    DIR *d;

    d = opendir(name);
#if 0 /* non-portable, not worth porting */
    if (d) {
	RegisterOpenFile(d->dd_fd, name, FDLEAK_OPENCODE_OPENDIR);
    }
#endif
    return(d);
}


void dbg_closedir(DIR *d)
{
#if 0 /* non-portable, not worth porting */
    RegisterCloseFile(d->dd_fd);
#endif
    closedir(d);
}


