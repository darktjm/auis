/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
static char *getnsub_rcsid = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/ams/libs/ms/RCS/getnsub.c,v 2.4 1991/09/12 15:44:08 bobg Stab74 $";

/*
$Header: /afs/cs.cmu.edu/project/atk-src-C++/ams/libs/ms/RCS/getnsub.c,v 2.4 1991/09/12 15:44:08 bobg Stab74 $
$Source: /afs/cs.cmu.edu/project/atk-src-C++/ams/libs/ms/RCS/getnsub.c,v $
*/
MS_GetNextSubsEntry(FullName, NickName, status)
char *FullName; /* Value passed in to MS AND returned */
char *NickName; /* RETURN BUFFER returned from MS */
int *status; /* RETURN VALUE returned from MS */
{
    return(GetNextSubsEntry(FullName, NickName, status));
}
