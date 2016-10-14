/* 
	Copyright IBM Corporation 1988,1991 - All Rights Reserved
	Copyright Carnegie Mellon University 1996
	For full copyright information see: <src or dest>/doc/COPYRITE
*/

#include <andrewos.h>
ATK_IMPL("hwview.H")
#include "graphic.H"
#include "hwview.H"

#define POSUNDEF -1


ATKdefineRegistryNoInit(helloworldview, view);

helloworldview::helloworldview()
        {
    this->x = POSUNDEF;
    this->y = POSUNDEF;
    this->HaveDownTransition = FALSE;
    THROWONFAILURE( TRUE);
}


void helloworldview::FullUpdate(enum view::UpdateType  type, long  left, long  top, long  width, long  height)
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


class view *helloworldview::Hit(enum view::MouseAction  action, long  x, long  y, long  numberOfClicks)
                    {
    if (action == view::LeftDown)  {  
        this->HaveDownTransition = TRUE;
    }
    else if (this ->HaveDownTransition) {
	if (action == view::LeftUp)  {
	    this->newX = x;
	    this->newY = y;
	    this->HaveDownTransition = FALSE;
	    (this)->WantUpdate( this);
	}
    }
    return (class view *) this;
}
