ATK_INTER
#ifndef _controlV_H_
#define _controlV_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1989 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libvalue
 * @{ */
#include <buttonV.H>
class controlV : public  buttonV{
public:
	virtual ATKregistryEntry *ATKregistry();

    
       class valueview * DoHit(enum view_MouseAction type, long x,long y,long hits)  ;
       void LookupParameters();
       void DrawFromScratch(long x,long y,long width,long height);
    
      controlV();
	
	static boolean InitializeClass();

      static void InhibitAutoInit();
      static void SetAutoInit(boolean val);
    
      char *cclass;
      const char *function;
      boolean autoinit,needsinit;
};
/** @} */
#endif /* _controlV_H_ */
