/* ***************************************************************** *\
 *	   Copyright Carnegie Mellon, 1994 - All Rights Reserved     *
 *        For full copyright information see:'andrew/doc/COPYRITE'   *
\* ***************************************************************** */

/* gofigv.c

	The view module for the gofig dataobject

User Interface:

A "spot" is an intersection, whether occupied or not.
A spot can be selected, which is indicated with a dotted circle.
The first click grabs input focus and selects the closest spot.
A left click while having inputfocus selects the nearest spot and
puts in it the appropriate contents, depend on tool selection.
Repeat clicks on the selected spot cycle the occupancy among
	empty -> black -> white -> repeat
A right click selects the spot clicked on.

Menu
	(in this list, the symbol '<=' appears when the given mode has been 
		selected;  only one <= will appear)

Set 1, stone mode
	Black-White <= - alternate colors starting with the opposite
			of most recently colored stone
	White <= - the first click on each spot will put a white stone
	Black <= - the first click on each spot will put a black stone
	Empty <= - the first click on a spot will empty it or leave it empty
	Select <= - clicking will only select the clicked spot

Set 2, note mode
	Auto a, b, c, ...  <=
		The next stone created is given the next available letter.
		Subsequently selected spots are given successive letters.
	Auto 1, 2, 3, ...  <=
		The next stone created is given the next available number.
		Subsequently selected spots are given successive numbers.
	No Auto Notes  <=
		Turns off automatic sequencing.
	(most keyboard operations change back to NoAutoNotes)

Separate items
	Enter Note - prompts for note and puts it in selected spot
		letter, number, or  ^ + % @ # - ~ _ &  (Respectively:  
			triangle, orthogonal cross, diagonal cross, circle, 
			square, dash, dash with line up, dash with line down,
			and upside down triangle)
	Board Layout - prompts for board dimensions
		add LRTB or subset to get edges at left, right, top , bottom
		add I to get indices A...T 1...19
		A for all edges
		N for no edges

Keyboard commands
	see gofig.help

debugging
	ESC-ctl-D-shift-D - turn on useless debugging stuff
	D - dump debug info to stdout


enhancements:
	define x, o, and . keys to move to the next spot after entering the stone
	use off-screen windows to store stones to speed up clicking
		eight saved images: empty, empty with note spot,
			black, white and repeat these four with selection

*/

/*
 *   $Log: gofigview.C,v $
 * Revision 1.10  1996/04/23  22:05:45  wjh
 * fixed so it prints n in a stone
 *
 * Revision 1.9  1995/10/25  17:26:50  wjh
 * make white stones be white in all cases (were the background color)
 *
 * Revision 1.8  1995/10/15  21:46:00  wjh
 * revised keymapping to say waht to do for a mouse hit
 * and to prvide additional ways to move cursor
 *
 * Revision 1.7  1994/11/10  21:58:26  wjh
 * fixed compile errors on Pmax
 * fixed core dump when printing via troff at top level
 *
 * Revision 1.6  1994/11/08  22:09:48  wjh
 * neater cleanup when removing stone
 * fix code for DELETE key (from \133 to \177 !)
 * fix surprising height change when change width
 * fix strokepath bug in the generated postscript
 *
 * Revision 1.5  1994/11/01  22:25:27  wjh
 * made threedigitfont wider
 * fixed limitcheck from strokepath by trebling flatness value
 *
 * Revision 1.4  1994/10/31  21:40:40  wjh
 * reduced printed size of hoshi dot
 * increased size of notes in stones
 * fixed to continue inserting numbers if type in a number
 *
 * Revision 1.3  1994/10/29  16:41:03  rr2b
 * Initialization of structs on the stack is not supported by all
 * compilers.  (ie no automatic aggregate initialization)
 * Initialized variables in switch statements must be enclosed in a
 * scope.  (otherwise C++ reports "jump past initializer")
 * BUG
 *
 * Revision 1.2  1994/10/29  16:37:22  rr2b
 * Commented out #includes of strings.h and ctype.h.
 * BUG
 *
 * Revision 1.1  1994/10/14  21:04:02  wjh
 * Initial revision
 * 
 * Revision 1.0  94/08/17  18:52:51  wjh
 * Copied from /usr/andrew.C++/lib/null
 */

#include <andrewos.h>
#include <math.h>
#include <util.h>

#include <graphic.H>
#include <view.H>
#include <fontdesc.H>
#include <color.H>
#include <environ.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <bind.H>
#include <proctable.H>
#include <im.H>
#include <rect.h>
#include <texttroff.H>
#include <message.H>
#include <print.H>

/* include headers for the data object and THIS view */
#include <gofig.H>
#include <gofigview.H>

static char  debug = FALSE;      /* This debug switch is toggled with ESC-^D-D */
#define DEBUG(s) {if (debug) {printf s ; fflush(stdout);}}
#define ENTER(r) DEBUG(("Enter %s(0x%p)\n", Stringize(r), this))
#define LEAVE(r) DEBUG(("Leave %s(0x%p)\n", Stringize(r), this))

static long const pagew = 612, pageh = 792;	  /* 8.5*11 */
	/* this paper size is used only as arguments to DesiredPrintSize, so it
	will suffice for most reasonable page sizes */

static char *BoardColor;	/* name of board color from preferences */

static fontdesc *Font20;

/* INDEXFRAC is the fraction of spotwidth devoted to indices */
#define INDEXFRAC 0.85

/* WTORATIO is ratio of width over height
	for normal board ratio this is 2.21/2.36 = 0.93644 */
#define WTOHRATIO 0.97	

/* SCREENEXPAND is multiplied times print size to get a reasonable screen size */
#define SCREENEXPAND 1.5

	
ATKdefineRegistry(gofigview, view, gofigview::InitializeClass);


static int leftend(gofigview *self);
static int rightend(gofigview *self);
static int topend(gofigview *self);
static int bottomend(gofigview *self);
static void computeHoshi(int hoshi[4], boolean leftedge, boolean rightedge, 
	int width);
static void drawspot(gofigview *self, struct stone *s, boolean isclear);
static  void RedrawView(gofigview  *self);
static void modifymenuentry( menulist *m, const char *f, const char *prior, const char *post, 
		struct proctable_Entry *proc, char code );
static struct stone *atrowcol(gofigview *self, int row, int col);
static struct stone *selectedspot(gofigview *self);
static void ClickFirst();
static void PostMyMenus(gofigview *self);
static void SetClickMode(gofigview *self, int code);
static void SetNoteMode(gofigview *self, int code);
static void DoNothing(gofigview *self, int code);
static void PutStone(gofigview *self, int code);
static void CharNote(gofigview *self, int code);
static void EnterNote(gofigview *self, int code);
static void MoveSpot(gofigview *self, int code);
static void SetLayout(gofigview *self, int code);
static void CopyCommand(gofigview *self, int code);
static void ReplaceCommand(gofigview *self, int code);
static void ToggleDebug(gofigview  *self, long  rock);
static void GeneratePostScript(FILE *file, gofig *dobj, const char *prefix, 
		int wpts, int hpts);
static void printdata(gofigview *self, int code);

static int inline mytrunc(double a) {return /* itrunc */( a );}

	static inline void 
setnextcolor(gofigview *self, char color) {
	switch(color) {
	case 'B': self->nextcolor = 'W'; break;
	case 'W': self->nextcolor = 'B'; break;
	}
}

	static int
leftend(gofigview *self) {
	unsigned edges;
	gofig *dobj = (gofig *)self->GetDataObject();
	dobj->getedges( &edges );
	return self->xoff 
		- ((edges&LEFTedge) ? self->linethickness/2 : self->radius);
}
	static int
rightend(gofigview *self) {
	unsigned edges;
	gofig *dobj = (gofig *)self->GetDataObject();
	dobj->getedges( &edges );
	return self->xoff + (dobj->width-1)*self->spotwidth 
		+ ((edges&RIGHTedge) ? self->linethickness/2: self->radius);
}
	static int
topend(gofigview *self) {
	unsigned edges;
	gofig *dobj = (gofig *)self->GetDataObject();
	dobj->getedges( &edges );
	return self->yoff 
		- ((edges&TOPedge) ? self->linethickness/2 : self->radius);
}
	static int
bottomend(gofigview *self) {
	unsigned edges;
	gofig *dobj = (gofig *)self->GetDataObject();
	dobj->getedges( &edges );
	return self->yoff + (dobj->height-1)*self->spotheight 
		+ ((edges&BOTTOMedge) ? self->linethickness/2 : self->radius);
}

	static void
computeHoshi(int hoshi[4], boolean leftedge, boolean rightedge, int width) {
	hoshi[0] = -1; hoshi[1] = -1; hoshi[2] = -1; hoshi[3] = -1;
	if ( ! leftedge && ! rightedge)  
		return;
	if (leftedge && ! rightedge) {
		if (width >= 4) hoshi[0] = 3; 
		if (width >= 10) hoshi[1] = 9; 
		if (width >= 16) hoshi[2] = 15;
		return;
	}
	if (rightedge && ! leftedge) {
		if (width >= 4) hoshi[0] = width - 4; 
		if (width >= 10) hoshi[1] = width - 10; 
		if (width >= 16) hoshi[2] = width - 16;
		return;
	}

	if (width == 7) {hoshi[0] = 3;  return;}
	if (width < 8) return;
	if (width <= 11) {hoshi[0] = 2;  hoshi[1] = width-3;  return;}

	hoshi[0] = 3;
	hoshi[1] = width-4;
	if (width >= 17 && (width & 0x1))
		hoshi[2] = (width-1)/2;
}

	static struct stone *
atrowcol(gofigview *self, int row, int col) {
	gofig *dobj = (gofig *)self->GetDataObject();
	int i;
	for (i = 0; i < dobj->nstones(); i++)
		if ((*dobj)[i].row == row 
				&& (*dobj)[i].col == col) 
			return &(*dobj)[i];
	return NULL;
}

	static struct stone *
selectedspot(gofigview *self) {
	gofig *dobj = (gofig *)self->GetDataObject();
	if (self->curspotrow == -1) return NULL; 
	struct stone *s = atrowcol(self, self->curspotrow, self->curspotcol);
	if (s) return s;

	/* create a stone at curspot */
	return dobj->addstone(self->curspotrow, self->curspotcol);
}

/* draw the contents of the spot for stone s
		if 'isclear', spot has got lines and nothing else
*/
	static void
drawspot(gofigview *self, struct stone *s, boolean isclear) {
	gofig *dobj = (gofig *)self->GetDataObject();
	boolean isselected = (s->row == self->curspotrow
				&& s->col == self->curspotcol);
	const int x = self->xoff + s->col * self->spotwidth;
	const int y = self->yoff  + s->row * self->spotheight;
	int radius = self->radius;
	int lthick = self->linethickness;
	int diam = 2*radius + ((lthick&1) ? 1 : 0);
	long dpycls = (self)->DisplayClass();
	boolean colordpy = (dpycls & graphic::Color) != 0;
	double fgr, fgg, fgb;

	if (colordpy)
		self->GetFGColor( &fgr, &fgg, &fgb );

	if ( ! isclear && s->color == '/') {
		/* need to clear the cell and draw lines */
		long left, top, width, height;
		left = x - radius 
				- (atrowcol( self, s->row, s->col-1 ) ? 0 : 1);
		width = diam 
				+ (atrowcol( self, s->row, s->col-1 ) ? 1 : 0)
				+ (atrowcol( self, s->row, s->col+1 ) ? 1 : 0);
		top = y - radius 
				- (atrowcol( self, s->row-1, s->col ) ? 0 : 1);
		height = diam 
				+ (atrowcol( self, s->row-1, s->col ) ? 1 : 0)
				+ (atrowcol( self, s->row+1, s->col ) ? 1 : 0);
		/* draw background (yellow, if possible) */
		if ( ! colordpy) {
			self->SetTransferMode( graphic::WHITE );
			self->FillRectSize( left, top, width, height, NULL );
		}
		else  { 
			self->SetForegroundColor( self->boardcolor );
			self->SetTransferMode( graphic::COPY );
			self->FillRectSize( left, top, width, height, NULL );
			self->SetFGColor( fgr, fgg, fgb );
		}
		/* draw vertical */
		int startend, stopend;
		self->SetTransferMode( graphic::BLACK );
		self->SetLineWidth( lthick );
		startend = (s->row == 0) ? topend(self) : y-radius ;
		stopend = (s->row == dobj->height-1) ? bottomend(self) 
				: y+radius+(lthick&1) ;
		if (startend > stopend)  stopend = startend + 1;
		const int x = self->xoff + self->spotwidth * s->col;
		self->MoveTo( x, startend-1 );
		self->DrawLineTo( x, stopend+1 );

		/* draw horizontal */
		startend = (s->col == 0) ? leftend(self) : x-radius ;
		stopend = (s->col == dobj->width-1) ? rightend(self) 
				: x+radius+(lthick&1) ;
		if (startend > stopend)  stopend = startend + 1;
		const int y = self->yoff + self->spotheight * s->row;
		self->MoveTo( startend-1, y );
		self->DrawLineTo( stopend+1, y );
	}

	if ((self->xhoshi&(1<<s->col)) != 0 
			&& (self->yhoshi&(1<<s->row)) != 0) {
		/* need a hoshi dot here */
		const int mult = 5;
		const int hr = mult*lthick + 1;
		self->FillOvalSize( x - mult*lthick/2 - 1, y- mult*lthick/2 - 1, 
				hr, hr, NULL );
	}

	/* draw center in stone color */
	int trad = radius;
	int tdiam = diam;
	switch (s->color) {
	case 'B':
		if (isselected && self->HasInputFocus) {
			/* draw circle in white to get white rim */
			(self)->SetTransferMode( graphic::WHITE );
			(self)->FillOvalSize( 
				x - radius - lthick/2, y - radius - lthick/2, 
				diam, diam, NULL );
			trad = radius - lthick;
			tdiam = diam - 2*lthick;
		}
		self->SetTransferMode( graphic::BLACK );
		(self)->FillOvalSize( x - trad - lthick/2, 
			y - trad - lthick/2, tdiam, tdiam, NULL );
		break;
	case 'W':
		if ( ! colordpy) {
			self->SetTransferMode( graphic::WHITE );
			(self)->FillOvalSize( x - radius - lthick/2, 
				y - radius - lthick/2, diam, diam, NULL );
		}
		else {
			self->SetForegroundColor("White", 0xFFFF, 0xFFFF, 0xFFFF);
			self->SetTransferMode( graphic::COPY );
			(self)->FillOvalSize( x - radius - lthick/2, 
				y - radius - lthick/2, diam, diam, NULL );
			self->SetFGColor( fgr, fgg, fgb );
		}
		break;
	case '/':
		if (s->note == 0)  trad = 0;
		else if (isselected && self->HasInputFocus)
			{}
		else if ('a' <= -s->note && 'z' >= -s->note)
			trad = self->spotwidth/3;
		else 
			trad -= 2*lthick;
		tdiam = 2*trad + ((lthick&1) ? 1 : 0);
		
		if (trad == 0) {}
		else if ( ! colordpy) {
			self->SetTransferMode( graphic::WHITE );
			(self)->FillOvalSize( x - trad - lthick/2, 
				y - trad - lthick/2, tdiam, tdiam, NULL );
		}
		else  {
			self->SetForegroundColor( self->boardcolor );
			self->SetTransferMode( graphic::COPY );
			(self)->FillOvalSize( x - trad - lthick/2, 
				y - trad - lthick/2, tdiam, tdiam, NULL );
			self->SetFGColor( fgr, fgg, fgb );
		}
		break;
	}

	int delta, deltay, deltax, fudge;
	char buf[25];

	/* draw annotation in contrasting color */
	if (s->color != 'B')
		self->SetTransferMode( graphic::BLACK );
	else	/* white on black */
		self->SetTransferMode( graphic::WHITE );
	self->SetLineWidth( 2*lthick );
	trad = mytrunc( radius * .7 );
	if (s->note == 0) {}
	else switch (-s->note) {
	default:
		/* display a letter or integer*/
		if (s->note < 0) {
			buf[0] = -s->note;  buf[1] = '\0';
		}
		else sprintf( buf, "%d",s->note );
		self->SetFont( self->notefont );
		self->MoveTo( x, y );
		self->DrawString( buf,
			graphic::BETWEENLEFTANDRIGHT 
				 |  graphic::BETWEENTOPANDBOTTOM );
		break;
	case '%':		/* diagonal cross */
		delta = mytrunc( trad/1.414 + 0.5 );
		self->MoveTo( x-delta, y-delta );
		self->DrawLineTo( x+delta, y+delta );
		self->MoveTo( x-delta, y +delta);
		self->DrawLineTo( x+delta, y-delta );
		break;
	case '+':		/* orthogonal cross */
		self->MoveTo( x-trad, y );
		self->DrawLineTo( x+trad, y );
		self->MoveTo( x, y +trad);
		self->DrawLineTo( x, y-trad );
		break;
	case '@':		/* circle */
		delta = mytrunc( .8 * trad );
		if ((delta&1) != (radius&1)) delta++;
		self->DrawOvalSize(  x - delta, y - delta, 2*delta, 2*delta );
		break;
	case '#':		/* square */
		delta = mytrunc( trad/1.414 + 0.5 );
		fudge = delta+lthick;	/* try to fill in corners */
		self->MoveTo( x-delta, y-fudge );		/* left */
		self->DrawLineTo( x-delta, y+fudge );
		self->MoveTo( x+delta, y-fudge );		/* right */
		self->DrawLineTo( x+delta, y+fudge ); 
		self->MoveTo( x-fudge, y-delta );		/* top */
		self->DrawLineTo( x+fudge, y-delta);
		self->MoveTo( x-fudge, y+delta );		/* bottom */
		self->DrawLineTo( x+fudge, y+delta );
		break;
	case '^':		/* triangle */
		deltay = trad/2;
		deltax = mytrunc( 1.732*trad/2.0 + 0.5 );
		self->MoveTo( x, y-trad );
		self->DrawLineTo( x+deltax, y+deltay );
		self->DrawLineTo( x-deltax, y+deltay);
		self->DrawLineTo( x, y-trad );
		break;
	case '-':		/* dash */
		self->MoveTo( x-trad, y );
		self->DrawLineTo( x+trad, y );
		break;
	case '&':		/* upside down triangle */
		deltay = trad/2;
		deltax = mytrunc( 1.732*trad/2.0 + 0.5 );
		self->MoveTo( x, y+trad );
		self->DrawLineTo( x+deltax, y-deltay );
		self->DrawLineTo( x-deltax, y-deltay);
		self->DrawLineTo( x, y+trad );
		break;
	case '~':		/* dash with line up */
		self->MoveTo( x-trad, y );
		self->DrawLineTo( x+trad, y );
		self->MoveTo( x, y );
		self->DrawLineTo( x, y-trad/2 );
		break;
	case '_':		/* dash with line down */
		self->MoveTo( x-trad, y );
		self->DrawLineTo( x+trad, y );
		self->MoveTo( x, y );
		self->DrawLineTo( x, y+trad/2 );
		break;
	}

	/* draw border in black */
	self->SetTransferMode( graphic::BLACK );
	self->SetLineWidth( lthick );
	if (isselected && self->HasInputFocus) {
		/* dotted border for the selected spot */
		char dotted[3];
		/* the following fails 
		 *	dotted[0] = dotted[1] = 2*lthick;
		 */
		dotted[0] = 2*lthick;
		dotted[1] = 2*lthick;
		dotted[2] = '\0';
		self->SetLineDash(dotted, 0, graphic::LineOnOffDash );
		(self)->DrawOvalSize(  x - radius, y - radius,
				diam-lthick, diam-lthick );
		self->SetLineDash(dotted, 0, graphic::LineSolid );
	}
	else if (s->color == 'W') {
		(self)->DrawOvalSize(  x - radius, y - radius, 
				diam-lthick, diam-lthick );
	}
}

/* RedrawView(self)
	redraw all stones with accnum greater than visibleaccnum 
	If visibleaccnum is 0, redraw grid as well.
	Input focus is indicated by a dotted circle at the currently 
	selected spot.
*/
	static void 
RedrawView(gofigview  *self) {
	if ( ! self->OnScreen) return;
	struct rectangle r;
	int left, top, vwidth, vheight;
	gofig *dobj = (gofig *)self->GetDataObject();
	long dpycls = (self)->DisplayClass();
	boolean colordpy = (dpycls & graphic::Color) != 0;
	unsigned edges;
	dobj->getedges( &edges );
	double fgr, fgg, fgb;

	if (colordpy)
		self->GetFGColor( &fgr, &fgg, &fgb );

	self->GetLogicalBounds( &r );	/* find rectangle bounds */
	rectangle_GetRectSize( &r, &left, &top, &vwidth, &vheight );

	if (self->visibleaccnum == 0) {
		/* redraw everything */

		/* draw background (yellow, if possible) */
		if ( ! colordpy) {
			self->SetTransferMode( graphic::WHITE );
			self->FillRectSize( left, top, vwidth, vheight, NULL );
		}
		else  { 
			self->SetForegroundColor( self->boardcolor );
			self->SetTransferMode( graphic::COPY );
			self->FillRectSize( left, top, vwidth, vheight, NULL );
			self->SetFGColor( fgr, fgg, fgb );
		}

		/* compute image parameters */
		if (edges&INDICESedge) {
			self->spotwidth = mytrunc( vwidth 
						/ (dobj->width+2*INDEXFRAC) );
			self->spotheight = mytrunc( vheight 
						/ (dobj->height+2*INDEXFRAC) );
		}
		else {
			self->spotwidth = vwidth / dobj->width;
			self->spotheight = vheight / dobj->height;
		}
		/* ensure a small border */
		if (vwidth - dobj->width * self->spotwidth < 4)
			self->spotwidth--;
		if (vheight - dobj->height * self->spotheight < 4)
			self->spotheight--;
		/* ensure ratio */
		if (self->spotwidth / WTOHRATIO / self->spotheight < 1.0) 
			self->spotheight = 
					mytrunc( self->spotwidth / WTOHRATIO );
		else if  (self->spotwidth / WTOHRATIO / self->spotheight > 1.0) 
			self->spotwidth = 
					mytrunc( self->spotheight * WTOHRATIO );
		if (self->spotwidth < 3) self->spotwidth = 3;
		if (self->spotheight < 3) self->spotheight = 3;
		/* compute offset to origin */
		self->xoff = left + (vwidth - dobj->width * self->spotwidth
				+ self->spotwidth + 1)/2;
		self->yoff = top + (vheight - dobj->height * self->spotheight
				+ self->spotheight + 1)/2;

		self->linethickness = self->spotwidth / 20;
		if (self->linethickness < 1) self->linethickness = 1;

		self->radius = (self->spotwidth-1)/2;
		if (self->radius < 2*self->linethickness+1) 
			self->radius = 2*self->linethickness+1;

		int i, j;
		int startend, stopend;
		/* draw verticals */
		self->SetTransferMode( graphic::BLACK );
		self->SetLineWidth( self->linethickness );
		startend = topend( self );
		stopend = bottomend( self );
		if (startend > stopend)  stopend = startend + 1;
		for (i = dobj->width; i--; ) {
			const int x = self->xoff + self->spotwidth * i;
			self->MoveTo( x, startend );
			self->DrawLineTo( x, stopend );
		}
		/* draw horizontals */
		startend = leftend( self );
		stopend = rightend( self );
		if (startend > stopend)  stopend = startend + 1;
		for (i = dobj->height; i--; ) {
			const int y = self->yoff + self->spotheight * i;
			self->MoveTo( startend, y );
			self->DrawLineTo( stopend, y );
		}

		/* put in hoshi dots */
		int xhoshi[4];			/* coords of hoshi points */
		int yhoshi[4];			/*     entry after last is zero */
		computeHoshi(xhoshi, (edges&LEFTedge) != 0, 
				(edges&RIGHTedge) != 0, dobj->width);
		computeHoshi(yhoshi, (edges&TOPedge) != 0, 
				(edges&BOTTOMedge) != 0, dobj->height);
		self->xhoshi = 0;
		self->yhoshi = 0;
		for (i = 0; xhoshi[i]>=0; i++) 
			self->xhoshi |= 1<<xhoshi[i];
		for (i = 0; yhoshi[i]>=0; i++) 
			self->yhoshi |= 1<<yhoshi[i];

		static struct stone hstone = {0, 0, 0, '/', 0};
		for (i = 0; xhoshi[i]>=0; i++) 
			for (j = 0; yhoshi[j]>=0; j++) {
				hstone.row = yhoshi[j];
				hstone.col = xhoshi[i];
				drawspot(self, &hstone, TRUE);
			}

		/* compute notefont */
		/* choose a font such that width of X is not greater than
				self->radius * MAXFONTFAC
			if window got bigger, increase font size only if 
			width of X is less than self->radius * MINFONTFAC
		*/
#define MAXFONTFAC 1.0
#define MINFONTFAC 0.8
		int fs = self->fontsz;
		if (self->notefont == NULL) {
			/* initially choose Font20 */
			int w, h;
			self->notefont = Font20;
			self->notefont->StringBoundingBox( self->GetDrawable(),
				"X", &w, &h);
			self->fontxw = w;
			self->fontsz = 20;
			fs = 20;
		}
		if (self->fontxw > self->radius*MAXFONTFAC) {
			/* notefont is too big.  get proportionately smaller font */
			fs = mytrunc( 1 + self->fontsz * self->radius * MAXFONTFAC
							/ self->fontxw );
			if (fs < 6)  fs = 6;
		}
		else if (self->fontxw < self->radius*MINFONTFAC) {
			/* notefont is too small, get proportionately bigger */
			fs = mytrunc( 1 + self->fontsz * self->radius * MAXFONTFAC
							/ self->fontxw );
			if (fs  > 24)  fs = 24;
		}
		if (fs != self->fontsz) {
			/* try font of size fs and successively smaller */
			int w, h;
			fontdesc *newfont;
			while (TRUE) {
				newfont  = fontdesc::Create("andysans", 
							fontdesc_Bold, fs);
				newfont->StringBoundingBox(
						self->GetDrawable(), "X", 
						&w, &h);
				if (fs <= 6 || w <= self->radius * MAXFONTFAC) 
					break;   /* use newfont */
				fs--;
			}
			self->notefont = newfont;
			self->fontxw = w;
			self->fontsz = fs;
		}

		if (edges&INDICESedge) {
			char buf[5];
			self->SetFont( self->notefont );

			/* display indices at left and right */
			int leftx = self->xoff 
				- mytrunc( (INDEXFRAC/2+.5) * self->spotwidth );
			int rightx = self->xoff + self->spotwidth*(dobj->width-1)
				+ mytrunc( (INDEXFRAC/2+.5) * self->spotwidth );
			for (i = 1; i <= dobj->height; i++) {
				int y = self->yoff + (dobj->height - i) * self->spotheight;
				sprintf( buf, "%d", i);
				self->MoveTo( leftx, y );
				self->DrawString( buf,
					graphic::BETWEENLEFTANDRIGHT 
					|  graphic::BETWEENTOPANDBOTTOM );
				self->MoveTo( rightx, y );
				self->DrawString( buf,
					graphic::BETWEENLEFTANDRIGHT 
					|  graphic::BETWEENTOPANDBOTTOM );
			}

			/* display indices at top and bottom */
			int topy = self->yoff 
				- mytrunc( (INDEXFRAC/2+.5) * self->spotheight );
			int bottomy = self->yoff + self->spotheight*(dobj->height-1)
				+ mytrunc( (INDEXFRAC/2+.5) * self->spotheight );
			buf[1] = '\0';
			static char indexletters[] =  /* NO 'I' IS USED HERE */
				 "ABCDEFGHJKLMNOPQRSTUVWXYZ";
			for (i = 0; i < dobj->width; i++) {
				int x = self->xoff + i * self->spotwidth;
				buf[0] = indexletters[i];
				self->MoveTo( x, topy );
				self->DrawString( buf,
					graphic::BETWEENLEFTANDRIGHT 
					|  graphic::BETWEENTOPANDBOTTOM );
				self->MoveTo( x, bottomy );
				self->DrawString( buf,
					graphic::BETWEENLEFTANDRIGHT 
					|  graphic::BETWEENTOPANDBOTTOM );
			}
		}

	}

	/* display stones*/
	int i;
	for (i = dobj->nstones(); i--; ) {
		struct stone *s = &((*dobj)[i]);
		if (self->visibleaccnum == 0 || s->accnum > self->visibleaccnum) 
			drawspot(self, s, self->visibleaccnum == 0);
	}

	self->visibleaccnum = dobj->accnum;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
 *	User Interface 
 *	
 *	Routines called from keystrokes or menu
 *
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static struct proctable_Entry *DoNothingProc, *SetClickProc, *SetNoteProc;


	static void
modifymenuentry( menulist *m, const char *f, const char *prior, const char *post,
			struct proctable_Entry *proc, char code ) {
	char buf[50];
	sprintf( buf, f, prior );
	m->DeleteFromML( buf );
	sprintf( buf, f, post );
	m->AddToML( buf, proc, code, 0 );
}

	static const char *
actionmenus(enum actionopts a, char *c) {
	switch (a) {
	case  Alternate: *c = 'A';  return "Gofig,Black-White%s~10";
	case  White: *c = 'W';  return "Gofig,White%s~12";
	case  Black: *c = 'B';  return "Gofig,Black%s~14";
        default:
	case  Empty: *c = 'E';  return "Gofig,Empty%s~16";
	case  Select: *c = 'S';  return "Gofig,Select Only%s~18";
	}
}
	static const char *
notemenus(enum noteopts a, char *c) {
	switch (a) {
	case  Letter: *c = 'a';  return "Gofig,Auto a b c ...%s~20";
	case  Number: *c = '0';  return "Gofig,Auto 1 2 3 ...%s~22";
	default:
	case  None: *c = '-';  return "Gofig,No Auto Notes%s~24";
	}
};

	static void 
ClickFirst() {
	message::DisplayString(NULL, 0, "First click on an intersection");
}

/* PostMyMenus(self)
	change menus if smartlevel != CurrSmart
	post menus if have input focus
*/
	static void
PostMyMenus(gofigview *self) {
	static char code = ' ';
	const char *format;

	if (self->hitaction != self->menuhitaction) {
		/* move <= from menuhitaction to hitaction */
		format = actionmenus(self->menuhitaction, &code);
		modifymenuentry(self->Menus, format, "  <=", "", 
				SetClickProc, code);
		self->menuhitaction = self->hitaction;
		format = actionmenus(self->hitaction, &code);
		modifymenuentry(self->Menus, format, "", "  <=", 
				DoNothingProc, code);
	}
	if (self->hitnote != self->menuhitnote) {
		/* move <= from menuhitnote to hitnote */
		format = notemenus(self->menuhitnote, &code);
		modifymenuentry(self->Menus, format, "  <=", "", 
				SetNoteProc, code);
		self->menuhitnote = self->hitnote;
		format = notemenus(self->hitnote, &code);
		modifymenuentry(self->Menus, format, "", "  <=", 
				DoNothingProc, code);
	}
	if (self->HasInputFocus) {
		DEBUG(("Posting menus %d %d\n", 
				self->menuhitnote, self->menuhitaction));
		self->PostMenus(self->Menus);
	}
}

	static void
HitnoteOff(gofigview *self) {
	if (self->hitnote != None) {
		self->hitnote = None;
		PostMyMenus(self);
		message::DisplayString(NULL, 0, 
				"Turned off automatic note mode");
	}
}


/* first click puts black or white, alternating
		just set mode  (not called from keys)
*/
	static void 
SetClickMode(gofigview *self, int code) {
	switch(code) {
	case 'A':
		self->hitaction = Alternate;
		break;
	case 'W':
		self->hitaction = White;
		break;
	case 'B':
		self->hitaction = Black;
		break;
	case 'E':
		self->hitaction = Empty;
		break;
	case 'S':
		self->hitaction = Select;
		break;
	}
	PostMyMenus(self);

#if 0
	/* deselect the current spot: avoid cycling its color */
	
	gofig *dobj = (gofig *)self->GetDataObject();
	struct stone *s = selectedspot(self);
	if (s != NULL) {
		dobj->changed(s);
		self->curspotrow  = -1;
	}
#endif

}

/* choose to put notes or letters on subsequent stones 
	ch - 'a': letters  '0': digits
*/
	static void 
SetNoteMode(gofigview *self, int code) {
	switch(code) {
	case 'a':
		self->hitnote = Letter;
		break;
	case '0':
		self->hitnote = Number;
		break;
	case '-':
		self->hitnote = None;
		break;
	}
	PostMyMenus(self);
}

	static void 
DoNothing(gofigview *self, int code) {
}

/* put stone or empty in selected spot 
*/
	static void 
PutStone(gofigview *self, int code) {
	gofig *dobj = (gofig *)self->GetDataObject();
	struct stone *s = selectedspot(self);
	if (s == NULL) {ClickFirst(); return;}
	if (s->color == '/') {
		/* put number or letter if called for */
		if (self->hitnote == Number)
			s->note = self->nextnum++;
		else if (self->hitnote == Letter)
			s->note = -(self->nextlet++);
	}
	s->color = code;
	setnextcolor(self, code);
	HitnoteOff(self);
	dobj->changed(s);
}

/* Character from keyboard
	letter - put as note
	digit - put as note or extend digit string note
	backspace, DEL:  
		if no note, remove stone
		if letter note, remove it
		if digit note, remove rightmost
*/
	static void 
CharNote(gofigview *self, int code) {
	gofig *dobj = (gofig *)self->GetDataObject();
	struct stone *s = selectedspot(self);
	if (s == NULL) {ClickFirst(); return;}

	if (isdigit(code))
		if (s->note > 0)
			s->note = 10*s->note + code - '0';
		else
			s->note = code - '0';
	else if (code == '\b' || code == '\177') {
		if (s->note == 0)
			s->color = '/';
		else if (s->note < 0)
			s->note = 0;
		else /* remove last digit */
			s->note = (s->note - s->note%10) / 10;
	}
	else 
		s->note = -code;

	if (s->note >= 499)
		s->note = 499;
	if (-s->note >= 'a' && -s->note <'n')
		self->nextlet = (-s->note)+1;
	else if (s->note > 0)
		self->nextnum = s->note+1;
	HitnoteOff(self);
	dobj->changed(s);
}

/* prompt for annotation for selected spot
*/
	static void 
EnterNote(gofigview *self, int code) {
	gofig *dobj = (gofig *)self->GetDataObject();
	struct stone *s = selectedspot(self);

	/* posting menus should not be necessary, but the 'A' on the menu 
		for EnterNote goes away ?!?! */
	PostMyMenus( self );

	if (s == NULL) {ClickFirst(); return;}

	char buffer[20];
	if (message::AskForString(self, 0, 
			"Number, letter, or one of ^ # @ % + - [none]", 
			"", buffer, sizeof(buffer)) == -1)
		return;
	if (buffer[0] == '\0') 
		s->note = 0;
	else if (buffer[0] >= '0' && buffer[0] <= '9') {
		s->note = atoi(buffer);
		self->nextnum = s->note+1;
	}
	else if (buffer[0] >= 'a' && buffer[0] <= 'n') {
		s->note =  -buffer[0];
		self->nextlet = buffer[0]+1;
	}
	else 
		s->note = -buffer[0];
	HitnoteOff(self);
	dobj->changed(s);
}

	static void 
SetLayout(gofigview *self, int code) {
	gofig *dobj = (gofig *)self->GetDataObject();
	int rows, cols;
	unsigned edges;
	dobj->getedges( & edges );

	char buffer[20];
	char prompt[50];
	sprintf(prompt, "%s [%d %d %s%s%s%s]: ", 
			"#rows #cols  LRTBAN (edges) I (indices)",
			dobj->height, dobj->width, 
			(edges&LEFTedge) ? "L" : "",
			(edges&RIGHTedge) ? "R" : "",
			(edges&TOPedge) ? "T" : "",
			(edges&BOTTOMedge) ? "B" : "");
	if (message::AskForString(self, 0, prompt, "", 
				buffer, sizeof(buffer)) == -1)
		return;
	if (sscanf(buffer, " %d %d", &rows, &cols) == 2) {
		if (cols < 1) cols = 1;  else if (cols > 25) cols = 25;
		if (rows < 1) rows = 1;  else if (rows > 25) rows = 25;
		dobj->setdimensions(cols, rows);
	}
	unsigned newedges = 0;
	int edgeset = 0;
	char *bx;
	for (bx = buffer; ; bx++)
		if ( ! isdigit( *bx ) && ! isspace( *bx )) break;
	for ( ; *bx; bx++) {
		switch (*bx) {
		case 'L': case 'l': newedges |= LEFTedge; edgeset++; break;
		case 'R': case 'r': newedges |= RIGHTedge; edgeset++; break;
		case 'T': case 't': newedges |= TOPedge; edgeset++; break;
		case 'B': case 'b': newedges |= BOTTOMedge; edgeset++; break;
		case 'I': case 'i': newedges |= INDICESedge; edgeset++; break;
		case 'A': newedges = LEFTedge | RIGHTedge 
				| TOPedge | BOTTOMedge;  edgeset++; break;
		case 'N': newedges = 0; edgeset++; break;
		case ' ': case '\t': 	break;
		default: 
			sprintf(prompt, "Unexpected character: '%c'", *bx);
			message::DisplayString(NULL, 0, prompt);
			dobj->NotifyObservers(gofig_SIZECHANGED);
			return;
		}
	}
	if (edgeset)
		dobj->setedges( newedges );
	HitnoteOff(self);
	dobj->NotifyObservers(gofig_SIZECHANGED);
}

	static void 
SetPrintSize(gofigview *self, int code) {
	gofig *dobj = (gofig *)self->GetDataObject();
	char buffer[20];
	char prompt[50];
	double newcolwidth;

	long wpts, hpts;
	self->DesiredPrintSize( pagew, pageh, view::NoSet, &wpts, &hpts );
	sprintf( prompt, "With spacing %0.3f, prints %0.1f by %0.1f inches.  %s", 
			dobj->printcolwidth/1000.0,
			wpts/72.0, hpts/72.0, 
			"New spacing: " );
	if (message::AskForString( self, 0, prompt, "", 
				buffer, sizeof(buffer) ) == -1)
		return;
	if (sscanf( buffer, " %lf", &newcolwidth ) != 1 ) {
		message::DisplayString( NULL, 0, "Unchanged.  No value found." );
		return;
	}
	if (newcolwidth < 5.0 || newcolwidth > 100.0) {
		message::DisplayString( NULL, 0, "Unchanged.  Value not in range 5.0 ... 100.0." );
		return;
	}

	dobj->printcolwidth = mytrunc( newcolwidth * 1000.0 );

	self->DesiredPrintSize( pagew, pageh, view::NoSet, &wpts, &hpts );
	sprintf( prompt, "Now prints %0.1f by %0.1f inches", wpts/72.0, hpts/72.0 );
	message::DisplayString( NULL, 0, prompt );
}

/* move selected spot
	ch encodes direction: A - up;  B -  down;  C - right;  D - left
	reset curspotrow,col
	note that dobj->changed() is necessary to get display right
*/
	static void 
MoveSpot(gofigview *self, int code) {
	gofig *dobj = (gofig *)self->GetDataObject();
	struct stone *s = selectedspot(self);
	int row, col;
	if (s == NULL) {
		/* if no spot, start in the middle */
		row = dobj->height/2;
		col = dobj->width/2;
	}
	else {
		dobj->changed( s );  /* losing selectedness */
		row = self->curspotrow;
		col = self->curspotcol;
		switch (code) {
		case 'A':  row--;  if (row < 0) row = dobj->height-1;  break;
		case 'B':  row++;  if (row >= dobj->height) row = 0;  break;
		case 'C':  col++;  if (col >= dobj->width) col = 0;  break;
		case 'D':  col--;  if (col < 0) col = dobj->width-1;  break;
		}
	}
	self->curspotrow = row;
	self->curspotcol = col;
	dobj->changed( selectedspot( self ));  /* gain selectedness */
}

/*
	copy the entire diagram to the cut buffer
	AS ASCII
*/
	static void 
CopyCommand(gofigview *self, int code) {
	gofig *dobj = (gofig *)self->GetDataObject();
	if (dobj == NULL) return;
	FILE *copyFile;

	copyFile = ((self)->GetIM())->ToCutBuffer();
	(dobj)->WriteASCII( copyFile );
	((self)->GetIM())->CloseToCutBuffer( copyFile );
}

/*
	reads replacement contents from cutbuffer
	cut buffer is probably ASCII, but might be gofig

	if first char is \, assume is gofig object
*/
	static void 
ReplaceCommand(gofigview *self, int code) {
	gofig *dobj = (gofig *)self->GetDataObject();
	if (dobj == NULL) return;
	FILE *inf;
	int c;
	int id;

	inf = ((self)->GetIM())->FromCutBuffer();
	c = getc(inf);
	if (c == '\\') {
		/* read gofig object */
		if (fscanf( inf, "begindata{gofig, %d}\n", &id) != 1)
			id = -1;
	}
	else {
		ungetc( c, inf );
		id = 0;		/* read ASCII */
	}

	if (id >= 0)
		(dobj)->Read( inf, id );

	((self)->GetIM())->CloseFromCutBuffer( inf );
	HitnoteOff(self);
}

	static void
ToggleDebug(gofigview  *self, long  rock)  {
	debug = ! debug;
	printf("debug is now %d\n", debug);  fflush (stdout);
}


/*  Menus and Keymaps
	The menus and keymaps are initialized to those of
	    EmbeddedMenus and EmbeddedKeymap in InitializeObject.

	Debugging can be toggled with the key sequence  ESC-^D -D

*/
static menulist  *EmbeddedMenus;
static keymap  *EmbeddedKeymap;

static struct bind_Description EmbeddedBindings[]={
	{"gofig-invert-debug", "\033\004D",	0,	/* ESC - ^D - D */
			0, 0, 0, 			/* no menu entry */
			(proctable_fptr)ToggleDebug,
			"Toggle the gofigview debug flag", NULL},

/* menu sets */
	{"gofigview-set-click-mode", "O", 'W',
		"Gofig,White~12", 'W', 0, 
		(proctable_fptr)SetClickMode, 
		"set action for clicking on stones (A, B, W, E, or S)", NULL},
	{"gofigview-set-click-mode", "A", 'A',
		"Gofig,Black-White  <=~10", 'A', 0, 
		NULL, NULL, NULL},
	{"gofigview-set-click-mode", "X", 'B',
		"Gofig,Black~14", 'B', 0, 
		NULL, NULL, NULL},
	{"gofigview-set-click-mode", "E", 'E',
		"Gofig,Empty~16",'E', 0, 
		NULL, NULL, NULL},
	{"gofigview-set-click-mode", "S", 'S',
		"Gofig,Select~18",'S', 0, 
		NULL, NULL, NULL},

	{"gofigview-set-note-mode", 0, 0,
		"Gofig,Auto a b c ...~20", 'a', 0, 
		(proctable_fptr)SetNoteMode, 
		"chooses note sequence for subsequent stones (a, 0, or -)", NULL},
	{"gofigview-set-note-mode", 0, 0,
		"Gofig,Auto 1 2 3 ...~22", '0', 0, 
		NULL, NULL, NULL},
	{"gofigview-do-nothing", 0, 0,
		"Gofig,No Auto Notes  <=~24", '-', 0, 
		NULL, NULL, NULL},

/* other menu entries */

	{"gofigview-prompt-for-note", "N", 0,
		"Gofig,Enter Note~30", 0, 0, 
		(proctable_fptr)EnterNote, 
		"prompts for note to put in stone", NULL},

	{"gofigview-prompt-for-board-layout", "L", 0,
		"Gofig,Board layout~40", 0, 0, 
		(proctable_fptr)SetLayout, 
		"prompts for size and edges of board", NULL},
	{"gofigview-prompt-for-print-size", "P", 0,
		"Gofig,Printed size~42", 0, 0, 
		(proctable_fptr)SetPrintSize, 
		"prompts for printing size parameter", NULL},

	{"gofigview-copy", "\027", 0,		/* ctl-W */
		"Copy ~2", 0, 0, 
		(proctable_fptr)CopyCommand, 
		"copy the go diagram as ASCII to the cut buffer", NULL},	
	{"gofigview-copy", "\033w", 0,		/* esc-w */
		0,  0,  0, NULL, NULL, NULL},
	{"gofigview-replace", "\031", 0,		/* ctl-Y */
		"Replace~4", 0, 0, 
		(proctable_fptr)ReplaceCommand, 
		"replaces contents from cut buffer", NULL},
	{"gofigview-replace", "\033y", 0,		/* esc-y */
		0,  0,  0, NULL, NULL, NULL},

/* keyboard commands */

	{"gofigview-put-stone", "o", 'W',
		0,  0,  0, 
		(proctable_fptr)PutStone, 
		"put stone or empty in selected spot (arg is 'B' 'W' or '/')", 
		NULL},
	{"gofigview-put-stone", "x", 'B',
		0,  0,  0, NULL, NULL, NULL},
	{"gofigview-put-stone", " ", '/',
		0,  0,  0, NULL, NULL, NULL},

	{"gofigview-move-spot", "\033A", 'A',	/* UP arrow*/
		0,  0,  0, 
		(proctable_fptr)MoveSpot, 
		"move selected spot", NULL},
	{"gofigview-move-spot", "\033B", 'B',	/* DOWN arrow */
		0,  0,  0, NULL, NULL, NULL},
	{"gofigview-move-spot", "\033C", 'C',	/* RIGHT arrow */
		0,  0,  0, NULL, NULL, NULL},
	{"gofigview-move-spot", "\033D", 'D',	/* LEFT arrow */
		0,  0,  0, NULL, NULL, NULL},
	{"gofigview-move-spot", "\020", 'A',	/* ^P - UP */
		0,  0,  0, NULL, NULL, NULL},
	{"gofigview-move-spot", "\016", 'B',	/* ^N - DOWN */
		0,  0,  0, NULL, NULL, NULL},
	{"gofigview-move-spot", "\002", 'C',	/* ^B - RIGHT */
		0,  0,  0, NULL, NULL, NULL},
	{"gofigview-move-spot", "\006", 'D',	/* ^F - LEFT */
		0,  0,  0, NULL, NULL, NULL},
	{"gofigview-move-spot", "m", 'A',   	/* m - UP */
		0,  0,  0, NULL, NULL, NULL},
	{"gofigview-move-spot", ",", 'B',     	/* n - DOWN */
		0,  0,  0, NULL, NULL, NULL},
	{"gofigview-move-spot", ".", 'C',     	/* . - RIGHT */
		0,  0,  0, NULL, NULL, NULL},
	{"gofigview-move-spot", "n", 'D',     	/* , - LEFT */
		0,  0,  0, NULL, NULL, NULL},

	{"gofigview-char-to-note", "a", 'a',
		0,  0,  0, 
		(proctable_fptr)CharNote, 
		"put character in selected spot", NULL},

	{"gofigview-print-dobj", "D", 0, 0,  0,  0, 
		(proctable_fptr)printdata, 
		"print contents of dobj to stdout", NULL},

	NULL
};

	boolean
gofigview::InitializeClass() {
	/* establish board color */
	const char *colorstr;
	colorstr = environ::GetProfile("BoardColor");
	if (colorstr == NULL) colorstr = "#FFD29B";   /* 255, 210, 155 */
	BoardColor = strdup(colorstr);

	EmbeddedMenus = new menulist;
	EmbeddedKeymap = new keymap;
	if (EmbeddedMenus == NULL || EmbeddedKeymap == NULL)
		return FALSE;
	bind::BindList(EmbeddedBindings, EmbeddedKeymap, EmbeddedMenus,
				&gofigview_ATKregistry_ );

	struct proctable_Entry *charnote =
		 proctable::DefineProc("gofigview-char-to-note", 
			(proctable_fptr)CharNote, &gofigview_ATKregistry_,
			 "gofigview", 
			"put letter in selected spot");
	static const char *chars = "abcdefghijkl^+%@#-0123456789\b\177";
	const char *lx;
	char str[2];
	str[1] = '\0';
	for (lx = chars; (str[0] = *lx); lx++) 
		EmbeddedKeymap->BindToKey(str, charnote, str[0]);

	/* set globals */
	DoNothingProc =
		 proctable::DefineProc("gofigview-do-nothing", 
			(proctable_fptr)DoNothing, &gofigview_ATKregistry_,
			 "gofigview", 
			"does nothing");
	SetClickProc =
		 proctable::DefineProc("gofigview-set-click-mode", 
			(proctable_fptr)SetClickMode, &gofigview_ATKregistry_,
			 "gofigview", 
			"set action for clicking on stones (A, B, W, E, or S)");
	SetNoteProc  =
		 proctable::DefineProc("gofigview-set-note-mode", 
			(proctable_fptr)SetNoteMode, &gofigview_ATKregistry_,
			 "gofigview", 
			"chooses note sequence for subsequent stones (a, 0, or -)");

	Font20 = fontdesc::Create("andysans", fontdesc_Bold, 24);
	return TRUE;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
 *	
 *	Override methods
 *	
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
gofigview::gofigview() {
	ATKinit;

	Menus = (EmbeddedMenus)->DuplicateML( this );
	Keystate = keystate::Create( this, EmbeddedKeymap );
	boardcolor = NULL;

	if (Menus == NULL || Keystate == NULL)
		THROWONFAILURE( FALSE);

	OnScreen = FALSE;
	embedded = TRUE;
	HasInputFocus = FALSE;
	updaterequested = FALSE;
	visibleaccnum = 0;
	curspotrow = -1;	/* none selected */
	notefont = NULL;
	fontxw = 0;
	fontsz = 0;

	hitaction = Alternate;
	hitnote = None;
	menuhitaction = Alternate;	/* established in EmbeddedMenus */
	menuhitnote = None;

	nextcolor = 'B';
	nextnum = 1;
	nextlet = 'a';

	THROWONFAILURE( TRUE);
}

gofigview::~gofigview() {
	ATKinit;

	delete Menus;
	delete Keystate;
}

	void
gofigview::ObservedChanged(observable  *dobj, long  status) {
	if (status == observable::OBJECTDESTROYED) 
		/* do not call wantupdate in this case,
			it will bomb the program */
		{}
	else if (status == gofig_DATACHANGED) {
		if ( ! updaterequested) 
			WantUpdate( this );
		updaterequested = TRUE;
	}
	else if (status == gofig_SIZECHANGED) {
		int width = ((gofig *)dobj)->width;
		int height = ((gofig *)dobj)->height;

		if (curspotrow >= width || curspotcol >= height)
			curspotrow = -1;
		if (previousrow >= width || previouscol >= height)
			previousrow = -1;
		if (stonewheredown.row >= width 
					|| stonewheredown.col >= height)
			stonewheredown.row = -1;

		if ( ! updaterequested) 
			WantUpdate( this );
		updaterequested = TRUE;
		visibleaccnum = 0;
		WantNewSize(this);
	}
}


/* DO NOT DO screen operations in ReceiveInputFocus */

	void 
gofigview::ReceiveInputFocus() {
	ENTER(gofigv_ReceiveInputFocus);

	Keystate->next = NULL;
	PostKeyState( Keystate );
	PostMenus( Menus);

	gofig *dobj = (gofig *)GetDataObject();
	struct stone *sel = selectedspot( this );
	if (sel) dobj->changed(sel);

	HasInputFocus = TRUE;
	LEAVE(gofigv_ReceiveInputFocus);
}

/* DO NOT DO screen operations in ReceiveInputFocus,
	but DO remove any defined cursor */

	void 
gofigview::LoseInputFocus() {
	ENTER(gofigview_LoseInputFocus);
	HasInputFocus = FALSE;
	gofig *dobj = (gofig *)GetDataObject();
	struct stone *sel = selectedspot( this );
	if (OnScreen && sel) dobj->changed(sel);
	LEAVE(gofigview_LoseInputFocus);
}

/* gofigview__FullUpdate(self, type, left, top, width, height)
	Redraws the entire image.  (The last four arguments are only used in 
	case the 'type' is a PartialRedraw;  they specify which part to update.)
*/
	void 
gofigview::FullUpdate(enum view::UpdateType   type, 
		long   left , long   top , long   width , long   height) {
	DEBUG(("FullUpdate(%d, %ld, %ld, %ld, %ld)\n", type, 
		left, top, width, height));
	if (type == view::Remove  
			||  GetLogicalWidth() == 0 
			|| GetLogicalHeight() == 0) {
		/* view::Remove means the view has left the screen.
			A zero dimension means the view is not visible */
		OnScreen = FALSE;
		return;
	}
	if (type != view::FullRedraw && type != view::LastPartialRedraw)
		return;
	/* we continue only for a FullRedraw or 
		the last of a sequence of PartialRedraw requests.  */

	if (boardcolor == NULL)
		boardcolor = GetIM()->CreateColor(BoardColor, 0, 0, 0);

	OnScreen = TRUE;
	visibleaccnum = 0;
	RedrawView(this);
	PostMyMenus( this );		/* to get keycode equivs in menus ?!?!? */
	updaterequested = FALSE;
	LEAVE(gofigview_FullUpdate);
}


	void 
gofigview::Update() {
	ENTER(gofigview_Update);
	if (! OnScreen) return;
	RedrawView(this);
	updaterequested = FALSE;
	LEAVE(gofigview_Update);
}

	view *
gofigview::Hit(enum view::MouseAction   action, 
			long   x , long   y , long   num_clicks) {
	gofig *dobj = (gofig *)GetDataObject();
	boolean newselection = FALSE;
	DEBUG(("Hit at (%ld, %ld) type %d\n", x, y, action));
	if (action == view::NoMouseEvent)
		return (view *)this;
	if (! OnScreen) return NULL;

	/* map hit coords to row, column */
	int hitcol = mytrunc( (x - xoff) * 1.0 / spotwidth + 0.5 );
	int hitrow = mytrunc( (y - yoff) * 1.0 / spotheight + 0.5 );
	struct stone *sel = selectedspot( this);
	struct stone *where = atrowcol( this, hitrow, hitcol );

	/* map hit to board range */
	if (hitrow < 0) hitrow = 0;
	else if (hitrow >= dobj->height) 
		hitrow = dobj->height-1;
	if (hitcol < 0) hitcol = 0;
	else if (hitcol >= dobj->width) 
		hitcol = dobj->width-1;

	/* change selection if warranted */
	if ( ! sel || sel->row != hitrow || sel->col != hitcol) {
		if (sel) dobj->changed(sel);
		curspotrow = hitrow;
		curspotcol = hitcol;
		sel = selectedspot( this );
		dobj->changed( sel );
		newselection = TRUE;
	}

	if ( ! HasInputFocus) {
		/*must be a button-down
			either button merely selects the down spot */
		WantInputFocus( this );
	}
	else if (action == view::RightDown) { /* selection already moved */}
	else if (action == view::LeftDown) {
		/* save conditions at time of mouse down */
		if (where != NULL) 
			stonewheredown = *where;
		else{
			stonewheredown.row = hitrow;
			stonewheredown.col = hitcol;
			stonewheredown.color = '/';
			stonewheredown.note = 0;
		}

		if (! newselection) {
			/* Left mouse down at selectedspot, "multiple click":
				toggle color empty->black, 
					black->white, white->empty
				do not modify note
			*/
			switch (sel->color) {
			case '/': sel->color = 'B'; break;
			case 'B': sel->color = 'W'; break;
			case 'W': sel->color = '/'; break;
			}
			setnextcolor(this, sel->color);
			dobj->changed( sel );
		}
		else {
			/* left down at other than previously selected spot
				Alternate: put nextcolor
				White, Black, Empty: put corresponding color
			*/
			switch (hitaction) {
			case Alternate: sel->color = nextcolor; break;
			case Black: sel->color = 'B';  break;
			case White: sel->color = 'W';  break;
			case Empty: sel->color = '/';  break;
			case Select: break;
			}
			setnextcolor(this, sel->color);
			if (hitaction != Select) 
				sel->note = 0;

			/*  put note per hitnote */
			switch (hitnote) {
			case Number:  sel->note = nextnum++;  	break;
			case Letter:  sel->note = -nextlet;
				if (nextlet < 'n') nextlet++;	break;
			case None:   break;
			}
		}
		downstonemodified = TRUE;
	}
	else if (hitrow != previousrow || hitcol != previouscol) {
		/*mouse has moved and the action is one of  
				rightmove, rightup, leftmove, leftup:
		       if modified spot where the down was,
				restore from stonewheredown */
		if (downstonemodified) {
			downstonemodified = FALSE;
			where = atrowcol( this, stonewheredown.row, 
				stonewheredown.col );
			if (where) {
				where->color = stonewheredown.color;
				where->note = stonewheredown.note;
				dobj->changed( where );
			}
		}
	}
	else {/* ignore small moves */}

	/* remember where mouse was */
	previousrow = hitrow;
	previouscol = hitcol;
	LEAVE(::Hit);
	return (view *)this;
}

	view::DSattributes
gofigview::DesiredSize(long  width, long  height, enum view::DSpass  pass, 
			long  *desiredWidth, long  *desiredHeight) {
	DEBUG(("DesiredSize(...%ld, %ld, %d...)\n", width, height, pass));
	gofig *dobj = (gofig *)GetDataObject();
	unsigned edges;
	dobj->getedges( & edges );
	double addinx = (edges&INDICESedge) ? 2.0*INDEXFRAC : 0.0;
	double colwidth;
	int hres = GetHorizontalResolution(), vres = GetVerticalResolution();

	if (pass == view::NoSet) {
		colwidth  = dobj->printcolwidth * hres / 72000.0 
				* SCREENEXPAND;
	}
	else if (pass == view::WidthSet) 
		colwidth = width / (dobj->width + addinx);
	else /* pass == view::HeightSet */ 
		colwidth = WTOHRATIO * height * hres 
			/ (vres * (dobj->height + addinx));

	*desiredWidth = mytrunc( colwidth * (dobj->width + addinx));
	*desiredHeight = mytrunc( colwidth * vres * (dobj->height + addinx)
				/ (WTOHRATIO * hres) );
	DEBUG(("Leave DesiredSize: %ld x %ld\n", 
			*desiredWidth, *desiredHeight));
	return view::HeightFlexible | view::WidthFlexible;
}

/* # # # # # # # # # # # # # # 
 *	PRINTING	
 *  # # # # # # # # # # # # #  */

/* functions for drawing stones */
	static const char * const
stonefuncs[]  =  {
	"", 
	"godict begin",
	"",
	"% basic parameters",
	"/Black {0 0 0 setrgbcolor} def",
	"/White {1 1 1 setrgbcolor} def",
	"/BoardColor {1 1 1 setrgbcolor} def  ",
	"%/BoardColor {.94 .90 .55 setrgbcolor} def  % for color printer (burlywood1)",
	"/linethick .04 def	% linethickness",
	"",
	"%derived parameters",
	"/bscolor /Black load def			% black stone color",
	"/bsnotecolor  /White load def		% color of notes on black stones",
	"/bsradius  .5 def				% black stone radius",
	"",
	"/wscolor /White load def			% white stone color",
	"/wsedgecolor  /Black load def		% color of whiteestone edge",
	"/wsnotecolor  /Black load def		% color of notes on white stones",
	"/wsthick  linethick 5 mul 4 div def		% white stone edge thickness",
	"/wsradius bsradius wsthick 2 div sub def   	%white stone radius",
	"",
	"/linecolor /Black load def			% color of lines on board",
	"% /linethick   defined above",
	"/edgethick linethick 1.5 mul def		% thickness of edge lines",
	"",
	"/hoshicolor /linecolor load def		% color of hoshi dot",
	"/hoshiradius  linethick 3 mul def		% radius of hoshi dot",
	"",
	"/boardnotecolor /Black load def",
	"/notethick linethick 2 mul def		% thickness of lines in symbols",
	"/noteaura .15 def				% erase lines closer to letters than this",
	"/symradius wsradius .7 mul def		% radius of symbols",
	"/letht .53 def					% height of letters and digits",
	"/notew .84 def				% width of multidigit numbers",
	"/corner  2 sqrt 2 div symradius mul def	% x&y coords of end of diagonal cross",
	"/trix 3 sqrt 2 div symradius mul def		% x for triangle bottom corners",
	"",
	"/strbounds {		% start with string on stack.  leave  llx urx ury lly",
	"	gsave ",
	"	newpath 0 0 moveto ",
	"	false charpath flattenpath pathbbox  % llx lly urx ury",
	"	3 -1 roll				% llx urx ury lly",
	"	grestore",
	"} def",
	"",
	"% define fonts for notes in stones",
	"	/Helvetica-Bold  findfont  setfont",
	"% onedigitfont is scaled so height of (8) is letht",
	"	(8) strbounds sub  /dy exch def   pop pop",
	" /onedigitfont",
	"	currentfont  letht dy div dup neg [ 1 0 0 1 0 0 ] scale",
	"	makefont  def",
	"% twodigitfont is same height but scaled to fit (88) in notew width",
	"	(88) strbounds   pop pop    exch sub  /dx exch def",
	"/twodigitfont  onedigitfont  [ notew dx div   0 0  1 0 0 ]  makefont  def",
	"	(111) strbounds  pop pop    exch sub  /dx exch def",
	"/threedigitfont   onedigitfont  [ notew dx div   0 0  1 0 0 ]  makefont  def",
	"",
	"(%) {			% draw a diagonal cross",
	"	moveto  corner neg dup rmoveto ",
	"	corner 2 mul ",
	"	dup dup rlineto ",
	"	dup neg  0 rmoveto ",
	"	dup neg rlineto",
	"} def",
	"",
	"(+) {			% draw an orthogonal cross",
	"	moveto  symradius neg 0 rmoveto  symradius 2 mul  0 rlineto",
	"	symradius neg dup rmoveto 0 symradius 2 mul rlineto",
	"	",
	"} def",
	"",
	"(#) {			% draw a square",
	"	moveto  corner dup rmoveto",
	"	corner 2 mul   ",
	"	dup neg 0 rlineto  dup 0 exch neg rlineto ",
	"	0 rlineto closepath",
	"} def",
	"",
	"(@) {			% draw a circle ",
	"	symradius notethick 2 div sub    0 360 arc",
	"} def",
	"",
	"(^) {			% draw a triangle",
	"	moveto  0  symradius neg  rmoveto  ",
	"	trix  symradius 1.5 mul  rlineto ",
	"	trix -2 mul  0  rlineto ",
	"	closepath",
	"} def",
	"",
	"(-) {			% draw a dash",
	"	moveto  symradius neg 0  rmoveto",
	"	symradius 2 mul  0  rlineto ",
	"} def",
	"",
	"(&) {			% upside down triangle",
	"	moveto  0  symradius  rmoveto  ",
	"	trix  symradius -1.5 mul  rlineto ",
	"	trix -2 mul  0  rlineto ",
	"	closepath",
	"} def",
	"",
	"(~) {			%% draw a dash with line up",
	"	moveto  symradius neg 0  rmoveto",
	"	symradius 2 mul  0  rlineto ",
	"	symradius neg  0  rmoveto ",
	"	0 symradius -2 div  rlineto ",
	"} def",
	"",
	"(_) {			% draw a dash with line down",
	"	moveto  symradius neg 0  rmoveto",
	"	symradius 2 mul  0  rlineto ",
	"	symradius neg  0  rmoveto ",
	"	0 symradius 2 div  rlineto ",
	"} def",
	"",
	"/dxdy {		% compute dx and dy to center the string on stack",
	"	strbounds",
	"	add 2 div neg /dy exch def",
	"	add 2 div neg /dx exch def",
	"} def",
	"",
	"/showcentered {  % shows its arg string centered at current point ",
	"	dup dxdy dx dy rmoveto show		",
	"} def",
	"",
	"/noteparams {    % compute offset to center string on a point",
	"			% stack : ntxt      ",
	"			% results: set sproc, dx and dy;  choose and set font",
	"	/ntxt exch def",
	"	godict ntxt known {",
	"		% special symbol",
	"		/proc godict ntxt get def",
	"	}{",
	"		% text : get offsets from pathbbox",
	"		/proc 0 def",
	"		[ onedigitfont onedigitfont twodigitfont threedigitfont ]",
	"			ntxt length {get} stopped {onedigitfont} if setfont",
	"		ntxt dxdy",
	"	} ifelse",
	"} def",
	"",
	"/note {		% draw annotation    stack: x y note",
	"		% color must have been set",
	"	/ntxt exch def ",
	"	newpath		% at center of spot",
	"	/proc load  0 ne {",
	"		% special symbol",
	"		0 setlinecap",
	"		notethick setlinewidth",
	"		proc  stroke     % perform function for special symbol",
	"	}{",
	"		% text",
	"		moveto  dx dy rmoveto",
	"		ntxt show",
	"	} ifelse",
	"} def",
	"",
	"/wst {		% white stone     stack: x y",
	"	2 copy",
	"	wscolor",
	"	newpath wsradius 0 360 arc fill",
	"	wsthick setlinewidth",
	"	wsedgecolor",
	"	newpath wsradius 0 360 arc stroke",
	"} def",
	"",
	"/bst {		% black stone    stack: x y",
	"	bscolor",
	"	newpath bsradius 0 360 arc fill",
	"} def",
	"",
	"/wstn {	% white stone with note    stack: x y note",
	"	3 copy noteparams",
	"	wst",
	"	wsnotecolor",
	"	note",
	"} def",
	"",
	"/bstn {		% black stone with note    stack: x y note",
	"	3 copy  noteparams",
	"	bst",
	"	bsnotecolor",
	"	note",
	"} def",
	"",
	"/hoshi {	% draw hoshi dot    stack: x y",
	"	hoshicolor",
	"	newpath hoshiradius 0 360 arc fill",
	"} def",
	"",
	"",
	"/spotn {	% note on empty spot   stack: x y note",
	"		% stroke with wide stroke and then scale to size to fill",
	"	/ntxt exch def",
	"	2 copy		% x y  x y ",
	"	% figure out what part of grid to hide",
	"	ntxt noteparams",
	"	gsave",
	"	newpath  translate   0 0",
	"	/proc load 0 ne {",
	"		proc  strokepath   % outline the symbol",
	"	}{",
	"		moveto dx dy rmoveto",
	"		ntxt false charpath",
	"	} ifelse",
	"	noteaura setlinewidth	",
	"	1 setlinejoin",
	"	1 setlinecap",
	"	currentflat 3 mul setflat",
	"	BoardColor  ",
	"	stroke		% stroke line edges with BoardColor",
	"	0 0 noteaura 2 mul 0 360 arc  	fill    % and fill an interior circle",
	"	grestore",
	"",
	"	boardnotecolor  ntxt  note",
	"} def",
	"",
	"end	% stop defining godict",
	"",
	NULL
};

	static void
putgodict( FILE *f, const char *prefix ) {
	const char * const *dx;
	for (dx = stonefuncs; *dx; dx++)
		fprintf( f, "%s %s\n", prefix, *dx );
}

	static void 
GeneratePostScript(FILE *file, gofig *dobj, const char *prefix, int wpts, int hpts) {

	int height = dobj->height,  width = dobj->width;

	/* generate PostScript  */
	
	fprintf(file, "%s  %% gofig: Go Diagram   %d rows by %d columns\n", 
			prefix, height, width );	

	int i, j;
	unsigned edges;
	dobj->getedges( & edges );
	float leftlim, rightlim, toplim, botlim;
	leftlim = (edges & LEFTedge) ? 0 : -0.5;
	rightlim = width  - 1 + ((edges & RIGHTedge) ? 0 : 0.5);
	toplim = (edges&TOPedge) ? 0 : -0.5;
	botlim = height - 1 + ((edges&BOTTOMedge) ? 0 : 0.5);

		/* compute image parameters */
	float spotwidth, spotheight;

	/* compute nominal spot width and height */
	if (edges&INDICESedge) {
		spotwidth = wpts / (dobj->width+2*INDEXFRAC);
		spotheight =  hpts / (dobj->height+2*INDEXFRAC);
	}
	else {
		spotwidth = 1.0 * wpts / dobj->width;
		spotheight = 1.0 * hpts / dobj->height;
	}
	if (spotwidth / (WTOHRATIO * spotheight) < 1.0) 
		spotheight = spotwidth / WTOHRATIO ;
	else if  (spotwidth / (WTOHRATIO * spotheight) > 1.0) 
		spotwidth = spotheight * WTOHRATIO;
	/* the dtransform/idtransform hack can EXPAND the pixel size,
		so we need an additional half pixel per row or column */
	spotwidth -= 0.5;
	spotheight -= 0.5;

	/* send code to establish coordinate space 
		upper left intersection is 0,0    unit is line spacing
		positive values are to the left in x and down in y
	*/

	fprintf( file, "%s\n", prefix );
	fprintf( file, "%s godict begin\n", prefix );
	fprintf( file, "%s %0.4f %0.4f  %% spot size\n", 
			prefix, spotwidth, spotheight );
	fprintf( file, "%s dtransform round exch round exch idtransform\n",
			prefix );
	fprintf( file, "%s /sph exch def  /spw exch def  %% in print pixels\n",
			prefix );
	fprintf( file, "%s %d spw %d mul add 2 div \n", 
			prefix, wpts, 1-dobj->width );
	fprintf( file, "%s %d sph %d mul add 2 div \n", 
			prefix, hpts, dobj->height-1 );
	fprintf( file, "%stranslate     spw sph neg scale\n", prefix );

	if (edges&INDICESedge) {
			/* labels centered in space INDEXFRAC wide */
		fprintf( file, "%s  /Times-Roman  findfont [%0.3f 0 0 -%0.3f 0 0 ]\n",
			prefix, INDEXFRAC*0.8, INDEXFRAC*0.8 ); 
		fprintf( file, "%s  makefont setfont\n",
			prefix );
		fprintf( file, "%s  /inxlet (ABCDEFGHJKLMNOPQRSTUVWXYZ) def\n",
			prefix );
		fprintf( file, "%s  0 1 %d { %%draw top/bottom indices \n",
			prefix, dobj->width - 1 );
		fprintf( file, "%s  	dup dup  newpath \n",
			prefix );
		fprintf( file, "%s  	inxlet exch 1 getinterval /let exch def\n",
			prefix );
		fprintf( file, "%s  	-%0.3f moveto let showcentered\n",
			prefix, 0.5+INDEXFRAC/2 );
		fprintf( file, "%s  	%0.3f moveto let showcentered\n",
			prefix, dobj->height - 0.5 + INDEXFRAC/2 );
		fprintf( file, "%s  } for\n",
			prefix );
		fprintf( file, "%s  0 1 %d { %% draw side indices\n",
			prefix, dobj->height - 1 );
		fprintf( file, "%s  	dup dup newpath\n",
			prefix );
		fprintf( file, "%s  	%d exch sub inxlet cvs /let exch def\n",
			prefix, dobj->width );
		fprintf( file, "%s  	-%0.3f exch moveto let showcentered\n",
			prefix, 0.5 + INDEXFRAC/2 );
		fprintf( file, "%s  	%0.3f exch moveto let showcentered\n",
			prefix, dobj->width - 0.5 +INDEXFRAC/2 );
		fprintf( file, "%s  } for\n",
			prefix );
	}

	/* draw non-edge grid lines */
	fprintf( file, "%s 2 setlinecap  linecolor  linethick  setlinewidth\n", prefix );
	fprintf( file, "%s %d 1 %d  {  %%draw horizontals\n", prefix, 
			(edges & TOPedge) ? 1 : 0,  
			height- ((edges & BOTTOMedge) ? 2 : 1 ) );
	fprintf( file, "%s 	dup newpath %g exch moveto %g exch lineto stroke\n",
			prefix, leftlim, rightlim );
	fprintf( file, "%s } for\n", prefix );
	fprintf( file, "%s %d 1 %d  {  %%draw verticals\n", prefix, 
			(edges & LEFTedge) ? 1 : 0,  
			width-((edges & RIGHTedge) ? 2 : 1) );
	fprintf( file, "%s 	dup newpath %g moveto %g lineto stroke\n",
			prefix, toplim, botlim );
	fprintf( file, "%s } for\n", prefix );

	if (edges)
		fprintf( file, "%s edgethick setlinewidth  	%% board edges\n",
				prefix );
	if (edges & LEFTedge)
		fprintf( file, "%s newpath 0 %g moveto 0 %g lineto  stroke\n", 
				prefix, toplim, botlim );
	if (edges & RIGHTedge)
		fprintf( file, "%s newpath %d %g moveto %d %g lineto  stroke\n", 
				prefix, width-1, toplim, width-1, botlim );
	if (edges & TOPedge)
		fprintf( file, "%s newpath %g 0 moveto %g 0 lineto  stroke\n", 
				prefix, leftlim, rightlim );
	if (edges & BOTTOMedge)
		fprintf( file, "%s newpath %g %d moveto %g %d lineto  stroke\n", 
				prefix, leftlim, height-1, rightlim, height-1 );

	/* draw hoshi */
	int xhoshi[4];			/* coords of hoshi points */
	int yhoshi[4];			/*     entry after last is zero */
	computeHoshi(xhoshi, (edges&LEFTedge) != 0, 
				(edges&RIGHTedge) != 0, width);
	computeHoshi(yhoshi, (edges&TOPedge) != 0, 
				(edges&BOTTOMedge) != 0, height);
	for (i = 0; xhoshi[i]>=0; i++) 
		for (j = 0; yhoshi[j]>=0; j++)
			fprintf( file, "%s %d %d hoshi\n", 
					prefix, xhoshi[i], yhoshi[j] );

	/* draw each stone */
	for (i = dobj->nstones(); i--; ) {
		struct stone &s = (*dobj)[i];
		if (s.note == 0 && s.color == '/')  
			continue;		/* skip meaningless stone */
		char buf[25];
		if (s.note == 0) buf[0] = '\0';
		else if (s.note > 0)
			sprintf( buf, " (%d)", s.note );
		else
			sprintf( buf, " (%c)", -s.note );
		fprintf ( file, "%s %d %d%s %s%s\n",  prefix, 
			s.col, s.row, buf, 
			(s.color == 'W') ? "wst" 
				: (s.color == 'B') ? "bst" : "spot",
			(buf[0]) ? "n" : "" );
	}

	fprintf( file, "%s end  %% discard godict\n", prefix );
}


	void
gofigview::DesiredPrintSize(long width, long height, enum view::DSpass pass,
		long *desiredwidth, long *desiredheight) {

	gofig *dobj = (gofig *)GetDataObject();
	unsigned edges;
	dobj->getedges( & edges );
	int wpts, hpts;
	double colwidth;

	/* the dtransform/idtransform hack can EXPAND the pixel size,
		so we need an additional half pixel per row or column */
	float indexadd = ((edges&INDICESedge) ? 2.0*INDEXFRAC : 0.0);
	if (pass == view::NoSet) 
		colwidth  = dobj->printcolwidth / 1000.0 + 0.5;
	else if (pass == view::WidthSet) 
		colwidth = width / (dobj->width + indexadd) - 0.5;
	else /* pass == view::HeightSet */ 
		colwidth = WTOHRATIO * height / (dobj->height +indexadd) - 0.5;

	wpts = mytrunc( colwidth * (dobj->width + indexadd) );
	hpts = mytrunc( colwidth * (dobj->height +indexadd) / WTOHRATIO );
	if (wpts > pagew) {
		hpts = pagew*hpts / wpts;
		wpts = pagew;
	}
	if (hpts > pageh) {
		wpts = pageh * wpts / hpts;
		hpts = pageh;
	}
	*desiredwidth = wpts;
	*desiredheight = hpts;
}

	void *
gofigview::GetPSPrintInterface(const char *printtype) {
	if (strcmp(printtype, "generic") == 0)
		return (void *)this;
	return NULL;
}

	static void
registerdefs(gofigview *self) {
	gofig *dobj = (gofig *)self->GetDataObject();
	unsigned edges;
	dobj->getedges( & edges );
	print::PSRegisterDef("godict", "60 dict");
		/* when border indices appear, 55 elements of godict are used */
	print::PSRegisterHeader("gofig_godict", (print_header_fptr)putgodict, "" );
	print::PSRegisterFont("Helvetica-Bold");
	if (edges&INDICESedge) 
		print::PSRegisterFont("Times-Roman");
}

	void
gofigview::PrintPSRect(FILE *outfile, long logwidth, long logheight, 
		struct rectangle *visrect) {
	registerdefs( this );

	GeneratePostScript(outfile, (gofig *)GetDataObject(), "", logwidth, logheight );
}

	void
gofigview::PrintPSDoc(FILE *outfile, long pagew, long pageh) {
	long dw, dh;
	registerdefs( this );
	print::PSNewPage(1);

	DesiredPrintSize(pagew, pageh, view::NoSet, &dw, &dh);
	fprintf(outfile, "%ld %ld translate\n",  (pagew-dw)/2, (pageh-dh)/2);

	GeneratePostScript(outfile, (gofig *)GetDataObject(), "", dw, dh);
}

	void
gofigview::Print(FILE *file, const char *processor, const char *finalFormat, boolean topLevel) {
	gofig *dobj = (gofig *)GetDataObject();

	/* compute size */
	long wpts, hpts;
	DesiredPrintSize(pagew, pageh, view::NoSet, &wpts, &hpts);

	/* generate preface and prefix */
	const char *prefix;
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

	putgodict( file, prefix );	/* define dictionary */

	if (topLevel) {
		/* center image on page */
		fprintf(file, "%s %ld %ld translate\n", prefix,
			(pagew - wpts)/2, (pageh-hpts)/2);
	}

	/* DO IT */
	GeneratePostScript( file, dobj, prefix, wpts, hpts );

	/* generate postface */
	if (strcmp(processor, "troff") == 0){
		texttroff::EndPS( file, wpts, hpts );
		if (topLevel)
			texttroff::EndDoc( file );
	}

}



	static void 
printdata(gofigview *self, int code) {
	gofig *dobj = (gofig *)self->GetDataObject();
	printf("visacc:  %d  currow,col:  %d %d  sel: 0x%p\n",
		 self->visibleaccnum,	self->curspotrow, self->curspotcol,
		selectedspot( self ));
	printf("gofig data object at 0x%p\n", dobj);
	printf( "board is %dx%d  (printcolwidth: %ld)  edges lrtb:%c%c%c%c\n",
			dobj->width, dobj->height, dobj->printcolwidth, 
			dobj->edges&LEFTedge?'|':'.',
			dobj->edges&RIGHTedge?'|':'.', 
			dobj->edges & TOPedge?'-':'.', 
			dobj->edges & BOTTOMedge?'_':'.' );
	int i;
	struct stone *s0 = &(*dobj)[0];
	for (i = 0; i < dobj->nstones(); i++) {
		struct stone *s = &(*dobj)[i];
		printf( "	0x%p  [%ld]:  %c @ (%d,%d) ", 
				s, s-s0, s->color, s->row, s->col );
		if (s->note < 0) printf( "%c", -s->note ); 
		else if (s->note > 0) printf( "%d", s->note );
 		printf( "    accnum: %ld\n", s->accnum );
	}
	fflush(stdout);
}

