/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include "champ.H"




ATKdefineRegistryNoInit(champ, ATK);

int champ::ReadDatesFromChampPath(const char  *path)
{
    return(::ReadDatesFromChampPath(path));
}

void champ::ClearAllFlaggedEvents()
{
    ::ClearAllFlaggedEvents();
}

int champ::FlagEventsMatchingDate(struct tm  *thisdate)
{
    return(::FlagEventsMatchingDate(thisdate));
}


void champ::IterateFlaggedEvents(champ_ifefptr proc, long  rock)
{
    ::IterateFlaggedEvents(proc, rock);
}

void champ::IncrementDate(struct tm  *d)
{
    ::IncrementDate(d);
}

struct eventnode *champ::ReadDateIntoEventNode(char  *str)
{
    return(readdateintoeventnode(str));
}

