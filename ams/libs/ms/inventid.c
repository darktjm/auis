/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
#include <andrewos.h>

static UNUSED const char inventid_rcsid[] = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/ams/libs/ms/RCS/inventid.c,v 2.5 1991/09/12 15:45:07 bobg Stab74 $";

/*
$Header: /afs/cs.cmu.edu/project/atk-src-C++/ams/libs/ms/RCS/inventid.c,v 2.5 1991/09/12 15:45:07 bobg Stab74 $
$Source: /afs/cs.cmu.edu/project/atk-src-C++/ams/libs/ms/RCS/inventid.c,v $
*/
#include <ms.h>

InventID(msg)
struct MS_Message *msg;
{
    debug(1, ("Invent ID\n"));

    strcpy(AMS_ID(msg->Snapshot), ams_genid(1));
    return(0);
}
