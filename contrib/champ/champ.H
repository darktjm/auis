#ifndef _champ_H_
#define _champ_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */
#include "champ.h"

#include <ATK.H>
class champ : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      static int ReadDatesFromChampPath(const char *path)  ;
      static void ClearAllFlaggedEvents();
      static int FlagEventsMatchingDate(struct tm *thisdate)  ;
      static void IterateFlaggedEvents(champ_ifefptr proc, long rock);
      static void IncrementDate(struct tm *thisdate);
      static struct eventnode * ReadDateIntoEventNode(char *str)  ;
};


#endif /* _champ_H_ */
