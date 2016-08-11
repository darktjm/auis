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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/adew/RCS/wincelview.C,v 1.5 1994/11/30 20:42:06 rr2b Stab74 $";
#endif


#include <andrewos.h>
ATK_IMPL("wincelview.H")
#include <cel.H>
#include <arbiterview.H>
#include <celview.H>
#include <view.H>
#include <im.H>
#include <rect.h>
#include <wincelview.H>

#define DataObject(A) (A->dataobject)
#define Cel(A) ((class cel *) DataObject(A))


ATKdefineRegistry(wincelview, view, NULL);
#ifndef NORCSID
#endif
static void DoUpdate(class wincelview  *self);


class view *wincelview::Hit(enum view_MouseAction  action,long  mousex ,long  mousey ,long  numberOfClicks) 
{
    if(action == view_LeftUp){
	if(this->celviewp == NULL)(this)->pushchild();
	else (this)->popchild();
    }	
    return (class view *) this;
}
void wincelview::pushchild()
{
    class arbiterview *abv;
    class cel *pc = Cel(this);
    abv = arbiterview::FindArb(this);
    if(pc && abv != NULL){
	if(this->celviewp == NULL){
	    if((this->celviewp = new celview) == NULL) {
		fprintf(stderr,"Could not allocate enough memory.\n");
		return;
	    }
	    (this->celviewp)->AddObserver(this);
	    (this->celviewp)->SetDataObject(pc);
	    (this->celviewp)->SetRemoteArb(abv);
	    if ((this->window = im::Create(NULL)) != NULL) {
		(this->window)->SetView( this->celviewp);
		(this->window)->AddObserver(this);
	    }
	    else (this->celviewp)->Destroy();
	}
    }
}
void wincelview::popchild()
{
    if(this->celviewp != NULL && this->window != NULL){
	(this->window)->Destroy();
    }
}

static void DoUpdate(class wincelview  *self)
{
    struct rectangle enclosingRect;
    long xsize,ysize;
    struct point pt[5];
    enclosingRect.top = 0; enclosingRect.left = 0;
    enclosingRect.width  = (self)->GetLogicalWidth() -1 ;
    enclosingRect.height = (self)->GetLogicalHeight() -1 ;
    (self)->SetTransferMode(graphic_WHITE);
    (self)->EraseRect(&(enclosingRect));
    (self)->SetTransferMode(graphic_INVERT);
    enclosingRect.left = enclosingRect.width / 3;
    enclosingRect.top =enclosingRect.height / 3;
    enclosingRect.width  =  enclosingRect.width / 2 ;
    enclosingRect.height = enclosingRect.height / 2 ;
    ysize = enclosingRect.height - enclosingRect.top;
    xsize = enclosingRect.width - enclosingRect.left;
    (self)->DrawRect(&(enclosingRect));
    pt[0].x = enclosingRect.left - 1;
    pt[0].y = enclosingRect.height + enclosingRect.top - ysize;
    pt[1].x = pt[0].x - xsize;
    pt[1].y = pt[0].y ;
    pt[2].x = pt[1].x ;
    pt[2].y = pt[0].y  + ysize + ysize;
    pt[3].x = pt[0].x + xsize;
    pt[3].y = pt[2].y;
    pt[4].x = pt[3].x;
    pt[4].y = enclosingRect.top + enclosingRect.height + 1;
    (self)->DrawPath(pt,5);
}
void wincelview::ObservedChanged(class observable  *changed, long  value)
{
    if(value == observable_OBJECTDESTROYED){
	if(changed == (class observable *)this->window){
	    this->window = NULL;
	    (this->celviewp)->RemoveObserver(this);
	    (this->celviewp)->Destroy();
	}
	this->celviewp = NULL;
	this->window = NULL;
    }
}
void wincelview::FullUpdate(enum view_UpdateType  type,long  left,long  top,long  width,long  height)
{
    DoUpdate(this);
}
wincelview::wincelview()
{
    this->celviewp = NULL;
    THROWONFAILURE( TRUE);
}

