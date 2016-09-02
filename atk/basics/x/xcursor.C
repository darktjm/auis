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

#include <andrewos.h>
ATK_IMPL("xcursor.H")
#include <X11/Xlib.h>
#include <X11/Xatom.h>

ATK_IMPL("xcursor.H")
#include <fontdesc.H>
#include <xfontdesc.H>
#include <xgraphic.H>
#include <cursor.H>
#include <view.H>
#include <im.H>
#include <xim.H>
#include <environ.H>
#include <atom.H>

static char CursorTable[] = {  'a',
    'a','g','x','H','h','l','u','v','w','|',
    'J',':','C','R','M',';','/','^','s','U',
    'D','@','X','~','}' };
#define lookup(A) ((A >= 0 && A < Cursor_TABLELEN)?  CursorTable[A] :CursorTable[0])

#include <xcursor.H>

extern int cursordebug;
static const char *foreground=NULL;
static const char *background=NULL;
static XColor forecolor;
static XColor backcolor;
static Display *colorDisplay = NULL;
static int colorScreen = -1;

static struct ccache {
    Display *dpy;
    class xfontdesc *xfd;
    int c;
    Cursor cursor;
    struct ccache *next;
} *fc=NULL;



ATKdefineRegistry(xcursor, cursor, xcursor::InitializeClass);
Cursor LookupCursor(Display  *dpy, class xfontdesc  *xfd, int  c);


void xcursor::ChangeShape()
{
    this->valid=FALSE;
    if((this)->IsPosted()){
	this->changed = TRUE;
	(this->posted)->PostCursor(NULL,this);
    }

    if((this)->IsWindowCursor()){
	this->changed = TRUE;
	(this->windowim)->SetWindowCursor(this);
	}
    if((this)->IsProcessCursor()){
	this->changed = TRUE;
	im::SetProcessCursor(this);
	}
}


#define DEFAULTFONTNAME "icon"
#define DEFAULTFONTSIZE 12
#define DEFAULTMASKNAME "icon"
void xcursor::SetStandard(short  ch)
{
    class fontdesc *oldfont=this->fillFont;
    char oldchar=this->fillChar;
    
    this->fillFont = fontdesc::Create(DEFAULTFONTNAME,0,DEFAULTFONTSIZE);
    this->fillChar = lookup(ch);
    if(oldfont!=this->fillFont || oldchar!=this->fillChar) (this)->ChangeShape();
}

Cursor LookupCursor(Display  *dpy, class xfontdesc  *xfd, int  c)
{
    struct ccache *cc=fc;
    while(cc!=NULL) {
	if(cc->dpy==dpy && cc->xfd==xfd && cc->c==c) return cc->cursor;
	cc=cc->next;
    }
    return None;
}

void xcursor::Make(class xim  *im)
{
    Cursor tmp = None;
    XFontStruct *info;
    XCharStruct *ci;
    XGCValues gcattr;
    Pixmap sourcePixmap, maskPixmap;
    GC gc;
    char str[1];
    unsigned int width, height;
    long x, y;
    class xfontdesc * xfillFont = (class xfontdesc *) this->fillFont;

    if (cursordebug) printf("xim_MakeXCursor: making cursor for %d in %p for %p\n", this->fillChar, xfillFont, this);

    if (this->valid && this->Xc != None && this->im == im) {
	if(!this->changed) return; /* Already valid cursor */

	/* XFreeCursor(xim2display(im), self->Xc); don't free it since it will stay in the cache */

	this->Xc=None;	
    }

    tmp=LookupCursor(xim2display(im), xfillFont, this->fillChar);

    if (tmp==None && this->fillChar) {

	info = (XFontStruct *)(xfillFont)->GetRealFontDesc(
							im->drawable);

	/* This is the code for constructing an andrew semantics cursor from the single character
	    The basic idea is:
	    get the size of the icon
	    make the mask:
	    or in the icon at each of the nine locations centered around its hot spot
	    make the shape
	    or in the icon at 8 locations around hot spot and xor in at the hotspot
	    create it
	    toss away temp pixmaps, gc, etc. */

	/*      fillChr = cursor->header.cursor.fillChar; */
	if (this->fillChar >= info->min_char_or_byte2 && this->fillChar <= info->max_char_or_byte2)  {
	    ci = (info->per_char != NULL) ? &(info->per_char[this->fillChar - info->min_char_or_byte2]) : &(info->max_bounds);
	    /* Note that we use a slightly bigger pixmap so that we have room to create a white outline for a font that is defined right to the edge. Diito for moving the original over a little bit, so that we have room in the larger pixmap to fit the white board on any of the four sides */
	    width = ci->rbearing - ci->lbearing + 2;
	    /* Bug patch -- 15 pixel wide cursors don't work, so instead we enalrge it by one more pixel. Sigh. */
	    if (width == 15) width++;
	    height = ci->descent + ci->ascent + 2;
	    x = -ci->lbearing + 1;
	    y = ci->ascent + 1;
	    if (width == 0) {
		fprintf(stderr,"xim_MakeXCursor: pixmap of width 0 requested by %p\n", im);
		width++;
	    }
	    if (height == 0) {
		fprintf(stderr,"xim_MakeXCursor: pixmap of height 0 requested by %p\n", im);
		height++;
	    }

#ifndef M_UNIX
	    XQueryBestCursor(xim2display(im), xim2window(im), width, height, &width, &height);
#endif /* M_UNIX */

	    maskPixmap = XCreatePixmap(xim2display(im),RootWindow(xim2display(im), DefaultScreen(xim2display(im))), width, height, 1);
	    gcattr.fill_style = FillSolid;

	    gcattr.foreground = 1;
	    gcattr.font = info->fid;
	    gcattr.function = GXclear;
	    gc = XCreateGC(xim2display(im), maskPixmap, GCFillStyle | GCForeground | GCFont | GCFunction, &gcattr);

	    /* Clear out the pixmap so it has all zeros */
	    XFillRectangle(xim2display(im),maskPixmap, gc, 0, 0, width, height);
	    XSetFunction(xim2display(im),gc,GXor);
	    str[0] = this->fillChar;
	    XDrawString(xim2display(im), maskPixmap, gc, x, y, str, 1);
	    XDrawString(xim2display(im), maskPixmap, gc, x-1, y, str, 1);
	    XDrawString(xim2display(im), maskPixmap, gc, x, y-1, str, 1);
	    XDrawString(xim2display(im), maskPixmap, gc, x-1, y-1, str, 1);
	    XDrawString(xim2display(im), maskPixmap, gc, x+1, y, str, 1);
	    XDrawString(xim2display(im), maskPixmap, gc, x, y+1, str, 1);
	    XDrawString(xim2display(im), maskPixmap, gc, x+1, y+1, str, 1);
	    XDrawString(xim2display(im), maskPixmap, gc, x-1, y+1, str, 1);
	    XDrawString(xim2display(im), maskPixmap, gc, x+1, y-1, str, 1);
	    XFreeGC(xim2display(im), gc);

	    /* Now we have the mask, let's build the source */
	    sourcePixmap = XCreatePixmap(xim2display(im),RootWindow(xim2display(im), DefaultScreen(xim2display(im))),width,height,1);
	    gc = XCreateGC(xim2display(im), sourcePixmap, GCFillStyle | GCForeground | GCFont | GCFunction, &gcattr);

	    /* Clear out the pixmap so it has all zeros */
	    XFillRectangle(xim2display(im),sourcePixmap, gc, 0, 0, width, height);
	    XSetFunction(xim2display(im),gc,GXor);
	    str[0] = this->fillChar;
	    XSetFunction(xim2display(im),gc,GXcopy);
	    XDrawString(xim2display(im), sourcePixmap, gc, x, y, str, 1);
	    XFreeGC(xim2display(im), gc);

	    if (xim2display(im) != colorDisplay || xim2screen(im) != colorScreen) {
		Status status;

		colorDisplay = xim2display(im);
		colorScreen = xim2screen(im);

		status = XParseColor(colorDisplay, DefaultColormap(colorDisplay, colorScreen), foreground, &forecolor);
		if (status == 0) {
		    forecolor.pixel = BlackPixel(colorDisplay, DefaultScreen(colorDisplay)) /* or so FontCursor does */;
		    forecolor.red = forecolor.green = forecolor.blue = 0 /* ~0 */ ;
		    forecolor.flags = 0;
		}

		status = XParseColor(colorDisplay, DefaultColormap(colorDisplay, colorScreen), background, &backcolor);
		if (status == 0) {
		    backcolor.pixel = WhitePixel(colorDisplay, DefaultScreen(colorDisplay)) /* or so FontCursor does */;
		    backcolor.red = backcolor.green = backcolor.blue = 65535 /* ~0 */ ;
		    backcolor.flags = 0;
		}
	    }

	    /* We have the pixmaps in hand, let's create cursors! */
	    tmp = XCreatePixmapCursor(xim2display(im),sourcePixmap,maskPixmap, &forecolor,&backcolor, x, y);
	    if(tmp!=None) {
		struct ccache *nc=(struct ccache *)malloc(sizeof(struct ccache));
		if(nc==NULL) return;
		nc->dpy=xim2display(im);
		nc->xfd=xfillFont;
		nc->c=this->fillChar;
		nc->cursor=tmp;
		nc->next=fc;
		fc=nc;
	    }
	    /* And toss away the pixmaps */
	    XFreePixmap(xim2display(im),maskPixmap);
	    XFreePixmap(xim2display(im),sourcePixmap);
	} /* have legal characters */
	else printf("xim_ActivateCursor: illegal character %c used for cursor\n",this->fillChar);
    }
    
    if (tmp != None) {
	/* if (self->Xc) XFreeCursor(self->Xd, self->Xc); */
	this->Xc = tmp;
	this->Xd = xim2display(im);
	this->im = im;
	this->valid = TRUE;
    }
}

xcursor::xcursor()
{
	ATKinit;

    this->Xw = 0;
    this->Xc = 0;
    this->Xd = NULL;
    this->im = NULL;
    this->valid = FALSE;

    THROWONFAILURE( TRUE);
}


xcursor::~xcursor()
{
	ATKinit;

	/* if(self->Xc) XFreeCursor(self->Xd, self->Xc); */ /* bogus */
	this->Xc = 0;
	if(this->Xw) XDestroyWindow(this->Xd, this->Xw);
        this->Xw = 0;
}

boolean xcursor::InitializeClass()
{
    class atom *a;
    foreground = environ::GetProfile("cursorforegroundcolor");
    if (foreground == NULL || *foreground == '\0') {
	foreground = "black";
    }
    background = environ::GetProfile("cursorbackgroundcolor");
    if (background == NULL || *background == '\0') {
	background = "white";
    }
    if(foreground) {
	a=atom::Intern(foreground);
	if(a) foreground=a->Name();
	else foreground=NULL;
    }
    if(background) {
	a=atom::Intern(background);
	if(a) background=a->Name();
	else background=NULL;
    }
    return TRUE;
}


void xcursor::FinalizeDisplay(Display  *dpy)
{
	ATKinit;

    struct ccache *cc=fc;
    struct ccache **lc=(&fc);
    while(cc) {
	if(cc->dpy==dpy) {
	    (*lc)=cc->next;
	    XFreeCursor(dpy, cc->cursor);
	    free(cc);
	    cc=(*lc);
	} else cc=cc->next;
    }
}
	    
