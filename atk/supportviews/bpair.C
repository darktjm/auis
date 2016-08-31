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

/* bpair.c	

	An lpair without the line between the parts
	and without movable or cursors 

*/

#include <andrewos.h>
ATK_IMPL("bpair.H")
#include <graphic.H>
#include <view.H>
#include <cursor.H>

#include <im.H>
#include <rect.h>

#include <lpair.H>
#include <bpair.H>


#define BARWIDTH 0
#define GRAVITY 1

/* Forward Declarations */



/* the following two routines are needed for FullUpdate 
	They are copied from lpair.c, with deletion of the line-between code,
	code for movable borders, and the calls on DesiredSize   */


ATKdefineRegistry(bpair, lpair, NULL);
static void lpair_ComputeSizes (register class lpair  *l);
static void lpair_ResetDimensions(register class lpair  *self);


static void lpair_ComputeSizes (register class lpair  *l)
{

    int totalsize, i = 0;

/* Find out how much space the two must share. */
    if (l->typex == lpair_VERTICAL)
        totalsize = (l)->GetLogicalWidth();
    else
        totalsize = (l)->GetLogicalHeight();

/* See if we don't have any space -- actually we should be testing 
	to see if children can fit at all, but for, now a simple test for zero */
    if (totalsize == 0) {
	l->objcvt[0] = l->objcvt[1] = 0;
	return;
    }

    --totalsize;	/* Make room for the bar in the middle. */

    switch(l->sizeform) { /* Allocate space for the 'hard' allocated view. */
        case lpair_PERCENTAGE:
            i = 1;
            l->objcvt[i] = (l->objsize[i] * totalsize) / 100;
            break;
        case lpair_BOTTOMFIXED:
            i = 1;
            l->objcvt[i] = l->objsize[i];
            break;
        case lpair_TOPFIXED:
            i = 0;
            l->objcvt[i] = l->objsize[i];
            break;
    }
/* Give the rest to the other view. */
    l->objcvt[1-i] = totalsize - l->objcvt[i];
}

static void lpair_ResetDimensions(register class lpair  *self)
{

    register int i, x, y;
    register class  view *child;
    struct rectangle enclosingRect;

    x = 0; y = 0;
    for (i = 0; i < 2; i++) { /* Loop over the two halves of the lpair. */
	child = (class view *)self->obj[i];
	if (self->typex == lpair_VERTICAL) {
	      rectangle_SetRectSize(&enclosingRect,
			x, y, self->objcvt[i],  (self)->GetLogicalHeight());
	      (child)->InsertView( self, &enclosingRect);
	      x += self->objcvt[i] + 2 * BARWIDTH + 1;
	}
	else {
	      rectangle_SetRectSize(&enclosingRect,
			x, y,  (self)->GetLogicalWidth(), self->objcvt[i]);
	      (child)->InsertView( self, &enclosingRect);
	      y += self->objcvt[i] + 2 * BARWIDTH + 1;
	}
    }
}


	bpair::bpair()
		{

    this->movable = FALSE;
    THROWONFAILURE( TRUE);
}
	bpair::~bpair()
		{
}


void 
bpair::FullUpdate(register enum view_UpdateType   type, register long   left , register long   top , register long   width , register long   height)
			{
/* ( the following, including derogatory comment, is copied from lpair.c) */

			/*  All this code needs changed */
    register class view * leftTopObject = this->obj[0];
    register class view * rightBottomObject = this->obj[1];
    struct rectangle r;

    this->movable = FALSE;

    this->needsfull = 0;

    lpair_ComputeSizes((class lpair *)this);
    lpair_ResetDimensions((class lpair *)this);	/* reset the child lpair sizes */

    (leftTopObject)->GetLogicalBounds( &r);
    (leftTopObject)->FullUpdate( type, r.left, r.top, r.width, r.height);
    (rightBottomObject)->GetLogicalBounds( &r);
    (rightBottomObject)->FullUpdate( type, r.left, r.top, r.width, r.height);

	/* deleted the line and cursor code */
}
