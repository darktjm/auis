#ifndef _chimp_H_
#define _chimp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libchamp
 * @{ */
struct comment {
    char *line;
    struct comment *next;
};

#include <chlist.H>
/** A data object for storing event calendars.
 *  This is meant to be viewed using \ref chimpview.  The event calendar
 *  is written out and read in its internal text format.  It will wrap
 *  this so that it is a valid stream element if embedded as an inset.
 */
class chimp: public  chlist {
public:
	TRACED_CLASS_NODEST(chimp);
	chimp();

    
       long Read(FILE *file, long id)  ;
       long Write(FILE *file, long writeID, int level)  ;
    
      virtual void AddNew(struct eventnode *en);
    
      class enode *en;
      struct comment *comment;
};
/** @} */
#endif /* _chimp_H_ */
