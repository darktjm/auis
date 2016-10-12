/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("wincelview.H")
#include <cel.H>
#include <arbiterview.H>
#include <celview.H>
#include <view.H>
#include <im.H>
#include <rect.h>
#include <wincelview.H>

#define DataObject(A) (A->GetDataObject())
#define Cel(A) ((class cel *) DataObject(A))


ATKdefineRegistry(wincelview, view, NULL);
static void DoUpdate(class wincelview  *self);


class view *wincelview::Hit(enum view::MouseAction  action,long  mousex ,long  mousey ,long  numberOfClicks) 
{
    if(action == view::LeftUp){
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
    (self)->SetTransferMode(graphic::WHITE);
    (self)->EraseRect(&(enclosingRect));
    (self)->SetTransferMode(graphic::INVERT);
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
    if(value == observable::OBJECTDESTROYED){
	if(changed == (class observable *)this->window){
	    this->window = NULL;
	    (this->celviewp)->RemoveObserver(this);
	    (this->celviewp)->Destroy();
	}
	this->celviewp = NULL;
	this->window = NULL;
    }
}
void wincelview::FullUpdate(enum view::UpdateType  type,long  left,long  top,long  width,long  height)
{
    DoUpdate(this);
}
wincelview::wincelview()
{
    this->celviewp = NULL;
    THROWONFAILURE( TRUE);
}

