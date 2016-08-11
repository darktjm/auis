/* figv.c - drawing object view */
/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
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
#ifndef NORCSID
char *figv_c_rcsid = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/figure/RCS/figview.C,v 3.25 1996/02/16 16:46:01 robr Stab74 $";
#endif 

#include <andrewos.h>
ATK_IMPL("figview.H")
#include <math.h>
#include <figview.H>

#include <figure.H>
#include <figtoolview.H>
#include <figobj.H>
#include <figogrp.H>
#include <figoins.H>

#include <figio.H>

#include <region.H>
#include <graphic.H>
#include <view.H>
#include <dataobject.H>
#include <fontdesc.H>
#include <menulist.H>
#include <keymap.H>
#include <scroll.H>
#include <keystate.H>
#include <proctable.H>
#include <im.H>
#include <buffer.H>
#include <frame.H>
#include <texttroff.H>
#include <completion.H>
#include <environ.H>
#include <print.H>

#include <rect.h>

#define figview_InitNumHighlights (2)
#define SCROLL_EXTRA_SPACE (256)
#define recttopix_Exact (-9999)

static char debug=0;     
#define DEBUG(s) {if (debug) {printf s ; fflush(stdout);}}
#define ENTER(r) DEBUG(("Enter %s(0x%lx)\n", Stringize(r), self))
#define LEAVE(r) DEBUG(("Leave %s(0x%lx)\n", Stringize(r), self))

struct printlump {
    FILE *file;
    char *prefix;
    class figview *figview;
    char *processor, *format; /* if these are NULL, we're using the new PS printing code. */
    struct rectangle visrect; /* useful for optimizing */
};



ATKdefineRegistry(figview, view, figview::InitializeClass);

static void ToggleDebugProc(class figview  *self, long  rock);
static void SetExpertModeProc(class figview  *self, long  val);
static void ToggleReadOnlyProc(class figview  *self, long  val);
static void x_getinfo(class figview  *self, struct range  *total , struct range  *seen , struct range  *dot);
static long x_whatisat(class figview  *self, long  coordinate , long  outof);
static void x_setframe(class figview  *self, int  position, long  coordinate , long  outof);
static void y_getinfo(class figview  *self, struct range  *total , struct range  *seen , struct range  *dot);
static long y_whatisat(class figview  *self, long  coordinate , long  outof);
static void y_setframe(class figview  *self, int  position, long  coordinate , long  outof);
static void RepostMenus(class figview  *self);
static void IncreaseTmpProc(class figview  *self, long  num);
static void IncreaseRedrawProc(class figview  *self, long  num);
static void IncreaseClipRegProc(class figview  *self, long  num);
static void FlattenRefList(class figview  *self, long  ix);
static void RectToPix(class figview  *self, struct rectangle  *dest , struct rectangle  *src, long  delta);
static void RedrawView(class figview  *self, boolean  recterased);
static void RedrawGroup(class figview  *self, class figure  *fig, long  gref, struct rectangle  *B);
static void UpdateCache(class figview  *self, boolean  needfull );
static void OldUpdateCache(class figview  *self);
static void UpdateWindowSize(class figview  *self);
static void FixPixelPanning(class figview  *self);

static void DoRedraws(class figview  *self, struct rectangle  *ux, struct rectangle  *uy, struct rectangle  *dr, long  diffx , long  diffy);
static void DoBlit(class figview  *self, long  diffx , long  diffy);
static boolean TEI_Splot(class figobj  *o, long  ref, class figure  *self, long  *vv);
static void EnumSelSplot(class figview  *self, class figure  *fig, long  grp, void  (*func)(), long  rock);
static void AbortObjectProc(class figview  *self, long  rock);
static void FocusUpProc(class figview  *self, long  rock);
static void FocusDownProc(class figview  *self, long  rock);
static void FocusLeftProc(class figview  *self, long  rock);
static void FocusRightProc(class figview  *self, long  rock);
static void CutSelSplot(class figview  *self, class figure  *fig, long  gref, FILE  *fp);
static void CutSelProc(class figview  *self, long  rock);
static void CopySelProc(class figview  *self, long  rock);
static void CopySelInsetProc(class figview  *self, long  rock);
static void RotatePasteProc(class figview  *self, long  rock);
static void PasteSelProc(class figview  *self, long  rock);
static void UnselectProc(class figview  *self, long  rock);
static void ShowPrintAreaProc(class figview  *self, long  rock /* 0 for off, 1 for on, 2 for recalc */);
static void SetPrintLandscapeProc(class figview  *self, long  rock);
static void SetPrintScaleProc(class figview  *self, long  rock);
static void ReadZipProc(class figview  *self, long  rock);
static boolean PrintSplot(class figobj  *o, long  ref, class figure  *fig, struct printlump  *lump);
static void ToolsetCreateProc(class figview  *self, char  *rock);
static void ToolsetKillProc(class figview  *self, char  *rock);
static void ChangeZoomProc(class figview  *self, long  rock);
static void PanToOriginProc(class figview  *self, long  rock);
static void ScrollToObj(class figview *self, class figobj *o);


static void ToggleDebugProc(class figview  *self, long  rock)
{
    debug = ! debug;
    printf("debug is now %d\n", debug);  fflush (stdout);
}

static class menulist *EmbeddedMenus;
static class keymap *EmbeddedKeymap;
static boolean DefaultExpertMode = FALSE;
static char *FigureBackgroundColor = NULL;

#define	ML_expertmode	    (1)
#define	ML_nonexpertmode    (2)
#define	ML_toolset	    (4)
#define	ML_nontoolset	    (8)
#define	ML_focuschange	    (16)
#define	ML_selected	    (32)
#define ML_oneinsetselected (64)
#define	ML_noshowprintarea  (128)
#define	ML_showprintarea    (256)
#define	ML_printlandscape   (512)
#define	ML_printportrait    (1024)

boolean figview::InitializeClass()
{
    struct proctable_Entry *proc = NULL;

    EmbeddedMenus = new menulist;
    EmbeddedKeymap = new keymap;

    proc = proctable::DefineProc("figview-toggle-debug",(proctable_fptr) ToggleDebugProc, &figview_ATKregistry_ , NULL, "Turn debugging on or off.");

    proc = proctable::DefineProc("figview-toggle-readonly",(proctable_fptr) ToggleReadOnlyProc, &figview_ATKregistry_ , NULL, "Turn read-only flag on or off.");

    proc = proctable::DefineProc("figview-toolset-create",(proctable_fptr) ToolsetCreateProc, &figview_ATKregistry_ , NULL, "Creates toolset window.");
    (EmbeddedMenus)->AddToML( "Figure~5,Toolset~30", proc, 0, ML_nontoolset);

    proc = proctable::DefineProc("figview-toolset-destroy",(proctable_fptr) ToolsetKillProc, &figview_ATKregistry_ , NULL, "Deletes toolset window.");
    (EmbeddedMenus)->AddToML( "Figure~5,Remove Toolset~31", proc, 0, ML_toolset);

    proc = proctable::DefineProc("figview-set-expert-mode",(proctable_fptr) SetExpertModeProc, &figview_ATKregistry_ , NULL, "Turns on expert mode for self and toolset.");
    (EmbeddedMenus)->AddToML( "Figure~5,Expert Mode~40", proc, 1, ML_nonexpertmode);

    proc = proctable::DefineProc("figview-zoom",(proctable_fptr) ChangeZoomProc, &figview_ATKregistry_ , NULL, "Change scale of view.");
    (EmbeddedMenus)->AddToML( "Figure~5,Zoom In~10", proc, 1, 0);
    (EmbeddedKeymap)->BindToKey( "\033Z", proc, 1); /* esc-Z */
    (EmbeddedMenus)->AddToML( "Figure~5,Zoom Out~11", proc, -1, 0);
    (EmbeddedKeymap)->BindToKey( "\033z", proc, -1); /* esc-z */
    (EmbeddedMenus)->AddToML( "Figure~5,Normal Size~12", proc, 0, 0);

    proc = proctable::DefineProc("figview-pan-to-origin",(proctable_fptr) PanToOriginProc, &figview_ATKregistry_ , NULL, "Pan to (0,0).");
    (EmbeddedMenus)->AddToML( "Figure~5,Pan to Origin~19", proc, 0, 0);

    proc = proctable::DefineProc("figview-zap-selection",(proctable_fptr) CutSelProc, &figview_ATKregistry_ , NULL, "Remove selected objects and put them in cut buffer.");
    (EmbeddedMenus)->AddToML( "Cut~1", proc, 0, ML_selected);
    (EmbeddedKeymap)->BindToKey( "\027", proc, 0); /* ^W */
    proc = proctable::DefineProc("figview-copy-selection",(proctable_fptr) CopySelProc, &figview_ATKregistry_ , NULL, "Put selected objects in cut buffer.");
    (EmbeddedMenus)->AddToML( "Copy~2", proc, 0, ML_selected);
    (EmbeddedKeymap)->BindToKey( "\033w", proc, 0); /* esc-w */
    proc = proctable::DefineProc("figview-copy-selected-inset",(proctable_fptr) CopySelInsetProc, &figview_ATKregistry_ , NULL, "Put dataobject of selected inset in cut buffer.");
    (EmbeddedMenus)->AddToML( "Copy Inset Contents~3", proc, 0, ML_oneinsetselected);

    proc = proctable::DefineProc("figview-unselect", (proctable_fptr) UnselectProc, &figview_ATKregistry_ , NULL, "Select nothing.");
    (EmbeddedMenus)->AddToML( "Unselect~11", proc, 0, ML_selected);

    proc = proctable::DefineProc("figview-yank-selection",(proctable_fptr) PasteSelProc, &figview_ATKregistry_ , NULL, "Paste an object from the cut buffer.");
    (EmbeddedMenus)->AddToML( "Paste~4", proc, 0, 0);
    (EmbeddedKeymap)->BindToKey( "\031", proc, 0); /* ^Y */
    proc = proctable::DefineProc("figview-rotate-yank-selection",(proctable_fptr) RotatePasteProc, &figview_ATKregistry_ , NULL, "Paste an object from the cut buffer and rotate the buffer.");
    (EmbeddedKeymap)->BindToKey( "\033y", proc, 0); /* esc-y */

    proc = proctable::DefineProc("figview-read-zip-file",(proctable_fptr) ReadZipProc, &figview_ATKregistry_ , NULL, "Read a zip datastream into the figure.");
    /*menulist_AddToML(EmbeddedMenus, "File,Read Zip File~18", proc, 0, 0);*/

    proc = proctable::DefineProc("figview-set-print-landscape",(proctable_fptr) SetPrintLandscapeProc, &figview_ATKregistry_ , NULL, "Set landscape or portrait printing.");
    (EmbeddedMenus)->AddToML( "File,Portrait Printing~25", proc, 0, ML_printlandscape);
    (EmbeddedMenus)->AddToML( "File,Landscape Printing~25", proc, 1, ML_printportrait);

    proc = proctable::DefineProc("figview-set-print-scale",(proctable_fptr) SetPrintScaleProc, &figview_ATKregistry_ , NULL, "Set print scale.");
    (EmbeddedMenus)->AddToML( "File,Set Print Scale~24", proc, 0, 0);
    (EmbeddedKeymap)->BindToKey( "\033P", proc, 0); /* esc-P */

    proc = proctable::DefineProc("figview-show-print-area",(proctable_fptr) ShowPrintAreaProc, &figview_ATKregistry_ , NULL, "Show page boundaries.");
    (EmbeddedMenus)->AddToML( "File,Show Print Area~26", proc, 1, ML_noshowprintarea);
    (EmbeddedMenus)->AddToML( "File,Hide Print Area~26", proc, 0, ML_showprintarea);

    proc = proctable::DefineProc("figview-focus-down",(proctable_fptr) FocusDownProc, &figview_ATKregistry_ , NULL, "move focus down to subgroup.");
    (EmbeddedKeymap)->BindToKey( "\033B", proc, 0); /* esc-B */
    proc = proctable::DefineProc("figview-focus-up",(proctable_fptr) FocusUpProc, &figview_ATKregistry_ , NULL, "move focus up to parent group.");
    (EmbeddedKeymap)->BindToKey( "\033A", proc, 0); /* esc-A */
    proc = proctable::DefineProc("figview-focus-left",(proctable_fptr) FocusLeftProc, &figview_ATKregistry_ , NULL, "move focus left to sibling group.");
    (EmbeddedKeymap)->BindToKey( "\033D", proc, 0); /* esc-D */
    proc = proctable::DefineProc("figview-focus-right",(proctable_fptr) FocusRightProc, &figview_ATKregistry_ , NULL, "move focus left to sibling group.");
    (EmbeddedKeymap)->BindToKey( "\033C", proc, 0); /* esc-C */

    proc = proctable::DefineProc("figview-abort-object",(proctable_fptr) AbortObjectProc, &figview_ATKregistry_ , NULL, "abort object being created.");
    (EmbeddedKeymap)->BindToKey( "\007", proc, 0); /* ^G */

    DefaultExpertMode = environ::GetProfileSwitch("FigureExpertMode", FALSE);
    FigureBackgroundColor = environ::GetProfile("FigureMatteColor");
    if (!FigureBackgroundColor)
	FigureBackgroundColor = "white";

    return TRUE;
}

figview::figview()
{
	ATKinit;

    int hx;

    this->Menus = (EmbeddedMenus)->DuplicateML( this);
    this->Keystate = keystate::Create(this, EmbeddedKeymap);
    this->BuildKeystate = NULL;
    (this->Menus)->SetMask( ML_nontoolset | ML_nonexpertmode | ML_noshowprintarea);

    this->toolset = NULL;
    this->expertmode = FALSE;
    this->focuschange = FALSE;

    this->objs_size = 8;
    this->objs = (struct figv_oref *)malloc(this->objs_size * sizeof(struct figv_oref));
    FlattenRefList(this, 0);

    this->tmp_size = 0;
    this->tmplist = NULL;

    this->redraw_size = 0;
    this->redrawlist = NULL;

    this->clipreg_size = 0;
    this->clipreglist = NULL;
    IncreaseClipRegProc(this, 1);
    this->tmpregion = region::CreateEmptyRegion();
    this->currentclipreg = NULL;
    this->figureclip = region::CreateEmptyRegion();

    rectangle_EmptyRect(&this->UpdateRect);
    rectangle_EmptyRect(&this->MustEraseRect);

    this->numhighlights = 0;
    this->highlights = NULL;
    (this)->SetNumHighlights( figview_InitNumHighlights);

    this->focusgroup = figure_NULLREF;
    this->numselected = 0;

    this->originx = 0;
    this->originy = 0;
    this->ppanx = this->panx = 0;
    this->ppany = this->pany = 0;
    this->scale = figview_NormScale;
    this->lastwidth = 0;
    this->lastheight = 0;

    this->lastupdater = figure_NULLREF;

    this->OnScreen = FALSE;
    this->HasInputFocus = FALSE;
    this->embedded = TRUE;
    this->ignoreUp = FALSE;
    this->NeedFullUpdate = FALSE;
    this->DoingFullUpdate = FALSE;
    this->UpdateCached = FALSE;
    this->UpdatesBlocked = FALSE;
    this->InputFocusClick = FALSE;
    this->ShowPrintArea = FALSE;
    this->PSFileName = NULL;
    this->PrintRect = NULL;
    this->ShowFocusAttachments = FALSE; 

    this->recsearchvalid = FALSE;
    this->recsearchchild = NULL;

    if (DefaultExpertMode)
	(this)->SetExpertMode( TRUE);

    graphic::GetDefaultColors(&this->ForegroundColor, &this->BackgroundColor);
    static char *foreground=environ::GetProfile("FigureForegroundColor");
    if(foreground) this->ForegroundColor=atom::Intern(foreground)->Name();
    else if(this->ForegroundColor) this->ForegroundColor=atom::Intern(this->ForegroundColor)->Name();
    static char *background=environ::GetProfile("FigureBackgroundColor");
    if(background) this->BackgroundColor=atom::Intern(background)->Name();
    else if(this->BackgroundColor) this->BackgroundColor=atom::Intern(this->BackgroundColor)->Name();
    this->FigBackColor = FigureBackgroundColor;
    /*if (!self->ForegroundColor)
	self->ForegroundColor = "black";
    if (!self->BackgroundColor)
	self->BackgroundColor = "white";*/

    THROWONFAILURE( TRUE);
}

figview::~figview()
{
    ATKinit;

    int ix;

    if (this->PSFileName)
	free(this->PSFileName);

    if (this->tmplist)
	free(this->tmplist);

    if (this->highlights)
	free(this->highlights);

    if (this->redrawlist)
	free(this->redrawlist);

    if (this->clipreglist) {
	for (ix=0; ix<this->clipreg_size; ix++) 
	    if (this->clipreglist[ix])
		delete this->clipreglist[ix];
	free(this->clipreglist);
    }

    if (this->tmpregion)
	delete this->tmpregion;
    if (this->figureclip)
	delete this->figureclip;

    if (this->toolset) 
	(this)->DestroyToolset();

    for (ix=0; ix<this->objs_size; ix++) {
	if (this->objs[ix].insetv) 
	    (this->objs[ix].insetv)->Destroy();
    }
    free(this->objs);

    delete this->Menus;
    delete this->Keystate;
}

void figview::SetDataObject(class dataobject  *f)
{
    class figure *fig=(class figure *)f;
    (this)->view::SetDataObject( fig);
    this->focusgroup = (fig)->RootObjRef();
    this->originx = (fig)->GetOriginX();
    this->originy = (fig)->GetOriginY();
    this->panx = this->originx;
    this->pany = this->originy;
    RepostMenus(this);
}

void figview::SetExpertMode(boolean  val)
{
    val = (val) ? TRUE : FALSE;

    if (val==this->expertmode)
	return;

    this->ShowFocusAttachments = val;
    this->expertmode = val;
    this->focuschange = val;
    if (this->toolset)
	(this->toolset)->SetExpertMode( val);
    (this)->ClearSelection();
    (this)->WantUpdate( this);
    RepostMenus(this);
}

static void SetExpertModeProc(class figview  *self, long  val)
{
    (self)->SetExpertMode( val);
}

static void ToggleReadOnlyProc(class figview  *self, long  val)
{
    class figure *fig = (class figure *)(self)->GetDataObject();
    if (!fig) return;

    fig->ReadOnly = !fig->ReadOnly;

    if (fig->ReadOnly) 
	message::DisplayString(self, 10, "Document is now read-only.");
    else
	message::DisplayString(self, 10, "Document is now writable.");
}

void figview::ObservedChanged(class observable  *obs, long  status)
{
    if (obs == (class observable *)this->toolset) {
	if (status==observable_OBJECTDESTROYED) {
	    fprintf(stderr, "figview: observed toolset destroyed!\n");
	}
	else {
	}
    }
    else {
	if (status == observable_OBJECTDESTROYED) {
	}
	else if (status == figure_DATACHANGED) 
	    (this)->WantUpdate( this);	
    }
}

class view *figview::GetApplicationLayer()
{
    class scroll *view;

    view = scroll::CreateScroller(this, scroll_LEFT | scroll_BOTTOM, environ::GetProfile("FigureScrollClass"));

    this->embedded = FALSE;
    return (class view *) view;
}

   
 
static struct scrollfns	vertical_scroll_interface =
{(scroll_getinfofptr)y_getinfo, (scroll_setframefptr)y_setframe, NULL, (scroll_whatfptr)y_whatisat};
static struct scrollfns	horizontal_scroll_interface =
{(scroll_getinfofptr)x_getinfo, (scroll_setframefptr)x_setframe, NULL, (scroll_whatfptr)x_whatisat};

char *figview::GetInterface(char  *interface_name)
{
    struct scrollfns *interface;
    
    if (strcmp(interface_name, "scroll,vertical") == 0)
	interface = &vertical_scroll_interface;
    else if (strcmp(interface_name, "scroll,horizontal") == 0)
	interface = &horizontal_scroll_interface;
    else
	interface = NULL;
    return (char *)interface;
}

static void x_getinfo(class figview  *self, struct range  *total , struct range  *seen , struct range  *dot)
{
    class figure *fig = (class figure *)(self)->GetDataObject();
    struct rectangle *figrect = (fig)->GetOverallBounds();
    struct rectangle visrect;
    struct rectangle *focrect;

    if (rectangle_IsEmptyRect(figrect)) {
	total->beg = 0;
	total->end = 1;
	seen->beg = -1;
	seen->end = -1;
	dot->beg = -1;
	dot->end = -1;
    }
    else {
	total->beg = 0;
	total->end = figrect->width + SCROLL_EXTRA_SPACE;

	(self)->GetVisualBounds( &visrect);
	seen->beg = (self)->ToFigX( visrect.left) - (figrect->left - SCROLL_EXTRA_SPACE);
	seen->end = (self)->ToFigX( visrect.left+visrect.width) - (figrect->left - SCROLL_EXTRA_SPACE);

	if (seen->beg < total->beg)
	    seen->beg = total->beg;
	if (seen->end > total->end)
	    seen->end = total->end;

	if (self->focusgroup == (fig)->RootObjRef())
	    focrect = NULL;
	else
	    focrect = (self->objs[self->focusgroup].o)->GetBounds( self);

	if (focrect && !rectangle_IsEmptyRect(focrect)) {
	    dot->beg = focrect->left - (figrect->left - SCROLL_EXTRA_SPACE);
	    dot->end = focrect->left+focrect->width - (figrect->left - SCROLL_EXTRA_SPACE);

	    if (dot->beg < total->beg)
		dot->beg = total->beg;
	    if (dot->end > total->end)
		dot->end = total->end;
	}
	else {
	    dot->beg = -1;
	    dot->end = -1;
	}
    }
}

static long x_whatisat(class figview  *self, long  coordinate , long  outof)
{
    class figure *fig = (class figure *)(self)->GetDataObject();
    struct rectangle *figrect = (fig)->GetOverallBounds();

    return (self)->ToFigX( coordinate) - (figrect->left - SCROLL_EXTRA_SPACE);
}

static void x_setframe(class figview  *self, int  position, long  coordinate , long  outof) 
{
    long diffpos;
    class figure *fig = (class figure *)(self)->GetDataObject();
    struct rectangle *figrect = (fig)->GetOverallBounds();

    diffpos = ((self)->ToFigX( coordinate) - (figrect->left - SCROLL_EXTRA_SPACE)) - position;

    if (diffpos) {
	self->panx -= diffpos;
	/* self->NeedFullUpdate = TRUE; */
	(self)->WantUpdate( self);
    }
}

static void y_getinfo(class figview  *self, struct range  *total , struct range  *seen , struct range  *dot)
{
    class figure *fig = (class figure *)(self)->GetDataObject();
    struct rectangle *figrect = (fig)->GetOverallBounds();
    struct rectangle visrect;
    struct rectangle *focrect;

    if (rectangle_IsEmptyRect(figrect)) {
	total->beg = 0;
	total->end = 1;
	seen->beg = -1;
	seen->end = -1;
	dot->beg = -1;
	dot->end = -1;
    }
    else {
	total->beg = 0;
	total->end = figrect->height + SCROLL_EXTRA_SPACE;

	(self)->GetVisualBounds( &visrect);
	seen->beg = (self)->ToFigY( visrect.top) - (figrect->top - SCROLL_EXTRA_SPACE);
	seen->end = (self)->ToFigY( visrect.top+visrect.height) - (figrect->top - SCROLL_EXTRA_SPACE);

	if (seen->beg < total->beg)
	    seen->beg = total->beg;
	if (seen->end > total->end)
	    seen->end = total->end;

	if (self->focusgroup == (fig)->RootObjRef())
	    focrect = NULL;
	else
	    focrect = (self->objs[self->focusgroup].o)->GetBounds( self);

	if (focrect && !rectangle_IsEmptyRect(focrect)) {
	    dot->beg = focrect->top - (figrect->top - SCROLL_EXTRA_SPACE);
	    dot->end = focrect->top+focrect->height - (figrect->top - SCROLL_EXTRA_SPACE);

	    if (dot->beg < total->beg)
		dot->beg = total->beg;
	    if (dot->end > total->end)
		dot->end = total->end;
	}
	else {
	    dot->beg = -1;
	    dot->end = -1;
	}
    }
}

static long y_whatisat(class figview  *self, long  coordinate , long  outof)
{
    class figure *fig = (class figure *)(self)->GetDataObject();
    struct rectangle *figrect = (fig)->GetOverallBounds();

    return (self)->ToFigY( coordinate) - (figrect->top - SCROLL_EXTRA_SPACE);
}

static void y_setframe(class figview  *self, int  position, long  coordinate , long  outof) 
{
    long diffpos;
    class figure *fig = (class figure *)(self)->GetDataObject();
    struct rectangle *figrect = (fig)->GetOverallBounds();

    diffpos = ((self)->ToFigY( coordinate) - (figrect->top - SCROLL_EXTRA_SPACE)) - position;

    if (diffpos) {
	self->pany -= diffpos;
	/* self->NeedFullUpdate = TRUE; */
	(self)->WantUpdate( self);
    }
}

/* assumes self is input focus */
static void RepostMenus(class figview  *self)
{
    class figure *fig = (class figure *)(self)->GetDataObject();
    long menumask = 0;
    long val;

    if (self->expertmode)
	menumask |= ML_expertmode;
    else
	menumask |= ML_nonexpertmode;

    if (self->toolset)
	menumask |= ML_toolset;
    else
	menumask |= ML_nontoolset;

    if (self->focuschange)
	menumask |= ML_focuschange;

    if (self->numselected)
	menumask |= ML_selected;

    if (self->ShowPrintArea)
	menumask |= ML_showprintarea;
    else
	menumask |= ML_noshowprintarea;

    if (fig && fig->GetPrintLandscape())
	menumask |= ML_printlandscape;
    else
	menumask |= ML_printportrait;

    val = (self)->GetOneSelected();
    if (val != figure_NULLREF && (self->objs[val].o)->IsInset())
	menumask |= ML_oneinsetselected;

    if ((self->Menus)->SetMask( menumask)) {
	(self)->PostMenus( NULL);
    }
}

void figview::PostMenus(class menulist  *ml)
{
    /* Enable the menus for this object. */
    (this->Menus)->UnchainML( (long)this);
    if (ml)
	(this->Menus)->ChainBeforeML( ml, (long)this);

    (this)->view::PostMenus( this->Menus);
}

void figview::PostKeyState(class keystate  *ks)
{
    /* Enable the keys for this object. */
    class keystate *newch;

    if (this->BuildKeystate) {
	newch = (this->BuildKeystate)->AddBefore( ks);
	newch = (this->Keystate)->AddAfter( newch);
	(this)->view::PostKeyState( newch);
    }
    else {
	newch = (this->Keystate)->AddAfter( ks);
	(this)->view::PostKeyState( newch);
    }
}

void figview::SetBuildKeystate(class keystate  *ks)
{
    this->BuildKeystate = ks;
    (this)->PostKeyState( NULL);
}

void figview::ReceiveInputFocus()
{
    this->HasInputFocus = TRUE;
    (this)->PostKeyState( NULL);
    (this)->PostMenus( NULL);
    /*figview_WantUpdate(self, self);  if there's any visual change */
}

void figview::LoseInputFocus()
{
    this->HasInputFocus = FALSE;
    /*figview_WantUpdate(self, self);    if there's any visual change */
}

/* kind of dull right now, but we may want to change it later */
view_DSattributes figview::DesiredSize(long  width, long  height, enum view_DSpass  pass, long  *dwidth, long  *dheight)
{
    return (this)->view::DesiredSize( width, height, pass, dwidth, dheight);
}

static void IncreaseTmpProc(class figview  *self, long  num)
{
    if (self->tmplist == NULL) {
	self->tmp_size = num+8;
	self->tmplist = (long *)malloc(sizeof(long) * self->tmp_size);
    }
    else {
	if (self->tmp_size >= num)
	    return;

	while (self->tmp_size < num)
	    self->tmp_size *= 2;
	self->tmplist = (long *)realloc(self->tmplist, sizeof(long) * self->tmp_size);
    }
}

static void IncreaseRedrawProc(class figview  *self, long  num)
{
    if (self->redrawlist == NULL) {
	self->redraw_size = num+8;
	self->redrawlist = (struct figv_redraw_item *)malloc(sizeof(struct figv_redraw_item) * self->redraw_size);
    }
    else {
	if (self->redraw_size >= num)
	    return;

	while (self->redraw_size < num)
	    self->redraw_size *= 2;
	self->redrawlist = (struct figv_redraw_item *)realloc(self->redrawlist, sizeof(struct figv_redraw_item) * self->redraw_size);
    }
}

static void IncreaseClipRegProc(class figview  *self, long  num)
{
    int ix;

    if (self->clipreglist == NULL) {
	self->clipreg_size = num+8;
	self->clipreglist = (class region **)malloc(sizeof(class region *) * self->clipreg_size);
	for (ix=0; ix<self->clipreg_size; ix++)
	    self->clipreglist[ix] = NULL;
    }
    else {
	if (self->clipreg_size >= num)
	    return;

	ix = self->clipreg_size;
	while (self->clipreg_size < num)
	    self->clipreg_size *= 2;
	self->clipreglist = (class region **)realloc(self->clipreglist, sizeof(class region *) * self->clipreg_size);

	for (; ix<self->clipreg_size; ix++)
	    self->clipreglist[ix] = NULL;
    }
}

void figview::SetNumHighlights(int  num)
{
    int oldnum = this->numhighlights;
    int hx;

    if (num <= oldnum)
	return;

    this->numhighlights = num;

    if (this->highlights) {
	this->highlights = (struct figv_highlight *)realloc(this->highlights, this->numhighlights * sizeof(struct figv_highlight));
    }
    else {
	this->highlights = (struct figv_highlight *)malloc(this->numhighlights * sizeof(struct figv_highlight));
    }

    for (hx=oldnum; hx<this->numhighlights; hx++) {
	rectangle_EmptyRect(&this->highlights[hx].r);
	rectangle_EmptyRect(&this->highlights[hx].old);
	this->highlights[hx].oldon = FALSE;
	this->highlights[hx].changed = FALSE;
    }
}

static void FlattenRefList(class figview  *self, long  ix)
{
    for (; ix<self->objs_size; ix++) {
	self->objs[ix].o = NULL;
	self->objs[ix].selected = FALSE;
	self->objs[ix].knownselected = FALSE;
	self->objs[ix].drawnselected = 0;
	self->objs[ix].selectdamaged = FALSE;
	self->objs[ix].insetv = NULL;
	self->objs[ix].inseto = NULL;
	self->objs[ix].insetbmoved = FALSE;
	self->objs[ix].wantupdate = FALSE;
	rectangle_EmptyRect(&(self->objs[ix].insetb));
	rectangle_EmptyRect(&(self->objs[ix].vbox));
	rectangle_EmptyRect(&(self->objs[ix].vselbox));
    }
}

/* convert a rectangle in fig coords to pix coords, expanding it by delta pixels. If delta = recttopix_Exact, do not expand it at all. */
static void RectToPix(class figview  *self, struct rectangle  *dest , struct rectangle  *src, long  delta)
{
    if (rectangle_IsEmptyRect(src)) {
	rectangle_EmptyRect(dest);
	return;
    }

    if (delta == recttopix_Exact) 
	rectangle_SetRectSize(dest,
			      (self)->ToPixX( rectangle_Left(src)),
			      (self)->ToPixY( rectangle_Top(src)), 
			      (self)->ToPixW( rectangle_Width(src)), 
			      (self)->ToPixH( rectangle_Height(src)));
    else
	rectangle_SetRectSize(dest,
			      -delta + (self)->ToPixX( rectangle_Left(src)),
			      -delta + (self)->ToPixY( rectangle_Top(src)), 
			      2*delta + 2 + (self)->ToPixW( rectangle_Width(src)), 
			      2*delta + 2 + (self)->ToPixH( rectangle_Height(src)));
}

/* Let B = UpdateRect clipped to visual bounds. Clip drawing to B, erase B, and redraw all elements (in order) that intersect B. 
If recterased is TRUE, we can assume that the drawing area has been erased already. */
static void RedrawView(class figview  *self, boolean  recterased)
{
    struct rectangle B, foc;
    struct rectangle inrec;
    class figure *fig 
      = (class figure *)(self)->GetDataObject();
    long hx, ix, jx, numclips, clipnum;
    class figobj *o;

    /* Check the focus group. Yeah, look, I know it's a cheesy place to do it. */
    {
	struct rectangle newgrprec;
	long ref = (self)->GetFocusRef();
	if (ref!=figure_NULLREF && ref != (fig)->RootObjRef()) {
	    class figogrp *o = (class figogrp *)fig->objs[ref].o;
	    newgrprec = *(o)->GetBounds( self);
	}
	else
	    rectangle_EmptyRect(&newgrprec);
	if (!rectangle_IsEqualRect(&newgrprec, &self->highlights[0].r))
	    (self)->SetHighlight( 0, &newgrprec);
    }

    /* the plan: r is in fig coords, old is in pixel coords */

    (self)->SetTransferMode( graphic_INVERT);
    for (hx=0; hx<self->numhighlights; hx++) {
	self->highlights[hx].focgone = FALSE;

	if (recterased) {
	    /* old one already erased */
	    self->highlights[hx].focgone = TRUE;
	}
	else if (self->DoingFullUpdate || self->highlights[hx].changed) {
	    /* erase old one */
	    self->highlights[hx].focgone = TRUE;
	    if (self->highlights[hx].oldon)
		(self)->DrawRect( &self->highlights[hx].old);
	}
	if (self->highlights[hx].focgone) {
	    /* defigop new one */
	    if (rectangle_IsEmptyRect(&self->highlights[hx].r))
		self->highlights[hx].oldon = FALSE;
	    else {
		self->highlights[hx].oldon = TRUE;
		RectToPix(self, &self->highlights[hx].old, &self->highlights[hx].r, figview_SpotRad);
	    }
	}
    }

    (self)->GetVisualBounds( &B);
    rectangle_IntersectRect(&B, &B, &self->UpdateRect);
    self->ClippedUpdateRect=B;
    
    if (!rectangle_IsEmptyRect(&B)) {

	self->redrawnum = 0;
	DEBUG(("Adding: "));
	RedrawGroup(self, fig, (fig)->RootObjRef(), &B);
	DEBUG(("end.\n"));

	/* IncreaseClipRegProc(1) is already ensured in InitObject(). */
	if (!self->clipreglist[0])
	    self->clipreglist[0] = region::CreateEmptyRegion();
	(self->clipreglist[0])->RectRegion( &B);
	if (self->embedded)
	    (self->clipreglist[0])->IntersectRegion( self->figureclip, self->clipreglist[0]);
	numclips = 1;

	
	/*printf("B is (%d..%d) (%d..%d)\n", B.left, B.left+B.width, B.top, B.top+B.height);*/

	DEBUG(("Clipping:\n"));	
	for (jx=self->redrawnum-1; jx>=0; jx--) {
	    self->redrawlist[jx].clip = numclips-1;
	    ix = self->redrawlist[jx].oref;
	    o = self->objs[ix].o;
	    if ((o)->IsInset()) {
		IncreaseClipRegProc(self, numclips+1);
		if (!self->clipreglist[numclips])
		    self->clipreglist[numclips] = region::CreateEmptyRegion();
		inrec = self->objs[ix].insetb;
		// rectangle_InsetRect(&inrec, -1, -1);
		(self->tmpregion)->RectRegion( &inrec);
		(self->clipreglist[numclips-1])->SubtractRegion( self->tmpregion, self->clipreglist[numclips]);
		numclips++;
	    }
	}

	(self)->SetTransferMode( graphic_COPY);
	
	clipnum = numclips-1;
	self->currentclipreg = self->clipreglist[clipnum];
	(self)->SetClippingRegion( self->currentclipreg);

	if (!recterased) {
	    (self)->SetBackgroundColor( self->FigBackColor, 65535, 65535, 65535);
	    (self)->EraseRect( &B); 
	    /*figview_FillRect(self, &B, figview_WhitePattern(self)); */
	}

	DEBUG(("Drawing: %d...", self->redrawnum));
	DEBUG(("[clip %d] ", clipnum));
	for (jx=0; jx<self->redrawnum; jx++) {
	    ix = self->redrawlist[jx].oref;
	    if (clipnum != self->redrawlist[jx].clip) {
		clipnum = self->redrawlist[jx].clip;
		self->currentclipreg = self->clipreglist[clipnum];
		(self)->SetClippingRegion( self->currentclipreg);
		DEBUG(("[clip %d] ", clipnum));
	    }
	    DEBUG(("%d ", ix));
	    o = self->objs[ix].o;
	    if ((o)->IsGroup()) {
	    }
	    else if ((o)->IsInset()) {
		(o)->Draw( self);
		if (self->objs[ix].insetv) {
		    struct rectangle *currec, tmprec;
		    boolean drawfull = recterased;

		    DEBUG(("%s ", (self->objs[ix].insetv)->GetTypeName()));
		    currec = &(self->objs[ix].insetb);
		    if (self->DoingFullUpdate || self->objs[ix].insetbmoved) {
			(self->objs[ix].insetv)->LinkTree( self);
			(self->objs[ix].insetv)->InsertView( self, currec);
			drawfull = TRUE;
		    }

		    if (!drawfull) {
			rectangle_IntersectRect(&tmprec, &self->MustEraseRect, currec);
			if (!rectangle_IsEmptyRect(&tmprec))
			    drawfull = TRUE;
		    }

		    region::CopyRegion(self->tmpregion, self->clipreglist[clipnum]);
		    (self->tmpregion)->OffsetRegion( -currec->left, -currec->top);
		    (self->objs[ix].insetv)->SetClippingRegion( self->tmpregion);

		    /*printf("ins %d: drawfull = %s\n", ix, (drawfull ? "TRUE" : "FALSE"));*/
		    if (drawfull) {
			(self)->SetBackgroundColor( self->BackgroundColor, 65535, 65535, 65535);
			(self)->EraseRect(  &(self->objs[ix].insetb));
			(self->objs[ix].insetv)->FullUpdate( view_FullRedraw, 0, 0, -1, -1);
		    }
		    else
			(self->objs[ix].insetv)->Update();

		}
		else {
		    DEBUG(("NULL "));
		}
	    }
	    else { /* neither inset nor group */
		(o)->Draw( self);
	    }
	    if (self->objs[ix].drawnselected)
		self->objs[ix].selectdamaged = TRUE;
	    self->objs[ix].insetbmoved = FALSE;
	}
	DEBUG(("end.\n"));

	(self)->SetForegroundColor( self->ForegroundColor, 0, 0, 0); 

	/* draw selection bits that were erased by the rectangle wipe */
	for (ix=0; ix<self->objs_size; ix++) {
	    if (self->objs[ix].o && self->objs[ix].selectdamaged) {
		self->objs[ix].selectdamaged = FALSE;
		(self->objs[ix].o)->Select( self);
		if (self->objs[ix].drawnselected==2) {
		    (self->objs[ix].o)->DrawAttachments( self);
		}
	    }
	}
	(self)->SetTransferMode( graphic_INVERT);
	for (hx=0; hx<self->numhighlights; hx++) {
	    if (!self->highlights[hx].focgone) {
		if (self->highlights[hx].oldon)
		    (self)->DrawRect( &self->highlights[hx].old);
	    }
	}

	self->currentclipreg = NULL;
	if (self->embedded)
	    (self)->SetClippingRegion( self->figureclip);
	else
	    (self)->ClearClippingRect(); 
    }

    for (ix=0; ix<self->objs_size; ix++) {
	if (self->objs[ix].o) {
	    class figobj *o = self->objs[ix].o;
	    /*boolean looksselected = 
	      (self->objs[ix].selected 
	       || (self->ShowFocusAttachments 
		   && figview_GetFocusRef(self)==ix 
		   && ((struct figogrp *)o)->doconstraints));*/

	    /* draw selection bits */
	    if (self->objs[ix].knownselected && self->objs[ix].drawnselected==0) {
		(o)->Select( self);
		if (self->ShowFocusAttachments && (self->objs[ix].o)->GetParentRef() == (self)->GetFocusRef()) {
		    (self->objs[ix].o)->DrawAttachments( self);
		    self->objs[ix].drawnselected = 2;
		}
		else {
		    self->objs[ix].drawnselected = 1;
		}
	    }
	    /* undraw selection bits */
	    else if (!self->objs[ix].knownselected && self->objs[ix].drawnselected) {
		(o)->Select( self);
		if (self->objs[ix].drawnselected == 2) {
		    (self->objs[ix].o)->DrawAttachments( self);
		}
		self->objs[ix].drawnselected = 0;
	    }
	}
    }

    (self)->SetTransferMode( graphic_INVERT);
    for (hx=0; hx<self->numhighlights; hx++) {
	if (self->highlights[hx].focgone) {
	    if (self->highlights[hx].oldon)
		(self)->DrawRect( &self->highlights[hx].old);
	}
    }

    /* erase rectangles in preparation for another go-round */
    rectangle_EmptyRect(&self->UpdateRect);
    rectangle_EmptyRect(&self->MustEraseRect);
    self->DoingFullUpdate = FALSE;
}

static void RedrawGroup(class figview  *self, class figure  *fig, long  gref, struct rectangle  *B)
{
    int ix;
    struct rectangle tmp;
    class figobj *o;

    /*printf("clipped to %d, %d, %d, %d\n", B->left, B->top, B->width, B->height);*/

    for (ix=gref; ix!=figure_NULLREF; ix=fig->objs[ix].next) {
	/* this is the one usage of vbox that should not be ignored if self->objs[ix].ignorevbox is TRUE. If we clobbered it somehow, we could get rid of ignorevbox. Unfortunately, we can't. */
	rectangle_IntersectRect(&tmp, B, &(self->objs[ix].vbox));
	if (!rectangle_IsEmptyRect(&tmp)) {
	    o = self->objs[ix].o;
	    DEBUG(("%d ", ix));
	    IncreaseRedrawProc(self, self->redrawnum+1);
	    self->redrawlist[self->redrawnum].oref = ix;
	    self->redrawnum++;
	    if ((o)->IsGroup()) {
		DEBUG(("("));
		RedrawGroup(self, fig, ((class figogrp *)o)->GetRoot(), B);
		DEBUG((") "));
	    }
	}
    }
}

/* update cached object / vbox lists; store old, changed, new rectangles in UpdateRect. */
static void UpdateCache(class figview  *self, boolean  needfull )
 /* this indicates a need to update all bounds, but not
  necessarily redraw everything. */
{
    long ix;
    long tstamp, tstampi;
    class dataobject *dobj;
    short needup;
    class figure *fig = (class figure *)(self)->GetDataObject();
    struct rectangle inrec, *tmprec;
    class figobj *o;

    if (fig->objs_size > self->objs_size) {
	ix = self->objs_size;
	while (fig->objs_size > self->objs_size)
	    self->objs_size *= 2;
	self->objs = (struct figv_oref *)realloc(self->objs, self->objs_size * sizeof(struct figv_oref));
	FlattenRefList(self, ix);
    }

   /* this section moved to OldUpdateCache
    needfull = self->NeedFullUpdate;
    if (needfull)
	self->DoingFullUpdate = TRUE;
    self->NeedFullUpdate = FALSE;
*/
    for (ix=0; ix<fig->objs_size; ix++) {
	tstamp = 0;
	tstampi = 0;
	needup = 0;
	if (self->objs[ix].o != fig->objs[ix].o) {

	    /* handle old one */
	    if (self->objs[ix].o) {
		DEBUG(("Cache: old %d\n", ix));
		if (self->objs[ix].selected)
		    self->numselected--;
		if (self->objs[ix].drawnselected == 0) {
		    if (!self->objs[ix].ignorevbox) {
			rectangle_UnionRect(&self->MustEraseRect, &self->MustEraseRect, &(self->objs[ix].vbox));
		    }
		}
		else {
		    rectangle_UnionRect(&self->MustEraseRect, &self->MustEraseRect, &(self->objs[ix].vselbox));
		}
		if (ix == self->focusgroup)
		    self->focusgroup = (fig)->RootObjRef();
	    }

	    /* handle new one */
	    if (fig->objs[ix].o) {
		class figobj *o = fig->objs[ix].o;
		DEBUG(("Cache: new %d\n", ix));
		self->objs[ix].o = o;
		self->objs[ix].selected = FALSE;
		self->objs[ix].drawnselected = FALSE;

		if (self->objs[ix].selected
		    || (self->ShowFocusAttachments
			&& (o)->IsGroup()
			&& (self)->GetFocusRef()==ix 
			&& ((class figogrp *)o)->doconstraints))
		    self->objs[ix].knownselected = TRUE;
		else
		    self->objs[ix].knownselected = FALSE;

		RectToPix(self, &self->objs[ix].vbox, (o)->GetBounds( self), figview_SpotRad);
		self->objs[ix].ignorevbox = ((o)->IsGroup());
		/* if (self->objs[ix].knownselected) then */
		RectToPix(self, &self->objs[ix].vselbox, (o)->GetSelectedBounds( self), figview_AnchRad+1);

		if ((o)->IsInset()) {
		    RectToPix(self, &inrec, (o)->GetBounds( self), recttopix_Exact);
		    self->objs[ix].insetb = inrec;
		    self->objs[ix].insetbmoved = TRUE;
		}

		if (self->objs[ix].knownselected) {
		    rectangle_UnionRect(&self->UpdateRect, &self->UpdateRect, &(self->objs[ix].vselbox));
		}
		else {
		    if (!self->objs[ix].ignorevbox) {
			rectangle_UnionRect(&self->UpdateRect, &self->UpdateRect, &(self->objs[ix].vbox));
		    }
		}

		self->objs[ix].timestamp = (o)->GetModified();
	    }
	    else {
		DEBUG(("Cache: new NULL %d\n", ix));
		self->objs[ix].o = NULL;
	    }
	} /* end case self->objs[ix].o != fig->objs[ix].o */

	else if (self->objs[ix].o != NULL) { 

	    /* figure out if old object needs to be changed */
	    if (needfull 
		|| (tstamp=(self->objs[ix].o)->GetModified()) > self->objs[ix].timestamp) {
		needup = 1;
	    }
	    if (!needup && (
			    self->objs[ix].wantupdate
			    || ((self->objs[ix].o)->IsInset() 
				&& (dobj=((class figoins *)self->objs[ix].o)->GetDataObject())
				&& (tstampi=(dobj)->GetModified()) > self->objs[ix].timestamp))) {
		needup = 2;
		if (self->objs[ix].wantupdate) {
		    (self->objs[ix].o)->SetModified();
		    tstamp=(self->objs[ix].o)->GetModified();
		    self->objs[ix].wantupdate = FALSE;
		}
	    }
	    if (needup) {
		class figobj *o = self->objs[ix].o;
		DEBUG(("Cache: changed %d\n", ix));

		if (self->objs[ix].selected
		    || (self->ShowFocusAttachments
			&& (o)->IsGroup()
			&& (self)->GetFocusRef()==ix 
			&& ((class figogrp *)o)->doconstraints))
		    self->objs[ix].knownselected = TRUE;
		else
		    self->objs[ix].knownselected = FALSE;

		/* add bbox of object and of old image */
		if (needup != 2)
		    if (self->objs[ix].drawnselected == 0) {
			if (!self->objs[ix].ignorevbox) {
			    rectangle_UnionRect(&self->MustEraseRect, &self->MustEraseRect, &(self->objs[ix].vbox));
			}
		    }
		    else {
			rectangle_UnionRect(&self->MustEraseRect, &self->MustEraseRect, &(self->objs[ix].vselbox));
		    }

		RectToPix(self, &self->objs[ix].vbox, (o)->GetBounds( self), figview_SpotRad);
		self->objs[ix].ignorevbox = ((o)->IsGroup());
		/* if [old] (self->objs[ix].knownselected) then */
		RectToPix(self, &self->objs[ix].vselbox, (o)->GetSelectedBounds( self), figview_AnchRad+1);

		if ((o)->IsInset()) {
		    RectToPix(self, &inrec, (o)->GetBounds( self), recttopix_Exact);
		    if (!rectangle_IsEqualRect(&inrec, &(self->objs[ix].insetb))) {
			self->objs[ix].insetb = inrec;
			self->objs[ix].insetbmoved = TRUE;
		    }
		}

		if (self->objs[ix].knownselected) {
		    rectangle_UnionRect(&self->UpdateRect, &self->UpdateRect, &(self->objs[ix].vselbox));
		}
		else {
		    if (!self->objs[ix].ignorevbox) /* an unselected group has empty bbox */ {
			rectangle_UnionRect(&self->UpdateRect, &self->UpdateRect, &(self->objs[ix].vbox));
		    }
		}

		if (tstampi > tstamp) {
		    tstamp = tstampi;
		}
		if (tstamp > self->objs[ix].timestamp)
		    self->objs[ix].timestamp = tstamp;
	    }
	    else { /* !needup */
		/* we now check for changes in the selection value. If it changes, we may have to update the vselbox size (but not the update rectangles) */
		o = self->objs[ix].o;

		if (self->objs[ix].selected
		    || (self->ShowFocusAttachments
			&& (o)->IsGroup()
			&& (self)->GetFocusRef()==ix 
			&& ((class figogrp *)o)->doconstraints)) {
		    if (!self->objs[ix].knownselected) 
			self->objs[ix].knownselected = TRUE;
		}
		else {
		    if (self->objs[ix].knownselected) 
			self->objs[ix].knownselected = FALSE;
		}
	    }
	}

	/* entirely separate things to deal with inset bits. Note that o may be NULL in these blocks. */
	o = self->objs[ix].o;
	{
	    char *name;

	    if (o && (o)->IsInset())
		dobj = ((class figoins *)o)->GetDataObject();
	    else
		dobj = NULL;

	    if (dobj != self->objs[ix].inseto) {
		if (self->objs[ix].insetv) {
		    DEBUG(("Inset: destroying: %s\n", (self->objs[ix].insetv)->GetTypeName())); 
		    (self->objs[ix].insetv)->Destroy();
		    self->objs[ix].insetv = NULL;
		}
		self->objs[ix].inseto = dobj;
		if (dobj) {
		    name = (dobj)->ViewName();
		   
		    DEBUG(("Inset: creating: %s\n", name));
		    if (name)
			self->objs[ix].insetv = (class view *)ATK::NewObject(name);
		    if (!self->objs[ix].insetv) {
			fprintf(stderr, "figview: unable to get view!\n");
		    }
		    else {
			(self->objs[ix].insetv)->SetDataObject( dobj);
		    }
		    self->objs[ix].insetbmoved = TRUE;
		}
	    }
	}
    }
    rectangle_UnionRect(&self->UpdateRect, &self->UpdateRect, &self->MustEraseRect);
}

static void OldUpdateCache(class figview  *self)
{
    boolean needfull = self->NeedFullUpdate;
    if (needfull)
	self->DoingFullUpdate = TRUE;
    self->NeedFullUpdate = FALSE;
    UpdateCache(self, needfull);
}

void figview::FlushDataChanges()
{
    OldUpdateCache(this);
}

static void UpdateWindowSize(class figview  *self)
{
    class figobj *foc;
    struct rectangle logrec;
    class figure *fig = (class figure *)(self)->GetDataObject();

    (self)->GetLogicalBounds( &logrec);
    self->lastwidth = logrec.width;
    self->lastheight = logrec.height;
    if (self->embedded) {
	/* self->figureclip is used if and only if self->embedded. */
	rectangle_InsetRect(&logrec, 1, 1);
	(self->figureclip)->RectRegion( &logrec);
    }

    foc = (fig)->FindObjectByRef( (fig)->RootObjRef());
    if (foc 
	 && (foc)->IsGroup() 
	 && ((class figogrp *)foc)->doconstraints) {
	long x1 = (self)->ToDefFigX( 0);
	long y1 = (self)->ToDefFigY( 0);
	long x2 = (self)->ToDefFigX( 0+self->lastwidth);
	long y2 = (self)->ToDefFigY( 0+self->lastheight);

	((class figogrp *)foc)->MoveHandle( x1, y1, 3);
	((class figogrp *)foc)->MoveHandle( x2, y2, 7);
    }

}

static void FixPixelPanning(class figview  *self)
{
    self->ppanx=(self)->ToPixW( (self)->panx);
    self->ppany=(self)->ToPixH( (self)->pany);
}

void figview::FullUpdate(enum view_UpdateType  type, long  left , long  top , long  width , long  height)
{
    this->UpdateCached = FALSE;

    if ((this)->GetIM()) FixPixelPanning(this);
    
    if (type == view_Remove  
	 ||  (this)->GetLogicalWidth() == 0 
	 || (this)->GetLogicalHeight() == 0) {
	/* view_Remove means the view has left the screen.
	 A zero dimension means the view is not visible */
	this->OnScreen = FALSE;
	long ix;
	for (ix=0; ix<this->objs_size; ix++) {
	    if (objs[ix].o && objs[ix].insetv && objs[ix].insetv->parent==this) {
		objs[ix].insetv->FullUpdate(view_Remove, 0, 0, 0, 0);
	    }
	}
	return;
    }

    if(type == view_MoveNoRedraw) {
	long ix;
	for (ix=0; ix<this->objs_size; ix++) {
	    if (objs[ix].o && objs[ix].insetv && objs[ix].insetv->parent==this) {
		(objs[ix].insetv)->InsertView(this, &(objs[ix].insetb));
		objs[ix].insetv->FullUpdate(view_MoveNoRedraw, 0, 0, 0, 0);
	    }
	}
	return;
    }
    
    if (this->lastwidth != (this)->GetLogicalWidth() 
	 || this->lastheight != (this)->GetLogicalHeight()) {
	UpdateWindowSize(this);
    }

    (this)->SetTransferMode( graphic_COPY);

    /* draw inset border */
    if (this->embedded) {
	(this)->ClearClippingRect();
	(this)->DrawRectSize( 0, 0, this->lastwidth-1, this->lastheight-1);

	(this)->SetClippingRegion( this->figureclip);
    }

    switch (type) {
	case view_FullRedraw:
	    this->OnScreen = TRUE;
	    this->DoingFullUpdate = TRUE;
	    OldUpdateCache(this);
	    (this)->GetLogicalBounds( &this->UpdateRect);
	    (this)->SetBackgroundColor( this->FigBackColor, 65535, 65535, 65535);
	    (this)->EraseRect( &this->UpdateRect); 
	    RedrawView(this, TRUE); /* area is already erased */
	    break;
	case view_PartialRedraw:
	case view_LastPartialRedraw:
	    this->OnScreen = TRUE;
	    rectangle_SetRectSize(&this->UpdateRect, left, top, width, height);
	    (this)->SetBackgroundColor( this->FigBackColor, 65535, 65535, 65535);
	    (this)->EraseRect( &this->UpdateRect); 
	    RedrawView(this, TRUE); /* area is already erased */
	    break;
	default:
	    break;
    };
}

#ifndef ABS
#define ABS(x) ((x<0)?-(x):x)
#endif

static void DoRedraws(class figview  *self, struct rectangle  *ux, struct rectangle  *uy, struct rectangle  *dr, long  diffx , long  diffy)
{
    struct rectangle vb;
    class region *vr=region::CreateEmptyRegion();
    class region *vbr;
    class region *tmp;
    
    if (vr==NULL) return;
    
    (self)->GetVisualRegion( vr);
    (self)->GetVisualBounds( &vb);

    vbr=region::CreateRectRegion(&vb);

    if (vbr==NULL) return;

   
    UpdateCache(self, TRUE);
    if (uy->height>0) {
	(self)->EraseRect( uy);
	(self)->FullUpdate( view_LastPartialRedraw, uy->left, uy->top, uy->width, uy->height);
	(self)->SetTransferMode( graphic_COPY);
    }
    if (ux->width>0) {
	(self)->EraseRect( ux);
	(self)->FullUpdate( view_LastPartialRedraw, ux->left, ux->top, ux->width, ux->height);
	(self)->SetTransferMode( graphic_COPY);
    }

    /* is there a better way to detect the case where
     the entire view is visible? */
    if ((vr)->AreRegionsEqual( vbr)) {
	delete vr;
	delete vbr;
	return;
    }

    tmp=region::CreateEmptyRegion();
    if (tmp==NULL) return;
    
    (vbr)->SubtractRegion( vr, tmp);
    (tmp)->OffsetRegion( diffx, diffy);
    
    (vbr)->RectRegion( dr);

    (vbr)->IntersectRegion( tmp, vr);

    if (!(vr)->IsRegionEmpty()) {
	struct rectangle rr;
	(vr)->GetBoundingBox( &rr);
	if (rr.width>0 && rr.height>0) {
	    (self)->EraseRect( &rr);

	    (self)->FullUpdate( view_LastPartialRedraw, rr.left, rr.top, rr.width, rr.height);
	    (self)->SetTransferMode( graphic_COPY);
	}	 
    }
    delete vr;
    delete vbr;
    delete tmp;	
}

    
static void DoBlit(class figview  *self, long  diffx , long  diffy)
{
    struct rectangle s, ux, uy, vr;
    struct point d;
    class region *vb = self->tmpregion;
    int border;

    border = (self->embedded ? 1 : 0);
    if (vb) {
	(self)->GetVisualRegion( vb);
	(vb)->OffsetRegion( diffx, diffy);
    }

    (self)->GetVisualBounds( &ux);
    (self)->GetVisualBounds( &uy);
    
    if (diffx<0) {
	s.left=ABS(diffx)+border;
	d.x=0+border;
	s.width = ((self )->GetVisualWidth( ) - s.left) - border;
	ux.left=s.width+border;
	ux.width=s.left-border;
    } else {
	d.x = ABS(diffx)+border;
	s.left=0+border;
	s.width = ((self )->GetVisualWidth( ) - d.x) - border;
	ux.left=0+border;
	ux.width=d.x-border;
    }
    if (diffy<0) {
	s.top=ABS(diffy)+border;
	d.y=0+border;
	s.height = ((self )->GetVisualHeight( ) - s.top) - border;
	uy.top=s.height+border;
	uy.height=s.top-border;
    } else {
	d.y = ABS(diffy)+border;
	s.top=0+border;
	s.height = ((self )->GetVisualHeight( ) - d.y) - border;
	uy.top=0+border;
	uy.height=d.y-border;
    }
    
    (self)->SetTransferMode( graphic_COPY);
    if (vb) (self)->SetClippingRegion( vb);
    (self)->BitBlt( &s, self, &d, NULL);
    if (vb) {
	(self)->GetVisualRegion( vb);
	if (self->embedded)
	    (vb)->IntersectRegion( self->figureclip, vb);
	(self)->SetClippingRegion( vb);
    }

    s.left=d.x;
    s.top=d.y;
    DoRedraws(self, &ux, &uy, &s, diffx, diffy);
}

void figview::Update()
{
    class figure *fig = (class figure *)(this)->GetDataObject();
    long lppanx=this->ppanx;
    long lppany=this->ppany;
    long diffx;
    long diffy;

    this->UpdateCached = FALSE;
  
    if (!fig) return;
    if (!this->OnScreen) return;

    if ((this)->GetIM()) FixPixelPanning(this);
    diffx=lppanx-this->ppanx;
    diffy=lppany-this->ppany;

    if (this->lastwidth != (this)->GetLogicalWidth() 
	 || this->lastheight != (this)->GetLogicalHeight()) {
	UpdateWindowSize(this);
    }

    if (diffx || diffy) {
	if (!this->NeedFullUpdate) {
	    if (ABS(diffx)<(this)->GetVisualWidth() && ABS(diffy)<(this)->GetVisualHeight()) {
		DoBlit(this, diffx, diffy);
		return;
	    } else this->NeedFullUpdate=TRUE;
	}
    }
    OldUpdateCache(this);
    RedrawView(this, FALSE);
}

/* if the requestor is an inset, we have to find the objref corresponding to it. This is a linear search. self->lastupdated is a cheap hack to speed this up. */
void figview::WantUpdate(class view  *requestor)
{
    if ((class view *)this != requestor) {
	int ix;

	ix = this->lastupdater;
	if (ix>=0 && ix<this->objs_size && this->objs[ix].o && (this->objs[ix].insetv==requestor || requestor->IsAncestor(this->objs[ix].insetv))) {
	    /*figobj_SetModified(self->objs[ix].o);*/
	    this->objs[ix].wantupdate = TRUE;
	}
	else for (ix=0; ix<this->objs_size; ix++) {
	    if (this->objs[ix].o && (this->objs[ix].insetv==requestor || requestor->IsAncestor(this->objs[ix].insetv))) {
		this->lastupdater = ix;
		/*figobj_SetModified(self->objs[ix].o);*/
		this->objs[ix].wantupdate = TRUE;
		break;
	    }
	};
	(this)->view::WantUpdate( this);
    }
    else {
	if (!this->UpdateCached) {
	    this->UpdateCached = TRUE;
	    /* if blocking is on, we leave UpdateCached on but don't pass the update request; it'll be passed on when the blockage is turned off. */
	    if (!this->UpdatesBlocked)
		(this)->view::WantUpdate( requestor);
	}
    }
}

void figview::BlockUpdates(boolean  val)
{
    if (val) {
	/* turn blocking on */
	if (this->UpdatesBlocked)
	    return;
	this->UpdatesBlocked = TRUE;
    }
    else {
	/* turn blocking off */
	 if (!this->UpdatesBlocked)
	     return;
	 this->UpdatesBlocked = FALSE;
	 if (this->UpdateCached) {
	     (this)->view::WantUpdate( this); /* super_ to avoid the mess above; it might be more correct to turn off UpdatesCached and then call figview_WantUpdate(), but this will work */
	 }
    }
}

static boolean TEI_Splot(class figobj  *o, long  ref, class figure  *self, long  *vv)
{
    if ((o)->IsInset() 
	 && (int)((o)->HitMe( vv[1], vv[2], 0, NULL)) >= (int)(figobj_HitInside))
	vv[0] = ref;

    return FALSE;
}

class view *figview::Hit(enum view_MouseAction  action, long  x , long  y , long  num_clicks)
{
    if (! this->OnScreen) return NULL;

    /*if (action == view_NoMouseEvent)
	return (struct view *)self;*/
    if (!this->toolset || this->toolset->toolnum==1) /* 1==pan ### */ {
	long px, py, vwid, vhgt, oref[3];
	long vx, vy;
	class figure *fig;

	vwid = (this)->GetLogicalWidth();
	vhgt = (this)->GetLogicalHeight();
	vx = (this)->ToFigX( x+(this)->ToPixW(1)/2);
	vy = (this)->ToFigY( y+(this)->ToPixH(1)/2);

	if (action==view_LeftDown || action==view_RightDown) {
	    fig = (class figure *)(this)->GetDataObject();

	    oref[0] = figure_NULLREF;
	    oref[1] = vx;
	    oref[2] = vy;
	    (fig)->EnumerateObjectTree( figure_NULLREF, NULL, FALSE, (figure_eofptr)TEI_Splot, (long)oref);
	    if (oref[0] != figure_NULLREF) {
		/*struct figoins *o = (struct figoins *)figure_FindObjectByRef(fig, oref);*/
		class view *vo = this->objs[oref[0]].insetv;
		if (!vo) {
		    message::DisplayString(this, 10, "This inset object has no inset.");
		    return (class view *)this;
		}
		x = (vo)->EnclosedXToLocalX( x);
		y = (vo)->EnclosedYToLocalY( y);

		return (vo)->Hit( action, x, y, num_clicks);
	    }
	}

	if (!this->HasInputFocus) {
	    this->InputFocusClick = TRUE;
	    (this)->WantInputFocus( this);
	    return (class view *)this;
	}
	if (this->InputFocusClick) {
	    if (action==view_LeftUp || action==view_RightUp)
		this->InputFocusClick = FALSE;
	    return (class view *)this;
	}

	/* do panning */
	switch (action) {
	    case view_LeftDown:
	    case view_RightDown:
		this->rockx = vx;
		this->rocky = vy;
		px = (this)->ToPixX( vx);
		py = (this)->ToPixY( vy);
		(this)->SetTransferMode( graphic_INVERT);
		(this)->MoveTo( 0, py);
		(this)->DrawLineTo( vwid-1, py);
		(this)->MoveTo( px, 0);
		(this)->DrawLineTo( px, vhgt-1);
		this->lastx = px;
		this->lasty = py;
		break;
	    case view_LeftMovement:
	    case view_RightMovement:
		px = (this)->ToPixX( vx);
		py = (this)->ToPixY( vy);
		if (px!=this->lastx || py!=this->lasty) {
		    (this)->SetTransferMode( graphic_INVERT);
		    (this)->MoveTo( 0, this->lasty);
		    (this)->DrawLineTo( vwid-1, this->lasty);
		    (this)->MoveTo( this->lastx, 0);
		    (this)->DrawLineTo( this->lastx, vhgt-1);
		    (this)->MoveTo( 0, py);
		    (this)->DrawLineTo( vwid-1, py);
		    (this)->MoveTo( px, 0);
		    (this)->DrawLineTo( px, vhgt-1);
		    this->lastx = px;
		    this->lasty = py;
		}
		break;
	    case view_LeftUp:
	    case view_RightUp:
		(this)->SetTransferMode( graphic_INVERT);
		(this)->MoveTo( 0, this->lasty);
		(this)->DrawLineTo( vwid-1, this->lasty);
		(this)->MoveTo( this->lastx, 0);
		(this)->DrawLineTo( this->lastx, vhgt-1);
		this->panx += (this->rockx - vx);
		this->pany += (this->rocky - vy);
		/*self->NeedFullUpdate = TRUE;*/

		/*figview_WantUpdate(self, self);
		im_ForceUpdate();*/
		(this)->Update(); /* morally equivalent to the two lines above, and a tad faster */
		break;
	}
    }
    else {
	figtoolview_layout_f proc;

	if (!this->HasInputFocus) {
	    this->InputFocusClick = TRUE;
	    (this)->WantInputFocus( this);
	    return (class view *)this;
	}
	if (this->InputFocusClick) {
	    if (action==view_LeftUp || action==view_RightUp)
		this->InputFocusClick = FALSE;
	    return (class view *)this;
	}

	proc = (this->toolset)->GetToolProc();
	x = (this)->ToFigX( x+(this)->ToPixW(1)/2);
	y = (this)->ToFigY( y+(this)->ToPixH(1)/2);
	switch (action) {
	    case view_LeftDown:
	    case view_RightDown:
		this->lastx = x;
		this->lasty = y;
		break;
	    case view_LeftMovement:
	    case view_RightMovement:
		if (this->lastx == x && this->lasty == y)
		    proc = NULL;
		else {
		    this->lastx = x;
		    this->lasty = y;
		}
		break;
	    default:
		break;
	};
	if (proc) {
	    return (*proc)(this->toolset, action, x, y, num_clicks);
	}
    }

    return (class view *)this;		/* where to send subsequent hits */
}

boolean figview::IsSelected(long  ref)
{
    if (ref<0 || ref>=this->objs_size) 
	return FALSE;
    return (this->objs[ref].o && this->objs[ref].selected);
}

void figview::ClearSelection()
{
    int ix;

    for (ix=0; ix<this->objs_size; ix++) {
	this->objs[ix].selected = FALSE;
    }
    this->numselected = 0;
    RepostMenus(this);
}

void figview::Select(class figobj  *o)
{
    class figure *fig = (class figure *)(this)->GetDataObject();
    long ref = (fig)->FindRefByObject( o);

    if (ref != figure_NULLREF)
	(this)->SelectByRef( ref);
}

void figview::SelectByRef(long  ref)
{
    if (ref<0 || ref>=this->objs_size) 
	return;

    if (!this->objs[ref].selected) {
	this->objs[ref].selected = TRUE;
	this->numselected++;
	RepostMenus(this);
    }
}

void figview::ToggleSelect(class figobj  *o)
{
    class figure *fig = (class figure *)(this)->GetDataObject();
    long ref = (fig)->FindRefByObject( o);

    if (ref != figure_NULLREF)
	(this)->ToggleSelectByRef( ref);
}

void figview::ToggleSelectByRef(long  ref)
{
    if (ref<0 || ref>=this->objs_size) 
	return;

    if (!this->objs[ref].selected) {
	this->objs[ref].selected = TRUE;
	this->numselected++;
    }
    else {
	this->objs[ref].selected = FALSE;
	this->numselected--;
    }
    RepostMenus(this);
}

void figview::Unselect(class figobj  *o)
{
    class figure *fig = (class figure *)(this)->GetDataObject();
    long ref = (fig)->FindRefByObject( o);

    if (ref != figure_NULLREF) 
	(this)->UnselectByRef( ref);
}

void figview::UnselectByRef(long  ref)
{
    if (ref<0 || ref>=this->objs_size) 
	return;

    if (this->objs[ref].selected) {
	this->objs[ref].selected = FALSE;
	this->numselected--;
	RepostMenus(this);
    }
}

/* if there is exactly one object selected, return its ref. Otherwise, return figure_NULLREF */
long figview::GetOneSelected()
{
    /* could be more efficient */
    int ix;

    if (this->numselected != 1)
	return figure_NULLREF;

    for (ix=0; ix<this->objs_size; ix++)
	if (this->objs[ix].o && this->objs[ix].selected) 
	    return ix;

    return figure_NULLREF; /* should never happen, but what the hell */
}

static void EnumSelSplot(class figview  *self, class figure  *fig, long  grp, figview_esfptr func, long  rock)
{
    long ix;
    class figogrp *gr = (class figogrp *)fig->objs[grp].o;
    class figobj *this_c;

    for (ix=(gr)->GetRoot(); ix!=figure_NULLREF; ix=fig->objs[ix].next) {

	this_c=fig->objs[ix].o;
	if (self->objs[ix].selected) {
	    (*func)(this_c, ix, self, rock);
	}

	if ((this_c)->IsGroup()) {
	    EnumSelSplot(self, fig, ix, func, rock);
	}
    }
}

/* enumerate in order from back to front
func should be of the form
void func(struct figobj *o, long ref, struct figview *self, rock)
*/
void figview::EnumerateSelection(figview_esfptr func, long  rock)
{
    long grp;
    class figure *fig = (class figure *)(this)->GetDataObject();
    if (!fig) return;

    grp = (fig)->RootObjRef();
    if (grp==figure_NULLREF) return;

    if (this->objs[grp].selected) {
	(*func)(fig->objs[grp].o, grp, this, rock);
    }
    EnumSelSplot(this, fig, grp, func, rock);
}

/* if ref==figure_NULLREF, set to root group */
void figview::SetFocusByRef(long  ref)
{
    class figure *fig = (class figure *)(this)->GetDataObject();
    if (!fig) return;

    if (ref==figure_NULLREF)
	ref = (fig)->RootObjRef();
    else if (!(fig->objs[ref].o)->IsGroup()) {
	/* not a group */
	return;
    }

    if (ref==figure_NULLREF || !(fig->objs[ref].o)->IsGroup()) {
	/* root object is not a group */
	ref = figure_NULLREF;
    }

    this->focusgroup = ref;
    if (ref!=figure_NULLREF) {
	class figogrp *o = (class figogrp *)fig->objs[ref].o;
	if ((o)->GetRoot() == figure_NULLREF) 
	    message::DisplayString(this, 0, "The focus is on an empty group.");
	else
	    message::DisplayString(this, 0, "");
	if (ref == (fig)->RootObjRef())
	    (this)->SetHighlightSize( 0, 0, 0, -1, -1);
	else
	    (this)->SetHighlight( 0, (o)->GetBounds( this));
    }
}

static void AbortObjectProc(class figview  *self, long  rock)
{
    if (self->toolset)
	(self->toolset)->AbortObjectBuilding();
    else
	message::DisplayString(self, 10, "No object is being created.");
}

static void FocusUpProc(class figview  *self, long  rock)
{
    long old, ref;

    if (!self->focuschange) return;

    old = self->focusgroup;
    if (old==figure_NULLREF) {
	(self)->SetFocusByRef( figure_NULLREF);
	(self)->SelectByRef( self->focusgroup);
	(self)->WantUpdate( self);
	return;
    }

    self->focussib = 0; /* ### not right */
    (self)->ClearSelection();

    ref = (self->objs[old].o)->GetParentRef();
    if (ref==figure_NULLREF) {
	message::DisplayString(self, 10, "Cannot go up; the focus is on the root group.");
    }
    else {
	(self)->SetFocusByRef( ref);
	/*figview_SelectByRef(self, old);*/
    }
    (self)->WantUpdate( self);
}

static void FocusDownProc(class figview  *self, long  rock)
{
    long old, ref;
    class figogrp *foc;
    class figure *fig = (class figure *)(self)->GetDataObject();
    if (!fig) return;

    if (!self->focuschange) return;

    old = self->focusgroup;
    foc = (class figogrp *)fig->objs[old].o;
    
    for (ref=(foc)->GetRoot(); ref!=figure_NULLREF; ref=fig->objs[ref].next)
	if ((fig->objs[ref].o)->IsGroup())
	    break;

    if (ref==figure_NULLREF) {
	message::DisplayString(self, 10, "Cannot go down; this group contains no groups.");
	return;
    }

    (self)->SetFocusByRef( ref);
    self->focussib = 0;
    (self)->ClearSelection();
    (self)->WantUpdate( self);
}

static void FocusLeftProc(class figview  *self, long  rock)
{
    long old, ref, fref;
    long count;
    class figogrp *paro;
    class figure *fig = (class figure *)(self)->GetDataObject();
    if (!fig) return;

    if (!self->focuschange) return;

    old = self->focusgroup;

    ref = (self->objs[old].o)->GetParentRef();
    if (ref==figure_NULLREF) {
	return;
    }

    paro = (class figogrp *)fig->objs[ref].o;

    if (self->focussib==0) {
	count = 0;
	for (ref=(paro)->GetRoot(); ref!=figure_NULLREF; ref=fig->objs[ref].next)
	    if ((fig->objs[ref].o)->IsGroup()) {
		fref = ref;
		count++;
	    }
	self->focussib = count-1;
	ref = fref;
    }
    else {
	self->focussib--;
	count = 0;
	for (ref=(paro)->GetRoot(); ref!=figure_NULLREF; ref=fig->objs[ref].next)
	    if ((fig->objs[ref].o)->IsGroup()) 
		if (count++ == self->focussib) break;
    }

    (self)->SetFocusByRef( ref);
    (self)->ClearSelection();
    (self)->WantUpdate( self);
}

static void FocusRightProc(class figview  *self, long  rock)
{
    long old, ref;
    long count;
    class figogrp *paro;
    class figure *fig = (class figure *)(self)->GetDataObject();
    if (!fig) return;

    if (!self->focuschange) return;

    old = self->focusgroup;

    ref = (self->objs[old].o)->GetParentRef();
    if (ref==figure_NULLREF) {
	return;
    }

    paro = (class figogrp *)fig->objs[ref].o;

    count = 0;
    for (ref=(paro)->GetRoot(); ref!=figure_NULLREF; ref=fig->objs[ref].next)
	if ((fig->objs[ref].o)->IsGroup())
	    if (count++ > self->focussib) break;

    if (ref==figure_NULLREF) {
	for (ref=(paro)->GetRoot(); ref!=figure_NULLREF; ref=fig->objs[ref].next)
	    if ((fig->objs[ref].o)->IsGroup())
		break;
	self->focussib = 0;
    }
    else {
	self->focussib++;
    }

    (self)->SetFocusByRef( ref);
    (self)->ClearSelection();
    (self)->WantUpdate( self);
}

void figview::CutNPaste(short  operation, long  rock /* currently unused */)
{
    switch (operation) {
	case figview_OpCopy:
	    CopySelProc(this, rock);
	    break;
	case figview_OpCopyInset:
	    CopySelInsetProc(this, rock);
	    break;
	case figview_OpCut:
	    CutSelProc(this, rock);
	    break;
	case figview_OpPaste:
	    PasteSelProc(this, rock);
	    break;
	case figview_OpPasteRotate:
	    RotatePasteProc(this, rock);
	    break;
	default:
	    break;
    }
}

static void CutSelSplot(class figview  *self, class figure  *fig, long  gref, FILE  *fp)
{
    long ix;
    class figogrp *gr = (class figogrp *)fig->objs[gref].o;
    class figobj *this_c;

    for (ix=(gr)->GetRoot(); ix!=figure_NULLREF; ix=fig->objs[ix].next) {
	this_c=fig->objs[ix].o;
	if (self->objs[ix].selected) {
	    IncreaseTmpProc(self, self->tmpnum+1);
	    self->tmplist[self->tmpnum] = ix;
	    self->tmpnum++;
	}
	else if ((this_c)->IsGroup()) {
	    CutSelSplot(self, fig, ix, fp);
	}
    }
}

static void CutSelProc(class figview  *self, long  rock)
{
    class figure *fig = (class figure *)(self)->GetDataObject();
    long ref;
    int ix;
    FILE *fp;
    class figobj *this_c;
    class dataobject *dobj;
    struct point tmppt;

    long cutbuf_writeid = im::GetWriteID();

    if (!fig) return;
    if ((fig)->GetReadOnly()) {
	CopySelProc(self, rock);
	return;
    }
    (self)->FlushDataChanges();
    if (self->numselected==0)
	return;
    
    self->tmpnum = 0;
    ref = (fig)->RootObjRef();
    if (ref==figure_NULLREF)
	return;
    CutSelSplot(self, fig, ref, NULL);

    fp = ((self)->GetIM())->ToCutBuffer(); 
    (fig)->WritePartial( fp, cutbuf_writeid, 0, self->tmplist, self->tmpnum, &tmppt);
    self->lastpaste[0] = tmppt.x;
    self->lastpaste[1] = tmppt.y;
    self->lastpasteoffset = (-1);
    ((self)->GetIM())->CloseToCutBuffer( fp); 

    for (ix=0; ix<self->tmpnum; ix++) {
	this_c=fig->objs[self->tmplist[ix]].o;
	(fig)->DeleteObject( this_c);
	(this_c)->Destroy();
    }

    (self)->ClearSelection();
    (fig)->NotifyObservers( figure_DATACHANGED);
    RepostMenus(self);
}

static void CopySelProc(class figview  *self, long  rock)
{
    class figure *fig = (class figure *)(self)->GetDataObject();
    long ref;
    FILE *fp;
    class figobj *this_c;
    class dataobject *dobj;
    struct point tmppt;

    long cutbuf_writeid = im::GetWriteID();

    if (!fig) return;
    (self)->FlushDataChanges();
    if (self->numselected==0)
	return;
    
    self->tmpnum = 0;
    ref = (fig)->RootObjRef();
    if (ref==figure_NULLREF)
	return;
    CutSelSplot(self, fig, ref, NULL);

    fp = ((self)->GetIM())->ToCutBuffer(); 
    (fig)->WritePartial( fp, cutbuf_writeid, 0, self->tmplist, self->tmpnum, &tmppt);
    self->lastpaste[0] = tmppt.x;
    self->lastpaste[1] = tmppt.y;
    self->lastpasteoffset = 0;

    ((self)->GetIM())->CloseToCutBuffer( fp); 
}

static void CopySelInsetProc(class figview  *self, long  rock)
{
    class figure *fig = (class figure *)(self)->GetDataObject();
    long ref;
    FILE *fp;
    class dataobject *dobj;

    long cutbuf_writeid = im::GetWriteID();

    if (!fig) return;
    (self)->FlushDataChanges();

    ref = (self)->GetOneSelected();
    if (ref == figure_NULLREF || !(self->objs[ref].o)->IsInset()) {
	message::DisplayString((class view *)self, 10, "There must be exactly one inset selected.");
	return;
    }
    dobj = ((class figoins *)self->objs[ref].o)->GetDataObject();
    if (!dobj) 
	return;

    fp = ((self)->GetIM())->ToCutBuffer(); 
    (dobj)->Write( fp, cutbuf_writeid, 0);
    ((self)->GetIM())->CloseToCutBuffer( fp); 
}

static void RotatePasteProc(class figview  *self, long  rock)
{
    class figure *fig = (class figure *)(self)->GetDataObject();
    long ref;
    int ix;
    class figobj *this_c;
    class dataobject *dobj;
    struct point tmppt;

    if (!fig) return;
    if ((fig)->GetReadOnly()) {
	message::DisplayString((class view *)self, 10, "Document is read-only.");
	return;
    }
    (self)->FlushDataChanges();
    
    self->tmpnum = 0;
    ref = (fig)->RootObjRef();
    if (ref==figure_NULLREF)
	return;
    CutSelSplot(self, fig, ref, NULL);

    for (ix=0; ix<self->tmpnum; ix++) {
	this_c=fig->objs[self->tmplist[ix]].o;
	(fig)->DeleteObject( this_c);
	(this_c)->Destroy();
    }

    PasteSelProc(self, rock);
    ((self)->GetIM())->RotateCutBuffers( 1);
}

static void PasteSelProc(class figview  *self, long  rock)
{
    class figure *fig = (class figure *)(self)->GetDataObject();
    FILE *fp;
    /*static char hdr[] = "\\begindata{figure,";
     char *hx = hdr;*/
    char namebuf[100];
#define LINELENGTH (250)
    char buf[LINELENGTH+1];
    int c, ix;
    long foc, count1, count2, offx, offy, snap, tid;
    class figogrp *o;
    struct point tmppt;

    if (!fig) return;
    if ((fig)->GetReadOnly()) {
	message::DisplayString((class view *)self, 10, "Document is read-only.");
	return;
    }
    fp = ((self)->GetIM())->FromCutBuffer();

    /* We no longer bother to search the datastream for a figure. It causes massive damage when trying to deal with subclasses of figure. */
    if (fgets(buf, LINELENGTH, fp) == NULL
	|| sscanf(buf, "\\begindata{%[^,],%ld}", namebuf, &tid) != 2
	|| !ATK::IsTypeByName(namebuf, "figure")) {
	message::DisplayString((class view *)self, 0, "No figure found in cut buffer.");
	((self)->GetIM())->CloseFromCutBuffer( fp);
	return;
    }

    foc = (self)->GetFocusRef();
    count1 = (fig)->GetObjectCounter();
    ix = (fig)->ReadPartial( fp, 0, foc, &tmppt);
    if (self->lastpaste[0] == tmppt.x && self->lastpaste[1] == tmppt.y) {
	self->lastpasteoffset++;
    }
    else {
	self->lastpaste[0] = tmppt.x;
	self->lastpaste[1] = tmppt.y;
	self->lastpasteoffset = 0;
    }

    count2 = (fig)->GetObjectCounter();
    if (ix!=dataobject_NOREADERROR) {
	message::DisplayString(self, 0, "Unable to read figure from cut buffer.");
	((self)->GetIM())->CloseFromCutBuffer( fp);
	return;
    }
    ((self)->GetIM())->CloseFromCutBuffer( fp);

    (self)->FlushDataChanges();
    (self)->ClearSelection();

    if (self->toolset)
	snap = (self->toolset)->GetSnapGrid();
    else
	snap = 0;
    if (snap) {
	offx = 128 + snap - (128%snap);
	offy = 64 + snap - (64%snap);
    }
    else {
	offx = 128;
	offy = 64;
    }
    o = (class figogrp *)(fig)->FindObjectByRef( foc);
    for (ix=(o)->GetRoot(); ix!=figure_NULLREF; ix=fig->objs[ix].next) {
	if (fig->objs[ix].counter >= count1 && fig->objs[ix].counter < count2) {
	    ((fig)->FindObjectByRef( ix))->Reposition( self->lastpasteoffset*offx, self->lastpasteoffset*offy);   
	    (self)->SelectByRef( ix);
	}
    }

    (fig)->NotifyObservers( figure_DATACHANGED);
}

static void UnselectProc(class figview  *self, long  rock)
{
    (self)->FlushDataChanges();
    (self)->ClearSelection();
    RepostMenus(self);
}

static void ShowPrintAreaProc(class figview  *self, long  rock /* 0 for off, 1 for on, 2 for recalc */)
{
    long wpts, hpts, swap;
    class figure *fig = (class figure *)(self)->GetDataObject();
    struct rectangle *rtmp;

    if (rock) {
	if (rock==2 && !self->ShowPrintArea)
	    return;

	wpts = 540 * figview_FigUPerPix;
	hpts = 648 * figview_FigUPerPix;
	if (fig->GetPrintLandscape()) {
	    swap = wpts;
	    wpts = hpts;
	    hpts = swap;
	}
	wpts = (long)((double)wpts / (fig)->GetPrintScaleX());
	hpts = (long)((double)hpts / (fig)->GetPrintScaleY());
	(self)->SetHighlightSize( 1, self->originx, self->originy, wpts, hpts);
	self->ShowPrintArea = TRUE;
	RepostMenus(self);
	(self)->WantUpdate( self);
    }
    else {
	if (self->ShowPrintArea) {
	    (self)->SetHighlightSize( 1, 0, 0, -1, -1);
	    self->ShowPrintArea = FALSE;
	    RepostMenus(self);
	    (self)->WantUpdate( self);
	}
    }
}

static void SetPrintLandscapeProc(class figview  *self, long  rock)
{
    class figure *fig = (class figure *)(self)->GetDataObject();
    int ix;
    if (!fig) return;

    ix = fig->GetPrintLandscape();
    if ((ix && rock) || (!ix && !rock))
	return;

    fig->SetPrintLandscape(rock);
    (fig)->SetModified();
    (fig)->NotifyObservers( figure_DATACHANGED);
    if (rock)
	message::DisplayString(self, 10, "Figure will be printed in landscape (sideways) mode.");
    else
	message::DisplayString(self, 10, "Figure will be printed in portrait (normal) mode.");
    ShowPrintAreaProc(self, 2);
    RepostMenus(self);
}

static void SetPrintScaleProc(class figview  *self, long  rock)
{
    char buffer[64];
    char obuffer[256];
    int res;
    struct rectangle *rtmp;
    double newx, newy;
    long wpts, hpts;
    class figure *fig = (class figure *)(self)->GetDataObject();
    if (!fig) return;

    if ((fig)->GetPrintScaleX() == (fig)->GetPrintScaleY()) 
	sprintf(obuffer, "Print scale is %.2f.  New width scale [%.2f]: ", (fig)->GetPrintScaleX(), (fig)->GetPrintScaleX());
    else
	sprintf(obuffer, "Print scale is %.2f by %.2f.  New width scale [%.2f]: ", (fig)->GetPrintScaleX(), (fig)->GetPrintScaleY(), (fig)->GetPrintScaleX());

    res = message::AskForString (self, 40, obuffer, NULL, buffer, 30); 
    if (res<0) {
	message::DisplayString(self, 10, "Cancelled.");
	return;
    }
    if (strlen(buffer)==0)
	newx = (fig)->GetPrintScaleX();
    else {
	newx = atof(buffer);
	if (newx <= 0.0) {
	    message::DisplayString(self, 10, "Scale must be a positive number.");
	    return;
	}
    }

    sprintf(obuffer, "Width scale is %.2f.  New height scale [same]: ",	newx);
    res = message::AskForString (self, 40, obuffer, NULL, buffer, 30); 
    if (res<0) {
	message::DisplayString(self, 10, "Cancelled.");
	return;
    }
    if (strlen(buffer)==0)
	newy = newx;
    else {
	newy = atof(buffer);
	if (newy <= 0.0) {
	    message::DisplayString(self, 10, "Scale must be a positive number.");
	    return;
	}
    }

    if (newx != (fig)->GetPrintScaleX() || newy != (fig)->GetPrintScaleY()) {
	(fig)->SetPrintScale( newx, newy);
	ShowPrintAreaProc(self, 2);
	(fig)->SetModified();
	(fig)->NotifyObservers( figure_DATACHANGED);
    }

    rtmp = (fig)->GetOverallBounds();
    wpts = (self)->ToPrintPixX( rtmp->left+rtmp->width)+12;
    hpts = (self)->ToPrintPixY( rtmp->top+rtmp->height)+12;
    if (wpts<1) wpts = 1;
    if (hpts<1) hpts = 1;
    newx = (double)wpts * (fig)->GetPrintScaleX() / 72.0;
    newy = (double)hpts * (fig)->GetPrintScaleY() / 72.0;

    if ((fig)->GetPrintScaleX() == (fig)->GetPrintScaleY()) 
	sprintf(obuffer, "Print scaling is now %.2f  (document will print %.2f by %.2f in.)\n", (fig)->GetPrintScaleX(), newx, newy);
    else
	sprintf(obuffer, "Print scaling is now %.2f by %.2f  (document will print %.2f by %.2f in.)\n", (fig)->GetPrintScaleX(), (fig)->GetPrintScaleY(), newx, newy);
    message::DisplayString(self, 10, obuffer);
}

static void ReadZipProc(class figview  *self, long  rock)
{
    int res;
    long count1, count2, ix;
    double ratio;
    char buffer[296], obuffer[296], scbuffer[32];
    char *ctmp;
    boolean eofl;
    FILE *fl;
    class figogrp *o;
    class figure *fig = (class figure *)(self)->GetDataObject();
    if (!fig) return;

    res = completion::GetFilename(self, "Read Zip file: ", NULL, buffer, 255, FALSE, FALSE);
    if (res==(-1) || strlen(buffer)==0) {
	message::DisplayString(self, 10, "Cancelled.");
	return;
    }

    res = message::AskForString(self, 40, "Scale Zip document by [1.00]: ", NULL, scbuffer, 30); 
    if (res<0) {
	message::DisplayString(self, 10, "Cancelled.");
	return;
    }
    if (strlen(scbuffer)==0)
	ratio = 1.0;
    else {
	ratio = atof(scbuffer);
	if (ratio <= 0.0) {
	    message::DisplayString(self, 10, "Scale must be a positive number.");
	    return;
	}
    }

    fl = fopen(buffer, "r");
    if (!fl) {
	sprintf(obuffer, "Unable to open file '%s'.", buffer);
	message::DisplayString(self, 10, obuffer);
	return;
    }

    /* snarf off \begindata */ 
    strcpy(buffer, "");
    eofl = FALSE;
    ctmp = fgets(buffer, 294, fl);
    res = sscanf(buffer, "\\begindata{%[^,],%ld}", obuffer, &ix);
    if (res!=2) {
	/* treat as raw Zip data */
	rewind(fl);
    }
    else {
	/* ATK datastream */
	while (!eofl && strcmp(obuffer, "zip")) {
	    do {
		ctmp = fgets(buffer, 294, fl);
		if (!ctmp) eofl=TRUE;
		res = sscanf(buffer, "\\begindata{%[^,],%ld}", obuffer, &ix);
	    } while (!eofl && res!=2);
	}
    }

    if (eofl)
	res = dataobject_NOTATKDATASTREAM;
    else {
	count1 = (fig)->GetObjectCounter();
	res = figio::ReadZipFile(fl, fig, (self)->GetFocusRef(), ratio);
	count2 = (fig)->GetObjectCounter();
    }

    switch (res) {
	case dataobject_NOREADERROR:
	    message::DisplayString(self, 10, "Zip data read in.");
	    break;
	case dataobject_PREMATUREEOF:
	    message::DisplayString(self, 10, "Error: unexpected end of file.");
	    break;
	case dataobject_NOTATKDATASTREAM:
	    message::DisplayString(self, 10, "Error: apparently not Zip file.");
	    break;
	case dataobject_MISSINGENDDATAMARKER:
	    message::DisplayString(self, 10, "Error: missing enddata line.");
	    break;
	case dataobject_OBJECTCREATIONFAILED:
	    message::DisplayString(self, 10, "Error: unable to create an object.");
	    break;
	case dataobject_BADFORMAT:
	    message::DisplayString(self, 10, "Error in Zip data file.");
	    break;
	default:
	    message::DisplayString(self, 10, "Error in reading Zip data file.");
	    break;
    }

    (self)->FlushDataChanges();
    (self)->ClearSelection();

    o = (class figogrp *)(fig)->FindObjectByRef( (self)->GetFocusRef());
    for (ix=(o)->GetRoot(); ix!=figure_NULLREF; ix=fig->objs[ix].next) {
	if (fig->objs[ix].counter >= count1 && fig->objs[ix].counter < count2) {
	    (self)->SelectByRef( ix);
	}
    }

    (fig)->SetModified();
    (fig)->NotifyObservers( figure_DATACHANGED);
}

/* deal with landscape mode -- oy. Let's make this clear: when we get view wrappers working, we will write a print-diddle wrapper which handles rotation and scaling of insets; and then all this figure landscape code will be deleted, you hear me, deleted.  */

static boolean PrintSplot(class figobj  *o, long  ref, class figure  *fig, struct printlump  *lump)
{
    class view *vtmp;
    struct rectangle insetb, *bbox;

    bbox = o->GetBounds(lump->figview);
    if (bbox->left+bbox->width < lump->visrect.left
	|| bbox->left > lump->visrect.left+lump->visrect.width
	|| bbox->top+bbox->height < lump->visrect.top
	|| bbox->top > lump->visrect.top+lump->visrect.height) {
	return FALSE; /* outside lump->visrect */
    }

    fprintf(lump->file, "%s  gsave\n", lump->prefix);	
    (o)->PrintObject( lump->figview, lump->file, lump->prefix, (lump->processor==NULL));
    fprintf(lump->file, "%s  grestore\n", lump->prefix);
    if ((o)->IsInset() && (vtmp=lump->figview->objs[ref].insetv)) {

	bbox = (lump->figview->objs[ref].o)->GetBounds( lump->figview);
	insetb.left = (lump->figview)->ToPrintPixX( bbox->left);
	insetb.top  = (lump->figview)->ToPrintPixY( bbox->top);
	insetb.width  = (lump->figview)->ToPrintPixW( bbox->width);
	insetb.height = (lump->figview)->ToPrintPixH( bbox->height);

	if (ATK::IsTypeByName((vtmp)->GetTypeName(), "figview")) {
	    ((class figview *)vtmp)->SetPrintRect( &insetb);
	}

	fprintf(lump->file, "%s  gsave newpath\n", lump->prefix);	
	fprintf(lump->file, "%s  %d %d translate\n", lump->prefix, insetb.left, insetb.top);
	fprintf(lump->file, "%s  0 0 moveto  %d 0 rlineto  0 %d rlineto  %d 0 rlineto closepath clip newpath\n", lump->prefix, insetb.width, insetb.height, -insetb.width);
	fprintf(lump->file, "%s  1 -1 scale %d %d translate\n", lump->prefix, 0, -insetb.height);
	if (lump->processor==NULL) {
	    struct rectangle visrect;
	    visrect.left = 0;
	    visrect.width = insetb.width;
	    visrect.top = insetb.height;
	    visrect.height = insetb.height;

	    (vtmp)->PrintPSRect(lump->file, insetb.width, insetb.height, &visrect);
	}
	else if (strcmp(lump->processor, "troff") == 0) {
	    (vtmp)->Print( lump->file, "PostScript", "troff", FALSE);
	}
	else {
	    (vtmp)->Print( lump->file, lump->processor, lump->format, FALSE);
	}
	fprintf(lump->file, "%s  grestore\n", lump->prefix);

	if (ATK::IsTypeByName((vtmp)->GetTypeName(), "figview")) {
	    ((class figview *)vtmp)->SetPrintRect( NULL);
	}

    }
    return FALSE;
}

void figview::Print(FILE  *file, char  *processor, char  *format, boolean  toplevel)
{
    class figure *fig = (class figure *)this->dataobject;
    long wpts, hpts;  /* image dimensions in points */
    long tmpval;
    char *prefix;
    struct printlump lump;
    boolean landscape = fig->GetPrintLandscape();
    
    this->FlushDataChanges();

    if (toplevel) {
	struct rectangle *rtmp = fig->GetOverallBounds();
	wpts = (long)(fig->GetPrintScaleX() * (double)this->ToPrintPixX( rtmp->left+rtmp->width));
	hpts = (long)(fig->GetPrintScaleY() * (double)this->ToPrintPixY( rtmp->top+rtmp->height));
	wpts += 12;
	hpts += 12;
	if (wpts<1) wpts = 1;
	if (hpts<1) hpts = 1;
	if (landscape) {
	    tmpval = wpts;
	    wpts = hpts;
	    hpts = tmpval;
	}
    }
    else {
	if (this->PrintRect) {
	    wpts = this->PrintRect->width;
	    hpts = this->PrintRect->height;
	}
	else {
	    wpts = this->GetLogicalWidth();
	    hpts = this->GetLogicalHeight();
	    if (wpts == 0  ||  hpts == 0) {
		/* the parent has GOOFED and has not
		 supplied a logical rectangle for printing */
		struct rectangle *RR;
		RR = fig->GetOverallBounds();
		wpts = this->ToPrintPixX( RR->left+RR->width);
		hpts = this->ToPrintPixY( RR->top+RR->height);
		if (landscape) {
		    tmpval = wpts;
		    wpts = hpts;
		    hpts = tmpval;
		}
	    }
	}
    }
    if (wpts < 0) wpts = 0;
    if (hpts < 0) hpts = 0;
    /* trim to 7.5 by 9 inches */
    if (wpts > 540) wpts = 540;
    if (hpts > 648) hpts = 648;

    /*printf("figview_Print: proc=%s, form=%s: %d, %d\n", processor, format, wpts, hpts);*/

    if (strcmp(processor, "troff") == 0) {
	/* output to troff */
	if (toplevel) {
	    /* take care of initial troff stream */
	    texttroff::BeginDoc(file);
	}
	/*  Put macro to interface to postscript */
	texttroff::BeginPS(file, wpts, hpts);
	prefix = "\\!  ";
    }
    else if (strcmp(format, "troff") == 0)
	prefix = "\\!  ";
    else prefix = "";

    /* generate PostScript  */
    fprintf(file, "%s  %% ATK figure inset beginning\n", prefix);
    fprintf(file, "%s  1 -1 scale  0 %d translate\n", prefix, -hpts);
    fprintf(file, "%s  newpath 0 0 moveto 0 %d lineto %d %d lineto\n", prefix, hpts, wpts, hpts);
    fprintf(file, "%s  %d 0 lineto clip newpath   %% clip to full (logical) area\n", prefix, wpts);

    lump.file = file;
    lump.prefix = prefix;
    lump.figview = this;
    lump.processor = processor;
    lump.format = format;

#if 0
    printf("visrect (PS cds) = [%d..%d][%d..%d]\n", 0, 0+wpts, hpts-hpts, hpts);

    /* dots mark visrect in print coords. Note that we have to flip within the logical rectangle to account for the original flip transform */
    fprintf(file, "%s  gsave %d %d 10 0 360 arc 0.5 setgray fill grestore\n", prefix, 0, hpts-(hpts));
    fprintf(file, "%s  gsave %d %d 10 0 360 arc 0.5 setgray fill grestore\n", prefix, 0, hpts-(hpts-hpts));
    fprintf(file, "%s  gsave %d %d 10 0 360 arc 0.5 setgray fill grestore\n", prefix, 0+wpts, hpts-(hpts));
    fprintf(file, "%s  gsave %d %d 10 0 360 arc 0.5 setgray fill grestore\n", prefix, 0+wpts, hpts-(hpts-hpts));

#endif

    if (landscape) {
	fprintf(file, "%s  90 rotate  0 %d translate   %% landscape printing\n", prefix, -wpts);
    }
    fprintf(file, "%s  %g %g scale\n", prefix, fig->GetPrintScaleX(), fig->GetPrintScaleY());	

    if (!landscape) {
	lump.visrect.left = this->FromPrintPixX((long)(0.0 / fig->GetPrintScaleX()));
	lump.visrect.top  = this->FromPrintPixY((long)(0.0 / fig->GetPrintScaleY()));
	lump.visrect.width = this->FromPrintPixW((long)((double)(wpts) / fig->GetPrintScaleX()));
	lump.visrect.height = this->FromPrintPixH((long)((double)(hpts) / fig->GetPrintScaleY()));
    }
    else {
	lump.visrect.left = this->FromPrintPixX((long)((double)(hpts - hpts) / fig->GetPrintScaleX()));
	lump.visrect.top  =  this->FromPrintPixY((long)((double)(wpts - wpts - 0) / fig->GetPrintScaleY()));
	lump.visrect.width = this->FromPrintPixW((long)((double)(hpts) / fig->GetPrintScaleX()));
	lump.visrect.height = this->FromPrintPixH((long)((double)(wpts) / fig->GetPrintScaleY()));
    }

#if 0
    { /* debugging -- draw visrect rectangle */
	long x, y, w, h;

	x = this->ToPrintPixX(lump.visrect.left);
	y = this->ToPrintPixY(lump.visrect.top);
	w = this->ToPrintPixW(lump.visrect.width);
	h = this->ToPrintPixH(lump.visrect.height);
	fprintf(file, "%s  gsave %d %d moveto  %d %d lineto  %d %d lineto  %d %d lineto closepath 2 setlinewidth 0.5 setgray stroke grestore\n", prefix, x, y,  x, y+h,  x+w, y+h,  x+w, y);

    }
#endif

    fig->EnumerateObjectTree( figure_NULLREF, NULL, FALSE, (figure_eofptr)PrintSplot, (long)&lump);

    if (strcmp(processor, "troff") == 0) {
	texttroff::EndPS(file, wpts, hpts);
	if (toplevel)
	    texttroff::EndDoc(file);
    }
}

void figview::PrintPSRect(FILE *file, long logwidth, long logheight, struct rectangle *visrect)
{
    class figure *fig = (class figure *)this->dataobject;
    struct printlump lump;
    boolean landscape;

    if (!fig)
	return;

    landscape = fig->GetPrintLandscape();
    /* in landscape mode, we rotate the image 90 degrees within the logical rectangle. */

    /* generate PostScript  */
    fprintf(file, "%% ATK figure inset (PrintPSRect) beginning\n");
    fprintf(file, "1 -1 scale  0 %d translate\n", -logheight); /* flip within logical rectangle */
    fprintf(file, "newpath 0 0 moveto 0 %d lineto %d %d lineto\n", logheight, logwidth, logheight);
    fprintf(file, "%d 0 lineto clip newpath   %% clip to full (logical) area\n", logwidth);

    lump.file = file;
    lump.prefix = "";
    lump.figview = this;
    lump.processor = NULL;
    lump.format = NULL;

#if 0
    printf("visrect (PS cds) = [%d..%d][%d..%d]\n", visrect->left, visrect->left+visrect->width, visrect->top-visrect->height, visrect->top);

    /* dots mark visrect in print coords. Note that we have to flip within the logical rectangle to account for the original flip transform */
    fprintf(file, "gsave %d %d 10 0 360 arc 0.5 setgray fill grestore\n", visrect->left, logheight-(visrect->top));
    fprintf(file, "gsave %d %d 10 0 360 arc 0.5 setgray fill grestore\n", visrect->left, logheight-(visrect->top-visrect->height));
    fprintf(file, "gsave %d %d 10 0 360 arc 0.5 setgray fill grestore\n", visrect->left+visrect->width, logheight-(visrect->top));
    fprintf(file, "gsave %d %d 10 0 360 arc 0.5 setgray fill grestore\n", visrect->left+visrect->width, logheight-(visrect->top-visrect->height));

#endif

    /* adjust rotation and scale */
    if (landscape) {
	fprintf(file, "90 rotate  0 %d translate   %% landscape printing\n", -logwidth);
    }
    fprintf(file, "%g %g scale\n", fig->GetPrintScaleX(), fig->GetPrintScaleY());	

    /* Now let's get some stuff clear here.
     The following code converts print (postscript) coordinates to figure coordinates, so that the object-clipping operations in PrintSplot will work correctly.
     I am fairly certain I understand the math behind the portrait-mode transformation. I do not even slightly understand the math behind the landscape-mode transformation. I generated this code by printing some test figures, measuring how far off the transformation was, finding some variables which could be added together to sum up to the error, and subtracting those variables from the expressions below.
     As of today (4/25/94), it works. It will not be changed. If you think you have a reason to change it, you are wrong. If you change it anyway, and it breaks, I will not fix it. If you ask me to fix it anyway, I will kill you. Then I will kill myself.
     Thank you. -ap1i */
     

    if (!landscape) {
	/* gack -- convert visrect (print coords) to lump.visrect (fig coords) */
	lump.visrect.left = this->FromPrintPixX((long)((double)(visrect->left) / fig->GetPrintScaleX()));
	lump.visrect.top  = this->FromPrintPixY((long)((double)(logheight-visrect->top) / fig->GetPrintScaleY()));
	lump.visrect.width = this->FromPrintPixW((long)((double)(visrect->width) / fig->GetPrintScaleX()));
	lump.visrect.height = this->FromPrintPixH((long)((double)(visrect->height) / fig->GetPrintScaleY()));
    }
    else {
	/* gack -- convert visrect (print coords) to lump.visrect (fig coords) with a 90 degree rotation */
	lump.visrect.left = this->FromPrintPixX((long)((double)(logheight - visrect->top) / fig->GetPrintScaleX()));
	lump.visrect.top  =  this->FromPrintPixY((long)((double)(logwidth - visrect->width - visrect->left) / fig->GetPrintScaleY()));
	lump.visrect.width = this->FromPrintPixW((long)((double)(visrect->height) / fig->GetPrintScaleX()));
	lump.visrect.height = this->FromPrintPixH((long)((double)(visrect->width) / fig->GetPrintScaleY()));
    }

#if 0
    { /* debugging -- draw visrect rectangle */
	long x, y, w, h;

	printf("ditto  (fig cds) = [%d..%d][%d..%d]\n", lump.visrect.left, lump.visrect.left+lump.visrect.width, lump.visrect.top, lump.visrect.top+lump.visrect.height);

	/* rectangle marks visrect in figure coords */
	x = this->ToPrintPixX(lump.visrect.left);
	y = this->ToPrintPixY(lump.visrect.top);
	w = this->ToPrintPixW(lump.visrect.width);
	h = this->ToPrintPixH(lump.visrect.height);
	fprintf(file, "gsave %d %d moveto  %d %d lineto  %d %d lineto  %d %d lineto closepath 2 setlinewidth 0.5 setgray stroke grestore\n", x, y,  x, y+h,  x+w, y+h,  x+w, y);

    }
#endif

    fig->EnumerateObjectTree( figure_NULLREF, NULL, FALSE, (figure_eofptr)PrintSplot, (long)&lump);

}

#define MARGIN (36)  /* leave a half-inch margin, ideally */

void figview::PrintPSDoc(FILE *outfile, long pagew, long pageh)
{
    class figure *fig = (class figure *)this->GetDataObject();
    boolean landscape;
    long wpts, hpts, tmpval, posx, posy, marginw, marginh;
    struct rectangle visrect, *rtmp;
    long areaw, areah; /* size of printing area (page size minus margin) */

    if (fig == NULL) {
	/* no image */
	return;
    }

    this->FlushDataChanges();
    landscape = fig->GetPrintLandscape();

    rtmp = fig->GetOverallBounds();
    wpts = (long)(fig->GetPrintScaleX() * (double)this->ToPrintPixX( rtmp->left+rtmp->width));
    hpts = (long)(fig->GetPrintScaleY() * (double)this->ToPrintPixY( rtmp->top+rtmp->height));
    wpts += 12;
    hpts += 12;
    if (wpts<1) wpts = 1;
    if (hpts<1) hpts = 1;
    if (landscape) {
	tmpval = wpts;
	wpts = hpts;
	hpts = tmpval;
    }

    if (pagew >= 4*MARGIN)
	marginw = MARGIN;
    else
	marginw = 0;
    if (pageh >= 4*MARGIN)
	marginh = MARGIN;
    else
	marginh = 0;
    areaw = pagew - (2*marginw);
    areah = pageh - (2*marginh);

    fprintf(outfile, "%% This document was generated by an AUIS figure object.\n");

    for (posy=0; posy<hpts; posy+=areah)
	for (posx=0; posx<wpts; posx+=areaw) {
	    print::PSNewPage(print_UsualPageNumbering);
	    /* posx,posy are print coords of upper left corner of this image section. Oop, except that posy should be (hpts-posy). */
	    fprintf(outfile, "%d %d translate\n", marginw-posx, marginh+areah-(hpts-posy));
	    visrect.left = 0 + posx;
	    visrect.top = hpts-posy;
	    visrect.width = areaw;
	    visrect.height = areah;
	    fprintf(outfile, "%d %d moveto\n", visrect.left, visrect.top-visrect.height);
	    fprintf(outfile, "%d 0 rlineto  0 %d rlineto  %d 0 rlineto  0 %d rlineto\n", visrect.width, visrect.height, -visrect.width, -visrect.height);
	    fprintf(outfile, "closepath clip newpath\n");
	    this->PrintPSRect(outfile, wpts, hpts, &visrect);
	}

}

void *figview::GetPSPrintInterface(char *printtype)
{
    if (!strcmp(printtype, "generic"))
	return (void *)this;

    return NULL;
}

void figview::DesiredPrintSize(long width, long height, enum view_DSpass pass, long *desiredwidth, long *desiredheight)
{
    class figure *fig = (class figure *)this->GetDataObject();
    boolean landscape;
    long wpts, hpts, tmpval;  
    struct rectangle *rtmp;

    if (fig == NULL) {
	/* no image */
	*desiredwidth = 0;
	*desiredheight = 0;
	return;
    }

    this->FlushDataChanges();
    landscape = fig->GetPrintLandscape();

    rtmp = fig->GetOverallBounds();
    wpts = (long)(fig->GetPrintScaleX() * (double)this->ToPrintPixX( rtmp->left+rtmp->width));
    hpts = (long)(fig->GetPrintScaleY() * (double)this->ToPrintPixY( rtmp->top+rtmp->height));
    wpts += 12;
    hpts += 12;
    if (wpts<1) wpts = 1;
    if (hpts<1) hpts = 1;
    if (landscape) {
	tmpval = wpts;
	wpts = hpts;
	hpts = tmpval;
    }

    *desiredwidth = wpts;
    *desiredheight = hpts;
}



static void ToolsetCreateProc(class figview  *self, char  *rock)
{
    class figure *fig = (class figure *)(self)->GetDataObject();
    class im *im;
    class frame *fr;
    class figtoolview *tv;
    boolean res;

    if (self->toolset)  {
	message::DisplayString(self, 40, "There is already a toolset window.");
	return;
    }

    if (!fig) {
	message::DisplayString(self, 40, "This view has no data!");
	return;
    }

    im = im::Create(NULL);
    if (!im) {
	message::DisplayString(self, 40, "Unable to create new window.");
	return;
    }

    fr = new frame;
    if (!fr) {
	message::DisplayString(self, 40, "Unable to create window frame.");
	return;
    }

    /*frame_SetCommandEnable(fr, TRUE);*/
    (fr)->PostDefaultHandler( "message", (fr)->WantHandler( "message"));

    tv = new figtoolview;
    if (!tv) {
	message::DisplayString(self, 40, "Unable to create toolset.");
	return;
    }

    res = (tv)->SetPrimaryView( self);
    if (!res) {
	message::DisplayString(self, 40, "Unable to initialize toolset (this shouldn't happen).");
	return;
    }

    self->toolset = tv;
    (fr)->SetView( tv);
    /*(fr)->SetQuitWindowFlag( TRUE);*/
    (im)->SetView( fr);
    {   
	class buffer *buf = buffer::FindBufferByData(fig); 
	char *nm;
	if (buf) {
	    nm = (buf)->GetFilename();
	    if (nm)
		(im)->SetTitle( nm);
	}
    }
    (tv)->WantInputFocus( tv);
    (tv)->SetExpertMode( self->expertmode);
    RepostMenus(self);
}

void figview::DestroyToolset()
{
    ToolsetKillProc(this, 0);
}

static void ToolsetKillProc(class figview  *self, char  *rock)
{
    class im *toolim = NULL;
    class frame *toolfr = NULL;

    if (!self->toolset) {
	message::DisplayString(self, 40, "There is no toolset window.");
	return;
    }

    if (self->toolset->creating)
	(self->toolset)->AbortObjectBuilding();

    toolim = (self->toolset)->GetIM();
    if (toolim)
	toolfr = (class frame *)toolim->topLevel;

    (self->toolset)->RemoveObserver( self);
    (self)->RemoveObserver( self->toolset);

    /*(self->toolset)->SetMoribund( 1);*/ /* ensure that toolset doesn't try to kill itself when it sees it's being unlinked */

    if (toolim) {
	(toolim)->SetView( NULL);
    }

    if (toolfr) {
	(toolfr)->SetView( NULL);
    }

    (self->toolset)->Destroy();
    self->toolset = NULL;

    if (toolim) {
	(toolim)->Destroy(); 
    }
    if (toolfr) {
	(toolfr)->Destroy();
    }

    (self)->ClearSelection();
    (self)->WantUpdate( self);
    RepostMenus(self); 
}

void figview::BuildToolList(struct figtoolview_tool_t *&list, int &listnum, int &listsize)
{
#define FIGOBJS_NUM (12)
    static struct figtoolview_tool_t objectlayout[FIGOBJS_NUM] = {
	{"figotext", 0},
	{"figoplin", 1},
	{"figoplin", 0},
	{"figoplin", 2},
	{"figoplin", 3},
	{"figospli", 0},
	{"figospli", 2},
	{"figorect", 0},
	{"figoell",  0},
	{"figorrec", 0},
	{"figoins",  0},
	{"figoins",  1},
    };
    int ix;

    if (listnum+FIGOBJS_NUM > listsize) {
	while (listnum+FIGOBJS_NUM > listsize)
	    listsize *= 2;
	list = (struct figtoolview_tool_t *)realloc(list, listsize * sizeof(struct figtoolview_tool_t));
    }

    for (ix=0; ix<FIGOBJS_NUM; ix++) {
	list[listnum+ix] = objectlayout[ix];
    }
    listnum += FIGOBJS_NUM;
}

static void ChangeZoomProc(class figview  *self, long  rock)
{
    long newscale;
    long midx, midy, offx, offy;
    struct rectangle logrec;
    boolean atorigin = FALSE;

    if (self->panx == self->originx && self->pany == self->originy) 
	atorigin = TRUE;

    (self)->GetLogicalBounds( &logrec);
    midx = (self)->ToFigW( logrec.left+logrec.width/2) - self->panx;
    midy = (self)->ToFigH( logrec.top+logrec.height/2) - self->pany;

    if (rock<0) 
	newscale = self->scale / 2;
    else if (rock>0)
	newscale = self->scale * 2;
    else newscale = figview_NormScale;

    if (newscale<1) newscale=1;
    else if (newscale>1024) newscale=1024;

    if (newscale != self->scale) {
	self->scale = newscale;

	if (newscale == figview_NormScale && atorigin) {
	    /* stay at origin */
	}
	else {
	    offx = (self)->ToFigW( logrec.left+logrec.width/2) - self->panx;
	    offy = (self)->ToFigH( logrec.top+logrec.height/2) - self->pany;
	    self->panx -= (offx - midx);
	    self->pany -= (offy - midy);
	}
	self->NeedFullUpdate = TRUE;
	(self)->WantUpdate( self);
    }


}

/* 1 to zoom in, -1 to zoom out, 0 to zoom norm */
void figview::ChangeZoom(long  val)
{
    ChangeZoomProc(this, val);
}

static void PanToOriginProc(class figview  *self, long  rock)
{
    if (self->panx==self->originx && self->pany==self->originy)
	return;

    self->panx = self->originx;
    self->pany = self->originy;
    /* self->NeedFullUpdate = TRUE; */
    (self)->WantUpdate( self);
}

void figview::LinkTree( class view  *parent )
{
    this->view::LinkTree( parent);
    if (parent && this->GetIM()) {
	register int ix;
	class figobj *o;
	for (ix = 0; ix < this->objs_size; ix++ ) {
	    o = this->objs[ix].o;
	    if (o && (o)->IsInset() && this->objs[ix].insetv)
		(this->objs[ix].insetv)->LinkTree( this);
	}
    }
}

static boolean figview_RecSearchLoop(class figview *self, long pos, struct SearchPattern *pat)
{
    long ix;
    class figobj *o;
    class view *v;

    for (ix = pos; ix < self->objs_size; ix++ ) {
	o = self->objs[ix].o;
	if (o) {
	    if ((o)->IsInset() && (v=self->objs[ix].insetv)) {
		if (v->RecSearch(pat, FALSE)) {
		    self->recsearchvalid = TRUE;
		    self->recsearchpos = ix;
		    self->recsearchchild = v;
		    return TRUE;
		}
	    }
	    else {
		if (o->ORecSearch(pat)) {
		    self->recsearchvalid = TRUE;
		    self->recsearchpos = ix;
		    self->recsearchchild = NULL;
		    return TRUE;
		}
	    }
	}
    }
    self->recsearchvalid = FALSE;
    self->recsearchchild = NULL;
    return FALSE;
}

boolean figview::RecSearch(struct SearchPattern *pat, boolean toplevel)
{
    this->FlushDataChanges();

    return figview_RecSearchLoop(this, 0, pat);
}

boolean figview::RecSrchResume(struct SearchPattern *pat)
{
    long pos;

    this->FlushDataChanges();

    if (!this->recsearchvalid)
	return FALSE;

    pos = this->recsearchpos;

    if (this->recsearchchild) {
	/* recsearchchild is the child that got the last success */
	if (this->recsearchchild->RecSrchResume(pat)) {
	    /* pos and child stay the same */
	    this->recsearchvalid = TRUE;
	    return TRUE;
	}

	/* ok, that child ran out of matches. restart right after it. */
	pos ++;
    }
    else {
	class figobj *o;
	o = this->objs[this->recsearchpos].o;
	if (o->ORecSrchResume(pat)) {
	    /* pos and child stay the same */
	    this->recsearchvalid = TRUE;
	    return TRUE;
	}
	/* ok, that figobj ran out of matches. restart right after it */
	pos ++;
    }

    return figview_RecSearchLoop(this, pos, pat);
}

boolean figview::RecSrchReplace(class dataobject *srctext, long srcpos, long srclen)
{
    class figobj *o;

    if (!this->recsearchvalid)
	return FALSE;

    if (this->recsearchchild) {
	/* hand it off to the child instead */
	return this->recsearchchild->RecSrchReplace(srctext, srcpos, srclen);
    }

    o = this->objs[this->recsearchpos].o;
    return o->ORecSrchReplace(srctext, srcpos, srclen);
}

static void ScrollToObj(class figview *self, class figobj *o)
{
    struct rectangle log;
    long px, py;
    static ATKregistryEntry *figorect_reg = NULL;

    if (!figorect_reg) {
	figorect_reg = ATK::LoadClass("figorect");
    }

    self->GetLogicalBounds(&log);

    if (o->IsType(figorect_reg)) {
	px = o->PosX() + ((class figorect *)o)->PosW() / 2;
	py = o->PosY() + ((class figorect *)o)->PosH() / 2;
    }
    else {
	px = o->PosX();
	py = o->PosY();
    }

    self->panx = px - self->ToFigW(log.width/2);
    self->pany = py - self->ToFigH(log.height/2);
    (self)->WantUpdate(self);
}

/* This needs to be fixed to actually compute where the child should be positioned in the logical rectangle. */
void figview::RecSrchExpose(const struct rectangle &logical, struct rectangle &hit)
{
    class figobj *o;

    if (!this->recsearchvalid) return;

    if (this->recsearchchild) {
	/* hand it off to the child instead */
	o = this->objs[this->recsearchpos].o;
	ScrollToObj(this, o);
	hit=logical;
	if(!recsearchchild->GetIM()) recsearchchild->LinkTree(this);
	this->recsearchchild->RecSrchExpose(logical, hit);
	return;
    }

    o = this->objs[this->recsearchpos].o;
    ScrollToObj(this, o);
    this->ClearSelection();
    this->Select(o);
    this->WantInputFocus(this);

    return;
}

void figview::ExposeChild(class view *v)
{
    int ix;
    class figobj *o;

    for (ix = 0; ix < this->objs_size; ix++ ) {
	o = this->objs[ix].o;
	if (o) {
	    if ((o)->IsInset() && (v==this->objs[ix].insetv)) {
		ScrollToObj(this, o);
		return;
	    }
	}
    }
}

