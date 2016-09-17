ATK_INTER
#ifndef _bp_H_
#define _bp_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */
 
#include <dataobject.H>

#define bp_DATACHANGED 1

class bp : public  dataobject  {
    public:
      virtual ATKregistryEntry *ATKregistry();
      bp();

      const char * ViewName()  ;
      long Read (FILE *file, long id)  ;
      long Write (FILE *file, long writeid, int level)  ;

      virtual void SetPageNum(long newval);
      virtual void ClearPageNum();

      boolean haspagenum; /* flag whether this page break specifies the next page's number */
      long pagenum; /* page number of next page, if haspagenum is TRUE */
};


#endif /* _bp_H_ */
