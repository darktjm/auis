/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("filler.H")
#include <filler.H>

/* return corresponding view name */


ATKdefineRegistryNoInit(filler, cel);


const char *				    /* returns "fillerview" */
filler::ViewName()
{
    return "fillerview";
}
