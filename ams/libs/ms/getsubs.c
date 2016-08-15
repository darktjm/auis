/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
#include <andrewos.h>

static UNUSED const char getsubs_rcsid[] = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/ams/libs/ms/RCS/getsubs.c,v 2.3 1991/09/12 15:44:21 bobg Stab74 $";

/*
$Header: /afs/cs.cmu.edu/project/atk-src-C++/ams/libs/ms/RCS/getsubs.c,v 2.3 1991/09/12 15:44:21 bobg Stab74 $
$Source: /afs/cs.cmu.edu/project/atk-src-C++/ams/libs/ms/RCS/getsubs.c,v $
*/
MS_GetSubscriptionEntry(FullName, NickName, status)
char *FullName; /* Value passed in to MS */
char *NickName; /* RETURN BUFFER returned from MS */
int *status; /* RETURN VALUE returned from MS */
{
    return(GetSubsEntry(FullName, NickName, status));
}

