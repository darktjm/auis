#ifndef _ps_H_
#define _ps_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <icon.H>
class ps: public  icon {
public:
	virtual ATKregistryEntry *ATKregistry();

 
    void SetChild(class dataobject * child);
     long Read (FILE *file, long id)  ;
     long Write (FILE *file, long writeid, int level)  ;
 
 
    inline long   GetPixelWidth() { return ((this)->pwidth); }
    inline long   GetPixelHeight() { return ((this)->pheight); }
    inline void SetPixelWidth(long value)  { ((this)->pwidth=(value)); }
    inline void SetPixelHeight(long value)  { ((this)->pheight=(value)); }
 
    
	ps();
    
	static boolean InitializeClass();
	static boolean atk_inited;

 
    long pwidth;
    long pheight;
};


#endif /* _ps_H_ */
