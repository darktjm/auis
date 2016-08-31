/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
#include <andrewos.h>
#include <ms.h>

MS_GetAssociatedFileTime(FullName, fdate)
char *FullName;
long *fdate;
{
    debug(1, ("MS_GetAssociatedFileTime %s\n", FullName));
    return(GetAssocFileTime(FullName, fdate));
}
