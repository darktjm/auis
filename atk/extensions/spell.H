ATK_INTER
#ifndef _spell_H_
#define _spell_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */
#include <ATK.H>

class spell : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

    
        static void CheckDocument(class textview *self, long rock);
        
	static boolean InitializeClass();

};

#endif /* _spell_H_ */
