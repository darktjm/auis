/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
#include <andrewos.h>
#include <ms.h>

InventID(msg)
struct MS_Message *msg;
{
    debug(1, ("Invent ID\n"));

    strcpy(AMS_ID(msg->Snapshot), ams_genid(1));
    return(0);
}
