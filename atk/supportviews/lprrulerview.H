#ifndef _lprrulerview_H_
#define _lprrulerview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libsupviews
 * @{ */
/*
lprrulerview.H

	Class definitions for lprruler view

	displays and allows editting of the values for three icons: 
		leftmargin, rightmargin, and paragraph indent
	the measurement units toggle among Inches, Centimeters, and Points

	values are communicated with a client in fractional pixels: 
		the value is a long word with the binary point to the right of 1<<16.

	When there is a change of a value, the client can have a procedure called.
	The client calls lprrulerview_SetValueChangeProc(table, proc, rock).
		When there is a change of a value, a call to the proc is made with
		the following parameters:
			valuechangeproc(lprrulerview, rock, iconcode, newvalue)
		where rock is the value passed to SetValueChangeProc,
		iconcode is one of leftIcon, rightIcon, or paraIcon, and
		the new value is as for SetValue and GetValue

*/

#define  lprrulerview_PROGRAMMERVERSION    1

#define lprrulerview_NoValue (-999<<16)

#include <view.H>
#include <style.H>
enum lprrulerview_iconcode {leftIcon, rightIcon, paraIcon, noIcon} ;
struct lprrulerview_icondata {
    long value;		/* the value set by client */
    char icon;		/* the icon to display */
    short x;			/* the x value corresponding to value */
    short zero;		/* the zero x for ruler segment */
    short left;			/* left edge x of ruler segment */
    short right;		/* right edge x of ruler segment */
    short parkx;		/* the x value of center of parking lot */
    short parkoffset;		/* the offset from parkx to where to plot icon */
    boolean isBlack;		/* TRUE iff there is a value, but it is off the ruler */
    boolean icondirty;		/* TRUE iff needs to be plotted */
    boolean parkdirty;		/* TRUE iff park needs painted */
};

	typedef void (*lprrulerview_valuefptr)(class lprrulerview *self, long rock, enum lprrulerview_iconcode code, long value);

class lprrulerview : public  view {
public:
	virtual ATKregistryEntry *ATKregistry();

	 void FullUpdate( enum view_UpdateType type, 
		long left, long top, long width, long height );
	 void Update();
	 class view * Hit( enum view_MouseAction action, long x, long y, long n)
			 ;
	 view_DSattributes DesiredSize( long width, long height, enum view_DSpass pass, 
				long *desiredWidth, long *desiredHeight ) 
			 ;



	/* all values are pixel positions expressed as long int's with the binary point at 1<<16 */

	virtual void SetValues(/* struct lprrulerview *self, */ long leftmargin, long rightmargin, long paraindent);
		/* sets the values for the icon positions.  Values -999<<16 and lower indicate
			that no value is to be displayed */
	virtual void GetValues(/* struct lprrulerview *self, */ long *leftmargin, long *rightmargin, long *paraindent);
		/* sets the three parameters to the values of the icon positions */

	

	inline void SetValueChangeProc(lprrulerview_valuefptr proc, long rock)   /* store a proc to call when a value changes */   
				{ (this->ValueChangeRock = (long)rock, 
				this->ValueChangeProc = proc); }


	
	static boolean InitializeClass();

	
	lprrulerview();
	
	virtual ~lprrulerview();



	boolean OnScreen;			/* if not view_Removed */
	boolean rulerchanged, iconschanged, textchanged;	/* indicate redraw requirements */
	
	/* Image patterns might differ between instances  on different displays: */
	class graphic  *BlackPattern, *Grey25Pattern, *WhitePattern;	

	short leftline, rightline, bottomline, topline, middle, textloc;
	short leftzero, rightzero;
	struct TickTbl *TickTbl;

	lprrulerview_valuefptr ValueChangeProc;		/* called when a value changes */
		/* ValueChangeProc(lprrulerview, ValueChangeRock, iconcode, newvalue) */
	long ValueChangeRock;			/* passed as second arg to ValueChangeProc */

	enum style_Unit unit;

	enum lprrulerview_iconcode MovingIcon;
#define LEFT (self->iconloc[(short)leftIcon])
#define RIGHT (self->iconloc[(short)rightIcon])
#define PARA (self->iconloc[(short)paraIcon])

	struct lprrulerview_icondata iconloc[3];

	short icony;			/* baseline for icons */
};
/** @} */
#endif /* _lprrulerview_H_ */
