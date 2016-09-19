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
#include "bind.H"

#define POSUNDEF -1

static class keymap *helloworldviewKeymap;
static class menulist *helloworldviewMenulist;


ATKdefineRegistry(helloworldview, view, helloworldview::InitializeClass);
static void Center(class helloworldview  *hwv, long  rock);
static void Invert(class helloworldview  *hwv, long  rock);


helloworldview::helloworldview()
{
	ATKinit;

    this->x = POSUNDEF;
    this->y = POSUNDEF;
    this->HaveDownTransition = FALSE;
    this->haveInputFocus = FALSE;
    this->keystate = keystate::Create(this, helloworldviewKeymap);
    this->blackOnWhite = TRUE;
    this->newBlackOnWhite = TRUE;
    this->menulistDup = (helloworldviewMenulist)->DuplicateML( this);
    THROWONFAILURE( TRUE);
}

void helloworldview::FullUpdate(enum view_UpdateType  type, long  left, long  top, long  width, long  height )
{
    struct rectangle myVisualRect;

    (this)->GetVisualBounds(&myVisualRect);

    if (this->x == POSUNDEF) {
	this->x = rectangle_Left(&myVisualRect) + rectangle_Width(&myVisualRect) / 2;
	this->y = rectangle_Top(&myVisualRect) + rectangle_Height(&myVisualRect) / 2;
	this->newX = this->x;
	this->newY = this->y;
    }
    else {
	this->x = this->newX;
	this->y = this->newY;
    }

    (this)->SetTransferMode( graphic_COPY);

    if (this->blackOnWhite)
	(this)->FillRect( &myVisualRect, (this)->WhitePattern());
    else
	(this)->FillRect( &myVisualRect, (this)->BlackPattern());

    (this)->SetTransferMode( graphic_INVERT);

    (this)->MoveTo( this->x, this->y);
    (this)->DrawString("hello world",
			   graphic_BETWEENTOPANDBASELINE | graphic_BETWEENLEFTANDRIGHT);    
}


void helloworldview::Update()
{    
    /* TransferMode is graphic_INVERT from the last FullUpdate */

    if (this->newBlackOnWhite != this->blackOnWhite)  {
	struct rectangle vr;

	(this)->GetVisualBounds(&vr);
	(this)->FillRect( &vr, (this)->BlackPattern());
	this->blackOnWhite = this->newBlackOnWhite;
    }

    if (this->newX != this->x || this->newY != this->y) {
	(this)->MoveTo( this->x, this->y);
	(this)->DrawString( "hello world", graphic_BETWEENTOPANDBASELINE | graphic_BETWEENLEFTANDRIGHT);

	this->x = this->newX;
	this->y = this->newY;

	(this)->MoveTo( this->x, this->y);
	(this)->DrawString( "hello world", graphic_BETWEENTOPANDBASELINE | graphic_BETWEENLEFTANDRIGHT);
    }
}


class view *helloworldview::Hit(enum view_MouseAction  action, long  x, long  y, long  numberOfClicks)
{
    if(this->HaveDownTransition)
	switch(action){
	    case view_RightUp:
		this->HaveDownTransition=FALSE;
		/* fall through */
	    case view_RightMovement:
		this->newX=x-this->distX;
		this->newY=y-this->distY;
		break;
	    case view_LeftUp:
		this->HaveDownTransition=FALSE;
		this->newX=x;
		this->newY=y;
		break;
	    case view_LeftMovement:
		/* do nothing */
		break;
	    default:
		/* re-synchronize mouse */
		this->HaveDownTransition=FALSE;
	}

    if(!this->HaveDownTransition)
	switch(action){
	    case view_RightDown:
		this->distX=x-this->x;
		this->distY=y-this->y;
		/* fall through */
	    case view_LeftDown:
		this->HaveDownTransition=TRUE;
		(this)->WantInputFocus(this);
		break;
	    default:
		break;
	}

    (this)->WantUpdate(this);

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
    struct rectangle myVisualRect;

    (hwv)->GetVisualBounds(&myVisualRect);
    hwv->newX = rectangle_Left(&myVisualRect) + rectangle_Width(&myVisualRect) / 2;
    hwv->newY = rectangle_Top(&myVisualRect) + rectangle_Height(&myVisualRect) / 2;
    (hwv)->WantUpdate( hwv);
}


static void Invert(class helloworldview  *hwv, long  rock)
{
    hwv->newBlackOnWhite = ! hwv->newBlackOnWhite;
    (hwv)->WantUpdate( hwv);
}


static struct bind_Description helloworldviewBindings[]={
    {"helloworld-center", "\003",0, "Hello World,Center",0,0, (proctable_fptr)Center, "Center the helloworld string."},
    {"helloworld-invert", "\011",0, "Hello World,Invert",0,0, (proctable_fptr)Invert, "Invert the helloworld string."},
    NULL
};

boolean helloworldview::InitializeClass()
    {
    helloworldviewMenulist=new menulist;
    helloworldviewKeymap=new keymap;
    bind::BindList(helloworldviewBindings, helloworldviewKeymap,helloworldviewMenulist,&helloworldview_ATKregistry_ );
    return TRUE;
}
