/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
#include <andrewos.h>
#include <ms.h>

MS_GetAssociatedTime(FullName, Answer, lim)
char *FullName, *Answer;
int lim;
{
    debug(1, ("MS_GetAssociatedTime %s\n", FullName));
    return(GetAssocTime(FullName, Answer, lim));
}
