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

#include <andrewos.h>
ATK_IMPL("tabrulerview.H")
#include <graphic.H>
#include <view.H>
#include <environ.H>
#include <fontdesc.H>
#include <tabs.H>
#define class_StaticEntriesOnly
#include <style.H>
#undef  class_StaticEntriesOnly
#include <im.H>
#include <rect.h>

#include <lprruler.H>

#include <tabrulerview.H>

#define	IconStringLeft		"\242"
#define	IconStringRight		"\241"
#define	IconStringCentered	"\243"
#define	IconStringOther		"\244"
#define	ClearString	"AllClear"
#define	CancelString	"Cancel"

      

#define ICONHEIGHT 14	/*C*//* height of area where icons move */
#define PARKWIDTH  19	/*C*//* width of icon parking areas */
#define PARKSPACE  24   /* spacing of icon parking areas */
#define tabrulerHEIGHT 22 /*C*//* bottom line = topline + tabrulerHEIGHT */

static class fontdesc *TextFont, *IconFont;


struct TickTbl {
    short majorpix, minorpix;	/* number of pixels for major and minor cycles */
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



ATKdefineRegistry(tabrulerview, view, tabrulerview::InitializeClass);
static boolean BogusCallFromParent(class tabrulerview  *self, const char  *where , const char  *msg);
static boolean CheckWindow(class tabrulerview  *self, const char  *where);
static void MoveIcon(class tabrulerview  *self, long  newx, short which);
static void RepaintIcon(class tabrulerview   *self, long  position, short which, short  color);
static void RemoveIcon(class tabrulerview  *self, long  pos, short which);
static void DoTicks(class tabrulerview  *self, short  zeroloc , short  left, short  right, struct TickTbl  *tbl);
static void RecomputeAndRedraw(class tabrulerview  *self);
static void RedrawRuler(class tabrulerview  *self);
static void RedrawText(class tabrulerview  *self, float  number, boolean  flag);
static void RedrawCommands(class tabrulerview  *self);
static void RedrawPark(class tabrulerview  *self);
static void RedrawIcons(class tabrulerview   *self);
int FindClosestTab(class tabrulerview  *self, long  pos);


static boolean
BogusCallFromParent(class tabrulerview  *self, const char  *where , const char  *msg)
{
    fprintf(stderr, "<tabrulerview>Bogus call to %s, %s\n", where, msg);
    return FALSE;
}

static boolean
CheckWindow(class tabrulerview  *self, const char  *where)
{
    class graphic *g
      = (class graphic *)(self)->GetDrawable();
    if ( ! g) return BogusCallFromParent(self, where, "No Graphic");
    return TRUE;
}



boolean
tabrulerview::InitializeClass()
{
    TextFont = fontdesc::Create("andysans", fontdesc_Bold, 12);
    IconFont = fontdesc::Create("icon", fontdesc_Plain, 12);
    return TRUE;
}

tabrulerview::tabrulerview()
{
	ATKinit;

    this->OnScreen = FALSE;
    this->tabrulerchanged = this->iconschanged = this->textchanged = TRUE;
    this->unit = style_Inches;
    this->TickTbl = &InchTbl;
    this->ValueChangeProc = NULL;
    this->tabs = NULL;
    this->Moving = 0;
    this->mul = environ::GetProfileInt("TabScalingMultiplier", 14);
    this->div = environ::GetProfileInt("TabScalingDivisor", 12);
    THROWONFAILURE( TRUE);
}

tabrulerview::~tabrulerview()
{
	ATKinit;

}

#if 0
/* this wasn't declared in the .ch file, so it wasn't
 being used... it still isn't -rr2b */
void
tabrulerview::ObservedChanged(class observable  *dobj, long  status)
{
    if (status == lprruler_DATACHANGED) 
	this->iconschanged = this->textchanged = TRUE;
    else if (status == observable_OBJECTDESTROYED)
	return;

    (this)->WantUpdate( this);
}
#endif

static void
MoveIcon(class tabrulerview  *self, long  newx, short which)
{
    RemoveIcon(self, self->Movex, which);
    self->Movex = newx;
    RepaintIcon(self, newx, which, graphic_COPY);
}
      

static void
RepaintIcon(class tabrulerview   *self, long  position, short which, short  color)
{
    const char *str;
    switch (which) {
	case style_LeftAligned:
	    str = IconStringLeft;
	    break;
	case style_RightAligned:
	    str = IconStringRight;
	    break;
	case style_CenteredOnTab:
	    str = IconStringCentered;
	    break;
	default:
	    str = IconStringOther;
	    break;
    }
    (self)->SetTransferMode( color);
    (self)->MoveTo( position*self->mul/self->div, self->icony);
    (self)->DrawText( str, 1, graphic_NOMOVEMENT);
}

static void
RemoveIcon(class tabrulerview  *self, long  pos, short which) 
{
    long dx;
    int i;

    RepaintIcon(self, pos, which, graphic_WHITE);
    for (i = 0; i < self->tabs->number; i++) {
	dx = self->tabs->Positions[i] - pos;
	if (dx < 10 && dx > -10) /*C*/
	    RepaintIcon(self, self->tabs->Positions[i], self->tabs->Types[i], graphic_COPY);
    }
}


/* Draw tick marks in a tabruler for -self- on the topline, from -left- to -right-
    the value zero would be plotted at location zeroloc, which may be off
    the tabruler to either end
    the length of tick marks is given by -tbl-, with one entry for
	each minor cycle within a major cycle.
	The lengths of the cycles are given by -major- and -minor-
	assume major % minor == 0 */
static void
DoTicks(class tabrulerview  *self, short  zeroloc , short  left, short  right, struct TickTbl  *tbl)
{
    short cycmax = tbl->majorpix / tbl->minorpix;	/* number of minor cycles in a major */
    short tickloc;					/* where to place next tick */
    short cyclecnt;				/* count minor ticks within major cycle */
    short ordloc;						/* where to first plot an ordinate value */
    short ordval;						/* next ordinate value to plot */
    short x;

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
	x = tickloc * self->mul / self->div;
	(self)->MoveTo( x, self->topline);
	(self)->DrawLineTo( x, self->topline + tbl->ht[cyclecnt]);
	if (cyclecnt == 0) {
	    /* draw digit */
	    char buf[10];
	    if (ordval > 0)
		sprintf(buf, "%+d", ordval);
	    else sprintf(buf, "%d", ordval);	/* no sign if ordval==0 */
	    (self)->MoveTo( x-1, self->topline + 7);	/*C*/
	    (self)->DrawString( buf,
				    graphic_BETWEENLEFTANDRIGHT | graphic_ATTOP);
	    ordval += tbl->one;
	}
	if (++cyclecnt >= cycmax) cyclecnt = 0;
    }
}

static void
RecomputeAndRedraw(class tabrulerview  *self)
{
    struct rectangle r;

    (self)->GetLogicalBounds( &r);

    self->leftline = 0;
    self->rightline = r.width;
    /* Assuming icons are square... */
    self->textloc = self->leftline + ICONHEIGHT*4 + 4 + 2*PARKSPACE;   /* with icon for unit change to its left */
    self->cleartxt = (self->rightline - self->leftline) / 2 + self->leftline;
    self->canceltxt = self->rightline - 4;

    self->topline = 4 + ICONHEIGHT;
    self->bottomline = self->topline + tabrulerHEIGHT;
    self->icony = self->topline - 2;
    self->leftzero = self->leftline;

    (self)->SetTransferMode( graphic_COPY);
    (self)->FillRect( &r, self->WhitePattern);

    RedrawRuler(self);
    RedrawPark(self);
    RedrawIcons(self);
    RedrawText(self, 0.0, FALSE);
    RedrawCommands(self);

    self->tabrulerchanged = FALSE;
}

static void
RedrawRuler(class tabrulerview  *self)
{
    struct rectangle r;
    r.left   = self->leftline;
    r.top    = self->topline;
    r.width  = self->rightline - self->leftline;
    r.height = self->bottomline - self->topline;

    /* Clean the area */
    (self)->SetTransferMode( graphic_COPY);
    (self)->FillRect( &r, self->WhitePattern);

    /* draw outline */
    (self)->MoveTo( self->leftline, self->topline);
    (self)->DrawLineTo( self->rightline, self->topline);
    (self)->DrawLineTo( self->rightline, self->bottomline);
    (self)->DrawLineTo( self->leftline, self->bottomline);
    (self)->DrawLineTo( self->leftline, self->topline);

    DoTicks(self, self->leftline, self->leftline,
	     self->rightline, self->TickTbl);

}

static void
RedrawText(class tabrulerview  *self, float  number, boolean  flag)
{
    struct rectangle r;
    char buf[10];
    long buttony;

    r.top = self->bottomline + 2, r.left = self->textloc;
    r.height = ICONHEIGHT + 4/*C*/;
    r.width = self->clearpos - self->textloc - ICONHEIGHT;
    buttony = self->bottomline + (ICONHEIGHT>>1) + 4;
    (self)->SetTransferMode( graphic_COPY);
    (self)->FillRect( &r, self->WhitePattern);
    (self)->SetFont( TextFont);
    (self)->MoveTo( self->textloc, self->bottomline + 2);
    if (flag)
	/* We want to display a number here */
	sprintf(buf, self->TickTbl->fmt, number);
    else
	strcpy(buf, self->TickTbl->unitstring);

    (self)->DrawString( buf, graphic_ATLEFT |  graphic_ATTOP);

    /* display button icons */
    (self)->SetFont( IconFont);
    (self)->MoveTo( self->textloc-ICONHEIGHT, buttony);
    (self)->DrawText( "\'", 1, graphic_NOMOVEMENT);
    
    self->textchanged = FALSE;
}    

static void
RedrawCommands(class tabrulerview  *self)
{
    long x, y, buttony;
    struct rectangle r;

    /*
      fontdesc_StringSize(TextFont, (struct graphic *) self, ClearString, &x,&y);
      */
    x = 60; /* x = Width of ClearString */
    self->clearpos = self->cleartxt - (x>>1) - ICONHEIGHT;
    /*
      fontdesc_StringSize(TextFont, (struct graphic *) self, CancelString, &x,&y);
      */
    x = 50; /* x = Width of CancelString */
    self->cancelpos = self->canceltxt - x - ICONHEIGHT;

    r.top = self->bottomline + 2;
    r.left = self->clearpos - ICONHEIGHT;
    r.height = ICONHEIGHT + 4/*C*/;
    r.width = self->rightline - r.left;
    buttony = self->bottomline + (ICONHEIGHT>>1) + 4;

    (self)->SetTransferMode( graphic_COPY);
    (self)->FillRect( &r, self->WhitePattern);
    (self)->SetFont( TextFont);

    /* The text for AllClear and Cancel */
    (self)->MoveTo( self->cleartxt, self->bottomline + 2);
    (self)->DrawString( ClearString, graphic_BETWEENLEFTANDRIGHT | graphic_ATTOP);

    (self)->MoveTo( self->canceltxt, self->bottomline + 2);
    (self)->DrawString( CancelString, graphic_ATRIGHT | graphic_ATTOP);

    /* display button icons */
    (self)->SetFont( IconFont);
    (self)->MoveTo( self->cancelpos, buttony);
    (self)->DrawText( "\'", 1, graphic_NOMOVEMENT);
    (self)->MoveTo( self->clearpos, buttony);
    (self)->DrawText( "\'", 1, graphic_NOMOVEMENT);
}

static void
RedrawPark(class tabrulerview  *self)
{
    struct rectangle r;

    /* Redraw the parking lot(s)... */
    /* If we are moving the icon, then we leave that park empty */

    (self)->SetTransferMode( graphic_COPY);

    r. height = ICONHEIGHT+4, r.width = PARKWIDTH;
    r.top = self->bottomline + 2;

    r.left = self->leftline + 2;
    (self)->FillRect( &r, self->Grey25Pattern);
    if (self->Moving != style_LeftAligned) {
	(self)->MoveTo( r.left+PARKWIDTH/2,
			    self->bottomline+ICONHEIGHT+2); 
	(self)->DrawText( IconStringLeft, 1, graphic_NOMOVEMENT);
    }

    r.left = self->leftline + PARKSPACE + 2;
    (self)->FillRect( &r, self->Grey25Pattern);
    if (self->Moving != style_RightAligned) {
	(self)->MoveTo( r.left+PARKWIDTH/2,
			    self->bottomline+ICONHEIGHT+2); 
	(self)->DrawText( IconStringRight, 1, graphic_NOMOVEMENT);
    }

    r.left = self->leftline + 2*PARKSPACE + 2;
    (self)->FillRect( &r, self->Grey25Pattern);
    if (self->Moving != style_CenteredOnTab) {
	(self)->MoveTo( r.left+PARKWIDTH/2,
			    self->bottomline+ICONHEIGHT+2); 
	(self)->DrawText( IconStringCentered, 1, graphic_NOMOVEMENT);
    }

}

static void
RedrawIcons(class tabrulerview   *self)
{
    int i;
    struct rectangle r;

    /* Clear the tabstops */
    r.top = self->topline - ICONHEIGHT - 4/*C*/, r.left = 0;
    r.height = ICONHEIGHT + 3/*C*/, r.width = self->rightline;	
    (self)->SetTransferMode( graphic_COPY);
    (self)->FillRect( &r, self->WhitePattern);
    (self)->SetFont( IconFont);

    if (self->tabs)
	for (i = 0; i < self->tabs->number; i++)
	    RepaintIcon(self, self->tabs->Positions[i] + self->leftline, self->tabs->Types[i], graphic_COPY);

    /* The moving icon... */
    if (self->Moving)
	RepaintIcon(self, self->Movex, self->Moving, graphic_COPY);

    RedrawPark(self);

    self->iconschanged = FALSE;
}

void 
tabrulerview::FullUpdate( enum view_UpdateType   type, long   left , long   top , long   width , long   height )
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
	this->Grey25Pattern = (this)->GrayPattern( 4, 16);
	this->WhitePattern = (this)->view::WhitePattern();
    }
    this->tabrulerchanged = this->textchanged = this->iconschanged = TRUE;
    RecomputeAndRedraw(this);
}


void 
tabrulerview::Update( )
{
    if (! this->OnScreen || ! CheckWindow(this, "Update")) return;
    if (this->tabrulerchanged) RedrawRuler(this);
    if (this->textchanged) RedrawText(this, 0.0, FALSE);
    if (this->iconschanged) RedrawIcons(this); 
}

class view *
tabrulerview::Hit(enum view_MouseAction   action, long   x , long   y , long   num_clicks)
{
    if (action == view_NoMouseEvent)
	return (class view *) this;
    if (!this->OnScreen || !CheckWindow(this, "Hit"))
	return NULL;

    if (action == view_LeftDown || action == view_RightDown) {
	if (this->Moving) { 
	    /* Cancel */
	    RemoveIcon(this, this->Movex, this->Moving);
	    this->Moving = 0;
	    RedrawPark(this);
	} else {
	    short dx, dy;
	    dx = x - (this->textloc - ICONHEIGHT);
	    dy = y - (this->bottomline + ICONHEIGHT/2 + 4);
	    if (dy < 10 && dy > -10) {
		/* Vertically, we're in the right place */
		if (dx < 10 && dx > -10) {
		    /* change units button */
		    switch(this->unit) {
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
		    RedrawText(this, 0.0, FALSE);
		} else if (this->tabs) {
		    if ((dx = x - this->cancelpos) < 10 && dx >-10) {
			/* The cancel button */
			this->tabs = tabs::Create();
			RedrawIcons(this);
			RedrawText(this, 0.0, FALSE);
			if (this->ValueChangeProc)
			    /* notify client of new value */
			    /* cannot have -ve tabstops, so we use it
			     * to indicate cancel code
			     */
			    (this->ValueChangeProc)(this, 
						    this->ValueChangeRock, 
						    (enum lprrulerview_iconcode)-1, style_AllClear,
						    style_Points);
		    } else if ((dx = x - this->clearpos) < 10 && dx > -10) {
			/* The AllClear button */
			this->tabs = (this->tabs)->Clear();
			RedrawIcons(this);
			RedrawText(this, 0.0, FALSE);
			if (this->ValueChangeProc)
			    /* notify client of new value */
			    (this->ValueChangeProc)(this, 
						    this->ValueChangeRock, 
						    (enum lprrulerview_iconcode )0, style_AllClear,
						    style_Points);
		    } else if (x < 2*PARKSPACE+PARKWIDTH) {
			/* We are in the park! - create a new tab in process */
			if (x < PARKWIDTH)
			    this->Moving = style_LeftAligned;
			else if (x < PARKSPACE+PARKWIDTH)
			    this->Moving = style_RightAligned;
			else
			    this->Moving = style_CenteredOnTab;
			this->Movex = x;
			this->oldtab = -1; /* Cannot have -ve tabs */
			RepaintIcon(this, x, this->Moving, graphic_COPY);
			RedrawPark(this);
		    }
		}
	    } else if (y < this->topline) {
		/* Playing with existing tabstops */
		int i = FindClosestTab(this, (x - this->leftzero)*this->div / this->mul);
		if (i >= 0) {
		    /* Only do something if we are close enough to a tab */
		    float f = ((float) this->tabs->Positions[i]) /
		      this->TickTbl->majorpix *
		      this->TickTbl->one;
		    if (action == view_LeftDown) {
			/* Hit on a tab - let's move it... */
			this->Moving = this->tabs->Types[i];
			this->Movex = this->tabs->Positions[i];
			/* remove it from its old position */
			this->tabs = (this->tabs)->Delete( i);
			this->oldtab = this->Movex;
			RedrawText(this, f, TRUE);
		    } else if (action == view_RightDown) {
			/* we want to remove a tab */
			long x = this->tabs->Positions[i];
			short deltype = this->tabs->Types[i];
			this->tabs = (this->tabs)->Delete( i);
			RemoveIcon(this, x, deltype);
			RedrawText(this, 0.0, FALSE);
			if (this->ValueChangeProc)
			    /* notify client of new value */
			    (this->ValueChangeProc)(this, this->ValueChangeRock, (enum lprrulerview_iconcode )x, style_TabClear, style_Points);
		    }
		}
	    }
	}
    } else {
	/* Either move or up event */
	if (action == view_LeftUp && this->Moving) {
	    float f;
	    /* Plant a tab here */
	    int i;
	    short newtype;
	    x = (x - this->leftline) * this->div / this->mul;
	    i = FindClosestTab(this, x);
	    newtype = this->Moving;
	    this->Moving = 0;
	    if (i == -1) {
		f = ((float) (x - this->leftline)) /
		  this->TickTbl->majorpix *
		  this->TickTbl->one;
		if (this->oldtab == x) {
		    /* Seems user has moved an old tab nowhere */
		    MoveIcon(this, x, newtype);
		    this->tabs=(this->tabs)->Add( x, (enum style_TabAlignment)newtype);
		} else {
		    /* We have just created a new tab */
		    MoveIcon(this, x, newtype);
		    if (this->ValueChangeProc) {
			/* notify client of new value */
			(this->ValueChangeProc)(this, this->ValueChangeRock, (enum lprrulerview_iconcode )x, (enum style_TabAlignment)newtype, style_Points);
			if (this->oldtab >= 0)
			    (this->ValueChangeProc)(this, this->ValueChangeRock, (enum lprrulerview_iconcode ) this->oldtab, style_TabClear, style_Points);
		    }
		    this->tabs = (this->tabs)->Add( x, (enum style_TabAlignment)newtype);
		}
	    } else {
		f = ((float) (this->tabs->Positions[i] - this->leftline)) /
		  this->TickTbl->majorpix *
		  this->TickTbl->one;
		RemoveIcon(this, x, newtype);
		/* If we were moving a tab around, then there's more work */
		if (this->oldtab >= 0)
		    if (this->ValueChangeProc)
			(this->ValueChangeProc)(this, this->ValueChangeRock, (enum lprrulerview_iconcode)this->oldtab, style_TabClear, style_Points);
	    }
	    RedrawPark(this);
	    RedrawText(this, f, TRUE);
	} else if (this->Moving &&
		   (action == view_LeftMovement ||
		    action == view_RightMovement)) {
	    /* We are still moving the new tab to its position */
	    float f;
	    if (x - this->leftline >= 0) {
		x = (x - this->leftline) *this->div / this->mul;
		f = ((float) (x - this->leftline)) /
		  this->TickTbl->majorpix *
		  this->TickTbl->one;
		MoveIcon(this, x, this->Moving);
		RedrawText(this, f, TRUE);
	    }
	}
    }
    if (action == view_LeftDown || action == view_RightDown)
	(this)->WantInputFocus( this);
    return (class view *) this;
}

view_DSattributes
tabrulerview::DesiredSize( long  width, long  height, enum view_DSpass  pass, 
			   long  *desiredWidth, long  *desiredHeight ) 
{
    *desiredHeight = 80;
    *desiredWidth = 700;
    return view_Fixed;
}

void
tabrulerview::SetValues(class tabs  *tabs)
{
    this->tabs = tabs;
    this->iconschanged = this->textchanged = TRUE;
    (this)->WantUpdate( this);
}

void
tabrulerview::GetValues(class tabs  **tabs)
{
    *tabs = this->tabs;
}

int
FindClosestTab(class tabrulerview  *self, long  pos)
{
    int i;
    long dx;

    if (self->tabs)
	for (i = 0; i < self->tabs->number; i++) {
	    dx = pos - self->tabs->Positions[i];
	    if (dx < 0)
		dx = -dx;
	    if (dx < 6) /* Cannot have tabs closer than 6 pts apart */
		return i;
	}
    return -1;
}   
