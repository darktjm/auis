/* 
	Copyright IBM Corporation 1988,1991 - All Rights Reserved
	Copyright Carnegie Mellon University 1996
	For full copyright information see: <src or dest>/doc/COPYRITE
*/

#include <andrewos.h>
ATK_IMPL("hwview.H")
#include "hwview.H"
#include "graphic.H"

#define POSUNDEF -1


ATKdefineRegistry(helloworldview, view, NULL);

helloworldview::helloworldview()
{
    this->x = POSUNDEF;
    this->y = POSUNDEF;
    this->HaveDownTransition = FALSE;
    THROWONFAILURE( TRUE);
}


void helloworldview::FullUpdate(enum view_UpdateType  type, long  left, long  top, long  width, long  height)
                        {
    struct rectangle myVisualRect; 

    if (this->x == POSUNDEF)  {
	(this)->GetVisualBounds(&myVisualRect);
	this->x = rectangle_Left(&myVisualRect) + rectangle_Width(&myVisualRect)/2;
	this->y = rectangle_Top(&myVisualRect) + rectangle_Height(&myVisualRect)/2;
    }

    (this)->MoveTo(this->x,this->y);
    (this)->DrawString("hello world",
	graphic::BETWEENTOPANDBASELINE | graphic::BETWEENLEFTANDRIGHT);    
}

    
void helloworldview::Update()
{
   
    if (this->newX != this->x ||  this->newY != this->y) {
        (this)->SetTransferMode(graphic::INVERT);

        (this)->MoveTo( this->x , this->y);
        (this)->DrawString( "hello world", graphic::BETWEENTOPANDBASELINE | graphic::BETWEENLEFTANDRIGHT);

        this->x = this->newX;
        this->y = this->newY;

        (this)->MoveTo( this->x , this->y);
        (this)->DrawString( "hello world", graphic::BETWEENTOPANDBASELINE | graphic::BETWEENLEFTANDRIGHT);
    
    }
}


class view *helloworldview::Hit(enum view_MouseAction  action,long  x,long  y,long  numberOfClicks)
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
