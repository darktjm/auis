/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
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
ATK_IMPL("rectlist.H")
#include <rectlist.H>

#include <graphic.H>
#include <view.H>

struct rlist_rectangle {
    int bottom, top, left,right;
};

#define MAXRECTANGLES 20

/* SCANAGAIN should either be set to -1 or to (oldnum-1).  It should be set to -1 if the rectangles added to the OldList stack ontop of each other. */

#define SCANAGAIN (-1)

static struct rlist_rectangle OldList[MAXRECTANGLES];
static struct rlist_rectangle NewList[MAXRECTANGLES];
static int EndScan = 0;
static int EndOld = 0;
static int EndNew = 0;



ATKdefineRegistry(rectlist, ATK, NULL);
static void Intersect(int  oldnum , int  newnum  )
{
    int ob, ot, ol, Or, nb, nt, nl, nr, ib, it;
    
    ob = OldList[oldnum].bottom;
    ot = OldList[oldnum].top;
    ol = OldList[oldnum].left;
    Or = OldList[oldnum].right;
    nb = NewList[newnum].bottom;
    nt = NewList[newnum].top;
    nl = NewList[newnum].left;
    nr = NewList[newnum].right;

    if (nb <= ot || nt >= ob || nr <= ol || nl >= Or) return;
    if (nb <= ob)  {
	OldList[oldnum].top = nb;
	if (nt <= ot)  {
	    it = ot;
	}
	else  {
	    rectlist::AddOldRectangle(nt, ot, ol, Or);
	    it = nt;
	}
	NewList[newnum].bottom = it;
	ib = nb;
    }
    else  {
	NewList[newnum].top = ob;
	if (nt < ot)  {
	    rectlist::AddNewRectangle(ot, nt, nl, nr, oldnum + 1);
	    it = ot;
	}
	else  {
	    it = nt;
	}
	OldList[oldnum].bottom = it;
	ib = ob;
    }
    if (nl < ol)  {
	rectlist::AddNewRectangle(ib, it, nl, ol, SCANAGAIN);
    }
    else if (nl > ol)  {
	rectlist::AddOldRectangle(ib, it, ol, nl);
    }
    if (nr < Or)  {
	rectlist::AddOldRectangle(ib, it, nr, Or);
    }
    else if (nr > Or)  {
	rectlist::AddNewRectangle(ib, it, Or, nr, SCANAGAIN);
    }
}

static struct rectangle bounds;

void rectlist::ResetList()
{
    EndScan = 0;
    EndOld = 0;
    EndNew = 0;
    rectangle_EmptyRect(&bounds);
}

struct rectangle &rectlist::Bounds() {
    return bounds;
}

void rectlist::AddOldRectangle(long  bottom , long  top , long  left , long  right  )
{
    OldList[EndOld].bottom = bottom;
    OldList[EndOld].top = top;
    OldList[EndOld].left = left;
    OldList[EndOld++].right = right;
}

void rectlist::AddNewRectangle(long  bottom , long  top , long  left , long  right , long  startscan ) 
{
    /* This routine adds a new rectangle to the newlist and then intersects it with the elements in the OldList.  If startscan is -1 the intersection i not done.  Otherwise it gives the location in the OldList to start doing the intersection. */

    int i;
    int newnum = EndNew++;
    if(rectangle_IsEmptyRect(&bounds)) {
	rectangle_SetLeft(&bounds,left);
	rectangle_SetTop(&bounds,top);
	rectangle_SetBottom(&bounds,bottom);
	rectangle_SetRight(&bounds,right);
    } else {
	if(rectangle_Left(&bounds)>left) rectangle_SetLeft(&bounds,left);
	if(rectangle_Right(&bounds)<right) rectangle_SetRight(&bounds,right);

	if(rectangle_Top(&bounds)>top) rectangle_SetTop(&bounds,top);
	if(rectangle_Bottom(&bounds)<bottom) rectangle_SetBottom(&bounds,bottom);

    }
    NewList[newnum].bottom = bottom;
    NewList[newnum].top = top;
    NewList[newnum].left = left;
    NewList[newnum].right = right;
    
    if (startscan == -1) return;
    if (startscan == 0) EndScan = EndOld;
    for (i = startscan; i < EndScan && NewList[newnum].bottom != NewList[newnum].top && NewList[newnum].left != NewList[newnum].right; i++)  {
	if (OldList[i].bottom != OldList[i].top && OldList[i].left != OldList[i].right)
	    Intersect(i, newnum);
    }
}

void rectlist::InvertRectangles(class view  *vPtr, spotcolor *spots)
{
    int i;
    struct rectangle invertRect;
    class graphic *pat;

    for (i = 0; i < EndOld; i++)  {
	if (OldList[i].bottom != OldList[i].top && OldList[i].left != OldList[i].right)  {

	    rectangle_SetRectSize(&invertRect, OldList[i].left, OldList[i].top, OldList[i].right - OldList[i].left, OldList[i].bottom - OldList[i].top);
	    pat = (vPtr)->BlackPattern();
	    (vPtr)->FillRect( &invertRect, pat);
	}
    }    
    for (i = 0; i < EndNew; i++)  {
	if (NewList[i].bottom != NewList[i].top && NewList[i].left != NewList[i].right)  {
	    rectangle_SetRectSize(&invertRect, NewList[i].left, NewList[i].top, NewList[i].right - NewList[i].left, NewList[i].bottom - NewList[i].top);
	    pat = (vPtr)->BlackPattern();
	    (vPtr)->FillRect( &invertRect, pat);

	}
    }

    const char *obcolor=NULL;
    long obc1, obc2, obc3;
    const char *ofcolor=NULL;
    long ofc1,ofc2,ofc3;
    boolean reset=FALSE;
    
    for (i = 0; i < EndOld; i++)  {
	if (OldList[i].bottom != OldList[i].top && OldList[i].left != OldList[i].right)  {

	    rectangle_SetRectSize(&invertRect, OldList[i].left, OldList[i].top, OldList[i].right - OldList[i].left, OldList[i].bottom - OldList[i].top);
	    spotcolor *p=spots;
	    while(p) {
		struct rectangle actual, inter;
		actual.left=p->left+p->ox;
		actual.top=p->top+p->oy;
		actual.width=p->width;
		actual.height=p->height;
		rectangle_IntersectRect(&inter, &actual, &invertRect);
		if(!rectangle_IsEmptyRect(&inter)) {
		    if(!reset) {
			vPtr->GetBackgroundColor(&obcolor, &obc1, &obc2, &obc3);
			vPtr->GetForegroundColor(&ofcolor, &ofc1, &ofc2, &ofc3);
			if(ofcolor) {
			    atom *a=atom::Intern(ofcolor);
			    if(a) ofcolor=a->Name();
			}
			if(obcolor) {
			    atom *a=atom::Intern(obcolor);
			    if(a) obcolor=a->Name();
			}
			vPtr->SetTransferMode(graphic_XOR);
			reset=TRUE;
		    }
		    vPtr->SetBackgroundColor(&p->color);
		    long p1, p2, p3;
		    const char *dummy;
		    vPtr->GetBackgroundColor(&dummy, &p1, &p2, &p3);
		    if((p1!=obc1 || p2!=obc2 || p3!=obc3) && (p1!=ofc1 || p2!=ofc2 || p3!=ofc3)) {
			vPtr->FillRect(&inter, NULL);
		    }
		}
		if(p->next) p=p->next;
		else p=p->nextgroup;
	    }
	}
    }
    for (i = 0; i < EndNew; i++)  {
	if (NewList[i].bottom != NewList[i].top && NewList[i].left != NewList[i].right)  {
	    rectangle_SetRectSize(&invertRect, NewList[i].left, NewList[i].top, NewList[i].right - NewList[i].left, NewList[i].bottom - NewList[i].top);
	    spotcolor *p=spots;
	    while(p) {
		struct rectangle actual, inter;
		actual.left=p->left+p->ox;
		actual.top=p->top+p->oy;
		actual.width=p->width;
		actual.height=p->height;
		rectangle_IntersectRect(&inter, &actual, &invertRect);
		if(!rectangle_IsEmptyRect(&inter)) {
		    if(!reset) {
			vPtr->GetBackgroundColor(&obcolor, &obc1, &obc2, &obc3);
			vPtr->GetForegroundColor(&ofcolor, &ofc1, &ofc2, &ofc3);
			if(ofcolor) {
			    atom *a=atom::Intern(ofcolor);
			    if(a) ofcolor=a->Name();
			}
			if(obcolor) {
			    atom *a=atom::Intern(obcolor);
			    if(a) obcolor=a->Name();
			}
			reset=TRUE;
		    }
		    vPtr->SetBackgroundColor(&p->color);
		    long p1, p2, p3;
		    const char *dummy;
		    vPtr->GetBackgroundColor(&dummy, &p1, &p2, &p3);
		    if((p1!=obc1 || p2!=obc2 || p3!=obc3) && (p1!=ofc1 || p2!=ofc2 || p3!=ofc3)) {
			vPtr->FillRect(&inter, NULL);
		    }
		}
		if(p->next) p=p->next;
		else p=p->nextgroup;
	    }
	}
    }
    if(reset) {
	vPtr->SetBackgroundColor(obcolor, obc1, obc2, obc3);
    }
#if 1
    reset=FALSE;
    for (i = 0; i < EndOld; i++)  {
	if (OldList[i].bottom != OldList[i].top && OldList[i].left != OldList[i].right)  {

	    rectangle_SetRectSize(&invertRect, OldList[i].left, OldList[i].top, OldList[i].right - OldList[i].left, OldList[i].bottom - OldList[i].top);
	    spotcolor *p=spots;
	    while(p) {
		struct rectangle actual, inter;
		actual.left=p->left+p->ox;
		actual.top=p->top+p->oy;
		actual.width=p->width;
		actual.height=p->height;
		rectangle_IntersectRect(&inter, &actual, &invertRect);
		if(!rectangle_IsEmptyRect(&inter)) {
		    if(!reset) {
			vPtr->GetBackgroundColor(&obcolor, &obc1, &obc2, &obc3);
			vPtr->GetForegroundColor(&ofcolor, &ofc1, &ofc2, &ofc3);
			if(ofcolor) {
			    atom *a=atom::Intern(ofcolor);
			    if(a) ofcolor=a->Name();
			}
			if(obcolor) {
			    atom *a=atom::Intern(obcolor);
			    if(a) obcolor=a->Name();
			}
			vPtr->SetTransferMode(graphic_XOR);
			reset=TRUE;
		    }
		    vPtr->SetForegroundColor(&p->bgcolor);
		    long p1, p2, p3;
		    const char *dummy;
		    vPtr->GetForegroundColor(&dummy, &p1, &p2, &p3);
                    if((p1!=obc1 || p2!=obc2 || p3!=obc3) && (p1!=ofc1 || p2!=ofc2 || p3!=ofc3)) { vPtr->FillRect(&inter, NULL);
		    }
		}
		if(p->next) p=p->next;
		else p=p->nextgroup;
	    }
	}
    }
    for (i = 0; i < EndNew; i++)  {
	if (NewList[i].bottom != NewList[i].top && NewList[i].left != NewList[i].right)  {
	    rectangle_SetRectSize(&invertRect, NewList[i].left, NewList[i].top, NewList[i].right - NewList[i].left, NewList[i].bottom - NewList[i].top);
	    spotcolor *p=spots;
	    while(p) {
		struct rectangle actual, inter;
		actual.left=p->left+p->ox;
		actual.top=p->top+p->oy;
		actual.width=p->width;
		actual.height=p->height;
		rectangle_IntersectRect(&inter, &actual, &invertRect);
		if(!rectangle_IsEmptyRect(&inter)) {
		    if(!reset) {
			vPtr->GetBackgroundColor(&obcolor, &obc1, &obc2, &obc3);
			vPtr->GetForegroundColor(&ofcolor, &ofc1, &ofc2, &ofc3);
			if(ofcolor) {
			    atom *a=atom::Intern(ofcolor);
			    if(a) ofcolor=a->Name();
			}
			if(obcolor) {
			    atom *a=atom::Intern(obcolor);
			    if(a) obcolor=a->Name();
			}
			reset=TRUE;
		    }
		    vPtr->SetForegroundColor(&p->bgcolor);
		    long p1, p2, p3;
		    const char *dummy;
		    vPtr->GetForegroundColor(&dummy, &p1, &p2, &p3);
		    if((p1!=obc1 || p2!=obc2 || p3!=obc3) && (p1!=ofc1 || p2!=ofc2 || p3!=ofc3)) {
			vPtr->FillRect(&inter, NULL);
		    }
		}
		if(p->next) p=p->next;
		else p=p->nextgroup;
	    }
	}
    }
    if(reset) {
	vPtr->SetForegroundColor(ofcolor, ofc1, ofc2, ofc3);
    }
#endif
}

/* To use this code for the selection region:

ResetRectangleList();
AddOldRectangle(TopRectangle);
AddOldRectangle(MiddleRectangle);
AddOldRectangle(BottomRectangle);
AddNewRectangle(NewTopRectangle,0);
AddNewRectangle(NewMiddleRectangle,0);
AddNewRectangle(NewEndRectangle,0);
InvertRectangles();
 */
