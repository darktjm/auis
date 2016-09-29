#ifndef _nullview_H_
#define _nullview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *	   Copyright Carnegie Mellon, 1992 - All Rights Reserved          *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/*
nullview.H

	Class definitions for null inset view

*/

#include <view.H>
class nullview  : public  view
{
public:
	virtual ATKregistryEntry *ATKregistry();



	 void FullUpdate(enum view_UpdateType type, long left, long top, long width, long height);
	 void Update();
	 class view * Hit(enum view_MouseAction action, long x, long y, long n)	 ;
	 view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long *desiredWidth, long *desiredHeight) 
				 ;
	 void ReceiveInputFocus();
	 void LoseInputFocus();
	 void ObservedChanged(class observable *dobj, long status);
	 void Print(FILE *file, const char *processor, const char *finalFormat, boolean topLevel);
	 void *GetPSPrintInterface(const char *printtype);
	 void PrintPSDoc(FILE *outfile, long pagew, long pageh);
	 void PrintPSRect(FILE *file, long logwidth, long logheight, struct rectangle *visrect);
	 class view *GetApplicationLayer();
	
	/* $$$ methods unique to this view.
		These would be defined if a parent view were to be given 
		greater control over this view. */
	
	static boolean InitializeClass();


	nullview();
	virtual ~nullview();

/* Create default Keymap & Menus */

	class menulist  *Menus;
	class keystate *Keystate;

	class graphic  *bpattern;	/* these might differ between instances . . . */
	class graphic  *wpattern;	/*		. . . on different displays */

	boolean OnScreen;			/* if not view_Removed */
	boolean embedded;		/* TRUE if no call to GetApplicationLayer */
	boolean HasInputFocus;		/* T if received input focus */
	boolean ignoreUp;			/* T iff have just asked for InputFocus */
	boolean sizeknown;		/* T if haven't called ComputeArea */

	/* $$$ some facts basic to computing desired size
		null chooses to have a given area if the parent
		constrains height or width */
	long DesiredArea;			/* total space desirable in square pixels */

	/* $$$ save desired size if the computation of it was expensive */
	long DesiredHeight, DesiredWidth;	/* requested dimensions; set by 
						ComputeArea*/
};

#endif /* _nullview_H_ */
