ATK_INTER
#ifndef _tabrulerview_H_
#define _tabrulerview_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/



#define  tabrulerview_PROGRAMMERVERSION    1

#define tabrulerview_NoValue (-999<<16)

#include <view.H>
#include <lprrulerview.H>
typedef void (*tabrulerview_valuefptr)(class tabrulerview *self, long rock, enum lprrulerview_iconcode code, enum style_TabAlignment align, long newvalue);

class tabrulerview : public  view {
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


    virtual void SetValues(/* struct tabrulerview *self, */ class tabs *tabs);
    virtual void GetValues(/* struct tabrulerview *self, */ class tabs **tabs);
	
    inline void SetValueChangeProc(tabrulerview_valuefptr proc, long rock)   /* store a proc to call when a value changes */   
      { (this->ValueChangeRock = (long)rock, 
	this->ValueChangeProc = proc); }

    
	static boolean InitializeClass();

    
	tabrulerview();
    
	virtual ~tabrulerview();

    boolean OnScreen;			/* if not view_Removed */
    boolean tabrulerchanged;
    boolean iconschanged;
    boolean textchanged;	/* indicate redraw requirements */

    /* Image patterns might differ between instances  on different displays: */
    class graphic  *Grey25Pattern, *WhitePattern;

    long leftline, rightline, bottomline, topline, textloc;
    long cleartxt, canceltxt; /* Where to place the text */
    long clearpos, cancelpos; /* Where to place the button */
    long leftzero;
    struct TickTbl *TickTbl;

    tabrulerview_valuefptr ValueChangeProc;		/* called when a value changes */
    /* ValueChangeProc(tabrulerview, ValueChangeRock, iconcode, newvalue) */
    long ValueChangeRock;			/* passed as second arg to ValueChangeProc */

    enum style_Unit unit;
    class tabs *tabs;
    short icony;			/* baseline for icons */
    
    short Moving; /* 0 for none, or style_Left/RightAligned */
    long Movex;
    int oldtab;

    int mul; /* Used to scale the ruler to pretend it's real... */
    int div;
};

#endif /* _tabrulerview_H_ */
