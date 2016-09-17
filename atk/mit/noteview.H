#ifndef _noteview_H_
#define _noteview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

#include <iconview.H>
class noteview : public  iconview
{
public:
	virtual ATKregistryEntry *ATKregistry();

 
     void Print(FILE *file, const char *processor, const char *finalFormat, boolean topLevel);
     void PostMenus(class menulist *menulist);
 
 
    
	noteview();
    
	static boolean InitializeClass();
	static boolean atk_inited;

 
	class menulist *menus;
};

#endif /* _noteview_H_ */
