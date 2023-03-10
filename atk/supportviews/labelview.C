/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/* labelv.c	

	The view module for the label dataobject


*/

/* sys/types.h in AIX PS2 defines "struct label", causing a type name clash.
   Avoid this by temporarily redefining "label" to be something else in the preprocessor. */
#define label gezornenplatz
#include <andrewos.h> /* strings.h */
ATK_IMPL("labelview.H")
#undef label

#include <graphic.H>
#include <view.H>
#include <fontdesc.H>

#include <menulist.H>
#include <keymap.H>
#include <keystate.H>
#include <bind.H>

#include <im.H>
#include <rect.h>

#include <label.H>
#include <labelview.H>


	
ATKdefineRegistryNoInit(labelview, view);
static boolean BogusCallFromParent(class labelview  *self, const char  *where , const char  *msg);
static boolean CheckWindow(class labelview  *self, const char  *where);
static void RedrawTable(class labelview  *self);


static boolean
BogusCallFromParent(class labelview  *self, const char  *where , const char  *msg)
		{
	fprintf(stderr, "<labelview>Bogus call to %s, %s\n", where, msg);
	return FALSE;
}

	static boolean
CheckWindow(class labelview  *self, const char  *where)
		{
	class graphic *g
		= (class graphic *)(self)->GetDrawable();
	if ( ! g) return BogusCallFromParent(self, where, "No Graphic");
	return TRUE;
}

	labelview::labelview()
		{
	this->GaveSize = FALSE;
	this->embedded = TRUE;
	this->OnScreen = FALSE;
	this->inverted = FALSE;
	this->hitproc = NULL;
	THROWONFAILURE( TRUE);
}

	labelview::~labelview()
		{
}

	void
labelview::ObservedChanged(class observable  *dobj, long  status)
			{
	if (status == label_DATACHANGED) {
		this->GaveSize = FALSE;
		(this)->WantNewSize( this);
	}
	else if (status == observable::OBJECTDESTROYED) 
		return;	/* deleting it */
	(this)->WantUpdate( this);
}

	class view *
labelview::GetApplicationLayer()
	{
	this->embedded = FALSE;
	return this;
}

	static void
RedrawTable(class labelview  *self)
	{
	class label *st 
			= (class label *)self->GetDataObject();
	struct rectangle r;
	int x, y;
	
	(self)->SetTransferMode( graphic::COPY);
	(self)->GetLogicalBounds( &r);
	(self)->FillRect( &r, self->WhitePattern);

	if (st->flags & label_BOXED) {
	    (self)->SetTransferMode( graphic::INVERT);
	    r.top++;
	    r.left++;
	    r.width -= 2;
	    r.height -= 2;
	    (self)->DrawRect(&r);
#if 0
	    int oldwidth = (self)->GetLineWidth();
	    (self)->SetLineWidth( 2);
	    (self)->SetLineWidth( oldwidth);
#endif
	}
	if (st->text) {
		(self)->SetFont( st->font);
		if (*(st->text+1)) {
			/* more than one character */
		        x = (st->flags & label_RIGHTJUST) ? r.left + r.width - 3 :
			    (st->flags & label_HCENTERJUST) ? r.left + (r.width / 2) :
				r.left + 3;
			y = (st->flags & label_BOTTOMJUST) ? r.top + r.height - 3 :
			    (st->flags & label_VCENTERJUST) ? r.top + (r.height / 2):
				r.top + 3;
			(self)->MoveTo( x, y);
			(self)->DrawString( st->text,
				 (st->flags == 0) ? graphic::ATLEFT | graphic::ATTOP :
					            st->flags & ~label_BOXED);
		}
		else {
			/* plot one character.  Use character size rather than font sizes
				because font sizes for icon12 are terrible for most icons 
				center the character in the rectangle */
			struct fontdesc_charInfo charInfo;
			((self)->GetFont())->CharSummary( 
					(self)->GetDrawable(), *st->text, &charInfo);
			(self)->MoveTo( 
				r.left + charInfo.xOriginOffset 
					+ ((r.width-charInfo.width)>>1),
				r.top + charInfo.yOriginOffset
					+ ((r.height-charInfo.height)>>1)
			);
			(self)->DrawString( st->text, graphic::NOMOVEMENT);
		}
	}
	if (self->inverted) {
		(self)->SetTransferMode( graphic::INVERT);
		(self)->FillRect( &r, self->BlackPattern);
	}
}

	void 
labelview::FullUpdate(enum view::UpdateType   type, long   left , long   top , long   width , long   height)
			{
	if (type == view::Remove) {
		this->OnScreen = FALSE;
		return;
	}
	if ( ! CheckWindow(this, "FullUpdate")) return;
	if ((type != view::FullRedraw 
				&& type != view::LastPartialRedraw)
			|| (this)->GetLogicalWidth() == 0 
			|| (this)->GetLogicalHeight() == 0) 
		return;
	this->OnScreen = TRUE;
	if (type == view::FullRedraw  || type == view::LastPartialRedraw) {
	    this->WhitePattern = (this)->view::WhitePattern();
	    this->BlackPattern = (this)->view::BlackPattern();
	}
	if ( ! this->GaveSize)
		(this)->WantNewSize( this);
	RedrawTable(this);
}


	void 
labelview::Update()
	{
	if (! this->OnScreen || ! CheckWindow(this, "Update")) return;
	RedrawTable(this);
}

	class view *
labelview::Hit(enum view::MouseAction   action, long   x , long   y , long   num_clicks)
			{
	if (action == view::NoMouseEvent)
		return (class view *)this;
	if (! this->OnScreen || ! CheckWindow(this, "Hit")) return NULL;
	if (this->hitproc)
		(this->hitproc)(this, action, this->hitrock);
	return (class view *)this;		/* where to send subsequent hits */
}

	view::DSattributes
labelview::DesiredSize( long  width, long  height, enum view::DSpass  pass, 
				long  *desiredWidth, long  *desiredHeight ) 
						{
	if ( ! this->GaveSize ) {
		long w, h;
		char *tail;
		class label *st 
			= (class label *)this->GetDataObject();
		class graphic *g;
		struct FontSummary *FS;
		if ( ! CheckWindow(this, "DSize")) {
			/* can't compute size without a window (we need a font)
				give dummy values */
			*desiredWidth = 144;
			*desiredHeight = 20;
			return (view::DSattributes)(view::HeightFlexible | view::WidthFlexible);
		}
		this->minwidth = this->minheight = 0;
		g = (this)->GetDrawable();
		FS = (st->font)->FontSummary( g);
		tail = (char *)((class label *)this->GetDataObject())->GetText();
		while (tail) {
			char *tend = strchr(tail, '\n');
			if (tend) {
				(st->font)->TextSize( g, tail, tend-tail, &w, &h);
				tail = tend+1;
			}
			else {
				(st->font)->StringSize( g, tail, &w, &h);
				tail = NULL;
			}
			if (w > this->minwidth)  this->minwidth = w;
			this->minheight += FS->maxHeight;
		}
		this->GaveSize = TRUE;
	}

	*desiredWidth = this->minwidth+6;
	*desiredHeight = this->minheight+6;
/*
fprintf(stderr, "input %dx%d    output %dx%d\n", 
width, height, *desiredWidth, *desiredHeight); fflush(stderr);
*/
	return (view::DSattributes)(view::HeightLarger | view::WidthLarger);
}

	void
labelview::Print( FILE    *file, const char  	  *processor, const char  	  *format, boolean  	 level )
					{
	/* XXX sigh */
}

	void
labelview::SetHitProc(labelview_hitfptr proc, char  *rock)
			{
	this->hitproc = proc;
	this->hitrock = rock;
}

	char *
labelview::GetHitRock()
	{
	return this->hitrock;
}

	void
labelview::SetInversion(boolean  invert)
		{
	if (this->inverted == invert)
		return;
	this->inverted = invert;
	(this)->WantUpdate( this);
}

	boolean
labelview::GetInversion()
	{
	return this->inverted;
}
