/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *	   Copyright Carnegie Mellon, 1992 - All Rights Reserved
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
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

/* nullv.c	

	The view module for the null dataobject

*/

/*
 *   $Log: nullview.C,v $
 * Revision 1.13  1995/02/09  22:46:21  Zarf
 * Added methods for new printing mechanism
 *
 * Revision 1.12  1995/01/31  01:58:56  wjh
 * removed include of <strings.h>
 * (unneeded since this is done by andrewos.h)
 *
 * Revision 1.11  1994/11/30  20:42:06  rr2b
 * Start of Imakefile cleanup and pragma implementation/interface hack for g++
 *
 * Revision 1.10  1994/08/11  02:53:14  rr2b
 * The great gcc-2.6.0 new fiasco, new class foo -> new foo
 *
 * Revision 1.9  1994/03/21  16:59:25  rr2b
 * bzero->memset
 * bcopy->memmove
 * index->strchr
 * rindex->strrchr
 * some mktemp->tmpnam
 *
 * Revision 1.8  1994/03/19  03:06:48  rr2b
 * Fixed to use this instead of self when drawing
 * the dots.
 * BUILD
 *
 * Revision 1.7  1994/03/12  22:42:52  rr2b
 * Removed second argument to Stringize, an empty string may not be stringized
 *
 * Revision 1.6  1994/02/15  18:15:54  wjh
 * changed blobs to circles
 *
 * Revision 1.5  1993/10/20  16:30:47  rr2b
 * Added use of stringize for traditional vs ANSI compatibility.
 *
 * Revision 1.4  1993/09/28  04:52:23  rr2b
 * Fixed view_DSattributes to be an int not an enum.
 *
 * Revision 1.3  1993/06/17  21:56:58  rr2b
 * Weird... honest, it compiled before..
 * Anyway, added a cast on the second argument to ChainBeforeML.
 *
 * Revision 1.2  1993/06/05  00:32:29  rr2b
 * Converted to C++
 *
 * Revision 1.1  1993/06/04  23:26:51  rr2b
 * Initial revision
 *
 * Revision 1.6  1992/12/15  21:32:24  rr2b
 * more disclaimerization fixing
 *
 * Revision 1.5  1992/12/14  20:40:31  rr2b
 * disclaimerization
 *
 * Revision 1.4  1992/06/22  21:48:52  wjh
 * test for NULL in getting fontdesc
 * made blobs chartreuse on color displays
 * removed extraneous clearing of screen in _ClearRectangle
 * check for NULLs in InitializeClass
 * give a default desiredSize
 * .
 *
 * Revision 1.3  1991/09/12  16:11:44  bobg
 * Update copyright notice and rcsid
 *
 * Revision 1.2  1989/12/12  15:00:08  ghoti
 * sync with MIT tape
 *
 * Revision 1.2  89/12/05  11:33:39  xguest
 * change instances of nullv to nullview
 * 
 * Revision 1.1  89/12/04  20:28:25  xguest
 * Initial revision
 * 
 * Revision 1.1  89/07/31  15:35:20  wjh
 * Initial revision
 * 
 *  
 * Revision 1.0  88/05/14  15:40:34  wjh
 * Copied from /usr/andrew/lib/genericinset
 */


#include <andrewos.h>
ATK_IMPL("nullview.H")

#include <graphic.H>
#include <view.H>
#include <fontdesc.H>

#include <menulist.H>
#include <keymap.H>
#include <keystate.H>
#include <bind.H>
#include <im.H>
#include <print.H>
#include <rect.h>
#include <texttroff.H>

/* $$$ include xxx.ih for any other file for which there is below a call to xxx_routine() */

/* include headers for the data object and THIS view */
#include <null.H>
#include <nullview.H>


static char  debug;      /* This debug switch is toggled with ESC-^D-D */
	/*	$$$ you could provide a method in this module which sets -debug-
		Then nullvt.c could call it to set this debug from the -d switch */
#define DEBUG(s) {if (debug) {printf s ; fflush(stdout);}}
#define ENTER(r) DEBUG(("Enter %s(0x%p)\n", Stringize(r), this))
#define LEAVE(r) DEBUG(("Leave %s(0x%p)\n", Stringize(r), this))

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
 *
 *	Supporting procedures
 *	
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/* $$$ ComputeArea is one approach to a computation satisfying xxx_DesiredSize. 
	This code also illustrates how to find the size of a character or string.
	You can delete this function if size is computed entirely 
	within the xxx_DesiredSize routine. */

/* ComputeArea(self)
	set fields self->Area / Width / Height
	The width and height are 10 times the size of a 'W' and area is the product.

	The routine is more general, though; variable c can be set to any string and 
	ten times the size of that string is used.

	(Should modify so it checks the coordinates in its data object and
	requests at least enough room for the largest.)
*/
	
ATKdefineRegistry(nullview, view, nullview::InitializeClass);

static void ComputeArea(class nullview  *self);
static void  InvertRectangle(class nullview  *self, long left, long top, long width, long height);
static void ShowString(class nullview  *self, long  x , long  y, char  *string);
static  void RedrawView(class nullview  *self, enum view_UpdateType  type, long  left , long  top , long  width , long  height);
static void nullview_ClearRectangle(class nullview  *self, long  rock);
static void ToggleDebug(class nullview  *self, long  rock);


static void
ComputeArea(class nullview  *self)
{
	long minwidth, minheight;
	const char *tail;
	const char *c = "W";
	class graphic *g;
	class fontdesc *fd;
	struct FontSummary *FS;
	long cnt;

	if (self->sizeknown)
		return;

	g = (class graphic *)(self)->GetDrawable();
	fd = (self)->GetFont();
	if (g == NULL || fd == NULL) 
		return;
	FS = (fd)->FontSummary( g);
	if (FS == NULL) return;

	minwidth = minheight = 0;
	for (cnt=0; cnt < 2; cnt++) {
		tail = c;
		/* if c is multiple lines, the size must be computed by
			adding the sizes of each line */
		while (tail) {
			const char *tend = strchr(tail, '\n');
			long w, h;
			if (tend) {
				(fd)->TextSize( g, tail, tend-tail, &w, &h);
				tail = tend+1;
			}
			else {
				(fd)->StringSize( g, tail, &w, &h);
				tail = NULL;
			}
			if (w > minwidth)  minwidth = w;
			minheight += FS->maxHeight;
		}
		if (minwidth != 0  &&  minheight != 0) 
			break;
		/* first tried current font: it had no 'W'.  Now try andy12 font */
		fd = fontdesc::Create("andy", 0, 12);
	}
	/* The string width is more or less minwidth.  
		(In fact minwidth is the distance
		from the origin of the first character of the string 
		to the origin of the next character after the string.)
	    The string height is no greater than minheight
		(We should really use FS->newlineHeight, 
		but that field is not implemented at this writing.)  */

	self->DesiredWidth = 10 * minwidth;
	self->DesiredHeight = 10 * minheight;
	self->DesiredArea = 100 * minwidth * minheight;

	self->sizeknown = TRUE;

	DEBUG(("ComputeArea: %ld x %ld   (cnt: %ld)\n", 
			self->DesiredWidth, self->DesiredHeight, cnt));
}


/* 	$$$ This routine is not used.  It illustrates using a graphic primitive */
/* InvertRectangle(self, left, top, width, height)
	Changes white to black and black to white within the rectangle. 
*/
	static void 
InvertRectangle(class nullview  *self,long left,long top,long width,long height)
{
	/* this code shows how to invert rectangles */
	/* it will give unpredictable results on color displays */
	struct rectangle r;

	(self)->SetTransferMode( graphic_INVERT);
	rectangle_SetRectSize(&r, left, top, width, height);
	(self)->FillRect( &r, self->bpattern);
}

/* 	$$$ This routine is not used.  It illustrates using a graphic primitive */
/* ShowString(self, x, y, string)
	Writes the text of 'string' centered at (x, y) in the logical rectangle.
*/
	static void
ShowString(class nullview *self, long x ,long y, char *string)
{
	(self)->MoveTo( x, y);
	(self)->DrawString( string,
			graphic_BETWEENLEFTANDRIGHT |
			    graphic_BETWEENTOPANDBASELINE);
}


/* 	$$$ replace the following routine with one that redraws 
	whatever image your view is to provide 
		The algorithm here flashes the screen because it erases all the
		dots each time a new one is added.
		The correct approach is for the view to keep track of
		what it has on the screen and compare it with the data object
		each time there is to be a redraw.
		This will then continue to work if we add an operation to 
		remove dots.
*/
/* RedrawView(self, type, left, top, width, height)
	Redraws the view from the data structure.
	If the type specifies a partial redraw, we only really need to redraw the
	portion delimited by left, top, width, height. ($$$)
*/
	static 
void RedrawView(class nullview  *self, enum view_UpdateType  type,long left,long top,long width,long height)
{
	struct rectangle r;
	struct dotlist *d;
	class null *dobj 
		= (class null *)self->dataobject;
	boolean colordpy = ((self)->DisplayClass() & graphic_Color) != 0;
	long fgr, fgg, fgb;

	(self)->SetTransferMode( graphic_COPY);
	(self)->GetLogicalBounds( &r);		/* find rectangle bounds */
	(self)->FillRect( &r, self->wpattern);	/* clear the rectangle */

	rectangle_GetRectSize(&r, &left, &top, &width, &height);
		/* we shamelessly utilize the parameters as locals */
	DEBUG(("RedrawView(%d) in [%ld, %ld, %ld, %ld]\n", type, left, top, width, height));
		/* we have to subtract one from 
			right and bottom to get inside the rectangle */
	if (! self->HasInputFocus)  {
		/* draw a diagonal line to indicate lack of input focus */
		(self)->MoveTo( left, top);
		(self)->DrawLineTo( left+width-1, top+height-1);
	}
	else 
		/* has input focus.  draw outline of rectangle */
		(self)->DrawRectSize( left, top, width-1, height-1);

	if (colordpy)
		(self)->GetForegroundColor( NULL, &fgr, &fgg, &fgb);

	for (d = (dobj)->GetFirstDot();  d != NULL;
			d = (dobj)->GetNextDot( d)) {
		/* display each dot as a ring  */
		struct rectangle r;
		(self)->SetTransferMode( graphic_COPY);
		rectangle_SetRectSize(&r, (dobj)->GetDotX(d)-4,
			(dobj)->GetDotY(d)-4, 9, 9);
		(self)->FillOval(&r, self->bpattern);
		if (colordpy) {
			rectangle_SetRectSize(&r, (dobj)->GetDotX(d)-2,
				(dobj)->GetDotY(d)-2, 5, 5);
			/* for colors, see /usr/lib/X11/rgb.txt */
			(self)->SetForegroundColor(NULL,
					127<<8, 255<<8, 0<<8);  /* chartreuse */
			(self)->FillOval(&r, NULL);	/* use foreground color */
			(self)->SetForegroundColor(NULL, fgr, fgg, fgb);
		}
	}
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
 *	User Interface 
 *	
 *	Routines called from keystrokes or menu
 *
 *	$$$ in this section put any operations that are to apply to your view
 *	The -rock- is the 3rd or 5th value from the bind description line (depending
 *	on whether the operation is invoked from menu or keyboard).
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

	static void
nullview_ClearRectangle(class nullview  *self, long  rock)
{
	class null *dobj = (class null *)self->dataobject;
	(dobj)->ClearDots();
	(dobj)->NotifyObservers( null_DATACHANGED);
}

	static void
ToggleDebug(class nullview  *self, long  rock)
{
	debug = ! debug;
	printf("debug is now %d\n", debug);  fflush (stdout);
}


/*  Menus and Keymaps
	The menus and keymaps are initialized to those of
	    EmbeddedMenus and EmbeddedKeymap in InitializeObject.
	The ApplicationMenus and ApplicationKeymap get
	    added to the menus and keymap in GetApplicationLayer

	Debugging can be toggled with the key sequence  ESC-^D -D

	$$$ Replace the initialization of the arrays with keybindings (2nd value)
	and/or menu entries (4th value) of your choice.  The function to be called
	is the 7th value in each array line.  The first value is a name and the 8th is 
	a comment.  The 6th controls non-constant menus and the 9th is ignored.
*/
static class menulist  *EmbeddedMenus, *ApplicationMenus;
static class keymap  *EmbeddedKeymap, *ApplicationKeymap;

static struct bind_Description EmbeddedBindings[]={
    {"null-clear-rectangle", "\003", 0, "Clear~10", 0, 0, 
			(proctable_fptr)nullview_ClearRectangle, "Clear the rectangle", NULL},
    {"null-invert-debug", "\033\004D",		/* ESC - ^D - D */
			0, 0, 0, 0, (proctable_fptr)ToggleDebug,
			"Toggle the nullview debug flag", NULL},
	NULL
};
static struct bind_Description ApplicationBindings[]={
	NULL
};



	boolean
nullview::InitializeClass()
{
	EmbeddedMenus = new menulist;
	ApplicationMenus = new menulist;
	EmbeddedKeymap = new keymap;
	ApplicationKeymap = new keymap;
	if (EmbeddedMenus == NULL || ApplicationMenus == NULL
			|| EmbeddedKeymap == NULL 
			|| ApplicationKeymap == NULL)
		return FALSE;
	bind::BindList(EmbeddedBindings, EmbeddedKeymap, EmbeddedMenus,
				&nullview_ATKregistry_ );
	bind::BindList(ApplicationBindings, ApplicationKeymap, ApplicationMenus,
				&nullview_ATKregistry_ );
	return TRUE;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
 *	
 *	Override methods
 *	
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
	nullview::nullview()
{
	ATKinit;

	this->Menus = (EmbeddedMenus)->DuplicateML( this);
	/* ApplicationMenus are added in GetApplicationLayer */
	this->Keystate = keystate::Create(this, EmbeddedKeymap);

	if (this->Menus == NULL || this->Keystate == NULL)
		THROWONFAILURE( FALSE);

	this->sizeknown = FALSE;
	this->OnScreen = FALSE;
	this->HasInputFocus = FALSE;
	this->embedded = TRUE;
	this->ignoreUp = FALSE;

	this->DesiredWidth = 145;
	this->DesiredHeight = 91;
	this->DesiredArea = 100 * 91 * 45;

	/* $$$ here initialize any variables added to the "data:" portion of 
		nullv.ch */

	THROWONFAILURE( TRUE);
}

	nullview::~nullview()
{
	ATKinit;

	delete this->Menus;
	delete this->Keystate;

	/* $$$ is variables in the data: of nullv.ch have been assigned
	pointers to malloc'ed storage, that memory should be free'd here.  */

}

	void
nullview::ObservedChanged(class observable  *dobj, long  status)
{
	if (status == observable_OBJECTDESTROYED) 
		/* do not call wantupdate in this case,
			it will bomb the program */
		{}
	else if (status == null_DATACHANGED) 
		/* $$$ it would be a good idea to make a note of which parts 
			of the data have changed, so only the corresponding parts 
			of the image need to be changed.  */ 
		(this)->WantUpdate( this);	/* request a chance to 
						update the image */
}

	class view *
nullview::GetApplicationLayer()
{
	this->embedded = FALSE;
	(this->Menus)->ChainBeforeML( 
		(ApplicationMenus)->DuplicateML( this), (long)NULL);
	(keystate::Create(this, ApplicationKeymap))->AddBefore( this->Keystate);
	return this;
}

/* DON'T DO screen operations in ReceiveInputFocus */

	void 
nullview::ReceiveInputFocus()
{
	ENTER(nullv_ReceiveInputFocus);
	(this)->PostMenus( this->Menus);

	this->Keystate->next = NULL;
	(this)->PostKeyState( this->Keystate);

	this->HasInputFocus = TRUE;
	(this)->WantUpdate( this);   /* schedule an update to 
			provide an indication that we have the input focus */
	LEAVE(nullv_ReceiveInputFocus);
}

/* DON'T DO screen operations in ReceiveInputFocus,
	but DO remove any defined cursor */

	void 
nullview::LoseInputFocus()
{
	ENTER(nullview_LoseInputFocus);
	this->HasInputFocus = FALSE;
	(this)->WantUpdate( this);  /* schedule an update to display 
			 indication that we no longer have the input focus */

	/* menus and keystate are deactivated by parent */

	LEAVE(nullview_LoseInputFocus);
}

/* nullview__FullUpdate(self, type, left, top, width, height)
	Redraws the entire image.  (The last four arguments are only used in 
	case the 'type' is a PartialRedraw;  they specify which part to update.)
*/
	void 
nullview::FullUpdate(enum view_UpdateType   type, long   left , long   top , long   width , long   height)
{
	DEBUG(("FullUpdate(%d, %ld, %ld, %ld, %ld)\n", type, left, top, width, height));
	if (type == view_Remove  
			||  (this)->GetLogicalWidth() == 0 
			|| (this)->GetLogicalHeight() == 0) {
		/* view_Remove means the view has left the screen.
			A zero dimension means the view is not visible */
		this->OnScreen = FALSE;
		return;
	}
	if (type != view_FullRedraw && type != view_LastPartialRedraw)
		return;
	/* we continue only for a FullRedraw or the last of a sequence of PartialRedraw
		requests.  */

	this->OnScreen = TRUE;
/* the following line is appropriate, but on 24 April 1988 it tickles a bug in ez 
	if (type == view_FullRedraw) 
*/
	{
		/* must recompute graphics info because image
			may be on different display hardware */

		this->wpattern = (this)->WhitePattern();
		this->bpattern = (this)->BlackPattern();
		DEBUG(("Drawable: 0x%p   White: 0x%p   Black: 0x%p\n",
			(this)->GetDrawable(), 
			this->wpattern, this->bpattern));
	}
	RedrawView(this, type, left, top, width, height);
	LEAVE(nullview_FullUpdate);
}


	void 
nullview::Update()
{
ENTER(nullview_Update);
	if (! this->OnScreen) return;

	/* $$$ the code in this example is highly bogus: it redraws the entire view
		every time there is any change.  Note that this leads to unacceptable
		flashing.  Instead the notes taken in ObservedChanged should be
		used here to update only the changed portion of the image */

	RedrawView(this, view_FullRedraw, 0, 0, 0, 0);

LEAVE(nullview_Update);
}

	class view *
nullview::Hit(enum view_MouseAction   action, long   x , long   y , long   num_clicks)
{
	class null *dobj
		= (class null *)this->dataobject;
DEBUG(("Hit at (%ld, %ld) type %d\n", x, y, action));
	if (action == view_NoMouseEvent)
		return (class view *)this;
	if (! this->OnScreen) return NULL;

	/* $$$ replace the following section with code to provide the
		desired response to mouse actions.  In this example, 
		a new dot is drawn and added to the data structure
		at the coordinates of the mouse hit.  It is a convention
		that permanent actions should take place on Up movement
		of a mouse button.   */
	if (action == view_LeftDown || action == view_RightDown) {
		if ( ! this->HasInputFocus) {
			(this)->WantInputFocus( this);
			this->ignoreUp = TRUE;
		}
	}
	else if (action == view_LeftUp || action == view_RightUp) {
		struct rectangle r;

		if (this->ignoreUp) {
			this->ignoreUp = FALSE;
			return NULL;
		}
		(this)->GetLogicalBounds( &r);	/* find rectangle bounds */
		if (x < rectangle_Left(&r)  ||  x >= rectangle_Right(&r)
				|| y < rectangle_Top(&r)
				|| y >= rectangle_Bottom(&r))
			/* if the mouse up is out of the rectangle, we do not
				want to add a dot.  Note the '=' signs;
				rectangles are not symmetrically bounded.  */
			return NULL;

		(dobj)->AddDot( x, y);
		(dobj)->NotifyObservers( null_DATACHANGED);

		/* draw the image immediately
			(It will be redrawn by the Update routine
			called because of the NotifyObservers) */
		(this)->SetTransferMode( graphic_COPY);
		rectangle_SetRectSize(&r, x-4, y-4, 9, 9);
		(this)->FillOval(&r, this->bpattern);
	}

LEAVE(::Hit);
	return (class view *)this;		/* where to send subsequent hits */
}

/* 	$$$ replace with some routine that responds with a size for your view */
/* nullview__DesiredSize(self, width, height, pass, desiredWidth, desiredHeight) 
	The parent calls this to find out how big the view would like to be.
	This routine sets 'desiredWidth' and 'desiredHeight' and returns a
		code value indicating which is flexible for further negotiation.
	The 'width' and 'height' parameters are tentative values the parent is suggesting.
	'pass' indicates which of 'width' and 'height' should be considered fixed.
	If neither is fixed, they may be arbitrary values. 
*/
	view_DSattributes
nullview::DesiredSize(long  width, long  height, enum view_DSpass  pass, long  *desiredWidth, long  *desiredHeight) 
{
	DEBUG(("DesiredSize(...%ld, %ld, %d...)\n", width, height, pass));

	ComputeArea(this);	/* set self->DesiredArea/Width/Height */

	if (pass == view_NoSet) {
		*desiredWidth = this->DesiredWidth;
		*desiredHeight = this->DesiredHeight;
	}
	else if (pass == view_WidthSet) {
		*desiredWidth = width;
		*desiredHeight = this->DesiredArea / width;
	}
	else /* pass == view_HeightSet */ {
		*desiredWidth = this->DesiredArea / height;
		*desiredHeight = height;
	}

	DEBUG(("Leave DesiredSize: %ld x %ld\n", *desiredWidth, *desiredHeight));
	return view_HeightFlexible | view_WidthFlexible;
}

/* # # # # # # # # # # # # # # 
 *	PRINTING	
 *  # # # # # # # # # # # # #  */

/* nullview::Print is the method that is used by the old printing mechanism. */
	void
nullview::Print(FILE   *file, const char    *processor, const char    *format, boolean    topLevel)
					{
#ifdef texttroff_HASPBPE		/* use this until new texttroff is everywhere  */
	class null *dobj = (class null *)this->dataobject;
	long wpts, hpts;  /* image dimensions in points */
	struct dotlist *dot;
	char *prefix;

	/* $$$  If you want to describe the print image of your view in troff,
		replace the contents of this routine.  To use PostScript,
		replace the pieces noted below. */

	wpts = (this)->GetLogicalWidth();
	hpts = (this)->GetLogicalHeight();
	if (wpts == 0  ||  hpts == 0) {
		/* the parent has GOOFED and has not
			supplied a logical rectangle for printing */
		ComputeArea(this);
		wpts = this->DesiredWidth;
		hpts = this->DesiredHeight;
	}

	/* We should restrict wpts and hpts to (say) 576x720  (8x10 inches).
		We could do so by scaling the image */

	if (strcmp(processor, "troff") == 0) {
		/* output to troff */
		if (topLevel)
			/* take care of initial troff stream */
			texttroff::BeginDoc(file);
		/*  Put macro to interface to postscript */
		texttroff::BeginPS(file, wpts, hpts);
		prefix = "\\!  ";
	}
	else prefix = "";

	/* generate PostScript  */
	fprintf(file, "%s  /width %d def  /height %d def\n", prefix, wpts, hpts);
	fprintf(file, "%s  newpath 0 0 moveto 0 height lineto width height lineto\n", prefix);
	fprintf(file, "%s  \twidth 0 lineto clip    %% clip to assigned area\n", prefix);	

	/* $$$ if you want to describe the image of your inset in postscript, replace the 
	following fprints and the for-loop below with code to generate the PostScript. */

	/* define a postscript routine to display a dot */
	fprintf(file, "%s  /blob {\n", prefix);
	fprintf(file, "%s  \tneg height add    %% convert from screen to postscript coords\n",
			prefix);
	fprintf(file, "%s  \tmoveto	%% use coords from call\n", prefix);
	fprintf(file, "%s  \t10 0 rlineto\n", prefix);
	fprintf(file, "%s  \t0 -7 rlineto\n", prefix);
	fprintf(file, "%s  \t-10 0 rlineto\n", prefix);
	fprintf(file, "%s  \tfill\n", prefix);
	fprintf(file, "%s  } def\n", prefix);

	fprintf(file, "%s  newpath width 0 moveto 0 height lineto 0.5 setlinewidth stroke \t\n",
			prefix);		/* draw diagonal line */

	/* draw the dots */
	for (dot = (dobj)->GetFirstDot(); dot != NULL; 
			dot = (dobj)->GetNextDot( dot))
		fprintf(file, "%s  %d %d blob\n", prefix,
			(dobj)->GetDotX( dot),
			(dobj)->GetDotY( dot));

	if (strcmp(processor, "troff") == 0){
		texttroff::EndPS(file, wpts, hpts);
		if (topLevel)
			texttroff::EndDoc(file);
	}
#endif /* texttroff_HASPBPE */	/* use this until new texttroff is everywhere */
}

/* GetPSPrintInterface, PrintPSDoc and PrintPSRect are used by the new printing mechanism. */
	void *
nullview::GetPSPrintInterface(const char *printtype)
{
    if (!strcmp(printtype, "generic"))
	return (void *)this;

    return NULL;
}

	void
nullview::PrintPSDoc(FILE *outfile, long pagew, long pageh)
{
    boolean printthis;
    printthis = print::PSNewPage(print_UsualPageNumbering);
    if (printthis) {
	/* if the user wants this page printed, print it. */
	this->PrintPSRect(outfile, pagew, pageh, NULL);
    }
}

	void
nullview::PrintPSRect(FILE *file, long logwidth, long logheight, struct rectangle *visrect)
{
    class null *dobj = (class null *)this->dataobject;
    struct dotlist *dot;

    fprintf(file, "%% ATK null inset\n");

    /* register the PS procedure that we want to use */
    print::PSRegisterDef("nullBlot", "{ moveto %% use coords from call\n 10 0 rlineto\n 0 -7 rlineto\n -10 0 rlineto\n fill\n}");

    /* draw a line */
    fprintf(file, "newpath %ld 0 moveto 0 %ld lineto 0.5 setlinewidth stroke \t\n", logwidth, logheight);		/* draw diagonal line */

    /* draw the dots */
    for (dot = (dobj)->GetFirstDot(); dot != NULL; dot = (dobj)->GetNextDot( dot)) {
	fprintf(file, "%ld %ld nullBlot\n",
		(dobj)->GetDotX( dot),
		logheight - (dobj)->GetDotY( dot));
    }
}
