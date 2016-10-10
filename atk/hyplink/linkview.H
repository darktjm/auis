#ifndef _linkview_H_
#define _linkview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *	Copyright Carnegie Mellon University, 1996                        *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */
/** \addtogroup liblink
 * @{ */
#include <pshbttnv.H>
class linkview: public  pushbuttonview {
public:
	virtual ATKregistryEntry *ATKregistry();
	static boolean InitializeClass();
	linkview();
	virtual ~linkview();
 	
	void PostMenus(class menulist *ml);
	class view *Hit (enum view::MouseAction action, 
			long x, long y, long numberOfClicks)  ;

	virtual void Link();
	virtual void AutoLink();
	virtual void LinkFile(char *dest);
 
	class menulist *ml;
};
/** @} */
#endif /* _linkview_H_ */
