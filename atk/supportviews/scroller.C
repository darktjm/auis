/*********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
*/

/* Scrollbar code for ATK based on the original scrollbar code for be2. */



#include <andrewos.h>
ATK_IMPL("scroller.H")
#include <scroller.H>

#include <graphic.H>
#include <view.H>
#include <updatelist.H>
#include <im.H>
#include <cursor.H>
#include <event.H>
#include <environ.H>
#include <sbuttonv.H>
#include <sbutton.H>
#include <region.H>

#include <point.h>
#include <rect.h>

#define ENDTOBARSPACE 4
#define MINDOTLENGTH 6
#define SMALLDIST 5

#define COLORWINDOWPADDING 1
#define COLORENDZONELENGTH 16
#define COLORBARTOBUTTONSPACE 4
#define COLORBARWIDTH 20
#define COLORDOTWIDTH 6
#define COLORELEVATORWIDTH 16
#define COLORDRAWBORDERS 1
/* #define COLORDOTBOTTOMSHADOW "#666" */

#define MONOWINDOWPADDING 0
#define MONOENDZONELENGTH 12
#define MONOBARTOBUTTONSPACE -1
#define MONOBARWIDTH 18
#define MONODOTWIDTH 8
#define MONOELEVATORWIDTH 18
#define MONODRAWBORDERS 0

#define MONOSTYLE 3
#define MONOBARTOP "#ccc"
#define MONOBARFOREGROUND MONOBARTOP
#define MONOBUTTONSTYLE 4
#define MONOBUTTONTOP "#eee"
#define MONOBUTTONTOPSHADOW MONOBUTTONTOP
#define MONOBUTTONBOTTOMSHADOW MONOBUTTONTOP
#define MONODOTSTYLE 4
#define MONODOTTOP "white"
#define MONODOTTOPSHADOW "black"
#define MONODOTBOTTOMSHADOW "black"

#define PIXELSPERINCH(self) 75


/* The descriptions of the different types of scrollerbars */
static int Type[scroller_SIDES] = {scroller_VERT, scroller_VERT, scroller_HORIZ, scroller_HORIZ};    

static int cursortypes[scroller_TYPES]={Cursor_VerticalArrows, Cursor_HorizontalArrows};

static char *InterfaceName[scroller_TYPES] = {"scroller,vertical", "scroller,horizontal"};

#define REALBARWIDTH(self) ((self)->barWidth)
#define WPADDING(x) ((x)->windowPadding)
#define VPADDING(x) ((x)->viewPadding)

/* Useful macros */
#undef MIN
#undef MAX
#undef ABS
#undef SWAP
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ABS(a) ((a) < 0 ? -(a) : (a))
#define SWAP(a,b) (temp)=(a);(a)=(b);(b)=(temp)


#define INTERSECT(top1, bot1, top2, bot2) ((bot1) >= (top2) && (top1) <= (bot2))

#define XYTOHITCOORD(self, side, x, y) ((((side)==scroller__LEFT || (side)==scroller__RIGHT)?(y)-(self)->childrect.top:(x)-(self)->childrect.left))

#define CHILDBARHEIGHT(self, side) ((Type[side]==scroller_VERT)?(self)->childrect.height:(self)->childrect.width)

static struct point arrowpts[]={
    {65, 35},
    {35, 50},
    {65, 65}
};

static struct sbutton_info thebutton={
    NULL,   /* the prefs struct will be filled later */
    "",	    /* the label is empty */
    0,	    /* the rock isn't needed */
    NULL,   /* ditto for the trigger atom */
    FALSE,  /* initially not lit, will be set appropriately */
    TRUE    /* should be drawn as if sensitive */
};

#define DRAWRECT(self, left, top, width, height) (self)->DrawRectSize( left, top, (width)-1, (height)-1)
#define FILLRECT(self, left, top, width, height) (self)->FillRectSize( left, top, (width), (height), NULL)


ATKdefineRegistry(scroller, view, scroller::InitializeClass);
static void draw_arrow(class scroller  *self, int  side, struct rectangle  *r, int  dir, boolean  lit);
static long bar_height(class scroller  *self, int  side);
static void set_frame(class scroller  *self, int  side, long posn, long  coord);
static void endzone(class scroller  *self, int  side, int  end, enum view_MouseAction  action);
static long what_is_at(class scroller  *self, int  side, long coord);
static boolean barrects(class scroller  *self, int  side, struct rectangle  *boxrect , struct rectangle  *topbuttonrect , struct rectangle  *botbuttonrect);
static void compute_inner(class scroller  *self, boolean  draw);
static void calc_desired(class scroller  *self);
static void draw_elevator(class scroller  *self, int  side);
static void draw_dot(class scroller  *self, int  side);
static void move_elevator(class scroller  *self, int  side);
static void draw_bar(class scroller  *self, int  side);
static void draw_everything(class scroller  *self);
static void InitPrefs(class scroller  *self);
static void DoRepeatScroller(class scroller  *self);
static void RepeatScroller(class scroller  *self, long  cTime);
static void RepeatEndZone(class scroller  *self, long  cTime);
static void CancelScrollerEvent(class scroller  *self);
static void ScheduleRepeatEndZone(class scroller  *self);
static void HandleEndZone(class scroller  *self, enum view_MouseAction  action, long  x , long  y);
static void HandleRepeatMode(class scroller  *self, enum view_MouseAction  action, long  x , long  y);
static boolean CheckEndZones(class scroller  *self, enum view_MouseAction  action, long  x , long  y);
static void HandleThumbing(class scroller  *self, enum view_MouseAction  action, long  x , long  y);
static void CheckBars(class scroller  *self, enum view_MouseAction  action, long  x , long  y);
static void MaybeStartThumbing(class scroller  *self, enum view_MouseAction  action, long  x , long  y);


static void draw_arrow(class scroller  *self, int  side, struct rectangle  *r, int  dir, boolean  lit)
{
    struct rectangle t;
    double oldfg[3];

    if (self->emulation) {
        int i;

        (self)->SetTransferMode( graphic_WHITE);
        for(i=0;i<10;i++) {
            int offx=0, offy=0, t;
            switch(i) {
                case 0:
                    offx=(-1);
                    break;
                case 1:
                    offx=(1);
                    break;
                case 2:
                    offy=(-1);
                    break;
                case 3:
                    offy=(1);
                    break;
                case 4:
                    offx=(-1);
                    offy=(-1);
                    break;
                case 5:
                    offx=(1);
                    offy=(1);
                    break;
                case 6:
                    offx=(-1);
                    offy=(1);
                    break;
                case 7:
                    offx=(1);
                    offy=(-1);
                    break;
                case 8:
                    (self)->SetTransferMode( graphic_BLACK);
                    break;
                case 9:
                    offx=(1);
                    break;
                default: ;
            }
            switch(side) {
                case scroller__LEFT:
                case scroller__RIGHT:
                    (self)->MoveTo( offx+r->left+arrowpts[0].x*r->width/100, offy+r->top+(dir<0?r->height:0)+dir*arrowpts[0].y*r->height/100);
                    (self)->DrawLineTo( offx+r->left+arrowpts[1].x*r->width/100, offy+r->top+(dir<0?r->height:0)+dir*arrowpts[1].y*r->height/100);
                    (self)->DrawLineTo( offx+r->left+arrowpts[2].x*r->width/100, offy+r->top+(dir<0?r->height:0)+dir*arrowpts[2].y*r->height/100);
                    break;
                case scroller__TOP:
                case scroller__BOTTOM:
                    t=offx;
                    offx=offy;
                    offy=t;
                    (self)->MoveTo( offx+r->left+(dir<0?r->width:0)+dir*arrowpts[0].y*r->width/100, offy+r->top+arrowpts[0].x*r->height/100);
                    (self)->DrawLineTo( offx+r->left+(dir<0?r->width:0)+dir*arrowpts[1].y*r->width/100, offy+r->top+arrowpts[1].x*r->height/100);
                    (self)->DrawLineTo( offx+r->left+(dir<0?r->width:0)+dir*arrowpts[2].y*r->width/100, offy+r->top+arrowpts[2].x*r->height/100);
                    break;
            }
        }
        (self)->SetTransferMode( graphic_SOURCE);
    }
    else { /* not emulating motif */
	class ScrollRegion *s=NULL;
	if(Type[side]==scroller_VERT) {
	    if(dir>0) s=self->regions[scroller_BottomEnd];
	    else s=self->regions[scroller_UpperEnd];
	} else {
	    if(dir>0) s=self->regions[scroller_RightEnd];
	    else s=self->regions[scroller_LeftEnd];
	}
	if(s==NULL) {
	    thebutton.prefs=self->buttonprefs;
	    thebutton.lit=lit;
	    sbuttonv::SafeDrawButton(self, &thebutton, r);
	    t.left = r->left + r->width/2 - 3;
	    t.width = 6 + (r->width % 2);
	    t.top = r->top + r->height/2 - 2;
	    t.height = 4 + (r->height % 2);
	    thebutton.lit = !lit;
	    sbuttonv::SafeDrawButton(self, &thebutton, &t);
	} else {
	    s->SetHighlight(lit);
	    s->ComputeRect(*self, Type[side], self->interiors[side], *r);
	    s->Draw(*self, Type[side], view_FullRedraw, *r);
	}
    }

    if (!self->drawborder) {
        /* reuse t to calc gray area next to button */
        t = *r;
        switch (side) {
            case scroller__LEFT:
            case scroller__RIGHT:
                t.height = self->buttonSpace;
                if (dir == 1) {
                    t.top += r->height;
                }
                else {
                    t.top -= self->buttonSpace;
                }
                break;
            case scroller__TOP:
            case scroller__BOTTOM:
                t.width = self->buttonSpace;
                if (dir == 1) {
                    t.left += r->width;
                }
                else {
                    t.left -= self->buttonSpace;
                }
                break;
        }

        (self)->GetFGColor( oldfg, oldfg+1, oldfg+2);
        (self)->SetFGColor( self->mattebackground[0], self->mattebackground[1], self->mattebackground[2]);
        (self)->SetTransferMode( graphic_SOURCE);
        if (t.width > 0 && t.height > 0) {
            (self)->FillRect( &t, NULL);
        }
        (self)->SetFGColor( oldfg[0], oldfg[1], oldfg[2]);
    }
}

/* Creation and Destruction routines. */

boolean scroller::InitializeClass()
{
 
    return TRUE;
}

void scroller::SetRegion(enum scroller_Regions sr, class ScrollRegion *r) {
    regions[sr]=r;
    r->SetBaseSButtonPrefs(elevatorprefs);
}
    
static const char scrollbarstr[]="scroll";
static const char scrollbarmatte[]="scrollmatte";
static const char scrollbarbox[]="scrollbar";
static const char scrollbarbutton[]="scrollbutton";
static const char scrollbarelevator[]="scrollelevator";
static const char scrollbardot[]="scrolldot";

scroller::scroller()
{
	ATKinit;

    int i;

    this->prefsready = FALSE;

    this->emulation = environ::GetProfileSwitch("MotifScrollbars", FALSE);
    this->startScrollerTime = environ::GetProfileInt("StartScrollTime", 0);
    this->minContScrollerTime = environ::GetProfileInt("ContScrollTime", 500);
    this->maxContScrollerTime = environ::GetProfileInt("MaxContScrollTime", this->minContScrollerTime);
    this->endzonereptime = environ::GetProfileInt("ButtonRepeatTime", 100);
    this->adjustScroller = environ::GetProfileSwitch("AdjustScroll", FALSE);
    this->thumbScroller = environ::GetProfileSwitch("ThumbScroll", FALSE);

    this->side=(-1);
    this->lastwidth=(-1);
    this->lastheight=(-1);
    this->dir=scroller_NOWHERE;
    
    this->current.location = 0;
    this->child = this->scrolleree = NULL;
    for (i = 0; i < scroller_TYPES; i++) {
	this->barcursor[i]=cursor::Create((class view *)this);
	if(this->barcursor[i]!=NULL) (this->barcursor[i])->SetStandard( cursortypes[i]);
    }

    this->desired = this->current;

    this->pending_update = 0;
    
    this->updatelist = new updatelist;
    if(this->updatelist==NULL) THROWONFAILURE( FALSE);
    
    this->ideal_location = scroller_LEFT;
    
    this->endzone_threshold = 80;
    this->bar_threshold = 0;
    this->endbarSpace = ENDTOBARSPACE;

    this->min_elevator[0] = 5;
    this->min_elevator[1] = 18;
    this->force_full_update = FALSE;
    this->scrollerEvent = NULL;

    this->barbackground[0] = this->barbackground[1] = this->barbackground[2] = -1.0;

    this->cursor = cursor::Create((class view *)this);

    if(this->cursor!=NULL) (this->cursor)->SetStandard( Cursor_Octagon);

    this->mousestate=scroller_NOTHING;
    this->prefs = this->boxprefs = this->matteprefs = this->buttonprefs = this->elevatorprefs = this->dotprefs = NULL;
    this->si=NULL;
    for(i=0;i<NUM_SCROLLERREGIONS;i++) {
	this->regions[i]=NULL;
    }
    custom=FALSE;
    THROWONFAILURE( TRUE);
}
    
#define sfree(x)  do { if(x) sbutton::FreePrefs(x); x=NULL; } while (0)

scroller::~scroller()
{
	ATKinit;

    int i;
    if (this->child != NULL)
        (this->child)->UnlinkTree();

    if(this->updatelist) delete this->updatelist;
    
    if(this->cursor) delete this->cursor;

    for(i=0;i<scroller_TYPES;i++) if(this->barcursor[i]!=NULL) delete this->barcursor[i];
    
    sfree(this->prefs);
    sfree(this->boxprefs);
    sfree(this->matteprefs);
    sfree(this->buttonprefs);
    sfree(this->elevatorprefs);
    sfree(this->dotprefs);

    if(!custom) {
	delete regions[scroller_Elevator];
	delete regions[scroller_Dot];
	delete regions[scroller_UpperEnd];
	delete regions[scroller_LeftEnd];
	delete regions[scroller_BottomEnd];
	delete regions[scroller_RightEnd];
    }
}


static class scroller *CreateBasicScroller(class view  *scrolleree, int  location, char  *name)

{

    struct ATKregistryEntry  *oscroller;
    
    class scroller *retval=NULL;

    if(name==NULL) name=environ::GetProfile("ScrollerClass");
    
    if(name!=NULL && (oscroller=ATK::LoadClass(name))!=NULL && (oscroller)->IsType( &scroller_ATKregistry_ )) retval= (class scroller *)ATK::NewObject(name);
    
    if(retval==NULL) {
	retval=new scroller;
	if(retval==NULL) return NULL;
    }

    (retval)->SetView( scrolleree);
    (retval)->SetLocation( location);

    return retval;
}
#define scroller_LEFT (1<<scroller__LEFT)
#define scroller_RIGHT (1<<scroller__RIGHT)
#define scroller_TOP (1<<scroller__TOP)
#define scroller_BOTTOM (1<<scroller__BOTTOM)

static void AddStandardRegions(class scroller *s, int location) {
    s->SetRegion(scroller_Elevator, new ScrollRegionElevator);
    s->SetRegion(scroller_Dot, new ScrollRegionDot);
    if(location&(scroller_LEFT|scroller_RIGHT)) {
	s->SetRegion(scroller_UpperEnd, new ScrollRegionEndzone);
	s->SetRegion(scroller_BottomEnd, new ScrollRegionEndzone);
    }
    if(location&(scroller_TOP|scroller_BOTTOM)) {
	s->SetRegion(scroller_LeftEnd, new ScrollRegionEndzone);
	s->SetRegion(scroller_RightEnd, new ScrollRegionEndzone);
    }
}

class scroller *scroller::CreateScroller(class view  *scrolleree, int  location, char  *name)

{
    ATKinit;

    class scroller *s=CreateBasicScroller(scrolleree,location, name);
    if(s) {
	AddStandardRegions(s, location);
	s->si=new ScrollInterfaceClassic(scrolleree);
    }
    return s;
}

class scroller *scroller::CreateScroller(class ScrollInterface *is, class view *scrollee, int location, char *name) {
    ATKinit;
    class scroller *s=CreateBasicScroller(scrollee, location, name);
    if(s) {
	AddStandardRegions(s, location);
	s->SetInterface(is);
    }
    return s;
}

class scroller *scroller::CreateCustomScroller(class view *scrollee, int location, char *name) {
    ATKinit;
    class scroller *s=CreateBasicScroller(scrollee, location, name);
    if(s) s->custom=TRUE;
    return s;
}


class scroller *scroller::Create(class view  *scrolleree, int  location)
{
	ATKinit;

    return scroller::CreateScroller(scrolleree, location, NULL);
}


/* State modification routines. */

void scroller::SetView(class view  *view)
{

    (this)->SetChild( view);
    (this)->SetScrolleree( view);
}

void scroller::SetChild(class view  *child)
{
    if (this->child != child) {
        this->force_full_update = TRUE;
        if (this->child)
            (this->child)->UnlinkTree();
        this->child = child;
        if (child)
            (child)->LinkTree( this);
        (this)->WantUpdate( this);
    }
}

void scroller::SetScrolleree(class view  *scrolleree)
{
    if (this->scrolleree != scrolleree) {
        this->scrolleree = scrolleree;
        (this)->WantUpdate( this);
    }
}

class view *scroller::GetChild()
{
    return this->child;
}

class view *scroller::GetScrolleree()
{
    return this->scrolleree;
}

void scroller::SetLocation(int  location)
{
    this->ideal_location = location;
    (this)->WantUpdate( this);  
}

int scroller::GetLocation()
{
    return this->ideal_location;
}

int scroller::GetCurrentLocation()
{
    return this->current.location;
}

void scroller::SetParameters(long  endzone , long  bar, int  without , int  with)
{
    this->endzone_threshold = endzone;
    this->bar_threshold = bar;
    this->min_elevator[0] = without;
    this->min_elevator[1] = with;
    (this)->WantUpdate( this);
}

void scroller::GetParameters(long  *endzone , long  *bar, int  *without , int  *with)
{
    *endzone = this->endzone_threshold;
    *bar = this->bar_threshold;
    *without = this->min_elevator[0];
    *with = this->min_elevator[1];
}

void scroller::SetWidth(long  newWidth)
{
    this->barWidth = newWidth;
    
    if (this->dotWidth > newWidth - 2) {
	(this)->SetDotWidth( newWidth - 2);
    }
    (this)->WantUpdate( this);
}

long scroller::GetWidth()
{
    return this->barWidth;
}

void scroller::SetDotWidth(long  newWidth)
{
    if (newWidth > this->barWidth - 2) {
	newWidth = this->barWidth - 2;
    }
    if (newWidth < 0) {
	newWidth = 0;
    }
    this->dotWidth = newWidth;
    (this)->WantUpdate( this);
}

long scroller::GetDotWidth()
{
    return this->dotWidth;
}


void scroller::SetElevatorWidth(long  newWidth)
{
    if(newWidth > this->barWidth) newWidth=this->barWidth;

    if(newWidth<0) newWidth=0;

    this->elevatorWidth=newWidth;
    (this)->WantUpdate( this);
}

long scroller::GetElevatorWidth()
{
    return this->elevatorWidth;
}

void scroller::SetWindowPadding(long  newPadding)
{
    this->windowPadding=newPadding;
}

void scroller::SetViewPadding(long  newPadding)
{
    this->viewPadding=newPadding;
}

long scroller::GetWindowPadding()
{
    return this->windowPadding;
}

long scroller::GetViewPadding()
{
    return this->viewPadding;
}

void scroller::SetEndZoneLength(long  newLength)
{
    this->endzoneLength = newLength;
    if ((this->endzoneLength + this->endbarSpace + this->buttonSpace) * 2 > this->endzone_threshold) {
	this->endzone_threshold = (this->endzoneLength + this->endbarSpace + this->buttonSpace) * 2;
    }
    (this)->WantUpdate( this);
}

long scroller::GetEndZoneLength()
{
    return this->endzoneLength;
}

void scroller::SetEndToBarSpace(long  newSpace)
{
    this->endbarSpace = newSpace;
    if ((this->endzoneLength + this->endbarSpace + this->buttonSpace) * 2 > this->endzone_threshold) {
	this->endzone_threshold = (this->endzoneLength + this->endbarSpace + this->buttonSpace) * 2;
    }
    (this)->WantUpdate( this);
}


long scroller::GetEndToBarSpace()
{
    return this->endbarSpace;
}


/* Calculation routines. */

static long bar_height(class scroller  *self, int  side)
{
    switch(side) {
	case scroller__LEFT:
	case scroller__RIGHT:
	    return self->interiors[side].height;
	case scroller__TOP:
	case scroller__BOTTOM:
	    return self->interiors[side].width;
    }
    return 0;
}


static boolean barrects(class scroller  *self, int  side, struct rectangle  *boxrect , struct rectangle  *topbuttonrect , struct rectangle  *botbuttonrect)
{
    long x1, x2, y1, y2;
    long tx1, tx2, ty1, ty2;
    long bx1, bx2, by1, by2;
    long w, h;
    int c=2;
    long tezl=self->endzoneLength + self->buttonSpace;
    boolean result=TRUE;
    long height;
    
    while(c--) {
	switch(side) {
	    case scroller__LEFT:
	    case scroller__RIGHT:
		y1=self->childrect.top + tezl;
		y2=self->childrect.top + self->childrect.height - 1 - tezl;
		height= (y2 - y1 + 1) + 2 * tezl;
		if(height < self->bar_threshold) result=FALSE;
		
		ty1=self->childrect.top;
		ty2=ty1 + self->endzoneLength - 1;
		by2=self->childrect.top + self->childrect.height - 1;
		by1=by2 - self->endzoneLength + 1;
		break;
	    case scroller__TOP:
	    case scroller__BOTTOM:
		x1= self->childrect.left + tezl;
		x2= self->childrect.left + self->childrect.width - 1 - tezl;
		height= (x2 - x1 + 1) + 2 * tezl;
		if(height < self->bar_threshold) result=FALSE;
		tx1=self->childrect.left;
		tx2=tx1 + self->endzoneLength -1;
		bx2=self->childrect.left + self->childrect.width - 1;
		bx1=bx2 - self->endzoneLength + 1;
		break;
	}
	if(self->child) {
	    switch(side) {
		case scroller__LEFT:
		    x1=bx1=tx1=self->fullinterior.left;
		    x2=bx2=tx2=tx1 + REALBARWIDTH(self) - 1;
		    break;
		case scroller__RIGHT:
		    x2=bx2=tx2=self->fullinterior.left + self->fullinterior.width - 1;
		    x1=bx1=tx1=tx2 - REALBARWIDTH(self) + 1;
		    break;
		case scroller__TOP:
		    y1=by1=ty1=self->fullinterior.top;
		    y2=by2=ty2=ty1 + REALBARWIDTH(self) - 1;
		    break;
		case scroller__BOTTOM:
		    y2=by2=ty2=self->fullinterior.top + self->fullinterior.height - 1;
		    y1=by1=ty1=ty2 - REALBARWIDTH(self) + 1;
	    }
	} else {
	    switch(side) {
		case scroller__LEFT:
		case scroller__RIGHT:
		    {
		    int mid=self->fullinterior.left + self->fullinterior.width / 2;
		    x2=bx2=tx2=mid + REALBARWIDTH(self) / 2;
		    x1=bx1=tx1= x2 - REALBARWIDTH(self)  + (REALBARWIDTH(self)&1);
		    }
		    break;
		case scroller__TOP:
		case scroller__BOTTOM:
		    {
		    int mid=self->fullinterior.top + self->fullinterior.height / 2;
		    y2=by2=ty2=mid + REALBARWIDTH(self) / 2;
		    y1=by1=ty1=y2 - REALBARWIDTH(self)  + (REALBARWIDTH(self)&1);
		    }
		    break;
	    }

	}

	if(boxrect) {
	    w=ABS(x1-x2)+1;
	    h=ABS(y1-y2)+1;
	    x1=MIN(x1,x2);
	    y1=MIN(y1,y2);
	    boxrect->left=x1;
	    boxrect->top=y1;
	    boxrect->width=w;
	    boxrect->height=h;
	}
	
	if(height>=self->endzone_threshold && c!=0) {
	    if(topbuttonrect) {
		w=ABS(tx1-tx2)+1;
		h=ABS(ty1-ty2)+1;
		tx1=MIN(tx1,tx2);
		ty1=MIN(ty1,ty2);
		topbuttonrect->left=tx1;
		topbuttonrect->top=ty1;
		topbuttonrect->width=w;
		topbuttonrect->height=h;
	    }

	    if(botbuttonrect) {
		w=ABS(bx1-bx2)+1;
		h=ABS(by1-by2)+1;
		bx1=MIN(bx1,bx2);
		by1=MIN(by1,by2);
		botbuttonrect->left=bx1;
		botbuttonrect->top=by1;
		botbuttonrect->width=w;
		botbuttonrect->height=h;
	    }
	    self->desired.bar[Type[side]].endzones=TRUE;
	    return result;
	} else {
	    self->desired.bar[Type[side]].endzones=FALSE;
	    if(topbuttonrect) topbuttonrect->width=0;
	    if(botbuttonrect) botbuttonrect->width=0;
	    tezl=self->buttonSpace;
	}
    }
    return result;
}


static void compute_inner(class scroller  *self, boolean  draw)
{
    int i;
    int diff;
    int lastlocation;
    struct rectangle r;
    (self)->GetLogicalBounds( &r);

    self->desired.location=self->ideal_location;
    diff=REALBARWIDTH(self) + VPADDING(self);
    do {
        lastlocation=self->desired.location;

        self->childrect=r;
	if (self->drawborder) {
            /* around the entire window */
            sbuttonv::DrawRectBorder(self, &self->childrect, self->matteprefs, sbuttonv_BORDEROUT, draw, &self->childrect);

	    self->childrect.left+=WPADDING(self);
	    self->childrect.top+=WPADDING(self);
	    self->childrect.width-=WPADDING(self)*2;
	    self->childrect.height-=WPADDING(self)*2;
	    
            self->fullinterior = self->childrect;

	    if(self->child) for (i = 0; i < scroller_SIDES; i++) {
                if (self->desired.location & (1<<i)) {
                    switch(i) {
                        case scroller__LEFT:
                            self->childrect.left+=diff;
                            self->childrect.width-=diff;
                            break;
                        case scroller__RIGHT:
                            self->childrect.width-=diff;
                            break;
                        case scroller__TOP:
                            self->childrect.top+=diff;
                            self->childrect.height-=diff;
                            break;
                        case scroller__BOTTOM:
                            self->childrect.height-=diff;
                            break;
                    }
                }
            }
            if (self->child && self->childrect.width>0 && self->childrect.height>0) {
                /* around the child's portion */
                sbuttonv::DrawRectBorder(self, &self->childrect, self->matteprefs, sbuttonv_BORDERIN, draw, &self->childrect);
            }
	}
        else {
            self->fullinterior = r;

            for (i = 0; i < scroller_SIDES; i++) {
                if (self->desired.location & (1<<i)) {
                    switch(i) {
                        case scroller__LEFT:
                            self->childrect.left+=diff;
                            self->childrect.width-=diff;
                            break;
                        case scroller__RIGHT:
                            self->childrect.width-=diff;
                            break;
                        case scroller__TOP:
                            self->childrect.top+=diff;
                            self->childrect.height-=diff;
                            break;
                        case scroller__BOTTOM:
                            self->childrect.height-=diff;
                            break;
                    }
                }
            }

            for (i = 0; i < scroller_SIDES; i++) {
                int x1, y1, x2, y2;

                if (self->desired.location & (1<<i)) {
                    switch(i) {
                        case scroller__LEFT:
                            x1 = self->childrect.left - 1;
                            y1 = self->childrect.top;
                            x2 = x1;
                            y2 = y1 + self->childrect.height - 1;
                            break;
                        case scroller__RIGHT:
                            x1 = self->childrect.left + self->childrect.width;
                            y1 = self->childrect.top;
                            x2 = x1;
                            y2 = y1 + self->childrect.height - 1;
                            break;
                        case scroller__TOP:
                            x1 = self->childrect.left;
                            y1 = self->childrect.top - 1;
                            x2 = x1 + self->childrect.width - 1;
                            y2 = y1;
                            break;
                        case scroller__BOTTOM:
                            x1 = self->childrect.left;
                            y1 = self->childrect.top + self->childrect.height;
                            x2 = x1 + self->childrect.width - 1;
                            y2 = y1;
                            break;
                    }
		    if(self->child && draw) {
			(self)->MoveTo( x1, y1);
			(self)->DrawLineTo( x2, y2);
		    }
                }
            }
	}

	if(self->child) for (i = 0; i < scroller_SIDES; i++)
	    if (self->desired.location & (1<<i)) {
		if(!barrects(self, i, NULL, NULL, NULL)) self->desired.location &= ~(1<<i);
	    }

	if((self->childrect.height<=0 || self->childrect.width<=0) && self->child!=NULL) self->desired.location=0;
    } while (lastlocation != self->desired.location);
}


static void calc_desired(class scroller  *self)
{
    int i, exists[scroller_TYPES];

    for (i = 0; i < scroller_TYPES; i++) 
        exists[i] = 0;

    for (i = 0; i < scroller_SIDES; i++) 
        if (self->desired.location & (1<<i)) 
            exists[Type[i]] = 1;

}




static boolean calc_elevator(class scroller  *self, int  side, struct scrollerbar  *bar, struct rectangle  *r, boolean latest=TRUE)
{
    long min, height=bar_height(self, side) - self->endbarSpace;
    int diff;
    long x1, x2, y1, y2;
    if(self->regions[scroller_Elevator]==NULL) return FALSE;

    if (bar->endzones)
	min = self->min_elevator[1];
    else
	min = self->min_elevator[0];

    if (min > height) min = height;

    switch(side) {
	case scroller__LEFT:
	case scroller__RIGHT:
	    self->regions[scroller_Elevator]->SetMinHeight(min);
	    x1=self->interiors[side].left;
	    x2=self->interiors[side].left + self->interiors[side].width - 1;
	    diff=self->interiors[side].width - self->elevatorWidth;
	    /* the -2 is so that the elevator can overlap
	     the enclosing box by one pixel */
	     if(diff>=-2) {
		 x1+=diff/2;
		 x2-=diff/2;
	     }
	     r->left=x1;
	     r->width=(x2-x1)+1;
	     break;
	 case scroller__TOP:
	 case scroller__BOTTOM:
	    self->regions[scroller_Elevator]->SetMinWidth(min);
	     y1=self->interiors[side].top;
	     y2=self->interiors[side].top + self->interiors[side].height - 1;
	     diff=self->interiors[side].height - self->elevatorWidth;
	    /* the -2 is so that the elevator can overlap
	     the enclosing box by one pixel */
	     if(diff>=-2) {
		 y1+=diff/2;
		 y2-=diff/2;
	     }
	     r->top=y1;
	     r->height=(y2-y1)+1;
	     break;
    }
    if(latest) self->regions[scroller_Elevator]->ComputeRect(*self, Type[side], self->interiors[side], *r);
    else self->regions[scroller_Elevator]->GetLastRect(*r, Type[side]);
#if 0
    /* I don't understand this section it apparently enforces a minimum elevator size while maintaining some
     other condition as well.  The other condition is related to endbarSpace somehow... -rr2b */
    switch(side) {
	case scroller__LEFT:
	case scroller__RIGHT:
	    y1=r->top;
	    y2=r->top+r->height;
	    if (y2 - y1 < min) {
		y1 = (y1 + y2 - min) / 2;
		if(y1<self->endbarSpace) y1=self->endbarSpace;
		if (y1 + min >= height) {
		    y2 = height  - 1;
		    y1 = y2 - min + 1;
		} else y2 = y1 + min - 1;
	    }
	    break;
	case scroller__TOP:
	case scroller__BOTTOM:
	    x1=r->left;
	    x2=r->left+r->height;
	    if (x2 - x1 < min) {
		x1 = (x1 + x2 - min) / 2;
		if(x1<self->endbarSpace) x1=self->endbarSpace;
		if (x1 + min >= height) {
		    x2 = height  - 1;
		    x1 = x2 - min + 1;
		} else x2 = x1 + min - 1;
	    }
	    break;
    }
    r->left=MIN(x1, x2);
    r->top=MIN(y1, y2);
    r->width=ABS(x1-x2)+1;
    r->height=ABS(y1-y2)+1;
#endif
    return TRUE;
}

static boolean calc_dot(class scroller  *self, int  side, struct rectangle  *r, boolean latest=TRUE)
{
    int diff;
   
    long x1, y1, x2, y2;
    
    if(self->regions[scroller_Dot]==NULL) return FALSE;
      switch(side) {
	case scroller__LEFT:
	case scroller__RIGHT:

	    self->regions[scroller_Dot]->SetMinHeight(MINDOTLENGTH);
	    x1=self->interiors[side].left;
	    x2=self->interiors[side].left + self->interiors[side].width - 1;
	    diff=self->interiors[side].width - self->dotWidth;
	    if(diff>0) {
		x1+=diff/2;
		x2-=diff/2;
	    }
	    r->left=x1;
	    r->width=(x2-x1)+1;
	    break;
	case scroller__TOP:
	case scroller__BOTTOM:
	    self->regions[scroller_Dot]->SetMinWidth(MINDOTLENGTH);
	    y1=self->interiors[side].top;
	    y2=self->interiors[side].top + self->interiors[side].height - 1;
	    diff=self->interiors[side].height - self->dotWidth;
	    if(diff>0) {
		y1+=diff/2;
		y2-=diff/2;
	    }
	    r->top=y1;
	    r->height=(y2-y1)+1;
	    break;
      }
    if(latest) self->regions[scroller_Dot]->ComputeRect(*self, Type[side], self->interiors[side], *r);
    else self->regions[scroller_Dot]->GetLastRect(*r, Type[side]);
    return TRUE;
}

static void draw_elevator(class scroller  *self, int  side)
{
    struct rectangle r;
    
    if(calc_elevator(self, side, &self->desired.bar[Type[side]], &r) && self->regions[scroller_Elevator] && !rectangle_IsEmptyRect(&r)) {
	self->regions[scroller_Elevator]->Draw(*self, Type[side], view_FullRedraw, r);
    }
}

static void draw_dot(class scroller  *self, int  side)
{
    struct rectangle r;
    if(calc_dot(self, side, &r) && self->regions[scroller_Dot] && !rectangle_IsEmptyRect(&r)) {
	self->regions[scroller_Dot]->Draw(*self, Type[side], view_FullRedraw, r);
    }
}

static void move_elevator(class scroller  *self, int  side)
{
    struct rectangle ner, oer, ndr, odr, r1, r2;

    boolean eisvisible;
    boolean disvisible;
    
    boolean redraw_elevator=FALSE;
    boolean redraw_dot=FALSE;
    boolean intersect=FALSE;
    
    double oldfg[3];
    
    eisvisible= calc_elevator(self, side, &self->desired.bar[Type[side]], &ner);
    calc_elevator(self, side, &self->current.bar[Type[side]], &oer, FALSE);
    
    disvisible= calc_dot(self, side, &ndr);
    if(self->regions[scroller_Dot]) calc_dot(self, side, &odr, FALSE);
	  
    (self)->GetFGColor( oldfg, oldfg+1, oldfg+2);
    
    (self)->SetFGColor( self->barbackground[0], self->barbackground[1], self->barbackground[2]);
    
     (self)->SetTransferMode( graphic_SOURCE);
     
    rectangle_IntersectRect(&r1, &odr, &oer);
    rectangle_IntersectRect(&r2, &odr, &ner);
    if(!rectangle_IsEmptyRect(&r1) || !rectangle_IsEmptyRect(&r2)) intersect=TRUE;
    
    if(odr.left!=ndr.left || odr.top!=ndr.top || odr.width!=ndr.width || odr.height!=ndr.height) {
	(self)->FillRect( &odr, NULL);
	redraw_dot=TRUE;
	if(intersect) redraw_elevator=TRUE;	    
    }
    if(oer.left!=ner.left || oer.top!=ner.top || oer.width!=ner.width || oer.height!=ner.height) {
	redraw_elevator=TRUE;
	if(intersect) redraw_dot=TRUE;

	rectangle_IntersectRect(&r1, &oer, &ner);
	if(rectangle_IsEmptyRect(&r1)) (self)->FillRect( &oer, NULL);
	else switch(side) {
	    case scroller__LEFT:
	    case scroller__RIGHT:
		if(oer.top<ner.top) (self)->FillRectSize( oer.left, oer.top, oer.width, ner.top-oer.top+1, NULL);
		if(oer.top+oer.height>ner.top+ner.height) (self)->FillRectSize( oer.left, ner.top+ner.height, oer.width, oer.top+oer.height-ner.top-ner.height, NULL);
		break;
	    case scroller__TOP:
	    case scroller__BOTTOM:
		if(oer.left<ner.left) (self)->FillRectSize( oer.left, oer.top, ner.left-oer.left+1, oer.height, NULL);
		if(oer.left+oer.width>ner.left+ner.width) (self)->FillRectSize( ner.left+ner.width, oer.top, oer.left+oer.width-ner.left-ner.width, oer.height, NULL);
		break;
	}
    }
    (self)->SetFGColor( oldfg[0], oldfg[1], oldfg[2]);
    if(redraw_elevator && eisvisible) draw_elevator(self, side);
    if(redraw_dot && disvisible) draw_dot(self, side);
}


static void draw_bar(class scroller  *self, int  side)
{
    struct sbuttonv_view_info vi;
    barrects(self, side, &self->interiors[side], &self->topbutton[side], &self->botbutton[side]);

    sbuttonv::SaveViewState(self, &vi);
    sbuttonv::DrawBorder(self, self->interiors[side].left, self->interiors[side].top, self->interiors[side].width, self->interiors[side].height, self->boxprefs, sbuttonv_BORDERIN, TRUE, &self->interiors[side]);

    sbuttonv::RestoreViewState(self, &vi);
    draw_elevator(self, side);
    draw_dot(self, side);
    if(self->desired.bar[Type[side]].endzones) {
	draw_arrow(self, side, &self->topbutton[side], 1, self->mousestate==scroller_TOPENDZONE && self->side==side);
	draw_arrow(self, side, &self->botbutton[side], -1, self->mousestate==scroller_BOTTOMENDZONE && self->side==side);
    }
}


static void draw_everything(class scroller  *self)
{
    int i;
    struct rectangle r;
    struct sbuttonv_view_info vi;
    class region *re1, *re2;

    if(self->child) {
	
	(self)->GetLogicalBounds( &r);
	
	re1=region::CreateEmptyRegion();
	re2=region::CreateEmptyRegion();

	if(re1==NULL || re2==NULL) {
	    if(re1!=NULL) delete re1;
	    if(re2!=NULL) delete re2;
	    return;
	}

	(re1)->RectRegion( &r);

	(re2)->RectRegion( &self->childrect);

	(re1)->SubtractRegion( re2, re2);

	(self)->SetClippingRegion( re2);
	
	delete re1;
	delete re2;
    }
    
    sbuttonv::SaveViewState(self, &vi);

    compute_inner(self, TRUE);

    sbuttonv::RestoreViewState(self, &vi);

    for (i = 0; i < scroller_SIDES; i++)
	if (self->desired.location & (1<<i)) {
	    draw_bar(self, i);
	}
}

static void InitPrefs(class scroller  *self)
{
    boolean graphicIsMono;
    boolean mono;

    graphicIsMono = ((self)->GetDrawable())->DisplayClass() & graphic_Monochrome;
    mono = environ::GetProfileSwitch("MimicOldScrollerbar", graphicIsMono ? TRUE : FALSE);

    self->drawborder = environ::GetProfileSwitch("ScrollerDrawBorders", mono ? MONODRAWBORDERS : COLORDRAWBORDERS);
    self->prefs=sbutton::GetNewPrefs(scrollbarstr);
    if(self->prefs==NULL) return;
    if (mono) {
	sbutton::GetStyle(self->prefs) = MONOSTYLE;
    } 
    sbutton::InitPrefs(self->prefs, scrollbarstr);

    self->matteprefs=sbutton::DuplicatePrefs(self->prefs, scrollbarmatte);
    if(self->matteprefs==NULL) return;
    sbutton::InitPrefs(self->matteprefs, scrollbarmatte);

    self->boxprefs=sbutton::DuplicatePrefs(self->prefs, scrollbarbox);
    if(self->boxprefs==NULL) return;
    if (mono) {
	sbutton::GetTop(self->boxprefs) = MONOBARTOP;
	sbutton::GetForeground(self->boxprefs) = MONOBARFOREGROUND;
    }
    sbutton::InitPrefs(self->boxprefs, scrollbarbox);

    self->buttonprefs= sbutton::DuplicatePrefs(self->prefs, scrollbarbutton);
    if(self->buttonprefs==NULL) return;
    if (mono) {
	sbutton::GetStyle(self->buttonprefs) = MONOBUTTONSTYLE;
	sbutton::GetTop(self->buttonprefs) = MONOBUTTONTOP;
	sbutton::GetTopShadow(self->buttonprefs) = MONOBUTTONTOPSHADOW;
	sbutton::GetBottomShadow(self->buttonprefs) = MONOBUTTONBOTTOMSHADOW;
    }
    sbutton::InitPrefs(self->buttonprefs, scrollbarbutton);
    
    if(self->regions[scroller_UpperEnd]) self->regions[scroller_UpperEnd]->SetBaseSButtonPrefs(self->buttonprefs);
    if(self->regions[scroller_BottomEnd]) self->regions[scroller_BottomEnd]->SetBaseSButtonPrefs(self->buttonprefs);
    if(self->regions[scroller_LeftEnd]) self->regions[scroller_LeftEnd]->SetBaseSButtonPrefs(self->buttonprefs);
    if(self->regions[scroller_RightEnd]) self->regions[scroller_RightEnd]->SetBaseSButtonPrefs(self->buttonprefs);
    
    self->elevatorprefs= sbutton::DuplicatePrefs(self->prefs, scrollbarelevator);
    if(self->elevatorprefs==NULL) return;
    sbutton::InitPrefs(self->elevatorprefs, scrollbarelevator);
    if(self->regions[scroller_Elevator]) self->regions[scroller_Elevator]->SetBaseSButtonPrefs(self->elevatorprefs);
    self->dotprefs= sbutton::DuplicatePrefs(self->prefs, scrollbardot);
    if(self->dotprefs==NULL) return;
    if (mono) {
	sbutton::GetStyle(self->dotprefs) = MONODOTSTYLE;
	sbutton::GetTop(self->dotprefs) = MONODOTTOP;
	sbutton::GetTopShadow(self->dotprefs) = MONODOTTOPSHADOW;
	sbutton::GetBottomShadow(self->dotprefs) = MONODOTBOTTOMSHADOW;
    }
    else {
	/* sbutton_GetBottomShadow(self->dotprefs) = COLORDOTBOTTOMSHADOW; */
    }
    sbutton::InitPrefs(self->dotprefs, scrollbardot);
    if(self->regions[scroller_Dot]) self->regions[scroller_Dot]->SetBaseSButtonPrefs(self->dotprefs);

    self->windowPadding = environ::GetProfileInt("ScrollerWindowPadding", mono ? MONOWINDOWPADDING : COLORWINDOWPADDING);

    self->viewPadding = self->windowPadding + 1;

    self->barWidth = environ::GetProfileInt("scrollbarWidth", mono ? MONOBARWIDTH : COLORBARWIDTH);
    self->dotWidth = environ::GetProfileInt("DotWidth", mono ? MONODOTWIDTH : COLORDOTWIDTH);

    self->elevatorWidth = environ::GetProfileInt("ElevatorWidth", mono ? MONOELEVATORWIDTH : COLORELEVATORWIDTH);

    self->endzoneLength = environ::GetProfileInt("ScrollerButtonSize", mono ? MONOENDZONELENGTH : COLORENDZONELENGTH);

    self->buttonSpace = environ::GetProfileInt("ScrollerButtonPadding", mono ? MONOBARTOBUTTONSPACE : COLORBARTOBUTTONSPACE);

    sbuttonv::InteriorBGColor(self, self->boxprefs, TRUE, self->barbackground);
    sbuttonv::InteriorBGColor(self, self->matteprefs, TRUE, self->mattebackground);

    self->prefsready=TRUE;

    if(self->elevatorWidth<=0) self->elevatorWidth=1;
    if(self->dotWidth<=0) self->dotWidth=1;
    if(self->barWidth<4) self->barWidth=4;
   
}

/* Overrides of the view routines: */

void scroller::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
    struct rectangle r;
    int i;

    if((this)->GetIM() && !this->prefsready) {
	    InitPrefs(this);
    }
    (this)->GetLogicalBounds( &r);
    
    this->pending_update = 0;
    
    if((type==view_FullRedraw || type==view_MoveNoRedraw ||
	(type!=view_Remove && (this->lastwidth!=r.width ||
		this->lastheight!=r.height))) && this->child) {
	this->lastwidth=r.width;
	this->lastheight=r.height;
	this->childrect=r;
	compute_inner(this, FALSE);
	(this->child)->InsertView( this, &this->childrect);
	
    }
    
    this->force_full_update = FALSE;
    switch(type) {
	case view_Remove:
	    for (i = 0; i < scroller_SIDES; i++)
	    if ((this->desired.location & (1<<i)) && this->barcursor[Type[i]]!=NULL) {
		(this)->RetractCursor( this->barcursor[Type[i]]);
	    }
	    if(this->child) (this->child)->FullUpdate( type, left, top, width, height);
	    return;
	case view_MoveNoRedraw:
	    if(this->child) (this->child)->FullUpdate( view_MoveNoRedraw, left, top, width, height);
	    break;
	case view_PartialRedraw:
	    if(this->child) (this->child)->FullUpdate( type, (this->child)->EnclosedXToLocalX( left),  (this->child)->EnclosedYToLocalY(top), width, height);
	    break;
	case view_LastPartialRedraw:
	    if(this->child) (this->child)->FullUpdate( type, (this->child)->EnclosedXToLocalX( left),  (this->child)->EnclosedYToLocalY(top), width, height);			      
	    if(this->si) this->si->UpdateRegions(*this);
	    calc_desired(this);
	    draw_everything(this);
	    break;
	case view_FullRedraw:
	    if(this->child) (this->child)->FullUpdate( view_FullRedraw, r.left, r.top, r.width, r.height);			      
	    if(this->si) this->si->UpdateRegions(*this);
	    calc_desired(this);
	    draw_everything(this);
	    break;
	default: ;
    }
    if(type!=view_Remove && type!=view_PartialRedraw) {
	for (i = 0; i < scroller_SIDES; i++)
	    if ((this->desired.location & (1<<i)) && this->barcursor[Type[i]]!=NULL) {
		(this)->RetractCursor( this->barcursor[Type[i]]);
		(this)->PostCursor( &this->interiors[i], this->barcursor[Type[i]]);
	    }
    }

    this->current = this->desired;
    (this)->FlushGraphics();
}


void scroller::Update()
{
    int i;
    struct rectangle r;

    (this)->GetLogicalBounds( &r);

    if (r.width <= 0 || r.height <= 0) return;
    this->pending_update = 0;
    /* Let the children modify their state however they want. */
    (this->updatelist)->Clear();

    if(this->si) this->si->UpdateRegions(*this);
    
    calc_desired(this);

    if (this->current.location != this->desired.location || this->force_full_update) {
	(this)->EraseVisualRect();
	(this)->FullUpdate( view_FullRedraw, r.left, r.top, r.width, r.height);
    } else {
	for (i = 0; i < scroller_SIDES; i++)
	    if (this->desired.location & (1<<i)) {
		int type = Type[i];
		struct scrollerbar *des = &this->desired.bar[type], *cur = &this->current.bar[type];
		if (des->endzones != cur->endzones)
		    /* The entire scrollerbar need redrawing? */
		    draw_bar(this, i);
		else {
		    if((this->regions[scroller_Elevator] && this->regions[scroller_Elevator]->changed) || (this->regions[scroller_Dot && this->regions[scroller_Dot]->changed])) move_elevator(this, i);
		}
	    }
	this->current = this->desired;
    }
}

void scroller::WantUpdate(class view  *requestor)
{
    if ((class view *)this != requestor)
        (this->updatelist)->AddTo( requestor);

    if (!this->pending_update) {
        this->pending_update = 1;
        (this)->view::WantUpdate( this);
    }
}

static void DoRepeatScroller(class scroller  *self)
{
    if(self->si==NULL) return;
    if(self->emulation) {
	
	if(self->regions[scroller_Elevator]==NULL) return;
	switch(self->dir) {
	    case scroller_UP:
		if(Type[self->side]==scroller_VERT) self->si->ScreenDelta(0, CHILDBARHEIGHT(self, self->side)-1);
		else self->si->ScreenDelta(CHILDBARHEIGHT(self, self->side)-1, 0);
		break;
	    case scroller_DOWN:
		if(Type[self->side]==scroller_VERT) self->si->ScreenDelta(0, -CHILDBARHEIGHT(self, self->side)+1);
		else self->si->ScreenDelta(-(CHILDBARHEIGHT(self, self->side)-1), 0);
	}	
    } else {
	if (self->lastaction==view_LeftDown) {
	    if(Type[self->side]==scroller_VERT) self->si->ScreenDelta(0, -self->hitcoord);
	    else self->si->ScreenDelta(-self->hitcoord, 0);
	} else if (self->lastaction==view_RightDown) {
	    if(Type[self->side]==scroller_VERT) self->si->ScreenDelta(0, self->hitcoord);
	    else self->si->ScreenDelta(self->hitcoord, 0);
	}
    }
}

static void RepeatScroller(class scroller  *self, long  cTime)
{
    long timeInterval;

    self->scrollerEvent = NULL;
    if(self->side == -1 || (self->mousestate!=scroller_MAYBETHUMBING && self->mousestate!=scroller_REPEATING)) return;

    self->mousestate=scroller_REPEATING;
    DoRepeatScroller(self);
    timeInterval = self->minContScrollerTime + (((self->maxContScrollerTime - self->minContScrollerTime) * self->hitcoord) / (PIXELSPERINCH(self) * 10));

    self->scrollerEvent = im::EnqueueEvent((event_fptr) RepeatScroller, (char *) self, event_MSECtoTU(timeInterval));
}

#define PTINRECT(r, x, y) ((x)>=(r)->left && (x)<(r)->left+(r)->width && (y)>=(r)->top && (y)<(r)->top+(r)->height)
#define ENDZONEREPTIME(self) (self->endzonereptime)


static void endzone(class scroller  *self, int  side, int  end, enum view_MouseAction  action)
{
    int type = Type[side];
    int typedEnd;
    if(action!=view_LeftDown && action!=view_RightDown) return;
    
    if(self->si==NULL) return;
    
    if(type==scroller_VERT) {
	if(action==view_LeftDown && !self->emulation) {
	    if(end==scroller_TOPENDZONE) self->si->Extreme(scroll_Up);
	    else self->si->Extreme(scroll_Down);
	} else {
	    if(end==scroller_TOPENDZONE) self->si->Shift(scroll_Up);
	    else self->si->Shift(scroll_Down);
	}
    } else {
	if(action==view_LeftDown && !self->emulation) {
	    if(end==scroller_TOPENDZONE) self->si->Extreme(scroll_Left);
	    else self->si->Extreme(scroll_Right);
	} else {
	    if(end==scroller_TOPENDZONE) self->si->Shift(scroll_Left);
	    else self->si->Shift(scroll_Right);
	}
    }
}

static void RepeatEndZone(class scroller  *self, long  cTime)
{
    self->scrollerEvent=NULL;
    if(self->mousestate!=scroller_TOPENDZONE && self->mousestate!=scroller_BOTTOMENDZONE) return;
    
    endzone(self, self->side, self->mousestate, self->lastaction);
    ScheduleRepeatEndZone(self);
}

static void CancelScrollerEvent(class scroller  *self)
{
    if(self->scrollerEvent) {
	(self->scrollerEvent)->Cancel();
	self->scrollerEvent=NULL;
    }
}

static void ScheduleRepeatEndZone(class scroller  *self)
{
    CancelScrollerEvent(self);
    self->scrollerEvent = im::EnqueueEvent((event_fptr) RepeatEndZone, (char *) self, event_MSECtoTU(ENDZONEREPTIME(self)));
}

static void HandleEndZone(class scroller  *self, enum view_MouseAction  action, long  x , long  y)
{

    struct rectangle *r;
    int dir;
    
    if(self->side==-1) return;
    
    /* if we're here self->mousestate==scroller_TOPENDZONE or self->mousestate==scroller_BOTTOMENDZONE and self->side is a valid side */
    
    if(self->mousestate==scroller_TOPENDZONE) {
	r = &self->topbutton[self->side];
	dir=1;
    } else {
	r = &self->botbutton[self->side];
	dir=(-1);
    }

    switch(action) {
	case view_LeftMovement:
	case view_RightMovement:
	    if(PTINRECT(r, x, y)) {
		ScheduleRepeatEndZone(self);
		if(!self->lastbuttonstate) draw_arrow(self, self->side, r, dir, self->lastbuttonstate=TRUE);

	    } else {
		CancelScrollerEvent(self);
		if(self->lastbuttonstate) draw_arrow(self, self->side, r, dir, self->lastbuttonstate=FALSE);
	    }
	    break;
	case view_LeftUp:
	case view_RightUp:
	    /* the code in scroller_Hit will handle canceling the event */
	    if(self->lastbuttonstate) draw_arrow(self, self->side, r, dir, self->lastbuttonstate=FALSE);
    }
    endzone(self, self->side, self->mousestate, action);
}

static void HandleRepeatMode(class scroller  *self, enum view_MouseAction  action, long  x , long  y)
{
    long coord;
    
    if(self->side==-1) return;

    
    coord=XYTOHITCOORD(self, self->side, x, y);
    if(self->adjustScroller && self->mousestate!=scroller_MAYBETHUMBING) self->hitcoord=coord;
    
    if(action!=view_LeftUp && action!=view_RightUp) return;

    DoRepeatScroller(self);
    
    im::ForceUpdate();
}

static boolean CheckEndZones(class scroller  *self, enum view_MouseAction  action, long  x , long  y)
{
    int i;

    /* if we're here self->mousestate==scroller_NOTHING */
    
    if(action!=view_LeftDown && action!=view_RightDown) return FALSE;

    for(i=0;i<scroller_SIDES;i++)
	if((self->desired.location & (1<<i))) {
	    int dir;
	    struct rectangle *r;
	    if(PTINRECT(&self->topbutton[i], x, y)) {
		r = &self->topbutton[i];
		dir=1;
		self->mousestate=scroller_TOPENDZONE;
	    } else if(PTINRECT(&self->botbutton[i], x, y)) {
		r = &self->botbutton[i];
		dir=(-1);
		self->mousestate=scroller_BOTTOMENDZONE;
	    }
	    /* else the mouse state will be scroller_NOTHING */

	    self->lastaction=view_NoMouseEvent;

	    if(self->mousestate!=scroller_NOTHING) {
		self->side=i;
		self->lastbuttonstate=TRUE;
		self->lastaction=action;

		draw_arrow(self, i, r, dir, self->lastbuttonstate);
		ScheduleRepeatEndZone(self);
		endzone(self, i, self->mousestate, action);
		return TRUE;
	    }
	}
    return FALSE;
}

static void HandleThumbing(class scroller  *self, enum view_MouseAction  action, long  x , long  y)
{
    /* Assure that we have the correct button */
    if (((action == view_LeftMovement || action == view_LeftUp) && (self->lastaction != view_LeftDown)) || ((action == view_RightMovement || action == view_RightUp) && (self->lastaction != view_RightDown)) || self->side==(-1))
	return;
    
    struct scrollerbar *cur, *des;
    long coord=0;
    int i;
    long posn;
    long logicalTop, logicalHeight, logicalPos;
    long stype=self->side>0?Type[self->side]:scroller_VERT;
    
    long top=(Type[self->side]==scroller_VERT)?self->regions[scroller_Elevator]->rect[stype].top:self->regions[scroller_Elevator]->rect[stype].left;
    long bot=(Type[self->side]==scroller_VERT)?self->regions[scroller_Elevator]->rect[stype].top+self->regions[scroller_Elevator]->rect[stype].height:self->regions[scroller_Elevator]->rect[stype].left+self->regions[scroller_Elevator]->rect[stype].width;
    long dbeg=top;
    long dend=bot;
    long lheight=(Type[self->side]==scroller_VERT)?self->interiors[self->side].height:self->interiors[self->side].width;

    cur = &self->current.bar[Type[self->side]];
    des = &self->desired.bar[Type[self->side]];

    if (self->side == scroller__LEFT || self->side == scroller__RIGHT) {
	logicalTop = self->interiors[self->side].top;
	logicalHeight = self->interiors[self->side].height;
	logicalPos = y;
    }
    else {
	logicalTop = self->interiors[self->side].left;
	logicalHeight = self->interiors[self->side].width;
	logicalPos = x;
    }
    coord = logicalPos - logicalTop;
    if(coord > logicalHeight) {
	coord=logicalHeight-self->endbarSpace;
	logicalPos=logicalTop+coord;
    }
    
    if (action == view_LeftMovement || action == view_RightMovement) {
	posn = coord;
	if(ABS(posn-top+self->seenLength/2)>=self->seenLength/10) {
	    int location=self->current.location;

	    dbeg=posn-self->seenLength/2;
	    if (dbeg<0) {
		dbeg=0;
		dend = MIN(lheight, dbeg + self->seenLength);
	    }
	    else if (dbeg > lheight) {
		dbeg= lheight;
		dend = dbeg;
	    }
	    else
		dend = MIN(lheight, dbeg + self->seenLength);
	    for (i = 0; i < scroller_SIDES; i++)  {
		if ((location & (1<<i)) && Type[i] == Type[self->side])  {
		    if (self->si && self->thumbScroller && (self->lastaction==view_LeftDown || (self->emulation && self->lastaction==view_RightDown)))  {
			if(Type[self->side]==scroller_VERT) self->si->Absolute(0,0,lheight,dbeg);
			else self->si->Absolute(lheight, dbeg, 0, 0);
		    } else move_elevator(self, i);
		}
	    }
	    if (self->thumbScroller && (self->lastaction==view_LeftDown || (self->emulation && self->lastaction==view_RightDown))) {
		im::ForceUpdate();
	    } else *cur = *des;
	}
    } else { /* this is an up transition */
	(self->imPtr)->SetWindowCursor( NULL);

	dbeg=coord-self->seenLength/2;
	
	if (dbeg < 0)
	    dbeg = 0;
	else if (dbeg > lheight) {
	    dbeg = lheight;
	}
	if(self->si) if(Type[self->side]==scroller_VERT) self->si->Absolute(0,0,lheight,dbeg);
	else self->si->Absolute(lheight,dbeg, 0, 0);
	im::ForceUpdate();
    }
}

static void CheckBars(class scroller  *self, enum view_MouseAction  action, long  x , long  y)
{
    int i;
    struct scrollerbar *bar=NULL;
    struct rectangle r;
    
    if(action!=view_LeftDown && action!=view_RightDown) return;
    
    for(i=0;i<scroller_SIDES;i++) if(PTINRECT(&self->interiors[i], x, y)) break;

    if(i>=scroller_SIDES) return;
    
    bar= &self->current.bar[Type[self->side=i]];
    
    self->hitcoord=XYTOHITCOORD(self, i, x, y);

    self->mousestate=scroller_REPEATING;
    if (calc_elevator(self, i, bar, &r)) {
	switch(i) {
	    case scroller__LEFT:
	    case scroller__RIGHT:
		r.left=self->interiors[i].left;
		r.width=self->interiors[i].width;
		if(self->hitcoord<r.top) self->dir=scroller_UP;
		else if(self->hitcoord>r.top+r.height) self->dir=scroller_DOWN;
		else self->dir=scroller_NOWHERE;
		if(PTINRECT(&r, x, y)) {
		    self->mousestate=scroller_MAYBETHUMBING;
		    self->seenLength = r.height;
		}
		break;
	    case scroller__TOP:
	    case scroller__BOTTOM:
		r.top=self->interiors[i].top;
		r.height=self->interiors[i].height;
		if(self->hitcoord<r.left) self->dir=scroller_UP;
		else if (self->hitcoord>r.left+r.width) self->dir=scroller_DOWN;
		else self->dir=scroller_NOWHERE;
		if(PTINRECT(&r, x, y)) {
		    self->mousestate=scroller_MAYBETHUMBING;
		    self->seenLength = r.width;
		}
	}
    }
    self->side = i;
    self->lastaction=action;
    if(self->emulation && action==view_RightDown) {
	if(Type[i]==scroller_VERT) self->seenLength = r.height;
	else self->seenLength=r.width;
	self->mousestate=scroller_THUMBING;
	if(self->cursor!=NULL) ((self)->GetIM())->SetWindowCursor( self->cursor);
	HandleThumbing(self, view_RightMovement, x, y);
    } else if (self->startScrollerTime > 0)  {
	self->scrollerEvent = im::EnqueueEvent((event_fptr)RepeatScroller, (char *) self, event_MSECtoTU(self->startScrollerTime));
    }
}

static void MaybeStartThumbing(class scroller  *self, enum view_MouseAction  action, long  x , long  y)
{
    if(self->side==-1) return;

    if(ABS(XYTOHITCOORD(self, self->side, x, y) - self->hitcoord) <= SMALLDIST) {
	HandleRepeatMode(self, action, x, y);
	return;
    }
    
    if(action==view_LeftDown || action==view_RightDown) return;
    
    self->mousestate=scroller_THUMBING;
    CancelScrollerEvent(self);

    if(self->cursor!=NULL) ((self)->GetIM())->SetWindowCursor( self->cursor);
    
    HandleThumbing(self, action, x, y);
}


class view *scroller::Hit(enum view_MouseAction  action, long  x , long  y , long  num_clicks)
{
    switch(this->mousestate) {
	case scroller_THUMBING:
	    HandleThumbing(this, action, x, y);
	    break;
	case scroller_MAYBETHUMBING:
	    MaybeStartThumbing(this, action, x, y);
	    break;
	case scroller_REPEATING:
	    HandleRepeatMode(this, action, x, y);
	    break;
	case scroller_TOPENDZONE:
	case scroller_BOTTOMENDZONE:
	    HandleEndZone(this, action, x, y);
	    break;
	case scroller_NOTHING:
	    if(action!=view_LeftDown && action!=view_RightDown &&
	       action!=view_LeftFileDrop && action!=view_MiddleFileDrop &&
	       action!=view_RightFileDrop) return (class view *)this;
	    if(this->child && PTINRECT(&this->childrect, x, y)) return (this->child)->Hit( action, (this->child)->EnclosedXToLocalX( x), (this->child)->EnclosedYToLocalY( y), num_clicks);

	    if(CheckEndZones(this, action, x, y)) return (class view *)this;

	    CheckBars(this, action, x, y);
	default: ;
    }
    
    /* if a button has gone up cancel any scheduled scrollering and set the state back to normal (ie anything can happen). */
    if(action==view_RightUp || action==view_LeftUp) {
	CancelScrollerEvent(this);
	this->mousestate=scroller_NOTHING;
	this->side=(-1);
    }

    return (class view *)this;
}

void scroller::LinkTree(class view  *parent)
{
    (this)->view::LinkTree( parent);
    if (this->child)
	(this->child)->LinkTree( this);
}

void scroller::UnlinkNotification(class view  *unlinkedTree)
{
    (this)->view::UnlinkNotification( unlinkedTree);
    (this->updatelist)->DeleteTree( unlinkedTree);
}

boolean scroller::RecSearch(struct SearchPattern *pat, boolean toplevel)
{
    if (this->scrolleree)
	return this->scrolleree->RecSearch(pat, toplevel);
    return FALSE;
}

boolean scroller::RecSrchResume(struct SearchPattern *pat)
{
    if (this->scrolleree)
	return this->scrolleree->RecSrchResume(pat);
    return FALSE;
}

boolean scroller::RecSrchReplace(class dataobject *text, long pos, long len)
{
    if (this->scrolleree)
	return this->scrolleree->RecSrchReplace(text, pos, len);
    return FALSE;
}

boolean scroller::RecSrchExpose()
{
    if (this->scrolleree)
	return this->scrolleree->RecSrchExpose();
    return FALSE;
}

void scroller::PrintPSDoc(FILE *outfile, long pagew, long pageh)
{
    if (this->scrolleree)
	this->scrolleree->PrintPSDoc(outfile, pagew, pageh);
}
void *scroller::GetPSPrintInterface(const char *printtype)
{
    if (this->scrolleree)
	return this->scrolleree->GetPSPrintInterface(printtype);
    return NULL;
}
void scroller::DesiredPrintSize(long width, long height, enum view_DSpass pass, long *desiredwidth, long *desiredheight)
{
    if (this->scrolleree)
	this->scrolleree->DesiredPrintSize(width, height, pass, desiredwidth, desiredheight);
}
void scroller::PrintPSRect(FILE *outfile, long logwidth, long logheight, struct rectangle *visrect)
{
    if (this->scrolleree)
	this->scrolleree->PrintPSRect(outfile, logwidth, logheight, visrect);
}
