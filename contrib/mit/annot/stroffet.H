#ifndef _stroffet_H_
#define _stroffet_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

#include <icon.H>
class stroffet : public  icon {
public:
	virtual ATKregistryEntry *ATKregistry();

 
    void SetChild(class dataobject * child);
 
 

	stroffet();

	static boolean InitializeClass();
	static boolean atk_inited;

 
};


#endif /* _stroffet_H_ */