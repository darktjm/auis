/************************************************************************ *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* Scrollbar code for ATK based on the original scrollbar code for be2. */

#include <andrewos.h>
ATK_IMPL("scroll.H")
#include <scroll.H>

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


/* The descriptions of the different types of scrollbars */
static const int Type[scroll_SIDES] = {scroll_VERT, scroll_VERT, scroll_HORIZ, scroll_HORIZ};    

static const int cursortypes[scroll_TYPES]={Cursor_VerticalArrows, Cursor_HorizontalArrows};

static const char * const InterfaceName[scroll_TYPES] = {"scroll,vertical", "scroll,horizontal"};

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

#define XYTOHITCOORD(self, side, x, y) ((((side)==scroll__LEFT || (side)==scroll__RIGHT)?(y)-(self)->childrect.top:(x)-(self)->childrect.left))

#define CHILDBARHEIGHT(self, side) ((Type[side]==scroll_VERT)?(self)->childrect.height:(self)->childrect.width)

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
    TRUE, /* should be drawn as if it is sensitive. */
};

#define DRAWRECT(self, left, top, width, height) (self)->DrawRectSize( left, top, (width)-1, (height)-1)
#define FILLRECT(self, left, top, width, height) (self)->FillRectSize( left, top, (width), (height), NULL)


ATKdefineRegistry(scroll, view, scroll::InitializeClass);
static void draw_arrow(class scroll  *self, int  side, struct rectangle  *r, int  dir, boolean  lit);
static long bar_height(class scroll  *self, int  side);
static void endzone(class scroll  *self, int  side, int  end, enum view_MouseAction  action);
/* static void calc_desired(class scroll  *self); */
static void draw_elevator(class scroll  *self, int  side);
static void draw_dot(class scroll  *self, int  side);
static void move_elevator(class scroll  *self, int  side);
static void draw_bar(class scroll  *self, int  side);
static void draw_everything(class scroll  *self);
static void InitPrefs(class scroll  *self);
static void DoRepeatScroll(class scroll  *self);
static void RepeatScroll(class scroll  *self, long  cTime);
static void RepeatEndZone(class scroll  *self, long  cTime);
static void CancelScrollEvent(class scroll  *self);
static void ScheduleRepeatEndZone(class scroll  *self);
static void HandleEndZone(class scroll  *self, enum view_MouseAction  action, long  x , long  y);
static void HandleRepeatMode(class scroll  *self, enum view_MouseAction  action, long  x , long  y);
static boolean CheckEndZones(class scroll  *self, enum view_MouseAction  action, long  x , long  y);
static void HandleThumbing(class scroll  *self, enum view_MouseAction  action, long  x , long  y);
static void CheckBars(class scroll  *self, enum view_MouseAction  action, long  x , long  y);
static void MaybeStartThumbing(class scroll  *self, enum view_MouseAction  action, long  x , long  y);


static void draw_arrow(class scroll  *self, int  side, struct rectangle  *r, int  dir, boolean  lit)
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
                case scroll__LEFT:
                case scroll__RIGHT:
                    (self)->MoveTo( offx+r->left+arrowpts[0].x*r->width/100, offy+r->top+(dir<0?r->height:0)+dir*arrowpts[0].y*r->height/100);
                    (self)->DrawLineTo( offx+r->left+arrowpts[1].x*r->width/100, offy+r->top+(dir<0?r->height:0)+dir*arrowpts[1].y*r->height/100);
                    (self)->DrawLineTo( offx+r->left+arrowpts[2].x*r->width/100, offy+r->top+(dir<0?r->height:0)+dir*arrowpts[2].y*r->height/100);
                    break;
                case scroll__TOP:
                case scroll__BOTTOM:
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
	if(Type[side]==scroll_VERT) {
	    if(dir>0) s=self->regions[scroll_BottomEnd];
	    else s=self->regions[scroll_UpperEnd];
	} else {
	    if(dir>0) s=self->regions[scroll_RightEnd];
	    else s=self->regions[scroll_LeftEnd];
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
            case scroll__LEFT:
            case scroll__RIGHT:
                t.height = self->buttonSpace;
                if (dir == 1) {
                    t.top += r->height;
                }
                else {
                    t.top -= self->buttonSpace;
                }
                break;
            case scroll__TOP:
            case scroll__BOTTOM:
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

boolean scroll::InitializeClass()
{
 
    return TRUE;
}

void scroll::SetRegion(enum scroll_Regions sr, class ScrollRegion *r) {
    regions[sr]=r;
    r->SetBaseSButtonPrefs(elevatorprefs);
}
    
static const char scrollbarstr[]="scroll";
static const char scrollbarmatte[]="scrollmatte";
static const char scrollbarbox[]="scrollbar";
static const char scrollbarbutton[]="scrollbutton";
static const char scrollbarelevator[]="scrollelevator";
static const char scrollbardot[]="scrolldot";

scroll::scroll()
{
	ATKinit;

    int i;

    
    this->prefsready = FALSE;

    this->emulation = environ::GetProfileSwitch("MotifScrollbars", FALSE);
    this->startScrollTime = environ::GetProfileInt("StartScrollTime", 0);
    this->minContScrollTime = environ::GetProfileInt("ContScrollTime", 500);
    this->maxContScrollTime = environ::GetProfileInt("MaxContScrollTime", this->minContScrollTime);
    this->endzonereptime = environ::GetProfileInt("ButtonRepeatTime", 100);
    this->adjustScroll = environ::GetProfileSwitch("AdjustScroll", FALSE);
    this->thumbScroll = environ::GetProfileSwitch("ThumbScroll", FALSE);

    this->side=(-1);
    this->lastwidth=(-1);
    this->lastheight=(-1);
    this->dir=scroll_NOWHERE;
    
    this->current.location = 0;
    this->child = this->scrollee = NULL;
    for (i = 0; i < scroll_TYPES; i++) {
	this->barcursor[i]=cursor::Create((class view *)this);
	if(this->barcursor[i]!=NULL) (this->barcursor[i])->SetStandard( cursortypes[i]);
    }
    for(i=0;i<scroll_SIDES;i++) rectangle_EmptyRect(&interiors[i]);

    this->desired = this->current;

    this->pending_update = 0;
    
    this->updatelistp = new updatelist;
    if(this->updatelistp==NULL) THROWONFAILURE( FALSE);
    
    this->ideal_location = scroll_LEFT;
    
    this->endzone_threshold = 80;
    this->bar_threshold = 0;
    this->endbarSpace = ENDTOBARSPACE;

    this->min_elevator[0] = 5;
    this->min_elevator[1] = 18;
    this->force_full_update = FALSE;
    this->scrollEvent = NULL;

    this->barbackground[0] = this->barbackground[1] = this->barbackground[2] = -1.0;

    this->cursor = cursor::Create((class view *)this);

    if(this->cursor!=NULL) (this->cursor)->SetStandard( Cursor_Octagon);

    this->mousestate=scroll_NOTHING;
    this->prefs = this->boxprefs = this->matteprefs = this->buttonprefs = this->elevatorprefs = this->dotprefs = NULL;
    this->si=NULL;
    for(i=0;i<NUM_SCROLLREGIONS;i++) {
	this->regions[i]=NULL;
    }
    customsi=FALSE;
    custom=FALSE;
    THROWONFAILURE( TRUE);
}
    
#define sfree(x)  do { if(x) sbutton::FreePrefs(x); x=NULL; } while (0)

scroll::~scroll()
{
	ATKinit;

    int i;
    if (this->child != NULL)
        (this->child)->UnlinkTree();

    if(this->updatelistp) delete this->updatelistp;
    
    if(this->cursor) delete this->cursor;

    for(i=0;i<scroll_TYPES;i++) if(this->barcursor[i]!=NULL) delete this->barcursor[i];
    
    sfree(this->prefs);
    sfree(this->boxprefs);
    sfree(this->matteprefs);
    sfree(this->buttonprefs);
    sfree(this->elevatorprefs);
    sfree(this->dotprefs);

    if(!customsi) {
	delete si;
    }
    
    if(!custom) {
	delete regions[scroll_Elevator];
	delete regions[scroll_Dot];
	delete regions[scroll_UpperEnd];
	delete regions[scroll_LeftEnd];
	delete regions[scroll_BottomEnd];
	delete regions[scroll_RightEnd];
    }
}


static class scroll *CreateBasicScroller(class view  *scrollee, int  location, const char  *name)

{

    struct ATKregistryEntry  *oscroll;
    
    class scroll *retval=NULL;

    if(name==NULL) name=environ::GetProfile("ScrollClass");
    
    if(name!=NULL && (oscroll=ATK::LoadClass(name))!=NULL && (oscroll)->IsType( &scroll_ATKregistry_ )) retval= (class scroll *)ATK::NewObject(name);
    
    if(retval==NULL) {
	retval=new scroll;
	if(retval==NULL) return NULL;
    }

    (retval)->SetView( scrollee);
    (retval)->SetLocation( location);

    return retval;
}
#define scroll_LEFT (1<<scroll__LEFT)
#define scroll_RIGHT (1<<scroll__RIGHT)
#define scroll_TOP (1<<scroll__TOP)
#define scroll_BOTTOM (1<<scroll__BOTTOM)

void scroll::SetStandardRegions(int location) {
    SetRegion(scroll_Elevator, new ScrollRegionElevator);
    SetRegion(scroll_Dot, new ScrollRegionDot);
    if(location&(scroll_LEFT|scroll_RIGHT)) {
	SetRegion(scroll_UpperEnd, new ScrollRegionEndzone);
	SetRegion(scroll_BottomEnd, new ScrollRegionEndzone);
    }
    if(location&(scroll_TOP|scroll_BOTTOM)) {
	SetRegion(scroll_LeftEnd, new ScrollRegionEndzone);
	SetRegion(scroll_RightEnd, new ScrollRegionEndzone);
    }
}

class scroll *scroll::CreateScroller(class view  *scrollee, int  location, const char  *name)

{
    ATKinit;

    class scroll *s=CreateBasicScroller(scrollee,location, name);
    if(s) {
	s->SetStandardRegions(location);
	if(s->si==NULL) s->si=new ScrollInterfaceClassic(scrollee);
    }
    return s;
}

class scroll *scroll::CreateScroller(class ScrollInterface *is, class view *scrollee, int location, const char *name) {
    ATKinit;
    class scroll *s=CreateBasicScroller(scrollee, location, name);
    if(s) {
	s->SetStandardRegions(location);
	s->SetInterface(is);
    }
    return s;
}

class scroll *scroll::CreateCustomScroller(class view *scrollee, int location, const char *name) {
    ATKinit;
    class scroll *s=CreateBasicScroller(scrollee, location, name);
    if(s) s->custom=TRUE;
    return s;
}


class scroll *scroll::Create(class view  *scrollee, int  location)
{
	ATKinit;

    return scroll::CreateScroller(scrollee, location, NULL);
}


/* State modification routines. */

void scroll::SetView(class view  *view)
{

    (this)->SetChild( view);
    (this)->SetScrollee( view);
}

void scroll::SetChild(class view  *child)
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

void scroll::SetScrollee(class view  *scrollee)
{
    if (this->scrollee != scrollee && !customsi) {
	this->scrollee = scrollee;
	if(si) delete si;
	if(scrollee && !(si=scrollee->GetScrollInterface())) {
	    ScrollInterfaceClassic *sic=new ScrollInterfaceClassic;
	    si=sic;
	    if(sic) sic->SetScrollee(scrollee);
	}
        (this)->WantUpdate( this);
    }
}

class view *scroll::GetChild()
{
    return this->child;
}

class view *scroll::GetScrollee()
{
    return this->scrollee;
}

void scroll::SetLocation(int  location)
{
    this->ideal_location = location;
    (this)->WantUpdate( this);  
}

int scroll::GetLocation()
{
    return this->ideal_location;
}

int scroll::GetCurrentLocation()
{
    return this->current.location;
}

void scroll::SetParameters(long  endzone , long  bar, int  without , int  with)
{
    this->endzone_threshold = endzone;
    this->bar_threshold = bar;
    this->min_elevator[0] = without;
    this->min_elevator[1] = with;
    (this)->WantUpdate( this);
}

void scroll::GetParameters(long  *endzone , long  *bar, int  *without , int  *with)
{
    *endzone = this->endzone_threshold;
    *bar = this->bar_threshold;
    *without = this->min_elevator[0];
    *with = this->min_elevator[1];
}

void scroll::SetWidth(long  newWidth)
{
    this->barWidth = newWidth;
    
    if (this->dotWidth > newWidth - 2) {
	(this)->SetDotWidth( newWidth - 2);
    }
    (this)->WantUpdate( this);
}

long scroll::GetWidth()
{
    return this->barWidth;
}

void scroll::SetDotWidth(long  newWidth)
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

long scroll::GetDotWidth()
{
    return this->dotWidth;
}


void scroll::SetElevatorWidth(long  newWidth)
{
    if(newWidth > this->barWidth) newWidth=this->barWidth;

    if(newWidth<0) newWidth=0;

    this->elevatorWidth=newWidth;
    (this)->WantUpdate( this);
}

long scroll::GetElevatorWidth()
{
    return this->elevatorWidth;
}

void scroll::SetWindowPadding(long  newPadding)
{
    this->windowPadding=newPadding;
}

void scroll::SetViewPadding(long  newPadding)
{
    this->viewPadding=newPadding;
}

long scroll::GetWindowPadding()
{
    return this->windowPadding;
}

long scroll::GetViewPadding()
{
    return this->viewPadding;
}

void scroll::SetEndZoneLength(long  newLength)
{
    this->endzoneLength = newLength;
    if ((this->endzoneLength + this->endbarSpace + this->buttonSpace) * 2 > this->endzone_threshold) {
	this->endzone_threshold = (this->endzoneLength + this->endbarSpace + this->buttonSpace) * 2;
    }
    (this)->WantUpdate( this);
}

long scroll::GetEndZoneLength()
{
    return this->endzoneLength;
}

void scroll::SetEndToBarSpace(long  newSpace)
{
    this->endbarSpace = newSpace;
    if ((this->endzoneLength + this->endbarSpace + this->buttonSpace) * 2 > this->endzone_threshold) {
	this->endzone_threshold = (this->endzoneLength + this->endbarSpace + this->buttonSpace) * 2;
    }
    (this)->WantUpdate( this);
}


long scroll::GetEndToBarSpace()
{
    return this->endbarSpace;
}


/* Calculation routines. */

static long bar_height(class scroll  *self, int  side)
{
    switch(side) {
	case scroll__LEFT:
	case scroll__RIGHT:
	    return self->interiors[side].height;
	case scroll__TOP:
	case scroll__BOTTOM:
	    return self->interiors[side].width;
    }
    return 0;
}


static boolean barrects(class scroll  *self, int  side, struct rectangle  *boxrect , struct rectangle  *topbuttonrect , struct rectangle  *botbuttonrect, const struct rectangle &childrect, const struct rectangle &fullinterior, struct scrollstate &desired)
{
    long x1 = 0, x2 = 0, y1 = 0, y2 = 0;
    long tx1 = 0, tx2 = 0, ty1 = 0, ty2 = 0;
    long bx1 = 0, bx2 = 0, by1 = 0, by2 = 0;
    long w, h;
    int c=2;
    long tezl=self->endzoneLength + self->buttonSpace;
    boolean result=TRUE;
    long height = 0;
    
    while(c--) {
	switch(side) {
	    case scroll__LEFT:
	    case scroll__RIGHT:
		y1=childrect.top + tezl;
		y2=childrect.top + childrect.height - 1 - tezl;
		height= (y2 - y1 + 1) + 2 * tezl;
		if(height < self->bar_threshold) result=FALSE;
		
		ty1=childrect.top;
		ty2=ty1 + self->endzoneLength - 1;
		by2=childrect.top + childrect.height - 1;
		by1=by2 - self->endzoneLength + 1;
		break;
	    case scroll__TOP:
	    case scroll__BOTTOM:
		x1= childrect.left + tezl;
		x2= childrect.left + childrect.width - 1 - tezl;
		height= (x2 - x1 + 1) + 2 * tezl;
		if(height < self->bar_threshold) result=FALSE;
		tx1=childrect.left;
		tx2=tx1 + self->endzoneLength -1;
		bx2=childrect.left + childrect.width - 1;
		bx1=bx2 - self->endzoneLength + 1;
		break;
	}
	if(self->child) {
	    switch(side) {
		case scroll__LEFT:
		    x1=bx1=tx1=fullinterior.left;
		    x2=bx2=tx2=tx1 + REALBARWIDTH(self) - 1;
		    break;
		case scroll__RIGHT:
		    x2=bx2=tx2=fullinterior.left + fullinterior.width - 1;
		    x1=bx1=tx1=tx2 - REALBARWIDTH(self) + 1;
		    break;
		case scroll__TOP:
		    y1=by1=ty1=fullinterior.top;
		    y2=by2=ty2=ty1 + REALBARWIDTH(self) - 1;
		    break;
		case scroll__BOTTOM:
		    y2=by2=ty2=fullinterior.top + fullinterior.height - 1;
		    y1=by1=ty1=ty2 - REALBARWIDTH(self) + 1;
	    }
	} else {
	    switch(side) {
		case scroll__LEFT:
		case scroll__RIGHT:
		    {
		    int mid=fullinterior.left + fullinterior.width / 2;
		    x2=bx2=tx2=mid + REALBARWIDTH(self) / 2;
		    x1=bx1=tx1= x2 - REALBARWIDTH(self)  + (REALBARWIDTH(self)&1);
		    }
		    break;
		case scroll__TOP:
		case scroll__BOTTOM:
		    {
		    int mid=fullinterior.top + fullinterior.height / 2;
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
	    desired.bar[Type[side]].endzones=TRUE;
	    return result;
	} else {
	    desired.bar[Type[side]].endzones=FALSE;
	    if(topbuttonrect) topbuttonrect->width=0;
	    if(botbuttonrect) botbuttonrect->width=0;
	    tezl=self->buttonSpace;
	}
    }
    return result;
}


static void compute_inner(class scroll  *self, boolean  draw, const struct rectangle &r, struct rectangle &childrect, struct rectangle &fullinterior, struct scrollstate &desired)
{
    int i;
    int diff;
    int lastlocation;

    desired.location=self->ideal_location;
    diff=REALBARWIDTH(self) + VPADDING(self);
    do {
        lastlocation=desired.location;

        childrect=r;
	if (self->drawborder) {
            /* around the entire window */
            sbuttonv::DrawRectBorder(self, &childrect, self->matteprefs, sbuttonv_BORDEROUT, draw, &childrect);

	    childrect.left+=WPADDING(self);
	    childrect.top+=WPADDING(self);
	    childrect.width-=WPADDING(self)*2;
	    childrect.height-=WPADDING(self)*2;
	    
            self->fullinterior = childrect;

	    if(self->child) for (i = 0; i < scroll_SIDES; i++) {
                if (desired.location & (1<<i)) {
                    switch(i) {
                        case scroll__LEFT:
                            childrect.left+=diff;
                            childrect.width-=diff;
                            break;
                        case scroll__RIGHT:
                            childrect.width-=diff;
                            break;
                        case scroll__TOP:
                            childrect.top+=diff;
                            childrect.height-=diff;
                            break;
                        case scroll__BOTTOM:
                            childrect.height-=diff;
                            break;
                    }
                }
            }
            if (self->child && childrect.width>0 && childrect.height>0) {
                /* around the child's portion */
                sbuttonv::DrawRectBorder(self, &childrect, self->matteprefs, sbuttonv_BORDERIN, draw, &childrect);
            }
	}
        else {
            self->fullinterior = r;

            for (i = 0; i < scroll_SIDES; i++) {
                if (desired.location & (1<<i)) {
                    switch(i) {
                        case scroll__LEFT:
                            childrect.left+=diff;
                            childrect.width-=diff;
                            break;
                        case scroll__RIGHT:
                            childrect.width-=diff;
                            break;
                        case scroll__TOP:
                            childrect.top+=diff;
                            childrect.height-=diff;
                            break;
                        case scroll__BOTTOM:
                            childrect.height-=diff;
                            break;
                    }
                }
            }

            for (i = 0; i < scroll_SIDES; i++) {
                int x1, y1, x2, y2;

                if (desired.location & (1<<i)) {
                    switch(i) {
                        case scroll__LEFT:
                            x1 = childrect.left - 1;
                            y1 = childrect.top;
                            x2 = x1;
                            y2 = y1 + childrect.height - 1;
                            break;
                        case scroll__RIGHT:
                            x1 = childrect.left + childrect.width;
                            y1 = childrect.top;
                            x2 = x1;
                            y2 = y1 + childrect.height - 1;
                            break;
                        case scroll__TOP:
                            x1 = childrect.left;
                            y1 = childrect.top - 1;
                            x2 = x1 + childrect.width - 1;
                            y2 = y1;
                            break;
                        case scroll__BOTTOM:
                            x1 = childrect.left;
                            y1 = childrect.top + childrect.height;
                            x2 = x1 + childrect.width - 1;
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

	if(self->child) for (i = 0; i < scroll_SIDES; i++)
	    if (desired.location & (1<<i)) {
		if(!barrects(self, i, NULL, NULL, NULL, childrect, fullinterior, desired)) desired.location &= ~(1<<i);
	    }

	if((childrect.height<=0 || childrect.width<=0) && self->child!=NULL) desired.location=0;
    } while (lastlocation != desired.location);
}


view_DSattributes scroll::DesiredSize(long width, long height, enum view_DSpass pass, long *desired_width, long *desired_height) {
    view_DSattributes result=(view_DSattributes)  (view_HeightFlexible | view_WidthFlexible);
    if(child==NULL) {
        *desired_width=width;
        *desired_height=height;
        return result;
    }
    
    if((this)->GetIM() && !this->prefsready) {
	    InitPrefs(this);
    }
    
    int i;
    long w=width, h=height;

    // first compute the area we would give to the child
    // if we actually get the suggested size.
    struct rectangle given;
    given.left=given.top=0;
    given.width=width;
    given.height=height;
    struct rectangle cr;
    struct rectangle fi;
    compute_inner(this, FALSE, given, cr, fi, desired);

    int diff=REALBARWIDTH(this) + VPADDING(this);
    if(drawborder) {

        result=child->DesiredSize(cr.width, cr.height, pass, &cr.width, &cr.height);
        
        long cdw, cdh;
        sbuttonv::SizeForBorder(this, sbuttonv_Enclosing, matteprefs, sbuttonv_BORDERIN, cr.width, cr.height, &cdw, &cdh);
        
        sbuttonv::SizeForBorder(this, sbuttonv_Enclosing, matteprefs, sbuttonv_BORDEROUT, cdw, cdh, &w, &h);
        w+=WPADDING(this)*2;
        h+=WPADDING(this)*2;
        for (i = 0; i < scroll_SIDES; i++) {
            if (ideal_location & (1<<i)) {
                switch(i) {
                    case scroll__LEFT:
                        w+=diff;
                        break;
                    case scroll__RIGHT:
                        w+=diff;
                        break;
                    case scroll__TOP:
                        h+=diff;
                        break;
                    case scroll__BOTTOM:
                        h+=diff;
                        break;
                }
            }
        }
    } else {
        result=child->DesiredSize(cr.width, cr.height, pass, &w, &h);
        
        for (i = 0; i < scroll_SIDES; i++) {
            if (ideal_location & (1<<i)) {
                switch(i) {
                    case scroll__LEFT:
                        w+=diff;
                        break;
                    case scroll__RIGHT:
                        w+=diff;
                        break;
                    case scroll__TOP:
                        h+=diff;
                        break;
                    case scroll__BOTTOM:
                        h+=diff;
                        break;
                }
            }
        }
    }
    *desired_width=w;
    *desired_height=h;
    return result;        
}

void scroll::GetOrigin(long width, long height, long * originX, long * originY) {
    struct rectangle given;
    given.left=given.top=0;
    given.width=width;
    given.height=height;
    struct rectangle cr;
    struct rectangle fi;
    
    if((this)->GetIM() && !this->prefsready) {
	    InitPrefs(this);
    }
    
    compute_inner(this, FALSE, given, cr, fi, desired);
    if(child) child->GetOrigin(cr.width, cr.height, originX, originY);
    *originY+=cr.top;
}

#if 0 /* does nothing useful */
static void calc_desired(class scroll  *self)
{
    int i, exists[scroll_TYPES];

    for (i = 0; i < scroll_TYPES; i++) 
        exists[i] = 0;

    for (i = 0; i < scroll_SIDES; i++) 
        if (self->desired.location & (1<<i)) 
            exists[Type[i]] = 1;

}
#endif



static boolean calc_elevator(class scroll  *self, int  side, struct scrollbar  *bar, struct rectangle  *r, boolean latest=TRUE)
{
    long min, height=bar_height(self, side) - self->endbarSpace;
    int diff;
    long x1, x2, y1, y2;
    if(self->regions[scroll_Elevator]==NULL) return FALSE;

    if (bar->endzones)
	min = self->min_elevator[1];
    else
	min = self->min_elevator[0];

    if (min > height) min = height;

    switch(side) {
	case scroll__LEFT:
	case scroll__RIGHT:
	    self->regions[scroll_Elevator]->SetMinHeight(min);
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
	 case scroll__TOP:
	 case scroll__BOTTOM:
	    self->regions[scroll_Elevator]->SetMinWidth(min);
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
    if(latest) self->regions[scroll_Elevator]->ComputeRect(*self, Type[side], self->interiors[side], *r);
    else self->regions[scroll_Elevator]->GetLastRect(*r, Type[side]);
#if 0
    /* I don't understand this section it apparently enforces a minimum elevator size while maintaining some
     other condition as well.  The other condition is related to endbarSpace somehow... -rr2b */
    switch(side) {
	case scroll__LEFT:
	case scroll__RIGHT:
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
	case scroll__TOP:
	case scroll__BOTTOM:
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

static boolean calc_dot(class scroll  *self, int  side, struct rectangle  *r, boolean latest=TRUE)
{
    int diff;
   
    long x1, y1, x2, y2;
    
    if(self->regions[scroll_Dot]==NULL) return FALSE;
      switch(side) {
	case scroll__LEFT:
	case scroll__RIGHT:

	    self->regions[scroll_Dot]->SetMinHeight(MINDOTLENGTH);
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
	case scroll__TOP:
	case scroll__BOTTOM:
	    self->regions[scroll_Dot]->SetMinWidth(MINDOTLENGTH);
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
    if(latest) self->regions[scroll_Dot]->ComputeRect(*self, Type[side], self->interiors[side], *r);
    else self->regions[scroll_Dot]->GetLastRect(*r, Type[side]);
    return TRUE;
}

static void draw_elevator(class scroll  *self, int  side)
{
    struct rectangle r;
    
    if(calc_elevator(self, side, &self->desired.bar[Type[side]], &r) && self->regions[scroll_Elevator] && !rectangle_IsEmptyRect(&r)) {
	self->regions[scroll_Elevator]->Draw(*self, Type[side], view_FullRedraw, r);
    }
}

static void draw_dot(class scroll  *self, int  side)
{
    struct rectangle r;
    if(calc_dot(self, side, &r) && self->regions[scroll_Dot] && !rectangle_IsEmptyRect(&r)) {
	self->regions[scroll_Dot]->Draw(*self, Type[side], view_FullRedraw, r);
    }
}

static void move_elevator(class scroll  *self, int  side)
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
    if(self->regions[scroll_Dot]) calc_dot(self, side, &odr, FALSE);
	  
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
	    case scroll__LEFT:
	    case scroll__RIGHT:
		if(oer.top<ner.top) (self)->FillRectSize( oer.left, oer.top, oer.width, ner.top-oer.top+1, NULL);
		if(oer.top+oer.height>ner.top+ner.height) (self)->FillRectSize( oer.left, ner.top+ner.height, oer.width, oer.top+oer.height-ner.top-ner.height, NULL);
		break;
	    case scroll__TOP:
	    case scroll__BOTTOM:
		if(oer.left<ner.left) (self)->FillRectSize( oer.left, oer.top, ner.left-oer.left+1, oer.height, NULL);
		if(oer.left+oer.width>ner.left+ner.width) (self)->FillRectSize( ner.left+ner.width, oer.top, oer.left+oer.width-ner.left-ner.width, oer.height, NULL);
		break;
	}
    }
    (self)->SetFGColor( oldfg[0], oldfg[1], oldfg[2]);
    if(redraw_elevator && eisvisible) draw_elevator(self, side);
    if(redraw_dot && disvisible) draw_dot(self, side);
}


static void draw_bar(class scroll  *self, int  side)
{
    struct sbuttonv_view_info vi;
    barrects(self, side, &self->interiors[side], &self->topbutton[side], &self->botbutton[side], self->childrect, self->fullinterior, self->desired);

    sbuttonv::SaveViewState(self, &vi);
    sbuttonv::DrawBorder(self, self->interiors[side].left, self->interiors[side].top, self->interiors[side].width, self->interiors[side].height, self->boxprefs, sbuttonv_BORDERIN, TRUE, &self->interiors[side]);

    sbuttonv::RestoreViewState(self, &vi);
    draw_elevator(self, side);
    draw_dot(self, side);
    if(self->desired.bar[Type[side]].endzones) {
	draw_arrow(self, side, &self->topbutton[side], 1, self->mousestate==scroll_TOPENDZONE && self->side==side);
	draw_arrow(self, side, &self->botbutton[side], -1, self->mousestate==scroll_BOTTOMENDZONE && self->side==side);
    }
}


static void draw_everything(class scroll  *self)
{
    int i;
    struct rectangle r;
    struct sbuttonv_view_info vi;
    class region *re1, *re2;
    (self)->GetLogicalBounds( &r);


    if(self->child) {
	
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

    compute_inner(self, TRUE, r, self->childrect, self->fullinterior, self->desired);

    sbuttonv::RestoreViewState(self, &vi);

    for (i = 0; i < scroll_SIDES; i++)
	if (self->desired.location & (1<<i)) {
	    draw_bar(self, i);
	}
}

static void InitPrefs(class scroll  *self)
{
    boolean graphicIsMono;
    boolean mono;

    graphicIsMono = ((self)->GetDrawable())->DisplayClass() & graphic_Monochrome;
    mono = environ::GetProfileSwitch("MimicOldScrollbar", graphicIsMono ? TRUE : FALSE);

    self->drawborder = environ::GetProfileSwitch("ScrollDrawBorders", mono ? MONODRAWBORDERS : COLORDRAWBORDERS);
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
    
    if(self->regions[scroll_UpperEnd]) self->regions[scroll_UpperEnd]->SetBaseSButtonPrefs(self->buttonprefs);
    if(self->regions[scroll_BottomEnd]) self->regions[scroll_BottomEnd]->SetBaseSButtonPrefs(self->buttonprefs);
    if(self->regions[scroll_LeftEnd]) self->regions[scroll_LeftEnd]->SetBaseSButtonPrefs(self->buttonprefs);
    if(self->regions[scroll_RightEnd]) self->regions[scroll_RightEnd]->SetBaseSButtonPrefs(self->buttonprefs);
    
    self->elevatorprefs= sbutton::DuplicatePrefs(self->prefs, scrollbarelevator);
    if(self->elevatorprefs==NULL) return;
    sbutton::InitPrefs(self->elevatorprefs, scrollbarelevator);
    if(self->regions[scroll_Elevator]) self->regions[scroll_Elevator]->SetBaseSButtonPrefs(self->elevatorprefs);
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
    if(self->regions[scroll_Dot]) self->regions[scroll_Dot]->SetBaseSButtonPrefs(self->dotprefs);

    self->windowPadding = environ::GetProfileInt("ScrollWindowPadding", mono ? MONOWINDOWPADDING : COLORWINDOWPADDING);

    self->viewPadding = self->windowPadding + 1;

    self->barWidth = environ::GetProfileInt("scrollbarWidth", mono ? MONOBARWIDTH : COLORBARWIDTH);
    self->dotWidth = environ::GetProfileInt("DotWidth", mono ? MONODOTWIDTH : COLORDOTWIDTH);

    self->elevatorWidth = environ::GetProfileInt("ElevatorWidth", mono ? MONOELEVATORWIDTH : COLORELEVATORWIDTH);

    self->endzoneLength = environ::GetProfileInt("ScrollButtonSize", mono ? MONOENDZONELENGTH : COLORENDZONELENGTH);

    self->buttonSpace = environ::GetProfileInt("ScrollButtonPadding", mono ? MONOBARTOBUTTONSPACE : COLORBARTOBUTTONSPACE);

    sbuttonv::InteriorBGColor(self, self->boxprefs, TRUE, self->barbackground);
    sbuttonv::InteriorBGColor(self, self->matteprefs, TRUE, self->mattebackground);

    self->prefsready=TRUE;

    if(self->elevatorWidth<=0) self->elevatorWidth=1;
    if(self->dotWidth<=0) self->dotWidth=1;
    if(self->barWidth<4) self->barWidth=4;
   
}

/* Overrides of the view routines: */

void scroll::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
    struct rectangle r;
    int i;

    if((this)->GetIM() && !this->prefsready) {
	    InitPrefs(this);
    }
    (this)->GetLogicalBounds( &r);
    
    this->pending_update = 0;
    
    if((type==view_FullRedraw || type==view_LastPartialRedraw || type==view_MoveNoRedraw ||
	(type!=view_Remove && (this->lastwidth!=r.width ||
		this->lastheight!=r.height))) && this->child) {
	this->lastwidth=r.width;
	this->lastheight=r.height;
	this->childrect=r;
	compute_inner(this, FALSE, r, childrect, fullinterior, this->desired);
	(this->child)->InsertView( this, &this->childrect);
	
    }
    
    this->force_full_update = FALSE;
    switch(type) {
	case view_Remove:
	    for (i = 0; i < scroll_SIDES; i++)
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
	    /* calc_desired(this); */
	    draw_everything(this);
	    break;
	case view_FullRedraw:
	    if(this->child) (this->child)->FullUpdate( view_FullRedraw, r.left, r.top, r.width, r.height);			      
	    if(this->si) this->si->UpdateRegions(*this);
	    /* calc_desired(this); */
	    draw_everything(this);
	    break;
	default: ;
    }
    if(type!=view_Remove && type!=view_PartialRedraw) {
	for (i = 0; i < scroll_SIDES; i++)
	    if ((this->desired.location & (1<<i)) && this->barcursor[Type[i]]!=NULL) {
		(this)->RetractCursor( this->barcursor[Type[i]]);
		(this)->PostCursor( &this->interiors[i], this->barcursor[Type[i]]);
	    }
    }

    this->current = this->desired;
    (this)->FlushGraphics();
}


void scroll::Update()
{
    int i;
    struct rectangle r;

    (this)->GetLogicalBounds( &r);

    if (r.width <= 0 || r.height <= 0) return;
    this->pending_update = 0;
    /* Let the children modify their state however they want. */
    (this->updatelistp)->Clear();

    if(this->si) this->si->UpdateRegions(*this);
    
    /* calc_desired(this); */

    if (this->current.location != this->desired.location || this->force_full_update) {
	(this)->EraseVisualRect();
	(this)->FullUpdate( view_FullRedraw, r.left, r.top, r.width, r.height);
    } else {
	for (i = 0; i < scroll_SIDES; i++)
	    if (this->desired.location & (1<<i)) {
		int type = Type[i];
		struct scrollbar *des = &this->desired.bar[type], *cur = &this->current.bar[type];
		if (des->endzones != cur->endzones)
		    /* The entire scrollbar need redrawing? */
		    draw_bar(this, i);
		else {
		    if((this->regions[scroll_Elevator] && this->regions[scroll_Elevator]->changed) || (this->regions[scroll_Dot] && this->regions[scroll_Dot]->changed)) move_elevator(this, i);
		}
	    }
	this->current = this->desired;
    }
}

void scroll::WantUpdate(class view  *requestor)
{
    if ((class view *)this != requestor)
        (this->updatelistp)->AddTo( requestor);

    if (!this->pending_update) {
        this->pending_update = 1;
        (this)->view::WantUpdate( this);
    }
}

static void DoRepeatScroll(class scroll  *self)
{
    if(self->si==NULL) return;
    if(self->emulation) {
	
	if(self->regions[scroll_Elevator]==NULL) return;
	switch(self->dir) {
	    case scroll_UP:
		if(Type[self->side]==scroll_VERT) self->si->ScreenDelta(0, CHILDBARHEIGHT(self, self->side)-1);
		else self->si->ScreenDelta(CHILDBARHEIGHT(self, self->side)-1, 0);
		break;
	    case scroll_DOWN:
		if(Type[self->side]==scroll_VERT) self->si->ScreenDelta(0, -CHILDBARHEIGHT(self, self->side)+1);
		else self->si->ScreenDelta(-(CHILDBARHEIGHT(self, self->side)-1), 0);
	}	
    } else {
	if (self->lastaction==view_LeftDown) {
	    if(Type[self->side]==scroll_VERT) self->si->ScreenDelta(0, -self->hitcoord);
	    else self->si->ScreenDelta(-self->hitcoord, 0);
	} else if (self->lastaction==view_RightDown) {
	    if(Type[self->side]==scroll_VERT) self->si->ScreenDelta(0, self->hitcoord);
	    else self->si->ScreenDelta(self->hitcoord, 0);
	}
    }
}

static void RepeatScroll(class scroll  *self, long  cTime)
{
    long timeInterval;

    self->scrollEvent = NULL;
    if(self->side == -1 || (self->mousestate!=scroll_MAYBETHUMBING && self->mousestate!=scroll_REPEATING)) return;

    self->mousestate=scroll_REPEATING;
    DoRepeatScroll(self);
    timeInterval = self->minContScrollTime + (((self->maxContScrollTime - self->minContScrollTime) * self->hitcoord) / (PIXELSPERINCH(self) * 10));

    self->scrollEvent = im::EnqueueEvent((event_fptr) RepeatScroll, (char *) self, event_MSECtoTU(timeInterval));
}

#define PTINRECT(r, x, y) ((x)>=(r)->left && (x)<(r)->left+(r)->width && (y)>=(r)->top && (y)<(r)->top+(r)->height)
#define ENDZONEREPTIME(self) (self->endzonereptime)


static void endzone(class scroll  *self, int  side, int  end, enum view_MouseAction  action)
{
    int type = Type[side];
    if(action!=view_LeftDown && action!=view_RightDown) return;
    
    if(self->si==NULL) return;
    
    if(type==scroll_VERT) {
	if(action==view_LeftDown && !self->emulation) {
	    if(end==scroll_TOPENDZONE) self->si->Extreme(scroll_Up);
	    else self->si->Extreme(scroll_Down);
	} else {
	    if(end==scroll_TOPENDZONE) self->si->Shift(scroll_Up);
	    else self->si->Shift(scroll_Down);
	}
    } else {
	if(action==view_LeftDown && !self->emulation) {
	    if(end==scroll_TOPENDZONE) self->si->Extreme(scroll_Left);
	    else self->si->Extreme(scroll_Right);
	} else {
	    if(end==scroll_TOPENDZONE) self->si->Shift(scroll_Left);
	    else self->si->Shift(scroll_Right);
	}
    }
}

static void RepeatEndZone(class scroll  *self, long  cTime)
{
    self->scrollEvent=NULL;
    if(self->mousestate!=scroll_TOPENDZONE && self->mousestate!=scroll_BOTTOMENDZONE) return;
    
    endzone(self, self->side, self->mousestate, self->lastaction);
    ScheduleRepeatEndZone(self);
}

static void CancelScrollEvent(class scroll  *self)
{
    if(self->scrollEvent) {
	(self->scrollEvent)->Cancel();
	self->scrollEvent=NULL;
    }
}

static void ScheduleRepeatEndZone(class scroll  *self)
{
    CancelScrollEvent(self);
    self->scrollEvent = im::EnqueueEvent((event_fptr) RepeatEndZone, (char *) self, event_MSECtoTU(ENDZONEREPTIME(self)));
}

static void HandleEndZone(class scroll  *self, enum view_MouseAction  action, long  x , long  y)
{

    struct rectangle *r;
    int dir;
    
    if(self->side==-1) return;
    
    /* if we're here self->mousestate==scroll_TOPENDZONE or self->mousestate==scroll_BOTTOMENDZONE and self->side is a valid side */
    
    if(self->mousestate==scroll_TOPENDZONE) {
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
		CancelScrollEvent(self);
		if(self->lastbuttonstate) draw_arrow(self, self->side, r, dir, self->lastbuttonstate=FALSE);
	    }
	    break;
	case view_LeftUp:
	case view_RightUp:
	    /* the code in scroll_Hit will handle canceling the event */
	    if(self->lastbuttonstate) draw_arrow(self, self->side, r, dir, self->lastbuttonstate=FALSE);
	default:
	    break;
    }
    endzone(self, self->side, self->mousestate, action);
}

static void HandleRepeatMode(class scroll  *self, enum view_MouseAction  action, long  x , long  y)
{
    long coord;
    
    if(self->side==-1) return;

    
    coord=XYTOHITCOORD(self, self->side, x, y);
    if(self->adjustScroll && self->mousestate!=scroll_MAYBETHUMBING) self->hitcoord=coord;
    
    if(action!=view_LeftUp && action!=view_RightUp) return;

    DoRepeatScroll(self);
    
    im::ForceUpdate();
}

static boolean CheckEndZones(class scroll  *self, enum view_MouseAction  action, long  x , long  y)
{
    int i;

    /* if we're here self->mousestate==scroll_NOTHING */
    
    if(action!=view_LeftDown && action!=view_RightDown) return FALSE;

    for(i=0;i<scroll_SIDES;i++)
	if((self->desired.location & (1<<i)) &&
	   self->desired.bar[Type[i]].endzones) {
	    int dir=1;
	    struct rectangle *r = NULL;
	    if(PTINRECT(&self->topbutton[i], x, y)) {
		r = &self->topbutton[i];
		dir=1;
		self->mousestate=scroll_TOPENDZONE;
	    } else if(PTINRECT(&self->botbutton[i], x, y)) {
		r = &self->botbutton[i];
		dir=(-1);
		self->mousestate=scroll_BOTTOMENDZONE;
	    }
	    /* else the mouse state will be scroll_NOTHING */

	    self->lastaction=view_NoMouseEvent;

	    if(self->mousestate!=scroll_NOTHING) {
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

static void HandleThumbing(class scroll  *self, enum view_MouseAction  action, long  x , long  y)
{
    /* Assure that we have the correct button */
    if (((action == view_LeftMovement || action == view_LeftUp) && (self->lastaction != view_LeftDown)) || ((action == view_RightMovement || action == view_RightUp) && (self->lastaction != view_RightDown)) || self->side==(-1))
	return;
    
    struct scrollbar *cur, *des;
    long coord=0;
    int i;
    long posn;
    long logicalTop, logicalHeight, logicalPos;
    long stype=self->side>0?Type[self->side]:scroll_VERT;
    
    long top=(Type[self->side]==scroll_VERT)?self->regions[scroll_Elevator]->rect[stype].top:self->regions[scroll_Elevator]->rect[stype].left;
#if 0
    long bot=(Type[self->side]==scroll_VERT)?self->regions[scroll_Elevator]->rect[stype].top+self->regions[scroll_Elevator]->rect[stype].height:self->regions[scroll_Elevator]->rect[stype].left+self->regions[scroll_Elevator]->rect[stype].width;
#endif
    long dbeg=top;
    long lheight=(Type[self->side]==scroll_VERT)?self->interiors[self->side].height:self->interiors[self->side].width;

    cur = &self->current.bar[Type[self->side]];
    des = &self->desired.bar[Type[self->side]];

    if (self->side == scroll__LEFT || self->side == scroll__RIGHT) {
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
	coord=logicalHeight/* -self->endbarSpace */;
	logicalPos=logicalTop+coord;
    }
    
    if (action == view_LeftMovement || action == view_RightMovement) {
	posn = coord;
	if(1 ||ABS(posn-top+self->seenLength/2)>=self->seenLength/10) {
	    int location=self->current.location;

	    dbeg=posn/* -self->seenLength/2 */;
	    if (dbeg<0) {
		dbeg=0;
	    }
	    else if (dbeg > lheight) {
		dbeg= lheight;
	    }
	    for (i = 0; i < scroll_SIDES; i++)  {
		if ((location & (1<<i)) && Type[i] == Type[self->side])  {
		    if (self->si && self->thumbScroll && (self->lastaction==view_LeftDown || (self->emulation && self->lastaction==view_RightDown)))  {
			if(Type[self->side]==scroll_VERT) self->si->Absolute(0,0,lheight,dbeg);
			else self->si->Absolute(lheight, dbeg, 0, 0);
		    } else move_elevator(self, i);
		}
	    }
	    if (self->thumbScroll && (self->lastaction==view_LeftDown || (self->emulation && self->lastaction==view_RightDown))) {
		im::ForceUpdate();
	    } else *cur = *des;
	}
    } else { /* this is an up transition */
	(self->imPtr)->SetWindowCursor( NULL);

	dbeg=coord/* -self->seenLength/2 */;
	
	if (dbeg < 0)
	    dbeg = 0;
	else if (dbeg > lheight) {
	    dbeg = lheight;
	}
	if(self->si) {
	    if(Type[self->side]==scroll_VERT)
		self->si->Absolute(0,0,lheight,dbeg);
	    else self->si->Absolute(lheight,dbeg, 0, 0);
	}
	im::ForceUpdate();
    }
}

static void CheckBars(class scroll  *self, enum view_MouseAction  action, long  x , long  y)
{
    int i;
    struct scrollbar *bar=NULL;
    struct rectangle r;
    
    if(action!=view_LeftDown && action!=view_RightDown) return;
    
    for(i=0;i<scroll_SIDES;i++) if(PTINRECT(&self->interiors[i], x, y)) break;

    if(i>=scroll_SIDES) return;
    
    bar= &self->current.bar[Type[self->side=i]];
    
    self->hitcoord=XYTOHITCOORD(self, i, x, y);

    self->mousestate=scroll_REPEATING;
    if (calc_elevator(self, i, bar, &r)) {
	switch(i) {
	    case scroll__LEFT:
	    case scroll__RIGHT:
		r.left=self->interiors[i].left;
		r.width=self->interiors[i].width;
		if(self->hitcoord<r.top) self->dir=scroll_UP;
		else if(self->hitcoord>r.top+r.height) self->dir=scroll_DOWN;
		else self->dir=scroll_NOWHERE;
		if(PTINRECT(&r, x, y)) {
		    self->mousestate=scroll_MAYBETHUMBING;
		    self->seenLength = r.height;
		}
		break;
	    case scroll__TOP:
	    case scroll__BOTTOM:
		r.top=self->interiors[i].top;
		r.height=self->interiors[i].height;
		if(self->hitcoord<r.left) self->dir=scroll_UP;
		else if (self->hitcoord>r.left+r.width) self->dir=scroll_DOWN;
		else self->dir=scroll_NOWHERE;
		if(PTINRECT(&r, x, y)) {
		    self->mousestate=scroll_MAYBETHUMBING;
		    self->seenLength = r.width;
		}
	}
    }
    self->side = i;
    self->lastaction=action;
    if(self->emulation && action==view_RightDown) {
	if(Type[i]==scroll_VERT) self->seenLength = r.height;
	else self->seenLength=r.width;
	self->mousestate=scroll_THUMBING;
	if(self->cursor!=NULL) ((self)->GetIM())->SetWindowCursor( self->cursor);
	HandleThumbing(self, view_RightMovement, x, y);
    } else if (self->startScrollTime > 0)  {
	self->scrollEvent = im::EnqueueEvent((event_fptr)RepeatScroll, (char *) self, event_MSECtoTU(self->startScrollTime));
    }
}

static void MaybeStartThumbing(class scroll  *self, enum view_MouseAction  action, long  x , long  y)
{
    if(self->side==-1) return;

    if(ABS(XYTOHITCOORD(self, self->side, x, y) - self->hitcoord) <= SMALLDIST) {
	HandleRepeatMode(self, action, x, y);
	return;
    }
    
    if(action==view_LeftDown || action==view_RightDown) return;
    
    self->mousestate=scroll_THUMBING;
    CancelScrollEvent(self);

    if(self->cursor!=NULL) ((self)->GetIM())->SetWindowCursor( self->cursor);
    
    HandleThumbing(self, action, x, y);
}


class view *scroll::Hit(enum view_MouseAction  action, long  x , long  y , long  num_clicks)
{
    switch(this->mousestate) {
	case scroll_THUMBING:
	    HandleThumbing(this, action, x, y);
	    break;
	case scroll_MAYBETHUMBING:
	    MaybeStartThumbing(this, action, x, y);
	    break;
	case scroll_REPEATING:
	    HandleRepeatMode(this, action, x, y);
	    break;
	case scroll_TOPENDZONE:
	case scroll_BOTTOMENDZONE:
	    HandleEndZone(this, action, x, y);
	    break;
	case scroll_NOTHING:
	    if(action!=view_LeftDown && action!=view_RightDown &&
	       action!=view_LeftFileDrop && action!=view_MiddleFileDrop &&
	       action!=view_RightFileDrop) return (class view *)this;
	    if(this->child && PTINRECT(&this->childrect, x, y)) return (this->child)->Hit( action, (this->child)->EnclosedXToLocalX( x), (this->child)->EnclosedYToLocalY( y), num_clicks);

	    if(CheckEndZones(this, action, x, y)) return (class view *)this;

	    CheckBars(this, action, x, y);
	default: ;
    }
    
    /* if a button has gone up cancel any scheduled scrolling and set the state back to normal (ie anything can happen). */
    if(action==view_RightUp || action==view_LeftUp) {
	CancelScrollEvent(this);
	this->mousestate=scroll_NOTHING;
	this->side=(-1);
    }

    return (class view *)this;
}

void scroll::LinkTree(class view  *parent)
{
    (this)->view::LinkTree( parent);
    if (this->child)
	(this->child)->LinkTree( this);
}

void scroll::UnlinkNotification(class view  *unlinkedTree)
{
    (this)->view::UnlinkNotification( unlinkedTree);
    (this->updatelistp)->DeleteTree( unlinkedTree);
}

boolean scroll::RecSearch(struct SearchPattern *pat, boolean toplevel)
{
    if (this->scrollee)
	return this->scrollee->RecSearch(pat, toplevel);
    return FALSE;
}

boolean scroll::RecSrchResume(struct SearchPattern *pat)
{
    if (this->scrollee)
	return this->scrollee->RecSrchResume(pat);
    return FALSE;
}

boolean scroll::RecSrchReplace(class dataobject *text, long pos, long len)
{
    if (this->scrollee)
	return this->scrollee->RecSrchReplace(text, pos, len);
    return FALSE;
}

void scroll::RecSrchExpose(const struct rectangle &logical, struct rectangle &hit)
{
    if (scrollee) {
	struct rectangle child, full;
	struct scrollstate ds=desired;
	scrollee->LinkTree(this);
	struct rectangle oldrect;
	scrollee->GetEnclosedBounds(&oldrect);
	scrollee->InsertView(this, &oldrect);

	if((this)->GetIM() && !this->prefsready) {
	    InitPrefs(this);
	}
	compute_inner(this, FALSE, logical, child, full, ds);
	scrollee->RecSrchExpose(child, hit);
    }
}

void scroll::PrintPSDoc(FILE *outfile, long pagew, long pageh)
{
    if (this->scrollee)
	this->scrollee->PrintPSDoc(outfile, pagew, pageh);
}

void *scroll::GetPSPrintInterface(const char *printtype)
{
    if (this->scrollee)
	return this->scrollee->GetPSPrintInterface(printtype);
    return NULL;
}

void scroll::DesiredPrintSize(long width, long height, enum view_DSpass pass, long *desiredwidth, long *desiredheight)
{
    if (this->scrollee)
	this->scrollee->DesiredPrintSize(width, height, pass, desiredwidth, desiredheight);
}

void scroll::PrintPSRect(FILE *outfile, long logwidth, long logheight, struct rectangle *visrect)
{
    if (this->scrollee)
	this->scrollee->PrintPSRect(outfile, logwidth, logheight, visrect);
}

void scroll::GetPrintOrigin(long w, long h, long *xoff, long *yoff) {
    if(scrollee) scrollee->GetPrintOrigin(w, h, xoff, yoff);
}

