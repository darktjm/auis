#ifndef _month_H_
#define _month_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libchamp
 * @{ */
#include <dataobject.H>
class month: public dataobject {
public:
	virtual ATKregistryEntry *ATKregistry();
	month();

    
       long Read (FILE *file, long id)  ;
       long Write (FILE *file, long writeid, int level)  ;
    
      virtual void SetMonthAndYear(int mon, int year);
    
      inline int GetMonth() { return (this->mon); }
      inline int GetYear() { return (this->year); }
    
      int mon, year;
};
/** @} */
#endif /* _month_H_ */
