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
	TRACED_CLASS(logview);


     void LoseInputFocus();
     void PostMenus(class menulist *menulist);
     void FrameDot(long pos);

    logview();
	
	static boolean InitializeClass();
	

    class menulist *menu;
};
/** @} */
#endif /* _logview_H_ */
