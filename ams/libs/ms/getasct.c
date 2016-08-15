/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
#include <andrewos.h>

static UNUSED const char getasct_rcsid[] = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/ams/libs/ms/RCS/getasct.c,v 2.4 1991/09/12 15:43:41 bobg Stab74 $";

/*
$Header: /afs/cs.cmu.edu/project/atk-src-C++/ams/libs/ms/RCS/getasct.c,v 2.4 1991/09/12 15:43:41 bobg Stab74 $
$Source: /afs/cs.cmu.edu/project/atk-src-C++/ams/libs/ms/RCS/getasct.c,v $
*/
#include <ms.h>

MS_GetAssociatedTime(FullName, Answer, lim)
char *FullName, *Answer;
int lim;
{
    debug(1, ("MS_GetAssociatedTime %s\n", FullName));
    return(GetAssocTime(FullName, Answer, lim));
}
