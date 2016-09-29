#ifndef _box_H_
#define _box_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup liblayout
 * @{ */
/* box.H - box data object definition and interface */

#include <rect.h>

#define box_VERSION 1
/* globals for entire package */

/* actual interface definition */

#include <dataobject.H>
class box: public  dataobject {
public:
	virtual ATKregistryEntry *ATKregistry();



   const char * ViewName()  ;
   long Read (FILE * f, long id)  ;
   long Write (FILE * f, long writeid, int level)  ;
   long GetModified()  ;


  virtual void ToggleDebug();
  virtual void FillInContents(const char *name);




  
	static boolean InitializeClass();

  
	box();
  
	virtual ~box();


  int inGetModified;			    /* recursion protection switch */
  class dataobject *contents;		    /* contained object */
};

/* end of box.H */
/** @} */
#endif /* _box_H_ */
