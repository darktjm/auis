#ifndef _logview_H_
#define _logview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libconsoleapp
 * @{ */
#include <textview.H>
class logview : public  textview {
public:
	virtual ATKregistryEntry *ATKregistry();


     void LoseInputFocus();
     void PostMenus(class menulist *menulist);
     void FrameDot(long pos);

    logview();
	
	static boolean InitializeClass();
	
	virtual ~logview();

    class menulist *menu;
};
/** @} */
#endif /* _logview_H_ */
