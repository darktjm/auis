/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
#include <andrewos.h>

MS_GetNextSubsEntry(FullName, NickName, status)
char *FullName; /* Value passed in to MS AND returned */
char *NickName; /* RETURN BUFFER returned from MS */
int *status; /* RETURN VALUE returned from MS */
{
    return(GetNextSubsEntry(FullName, NickName, status));
}
