/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/mit/RCS/dpstextview.C,v 1.3 1995/11/07 20:17:10 robr Stab74 $";
#endif


 

#include <andrewos.h>		/* for DPS_ENV */

#include <dpstextview.H>

#ifdef DPS_ENV
#include <X11/X.h>
#include <X11/Xlib.h>
#include <DPS/XDPS.h>
#include <DPS/XDPSlib.h>
#include <DPS/dpsXclient.h>
#include <DPS/dpsclient.h>

#include <xgraphic.H>
#endif /* DPS_ENV */

#include <view.H>
#include <text.H>
#include <im.H>
#include <graphic.H>
#include <rect.h>
#include <stdio.h>
#include <signal.h>


/* #define DEBUG 1 */
#ifdef DEBUG
#define DBG(x) fprintf(stderr, "%s\n", (x))
#else
#define DBG(x) 
#endif

/****************************************************************/
/*		private functions				*/
/****************************************************************/


ATKdefineRegistry(dpstextview, view, dpstextview::InitializeClass);
static int computeDPSscaling(class dpstextview  *self);
void DoInterpret(class dpstextview  *self);

static int computeDPSscaling(class dpstextview  *self)
     {
    long pw, ph, dw, dh, sw, sh;
    long x_offset, y_offset;
    double scale_factor;

    /* what do we want to be */
    pw = self->desired_width;
    ph = self->desired_height;

    /* what have we got to work with */
    dw = self->cached_width;
    dh = self->cached_height;

    /* try to be as tall as possible */
    sh = dh;
    scale_factor = (double)sh / (double)ph;
    sw = pw * scale_factor;
    y_offset = 0;
    x_offset = (dw - sw)/2;

    if (sw > dw) {
	/* try to be as wide as possible */
	sw = dw;
	scale_factor = (double)sw / (double)pw;
	sh = ph * scale_factor;
	y_offset = (dh - sh)/2;
	x_offset = 0;
    }
    if ((x_offset != self->offset_x)
	|| (y_offset != self->offset_y)
	|| (scale_factor != self->scale_width)
	|| (scale_factor != self->scale_height)) {
	(self)->SetScaling( x_offset, y_offset, scale_factor, scale_factor);
	return(-1);
    }
    return(0);
    
} /* computeDPSscaling */

#define BADPOINTER (-1)  /* pointer to use for uninitialized state since NULL may be a valid value for DisplayString */

void DoInterpret(class dpstextview  *self)
     {
#ifdef DPS_ENV
    char *displayname = "";
    Display *dpy;
    Screen *scr;
    GC gc;
    Window win;
    XWindowAttributes wa;
    DPSContext ctxt;
    class text *txtobj;
    class xgraphic *xgr;
    char *buf;
    long length;
    struct rectangle r1, r2;
    long x_off, y_off;
    double dpi_x, dpi_y;
    static char *olddpystring=(char *)BADPOINTER; /* I hope this is always an invalid pointer. */
    static Display *olddpy=(Display *)BADPOINTER;
    
    txtobj = (class text *)(self)->GetDataObject();

    ((self)->GetIM())->GetLoc( (self)->GetIM(), &r1);
    ((self)->GetIM())->GetLoc( self, &r2);
    xgr = (class xgraphic *)(self)->GetDrawable();
    /* assumption: for any given DisplayString xim will have at most one connection, and hence the address of DisplayString on that dpy will uniquely identify the display.  And that xim never closes displays so the DisplayString is available permanently. -rr2b */
    if(olddpystring == DisplayString((xgr)->XDisplay())) dpy=olddpy;
    else {
	if(olddpy && olddpy!=(Display *)BADPOINTER) XCloseDisplay(olddpy);
	olddpystring = DisplayString((xgr)->XDisplay());
	olddpy = XOpenDisplay(olddpystring);
    }
    dpy=olddpy;
    if (dpy == NULL) {
 	fprintf (stderr, "dpstextview: NULL display.\n");
 	fprintf (stderr, "dpstextview: maybe I'm not visible?\n");
	return;
    }
    gc = (xgr)->XGC();
    win = (Window)(xgr)->XWindow();
    dpi_x = (xgr)->GetHorizontalResolution();
    dpi_y = (xgr)->GetVerticalResolution();

    x_off = r2.left - r1.left + self->offset_x;
    y_off = r2.top - r1.top + r2.height - self->offset_y;
#ifdef DEBUG
    ctxt = DPSCreateTextContext(DPSDefaultTextBackstop, DPSDefaultErrorProc); 
#else
    ctxt = XDPSCreateSimpleContext(dpy, win, gc, x_off, y_off,
				   (DPSTextProc)DPSDefaultTextBackstop,
				   (DPSErrorProc)DPSDefaultErrorProc, NULL);
#endif
    
    if (ctxt == NULL) {
	fprintf (stderr, "dpstextview: XDPSCreateSimpleContext returns NULL.\n");
 	fprintf (stderr, "dpstextview: Perhaps this server doesn't have the Display PostScript extension?\n");
	return;
    }

    DPSPrintf(ctxt, "gsave %g %g scale\n", 72.0/dpi_x, 72.0/dpi_y);
    DPSPrintf(ctxt, "0.9 setgray %d %d %d %d rectfill\n",
	      -(self->offset_x), -(self->offset_y), r2.width, r2.height);
    DPSPrintf(ctxt, "1.0 setgray 0 0 %g %g rectfill\n",
	      self->desired_width * self->scale_width,
	      self->desired_height * self->scale_height);
    DPSPrintf(ctxt, "grestore\n");
    DPSPrintf(ctxt, "%g %g scale\n", self->scale_width, self->scale_height);
    DPSPrintf(ctxt, "/showpage {} def\n");

    /* Make sure that gap is at end of buffer so that GetBuf
       gives us the whole buffer, not just the part before the gap.  */
    (txtobj)->GetGap( (txtobj)->GetLength(), 0L);
    buf = (txtobj)->GetBuf( 0L, (txtobj)->GetLength(), &length);
    DPSWritePostScript(ctxt, buf, length);
    DPSWaitContext(ctxt);
    
    DPSDestroySpace(DPSSpaceFromContext(ctxt));
    XFlush(dpy);
#endif /* DPS_ENV */
} /* DoInterpret */

/****************************************************************/
/*		class procedures				*/
/****************************************************************/

boolean
dpstextview::InitializeClass()
    {
    return TRUE;
}


dpstextview::dpstextview()
{
	ATKinit;

    (this)->SetScaling( 0L, 0L, 1.0, 1.0);
    this->drawn_at_least_once = 0;
    this->cached_width = 0;
    this->cached_height = 0;
    this->offset_x = 0;
    this->offset_y = 0;
    THROWONFAILURE( TRUE);
}

dpstextview::~dpstextview()
{
	ATKinit;

}


/****************************************************************/
/*		instance methods				*/
/****************************************************************/

class view *dpstextview::Hit(enum view_MouseAction  action, long  x, long  y, long  clicks)
{
    (this)->WantInputFocus( this);
    return (class view *)this;
}

void dpstextview::SetDesired(long  w, long  h)
          {  
    this->desired_height = h;
    this->desired_width = w;

    if (computeDPSscaling(this) && this->drawn_at_least_once) {
	(this)->WantUpdate( this);
    }
} /* dpstextview__SetDesired */

void dpstextview::Update()
     {
    struct rectangle r;

    (this)->SetTransferMode( graphic_COPY);
    (this)->EraseVisualRect();
    (this)->GetLogicalBounds( &r);
    (this)->FullUpdate( view_FullRedraw, r.left, r.top, r.width, r.height);
} /* dpstextview_Update */

void
dpstextview::FullUpdate(enum view_UpdateType  type, long  x, long  y, long  w, long  h)
               {  
    if (type == view_FullRedraw
	|| type == view_LastPartialRedraw) {
	struct rectangle r;

	(this)->GetLogicalBounds( &r);
	if ((r.width != this->cached_width) || (r.height != this->cached_height)) {
	    this->cached_width = r.width;
	    this->cached_height = r.height;
	    computeDPSscaling(this);
	}

	DoInterpret(this);
	this->drawn_at_least_once = -1;
    }
} /* dpstextview__FullUpdate */

