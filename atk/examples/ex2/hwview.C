/* 
	Copyright IBM Corporation 1988,1991 - All Rights Reserved
	Copyright Carnegie Mellon University 1996
	For full copyright information see: <src or dest>/doc/COPYRITE

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

#include <andrewos.h>
ATK_IMPL("hwview.H")
#include "graphic.H"
#include "hwview.H"

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
	graphic_BETWEENTOPANDBASELINE | graphic_BETWEENLEFTANDRIGHT);    
}

    
void helloworldview::Update()
    {
    if (this->newX != this->x ||  this->newY != this->y) {
        (this)->SetTransferMode(graphic_INVERT);

        (this)->MoveTo( this->x , this->y);
        (this)->DrawString( "hello world", graphic_BETWEENTOPANDBASELINE | graphic_BETWEENLEFTANDRIGHT);

        this->x = this->newX;
        this->y = this->newY;

        (this)->MoveTo( this->x , this->y);
        (this)->DrawString( "hello world", graphic_BETWEENTOPANDBASELINE | graphic_BETWEENLEFTANDRIGHT);
   }
}


class view *helloworldview::Hit(enum view_MouseAction  action, long  x, long  y, long  numberOfClicks)
                    {
    if (action == view_LeftDown)  {  
        this->HaveDownTransition = TRUE;
    }
    else if (this ->HaveDownTransition) {
	if (action == view_LeftUp)  {
	    this->newX = x;
	    this->newY = y;
	    this->HaveDownTransition = FALSE;
	    (this)->WantUpdate( this);
	}
    }
    return (class view *) this;
}
