/* 
	Copyright IBM Corporation 1988,1991 - All Rights Reserved
	Copyright Carnegie Mellon University 1996
	For full copyright information see: <src or dest>/doc/COPYRITE
*/

#include <andrewos.h>
ATK_IMPL("hwview.H")
#include "hwview.H"

#include "graphic.H"
#include "rect.h"
#include "keymap.H"
#include "keystate.H"
#include "menulist.H"
#include "scroll.H"
#include "bind.H"
#include "message.H"

#include "helloworld.H"

#define TOTALSIZE 1500

   
 

ATKdefineRegistry(helloworldview, view, helloworldview::InitializeClass);
static void Center(class helloworldview  *hwv, long  rock);
static void Invert(class helloworldview  *hwv, long  rock);
static void xgetinfo(class helloworldview  *hwv, struct range  *total , struct range  *seen , struct range  *dot);
static void ygetinfo(class helloworldview  *hwv, struct range  *total , struct range  *seen , struct range  *dot);
static void xsetframe(class helloworldview  *hwv, int  posn, long  cord , long  outof);
static void ysetframe(class helloworldview  *hwv, int  posn, long  cord , long  outof);
static long xwhat(class helloworldview  *hwv, long  cord , long  outof);
static long ywhat(class helloworldview  *hwv, long  cord , long  outof);


static const struct scrollfns horizInterface = {
    (scroll_getinfofptr)xgetinfo, (scroll_setframefptr)xsetframe, NULL, (scroll_whatfptr)xwhat
};

static const struct scrollfns vertInterface = {
    (scroll_getinfofptr)ygetinfo, (scroll_setframefptr)ysetframe, NULL, (scroll_whatfptr)ywhat
};

static class keymap *helloworldviewKeymap;
static class menulist *helloworldviewMenulist;



helloworldview::helloworldview()
{
	ATKinit;

    this->haveInputFocus = FALSE;
    this->HaveDownTransition=FALSE;
    this->keystate = keystate::Create(this, helloworldviewKeymap);
    this->menulistDup = (helloworldviewMenulist)->DuplicateML( this);
    this->newFrameX = this->newFrameY = 0;
    THROWONFAILURE( TRUE);
}

class view *helloworldview::GetApplicationLayer()
{
    return (class view *)scroll::Create(this,scroll_LEFT+scroll_BOTTOM);
}

void helloworldview::DeleteApplicationLayer(class view  *scrollbar)
{
    ((class scroll *)scrollbar)->Destroy();
}

void helloworldview::FullUpdate(enum view::UpdateType  type, long  left, long  top, long  width, long  height )
{
    class helloworld *hw=(class helloworld *)this->GetDataObject();
    struct rectangle myVisualRect;

    (this)->GetVisualBounds(&myVisualRect);
    this->vrWidth=rectangle_Width(&myVisualRect);
    this->vrHeight=rectangle_Height(&myVisualRect);

    if (this->newFrameX + this->vrWidth > TOTALSIZE)
	this->newFrameX = TOTALSIZE - this->vrWidth;
    if (this->newFrameY + this->vrHeight > TOTALSIZE)
	this->newFrameY = TOTALSIZE - this->vrHeight;

    this->frameX = this->newFrameX;
    this->frameY = this->newFrameY;

    if (hw->x == POSUNDEF) {
	hw->x = this->frameX + this->vrWidth / 2;
	hw->y = this->frameY + this->vrHeight / 2;
    }

    this->x=hw->x;
    this->y=hw->y;
    this->blackOnWhite=hw->blackOnWhite;

    (this)->SetTransferMode( graphic::COPY);

    if (hw->blackOnWhite)
	(this)->FillRect( &myVisualRect, (this)->WhitePattern());
    else 
	(this)->FillRect( &myVisualRect, (this)->BlackPattern());

    (this)->SetTransferMode(graphic::INVERT);

    (this)->MoveTo( hw->x - this->frameX, hw->y - this->frameY);
    (this)->DrawString("hello world",
			   graphic::BETWEENTOPANDBASELINE | graphic::BETWEENLEFTANDRIGHT);    
}


void helloworldview::Update()
{    
    class helloworld *hw=(class helloworld *)this->GetDataObject();

    (this)->SetTransferMode( graphic::INVERT);

    if (hw->blackOnWhite!=this->blackOnWhite) {
	struct rectangle vr;
	(this)->GetVisualBounds(&vr);
	(this)->FillRect(&vr,(this)->BlackPattern());
	this->blackOnWhite=hw->blackOnWhite;
    }

    if (this->x!=hw->x || this->y!=hw->y || this->frameX!=this->newFrameX || this->frameY!=this->newFrameY) {
	if(this->x!=hw->x || this->y!=hw->y){
	    static char buf[100];
	    sprintf(buf,"Hello world at (%ld,%ld)",hw->x,hw->y);
	    message::DisplayString(this,0,buf);
	}	    

	(this)->MoveTo(
			      this->x-this->frameX,this->y-this->frameY);
	(this)->DrawString( "hello world",
				  graphic::BETWEENTOPANDBASELINE |
				  graphic::BETWEENLEFTANDRIGHT);
  
	this->x=hw->x;
	this->y=hw->y;
  	this->frameX = this->newFrameX;
  	this->frameY = this->newFrameY;
  
	(this)->MoveTo(
			      this->x-this->frameX,this->y-this->frameY);
	(this)->DrawString( "hello world",
				  graphic::BETWEENTOPANDBASELINE |
				  graphic::BETWEENLEFTANDRIGHT);
    }
}


class view *helloworldview::Hit(enum view::MouseAction  action, long  x, long  y, long  numberOfClicks)
{
    class helloworld *hw=(class helloworld *)this->GetDataObject();

    if(this->HaveDownTransition)
	switch(action){
	    case view::RightUp:
		this->HaveDownTransition=FALSE;
		/* fall through */
	    case view::RightMovement:
		hw->x+=x-this->hitX;
		hw->y+=y-this->hitY;
		this->hitX=x;
		this->hitY=y;
		break;
	    case view::LeftUp:
		this->HaveDownTransition=FALSE;
		hw->x=x+this->frameX;
		hw->y=y+this->frameY;
		break;
	    case view::LeftMovement:
		/* do nothing */
		break;
	    default:
		/* re-synchronize mouse */
		this->HaveDownTransition=FALSE;
	}

    if(!this->HaveDownTransition)
	switch(action){
	    case view::RightDown:
		this->hitX=x;
		this->hitY=y;
		/* fall through */
	    case view::LeftDown:
		this->HaveDownTransition=TRUE;
		(this)->WantInputFocus(this);
		break;
	    default:
		break;
	}

    (hw)->NotifyObservers(0);

    return (class view *)this;
}


void helloworldview::ReceiveInputFocus()
{
    this->haveInputFocus = TRUE;
    this->keystate->next = NULL;
    (this)->PostKeyState( this->keystate);
    (this)->PostMenus( this->menulistDup);
}


void helloworldview::LoseInputFocus()
{
    this->haveInputFocus = FALSE;
}

static void Center(class helloworldview  *hwv, long  rock)
{
    class helloworld *hw=(class helloworld *)hwv->GetDataObject();

    hw->x = hwv->newFrameX + hwv->vrWidth / 2;
    hw->y = hwv->newFrameY + hwv->vrHeight / 2;

    (hw)->NotifyObservers(0);
}


static void Invert(class helloworldview  *hwv, long  rock)
{
    class helloworld *hw=(class helloworld *)hwv->GetDataObject();

    hw->blackOnWhite = !hw->blackOnWhite;
    (hw)->NotifyObservers(0);
}


static void relocate(class helloworldview  *hwv,long  rock)
{
    class helloworld *hw=(class helloworld *)hwv->GetDataObject();
    char buf[100];
    int x,y;

    message::AskForString(hwv,0,"New location (x,y): ",NULL,buf,sizeof(buf));

    if(sscanf(buf,"%d,%d",&x,&y)!=2)
	message::DisplayString(hwv,1,"Bad format; must be: number,number");
    else{
	hw->x=x;
	hw->y=y;

	(hw)->NotifyObservers(0);
    }
}


static void xgetinfo(class helloworldview  *hwv, struct range  *total , struct range  *seen , struct range  *dot)
{
    class helloworld *hw=(class helloworld *)hwv->GetDataObject();

    total->beg = 0;
    total->end = TOTALSIZE;
    seen->beg = hwv->frameX;
    seen->end = hwv->frameX + hwv->vrWidth;
    dot->beg = dot->end = hw->x;
}

static void ygetinfo(class helloworldview  *hwv, struct range  *total , struct range  *seen , struct range  *dot)
{
    class helloworld *hw=(class helloworld *)hwv->GetDataObject();

    total->beg = 0;
    total->end = TOTALSIZE;
    seen->beg = hwv->frameY;
    seen->end = hwv->frameY + hwv->vrHeight;
    dot->beg = dot->end = hw->y;
}

static void xsetframe(class helloworldview  *hwv, int  posn, long  cord , long  outof)
{
    hwv->newFrameX = posn - hwv->vrWidth * cord / outof;
    if (hwv->newFrameX + hwv->vrWidth > TOTALSIZE)
	hwv->newFrameX = TOTALSIZE - hwv->vrWidth;
    else if (hwv->newFrameX < 0)
	hwv->newFrameX = 0;
    (hwv)->WantUpdate( hwv);
}

static void ysetframe(class helloworldview  *hwv, int  posn, long  cord , long  outof)
{
    hwv->newFrameY = posn - hwv->vrHeight * cord / outof;
    if (hwv->newFrameY + hwv->vrHeight > TOTALSIZE)
	hwv->newFrameY = TOTALSIZE - hwv->vrHeight;
    else if (hwv->newFrameY < 0)
	hwv->newFrameY = 0;
    (hwv)->WantUpdate( hwv);
}

static long xwhat(class helloworldview  *hwv, long  cord , long  outof)
{
    return hwv->frameX + hwv->vrWidth * cord / outof;
}

static long ywhat(class helloworldview  *hwv, long  cord , long  outof)
{
    return hwv->frameY + hwv->vrHeight * cord / outof;
}


const void *helloworldview::GetInterface(const char  *type)
{
    if (strcmp(type, "scroll,vertical") == 0)
	return &vertInterface;
    else if (strcmp(type, "scroll,horizontal") == 0)
	return &horizInterface;
    else
	return NULL;
}


static struct bind_Description helloworldviewBindings[]={
    {"helloworld-center", "\003",0, "Hello World,Center",0,0, (proctable_fptr)Center, "Center the helloworld string."},
    {"helloworld-invert", "\011",0, "Hello World,Invert",0,0, (proctable_fptr)Invert, "Invert the helloworld string."},
    {"helloworld-relocate", "\022",0, "Hello World,Relocate",0,0, (proctable_fptr)relocate, "Relocate the helloworld string."},
    NULL
};

boolean helloworldview::InitializeClass()
{
    helloworldviewMenulist=new menulist;
    helloworldviewKeymap=new keymap;
    bind::BindList(helloworldviewBindings, helloworldviewKeymap,helloworldviewMenulist, &helloworldview_ATKregistry_ );
    return TRUE;
}
