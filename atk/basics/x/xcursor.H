#ifndef _xcursor_H_
#define _xcursor_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
/* 
 *	cursor.ch
 */

#include <X11/X.h>
#include <X11/Xlib.h>
#include <rect.h>

#include <cursor.H>
class xcursor : public  cursor {
public:
	virtual ATKregistryEntry *ATKregistry();


	virtual void SetStandard(short ch);
	virtual void ChangeShape();

	virtual void Make(class xim *im);

	
	xcursor();
	
	virtual ~xcursor(); /*  used to get rid of active cursors */
	
	static boolean InitializeClass();

	static void FinalizeDisplay(Display *dpy);

	Cursor  Xc;
	Window Xw;
	Display * Xd;
	class xim *im;
	boolean valid;
};
/** @} */
#endif /* _xcursor_H_ */
