#ifndef _labelview_H_
#define _labelview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libsupviews
 * @{ */
/*
labelview.H

	Class definitions for label view


	A labelview displays the text stored by a label data object.
	In addition, the labelview can provide image inversion and a hit proc.
	The HitProc is called whenever there is a mouse action on the 
	text within the label image.  The proc is called with these parameters:
		HitProc(labelview, mouseaction, rock)
	where the rock is a 32 bit value passed as a parameter to SetHitProc.
*/

#define  labelview_PROGRAMMERVERSION    1

#include <view.H>

	typedef void (*labelview_hitfptr)(class labelview *self, enum view_MouseAction action, char *rock);

class labelview : public  view {
public:
	virtual ATKregistryEntry *ATKregistry();


	 void FullUpdate(/* struct labelview *self, */ enum view_UpdateType type, 
			long left, long top, long width, long height );
	 void Update(/* struct labelview *self, */);
	 class view * Hit(/* struct labelview *self, */ enum view_MouseAction action, 
			long x, long y, long n)	 ;
	 view_DSattributes DesiredSize(/* struct labelview *self, */ long width, long height, 
			enum view_DSpass pass, long *desiredWidth, long *desiredHeight ) 
			 ;
	 void ObservedChanged(/* struct labelview *self, */ class observable *dobj, long status );
	 void Print(/* struct labelview *self, */ FILE *file, 
			const char *processor, const char *finalFormat, boolean topLevel );
	 class view * GetApplicationLayer(/* struct labelview *self */)  ;
	


	virtual void SetHitProc(/* struct labelview *self, */ labelview_hitfptr proc, char *rock);
	virtual char * GetHitRock(/* struct labelview *self */)  ;
	virtual void SetInversion(/* struct labelview *self, */ boolean invert);
	virtual boolean GetInversion(/* struct labelview *self */)  ;



	/* the following is a macromethod because the class processor cannot handle 
			"  returns void(*)()  "		XXX
	*/
	inline labelview_hitfptr GetHitProc() { return (this->hitproc); }



	
	labelview();
	
	virtual ~labelview();



	boolean GaveSize;			/* set after successful DesiredSize */
	
	class graphic  *BlackPattern;	/* these might differ between instances . . . */
	class graphic  *WhitePattern;	/*		. . . on different displays */

	boolean embedded;		/* TRUE if no call to GetApplicationLayer */
	boolean OnScreen;			/* if not view_Removed */

	labelview_hitfptr hitproc;			/* called for a mouse hit */
	char *hitrock;				/* passed to the hit proc */
	boolean inverted;			/* TRUE if image is supposed to be inverted */
	long minwidth, minheight;		/* minimum width and height of the text */
};
/** @} */
#endif /* _labelview_H_ */
