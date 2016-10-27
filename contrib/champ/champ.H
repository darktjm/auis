#ifndef _champ_H_
#define _champ_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libchamp Object champ
 *  A dynamically loadable ATK class file.  Contains butter, butterview,
 *  champ, chimp, chimpview, chomp, chompview, enode, enodeview, month,
 *  monthview.
 * @{ */
#include "champ.h"

#include <ATK.H>
/** Load and check an event calendar.
 *  Objects of this class manage an event list.  It is loaded from a file
 *  in its own crontab-like format.  By default, this file is
 *  ~/events.champ or ~/.events.champ, whichever is found first.  However,
 *  that can be overridden with colon-separated list of file names in the
 *  ChampPath preference.
 */
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
/** @} */
#endif /* _champ_H_ */
