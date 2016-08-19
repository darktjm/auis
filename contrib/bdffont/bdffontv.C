/* bdffontv.ch  -  view for font editor for bdf files */

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/contrib/bdffont/RCS/bdffontv.C,v 1.3 1994/08/11 03:02:06 rr2b Stab74 $";
#endif

/*
	Copyright Carnegie Mellon University 1991, 1993 - All rights reserved
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

/* known problems  26 Mar, 1993 
BUGS
	while editing a character, the user is prompted to save it everytime 
		a checkpoint happens
	the initially displayed character is centered, but thereafter characters are upper left
		this is a problem in rasterv.c  (and missing methods in rasterv.ch)
	when zoomed, the initial display of a character should be with top left corner 
		at top left,  but it is not.  Again a rasterv problem.
	changing the width of the initially displayed character clears its raster (!)
		another raster problem ?
	^X^V does not refresh from the file 
	Suppose font foo12.bdf is being edited and character C has been changed, 
		but not updated.  If I then try to edit foo12.bdf in raw mode, I am 
		prompted to save the current character.  After saving it, the file
		is brought up in regular mode instead of raw mode.
	after changing pointsize (or other action), menus for raster are not reposted
		(raster is the currentinputfocus, but menus are not up ! )
		this is a bug in rasterv:PostMenus.  The mask doesn't change, so menus
		are not reposted.
	a click in the charedit raster gives it the input focus, but without calling 
		WantInputFocus of bdffontv  ?!?!

INTERFACE IMPROVEMENTS
	as the initial parameters for a new font are entered, they should be displayed
		in the relevant fields
	there ought to be a way to say "update the char" without getting 
		prompted each time
	there oughta be a way to review/edit other font parameters such as defaults for
		deltas, ascent, and descent
	it would be awfully nice to have the origin point displayed in the raster image
	should add the dash to character menu after adding a new character

BIG CHANGES
	display the characters in the char menu
	display character names in the char menu
	provide a mode for typing text in the revised font
*/


#include <andrewos.h>
#include <bdffontv.H>

#include <ansitext.h>
#include <mathaux.h>

#include <buffer.H>
#include <frame.H>
#include <im.H>

#include <view.H>
#include <bind.H>
#include <keymap.H>
#include <proctable.H>
#include <keystate.H>
#include <menulist.H>
#include <rasterimage.H>
#include <raster.H>
#include <rasterview.H>

#include <message.H>
#include <chlist.H>
#include <chlistview.H>
#include <text.H>
#include <textview.H>
#include <lpair.H>
#include <style.H>
#include <fontdesc.H>
#include <im.H>

#include <rect.h>
#include <ctype.h>
#include <txtstvec.h>
#include <util.h>



#define RoundUp(x) ((long) ((x) + 0.5))

static long bdffontv_DefaultPointSize = 12;
static long bdffontv_DefaultResolution = 100;
static long bdffontv_DefaultWidth = 12;




ATKdefineRegistry(bdffontv, lpair, bdffontv::InitializeClass);
#ifndef NORCSID
#endif
static void EnsureFocus(class bdffontv  *self);
static long bdffontv_ComputeZoom(class bdffontv  *self, long  zoomhow);
static void bdffontv_ZoomCmd(class bdffontv  *self	, long  zoomhow);
static void bdffontv_SetZoomDeltaCmd(class bdffontv  *self, long  rock);
static void bdffontv_ForwardCmd(class bdffontv  *self, long  proc);
static int AskFor1(class bdffontv  *self, long  *value, char  *prompt);
static int AskFor2(class bdffontv  *self, long  *value1 , long  *value2, char  *prompt1 , char  *prompt2);
static boolean FindBuffer(class frame  *f,class buffer  *b);
static class view *FindViewOfBuffer(class buffer  *b);
static int bdffontv_HelpCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn);
static int bdffontv_FontExtentCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn);
static int bdffontv_SetFontFaceCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn);
static int bdffontv_SetResolutionCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn);
static int bdffontv_SetPointSizeCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn);
static int bdffontv_SetDefaultDeltaCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn);
static int bdffontv_SetDefaultOriginCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn);
static int bdffontv_SetDefaultExtentCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn);
static int bdffontv_SetFontNameCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn);
static int bdffontv_SetFontFamilyCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn);
static int bdffontv_SetDeltaCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn);
static int bdffontv_SetOriginCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn);
static int bdffontv_SetExtentCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn);
static int bdffontv_SetNameCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn);
static int bdffontv_SetEncodingCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn);
static void bdffontv_UpdateFontExtent(class bdffontv  *self, class bdffont  *fontinfo);
static int bdffontv_SelectCharCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn);
static void CreateIndexString(char  *menustr, int  menuval);
static void EnsureCharMenu(class bdffontv  *self, long  num);
static void SetFontCharacteristics(class bdffontv  *self);
static void GetNewFontParameters(class bdffontv  *self, long  time	/* ignored */);
static boolean bdffontv_InitializeViews(class bdffontv  *self);


static void EnsureFocus(class bdffontv  *self)
{
#if 0
    if (self) {
	class im *im=(self)->GetIM();
	class view *v=NULL;
	if (im != NULL) {
	    v=(im)->GetInputFocus();
	}
	if ( ! self->chareditV) {}
/* 	else if ((struct view *)self->chareditV != v)
*/ else
		(self->chareditV)->WantInputFocus( self->chareditV);
    }
#endif
}
    
static char bdffontv_EncodingLabel[] = "Index: ";
static char bdffontv_NameLabel[] = "Name: ";
static char bdffontv_DeltaXLabel[] = "dx: ";
static char bdffontv_DeltaYLabel[] = "dy: ";
static char bdffontv_OriginXLabel[] = "origin X: ";
static char bdffontv_OriginYLabel[] = "origin Y: ";
static char bdffontv_WidthLabel[] = "Width: ";
static char bdffontv_HeightLabel[] = "Height: ";
static char bdffontv_ResolutionXLabel[] = "X res: ";
static char bdffontv_ResolutionYLabel[] = "Y res: ";
static char bdffontv_PointLabel[] = "Pt size: ";
static char bdffontv_FaceLabel[] = "Face: ";
static char bdffontv_FamilyLabel[] = "Family: ";
static char bdffontv_HelpLabel[] = " HELP! ";

#define bdffontv_CharNameMod	(0x01)
#define bdffontv_CharDeltaMod	(0x02)
#define bdffontv_CharOriginMod	(0x04)
#define bdffontv_CharExtentMod	(0x08)
#define bdffontv_NewCharDefnMod	(0x0F)
#define	bdffontv_CreatedMod	(0x10)

#define bdffontv_ZoomInCmd	 (1)
#define bdffontv_ZoomOutCmd	(-1)
#define bdffontv_ZoomNormalCmd	 (2)
#define bdffontv_ZoomPreviousCmd (3)
#define bdffontv_ZoomToFitCmd	 (4)

static long bdffontv_ComputeZoom(class bdffontv  *self, long  zoomhow)
        {
    struct rectangle viewbds;
    long oldzoom, newzoom, zw, zh;

    oldzoom = (self->chareditV)->GetScale();

    switch (zoomhow) {
	case bdffontv_ZoomNormalCmd:
	    newzoom = 1;
	    break;
	case bdffontv_ZoomPreviousCmd:
	    newzoom = self->prevzoom;
	    break;
	case bdffontv_ZoomToFitCmd:
	    (self->chareditV)->GetVisualBounds( &viewbds);
	    zw = rectangle_Width(&viewbds) / (self->charedit)->GetWidth();
	    zh = rectangle_Height(&viewbds) / (self->charedit)->GetHeight();
	    newzoom = (zw < zh) ? zw : zh;
	    break;
	default:	
	    newzoom = oldzoom + zoomhow * self->zoomdelta;
	    break;
    }

    if (newzoom < 1) {
	newzoom = 1;
    }

    return (newzoom);
} /* bdffontv_ComputeZoom */

static void bdffontv_ZoomCmd(class bdffontv  *self	, long  zoomhow)
        {
    long oldzoom, newzoom;

    oldzoom = (self->chareditV)->GetScale();
    newzoom = bdffontv_ComputeZoom(self, zoomhow);

    if (newzoom == oldzoom) {
	return;
    }

    if (oldzoom != 1) {
	self->prevzoom = oldzoom;
    }

    (self->chareditV)->SetScale( newzoom);
} /* bdffontv_ZoomCmd */

static void bdffontv_SetZoomDeltaCmd(class bdffontv  *self, long  rock)
        {
    char msg[128];
    char localbuffer[128], *bufp;
    long newdelta;

    sprintf(msg, "New zoom in/out delta [%ld]: ", self->zoomdelta);

    if (message::AskForString(self, 0, msg, "", localbuffer, sizeof(localbuffer) - 1) < 0)
    {
	message::DisplayString(self, 0, "Cancelled.");
	return;
    }

    bufp = &localbuffer[0];
    while (isspace(*bufp)) {
	bufp++;
    }

    if (*bufp != '\0') {
	if (sscanf(localbuffer, "%ld", &newdelta) != 1) {
	    message::DisplayString(self, 0, "Value must be a number.");
	    return;
	}
	self->zoomdelta = newdelta;

	sprintf(msg, "New display delta set to %ld.", self->zoomdelta);
	message::DisplayString(self, 0, msg);
    }
    else {
	message::DisplayString(self, 0, "No change to display delta.");
    }
} /* bdffontv_SetZoomDeltaCmd */

static void bdffontv_ForwardCmd(class bdffontv  *self, long  procl)
{
    chlist_itemfptr proc=(chlist_itemfptr)procl;
    (*proc)((long)self, NULL, view_LeftUp, 1, 0, 0);
} /* bdffontv_ForwardCmd */

#define bdffontv_AskAborted	(0)
#define bdffontv_AskUnchanged	(1)
#define bdffontv_AskChanged	(2)

static int AskFor1(class bdffontv  *self, long  *value, char  *prompt)
            {
    char msg[128];
    char localbuffer[128], *bufp;

    sprintf(msg, "%s [%ld]: ", prompt, *value);

    if (message::AskForString(self, 0, msg, "", localbuffer, sizeof(localbuffer) - 1) < 0)
    {
	message::DisplayString(self, 0, "Cancelled.");
	return (bdffontv_AskAborted);
    }

    bufp = &localbuffer[0];
    while (isspace(*bufp)) {
	bufp++;
    }

    if (*bufp != '\0') {
	if (sscanf(localbuffer, "%ld", value) != 1) {
	    message::DisplayString(self, 0, "Value must be a number.");
	    return (bdffontv_AskAborted);
	}

	return (bdffontv_AskChanged);
    }

    return (bdffontv_AskUnchanged);
} /* AskFor1 */

static int AskFor2(class bdffontv  *self, long  *value1 , long  *value2, char  *prompt1 , char  *prompt2)
            {
    switch (AskFor1(self, value1, prompt1)) {
	case bdffontv_AskAborted:
	    return (bdffontv_AskAborted);

	case bdffontv_AskUnchanged:
	    return (AskFor1(self, value2, prompt2));

	case bdffontv_AskChanged:
	    if (AskFor1(self, value2, prompt2) == bdffontv_AskAborted) {
		return (bdffontv_AskAborted);
	    }
	    return (bdffontv_AskChanged);
    }
    return bdffontv_AskAborted;
} /* AskFor2 */

static char *bdffontv_FaceChoices[] =
      { "Cancel",
	"Normal",
	"Bold",
	"Italic",
	"Bold Italic",
	"Fixed",
	"Bold Fixed",
	"Italic Fixed",
	"Bold Italic Fixed",
	"Shadowed",
	"Bold Shadowed",
	"Italic Shadowed",
	"Bold Italic Shadowed",
	"Fixed Shadowed",
	"Bold Fixed Shadowed",
	"Italic Fixed Shadowed",
	"Bold Italic Fixed Shadowed",
	NULL };

static boolean FindBuffer(class frame  *f,class buffer  *b)
        {
/*
  Little, dippy routine passed to frame_Enumerate to find the
  frame which contains the buffer we want.
*/

  return ((f)->GetBuffer() == b);
} /* FindBuffer */

static class view *FindViewOfBuffer(class buffer  *b)
    {
/*
  I don't know why *I* have to do this, it should be a buffer method.
  Anyways, this tries to find the frame of our buffer.  If there is no
  such frame, make one, make a new IM for it (new window), and put the
  buffer in the frame in the IM.  *phew*

	(he probably should have used frame_GetFrameInWindowForBuffer  -wjh)
*/

  class frame *f;
  class im *im;

  if ((f = frame::Enumerate((frame_effptr)FindBuffer, (long)b)) == NULL) {
    /* No frame--need to map buffer to new window */

    if ((f = new frame) == NULL) {
	fprintf(stderr,"bdffontv: Could not create new frame.\n");
	return ((class view *) NULL);
    }
    if ((im = im::Create(NULL)) == NULL) {
	fprintf(stderr,"bdffontv: Could not create new window.\n");
	(f)->Destroy();
	return ((class view *) NULL);
    }
    (im)->SetView( f);

    (f)->SetCommandEnable( TRUE);
    (f)->PostDefaultHandler( "message", (f)->WantHandler( "message"));
    (f)->SetBuffer( b, TRUE);
  }
  return ((f)->GetView());
} /* FindViewOfBuffer */

static int bdffontv_HelpCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn)
                        {
    class buffer *buf;
    class view *v;

    buf = buffer::GetBufferOnFile(AndrewDir("/help/bdffont.help"), 0); 
    if (buf == NULL) {
	message::DisplayString(self, 0, "Could not find help text file.");
    }
    else {
	v = FindViewOfBuffer(buf);
	if (v == NULL) {
	    message::DisplayString(self, 0, "Could not create help window.");
	}
	else {
	    ((v)->GetIM())->ExposeWindow();
	}
    }

    EnsureFocus(self);

    return (0);
} /* bdffontv_HelpCmd */

static int bdffontv_FontExtentCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn)
                        {
    message::DisplayString(self,
			  0,
			  "Cannot change font extent summary!");

    EnsureFocus(self);

    return (0);
} /* bdffontv_FontExtentCmd */

static int bdffontv_SetFontFaceCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn)
                        {
    class chlist *chl;
    char msg[128];
    long face, newface;
    class bdffont *fontinfo;

    if ((action == view_LeftUp) || (action == view_RightUp)) {
	fontinfo = (class bdffont *) (self)->GetDataObject();
	face = (fontinfo)->GetFontFace() + 1;
	if (message::MultipleChoiceQuestion(self,
					   0,
					   "Select font face:",
					   face,
					   &newface,
					   bdffontv_FaceChoices,
					   NULL) != -1)
	{
	    if ((newface != face) && (newface != 0 /* i.e. cancel */)) {
		(fontinfo)->SetFontFace( newface - 1);
		(fontinfo)->SetModified();

		chl =
		   (class chlist *) (self->fontfaceV)->GetDataObject();
		sprintf(msg,
			"%s%s",
			bdffontv_FaceLabel, bdffontv_FaceChoices[newface]);
		(chl)->ChangeItemByIndex( 0, msg);

		sprintf(msg,
			"New font face = %s.",
			bdffontv_FaceChoices[newface]);
		message::DisplayString(self, 0, msg);
	    }
	    else {
		message::DisplayString(self, 0, "No change to font face.");
	    }
	}
	else {
	    message::DisplayString(self, 0, "No change to font face.");
	}
    }

    EnsureFocus(self);

    return (0);
} /* bdffontv_SetFontFaceCmd */

static int bdffontv_SetResolutionCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn)
                        {
    long x, y;
    class chlist *chl;
    char msg[128];
    class bdffont *fontinfo;

    if ((action == view_LeftUp) || (action == view_RightUp)) {
	fontinfo = (class bdffont *) (self)->GetDataObject();
	(fontinfo)->GetResolution( &x, &y);
	if (AskFor2(self,
		    &x, &y,
		    "New X resolution",
		    "New Y resolution") == bdffontv_AskChanged)
	{
	    (fontinfo)->SetResolution( x, y);
	    (fontinfo)->SetModified();

	    chl = (class chlist *) (self->resV)->GetDataObject();
	    sprintf(msg, "%s%ld", bdffontv_ResolutionXLabel, x);
	    (chl)->ChangeItemByIndex( 0, msg);

	    sprintf(msg, "%s%ld", bdffontv_ResolutionYLabel, y);
	    (chl)->ChangeItemByIndex( 1, msg);

	    sprintf(msg,
		    "New X resolution = %ld, new Y resolution = %ld.", x, y);
	    message::DisplayString(self, 0, msg);
	}
	else {
	    message::DisplayString(self, 0, "No change to font resolution.");
	}
    }

    EnsureFocus(self);

    return (0);
} /* bdffontv_SetResolutionCmd */

static int bdffontv_SetPointSizeCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn)
                        {
    long pts;
    class chlist *chl;
    char msg[128];
    class bdffont *fontinfo;

    if ((action == view_LeftUp) || (action == view_RightUp)) {
	fontinfo = (class bdffont *) (self)->GetDataObject();
	pts = (fontinfo)->GetPointSize();
	if (AskFor1(self, &pts, "New point size") == bdffontv_AskChanged) {
	    (fontinfo)->SetPointSize( pts);
	    (fontinfo)->SetModified();

	    chl = (class chlist *) (self->pthelpV)->GetDataObject();
	    sprintf(msg, "%s%ld", bdffontv_PointLabel, pts);
	    (chl)->ChangeItemByIndex( 0, msg);

	    sprintf(msg, "New point size = %ld.", pts);
	    message::DisplayString(self, 0, msg);
	}
	else {
	    message::DisplayString(self, 0, "No change to font point size.");
	}
    }

    EnsureFocus(self);

    return (0);
} /* bdffontv_SetPointSizeCmd */

static int bdffontv_SetDefaultDeltaCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn)
                        {
    long x, y;
    char msg[128];
    class bdffont *fontinfo;

    if ((action == view_LeftUp) || (action == view_RightUp)) {
	fontinfo = (class bdffont *) (self)->GetDataObject();
	(fontinfo)->GetDefaultDelta( &x, &y);
	if (AskFor2(self,
		    &x, &y,
		    "New default X delta",
		    "New default Y delta") == bdffontv_AskChanged)
	{
	    (fontinfo)->SetDefaultDelta( x, y);
	    (fontinfo)->SetModified();

	    sprintf(msg,
		    "New default X delta = %ld, new default Y delta = %ld.",
		    x, y);
	    message::DisplayString(self, 0, msg);
	}
	else {
	    message::DisplayString(self, 0, "No change to default delta.");
	}
    }

    EnsureFocus(self);

    return (0);
} /* bdffontv_SetDefaultDeltaCmd */

static int bdffontv_SetDefaultOriginCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn)
                        {
    long x, y;
    char msg[128];
    class bdffont *fontinfo;

    if ((action == view_LeftUp) || (action == view_RightUp)) {
	fontinfo = (class bdffont *) (self)->GetDataObject();
	(fontinfo)->GetDefaultOrigin( &x, &y);
	if (AskFor2(self,
		    &x, &y,
		    "New default X origin",
		    "New default Y origin") == bdffontv_AskChanged)
	{
	    (fontinfo)->SetDefaultOrigin( x, y);
	    (fontinfo)->SetModified();

	    sprintf(msg,
		    "New default X origin = %ld, new default Y origin = %ld.",
		    x, y);
	    message::DisplayString(self, 0, msg);
	}
	else {
	    message::DisplayString(self, 0, "No change to default origin.");
	}
    }

    EnsureFocus(self);

    return (0);
} /* bdffontv_SetDefaultOriginCmd */

static int bdffontv_SetDefaultExtentCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn)
                        {
    long width, height;
    char msg[128];
    class bdffont *fontinfo;

    if ((action == view_LeftUp) || (action == view_RightUp)) {
	fontinfo = (class bdffont *) (self)->GetDataObject();
	(fontinfo)->GetDefaultExtent( &width, &height);
	if (AskFor2(self,
		    &width, &height,
		    "New default width",
		    "New default height") == bdffontv_AskChanged)
	{
	    (fontinfo)->SetDefaultExtent( width, height);
	    (fontinfo)->SetModified();

	    sprintf(msg,
		    "New default width = %ld, new default height = %ld.",
		    width, height);
	    message::DisplayString(self, 0, msg);
	}
	else {
	    message::DisplayString(self, 0, "No change to default extent.");
	}
    }

    EnsureFocus(self);

    return (0);
} /* bdffontv_SetDefaultExtentCmd */

static int bdffontv_SetFontNameCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn)
                        {
    char localbuffer[512], *bufp, *tail, *oldname;
    class chlist *chl;
    class bdffont *fontinfo;

    if ((action == view_LeftUp) || (action == view_RightUp)) {
	fontinfo = (class bdffont *) (self)->GetDataObject();
	oldname = (fontinfo)->GetFontName();

	if (message::AskForString(self, 0,
				 "New name: ",
				 oldname,
				 localbuffer, sizeof(localbuffer) - 1) < 0)
	{
	    message::DisplayString(self, 0, "Cancelled.");
	}
	else {
	    /* trim leading white space */
	    bufp = &localbuffer[0];
	    while (isspace(*bufp)) {
		bufp++;
	    }

	    /* trim trailing white space */
	    tail = bufp + strlen(bufp) - 1;
	    while ((tail > bufp) && isspace(*tail)) {
		*tail-- = '\0';
	    }

/* ... no embedded spaces??? */

	    if ((*bufp != '\0') && ( ! oldname || (strcmp(oldname, bufp) != 0))) {
		(fontinfo)->SetFontName( bufp);
		(fontinfo)->SetModified();

		chl = (class chlist *)
			(self->fontnameV)->GetDataObject();
		(chl)->ChangeItemByIndex( 0, (fontinfo)->GetFontName());

		sprintf(localbuffer,
			"New font name set to %s.",
			(fontinfo)->GetFontName());
		message::DisplayString(self, 0, localbuffer);
	    }
	    else {
		message::DisplayString(self, 0, "No change to font name.");
	    }
	}
    }

    EnsureFocus(self);

    return (0);
} /* bdffontv_SetFontNameCmd */

static int bdffontv_SetFontFamilyCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn)
                        {
    char localbuffer[512], *bufp, *tail, *oldfamily;
    class chlist *chl;
    class bdffont *fontinfo;

    if ((action == view_LeftUp) || (action == view_RightUp)) {
	fontinfo = (class bdffont *) (self)->GetDataObject();
	oldfamily = (fontinfo)->GetFontFamily();

	if (message::AskForString(self, 0,
				 "New font family: ",
				 oldfamily,
				 localbuffer, sizeof(localbuffer) - 1) < 0)
	{
	    message::DisplayString(self, 0, "Cancelled.");
	}
	else {
	    /* trim leading white space */
	    bufp = &localbuffer[0];
	    while (isspace(*bufp)) {
		bufp++;
	    }

	    /* trim trailing white space */
	    tail = bufp + strlen(bufp) - 1;
	    while ((tail > bufp) && isspace(*tail)) {
		*tail-- = '\0';
	    }

/* ... no embedded spaces??? */

	    if ((*bufp != '\0') && ( ! oldfamily || (strcmp(oldfamily, bufp) != 0))) {
		(fontinfo)->SetFontFamily( bufp);
		(fontinfo)->SetModified();

		chl = (class chlist *)
			(self->fontfamilyV)->GetDataObject();
		sprintf(localbuffer,
			"%s%s",
			bdffontv_FamilyLabel,
			(fontinfo)->GetFontFamily());
		(chl)->ChangeItemByIndex( 0, localbuffer);

		sprintf(localbuffer,
			"New font family set to %s.",
			(fontinfo)->GetFontFamily());
		message::DisplayString(self, 0, localbuffer);
	    }
	    else {
		message::DisplayString(self, 0, "No change to font family.");
	    }
	}
    }

    EnsureFocus(self);

    return (0);
} /* bdffontv_SetFontFamilyCmd */

static int bdffontv_SetDeltaCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn)
                        {
    long x, y;
    class chlist *chl;
    char msg[128];

    if ((action == view_LeftUp) || (action == view_RightUp)) {
	bdffont_GetDWidth(&self->modinfo, &x, &y);
	if (AskFor2(self,
		    &x, &y,
		    "New character X delta",
		    "New character Y delta") == bdffontv_AskChanged)
	{
	    bdffont_SetDWidth(&self->modinfo, x, y);
	    self->charmodified |= bdffontv_CharDeltaMod;

	    chl = (class chlist *) (self->charencodingV)->GetDataObject();
	    sprintf(msg, "%s%ld", bdffontv_DeltaXLabel, x);
	    (chl)->ChangeItemByIndex( 2, msg);

	    sprintf(msg, "%s%ld", bdffontv_DeltaYLabel, y);
	    (chl)->ChangeItemByIndex( 3, msg);

	    sprintf(msg, "New X delta = %ld, new Y delta = %ld.", x, y);
	    message::DisplayString(self, 0, msg);
	}
	else {
	    message::DisplayString(self, 0, "No change to character delta.");
	}
    }

    EnsureFocus(self);

    return (0);
} /* bdffontv_SetDeltaCmd */

static int bdffontv_SetOriginCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn)
                        {
    long x, y;
    class chlist *chl;
    char msg[128];

    if ((action == view_LeftUp) || (action == view_RightUp)) {
	bdffont_GetOrigin(&self->modinfo, &x, &y);
	if (AskFor2(self,
		    &x, &y,
		    "New character X origin",
		    "New character Y origin") == bdffontv_AskChanged)
	{
	    bdffont_SetOrigin(&self->modinfo, x, y);
	    self->charmodified |= bdffontv_CharOriginMod;

	    chl = (class chlist *) (self->charextentV)->GetDataObject();
	    sprintf(msg, "%s%ld", bdffontv_OriginXLabel, x);
	    (chl)->ChangeItemByIndex( 0, msg);

	    sprintf(msg, "%s%ld", bdffontv_OriginYLabel, y);
	    (chl)->ChangeItemByIndex( 1, msg);

	    sprintf(msg, "New X origin = %ld, new Y origin = %ld.", x, y);
	    message::DisplayString(self, 0, msg);
	}
	else {
	    message::DisplayString(self, 0, "No change to character origin.");
	}
    }

    EnsureFocus(self);

    return (0);
} /* bdffontv_SetOriginCmd */

static int bdffontv_SetExtentCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn)
                        {
    long width, height;
    class chlist *chl;
    char msg[128];

    if ((action == view_LeftUp) || (action == view_RightUp)) {
	bdffont_GetExtent(&self->modinfo, &width, &height);
	if (AskFor2(self,
		    &width, &height,
		    "New character width",
		    "New character height") == bdffontv_AskChanged)
	{
	    bdffont_SetExtent(&self->modinfo, width, height);
	    self->charmodified |= bdffontv_CharExtentMod;

	    chl = (class chlist *) (self->charextentV)->GetDataObject();
	    sprintf(msg, "%s%ld", bdffontv_WidthLabel, width);
	    (chl)->ChangeItemByIndex( 2, msg);

	    sprintf(msg, "%s%ld", bdffontv_HeightLabel, height);
	    (chl)->ChangeItemByIndex( 3, msg);

	    (self->chareditV)->ResizeRaster( width, height);

	    sprintf(msg, "New width = %ld, new height = %ld.", width, height);
	    message::DisplayString(self, 0, msg);
	}
	else {
	    message::DisplayString(self, 0, "No change to character extent.");
	}
    }

    EnsureFocus(self);

    return (0);
} /* bdffontv_SetExtentCmd */

static int bdffontv_SetNameCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn)
                        {
    char msg[128];
    char localbuffer[128], *bufp, *tail;
    class chlist *chl;

    if ((action == view_LeftUp) || (action == view_RightUp)) {
	sprintf(msg,
		"New name for character (max. length: 14) [%s]: ",
		bdffont_GetCharName(&self->modinfo));

	if (message::AskForString(self, 0, msg, "", localbuffer, sizeof(localbuffer) - 1) < 0)
	{
	    message::DisplayString(self, 0, "Cancelled.");
	}
	else {
	    /* trim leading white space */
	    bufp = &localbuffer[0];
	    while (isspace(*bufp)) {
		bufp++;
	    }

	    /* trim trailing white space */
	    tail = bufp + strlen(bufp) - 1;
	    while ((tail > bufp) && isspace(*tail)) {
		*tail-- = '\0';
	    }

	    /* check characters */
	    for (tail = bufp;  bufp && *tail; tail++) 
		if ( ! isprint(*tail) || isspace(*tail))
			bufp = NULL;

	    if ( ! bufp) {
		message::DisplayString(self, 0, "Names must be printable chars.");
	    }
	    else if (*bufp != '\0') {
		bdffont_SetCharName(&self->modinfo, bufp);

		self->charmodified |= bdffontv_CharNameMod;

		chl = (class chlist *)
			(self->charencodingV)->GetDataObject();
		sprintf(msg,
			"%s%s",
			bdffontv_NameLabel,
			bdffont_GetCharName(&self->modinfo));
		(chl)->ChangeItemByIndex( 1, msg);

		sprintf(msg,
			"New character name set to %s.",
			bdffont_GetCharName(&self->modinfo));
		message::DisplayString(self, 0, msg);
	    }
	    else {
		message::DisplayString(self, 0, "No change to character name.");
	    }
	}
    }

    EnsureFocus(self);

    return (0);
} /* bdffontv_SetNameCmd */

static int bdffontv_SetEncodingCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn)
                        {
    message::DisplayString(self,
			  0,
			  "Please use menu at left to select character to edit.");

    EnsureFocus(self);

    return (0);
} /* bdffontv_SetEncodingCmd */

static struct bind_Description bdffontv_Bindings[] =
{
/*  {"external-name", "key", KeyRock,
	"menu,entry~11", MenuRock, MenuMask,
	xyzzy_BindingProc, "One line description"}, */
    {"bdffont-set-font-name", NULL, 0,
	"Font Edit,Set Font Name~29", (long) bdffontv_SetFontNameCmd, 0,
	(proctable_fptr)bdffontv_ForwardCmd, "Set font name"},
    {"bdffont-set-font-family", NULL, 0,
	"Font Edit,Set Font Family~29", (long) bdffontv_SetFontFamilyCmd, 0,
	(proctable_fptr)bdffontv_ForwardCmd, "Set font family"},
    {"bdffont-set-font-face", NULL, 0,
	"Font Edit,Set Font Face~29", (long) bdffontv_SetFontFaceCmd, 0,
	(proctable_fptr)bdffontv_ForwardCmd, "Set font face"},
    {"bdffont-set-font-resolution", NULL, 0,
	"Font Edit,Set Font Resolution~29", (long) bdffontv_SetResolutionCmd, 0,
	(proctable_fptr)bdffontv_ForwardCmd, "Set font resolution"},
    {"bdffont-set-font-size", NULL, 0,
	"Font Edit,Set Point Size~29", (long) bdffontv_SetPointSizeCmd, 0,
	(proctable_fptr)bdffontv_ForwardCmd, "Set font point size"},
    {"bdffont-set-default-extent", NULL, 0,
	"Font Edit,Set Default Extent~29", (long) bdffontv_SetDefaultExtentCmd, 0,
	(proctable_fptr)bdffontv_ForwardCmd, "Set default character extent"},
    {"bdffont-set-default-origin", NULL, 0,
	"Font Edit,Set Default Origin~29", (long) bdffontv_SetDefaultOriginCmd, 0,
	(proctable_fptr)bdffontv_ForwardCmd, "Set default character origin"},
    {"bdffont-set-default-delta", NULL, 0,
	"Font Edit,Set Default Delta~29", (long) bdffontv_SetDefaultDeltaCmd, 0,
	(proctable_fptr)bdffontv_ForwardCmd, "Set default character delta"},
    {"bdffont-help", NULL, 0,
	"Font Edit,Help~10", (long) bdffontv_HelpCmd, 0,
	(proctable_fptr)bdffontv_ForwardCmd, "Display help information"},

	{"bdffont-zoom-in", NULL, bdffontv_ZoomInCmd,
	"Char Edit,Zoom In~21", bdffontv_ZoomInCmd, 0,
	(proctable_fptr)bdffontv_ZoomCmd, "Increase zoom factor by delta to max"},
    {"bdffont-zoom-out", NULL, bdffontv_ZoomOutCmd,
	"Char Edit,Zoom Out~22", bdffontv_ZoomOutCmd, 0,
	(proctable_fptr)bdffontv_ZoomCmd, "Decrease zoom factor by delta to min"},
    {"bdffont-zoom-normal", NULL, bdffontv_ZoomNormalCmd,
	"Char Edit,Normal Zoom~23", bdffontv_ZoomNormalCmd, 0,
	(proctable_fptr)bdffontv_ZoomCmd, "View at normal zoom"},
    {"bdffont-zoom-previous", NULL, bdffontv_ZoomPreviousCmd,
	"Char Edit,Previous Zoom~25", bdffontv_ZoomPreviousCmd, 0,
	(proctable_fptr)bdffontv_ZoomCmd, "View at previous zoom factor"},
    {"bdffont-zoom-fit", NULL, bdffontv_ZoomToFitCmd,
	"Char Edit,Zoom To Fit~24", bdffontv_ZoomToFitCmd, 0,
	(proctable_fptr)bdffontv_ZoomCmd, "View at highest zoom that fits"},
    {"bdffont-set-zoom-delta", NULL, 0,
	"Char Edit,Set Zoom Delta~26", 0, 0,
	(proctable_fptr)bdffontv_SetZoomDeltaCmd, "Set delta for zoom in/out"},

    {"bdffont-set-char-extent", NULL, 0,
	"Char Edit,Set Character Extent~11", (long) bdffontv_SetExtentCmd, 0,
	(proctable_fptr)bdffontv_ForwardCmd, "Set character extent"},
    {"bdffont-set-char-origin", NULL, 0,
	"Char Edit,Set Character Origin~12", (long) bdffontv_SetOriginCmd, 0,
	(proctable_fptr)bdffontv_ForwardCmd, "Set character origin"},
    {"bdffont-set-char-delta", NULL, 0,
	"Char Edit,Set Character Delta~13", (long) bdffontv_SetDeltaCmd, 0,
	(proctable_fptr)bdffontv_ForwardCmd, "Set character delta"},
    {"bdffont-set-char-name", NULL, 0,
	"Char Edit,Set Character Name~14", (long) bdffontv_SetNameCmd, 0,
	(proctable_fptr)bdffontv_ForwardCmd, "Set character name"},

    { 0 }
};

static class keymap *keys;
static class menulist *menus;
static class style *fieldstyle;
static class style *valuestyle;
static class style *fontnamestyle;
static class style *helpstyle;

boolean bdffontv::InitializeClass()
    {
    ::keys = new keymap;
    ::menus = new menulist;

    bind::BindList(bdffontv_Bindings, ::keys, ::menus, &bdffontv_ATKregistry_ );

    fieldstyle = new style;
    (fieldstyle)->AddNewFontFace( fontdesc_Italic);

    valuestyle = new style;
    (valuestyle)->AddNewFontFace( fontdesc_Plain);

    fontnamestyle = new style;
    (fontnamestyle)->AddNewFontFace( fontdesc_Bold);

    helpstyle = new style;
    (helpstyle)->AddNewFontFace( fontdesc_Bold);
    (helpstyle)->AddNewFontFace( fontdesc_Outline);
    (helpstyle)->SetJustification( style_Centered);

    return (TRUE);
} /* bdffontv__InitializeClass */


static void bdffontv_UpdateFontExtent(class bdffontv  *self, class bdffont  *fontinfo)
        {
    class chlist *chl;
    char msg[512];
    long x, y, w, h;

    (fontinfo)->GetBoundingBox( &w, &h, &x, &y);
    chl = (class chlist *) (self->fontoriginV)->GetDataObject();
    sprintf(msg, "%s%ld", bdffontv_OriginXLabel, x);
    (chl)->ChangeItemByIndex( 0, msg);
    sprintf(msg, "%s%ld", bdffontv_OriginYLabel, y);
    (chl)->ChangeItemByIndex( 1, msg);

    chl = (class chlist *) (self->fontextentV)->GetDataObject();
    sprintf(msg, "%s%ld", bdffontv_WidthLabel, w);
    (chl)->ChangeItemByIndex( 0, msg);
    sprintf(msg, "%s%ld", bdffontv_HeightLabel, h);
    (chl)->ChangeItemByIndex( 1, msg);
} /* bdffontv_UpdateFontExtent */

static char bdffontv_UpdateConfirm[] = "Retain the changes you made to";

static char *bdffontv_UpdateChoices[] = { "Yes", "No", "Cancel", NULL };

#define bdffontv_UpdateYes	0
#define bdffontv_UpdateNo	1
#define bdffontv_UpdateCancel	2

/* FALSE means cancel, TRUE means continue */
boolean bdffontv::UpdateCharModification()
    {
    char buf[100];
    long choice;
    class bdffont *fontinfo;
    long x, y;
    class rasterimage *pix;

    /* put up update dialog if anything has been modified, unless the only change is having created the character. */
    if ((this->charmodified!=0 && this->charmodified!=bdffontv_CreatedMod)
	 || (this->rastermodified != (this->charedit)->GetModified()))
    {
	sprintf(buf, "%s '%s'?", 
		bdffontv_UpdateConfirm, bdffont_GetCharName(&this->modinfo));
	if (-1 == message::MultipleChoiceQuestion(this, 0, buf, bdffontv_UpdateYes, &choice, bdffontv_UpdateChoices, NULL))
	{
	    return (FALSE);
	}

	if (choice == bdffontv_UpdateCancel) {
	    return (FALSE);
	}

	if (choice == bdffontv_UpdateYes) {
	    /* if character was just created, all fields have been updated */
	    if (this->charmodified & bdffontv_CreatedMod)
		this->charmodified = bdffontv_NewCharDefnMod;

	    fontinfo = (class bdffont *) (this)->GetDataObject();
	    (fontinfo)->SetModified();
   
	    if (! bdffont_IsActive(this->charinfo)) {
		(fontinfo)->ModifyActiveDefnsBy( 1);
		bdffont_SetBitmap(this->charinfo,
				  malloc(bdffont_BitmapSize(&this->modinfo)));
	    }

	    if (this->charmodified & bdffontv_CharNameMod) {
		bdffont_SetCharName(this->charinfo,
				    bdffont_GetCharName(&this->modinfo));
	    }

	    if (this->charmodified & bdffontv_CharDeltaMod) {
		bdffont_GetDWidth(&this->modinfo, &x, &y);
		(fontinfo)->SetCharDWidth(
				      bdffont_GetCharEncoding(this->charinfo),
				      x, y);
	    }

	    if (this->charmodified & bdffontv_CharOriginMod) {
		bdffont_GetOrigin(&this->modinfo, &x, &y);
		(fontinfo)->SetCharOrigin(
				      bdffont_GetCharEncoding(this->charinfo),
				      x, y);
		bdffontv_UpdateFontExtent(this, fontinfo);
	    }

	    if (this->charmodified & bdffontv_CharExtentMod) {
		bdffont_GetExtent(&this->modinfo, &x, &y);
		(fontinfo)->SetCharExtent(
				      bdffont_GetCharEncoding(this->charinfo),
				      x, y);
		bdffontv_UpdateFontExtent(this, fontinfo);
	    }

	    if (this->rastermodified != (this->charedit)->GetModified()) {
		pix = ((class raster *) this->charedit)->GetPix();
		memcpy(bdffont_GetBitmap(this->charinfo),
		       (pix)->GetBitsPtr(),
		       bdffont_BitmapSize(this->charinfo));
		this->rastermodified = (this->charedit)->GetModified();

/* ... some font manipulation here! */
		(this->charmenuV)->WantUpdate( this->charmenuV);
	    }
	}
    }

    return (TRUE);
} /* bdffontv__UpdateCharModification */ 

void bdffontv::SelectCharacter(int  index)
        {
    class bdffont *fontinfo;
    long x, y, w, h, zoom;
    class chlist *encodingl, *extentl;
    char msg[128];
    class rasterimage *pix;

    if ((this)->UpdateCharModification()) {
	fontinfo = (class bdffont *) (this)->GetDataObject();

	this->charinfo = (fontinfo)->GetDefinition( index);

	encodingl =
	    (class chlist *) (this->charencodingV)->GetDataObject();
	extentl =
	    (class chlist *) (this->charextentV)->GetDataObject();

	strcpy(msg, bdffontv_EncodingLabel);
	CreateIndexString(msg + strlen(bdffontv_EncodingLabel), index);

	(encodingl)->ChangeItemByIndex( 0, msg);

	if (! bdffont_IsActive(this->charinfo)) {
		/* creating a new character */

	    this->charmodified = bdffontv_CreatedMod;

	    /* generate name in usual style  (sp, char, or ch-ddd) */
	    if (index == 32)
		strcpy(msg, "sp");
	    else if (isprint((char) index)) {
		sprintf(msg, "%c", (char) index);
	    }
	    else {
		sprintf(msg, "ch-%d", index);
	    }
	    bdffont_SetCharName(&this->modinfo, msg);

	    (fontinfo)->GetDefaultExtent( &w, &h);
	    bdffont_SetExtent(&this->modinfo, w, h);

	    (fontinfo)->GetDefaultOrigin( &x, &y);
	    bdffont_SetOrigin(&this->modinfo, 0, 0);

	    (fontinfo)->GetDefaultDelta( &x, &y);
	    bdffont_SetSWidth(&this->modinfo, 0, 0); /* doesn't get used */
	    bdffont_SetDWidth(&this->modinfo, x, y);

	    bdffont_SetAttrs(&this->modinfo, 0);
	}
	else {
	    this->charmodified = 0;
	    this->modinfo = *this->charinfo;
	}

	sprintf(msg,
		"%s%s",
		bdffontv_NameLabel,
		bdffont_GetCharName(&this->modinfo));
	(encodingl)->ChangeItemByIndex( 1, msg);

	bdffont_GetDWidth(&this->modinfo, &x, &y);
	/* ... frasterv_SetDelta(self->chareditV, x, y); */
	sprintf(msg, "%s%ld", bdffontv_DeltaXLabel, x);
	(encodingl)->ChangeItemByIndex( 2, msg);

	sprintf(msg, "%s%ld", bdffontv_DeltaYLabel, y);
	(encodingl)->ChangeItemByIndex( 3, msg);

	bdffont_GetOrigin(&this->modinfo, &x, &y);
	/* ... frasterv_SetOrigin(self->chareditV, x, y); */
	sprintf(msg, "%s%ld", bdffontv_OriginXLabel, x);
	(extentl)->ChangeItemByIndex( 0, msg);

	sprintf(msg, "%s%ld", bdffontv_OriginYLabel, y);
	(extentl)->ChangeItemByIndex( 1, msg);

	bdffont_GetExtent(&this->modinfo, &w, &h);
	/* ... frasterv_ResizeRaster(self->chareditV, w, h); */
	/* ... */ (this->chareditV)->ResizeRaster( w, h);

	pix = (this->charedit)->GetPix();
	(pix)->Clear();
	if (bdffont_IsActive(this->charinfo)) {
	    memcpy((pix)->GetBitsPtr(),
		   bdffont_GetBitmap(this->charinfo),
		   bdffont_BitmapSize(this->charinfo));
	}
	(pix)->NotifyObservers( raster_BITSCHANGED);
	this->rastermodified = (this->charedit)->GetModified();

	/* ... char info is never NULL. need boolean once zoomtofit works */
	if (this->charinfo != NULL) {
	    zoom = (this->chareditV)->GetScale();
	}
	else {
	    zoom = bdffontv_ComputeZoom(this, bdffontv_ZoomToFitCmd);
	}
	(this->chareditV)->SetScale( zoom);

	sprintf(msg, "%s%ld", bdffontv_WidthLabel, w);
	(extentl)->ChangeItemByIndex( 2, msg);

	sprintf(msg, "%s%ld", bdffontv_HeightLabel, h);
	(extentl)->ChangeItemByIndex( 3, msg);

	(this->charmenuV)->HighlightItem( index);	

	(this)->WantUpdate( this);	/* ... need this? */
    }
} /* bdffontv__SelectCharacter */

static int bdffontv_SelectCharCmd(class bdffontv  *self, class chlist  *l, enum view_MouseAction  action, long  nclicks, int  index, int  rgn)
                        {
    if ((action != view_LeftUp) && (action != view_RightUp)) {
	return (0);
    }

    (self)->SelectCharacter( index);

    EnsureFocus(self);

    return (0);
} /* bdffontv_SelectCharCmd */

static void CreateIndexString(char  *menustr, int  menuval)
{
    if (menuval==32)
	strcpy(menustr, "space");
    else if (menuval>=512)
	sprintf(menustr, "\\%06o", menuval);
    else if (menuval>=256)
	sprintf(menustr, "\\%03o", menuval);
    else if (menuval>=128 || isprint(menuval))
	sprintf(menustr, "%c", menuval);
    else
	sprintf(menustr, "\\%03o", menuval);
}

static void EnsureCharMenu(class bdffontv  *self, long  num)
{
    char menustr[25];
    int menuval;
    struct bdffont_fontchar *fc;
    class bdffont *font = (class bdffont *)(self)->GetDataObject();

    /* we display all control characters in \0xx form, because they would probably look bad in the charmenuV. If your default text font has glyphs for chars 0..31, you don't get to see them. Tough. Doing it correctly would involve checking fontdesc_ValidChar, I think. */

    (self->charmenuL)->Clear();
    for (menuval=0; menuval < num; menuval++) {
	CreateIndexString(menustr, menuval);
	if (font 
			&& (fc=(font)->GetDefinition( menuval)) 
			&&  bdffont_IsActive(fc))
		strcat(menustr, " -");
	(self->charmenuL)->AddItemToEnd( menustr, (chlist_itemfptr)bdffontv_SelectCharCmd, (long)self);
    } 
    self->defns_size = num;
}

static void SetFontCharacteristics(class bdffontv  *self)
    {
    class bdffont *fontinfo;
    class chlist *chl;
    char msg[512];
    long x, y;
    char *fam;

    fontinfo = (class bdffont *) (self)->GetDataObject();
    fam = (fontinfo)->GetFontFamily();

    chl = (class chlist *) (self->fontnameV)->GetDataObject();
    (chl)->ChangeItemByIndex( 0, (fontinfo)->GetFontName());

    chl = (class chlist *) (self->fontfamilyV)->GetDataObject();
    sprintf(msg,
	    "%s%s",
	    bdffontv_FamilyLabel, (fam) ? fam : "");
    (chl)->ChangeItemByIndex( 0, msg);

    chl = (class chlist *) (self->fontfaceV)->GetDataObject();
    sprintf(msg,
	    "%s%s",
	    bdffontv_FaceLabel,
	    bdffontv_FaceChoices[(fontinfo)->GetFontFace() + 1]);
    (chl)->ChangeItemByIndex( 0, msg);

    chl = (class chlist *) (self->pthelpV)->GetDataObject();
    sprintf(msg, "%s%ld", bdffontv_PointLabel, (fontinfo)->GetPointSize());
    (chl)->ChangeItemByIndex( 0, msg);

    (fontinfo)->GetResolution( &x, &y);
    chl = (class chlist *) (self->resV)->GetDataObject();
    sprintf(msg, "%s%ld", bdffontv_ResolutionXLabel, x);
    (chl)->ChangeItemByIndex( 0, msg);
    sprintf(msg, "%s%ld", bdffontv_ResolutionYLabel, y);
    (chl)->ChangeItemByIndex( 1, msg);

    bdffontv_UpdateFontExtent(self, fontinfo);

    x = (fontinfo)->GetDefaultChar();
    if (x<0) x = 0;
    if (x>=fontinfo->defns_size) x = fontinfo->defns_size;
    (self)->SelectCharacter( x);

    EnsureCharMenu(self, fontinfo->defns_size);

} /* SetFontCharacteristics */

	static void
GetNewFontParameters(class bdffontv  *self, long  time	/* ignored */)
		{
	class bdffont *fontinfo;
	long ptsize, res, dwidth, dheight, descent;
	double size;
	char name[1024], *newname;

	fontinfo = (class bdffont *) (self)->GetDataObject();
	if (message::AskForString(self, 100,
				 "Name for NEW font: ", "",
				 name, sizeof(name) - 1) >= 0)
	{
	    /* trim trailing white space */
	    newname = &name[sizeof(name) - 1];
	    while ((newname > &name[0]) && isspace(*newname)) {
		*newname-- = '\0';
	    }

	    /* trim leading white space */
	    newname = &name[0];
	    while (isspace(*newname)) {
		newname++;
	    }

/* ... no embedded spaces??? */

	    if (*newname != '\0') {
		(fontinfo)->SetFontName( newname);
	    }
	}
	ptsize = bdffontv_DefaultPointSize;
	if (AskFor1(self,
		    &ptsize,
		    "Nominal point size") == bdffontv_AskChanged)
	{
	    bdffontv_DefaultPointSize = ptsize;
	}
	else {
	    ptsize = bdffontv_DefaultPointSize;
	}

	res = bdffontv_DefaultResolution;
	if (AskFor1(self,
		    &res,
		   "Target resolution") == bdffontv_AskChanged)
	{
	    bdffontv_DefaultResolution = res;
	}
	else {
	    res = bdffontv_DefaultResolution;
	}

	dwidth = bdffontv_DefaultWidth;
	if (AskFor1(self,
		    &dwidth,
		    "Default width") == bdffontv_AskChanged)
	{
	    bdffontv_DefaultWidth = dwidth;
	}
	else {
	    dwidth = bdffontv_DefaultWidth;
	}

	(fontinfo)->SetPointSize( ptsize);
	(fontinfo)->SetResolution( res, res);
	size = (fontinfo)->ComputeFontSize();
	dheight = /*mathaux_*/RoundUp(ansitext_ComputeAscent(size));
	descent = /*mathaux_*/RoundUp(ansitext_ComputeDescent(size));
	(fontinfo)->SetFontAscent( dheight);
	(fontinfo)->SetFontDescent( descent);
	(fontinfo)->SetBoundingBox( dwidth, dheight+descent, 0, -descent);
	(fontinfo)->SetDefaultExtent( dwidth, dheight);
	(fontinfo)->SetDefaultOrigin( 0, 0);
	(fontinfo)->SetDefaultDelta( dwidth, 0);

	SetFontCharacteristics(self);
    }


static boolean bdffontv_InitializeViews(class bdffontv  *self)
    {
    class chlist *chl;
    int i;
    int boxheight;

    if (self->initialized) {
	return TRUE;
    }

    self->fontinfo = new lpair;
    self->fontdata = new lpair;
    self->fontpresentation = new lpair;
    self->fontsummary = new lpair;
    self->ptres = new lpair;
    self->originextent = new lpair;
    self->menuedit = new lpair;
    self->infoedit = new lpair;
    self->encodingextent = new lpair;

    /* setup pthelpV so we can interrrogate font */
   chl = new chlist;
    i = (chl)->GetLength();
    (chl)->AddItemToEnd( bdffontv_PointLabel,
			(chlist_itemfptr)bdffontv_SetPointSizeCmd, (long)self);
    (chl)->AlwaysAddStyle(
			  i, sizeof(bdffontv_PointLabel) - 2,
			  fieldstyle);
    i = (chl)->GetLength();
    (chl)->AddItemToEnd( bdffontv_HelpLabel,
			(chlist_itemfptr)bdffontv_HelpLabel, (long)self);
    (chl)->AlwaysAddStyle(
			  i, sizeof(bdffontv_HelpLabel) - 2,
			  helpstyle);
    self->pthelpV = new chlistview;
    (self->pthelpV)->SetDataObject( chl);

    /* try to determine size of lines needed */
    {
	struct text_statevector sv;
	struct FontSummary *chvfont;

	(self->pthelpV)->GetStyleInformation( &sv, 1, NULL);

	chvfont = (fontdesc::Create(sv.CurFontFamily,
					sv.CurFontAttributes, sv.CurFontSize))->FontSummary(
			
			(self)->GetDrawable()  );

	boxheight = chvfont->maxHeight + chvfont->maxBelow + 5;
	if (boxheight < chvfont->newlineHeight + 5)
		boxheight = chvfont->newlineHeight + 5;
	if (boxheight < 20) boxheight = 20;
	if (boxheight > 150) boxheight = 150;
/* printf("newline ht:  %d  maxHt: %d   maxBelow: %d  ==>  boxht: %d\n",
chvfont->newlineHeight, chvfont->maxHeight, 
chvfont->maxBelow, boxheight); */
    }

    /* split window.  Put font info on top and character info below */
    (self)->SetUp(
		   self->fontinfo,
		   self->menuedit,
		   4*boxheight - 3, lpair_TOPFIXED, lpair_HORIZONTAL, TRUE);	/* FALSE */

    chl = new chlist;
    i = (chl)->GetLength();
    (chl)->AddItemToEnd( " ", (chlist_itemfptr)bdffontv_SetFontNameCmd, (long)self);
    (chl)->AlwaysAddStyle( i, 1L, fontnamestyle);

    self->fontnameV = new chlistview;
    (self->fontnameV)->SetDataObject( chl);

    /* split top area horizontally: top is font name and below is data */
    (self->fontinfo)->SetUp(
		self->fontnameV,
		self->fontdata,
		boxheight, lpair_TOPFIXED, lpair_HORIZONTAL, TRUE);	/* FALSE */

    /* split data area in the middle: left is family, pointsize, resolution
		right is face, origin, and width&height */
    (self->fontdata)->SetUp(
		self->fontpresentation,
		self->fontsummary,
		50, lpair_PERCENTAGE, lpair_VERTICAL, TRUE);	/* FALSE */

    chl = new chlist;
    i = (chl)->GetLength();
    (chl)->AddItemToEnd( bdffontv_FamilyLabel,
			(chlist_itemfptr)bdffontv_SetFontFamilyCmd, (long)self);
    (chl)->AlwaysAddStyle(
			  i, sizeof(bdffontv_FamilyLabel) - 2,
			  fieldstyle);

    self->fontfamilyV = new chlistview;
    (self->fontfamilyV)->SetDataObject( chl);

    /* split left : family above pointsize/resolution */
    (self->fontpresentation)->SetUp(
		self->fontfamilyV,
		self->ptres,
		boxheight, lpair_TOPFIXED, lpair_HORIZONTAL, TRUE);	/* FALSE */

    chl = new chlist;
    i = (chl)->GetLength();
    (chl)->AddItemToEnd( bdffontv_FaceLabel,
			(chlist_itemfptr)bdffontv_SetFontFaceCmd, (long)self);
    (chl)->AlwaysAddStyle(
			  i, sizeof(bdffontv_FaceLabel) - 2,
			  fieldstyle);

    self->fontfaceV = new chlistview;
    (self->fontfaceV)->SetDataObject( chl);

    /* split right: face above origin and width&height */
    (self->fontsummary)->SetUp(
		self->fontfaceV,
		self->originextent,
		boxheight, lpair_TOPFIXED, lpair_HORIZONTAL, TRUE);	/* FALSE */

    chl = new chlist;
    i = (chl)->GetLength();
    (chl)->AddItemToEnd( bdffontv_ResolutionXLabel,
			(chlist_itemfptr)bdffontv_SetResolutionCmd, (long)self);
    (chl)->AlwaysAddStyle(
			  i, sizeof(bdffontv_ResolutionXLabel) - 2,
			  fieldstyle);
    i = (chl)->GetLength();
    (chl)->AddItemToEnd( bdffontv_ResolutionYLabel,
			(chlist_itemfptr)bdffontv_SetResolutionCmd, (long)self);
    (chl)->AlwaysAddStyle(
			  i, sizeof(bdffontv_ResolutionYLabel) - 2,
			  fieldstyle);

    self->resV = new chlistview;
    (self->resV)->SetDataObject( chl);

    /* split to put pt size on left and resolution on the right */
    (self->ptres)->SetUp(
		self->pthelpV,
		self->resV,
		50, lpair_PERCENTAGE, lpair_VERTICAL, TRUE);	/* FALSE */

    chl = new chlist;
    i = (chl)->GetLength();
    (chl)->AddItemToEnd( bdffontv_OriginXLabel,
			(chlist_itemfptr)bdffontv_FontExtentCmd, (long)self);
    i = (chl)->GetLength();
    (chl)->AddItemToEnd( bdffontv_OriginYLabel,
			(chlist_itemfptr)bdffontv_FontExtentCmd, (long)self);

    self->fontoriginV = new chlistview;
    (self->fontoriginV)->SetDataObject( chl);

    chl = new chlist;
    i = (chl)->GetLength();
    (chl)->AddItemToEnd( bdffontv_WidthLabel,
			(chlist_itemfptr)bdffontv_FontExtentCmd, (long)self);
    i = (chl)->GetLength();
    (chl)->AddItemToEnd( bdffontv_HeightLabel,
			(chlist_itemfptr)bdffontv_FontExtentCmd, (long)self);

    self->fontextentV = new chlistview;
    (self->fontextentV)->SetDataObject( chl);

    /* split to put origin on the left and width&height on the right */
    (self->originextent)->SetUp(
		self->fontoriginV,
		self->fontextentV,
		50, lpair_PERCENTAGE, lpair_VERTICAL, TRUE);	/* FALSE */

    self->charmenuL = new chlist;
    self->defns_size = 0;

    self->charmenuV = new chlistview;
    (self->charmenuV)->SetDataObject( self->charmenuL);

    EnsureCharMenu(self, 256);

    /* list of characters on the left / info and edit area on the right */
    (self->menuedit)->SetUp(
		(self->charmenuV)->GetApplicationLayer(),
		self->infoedit,
		80, lpair_PERCENTAGE, lpair_VERTICAL, TRUE);	/* FALSE */

    self->charedit = raster::Create(16, 16);
    self->rastermodified = (self->charedit)->GetModified();
    self->chareditV = /* ... frasterv_New() */ new rasterview;
    /* ... frasterv */ (self->chareditV)->SetDataObject( self->charedit);

   /* character info above raster for editing */
    (self->infoedit)->SetUp(
		self->encodingextent,
		/* ... frasterv */ (self->chareditV)->GetApplicationLayer(),
		4*boxheight-6, lpair_TOPFIXED, lpair_HORIZONTAL, TRUE);	/* FALSE */

    chl = new chlist;
    i = (chl)->GetLength();
    (chl)->AddItemToEnd( bdffontv_EncodingLabel,
			(chlist_itemfptr)bdffontv_SetEncodingCmd, (long)self);
    (chl)->AlwaysAddStyle(
			  i, sizeof(bdffontv_EncodingLabel) - 2,
			  fieldstyle);
    i = (chl)->GetLength();
    (chl)->AddItemToEnd( bdffontv_NameLabel, (chlist_itemfptr)bdffontv_SetNameCmd, (long)self);
    (chl)->AlwaysAddStyle(
			  i, sizeof(bdffontv_NameLabel) - 2,
			  fieldstyle);
    i = (chl)->GetLength();
    (chl)->AddItemToEnd( bdffontv_DeltaXLabel, (chlist_itemfptr)bdffontv_SetDeltaCmd, (long)self);
    (chl)->AlwaysAddStyle(
			  i, sizeof(bdffontv_DeltaXLabel) - 2,
			  fieldstyle);
    i = (chl)->GetLength();
    (chl)->AddItemToEnd( bdffontv_DeltaYLabel, (chlist_itemfptr)bdffontv_SetDeltaCmd, (long)self);
    (chl)->AlwaysAddStyle(
			  i, sizeof(bdffontv_DeltaYLabel) - 2,
			  fieldstyle);

    self->charencodingV = new chlistview;
    (self->charencodingV)->SetDataObject( chl);

    chl = new chlist;
    i = (chl)->GetLength();
    (chl)->AddItemToEnd( bdffontv_OriginXLabel,
			(chlist_itemfptr)bdffontv_SetOriginCmd, (long)self);
    (chl)->AlwaysAddStyle(
			  i, sizeof(bdffontv_OriginXLabel) - 2,
			  fieldstyle);
    i = (chl)->GetLength();
    (chl)->AddItemToEnd( bdffontv_OriginYLabel,
			(chlist_itemfptr)bdffontv_SetOriginCmd, (long)self);
    (chl)->AlwaysAddStyle(
			  i, sizeof(bdffontv_OriginYLabel) - 2,
			  fieldstyle);
    i = (chl)->GetLength();
    (chl)->AddItemToEnd( bdffontv_WidthLabel, (chlist_itemfptr)bdffontv_SetExtentCmd, (long)self);
    (chl)->AlwaysAddStyle(
			  i, sizeof(bdffontv_WidthLabel) - 2,
			  fieldstyle);
    i = (chl)->GetLength();
    (chl)->AddItemToEnd( bdffontv_HeightLabel,
			(chlist_itemfptr)bdffontv_SetExtentCmd, (long)self);
    (chl)->AlwaysAddStyle(
			  i, sizeof(bdffontv_HeightLabel) - 2,
			  fieldstyle);
    self->charextentV = new chlistview;
    (self->charextentV)->SetDataObject( chl);

    (self->encodingextent)->SetUp(
		self->charencodingV,
		self->charextentV,
		50, lpair_PERCENTAGE, lpair_VERTICAL, TRUE);	/* FALSE */

    self->initialized = TRUE;

    SetFontCharacteristics(self);

    return (self->initialized);
} /* bdffontv_InitializeViews */

void bdffontv::SetDataObject(class dataobject  *dobj)
{
    (this)->lpair::SetDataObject( dobj);
    if (this->charmenuL)
	SetFontCharacteristics(this);

} /* bdffontv__SetDataObject */

void bdffontv::ObservedChanged(class observable  *changed, long  value)
{
    class bdffont *dobj = (class bdffont *)(this)->GetDataObject();
    if (value == bdffont_Writing) {
	if ( ! (this)->UpdateCharModification()) 
		message::DisplayString(this, 0, "Character changes NOT saved");
    }
    else if (dobj && this->charmenuL) {
	EnsureCharMenu(this, dobj->defns_size);
    }
    (this)->lpair::ObservedChanged( changed, value);
}

bdffontv::bdffontv()
        {
	ATKinit;

    this->initialized = FALSE;
    this->charmodified = 0;
    this->rastermodified = 0;
    this->charinfo = NULL;
    this->zoomdelta = 1;
    this->prevzoom = 1;
    this->charmenuL = NULL;
    this->chareditV = NULL;

    this->keys = keystate::Create(this, ::keys);
    this->menus = (::menus)->DuplicateML( this);

    THROWONFAILURE( (TRUE));
} /* bdffontv__InitializeObject */

bdffontv::~bdffontv()
        {
	ATKinit;

    class chlist *chl;

    if (this->keys) {
	delete this->keys;
    }

    if (this->menus) {
	delete this->menus;
    }

    if (this->fontinfo) {
	/* destroy views from top down. Because, that's why. */
	(this)->SetNth( 0, NULL);
	(this)->SetNth( 1, NULL);

	(this->fontinfo)->Destroy();
	(this->fontdata)->Destroy();
	(this->fontpresentation)->Destroy();
	(this->fontsummary)->Destroy();
	(this->ptres)->Destroy();
	(this->originextent)->Destroy();
	(this->menuedit)->Destroy();
	(this->infoedit)->Destroy();
	(this->encodingextent)->Destroy();

	chl = (class chlist *) (this->charextentV)->GetDataObject();
	(this->charextentV)->Destroy();
	(chl)->Destroy();
	chl = (class chlist *) (this->charencodingV)->GetDataObject();
	(this->charencodingV)->Destroy();
	(chl)->Destroy();
	chl = (class chlist *) (this->charmenuV)->GetDataObject();
	(this->charmenuV)->Destroy();
	(chl)->Destroy();
	chl = (class chlist *) (this->fontextentV)->GetDataObject();
	(this->fontextentV)->Destroy();
	(chl)->Destroy();
	chl = (class chlist *) (this->fontoriginV)->GetDataObject();
	(this->fontoriginV)->Destroy();
	(chl)->Destroy();
	chl = (class chlist *) (this->resV)->GetDataObject();
	(this->resV)->Destroy();
	(chl)->Destroy();
	chl = (class chlist *) (this->pthelpV)->GetDataObject();
	(this->pthelpV)->Destroy();
	(chl)->Destroy();
	chl = (class chlist *) (this->fontfaceV)->GetDataObject();
	(this->fontfaceV)->Destroy();
	(chl)->Destroy();
	chl = (class chlist *) (this->fontfamilyV)->GetDataObject();
	(this->fontfamilyV)->Destroy();
	(chl)->Destroy();
	chl = (class chlist *) (this->fontnameV)->GetDataObject();
	(this->fontnameV)->Destroy();
	(chl)->Destroy();
    }
    if (this->chareditV) {
	(this->chareditV)->Destroy();
    }

    if (this->charedit) {
	(this->charedit)->Destroy();
    }


} /* bdffontv__FinalizeObject */

void bdffontv::PostKeyState(class keystate  *keystate)
        {
    (this->keys)->Reset();
    (this->keys)->AddBefore( keystate);
    (this)->lpair::PostKeyState( this->keys);
} /* bdffontv__PostKeyState */

void bdffontv::PostMenus(class menulist  *menulist)
        {
    (this->menus)->ClearChain();
    if (menulist) {
        (this->menus)->ChainBeforeML( menulist, (long) menulist);
    }
    (this)->lpair::PostMenus( this->menus);
} /* bdffontv__PostMenus */


void bdffontv::FullUpdate(enum view_UpdateType  type, long  left, long  top, long  width, long  height)
                        {
    class bdffont *fontinfo;

    if ( ! this->initialized && type != view_MoveNoRedraw && type != view_Remove)
	bdffontv_InitializeViews(this);

    (this)->lpair::FullUpdate( type, left, top, width, height);

    /*
	Check here if no info in order to generate questions of user
	to fill in parameters
    */

    fontinfo = (class bdffont *) (this)->GetDataObject();
    if ((fontinfo)->GetFontName() == NULL) 
	im::EnqueueEvent((event_fptr)GetNewFontParameters, (char *)this, 0);

    /* EnsureFocus(self); */
    if (this->chareditV && type != view_MoveNoRedraw && type != view_Remove)
	(this->chareditV)->WantInputFocus( this->chareditV);

} /* bdffontv__FullUpdate */

	void
bdffontv::ReceiveInputFocus()
	{
	if (this->chareditV)
		(this->chareditV)->WantInputFocus( this->chareditV);
}

	void
bdffontv::WantInputFocus(class view  *target)
		{
	if (this->chareditV) {
		if (target == (class view *)this->chareditV)
			(this)->lpair::WantInputFocus( target);
		else (this->chareditV)->WantInputFocus( this->chareditV);
	}
}
