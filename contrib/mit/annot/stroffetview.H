#ifndef _stroffetview_H_
#define _stroffetview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

#include <iconview.H>
class stroffetview : public  iconview
{
public:
	virtual ATKregistryEntry *ATKregistry();

 
     void Print(FILE *file, const char *processor, const char *finalFormat, boolean topLevel);
     void PostMenus(class menulist *menulist);
 
 
    
	stroffetview();
    
	static boolean InitializeClass();
	static boolean atk_inited;

 
	class menulist *menus;
};

#endif /* _stroffetview_H_ */
