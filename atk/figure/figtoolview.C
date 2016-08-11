/* figtoolv.c - drawing editor toolset */
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
char *figotoolv_c_rcsid = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/figure/RCS/figtoolview.C,v 3.12 1995/09/18 21:51:50 robr Stab74 $";
#endif

#include <andrewos.h>
ATK_IMPL("figtoolview.H")
#include <figtoolview.H>

#include <ctype.h>

#include <math.h>

#include <menulist.H>
#include <message.H>
#include <keymap.H>
#include <keystate.H>
#include <cursor.H>
#include <proctable.H>
#include <stringtbl.H>
#include <strtblview.H>
#include <lpair.H>
#include <view.H>

#include <fontsel.H>
#include <fontselview.H>

#include <figview.H>
#include <figure.H>
#include <figobj.H>
#include <figogrp.H>
#include <figattr.H>
#include <figoplin.H>
#include <figospli.H>
#include <figoins.H>

#include <point.h>
#include <rect.h>

#define figtoolview_SelectClickDistance (6)

static class menulist *MyMenus;
static class keymap *Keymap;


ATKdefineRegistry(figtoolview, lpair, figtoolview::InitializeClass);

static void SetExpertModeProc(class figtoolview  *self, long  val);
static void RebuildToolMenu(class figtoolview *self);
static void RepostMenus(class figtoolview  *self);
static void SetToolProc(class stringtbl  *st, class figtoolview  *self, short  accnum);
static void SetShadeProc(class stringtbl  *st, class figtoolview  *self, short  accnum);
static void SetTextPosProc(class stringtbl  *st, class figtoolview  *self, short  accnum);
static void SetLineStyleProc(class stringtbl  *st, class figtoolview  *self, short  accnum);
static void InsertLineWidth(class figtoolview  *self, short  val);
static void SetLineWidthProc(class stringtbl  *st, class figtoolview  *self, short  accnum);
static void InsertArrowSize(class figtoolview  *self, short  val);
static void SetArrowProc(class stringtbl  *st, class figtoolview  *self, short  accnum);
static void InsertRRectCorner(class figtoolview  *self, short  val);
static void SetRRectCornerProc(class stringtbl  *st, class figtoolview  *self, short  accnum);
static void InsertColor(class figtoolview  *self, char  *val);
static void SetColorProc(class stringtbl  *st, class figtoolview  *self, short  accnum);
static void InsertSnapGrid(class figtoolview  *self, short  val);
static void SetSnapGridProc(class stringtbl  *st, class figtoolview  *self, short  accnum);
static void CallCmdProc(class stringtbl  *st, class figtoolview  *self, short  accnum);
static void Command_Quit(class figtoolview  *self, char  *rock);
static boolean CSA_Splot(class figobj  *o, long  ref, class figure  *fig, class figview  *figv);
static void Command_LockCreate(class figtoolview  *self, char  *rock);
static void Command_CutNPaste(class figtoolview  *self, long  rock);
static void Command_SelectAll(class figtoolview  *self, char  *rock);
static void Command_Refresh(class figtoolview  *self, char  *rock);
static void Command_Zoom(class figtoolview  *self, long  rock);
static void Command_PanToOrigin(class figtoolview  *self, long  rock);
static void ATSPSplot(class figobj  *o, long  ref, class figview  *vv, class figattr  *attr);
static void ApplyToSelProc(class figtoolview  *self, long  rock);
static void AdjustToMenus(class figtoolview  *self, long  mask);
static void AdjustToSelection(class figtoolview  *self);
static void IncreaseTmpProc(class figtoolview  *self, long  num);
static void CacheSelectProc(class figobj  *o, long  ref, class figview  *vv, class figtoolview  *figt);
static void Command_GroupSel(class figtoolview  *self, char  *rock);
static boolean CacheContentsProc(class figobj  *o, long  ref, class figure  *fig, class figtoolview  *figt);
static void Command_UngroupSel(class figtoolview  *self, char  *rock);
static void Command_MoveToExtreme(class figtoolview  *self, long  infront);
static void ToggleSmoothProc(class figtoolview  *self, long  rock);
static void ToggleClosedProc(class figtoolview  *self, long  rock);
static void Command_SetDoConstraint(class figtoolview  *self, boolean  rock);
static void DefaultAnchorSplot(class figobj  *o, long  ref, class figview  *self, long  rock);
static void ProportAnchorSplot(class figobj  *o, long  ref, class figview  *self, long  rock);
static void ClearAnchorSplot(class figobj  *o, long  ref, class figview  *self, long  rock);
static void Command_ClearAnchors(class figtoolview  *self, char  *rock);
static void Command_Name(figtoolview *self, char *rock);
static void Command_DefaultAnchors(class figtoolview  *self, char  *rock);
static void Command_ProportAnchors(class figtoolview  *self, char  *rock);
static boolean SelAddProc(class figobj  *o, long  ref, class figure  *fig, class figview  *vv);
static boolean SelTogProc(class figobj  *o, long  ref, class figure  *fig, class figview  *vv);
static void MakeBoxListProc(class figobj  *o, long  ref, class figview  *vv, struct rectangle  **rec);
static void MoveObjsProc(class figobj  *o, long  ref, class figview  *vv, struct point  *pt);
static void Toolsub_Reshape(class figtoolview  *self, enum view_MouseAction  action, long  x , long  y, long  oref , long  ptref);
static void Toolsub_AddAnch(class figtoolview  *self, enum view_MouseAction  action, long  x , long  y, boolean  onhandle, long  oref , long  ptref);
static void Toolsub_DelAnch(class figtoolview  *self, enum view_MouseAction  action, long  x , long  y, boolean  onhandle, long  oref , long  ptref);
static void Toolsub_Add(class figtoolview  *self, enum view_MouseAction  action, long  x , long  y, boolean  onhandle, long  oref , long  ptref);
static void Toolsub_Del(class figtoolview  *self, enum view_MouseAction  action, long  x , long  y, boolean  onhandle, long  oref , long  ptref);
static void Toolsub_Drag(class figtoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks);
static void Toolsub_Select(class figtoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks);
static void FindHitObjProc(class figobj  *o, long  ref, class figview  *vv, struct FHOP_lump  *val);
static void FindHitObjAnchProc(class figobj  *o, long  ref, class figview  *vv, struct FHOP_lump  *val);
static class view *Tool_Select(class figtoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks);
static class view *Tool_AddPoints(class figtoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks);
static class view *Tool_DelPoints(class figtoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks);
static class view *Tool_AddAnchor(class figtoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks);
static class view *Tool_DelAnchor(class figtoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks);
static void Toolmod_Select(class figtoolview  *self, long  rock);
static class view *Tool_CreateProc(class figtoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks);
static void Tool_Name(class figtoolview  *self, long  rock);
static void AbortObjectProc(class figtoolview  *self, long  rock);
static char *CopyString(char  *str);
static void LowerString(char  *str);
static char *WhiteKillString(char  *buf);
  

#ifndef ABS
#define ABS(x) (((x)<0)?(-(x)):(x))
#endif


struct layout_t {
    figtoolview_layout_f proc; /* tool procedure */
    figtoolview_mod_f procmod; /* procedure to call when tool is double-clicked */
    boolean writes; /* does the tool change the raster data? */
    char *name; /* name in the toolset window */
};

struct comlayout_t {
    figtoolview_comm_f proc; /* command procedure */
    char *rock;	    /* a rock */
    char *name; /* name in the toolset window */
};

#define FIGTOOLS_NUM (5)
#define FIGTOOLSEX_NUM (2)
#define NumFigTools(self)  ((self)->toolextras ? (FIGTOOLS_NUM+FIGTOOLSEX_NUM) : FIGTOOLS_NUM)

static struct layout_t toollayout[FIGTOOLS_NUM+FIGTOOLSEX_NUM] = {
    {NULL, NULL, 0, "TOOLS:"},
    {NULL, NULL, 0, "Browse"},
    {(figtoolview_layout_f)Tool_Select, Toolmod_Select, 0, "Reshape"},
    {(figtoolview_layout_f)Tool_AddPoints, NULL, 1, "Add Points"},
    {(figtoolview_layout_f)Tool_DelPoints, NULL, 1, "Del Points"},

    {(figtoolview_layout_f)Tool_AddAnchor, NULL, 1, "Set Anchors"},
    {(figtoolview_layout_f)Tool_DelAnchor, NULL, 1, "Del Anchors"},
};

struct objlayout_t {
    char *name;	    /* name in the toolset window */
    long rock;	    /* rock passed to Instantiate() */
};

/*
#define FIGOBJS_NUM (12)
static struct objlayout_t objectlayout[FIGOBJS_NUM] = {
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
*/

#define FIGCMDS_NUM (16)
#define FIGCMDSEX_NUM (5)
#define FIGCMD_LOCKCREATE (14)
static struct comlayout_t cmdlayout[FIGCMDS_NUM+FIGCMDSEX_NUM] = {
    {NULL,		    NULL,	"COMMANDS:"},
    {(figtoolview_comm_f)Command_SelectAll,	    NULL,	"Select all"},
    {(figtoolview_comm_f)Command_CutNPaste,	    (char *)figview_OpCut, "Cut"},
    {(figtoolview_comm_f)Command_CutNPaste,	    (char *)figview_OpCopy, "Copy"},
    {(figtoolview_comm_f)Command_CutNPaste,	    (char *)figview_OpPaste, "Paste"},
    {(figtoolview_comm_f)Command_Zoom,	    (char *)1,	"Zoom in"},
    {(figtoolview_comm_f)Command_Zoom,	    (char *)-1,	"Zoom out"},
    {(figtoolview_comm_f)Command_Zoom,	    (char *)0,	"Normal size"},
    {(figtoolview_comm_f)Command_PanToOrigin,   NULL,	"Pan to origin"},
    {(figtoolview_comm_f)Command_Refresh,	    NULL,	"Refresh"},
    {(figtoolview_comm_f)Command_GroupSel,	    NULL,	"Group selec."},
    {(figtoolview_comm_f)Command_UngroupSel,    NULL,	"Ungroup selec."},
    {(figtoolview_comm_f)Command_MoveToExtreme, (char *)1,	"Move to front"},
    {(figtoolview_comm_f)Command_MoveToExtreme, (char *)0,	"Move to back"},
    {(figtoolview_comm_f)Command_LockCreate,    NULL,	"Keep creating"},

    {(figtoolview_comm_f)Command_SetDoConstraint,	(char *)1,  "Resizing on"},
    {(figtoolview_comm_f)Command_SetDoConstraint,	(char *)0,  "Resizing off"},
    {(figtoolview_comm_f)Command_ClearAnchors,	NULL,	    "Clear anchors"},
    {(figtoolview_comm_f)Command_DefaultAnchors,	NULL,	    "Standard anchors"},
    {(figtoolview_comm_f)Command_ProportAnchors,	NULL,	    "Propor. anchors"},
    {(figtoolview_comm_f)Command_Name,	NULL,	    "Set Name"}
};

#define FIGSHADES_NUM (12)
#define figtoolv_shade_Inherit (1)
#define figtoolv_shade_Clear (2)
#define figtoolv_shade_Zero (3)

static char *shadelayout[FIGSHADES_NUM] = {
    "SHADES:",
    "<default>",
    "clear",
    "white",
    "grey 1/8", 
    "grey 1/4", 
    "grey 3/8", 
    "grey 1/2", 
    "grey 5/8", 
    "grey 3/4", 
    "grey 7/8", 
    "black"
};

#define FIGTEXTPOSS_NUM (5)
#define figtoolv_textpos_Inherit (1)
#define figtoolv_textpos_Zero (2)

static char *textposlayout[FIGTEXTPOSS_NUM] = {
    "TEXT POS:",
    "<default>",
    "Center",
    "Left", 
    "Right"
};

#define FIGLINESTYLES_NUM (8)
#define figtoolv_linestyle_Inherit (1)
#define figtoolv_linestyle_Zero (2)

static char *linestylelayout[FIGLINESTYLES_NUM] = {
    "LINE STYLES:",
    "<default>",
    "________",
    ".  .  .  .",
    "_  _  _  _ ",
    "__  __  __",
    "__  .  __",
    "__  . .  __"
};

#define	FIGLINEWIDTHS_NUM_INIT (9)
#define figtoolv_linewidth_Inherit (1)
#define figtoolv_linewidth_Other (2)
#define figtoolv_linewidth_Zero (3)

static char *linewidthlayout[FIGLINEWIDTHS_NUM_INIT] = {
    "LINE WIDTHS:",
    "<default>",
    "<other>",
    "0",
    "1", 
    "2", 
    "4", 
    "6", 
    "8"
};

#define	FIGRRECTCORNERS_NUM_INIT (6)
#define figtoolv_rrectcorner_Inherit (1)
#define figtoolv_rrectcorner_Other (2)
#define figtoolv_rrectcorner_Zero (3)

static char *rrectcornerlayout[FIGRRECTCORNERS_NUM_INIT] = {
    "ROUND CORNER:",
    "<default>",
    "<other>",
    "10",
    "20", 
    "30"
};

#define	FIGCOLORS_NUM_INIT (10)
#define figtoolv_color_Inherit (1)
#define figtoolv_color_Other (2)
#define figtoolv_color_Zero (3)

static char *colorlayout[FIGCOLORS_NUM_INIT] = {
    "COLORS:",
    "<default>",
    "<other>",
    "black",
    "red", 
    "blue",
    "green",
    "cyan",
    "magenta",
    "yellow"
};

#define	FIGARROWS_NUM_INIT (15)
#define figtoolv_arrowpos_Inherit (1)
#define figtoolv_arrowpos_Tail (2)
#define figtoolv_arrowpos_Head (3)
#define figtoolv_arrowshape_Inherit (4)
#define figtoolv_arrowshape_Zero (5)
#define figtoolv_arrowshape_IsoTriangle (5)
#define figtoolv_arrowshape_Circle (6)
#define figtoolv_arrowshape_Square (7)
#define figtoolv_arrowshape_EquTriangle (8)
#define figtoolv_arrowsize_Inherit (9)
#define figtoolv_arrow_Other (10)
#define figtoolv_arrow_Zero (11)

static char *arrowlayout[FIGARROWS_NUM_INIT] = {
    "ARROWS:",
    "<default pos>",
    "on tails",
    "on heads",
    "<default shape>",
    "triangle",
    "circle",
    "square",
    "eq.triangle",
    "<default size>",
    "<other>",
    "4", 
    "8", 
    "10", 
    "12"
};

struct snapgridlayout_t {
    char *name;  /* name in the toolset window */
    long size;	    /* snap distance in figs */
};

#define figview_UnitConvert(unt)  (((unt)==figtoolv_snapgrid_inches) ? (double)(figview_FigUPerPix * 72) : (((unt)==figtoolv_snapgrid_cms) ? ((double)figview_FigUPerPix) * 28.25 : (double)figview_FigUPerPix))
/* 28.25 should be 28.346456692913, but let's not worry too much */

#define	FIGSNAPGRIDS_NUM_INIT	 (9)
#define figtoolv_snapgrid_points (1)
#define figtoolv_snapgrid_inches (2)
#define	figtoolv_snapgrid_cms	 (3)
#define figtoolv_snapgrid_Other  (4)
#define	figtoolv_snapgrid_Zero	 (5)

static struct snapgridlayout_t snapgridlayout[FIGSNAPGRIDS_NUM_INIT] = {
    {"GRID SNAP:",  0},
    {"points",	    0},
    {"inches", 	    0},
    {"cms",	    0},
    {"<other>",	    0},
    {"snap off",    0},
    {"0.125",	    9*figview_FigUPerPix},
    {"0.25",	    18*figview_FigUPerPix},
    {"0.5",	    36*figview_FigUPerPix}
};

#define ML_expertmode (1)
#define ML_objectcreating (2)



boolean figtoolview::InitializeClass()
{
    struct proctable_Entry *proc = NULL;
    
    MyMenus = new menulist;
    Keymap = new keymap;

    proc = proctable::DefineProc("figtoolv-toolset-destroy", (proctable_fptr)Command_Quit, &figtoolview_ATKregistry_ , NULL, "Deletes toolset window.");
    (Keymap)->BindToKey( "\030\004", proc, 0);	/* ^X^D */
    (Keymap)->BindToKey( "\030\003", proc, 0);	/* ^X^C */
    (MyMenus)->AddToML( "Delete Window~89", proc, 0, 0);
    
    proc = proctable::DefineProc("figtoolv-apply-to-selection", (proctable_fptr)ApplyToSelProc, &figtoolview_ATKregistry_ , NULL, "Apply attributes to selected objects.");
    (MyMenus)->AddToML( "Apply to Selection~33", proc, 0, 0);

    proc = proctable::DefineProc("figtoolv-toggle-smooth", (proctable_fptr)ToggleSmoothProc, &figtoolview_ATKregistry_ , NULL, "Turn selected polylines into splines and back.");
    (MyMenus)->AddToML( "Smooth / Unsmooth~31", proc, 0, 0);

    proc = proctable::DefineProc("figtoolv-toggle-closed", (proctable_fptr)ToggleClosedProc, &figtoolview_ATKregistry_ , NULL, "Open or close selected polylines and splines.");
    (MyMenus)->AddToML( "Close / Open~32", proc, 0, 0);

    proc = proctable::DefineProc("figtoolv-abort-object", (proctable_fptr)AbortObjectProc, &figtoolview_ATKregistry_ , NULL, "Abort the object being built in the figview.");
    (MyMenus)->AddToML( "Cancel Object~11", proc, 0, ML_objectcreating);

    return TRUE;
}

figtoolview::figtoolview()
{
    ATKinit;

    int ix;
    class stringtbl *tl;
    class strtblview *tlv;

    this->Menus = (MyMenus)->DuplicateML( this);
    this->Keystate = keystate::Create(this, Keymap);
    this->expertmode = FALSE;
    /*this->moribund = FALSE;*/
    this->primaryview = NULL;
    this->selectonup = FALSE;
    this->LockCreateMode = FALSE;

    this->menuatt = new figattr;
    this->toolnum = 1; /* ### pan */

    this->shadenum = figtoolv_shade_Clear; 
    (this->menuatt)->SetShade( figattr_ShadeClear);
    this->textposnum = figtoolv_textpos_Zero;
    (this->menuatt)->SetTextPos( figattr_PosCenter);
    this->linestylenum = figtoolv_linestyle_Zero;
    (this->menuatt)->SetLineStyle( figattr_LineSolid);
    this->linewidthnum = figtoolv_linewidth_Zero+1; 
    (this->menuatt)->SetLineWidth( atoi(linewidthlayout[this->linewidthnum]));
    this->rrectcornernum = figtoolv_rrectcorner_Zero; 
    (this->menuatt)->SetRRectCorner( atoi(rrectcornerlayout[this->rrectcornernum]));
    this->colornum = figtoolv_color_Zero; 
    (this->menuatt)->SetColor( colorlayout[this->colornum]);
    (this->menuatt)->SetFontSize( fontsel_default_Size);
    (this->menuatt)->SetFontStyle( fontsel_default_Style);
    (this->menuatt)->SetFontFamily( fontsel_default_Family);
    this->arrowsizenum = figtoolv_arrow_Zero+1; /* ### 4 */
    this->arrowposnum = figattr_ArrowNone;
    this->arrownum = figtoolv_arrowshape_Zero;
    (this->menuatt)->SetArrowSize( atoi(arrowlayout[this->arrowsizenum]));
    (this->menuatt)->SetArrow( this->arrownum);
    (this->menuatt)->SetArrowPos(figattr_IsoTriangle);
    /* ##new */

    this->toolproc = NULL;
    this->toollistnum = 0;
    this->toollistsize = 4;
    this->toollist = (struct figtoolview_tool_t *)malloc(this->toollistsize * sizeof(struct figtoolview_tool_t));
    this->dummylist = (class figobj **)malloc(this->toollistsize * sizeof(class figobj *));
    for (ix=0; ix<this->toollistsize; ix++)
	this->dummylist[ix] = NULL;
    this->creating = NULL;
    this->selectdeep = FALSE;
    this->rect_size = 8;
    this->rectlist = (struct rectangle *)malloc(this->rect_size * sizeof(struct rectangle));
    this->tmp_size = 0;
    this->tmplist = NULL;

    this->lp1 = new lpair;
    this->lp2 = new lpair;
    this->lp3 = new lpair;
    this->lp4 = new lpair;
    this->lp5 = new lpair;
    this->lp6 = new lpair;
    this->lp7 = new lpair;
    this->lp8 = new lpair;
    this->lp9 = new lpair;

    if (!this->lp1 || !this->lp2 || !this->lp3 || !this->lp4 || !this->lp5 || !this->lp6 || !this->lp7 || !this->lp8 || !this->lp9 || !this->rectlist || !this->dummylist || !this->Menus || !this->Keystate)
	THROWONFAILURE( FALSE);

    this->vfontsel = new fontselview;
    this->fontselp = new fontsel;
    if (!this->vfontsel || !this->fontselp)
	THROWONFAILURE( FALSE);
    (this->vfontsel)->SetDataObject( this->fontselp);
    (this->fontselp)->AddObserver( this);
    (this->vfontsel)->ShowExtraOption();

    /* tool table */
    tl = new stringtbl;
    tlv = new strtblview;
    if (!tl || !tlv) THROWONFAILURE( FALSE);
    this->tooltbl = tl;
    this->vtooltbl = tlv;
    (tl)->Clear();
    this->toolacc = (short *)malloc(sizeof(short) * (this->toollistnum + FIGTOOLS_NUM));
    this->toolextras = FALSE;
    RebuildToolMenu(this);
    /*for (ix=0; ix<FIGTOOLS_NUM; ix++) {
	this->toolacc[ix] = (tl)->AddString( toollayout[ix].name);
	if (this->toolacc[ix] == (-1))
	    THROWONFAILURE( FALSE);
    }*/
    /* ### handle elsewhere 
    for (ix=0; ix<FIGOBJS_NUM; ix++) {
	this->dummylist[ix] = (class figobj *)ATK::NewObject(objectlayout[ix].name);
	if (!this->dummylist[ix]) 
	    THROWONFAILURE( FALSE);
	this->toolacc[ix+FIGTOOLS_NUM] = (tl)->AddString( (this->dummylist[ix])->ToolName( this, objectlayout[ix].rock));
	if (this->toolacc[ix+FIGTOOLS_NUM] == (-1))
	    THROWONFAILURE( FALSE);
    }
     */
    (tlv)->SetItemHitProc( (strtblview_hitfptr)SetToolProc, (long)this);
    /*(tl)->ClearBits();
    (tl)->SetBitOfEntry( this->toolacc[this->toolnum], TRUE);*/
    (tlv)->SetDataObject( tl);

    /* shade table */
    tl = new stringtbl;
    tlv = new strtblview;
    if (!tl || !tlv) THROWONFAILURE( FALSE);
    (tl)->Clear();
    this->shadeacc = (short *)malloc(sizeof(short) * FIGSHADES_NUM);
    for (ix=0; ix<FIGSHADES_NUM; ix++) {
	this->shadeacc[ix] = (tl)->AddString( shadelayout[ix]);
	if (this->shadeacc[ix] == (-1))
	    THROWONFAILURE( FALSE);
    }
    (tlv)->SetItemHitProc( (strtblview_hitfptr)SetShadeProc, (long)this);
    (tl)->ClearBits();
    (tl)->SetBitOfEntry( this->shadeacc[this->shadenum], TRUE);
    (tlv)->SetDataObject( tl);
    this->shadetbl = tl;
    this->vshadetbl = tlv;

    /* textpos table */
    tl = new stringtbl;
    tlv = new strtblview;
    if (!tl || !tlv) THROWONFAILURE( FALSE);
    (tl)->Clear();
    this->textposacc = (short *)malloc(sizeof(short) * FIGTEXTPOSS_NUM);
    for (ix=0; ix<FIGTEXTPOSS_NUM; ix++) {
	this->textposacc[ix] = (tl)->AddString( textposlayout[ix]);
	if (this->textposacc[ix] == (-1))
	    THROWONFAILURE( FALSE);
    }
    (tlv)->SetItemHitProc( (strtblview_hitfptr)SetTextPosProc, (long)this);
    (tl)->ClearBits();
    (tl)->SetBitOfEntry( this->textposacc[this->textposnum], TRUE);
    (tlv)->SetDataObject( tl);
    this->textpostbl = tl;
    this->vtextpostbl = tlv;

    /* linestyle table */
    tl = new stringtbl;
    tlv = new strtblview;
    if (!tl || !tlv) THROWONFAILURE( FALSE);
    (tl)->Clear();
    this->linestyleacc = (short *)malloc(sizeof(short) * FIGLINESTYLES_NUM);
    for (ix=0; ix<FIGLINESTYLES_NUM; ix++) {
	this->linestyleacc[ix] = (tl)->AddString( linestylelayout[ix]);
	if (this->linestyleacc[ix] == (-1))
	    THROWONFAILURE( FALSE);
    }
    (tlv)->SetItemHitProc( (strtblview_hitfptr)SetLineStyleProc, (long)this);
    (tl)->ClearBits();
    (tl)->SetBitOfEntry( this->linestyleacc[this->linestylenum], TRUE);
    (tlv)->SetDataObject( tl);
    this->linestyletbl = tl;
    this->vlinestyletbl = tlv;

    /* linewidth table */
    tl = new stringtbl;
    tlv = new strtblview;
    if (!tl || !tlv) THROWONFAILURE( FALSE);
    (tl)->Clear();
    this->linewidths_num = FIGLINEWIDTHS_NUM_INIT;
    this->linewidthacc = (short *)malloc(sizeof(short) * this->linewidths_num);
    this->linewidthlist = (short *)malloc(sizeof(short) * this->linewidths_num);
    for (ix=0; ix<this->linewidths_num; ix++) {
	this->linewidthacc[ix] = (tl)->AddString( linewidthlayout[ix]);
	if (this->linewidthacc[ix] == (-1))
	    THROWONFAILURE( FALSE);
	if (ix>figtoolv_linewidth_Other)
	    this->linewidthlist[ix] = atoi(linewidthlayout[ix]);
    }
    (tlv)->SetItemHitProc( (strtblview_hitfptr)SetLineWidthProc, (long)this);
    (tl)->ClearBits();
    (tl)->SetBitOfEntry( this->linewidthacc[this->linewidthnum], TRUE);
    (tlv)->SetDataObject( tl);
    this->linewidthtbl = tl;
    this->vlinewidthtbl = tlv;

    /* arrow{size,pos,shape} table */
    tl = new stringtbl;
    tlv = new strtblview;
    if (!tl || !tlv) THROWONFAILURE( FALSE);
    (tl)->Clear();
    this->arrows_num = FIGARROWS_NUM_INIT;
    this->arrowacc = (short *)malloc(sizeof(short) * this->arrows_num);
    this->arrowlist = (short *)malloc(sizeof(short) * this->arrows_num);
    for (ix=0; ix<this->arrows_num; ix++) {
	this->arrowacc[ix] = (tl)->AddString( arrowlayout[ix]);
	if (this->arrowacc[ix] == (-1))
	    THROWONFAILURE( FALSE);
	if (ix>figtoolv_arrow_Other)
	    this->arrowlist[ix] = atoi(arrowlayout[ix]);
	else if (ix>=figtoolv_arrowshape_Zero)
	    this->arrowlist[ix] = ((ix-figtoolv_arrowshape_Zero) << 4);
    }
    (tlv)->SetItemHitProc( (strtblview_hitfptr)SetArrowProc, (long)this);
    (tl)->ClearBits();
    (tl)->SetBitOfEntry( this->arrowacc[this->arrowsizenum], TRUE);
    (tl)->SetBitOfEntry( this->arrowacc[this->arrownum], TRUE);
    /* leave head and tail flags off */
    (tlv)->SetDataObject( tl);
    this->arrowtbl = tl;
    this->varrowtbl = tlv;

    /* rrectcorner table */
    tl = new stringtbl;
    tlv = new strtblview;
    if (!tl || !tlv) THROWONFAILURE( FALSE);
    (tl)->Clear();
    this->rrectcorners_num = FIGRRECTCORNERS_NUM_INIT;
    this->rrectcorneracc = (short *)malloc(sizeof(short) * this->rrectcorners_num);
    this->rrectcornerlist = (short *)malloc(sizeof(short) * this->rrectcorners_num);
    for (ix=0; ix<this->rrectcorners_num; ix++) {
	this->rrectcorneracc[ix] = (tl)->AddString( rrectcornerlayout[ix]);
	if (this->rrectcorneracc[ix] == (-1))
	    THROWONFAILURE( FALSE);
	if (ix>figtoolv_rrectcorner_Other)
	    this->rrectcornerlist[ix] = atoi(rrectcornerlayout[ix]);
    }
    (tlv)->SetItemHitProc((strtblview_hitfptr) SetRRectCornerProc, (long)this);
    (tl)->ClearBits();
    (tl)->SetBitOfEntry( this->rrectcorneracc[this->rrectcornernum], TRUE);
    (tlv)->SetDataObject( tl);
    this->rrectcornertbl = tl;
    this->vrrectcornertbl = tlv;

    /* color table */
    tl = new stringtbl;
    tlv = new strtblview;
    if (!tl || !tlv) THROWONFAILURE( FALSE);
    (tl)->Clear();
    this->colors_num = FIGCOLORS_NUM_INIT;
    this->coloracc = (short *)malloc(sizeof(short) * this->colors_num);
    this->colorlist = (char **)malloc(sizeof(char *) * this->colors_num);
    for (ix=0; ix<this->colors_num; ix++) {
	this->coloracc[ix] = (tl)->AddString( colorlayout[ix]);
	if (this->coloracc[ix] == (-1))
	    THROWONFAILURE( FALSE);
	if (ix>figtoolv_color_Other)
	    this->colorlist[ix] = CopyString(colorlayout[ix]);
	else
	    this->colorlist[ix] = NULL;
    }
    (tlv)->SetItemHitProc((strtblview_hitfptr) SetColorProc, (long)this);
    (tl)->ClearBits();
    (tl)->SetBitOfEntry( this->coloracc[this->colornum], TRUE);
    (tlv)->SetDataObject( tl);
    this->colortbl = tl;
    this->vcolortbl = tlv;

    /* ##new */

    /* snapgrid table */
    this->snapgrid = 0;
    this->snapgridnum = figtoolv_snapgrid_Zero;
    this->snapgridunit = figtoolv_snapgrid_inches;
    tl = new stringtbl;
    tlv = new strtblview;
    if (!tl || !tlv) THROWONFAILURE( FALSE);
    (tl)->Clear();
    this->snapgrids_num = FIGSNAPGRIDS_NUM_INIT;
    this->snapgridacc = (short *)malloc(sizeof(short) * this->snapgrids_num);
    this->snapgridlist = (short *)malloc(sizeof(short) * this->snapgrids_num);
    for (ix=0; ix<this->snapgrids_num; ix++) {
	this->snapgridacc[ix] = (tl)->AddString( snapgridlayout[ix].name);
	if (this->snapgridacc[ix] == (-1))
	    THROWONFAILURE( FALSE);
	if (ix>figtoolv_snapgrid_Other)
	    this->snapgridlist[ix] = (snapgridlayout[ix].size);
    }
    (tlv)->SetItemHitProc((strtblview_hitfptr) SetSnapGridProc, (long)this);
    (tl)->ClearBits();
    (tl)->SetBitOfEntry( this->snapgridacc[this->snapgridnum], TRUE);
    (tl)->SetBitOfEntry( this->snapgridacc[this->snapgridunit], TRUE);
    (tlv)->SetDataObject( tl);
    this->snapgridtbl = tl;
    this->vsnapgridtbl = tlv;

    /* cmd table */
    tl = new stringtbl;
    tlv = new strtblview;
    if (!tl || !tlv) THROWONFAILURE( FALSE);
    (tl)->Clear();
    this->cmdacc = (short *)malloc(sizeof(short) * FIGCMDS_NUM);
    for (ix=0; ix<FIGCMDS_NUM; ix++) {
	this->cmdacc[ix] = (tl)->AddString( cmdlayout[ix].name);
	if (this->cmdacc[ix] == (-1))
	    THROWONFAILURE( FALSE);
    }
    (tlv)->SetItemHitProc((strtblview_hitfptr) CallCmdProc, (long)this);
    (tl)->ClearBits();
    (tlv)->SetDataObject( tl);
    this->cmdtbl = tl;
    this->vcmdtbl = tlv;

    (this->lp4)->SetUp( this->vtooltbl, this->vcmdtbl, 50, lpair_PERCENTAGE, lpair_VERTICAL, TRUE);
    (this->lp6)->SetUp( this->vlinewidthtbl, this->varrowtbl, 60, lpair_PERCENTAGE, lpair_HORIZONTAL, TRUE);
    (this->lp8)->SetUp( this->vrrectcornertbl, this->vsnapgridtbl, 50, lpair_PERCENTAGE, lpair_HORIZONTAL, TRUE);
    (this->lp3)->SetUp( this->lp6, this->lp8, 40, lpair_PERCENTAGE, lpair_HORIZONTAL, TRUE);
    (this->lp2)->SetUp( this->vshadetbl, this->vcolortbl, 45, lpair_PERCENTAGE, lpair_HORIZONTAL, TRUE);
    (this->lp9)->SetUp( this->lp2, this->vlinestyletbl, 25, lpair_PERCENTAGE, lpair_HORIZONTAL, TRUE);
    (this->lp7)->SetUp( this->vfontsel, this->vtextpostbl, 25, lpair_PERCENTAGE, lpair_HORIZONTAL, TRUE);
    (this->lp1)->SetUp( this->lp4, this->lp7, 33, lpair_PERCENTAGE, lpair_HORIZONTAL, TRUE);
    (this->lp5)->SetUp( this->lp9, this->lp3, 50, lpair_PERCENTAGE, lpair_VERTICAL, TRUE);
    (this)->SetUp( this->lp1, this->lp5, 50, lpair_PERCENTAGE, lpair_VERTICAL, TRUE);

    THROWONFAILURE( TRUE);
}

figtoolview::~figtoolview()
{
	ATKinit;

    int ix;

    (this->Menus)->ClearChain();
    delete this->Menus;
    delete this->Keystate;

    (this->fontselp)->RemoveObserver( this);

    (this)->SetNth( 0, NULL);
    (this)->SetNth( 1, NULL);

    (this->lp5)->Destroy();
    (this->lp1)->Destroy();
    (this->lp7)->Destroy();
    (this->lp9)->Destroy();
    (this->lp2)->Destroy();
    (this->lp3)->Destroy();
    (this->lp8)->Destroy();
    (this->lp6)->Destroy();
    (this->lp4)->Destroy();

    (this->varrowtbl)->Destroy();
    (this->vshadetbl)->Destroy();
    (this->vcolortbl)->Destroy();
    (this->vlinewidthtbl)->Destroy();
    (this->vrrectcornertbl)->Destroy();
    (this->vtooltbl)->Destroy();
    (this->vcmdtbl)->Destroy();
    (this->vsnapgridtbl)->Destroy();
    (this->vfontsel)->Destroy();

    (this->arrowtbl)->Destroy();
    (this->shadetbl)->Destroy();
    (this->colortbl)->Destroy();
    (this->linewidthtbl)->Destroy();
    (this->rrectcornertbl)->Destroy();
    (this->tooltbl)->Destroy();
    (this->cmdtbl)->Destroy();
    (this->snapgridtbl)->Destroy();
    (this->fontselp)->Destroy();

    free(this->shadeacc);
    free(this->coloracc);
    free(this->arrowacc);
    free(this->linewidthacc);
    free(this->rrectcorneracc);
    free(this->toolacc);
    free(this->cmdacc);
    free(this->snapgridacc);

    for (ix=0; ix<this->colors_num; ix++) 
	if (this->colorlist[ix])
	    free(this->colorlist[ix]);

    free(this->colorlist);
    free(this->arrowlist);
    free(this->linewidthlist);
    free(this->rrectcornerlist);
    free(this->snapgridlist);

    for (ix=0; ix<this->toollistsize; ix++) {
	if (this->dummylist[ix])
	    (this->dummylist[ix])->Destroy();
    }
    free(this->dummylist);
    free(this->rectlist);
    (this->menuatt)->Destroy();
    if (this->tmplist)
	free(this->tmplist);
}

void figtoolview::SetExpertMode(boolean  val)
{
    int ix;

    val = (val) ? TRUE : FALSE;

    if (val==this->expertmode)
	return;

    this->expertmode = val;
    if (this->primaryview)
	(this->primaryview)->SetExpertMode( val);
    (this->vfontsel)->ShowExtraOption();

    if ((this->cmdtbl)->NStrings() == FIGCMDS_NUM) {
	this->cmdacc = (short *)realloc(this->cmdacc, sizeof(short) * (FIGCMDS_NUM+FIGCMDSEX_NUM));
	for (ix=FIGCMDS_NUM; ix<FIGCMDS_NUM+FIGCMDSEX_NUM; ix++) {
	    this->cmdacc[ix] = (this->cmdtbl)->AddString( cmdlayout[ix].name);
	}
	(this->cmdtbl)->ClearBits();
    }

    this->toolextras = TRUE;
    (this->tooltbl)->Clear();
    RebuildToolMenu(this);
    /*this->toolacc = (short *)realloc(this->toolacc, sizeof(short) * (this->toollistnum + FIGTOOLS_NUM + FIGTOOLSEX_NUM));
    for (ix=0; ix<FIGTOOLS_NUM+FIGTOOLSEX_NUM; ix++) {
	this->toolacc[ix] = (this->tooltbl)->AddString( toollayout[ix].name);
    }
    for (ix=0; ix<this->toollistnum; ix++) {
	this->toolacc[ix+FIGTOOLS_NUM+FIGTOOLSEX_NUM] = (this->tooltbl)->AddString( (this->dummylist[ix])->ToolName( this, (this->toollist)[ix].rock));
    }
    (this->tooltbl)->ClearBits();
    if (this->toolnum >= FIGTOOLS_NUM)
	this->toolnum += FIGTOOLSEX_NUM;
    (this->tooltbl)->SetBitOfEntry( this->toolacc[this->toolnum], TRUE);
     */

    RepostMenus(this);
}

static void RebuildToolMenu(class figtoolview *self)
{
    int ix;
    int headnum = (self->expertmode) ? (FIGTOOLS_NUM + FIGTOOLSEX_NUM) : FIGTOOLS_NUM;

    (self->tooltbl)->Clear();
    self->toolacc = (short *)realloc(self->toolacc, sizeof(short) * (self->toollistnum + headnum));
    for (ix=0; ix<headnum; ix++) {
	self->toolacc[ix] = (self->tooltbl)->AddString( toollayout[ix].name);
    }

    for (ix=0; ix<self->toollistsize; ix++) {
	if (self->dummylist[ix]) {
	    self->dummylist[ix]->Destroy();
	    self->dummylist[ix] = NULL;
	}
    }

    for (ix=0; ix<self->toollistnum; ix++) {
	self->dummylist[ix] = (class figobj *)ATK::NewObject(self->toollist[ix].name);
	self->toolacc[ix+headnum] = (self->tooltbl)->AddString( (self->dummylist[ix])->ToolName( self, self->toollist[ix].rock));
    }

    (self->tooltbl)->ClearBits();
    if (self->toolnum >= FIGTOOLS_NUM)
	self->toolnum = 1; /* += FIGTOOLSEX_NUM ### */
    (self->tooltbl)->SetBitOfEntry( self->toolacc[self->toolnum], TRUE);

}

static void SetExpertModeProc(class figtoolview  *self, long  val)
{
    (self)->SetExpertMode( val);
}

/* assumes self is input focus */
static void RepostMenus(class figtoolview  *self)
{
    long menumask = 0;

    if (self->expertmode)
	menumask |= ML_expertmode;

    if (self->creating)
	menumask |= ML_objectcreating;

    if ((self->Menus)->SetMask( menumask)) {
	(self)->PostMenus( NULL);
    }
}

void figtoolview::PostMenus(class menulist  *ml)
{
/* Enable the menus for this object. */

    (this->Menus)->ClearChain();
    if (ml) (this->Menus)->ChainBeforeML( ml, (long)ml);
    (this)->lpair::PostMenus( this->Menus);
}

void figtoolview::PostKeyState(class keystate  *ks)
{
/* Enable the keys for this object. */

    this->Keystate->next = NULL;
    (this->Keystate)->AddBefore( ks);
    (this)->lpair::PostKeyState( this->Keystate);
}

boolean figtoolview::SetPrimaryView(class figview  *view)
{
    int oldval;

    if (this->primaryview) {
	(this)->RemoveObserver( this->primaryview);
	(this->primaryview)->RemoveObserver( this);
    }

    if (!view) { /* odd, but somebody might do it */
	this->primaryview = NULL;
	(this)->SetDataObject( NULL);
	return TRUE;
    }

    this->primaryview = view;
    (this)->SetDataObject( (view)->GetDataObject());
    (this)->AddObserver( view);
    (view)->AddObserver( this);

    oldval = this->toollistsize;
    view->BuildToolList(this->toollist, this->toollistnum, this->toollistsize);
    this->dummylist = (class figobj **)realloc(this->dummylist, this->toollistsize * sizeof(class figobj *));
    for (; oldval < this->toollistsize; oldval++) {
	this->dummylist[oldval] = NULL;
    }
    RebuildToolMenu(this);

    return TRUE;
}

void figtoolview::ObservedChanged(class observable  *observed, long  status)
{
    if (observed == (class observable *)this->primaryview) {
	if (status==observable_OBJECTDESTROYED) {
	    this->primaryview = NULL;
	}
	else {
	}
    }
    else if (observed == (class observable *)this->fontselp) {
	if (status!=observable_OBJECTDESTROYED) {
	    long changemask = 0;
	    /*printf("figtool: observing: %c%s %c%d (%c%d)\n", 
		   (fontsel_IsActive(self->fontselp, fontsel_Family) ? ' ' : '!'), fontsel_GetFamily(self->fontselp), 
		   (fontsel_IsActive(self->fontselp, fontsel_Size) ? ' ' : '!'), fontsel_GetSize(self->fontselp), 
		   (fontsel_IsActive(self->fontselp, fontsel_Style) ? ' ' : '!'), fontsel_GetStyle(self->fontselp));
	    printf("figtool: menuatt was: %c%s %c%d (%c%d)\n", 
		   (figattr_IsActive(self->menuatt, figattr_FontFamily) ? ' ' : '!'), figattr_GetFontFamilyVal(self->menuatt), 
		   (figattr_IsActive(self->menuatt, figattr_FontSize) ? ' ' : '!'), figattr_GetFontSizeVal(self->menuatt), 
		   (figattr_IsActive(self->menuatt, figattr_FontStyle) ? ' ' : '!'), figattr_GetFontStyleVal(self->menuatt));*/

	    if (!(this->fontselp)->IsActive( fontsel_Size)) {
		if ((this->menuatt)->IsActive( figattr_FontSize)) {
		    (this->menuatt)->SetActive( figattr_FontSize, FALSE);
		    changemask |= (1<<figattr_FontSize);
		}
	    }
	    else {
		if (!(this->menuatt)->IsActive( figattr_FontSize) || (this->menuatt)->GetFontSizeVal() != (this->fontselp)->GetSize()) {
		    (this->menuatt)->SetFontSize( (this->fontselp)->GetSize());
		    changemask |= (1<<figattr_FontSize);
		}
	    }

	    if (!(this->fontselp)->IsActive( fontsel_Style)) {
		if ((this->menuatt)->IsActive( figattr_FontStyle)) {
		    (this->menuatt)->SetActive( figattr_FontStyle, FALSE);
		    changemask |= (1<<figattr_FontStyle);
		}
	    }
	    else {
		if (!(this->menuatt)->IsActive( figattr_FontStyle) || (this->menuatt)->GetFontStyleVal() != (this->fontselp)->GetStyle()) {
		    (this->menuatt)->SetFontStyle( (this->fontselp)->GetStyle());
		    changemask |= (1<<figattr_FontStyle);
		}
	    }

	    if (!(this->fontselp)->IsActive( fontsel_Family)) {
		if ((this->menuatt)->IsActive( figattr_FontFamily)) {
		    (this->menuatt)->SetActive( figattr_FontFamily, FALSE);
		    changemask |= (1<<figattr_FontFamily);
		}
	    }
	    else {
		if (!(this->menuatt)->IsActive( figattr_FontFamily) || strcmp((this->menuatt)->GetFontFamilyVal(), (this->fontselp)->GetFamily())) {
		    (this->menuatt)->SetFontFamily( (this->fontselp)->GetFamily());
		    changemask |= (1<<figattr_FontFamily);
		}
	    }

	    if (changemask) {
		AdjustToMenus(this, changemask);
	    }
	}
    }
    else { /* observed = GetDataObject(self), we hope */
	if (status==observable_OBJECTDESTROYED) {
	    fprintf(stderr, "figtoolview: Primary fig dataobject destroyed.\n");
	}
	else {
	    AdjustToSelection(this);
	}
    }
}

void figtoolview::UnlinkTree()
{
    (this)->lpair::UnlinkTree();

  /* This is inappropriate, being unlinked doesn't necessarily imply the view should be destroyed.
   if (!self->moribund) {
	Command_Quit(self, 0);
   }
 */
}

static void SetToolProc(class stringtbl  *st, class figtoolview  *self, short  accnum)
{
    int toolnum, objnum;
    boolean booltmp;
    class figure *fig = (class figure *)(self)->GetDataObject();

    for (toolnum=0; toolnum<NumFigTools(self)+self->toollistnum; toolnum++) {
	if (self->toolacc[toolnum] == accnum) break;
    }
    if (toolnum==NumFigTools(self)+self->toollistnum) {
	return;
    }
    if (toolnum==0) {
	message::DisplayString(self, 10, "Click on a tool to select it; click again to set tool parameters.");
	return;
    }

    if (toolnum >= NumFigTools(self)) 
	objnum = toolnum - NumFigTools(self); 
    else
	objnum = (-1);

    if (fig->ReadOnly && (objnum >= 0 || toollayout[toolnum].writes)) {
	message::DisplayString(self, 10, "Document is read-only.");
	return;
    }

    if (self->toolnum==toolnum) {
	if (objnum == -1)
	    self->toolproc = toollayout[toolnum].proc;
	else
	    self->toolproc = Tool_CreateProc;

	if (objnum >= 0) {
	    (self->dummylist[objnum])->ToolModify( self, (self->toollist)[objnum].rock, FALSE);
	    return;
	}
	else {
	    figtoolview_mod_f prmod;
	    prmod = toollayout[toolnum].procmod;
	    if (prmod) 
		(*prmod)(self, 0);
	    return;
	}
    }
    else {
	/* changed tools */

	self->toolnum = toolnum;
	if (objnum == -1)
	    self->toolproc = toollayout[toolnum].proc;
	else
	    self->toolproc = Tool_CreateProc;

	if (objnum >= 0) {
	    (self->dummylist[objnum])->ToolModify( self, (self->toollist)[objnum].rock, TRUE);
	}

	self->selectdeep = FALSE;
	if (self->toolproc != Tool_CreateProc) {
	    self->LockCreateMode = FALSE;
	    (self->cmdtbl)->SetBitOfEntry( self->cmdacc[FIGCMD_LOCKCREATE], FALSE);
	}

	(self->primaryview)->WantInputFocus( self->primaryview);

	if (self->creating) {
	    /* what's this crap with LockCreateMode? Um, it's a hack. Smile and pretend it tastes good. */
	    booltmp = self->LockCreateMode;
	    self->LockCreateMode = TRUE;
	    (self)->AbortObjectBuilding();
	    self->LockCreateMode = booltmp;
	}

	if (self->toolproc==Tool_CreateProc || self->toolproc==NULL) {
	    (self->primaryview)->ClearSelection();
	    (self->primaryview)->WantUpdate( self->primaryview);
	}
    }

    if (!(st)->GetBitOfEntry( accnum)) {
	(st)->ClearBits();
	(st)->SetBitOfEntry( accnum, TRUE);
    }
}

static void SetShadeProc(class stringtbl  *st, class figtoolview  *self, short  accnum)
{
    int shadenum;

    for (shadenum=0; shadenum<FIGSHADES_NUM; shadenum++) {
	if (self->shadeacc[shadenum] == accnum) break;
    }
    if (shadenum==FIGSHADES_NUM) {
	return;
    }
    if (shadenum==0) {
	message::DisplayString(self, 10, "Click on a shade to select it.");
	return;
    }

    if (!(st)->GetBitOfEntry( accnum)) {
	(st)->ClearBits();
	(st)->SetBitOfEntry( accnum, TRUE);
    }

    /* regardless of whether it's a change */
    self->shadenum = shadenum;
    if (shadenum == figtoolv_shade_Inherit) {
	(self->menuatt)->SetActive( figattr_Shade, FALSE);
    }
    else {
	int val;
	if (shadenum == figtoolv_shade_Clear)
	    val = figattr_ShadeClear;
	else
	    val = shadenum - figtoolv_shade_Zero;
	(self->menuatt)->SetShade( val);
    }

    AdjustToMenus(self, (1<<figattr_Shade));
}

static void SetTextPosProc(class stringtbl  *st, class figtoolview  *self, short  accnum)
{
    int textposnum;

    for (textposnum=0; textposnum<FIGTEXTPOSS_NUM; textposnum++) {
	if (self->textposacc[textposnum] == accnum) break;
    }
    if (textposnum==FIGTEXTPOSS_NUM) {
	return;
    }
    if (textposnum==0) {
	message::DisplayString(self, 10, "Click on a text position to select it.");
	return;
    }

    if (!(st)->GetBitOfEntry( accnum)) {
	(st)->ClearBits();
	(st)->SetBitOfEntry( accnum, TRUE);
    }

    /* regardless of whether it's a change */
    self->textposnum = textposnum;
    if (textposnum == figtoolv_textpos_Inherit) {
	(self->menuatt)->SetActive( figattr_TextPos, FALSE);
    }
    else {
	int val;
	val = textposnum - figtoolv_textpos_Zero;
	(self->menuatt)->SetTextPos( val);
    }

    AdjustToMenus(self, (1<<figattr_TextPos));
}

static void SetLineStyleProc(class stringtbl  *st, class figtoolview  *self, short  accnum)
{
    int linestylenum;

    for (linestylenum=0; linestylenum<FIGLINESTYLES_NUM; linestylenum++) {
	if (self->linestyleacc[linestylenum] == accnum) break;
    }
    if (linestylenum==FIGLINESTYLES_NUM) {
	return;
    }
    if (linestylenum==0) {
	message::DisplayString(self, 10, "Click on a line style to select it.");
	return;
    }

    if (!(st)->GetBitOfEntry( accnum)) {
	(st)->ClearBits();
	(st)->SetBitOfEntry( accnum, TRUE);
    }

    /* regardless of whether it's a change */
    self->linestylenum = linestylenum;
    if (linestylenum == figtoolv_linestyle_Inherit) {
	(self->menuatt)->SetActive( figattr_LineStyle, FALSE);
    }
    else {
	int val;
	val = linestylenum - figtoolv_linestyle_Zero;
	(self->menuatt)->SetLineStyle( val);
    }

    AdjustToMenus(self, (1<<figattr_LineStyle));
}

static void InsertLineWidth(class figtoolview  *self, short  val)
{
    class stringtbl *st = self->linewidthtbl;   
    char name[24];
    int ix;

    ix = self->linewidths_num;
    self->linewidthacc = (short *)realloc(self->linewidthacc, sizeof(short) * (1+self->linewidths_num));
    self->linewidthlist = (short *)realloc(self->linewidthlist, sizeof(short) * (1+self->linewidths_num));

    sprintf(name, "%d", val);
    self->linewidthacc[ix] = (st)->AddString( name);
    if (self->linewidthacc[ix] == (-1))
	return; 
    self->linewidthlist[ix] = val;

    self->linewidths_num++;
}

static void SetLineWidthProc(class stringtbl  *st, class figtoolview  *self, short  accnum)
{
    int linewidthnum;

    for (linewidthnum=0; linewidthnum<self->linewidths_num; linewidthnum++) {
	if (self->linewidthacc[linewidthnum] == accnum) break;
    }
    if (linewidthnum==self->linewidths_num) {
	return;
    }
    if (linewidthnum==0) {
	message::DisplayString(self, 10, "Click on a line width to select it.");
	return;
    }

    if (linewidthnum == figtoolv_linewidth_Other) {
	int ix;
	short val;
	char buffer[32];
	int res;

	res = message::AskForString (self, 40, "Enter a new line width:  ", NULL, buffer, 30); 
	if (res<0 || strlen(buffer)==0) {
	    message::DisplayString(self, 10, "Cancelled.");
	    return;
	}
	val = atoi(buffer);

	if (val<0) {
	    message::DisplayString(self, 10, "Value must be a positive integer.");
	    return;
	}
	if (val>64) {
	    message::DisplayString(self, 10, "That value is too large.");
	    return;
	}

	for (ix=figtoolv_linewidth_Zero; ix<self->linewidths_num; ix++)
	    if (self->linewidthlist[ix] == val) {
		message::DisplayString(self, 10, "That value is already available.");
		linewidthnum = ix;
		break;
	    }
	if (linewidthnum == figtoolv_linewidth_Other) {
	    InsertLineWidth(self, val);

	    for (linewidthnum=figtoolv_linewidth_Zero; linewidthnum<self->linewidths_num; linewidthnum++) {
		if (self->linewidthlist[linewidthnum] == val) break;
	    }
	    if (linewidthnum==self->linewidths_num) {
		return;
	    }
	}
	accnum = self->linewidthacc[linewidthnum];
    }

    if (!(st)->GetBitOfEntry( accnum)) {
	(st)->ClearBits();
	(st)->SetBitOfEntry( accnum, TRUE);
    }

    /* regardless of whether it's a change */
    self->linewidthnum = linewidthnum;
    if (linewidthnum == figtoolv_linewidth_Inherit) {
	(self->menuatt)->SetActive( figattr_LineWidth, FALSE);
    }
    else {
	(self->menuatt)->SetLineWidth( self->linewidthlist[linewidthnum]);
    }

    AdjustToMenus(self, (1<<figattr_LineWidth));
}

static void InsertArrowSize(class figtoolview  *self, short  val)
{
    class stringtbl *st = self->arrowtbl;   
    char name[24];
    int ix;

    ix = self->arrows_num;
    self->arrowacc = (short *)realloc(self->arrowacc, sizeof(short) * (1+self->arrows_num));
    self->arrowlist = (short *)realloc(self->arrowlist, sizeof(short) * (1+self->arrows_num));

    sprintf(name, "%d", val);
    self->arrowacc[ix] = (st)->AddString( name);
    if (self->arrowacc[ix] == (-1))
	return; 
    self->arrowlist[ix] = val;

    self->arrows_num++;
}

static void SetArrowProc(class stringtbl  *st, class figtoolview  *self, short  accnum)
{
    int arrownum;

    for (arrownum=0; arrownum<self->arrows_num; arrownum++) {
	if (self->arrowacc[arrownum] == accnum) break;
    }
    if (arrownum==self->arrows_num) {
	return;
    }
    if (arrownum==0) {
	message::DisplayString(self, 10, "Click on a size or position to select it.");
	return;
    }

    if (arrownum == figtoolv_arrow_Other) {
	int ix;
	short val;
	char buffer[32];
	int res;

	res = message::AskForString (self, 40, "Enter a new arrow size:  ", NULL, buffer, 30); 
	if (res<0 || strlen(buffer)==0) {
	    message::DisplayString(self, 10, "Cancelled.");
	    return;
	}
	val = atoi(buffer);

	if (val<=0) {
	    message::DisplayString(self, 10, "Value must be a positive integer.");
	    return;
	}
	if (val>64) {
	    message::DisplayString(self, 10, "That value is too large.");
	    return;
	}

	for (ix=figtoolv_arrow_Zero; ix<self->arrows_num; ix++)
	    if (self->arrowlist[ix] == val) {
		message::DisplayString(self, 10, "That value is already available.");
		arrownum = ix;
		break;
	    }
	if (arrownum == figtoolv_arrow_Other) {
	    InsertArrowSize(self, val);

	    for (arrownum=figtoolv_arrow_Zero; arrownum<self->arrows_num; arrownum++) {
		if (self->arrowlist[arrownum] == val) break;
	    }
	    if (arrownum==self->arrows_num) {
		return;
	    }
	}
	accnum = self->arrowacc[arrownum];
    }

    if (arrownum >= figtoolv_arrowshape_Inherit && arrownum < figtoolv_arrowsize_Inherit) {

	int ix;
	short val, res;

	if (!(st)->GetBitOfEntry( accnum)) {
	    for (ix=figtoolv_arrowshape_Zero; ix<figtoolv_arrowsize_Inherit; ix++) {
		val = self->arrowacc[ix];
		if (val == accnum) 
		    (st)->SetBitOfEntry( val, TRUE);
		else
		    (st)->SetBitOfEntry( val, FALSE);
	    }
	    val = self->arrowacc[figtoolv_arrowshape_Inherit];
	    if (val == accnum) 
		(st)->SetBitOfEntry( val, TRUE);
	    else
		(st)->SetBitOfEntry( val, FALSE);
	}

	/* regardless of whether it's a change */
	self->arrownum = arrownum;
	if (arrownum == figtoolv_arrowshape_Inherit) {
	    (self->menuatt)->SetActive( figattr_Arrow, FALSE);
	}
	else {
	    (self->menuatt)->SetArrow( self->arrowlist[arrownum]);
	}

	AdjustToMenus(self, (1<<figattr_Arrow));
    }
    else if (!(arrownum == figtoolv_arrowpos_Head || arrownum == figtoolv_arrowpos_Tail || arrownum == figtoolv_arrowpos_Inherit)) {
	/* accnum may have been rediddled */
	int ix;
	short val, res;

	if (!(st)->GetBitOfEntry( accnum)) {
	    for (ix=figtoolv_arrow_Zero; ix<self->arrows_num; ix++) {
		val = self->arrowacc[ix];
		if (val == accnum) 
		    (st)->SetBitOfEntry( val, TRUE);
		else
		    (st)->SetBitOfEntry( val, FALSE);
	    }
	    val = self->arrowacc[figtoolv_arrowsize_Inherit];
	    if (val == accnum) 
		(st)->SetBitOfEntry( val, TRUE);
	    else
		(st)->SetBitOfEntry( val, FALSE);
	}

	/* regardless of whether it's a change */
	self->arrowsizenum = arrownum;
	if (arrownum == figtoolv_arrowsize_Inherit) {
	    (self->menuatt)->SetActive( figattr_ArrowSize, FALSE);
	}
	else {
	    (self->menuatt)->SetArrowSize( self->arrowlist[arrownum]);
	}

	AdjustToMenus(self, (1<<figattr_ArrowSize));
    }
    else {
	short val, res;
	boolean oldflag;
	if (arrownum == figtoolv_arrowpos_Inherit) {
	    (st)->SetBitOfEntry( accnum, TRUE);
	    val = self->arrowacc[figtoolv_arrowpos_Head];
	    (st)->SetBitOfEntry( val, FALSE);
	    val = self->arrowacc[figtoolv_arrowpos_Tail];
	    (st)->SetBitOfEntry( val, FALSE);
	    (self->menuatt)->SetActive( figattr_ArrowPos, FALSE);
	}
	else {
	    oldflag = !(st)->GetBitOfEntry(accnum);
	    (st)->SetBitOfEntry( accnum, oldflag);
	    val = self->arrowacc[figtoolv_arrowpos_Inherit];
	    (st)->SetBitOfEntry( val, FALSE);
	    res = 0;
	    val = self->arrowacc[figtoolv_arrowpos_Head];
	    if ((st)->GetBitOfEntry(val))
		res |= figattr_ArrowHead;
	    val = self->arrowacc[figtoolv_arrowpos_Tail];
	    if ((st)->GetBitOfEntry(val))
		res |= figattr_ArrowTail;
	    (self->menuatt)->SetArrowPos(res);
	}
	AdjustToMenus(self, (1<<figattr_ArrowPos));
    }
}

static void InsertRRectCorner(class figtoolview  *self, short  val)
{
    class stringtbl *st = self->rrectcornertbl;   
    char name[24];
    int ix;

    ix = self->rrectcorners_num;
    self->rrectcorneracc = (short *)realloc(self->rrectcorneracc, sizeof(short) * (1+self->rrectcorners_num));
    self->rrectcornerlist = (short *)realloc(self->rrectcornerlist, sizeof(short) * (1+self->rrectcorners_num));

    sprintf(name, "%d", val);
    self->rrectcorneracc[ix] = (st)->AddString( name);
    if (self->rrectcorneracc[ix] == (-1))
	return; 
    self->rrectcornerlist[ix] = val;

    self->rrectcorners_num++;
}

static void SetRRectCornerProc(class stringtbl  *st, class figtoolview  *self, short  accnum)
{
    int rrectcornernum;

    for (rrectcornernum=0; rrectcornernum<self->rrectcorners_num; rrectcornernum++) {
	if (self->rrectcorneracc[rrectcornernum] == accnum) break;
    }
    if (rrectcornernum==self->rrectcorners_num) {
	return;
    }
    if (rrectcornernum==0) {
	message::DisplayString(self, 10, "Click on a corner width to select it.");
	return;
    }

    if (rrectcornernum == figtoolv_rrectcorner_Other) {
	int ix;
	short val;
	char buffer[32];
	int res;

	res = message::AskForString (self, 40, "Enter a new corner width:  ", NULL, buffer, 30); 
	if (res<0 || strlen(buffer)==0) {
	    message::DisplayString(self, 10, "Cancelled.");
	    return;
	}
	val = atoi(buffer);

	if (val<0) {
	    message::DisplayString(self, 10, "Value must be a non-negative integer.");
	    return;
	}
	if (val>100) {
	    message::DisplayString(self, 10, "That value is too large.");
	    return;
	}

	for (ix=figtoolv_rrectcorner_Zero; ix<self->rrectcorners_num; ix++)
	    if (self->rrectcornerlist[ix] == val) {
		message::DisplayString(self, 10, "That value is already available.");
		rrectcornernum = ix;
		break;
	    }
	if (rrectcornernum == figtoolv_rrectcorner_Other) {
	    InsertRRectCorner(self, val);

	    for (rrectcornernum=figtoolv_rrectcorner_Zero; rrectcornernum<self->rrectcorners_num; rrectcornernum++) {
		if (self->rrectcornerlist[rrectcornernum] == val) break;
	    }
	    if (rrectcornernum==self->rrectcorners_num) {
		return;
	    }
	}
	accnum = self->rrectcorneracc[rrectcornernum];
    }

    if (!(st)->GetBitOfEntry( accnum)) {
	(st)->ClearBits();
	(st)->SetBitOfEntry( accnum, TRUE);
    }

    /* regardless of whether it's a change */
    self->rrectcornernum = rrectcornernum;
    if (rrectcornernum == figtoolv_rrectcorner_Inherit) {
	(self->menuatt)->SetActive( figattr_RRectCorner, FALSE);
    }
    else {
	(self->menuatt)->SetRRectCorner( self->rrectcornerlist[rrectcornernum]);
    }

    AdjustToMenus(self, (1<<figattr_RRectCorner));
}

static void InsertColor(class figtoolview  *self, char  *val)
{
    class stringtbl *st = self->colortbl;   
    int ix;

    ix = self->colors_num;
    self->coloracc = (short *)realloc(self->coloracc, sizeof(short) * (1+self->colors_num));
    self->colorlist = (char **)realloc(self->colorlist, sizeof(char *) * (1+self->colors_num));

    self->coloracc[ix] = (st)->AddString( val);
    if (self->coloracc[ix] == (-1))
	return; 
    self->colorlist[ix] = CopyString(val);

    self->colors_num++;
}

static void SetColorProc(class stringtbl  *st, class figtoolview  *self, short  accnum)
{
    int colornum;

    for (colornum=0; colornum<self->colors_num; colornum++) {
	if (self->coloracc[colornum] == accnum) break;
    }
    if (colornum==self->colors_num) {
	return;
    }
    if (colornum==0) {
	message::DisplayString(self, 10, "Click on a color to select it.");
	return;
    }

    if (colornum == figtoolv_color_Other) {
	int ix;
	char buffer[64];
	char *val;
	int res;

	res = message::AskForString (self, 40, "Enter a new color:  ", NULL, buffer, 62); 
	if (res<0 || strlen(buffer)==0) {
	    message::DisplayString(self, 10, "Cancelled.");
	    return;
	}
	LowerString(buffer);
	val = WhiteKillString(buffer);
	if (strlen(val)==0) {
	    message::DisplayString(self, 10, "Cancelled.");
	    return;
	}

	for (ix=figtoolv_color_Zero; ix<self->colors_num; ix++)
	    if (!strcmp(self->colorlist[ix], val)) {
		message::DisplayString(self, 10, "That value is already available.");
		colornum = ix;
		break;
	    }
	if (colornum == figtoolv_color_Other) {
	    InsertColor(self, val);

	    for (colornum=figtoolv_color_Zero; colornum<self->colors_num; colornum++) {
		if (!strcmp(self->colorlist[colornum], val)) break;
	    }
	    if (colornum==self->colors_num) {
		return;
	    }
	}
	accnum = self->coloracc[colornum];
    }

    if (!(st)->GetBitOfEntry( accnum)) {
	(st)->ClearBits();
	(st)->SetBitOfEntry( accnum, TRUE);
    }

    /* regardless of whether it's a change */
    self->colornum = colornum;
    if (colornum == figtoolv_color_Inherit) {
	(self->menuatt)->SetActive( figattr_Color, FALSE);
    }
    else {
	(self->menuatt)->SetColor( self->colorlist[colornum]);
    }

    AdjustToMenus(self, (1<<figattr_Color));
}

static void InsertSnapGrid(class figtoolview  *self, short  val)
{
    class stringtbl *st = self->snapgridtbl;   
    char name[24];
    int ix;
    double convert;

    convert = figview_UnitConvert(self->snapgridunit);

    ix = self->snapgrids_num;
    self->snapgridacc = (short *)realloc(self->snapgridacc, sizeof(short) * (1+self->snapgrids_num));
    self->snapgridlist = (short *)realloc(self->snapgridlist, sizeof(short) * (1+self->snapgrids_num));

    sprintf(name, "%.3g", (double)val / convert);
    self->snapgridacc[ix] = (st)->AddString( name);
    if (self->snapgridacc[ix] == (-1))
	return; 
    self->snapgridlist[ix] = val;

    self->snapgrids_num++;
}

static void SetSnapGridProc(class stringtbl  *st, class figtoolview  *self, short  accnum)
{
    int snapgridnum;
    int ix;
    char buffer[16];
    double convert;

    for (snapgridnum=0; snapgridnum<self->snapgrids_num; snapgridnum++) {
	if (self->snapgridacc[snapgridnum] == accnum) break;
    }
    if (snapgridnum==self->snapgrids_num) {
	return;
    }
    if (snapgridnum==0) {
	message::DisplayString(self, 10, "Click on a size to select it.");
	return;
    }
    if (snapgridnum < figtoolv_snapgrid_Other) {
	if (snapgridnum != self->snapgridunit) {
	    self->snapgridunit = snapgridnum;
	    convert = figview_UnitConvert(self->snapgridunit);
	    for (ix=figtoolv_snapgrid_Zero; ix<self->snapgrids_num; ix++)
		(st)->RemoveEntry( self->snapgridacc[ix]); 
	    for (ix=figtoolv_snapgrid_Zero; ix<self->snapgrids_num; ix++) {
		if (self->snapgridlist[ix]==0)
		    strcpy(buffer, "snap off");
		else
		    sprintf(buffer, "%.3g", (double)self->snapgridlist[ix] / convert);
		self->snapgridacc[ix] = (st)->AddString( buffer);
	    }

	    (st)->ClearBits();
	    (st)->SetBitOfEntry( self->snapgridacc[self->snapgridnum], TRUE);
	    (st)->SetBitOfEntry( accnum, TRUE);
	}
	return;
    }

    if (snapgridnum == figtoolv_snapgrid_Other) {
	double val;
	long ival;
	char buffer[32];
	char obuffer[256];
	int res;

	convert = figview_UnitConvert(self->snapgridunit);

	sprintf(obuffer, "Enter a new grid size (in %s):  ", snapgridlayout[self->snapgridunit].name);

	res = message::AskForString (self, 40, obuffer, NULL, buffer, 30); 
	if (res<0 || strlen(buffer)==0) {
	    message::DisplayString(self, 10, "Cancelled.");
	    return;
	}
	val = atof(buffer);
	
	if (val<=0.0) {
	    message::DisplayString(self, 10, "Value must be a positive number.");
	    return;
	}

	ival = (long)(val * convert);

	for (ix=figtoolv_snapgrid_Zero; ix<self->snapgrids_num; ix++)
	    if (self->snapgridlist[ix] == ival) {
		message::DisplayString(self, 10, "That value is already available.");
		snapgridnum = ix;
		break;
	    }
	if (snapgridnum == figtoolv_snapgrid_Other) {
	    InsertSnapGrid(self, ival);

	    for (snapgridnum=figtoolv_snapgrid_Zero; snapgridnum<self->snapgrids_num; snapgridnum++) {
		if (self->snapgridlist[snapgridnum] == ival) break;
	    }
	    if (snapgridnum==self->snapgrids_num) {
		return;
	    }
	}
	accnum = self->snapgridacc[snapgridnum];
    }

    if (!(st)->GetBitOfEntry( accnum)) {
	(st)->ClearBits();
	(st)->SetBitOfEntry( accnum, TRUE);
	(st)->SetBitOfEntry( self->snapgridacc[self->snapgridunit], TRUE);
    }

    /* regardless of whether it's a change */
    self->snapgridnum = snapgridnum;
    self->snapgrid = self->snapgridlist[self->snapgridnum];
}

static void CallCmdProc(class stringtbl  *st, class figtoolview  *self, short  accnum)
{
    int cmdnum;
    figtoolview_comm_f comproc;

    for (cmdnum=0; cmdnum<(self->cmdtbl)->NStrings(); cmdnum++) {
	if (self->cmdacc[cmdnum] == accnum) break;
    }
    if (cmdnum==(self->cmdtbl)->NStrings()) {
	return;
    }
    if (cmdnum==0) {
	message::DisplayString(self, 10, "Click on any of these commands.");
	return;
    }

    comproc = cmdlayout[cmdnum].proc;
    if (comproc)
	(*comproc)(self, cmdlayout[cmdnum].rock);
}

static void Command_Quit(class figtoolview  *self, char  *rock)
{
    if (self->primaryview) {
	(self->primaryview)->DestroyToolset();
    }
    else {
	fprintf(stderr, "figtoolview: warning: no primary figview.\n");
	(self)->Destroy();
    }
}

static boolean CSA_Splot(class figobj  *o, long  ref, class figure  *fig, class figview  *figv)
{
    (figv)->SelectByRef( ref);
    return FALSE;
}

static void Command_LockCreate(class figtoolview  *self, char  *rock)
{
    if (!self->LockCreateMode) {
	if (self->toolproc != Tool_CreateProc) {
	    message::DisplayString(self, 10, "Not in an object-creation mode.");
	}
	else {
	    self->LockCreateMode = TRUE;
	    (self->cmdtbl)->SetBitOfEntry( self->cmdacc[FIGCMD_LOCKCREATE], TRUE);
	    message::DisplayString(self, 10, "Will stay in object-creation mode until told otherwise.");
	}
    }
    else {
	self->LockCreateMode = FALSE;
	(self->cmdtbl)->SetBitOfEntry( self->cmdacc[FIGCMD_LOCKCREATE], FALSE);
	if (self->toolproc == Tool_CreateProc) {
	    message::DisplayString(self, 10, "Will return to reshape mode after next object.");
	}
    }
}

static void Command_CutNPaste(class figtoolview  *self, long  rock)
{
    class figview *figv = self->primaryview;
    if (!figv) return;

    (figv)->CutNPaste( rock, 0);
}

static void Command_SelectAll(class figtoolview  *self, char  *rock)
{
    class figview *figv = self->primaryview;
    class figure *fig;
    if (!figv) return;
    fig = (class figure *)(figv)->GetDataObject();
    if (!fig) return;

    (figv)->ClearSelection();
    (fig)->EnumerateObjectGroup( (figv)->GetFocusRef(), NULL, FALSE, (figure_eofptr)CSA_Splot, (long)figv);
    (figv)->WantUpdate( figv);

    AdjustToSelection(self);
}

static void Command_Refresh(class figtoolview  *self, char  *rock)
{
    class figview *figv = self->primaryview;

    if (!figv) return;

    figv->NeedFullUpdate = TRUE;
    (figv)->WantUpdate( figv);
}

static void Command_Zoom(class figtoolview  *self, long  rock)
{
    class figview *figv = self->primaryview;

    if (!figv) return;
    (figv)->ChangeZoom( rock);
}

static void Command_PanToOrigin(class figtoolview  *self, long  rock)
{
    class figview *figv = self->primaryview;

    if (!figv) return;
    if (figv->panx==0 && figv->pany==0)
	return;

    figv->panx = 0;
    figv->pany = 0;
    figv->NeedFullUpdate = TRUE;
    (figv)->WantUpdate( figv);
}

static void ATSPSplot(class figobj  *o, long  ref, class figview  *vv, class figattr *attr)
{
    (o)->UpdateVAttributes( attr, figattr_MaskAll);
}

static void ApplyToSelProc(class figtoolview  *self, long  rock)
{
    class figure *fig = (class figure *)(self->primaryview)->GetDataObject();
    if (!fig) return;

    if ((self->primaryview)->GetNumSelected() == 0) {
	message::DisplayString(self, 10, "No objects are selected in the document.");
	return;
    }

    (self->primaryview)->EnumerateSelection((figview_esfptr)ATSPSplot, (long)self->menuatt);
    (fig)->NotifyObservers( figure_DATACHANGED);
}

static void AdjustToMenus(class figtoolview  *self, long  mask)
{
    class figure *fig = (class figure *)(self)->GetDataObject();
    boolean changed = FALSE;
    long ref;
    class figobj *o;
    if (!fig) return;

    ref = (self->primaryview)->GetOneSelected(); 
    if (ref != figure_NULLREF) {
	o = (fig)->FindObjectByRef( ref);
	(o)->UpdateVAttributes( self->menuatt, mask);
	changed = TRUE;
    }
    if (self->creating) {
	(self->creating)->UpdateVAttributes( self->menuatt, mask);
	changed = TRUE;
    }

    if (changed)
	(fig)->NotifyObservers( figure_DATACHANGED);
}

static void AdjustToSelection(class figtoolview  *self)
{
    long ref = (self->primaryview)->GetOneSelected();
    long ix, vnum, accnum;
    char *cx;
    class figobj *o;
    boolean fselchanged = FALSE;

    if (ref == figure_NULLREF)
	return; /* more or less than 1 selected */

    o = self->primaryview->objs[ref].o;

    (self->menuatt)->CopyData( (o)->GetVAttributes(), figattr_MaskAll);

    if (!(self->menuatt)->IsActive( figattr_Shade))
	vnum = figtoolv_shade_Inherit;
    else {
	ix = (self->menuatt)->GetShadeVal();
	if (ix==figattr_ShadeClear)
	    vnum = figtoolv_shade_Clear;
	else
	    vnum = ix + figtoolv_shade_Zero;
    }
    accnum = self->shadeacc[vnum];
    if (!(self->shadetbl)->GetBitOfEntry( accnum)) {
	(self->shadetbl)->ClearBits();
	(self->shadetbl)->SetBitOfEntry( accnum, TRUE);
    }

    if (!(self->menuatt)->IsActive( figattr_TextPos))
	vnum = figtoolv_textpos_Inherit;
    else {
	ix = (self->menuatt)->GetTextPosVal();
	vnum = ix + figtoolv_textpos_Zero;
    }
    accnum = self->textposacc[vnum];
    if (!(self->textpostbl)->GetBitOfEntry( accnum)) {
	(self->textpostbl)->ClearBits();
	(self->textpostbl)->SetBitOfEntry( accnum, TRUE);
    }

    if (!(self->menuatt)->IsActive( figattr_LineStyle))
	vnum = figtoolv_linestyle_Inherit;
    else {
	ix = (self->menuatt)->GetLineStyleVal();
	vnum = ix + figtoolv_linestyle_Zero;
    }
    accnum = self->linestyleacc[vnum];
    if (!(self->linestyletbl)->GetBitOfEntry( accnum)) {
	(self->linestyletbl)->ClearBits();
	(self->linestyletbl)->SetBitOfEntry( accnum, TRUE);
    }

    if (!(self->menuatt)->IsActive( figattr_LineWidth))
	vnum = figtoolv_linewidth_Inherit;
    else {
	ix = (self->menuatt)->GetLineWidthVal();
	for (vnum=figtoolv_linewidth_Zero; vnum<self->linewidths_num; vnum++)
	    if (self->linewidthlist[vnum] == ix) break;
	if (vnum==self->linewidths_num) {
	    InsertLineWidth(self, ix);
	    vnum = self->linewidths_num - 1;
	}
    }
    accnum = self->linewidthacc[vnum];
    if (!(self->linewidthtbl)->GetBitOfEntry( accnum)) {
	(self->linewidthtbl)->ClearBits();
	(self->linewidthtbl)->SetBitOfEntry( accnum, TRUE);
    }

    /* arrow size stuff */
    if (!(self->menuatt)->IsActive( figattr_ArrowSize))
	vnum = figtoolv_arrowsize_Inherit;
    else {
	ix = (self->menuatt)->GetArrowSizeVal();
	for (vnum=figtoolv_arrow_Zero; vnum<self->arrows_num; vnum++)
	    if (self->arrowlist[vnum] == ix) break;
	if (vnum==self->arrows_num) {
	    InsertArrowSize(self, ix);
	    vnum = self->arrows_num - 1;
	}
    }
    accnum = self->arrowacc[vnum];
    if (!(self->arrowtbl)->GetBitOfEntry( accnum)) {
	for (ix = figtoolv_arrowsize_Inherit; ix < self->arrows_num; ix++)
	    (self->arrowtbl)->SetBitOfEntry( self->arrowacc[ix], (ix==vnum));
    }
    /* arrow shape */
    if (!(self->menuatt)->IsActive( figattr_Arrow))
	vnum = figtoolv_arrowshape_Inherit;
    else {
	ix = (self->menuatt)->GetArrowVal();
	for (vnum=figtoolv_arrowshape_Zero; vnum<figtoolv_arrowsize_Inherit; vnum++)
	    if (self->arrowlist[vnum] == ix) break;
    }
    accnum = self->arrowacc[vnum];
    if (!(self->arrowtbl)->GetBitOfEntry( accnum)) {
	for (ix = figtoolv_arrowshape_Inherit; ix < figtoolv_arrowsize_Inherit; ix++)
	    (self->arrowtbl)->SetBitOfEntry( self->arrowacc[ix], (ix==vnum));
    }
    /* arrow pos */
    if (!(self->menuatt)->IsActive( figattr_ArrowPos)) {
	vnum = figtoolv_arrowpos_Inherit;
	(self->arrowtbl)->SetBitOfEntry( self->arrowacc[vnum], TRUE);
	vnum = figtoolv_arrowpos_Head;
	(self->arrowtbl)->SetBitOfEntry( self->arrowacc[vnum], FALSE);
	vnum = figtoolv_arrowpos_Tail;
	(self->arrowtbl)->SetBitOfEntry( self->arrowacc[vnum], FALSE);
    }
    else {
	ix = (self->menuatt)->GetArrowPosVal();
	vnum = figtoolv_arrowpos_Inherit;
	(self->arrowtbl)->SetBitOfEntry( self->arrowacc[vnum], FALSE);
	vnum = figtoolv_arrowpos_Head;
	(self->arrowtbl)->SetBitOfEntry( self->arrowacc[vnum], ((ix & figattr_ArrowHead) ? TRUE : FALSE));
	vnum = figtoolv_arrowpos_Tail;
	(self->arrowtbl)->SetBitOfEntry( self->arrowacc[vnum], ((ix & figattr_ArrowTail) ? TRUE : FALSE));
    }

    /* rrect stuff */
    if (!(self->menuatt)->IsActive( figattr_RRectCorner))
	vnum = figtoolv_rrectcorner_Inherit;
    else {
	ix = (self->menuatt)->GetRRectCornerVal();
	for (vnum=figtoolv_rrectcorner_Zero; vnum<self->rrectcorners_num; vnum++)
	    if (self->rrectcornerlist[vnum] == ix) break;
	if (vnum==self->rrectcorners_num) {
	    InsertRRectCorner(self, ix);
	    vnum = self->rrectcorners_num - 1;
	}
    }
    accnum = self->rrectcorneracc[vnum];
    if (!(self->rrectcornertbl)->GetBitOfEntry( accnum)) {
	(self->rrectcornertbl)->ClearBits();
	(self->rrectcornertbl)->SetBitOfEntry( accnum, TRUE);
    }

    if (!(self->menuatt)->IsActive( figattr_Color))
	vnum = figtoolv_color_Inherit;
    else {
	cx = (self->menuatt)->GetColorVal();
	for (vnum=figtoolv_color_Zero; vnum<self->colors_num; vnum++)
	    if (!strcmp(self->colorlist[vnum], cx)) break;
	if (vnum==self->colors_num) {
	    InsertColor(self, cx);
	    vnum = self->colors_num - 1;
	}
    }
    accnum = self->coloracc[vnum];
    if (!(self->colortbl)->GetBitOfEntry( accnum)) {
	(self->colortbl)->ClearBits();
	(self->colortbl)->SetBitOfEntry( accnum, TRUE);
    }

    if (!(self->menuatt)->IsActive( figattr_FontFamily)) {
	if ((self->fontselp)->IsActive( fontsel_Family)) {
	    (self->fontselp)->UnsetFamily();
	    fselchanged = TRUE;
	}
    }
    else {
	if (!(self->fontselp)->IsActive( fontsel_Family)
	    || strcmp((self->menuatt)->GetFontFamilyVal(), (self->fontselp)->GetFamily())) {
	    (self->fontselp)->SetFamily( (self->menuatt)->GetFontFamilyVal());
	    fselchanged = TRUE;
	}
    }

    if (!(self->menuatt)->IsActive( figattr_FontSize)) {
	if ((self->fontselp)->IsActive( fontsel_Size)) {
	    (self->fontselp)->UnsetSize();
	    fselchanged = TRUE;
	}
    }
    else {
	if (!(self->fontselp)->IsActive( fontsel_Size)
	    || ((self->menuatt)->GetFontSizeVal() != (self->fontselp)->GetSize())) {
	    (self->fontselp)->SetSize( (self->menuatt)->GetFontSizeVal());
	    fselchanged = TRUE;
	}
    }

    if (!(self->menuatt)->IsActive( figattr_FontStyle)) {
	if ((self->fontselp)->IsActive( fontsel_Style)) {
	    (self->fontselp)->UnsetStyle();
	    fselchanged = TRUE;
	}
    }
    else {
	if (!(self->fontselp)->IsActive( fontsel_Style)
	    || ((self->menuatt)->GetFontStyleVal() != (self->fontselp)->GetStyle())) {
	    (self->fontselp)->SetStyle( (self->menuatt)->GetFontStyleVal());
	    fselchanged = TRUE;
	}
    }

    if (fselchanged) {
	(self->fontselp)->NotifyObservers( fontsel_DATACHANGED);
    }
    /* ##new */
}

static void IncreaseTmpProc(class figtoolview  *self, long  num)
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

static void CacheSelectProc(class figobj  *o, long  ref, class figview  *vv, class figtoolview  *figt)
{
    int ix;
    class figure *fig = (class figure *)(vv)->GetDataObject();

    if (ref==vv->focusgroup || ref==(fig)->RootObjRef())
	return;

    ix = figt->tmpnum;
    IncreaseTmpProc(figt, ix+1);
    figt->tmplist[ix] = ref;
    figt->tmpnum = ix+1;
}

static void Command_GroupSel(class figtoolview  *self, char  *rock)
{
    class figview *figv = self->primaryview;
    class figure *fig;
    int ix;
    long foc, grpr;
    class figogrp *grp;

    if (!figv) return;
    fig = (class figure *)(figv)->GetDataObject();
    if (!fig) return;

    foc = (figv)->GetFocusRef();

    self->tmpnum = 0;
    (figv)->EnumerateSelection((figview_esfptr) CacheSelectProc, (long)self);

    if (self->tmpnum==0) {
	message::DisplayString(figv, 10, "No objects selected.");
	return;
    }

    grp = new figogrp;
    ((grp)->GetVAttributes())->CopyData( self->menuatt, figattr_MaskAll);
    grpr = (fig)->InsertObject( grp, foc, -1);

    for (ix=0; ix<self->tmpnum; ix++)
	(fig)->UnlinkObjectByRef( self->tmplist[ix]);
    for (ix=0; ix<self->tmpnum; ix++)
	(fig)->LinkObjectByRef( self->tmplist[ix], grpr, -1);

    (figv)->FlushDataChanges();
    (figv)->ClearSelection();
    (figv)->SelectByRef( grpr); 
    AdjustToSelection(self);
    (fig)->NotifyObservers( figure_DATACHANGED);
}

static boolean CacheContentsProc(class figobj  *o, long  ref, class figure  *fig, class figtoolview  *figt)
{
    int ix;

    ix = figt->tmpnum;
    IncreaseTmpProc(figt, ix+1);
    figt->tmplist[ix] = ref;
    figt->tmpnum = ix+1;
    return FALSE;
}

static void Command_UngroupSel(class figtoolview  *self, char  *rock)
{
    class figview *figv = self->primaryview;
    class figure *fig;
    int ix;
    long grpr, foc;
    class figogrp *grp;

    if (!figv) return;
    fig = (class figure *)(figv)->GetDataObject();
    if (!fig) return;

    grpr = (figv)->GetOneSelected();
    if (grpr == figure_NULLREF || !((fig)->FindObjectByRef( grpr))->IsGroup()) {
	message::DisplayString(figv, 10, "You must select exactly one group to disassemble.");
	return;
    }
    grp = (class figogrp *)(fig)->FindObjectByRef( grpr);
    foc = (grp)->GetParentRef();

    self->tmpnum = 0;
    (fig)->EnumerateObjectGroup( grpr, NULL, FALSE, (figure_eofptr)CacheContentsProc, (long)self);

    for (ix=0; ix<self->tmpnum; ix++)
	(fig)->UnlinkObjectByRef( self->tmplist[ix]);
    for (ix=0; ix<self->tmpnum; ix++)
	(fig)->LinkObjectByRef( self->tmplist[ix], foc, -1);
    (fig)->DeleteObject( grp);

    (figv)->FlushDataChanges();
    (figv)->ClearSelection();
    for (ix=0; ix<self->tmpnum; ix++)
	(figv)->SelectByRef( self->tmplist[ix]); 
    AdjustToSelection(self);
    (fig)->NotifyObservers( figure_DATACHANGED);
}

static void Command_MoveToExtreme(class figtoolview  *self, long  infront)
{
    class figview *figv = self->primaryview;
    class figure *fig;
    int ix;
    long foc;

    if (!figv) return;
    fig = (class figure *)(figv)->GetDataObject();
    if (!fig) return;

    foc = (figv)->GetFocusRef();

    self->tmpnum = 0;
    (figv)->EnumerateSelection((figview_esfptr) CacheSelectProc, (long)self);

    if (self->tmpnum==0) {
	message::DisplayString(figv, 10, "No objects selected.");
	return;
    }

    for (ix=0; ix<self->tmpnum; ix++)
	(fig)->UnlinkObjectByRef( self->tmplist[ix]);
    if (infront) {
	for (ix=0; ix<self->tmpnum; ix++)
	    (fig)->LinkObjectByRef( self->tmplist[ix], foc, -1);
    }
    else {
	for (ix=self->tmpnum-1; ix>=0; ix--)
	    (fig)->LinkObjectByRef( self->tmplist[ix], foc, 0);
    }

    (fig)->NotifyObservers( figure_DATACHANGED);
}

static void ToggleSmoothProc(class figtoolview  *self, long  rock)
{
    class figview *figv = self->primaryview;
    class figure *fig;
    int ix;
    long ref, parent, depth;
    long didsmooth = 0, didunsmooth = 0;
    class figobj *o;
    class figoplin *newo;
    const char *cname;

    if (!figv) return;
    fig = (class figure *)(figv)->GetDataObject();
    if (!fig) return;

    if ((fig)->GetReadOnly()) {
	message::DisplayString(self->primaryview, 10, "Document is read-only.");
	return;
    }

    self->tmpnum = 0;
    (figv)->EnumerateSelection((figview_esfptr) CacheSelectProc, (long)self);

    for (ix=0; ix<self->tmpnum; ix++) {
	ref = self->tmplist[ix];
	o = (fig)->FindObjectByRef( ref);
	cname = (o)->GetTypeName();
	if (ATK::IsTypeByName(cname, "figoplin")) {
	    if (ATK::IsTypeByName(cname, "figospli")) {
		didunsmooth++;
		newo = (class figoplin *)ATK::NewObject("figoplin");
	    }
	    else {
		didsmooth++;
		newo = (class figoplin *)ATK::NewObject("figospli");
	    }
	    (newo)->CopyData( (class figoplin *)o);
	    parent = (o)->GetParentRef();
	    depth = (fig)->FindDepthByRef( ref);
	    (fig)->DeleteObject( o);
	    self->tmplist[ix] = (fig)->InsertObject( newo, parent, depth);
	}
    }

    if (!didunsmooth && !didsmooth) 
	message::DisplayString(figv, 10, "No polylines, polygons or splines are selected.");
    else {
	if (didunsmooth && didsmooth)
	    message::DisplayString(figv, 10, "Selected objects smoothed / unsmoothed.");
	else if (didunsmooth) {
	    if (didunsmooth>1)
		message::DisplayString(figv, 10, "Selected objects unsmoothed.");
	    else
		message::DisplayString(figv, 10, "Object unsmoothed.");
	}
	else if (didsmooth) {
	    if (didsmooth>1)
		message::DisplayString(figv, 10, "Selected objects smoothed.");
	    else
		message::DisplayString(figv, 10, "Object smoothed.");
	}
	(figv)->FlushDataChanges();
	for (ix=0; ix<self->tmpnum; ix++) {
	    (figv)->SelectByRef( self->tmplist[ix]);
	}
	(fig)->NotifyObservers( figure_DATACHANGED);
    }
}

static void ToggleClosedProc(class figtoolview  *self, long  rock)
{
    class figview *figv = self->primaryview;
    class figure *fig;
    int ix;
    long ref, parent, depth;
    long didclose = 0, didopen = 0;
    class figobj *o;
    class figoplin *plo;
    const char *cname;

    if (!figv) return;
    fig = (class figure *)(figv)->GetDataObject();
    if (!fig) return;

    if ((fig)->GetReadOnly()) {
	message::DisplayString(self->primaryview, 10, "Document is read-only.");
	return;
    }

    self->tmpnum = 0;
    (figv)->EnumerateSelection((figview_esfptr) CacheSelectProc, (long)self);

    for (ix=0; ix<self->tmpnum; ix++) {
	ref = self->tmplist[ix];
	o = (fig)->FindObjectByRef( ref);
	cname = (o)->GetTypeName();
	if (ATK::IsTypeByName(cname, "figoplin")) {
	    plo = (class figoplin *)o;
	    if ((plo)->Closed()) {
		didopen++;
		(plo)->SetClosed(FALSE);
	    }
	    else {
		didclose++;
		(plo)->SetClosed(TRUE);
	    }
	    (plo)->RecomputeBounds();
	    (plo)->SetModified();
	}
    }

    if (!didopen && !didclose) 
	message::DisplayString(figv, 10, "No polylines, polygons or splines are selected.");
    else {
	if (didopen && didclose)
	    message::DisplayString(figv, 10, "Selected objects closed / opened.");
	else if (didopen) {
	    if (didopen>1)
		message::DisplayString(figv, 10, "Selected objects opened.");
	    else
		message::DisplayString(figv, 10, "Object opened.");
	}
	else if (didclose) {
	    if (didclose>1)
		message::DisplayString(figv, 10, "Selected objects closed.");
	    else
		message::DisplayString(figv, 10, "Object closed.");
	}
	(fig)->NotifyObservers( figure_DATACHANGED);
    }
}

static void Command_SetDoConstraint(class figtoolview  *self, boolean  rock)
{
    class figview *vv = self->primaryview;
    class figure *fig = (class figure *)(vv)->GetDataObject();
    long ref = (vv)->GetFocusRef();

    class figogrp *focus = (class figogrp *)(fig)->FindObjectByRef( ref);
    if (focus==NULL) { 
	message::DisplayString(self, 0, "No focus group to clear attachments for!");
	return;
    }

    if (rock) {
	if (!focus->doconstraints) {
	    (focus)->SetConstraintsActive( TRUE);
	    message::DisplayString(vv, 0, "Resizing is now active for this group.");
	    if (ref == (fig)->RootObjRef()) {
		vv->lastheight--; /* tell view that it should resize the root group, by convincing it that it has changed size. */
	    }
	}
	else
	    message::DisplayString(vv, 0, "Resizing is already active for this group.");
    }
    else {
	if (focus->doconstraints) {
	    (focus)->SetConstraintsActive( FALSE);
	    message::DisplayString(vv, 0, "Resizing is now inactive for this group.");
	}
	else
	    message::DisplayString(vv, 0, "Resizing is already inactive for this group.");
    }
}

static void DefaultAnchorSplot(class figobj  *o, long  ref, class figview  *self, long  rock)
{
    long diff1, diff2;
    class figogrp *vg = (o)->GetParent();
    struct rectangle *R = (&(vg)->handlebox);
    long *canon, href;

    (o)->ClearAttachments();

    for (canon = (o)->GetCanonicalHandles(); (href = (*canon)) != figobj_NULLREF; canon++) {

	switch ((o)->GetHandleType( href)) {

	    case figobj_MiddleTop:
	    case figobj_MiddleBottom:
		diff1=(o)->GetHandleY( href)-R->top;
		diff2=(o)->GetHandleY( href)-(R->top+R->height);
		if (ABS(diff1)<ABS(diff2)) {
		    (o)->SetAttachmentPosY( href, 0);
		    (o)->SetAttachmentOffY( href, diff1);
		} 
		else {
		    (o)->SetAttachmentPosY( href, figogrp_Range);
		    (o)->SetAttachmentOffY( href, diff2);
		}
		(o)->SetAttachmentPosX( href, (((o)->GetHandleX( href)-R->left)*figogrp_Range)/R->width);
		(o)->SetAttachmentOffX( href, 0);
		break;

	    case figobj_MiddleLeft:
	    case figobj_MiddleRight:
		diff1=(o)->GetHandleX( href)-R->left;
		diff2=(o)->GetHandleX( href)-(R->left+R->width);
		if (ABS(diff1)<ABS(diff2)) {
		    (o)->SetAttachmentPosX( href, 0);
		    (o)->SetAttachmentOffX( href, diff1);
		}
		else {
		    (o)->SetAttachmentPosX( href, figogrp_Range);
		    (o)->SetAttachmentOffX( href, diff2);
		}
		(o)->SetAttachmentPosY( href, (((o)->GetHandleY( href)-R->top)*figogrp_Range)/R->height);
		(o)->SetAttachmentOffY( href, 0);
		break;

	    default:
		diff1=(o)->GetHandleX( href)-R->left;
		diff2=(o)->GetHandleX( href)-(R->left+R->width);
		if (ABS(diff1)<ABS(diff2)) {
		    (o)->SetAttachmentPosX( href, 0);
		    (o)->SetAttachmentOffX( href, diff1);
		}
		else {
		    (o)->SetAttachmentPosX( href, figogrp_Range);
		    (o)->SetAttachmentOffX( href, diff2);
		}
		diff1=(o)->GetHandleY( href)-R->top;
		diff2=(o)->GetHandleY( href)-(R->top+R->height);
		if (ABS(diff1)<ABS(diff2)) {
		    (o)->SetAttachmentPosY( href, 0);
		    (o)->SetAttachmentOffY( href, diff1);
		} 
		else {
		    (o)->SetAttachmentPosY( href, figogrp_Range);
		    (o)->SetAttachmentOffY( href, diff2);
		}
		break;

	}
	(o)->SetAttachmentActive( href, TRUE);
    }

    (o)->ComputeSelectedBounds();
    (o)->SetModified();
}

static void NameObject(class figobj  *o, long  ref, class figview  *self, long  rock)
{
    char buf[1024];
    long res = message::AskForString (self, 40, "Name for object:", NULL, buf, sizeof(buf)); 
    if (res<0 || strlen(buf)==0) {
	message::DisplayString(self, 10, "Cancelled.");
	return;
    }
    o->SetName(buf);
}

static void ProportAnchorSplot(class figobj  *o, long  ref, class figview  *self, long  rock)
{
    long diff1, diff2;
    class figogrp *vg = (o)->GetParent();
    struct rectangle *R = (&(vg)->handlebox);
    long *canon, href;

    (o)->ClearAttachments();

    for (canon = (o)->GetCanonicalHandles(); (href = (*canon)) != figobj_NULLREF; canon++) {

	(o)->SetAttachmentPosY( href, (((o)->GetHandleY( href)-R->top)*figogrp_Range)/R->height);
	(o)->SetAttachmentOffY( href, 0);
	(o)->SetAttachmentPosX( href, (((o)->GetHandleX( href)-R->left)*figogrp_Range)/R->width);
	(o)->SetAttachmentOffX( href, 0);
	(o)->SetAttachmentActive( href, TRUE);
    }

    (o)->ComputeSelectedBounds();
    (o)->SetModified();
}

static void ClearAnchorSplot(class figobj  *o, long  ref, class figview  *self, long  rock)
{
    (o)->ClearAttachments();
    (o)->ComputeSelectedBounds();
    (o)->SetModified();
}

static void Command_ClearAnchors(class figtoolview  *self, char  *rock) 
{
    class figure *fig = (class figure *)(self->primaryview)->GetDataObject();
    int numsel = (self->primaryview)->GetNumSelected();

    if (numsel==0) {
	message::DisplayString(self->primaryview, 0, "No objects are selected.");
	return;
    }

    (self->primaryview)->EnumerateSelection((figview_esfptr) ClearAnchorSplot, 0);
    (fig)->SetModified();
    (fig)->NotifyObservers( figure_DATACHANGED);

    if (numsel==1)
	message::DisplayString(self->primaryview, 0, "Anchors cleared for selected object.");
    else
	message::DisplayString(self->primaryview, 0, "Anchors cleared for selected objects.");
}

static void Command_DefaultAnchors(class figtoolview  *self, char  *rock)
{
    class figure *fig = (class figure *)(self->primaryview)->GetDataObject();
    int numsel = (self->primaryview)->GetNumSelected();

    if (numsel==0) {
	message::DisplayString(self->primaryview, 0, "No objects are selected.");
	return;
    }

    (self->primaryview)->EnumerateSelection((figview_esfptr) DefaultAnchorSplot, 0);
    (fig)->SetModified();
    (fig)->NotifyObservers( figure_DATACHANGED);

    if (numsel==1)
	message::DisplayString(self->primaryview, 0, "Standard anchors created for selected object.");
    else
	message::DisplayString(self->primaryview, 0, "Standard anchors created for selected objects.");
}

static void Command_Name(class figtoolview  *self, char  *rock)
{
    class figure *fig = (class figure *)(self->primaryview)->GetDataObject();
    
    int numsel = (self->primaryview)->GetNumSelected();

    if (numsel!=1) {
	message::DisplayString(self->primaryview, 0, "Exactly one object must be selected.");
	return;
    }
    
    (self->primaryview)->EnumerateSelection((figview_esfptr)NameObject, 0);
}

static void Command_ProportAnchors(class figtoolview  *self, char  *rock)
{
    class figure *fig = (class figure *)(self->primaryview)->GetDataObject();
    int numsel = (self->primaryview)->GetNumSelected();

    if (numsel==0) {
	message::DisplayString(self->primaryview, 0, "No objects are selected.");
	return;
    }

    (self->primaryview)->EnumerateSelection((figview_esfptr) ProportAnchorSplot, 0);
    (fig)->SetModified();
    (fig)->NotifyObservers( figure_DATACHANGED);

    if (numsel==1)
	message::DisplayString(self->primaryview, 0, "Proportional anchors created for selected object.");
    else
	message::DisplayString(self->primaryview, 0, "Proportional anchors created for selected objects.");
}

static boolean SelAddProc(class figobj  *o, long  ref, class figure  *fig, class figview  *vv)
{
    (vv)->SelectByRef( ref);
    return FALSE;
}

static boolean SelTogProc(class figobj  *o, long  ref, class figure  *fig, class figview  *vv)
{
    (vv)->ToggleSelectByRef( ref);
    return FALSE;
}

static void MakeBoxListProc(class figobj  *o, long  ref, class figview  *vv, struct rectangle  **rec)
{
    struct rectangle *src = (o)->GetBounds( vv);
    if (rectangle_Width(src)<=0 || rectangle_Height(src)<=0)
	return;

    rectangle_SetRectSize(*rec,
			   (vv)->ToPixX( rectangle_Left(src)),
			   (vv)->ToPixY( rectangle_Top(src)), 
			   (vv)->ToPixW( rectangle_Width(src)), 
			   (vv)->ToPixH( rectangle_Height(src)));

    (*rec)++;
}

static void MoveObjsProc(class figobj  *o, long  ref, class figview  *vv, struct point  *pt)
{
    (o)->Reposition( point_X(pt), point_Y(pt));
    (o)->StabilizeAttachments( FALSE);
}

static void Toolsub_Reshape(class figtoolview  *self, enum view_MouseAction  action, long  x , long  y, long  oref , long  ptref)
{
    class figobj *o = self->primaryview->objs[oref].o;
    class figure *fig;

    switch (action) {
	case view_LeftDown:
	case view_RightDown:
	    self->rock = (o)->Reshape( action, self->primaryview, x, y, TRUE, ptref);
	    break;
	case view_LeftMovement:
	case view_RightMovement:
	    (self)->SnapToGrid( x, y);
	    if (self->rock)
		self->rock = (o)->Reshape( action, self->primaryview, x, y, TRUE, ptref);
	    break;
	case view_LeftUp:
	case view_RightUp:
	    (self)->SnapToGrid( x, y);
	    if (self->rock) {
		self->rock = (o)->Reshape( action, self->primaryview, x, y, TRUE, ptref);
		if (self->rock) {
		    (o)->StabilizeAttachments( FALSE);
		    fig = (class figure *)(self)->GetDataObject();
		    (fig)->SetModified();
		    (fig)->NotifyObservers( figure_DATACHANGED);
		}
	    }
	    break;
    }
}

static void Toolsub_AddAnch(class figtoolview  *self, enum view_MouseAction  action, long  x , long  y, boolean  onhandle, long  oref , long  ptref)
{
    class figobj *o = self->primaryview->objs[oref].o;
    class figure *fig;
    long atx, aty;
    struct rectangle *R;
    class figogrp *vg;

    if (!onhandle || ptref==figobj_NULLREF) return;
    (self)->SnapToGrid( x, y);
    switch (action) {
	case view_LeftDown:
	case view_RightDown:
	    self->rockx = (self->primaryview)->ToPixX( (o)->GetHandleX( ptref));
	    self->rocky = (self->primaryview)->ToPixY( (o)->GetHandleY( ptref));
	    self->lastx = (self->primaryview)->ToPixX( x);
	    self->lasty = (self->primaryview)->ToPixY( y);
	    (self->primaryview)->SetTransferMode( graphic_INVERT);
	    (self->primaryview)->MoveTo( self->rockx, self->rocky);
	    (self->primaryview)->DrawLineTo( self->lastx, self->lasty);
	    break;
	case view_LeftMovement:
	case view_RightMovement:
	    (self->primaryview)->SetTransferMode( graphic_INVERT);
	    (self->primaryview)->MoveTo( self->rockx, self->rocky);
	    (self->primaryview)->DrawLineTo( self->lastx, self->lasty);
	    self->lastx = (self->primaryview)->ToPixX( x);
	    self->lasty = (self->primaryview)->ToPixY( y);
	    (self->primaryview)->MoveTo( self->rockx, self->rocky);
	    (self->primaryview)->DrawLineTo( self->lastx, self->lasty);
	    break;
	case view_LeftUp:
	case view_RightUp:
	    (self->primaryview)->SetTransferMode( graphic_INVERT);
	    (self->primaryview)->MoveTo( self->rockx, self->rocky);
	    (self->primaryview)->DrawLineTo( self->lastx, self->lasty);

	    vg = (o)->GetParent();
	    R = (&(vg)->handlebox);
	    fig = (class figure *)(self)->GetDataObject();

	    atx = (o)->GetHandleX( ptref);
	    aty = (o)->GetHandleY( ptref);
	    (o)->SetAttachmentPosX( ptref, ((x-R->left)*figogrp_Range)/R->width);
	    (o)->SetAttachmentPosY( ptref, ((y-R->top)*figogrp_Range)/R->height);
	    (o)->SetAttachmentOffX( ptref, atx-x);
	    (o)->SetAttachmentOffY( ptref, aty-y);
	    (o)->SetAttachmentActive( ptref, TRUE);

	    (o)->ComputeSelectedBounds();
	    (o)->SetModified();
	    (fig)->SetModified();
	    (fig)->NotifyObservers( figure_DATACHANGED);

	    break;
    }
}

static void Toolsub_DelAnch(class figtoolview  *self, enum view_MouseAction  action, long  x , long  y, boolean  onhandle, long  oref , long  ptref)
{
    class figobj *o = self->primaryview->objs[oref].o;
    class figure *fig;
    long atx, aty;
    struct rectangle *R;
    class figogrp *vg;

    if (!onhandle || ptref==figobj_NULLREF) return;

    switch (action) {
	case view_LeftDown:
	case view_RightDown:
	    if ((o)->IsAttachmentActive( ptref)) {
		fig = (class figure *)(self)->GetDataObject();
		(o)->SetAttachmentActive( ptref, FALSE);
		(o)->ComputeSelectedBounds();
		(o)->SetModified();
		(fig)->SetModified();
		(fig)->NotifyObservers( figure_DATACHANGED);
	    }

	    break;
	case view_LeftMovement:
	case view_RightMovement:
	    break;
	case view_LeftUp:
	case view_RightUp:
	    break;
    }
}

static void Toolsub_Add(class figtoolview  *self, enum view_MouseAction  action, long  x , long  y, boolean  onhandle, long  oref , long  ptref)
{
    class figobj *o = self->primaryview->objs[oref].o;
    class figure *fig;

    switch (action) {
	case view_LeftDown:
	case view_RightDown:
	    self->rock = (o)->AddParts( action, self->primaryview, x, y, onhandle, ptref);
	    break;
	case view_LeftMovement:
	case view_RightMovement:
	    (self)->SnapToGrid( x, y);
	    if (self->rock)
		self->rock = (o)->AddParts( action, self->primaryview, x, y, onhandle, ptref);
	    break;
	case view_LeftUp:
	case view_RightUp:
	    (self)->SnapToGrid( x, y);
	    if (self->rock) {
		self->rock = (o)->AddParts( action, self->primaryview, x, y, onhandle, ptref);
		if (self->rock) {
		    fig = (class figure *)(self)->GetDataObject();
		    (fig)->SetModified();
		    (fig)->NotifyObservers( figure_DATACHANGED);
		}
	    }
	    break;
    }
}

static void Toolsub_Del(class figtoolview  *self, enum view_MouseAction  action, long  x , long  y, boolean  onhandle, long  oref , long  ptref)
{
    class figobj *o = self->primaryview->objs[oref].o;
    class figure *fig;

    switch (action) {
	case view_LeftDown:
	case view_RightDown:
	    self->rock = (o)->DeleteParts( action, self->primaryview, x, y, onhandle, ptref);
	    break;
	case view_LeftMovement:
	case view_RightMovement:
	    (self)->SnapToGrid( x, y);
	    if (self->rock)
		self->rock = (o)->DeleteParts( action, self->primaryview, x, y, onhandle, ptref);
	    break;
	case view_LeftUp:
	case view_RightUp:
	    (self)->SnapToGrid( x, y);
	    if (self->rock) {
		self->rock = (o)->DeleteParts( action, self->primaryview, x, y, onhandle, ptref);
		if (self->rock) {
		    fig = (class figure *)(self)->GetDataObject();
		    (fig)->SetModified();
		    (fig)->NotifyObservers( figure_DATACHANGED);
		}
	    }
	    break;
    }
}

/* at this point, we know objects are selected */
static void Toolsub_Drag(class figtoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks)
{
    long ix;
    struct rectangle *tmp;
    struct point pt;
    class figure *fig;

    (self)->SnapToGrid( x, y);
    switch (action) {
	case view_LeftDown:
	case view_RightDown:
	    ix = (self->primaryview)->GetNumSelected();
	    if (ix > self->rect_size) {
		while (ix > self->rect_size)
		    self->rect_size *= 2;
		self->rectlist = (struct rectangle *)realloc(self->rectlist, self->rect_size * sizeof(struct rectangle));
	    }
	    tmp = self->rectlist;
	    (self->primaryview)->EnumerateSelection((figview_esfptr) MakeBoxListProc, (long)&tmp);
	    self->rock = ix;
	    self->rockx = x;
	    self->rocky = y;
	    self->lastx = 0;
	    self->lasty = 0;
	    (self->primaryview)->SetTransferMode( graphic_INVERT);
	    for (ix=0; ix<self->rock; ix++) {
		tmp = &(self->rectlist[ix]);
		(self->primaryview)->DrawRectSize( self->lastx+tmp->left, self->lasty+tmp->top, tmp->width, tmp->height);
	    }
	    break;
	case view_LeftMovement:
	case view_RightMovement:
	    (self->primaryview)->SetTransferMode( graphic_INVERT);
	    for (ix=0; ix<self->rock; ix++) {
		tmp = &(self->rectlist[ix]);
		(self->primaryview)->DrawRectSize( self->lastx+tmp->left, self->lasty+tmp->top, tmp->width, tmp->height);
	    }
	    self->lastx = (self->primaryview)->ToPixW( x - self->rockx);
	    self->lasty = (self->primaryview)->ToPixH( y - self->rocky);
	    for (ix=0; ix<self->rock; ix++) {
		tmp = &(self->rectlist[ix]);
		(self->primaryview)->DrawRectSize( self->lastx+tmp->left, self->lasty+tmp->top, tmp->width, tmp->height);
	    }
	    break;
	case view_LeftUp:
	case view_RightUp:
	    (self->primaryview)->SetTransferMode( graphic_INVERT);
	    for (ix=0; ix<self->rock; ix++) {
		tmp = &(self->rectlist[ix]);
		(self->primaryview)->DrawRectSize( self->lastx+tmp->left, self->lasty+tmp->top, tmp->width, tmp->height);
	    }
	    point_SetPt(&pt, x - self->rockx, y - self->rocky);
	    (self->primaryview)->EnumerateSelection((figview_esfptr) MoveObjsProc, (long)&pt);
	    fig = (class figure *)(self)->GetDataObject();
	    (fig)->SetModified();
	    (fig)->NotifyObservers( figure_DATACHANGED);
	    break;
    }
}

static void ShowName(class figobj  *o, long  ref, class figview  *self, long  rock)
{
    if(o->GetName()) message::DisplayString(self, 0, o->GetName());
}
static void Toolsub_Select(class figtoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks)
{
    long w, h;
    struct rectangle area;
    class figure *fig = (class figure *)(self)->GetDataObject();
    if (!fig) return;

    switch (action) {
	case view_LeftDown:
	case view_RightDown:
	    self->rockx = x;
	    self->rocky = y;
	    self->lastx = (self->primaryview)->ToPixX( x);
	    self->lasty = (self->primaryview)->ToPixY( y);
	    self->lastw = 0;
	    self->lasth = 0;
	    (self->primaryview)->SetTransferMode( graphic_INVERT);
	    (self->primaryview)->DrawRectSize( self->lastx, self->lasty, self->lastw, self->lasth);
	    break;
	case view_LeftMovement:
	case view_RightMovement:
	    (self->primaryview)->SetTransferMode( graphic_INVERT);
	    (self->primaryview)->DrawRectSize( self->lastx, self->lasty, self->lastw, self->lasth);
	    /* x, y, w, h in fig coords; self->last* in pix coords */
	    w = x - self->rockx;
	    if (w<0) {
		self->lastw = (self->primaryview)->ToPixW( -w);
		self->lastx = (self->primaryview)->ToPixX( x);
	    }
	    else {
		self->lastw = (self->primaryview)->ToPixW( w);
		self->lastx = (self->primaryview)->ToPixX( self->rockx);
	    }
	    h = y - self->rocky;
	    if (h<0) {
		self->lasth = (self->primaryview)->ToPixH( -h);
		self->lasty = (self->primaryview)->ToPixY( y);
	    }
	    else {
		self->lasth = (self->primaryview)->ToPixH( h);
		self->lasty = (self->primaryview)->ToPixY( self->rocky);
	    }
	    (self->primaryview)->DrawRectSize( self->lastx, self->lasty, self->lastw, self->lasth);
	    break;
	case view_LeftUp:
	case view_RightUp:
	    (self->primaryview)->SetTransferMode( graphic_INVERT);
	    (self->primaryview)->DrawRectSize( self->lastx, self->lasty, self->lastw, self->lasth);
	    /* x, y, w, h in fig coords */
	    w = x - self->rockx;
	    if (w<0) 
		w = -w;
	    else 
		x = self->rockx;
	    h = y - self->rocky;
	    if (h<0) 
		h = -h;
	    else 
		y = self->rocky;
	    if ((self->primaryview)->ToPixW( w) < figview_SelectDelta
		&& (self->primaryview)->ToPixH( h) < figview_SelectDelta) {
		/* cursor didn't move far enough; we have a point click. */
		long ptref;
		long ref, tmpref;
		long delta = (self->primaryview)->ToFigW( figtoolview_SelectClickDistance);
		ref = (fig)->FindRefByPos( (self->primaryview)->GetFocusRef(), TRUE, figobj_HitBody, delta, x, y, &ptref);
		if (!self->selectdeep) {
		    while (ref != figure_NULLREF && (tmpref=((fig)->FindObjectByRef( ref))->GetParentRef()) != (self->primaryview)->GetFocusRef())
			ref = tmpref;
		}
		if (ref != figure_NULLREF) {
		    if (action==view_LeftUp)
			(self->primaryview)->SelectByRef( ref);
		    else
			(self->primaryview)->ToggleSelectByRef( ref);
		    AdjustToSelection(self);
		    (self->primaryview)->WantUpdate( self->primaryview);
		}
	    }
	    else {
		rectangle_SetRectSize(&area, x, y, w, h);
		if (self->selectdeep)
		    (fig)->EnumerateObjectTree( (self->primaryview)->GetFocusRef(), &area, FALSE, (action==view_LeftUp ? (figure_eofptr)SelAddProc : (figure_eofptr)SelTogProc), (long)self->primaryview);
		else
		    (fig)->EnumerateObjectGroup( (self->primaryview)->GetFocusRef(), &area, FALSE, (action==view_LeftUp ? (figure_eofptr)SelAddProc : (figure_eofptr)SelTogProc), (long)self->primaryview);
		AdjustToSelection(self);
		(self->primaryview)->WantUpdate( self->primaryview);
	    }
	    break;
    }
    
    int numsel = (self->primaryview)->GetNumSelected();
    if(numsel==1) {
	(self->primaryview)->EnumerateSelection((figview_esfptr)ShowName, 0);
    }
}

struct FHOP_lump {
    enum figobj_HitVal result;
    long oref, ptref;
    long x, y;
    long delta;
};

static void FindHitObjProc(class figobj  *o, long  ref, class figview  *vv, struct FHOP_lump  *val)
{
    enum figobj_HitVal res;
    long tmp;

    res = (o)->HitMe( val->x, val->y, val->delta, &tmp);

    if ((int)(res) < (int)(val->result))
	return;

    val->result = res;
    val->oref = ref;
    val->ptref = tmp;
}

static void FindHitObjAnchProc(class figobj  *o, long  ref, class figview  *vv, struct FHOP_lump  *val) /* about the most unpleasant function name I've ever come up with. This counts a hit on an anchor as a hit on the handle. */
{
    enum figobj_HitVal res;
    long tmp;
    int ix;
    long dx, dy;

    res = (o)->HitMe( val->x, val->y, val->delta, &tmp);

    if (res==figobj_Miss || res==figobj_HitInside) {
	for (ix=0; ix<(o)->GetNumHandles(); ix++) {
	    dx = ((o)->GetHandleX( ix)-(o)->GetAttachmentOffX( ix)) - val->x;
	    dy = ((o)->GetHandleY( ix)-(o)->GetAttachmentOffY( ix)) - val->y;
	    if (dx<=val->delta && dx>=(-val->delta) && dy<=val->delta && dy>=(-val->delta)) {
		tmp = ix;
		res = figobj_HitHandle;
		break;
	    }
	}
    }

    if ((int)(res) < (int)(val->result))
	return;

    val->result = res;
    val->oref = ref;
    val->ptref = tmp;
}

/* the grotesque semantics:
Right-down is always select mode.
Left-down in a selected object:
  reshape mode if it's on a selected object's handle;
  drag mode otherwise;
Left-down in no selected object is select mode.
If the action is a mouse movement or mouse-up, continue in the mode determined by the mouse-down.
*/
static class view *Tool_Select(class figtoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks)
{
    struct FHOP_lump val;
    class figure *fig;

    if (action==view_LeftDown || action==view_RightDown) {
	if (action==view_RightDown) {
	    self->submode = 0;
	    Toolsub_Select(self, action, x, y, numclicks);
	}
	else {
	    val.result = figobj_Miss;
	    val.x = x;
	    val.y = y;
	    val.delta = (self->primaryview)->ToFigW( figtoolview_SelectClickDistance);
	    (self->primaryview)->EnumerateSelection((figview_esfptr) FindHitObjProc, (long)&val);	 
	    fig = (class figure *)(self)->GetDataObject();
	    if (!fig)
		self->submode = 3;
	    else if (val.result==figobj_Miss) {
		(self->primaryview)->ClearSelection();
		(self->primaryview)->WantUpdate( self->primaryview);
		self->submode = 0;
		Toolsub_Select(self, action, x, y, numclicks);
	    }
	    else if (!(fig)->GetReadOnly() && (val.result==figobj_HitInside || val.result==figobj_HitBody)) {
		self->submode = 1;
		Toolsub_Drag(self, action, x, y, numclicks);
	    }
	    else if (!(fig)->GetReadOnly()) {
		self->submode = 2;
		self->lastw = val.oref;
		self->lasth = val.ptref;
		/* this assumes, unfairly, that no object is resized by a HitBody click. */
		Toolsub_Reshape(self, action, x, y, self->lastw, self->lasth);
	    }
	    else
		self->submode = 3;
	}
    }
    else {
	/* mouse movement or up */
	switch (self->submode) {
	    case 0:
		Toolsub_Select(self, action, x, y, numclicks);
		break;
	    case 1:
		Toolsub_Drag(self, action, x, y, numclicks);
		break;
	    case 2:
		Toolsub_Reshape(self, action, x, y, self->lastw, self->lasth);
		break;
	    default:
		break;
	}
    }
    return (class view *)(self->primaryview);
}

static class view *Tool_AddPoints(class figtoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks)
{
    struct FHOP_lump val;

    if (self->creating) {
	/* this is a safety hack to deal with the figotext re-editing mode. */
	if (action==view_LeftDown || action==view_RightDown) {
	    (self)->AbortObjectBuilding();
	}
	self->submode = (-1);
	return (class view *)(self->primaryview);
    }

    if (action==view_LeftDown || action==view_RightDown) {
	if (action==view_RightDown) {
	    self->submode = 0;
	    Toolsub_Select(self, action, x, y, numclicks);
	}
	else {
	    val.result = figobj_Miss;
	    val.x = x;
	    val.y = y;
	    val.delta = (self->primaryview)->ToFigW( figtoolview_SelectClickDistance);
	    (self->primaryview)->EnumerateSelection((figview_esfptr) FindHitObjProc, (long)&val);	    
	    if (val.result==figobj_Miss || val.result==figobj_HitInside) {
		(self->primaryview)->ClearSelection();
		(self->primaryview)->WantUpdate( self->primaryview);
		self->submode = 0;
		Toolsub_Select(self, action, x, y, numclicks);
	    }
	    else {
		self->submode = 2;
		self->lastw = val.oref;
		self->lasth = val.ptref;
		self->lastb = (val.result == figobj_HitHandle);
		Toolsub_Add(self, action, x, y, self->lastb, self->lastw, self->lasth);
	    }
	}
    }
    else {
	/* mouse movement or up */
	switch (self->submode) {
	    case 0:
		Toolsub_Select(self, action, x, y, numclicks);
		break;
	    case 2:
		Toolsub_Add(self, action, x, y, self->lastb, self->lastw, self->lasth);
		break;
	    default:
		break;
	}
    }
    return (class view *)(self->primaryview);
}

static class view *Tool_DelPoints(class figtoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks)
{
    struct FHOP_lump val;

    if (action==view_LeftDown || action==view_RightDown) {
	if (action==view_RightDown) {
	    self->submode = 0;
	    Toolsub_Select(self, action, x, y, numclicks);
	}
	else {
	    val.result = figobj_Miss;
	    val.x = x;
	    val.y = y;
	    val.delta = (self->primaryview)->ToFigW( figtoolview_SelectClickDistance);
	    (self->primaryview)->EnumerateSelection((figview_esfptr) FindHitObjProc, (long)&val);	    
	    if (val.result==figobj_Miss || val.result==figobj_HitInside) {
		(self->primaryview)->ClearSelection();
		(self->primaryview)->WantUpdate( self->primaryview);
		self->submode = 0;
		Toolsub_Select(self, action, x, y, numclicks);
	    }
	    else {
		self->submode = 2;
		self->lastw = val.oref;
		self->lasth = val.ptref;
		self->lastb = (val.result == figobj_HitHandle);
		Toolsub_Del(self, action, x, y, self->lastb, self->lastw, self->lasth);
	    }
	}
    }
    else {
	/* mouse movement or up */
	switch (self->submode) {
	    case 0:
		Toolsub_Select(self, action, x, y, numclicks);
		break;
	    case 2:
		Toolsub_Del(self, action, x, y, self->lastb, self->lastw, self->lasth);
		break;
	    default:
		break;
	}
    }
    return (class view *)(self->primaryview);
}

static class view *Tool_AddAnchor(class figtoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks)
{
    struct FHOP_lump val;

    if (action==view_LeftDown || action==view_RightDown) {
	if (action==view_RightDown) {
	    self->submode = 0;
	    Toolsub_Select(self, action, x, y, numclicks);
	}
	else {
	    val.result = figobj_Miss;
	    val.x = x;
	    val.y = y;
	    val.delta = (self->primaryview)->ToFigW( figtoolview_SelectClickDistance);
	    (self->primaryview)->EnumerateSelection((figview_esfptr) FindHitObjAnchProc, (long)&val);	    
	    if (val.result==figobj_Miss || val.result==figobj_HitInside) {
		class figure *fig;
		long focref;
		class figobj *vg;
		enum figobj_HitVal res;

		fig = (class figure *)(self)->GetDataObject();
		if ((focref = (self->primaryview)->GetFocusRef()) != figure_NULLREF
		    && (vg = (fig)->FindObjectByRef( focref))
		    && (res = (vg)->HitMe( val.x, val.y, val.delta, &val.ptref)) == figobj_HitHandle) {
		    if (focref == (fig)->RootObjRef()) {
			message::DisplayString(self->primaryview, 10, "Resize the window to resize the root group.");
			self->submode = 3;
		    }
		    else {
			self->submode = 1;
			self->lastw = focref;
			self->lasth = val.ptref;
			Toolsub_Reshape(self, action, x, y, self->lastw, self->lasth);
		    }
		}
		else {
		    (self->primaryview)->ClearSelection();
		    (self->primaryview)->WantUpdate( self->primaryview);
		    self->submode = 0;
		    Toolsub_Select(self, action, x, y, numclicks);
		}
	    }
	    else {
		self->submode = 2;
		self->lastw = val.oref;
		self->lasth = val.ptref;
		self->lastb = (val.result == figobj_HitHandle);
		Toolsub_AddAnch(self, action, x, y, self->lastb, self->lastw, self->lasth);
	    }
	}
    }
    else {
	/* mouse movement or up */
	switch (self->submode) {
	    case 0:
		Toolsub_Select(self, action, x, y, numclicks);
		break;
	    case 1:
		Toolsub_Reshape(self, action, x, y, self->lastw, self->lasth);
		break;
	    case 2:
		Toolsub_AddAnch(self, action, x, y, self->lastb, self->lastw, self->lasth);
		break;
	    default:
		break;
	}
    }
    return (class view *)(self->primaryview);
}

static class view *Tool_DelAnchor(class figtoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks)
{
    struct FHOP_lump val;

    if (action==view_LeftDown || action==view_RightDown) {
	if (action==view_RightDown) {
	    self->submode = 0;
	    Toolsub_Select(self, action, x, y, numclicks);
	}
	else {
	    val.result = figobj_Miss;
	    val.x = x;
	    val.y = y;
	    val.delta = (self->primaryview)->ToFigW( figtoolview_SelectClickDistance);
	    (self->primaryview)->EnumerateSelection((figview_esfptr) FindHitObjAnchProc, (long)&val);	    
	    if (val.result==figobj_Miss || val.result==figobj_HitInside) {
		class figure *fig;
		long focref;
		class figobj *vg;
		enum figobj_HitVal res;

		fig = (class figure *)(self)->GetDataObject();
		if ((focref = (self->primaryview)->GetFocusRef()) != figure_NULLREF
		    && (vg = (fig)->FindObjectByRef( focref))
		    && (res = (vg)->HitMe( val.x, val.y, val.delta, &val.ptref)) == figobj_HitHandle) {
		    if (focref == (fig)->RootObjRef()) {
			message::DisplayString(self->primaryview, 10, "Resize the window to resize the root group.");
			self->submode = 3;
		    }
		    else {
			self->submode = 1;
			self->lastw = focref;
			self->lasth = val.ptref;
			Toolsub_Reshape(self, action, x, y, self->lastw, self->lasth);
		    }
		}
		else {
		    (self->primaryview)->ClearSelection();
		    (self->primaryview)->WantUpdate( self->primaryview);
		    self->submode = 0;
		    Toolsub_Select(self, action, x, y, numclicks);
		}
	    }
	    else {
		self->submode = 2;
		self->lastw = val.oref;
		self->lasth = val.ptref;
		self->lastb = (val.result == figobj_HitHandle);
		Toolsub_DelAnch(self, action, x, y, self->lastb, self->lastw, self->lasth);
	    }
	}
    }
    else {
	/* mouse movement or up */
	switch (self->submode) {
	    case 0:
		Toolsub_Select(self, action, x, y, numclicks);
		break;
	    case 1:
		Toolsub_Reshape(self, action, x, y, self->lastw, self->lasth);
		break;
	    case 2:
		Toolsub_DelAnch(self, action, x, y, self->lastb, self->lastw, self->lasth);
		break;
	    default:
		break;
	}
    }
    return (class view *)(self->primaryview);
}

static void Toolmod_Select(class figtoolview  *self, long  rock)
{
    if (self->expertmode) {
	if (self->selectdeep == FALSE) {
	    message::DisplayString(self, 10, "Deep selection mode on.");
	    self->selectdeep = TRUE;
	}
	else {
	    message::DisplayString(self, 10, "Deep selection mode off.");
	    self->selectdeep = FALSE;
	}
    }
}

static class view *Tool_CreateProc(class figtoolview  *self, enum view_MouseAction  action, long  x , long  y , long  numclicks)
{
    enum figobj_Status result;
    class figure *fig = (class figure *)(self)->GetDataObject();

    (self)->SnapToGrid( x, y);

    if (!self->creating) {
	class figobj *dummy;
	class figobj *vob;
	if (action != view_LeftDown) {
	    if (self->selectonup && (action==view_LeftUp || action==view_RightUp))
		SetToolProc(self->tooltbl, self, self->toolacc[2]); /* ### */
	    self->selectonup = FALSE;
	    return (class view *)(self->primaryview); 
	}

	self->selectonup = FALSE;

	dummy = self->dummylist[self->toolnum - NumFigTools(self)];
	vob = (dummy)->Instantiate( self, (self->toollist)[self->toolnum - NumFigTools(self)].rock);

	if (!vob) 
	    return (class view *)(self->primaryview);

	self->creating = vob;
	((vob)->GetVAttributes())->CopyData( self->menuatt, figattr_MaskAll/* & figobj_AttributesUsed(vob)*/); /* the commented-out bit would prevent new objects from taking on attributes they don't use. This makes for shorter datastreams, but makes the toolset menus jump to <default> all the time, which is confusing. So we leave it commented. */
	/* we don't call figobj_UpdateVAttributes(), 'cause that would do a little too much bouncing around. */
	(fig)->InsertObject( vob, (self->primaryview)->GetFocusRef(), -1);
	result = (vob)->Build( self->primaryview, action, x, y, numclicks);
	(self->primaryview)->FlushDataChanges();
	(self->primaryview)->ClearSelection();
	(self->primaryview)->WantUpdate( self->primaryview);
	RepostMenus(self);
    }
    else {
	result = (self->creating)->Build( self->primaryview, action, x, y, numclicks);
    }

    if (result == figobj_Done) {
	message::DisplayString(self->primaryview, 10, "Completed.");
	(self->primaryview)->Select( self->creating);
	(fig)->NotifyObservers( figure_DATACHANGED);
	self->creating = NULL;
	self->selectonup = !self->LockCreateMode;
	(self->primaryview)->SetBuildKeystate( NULL);
	if (!self->LockCreateMode && (action==view_LeftUp || action==view_RightUp))
	    SetToolProc(self->tooltbl, self, self->toolacc[2]); /* ### */
	RepostMenus(self);
    }
    else if (result == figobj_Failed) {
	/*message_DisplayString(self->primaryview, 10, "Object aborted.");*/
	(self->primaryview)->FlushDataChanges();
	(fig)->DeleteObject( self->creating);
	(fig)->NotifyObservers( figure_DATACHANGED); 
	(self->creating)->Destroy();
	self->creating = NULL;
	(self->primaryview)->SetBuildKeystate( NULL);
	RepostMenus(self);
    }

    return (class view *)(self->primaryview);
}

/* either abort or complete the object being built. */
void figtoolview::AbortObjectBuilding()
{
    enum figobj_Status result;
    class figure *fig = (class figure *)(this)->GetDataObject();

    if (!this->creating) {
	message::DisplayString(this->primaryview, 10, "No object is being created.");
	return;
    }

    result = (this->creating)->Build( this->primaryview, view_LeftDown, 0, 0, 0);

    if (result == figobj_Done) {
	message::DisplayString(this->primaryview, 10, "Object completed.");
	(this->primaryview)->FlushDataChanges();
	(this->primaryview)->Select( this->creating);
	(fig)->NotifyObservers( figure_DATACHANGED);
	this->creating = NULL;
	if (!this->LockCreateMode)
	    SetToolProc(this->tooltbl, this, this->toolacc[2]); /* ### */
    }
    else {
	/*message_DisplayString(self->primaryview, 10, "Object aborted.");*/
	(this->primaryview)->FlushDataChanges();
	(fig)->DeleteObject( this->creating);
	(fig)->NotifyObservers( figure_DATACHANGED); 
	(this->creating)->Destroy();
	this->creating = NULL;
    }
    (this->primaryview)->SetBuildKeystate( NULL);
    RepostMenus(this);
}

static void AbortObjectProc(class figtoolview  *self, long  rock)
{
    (self)->AbortObjectBuilding();
}

static char *CopyString(char  *str)
{
    char *tmp;

    if (str==NULL)
	return NULL;
    tmp = (char *)malloc(strlen(str)+1);
    if (!tmp)
	return NULL;
    strcpy(tmp, str);
    return tmp;
}

static void LowerString(char  *str)
{
    char *cx;

    for (cx=str; *cx; cx++) 
	if (isupper(*cx))
	    *cx = tolower(*cx);
}

/* trim whitespace off the front and back of a string. This modifies the buffer (trimming the back) and returns an updated pointer */
static char *WhiteKillString(char  *buf)
{
    char *cx;
    for (cx = buf; *cx && !isgraph(*cx); cx++);
    buf = cx;
    for (cx = buf + (strlen(buf)-1); cx>=buf && !isgraph(*cx); cx--);
    cx++;
    *cx = '\0';
    return buf;
}
