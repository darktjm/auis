/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/supportviews/RCS/lprrulerview.C,v 3.4 1994/11/30 20:42:06 rr2b Stab74 $";
#endif


 

/* lprrulv.c	

	The view module for the lprruler dataobject

deferred features:
	adjust the location of zero depending on the values
	click ruler to set margin to that position
	click icon to just display its value
	click park to select icon and display its value
	keystrokes for fine adjustment of the most recently moved icon
	the text of the size number should be editable as simpletext
	the conversion between CM and PTS is unstable so the values get smaller
	should an icon "jump" when button is pressed down?
		We can avoid this jump by computing an offset from the x 
		and using that offset while the button is down 

*/


#include <andrewos.h>
ATK_IMPL("lprrulerview.H")
#include <graphic.H>
#include <view.H>
#include <fontdesc.H>

#define class_StaticEntriesOnly
#include <style.H>
#undef  class_StaticEntriesOnly


/*
#include <menulist.ih>
#include <keymap.ih>
#include <keystate.ih>
*/

#include <im.H>
#include <rect.h>

#include <lprruler.H>
#include <lprrulerview.H>

/*
static struct menulist  *class_menulist;
static struct keymap  *class_keymap;
static struct keystate  *class_keystate;
*/


 

#define GAP 10		/*C*//* width of gap between rulers */
#define ICONHEIGHT 14	/*C*//* height of area where icons move */
#define PARKWIDTH  19	/*C*//* width of icon parking area */
#define RULERHEIGHT 22 /*C*//* bottom line = topline + RULERHEIGHT */

static class fontdesc *TextFont, *IconFont;


struct TickTbl {
	short majorpix, minorpix;/* number of pixels for major and minor cycles */
	short one;			/* increment for -zero- value */
	short ht[10];		/* height of each tick 
						cyclecnt==0 is the first tick 
						and is where numeric value is displayed */
	char fmt[10];		/* format for text area */
	char unitstring[10];	/* units name to be displayed */
};

static struct TickTbl InchTbl = {72, 9, 1, {6, 3, 5, 3, 7, 3, 5, 3}, "%+5.2f i", "In."};
static struct TickTbl PointTbl = {36, 6, 36, {6, 3, 3, 5, 3, 3}, "%+3.0f p", "Pts."};
static struct TickTbl CMTbl = {30, 6, 1, {6, 3, 3, 3, 3}, "%+5.2f c", "Cm."};



ATKdefineRegistry(lprrulerview, view, lprrulerview::InitializeClass);
#ifndef NORCSID
#endif
 ;
static boolean BogusCallFromParent(class lprrulerview  *self, char  *where , char  *msg);
static boolean CheckWindow(class lprrulerview  *self, char  *where);
static void RepaintIcon(class lprrulerview   *self, enum lprrulerview_iconcode  icon, short  color);
static void RepaintPark(class lprrulerview   *self, enum lprrulerview_iconcode  icon, short  color);
static void RecomputeIconX(class lprrulerview  *self, enum lprrulerview_iconcode  icon);
static void BoundIcon(class lprrulerview  *self, enum lprrulerview_iconcode  icon, short  zeroloc , short  left , short  right);
static void CheckBounds(class lprrulerview  *self);
static void CleanUpIconArea(class lprrulerview  *self);
static void RemoveIcon(class lprrulerview  *self, enum lprrulerview_iconcode  icon);
static void MoveLeftIcon(class lprrulerview  *self, long  newx);
static void MoveRightIcon(class lprrulerview  *self, long  newx);
static void MoveParaIcon(class lprrulerview  *self, long  newx);
static void DoTicks(class lprrulerview  *self, short  zeroloc , short  left, short  right, struct TickTbl  *tbl);
static void RedrawRuler(class lprrulerview  *self);
static void RedrawText(class lprrulerview   *self);
static void RedrawIcons(class lprrulerview   *self);


static boolean
BogusCallFromParent(class lprrulerview  *self, char  *where , char  *msg)
		{
	fprintf(stderr, "<lprrulerview>Bogus call to %s, %s\n", where, msg);
	return FALSE;
}

static boolean
CheckWindow(class lprrulerview  *self, char  *where)
		{
	class graphic *g
		= (class graphic *)(self)->GetDrawable();
	if ( ! g) return BogusCallFromParent(self, where, "No Graphic");
	return TRUE;
}



boolean
lprrulerview::InitializeClass()
	{
	TextFont = fontdesc::Create("andysans", fontdesc_Bold, 12);
	IconFont = fontdesc::Create("icon", fontdesc_Plain, 12);
	return TRUE;
}

lprrulerview::lprrulerview()
		{
	ATKinit;
	class lprrulerview *self=this;
	struct lprrulerview_icondata *i;
	long cnt;
	this->OnScreen = FALSE;
	this->rulerchanged = this->iconschanged = this->textchanged = TRUE;
	this->unit = style_Inches;
	this->TickTbl = &InchTbl;
	this->ValueChangeProc = NULL;
	for (i = &(this->iconloc[(short)leftIcon]), cnt = 3; cnt--; i++) {
		i->value = lprrulerview_NoValue;
		i->parkdirty = TRUE;
		i->icondirty = TRUE;
		i->isBlack = FALSE;
		RecomputeIconX(this, (enum lprrulerview_iconcode)(i - this->iconloc));
	}
	LEFT.icon = '(';   LEFT.parkoffset = 4;	/*C*/
	RIGHT.icon = ')';   RIGHT.parkoffset = -4;	/*C*/
	THROWONFAILURE( TRUE);
}

lprrulerview::~lprrulerview()
		{
	ATKinit;

}
#if 0
XXX
/* not declared as an override... probably
wasn't being used ever.... */ 
void
lprrulerview::ObservedChanged(class observable  *dobj, long  status)
			{
	if (status == lprruler_DATACHANGED) 
		this->iconschanged = this->textchanged = TRUE;
	else if (status == observable_OBJECTDESTROYED)
		return;
	(this)->WantUpdate( this);
}
XXX
#endif 
static void
RepaintIcon(class lprrulerview   *self, enum lprrulerview_iconcode  icon, short  color)
			{
	struct lprrulerview_icondata *i = &(self->iconloc[(short)icon]);
	(self)->SetTransferMode( color);
	(self)->MoveTo( i->x, self->icony);
	(self)->DrawText( &i->icon, 1, graphic_NOMOVEMENT);
}
static void
RepaintPark(class lprrulerview   *self, enum lprrulerview_iconcode  icon, short  color)
			{
	struct lprrulerview_icondata *i = &(self->iconloc[(short)icon]);
	struct rectangle r;		/* rectangle for parking lot */
	r. height = ICONHEIGHT+4, r.width = PARKWIDTH;	/*C*/
	r.top = self->icony - (ICONHEIGHT>>1) - 1;		/*C*/
	r.left = i->parkx - (PARKWIDTH>>1);
	(self)->SetTransferMode( color);
	switch (color) {
	    case graphic_COPY:
		(self)->FillRect( &r, self->Grey25Pattern);
		break;
	    case graphic_WHITE:
		(self)->FillRect( &r, self->WhitePattern);
		break;
	    case graphic_INVERT:
	    case graphic_BLACK:
	    default:
		(self)->FillRect( &r, self->BlackPattern);
		break;
	}
}

/* compute the x coordinate for an icon */
static void
RecomputeIconX(class lprrulerview  *self, enum lprrulerview_iconcode  icon)
		{
	struct lprrulerview_icondata *i = &(self->iconloc[(short)icon]);
	if (icon == paraIcon) {
		long pv = PARA.value;
		if (pv > lprrulerview_NoValue && pv < 0L) {
			/* para exdent icon */
			PARA.icon = '+';
			PARA.parkoffset = -4;
		}
		else {
			/* para indent icon */
			PARA.icon = '*';
			PARA.parkoffset = 0;
		}
	}
	if (i->value <= lprrulerview_NoValue || i->isBlack)
		i->x = i->parkx + i->parkoffset;
	else if (icon == paraIcon && LEFT.value > lprrulerview_NoValue)
		i->x = LEFT.x +  (i->value>>16);
	else
		i->x = i->zero + (i->value>>16);
}
/* set the range bounds for an icon */
static void
BoundIcon(class lprrulerview  *self, enum lprrulerview_iconcode  icon, short  zeroloc , short  left , short  right)
			{
	struct lprrulerview_icondata *i = &(self->iconloc[(short)icon]);
	i->zero = zeroloc;
	i->left = left;
	i->right = right;
}
/* check each value to set isBlack for the icon.  
	Remove icon if blackness changes */
static void
CheckBounds(class lprrulerview  *self)
	{
	struct lprrulerview_icondata *i;
	long cnt;
	for (i = &(self->iconloc[(short)leftIcon]), cnt = 3; cnt--; i++) {
		boolean wasBlack = i->isBlack;
		if (i->value <= lprrulerview_NoValue)
			i->isBlack = FALSE;
		else
			i->isBlack = (i->x < i->left || i->x > i->right);
		if (i->isBlack != wasBlack) {
			enum lprrulerview_iconcode icon = (enum lprrulerview_iconcode)(i - self->iconloc);
			RemoveIcon(self, icon);
			RecomputeIconX(self, icon);
			i->parkdirty = TRUE;
		}
	}
			
}

/* check dirty bits and repaint all parks and icons that claim to need it 
	never modifies isBlack */
static void
CleanUpIconArea(class lprrulerview  *self)
	{
	struct lprrulerview_icondata *i;
	long cnt;
	for (i = &(self->iconloc[(short)leftIcon]), cnt = 3; cnt--; i++)
		if (i->parkdirty)
			RepaintPark(self, (enum lprrulerview_iconcode)(i - self->iconloc), 
				(i->isBlack) ? graphic_WHITE : graphic_COPY);
	for (i = &(self->iconloc[(short)leftIcon]), cnt = 3; cnt--; i++)
		if (i->icondirty) {
			i->icondirty = FALSE;
			RepaintIcon(self, (enum lprrulerview_iconcode)(i - self->iconloc), graphic_COPY);
		}
	for (i = &(self->iconloc[(short)leftIcon]), cnt = 3; cnt--; i++)
		if (i->parkdirty) {
			i->parkdirty = FALSE;
			if (i->isBlack)
				RepaintPark(self, (enum lprrulerview_iconcode)(i - self->iconloc), graphic_INVERT);
		}
	self->iconschanged = FALSE;
}

/* paint an icon white and set dirty bits for all icons and parks that may be affected */
static void
RemoveIcon(class lprrulerview  *self, enum lprrulerview_iconcode  icon) 
		{
	struct lprrulerview_icondata *i;
	long cnt;
	short xlo, xhi;
	RepaintIcon(self, icon, graphic_WHITE);
	i = &(self->iconloc[(short)icon]);
	i->icondirty = TRUE;
	xlo = i->x - 20;		/*C*/
	xhi = i->x + 20;		/*C*/
	for (i = &(self->iconloc[(short)leftIcon]), cnt = 3; cnt--; i++) {
		/* in the general case we here have an N-squared problem.
			We have to check whether replotting any one symbol
			requires replotting any other.  For now we cheat and 
			and assume that the only overlap is between parks
			and their own icons. */
		if (xlo <= i->x && i->x <= xhi)   i->icondirty = TRUE;
		if (xlo <= i->parkx && i->parkx <= xhi)   i->parkdirty = TRUE;
		if (i->icondirty != i->parkdirty 
				&& i->parkx - 20 < i->x && i->x < i->parkx + 20)
			i->parkdirty = i->icondirty = TRUE;
	}
}

static void
MoveLeftIcon(class lprrulerview  *self, long  newx)
		{
	long deltav = (newx - LEFT.x)<<16;
	RemoveIcon(self, leftIcon);
	LEFT.value += deltav;
	RecomputeIconX(self, leftIcon);
	if (PARA.value > lprrulerview_NoValue) {
		/* don't update PARA.x.  It is set from LEFT.x+PARA.value */
		RemoveIcon(self, paraIcon);
		RecomputeIconX(self, paraIcon);
	}
}
static void
MoveRightIcon(class lprrulerview  *self, long  newx)
		{
	long deltav = (newx - RIGHT.x)<<16;
	RemoveIcon(self, rightIcon);
	RIGHT.value += deltav;
	RecomputeIconX(self, rightIcon);
}
static void
MoveParaIcon(class lprrulerview  *self, long  newx)
		{
	long deltav = (newx - PARA.x)<<16;
	RemoveIcon(self, paraIcon);
	PARA.value += deltav;
	RecomputeIconX(self, paraIcon);
}


/* Draw tick marks in a ruler for -self- on the topline, from -left- to -right-
	the value zero would be plotted at location zeroloc, which may be off
		the ruler to either end
	the length of tick marks is given by -tbl-, with one entry for
		each minor cycle within a major cycle.
	The lengths of the cycles are given by -major- and -minor-
	assume major % minor == 0 */
static void
DoTicks(class lprrulerview  *self, short  zeroloc , short  left, short  right, struct TickTbl  *tbl)
				{
	short cycmax = tbl->majorpix / tbl->minorpix;	/* number of minor cycles in a major */
	short tickloc;					/* where to place next tick */
	short cyclecnt;				/* count minor ticks within major cycle */
	short ordloc;						/* where to first plot an ordinate value */
	short ordval;						/* next ordinate value to plot */

	/* since division and remainder are ill-defined for negative operands:
		 the "<<12" items arrange that the %'s are done on positive operands
		the divisions always come out to exact values 
	*/
	tickloc = left + (zeroloc + (tbl->minorpix << 12) - left) % tbl->minorpix;
	ordloc = left + (zeroloc + (tbl->majorpix << 12) - left) % tbl->majorpix;
	ordval = (ordloc - zeroloc) / tbl->majorpix * tbl->one;
	cyclecnt = cycmax - (ordloc-tickloc)/tbl->minorpix;
	if (cyclecnt == cycmax) cyclecnt = 0;

	(self)->SetFont( TextFont);
	for ( ; tickloc < right; tickloc += tbl->minorpix) {
		(self)->MoveTo( tickloc, self->topline);
		(self)->DrawLineTo( tickloc, self->topline + tbl->ht[cyclecnt]);
		if (cyclecnt == 0) {
			/* draw digit */
			char buf[10];
			if (ordval > 0)
				sprintf(buf, "%+d", ordval);
			else sprintf(buf, "%d", ordval);	/* no sign if ordval==0 */
			(self)->MoveTo( tickloc-1, self->topline + 7);	/*C*/
			(self)->DrawString( buf,
				graphic_BETWEENLEFTANDRIGHT | graphic_ATTOP);
			ordval += tbl->one;
		}
		if (++cyclecnt >= cycmax) cyclecnt = 0;
	}
}

static void
RedrawRuler(class lprrulerview  *self)
	{
	struct rectangle r;
	short extra;
	short middle, left, right;	/* bounds for tick marks NOT FOR ICON PAINTING */

	(self)->GetLogicalBounds( &r);

	/* the following section computes the layout parameters
		it is full of artificial constants *//*C*/

	if (r.width < 200) r.width = 200;	/* truncate from the right if too narrow */
	/* leave space at the left for "parking" two icons and text underneath them
		leave space at the right for parking one icon */
	self->leftline = 70;
	self->rightline = r.width - PARKWIDTH - 10;
	self->middle = (r.width - 80 - PARKWIDTH - 10) * 15 / 24;
	self->textloc = 20;   /* with icon for unit change to its left */
	
	/* leave at least 16 pixels above the ruler for the icons */
	extra = r.height - ICONHEIGHT - 2 - RULERHEIGHT;
	if (extra < 0) extra = 0;		/* truncate at bottom if too short */
	self->topline = ICONHEIGHT + 2 + (extra>>1);	
	self->bottomline = self->topline + RULERHEIGHT;
	self->icony = self->topline - (ICONHEIGHT>>1) - 4;
	LEFT.parkx = self->leftline - 3 - (PARKWIDTH>>1);
	PARA.parkx = LEFT.parkx - 2 - PARKWIDTH;
	RIGHT.parkx = self->rightline + 3 + (PARKWIDTH>>1);
	
	(self)->SetTransferMode( graphic_COPY);
	(self)->FillRect( &r, self->WhitePattern);

	/* draw lprruler pieces */
	(self)->MoveTo( self->leftline, self->topline);
	(self)->DrawLineTo( self->middle, self->topline);
	(self)->DrawLineTo( self->middle+4, self->topline + 10);
	(self)->DrawLineTo( self->middle-2, self->topline + 15);
	(self)->DrawLineTo( self->middle, self->bottomline);
	(self)->DrawLineTo( self->leftline, self->bottomline);

	(self)->MoveTo( self->rightline, self->topline);
	(self)->DrawLineTo( self->middle+GAP, self->topline);
	(self)->DrawLineTo( self->middle+GAP+4, self->topline + 10);
	(self)->DrawLineTo( self->middle+GAP-2, self->topline + 15);
	(self)->DrawLineTo( self->middle+GAP, self->bottomline);
	(self)->DrawLineTo( self->rightline, self->bottomline);

	left = self->leftline+2;			/*C*/
	middle = self->middle-2;		/*C*/
	right = self->rightline-2;		/*C*/
	self->leftzero = (middle - left) * 3 / 10 + self->leftline;
	self->rightzero = (right - (middle + 4/*C*/ + GAP)) * 6 / 10 + middle + GAP + 2;

	DoTicks(self, self->leftzero, left, middle, self->TickTbl);
	DoTicks(self, self->rightzero, middle + 4/*C*/ + GAP, right, self->TickTbl);
	self->rulerchanged = FALSE;
}
static void
RedrawText(class lprrulerview   *self)
	{
	struct rectangle r;
	r.top = self->topline, r.left = self->textloc;
	r.height = RULERHEIGHT, r.width = self->leftline - self->textloc;	
	(self)->SetTransferMode( graphic_COPY);
	(self)->FillRect( &r, self->WhitePattern);
	(self)->SetFont( TextFont);
	(self)->MoveTo( self->textloc, self->bottomline - 6/*C*/);
	(self)->DrawString( self->TickTbl->unitstring, 
				graphic_ATLEFT |  graphic_ATBASELINE);

	/* display icon for changing unit */
	(self)->SetFont( IconFont);
	(self)->MoveTo( self->textloc - 12, self->bottomline - 12);		/*C*/
	(self)->DrawText( "\'", 1, graphic_NOMOVEMENT);
	self->textchanged = FALSE;
}
static void
RedrawIcons(class lprrulerview   *self)
	{
	struct lprrulerview_icondata *i;
	long cnt;
	struct rectangle r;
	r.top = self->topline - ICONHEIGHT - 4/*C*/, r.left = 0;
	r.height = ICONHEIGHT + 4/*C*/, r.width = self->rightline + PARKWIDTH + 4/*C*/;	
	(self)->SetTransferMode( graphic_COPY);
	(self)->FillRect( &r, self->WhitePattern);
	(self)->SetFont( IconFont);
	BoundIcon(self, leftIcon, self->leftzero, self->leftline, self->middle);
	BoundIcon(self, paraIcon, self->leftzero, self->leftline, self->middle);
	BoundIcon(self, rightIcon, self->rightzero, self->middle+GAP, self->rightline);
	
	for (i = &(self->iconloc[(short)leftIcon]), cnt = 3; cnt--; i++) {
		i->parkdirty = i->icondirty = TRUE;
		i->isBlack = FALSE;
		RecomputeIconX(self, (enum lprrulerview_iconcode)(i - self->iconloc));
	}
	CheckBounds(self);
	CleanUpIconArea(self);
}

void 
lprrulerview::FullUpdate( enum view_UpdateType   type, long   left , long   top , long   width , long   height )
			{
	if (type == view_Remove) {
		this->OnScreen = FALSE;
		return;
	}
	if ( ! CheckWindow(this, "FullUpdate")) return;
	if ((type != view_FullRedraw 
				&& type != view_LastPartialRedraw)
			|| (this)->GetLogicalWidth() == 0 
			|| (this)->GetLogicalHeight() == 0) 
		return;
	this->OnScreen = TRUE;
	if (type == view_FullRedraw) {
		/* must recompute graphics info because image
			may be on different display hardware */
	    this->WhitePattern = (this)->view::WhitePattern();
	    this->Grey25Pattern = (this)->view::GrayPattern( 4, 16);
		this->BlackPattern = (this)->view::BlackPattern();
	}
	this->rulerchanged = this->textchanged = this->iconschanged = TRUE;
	RedrawRuler(this);
	RedrawText(this);
	RedrawIcons(this);
}


void 
lprrulerview::Update( )
	{
	if (! this->OnScreen || ! CheckWindow(this, "Update")) return;
	if (this->rulerchanged) RedrawRuler(this);
	if (this->textchanged) RedrawText(this);
	if (this->iconschanged) RedrawIcons(this);
}

class view *
lprrulerview::Hit(enum view_MouseAction   action, long   x , long   y , long   num_clicks)
{
    class lprrulerview *self=this;
	if (action == view_NoMouseEvent)
		return (class view *)this;
	if (! this->OnScreen || ! CheckWindow(this, "Hit")) return NULL;
	if (this->MovingIcon == noIcon  &&  y > this->topline) {
		/* must be in text area or ruler */
		if (action == view_LeftDown || action == view_RightDown ) {
			/* iconAt(self, self->textloc - 12, self->bottomline - 12, "'"); */
			short delx, dely;
			delx = x - (this->textloc -12);
			dely = y - (this->bottomline - 12);
			if (delx<0) delx = -delx;
			if (dely<0) dely = -dely;
			if (delx < 10 && dely < 10) {
				/* change unit */
				switch (this->unit) {
					case style_CM: 
						this->TickTbl = &PointTbl;
						this->unit = style_Points; 
						break;
					case style_Points: 
						this->TickTbl = &InchTbl;
						this->unit = style_Inches; 
						break;
					default: 
						this->TickTbl = &CMTbl;
						this->unit = style_CM; 
						break;
				}
				RedrawRuler(this);
				RedrawIcons(this);
				RedrawText(this);
			}
		}
	}
	else {
		/* above topline,  assume trying to move an icon 
			Allow LEFT BUTTON only */

		(this)->SetFont( IconFont);

		if (action == view_LeftUp) {
			struct lprrulerview_icondata *i = &(this->iconloc[(short)this->MovingIcon]);
			long t = 0;	/* passed to the ValueChangeProc */
			switch (this->MovingIcon) {
				case leftIcon: 
					MoveLeftIcon(this, x);  
					t = i->value;
					break;
				case rightIcon: 
					MoveRightIcon(this, x);  
					t = - i->value;
					break;
				case paraIcon: 
					MoveParaIcon(this, x); 
					t = i->value; 
					break;
			}
			if (i->parkx - 11 <= i->x  && i->x <= i->parkx + 11) {
				t = i->value = lprrulerview_NoValue;
				RecomputeIconX(this, (enum lprrulerview_iconcode)(i - this->iconloc));
				if (this->MovingIcon == leftIcon)
					RecomputeIconX(this, paraIcon);
			}
			
			if (this->ValueChangeProc)
				/* notify client of new value */
				(this->ValueChangeProc)(this, 
						this->ValueChangeRock, 
						this->MovingIcon, 
						t);
			this->MovingIcon = noIcon;
			CheckBounds(this);
			CleanUpIconArea(this);
		}
		else {  /* Down or Move */
			short relx;	/* used later to display numeric value */
			if (action == view_LeftDown) {
				struct lprrulerview_icondata *i;
				long cnt;
				this->MovingIcon = noIcon;
				for (i = &(this->iconloc[(short)paraIcon]), cnt = 3; cnt--; i--) 
					/* NOTE REVERSED ORDER of loop 
						so paraIcon will be
						found first even if it is close to leftICon */
					/* check for closeness to icon i */
					if (i->x - 12 < x  &&  x < i->x + 12) {
						this->MovingIcon = (enum lprrulerview_iconcode)
								(i - this->iconloc);
						if (i->isBlack 
								|| i->value 
								<= lprrulerview_NoValue) {
							i->isBlack = FALSE;
							i->value = (i->x - i->zero)<<16;
						}
						if (this->MovingIcon == leftIcon 
								&& PARA.isBlack)
							PARA.isBlack = FALSE;
						/* no need to remove icons here
							they will be removed by
							MoveXxxxIcon below */
						break;
					}
			}

			/* Here for mouse down and mouse movement
				relx will be used for displaying the numeric value */
			switch (this->MovingIcon) {
				case leftIcon:
					MoveLeftIcon(this, x);
					relx = LEFT.x - this->leftzero;
					break;
				case rightIcon:
					MoveRightIcon(this, x);
					relx = RIGHT.x - this->rightzero;
					break;
				case paraIcon:
					MoveParaIcon(this, x);
					if (LEFT.value <= lprrulerview_NoValue)
						relx = PARA.x - this->leftzero;
					else
						relx = PARA.x - LEFT.x;
					break;
			}
			CleanUpIconArea(this);
			if (this->MovingIcon != noIcon) {
				/* plot the numeric value  (this is where we use relx) */
				struct rectangle r;
				char buf[10];
				r.top = this->topline, r.left = this->textloc;
				r.height = RULERHEIGHT, r.width = this->leftline - this->textloc;	
				(this)->SetTransferMode( graphic_COPY);
				(this)->FillRect( &r, this->WhitePattern);
				if (this->leftline <= x && x <= this->rightline)
					sprintf(buf, this->TickTbl->fmt, 
							(relx + 0.0) / this->TickTbl->majorpix * this->TickTbl->one);
				else 
					strcpy(buf, this->TickTbl->unitstring);
				(this)->SetFont( TextFont);
				(this)->MoveTo( this->textloc, this->bottomline - 6/*C*/);
				(this)->DrawString( buf, 
					graphic_ATLEFT |  graphic_ATBASELINE);
		 	}
		} /* end "down or move */
	} /* end "above topline" */
	if (action == view_LeftDown || action == view_RightDown)
		(this)->WantInputFocus( this);
	return ( class view * )this;		/* where to send subsequent hits */
}

view_DSattributes
lprrulerview::DesiredSize( long  width, long  height, enum view_DSpass  pass, 
				long  *desiredWidth, long  *desiredHeight ) 
						{
	*desiredHeight = 80;
	*desiredWidth = 700;
	return view_Fixed;
}



/* icon position values are pixel positions expressed as long int's with the binary point at 1<<16 */

/* set the values for the icon positions.  Values lprrulerview_NoValue (-999<<16) and lower indicate
			that no value is to be displayed */
void
lprrulerview::SetValues(long  leftmargin , long  rightmargin , long  paraindent)
		{
    class lprrulerview *self=this;
	LEFT.value = leftmargin;
	RIGHT.value =  (rightmargin <= lprrulerview_NoValue) ? rightmargin : - rightmargin;
	PARA.value = paraindent;
	this->iconschanged = this->textchanged = TRUE;
	(this)->WantUpdate( this);
}

void
lprrulerview::GetValues(long  *leftmargin , long  *rightmargin , long  *paraindent)
		/* sets the three parameters to the values of the icon positions */
		{
    class lprrulerview *self=this;
	*leftmargin = LEFT.value;
	*rightmargin = (RIGHT.value <= lprrulerview_NoValue) ? RIGHT.value : - RIGHT.value;
	*paraindent = PARA.value;
}

