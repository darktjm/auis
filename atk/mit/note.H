#ifndef _note_H_
#define _note_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <icon.H>
class note : public  icon {
public:
	virtual ATKregistryEntry *ATKregistry();

 
    void SetChild(class dataobject * child);
 
 

	note();

	static boolean InitializeClass();
	static boolean atk_inited;

 
};


#endif /* _note_H_ */
