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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/mit/RCS/psview.C,v 1.6 1996/11/07 17:36:49 wjh Exp $";
#endif


 

/*
  Still needs to be done:

  - fixed UI to refer to the point (not pixel) size, but code is
    still confused about point .vs. pixel
  - there's too much whitespace before the PostScript
  - showpages are wiped out in print and view modes
  - AskOrCancel leaves the input focus in the frameview on ^G
  - Still have a problem with dpstextview sucking up input focus
    and losing the menus....

*/

#include <andrewos.h>		/* for DPS_ENV */

#include <stdio.h>
#include <signal.h>

#include <view.H>
#include <textview.H>
#include <note.H>
#include <iconview.H>
#include <ps.H>
#include <psview.H>
#include <bind.H>
#include <menulist.H>
#include <keymap.H>
#include <print.H>
#include <text.H>
#include <proctable.H>
#include <fontdesc.H>
#include <style.H>
#include <texttroff.H>
#include <message.H>
#include <search.H>

#ifdef DPS_ENV
#include <dpstextview.H>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <xgraphic.H>
#ifdef DisplayString
#undef DisplayString
#endif
#endif /* DPS_ENV */


#define DEBUG 1
#ifdef DEBUG
#define DBG(x) fprintf(stderr, "%s\n", (x))
#else
#define DBG(x) 
#endif

#define ICONFONT "icon"
#define ICONSTYLE fontdesc_Plain
#define ICONPTS 12
#define ICONCHAR '\151'
#define TITLEFONT "andysans"
#define TITLESTYLE fontdesc_Plain
#define TITLEPTS 12

#ifdef DPS_ENV
#define EDIT_MODE 0x1
#define VIEW_MODE 0x2
#endif /* DPS_ENV */

class menulist *psviewMenus;
static class keymap *psviewKeyMap;

#define DisplayAndReturn(self, String) {message::DisplayString(self, 0, String); return;}

#define AskOrCancel(self, string, buf) \
	{if (message::AskForString(self, 0, string, "", buf, sizeof buf - 1) < 0) \
	{message::DisplayString(self, 0, "Cancelled."); \
	return;}}

/****************************************************************/
/*		private functions				*/
/****************************************************************/


ATKdefineRegistry(psview, iconview, psview::InitializeClass);
#ifndef NORCSID
#endif
#ifdef DPS_ENV
#endif /* DPS_ENV */
#ifdef DEBUG
#else
#endif
#ifdef DPS_ENV
#endif /* DPS_ENV */
static void Close(class psview  *v,long  l);
static void ps_open(class psview  *v,long  l);
static void closeall(class view  *v,long  l);
static void openall(class view  *v,long  l);
static void insert(class textview  *tv,long  l);
static void update_dpstextview(class psview  *self);
static void inchsize(class psview  *self);
static void pixelsize(class psview  *self);
#ifdef DPS_ENV
static void display(class psview  *self, long  rock);
static void edit(class psview  *self, long  rock);
#endif /* DPS_ENV */
static void autobounds(class psview  *self, long  rock);


static void
Close(class psview  *v,long  l)
{
    (v)->Close();
}
static void
ps_open(class psview  *v,long  l)
{
    (v)->Open();
}
static void
closeall(class view  *v,long  l)
{
    iconview::CloseRelated(v);
}
static void
openall(class view  *v,long  l)
{
    iconview::OpenRelated(v);
}

static void
insert(class textview  *tv,long  l)
{
    class text *t;
    long pos;
    t = (class text *) (tv)->GetDataObject();
    pos = (tv)->GetDotPosition() + (tv)->GetDotLength();
    tv->currentViewreference = (t)->InsertObject( pos,"ps", NULL);
    (t)->NotifyObservers(0);
}

static void update_dpstextview(class psview  *self)
     {
#ifdef DPS_ENV
    class ps *psobj = (class ps *)(self)->GetDataObject();
    long w = (psobj)->GetPixelWidth();
    long h = (psobj)->GetPixelHeight();
    double dpi_x, dpi_y;
    long pix_w, pix_h;
    class xgraphic *xgr=(class xgraphic *)(self)->GetDrawable();

    if (!(self->interpret) || xgr==NULL) return;
    dpi_x = (xgr)->GetHorizontalResolution();
    dpi_y = (xgr)->GetVerticalResolution();
    pix_w = w / 72.0 * dpi_x;
    pix_h = h / 72.0 * dpi_y;
    ((class dpstextview *)(((class iconview *)self)->child))->SetDesired( pix_w, pix_h);

#endif /* DPS_ENV */

    return;
} /* update_dpstextview */

static void inchsize(class psview  *self)
{
    class ps *psobj = (class ps *)(self)->GetDataObject();
    long w = (psobj)->GetPixelWidth();
    long h = (psobj)->GetPixelHeight();
    double dpi_x, dpi_y;
    double newdw = 0.0, newdh = 0.0;
    char newxsize[75], newysize[75], request[150];

    dpi_x = 72.0;
    dpi_y = 72.0;

    /* ask for height */
    sprintf(request, "Print height %0.2f  width %0.2f in.  New height [%0.2f]: ", h / dpi_y, w / dpi_x, h / dpi_y);
    AskOrCancel(self, request, newysize);
    if (*newysize) {
	/* height specified.  parse it and set width request */
	if (sscanf(newysize, "%lf", &newdh) != 1)		
	    DisplayAndReturn(self,
		"Value must be digits with at most one decimal point."); 
	h = (long) (newdh * dpi_y);
    }

    /* request new width */
    sprintf(request, "Print height %0.2f in.   New width [%0.2f] ", 
		h / dpi_y, w / dpi_x);
    AskOrCancel(self, request, newxsize);
    if (*newxsize) {
	if(sscanf(newxsize, "%lf", &newdw) != 1)
	    DisplayAndReturn(self, 
		"Value must be digits with at most one decimal point.");
	w = (long) (newdw * dpi_x);
    }

    /* display the new size */
    sprintf(request, "Print size is now height %0.2f width %0.2f in. ", 
			h / dpi_y, w / dpi_x );
    message::DisplayString(self, 0, request);
    (psobj)->SetPixelWidth( w);
    (psobj)->SetPixelHeight( h);
    update_dpstextview(self);
}

static void pixelsize(class psview  *self)
{
    class ps *psobj = (class ps *)(self)->GetDataObject();
    long w = (psobj)->GetPixelWidth();
    long h = (psobj)->GetPixelHeight();

    long newdw = 0, newdh = 0;
    char newxsize[75], newysize[75], request[150];

    /* ask for height */
    sprintf(request, "Print height %ld  width %ld points.   New height [%d]", h, w, h);
    AskOrCancel(self, request, newysize);
    if (*newysize) {
	/* height specified.  parse it and set width request */
	if (sscanf(newysize, "%ld", &newdh) != 1)		
	    DisplayAndReturn(self,
			     "Value must be digits with no decimal point."); 
	h = newdh;
    }

    /* request new width */
    sprintf(request, "Print height %ld points.   New width [%ld] ", h, w);
    AskOrCancel(self, request, newxsize);
    if (*newxsize) {
	if (sscanf(newxsize, "%ld", &newdw) != 1) 
		DisplayAndReturn(self, 
			"Value must be digits with no decimal point.");
	w = newdw;
    }

    /* display the new size */
    sprintf(request, "Print size is now height %ld points width %ld points.", h, w);
    message::DisplayString(self, 0, request);
    (psobj)->SetPixelWidth(w);
    (psobj)->SetPixelHeight(h);
    update_dpstextview(self);
}


#ifdef DPS_ENV
static void display(class psview  *self, long  rock)
          {
    class ps *psobj = (class ps *)(self)->GetDataObject();
    long w = (psobj)->GetPixelWidth();
    long h = (psobj)->GetPixelHeight();

    self->interpret = !0;

    if((self->menus)->SetMask( ((self->menus)->GetMask() & ~EDIT_MODE) | VIEW_MODE)) {
	(self)->PostMenus( NULL);
    }
    (self)->SetChild( "dpstextview");
    update_dpstextview(self);
} /* display */

static void edit(class psview  *self, long  rock)
          {
    class style * ds;

    self->interpret = 0;

    if((self->menus)->SetMask( ((self->menus)->GetMask() & ~VIEW_MODE) | EDIT_MODE)) {
	(self)->PostMenus( NULL);
    }
    (self)->SetChild( "textview");
    ds = ((class textview *)
				  self->bottomview)->GetDefaultStyle();
    (ds)->SetFontFamily( "AndyType");
    (ds)->SetFontSize( style_ConstantFontSize, 10);

} /* edit */
#endif /* DPS_ENV */

static void autobounds(class psview  *self, long  rock)
          {
    class textview *tvobj = (class textview *)(((class iconview *)self)->bottomview);
    class ps *psobj = (class ps *)(self)->GetDataObject();
    class text *tobj = (class text *)(psobj)->GetChild();
    char *pattern = "\n%%BoundingBox:";
    char *pat = NULL;		/* compiled search pattern */
    long pos, i, c;
    char bbox_buf[200];
    long llx, lly, urx, ury;
    char *translate = "%ld %ld translate %% inserted by ps inset at the request of user to make image visible\n";
    char tr_buf[200];
    char *pattern2 = "translate % inserted by ps inset at the request of user to make image visible\n";

    /* look for "\n%%BoundingBox: llx lly urx ury\n" information */
    if (search::CompilePattern(pattern, (struct SearchPattern **)&pat) != NULL) {
	message::DisplayString(self, 50, "should not happen:  psview could not compile search pattern.\n");
	return;
    }

    if ((pos = search::MatchPattern(tobj, 0L, (struct SearchPattern *)pat)) < 0) {
	/* bbox not found, exit */
	message::DisplayString(self, 50, "Could not find %%BoundingBox information.");
	return;
    }

    for (i=0, ++pos; ((c = (tobj)->GetChar( pos)) > 0) && ((char)c != '\n'); ++pos, ++i) {
	bbox_buf[i] = c;
    }
    bbox_buf[i] = '\0';
    if (sscanf(bbox_buf, "%%%%BoundingBox: %d %d %d %d", &llx, &lly, &urx, &ury) != 4) {
	message::DisplayString(self, 50, "%%BoundingBox was incomplete");
	return;
    }
    /* set the print size dimensions */
    (psobj)->SetPixelWidth( urx-llx);
    (psobj)->SetPixelHeight( ury-lly);

    /* insert the funky translate command */
    sprintf(tr_buf, translate, -llx, -lly);
    (tobj)->AlwaysInsertCharacters( 0L, tr_buf, strlen(tr_buf)); 
    (tobj)->NotifyObservers( observable_OBJECTCHANGED);
    (tvobj)->SetDotPosition( 0L);
    (tvobj)->SetDotLength( strlen(tr_buf));
    (tvobj)->FrameDot( 0L);
    (tvobj)->WantUpdate( tvobj);

    /* did an old translate command exist?  kill it */
    pat = NULL;			/* bug:  should we do this? */
    if (search::CompilePattern(pattern2, (struct SearchPattern **)&pat) != NULL) {
	message::DisplayString(self, 50, "should not happen:  psview could not compile second search pattern.\n");
	return;
    }

    if ((pos = search::MatchPattern(tobj, strlen(tr_buf), (struct SearchPattern *)pat)) < 0) {
	/* old not found, okay */
	message::DisplayString(self, 50, "BBox information set.");
	return;
    }
    /* find beginning of old */
    for(i = pos; (c = (tobj)->GetChar( i)) > 0 && c != '\n'; --i){
    }
    (tobj)->AlwaysDeleteCharacters( i+1, pos+strlen(pattern2)-i-1);
    message::DisplayString(self, 50, "BBox information set (and old translate nuked).");

    return;
} /* autobounds */

static struct bind_Description psviewBindings[]={
    {"psview-set-inch-size", NULL, 0, "PostScript,Set Inch Size~10", 0, 0, (proctable_fptr)inchsize, "Set print size in inches", NULL},
    {"psview-set-point-size", NULL, 0, "PostScript,Set Point Size~11", 0, 0, (proctable_fptr)pixelsize, "Set print size in points", NULL},
#ifdef DPS_ENV
    {"psview-edit", NULL, 0, "PostScript,Edit~1", 0, VIEW_MODE, (proctable_fptr)edit, "Edit the PostScript", NULL},
    {"psview-display", NULL, 0, "PostScript,View~1", 0, EDIT_MODE, (proctable_fptr)display, "Display the PostScript as an image", NULL},
#endif /* DPS_ENV */
    {"psview-autobound", NULL, 0, "PostScript,Scan for bounds~20", 0, 0, (proctable_fptr)autobounds, "Suck out the bounding box information and use it to set our bounds", NULL},
    NULL
};


void psview::PostMenus(class menulist  *menulist)
{
    (this->menus)->ClearChain();
    if (menulist) (this->menus)->ChainBeforeML( menulist, (long)menulist);
    (this)->iconview::PostMenus( this->menus);
}


/****************************************************************/
/*		class procedures				*/
/****************************************************************/

boolean
psview::InitializeClass()
    {
    struct ATKregistryEntry  *textviewtype = ATK::LoadClass("textview");
    struct ATKregistryEntry  *viewtype = ATK::LoadClass("view");

    psviewMenus = new menulist;
    psviewKeyMap =  new keymap;

    bind::BindList(psviewBindings, psviewKeyMap , psviewMenus, &psview_ATKregistry_ );
    return TRUE;
}


psview::psview()
{
	ATKinit;


    this->menus = (psviewMenus)->DuplicateML( this);
    this->interpret = 0;

    (this)->SetIconFont(ICONFONT,ICONSTYLE,ICONPTS); 
    (this)->SetIconChar(ICONCHAR);
    (this)->SetTitleFont(TITLEFONT,TITLESTYLE,TITLEPTS);
    THROWONFAILURE( TRUE);
}

psview::~psview()
{
	ATKinit;

    if(this->menus) delete this->menus;
}

static char *PSheader[] = {
    "%s  /width %d def  /height %d def /xScale %0.4f def /yScale %0.4f def\n",
    "%s xScale yScale scale\n",
    "%s /showpage {} def\n",
    NULL };

/****************************************************************/
/*		instance methods				*/
/****************************************************************/
void 
psview::Print(register FILE    *file, register char	 *processor, register char	 *format, register boolean  toplevel)
{
    class ps *psobj;
    class text *textobject;
    register char *text;
    char **psx;
    /* char *line = (char *)malloc(BUFSIZ); */
    long c, pos = 0, textlength = 0;
    char *prefix;
    double xdscale = 1.0, ydscale = 1.0;
    long height, width;

    psobj = (class ps *) (this)->GetDataObject();
    width = (psobj)->GetPixelWidth();
    height = (psobj)->GetPixelHeight();

    textobject = (class text *) (psobj)->GetChild();
    textlength = (textobject)->GetLength();
    if (strcmp(processor, "troff") == 0) {
	if (toplevel)
	    texttroff::BeginDoc(file);
	/*  Put macro to interface to postscript */
	texttroff::BeginPS(file, width, height);
	if ((width != 0) && (height != 0)) {
	    fprintf(file,"\\!  newpath 0 0 moveto %d 0 lineto ", width);
	    fprintf(file,"%d %d lineto 0 %d lineto closepath clip\n\n",
		    width,height,height);
	}

	prefix = "\\!  ";
    }
    else if (strcmp(format, "troff") == 0)
	prefix = "\\!  ";
    else prefix = "";

    for (psx = PSheader; *psx; psx++)  /*see PSheader def above. */
	fprintf(file, *psx, prefix, width, height, xdscale, ydscale);

    /*print out the file with prefix in front of each line */
    while ((c = (textobject)->GetChar( pos)) != EOF &&
	    pos < textlength){
	if (pos++ == 0) fprintf(file, "%s", prefix);
	if (c == '\n') fprintf(file,"%c%s",c, prefix);
	else fputc(c, file);
    }

    if (strcmp(processor, "troff") == 0) {
	texttroff::EndPS(file, width, height);
	if (toplevel)
	    texttroff::EndDoc(file); }


}

void psview::PrintPSDoc(FILE *outfile, long pagew, long pageh)
{
}

void *psview::GetPSPrintInterface(char *printtype)
{
    if (!strcmp(printtype, "generic"))
	return (void *)this;

    return NULL;
}

void psview::PrintPSRect(FILE *file, long logwidth, long logheight, struct rectangle *visrect)
{
    class ps *psobj;
    class text *textobject;
    register char *text;
    char **psx;
    /* char *line = (char *)malloc(BUFSIZ); */
    long c, pos = 0, textlength = 0;
    char *prefix = "";
    double xdscale = 1.0, ydscale = 1.0;
    long height, width;

    psobj = (class ps *) (this)->GetDataObject();
    width = (psobj)->GetPixelWidth();
    height = (psobj)->GetPixelHeight();
    
    textobject = (class text *) (psobj)->GetChild();
    textlength = (textobject)->GetLength();

    print::PSRegisterDef("psDict", "20 dict");
    fprintf(file, "%% open the temporary dictionary\n");
    fprintf(file, "psDict begin\n\n");

    if ((width != 0) && (height != 0)) {
	/* throw in the clipping path, without clearing it. I think this is a bug, but it stays for reasons of backwards schmompatibility. */
	fprintf(file, "newpath 0 0 moveto %d 0 lineto ", width);
	fprintf(file, "%d %d lineto 0 %d lineto closepath clip\n\n",
		width,height,height);
    }

    for (psx = PSheader; *psx; psx++)  /*see PSheader def above. */
	fprintf(file, *psx, prefix, width, height, xdscale, ydscale);

    while ((c = (textobject)->GetChar( pos)) != EOF &&
	   pos < textlength) {
	putc(c, file);
	pos++;
    }

    fprintf(file, "%% stop using temporary dictionary\n");
    fprintf(file, "end\n\n");
}

void psview::DesiredPrintSize(long width, long height, enum view_DSpass pass, long *desiredwidth, long *desiredheight)
{
    class ps *psobj;

    psobj = (class ps *) (this)->GetDataObject();
    *desiredwidth = (psobj)->GetPixelWidth();
    *desiredheight = (psobj)->GetPixelHeight();
    
}

	boolean 
psview::Gifify(char *filename, long *pmaxw, long *pmaxh, 
			struct rectangle *visrect) {
	ps *psobj = (ps *)GetDataObject();
	long width = (psobj)->GetPixelWidth();
	long height = (psobj)->GetPixelHeight();

	char command[300];
	char tfnm[L_tmpnam];
	tmpnam(tfnm);
	sprintf(command, "%s %s | %s | %s | %s > %s",
			"gs -q -sDEVICE=ppm -sOutputFile=-",
			tfnm,
			"pnmcrop", 
			"pnmmargin 3",
			"ppmtogif -interlace 2>/dev/null",
			filename);

        long pagew, pageh;
	print p;	// printing context
	p.SetPaperSize(width, height);
	p.SetFromProperties(this, &pagew, &pageh);
	print::PSNewPage(print_UsualPageNumbering);
	PrintPSRect(p.GetFILE(), pagew, pageh, visrect);
	if ( ! p.WritePostScript(tfnm, NULL)) 
		return FALSE;

	system(command);
	unlink(tfnm);

	if (pmaxw) *pmaxw = width+6;
	if (pmaxh) *pmaxh = height+6;
	return TRUE;
}


void
psview::SetDataObject(class dataobject  * dobj)
{
    class style * ds;
    (this)->iconview::SetDataObject(dobj);


#ifdef DPS_ENV
    if((this->menus)->SetMask( ((this->menus)->GetMask() & ~VIEW_MODE) | EDIT_MODE)) {
	(this)->PostMenus( NULL);
    }
#endif /* DPS_ENV */
    ds = ((class textview *)
				 this->bottomview)->GetDefaultStyle();
    (ds)->SetFontFamily( "AndyType");
    (ds)->SetFontSize( style_ConstantFontSize, 10);
}
