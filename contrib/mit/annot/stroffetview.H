#ifndef _stroffetview_H_
#define _stroffetview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup libstroffet
 * @{ */
#include <iconview.H>
/** A view for \ref stroffet objects.
 *  This looks like an anotation inset, and can be opened or closed
 *  (iconified in place) as needed.  Commands are added to open and close
 *  all such insets in the document, as well.  The text is added verbatim
 *  to the troff command stream when printed in troff mode, and is ignored
 *  otherwise.
 */
class stroffetview : public  iconview
{
public:
	TRACED_CLASS_NODEST(stroffetview);

 
     void Print(FILE *file, const char *processor, const char *finalFormat, boolean topLevel);
     void PostMenus(class menulist *menulist);
 
 
    
	stroffetview();
    
	static boolean InitializeClass();
	static boolean atk_inited;

 
	class menulist *menus;
};
/** @} */
#endif /* _stroffetview_H_ */
