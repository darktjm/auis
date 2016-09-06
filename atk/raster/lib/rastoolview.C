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

#include <andrewos.h>
ATK_IMPL("rastoolview.H")

#include <stdio.h>
#include <math.h>

#include <rastoolview.H>

#include <menulist.H>
#include <keymap.H>
#include <keystate.H>
#include <cursor.H>
#include <proctable.H>

#include <raster.H>
#include <rasterview.H>
#include <observable.H>
#include <fontdesc.H>
#include <graphic.H>
#include <frame.H>
#include <rasterimage.H>
#include <stringtbl.H>
#include <strtblview.H>
#include <lpair.H>
#include <text.H>
#include <textview.H>
#include <view.H>
#include <event.H>
#include <im.H>

#include <pattern.h>
#include <rect.h>

#define ZRPATTERN_INVERT ((unsigned char *)NULL)
#define ZRBRUSH_PIXEL ((unsigned char *)NULL)

      

ATKdefineRegistry(rastoolview, lpair, rastoolview::InitializeClass);

static void RepostMenus(class rastoolview  *self);
static void SetToolNum(class rastoolview  *self, int  toolnum);
static void Command_Refresh(class rastoolview  *self, char  *rock);
static void Command_Copy(class rastoolview  *self, char  *rock);
static void Command_ZoomIn(class rastoolview  *self, char  *rock);
static void Command_ZoomOut(class rastoolview  *self, char  *rock);
static void Command_ZoomNorm(class rastoolview  *self, char  *rock);
static void Command_Quit(class rastoolview  *self, char  *rock);
static void CallCommandProc(class stringtbl  *st, class rastoolview  *self, short  accnum);
static void CallPasteModeProc(class stringtbl  *st, class rastoolview  *self, short  accnum);
static void SetPatternProc(class stringtbl  *st, class rastoolview  *self, short  accnum);
static void SetBrushProc(class stringtbl  *st, class rastoolview  *self, short  accnum);
static void SetToolProc(class stringtbl  *st, class rastoolview  *self, short  accnum);
static void DrawLine(class rasterview  *rself, long  x0 , long  y0 , long  x1 , long  y1, unsigned char *pattern, unsigned char *brush);
static void DrawCircle(class rasterview  *rself, long  x0 , long  y0 , long  rad, unsigned char *pattern , unsigned char *brush);
static void DrawEllipse(class rasterview  *rself, long  x0 , long  y0, long  xrad , long  yrad /* both must be >= 0 */, unsigned char *pattern , unsigned char *brush);
static void FillRectangle(class rastoolview  *self, long  x0 , long  y0, long  wid , long  hgt, unsigned char *pattern);
static void FillCircle(class rastoolview  *self, long  x0 , long  y0, long  rad, unsigned char *pattern);
static void FillEllipse(class rastoolview  *self, long  x0 , long  y0, long  xrad , long  yrad /* both must be >= 0 */, unsigned char *pattern);
static void DrawRectangle(class rasterview  *rself, long  x0 , long  y0, long  wid , long  hgt, unsigned char *pattern , unsigned char *brush);
static void Tool_SolidRect(class rastoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks);
static void Tool_SolidCircle(class rastoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks);
static void Tool_SolidEllipse(class rastoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks);
static void Tool_Paint(class rastoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks);
static void Tool_Line(class rastoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks);
static void Tool_Circle(class rastoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks);
static void Tool_Ellipse(class rastoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks);
static void Tool_Rectangle(class rastoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks);
static void Tool_Text(class rastoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks);
static class raster *GetPasted(class rastoolview  *self);
static void Tool_Paste(class rastoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks);
static void Tool_FloodFill(class rastoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks);
static void FloodSplot(class rastoolview  *self);
static void Tool_SprayPaint(class rastoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks);
static void SpraySplot(class rastoolview  *self);
static void Tool_CurvePaint(class rastoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks);
static void CurveSplot(class rastoolview  *self);
static void PasteDownProc(class rastoolview  *self, char  *rock);
static void Toolmod_Paste(class rastoolview  *self, char  *rock);
static void PasteResplot(class rastoolview  *self, char  *rock);
static void Toolmod_FloodFill(class rastoolview  *self, char  *rock);
static void Toolmod_SprayPaint(class rastoolview  *self    , char  *rock);
static void Toolmod_CurvePaint(class rastoolview  *self    , char  *rock);
static void Toolmod_Text(class rastoolview  *self    , char  *rock);
static void ResizeInsetProc(class rastoolview  *self    , char  *rock);
static void RemoveInsetProc(class rastoolview  *self    , char  *rock);
   
 

struct layout_t {
    rastoolview_toolfptr proc; /* tool procedure */
    rastoolview_toolmodfptr procmod; /* procedure to call when tool is double-clicked */
    boolean writes; /* does the tool change the raster data? */
    char name[16]; /* name in the toolset window */
};

struct layout_c {
    rastoolview_procfptr proc; /* command procedure */
    char *rock;	    /* a rock */
    char name[16]; /* name in the toolset window */
};

#define RASTOOL_PAINT (6)
static struct layout_t toollayout[ZRTOOLS_NUM] = {
    {NULL, NULL, 0, "TOOLS:"},
    {NULL, NULL, 0, "Pan"},
    {NULL, NULL, 0, "Select"},
    {NULL, NULL, 1, "Touch up"},
    {(rastoolview_toolfptr)Tool_Paste, (rastoolview_toolmodfptr)Toolmod_Paste, 1, "Paste"},
    {(rastoolview_toolfptr)Tool_Text, (rastoolview_toolmodfptr)Toolmod_Text, 1, "Text"},
    {(rastoolview_toolfptr)Tool_Paint, NULL, 1, "Paint"},
    {(rastoolview_toolfptr)Tool_Line, NULL, 1, "Line"},
    {(rastoolview_toolfptr)Tool_Rectangle, NULL, 1, "Rectangle"},
    {(rastoolview_toolfptr)Tool_Circle, NULL, 1, "Circle"},
    {(rastoolview_toolfptr)Tool_Ellipse, NULL, 1, "Ellipse"},
    {(rastoolview_toolfptr)Tool_SolidRect, NULL, 1, "Solid rectangle"},
    {(rastoolview_toolfptr)Tool_SolidCircle, NULL, 1, "Solid circle"},
    {(rastoolview_toolfptr)Tool_SolidEllipse, NULL, 1, "Solid ellipse"},
    {(rastoolview_toolfptr)Tool_FloodFill, (rastoolview_toolmodfptr)Toolmod_FloodFill, 1, "Flood fill"},
    {(rastoolview_toolfptr)Tool_SprayPaint, (rastoolview_toolmodfptr)Toolmod_SprayPaint, 1, "Spraypaint"},
    {(rastoolview_toolfptr)Tool_CurvePaint, (rastoolview_toolmodfptr)Toolmod_CurvePaint, 1, "Curvy paint"}
};

static struct layout_c commandlayout[ZRCMDS_NUM] = {
    {NULL,		NULL,	"COMMANDS:"},
    {Command_Copy,	NULL,	"Copy"},
    {Command_ZoomIn,	NULL,	"Zoom in"},
    {Command_ZoomOut,	NULL,	"Zoom out"},
    {Command_ZoomNorm,	NULL,	"Normal size"},
    {Command_Refresh,	NULL,	"Refresh"}
};

static char pastemodelayout[ZRPASTEMODES_NUM][24] = {
    "PASTE MODES:",
    "Replace",
    "Combine",
    "Invert"
};

#define	ML_unpaste	    (1)
#define	ML_inset	    (2)

static class menulist *Menus;
static class keymap *Keymap;

#define DEFINEPROC(name, proc, registry, module, doc) proctable::DefineProc(name, (proctable_fptr)proc, registry, module, doc)
 
boolean rastoolview::InitializeClass()
{
    struct proctable_Entry *proc = NULL;
    
    ::Menus = new menulist;
    Keymap = new keymap;

    proc = DEFINEPROC("rastoolv-toolset-destroy", Command_Quit, &rastoolview_ATKregistry_ , 0, "Deletes toolset window.");
    (Keymap)->BindToKey( "\030\004", proc, 0);	/* ^X^D */
    (Keymap)->BindToKey( "\030\003", proc, 0);	/* ^X^C */
    (::Menus)->AddToML( "Delete Window~89", proc, 0, 0);

    proc = DEFINEPROC("rastoolv-pastedown", PasteDownProc, &rastoolview_ATKregistry_ , 0, "Paste down paste region onto raster.");
    (::Menus)->AddToML( "Paste Down Region~11", proc, 0, ML_unpaste);
    (Keymap)->BindToKey( "\031", proc, 0);	/* ^Y */

    proc = DEFINEPROC("rastoolv-unpaste", Toolmod_Paste, &rastoolview_ATKregistry_ , 0, "Remove paste region from raster.");
    (::Menus)->AddToML( "Remove Paste Region~12", proc, 0, ML_unpaste);

    proc = DEFINEPROC("rastoolv-remove-inset", RemoveInsetProc, &rastoolview_ATKregistry_ , 0, "Remove the overlaid inset.");
    (::Menus)->AddToML( "Remove Inset~22", proc, 0, ML_inset);

    proc = DEFINEPROC("rastoolv-resize-inset", ResizeInsetProc, &rastoolview_ATKregistry_ , 0, "Resize the overlaid inset.");
    (::Menus)->AddToML( "Resize Inset~23", proc, 0, ML_inset);

    proc = DEFINEPROC("rastoolv-imprint-inset", Toolmod_Text, &rastoolview_ATKregistry_ , 0, "Imprint the overlaid inset.");
    (::Menus)->AddToML( "Paste Down Inset~21", proc, 0, ML_inset);

    proc = DEFINEPROC("rastoolv-set-spray-size", Toolmod_SprayPaint, &rastoolview_ATKregistry_ , 0, "Set radius of spraypaint tool.");
    (::Menus)->AddToML( "Set Spray Radius~31", proc, 0, 0);

    proc = DEFINEPROC("rastoolv-set-curve-spring", Toolmod_CurvePaint, &rastoolview_ATKregistry_ , 0, "Set spring stiffness in curvepaint tool.");
    (::Menus)->AddToML( "Set Curvypaint Spring~32", proc, 0, 0);

    proc = DEFINEPROC("rastoolv-floodfill-abort", Toolmod_FloodFill, &rastoolview_ATKregistry_ , 0, "Abort flood fill operation.");
    (::Menus)->AddToML( "Abort Flood Fill~33", proc, 0, 0);
    (Keymap)->BindToKey( "\003", proc, 0);	/* ^C */

    return TRUE;
}

rastoolview::rastoolview()
{
	ATKinit;

    int ix;

    class lpair *lp1 = new lpair;
    class lpair *lp2 = new lpair;
    class lpair *lp3 = new lpair;

    class stringtbl *tl = new stringtbl;
    class strtblview *tlv = new strtblview;

    class stringtbl *tlc = new stringtbl;
    class strtblview *tlcv = new strtblview;

    class stringtbl *tlm = new stringtbl;
    class strtblview *tlmv = new strtblview;

    class stringtbl *tlp = new stringtbl;
    class strtblview *tlpv = new strtblview;

    class stringtbl *tlb = new stringtbl;
    class strtblview *tlbv = new strtblview;

    if (!lp1 || !lp2 || !tl || !tlv || !tlm || !tlmv || !tlc || !tlcv || !tlp || !tlpv  || !tlb || !tlbv) {
	THROWONFAILURE( FALSE);
    }

    (tl)->Clear();
    for (ix=0; ix<ZRTOOLS_NUM; ix++) {
	this->toolacc[ix] = (tl)->AddString( toollayout[ix].name);
	if (this->toolacc[ix] == (-1))
	    THROWONFAILURE( FALSE);
    }
    (tlv)->SetItemHitProc((strtblview_hitfptr) SetToolProc, (long)this);
    (tlv)->SetDataObject( tl);

    (tlc)->Clear();
    for (ix=0; ix<ZRCMDS_NUM; ix++) {
	this->commandacc[ix] = (tlc)->AddString( commandlayout[ix].name);
	if (this->commandacc[ix] == (-1))
	    THROWONFAILURE( FALSE);
    }
    (tlcv)->SetItemHitProc((strtblview_hitfptr) CallCommandProc, (long)this);
    (tlcv)->SetDataObject( tlc);

    (tlm)->Clear();
    for (ix=0; ix<ZRPASTEMODES_NUM; ix++) {
	this->pastemodeacc[ix] = (tlm)->AddString( pastemodelayout[ix]);
	if (this->pastemodeacc[ix] == (-1))
	    THROWONFAILURE( FALSE);
    }
    (tlmv)->SetItemHitProc((strtblview_hitfptr) CallPasteModeProc, (long)this);
    (tlmv)->SetDataObject( tlm);

    (tlp)->Clear();
    for (ix=0; ix<ZRPATTERNS_NUM; ix++) {
	this->patternacc[ix] = (tlp)->AddString( patternnames[ix]);
	if (this->patternacc[ix] == (-1))
	    THROWONFAILURE( FALSE);
    }
    (tlpv)->SetItemHitProc((strtblview_hitfptr) SetPatternProc, (long)this);
    (tlpv)->SetDataObject( tlp);

    (tlb)->Clear();
    for (ix=0; ix<ZRBRUSHES_NUM; ix++) {
	this->brushacc[ix] = (tlb)->AddString( brushnames[ix]);
	if (this->brushacc[ix] == (-1))
	    THROWONFAILURE( FALSE);
    }
    (tlbv)->SetItemHitProc((strtblview_hitfptr) SetBrushProc, (long)this);
    (tlbv)->SetDataObject( tlb);

    this->lpair1 = lp1;
    this->lpair2 = lp2;
    this->lpair3 = lp3;
    this->tooltbl = tl;
    this->vtooltbl = tlv;
    this->commandtbl = tlc;
    this->vcommandtbl = tlcv;
    this->pastemodetbl = tlm;
    this->vpastemodetbl = tlmv;
    this->patterntbl = tlp;
    this->vpatterntbl = tlpv;
    this->brushtbl = tlb;
    this->vbrushtbl = tlbv;
    this->pastemode = pixelimage_COPY;
    this->pastemodenum = PASTEMODE_COPY;
    this->toolnum = RASTOOL_PAN;
    this->toolproc = NULL;
    this->patternnum = 2;
    this->pattern = patternlist[this->patternnum];
    this->brushnum = 1;
    this->brush = NULL;
    this->fillstack = NULL;
    this->fillpix = NULL;
    this->springconst = 0.2;
    this->sprayradius = 10;
    (tl)->ClearBits();
    (tlc)->ClearBits();
    (tlm)->ClearBits();
    (tlp)->ClearBits();
    (tlb)->ClearBits();
    (tl)->SetBitOfEntry( this->toolacc[this->toolnum], TRUE);
    (tlm)->SetBitOfEntry( this->pastemodeacc[this->pastemodenum], TRUE);
    (tlp)->SetBitOfEntry( this->patternacc[this->patternnum], TRUE);
    (tlb)->SetBitOfEntry( this->brushacc[this->brushnum], TRUE);

    this->primaryview = NULL;
    this->moribund = 0;

    (lp3)->SetUp( tlcv, tlmv, 30, lpair_PERCENTAGE, lpair_HORIZONTAL, TRUE);
    (lp1)->SetUp( tlv, lp3, 50, lpair_PERCENTAGE, lpair_VERTICAL, TRUE);
    (lp2)->SetUp( tlpv, tlbv, 50, lpair_PERCENTAGE, lpair_VERTICAL, TRUE);
    (this)->SetUp( lp1, lp2, 50, lpair_PERCENTAGE, lpair_VERTICAL, TRUE);

    this->pasteraster = new raster;
    this->unpasteraster = new raster;
    this->unpaste = FALSE;

    this->Menus = (::Menus)->DuplicateML( this);
    this->Keystate = keystate::Create(this, Keymap);

    THROWONFAILURE( TRUE);
}

rastoolview::~rastoolview()
{
	ATKinit;


    if (this->fillpix) {
	(this->fillpix)->Destroy();
    }

    (this->pasteraster)->Destroy();
    (this->unpasteraster)->Destroy();
    (this->Menus)->ClearChain();
    delete this->Menus;
    delete this->Keystate;

    this->primaryobj = NULL;

    (this)->SetNth( 0, NULL);
    (this)->SetNth( 1, NULL);
    (this->lpair2)->Destroy();
    (this->lpair1)->Destroy();
    (this->lpair3)->Destroy();

    (this->vpatterntbl)->Destroy();
    (this->vbrushtbl)->Destroy();
    (this->vcommandtbl)->Destroy();
    (this->vpastemodetbl)->Destroy();
    (this->vtooltbl)->Destroy();

    (this->patterntbl)->Destroy();
    (this->brushtbl)->Destroy();
    (this->commandtbl)->Destroy();
    (this->pastemodetbl)->Destroy();
    (this->tooltbl)->Destroy();
}

static void RepostMenus(class rastoolview  *self)
{
    long menumask = 0;

    if (self->unpaste)
	menumask |= ML_unpaste;

    if ((self->primaryview)->GetOverlaidInset())
	menumask |= ML_inset;

    if ((self->Menus)->SetMask( menumask)) {
	(self)->PostMenus( NULL);
    }
}

void rastoolview::PostMenus(class menulist  *ml)
{
/* Enable the menus for this object. */

    (this->Menus)->ClearChain();
    if (ml) (this->Menus)->ChainBeforeML( ml, (long)ml);
    (this)->lpair::PostMenus( this->Menus);
}

void rastoolview::PostKeyState(class keystate  *ks)
{
/* Enable the keys for this object. */

    this->Keystate->next = NULL;
    (this->Keystate)->AddBefore( ks);
    (this)->lpair::PostKeyState( this->Keystate);
}

boolean rastoolview::WantSelectionHighlighted()
{
    rastoolview_toolfptr tpr;

    tpr = toollayout[this->toolnum].proc;

    return (tpr==(rastoolview_toolfptr)Tool_Paste || tpr==(rastoolview_toolfptr)Tool_Text);
}

boolean rastoolview::SetPrimaryView(class rasterview  *zrview)
{
    if (this->primaryview) {
	(this)->RemoveObserver( this->primaryview);
	(this->primaryview)->RemoveObserver( this);
    }

    if (!zrview) { /* odd, but somebody might do it */
	this->primaryview = NULL;
	this->primaryobj = NULL;
	return TRUE;
    }

    this->primaryview = zrview;
    this->primaryobj = (zrview)->GetDataObject();
    (this)->AddObserver( zrview);
    (zrview)->AddObserver( this);

    /* ### figure out tool to highlight */
    {
	int modenum = (zrview)->GetMode();
	switch (modenum) {
	    case RegionSelectMode:
		SetToolNum(this, RASTOOL_SELECT);
		break;
	    case TouchUpMode:
		SetToolNum(this, RASTOOL_TOUCHUP);
		break;
	    case PanMode:
		SetToolNum(this, RASTOOL_PAN);
		break;
	    default:
		SetToolNum(this, RASTOOL_PAN);
		break;
	}
    }

    return TRUE;
}

static void SetToolNum(class rastoolview  *self, int  toolnum)
{
    if (toolnum==0)
	toolnum = RASTOOL_PAN;
    if (self->toolnum != toolnum) {
	self->unpaste = FALSE;
	RepostMenus(self);
    }
    self->toolnum = toolnum;
    self->toolproc = toollayout[toolnum].proc;
    (self->tooltbl)->ClearBits();
    (self->tooltbl)->SetBitOfEntry( self->toolacc[self->toolnum], TRUE);
}

void rastoolview::ObservedChanged(class observable  *observed, long  status)
{
    if (observed == (class observable *)this->primaryview) {
	if (status==observable_OBJECTDESTROYED) {
	    this->primaryview = NULL;
	}
	else {
	    int modenum = (this->primaryview)->GetMode();
	    int newtnum;
	    switch (modenum) {
		case RegionSelectMode:
		    newtnum = (RASTOOL_SELECT);
		    break;
		case TouchUpMode:
		    newtnum = (RASTOOL_TOUCHUP);
		    break;
		case PanMode:
		    newtnum = (RASTOOL_PAN);
		    break;
		default:
		    newtnum = this->toolnum;
		    if (newtnum==RASTOOL_SELECT || newtnum==RASTOOL_TOUCHUP || newtnum==RASTOOL_PAN)
			newtnum = RASTOOL_PAINT;
		    break;
	    }
	    if (newtnum != this->toolnum)
		SetToolNum(this, newtnum);
	    RepostMenus(this);
	}
    }
    else {
	if (status==observable_OBJECTDESTROYED) {
	    fprintf(stderr, "rastoolview: Primary raster dataobject destroyed.\n");
	}
    }
}

void rastoolview::UnlinkTree()
{
    (this)->lpair::UnlinkTree();
}

static void Command_Refresh(class rastoolview  *self, char  *rock)
{
    class rasterview *rself = self->primaryview;

    if (!rself) return;

    rself->needsFullUpdate = TRUE;
    (rself)->WantUpdate( rself);
}

static void Command_Copy(class rastoolview  *self, char  *rock)
{
    class rasterview *rself = self->primaryview;

    if (!rself) return;

    (rself)->CopySelection();
    /* only display message if it won't overwrite the rasterv's message */
    if (!rself->ShowCoords)
	message::DisplayString(rself, 10, "Selection copied.");
}

static void Command_ZoomIn(class rastoolview  *self, char  *rock)
{
    class rasterview *rself = self->primaryview;
    int newscale;

    if (!rself) return;

    if (Cropped(((class rasterview *)rself))) {
	message::DisplayString(self, 10, "You cannot zoom in a cropped raster.");
	return;
    }
    newscale = (rself)->GetScale();
    newscale *= 2;
    (rself)->SetScale( newscale);
}

static void Command_ZoomOut(class rastoolview  *self, char  *rock)
{
    class rasterview *rself = self->primaryview;
    int newscale;

    if (!rself) return;

    if (Cropped(((class rasterview *)rself))) {
	message::DisplayString(self, 10, "You cannot zoom in a cropped raster.");
	return;
    }
    newscale = (rself)->GetScale();
    newscale /= 2;
    if (newscale < 1)
	newscale = 1;
    (rself)->SetScale( newscale);
}

static void Command_ZoomNorm(class rastoolview  *self, char  *rock)
{
    class rasterview *rself = self->primaryview;
    int newscale;

    if (!rself) return;

    newscale = 1;
    (rself)->SetScale( newscale);
}

static void Command_Quit(class rastoolview  *self, char  *rock)
{
    if (self->primaryview) {
	(self->primaryview)->DestroyToolset();
    }
    else {
	fprintf(stderr, "rastoolview: warning: no primary rasterview.\n");
	(self)->Destroy();
    }
}


static void CallCommandProc(class stringtbl  *st, class rastoolview  *self, short  accnum)
{
    int cmdnum;
    rastoolview_procfptr comproc;

    for (cmdnum=0; cmdnum<ZRCMDS_NUM; cmdnum++) {
	if (self->commandacc[cmdnum] == accnum) break;
    }
    if (cmdnum==ZRCMDS_NUM) {
	return;
    }
    if (cmdnum==0) {
	message::DisplayString(self, 10, "Click on any of these commands.");
	return;
    }

    comproc = commandlayout[cmdnum].proc;
    if (comproc)
	(*comproc)(self, commandlayout[cmdnum].rock);
}

static void CallPasteModeProc(class stringtbl  *st, class rastoolview  *self, short  accnum)
{
    int pmdnum;

    for (pmdnum=0; pmdnum<ZRPASTEMODES_NUM; pmdnum++) {
	if (self->pastemodeacc[pmdnum] == accnum) break;
    }
    if (pmdnum==ZRPASTEMODES_NUM) {
	return;
    }
    if (pmdnum==0) {
	message::DisplayString(self, 10, "Click on a mode to select it.");
	return;
    }

    self->pastemodenum = pmdnum;
    switch (pmdnum) {
	case PASTEMODE_COPY:
	    self->pastemode = pixelimage_COPY;
	    break;
	case PASTEMODE_OR:
	    self->pastemode = pixelimage_OR;
	    break;
	case PASTEMODE_XOR:
	    self->pastemode = pixelimage_XOR;
	    break;
    }

    if (!(st)->GetBitOfEntry( accnum)) {
	(st)->ClearBits();
	(st)->SetBitOfEntry( accnum, TRUE);
    }

    PasteResplot(self, 0);
}

static void SetPatternProc(class stringtbl  *st, class rastoolview  *self, short  accnum)
{
    int patternnum;

    for (patternnum=0; patternnum<ZRPATTERNS_NUM; patternnum++) {
	if (self->patternacc[patternnum] == accnum) break;
    }
    if (patternnum==ZRPATTERNS_NUM) {
	return;
    }
    if (patternnum==0) {
	message::DisplayString(self, 10, "Click on a pattern to select it.");
	return;
    }

    self->patternnum = patternnum;
    if (patternnum==1)
	self->pattern = ZRPATTERN_INVERT;
    else
	self->pattern = patternlist[patternnum];

    if (!(st)->GetBitOfEntry( accnum)) {
	(st)->ClearBits();
	(st)->SetBitOfEntry( accnum, TRUE);
    }
}

static void SetBrushProc(class stringtbl  *st, class rastoolview  *self, short  accnum)
{
    int brushnum;

    for (brushnum=0; brushnum<ZRBRUSHES_NUM; brushnum++) {
	if (self->brushacc[brushnum] == accnum) break;
    }
    if (brushnum==ZRBRUSHES_NUM) {
	return;
    }
    if (brushnum==0) {
	message::DisplayString(self, 10, "Click on a brush to select it.");
	return;
    }

    self->brushnum = brushnum;
    if (brushnum==1)
	self->brush = ZRBRUSH_PIXEL;
    else
	self->brush = brushlist[brushnum];

    if (!(st)->GetBitOfEntry( accnum)) {
	(st)->ClearBits();
	(st)->SetBitOfEntry( accnum, TRUE);
    }
}

static void SetToolProc(class stringtbl  *st, class rastoolview  *self, short  accnum)
{
    int toolnum;
    class raster *ras = (class raster *)self->primaryobj;

    for (toolnum=0; toolnum<ZRTOOLS_NUM; toolnum++) {
	if (self->toolacc[toolnum] == accnum) break;
    }
    if (toolnum==ZRTOOLS_NUM) {
	return;
    }
    if (toolnum==0) {
	message::DisplayString(self, 10, "Click on a tool to select it; click again to set tool parameters.");
	return;
    }

    if (ras->readOnly && toollayout[toolnum].writes) {
	message::DisplayString(self, 10, "Raster is read-only.");
	return;
    }

    if (self->toolnum==toolnum) {
	rastoolview_toolmodfptr prmod;
	prmod = toollayout[toolnum].procmod;
	if (prmod) 
	    (*prmod)(self, NULL);
	return;
    }
    else {
	self->unpaste = FALSE;
	RepostMenus(self);
    }

    self->toolnum = toolnum;
    self->toolproc = toollayout[toolnum].proc;

    if (!(st)->GetBitOfEntry( accnum)) {
	(st)->ClearBits();
	(st)->SetBitOfEntry( accnum, TRUE);
    }
    (self)->NotifyObservers(0); 
}

static void DrawLine(class rasterview  *rself, long  x0 , long  y0 , long  x1 , long  y1, unsigned char *pattern, unsigned char *brush)
{
    int dx, dy, x, y, d, incr_str, incr_diag;

    dx = x1-x0;
    dy = y1-y0;
    x = x0;
    y = y0;
    /* draw initial point */
    (rself)->BrushSetPixel( x, y, pattern, brush);

    /* standard midpoint line-drawing algorithm */
    /* split into 8 cases, depending on which way the line goes */
    if (dx>=0) {
	if (dy>=0) {
	    if (dy <= dx) {
		d = 2*dy-dx;
		incr_str = 2*dy;
		incr_diag = 2*(dy-dx);
		while (x<x1) {
		    if (d<=0) {
			d += incr_str;
			x++;
		    }
		    else {
			d += incr_diag;
			x++;
			y++;
		    }
		    (rself)->BrushSetPixel( x, y, pattern, brush);
		}
	    }
	    else {
		d = 2*dx-dy;
		incr_str = 2*dx;
		incr_diag = 2*(dx-dy);
		while (y<y1) {
		    if (d<=0) {
			d += incr_str;
			y++;
		    }
		    else {
			d += incr_diag;
			y++;
			x++;
		    }
		    (rself)->BrushSetPixel( x, y, pattern, brush);
		}
	    }
	}
	else {
	    if (-dy <= dx) {
		d = -2*dy-dx;
		incr_str = -2*dy;
		incr_diag = 2*(-dy-dx);
		while (x<x1) {
		    if (d<=0) {
			d += incr_str;
			x++;
		    }
		    else {
			d += incr_diag;
			x++;
			y--;
		    }
		    (rself)->BrushSetPixel( x, y, pattern, brush);
		}
	    }
	    else {
		d = 2*dx+dy;
		incr_str = 2*dx;
		incr_diag = 2*(dx+dy);
		while (y>y1) {
		    if (d<=0) {
			d += incr_str;
			y--;
		    }
		    else {
			d += incr_diag;
			y--;
			x++;
		    }
		    (rself)->BrushSetPixel( x, y, pattern, brush);
		}
	    }
	}
    }
    else {
	if (dy>=0) {
	    if (dy <= -dx) {
		d = 2*dy+dx;
		incr_str = 2*dy;
		incr_diag = 2*(dy+dx);
		while (x>x1) {
		    if (d<=0) {
			d += incr_str;
			x--;
		    }
		    else {
			d += incr_diag;
			x--;
			y++;
		    }
		    (rself)->BrushSetPixel( x, y, pattern, brush);
		}
	    }
	    else {
		d = -2*dx-dy;
		incr_str = -2*dx;
		incr_diag = 2*(-dx-dy);
		while (y<y1) {
		    if (d<=0) {
			d += incr_str;
			y++;
		    }
		    else {
			d += incr_diag;
			y++;
			x--;
		    }
		    (rself)->BrushSetPixel( x, y, pattern, brush);
		}
	    }
	}
	else {
	    if (-dy <= -dx) {
		d = -2*dy+dx;
		incr_str = -2*dy;
		incr_diag = 2*(-dy+dx);
		while (x>x1) {
		    if (d<=0) {
			d += incr_str;
			x--;
		    }
		    else {
			d += incr_diag;
			x--;
			y--;
		    }
		    (rself)->BrushSetPixel( x, y, pattern, brush);
		}
	    }
	    else {
		d = -2*dx+dy;
		incr_str = -2*dx;
		incr_diag = 2*(-dx+dy);
		while (y>y1) {
		    if (d<=0) {
			d += incr_str;
			y--;
		    }
		    else {
			d += incr_diag;
			y--;
			x--;
		    }
		    (rself)->BrushSetPixel( x, y, pattern, brush);
		}
	    }
	}
    }
}

/* draw a circle with midpoint algorithm */
static void DrawCircle(class rasterview  *rself, long  x0 , long  y0 , long  rad, unsigned char *pattern , unsigned char *brush)
{
    int d, x, y;

    x = 0;
    y = rad;
    d = 1-rad;
    (rself)->BrushSetPixel( x0, y0-y, pattern, brush);
    (rself)->BrushSetPixel( x0, y0+y, pattern, brush);
    (rself)->BrushSetPixel( x0-y, y0, pattern, brush);
    (rself)->BrushSetPixel( x0+y, y0, pattern, brush);
    if (d<0) {
	d += 2*x+3;
	x++;
    }
    else {
	d += 2*(x-y)+5;
	x++;
	y--;
    }
    while (y>x) {
	(rself)->BrushSetPixel( x0-x, y0+y, pattern, brush);
	(rself)->BrushSetPixel( x0-x, y0-y, pattern, brush);
	(rself)->BrushSetPixel( x0+x, y0-y, pattern, brush);
	(rself)->BrushSetPixel( x0+x, y0+y, pattern, brush);
	(rself)->BrushSetPixel( x0-y, y0+x, pattern, brush);
	(rself)->BrushSetPixel( x0-y, y0-x, pattern, brush);
	(rself)->BrushSetPixel( x0+y, y0-x, pattern, brush);
	(rself)->BrushSetPixel( x0+y, y0+x, pattern, brush);
	if (d<0) {
	    d += 2*x+3;
	    x++;
	}
	else {
	    d += 2*(x-y)+5;
	    x++;
	    y--;
	}
    }
    if (x==y) {
	(rself)->BrushSetPixel( x0-x, y0+y, pattern, brush);
	(rself)->BrushSetPixel( x0-x, y0-y, pattern, brush);
	(rself)->BrushSetPixel( x0+x, y0-y, pattern, brush);
	(rself)->BrushSetPixel( x0+x, y0+y, pattern, brush);
    }
}

/* draw an ellipse with midpoint algorithm */
static void DrawEllipse(class rasterview  *rself, long  x0 , long  y0, long  xrad , long  yrad /* both must be >= 0 */, unsigned char *pattern , unsigned char *brush)
{
    int d, x, y;

    x = 0;
    y = yrad;
    d = 4*yrad*yrad - xrad*xrad*yrad + xrad*xrad;
    (rself)->BrushSetPixel( x0, y0-y, pattern, brush);
    (rself)->BrushSetPixel( x0, y0+y, pattern, brush);

    while (xrad*xrad*(2*y-1) > 2*yrad*yrad*(x+1)) {
	if (d<0) {
	    d += 4*yrad*yrad*(2*x+3);
	    x++;
	}
	else {
	    d += 4*(yrad*yrad*(2*x+3) - xrad*xrad*(2*y-2));
	    x++;
	    y--;
	}
	(rself)->BrushSetPixel( x0-x, y0+y, pattern, brush);
	(rself)->BrushSetPixel( x0-x, y0-y, pattern, brush);
	(rself)->BrushSetPixel( x0+x, y0-y, pattern, brush);
	(rself)->BrushSetPixel( x0+x, y0+y, pattern, brush);
    }

    d = yrad*yrad*(2*x+1)*(2*x+1) + 4*xrad*xrad*(y-1)*(y-1) - 4*xrad*xrad*yrad*yrad;
    while (y > 1) {
	if (d>=0) {
	    d -= 4*xrad*xrad*(2*y-3);
	    y--;
	}
	else {
	    d += 4*(yrad*yrad*(2*x+2) - xrad*xrad*(2*y-3));
	    x++;
	    y--;
	}
	(rself)->BrushSetPixel( x0-x, y0+y, pattern, brush);
	(rself)->BrushSetPixel( x0-x, y0-y, pattern, brush);
	(rself)->BrushSetPixel( x0+x, y0-y, pattern, brush);
	(rself)->BrushSetPixel( x0+x, y0+y, pattern, brush);
    }
    if (d>=0) {
    }
    else {
	x++;
    }
    (rself)->BrushSetPixel( x0-x, y0, pattern, brush);
    (rself)->BrushSetPixel( x0+x, y0, pattern, brush);
}

static void FillRectangle(class rastoolview  *self, long  x0 , long  y0, long  wid , long  hgt, unsigned char *pattern)
{
    class rasterimage *pix = ((class raster *)self->primaryobj)->GetPix();
    int ix, iy;
    long tbit;

    if (wid<0) {
	x0 += wid;
	wid = (-wid);
    }

    if (hgt<0) {
	y0 += hgt;
	hgt = (-hgt);
    }

    for (iy=y0; iy<=y0+hgt; iy++) 
	for (ix=x0; ix<=x0+wid; ix++) {
	    if (self->pattern==ZRPATTERN_INVERT)
		tbit = !(pix)->GetPixel( ix, iy);
	    else
		tbit = (self->pattern[iy&7]>>(ix&7)) & 1;
	    (pix)->SetPixel( ix, iy, tbit);
	}
}

static void FillCircle(class rastoolview  *self, long  x0 , long  y0, long  rad, unsigned char *pattern)
{
    class rasterimage *pix = ((class raster *)self->primaryobj)->GetPix();
    int ix, iy;
    long tbit;
    int cosv, sinv;

    for (sinv=(-rad); sinv<=rad; sinv++) {
	cosv = (int)sqrt((double)(rad*rad-sinv*sinv));
	iy = y0+sinv;
	for (ix=x0-cosv; ix<=x0+cosv; ix++) {
	    if (self->pattern==ZRPATTERN_INVERT)
		tbit = !(pix)->GetPixel( ix, iy);
	    else
		tbit = (self->pattern[iy&7]>>(ix&7)) & 1;
	    (pix)->SetPixel( ix, iy, tbit);
	}
    }
}

static void FillEllipse(class rastoolview  *self, long  x0 , long  y0, long  xrad , long  yrad /* both must be >= 0 */, unsigned char *pattern)
{
    class rasterimage *pix = ((class raster *)self->primaryobj)->GetPix();
    int ix, iy;
    long tbit;
    double eccen;
    int cosv, sinv;

    if (yrad==0) yrad=1;
    eccen = (double)xrad / (double)yrad;

    for (sinv=(-yrad); sinv<=yrad; sinv++) {
	cosv = (int)(0.5 + eccen * sqrt((double)(yrad*yrad-sinv*sinv)));
	iy = y0+sinv;
	for (ix=x0-cosv; ix<=x0+cosv; ix++) {
	    if (self->pattern==ZRPATTERN_INVERT)
		tbit = !(pix)->GetPixel( ix, iy);
	    else
		tbit = (self->pattern[iy&7]>>(ix&7)) & 1;
	    (pix)->SetPixel( ix, iy, tbit);
	}
    }
}

static void DrawRectangle(class rasterview  *rself, long  x0 , long  y0, long  wid , long  hgt, unsigned char *pattern , unsigned char *brush)
{
    if (wid<0) {
	x0 += wid;
	wid = (-wid);
    }

    if (hgt<0) {
	y0 += hgt;
	hgt = (-hgt);
    }

    if (wid)
	DrawLine(rself, x0, y0, x0+wid-1, y0, pattern, brush);
    if (hgt)
	DrawLine(rself, x0+wid, y0, x0+wid, y0+hgt-1, pattern, brush);
    if (wid)
	DrawLine(rself, x0+wid, y0+hgt, x0+1, y0+hgt, pattern, brush);
    if (hgt)
	DrawLine(rself, x0, y0+hgt, x0, y0+1, pattern, brush);
}

static void Tool_SolidRect(class rastoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks)
{
    class rasterimage *pix = ((class raster *)self->primaryobj)->GetPix();

    switch (action) {
	case view_LeftDown:
	    self->rockx = x;
	    self->rocky = y;
	    self->lastx = 0;
	    self->lasty = 0;
	    DrawRectangle(self->primaryview, self->rockx, self->rocky, 0, 0, ZRPATTERN_INVERT, ZRBRUSH_PIXEL);
	    break;
	case view_LeftMovement:
	    DrawRectangle(self->primaryview, self->rockx, self->rocky, self->lastx, self->lasty, ZRPATTERN_INVERT, ZRBRUSH_PIXEL);
	    self->lastx = x - self->rockx;
	    self->lasty = y - self->rocky;
	    DrawRectangle(self->primaryview, self->rockx, self->rocky, self->lastx, self->lasty, ZRPATTERN_INVERT, ZRBRUSH_PIXEL);
	    break;
	case view_LeftUp: 
	    DrawRectangle(self->primaryview, self->rockx, self->rocky, self->lastx, self->lasty, ZRPATTERN_INVERT, ZRBRUSH_PIXEL);
	    self->lastx = x - self->rockx;
	    self->lasty = y - self->rocky;
	    FillRectangle(self, self->rockx, self->rocky, self->lastx, self->lasty, self->pattern);
	    (pix)->NotifyObservers( raster_BITSCHANGED);
	    break;
	default:
	    break;
    }
}

static void Tool_SolidCircle(class rastoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks)
{
    int rad;
    class rasterimage *pix = ((class raster *)self->primaryobj)->GetPix();

    switch (action) {
	case view_LeftDown:
	    self->rockx = x;
	    self->rocky = y;
	    self->lastx = 0; /* 0 radius */
	    DrawCircle(self->primaryview, self->rockx, self->rocky, 0, ZRPATTERN_INVERT, ZRBRUSH_PIXEL);
	    break;
	case view_LeftMovement:
	    rad = ((y-self->rocky)*(y-self->rocky)+(x-self->rockx)*(x-self->rockx));
	    rad = (int)(sqrt((double)rad));
	    if (rad == self->lastx)
		break;
	    DrawCircle(self->primaryview, self->rockx, self->rocky, self->lastx, ZRPATTERN_INVERT, ZRBRUSH_PIXEL);
	    DrawCircle(self->primaryview, self->rockx, self->rocky, rad, ZRPATTERN_INVERT, ZRBRUSH_PIXEL);
	    self->lastx = rad;
	    break;
	case view_LeftUp: 
	    rad = ((y-self->rocky)*(y-self->rocky)+(x-self->rockx)*(x-self->rockx));
	    rad = (int)(sqrt((double)rad));
	    DrawCircle(self->primaryview, self->rockx, self->rocky, self->lastx, ZRPATTERN_INVERT, ZRBRUSH_PIXEL);
	    FillCircle(self, self->rockx, self->rocky, rad, self->pattern);
	    (pix)->NotifyObservers( raster_BITSCHANGED);
	    break;
	default:
	    break;
    }
}

static void Tool_SolidEllipse(class rastoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks)
{
    int xrad, yrad;
    class rasterimage *pix = ((class raster *)self->primaryobj)->GetPix();

    switch (action) {
	case view_LeftDown:
	    self->rockx = x;
	    self->rocky = y;
	    self->lastx = 0; /* 0 radius */
	    self->lasty = 0; /* 0 radius */
	    DrawEllipse(self->primaryview, self->rockx, self->rocky, 0, 0, ZRPATTERN_INVERT, ZRBRUSH_PIXEL);
	    break;
	case view_LeftMovement:
	    xrad = x-self->rockx;
	    yrad = y-self->rocky;
	    if (xrad<0) xrad = (-xrad);
	    if (yrad<0) yrad = (-yrad);
	    DrawEllipse(self->primaryview, self->rockx, self->rocky, self->lastx, self->lasty, ZRPATTERN_INVERT, ZRBRUSH_PIXEL);
	    DrawEllipse(self->primaryview, self->rockx, self->rocky, xrad, yrad, ZRPATTERN_INVERT, ZRBRUSH_PIXEL);
	    self->lastx = xrad;
	    self->lasty = yrad;
	    break;
	case view_LeftUp: 
	    xrad = x-self->rockx;
	    yrad = y-self->rocky;
	    if (xrad<0) xrad = (-xrad);
	    if (yrad<0) yrad = (-yrad);
	    DrawEllipse(self->primaryview, self->rockx, self->rocky, self->lastx, self->lasty, ZRPATTERN_INVERT, ZRBRUSH_PIXEL);
	    FillEllipse(self, self->rockx, self->rocky, xrad, yrad, self->pattern);
	    (pix)->NotifyObservers( raster_BITSCHANGED);
	    break;
	default:
	    break;
    }
}

static void Tool_Paint(class rastoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks)
{
    class rasterimage *pix = ((class raster *)self->primaryobj)->GetPix();

    switch (action) {
	case view_LeftDown:
	    self->rockx = x;
	    self->rocky = y;
	    DrawLine(self->primaryview, self->rockx, self->rocky, x, y, self->pattern, self->brush);
	    (pix)->NotifyObservers( raster_BITSCHANGED);
	    break;
	case view_LeftMovement:
	    DrawLine(self->primaryview, self->rockx, self->rocky, x, y, self->pattern, self->brush);
	    self->rockx = x;
	    self->rocky = y;
	    (pix)->NotifyObservers( raster_BITSCHANGED);
	    break;
	case view_LeftUp: 
	    break;
	default:
	    break;
    }
}

static void Tool_Line(class rastoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks)
{
    class rasterimage *pix;

    switch (action) {
	case view_LeftDown:
	    self->rockx = x;
	    self->rocky = y;
	    DrawLine(self->primaryview, self->rockx, self->rocky, x, y, ZRPATTERN_INVERT, ZRBRUSH_PIXEL);
	    self->lastx = x;
	    self->lasty = y;
	    break;
	case view_LeftMovement:
	    DrawLine(self->primaryview, self->rockx, self->rocky, self->lastx, self->lasty, ZRPATTERN_INVERT, ZRBRUSH_PIXEL);
	    DrawLine(self->primaryview, self->rockx, self->rocky, x, y, ZRPATTERN_INVERT, ZRBRUSH_PIXEL);
	    self->lastx = x;
	    self->lasty = y;
	    break;
	case view_LeftUp: 
	    DrawLine(self->primaryview, self->rockx, self->rocky, self->lastx, self->lasty, ZRPATTERN_INVERT, ZRBRUSH_PIXEL);
	    DrawLine(self->primaryview, self->rockx, self->rocky, x, y, self->pattern, self->brush);
	    pix = ((class raster *)self->primaryobj)->GetPix();
	    (pix)->NotifyObservers( raster_BITSCHANGED);
	    break;
	default:
	    break;
    }
}

static void Tool_Circle(class rastoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks)
{
    class rasterimage *pix;
    int rad;

    switch (action) {
	case view_LeftDown:
	    self->rockx = x;
	    self->rocky = y;
	    self->lastx = 0; /* 0 radius */
	    DrawCircle(self->primaryview, self->rockx, self->rocky, 0, ZRPATTERN_INVERT, ZRBRUSH_PIXEL);
	    break;
	case view_LeftMovement:
	    rad = ((y-self->rocky)*(y-self->rocky)+(x-self->rockx)*(x-self->rockx));
	    rad = (int)(sqrt((double)rad));
	    if (rad == self->lastx)
		break;
	    DrawCircle(self->primaryview, self->rockx, self->rocky, self->lastx, ZRPATTERN_INVERT, ZRBRUSH_PIXEL);
	    DrawCircle(self->primaryview, self->rockx, self->rocky, rad, ZRPATTERN_INVERT, ZRBRUSH_PIXEL);
	    self->lastx = rad;
	    break;
	case view_LeftUp: 
	    rad = ((y-self->rocky)*(y-self->rocky)+(x-self->rockx)*(x-self->rockx));
	    rad = (int)(sqrt((double)rad));
	    DrawCircle(self->primaryview, self->rockx, self->rocky, self->lastx, ZRPATTERN_INVERT, ZRBRUSH_PIXEL);
	    DrawCircle(self->primaryview, self->rockx, self->rocky, rad, self->pattern, self->brush);
	    pix = ((class raster *)self->primaryobj)->GetPix();
	    (pix)->NotifyObservers( raster_BITSCHANGED);
	    break;
	default:
	    break;
    }
}

static void Tool_Ellipse(class rastoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks)
{
    class rasterimage *pix;
    long xrad, yrad;

    switch (action) {
	case view_LeftDown:
	    self->rockx = x;
	    self->rocky = y;
	    self->lastx = 0;
	    self->lasty = 0;
	    DrawEllipse(self->primaryview, self->rockx, self->rocky, 0, 0, ZRPATTERN_INVERT, ZRBRUSH_PIXEL);
	    break;
	case view_LeftMovement:
	    xrad = x-self->rockx;
	    yrad = y-self->rocky;
	    if (xrad<0) xrad = (-xrad);
	    if (yrad<0) yrad = (-yrad);
	    if (xrad == self->lastx && yrad == self->lasty)
		break;
	    DrawEllipse(self->primaryview, self->rockx, self->rocky, self->lastx, self->lasty, ZRPATTERN_INVERT, ZRBRUSH_PIXEL);
	    DrawEllipse(self->primaryview, self->rockx, self->rocky, xrad, yrad, ZRPATTERN_INVERT, ZRBRUSH_PIXEL);
	    self->lastx = xrad;
	    self->lasty = yrad;
	    break;
	case view_LeftUp: 
	    xrad = x-self->rockx;
	    yrad = y-self->rocky;
	    if (xrad<0) xrad = (-xrad);
	    if (yrad<0) yrad = (-yrad);
	    DrawEllipse(self->primaryview, self->rockx, self->rocky, self->lastx, self->lasty, ZRPATTERN_INVERT, ZRBRUSH_PIXEL);
	    DrawEllipse(self->primaryview, self->rockx, self->rocky, xrad, yrad, self->pattern, self->brush);
	    pix = ((class raster *)self->primaryobj)->GetPix();
	    (pix)->NotifyObservers( raster_BITSCHANGED);
	    break;
	default:
	    break;
    }
}

static void Tool_Rectangle(class rastoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks)
{
    class rasterimage *pix;

    switch (action) {
	case view_LeftDown:
	    self->rockx = x;
	    self->rocky = y;
	    self->lastx = 0;
	    self->lasty = 0;
	    DrawRectangle(self->primaryview, self->rockx, self->rocky, 0, 0, ZRPATTERN_INVERT, ZRBRUSH_PIXEL);
	    break;
	case view_LeftMovement:
	    DrawRectangle(self->primaryview, self->rockx, self->rocky, self->lastx, self->lasty, ZRPATTERN_INVERT, ZRBRUSH_PIXEL);
	    self->lastx = x - self->rockx;
	    self->lasty = y - self->rocky;
	    DrawRectangle(self->primaryview, self->rockx, self->rocky, self->lastx, self->lasty, ZRPATTERN_INVERT, ZRBRUSH_PIXEL);
	    break;
	case view_LeftUp: 
	    DrawRectangle(self->primaryview, self->rockx, self->rocky, self->lastx, self->lasty, ZRPATTERN_INVERT, ZRBRUSH_PIXEL);
	    self->lastx = x - self->rockx;
	    self->lasty = y - self->rocky;
	    DrawRectangle(self->primaryview, self->rockx, self->rocky, self->lastx, self->lasty, self->pattern, self->brush);
	    pix = ((class raster *)self->primaryobj)->GetPix();
	    (pix)->NotifyObservers( raster_BITSCHANGED);
	    break;
	default:
	    break;
    }
}

static void Tool_Text(class rastoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks)
{
    struct rectangle *VS, *sel;
    long w, h;

    VS = &self->primaryview->ViewSelection;
    sel = &(self->primaryview->DesiredSelection);

    if (x < VS->left) x = VS->left;
    if (y < VS->top) y = VS->top;
    if (x >= VS->left+VS->width) x = VS->left+VS->width;
    if (y >= VS->top+VS->height) y = VS->top+VS->height;

    switch (action) {
	case view_LeftDown:
	    self->rockx = x;
	    self->rocky = y;
	    self->lastx = 1;
	    self->lasty = 1;
	    rectangle_SetRectSize(sel, x, y, 1, 1);
	    (self->primaryview)->SetDesiredSelection( sel);
	    break;
	case view_LeftMovement:
	    if (x >= self->rockx) {
		w = x-self->rockx;
		x = self->rockx;
	    }
	    else {
		w = self->rockx-x;
	    }
	    if (y >= self->rocky) {
		h = y-self->rocky;
		y = self->rocky;
	    }
	    else {
		h = self->rocky-y;
	    }
	    self->lastx = x;
	    self->lasty = y;
	    rectangle_SetRectSize(sel, x, y, w, h);
	    (self->primaryview)->SetDesiredSelection( sel);
	    break;
	case view_LeftUp: 
	    if (x >= self->rockx) {
		w = x-self->rockx;
		x = self->rockx;
	    }
	    else {
		w = self->rockx-x;
	    }
	    if (y >= self->rocky) {
		h = y-self->rocky;
		y = self->rocky;
	    }
	    else {
		h = self->rocky-y;
	    }
	    if (w<8 && h<8) {
		h = 40;
		w = VS->width - x;
	    }
	    rectangle_SetRectSize(sel, x, y, w, h);
	    (self->primaryview)->SetDesiredSelection( sel);
	    if ((self->primaryview)->GetOverlaidInset()) {
		/* resize inset if there is one */
		(self->primaryview)->ResizeInset();
		self->primaryview->needsFullUpdate=FALSE;
		(self->primaryview)->WantUpdate( self); 
	    }
	    else {
		/* otherwise create inset */
		(self->primaryview)->OverlayInset( "text");
	    }
	    RepostMenus(self);
	    break;
	default:
	    break;
    }
}

static class raster *GetPasted(class rastoolview  *self)
{
    class raster *ras = self->pasteraster;
    FILE *pasteFile;
    static const char hdr[] = "\\begindata{raster,";
    const char *hx = hdr;
    int c;

    /* it would be nice if there was some way to determine whether the cut buffer contents hadn't changed since the last paste operation; if so, we could just return self->pasteraster immediately. */

    /* we snarf the rasterview's IM rather than our own; it might be more efficient. */
    pasteFile = ((self->primaryview)->GetIM())->FromCutBuffer();
    if (!pasteFile)
	return NULL;

    /* if the file does not begin with a raster, 
	we may as well scan until we find one */

    while ((c = getc(pasteFile)) != EOF && *hx) {
	if (c == *hx) hx++;
	else hx = (c == *hdr) ? hdr+1 : hdr;
    }

    if (*hx) {
	ras = NULL;
    }
    else {
	while ((c=getc(pasteFile)) != '\n' && c != EOF) ;
	/* skip to end of header */

	(ras)->Read( pasteFile, 1);
    }
    ((self)->GetIM())->CloseFromCutBuffer( pasteFile);

    return ras;
}

static void Tool_Paste(class rastoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks)
{
    class rasterimage *pix, *pix2, *pix3;
    struct point pt;
    struct rectangle *sel, *VS, R, CR;
    class raster *pras, *ras;
    long vx, vy, vwid, vhgt;

    sel = &(self->primaryview->DesiredSelection);
    ras = (class raster *)self->primaryobj;
    pix = (ras)->GetPix();

    switch (action) {
	case view_LeftDown:
	case view_RightDown:
	    pras = GetPasted(self);
	    if (!pras) {
		message::DisplayString(self->primaryview, 0, "No raster found in cut buffer.");
		self->rock = 0;
		return;
	    }
	    VS = &self->primaryview->ViewSelection;
	    vx = rectangle_Left(VS);
	    vy = rectangle_Top(VS);
	    vwid = rectangle_Width(VS);
	    vhgt = rectangle_Height(VS);
	    self->rockw = (self->pasteraster)->GetWidth();
	    self->rockh = (self->pasteraster)->GetHeight();

	    if (self->rockw > vwid || self->rockh > vhgt) {
		message::DisplayString(self->primaryview, 0, "The selection is too large to paste into this raster.");
		self->rock = 0;
		return;
	    }

	    self->rock = 1;

	    /* we now want to find out if the paste raster is the same as the rasterview's selected region. Since there's no good way to do this, we just check if they're the same size. */
	    if (self->rockw == rectangle_Width(sel) && self->rockh == rectangle_Height(sel)) {
		/* selection is probably the paste raster */
		/* leave selection alone */
	    }
	    else {
		rectangle_EmptyRect(sel);
		self->unpaste = FALSE;
		/* clear selection, don't allow unpaste dragging */
	    }

	    point_SetPt(&pt, x, y);
	    if (rectangle_IsPtInRect(&pt, sel)) {
		self->rockx = x - rectangle_Left(sel);
		self->rocky = y - rectangle_Top(sel);
		x = rectangle_Left(sel);
		y = rectangle_Top(sel);
	    }
	    else {
		self->rockx = 0;
		self->rocky = 0;
		if (x<vx) x = vx;
		if (y<vy) y = vy;
		if (x > vx+vwid-self->rockw) x = vx+vwid-self->rockw;
		if (y > vy+vhgt-self->rockh) y = vy+vhgt-self->rockh;
	    }

	    if (!self->unpaste) {
		(self->unpasteraster)->Resize( self->rockw, self->rockh); 
		rectangle_EmptyRect(&CR);
	    }
	    else {
		/* slap unpasteraster on screen */
		pix3 = (self->unpasteraster)->GetPix();
		rectangle_SetRectSize(&R, 0, 0, self->rockw, self->rockh);
		(pix)->BlitSubraster( self->lastx, self->lasty, pix3, &R, pixelimage_COPY);
		rectangle_SetRectSize(&CR, self->lastx, self->lasty, self->rockw, self->rockh);
	    }

	    /* yank current screen contents to unpasteraster */
	    pix3 = (self->unpasteraster)->GetPix();
	    rectangle_SetRectSize(&R, x, y, self->rockw, self->rockh);
	    (pix3)->BlitSubraster( 0, 0, pix, &R, pixelimage_COPY);

	    /* slap pasteraster onto screen */
	    pix2 = (self->pasteraster)->GetPix();
	    rectangle_SetRectSize(&R, 0, 0, self->rockw, self->rockh);
	    (pix)->BlitSubraster( x, y, pix2, &R, self->pastemode);

	    (pix)->NotifyObservers( raster_BITSCHANGED);

	    self->lastx = x;
	    self->lasty = y;

	    rectangle_EmptyRect(sel);
	    (self->primaryview)->SetDesiredSelection( sel);
	    /* maybe ought to call RepostMenus(self) here, but the user isn't too likely to call for a menu while dragging the mouse in another window */
	    break;

	case view_LeftMovement:
	case view_RightMovement:
	    if (!self->rock) return;
	    x -= self->rockx;
	    y -= self->rocky;
	    VS = &self->primaryview->ViewSelection;
	    vx = rectangle_Left(VS);
	    vy = rectangle_Top(VS);
	    vwid = rectangle_Width(VS);
	    vhgt = rectangle_Height(VS);
	    if (x<vx) x = vx;
	    if (y<vy) y = vy;
	    if (x > vx+vwid-self->rockw) x = vx+vwid-self->rockw;
	    if (y > vy+vhgt-self->rockh) y = vy+vhgt-self->rockh;
	    if (x != self->lastx || y != self->lasty) {

		/* slap unpasteraster on screen */
		pix3 = (self->unpasteraster)->GetPix();
		rectangle_SetRectSize(&R, 0, 0, self->rockw, self->rockh);
		(pix)->BlitSubraster( self->lastx, self->lasty, pix3, &R, pixelimage_COPY);
		rectangle_SetRectSize(&CR, self->lastx, self->lasty, self->rockw, self->rockh);

		/* yank current screen contents to unpasteraster */
		rectangle_SetRectSize(&R, x, y, self->rockw, self->rockh);
		(pix3)->BlitSubraster( 0, 0, pix, &R, pixelimage_COPY);

		/* slap pasteraster onto screen */
		pix2 = (self->pasteraster)->GetPix();
		rectangle_SetRectSize(&R, 0, 0, self->rockw, self->rockh);
		(pix)->BlitSubraster( x, y, pix2, &R, self->pastemode);

		(pix)->NotifyObservers( raster_BITSCHANGED);

		self->lastx = x;
		self->lasty = y;
	    }
	    break;
	case view_LeftUp: 
	case view_RightUp: 
	    if (!self->rock) return;
	    x -= self->rockx;
	    y -= self->rocky;
	    VS = &self->primaryview->ViewSelection;
	    vx = rectangle_Left(VS);
	    vy = rectangle_Top(VS);
	    vwid = rectangle_Width(VS);
	    vhgt = rectangle_Height(VS);
	    if (x<vx) x = vx;
	    if (y<vy) y = vy;
	    if (x > vx+vwid-self->rockw) x = vx+vwid-self->rockw;
	    if (y > vy+vhgt-self->rockh) y = vy+vhgt-self->rockh;
	    if (x != self->lastx || y != self->lasty) {

		/* slap unpasteraster on screen */
		pix3 = (self->unpasteraster)->GetPix();
		rectangle_SetRectSize(&R, 0, 0, self->rockw, self->rockh);
		(pix)->BlitSubraster( self->lastx, self->lasty, pix3, &R, pixelimage_COPY);
		rectangle_SetRectSize(&CR, self->lastx, self->lasty, self->rockw, self->rockh);
		
		/* yank current screen contents to unpasteraster */
		rectangle_SetRectSize(&R, x, y, self->rockw, self->rockh);
		(pix3)->BlitSubraster( 0, 0, pix, &R, pixelimage_COPY);

		/* slap pasteraster onto screen */
		pix2 = (self->pasteraster)->GetPix();
		rectangle_SetRectSize(&R, 0, 0, self->rockw, self->rockh);
		(pix)->BlitSubraster( x, y, pix2, &R, self->pastemode);

		(pix)->NotifyObservers( raster_BITSCHANGED);
	    }

	    if (action==view_LeftUp)
		self->unpaste = TRUE;
	    else
		self->unpaste = FALSE;

	    /* replace highlight and redo scrollbars */
	    rectangle_SetRectSize(sel, x, y, self->rockw, self->rockh);
	    (self->primaryview)->SetDesiredSelection( sel);
	    self->primaryview->needsFullUpdate=FALSE;
	    (self->primaryview)->WantUpdate( self); 

	    RepostMenus(self);
	    break;
	default:
	    break;
    }
}

static void Tool_FloodFill(class rastoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks)
{
    int bit;
    class rasterimage *pix = ((class raster *)self->primaryobj)->GetPix();
    long wid = (pix)->GetWidth();
    long hgt = (pix)->GetHeight();
    long xp, yp;
    struct span *tmp;
    struct rectangle R;

    if (action!=view_LeftDown && action!=view_RightDown)
	return;

    if (x<0 || y<0 || x>=wid || y>=hgt) {
	return;
    }

    if (self->fillstack) {
	message::DisplayString(self->primaryview, 10, "There is already a fill operation running.");
	return;
    }

    self->fillpix = rasterimage::Create(wid, hgt);
    rectangle_SetRectSize(&R, 0, 0, wid, hgt);
    (self->fillpix)->BlitSubraster( 0, 0, pix, &R, pixelimage_COPY); 
    bit = (pix)->GetPixel( x, y); 
    self->color = (bit ? 0 : 1);

    yp = y;
    for (xp=x; xp>=0 && (pix)->GetPixel( xp, yp)==bit; xp--);
    xp++;

    tmp = (struct span *)malloc(sizeof(struct span));
    tmp->x = xp;
    tmp->y = yp;
    tmp->next = NULL;
    self->fillstack = tmp;
    self->fillbit = bit;

    message::DisplayString(self->primaryview, 10, "Filling....");
    im::EnqueueEvent((event_fptr)FloodSplot, (char *)self, (event_MSECtoTU(10)));
}  

static void FloodSplot(class rastoolview  *self)
{
    class rasterimage *pix = ((class raster *)self->primaryobj)->GetPix();
    class rasterimage *fillpix = self->fillpix;
    long wid = (pix)->GetWidth();
    long hgt = (pix)->GetHeight();
    long xp, yp, startu = 0, startd = 0;
    long xmax, ymin, ymax;
    int wallu = 0, walld = 0;
    struct span *tmp;
    int bit;
    int numpoints = 0;

    ymin = hgt;
    xmax = -1;
    ymax = -1;

    bit = self->fillbit;

    if (!self->fillstack) {
	(pix)->NotifyObservers( raster_BITSCHANGED);
	message::DisplayString(self->primaryview, 10, "Filled.");
	if (fillpix) {
	    (fillpix)->Destroy();
	    self->fillpix = NULL;
	}
	return;
    }

    while (numpoints<100) {
	if (self->fillstack == NULL)
	    break;

	xp = self->fillstack->x;
	yp = self->fillstack->y;
	tmp = self->fillstack;
	self->fillstack = self->fillstack->next;
	free(tmp);

	if (yp>0) {
	    if (bit!=(fillpix)->GetPixel( xp, yp-1))
		wallu = 1;
	    else {
		for (startu=xp; startu>=0 && (fillpix)->GetPixel( startu, yp-1)==bit; startu--);
		startu++;
		wallu = 0;
	    }
	}
	if (yp<hgt-1) {
	    if (bit!=(fillpix)->GetPixel( xp, yp+1))
		walld = 1;
	    else {
		for (startd=xp; startd>=0 && (fillpix)->GetPixel( startd, yp+1)==bit; startd--);
		startd++;
		walld = 0;
	    }
	}
	if (yp>ymax) ymax = yp;
	if (yp<ymin) ymin = yp;
	for (; xp<wid && (fillpix)->GetPixel( xp, yp)==bit; xp++) {
	    int tbit;
	    if (self->pattern==ZRPATTERN_INVERT)
		tbit = self->color;
	    else
		tbit = (self->pattern[yp&7]>>(xp&7)) & 1;
	    (fillpix)->SetPixel( xp, yp, self->color);
	    (pix)->SetPixel( xp, yp, tbit);
	    numpoints++;
	    if (yp>0) {
		if (wallu==0) {
		    if (bit!=(fillpix)->GetPixel( xp, yp-1)) {
			tmp = (struct span *)malloc(sizeof(struct span));
			tmp->x = startu;
			tmp->y = yp-1;
			tmp->next = self->fillstack;
			self->fillstack = tmp;
			wallu = 1;
		    }
		}
		else {
		    if (bit==(fillpix)->GetPixel( xp, yp-1)) {
			startu = xp;
			wallu = 0;
		    }
		}
	    }
	    if (yp<hgt-1) {
		if (walld==0) {
		    if (bit!=(fillpix)->GetPixel( xp, yp+1)) {
			tmp = (struct span *)malloc(sizeof(struct span));
			tmp->x = startd;
			tmp->y = yp+1;
			tmp->next = self->fillstack;
			self->fillstack = tmp;
			walld = 1;
		    }
		}
		else {
		    if (bit==(fillpix)->GetPixel( xp, yp+1)) {
			startd = xp;
			walld = 0;
		    }
		}
	    }
	}
	if (xp>xmax) xmax = xp;
	if (wallu==0 && yp>0) {
	    tmp = (struct span *)malloc(sizeof(struct span));
	    tmp->x = startu;
	    tmp->y = yp-1;
	    tmp->next = self->fillstack;
	    self->fillstack = tmp;
	    wallu = 1;
	}
	if (walld==0 && yp<hgt-1) {
	    tmp = (struct span *)malloc(sizeof(struct span));
	    tmp->x = startd;
	    tmp->y = yp+1;
	    tmp->next = self->fillstack;
	    self->fillstack = tmp;
	    walld = 1;
	}
    }

    im::EnqueueEvent((event_fptr)FloodSplot, (char *)self, (event_MSECtoTU(10)));
    /*rasterimage_NotifyObservers(pix, raster_BITSCHANGED);*/
}

static void Tool_SprayPaint(class rastoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks)
{
    switch (action) {
	case view_LeftDown:
	    self->rockx = x;
	    self->rocky = y;
	    self->rock = 1;
	    im::EnqueueEvent((event_fptr)SpraySplot, (char *)self, (event_MSECtoTU(20)));

	case view_LeftMovement:
	    self->rockx = x;
	    self->rocky = y;
	    break;

	case view_LeftUp:
	    self->rock = 0;
	    break;
	default:
	    break;
    }
}

static void SpraySplot(class rastoolview  *self)
{
    int rad = self->sprayradius;
    int sqrad = rad*rad;
    int ix, xoff, yoff;
    int limit = sqrad / 10 + 1;

    for (ix=0; ix<limit; ix++) {
	xoff = RANDOM()%(2*rad+1) - rad;
	yoff = RANDOM()%(2*rad+1) - rad;

	if (xoff*xoff+yoff*yoff <= sqrad)
	    (self->primaryview)->BrushSetPixel( self->rockx+xoff, self->rocky+yoff, self->pattern, self->brush);
    }

    if (self->rock)
	im::EnqueueEvent((event_fptr)SpraySplot, (char *)self, (event_MSECtoTU(20)));
    else {
	class rasterimage *pix = ((class raster *)self->primaryobj)->GetPix();
	(pix)->NotifyObservers( raster_BITSCHANGED);
    }
}

static void Tool_CurvePaint(class rastoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks)
{
    switch (action) {
	case view_LeftDown:
	    self->rockx = x;
	    self->rocky = y;
	    self->lastx = x;
	    self->lasty = y;
	    self->lastvx = 0.0;
	    self->lastvy = 0.0;
	    self->rock = 1;
	    im::EnqueueEvent((event_fptr)CurveSplot, (char *)self, (event_MSECtoTU(20)));

	case view_LeftMovement:
	    self->rockx = x;
	    self->rocky = y;
	    break;

	case view_LeftUp:
	    self->rock = 0;
	    break;
	default:
	    break;
    }
}

static void CurveSplot(class rastoolview  *self)
{
    long x, y;
    
    self->lastvx += (double)(self->rockx - self->lastx) * self->springconst;
    self->lastvy += (double)(self->rocky - self->lasty) * self->springconst;
    self->lastvx *= 0.8;
    self->lastvy *= 0.8;
    x = (long)(self->lastx + self->lastvx);
    y = (long)(self->lasty + self->lastvy);

    DrawLine(self->primaryview, self->lastx, self->lasty, x, y, self->pattern, self->brush);    
    self->lastx = x;
    self->lasty = y;

    if (self->rock)
	im::EnqueueEvent((event_fptr)CurveSplot, (char *)self, (event_MSECtoTU(40)));
    else {
	class rasterimage *pix = ((class raster *)self->primaryobj)->GetPix();
	(pix)->NotifyObservers( raster_BITSCHANGED);
    }
}

static void PasteDownProc(class rastoolview  *self, char  *rock)
{
    if (self->unpaste) {
	self->unpaste = FALSE;
	RepostMenus(self);
    }
}

static void Toolmod_Paste(class rastoolview  *self, char  *rock)
{
    class rasterimage *pix, *pix3;
    struct rectangle R;
    class raster *ras;

    /* slap unpasteraster on screen */
    if (self->unpaste) {
	ras = (class raster *)self->primaryobj;
	pix = (ras)->GetPix();
	pix3 = (self->unpasteraster)->GetPix();
	rectangle_SetRectSize(&R, 0, 0, self->rockw, self->rockh);
	(pix)->BlitSubraster( self->lastx, self->lasty, pix3, &R, pixelimage_COPY);
	(pix)->NotifyObservers( raster_BITSCHANGED);
	self->unpaste = FALSE;
	message::DisplayString(self, 40, "Removed paste selection.");
	RepostMenus(self);
    }
}

static void PasteResplot(class rastoolview  *self, char  *rock)
{
    class rasterimage *pix, *pix2, *pix3;
    struct rectangle CR, R;
    class raster *ras;

    if (self->unpaste) {
	ras = (class raster *)self->primaryobj;
	pix = (ras)->GetPix();

	/* slap unpasteraster on screen */
	pix3 = (self->unpasteraster)->GetPix();
	rectangle_SetRectSize(&R, 0, 0, self->rockw, self->rockh);
	(pix)->BlitSubraster( self->lastx, self->lasty, pix3, &R, pixelimage_COPY);
	rectangle_SetRectSize(&CR, self->lastx, self->lasty, self->rockw, self->rockh);

	/* slap pasteraster onto screen */
	pix2 = (self->pasteraster)->GetPix();
	rectangle_SetRectSize(&R, 0, 0, self->rockw, self->rockh);
	(pix)->BlitSubraster( self->lastx, self->lasty, pix2, &R, self->pastemode);

	(pix)->NotifyObservers( raster_BITSCHANGED);
    }
}

static void Toolmod_FloodFill(class rastoolview  *self, char  *rock)
{
    struct span *tmp;
    if (self->fillstack==NULL) {
	message::DisplayString(self, 10, "Cancel the current flood-fill operation: none running.");
    }
    else {
	while (self->fillstack) {
	    tmp = self->fillstack;
	    self->fillstack = tmp->next;
	    free(tmp);
	}
	message::DisplayString(self, 10, "Cancelled the current flood-fill operation.");
    }
}

static void Toolmod_SprayPaint(class rastoolview  *self    , char  *rock)
{
    char buffer[32], buf2[32];
    int val, res;
    sprintf(buf2, "%d", self->sprayradius);
    res = message::AskForString (self, 40, "How large should the spraypaint circle be in pixels?  ", buf2, buffer, 30); 
    if (res<0 || strlen(buffer)==0) {
	message::DisplayString(self, 10, "Cancelled.");
	return;
    }
    val = atoi(buffer);
    if (val<=0) {
	message::DisplayString(self, 10, "Value must be a positive integer.");
	return;
    }
    if (val>100) {
	message::DisplayString(self, 10, "That value is too large.");
	return;
    }
    self->sprayradius = val;
    sprintf(buf2, "Spray area set to %d", self->sprayradius);
    message::DisplayString(self, 10, buf2);
}
		
static void Toolmod_CurvePaint(class rastoolview  *self    , char  *rock)
{
    char buffer[32], buf2[32];
    int res;
    double val;
    sprintf(buf2, "0.5");
    res = message::AskForString (self, 40, "How stiff should the pen spring be? [0.0 -- 1.0]  ", buf2, buffer, 30); 
    if (res<0 || strlen(buffer)==0) {
	message::DisplayString(self, 10, "Cancelled.");
	return;
    }
    val = atof(buffer);
    if (val > 1.0 || val < -1.0) {
	message::DisplayString(self, 10, "That value is out of range.");
	return;
    }
    if (val == 0.0) {
	message::DisplayString(self, 10, "You cannot set the stiffness to zero.");
	return;
    }
    self->springconst = val / 10.0;
    sprintf(buf2, "Spring stiffness set to %g", val);
    message::DisplayString(self, 10, buf2);
}

static void Toolmod_Text(class rastoolview  *self    , char  *rock)
{
    if (!(self->primaryview)->GetOverlaidInset()) {
	message::DisplayString(self, 10, "There is no overlay inset in the raster window.");
	return;
    }
    (self->primaryview)->ImprintInset( self->pastemode);
    RepostMenus(self);
}

static void ResizeInsetProc(class rastoolview  *self    , char  *rock)
{
    if (!(self->primaryview)->GetOverlaidInset()) {
	message::DisplayString(self, 10, "There is no overlay inset in the raster window.");
	return;
    }
    (self->primaryview)->ResizeInset();
    RepostMenus(self);
}

static void RemoveInsetProc(class rastoolview  *self    , char  *rock)
{
    if (!(self->primaryview)->GetOverlaidInset()) {
	message::DisplayString(self, 10, "There is no overlay inset in the raster window.");
	return;
    }
    (self->primaryview)->RemoveInset();
    RepostMenus(self);
}
