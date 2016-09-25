/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("lpair.H")
#include <lpair.H>

#include <im.H>
#include <graphic.H>
#include <view.H>
#include <cursor.H>
#include <rect.h>


/* In theory, BARWIDTH could be nuked, but it should be optimized out anyway and it may be useful "someday." */
#define BARWIDTH 0

/* 
 * <<Wed Apr 12 14:48:48 1989>>
 * Actually there is at least one BARWIDTH use (in __Hit) that would 
 * be wrong if BARWIDTH were non-0.  Be careful if today is "someday".
 * 
 * 						-t
 */

#define GRAVITY 1

/* Forward Declarations */





/* For use in ComputeSizes below. */
#define min(a, b) ((a < b) ? a : b)
#define max(a, b) ((a < b) ? b : a)

/* Basically, the only reason this routine exists is because the FullUpdate signature does not use rectangles. This routine expects its redrawRectangle argument to be valid know matter what the type argument is. The type arg is just passed through to the children that need to be redrawn. All in all, this, Update, and FullUpdate can probably be simplified. -Z- */

ATKdefineRegistry(lpair, view, NULL);
static void DoFullUpdate(class lpair  *self, enum view_UpdateType  type, struct rectangle  *redrawRectangle);
static void lpair_ComputeSizesFromTotal (class lpair  *l, int	 totalsize);
static void lpair_ComputeSizes (class lpair  *l);
static void lpair_ResetDimensions(class lpair  *self);


static void DoFullUpdate(class lpair  *self, enum view_UpdateType  type, struct rectangle  *redrawRectangle)
{
	/*  All this code needs changed */
	int	x, y;
	int	offset; /* Used to get the bar line in the right place. */
	class view *leftTopObject = self->obj[0];
	class view *rightBottomObject = self->obj[1];
	struct rectangle childRectangle;

	self->needsfull = 0;

	lpair_ComputeSizes(self);
	lpair_ResetDimensions(self);	/* reset the child lpair sizes */
	x = 0; 
	y = 0;
	if (self->typex == lpair_VERTICAL)
		x += self->objcvt[0];
	else
		y += self->objcvt[0];

	(self)->SetTransferMode( graphic_BLACK);
	offset = (self->movable) ? BARWIDTH : 0; /* If not movable, don't put extra space around bar. */
	if (self->typex == lpair_VERTICAL) {
		(self)->MoveTo( x + offset, 0);
		(self)->DrawLineTo( x + offset, (self)->GetLogicalHeight()/*-1*/);
	} else {
		(self)->MoveTo( 0, y + offset);
		(self)->DrawLineTo( (self)->GetLogicalWidth()/*-1*/, y + offset);
	}

	if (leftTopObject != NULL) {
		(leftTopObject)->GetEnclosedBounds( &childRectangle);
		rectangle_IntersectRect(&childRectangle, &childRectangle, redrawRectangle);
		if (!rectangle_IsEmptyRect(&childRectangle)) {
			/* lpair_RetractViewCursors(self, leftTopObject); */
			(leftTopObject)->FullUpdate( type,
			    (leftTopObject)->EnclosedXToLocalX(
			    rectangle_Left(&childRectangle)),
			    (leftTopObject)->EnclosedYToLocalY(
			    rectangle_Top(&childRectangle)),
			    rectangle_Width(&childRectangle),
			    rectangle_Height(&childRectangle));
		}
	}

	if (rightBottomObject != NULL) {
		(rightBottomObject)->GetEnclosedBounds( &childRectangle);
		rectangle_IntersectRect(&childRectangle, &childRectangle, redrawRectangle);
		if (!rectangle_IsEmptyRect(&childRectangle)) {
			/* lpair_RetractViewCursors(self, rightBottomObject); */
			(rightBottomObject)->FullUpdate( type,
			    (rightBottomObject)->EnclosedXToLocalX(
			    rectangle_Left(&childRectangle)),
			    (rightBottomObject)->EnclosedYToLocalY(
			    rectangle_Top(&childRectangle)),
			    rectangle_Width(&childRectangle),
			    rectangle_Height(&childRectangle));
		}
	}

	if (self->movable) {
		struct rectangle cursorRegion;

		if (self->typex == lpair_VERTICAL)
			rectangle_SetRectSize(&cursorRegion,
			    x - GRAVITY,
			    0,
			    2 * (BARWIDTH + GRAVITY) + 1,
			    (self)->GetLogicalHeight());
		else
			rectangle_SetRectSize(&cursorRegion,
			    0,
			    y - GRAVITY,
			    (self)->GetLogicalWidth(),
			    2 * (BARWIDTH + GRAVITY) + 1);

		(self)->PostCursor( &cursorRegion, self->cursor);
	}
}


void lpair::Update()
{
	if (this->needsfull) {

		struct rectangle redrawRectangle;
		class graphic *pat;

		(this)->SetTransferMode( graphic_COPY);
		(this)->GetLogicalBounds( &redrawRectangle);
		if (!(this->needsfull & 1)) {
			if (this->typex == lpair_HORIZONTAL)
				rectangle_SetTop(&redrawRectangle, this->objcvt[0] + BARWIDTH);
			else
				rectangle_SetLeft(&redrawRectangle, this->objcvt[0] + BARWIDTH);
		}
		if (!(this->needsfull & 2)) {
			if (this->typex == lpair_HORIZONTAL)
				rectangle_SetBottom(&redrawRectangle, this->objcvt[0] - BARWIDTH);
			else
				rectangle_SetRight(&redrawRectangle, this->objcvt[0] - BARWIDTH);
		}
		pat = (this)->WhitePattern();
		(this)->FillRect( &redrawRectangle, pat);
		/* I intentionally pass in view_FullRedraw here knowing that the DoFullUpdate
 * will still take the rectangle into account.
 */
		DoFullUpdate(this, view_FullRedraw, &redrawRectangle);
	}
}


void lpair::FullUpdate(enum view_UpdateType  type, long	 left, long	 top, long	 width, long	 height)
{

	struct rectangle redrawRectangle;

	/* This really shouldn't be neccesary since the other redraw types should be required to fill in an appropriate recangle. However there are a few places (drawtxtv.c) where view_FullRedraw and view_Remove types get sent with 0 by 0 rectangles. */
	switch (type) {
	case view_PartialRedraw:
	case view_LastPartialRedraw:
		rectangle_SetRectSize(&redrawRectangle, left, top, width, height);
		break;
	case view_MoveNoRedraw:
		/* Need to reinsert views so they get correct coordinate system. */
		lpair_ResetDimensions(this);
		/* Intentional fall through. */
	case view_Remove:
		if (this->obj[0] != NULL)
			(this->obj[0])->FullUpdate( type, 0, 0, 0, 0);
		if (this->obj[1] != NULL)
			(this->obj[1])->FullUpdate( type, 0, 0, 0, 0);
		return;
		break;
	default:
		(this)->GetVisualBounds( &redrawRectangle);
		break;
	}
	DoFullUpdate(this, type, &redrawRectangle);
}


class view *lpair::Hit(enum view_MouseAction  action, long	 x , long	 y, long	 numberOfClicks)
{

	long	dim;

	if (this->typex == lpair_VERTICAL) {
		dim = x;
	} else {
		dim = y;
	}

	if (this->movable && 
	    (action == view_RightDown || action == view_LeftDown) && 
	    (ABS(dim - this->objcvt[0] - BARWIDTH) <= BARWIDTH + GRAVITY)) {
		this->lasthit = dim;
		(((class view *) this)->GetIM())->SetWindowCursor( this->cursor);
		this->ismoving = TRUE;
		return((class view *) this); /* Our hit, return us. */
	}

	if (this->ismoving) {
		if ((action == view_RightUp || action == view_LeftUp)) {
			if (!this->maybeZero) {
				if (this->typex == lpair_VERTICAL) {
					x = max(x, BARWIDTH + GRAVITY);
					x = min(x,
					    (this)->GetLogicalWidth() - BARWIDTH - GRAVITY);
				} 
				else {
					y = max(y, BARWIDTH + GRAVITY);
					y = min(y,
					    (this)->GetLogicalHeight() - BARWIDTH - GRAVITY);
				}
			}

			if ((abs(this->lasthit - dim) > BARWIDTH + GRAVITY)) {
				if (this->typex == lpair_VERTICAL)
					if (this->sizeform == lpair_PERCENTAGE)
						this->objsize[1] = 100 - 
						    ((100 * x) / (this)->GetLogicalWidth());
					else if (this->sizeform == lpair_BOTTOMFIXED)
						this->objsize[1] = (this)->GetLogicalWidth() - x;
					else
						this->objsize[0] = x;
				else if (this->sizeform == lpair_PERCENTAGE)
					this->objsize[1] = 100 - 
					    ((100 * y) / (this)->GetLogicalHeight());
				else if (this->sizeform == lpair_BOTTOMFIXED)
					this->objsize[1] = (this)->GetLogicalHeight() - y;
				else
					this->objsize[0] = y;
				if (this->sizeform == lpair_PERCENTAGE) {
					if (this->objsize[1] > 100)
						this->objsize[1] = 100;
					else if (this->objsize[1] < 0)
						this->objsize[1] = 0;
				}
				this->needsfull = 3;
				(this)->WantUpdate( this);
			}
			((this)->GetIM())->SetWindowCursor( NULL);
			this->ismoving = FALSE;
		}
		return((class view *) this);
	}
	if (this->typex == lpair_VERTICAL)
		if (x < this->objcvt[0]) {
			if (this->obj[0] != NULL)
				return (this->obj[0])->Hit( action, (this->obj[0])->EnclosedXToLocalX( x),
				    (this->obj[0])->EnclosedYToLocalY( y), numberOfClicks);
		}
		else {
			if (this->obj[1] != NULL)
				return (this->obj[1])->Hit( action, (this->obj[1])->EnclosedXToLocalX( x),
				    (this->obj[1])->EnclosedYToLocalY( y), numberOfClicks);
		}
	else if (y < this->objcvt[0]) {
		if (this->obj[0] != NULL)
			return (this->obj[0])->Hit( action, (this->obj[0])->EnclosedXToLocalX( x),
			    (this->obj[0])->EnclosedYToLocalY( y), numberOfClicks);
	} else {
		if (this->obj[1] != NULL)
			return (this->obj[1])->Hit( action, (this->obj[1])->EnclosedXToLocalX( x),
			    (this->obj[1])->EnclosedYToLocalY( y), numberOfClicks);
	}
	return NULL; /* Catches the case where one of the lpair's views is NULL. */
}


/* This routineis special to prevent children from getting updates before a
 * FullUpdate. 
*/
void lpair::WantUpdate(class view  *requestor)
{

	/* If we are about to FullUpdate the view requesting an update, throw away the
 * request. This prevents views from getting updates before FullUpdates.
 */
	if (this->parent != NULL &&
	    !(((this->needsfull & 1) && this->obj[0] != NULL &&
	       (requestor)->IsAncestor( this->obj[0])) ||
	      ((this->needsfull & 2) && this->obj[1] != NULL &&
		  (requestor)->IsAncestor( this->obj[1]))))
		(this->parent)->WantUpdate( requestor);
}


#define STARTHEIGHT 256
view_DSattributes lpair::DesiredSize(long	 width , long	 height, enum view_DSpass  pass, long	 *desiredwidth , long	 *desiredheight)
{
	long	d0, d1, c0, c1;
	c0 = this->objcvt[0];
	c1 = this->objcvt[1];
	if (this->obj[0] && this->obj[1]) {
		if (this->typex == lpair_VERTICAL) {
			if (pass != view_HeightSet) {
				lpair_ComputeSizesFromTotal (this, width);
				(this->obj[0])->DesiredSize( this->objcvt[0], height, view_WidthSet, desiredwidth, &d0);
				(this->obj[1])->DesiredSize( this->objcvt[1], height, view_WidthSet, desiredwidth, &d1);
				*desiredwidth = width;
				this->objcvt[0] = c0;
				this->objcvt[1] = c1;
				if (d1 > 2048) {
					if (d0 > 2048) {
						*desiredheight = STARTHEIGHT;
						return(view_Fixed);
					}
					*desiredheight = d0;
					return(view_Fixed);
				}
				if (d0 > 2048) {
					*desiredheight = d1;
					return(view_Fixed);
				}
				*desiredheight = max(d0, d1);
				return(view_Fixed);
			}
		} else {
			if (pass != view_WidthSet) {
				lpair_ComputeSizesFromTotal (this, height);
				(this->obj[0])->DesiredSize( width, this->objcvt[0], view_HeightSet, &d0, desiredheight);
				(this->obj[1])->DesiredSize( width, this->objcvt[1], view_HeightSet, &d1, desiredheight);
				this->objcvt[0] = c0;
				this->objcvt[1] = c1;
				*desiredheight = (height > 2048) ? STARTHEIGHT : height;
				*desiredwidth = max(d0, d1);
				return(view_Fixed);
			}
		}
	}
	*desiredwidth = width;
	*desiredheight = (height > 2048) ? STARTHEIGHT : height;
	return(view_Fixed);
}


static void lpair_ComputeSizesFromTotal (class lpair  *l, int	 totalsize)
{
	int	i = 0;

	if (l->movable)
		totalsize -= 2 * BARWIDTH + 1;		/* If movable allocate an area for move mouse hits. */
	else
		totalsize -= 1;	/* Make room for the bar in the middle. */

	switch (l->sizeform) { /* Allocate space for the 'hard' allocated view. */
	case lpair_PERCENTAGE:
		i = 1;
		l->objcvt[i] = (l->objsize[i] * totalsize) / 100;
		break;
	case lpair_BOTTOMFIXED:
		i = 1;
		l->objcvt[i] = min(totalsize, l->objsize[i]);
		break;
	case lpair_TOPFIXED:
		i = 0;
		l->objcvt[i] = min(totalsize, l->objsize[i]);
		break;
	}
	/* Give the rest to the other view. */
	l->objcvt[1-i] = totalsize - l->objcvt[i];
}


static void lpair_ComputeSizes (class lpair  *l)
{

	int	totalsize;

	/* Find out how much space the two must share. */
	if (l->typex == lpair_VERTICAL)
		totalsize = (l)->GetLogicalWidth();
	else
		totalsize = (l)->GetLogicalHeight();

	/* See if we don't have any space -- actually we should be testing to see if children can fit at all, but for, now a simple test for zero */
	if (totalsize == 0) {
		l->objcvt[0] = l->objcvt[1] = 0;
		return;
	}
	lpair_ComputeSizesFromTotal (l, totalsize);
}


static void lpair_ResetDimensions(class lpair  *self)
{

	int	i, x, y;
	class view *child;
	struct rectangle enclosingRect;

	x = 0; 
	y = 0;
	for (i = 0; i < 2; i++) { /* Loop over the two halves of the lpair. */
		child = (class view *) self->obj[i];
		if (child != NULL) {
			if (self->typex == lpair_VERTICAL) {
				rectangle_SetRectSize(&enclosingRect, x, y, self->objcvt[i], (self)->GetLogicalHeight());
				(child)->InsertView( self, &enclosingRect);
				x += self->objcvt[i] + 2 * BARWIDTH + 1;
			}
			else {
				rectangle_SetRectSize(&enclosingRect, x, y, (self)->GetLogicalWidth(),  self->objcvt[i]);
				(child)->InsertView( self, &enclosingRect);
				y += self->objcvt[i] + 2 * BARWIDTH + 1;
			}
		}
	}
}


lpair::lpair ()
{

	this->obj[0] = NULL;
	this->obj[1] = NULL;
	this->objsize[0] = this->objsize[1] = 0;
	this->typex = lpair_HORIZONTAL;
	this->ismoving = FALSE;
	this->movable = FALSE;
	this->lasthit = 0;
	this->cursor = cursor::Create(this);
	(this->cursor)->SetStandard( Cursor_HorizontalBars);
	this->needsfull = 0;
	this->maybeZero = FALSE;
	this->recsearchvalid = FALSE;

	THROWONFAILURE( TRUE);
}


lpair::~lpair()
{

	if (this->obj[0] != NULL)
		(this->obj[0])->UnlinkTree();
	if (this->obj[1] != NULL)
		(this->obj[1])->UnlinkTree();
	delete this->cursor;
}


class lpair *lpair::Create (class view  *l1 , class view  *l2, long	 x)
{

	class lpair *newl;

	newl = new lpair;
	(newl)->Init( l1, l2, x);
	return newl;
}


void lpair::Init(class view  *l1 , class view  *l2, long	 x)
{

	if (x < 0)
		(this)->VSplit( l1, l2, -x, 1);
	else
		(this)->VFixed( l1, l2, x, 1);
	this->needsfull = 3;
}


class lpair *lpair::SetUp(class view  *l1 , class view  *l2 /* This lpairs prospective children. */, int	 bsize , int	 porf , int	 vorh /* Size of bottom (right) view area, size is percent or pixels, vertical or horizontal. */, boolean moveable)
{

	(this)->SetNth( 0, l1);
	(this)->SetNth( 1, l2);

	if (porf == lpair_TOPFIXED) {
		this->objsize[0] = bsize;
		this->objsize[1] = 0;
	} else {
		this->objsize[0] = 0;
		this->objsize[1] = bsize;
	}

	this->sizeform = porf;
	if (this->typex != vorh) {
		if ((this->typex = vorh) == lpair_VERTICAL)
			(this->cursor)->SetStandard( Cursor_VerticalBars);
		else
			(this->cursor)->SetStandard( Cursor_HorizontalBars);
	}
	this->movable = moveable;
	lpair_ComputeSizes(this);
	if (this->objcvt[0] == 0 || this->objcvt[1] == 0)
		this->maybeZero = TRUE;
	return this;
}


class lpair *lpair::VFixed (class view  *l1 , class view  *l2, int	 bsize, boolean moveable)
{

	(this)->SetUp( l1, l2, bsize, lpair_BOTTOMFIXED, lpair_HORIZONTAL, moveable);
	return this;
}


class lpair *lpair::VTFixed (class view  *l1 , class view  *l2, int	 bsize, boolean moveable)
{

	(this)->SetUp( l1, l2, bsize, lpair_TOPFIXED, lpair_HORIZONTAL, moveable);
	return this;
}


class lpair *lpair::VSplit(class view  *l1 , class view  *l2, int	 pct, boolean moveable)
{
	(this)->SetUp( l1, l2, pct, lpair_PERCENTAGE, lpair_HORIZONTAL, moveable);
	return this;
}


class lpair *lpair::HFixed (class view  *l1 , class view  *l2, int	 bsize, boolean moveable)
{

	(this)->SetUp( l1, l2, bsize, lpair_BOTTOMFIXED, lpair_VERTICAL, moveable);
	return this;
}


class lpair *lpair::HTFixed (class view  *l1 , class view  *l2, int	 bsize, boolean moveable)
{

	(this)->SetUp( l1, l2, bsize, lpair_TOPFIXED, lpair_VERTICAL, moveable);
	return this;
}


class lpair *lpair::HSplit(class view  *l1 , class view  *l2, int	 pct, boolean moveable)
{
	(this)->SetUp( l1, l2, pct, lpair_PERCENTAGE, lpair_VERTICAL, moveable);
	return this;
}


void lpair::SetLPState(int	 porf , int	 vorh , int	 movable /* Percent or fixed, Vertical or Horizontal, movable bar. * */)
{

	if (porf == lpair_PERCENTAGE || porf == lpair_BOTTOMFIXED || porf == lpair_TOPFIXED)
		this->sizeform = porf;
	if (vorh == lpair_VERTICAL || vorh == lpair_HORIZONTAL)
		if (this->typex != vorh) {
			if ((this->typex = vorh) == lpair_VERTICAL)
				(this->cursor)->SetStandard( Cursor_VerticalBars);
			else
				(this->cursor)->SetStandard( Cursor_HorizontalBars);
		}
	if (movable != lpair_NOCHANGE)
		this->movable = movable;
	this->needsfull = 3;
	(this)->WantUpdate( this);
}


void lpair::GetLPState(int	 *porf , int	 *vorh , int	 *movable)
{

	*porf = this->sizeform;
	*vorh = this->typex;
	*movable = this->movable;
}


void lpair::SetMovable(int	 i)
{

	this->movable = i;
}


class view *lpair::GetNth(int	 ai)
{

	return this->obj[ai];
}


void lpair::SetNth(int	 ai, class view  *x)
{

	if (ai >= 0 && ai <= 1 && this->obj[ai] != x) {
		int	other = 1 -ai;

		if (this->obj[other] == x) {
			(this)->SetNth( other, NULL);
		}

		if (this->obj[ai] != NULL)
			(this->obj[ai])->UnlinkTree();

		if ((this->obj[ai] = x) != NULL)
			(this->obj[ai])->LinkTree( this);

		this->needsfull |= (1 << ai);
		(this)->WantUpdate( this);
	}
}


void lpair::LinkTree(class view  *parent)
{

	(this)->view::LinkTree( parent);
	if (this->obj[0] != NULL)
		(this->obj[0])->LinkTree( this);
	if (this->obj[1] != NULL)
		(this->obj[1])->LinkTree( this);
}


void lpair::InitChildren()
{
	if (this->obj[0] != NULL)
		(this->obj[0])->InitChildren();
	if (this->obj[1] != NULL)
		(this->obj[1])->InitChildren();
}


boolean lpair::RecSearch(struct SearchPattern *pat, boolean toplevel)
{
    class view *v;
    int ai;

    for (ai=0; ai<2; ai++) {
	v = this->GetNth(ai);
	if (v) {
	    /*printf("### lpair(%s) %x: checking child %d, |%s|\n", this->GetTypeName(), this, ai, v->GetTypeName());*/
	    if (v->RecSearch(pat, FALSE)) {
		this->recsearchchild = ai;
		this->recsearchvalid = TRUE;
		return TRUE;
	    }
	}
    }

    this->recsearchvalid = FALSE;
    return FALSE;
}

boolean lpair::RecSrchResume(struct SearchPattern *pat)
{
    class view *v;
    int ai;

    if (!this->recsearchvalid)
	return FALSE;

    /* recsearchchild is the child that got the last success */
    ai = this->recsearchchild;
    v = this->GetNth(ai);
    /*printf("### lpair(%s) %x: rechecking child %d, |%s|\n", this->GetTypeName(), this, ai, v->GetTypeName());*/
    if (this->GetNth(ai) && this->GetNth(ai)->RecSrchResume(pat)) {
	/* pos and child stay the same */
	this->recsearchvalid = TRUE;
	return TRUE;
    }

    /* ok, that child ran out of matches. restart right after it. */

    for (ai++; ai<2; ai++) {
	v = this->GetNth(ai);
	if (v) {
	    /*printf("### lpair(%s) %x: checking child %d, |%s|\n", this->GetTypeName(), this, ai, v->GetTypeName());*/
	    if (v->RecSearch(pat, FALSE)) {
		this->recsearchchild = ai;
		this->recsearchvalid = TRUE;
		return TRUE;
	    }
	}
    }

    this->recsearchvalid = FALSE;
    return FALSE;
}

boolean lpair::RecSrchReplace(class dataobject *srctext, long srcpos, long srclen)
{
    class view *v;

    if (!this->recsearchvalid)
	return FALSE;

    v = this->GetNth(this->recsearchchild);

    return v->RecSrchReplace(srctext, srcpos, srclen);
}

void lpair::RecSrchExpose(const struct rectangle &logical,struct rectangle &hit)
{
    class view *v;

    if (!this->recsearchvalid)
	return;

    v = this->GetNth(this->recsearchchild);
    lpair_ComputeSizesFromTotal(this, (typex==lpair_VERTICAL)?logical.width:logical.height);

    int	i, x, y;
    class view *child;
    struct rectangle enclosingRect;

    x = 0; 
    y = 0;
    for (i = 0; i < 2; i++) { /* Loop over the two halves of the lpair. */
     child = (class view *) obj[i];
     if (child != NULL) {
	 if (typex == lpair_VERTICAL) {
	     rectangle_SetRectSize(&enclosingRect, x, y, objcvt[i], logical.height);
	     if(child==v) {
		 child->LinkTree(this);
		 struct rectangle oldrect;
		 child->GetEnclosedBounds(&oldrect);
		 child->InsertView(this, &oldrect);
		 child->RecSrchExpose(enclosingRect,hit);
	     }
	     x += objcvt[i] + 2 * BARWIDTH + 1;
	 }
	 else {
	     rectangle_SetRectSize(&enclosingRect, x, y, logical.width,  objcvt[i]);
	     if(child==v) {
		 child->LinkTree(this);
		 struct rectangle oldrect;
		 child->GetEnclosedBounds(&oldrect);
		 child->InsertView(this, &oldrect);
		 (child)->RecSrchExpose(enclosingRect, hit);
	     }
	     y += objcvt[i] + 2 * BARWIDTH + 1;
	 }
     }
    }
}

