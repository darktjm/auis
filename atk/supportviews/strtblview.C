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

/* strtblv.c	

	The view module for the stringtbl dataobject

known problems:
	As the height gets too small, the number of columns reduces, 
		even though the objects could still fit in more columns
	the inset fails if invoked via .ezinit for file.stringtbl
*/

#include <andrewos.h>
ATK_IMPL("strtblview.H")
#include <rect.h>
#include <graphic.H>
#include <im.H>
#include <view.H>
#include <fontdesc.H>

#include <stringtbl.H>
#include <strtblview.H>


/*
#include <menulist.ih>
#include <keymap.ih>
#include <keystate.ih>
*/

/*
static struct menulist  *class_menulist;
static struct keymap  *class_keymap;
static struct keystate  *class_keystate;
*/

#define BORDER 3		/* surrounding space */
#define INTER 3		/* interline spacing */

/* compute the parameters which say how big the largest item is */

ATKdefineRegistry(strtblview, view, NULL);
static void ComputeItemSize(class strtblview  *self);
static void ComputeOrganization (class strtblview  *self, short  width , short  height);
static void iRect(class strtblview    *self, short  i, struct rectangle  *r);
#ifdef NOTUSED
static void AddItem_Command( class strtblview   *self );
#endif /* NOTUSED ? */
static boolean BogusCallFromParent(class strtblview  *self, const char  *where , const char  *msg);
static boolean CheckWindow(class strtblview  *self, const char  *where);
static void  AdjustHighlight(class strtblview  *self);
static void RedrawTable(class strtblview  *self);


static void
ComputeItemSize(class strtblview  *self)
	{
	class stringtbl *st 
			= (class stringtbl *)self->dataobject;
	short n = (st)->NStrings();
	short maxwidth = 0;
	short i;
	class graphic *g;
	class fontdesc *fd;
	struct FontSummary *FS;
	g = (self)->GetDrawable();
	fd = (self)->GetFont();
	FS = (fd)->FontSummary( g);
	for (i = n; i--; ) {
		long w, h;
		(fd)->StringSize( g, (st)->IthString( i), &w, &h);
		if (w > maxwidth) maxwidth = w;
	}
	self->itemheight = FS->maxHeight;
	self->maxwidth = maxwidth;
	self->sizeknown = TRUE;
}

/* ComputeOrganization determines a good number of rows and columns
	for a given width and height of area */
static void
ComputeOrganization (class strtblview  *self, short  width , short  height)
		{
	/* the algorithm is to minimize the difference between inter-column white space
		and inter-row whitespace.  This is done by starting with one row, a situation
		with maximum inter-row whitespace and minimum inter-column spacing.
		Then we add one row at a time until the intercolumn spacing exceeds the
		inter-row spacing.  
			XXX this is a lousy algorithm.  It will compute with two columns for 
			about half the entries.  */
	short r, c;
	class stringtbl *st 
			= (class stringtbl *)self->dataobject;
	short n = (st)->NStrings();
	if ( ! self->sizeknown) 
		ComputeItemSize(self);
	if (n <= 0) {
		self->rows = self->cols = 1;  /* avoid divide by zero */
		return;
	}
	r = 1;	/* start with one row of all elements */
	while (TRUE) {
		short colwhite, rowwhite;
		c = (n+r-1) / r;			/* compute number of columns */
		if (c <= 1) break;
		colwhite = (width - c*self->maxwidth) / c;
		rowwhite = (height - r*self->itemheight) / r;
		if (colwhite > rowwhite) 
			break;
		r ++;
	}
	self->rows = r;
	self->cols = (c<1) ? 1 : c;
}

/* iRect converts its argument rectangle from the 
		entire logical image to the image for the i'th string */
static void
iRect(class strtblview    *self, short  i, struct rectangle  *r)
			{
	short col = i / self->rows;	/* strings run vertically */
	short row = i % self->rows;
	r->height /= self->rows;
	r->top += row * r->height;
	r->width /= self->cols;
	r->left += col * r->width;
	r->width -= 1;
	r->height -= 1;
}
#ifdef NOTUSED
static void
AddItem_Command( class strtblview   *self )
	{

	/* XXX  prompt for string to add */

	(self)->WantUpdate(  self );
}
#endif /* NOTUSED ? */
strtblview::strtblview()
		{
	this->OnScreen = FALSE;
	this->GaveSize = FALSE;
	this->sizeknown = FALSE;
	this->tablechanged = FALSE;
	this->maxused = 0;
	this->ItemHitProc = NULL;
	THROWONFAILURE( TRUE);
}

strtblview::~strtblview()
		{
}

void 
strtblview::Clear()
	{
	class stringtbl *st 
			= (class stringtbl *)this->dataobject;
	this->sizeknown = FALSE;
	this->GaveSize = FALSE;
	this->tablechanged = TRUE;
	this->maxused = 0;
	if (st)
		(st)->Clear();
}

void
strtblview::ObservedChanged(class observable  *dobj, long  status)
			{
	if (status == stringtbl_STRINGSCHANGED) {
		this->sizeknown = FALSE;
		this->GaveSize = FALSE;
		this->tablechanged = TRUE;
		(this)->WantNewSize( this);
	}
	else if (status == observable_OBJECTDESTROYED) 
		return;	/* deleting it */
	(this)->WantUpdate( this);
}


static boolean
BogusCallFromParent(class strtblview  *self, const char  *where , const char  *msg)
		{
	fprintf(stderr, "<strtblview>Bogus call to %s, %s\n", where, msg);
	return FALSE;
}

static boolean
CheckWindow(class strtblview  *self, const char  *where)
		{
	class graphic *g
		= (class graphic *)(self)->GetDrawable();
	if ( ! g) return BogusCallFromParent(self, where, "No Graphic");
	return TRUE;
}

static void 
AdjustHighlight(class strtblview  *self)
	{
	class stringtbl *st 
			= (class stringtbl *)self->dataobject;
	unsigned long which = self->BlackOnes ^ st->highlight;
		/* WARNING: direct use of st->highlight could be a problem if the
		data object has changed asynchronously with the view */
	struct rectangle rectangle, r;
	short i, n;

	(self)->SetTransferMode( graphic_INVERT);
	(self)->GetLogicalBounds( &rectangle);
	n = (st)->NStrings();
	for (i = 0; i < n; i++) {
		if (which & (1<<i)) {
			r = rectangle;
			iRect(self, i, &r);
			(self)->FillOval( &r, self->BlackPattern);
		}
	}
	self->BlackOnes = st->highlight;
}

static void
RedrawTable(class strtblview  *self)
	{
	class stringtbl *st 
			= (class stringtbl *)self->dataobject;
	struct rectangle rectangle, r;
	short i, n;
	
	(self)->GetLogicalBounds( &rectangle);
	if (rectangle.width == 0 || rectangle.height == 0) return;
	if ( ! self->sizeknown)
		ComputeOrganization(self, rectangle.width, rectangle.height);
	(self)->SetTransferMode( graphic_COPY);
	(self)->EraseRect( &rectangle);

	n = (st)->NStrings();
	for (i = 0; i < n; i++) {
		r = rectangle;
		iRect(self, i, &r);
		(self)->MoveTo( 
				r.left + (r.width>>1), 
				r.top + (r.height>>1));
		(self)->DrawString( (st)->IthString( i), 
				graphic_BETWEENLEFTANDRIGHT |
				   graphic_BETWEENTOPANDBASELINE);
	}
	self->tablechanged = FALSE;
	self->BlackOnes = 0;
	AdjustHighlight(self);
}

void 
strtblview::FullUpdate( enum view_UpdateType   type, long   left , long   top , long   width , long   height )
			{
	if (type == view_Remove) {
		this->OnScreen = FALSE;
		return;
	}
	if ( ! CheckWindow(this, "FullUpdate")) return;
	if ((type != view_FullRedraw 
				&& type != view_LastPartialRedraw)
			|| (this)->GetLogicalWidth() == 0 
			|| (this)->GetLogicalHeight() == 0) 
		return;
	this->OnScreen = TRUE;
	if (type == view_FullRedraw) {
	    this->WhitePattern = (this)->view::WhitePattern();
		this->BlackPattern = (this)->view::BlackPattern();
		this->sizeknown = FALSE;
	}
	if ( ! this->GaveSize)
		(this)->WantNewSize( this);
	RedrawTable(this);
}


void 
strtblview::Update( )
	{
	if (! this->OnScreen || ! CheckWindow(this, "Update")) return;
	if (this->tablechanged)
		RedrawTable(this);
	else
		AdjustHighlight(this);
}

class view *
strtblview::Hit(enum view_MouseAction   action, long   x , long   y , long   num_clicks)
			{
	if (action == view_NoMouseEvent)
		return (class view *)this;
	if (! this->OnScreen || ! CheckWindow(this, "Hit")) return NULL;
	if ( action == view_LeftDown || action == view_RightDown ) {
		class stringtbl *st 
				= (class stringtbl *)this->dataobject;
		short i, n = (st)->NStrings();
		struct rectangle r;
		(this)->GetLogicalBounds( &r);
		if (r.width < this->cols) 
			i = 1;
		else
			i = x / (r.width / this->cols) * this->rows
				+ y / (r.height / this->rows);
/*
printf("i %d  for hit at (%d, %d)  in box at (%d,%d) of size %dx%d\n", 
	i, x, y, r.top, r.left, r.width, r.height);  fflush(stdout);
*/
		if (i < n) {
			if (this->ItemHitProc)
				(this->ItemHitProc)(st, this->ItemHitRock, st->accmap[i] + 0L);
			else  {
				/* Implement a ZeroOrOne strategy if no ItemHitProc */
				(st)->ClearBits();
				if (((this->BlackOnes>>i) & 1) == 0) 
					(st)->SetBit( (st)->IthString( i), 1);
			}
			AdjustHighlight(this);
		}
		(this)->WantInputFocus( this);
	}
	return (class view *)this;		/* where to send subsequent hits */
}

view_DSattributes
strtblview::DesiredSize( long  width, long  height, enum view_DSpass  pass, 
				long  *desiredWidth, long  *desiredHeight ) 
						{
	class stringtbl *st 
			= (class stringtbl *)this->dataobject;
	short n = (st)->NStrings();
	if (n > this->maxused)  this->maxused = n;
	if ( ! CheckWindow(this, "DSize")) {
		/* can't compute size without a window (we need a font)
			give dummy values */
		*desiredWidth = 85;
		*desiredHeight = 60;
		return (view_DSattributes)(view_HeightFlexible | view_WidthFlexible);
	}

	/* compute the dimensions of the biggest string */
	if ( ! this->sizeknown ) 
		ComputeItemSize(this);

	/* the following three if sections set self->rows and self->cols */
	if (pass == view_NoSet) {
		if (30<width && width<1000 && 30<height && height<1000)
			ComputeOrganization(this, width, height);
		else if (this->itemheight < 5)
			this->rows = this->cols = 1;
		else {
			/* invalid width or height, choose good ones */
			long r, t = n * this->maxwidth / this->itemheight;
			short i;
			/* square root of t is a good number of rows */
			for (i = 1, r = t>>2; i<6; i++)
				/* iterate newton-raphson */
				r = (r + t/r)>>1;
			if (r < 1) r = 1;
			if (r > this->maxused) r = this->maxused;
			this->rows = r;
			this->cols = (this->maxused + r - 1) / r;
		}
	}
	else if (pass == view_WidthSet) {
		this->cols = (width - 2*BORDER) / (this->maxwidth + INTER);
		if (this->cols < 1) this->cols = 1;
		if (this->cols > this->maxused) this->cols = this->maxused;
		this->rows = (this->maxused + this->cols - 1) / this->cols;
	}
	else /* pass == view_HeightSet */ {
		this->rows = (height - 2*BORDER) / (this->itemheight + INTER);
		if (this->rows < 1) this->rows = 1;
		if (this->rows > this->maxused) this->rows = this->maxused;
		this->cols = (this->maxused + this->rows - 1) / this->rows;
	}
	/* now determine height, width, and return value from rows & cols */
	*desiredHeight = this->rows * (this->itemheight + INTER)
					 + 2*BORDER - INTER;
	*desiredWidth = this->cols * (this->maxwidth + INTER)
					 + 2*BORDER - INTER;
/*
fprintf(stderr, "input %dx%d    rows: %d   cols: %d   output %dx%d\n", 
width, height, self->rows, self->cols, *desiredWidth, *desiredHeight); fflush(stderr);
*/
	this->GaveSize = TRUE;
	{
/* with the following, the compiler complains about incompatible operands to |= 
*		enum view_DSattributes ret = view_HeightLarger | view_WidthLarger;
*		if (self->rows > 1) ret |= view_HeightSmaller;
*		if (self->cols >1) ret |= view_WidthSmaller;
*		return ret;
*/
		if (this->rows > 1 && this->cols > 1) 
			return (view_DSattributes)(view_HeightFlexible | view_WidthFlexible);
		if (this->rows > 1) 
			return (view_DSattributes)(view_HeightFlexible | view_WidthLarger);
		if (this->cols > 1)  
			return (view_DSattributes)(view_HeightLarger | view_WidthFlexible);
		return  (view_DSattributes)(view_HeightLarger | view_WidthLarger);
	}
}

void
strtblview::Print( FILE    *file, const char  	  *processor, const char  	  *format, boolean  	 level )
					{
	
}

/* OneOnly - is a generic hitproc for a string table which guarantees that exactly
	one bit is always on in the table */
void
strtblview::OneOnly(class stringtbl  *st, void *stv, short  accnum)
				{
	if (0 == (st)->GetBitOfEntry( accnum)) {
		(st)->ClearBits();
		(st)->SetBitOfEntry( accnum, 1);
	}
}

/* ZeroOrMany - is a generic hitproc for a string table 
		which allows any number of bits to be on */
void
strtblview::ZeroOrMany(class stringtbl  *st, void *stv, short  accnum)
				{
	(st)->SetBitOfEntry( accnum, ! (st)->GetBitOfEntry( accnum));
}


/* ZeroOrOne - is a generic hitproc for a string table 
		which guarantees that at most one bit is on */
void
strtblview::ZeroOrOne(class stringtbl  *st, void *stv, short  accnum)
				{
	if ((st)->GetBitOfEntry( accnum))
		(st)->ClearBits();
	else {
		(st)->ClearBits();
		(st)->SetBitOfEntry( accnum, 1);
	}
}
