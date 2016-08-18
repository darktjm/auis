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

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/supportviews/RCS/oscroll.C,v 3.7 1995/07/17 20:27:28 rr2b Stab74 $";
#endif


 

/* oscrollbar code for be2. */

#define DEBUG(X)



ATK_IMPL("oscroll.H")
#include <oscroll.H>

#include <graphic.H>
#include <view.H>
#include <updatelist.H>
#include <im.H>
#include <cursor.H>
#include <event.H>
#include <environ.H>

#include <point.h>
#include <rect.h>

/* The physical aspects of the scrollbar that cannot be changed dynamically */
/* percentage overlap when thumbing using the endzones */
#define	THUMBPCT	10

/* The physical aspects of the scrollbar that cannot be changed dynamically */

#define SBORDER 	(TD_GAP + TD_DEPTH)
				/* size of border around scrollbar */

#define INNERWIDTH 	15	/* width of the thumb and arrows */
#define ENDZONELENGTH (emulation?INNERWIDTH+TD_GAP:12)
#define ENDTOBARSPACE 4
#define BARWIDTH (emulation?INNERWIDTH:20)
#define DOTWIDTH 7
#define MINDOTLENGTH 6
#define SMALLDIST 5
#define REALBARWIDTH(self) ((self)->barWidth+(emulation?2*SBORDER:0))

#define PIXELSPERINCH 75

/* The descriptions of the different types of scrollbars */
static int Type[scroll_SIDES] = {scroll_VERT, scroll_VERT, scroll_HORIZ, scroll_HORIZ},
    LeftChange[scroll_SIDES] = {1, 0, 0, 0},
    TopChange[scroll_SIDES] = {0, 0, 1, 0},
    WidthChange[scroll_SIDES] = {-1, -1, 0, 0},
    HeightChange[scroll_SIDES] = {0, 0, -1, -1}, NoLeftChange[scroll_SIDES] = {TD_GAP, 0, 0, 0},
    NoTopChange[scroll_SIDES] = {0, 0, TD_GAP, 0},
    NoWidthChange[scroll_SIDES] = {-TD_GAP, -TD_GAP, 0, 0},
    NoHeightChange[scroll_SIDES] = {0, 0, -TD_GAP, -TD_GAP};    

static char *InterfaceName[scroll_TYPES] = {"scroll,vertical", "scroll,horizontal"};


/* Icon infomation */
static short CursorIcon[scroll_TYPES], ThumbIcon;

static long startScrollTime;
static long minContScrollTime;
static long maxContScrollTime;
static boolean adjustScroll;
static boolean thumbScroll;

/* status codes for from_bar_to_range */
#define OUTSIDEBAR 0
#define INTOPZONE scroll_TOPENDZONE
#define INSIDEBAR 3
#define INBOTZONE scroll_BOTTOMENDZONE


/* The different buttons that can be down */
#define NEITHER 0
#define LEFT 1
#define RIGHT 2
#define SMOOTH 4


/* The different states the thumb can be in */
#define NOPE 0
#define MAYBE 1
#define YES 2


/* Useful macros */
#undef MIN
#undef MAX
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ABS(a) ((a) < 0 ? -(a) : (a))
#define SWAP(a,b) (temp)=(a);(a)=(b);(b)=(temp)


#define INTERSECT(top1, bot1, top2, bot2) ((bot1) >= (top2) && (top1) <= (bot2))

/* Variables used by elevator redraw code. Statics instead to parameters for speed. */

static struct {
    long top, bot;
    boolean whitep;
} range[2];
static long seen_top, seen_bot, left, right;

/* Motif Emulation variables and #defines */

static int emulation=0;
static short NormalIcon;
static int current_end_state = 0;


/* The thumb direction codes */
#define scroll_UP 1
#define scroll_DOWN 2
#define scroll_NEITHER 3



#define FILLPAT		((self)->BlackPattern())


/* Creation and Destruction routines. */


ATKdefineRegistry(oscroll, scroll, oscroll::InitializeClass);
#ifndef NORCSID
#endif
static void get_interface(class oscroll  *self, int  type);
static void getinfo(class oscroll  *self, int  type, struct range  *total , struct range  *seen , struct range  *dot);
static void set_frame(class oscroll  *self, int  side, int  posn, long  coord);
static long bar_height(class oscroll  *self, int  side);
static void endzone(class oscroll  *self, int  side, int  end, enum view_MouseAction  action);
static int what_is_at(class oscroll  *self, int  side, int  coord);
static void rotate(class oscroll  *self, int  side, long  x , long  y , long  *scroll_x , long  *scroll_y);
static long from_range_to_bar(class oscroll  *self, int  side, struct scrollbar  *bar, long  posn);
static long from_bar_to_range(class oscroll  *self, int  side , struct scrollbar  *bar, long  posn, int  *status);
static void calc_location(class oscroll  *self);
static void calc_desired(class oscroll  *self);
static int calc_dot(class oscroll  *self, int  side, struct scrollbar  *bar, long  *x1 , long  *y1 , long  *x2 , long  *y2);
static int calc_elevator(class oscroll  *self, int  side, struct scrollbar  *bar, long  *x1 , long  *y1 , long  *x2 , long  *y2);
static void rectangle(class oscroll  *self, int  x1 , int  y1 , int  x2 , int  y2, class graphic  *tile);
static void draw_bar(class oscroll  *self, int  side, long  tt , long  tb			/* top and bottom of thumb in already-rotated
				   coordianates */);
static void draw_thumb_and_bar(class oscroll  *self, int  side, boolean  force);
static void init_arrows(class oscroll  *self);
static void draw_endzones(class oscroll  *self, int  side , int  height, int  state			/* which arrow on or off */);
static void motif_draw_whole_bar(class oscroll  *self, int  side);
static void draw_elevator(class oscroll  *self, int  side);
static void draw_dot(class oscroll  *self, int  side);
static void normal_draw_whole_bar(class oscroll  *self, int  side);
static void erase_dot(class oscroll  *self, int  side, long  top , long  bot);
static void draw_end_line(class oscroll  * self,long  x1,long  y1,long  x2,long  y2 );
static void move_elevator(class oscroll  *self, int  side);
static void  normal_full_update(class oscroll  *self, enum view_UpdateType  type, long  left , long  top , long  width , long  height);
static void motif_full_update(class oscroll  *self, enum view_UpdateType  type, long  left , long  top , long  width , long  height);
static void full_update(class oscroll  *self, enum view_UpdateType  type, long  left , long  top , long  width , long  height);
void normal_scroll__Update(class oscroll  *self);
void motif_scroll__Update(class oscroll  *self);
static void RepeatEvent(class oscroll  *self);
static void RepeatScroll(class oscroll  *self, long  cTime);
class view *normal_scroll__Hit(class oscroll  *self, enum view_MouseAction  action, long  x , long  y , long  num_clicks);
class view *motif_scroll__Hit(class oscroll  *self, enum view_MouseAction  action, long  x , long  y , long  num_clicks);
void motif_Draw3dBorder(class view  *v, long  x1 , long  y1 , long  x2 , long  y2, boolean  sense			/* "innie" or "outtie" */, class graphic  *fillp		/* center fill pattern, NULL for none */);
void motif_DrawBorder(class view  *v, long  x1 , long  y1 , long  x2 , long  y2		/* enclosing coords */, class graphic  *lt , class graphic  *dk , class graphic  *fillp /* patterns for light, dark and center */, int  depth			/* depth of border */);


boolean oscroll::InitializeClass()
{
    ::emulation = environ::GetProfileSwitch("MotifScrollBars", FALSE);
    if(::emulation) NormalIcon = Cursor_Arrow;

    CursorIcon[scroll_VERT] = Cursor_VerticalArrows;
    CursorIcon[scroll_HORIZ] = Cursor_HorizontalArrows;
    ThumbIcon = Cursor_Octagon;

    ::startScrollTime = environ::GetProfileInt("StartScrollTime", 0);
    ::minContScrollTime = environ::GetProfileInt("ContScrollTime", 500);
    ::maxContScrollTime = environ::GetProfileInt("MaxContScrollTime", ::minContScrollTime);
    ::adjustScroll = environ::GetProfileSwitch("AdjustScroll", FALSE);
    ::thumbScroll = environ::GetProfileSwitch("ThumbScroll", FALSE);
    return TRUE;
}

oscroll::oscroll()
{
	ATKinit;

    int i;

    this->current.location = 0;
    this->child = this->scrollee = NULL;
    for (i = 0; i < scroll_TYPES; i++) {
        struct scrollbar *bar = &this->current.bar[i];
        bar->total.beg = 0;
        bar->total.end = 1;
        bar->seen.beg = bar->seen.end = bar->dot.beg = bar->dot.end = -1;
    }

    this->desired = this->current;

    for (i = 0; i < scroll_TYPES; i++)
        this->fns[i] = NULL;

    this->pending_update = 0;
    this->updatelistp = new updatelist;
    this->left = this->top = this->width = this->height = 0;
    
    this->ideal_location = scroll_LEFT;
    
    this->endzone_threshold = 80;
    this->bar_threshold = 0;
    this->endzoneLength = ENDZONELENGTH;
    this->endbarSpace = ENDTOBARSPACE;
    this->barWidth = environ::GetProfileInt("ScrollBarWidth",BARWIDTH);
    this->dotWidth = environ::GetProfileInt("DotWidth", DOTWIDTH);
    this->min_elevator[0] = 5;
    this->min_elevator[1] = 18;
    this->button = NEITHER;
    this->force_full_update = FALSE;
    this->force_get_interface = FALSE;
    this->scrollEvent = NULL;

    if(this->dotWidth<4) this->dotWidth=4;
    if(this->barWidth<8) this->barWidth=8;
    
    if(this->dotWidth>this->barWidth-4) {
	this->dotWidth=this->barWidth-4;
    }

    if(!emulation) {
	for (i = 0; i < scroll_SIDES; i++) {
	    this->curse[i] = cursor::Create(this);
	    (this->curse[i])->SetStandard( CursorIcon[Type[i]]);
	}
    } else {
	this->direction = scroll_NEITHER;
	this->curse[0] = cursor::Create(this);
	(this->curse[0])->SetStandard( NormalIcon);
    }

    THROWONFAILURE( TRUE);
}
    
oscroll::~oscroll()
{
	ATKinit;

    int i;

    if (this->child != NULL)
        (this->child)->UnlinkTree();
    delete this->updatelistp;
    
    if(!emulation) {
	for (i = 0; i < scroll_SIDES; i++)
	delete this->curse[i];
    } else {
	delete this->curse[0];
    }
}

class oscroll *oscroll::Create(class view  *scrollee, int  location)
{
	ATKinit;

    class oscroll *retval = new oscroll;

    (retval)->SetView( scrollee);
    (retval)->SetLocation( location);

    return retval;
}



/* State modification routines. */

void oscroll::SetView(class view  *view)
{

    (this)->SetChild( view);
    (this)->SetScrollee( view);
}

void oscroll::SetChild(class view  *child)
{
    if (this->child != child) {
        this->force_full_update = TRUE;
        if (this->child)
            (this->child)->UnlinkTree();
        this->child = child;
        if (child)
            (child)->LinkTree( this);
        this->thumbing = NOPE;
        (this)->WantUpdate( this);
    }
}

void oscroll::SetScrollee(class view  *scrollee)
{
    if (this->scrollee != scrollee) {
        this->force_get_interface = TRUE;
        this->scrollee = scrollee;
        this->thumbing = NOPE;
        (this)->WantUpdate( this);
    }
}

class view *oscroll::GetChild()
{
    return this->child;
}

class view *oscroll::GetScrollee()
{
    return this->scrollee;
}

void oscroll::SetLocation(int  location)
{
    if(emulation && this->ideal_location != location) this->force_full_update = TRUE;
    this->ideal_location = location;
    (this)->WantUpdate( this);  
}

int oscroll::GetLocation()
{
    return this->ideal_location;
}

int oscroll::GetCurrentLocation()
{
    return this->current.location;
}

void oscroll::SetParameters(long  endzone , long  bar, int  without , int  with)
{
    this->endzone_threshold = endzone;
    this->bar_threshold = bar;
    this->min_elevator[0] = without;
    this->min_elevator[1] = with;
    (this)->WantUpdate( this);
}

void oscroll::GetParameters(long  *endzone , long  *bar, int  *without , int  *with)
{
    *endzone = this->endzone_threshold;
    *bar = this->bar_threshold;
    *without = this->min_elevator[0];
    *with = this->min_elevator[1];
}

void oscroll::SetWidth(long  newWidth)
{
    this->barWidth = newWidth;
    
    if (this->dotWidth > newWidth - 2) {
	(this)->SetDotWidth( newWidth - 2);
    }
    (this)->WantUpdate( this);
}

long oscroll::GetWidth()
{
    return this->barWidth;
}

void oscroll::SetDotWidth(long  newWidth)
{
    if(!emulation) {
	/* NORMAL SCROLLBARS */
	if (newWidth > this->barWidth - 2) {
	    newWidth = this->barWidth - 2;
	}
	if (newWidth < 0) {
	    newWidth = 0;
	}
	this->dotWidth = newWidth;
	(this)->WantUpdate( this);
    }
}

long oscroll::GetDotWidth()
{
    return this->dotWidth;
}

void oscroll::SetEndZoneLength(long  newLength)
{
    if(!emulation) {
	/* NORMAL SCROLLBARS */
	this->endzoneLength = newLength;
	if ((this->endzoneLength + this->endbarSpace) * 2 > this->endzone_threshold) {
	    this->endzone_threshold = (this->endzoneLength + this->endbarSpace) * 2;
	}
	(this)->WantUpdate( this);
    }
}

long oscroll::GetEndZoneLength()
{
    return this->endzoneLength;
}

void oscroll::SetEndToBarSpace(long  newSpace)
{
    if(!emulation) {
	/* NORMAL SCROLLBARS */
	this->endbarSpace = newSpace;
	if ((this->endzoneLength + this->endbarSpace) * 2 > this->endzone_threshold) {
	    this->endzone_threshold = (this->endzoneLength + this->endbarSpace) * 2;
	}
	(this)->WantUpdate( this);
    }
}


long oscroll::GetEndToBarSpace()
{
    return this->endbarSpace;
}

/* Interface routines. */

static void get_interface(class oscroll  *self, int  type)
{
    if (self->force_get_interface || self->fns[type] == NULL) {
        self->fns[type] = (struct scrollfns *)(self->scrollee)->GetInterface( InterfaceName[type]);
	self->force_get_interface = FALSE;
    }
}

static void getinfo(class oscroll  *self, int  type, struct range  *total , struct range  *seen , struct range  *dot)
{
    scroll_getinfofptr real_getinfo;

    get_interface(self, type);

    if (self->fns[type] != NULL && (real_getinfo = self->fns[type]->GetInfo) != NULL)
        real_getinfo(self->scrollee, total, seen, dot);

    if (total->beg == total->end) {
        total->end++;
        *seen = *total;
        dot->beg = dot->end = total->beg;
    }
}

static void set_frame(class oscroll  *self, int  side, int  posn, long  coord)
{
    scroll_setframefptr real_setframe;
    int type = Type[side];
    int realposn = posn;
    static int old[scroll_SIDES][2]; /* Type x {posn, coord} */

    if(!emulation) {
	/* NORMAL SCROLLBARS */
	get_interface(self, type);

	if (self->fns[type] != NULL && (real_setframe = self->fns[type]->SetFrame) != NULL)
	    real_setframe(self->scrollee, posn, coord, type == scroll_VERT ? self->height : self->width);
    } else {
	/* MOTIF SCROLLBARS */
	DEBUG(("set_frame: side: %d posn: %d num: %d dem: %d\n",side,posn,coord,
	       (type == scroll_VERT) ? self->height : self->width));
	DEBUG(("set_frame: oldposn: %d oldnum: %d\n",old[side][0],old[side][1]));

	if ((self->updatelistp)->UpdatesPending()) {
	    /* delete the list and add us once */
	    /*	updatelist_Clear(self->updatelistp); */
	    (self->updatelistp)->DeleteTree( (class view *)self);
	    if (self->child) {
		(self->updatelistp)->AddTo( self->child);
	    }
	}
	get_interface(self, type);


	old[side][0] = posn;
	old[side][1] = coord;

	DEBUG(("end: %d %d\n",
	       self->desired.bar[type].seen.end,
	       self->desired.bar[type].total.end));



	DEBUG(("realposn: %d\n", realposn));

	if (self->fns[type] != NULL &&
	    (real_setframe = self->fns[type]->SetFrame) != NULL)
	    real_setframe(self->scrollee, realposn, coord, type == scroll_VERT ?
			  self->height : self->width);
    }

}

/* Calculation routines. */

static long bar_height(class oscroll  *self, int  side)
{
    switch (Type[side]) {
        case scroll_VERT: return self->height;
        case scroll_HORIZ: return self->width;
    }
    return 0;
}

static void endzone(class oscroll  *self, int  side, int  end, enum view_MouseAction  action)
{
    scroll_endzonefptr real_endzone;
    int type = Type[side];
    int didSC = FALSE;

    get_interface(self, type);

    if(!emulation) {
	/* NORMAL SCROLLBARS */
	if (self->fns[type] != NULL && (real_endzone = self->fns[type]->EndZone) != NULL)
	    real_endzone(self->scrollee, end, action);
	else {
	    if (end == scroll_TOPENDZONE)
		set_frame(self, side, self->desired.bar[type].total.beg, 0);
	    else
		set_frame(self, side, self->desired.bar[type].total.end, bar_height(self, side) >> 2);
	}
    } else {
	/* MOTIF SCROLLBARS */

	if (self->fns[type] != NULL &&
	    (real_endzone = self->fns[type]->EndZone) != NULL)
	    real_endzone(self->scrollee, end, action);
	else {
	    if (end == scroll_TOPENDZONE) {
		set_frame(self, side, self->desired.bar[type].seen.beg,
			  bar_height(self, side) * (100 - THUMBPCT) / 100);
	    } else
		if (didSC)
		    set_frame(self, side,self->desired.bar[type].seen.end - 100, bar_height(self, side) * THUMBPCT / 100);
		else
		    set_frame(self, side, self->desired.bar[type].seen.end, bar_height(self, side) * THUMBPCT / 100);
	}
    }
}

static int what_is_at(class oscroll  *self, int  side, int  coord)
{
    scroll_whatfptr real_what;
    int type = Type[side];

    get_interface(self, type);

    if (self->fns[type] != NULL && (real_what = self->fns[type]->WhatIsAt) != NULL)
        return real_what(self->scrollee, coord, type == scroll_VERT ? self->height : self->width);
    else
        return 0;
}

static void rotate(class oscroll  *self, int  side, long  x , long  y , long  *scroll_x , long  *scroll_y)
{
    switch (side) {
        case scroll__LEFT:
            *scroll_x = x;
            *scroll_y = self->top + y;
            break;
	case scroll__RIGHT:
		*scroll_x = (self)->GetLogicalWidth() - 1 - x;
		*scroll_y = self->top + y;
	    break;
        case scroll__TOP:
            *scroll_x = self->left + y;
            *scroll_y = x;
            break;
	case scroll__BOTTOM:
		*scroll_x = self->left + y;
		*scroll_y = (self)->GetLogicalHeight() - 1 - x;
            break;
    }
}



static long from_range_to_bar(class oscroll  *self, int  side, struct scrollbar  *bar, long  posn)
{
    int endzones = bar->endzones ? self->endzoneLength : 0;
    long cords = bar_height(self, side) - 2*(endzones + self->endbarSpace) - 1;
    long retval;

    /* chosen because 1M x 2k pixels will just about overflow in 32 bits */
    if (bar->total.end < 1000000) {
        retval = endzones + self->endbarSpace 
                + ((posn - bar->total.beg) * cords + (bar->total.end - bar->total.beg) / 2) / (bar->total.end - bar->total.beg);
    }
    else {
        retval = endzones + self->endbarSpace 
                + ((long)(((double)(posn - bar->total.beg)) * (double)cords / ((double)(bar->total.end - bar->total.beg)) + .5));
    }
    return retval;
}


static long from_bar_to_range(class oscroll  *self, int  side , struct scrollbar  *bar, long  posn, int  *status)
{
    int endzones = bar->endzones ? self->endzoneLength : 0;
    long height = bar_height(self, side),
        cords = height - 2*(endzones + self->endbarSpace),
        retval;

    if (bar->total.end < 1000000) {
        retval = bar->total.beg + (((bar->total.end - bar->total.beg) * (posn - self->endbarSpace - endzones) + cords / 2) / cords);
    }
    else {
        retval = ((long) ((double)(bar->total.beg) +
                           ((((double)(bar->total.end - bar->total.beg)) *
                              ((double)(posn - self->endbarSpace - endzones))) / (double)cords)
                           + .5));
    }

    if (posn < 0 || posn >= height)
        *status = OUTSIDEBAR;
    else if (posn < endzones)
        *status = INTOPZONE;
    else if (posn >= height - endzones)
        *status = INBOTZONE;
    else
        *status = INSIDEBAR;

    return retval;
}


static void calc_location(class oscroll  *self)
{
    int i, lastlocation;

    /* This routine generates the desired.location from the ideal_location. 
	In doing so, it destroys the left, top, width, and height fields. 
	This works because the two places this routine is called 
	(oscroll__FullUpdate and oscroll__Update) force a full update 
	of the child if  they have changed. */

    /* start with the ideal configuration. */
    self->desired.location = self->ideal_location;
    do {
        self->left = 0;
        self->top = 0;
        self->width = (self)->GetLogicalWidth();
        self->height = (self)->GetLogicalHeight();
        lastlocation = self->desired.location;

	if(emulation) {
	    /* MOTIF SCROLLBARS */
	    /* give some room for the borders */
	    self->left += TD_BORDER;
	    self->top += TD_BORDER;
	    self->width -= (2*TD_BORDER);
	    self->height -= (2*TD_BORDER);
	}
	
        for (i = 0; i < scroll_SIDES; i++)
            if (self->desired.location & (1<<i)) {
                if (bar_height(self, i) < self->bar_threshold) {
                    /* If the scrollbar doesn't fit, turn it off. */
                    self->desired.location &= ~(1<<i);
                }
                else {
                    /* Otherwise allocate its space. */
                    self->left += LeftChange[i] * REALBARWIDTH(self);
                    self->top += TopChange[i] * REALBARWIDTH(self);
                    self->width += WidthChange[i] * REALBARWIDTH(self);
                    self->height += HeightChange[i] * REALBARWIDTH(self);
                }
	    } else if(emulation) {
		/* allocate the space for when the bar ISN'T there */
		self->left += NoLeftChange[i];
		self->top += NoTopChange[i];
		self->width += NoWidthChange[i];
		self->height += NoHeightChange[i];
	    }

        if (self->width < 0 || self->height < 0) {
            /* Turn off all scrollbars if there is no area left over. Happens when you have a very tall and narrow window, for example */
            self->width = (self)->GetLogicalWidth();
            self->height = (self)->GetLogicalHeight();
            self->left = self->top = 0;
            self->desired.location = 0;
        }
    } while (lastlocation != self->desired.location);
    /* Keep turning more and more off until something works. 
	Guaranteed to stop sometime because there are a finite number 
	of scrollbars that can be turned off (since we never turn any back on...) */
}

static void calc_desired(class oscroll  *self)
{
    int i, exists[scroll_TYPES];

    for (i = 0; i < scroll_TYPES; i++) 
        exists[i] = 0;

    for (i = 0; i < scroll_SIDES; i++) 
        if (self->desired.location & (1<<i)) 
            exists[Type[i]] = 1;

    for (i = 0; i < scroll_TYPES; i++) 
        if (exists[i]) {
            struct scrollbar *bar = &self->desired.bar[i];
            getinfo(self, i, &bar->total, &bar->seen, &bar->dot);
        }
}


static int calc_dot(class oscroll  *self, int  side, struct scrollbar  *bar, long  *x1 , long  *y1 , long  *x2 , long  *y2)
{

    if (bar->dot.end < bar->dot.beg || bar->dot.end < bar->total.beg || bar->dot.beg > bar->total.end)
        return 0;

    *y1 = from_range_to_bar(self, side, bar, bar->dot.beg);
    *y2 = from_range_to_bar(self, side, bar, bar->dot.end);

    *x1 = (REALBARWIDTH(self) - self->dotWidth) / 2;
    *x2 = *x1 + self->dotWidth - 1;

    if (bar->dot.beg == bar->dot.end) {
        *y1 -= 1;
        *y2 += 2;
    }
    else if (*y2 - *y1 < MINDOTLENGTH) {
        *y1 = (*y1 + *y2 - MINDOTLENGTH) / 2;
        *y2 = (*y1 + *y2 + MINDOTLENGTH) / 2;
    }

    return 1;
}

static int calc_elevator(class oscroll  *self, int  side, struct scrollbar  *bar, long  *x1 , long  *y1 , long  *x2 , long  *y2)
{
    int min,
        height = bar_height(self, side),
        extra = self->endbarSpace + (bar->endzones ? self->endzoneLength : 0);

    if (bar->seen.end < bar->seen.beg || bar->seen.end < bar->total.beg || bar->seen.beg > bar->total.end)
        return 0;

    *y1 = from_range_to_bar(self, side, bar, bar->seen.beg);
    *y2 = from_range_to_bar(self, side, bar, bar->seen.end);

    if(!emulation) {
	/* NORMAL SCROLLBARS */
	*x1 = 0;
	*x2 = REALBARWIDTH(self) - 2;
    } else {
	/* MOTIF SCROLLBARS */
	*x1 = SBORDER;
	*x2 = REALBARWIDTH(self) - SBORDER;
    }

    if (bar->endzones)
        min = self->min_elevator[1];
    else
        min = self->min_elevator[0];

    if (min > height - 2*extra)
        min = height - 2*extra;

    if (*y2 - *y1 < min) {
        *y1 = (*y1 + *y2 - min) / 2;
        if (*y1 < extra) {
            *y1 = extra;
            *y2 = *y1 + min - 1;
        }
        else if (*y1 + min >= height - extra) {
            *y2 = height - extra - 1;
            *y1 = *y2 - min + 1;
        }
        else
            *y2 = *y1 + min - 1;
    }
    return 1;
} 

static void rectangle(class oscroll  *self, int  x1 , int  y1 , int  x2 , int  y2, class graphic  *tile)
{
   struct rectangle rect;

    rectangle_SetRectSize(&rect, MIN(x1,x2), MIN(y1,y2), ABS(x1-x2) + 1, ABS(y1-y2) + 1);
    (self)->SetTransferMode( graphic_COPY);

    (self)->FillRect( &rect, tile);
}


/* MOTIF SCROLLBARS */
static void draw_bar(class oscroll  *self, int  side, long  tt , long  tb			/* top and bottom of thumb in already-rotated
				   coordianates */)
{
    struct scrollbar *bar = &self->desired.bar[Type[side]];
    long height;
    long x1, y1, x2, y2;

    height = bar_height(self, side);

    /* draw bar above thumb*/
    rotate(self, side, SBORDER , (bar->endzones ? self->endzoneLength : 0),
	   &x1, &y1);
    rotate(self, side, REALBARWIDTH(self) - SBORDER, 0, /* don't care */
	   &x2, &y2);

    if (Type[side] == scroll_VERT)
	rectangle(self, x1, y1, x2 - 1, tt, FILLPAT);
    else
	rectangle(self, x1, y1 - 1, tt, y2, FILLPAT);

    
    /* draw bar below thumb */
    rotate(self, side, SBORDER, 0, /* don't care */
	   &x1, &y1);
    rotate(self, side, REALBARWIDTH(self) - SBORDER,
	   height - (bar->endzones ? self->endzoneLength : 0),
	   &x2, &y2);

    if (Type[side] == scroll_VERT)
	rectangle(self, x1, tb, x2 - 1, y2, FILLPAT);
    else
	rectangle(self, tb, y1 - 1, x2, y2, FILLPAT);
}
    
static void draw_thumb_and_bar(class oscroll  *self, int  side, boolean  force)
{
    long x1, y1, x2, y2;
    static long ox1 = 0, oy1 = 0, ox2 = 0, oy2 = 0;
    int ret;
    struct scrollbar *bar = &self->desired.bar[Type[side]];

    ret = calc_elevator(self, side, bar, &x1, &y1, &x2, &y2);
    
    if (force || ret) {
	if (ret == 0) {		/* use old numbers */
	    x1 = ox1; y1 = oy1;
	    x2 = ox2; y2 = oy2;
	}

	if (!force && (x1 == ox1) && (y1 == oy1) && (x2 == ox2) && (y2 == oy2))
	    return;
	else {
	    ox1=x1; oy1=y1;
	    ox2=x2; oy2=y2;
	}

        rotate(self, side, x1, y1, &x1, &y1);
        rotate(self, side, x2, y2, &x2, &y2);

	motif_Draw3dBorder((class view *)self, x1, y1, x2, y2,
			   TRUE, TD_BACKPAT(self));
	/* pass in the top/left and bottom/right of the new thumb */
	if ((Type[side]) == scroll_VERT)
	    draw_bar(self, side, y1, y2);
	else
	    draw_bar(self, side, x1, x2);
    }
}


#define COPY_PT(s, d) (d).x = (s).x; (d).y = (s).y
#define SWAP_COPY_PT(s, d) (d).x = self->endzoneLength - (s).y; \
    			   (d).y = REALBARWIDTH(self) - (s).x;
    

static void init_arrows(class oscroll  *self)
{
    struct arrow *a, *b;
    int scale3d = (int)(TD_DEPTH * 1.84);

    /* do the up-pointing arrow */
    a = &self->arrows[ARROW_UP];
    a->pa[0].x = (REALBARWIDTH(self) >> 1);	a->pa[0].y = INNERWIDTH + scale3d;
    a->pa[1].x = a->pa[0].x;		a->pa[1].y = a->pa[0].y -
						2 * TD_DEPTH;
    a->pa[4].x = SBORDER;		a->pa[4].y = TD_DEPTH;
    a->pa[2].x = a->pa[4].x + scale3d;	a->pa[2].y = a->pa[4].y +
						TD_DEPTH;
    a->pa[5].x = REALBARWIDTH(self)-SBORDER;	a->pa[5].y = TD_DEPTH;
    a->pa[3].x = a->pa[5].x - scale3d;	a->pa[3].y = a->pa[5].y +
						TD_DEPTH;
    /* now the left-pointing arrow, a -90 degree rotation of the up. */
    b = &self->arrows[ARROW_LEFT];
    SWAP_COPY_PT(a->pa[0], b->pa[0]);	SWAP_COPY_PT(a->pa[1], b->pa[1]);
    SWAP_COPY_PT(a->pa[2], b->pa[2]);	SWAP_COPY_PT(a->pa[3], b->pa[3]);
    SWAP_COPY_PT(a->pa[4], b->pa[4]);	SWAP_COPY_PT(a->pa[5], b->pa[5]);
}
    
static void draw_endzones(class oscroll  *self, int  side , int  height, int  state			/* which arrow on or off */)
{
    long x1, y1, x2, y2;
    struct point pa[6];
    struct point t[4];		/* used for drawing the 3 traps and 1 tri */
    class graphic *lt, *dk;
    struct arrow *a = NULL;
    static int arrows_inited = FALSE;
    struct scrollbar *bar = &self->desired.bar[Type[side]];

    if (!bar->endzones)
	return;

    if (!arrows_inited)
	init_arrows(self);
    
    dk = TD_FGPAT(self);
    lt = TD_BGPAT(self);

    /* get a bounding box for the first arrow */
    rotate(self, side, 0, 0, &x1, &y1);
    rotate(self, side, REALBARWIDTH(self), self->endzoneLength, &x2, &y2);

    /* get a set of offsets to use */
    switch (Type[side]) {
        case scroll_VERT:
	    a = &self->arrows[ARROW_UP];
	    break;
	case scroll_HORIZ:
	    a = &self->arrows[ARROW_LEFT];
	    break;
	}

    pa[0].x = a->pa[0].x + MIN(x1,x2);
    pa[0].y = MAX(y1,y2) - a->pa[0].y;
    pa[1].x = a->pa[1].x + MIN(x1,x2);
    pa[1].y = MAX(y1,y2) - a->pa[1].y;
    pa[2].x = a->pa[2].x + MIN(x1,x2);
    pa[2].y = MAX(y1,y2) - a->pa[2].y;
    pa[3].x = a->pa[3].x + MIN(x1,x2);
    pa[3].y = MAX(y1,y2) - a->pa[3].y;
    pa[4].x = a->pa[4].x + MIN(x1,x2);
    pa[4].y = MAX(y1,y2) - a->pa[4].y;
    pa[5].x = a->pa[5].x + MIN(x1,x2);
    pa[5].y = MAX(y1,y2) - a->pa[5].y;

    /* draw it */
    COPY_PT(pa[0], t[0]);	COPY_PT(pa[1], t[1]);
    COPY_PT(pa[3], t[2]);	COPY_PT(pa[5], t[3]);
    (self)->FillPolygon( t, 4, (state == INTOPZONE) ? lt : dk);
    
    COPY_PT(pa[4], t[0]);	COPY_PT(pa[2], t[1]);
    (self)->FillPolygon( t, 4, (state == INTOPZONE) ? lt : dk);
    
    COPY_PT(pa[1], t[2]);	COPY_PT(pa[0], t[3]);
    (self)->FillPolygon( t, 4, (state == INTOPZONE) ? dk : lt);
    
    COPY_PT(pa[1], t[1]);	COPY_PT(pa[2], t[2]);	COPY_PT(pa[3],t[0]);
    (self)->FillPolygon( t, 3, TD_BACKPAT(self));


    /* get a bounding box for the second arrow */
    rotate(self, side, 0, height - self->endzoneLength, &x1, &y1);
    rotate(self, side, REALBARWIDTH(self), height, &x2, &y2);


    /* translate the offsets from the other arrow to the
       other end of the scrollbar */
    switch(Type[side]) {

        case scroll_VERT:
	    /* x's set from above */
	    a = &self->arrows[ARROW_UP];
	    pa[0].y = y1 + a->pa[0].y;
	    pa[1].y = y1 + a->pa[1].y;
	    pa[2].y = y1 + a->pa[2].y;
	    pa[3].y = y1 + a->pa[3].y;
	    pa[4].y = y1 + a->pa[4].y;
	    pa[5].y = y1 + a->pa[5].y;
	    break;

	case scroll_HORIZ:
	    /* y's set from above */
	    a = &self->arrows[ARROW_LEFT];
	    pa[0].x = x2 - a->pa[0].x;
	    pa[1].x = x2 - a->pa[1].x;
	    pa[2].x = x2 - a->pa[2].x;
	    pa[3].x = x2 - a->pa[3].x;
	    pa[4].x = x2 - a->pa[4].x;
	    pa[5].x = x2 - a->pa[5].x;
	    break;
	}

    /* draw it */
    COPY_PT(pa[0], t[0]);	COPY_PT(pa[1], t[1]);
    COPY_PT(pa[3], t[2]);	COPY_PT(pa[5], t[3]);
    (self)->FillPolygon( t, 4, (state == INBOTZONE) ? lt : dk);
    
    COPY_PT(pa[4], t[0]);	COPY_PT(pa[2], t[1]);
    (self)->FillPolygon( t, 4, (state == INBOTZONE) ? dk : lt);
    
    COPY_PT(pa[1], t[2]);	COPY_PT(pa[0], t[3]);
    (self)->FillPolygon( t, 4, (state == INBOTZONE) ? dk : lt);

    COPY_PT(pa[1], t[1]);	COPY_PT(pa[2], t[2]);	COPY_PT(pa[3],t[0]);
    (self)->FillPolygon( t, 3, TD_BACKPAT(self));
}
			  
static void motif_draw_whole_bar(class oscroll  *self, int  side)
{
    struct scrollbar *bar = &self->desired.bar[Type[side]];
    int height = bar_height(self, side);
    long x1, y1, x2, y2;

    /* clear out area around the scrollbar */
    rotate(self, side, 0, -TD_BORDER - TD_GAP, &x1, &y1);
    rotate(self, side, REALBARWIDTH(self), height + TD_BORDER + TD_GAP, &x2, &y2);
    rectangle(self, x1, y1, x2, y2, TD_BACKPAT(self));

    if (bar->endzones) {
	rotate(self, side, SBORDER, -TD_DEPTH, &x1, &y1);
	rotate(self, side, REALBARWIDTH(self) - SBORDER, self->endzoneLength + SBORDER, &x2, &y2);
	rectangle(self, x1, y1, x2, y2, FILLPAT);

	rotate(self, side, SBORDER, height - SBORDER - self->endzoneLength, &x1, &y1);
	rotate(self, side, REALBARWIDTH(self) - SBORDER, height + TD_DEPTH - 1, &x2, &y2);
	rectangle(self, x1, y1, x2, y2, FILLPAT);
	
	draw_endzones(self, side, height, current_end_state);
    }

    draw_thumb_and_bar(self, side, FALSE);

    /* draw bar border */

    switch (Type[side]) {
        case scroll_VERT:
    	    rotate(self, side, TD_GAP, -TD_BORDER,
		   &x1, &y1);
	    rotate(self, side, REALBARWIDTH(self) - TD_GAP, height + TD_BORDER,
		   &x2, &y2);
	    break;
        case scroll_HORIZ:
    	    rotate(self, side, TD_GAP, -TD_BORDER,
		   &x1, &y1);
	    rotate(self, side, REALBARWIDTH(self) - TD_GAP, height + TD_BORDER,
		   &x2, &y2);
	    break;
	}
    motif_Draw3dBorder(self, x1, y1, x2, y2, FALSE, NULL);
}

static void draw_elevator(class oscroll  *self, int  side)
{
    long x1, y1, x2, y2;
    long left, top, width, height;
    struct scrollbar *bar = &self->desired.bar[Type[side]];

    if (calc_elevator(self, side, bar, &x1, &y1, &x2, &y2)) {

        rotate(self, side, x1, y1, &x1, &y1);
        rotate(self, side, x2, y2, &x2, &y2);

        rectangle(self, x1, y1, x2, y2, self->elevatorFill);
        (self)->SetTransferMode( graphic_COPY);
	left = (x1<x2 ? x1 : x2);
	top =  (y1<y2 ? y1 : y2);
	width = abs(x1-x2);
	height = abs(y1-y2);
	(self)->DrawRectSize(left,top,width,height);
/*
        scroll_MoveTo(self, x1, y1);
        scroll_DrawLineTo(self, x2, y1);
        scroll_DrawLineTo(self, x2, y2);
        scroll_DrawLineTo(self, x1, y2);
        scroll_DrawLineTo(self, x1, y1);
*/
    }
}
    
static void draw_dot(class oscroll  *self, int  side)
{
    long x1, y1, x2, y2;
    long left, top, width, height;
    struct scrollbar *bar = &self->desired.bar[Type[side]];

    if (calc_dot(self, side, bar, &x1, &y1, &x2, &y2)) {
	rotate(self, side, x1, y1, &x1, &y1);
	rotate(self, side, x2, y2, &x2, &y2);

	(self)->SetTransferMode( graphic_COPY);
	/* drawing 2 zero-width rects should probably be faster 
	 than the old code that drew one double width rectangle.
	 Also, this code avoids known bugs in some X servers */
	left = (x1<x2 ? x1 : x2);
	top =  (y1<y2 ? y1 : y2);
	width = abs(x1-x2) ;
	height = abs(y1-y2);
	(self)->DrawRectSize(left,top,width,height);
	left++; top++;
	width = MAX((width - 2),1);
	height = MAX((height -2),1);
	(self)->DrawRectSize(left,top,width,height);
    }
}

static void normal_draw_whole_bar(class oscroll  *self, int  side)
{
    struct scrollbar *bar = &self->desired.bar[Type[side]];
    int height = bar_height(self, side);
    long x1, y1, x2, y2;

    (self)->SetTransferMode( graphic_COPY);

    /* line between bar and child */
    rotate(self, side, REALBARWIDTH(self) - 1, 0, &x1, &y1);
    (self)->MoveTo( x1, y1);
    rotate(self, side, REALBARWIDTH(self) - 1, height /*- 1*/, &x1, &y1);
    (self)->DrawLineTo( x1, y1);

    if (bar->endzones) {
        rotate(self, side, 0, 0, &x1, &y1);
        rotate(self, side, REALBARWIDTH(self) - 2, self->endzoneLength - 1, &x2, &y2);
        rectangle(self, x1, y1, x2, y2, self->endzoneFill);

        rotate(self, side, 0, height - self->endzoneLength, &x1, &y1);
        rotate(self, side, REALBARWIDTH(self) - 2, height - 1, &x2, &y2);
        rectangle(self, x1, y1, x2, y2, self->endzoneFill);
    }

    rotate(self, side, 0, bar->endzones ? self->endzoneLength : 0, &x1, &y1);
    rotate(self, side, REALBARWIDTH(self) - 2, 
		height - (bar->endzones ? self->endzoneLength : 0) - 1, &x2, &y2);
    rectangle(self, x1, y1, x2, y2, self->barFill);

    draw_elevator(self, side);
    draw_dot(self, side);
}


static void erase_dot(class oscroll  *self, int  side, long  top , long  bot)
{
    int i;
    long x1, x2, y1, y2;

    if (top > bot)
        return;

    for (i = 0; i < 2; i++)
	if (INTERSECT(top, bot, range[i].top, range[i].bot)) {
	    erase_dot(self, side, top, range[i].top-1);
	    erase_dot(self, side, range[i].bot+1, bot);
	    return;
	}

    rotate(self, side, left, top, &x1, &y1);
    rotate(self, side, right, bot, &x2, &y2);
    rectangle(self, x1, y1, x2, y2, 
		INTERSECT(top, bot, seen_top, seen_bot) ? self->elevatorFill : self->barFill);
}

static void draw_end_line(class oscroll  * self,long  x1,long  y1,long  x2,long  y2 )
{
    long temp;
    /* first order everything */
    if (x1>x2) { temp = x1; x1 = x2; x2 = temp; }
    if (y1>y2) { temp = y1; y1 = y2; y2 = temp; }
    (self)->MoveTo(x1,y1);
    /* Horizontal or vertical? */
    if (x1 == x2) { /* vertical drawing */
	(self)->DrawLineTo(x2,y2+1);
    }
    else {  /* horizontal drawing */
	(self)->DrawLineTo(x2+1,y2);
    }
}

static void move_elevator(class oscroll  *self, int  side)
{
    struct scrollbar *des = &self->desired.bar[Type[side]],
        *cur = &self->current.bar[Type[side]];
    int dot, i;
    long temp, old_top, old_bot, top, bot;

    if (!calc_elevator(self, side, cur, &left, &range[0].top, &right, &range[0].bot)) {
        range[0].top = range[0].bot = -1;
    }
    range[0].whitep = 0;

    if (!calc_elevator(self, side, des, &left, &range[1].top, &right, &range[1].bot)) {
        range[1].top = range[1].bot = -1;
    }
    range[1].whitep = 1;
    seen_top = range[1].top;
    seen_bot = range[1].bot;

    if (INTERSECT(range[0].top, range[0].bot, range[1].top, range[1].bot)) {
        /* The old range and the new range overlap. We don't need to do all the work. */
        if (range[0].top < range[1].top) {
            SWAP(range[0].bot, range[1].top);
            if (range[1].bot < range[1].top) {
                SWAP(range[1].top, range[1].bot);
                range[1].whitep = 0;
            }
        }
        else {
            SWAP(range[0].top, range[1].bot);
            if (range[0].bot < range[0].top) {
                SWAP(range[0].top, range[0].bot);
                range[0].whitep = 1;
            }
        }
    }

    for (i = 0; i < 2; i++) {
        long ul_x, ul_y, ur_x, ur_y, ll_x, ll_y, lr_x, lr_y;
	struct point tmpPoints[4];

        if (range[i].top != range[i].bot) {
            rotate(self, side, left, range[i].top, &ul_x, &ul_y);
            rotate(self, side, right, range[i].top, &ur_x, &ur_y);
            rotate(self, side, left, range[i].bot, &ll_x, &ll_y);
            rotate(self, side, right, range[i].bot, &lr_x, &lr_y);
            if (range[i].whitep) {
                rectangle(self, ul_x, ul_y, lr_x, lr_y, self->elevatorFill);
                (self)->SetTransferMode( graphic_COPY);
		/* One last pixel hack works for vertical scroll bars but I don't know about horizontal -- these calculations confuse me as to how they work horizontally -- maybe the rotations do work correctly, but it seems strange. */
		/* end of old way that almost works */
		/* beginning of new way to be tested */
		/* See which of four cases we are interested in: two sides, top U, bottom U, all four -- note that because of the rotation transform, the hints "ul" for upper left, etc, have meaning only for vertical scrollbars */
		/* Vertical or horizontal? */
		if (ll_x != ul_x) { /* upper left and lower left not vertical => horizontal scroll bars */
		    if ((seen_top == range[i].top) && (seen_bot == range[i].bot) ) {
			/* All four, draw a rectangle */
			(self)->DrawRectSize(ur_x, ur_y, lr_x-ur_x, ul_y-ur_y);
		    }
		    else if (seen_top == range[i].top) {
			/* draw left U */
			point_SetPt(&tmpPoints[0],ll_x+1,ll_y);
			point_SetPt(&tmpPoints[1],ul_x,ul_y);
			point_SetPt(&tmpPoints[2],ur_x,ur_y);
			point_SetPt(&tmpPoints[3],lr_x+1,lr_y);
			(self)->DrawPath(tmpPoints,4);
		    }
		    else if (seen_bot == range[i].bot) {
			/* draw right U */
			point_SetPt(&tmpPoints[0],ur_x,ur_y);
			point_SetPt(&tmpPoints[1],lr_x,lr_y);
			point_SetPt(&tmpPoints[2],ll_x,ll_y);
			point_SetPt(&tmpPoints[3],ul_x,ul_y);
			(self)->DrawPath(tmpPoints,4);
		    }
		    else {
			/* draw two horizontal sides only */
			(self)->MoveTo( ul_x, ul_y);
			(self)->DrawLineTo( ll_x, ll_y);
			(self)->MoveTo( ur_x, ur_y);
			(self)->DrawLineTo( lr_x, lr_y);
		    }
		}
		else { /* vertical scroll bars */
		    if ((seen_top == range[i].top) && (seen_bot == range[i].bot) ) {
			/* All four, draw a rectangle  */
			(self)->DrawRectSize(ul_x, ul_y, ur_x-ul_x, ll_y-ul_y);
		    }
		    else if (seen_top == range[i].top) {
			/* draw top U */
			point_SetPt(&tmpPoints[0],ll_x,ll_y+1);
			point_SetPt(&tmpPoints[1],ul_x,ul_y);
			point_SetPt(&tmpPoints[2],ur_x,ur_y);
			point_SetPt(&tmpPoints[3],lr_x,lr_y+1);
			(self)->DrawPath(tmpPoints,4);
		    }
		    else if (seen_bot == range[i].bot) {
			/* draw bottom U */
			point_SetPt(&tmpPoints[0],ur_x,ur_y);
			point_SetPt(&tmpPoints[1],lr_x,lr_y);
			point_SetPt(&tmpPoints[2],ll_x,ll_y);
			point_SetPt(&tmpPoints[3],ul_x,ul_y);
			(self)->DrawPath(tmpPoints,4);
		    }
		    else {
			/* draw two vertical sides only */
			(self)->MoveTo( ul_x, ul_y);
			(self)->DrawLineTo( ll_x, ll_y + 1 );
			(self)->MoveTo( ur_x, ur_y);
			(self)->DrawLineTo( lr_x, lr_y + 1);
		    }
		}
	    } /* end of test for any white part to be drawn */
            else {
                rectangle(self, ul_x, ul_y, lr_x, lr_y, self->barFill);
                (self)->SetTransferMode( graphic_COPY);
                if (seen_top == range[i].bot) {
		    draw_end_line(self,ll_x,ll_y,lr_x,lr_y);
/*                    scroll_MoveTo(self, ll_x, ll_y);
                    scroll_DrawLineTo(self, lr_x, lr_y);  */
                }
                if (seen_bot == range[i].top) {
		    draw_end_line(self,ul_x,ul_y,ur_x,ur_y);
/*                    scroll_MoveTo(self, ul_x, ul_y);
                    scroll_DrawLineTo(self, ur_x, ur_y); */
                }
            }
        }
    }

    dot = calc_dot(self, side, des, &left, &top, &right, &bot);
    if (calc_dot(self, side, cur, &left, &old_top, &right, &old_bot) && (!dot || top != old_top || bot != old_bot))
        erase_dot(self, side, old_top, old_bot);

    if (dot)
        draw_dot(self, side);
}


static void 
normal_full_update(class oscroll  *self, enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
    int i;
    struct rectangle rect, crect, VB;

    (self)->GetVisualBounds( &VB);
    self->force_full_update = FALSE;
    if (self->force_get_interface)
        for (i = 0; i < scroll_TYPES; i++)
            self->fns[i] = NULL;

    if (self->child) {
	(self)->RetractViewCursors(self->child);
        rectangle_SetRectSize(&rect, self->left, self->top, self->width, self->height);
        (self->child)->InsertView( self, &rect);
        rectangle_SetRectSize(&crect, left, top, width, height);
        rectangle_IntersectRect(&crect, &crect, &rect);

        (self->child)->FullUpdate( type, crect.left - self->left, crect.top - self->top, crect.width, crect.height);
    }
    /* If visual rectangle is bogus-- leave. */
    if(type == view_Remove || rectangle_IsEmptyRect(&VB) ) return;
    if(type != view_MoveNoRedraw) {
        /* Is it possible that the scrollee has changed his mind about the scrollbar locations now that he has been redrawn? If so, we need to account for his area changing, and recalling his full update. Then what if he changes his mind again? */
        if(self->child) calc_desired(self);
	if (self->left > 0) {
	    if (self->top > 0) {
		rectangle(self, 0, 0, self->left - 1, self->top - 1, self->cornerFill);
	    }
	    if (self->height + self->top < (self)->GetLogicalHeight()) {
		rectangle(self, 0, self->height + self->top, self->left - 1, (self)->GetLogicalHeight() - 1, self->cornerFill);
	    }
	}
	if (self->width + self->left < (self)->GetLogicalWidth()) {
	    int left = self->width + self->left;

	    if (self->top > 0) {
		rectangle(self, left, 0, (self)->GetLogicalWidth() - 1, self->top - 1, self->cornerFill);
	    }
	    if (self->height + self->top < (self)->GetLogicalHeight()) {
		rectangle(self, left, self->height + self->top, (self)->GetLogicalWidth() - 1, (self)->GetLogicalHeight() - 1, self->cornerFill);
	    }
	}
    }
    for (i = 0; i < scroll_SIDES; i++)
        if (self->desired.location & (1<<i)) {
            self->desired.bar[Type[i]].endzones = bar_height(self, i) >= self->endzone_threshold;
            normal_draw_whole_bar(self, i);
            switch (i) {
                case scroll__LEFT:
		    rectangle_SetRectSize(&rect, 0, self->top, REALBARWIDTH(self), self->height);
                    break;
                case scroll__RIGHT:
		    rectangle_SetRectSize(&rect, self->left + self->width, self->top, REALBARWIDTH(self), self->height);
                    break;
                case scroll__TOP:
		    rectangle_SetRectSize(&rect, self->left, 0, self->width, REALBARWIDTH(self));
                    break;
                case scroll__BOTTOM:
		    rectangle_SetRectSize(&rect, self->left, self->top + self->height, self->width, REALBARWIDTH(self));
                    break;
            }
            (self)->PostCursor( &rect, self->curse[i]);
        }
        else
            (self)->RetractCursor( self->curse[i]);

    if(type != view_MoveNoRedraw){
	self->current = self->desired;
	(self)->FlushGraphics();
    }
}


static void motif_full_update(class oscroll  *self, enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
    int i;
    struct rectangle rect, crect, VB;

    (self)->GetVisualBounds( &VB);
    self->force_full_update = FALSE;
    if (self->force_get_interface)
	for (i = 0; i < scroll_TYPES; i++)
	    self->fns[i] = NULL;

    /* clear out space between views' border and the view */

    motif_DrawBorder(self, self->left - TD_BORDER + TD_DEPTH + 0,
		      self->top - TD_BORDER + TD_DEPTH + 0,
		      self->left + self->width + TD_BORDER - TD_DEPTH - 0,
		      self->top + self->height + TD_BORDER - TD_DEPTH - 0,
		      (self)->WhitePattern(), (self)->WhitePattern(),
		      NULL, TD_BORDER + TD_DEPTH);

    /* do the border around the view */

    motif_Draw3dBorder(self, self->left - TD_BORDER + 0,
			self->top - TD_BORDER + 0,
			self->left + self->width + TD_BORDER - 0,
			self->top + self->height + TD_BORDER - 0,
			FALSE, NULL);

    if (self->child) {
	rectangle_SetRectSize(&rect, self->left, self->top, self->width,
			      self->height);
	(self->child)->InsertView( self, &rect);
	rectangle_SetRectSize(&crect, left, top, width, height);
	rectangle_IntersectRect(&crect, &crect, &rect);

	(self->child)->FullUpdate( type, crect.left - self->left,
			crect.top - self->top, crect.width, crect.height);
	/* Is it possible that the scrollee has changed his mind about the
	 scrollbar locations now that he has been redrawn? If so, we need to
	 account for his area changing, and recalling his full update. Then
	     what if he changes his mind again? */
	calc_desired(self);
    }

    /* If visual rectangle is bogus-- leave. */
    if(type == view_Remove || rectangle_IsEmptyRect(&VB) )
	return;

    if (type != view_FullRedraw && type != view_LastPartialRedraw)
	return;

    /* First, draw the border and background */
    for (i = 0; i < scroll_SIDES; i++)
	if (self->desired.location & (1<<i)) {
	    self->desired.bar[Type[i]].endzones = bar_height(self, i) >=
	      self->endzone_threshold;
	    switch (i) {
		case scroll__LEFT:
		    rectangle_SetRectSize(&rect, SBORDER, self->top,
					  REALBARWIDTH(self), self->height);
		    break;
		case scroll__RIGHT:
		    rectangle_SetRectSize(&rect, self->left + self->width + TD_BORDER + SBORDER, self->top,
					  REALBARWIDTH(self), self->height);
		    break;
		case scroll__TOP:
		    rectangle_SetRectSize(&rect, self->left - TD_BORDER, SBORDER,
					  self->width, REALBARWIDTH(self));
		    break;
		case scroll__BOTTOM:
		    rectangle_SetRectSize(&rect, self->left - TD_BORDER, self->top + self->height + TD_BORDER + SBORDER,
					  self->width, REALBARWIDTH(self));
		    break;
	    }
	    (self)->PostCursor( &rect, self->curse[0]);
	} else {		/* the bar isn't there */
	    (self)->RetractCursor( self->curse[0]);
	    switch (i) {
		case scroll__LEFT:
		    rectangle(self, 0, 0,
			      TD_GAP, (self)->GetLogicalHeight(),
			      TD_BACKPAT(self));
		    break;
		case scroll__RIGHT:
		    rectangle(self, (self)->GetLogicalWidth(), 0,
			      (self)->GetLogicalWidth() - TD_GAP,
			      (self)->GetLogicalHeight(), TD_BACKPAT(self));
		    break;
		case scroll__TOP:
		    rectangle(self, 0, 0, (self)->GetLogicalWidth(),
			      TD_GAP, TD_BACKPAT(self));
		    break;
		case scroll__BOTTOM:
		    rectangle(self, 0, (self)->GetLogicalHeight(),
			      (self)->GetLogicalWidth(),
			      (self)->GetLogicalHeight() - TD_GAP,
			      TD_BACKPAT(self));
	    }
	}


    /* Next, fill in the corners */

    if (self->left - TD_BORDER > 0) {
	if (self->top - TD_BORDER > 0) {
	    rectangle(self, 0, 0, self->left - TD_BORDER, self->top - TD_BORDER,
		      TD_BACKPAT(self));
	}

	if (self->height+self->top+2*TD_BORDER < (self)->GetLogicalHeight()) {
	    rectangle(self, 0, self->height + self->top + TD_BORDER,
		      self->left - TD_BORDER,
		      (self)->GetLogicalHeight(), TD_BACKPAT(self));
	}
    }

    if (self->width + self->left + 2*TD_BORDER< (self)->GetLogicalWidth()) {
	int left = self->width + self->left + TD_BORDER;

	if (self->top - TD_BORDER > 0) {
	    rectangle(self, left, 0, (self)->GetLogicalWidth(),
		      self->top - TD_BORDER, TD_BACKPAT(self));
	}

	if (self->height + self->top + TD_BORDER< (self)->GetLogicalHeight()) {
	    rectangle(self, left, self->height + self->top + TD_BORDER,
		      (self)->GetLogicalWidth(),
		      (self)->GetLogicalHeight(), TD_BACKPAT(self));
	}
    }


    /* Now, draw the scrollbars themselves */

    for (i = 0; i < scroll_SIDES; i++)
	if (self->desired.location & (1<<i)) {
	    motif_draw_whole_bar(self, i);
	    draw_thumb_and_bar(self, i, TRUE);
	}

    self->current = self->desired;
    (self)->FlushGraphics();
}


static void full_update(class oscroll  *self, enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
    if(!emulation) {
	/* NORMAL SCROLLBARS */
	normal_full_update(self, type, left, top, width, height);
    } else {
	/* MOTIF SCROLLBARS */
	motif_full_update(self, type, left, top, width, height);
    }
}

/* Overrides of the view routines: */

void oscroll::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
    this->pending_update = 0;
    if (type == view_FullRedraw || type == view_LastPartialRedraw) {

	this->cornerFill = (this)->GrayPattern(environ::GetProfileInt("CornerShade",8),16);
	this->endzoneFill = (this)->GrayPattern(environ::GetProfileInt("EndZoneShade",2),16);
	this->barFill = (this)->GrayPattern(environ::GetProfileInt("BackgroundShade",4),16);
	this->whiteFill =(this)->WhitePattern();
	this->elevatorFill =  (this)->GrayPattern(environ::GetProfileInt("ElevatorShade",0),16);
        calc_location(this);
        calc_desired(this);
    }
    full_update(this, type, left, top, width, height);
}

void normal_scroll__Update(class oscroll  *self)
{
    int i;
    long l, t, w, h;

    l = (self)->GetVisualLeft();
    t = (self)->GetVisualRight();
    w = (self)->GetVisualWidth();
    h = (self)->GetVisualHeight();

    if (w == 0 || h == 0) return;

    self->pending_update = 0;
    /* Let the children modify their state however they want. */
    (self->updatelistp)->Clear();

    if (self->thumbing == YES)
	return;

    calc_location(self);
    calc_desired(self);

    /* Is the change so drastic that we need to start from scratch? */
    if (self->current.location != self->desired.location || self->force_full_update) {
	rectangle(self, l, t, w, h, self->whiteFill);
        full_update(self, view_FullRedraw, l, t, w, h);
    }
    else {
        for (i = 0; i < scroll_SIDES; i++)
            if (self->desired.location & (1<<i)) {
                int type = Type[i];
                struct scrollbar *des = &self->desired.bar[type], *cur = &self->current.bar[type];

                if (des->endzones != cur->endzones)
                    /* The entire scrollbar need redrawing? */
                    normal_draw_whole_bar(self, i);
                else if (des->total.beg != cur->total.beg || des->total.end != cur->total.end || des->seen.beg != cur->seen.beg || des->dot.beg != cur->dot.beg || des->seen.end != cur->seen.end || des->dot.end != cur->dot.end) {
                    move_elevator(self, i);
                }
            }
        self->current = self->desired;
    }
}

void motif_scroll__Update(class oscroll  *self)
{
    int i;
    long l, t, w, h;

    l = (self)->GetVisualLeft();
    t = (self)->GetVisualRight();
    w = (self)->GetVisualWidth();
    h = (self)->GetVisualHeight();

    if (w == 0 || h == 0) return;

    self->pending_update = 0;
    /* Let the children modify their state however they want. */
    (self->updatelistp)->Clear();

    if (self->thumbing == YES) {
	return;
    }
    calc_location(self);
    calc_desired(self);

    /* Is the change so drastic that we need to start from scratch? */
    if (self->current.location != self->desired.location ||
	self->force_full_update) {
        rectangle(self, l, t, w, h, TD_BACKPAT(self));
        full_update(self, view_FullRedraw, l, t, w, h);
    }
    else {
        for (i = 0; i < scroll_SIDES; i++)
            if (self->desired.location & (1<<i)) {
		draw_thumb_and_bar(self, i, FALSE);
            }
        self->current = self->desired;
    }
}

void oscroll::Update()
{
    if(!emulation) {
	/* NORMAL SCROLLBARS */
	normal_scroll__Update(this);
    } else {
	/* MOTIF SCROLLBARS */
	motif_scroll__Update(this);
    }
}


void oscroll::WantUpdate(class view  *requestor)
{
    if ((class view *)this != requestor)
        (this->updatelistp)->AddTo( requestor);

    if (!this->pending_update) {
        this->pending_update = 1;
        if(((class view *)this)->parent) (((class view *)this)->parent)->WantUpdate( this);
    }
}
/*
 * What to do every time the timer repeat fires.
 * We separate this from RepeatScroll so that we can call
 * it right when we get a mouse hit.  This way there is
 * no scroll latency.
 */
static void RepeatEvent(class oscroll  *self)
{
    struct scrollbar *cur = NULL;
    static long lastcoord = 0;
    

    if (lastcoord == 0)
	lastcoord = self->hitcoord;
    
    if (self->side != -1) {
        cur = &self->current.bar[Type[self->side]];
    }

    if (current_end_state != 0) {
	endzone(self, self->side, (current_end_state == INTOPZONE ?
				   scroll_TOPENDZONE : scroll_BOTTOMENDZONE),
				   view_LeftDown);
    } else {
	switch (self->direction) {
	    case scroll_DOWN:
	        set_frame(self, self->side, what_is_at(self, self->side, self->hitcoord), 0);
		break;
	    case scroll_UP:
		set_frame(self, self->side, what_is_at(self, self->side, self->hitcoord), bar_height(self, self->side));
		break;
	    }
    }

    draw_thumb_and_bar(self, self->side, TRUE);

}

static void RepeatScroll(class oscroll  *self, long  cTime)
{
    struct scrollbar *cur = NULL;
    long timeInterval;

    self->scrollEvent=NULL;
    if(!emulation) {
	/* NORMAL SCROLLBARS */
	self->button |= SMOOTH;

	timeInterval = minContScrollTime + (((maxContScrollTime - minContScrollTime) * self->hitcoord) / (PIXELSPERINCH * 10));

	self->scrollEvent = im::EnqueueEvent((event_fptr) RepeatScroll, (char *) self, event_MSECtoTU(timeInterval));

	if (self->side != -1) {
	    cur = &self->current.bar[Type[self->side]];
	}

	if (self->button == (LEFT | SMOOTH))
	    set_frame(self, self->side, what_is_at(self, self->side, self->hitcoord), 0);
	else if (self->button == (RIGHT | SMOOTH))
	    set_frame(self, self->side, cur->seen.beg, self->hitcoord);

	self->thumbing = NOPE;

    } else {

	timeInterval = minContScrollTime +
	  (((maxContScrollTime - minContScrollTime) * self->hitcoord) /
	   (PIXELSPERINCH * 10));

	/* try and stop re-entrancy */
	if (self->scrollEvent != NULL) {
	    (self->scrollEvent)->Cancel();
	    self->scrollEvent = NULL;
	}

	self->scrollEvent = im::EnqueueEvent((event_fptr)RepeatScroll, (char *)self,
					    event_MSECtoTU(timeInterval));

	RepeatEvent(self);
    }
}


class view *normal_scroll__Hit(class oscroll  *self, enum view_MouseAction  action, long  x , long  y , long  num_clicks)
{
    int posn = 0, status,side = 0, delta, i, endzones;
    long coord = 0, temp, y1, y2;
    struct scrollbar *cur = NULL, *des = NULL;
    long logicalTop, logicalHeight, logicalPos;

    if (action == view_LeftDown || action == view_RightDown) {
	if (self->button != NEITHER)
	    return (class view *)self;            /* We already are dealing with the other button. */

	if (x >= self->left 
	    && y >= self->top 
	    && x < self->left + self->width 
	    && y < self->top + self->height 
	    && self->child) {
	    class view *retval = (self->child)->Hit( action, x - self->left, y - self->top, num_clicks);
	    return retval;
	}
	else if (x < self->left && (self->current.location & scroll_LEFT)) {
	    cur = &self->current.bar[Type[side = scroll__LEFT]];
	    posn = from_bar_to_range(self, side, cur, self->hitcoord = (y - self->top), &status);
	}
	else if (x >= self->left + self->width && (self->current.location & scroll_RIGHT)) {
	    cur = &self->current.bar[Type[side = scroll__RIGHT]];
	    posn = from_bar_to_range(self, side, cur, self->hitcoord = (y - self->top), &status);
	}
	else if (y < self->top && (self->current.location & scroll_TOP)) {
	    cur = &self->current.bar[Type[side = scroll__TOP]];
	    posn = from_bar_to_range(self, side, cur, self->hitcoord = (x - self->left), &status);
	}
	else if (y >= self->top + self->height && (self->current.location & scroll_BOTTOM)) {
	    cur = &self->current.bar[Type[side = scroll__BOTTOM]];
	    posn = from_bar_to_range(self, side, cur, self->hitcoord = (x - self->left), &status);
	}
	else
	    status = OUTSIDEBAR;

	switch (status) {
	    case OUTSIDEBAR:
		self->thumbing = NOPE;
		/* These -1's are dangerous. I am fairly certain they don't get used outside
		 * of this routine, and that they are handled properly within it. But you never
		 * know...
		 */
		self->side = -1;
		self->button = NEITHER;
		break;
	    case INTOPZONE:
	    case INBOTZONE:
		self->thumbing = NOPE;
		self->side = -1;
		self->button = NEITHER;
		endzone(self, side, status, action);
		break;
	    case INSIDEBAR:
		if (calc_elevator(self, side, cur, &temp, &y1, &temp, &y2) && y1 <= self->hitcoord && self->hitcoord <= y2) {
		    self->thumbing = MAYBE;
		    self->seenLength = cur->seen.end - cur->seen.beg;
		}
		else
		    self->thumbing = NOPE;
		self->side = side;

		if (action == view_LeftDown)
		    self->button = LEFT;
		else
		    self->button = RIGHT;
		self->hitposn = posn;

		if (startScrollTime > 0)  {
		    self->scrollEvent = im::EnqueueEvent((event_fptr)RepeatScroll, (char *) self, event_MSECtoTU(startScrollTime));
		}

		break;
	}
	return (class view *)self;
    }

    /* Assure that we have the correct button */
    if (((action == view_LeftMovement || action == view_LeftUp) && ((self->button & LEFT) == 0)) || ((action == view_RightMovement || action == view_RightUp) && ((self->button & RIGHT) == 0)))
	return (class view *)self;

    if (self->side != -1) {
	cur = &self->current.bar[Type[self->side]];
	des = &self->desired.bar[Type[self->side]];
	if (self->side == scroll__LEFT || self->side == scroll__RIGHT) {
	    logicalTop = self->top;
	    logicalHeight = self->height;
	    logicalPos = y;
	}
	else {
	    logicalTop = self->top;
	    logicalHeight = self->height;
	    logicalPos = x;
	}
	coord = logicalPos - logicalTop;
    }

    if (action == view_LeftMovement || action == view_RightMovement) {
	endzones = (cur->endzones ? self->endzoneLength : 0) + self->endbarSpace;

	switch (self->thumbing) {
		long newPos;

	    case MAYBE:
		if (ABS(coord - self->hitcoord) <= SMALLDIST) {
		    break;
		}
		
		self->hitposn = self->current.bar[Type[self->side]].seen.beg;

		self->thumbing = YES;
		if (self->scrollEvent != NULL) {
		    (self->scrollEvent)->Cancel();
		    self->scrollEvent = NULL;
		}
		(self->curse[self->side])->SetStandard( ThumbIcon);
		(self->imPtr)->SetWindowCursor( self->curse[self->side]);
		
		/* Fall through into the yes clause */

	    case YES:
		if (coord == self->hitcoord) {
		    /*
		     No movemove in the cursor - handles bottom of the
		     screen which for some reason the following code
			 doesn't get correct
		    */
		    break;
		}

		if (coord < self->hitcoord) {
		    if (self->hitposn == 0) {
			/*
			 The elevator is at the top of the bar and we want to
			 reset the hitcoordinate.
			 */

			if (coord < logicalTop + endzones) {
			    self->hitcoord = logicalTop + endzones;
			}
			else {
			    self->hitcoord = coord;
			}
			break;
		    }
		    else if (logicalPos > (logicalTop + logicalHeight - endzones)) {
			/*
			 We are moving back up from below the scroll bar
			 Do not start moving backwards until we are back in
			 the scroll bar area
			 */

			self->hitcoord = coord;
			break;
		    }
		}

		newPos = from_range_to_bar(self, self->side, cur, cur->seen.beg) +  coord - self->hitcoord;

		if (newPos < logicalTop + endzones)
		    newPos = logicalTop + endzones;
		if (newPos > logicalTop + logicalHeight - endzones)
		    newPos = logicalTop + logicalHeight - endzones;
		posn = from_bar_to_range(self, self->side, cur, newPos, &status);

		if (status == INSIDEBAR) {
		    delta = posn - self->hitposn;
		    if (delta != 0) {
			des->seen.beg += delta;
			if (des->seen.beg < des->total.beg) {
			    des->seen.beg = des->total.beg;
			    des->seen.end = MIN(des->total.end, des->seen.beg + self->seenLength);
			}
			else if (des->seen.beg > des->total.end) {
			    des->seen.beg = des->total.end;
			    des->seen.end = des->total.end;
			}
			else
			    des->seen.end = MIN(des->total.end, des->seen.beg + self->seenLength);


			for (i = 0; i < scroll_SIDES; i++)  {
			    if ((self->current.location & (1<<i)) && Type[i] == Type[self->side])  {
				move_elevator(self, i);
				if (thumbScroll && self->button == LEFT)  {
				    set_frame(self, self->side, des->seen.beg, 0);
				}
			    }
			}
			*cur = *des;
			self->hitposn = posn;
			self->hitcoord = coord;
		    }
		    else if (self->hitposn == 0 && coord < self->hitcoord) {
			self->hitcoord = coord;
		    }
		}
		break;
	    case NOPE:
		if (adjustScroll)  {
		    self->hitcoord = coord;
		}
		break;
	}
	return (class view *)self;
    }

    /* The action must be an up transition to get to here. */

    if (self->scrollEvent != NULL) {
	(self->scrollEvent)->Cancel();
	self->scrollEvent = NULL;
    }

    if (self->thumbing != NOPE) {
	(self->curse[self->side])->SetStandard( CursorIcon[Type[self->side]]);
	(self->imPtr)->SetWindowCursor( NULL);

	if (ABS(coord - self->hitcoord) > SMALLDIST || (self->thumbing == YES && (self->button == RIGHT || ! thumbScroll))) {
	    long newPos = from_range_to_bar(self, self->side, cur, cur->seen.beg) + coord - self->hitcoord;

	    posn = from_bar_to_range(self, self->side, cur, newPos, &status);
	    if (status == INSIDEBAR) {
		delta = posn - self->hitposn;
		des->seen.beg += delta;
		if (des->seen.beg < des->total.beg)
		    des->seen.beg = des->total.beg;
		else if (des->seen.beg > des->total.end) {
		    des->seen.beg = des->total.end;
		}
	    }
	    set_frame(self, self->side, des->seen.beg, 0);
	    (self)->WantUpdate( self);

	    self->thumbing = YES;
	}
    }


    if (self->thumbing != YES) {
	if (adjustScroll)  {
	    self->hitcoord = coord;
	}

	if (self->button == LEFT)
	    set_frame(self, self->side, what_is_at(self, self->side, self->hitcoord), 0);
	else if (self->button == RIGHT)
	    set_frame(self, self->side, cur->seen.beg, self->hitcoord);
    }

    self->button = NEITHER;
    self->thumbing = NOPE;

    return (class view *)self;
}

class view *motif_scroll__Hit(class oscroll  *self, enum view_MouseAction  action, long  x , long  y , long  num_clicks)
{
    int posn = 0, status, side = 0, delta, i, endzones;
    long coord = 0, temp, y1, y2;
    struct scrollbar *cur = NULL, *des = NULL;
    long logicalTop, logicalHeight, logicalPos;
    static int last_hit_side = 0;

    if (action == view_LeftDown || action == view_RightDown) {
	if (self->button != NEITHER)
	    /* We already are dealing with the other button. */
	    return (class view *)self;

	if (x >= self->left 
	    && y >= self->top 
	    && x < self->left + self->width 
	    && y < self->top + self->height 
	    && self->child) {
	    class view *retval = (self->child)->Hit( action, x - self->left,
					   y - self->top, num_clicks);
	    return retval;
	}
	else if (action == view_RightDown)
	    return (class view *)self;

	else if (x < self->left && (self->current.location & scroll_LEFT)) {
	    cur = &self->current.bar[Type[side = scroll__LEFT]];
	    self->hitcoord = y - self->top;
	    posn = from_bar_to_range(self, side, cur, self->hitcoord, &status);
	}
	else if (x >= self->left + self->width &&
		 (self->current.location & scroll_RIGHT)) {
	    cur = &self->current.bar[Type[side = scroll__RIGHT]];
	    self->hitcoord = y - self->top;
	    posn = from_bar_to_range(self, side, cur, self->hitcoord, &status);
	}
	else if (y < self->top && (self->current.location & scroll_TOP)) {
	    cur = &self->current.bar[Type[side = scroll__TOP]];
	    self->hitcoord = x - self->left;
	    posn = from_bar_to_range(self, side, cur, self->hitcoord, &status);
	}
	else if (y >= self->top + self->height &&
		 (self->current.location & scroll_BOTTOM)) {
	    cur = &self->current.bar[Type[side = scroll__BOTTOM]];
	    self->hitcoord = x - self->left;
	    posn = from_bar_to_range(self, side, cur, self->hitcoord, &status);
	}
	else
	    status = OUTSIDEBAR;

	current_end_state = 0;

	switch (status) {
	    case OUTSIDEBAR:
		self->thumbing = NOPE;
		/* These -1's are dangerous. I am fairly certain they don't get used outside
		 * of this routine, and that they are handled properly within it. But you never
		 * know...
		 */
		self->side = -1;
		self->button = NEITHER;
		break;
	    case INTOPZONE:
	    case INBOTZONE:
		self->direction = (status == INTOPZONE) ?
		  scroll_UP : scroll_DOWN;

		current_end_state = status;
		last_hit_side = side;
		if ((&self->desired.bar[Type[side]])->endzones)
		    draw_endzones(self, side, bar_height(self, side),
				  current_end_state);
		endzone(self, side, (current_end_state == INTOPZONE ?
				     scroll_TOPENDZONE : scroll_BOTTOMENDZONE),
			action);
	    case INSIDEBAR:
		last_hit_side = side;
		if (calc_elevator(self, side, cur, &temp, &y1, &temp, &y2)
		    && y1 <= self->hitcoord && self->hitcoord <= y2) {
		    self->thumbing = MAYBE;
		    self->seenLength = cur->seen.end - cur->seen.beg;
		} else
		    self->thumbing = NOPE;

		self->side = side;

		/* above or below thumb ? */
		self->direction = scroll_NEITHER;

		/* determine if above or below thumb  */
		if (self->hitcoord < y1)
		    self->direction = scroll_UP;

		if (self->hitcoord > y2)
		    self->direction = scroll_DOWN;

		self->side = side;

		if (action == view_LeftDown)
		    self->button = LEFT;
		else
		    self->button = RIGHT;

		self->hitposn = posn;

		RepeatEvent(self);

		if (startScrollTime > 0)  {
		    self->scrollEvent =
		      im::EnqueueEvent((event_fptr)RepeatScroll, (char *)self,
				      event_MSECtoTU(startScrollTime));
		}

		break;
	}
	return (class view *)self;
    }

    /* Assure that we have the correct button */
    if (((action == view_LeftMovement || action == view_LeftUp) &&
	  ((self->button & LEFT) == 0)))
	return (class view *)self;

    if (self->side != -1) {
	cur = &self->current.bar[Type[self->side]];
	des = &self->desired.bar[Type[self->side]];
	if (self->side == scroll__LEFT || self->side == scroll__RIGHT) {
	    logicalTop = self->top;
	    logicalHeight = self->height;
	    logicalPos = y;
	    /*	    coord = y - self->top; */
	} else {
	    logicalTop = self->top;
	    logicalHeight = self->height;
	    logicalPos = x;
	    /*	    coord = x - self->left; */
	}
	coord = logicalPos - logicalTop;
    }

    if (action == view_LeftMovement) {
	endzones = (cur->endzones ? self->endzoneLength : 0);

	switch (self->thumbing) {
		long newPos;

	    case YES:
		DEBUG(("YES hitc: %d coord: %d lt: %d lh: %d lp: %d\n", self->hitcoord, coord, logicalTop, logicalHeight, logicalPos));

		self->direction = scroll_NEITHER;
		if (coord == self->hitcoord) {
		    /* no movement, punt */
		    break;
		}
		if (coord < self->hitcoord) {
		    if (self->hitposn == 0) {
			/* thumb at top, want to reset hitcoord */
			if (coord < logicalTop + endzones) {
			    self->hitcoord = logicalTop + endzones;
			} else {
			    self->hitcoord = coord;
			}
			break;
		    } else if (logicalPos > (logicalTop + logicalHeight -
					     endzones)) {
			/*
			 We are moving back up from below the scroll bar
			 Do not start moving backwards until we are back in
			 the scroll bar area
			 */
			self->hitcoord = coord;
			break;
		    }
		}
		newPos = from_range_to_bar(self, self->side, cur,
					   cur->seen.beg) + coord -
		  self->hitcoord;
		if (newPos < logicalTop + endzones - 9)
		    newPos = logicalTop + endzones - 9;
		if (newPos > logicalTop + logicalHeight - endzones)
		    newPos = logicalTop + logicalHeight - endzones;
		posn = from_bar_to_range(self, self->side, cur, newPos,
					 &status);
		

		if (status == INSIDEBAR || status == INBOTZONE) {

		    delta = posn - self->hitposn;
		    if (delta != 0) {
			DEBUG(("\tdelta: %d newPos: %d posn: %d y: %d\n", delta, newPos, posn, y));
			des->seen.beg += delta;
			if (des->seen.beg < des->total.beg) {
			    des->seen.beg = des->total.beg;
			    des->seen.end = MIN(des->total.end,
						des->seen.beg +
						self->seenLength);
			} else if (des->seen.beg > des->total.end) {
			    des->seen.beg = des->total.end;
			    des->seen.end = des->total.end;
			} else
			    des->seen.end = MIN(des->total.end,
						des->seen.beg +
						self->seenLength);
			DEBUG(("des.seen.beg: %d\n", des->seen.beg));

			for (i = 0; i < scroll_SIDES; i++)  {
			    if ((self->current.location & (1<<i)) &&
				Type[i] == Type[self->side])  {
				draw_thumb_and_bar(self, i, FALSE);
				if (thumbScroll) {
				    set_frame(self, self->side, des->seen.beg, 0);
				}
			    }
			}
			*cur = *des;
			self->hitposn = posn;
			self->hitcoord = coord;
		    } else if (self->hitposn == 0 && coord < self->hitcoord) {
			self->hitcoord = coord;
		    }
		}
		break;
	    case MAYBE:
		self->direction = scroll_NEITHER;
		if (ABS(coord - self->hitcoord) > SMALLDIST) {
		    self->hitcoord = coord;
		    self->hitposn =
		      self->current.bar[Type[self->side]].seen.beg;
		    DEBUG(("MAYBE hc: %d hp: %d y: %d\n", self->hitcoord, self->hitposn, y));
		    self->thumbing = YES;
		    if (self->scrollEvent != NULL) {
			(self->scrollEvent)->Cancel();
			self->scrollEvent = NULL;
		    }
		}
		break;

	    case NOPE:
		if (adjustScroll)  {
		    self->hitcoord = coord;
		}
		break;
	}
	return (class view *)self;
    }

    /* The action must be an up transition to get to here. */

    current_end_state = 0;

    if ((&self->desired.bar[Type[last_hit_side]])->endzones)
	DEBUG(("last hit side %d\n",last_hit_side));
    draw_endzones(self, last_hit_side, bar_height(self, last_hit_side),
		   current_end_state);

    if (self->scrollEvent != NULL) {
	(self->scrollEvent)->Cancel();
	self->scrollEvent = NULL;
    }

    if (self->thumbing != NOPE) {
	(self->curse[0])->SetStandard( NormalIcon);
	(self->imPtr)->SetWindowCursor( NULL);

	if (ABS(coord - self->hitcoord) > SMALLDIST) {
	    long newPos = from_range_to_bar(self, self->side, cur,
					    cur->seen.beg) + coord -
	      self->hitcoord;

	    posn = from_bar_to_range(self, self->side, cur, newPos, &status);

	    if (status == INSIDEBAR || status == INBOTZONE) {
		delta = posn - self->hitposn;
		des->seen.beg += delta;
		if (des->seen.beg < des->total.beg)
		    des->seen.beg = des->total.beg;
		else if (des->seen.beg > des->total.end) {
		    des->seen.beg = des->total.end;
		}
	    }

	    set_frame(self, self->side, des->seen.beg, 0);
	    DEBUG(("beg: %d\n", des->seen.beg));
	    (self)->WantUpdate( self);

	    self->button = NEITHER;
	    self->thumbing = NOPE;

	    return (class view *)self;
	}
    }

    self->thumbing = NOPE;

    if (self->thumbing != YES) {
	if (adjustScroll)  {
	    self->hitcoord = coord;
	}
    }

    if (self->button == LEFT && (action == view_LeftDown ||
				  action == view_LeftMovement))
	set_frame(self, self->side, what_is_at(self, self->side,
					       self->hitcoord), 0);
    else if (self->button == RIGHT)
	set_frame(self, self->side, cur->seen.beg, self->hitcoord);

    self->button = NEITHER;
    self->thumbing = NOPE;

    return (class view *)self;
}

class view *oscroll::Hit(enum view_MouseAction  action, long  x , long  y , long  num_clicks)
{
    if(!emulation) {
	/* NORMAL SCROLLBARS */
	return normal_scroll__Hit(this, action, x, y, num_clicks);
    } else {
	/* MOTIF SCROLLBARS */
	return motif_scroll__Hit(this, action, x, y, num_clicks);
    }
}

void oscroll::LinkTree(class view  *parent)
        {

    (this)->scroll::LinkTree( parent);
    if (this->child)
        (this->child)->LinkTree( this);
}

void oscroll::UnlinkNotification(class view  *unlinkedTree)
        {

    view::UnlinkNotification( unlinkedTree);
    (this->updatelistp)->DeleteTree( unlinkedTree);
}

/* The 3-D drawing routines */

void motif_Draw3dBorder(class view  *v, long  x1 , long  y1 , long  x2 , long  y2, boolean  sense			/* "innie" or "outtie" */, class graphic  *fillp		/* center fill pattern, NULL for none */)
{
    motif_DrawBorder(v, x1, y1, x2, y2,
	       (sense) ? (v)->GrayPattern( TD_BGPATVAL, 16) :
		     (v)->GrayPattern( TD_FGPATVAL, 16),
	       (sense) ? (v)->GrayPattern( TD_FGPATVAL, 16) :
		     (v)->GrayPattern( TD_BGPATVAL, 16),
	       fillp,
	       TD_DEPTH);
}
    

void motif_DrawBorder(class view  *v, long  x1 , long  y1 , long  x2 , long  y2		/* enclosing coords */, class graphic  *lt , class graphic  *dk , class graphic  *fillp /* patterns for light, dark and center */, int  depth			/* depth of border */)
{

    long left, top, width, height;
    long left2, top2, width2, height2;
    long r2_bot, r_bot;

    left = (x1<x2 ? x1 : x2);
    top =  (y1<y2 ? y1 : y2);
    width = abs(x1-x2);
    height = abs(y1-y2);

    left2 = left + depth;
    top2 = top + depth;
    width2 = width - 2*depth;
    height2 = height - 2*depth;

    (v)->FillRectSize( left, top,
			depth, height, lt);
    
    (v)->FillRectSize( left + width - depth,
			top, depth, height, dk);
    
    r_bot = top + height;
    
    r2_bot = top2 + height2;

    if (fillp)
	(v)->FillRectSize( left2, top2,
			    width2, height2,
			    fillp);
    
    (v)->FillTrapezoid( left2, r2_bot, width2,
			 left, r_bot, width, dk);
    
    (v)->FillTrapezoid( left, top, width,
			 left2, top2, width2, lt);

    
    /* stolen from linkview */

}
