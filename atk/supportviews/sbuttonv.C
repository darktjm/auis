/* ********************************************************************* *\
 *         Copyright IBM Corporation 1991 - All Rights Reserved           *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("sbuttonv.H")
#include <stdio.h>


#include <environ.H>
#include <fontdesc.H>
#include <graphic.H>
#include <im.H>
#include <observable.H>
#include <owatch.H>
#include <cursor.H>
#include <view.H>

#include "sbutton.H"
#include "sbuttonv.H"

#define NO_MSG "Push Me"

#define PROMPTFONT "andysans12b"
#define FONT "andysans"
#define FONTTYPE fontdesc_Bold
#define FONTSIZE 12
#define BUTTONDEPTH 4
#define ULSHADE 0.25 /* upper & left sides */
#define LRSHADE 0.75 /* lower & right sides */
#define TOPSHADE 0.50 /* face of button */
#define MOTIFBUTTONDEPTH 2
#define MOTIFULSHADE 0.10
#define MOTIFLRSHADE 0.50
#define MOTIFTOPSHADE 0.25
#define BUTTONPRESSDEPTH 2
#define TEXTPAD 2
#define EXTRATEXTPAD 3
#define INSENSITIVELABELPCT 0.8
#define INSENSITIVETOPPCT 0.8

#define TEXTINMIDDLE (graphic::BETWEENLEFTANDRIGHT | graphic::BETWEENTOPANDBOTTOM)
#define TEXTATLEFTMIDDLE (graphic::ATLEFT | graphic::BETWEENTOPANDBOTTOM)

#define DEFAULTSTYLE(style) ((style<0 || style>sbutton_MOTIF)?sbutton_MOTIF:(style))

#define BUTTONS(self) ((self)->ButtonData()->buttons)

/* External Declarations */

/* Global Variables */
static const class atom *buttonpushed=NULL;
static boolean newcolors=TRUE;


ATKdefineRegistry(sbuttonv, view, sbuttonv::InitializeClass);
static void InitFGBG(class view  *self, struct sbutton_prefs  *prefs, double  *fg , double  *bg);
static void MyOldComputeColor(class view  *self, struct sbutton_prefs  *prefs, double  *foreground , double  *background, int  color, double  *result);
static void MyOldSetShade(class view  *self, struct sbutton_prefs  *prefs, double  *foreground , double  *background, int  color);
static void MyNewComputeColor(class view  *self, struct sbutton_prefs  *prefs,double  *foreground , double  *background, int  color, double  *result);
static void MyNewSetShade(class view  *self, struct sbutton_prefs  *prefs, double  *foreground , double  *background, int  color);
static void MySetShade(class view  *self, struct sbutton_prefs  *prefs, double  *foreground , double  *background, int  color);
static void DrawText(class view  *self, class fontdesc  *font, long  x , long  y, const char  *text, int  len, long  flags);
static void EnsureInfo(class sbuttonv  *self);
static int RectEnclosesXY(struct rectangle  *r, long  x , long  y);
static boolean definetriggers(class sbutton  *b, int  i, struct sbutton_info  *si, class sbuttonv  *self);
static void OutputLabel(FILE  *f, const char  *l);


static void InitFGBG(class view  *self, struct sbutton_prefs  *prefs, double  *fg , double  *bg)
{

    const char *bgcolor=sbutton::GetBackground(prefs);
    const char *fgcolor=sbutton::GetForeground(prefs);
    const char *deffg, *defbg;

    graphic::GetDefaultColors(&deffg, &defbg);

    if(bgcolor==NULL) bgcolor=defbg?defbg:"white";
    if(fgcolor==NULL) fgcolor=deffg?deffg:"black";

    /* This is necessary for consistent results on monochrome machines, the color setting functions try to pick a stipple on monochrome screens and they determine which stipple to use based on the fg (or bg for setfg) color in effect */ 
    (self)->SetBackgroundColor( bgcolor, 0, 0, 0);
    (self)->SetForegroundColor( fgcolor, 0, 0, 0);
    (self)->SetBackgroundColor( bgcolor, 0, 0, 0);

    (self)->GetBGColor( &bg[0], &bg[1], &bg[2]);
    (self)->GetFGColor( &fg[0], &fg[1], &fg[2]);
    if((!((self)->DisplayClass() & graphic::Monochrome)) && sbutton::GetBackground(prefs)==NULL && DEFAULTSTYLE(prefs->style)==sbutton_MOTIF && newcolors) {
	if(bg[0]==0.0 && bg[1]==0.0 && bg[2]==0.0) {
	    bg[0]=0.40;
	    bg[1]=0.40;
	    bg[2]=0.40;
	} else if(bg[0]==1.0 && bg[1]==1.0 && bg[2]==1.0) {
	    bg[0]=0.75;
	    bg[1]=0.75;
	    bg[2]=0.75;
	}
    }
}

static void MyOldComputeColor(class view  *self, struct sbutton_prefs  *prefs, double  *foreground , double  *background, int  color, double  *result)
{
    double pct=0.0;
    int style=DEFAULTSTYLE(prefs->style);
    switch(color) {
	case sbutton_INSENSITIVELABELBG:
	case sbutton_LABELBG:
	case sbutton_BACKGROUND:
	    pct=0.0;
	    break;
	case sbutton_TOPSHADOW:
	    if(style==sbutton_MOTIF) pct=MOTIFULSHADE;
	    else pct=ULSHADE;
	    break;
	case sbutton_INSENSITIVETOP:
	case sbutton_TOP:
	    switch(style) {
		case sbutton_MOTIF:
		    pct=MOTIFTOPSHADE;
		    break;
		case sbutton_THREEDEE:
		    pct=TOPSHADE;
		    break;
		default:
		    pct=0.0;
		    break;
	    }
	    if(color==sbutton_INSENSITIVETOP) pct=pct*INSENSITIVETOPPCT;
	    break;
	case sbutton_BOTTOMSHADOW:
	    if(style==sbutton_MOTIF) pct=MOTIFLRSHADE;
	    else pct=LRSHADE;
	    break;
	case sbutton_INSENSITIVELABELFG:
	    pct=INSENSITIVELABELPCT;
	    break;
	case sbutton_LABELFG:
	case sbutton_FOREGROUND:
	    pct=1.0;
	    break;
	default:
	    pct=0.0;
    }
    result[0]=foreground[0]*pct 
      + background[0]*(1.0-pct);
    result[1]=foreground[1]*pct 
      + background[1]*(1.0-pct);
    result[2]=foreground[2]*pct 
      + background[2]*(1.0-pct);
}

static void MyOldSetShade(class view  *self, struct sbutton_prefs  *prefs, double  *foreground , double  *background, int  color)
{
    double result[3];
    
    if(prefs->colors[color]) (self)->SetForegroundColor( prefs->colors[color], 0, 0, 0);
    else {
	MyOldComputeColor(self, prefs, foreground, background, color, result);
	(self)->SetFGColor( result[0], 
			result[1], 
			result[2]);
    }
}


static void MyNewComputeColor(class view  *self, struct sbutton_prefs  *prefs,double  *foreground , double  *background, int  color, double  *result)
{
    long br, bg, bb;
    long rr=0, rg=0, rb=0;
    boolean mono=((self)->DisplayClass()&graphic::Monochrome)?TRUE:FALSE;
    
    br=(long)(background[0]*65535.0);
    bg=(long)(background[1]*65535.0);
    bb=(long)(background[2]*65535.0);
    
    switch(color) {
	case sbutton_INSENSITIVELABELFG:
	    result[0]=foreground[0]*INSENSITIVELABELPCT;
	    result[1]=foreground[1]*INSENSITIVELABELPCT;
	    result[2]=foreground[2]*INSENSITIVELABELPCT;
	    break;
	case sbutton_LABELFG:
	case sbutton_FOREGROUND:
	    result[0]=foreground[0];
	    result[1]=foreground[1];
	    result[2]=foreground[2];
	    break;
	case sbutton_BOTTOMSHADOW:
	    if(mono && background[0]==background[1] && background[1]==background[2] && (background[2]==1.0 || background[2]==0.0)) {
		result[0]=1.0-background[0];
		result[1]=1.0-background[1];
		result[2]=1.0-background[2];
	    } else {
		graphic::ComputeShadow(br, bg, bb, &rr, &rg, &rb, shadows_BOTTOMSHADOW);
		result[0] = ((double)rr)/65535.0;
		result[1] = ((double)rg)/65535.0;
		result[2] = ((double)rb)/65535.0;
	    }
	    break;
	case sbutton_TOPSHADOW:
	    if(mono && background[0]==background[1] && background[1]==background[2] && (background[2]==1.0 || background[2]==0.0)) {
		result[0]=0.5;
		result[1]=0.5;
		result[2]=0.5;
	    } else {
		graphic::ComputeShadow(br, bg, bb, &rr, &rg, &rb, shadows_TOPSHADOW);
		result[0] = ((double)rr)/65535.0;
		result[1] = ((double)rg)/65535.0;
		result[2] = ((double)rb)/65535.0;
	    }
	    break;
	case sbutton_INSENSITIVETOP:
	    result[0]=background[0]*INSENSITIVETOPPCT;
	    result[1]=background[1]*INSENSITIVETOPPCT;
	    result[2]=background[2]*INSENSITIVETOPPCT;
	    break;
	case sbutton_TOP:
	case sbutton_INSENSITIVELABELBG:
	case sbutton_LABELBG:
	case sbutton_BACKGROUND:
	    result[0]=background[0];
	    result[1]=background[1];
	    result[2]=background[2];
	    break;
    }
}
static void MyNewSetShade(class view  *self, struct sbutton_prefs  *prefs, double  *foreground , double  *background, int  color)
{
    double result[3];
    
    if(prefs->colors[color]) (self)->SetForegroundColor( prefs->colors[color], 0, 0, 0);
    else {
	double fg[3];
	double bg[3];
	if(color==sbutton_INSENSITIVELABELFG) {
	    MyNewSetShade(self, prefs, foreground, background, sbutton_LABELFG);
	    self->GetFGColor(&fg[0], &fg[1], &fg[2]);
	    foreground=fg;
	}
	if(color==sbutton_INSENSITIVELABELBG) {
	    MyNewSetShade(self, prefs, foreground, background, sbutton_LABELBG);
	    self->GetBGColor(&bg[0], &bg[1], &bg[2]);
	    background=bg;
	}
	MyNewComputeColor(self, prefs, foreground, background, color, result);
	(self)->SetFGColor( result[0], result[1], result[2]);
    }
}

static void MySetShade(class view  *self, struct sbutton_prefs  *prefs, double  *foreground , double  *background, int  color)
{
    if(newcolors && DEFAULTSTYLE(prefs->style)==sbutton_MOTIF) MyNewSetShade(self, prefs, foreground, background, color);
    else MyOldSetShade(self, prefs, foreground, background, color);
}

void sbuttonv::SaveViewState(class view  *self, struct sbuttonv_view_info  *vi)
{
	ATKinit;

    vi->transfermode=(self)->GetTransferMode();
    (self)->GetFGColor( &vi->fgr, &vi->fgg, &vi->fgb);
    (self)->GetBGColor( &vi->bgr, &vi->bgg, &vi->bgb);
    vi->font=(self)->GetFont();
}

void sbuttonv::RestoreViewState(class view  *self, struct sbuttonv_view_info  *vi)
{
	ATKinit;

    (self)->SetTransferMode( vi->transfermode);
    (self)->SetBGColor( vi->bgr, vi->bgg, vi->bgb);
    (self)->SetFGColor( vi->fgr, vi->fgg, vi->fgb);
    (self)->SetBGColor( vi->bgr, vi->bgg, vi->bgb);
    (self)->SetFont( vi->font);
}

void sbuttonv::SafeDrawButton(class view  *self, struct sbutton_info  *si, struct rectangle  *r)
{
	ATKinit;

    struct sbuttonv_view_info vi;
    sbuttonv::SaveViewState(self, &vi);
    sbuttonv::DrawButton(self, si, r);
    sbuttonv::RestoreViewState(self, &vi);
}

void sbuttonv::SizeForBorder(class view  *self, enum sbuttonv_conv  dir,  int  style, boolean  lit, long  w , long  h, long  *rw , long  *rh)
{
	ATKinit;

    style=DEFAULTSTYLE(style);
    
    switch(style) {
	case sbutton_BOXEDRECT:
	    if(dir==sbuttonv_Enclosing) {
		*rw=w + 2*BUTTONDEPTH + 2;
		*rh=h + 2*BUTTONDEPTH + 2;
	    } else {
		*rw=w - (2*BUTTONDEPTH + 2);
		*rh=h - (2*BUTTONDEPTH + 2);
	    }
	    break;
	case sbutton_THREEDEE:
	    if(dir==sbuttonv_Enclosing) {
		*rw=w + 2*BUTTONDEPTH;
		*rh=h + 2*BUTTONDEPTH;
	    } else {
		*rw=w - 2*BUTTONDEPTH;
		*rh=h - 2*BUTTONDEPTH;
	    }
	    break;
	case sbutton_MOTIF:
	    if(dir==sbuttonv_Enclosing) {
		*rw=w + 2*MOTIFBUTTONDEPTH;
		*rh=h + 2*MOTIFBUTTONDEPTH;
	    } else {
		*rw=w - 2*MOTIFBUTTONDEPTH;
		*rh=h - 2*MOTIFBUTTONDEPTH;
	    }
	    break;
	case sbutton_PLAINBOX:
	    if(dir==sbuttonv_Enclosing) {
		*rw=w + 2;
		*rh=h + 2;
	    } else {
		*rw=w - 2;
		*rh=h - 2;
	    }
	    break;
	default:
	    *rw=w;
	    *rh=h;
	    break;
    }
}

void sbuttonv::SizeForBorder(class view  *self, enum sbuttonv_conv  dir,  struct sbutton_prefs *prefs, boolean  lit, long  w , long  h, long  *rw , long  *rh)
{
    ATKinit;
    int bdepth=sbutton::GetDepth(prefs);
    int style=DEFAULTSTYLE(sbutton::GetStyle(prefs));
    
    switch(style) {
	case sbutton_BOXEDRECT:
	    if(bdepth<0) bdepth=BUTTONDEPTH;
	    if(dir==sbuttonv_Enclosing) {
		*rw=w + 2*bdepth + 2;
		*rh=h + 2*bdepth + 2;
	    } else {
		*rw=w - (2*bdepth + 2);
		*rh=h - (2*bdepth + 2);
	    }
	    break;
	case sbutton_THREEDEE:
	    if(bdepth<0) bdepth=BUTTONDEPTH;	    
	    if(dir==sbuttonv_Enclosing) {
		*rw=w + 2*bdepth;
		*rh=h + 2*bdepth;
	    } else {
		*rw=w - 2*bdepth;
		*rh=h - 2*bdepth;
	    }
	    break;
	case sbutton_MOTIF:
	    if(bdepth<0) bdepth=MOTIFBUTTONDEPTH;
	    if(dir==sbuttonv_Enclosing) {
		*rw=w + 2*bdepth;
		*rh=h + 2*bdepth;
	    } else {
		*rw=w - 2*bdepth;
		*rh=h - 2*bdepth;
	    }
	    break;
	case sbutton_PLAINBOX:
	    if(dir==sbuttonv_Enclosing) {
		*rw=w + 2;
		*rh=h + 2;
	    } else {
		*rw=w - 2;
		*rh=h - 2;
	    }
	    break;
	default:
	    *rw=w;
	    *rh=h;
	    break;
    }
}
void sbuttonv::DrawRectBorder(class view  *self, struct rectangle  *enclosing, struct sbutton_prefs  *prefs, boolean  inout , boolean  draw, struct rectangle  *interior, boolean fill)
{
	ATKinit;

    sbuttonv::DrawBorder(self, enclosing->left, enclosing->top, enclosing->width, enclosing->height, prefs, inout, draw, interior, fill);
}

void sbuttonv::DrawBorder(class view  *self, long  x , long  y , long  w , long  h, struct sbutton_prefs  *prefs, boolean  inout, boolean  draw, struct rectangle  *interior, boolean fill)
{
	ATKinit;

    struct rectangle Rect2;
    int bdepth, r_bot, r2_bot;
    struct rectangle r;
    int style;
    double foreground[3], background[3];
    int ts, bs;

    style=DEFAULTSTYLE(prefs->style);
    
    r.left=x;
    r.top=y;
    r.width=w;
    r.height=h;

    if(interior!=NULL) *interior=r;

    if(draw) {
	InitFGBG(self, prefs, &foreground[0], &background[0]);
	(self)->SetTransferMode( graphic::SOURCE);
	if (fill && (style != sbutton_THREEDEE) && (style != sbutton_MOTIF)) {
	    /* Erase with TOP color, only if style is not 3-D (3-D draws all bits) */
	    MySetShade(self, prefs,  foreground, background, sbutton_TOP);
	    (self)->FillRectSize( r.left, r.top, r.width, r.height, NULL);
	}
	MySetShade(self, prefs,  foreground, background, sbutton_FOREGROUND);
    }
    
    switch (style) {
	case sbutton_BOXEDRECT:
	    /* Rect2 is the inner rect */
	    bdepth = (prefs->bdepth > 0 ? prefs->bdepth : BUTTONDEPTH);
	    Rect2.top = r.top + bdepth;
	    Rect2.left = r.left + bdepth;
	    Rect2.width = r.width - 2*bdepth - 1;
	    Rect2.height = r.height - 2*bdepth - 1;
	    if(draw) {
		(self)->DrawRectSize( r.left, r.top, r.width -1, r.height - 1);
		(self)->DrawRect( &Rect2);
	    }
	    if(interior) {
		interior->top=Rect2.top+1;
		interior->left=Rect2.left+1;
		interior->width=Rect2.width-2;
		interior->height=Rect2.height-2;
	    }
	    if(inout && draw) {
		(self)->SetTransferMode( graphic::INVERT);
		(self)->FillRect( &r, (self)->BlackPattern());
		(self)->FillRect( &Rect2, (self)->BlackPattern());
	    }

	    break;

	case sbutton_THREEDEE:
	case sbutton_MOTIF:
	    ts=sbutton_TOPSHADOW;
	    bs=sbutton_BOTTOMSHADOW;
	    if (style == sbutton_MOTIF) {
		bdepth = (prefs->bdepth > 0 ? prefs->bdepth : MOTIFBUTTONDEPTH);
		if(inout) {
		    ts=sbutton_BOTTOMSHADOW;
		    bs=sbutton_TOPSHADOW;
		}
	    } else bdepth = (prefs->bdepth > 0 ? prefs->bdepth : BUTTONDEPTH);
	    /* Rect2 is the inner (Text) region */
	    Rect2.top = r.top + bdepth;
	    Rect2.left = r.left + bdepth;
	    Rect2.width = r.width - 2*bdepth;
	    Rect2.height = r.height - 2*bdepth;
	    if(interior) *interior=Rect2;
	    r2_bot = (Rect2.top)+(Rect2.height);
	    r_bot = (r.top)+(r.height);

	    if(draw) {
		{
		    (self)->SetTransferMode( graphic::SOURCE);
		    MySetShade(self, prefs, foreground, background, ts);
		    (self)->FillRectSize( r.left, r.top, bdepth, r.height, NULL);	/* left bar */

		    MySetShade(self, prefs, foreground, background, bs);
		    (self)->FillRectSize( r.left + r.width - bdepth, r.top, bdepth, r.height, NULL); /* right bar */
		    (self)->FillTrapezoid( Rect2.left, r2_bot, Rect2.width, r.left, r_bot, r.width, NULL); /* lower trapz */

		    MySetShade(self, prefs,  foreground, background, ts);
		    (self)->FillTrapezoid( r.left, r.top, r.width, Rect2.left, Rect2.top, Rect2.width, NULL); /* upper trapz */
	
		    if(fill) {
			MySetShade(self, prefs,  foreground, background, sbutton_TOP);
			(self)->FillRect( &Rect2, NULL); /* the middle box */
		    }
		}
	    }
	    break;

	case sbutton_PLAINBOX:
	    if(draw) (self)->DrawRectSize( r.left, r.top, r.width-1, r.height-1);
	    if(interior) {
		interior->left=r.left+1;
		interior->width=r.width-2;
		interior->top=r.top+1;
		interior->height=r.height-2;
	    }
	    /* FALLTHROUGH */
	default: ;
    } /* switch (style) */
    if(interior && interior->width<=0) interior->width=1;
    if(interior && interior->height<=0) interior->height=1;
}

void sbuttonv::EraseRectBorder(class view  *self, struct rectangle  *enclosing, struct sbutton_prefs  *prefs)
{
	ATKinit;

    sbuttonv::EraseBorder(self, enclosing->left, enclosing->top, enclosing->width, enclosing->height, prefs);
}

void sbuttonv::EraseBorder(class view  *self, long  x , long  y , long  w , long  h, struct sbutton_prefs  *prefs)
{
	ATKinit;

    struct rectangle Rect2;
    int bdepth, r_bot, r2_bot;
    struct rectangle r;
    int style;
    double foreground[3], background[3];
    int ts, bs;

    style=DEFAULTSTYLE(prefs->style);
    
    r.left=x;
    r.top=y;
    r.width=w;
    r.height=h;

    InitFGBG(self, prefs, &foreground[0], &background[0]);
    (self)->SetTransferMode(graphic::COPY);
    MySetShade(self, prefs,  foreground, background, sbutton_TOP);
    
    switch (style) {
	case sbutton_BOXEDRECT:
	    /* Rect2 is the inner rect */
	    bdepth = (prefs->bdepth > 0 ? prefs->bdepth : BUTTONDEPTH);
	    Rect2.top = r.top + bdepth;
	    Rect2.left = r.left + bdepth;
	    Rect2.width = r.width - 2*bdepth - 1;
	    Rect2.height = r.height - 2*bdepth - 1;
	    // (self)->DrawRectSize( r.left, r.top, r.width -1, r.height - 1);
	    // (self)->DrawRect( &Rect2);
	    self->FillRectSize(r.left, r.top, r.width-1, bdepth+1, NULL);
	    self->FillRectSize(r.left, Rect2.top+Rect2.height, r.width-1, 2*bdepth, NULL);
	    self->FillRectSize(r.left, r.top, bdepth+1, r.height-1, NULL);
	    self->FillRectSize(Rect2.left+Rect2.width, r.top, bdepth+1, r.height-1, NULL);	    
	    break;

	case sbutton_THREEDEE:
	case sbutton_MOTIF:
	    ts=sbutton_TOP;
	    bs=sbutton_TOP;
	    if (style == sbutton_MOTIF) {
		bdepth = (prefs->bdepth > 0 ? prefs->bdepth : MOTIFBUTTONDEPTH);
	    } else bdepth = (prefs->bdepth > 0 ? prefs->bdepth : BUTTONDEPTH);
	    /* Rect2 is the inner (Text) region */
	    Rect2.top = r.top + bdepth;
	    Rect2.left = r.left + bdepth;
	    Rect2.width = r.width - 2*bdepth;
	    Rect2.height = r.height - 2*bdepth;
	    r2_bot = (Rect2.top)+(Rect2.height);
	    r_bot = (r.top)+(r.height);

	   
	    {
		(self)->SetTransferMode( graphic::SOURCE);
		MySetShade(self, prefs, foreground, background, ts);
		(self)->FillRectSize( r.left, r.top, bdepth, r.height, NULL);	/* left bar */

		MySetShade(self, prefs, foreground, background, bs);
		(self)->FillRectSize( r.left + r.width - bdepth, r.top, bdepth, r.height, NULL); /* right bar */
		(self)->FillTrapezoid( Rect2.left, r2_bot, Rect2.width, r.left, r_bot, r.width, NULL); /* lower trapz */

		MySetShade(self, prefs,  foreground, background, ts);
		(self)->FillTrapezoid( r.left, r.top, r.width, Rect2.left, Rect2.top, Rect2.width, NULL); /* upper trapz */
	    }
	    break;

	case sbutton_PLAINBOX:
	    (self)->DrawRectSize( r.left, r.top, r.width-1, r.height-1);
	    /* FALLTHROUGH */
	default: ;
    } /* switch (style) */
}

void sbuttonv::InteriorBGColor(class view  *self, struct sbutton_prefs  *prefs, boolean  lit,  double  *result)
{
	ATKinit;

    double fore[3], back[3];
    struct sbuttonv_view_info vi;
    int style;
    
    sbuttonv::SaveViewState(self, &vi);
    
    InitFGBG(self, prefs, &fore[0], &back[0]);

    if(prefs->colors[sbutton_TOP]) {
	(self)->SetForegroundColor( prefs->colors[sbutton_TOP], 0, 0, 0);
	(self)->GetFGColor( result, result+1, result+2);
    } else {
	style=DEFAULTSTYLE(prefs->style);
	switch(style) {
	    case sbutton_MOTIF:
		if(newcolors) MyNewComputeColor(self, prefs, fore, back, sbutton_TOP, result);
		else MyOldComputeColor(self, prefs, fore, back, sbutton_TOP, result);
		break;
	    case sbutton_THREEDEE:
	    default:
		result[0]=back[0];
		result[1]=back[1];
		result[2]=back[2];
		break;
	}
    }
    sbuttonv::RestoreViewState(self, &vi);
}

static void DrawText(class view  *self, class fontdesc  *font, long  x , long  y, const char  *text, int  len, long  flags)
{
    if(flags==TEXTINMIDDLE && len==1 && font!=NULL) {
	long tx, ty;
	struct fontdesc_charInfo ci;
	(font)->CharSummary( (self)->GetDrawable(), *text, &ci);
	tx = ci.width + 4;
	ty = ci.height + 4;
	(self)->MoveTo( x - tx/2 + ci.xOriginOffset + 2, y - ty/2 + ci.yOriginOffset +2);
	(self)->DrawText( text, len, 0);
    } else {
	(self)->MoveTo( x, y);
	(self)->DrawText( text, len, flags);
    }
}

#define SOMEFONT(prefs) (sbutton::GetFont(prefs)?sbutton::GetFont(prefs):fontdesc::Create(FONT, FONTTYPE, FONTSIZE))

static void DrawLabel(class view  *self, const char  *text, boolean  lit, struct sbutton_prefs  *prefs, double  *fg , double  *bg, long  x , long  y, long  flags, boolean sensitive)
{
    int style;
    int len;
    int slb, slf;
    class fontdesc *my_fontdesc;

    if(*text=='\0') return;

    len=strlen(text);
    
    style = DEFAULTSTYLE(sbutton::GetStyle(prefs));

    if (!(my_fontdesc = sbutton::GetFont(prefs))) {
	my_fontdesc= fontdesc::Create(FONT, FONTTYPE, FONTSIZE);
    }
    if (my_fontdesc) {
	(self)->SetFont( my_fontdesc);
    }

    if(sensitive) {
	slb=sbutton_LABELBG;
	slf=sbutton_LABELFG;
    } else {
	slb=sbutton_INSENSITIVELABELBG;
	slf=sbutton_INSENSITIVELABELFG;
    }
    (self)->SetTransferMode( graphic::BLACK);
    switch(style) {
	case sbutton_THREEDEE:
	    if(lit) {
		if(sensitive) {
		    slb=sbutton_LABELFG;
		    slf=sbutton_LABELBG;
		} else {
		    slb=sbutton_INSENSITIVELABELFG;
		    slf=sbutton_INSENSITIVELABELBG;
		}
	    }
	    /* FALLTHROUGH */
	case sbutton_MOTIF:
	    MySetShade(self, prefs, fg, bg, slb);
	    DrawText(self, my_fontdesc, x+1, y, text, len, flags);
	    DrawText(self, my_fontdesc, x, y+1, text, len, flags);
	    DrawText(self, my_fontdesc, x+1, y+1, text, len, flags);

	    MySetShade(self, prefs, fg, bg,  slf);
	    DrawText(self, my_fontdesc, x, y, text, len, flags);
	    break;
	case sbutton_PLAINBOX:
	case sbutton_BOXEDRECT:
	default:
	    MySetShade(self, prefs, fg, bg, slf);
	    DrawText(self, my_fontdesc, x, y, text, len, flags);
    }
}
    
void sbuttonv::DrawLabel(class view  *self, const char  *text,  long  x , long  y, struct sbutton_prefs  *prefs, boolean  lit, long  flags, boolean sensitive)
{
	ATKinit;

    
    double fg[3], bg[3];

    if(text==NULL || *text=='\0') return;
    
    InitFGBG(self, prefs, fg, bg);

    if(flags<0) flags=TEXTINMIDDLE;
    ::DrawLabel(self, text, lit, prefs, fg, bg, x, y, flags, sensitive);
}

void sbuttonv::DrawButtonLabel(class view  *self, const char  *text, struct rectangle  *interior, struct sbutton_prefs  *prefs, boolean  lit, boolean sensitive)
{
	ATKinit;

    long tx, ty;
    int style=DEFAULTSTYLE(sbutton::GetStyle(prefs));
    if(style==sbutton_BOXEDRECT) {
	tx = TEXTPAD + interior->left + ((interior->width-1) / 2);
	ty = TEXTPAD + interior->top + ((interior->height-1) /2);
    } else {
	tx = interior->left + ((interior->width-1) / 2);
	ty = interior->top + ((interior->height-1) / 2);
    }
    sbuttonv::DrawLabel(self, text, tx, ty, prefs, lit, -1, sensitive);
}

void sbuttonv::DrawButton(class view  *self, struct sbutton_info  *si, struct rectangle  *r)
{
	ATKinit;

    int style;
    struct rectangle Rect2;
    int tx, ty;
    int bdepth, r_bot, r2_bot;
    const char *text=si->label?si->label:NO_MSG;
    double fg[3], bg[3];
    class fontdesc *my_fontdesc=SOMEFONT(si->prefs);
    class graphic *my_graphic=(self)->GetDrawable();
    long lwidth=0, lheight=0;
    unsigned long flags=TEXTINMIDDLE;
    
    if(my_fontdesc==NULL || my_graphic==NULL) return;
    
    if(strlen(text)==1) {
	struct fontdesc_charInfo ci;
	(my_fontdesc)->CharSummary( my_graphic, *text, &ci);
	lwidth=ci.width+4;
    } else {
	(my_fontdesc)->StringSize( my_graphic, text, &lwidth, &lheight);
    }
    style = DEFAULTSTYLE(sbutton::GetStyle(si->prefs));
    
    InitFGBG(self, si->prefs, fg, bg);
       
    int topc=si->sensitive?sbutton_TOP:sbutton_INSENSITIVETOP;
    (self)->SetTransferMode( graphic::SOURCE);
    if ((style != sbutton_THREEDEE) && (style != sbutton_MOTIF)) {
	/* Erase with top color, only if style is not 3-D (3-D draws all bits) */
	MySetShade(self, si->prefs,  fg, bg, topc);
	(self)->FillRectSize( r->left, r->top, r->width, r->height, NULL);
    }
    MySetShade(self, si->prefs,  fg, bg, sbutton_FOREGROUND);
    switch (style) {
	case sbutton_BOXEDRECT:
	    /* Rect2 is the inner rect */
	    bdepth = (si->prefs->bdepth > 0 ? si->prefs->bdepth : BUTTONDEPTH);
	    Rect2.top = r->top + bdepth;
	    Rect2.left = r->left + bdepth;
	    Rect2.width = r->width - 2*bdepth - 1;
	    Rect2.height = r->height - 2*bdepth - 1;
	    if(lwidth>Rect2.width) {
		tx = TEXTPAD + Rect2.left;
		flags=TEXTATLEFTMIDDLE;
	    } else tx = TEXTPAD + Rect2.left + ((Rect2.width-1) / 2);
	    ty = TEXTPAD + Rect2.top + ((Rect2.height-1) /2);
	    (self)->SetTransferMode( graphic::COPY);
	    (self)->DrawRectSize( r->left, r->top, r->width -1, r->height - 1);
	    (self)->DrawRect( &Rect2);
	    break;

	case sbutton_MOTIF:
	case sbutton_THREEDEE:
	    if (style == sbutton_MOTIF) {
		bdepth = (si->prefs->bdepth > 0 ? si->prefs->bdepth : MOTIFBUTTONDEPTH);
	    } else {
		bdepth = (si->prefs->bdepth > 0? si->prefs->bdepth : BUTTONDEPTH);
	    }


	    /* Rect2 is the inner (Text) region */
	    Rect2.top = r->top + bdepth;
	    Rect2.left = r->left + bdepth;
	    Rect2.width = r->width - 2*bdepth;
	    Rect2.height = r->height - 2*bdepth;

	    r2_bot = (Rect2.top)+(Rect2.height);
	    r_bot = (r->top)+(r->height);


	    (self)->SetTransferMode( graphic::COPY);
	    MySetShade(self, si->prefs,  fg, bg, sbutton_TOPSHADOW);
	    (self)->FillRectSize( r->left, r->top, bdepth, r->height, NULL);	/* left bar */

	    MySetShade(self, si->prefs,  fg, bg, sbutton_BOTTOMSHADOW);
	    (self)->FillRectSize( r->left + r->width - bdepth, r->top, bdepth, r->height, NULL); /* right bar */
	    (self)->FillTrapezoid( Rect2.left, r2_bot, Rect2.width, r->left, r_bot, r->width, NULL); /* lower trapz */

	    MySetShade(self, si->prefs,  fg, bg, sbutton_TOPSHADOW);
	    (self)->FillTrapezoid( r->left, r->top, r->width, Rect2.left, Rect2.top, Rect2.width, NULL); /* upper trapz */

	    MySetShade(self, si->prefs,  fg, bg, topc);
	    (self)->FillRect( &Rect2, NULL); /* the middle box */

	    MySetShade(self, si->prefs,  fg, bg, sbutton_FOREGROUND);

	    if(lwidth>Rect2.width) {
		tx = Rect2.left + TEXTPAD;
		flags=TEXTATLEFTMIDDLE;
	    } else tx = Rect2.left + ((Rect2.width-1) / 2);
	    ty = Rect2.top + ((Rect2.height-1) / 2);
	    
	    break;

	case sbutton_PLAINBOX:
	    
	    (self)->SetTransferMode( graphic::BLACK);
	    (self)->DrawRectSize( r->left, r->top, r->width-1, r->height-1);

	    if(lwidth>r->width) {
		tx = r->left + TEXTPAD;
		flags=TEXTATLEFTMIDDLE;
	    } else tx = r->left + ((r->width-1) / 2);
	    ty = r->top + ((r->height-1) / 2);

	    
	    break;

	default: /* PLAIN */
	    if(lwidth>r->width) {
		tx= r->left + TEXTPAD;
		flags=TEXTATLEFTMIDDLE;
	    } else tx = r->left + ((r->width-1) / 2);
	    ty = r->top + ((r->height-1) / 2);
	    break;
    } /* switch (style) */
    ::DrawLabel(self, text, FALSE, si->prefs, fg, bg, tx, ty, flags, si->sensitive);
    if(si->lit) sbuttonv::HighlightButton(self, si, r);
}

boolean sbuttonv::InitializeClass()
{
    buttonpushed=atom::Intern("buttonpushed");
    if(buttonpushed==NULL) return FALSE;
    newcolors = environ::GetProfileSwitch("UseNewShadows", TRUE);
    newcolors = environ::GetProfileSwitch("SButtonUseNewShadows", newcolors);
    return(TRUE);
}

sbuttonv::sbuttonv()
{
	ATKinit;

/*
  Set up the data for each instance of the object.
*/
    this->lwidth=this->lheight=(-1);
    
    this->bcount=0;
    
    this->info=NULL;
    
    this->dotriggers=TRUE;
    
    this->activebuttons=sbuttonv_LEFTBUTTON | sbuttonv_RIGHTBUTTON;
    
    this->vborder=environ::GetProfileInt("sbuttonvborder", 5);
    this->hborder=environ::GetProfileInt("sbuttonhborder", 5);

    this->hspacing=environ::GetProfileInt("sbuttonhspacing", 5);
    this->vspacing=environ::GetProfileInt("sbuttonvspacing", 5);
    this->needredraw=FALSE;
    this->lastbutton=0;
    this->forceupdate=FALSE;
    this->lasthighlight=(-1);
    this->awaitingUpdate = 0;
    observable::DefineTrigger(this->ATKregistry(), buttonpushed);
    THROWONFAILURE((TRUE));
}

unsigned char sbuttonv::SetActiveMouseButtons(unsigned char active , unsigned char deactive)
{
    unsigned char oldbs=this->activebuttons;
    this->activebuttons|=active;
    this->activebuttons&=~deactive;
    return oldbs;
}

void sbuttonv::LinkTree(class view  *parent)
{
    (this)->view::LinkTree( parent);

} /* sbuttonv__LinkTree */



sbuttonv::~sbuttonv()
{
	ATKinit;

    if(this->info) free(this->info);
}

static void EnsureInfo(class sbuttonv  *self)
{
    class sbutton *b=(self)->ButtonData();
    long ocount=self->bcount;
    if(b->count!=self->bcount) {
	self->bcount=b->count;

	if(self->info) self->info=(struct sbuttonv_info *)realloc(self->info, self->bcount*sizeof(struct sbuttonv_info));
	else self->info=(struct sbuttonv_info *)malloc(self->bcount*sizeof(struct sbuttonv_info));
	if(!self->info) self->bcount=0;
	while(ocount<self->bcount) {
	    self->info[ocount].drawflag=sbutton_LABELCHANGED;
	    ocount++;
	}
    }
}

#define MAXWIDTH(col, max, left) ((col)<(left)?max+1:max)
#define MAXHEIGHT(row, max, left) ((row)<(left)?max+1:max)

void sbuttonv::FullUpdate(enum view::UpdateType  type, long  left , long  top , long  width , long  height)
{
/*
  Redisplay this object.  Specifically, set my font, and put my text label
  in the center of my display box. */
  struct rectangle vr;
  class sbutton *b =  (this)->ButtonData();
  int redraw;
  
  (this)->GetLogicalBounds( &vr);

  this->lwidth=vr.width;
  this->lheight=vr.height;
  
  if((b)->GetMattePrefs()) sbuttonv::DrawRectBorder((class view *)this, &vr, (b)->GetMattePrefs(), TRUE, TRUE, &vr);
  
  switch (type) {
    case view::FullRedraw:
    case view::LastPartialRedraw:
      redraw = 1;
      break;
    case view::MoveNoRedraw:
      redraw = 0;
      break;
  case view::PartialRedraw:
      return;
    case view::Remove:
     return;
    default:
      redraw = 1;
  }


  if (b) {
      int count=b->count;
      int maxwidth=(rectangle_Width(&vr) - 2*this->hborder + this->hspacing)/b->cols;
      int maxheight=(rectangle_Height(&vr) - 2*this->vborder + this->vspacing)/b->rows;
      int x=rectangle_Left(&vr) + this->hborder, y=rectangle_Top(&vr) + this->vborder;
      int vleftovers=(rectangle_Height(&vr)  - 2*this->vborder + this->vspacing) % b->rows;
      int hleftovers=(rectangle_Width(&vr) - 2*this->hborder + this->hspacing) % b->cols;
      int col=0;
      int row=0;
      struct sbuttonv_view_info v;
      
      this->maxwidth=this->specialwidth=maxwidth;
      this->maxheight=maxheight;

       
      if(redraw) sbuttonv::SaveViewState((class view *)this, &v);
      EnsureInfo(this);
      
      while(--count>=0) {
	  rectangle_SetRectSize( &this->info[count].rect, x, y, MAXWIDTH(col, maxwidth, hleftovers) - this->hspacing, MAXHEIGHT(row, maxheight, vleftovers) - this->vspacing);
	  if(redraw) {
	      this->info[count].drawflag=0;
	      sbuttonv::DrawButton((class view *)this, &BUTTONS(this)[count], &this->info[count].rect);
	  }
	  x+=MAXWIDTH(col, maxwidth, hleftovers);
	  col++;
	  if(x + MAXWIDTH(col, maxwidth-1, hleftovers) + this->hborder - this->hspacing> rectangle_Right(&vr)) {
	      col=0;
	      x=rectangle_Left(&vr) + this->hborder;
	      if(count>0 && count<b->cols) {
		  this->specialwidth = maxwidth = (rectangle_Width(&vr) - 2*this->hborder + this->hspacing)/count;
		  hleftovers=rectangle_Width(&vr)%maxwidth;
	      }
	      y+=MAXHEIGHT(row, maxheight, vleftovers);
	      row++;
	  }
      }
      if(redraw) sbuttonv::RestoreViewState((class view *)this, &v);
  } /* if (b && redraw) */
}


void sbuttonv::Update()
{

    class sbutton *b=(this)->ButtonData();
    struct rectangle r;
    int i;
    (this)->GetLogicalBounds( &r);

    this->awaitingUpdate=FALSE;

    if(this->needredraw || this->bcount!=b->count || r.width!=this->lwidth || r.height!=this->lheight) {
	this->needredraw=FALSE;
	(this)->EraseRect( &r);
	(this)->FullUpdate( view::FullRedraw, r.left, r.top, r.width, r.height);
	return;
    }

    for(i=0;i<this->bcount;i++) {
	if(this->info[i].drawflag&(sbutton_LABELCHANGED|sbutton_SENSITIVITYCHANGED)) sbuttonv::DrawButton((class view *)this, &BUTTONS(this)[i], &this->info[i].rect);
	else if(this->info[i].drawflag&sbutton_ACTIVATIONCHANGED) {
	    if((b)->GetLit( i))
		sbuttonv::HighlightButton(this, &(b)->GetButtons()[i], &this->info[i].rect);
	    else sbuttonv::UnHighlightButton(this, &(b)->GetButtons()[i], &this->info[i].rect);
	}
	this->info[i].drawflag=0;
    }
    ((this)->GetIM())->FlushGraphics();
}


static int RectEnclosesXY(struct rectangle  *r, long  x , long  y)
{
  return(   ( ((r->top)  <= y) && ((r->top + r->height) >= y) )
	 && ( ((r->left) <= x) && ((r->left + r->width) >= x) )
	 );
}



 void sbuttonv::HighlightButton(class view  *self, struct sbutton_info  *si, struct rectangle  *r)
{
	ATKinit;

    struct rectangle Rect2;
    class fontdesc *my_fontdesc;
    int tx, ty;
    int bdepth, r2_bot, r_bot;
    struct sbuttonv_view_info vi;
    int style=DEFAULTSTYLE(sbutton::GetStyle(si->prefs));
    const char *text=si->label?si->label:NO_MSG;
    double fg[3], bg[3];
    
    sbuttonv::SaveViewState(self, &vi);

    InitFGBG(self, si->prefs, fg, bg);

    switch (style) {
	case sbutton_PLAIN:
	case sbutton_PLAINBOX:
	    (self)->SetTransferMode( graphic::INVERT);
	    (self)->FillRect( r, (self)->BlackPattern());
	    break;

	case sbutton_BOXEDRECT:
	    /* Rect2 is the inner si->r */
	    bdepth = (si->prefs->bdepth > 0 ? si->prefs->bdepth : BUTTONDEPTH);
	    Rect2.top = r->top + bdepth;
	    Rect2.left = r->left + bdepth;
	    Rect2.width = r->width - 2*bdepth;
	    Rect2.height = r->height - 2*bdepth;

	    (self)->SetTransferMode( graphic::INVERT);
	    (self)->FillRect( r, (self)->BlackPattern());
	    (self)->FillRect( &Rect2, (self)->BlackPattern());

	    break;

	case sbutton_MOTIF:
	case sbutton_THREEDEE:
	    if (style == sbutton_MOTIF) {
		bdepth = (si->prefs->bdepth > 0 ? si->prefs->bdepth : MOTIFBUTTONDEPTH);
	    } else {
		bdepth = (si->prefs->bdepth > 0 ? si->prefs->bdepth : BUTTONDEPTH);
	    }

	    /* Rect2 is the inner (Text) region */
	    Rect2.top = r->top + bdepth;
	    Rect2.left = r->left + bdepth;
	    Rect2.width = r->width - 2*bdepth;
	    Rect2.height = r->height - 2*bdepth;
	    r2_bot = (Rect2.top)+(Rect2.height);
	    r_bot = (r->top)+(r->height);

	    if (style == sbutton_MOTIF) {
		(self)->SetTransferMode( graphic::COPY);
		MySetShade(self, si->prefs,  fg, bg, sbutton_BOTTOMSHADOW);
		(self)->FillRectSize( r->left, r->top, bdepth, r->height, NULL);	/* left bar */

		MySetShade(self, si->prefs,  fg, bg, sbutton_TOPSHADOW);
		(self)->FillRectSize( r->left + r->width - bdepth, r->top, bdepth, r->height, NULL); /* right bar */
		(self)->FillTrapezoid( Rect2.left, r2_bot, Rect2.width, r->left, r_bot, r->width, NULL); /* lower trapz */

		MySetShade(self, si->prefs,  fg, bg, sbutton_BOTTOMSHADOW);
		(self)->FillTrapezoid( r->left, r->top, r->width, Rect2.left, Rect2.top, Rect2.width, NULL); /* upper trapz */

		MySetShade(self, si->prefs,  fg, bg, sbutton_FOREGROUND);

	    } else {
		if (!(my_fontdesc = sbutton::GetFont(si->prefs))) {
		    my_fontdesc= fontdesc::Create(FONT, FONTTYPE, FONTSIZE);
		}
		if (my_fontdesc) {
		    (self)->SetFont( my_fontdesc);
		}

		tx = r->left + ((r->width-1) / 2);
		ty = r->top + ((r->height-1) / 2);
		::DrawLabel(self, text, TRUE, si->prefs, fg, bg, tx, ty, TEXTINMIDDLE, si->sensitive);
	    } /* MOTIF? */

	    break;
    }
    sbuttonv::RestoreViewState(self, &vi);
    si->lit = 1;
}



 void sbuttonv::UnHighlightButton(class view  *self, struct sbutton_info  *si, struct rectangle  *r)
{
	ATKinit;

    struct rectangle Rect2;
    int tx, ty;
    int bdepth, r2_bot, r_bot;
    const char *text=si->label?si->label:NO_MSG;
    double fg[3], bg[3];
    int style=DEFAULTSTYLE(sbutton::GetStyle(si->prefs));
    
    struct sbuttonv_view_info vi;
    sbuttonv::SaveViewState(self, &vi);
    InitFGBG(self, si->prefs, fg, bg);
    
    switch (style) {
	case sbutton_PLAIN:
	case sbutton_PLAINBOX:
	    (self)->SetTransferMode( graphic::INVERT);
	    (self)->FillRect( r, (self)->BlackPattern());
	    break;

	case sbutton_BOXEDRECT:
	    /* Rect2 is the inner rect */
	    bdepth = (si->prefs->bdepth > 0 ? si->prefs->bdepth : BUTTONDEPTH);
	    Rect2.top = r->top + bdepth;
	    Rect2.left = r->left + bdepth;
	    Rect2.width = r->width - 2*bdepth;
	    Rect2.height = r->height - 2*bdepth;

	    (self)->SetTransferMode( graphic::INVERT);
	    (self)->FillRect( r, (self)->BlackPattern());
	    (self)->FillRect( &Rect2, (self)->BlackPattern());

	    break;

	case sbutton_MOTIF:
	case sbutton_THREEDEE:
	    if (style == sbutton_MOTIF) {
		bdepth = (si->prefs->bdepth > 0 ? si->prefs->bdepth : MOTIFBUTTONDEPTH);
	    } else {
		bdepth = (si->prefs->bdepth > 0 ? si->prefs->bdepth : BUTTONDEPTH);
	    }


	    /* Rect2 is the inner (Text) region */
	    Rect2.top = r->top + bdepth;
	    Rect2.left = r->left + bdepth;
	    Rect2.width = r->width - 2*bdepth;
	    Rect2.height = r->height - 2*bdepth;
	    r2_bot = (Rect2.top)+(Rect2.height);
	    r_bot = (r->top)+(r->height);

	    if (style == sbutton_MOTIF) {
		(self)->SetTransferMode( graphic::COPY);
		MySetShade(self, si->prefs,  fg, bg, sbutton_TOPSHADOW);
		(self)->FillRectSize( r->left, r->top, bdepth, r->height, NULL);	/* left bar */

		MySetShade(self, si->prefs,  fg, bg, sbutton_BOTTOMSHADOW);
		(self)->FillRectSize( r->left + r->width - bdepth, r->top, bdepth, r->height, NULL); /* right bar */
		(self)->FillTrapezoid( Rect2.left, r2_bot, Rect2.width, r->left, r_bot, r->width, NULL); /* lower trapz */

		MySetShade(self, si->prefs,  fg, bg, sbutton_TOPSHADOW);
		(self)->FillTrapezoid( r->left, r->top, r->width, Rect2.left, Rect2.top, Rect2.width, NULL); /* upper trapz */

		MySetShade(self, si->prefs,  fg, bg, sbutton_FOREGROUND);

	    } else {
		tx = r->left + ((r->width-1) / 2);
		ty = r->top + ((r->height-1) / 2);
		
		::DrawLabel(self, text, FALSE, si->prefs, fg, bg, tx, ty, TEXTINMIDDLE, si->sensitive);
	    } /* MOTIF? */

	    break;
    }
    sbuttonv::RestoreViewState(self, &vi);
    si->lit=FALSE;
}


int sbuttonv::WhichButton(long  x , long  y)
{
    int row, col, spill, button;
    int i;
    class sbutton *b=(this)->ButtonData();
    row=(y-this->vborder)/(this->maxheight);
    if(row<0 || row>=b->rows) return -1;
    spill=b->count-(row+1)*b->cols;
    if(spill<0) {
	col=(x-this->hborder)/this->specialwidth;
    } else col = (x - this->hborder)/this->maxwidth;
    if(col<0 ||col>=b->cols) return -1;
   
    button=b->count - row*b->cols - col -1;
   
    if(button>=0 && button<b->count) {
	if(x>=rectangle_Left(&this->info[button].rect) && x<rectangle_Right(&this->info[button].rect) && y>=rectangle_Top(&this->info[button].rect) && y<rectangle_Bottom(&this->info[button].rect)) return button;
	else for(i=0;i<b->count;i++) { if(x>=rectangle_Left(&this->info[i].rect) && x<rectangle_Right(&this->info[i].rect) && y>=rectangle_Top(&this->info[i].rect) && y<rectangle_Bottom(&this->info[i].rect)) return i;
	}
	return button;
    } else return -1;
}

boolean sbuttonv::Touch(int  ind, enum view::MouseAction  action)
{
    class cursor *wait_cursor;
    class sbutton *b=(this)->ButtonData();
    struct owatch_data *w1, *w2;
    if(action!=view::LeftUp && action!=view::RightUp) return TRUE;
    w1=owatch::Create(this);
    w2=owatch::Create(b);
    if (ind>=0 && ind<b->count && BUTTONS(this)[ind].lit && ((this->activebuttons&sbuttonv_LEFTBUTTON) || (action==view::RightUp && this->activebuttons&sbuttonv_RIGHTBUTTON))) {
	if ((wait_cursor = cursor::Create(this))) {
	    (wait_cursor)->SetStandard( Cursor_Wait);
	    im::SetProcessCursor(wait_cursor);
	    (b)->Actuate( this->lastbutton);
	    if(owatch::Check(w1) && this->dotriggers) {
		
		if(owatch::Check(w2) && (b)->GetTrigger( ind)) {
		    (this)->PullTrigger( (b)->GetTrigger( ind));
		} else {
		    (this)->PullTrigger( buttonpushed);
		}
	    }
	    im::SetProcessCursor(NULL);
	    delete wait_cursor;
	}
    }
    return owatch::CheckAndDelete(w1) && owatch::CheckAndDelete(w2);
}


class view *sbuttonv::Hit(enum view::MouseAction  action, long  x , long  y, long  numclicks  )
{
    /*
      Handle the button event.  Currently, semantics are:

      Left/Right Down  -- Draw button pressed
      Left/Right Up    -- draw button at rest, pull trigger
      Left/Right Movement     -- unhighlight if moved off, highlight if moved on
	  */
    class sbutton *b=(this)->ButtonData();
    struct rectangle r;
    (this)->GetLogicalBounds( &r);
    if(!b->count) return((class view *)this);

    if(!RectEnclosesXY(&r, x, y)) {
	if( this->lasthighlight>=0 && this->lasthighlight<b->count) {
	    
	    (b)->DeActivateButton( this->lasthighlight);
	    this->lasthighlight=(-1);
	}
	return((class view *)this);
    }
    this->lastbutton=(this)->WhichButton( x, y);
    if(this->lastbutton<0) {
	if(this->lasthighlight>=0 && this->lasthighlight<b->count) { 
	    (b)->DeActivateButton( this->lasthighlight);
	    
	    this->lasthighlight=(-1);
	}
	return((class view *)this);
    }
    if(this->lastbutton>=this->bcount) {
	return ((class view *)this);
    }
    if((this)->Touch( this->lastbutton, action)) {
	switch (action) {
	    case view::RightMovement:
	    case view::RightDown:
	    case view::LeftDown:
	    case view::LeftMovement: {

		if(this->lasthighlight!=this->lastbutton && this->lasthighlight>=0 && this->lasthighlight<b->count) { 
		    (b)->DeActivateButton( this->lasthighlight);
		   
		    this->lasthighlight=(-1);
		}


		this->lasthighlight=this->lastbutton;

		if(!(b)->GetLit( this->lastbutton)) (b)->ActivateButton( this->lastbutton);
		}
		break;
	    case view::RightUp:
	    case view::LeftUp: {
		if(this->lasthighlight>=0 && this->lasthighlight<b->count) { 
		    (b)->DeActivateButton( this->lasthighlight);
		    
		    this->lasthighlight=(-1);
		}
		}
		break;
	    default:
	        break;
	}
    }
    return((class view *)this);
}


void sbuttonv::ObservedChanged(class observable  *ob, long  v)
{
    class sbutton *b=(class sbutton *)ob;
    class sbutton *b2=(this)->ButtonData();
    long bchange=(v>=sbutton_CHANGEBASE && v<sbutton_CHANGEBASE+this->bcount)?v-sbutton_CHANGEBASE:-1;
    long change=(b2)->GetChangeType();
    int i;
	
    if(b2!=b || v==observable::OBJECTDESTROYED) return;

    this->needredraw=FALSE;
    
    if(this->bcount != b2->count || change & sbutton_SIZECHANGED || change&(sbutton_LABELCHANGED | sbutton_FONTCHANGED | sbutton_SIZECHANGED | sbutton_STYLECHANGED)) {
	this->needredraw=TRUE;
	(this)->WantNewSize( this);
    }

    if(change&sbutton_ALLCHANGED) {
	for(i=0;i<this->bcount;i++) this->info[i].drawflag|=sbutton_LABELCHANGED;
    }
    
    if(bchange<0) {
	(this)->view::ObservedChanged( b, observable::OBJECTCHANGED);
	return;
    }
    
    if(change&sbutton_TRIGGERCHANGED) {
	if((b)->GetTrigger( bchange)) {
	    observable::DefineTrigger(this->ATKregistry(), (b)->GetTrigger( bchange));
	    return;
	}
    }

    if(change&(sbutton_LABELCHANGED | sbutton_FONTCHANGED | sbutton_STYLECHANGED)) this->info[bchange].drawflag|=sbutton_LABELCHANGED;

    if(change&sbutton_ACTIVATIONCHANGED) {
	this->forceupdate=TRUE;
	this->info[bchange].drawflag|=sbutton_ACTIVATIONCHANGED;
    }
    
    if(change&sbutton_SENSITIVITYCHANGED) {
	this->forceupdate=TRUE;
	this->info[bchange].drawflag|=sbutton_SENSITIVITYCHANGED;
    }
	
	
    (this)->view::ObservedChanged( b, observable::OBJECTCHANGED);
}


view::DSattributes sbuttonv::DesiredSize(long  width, long  height, enum view::DSpass  pass, long  *desired_width, long  *desired_height)
{
    /* 
      Tell parent that this object  wants to be as big as the box around its
      text string.  For some reason IM allows resizing of this object. (BUG)
      */
    long maxheight=0;
    long maxwidth=0;
    class fontdesc *my_fontdesc;
    struct FontSummary *my_FontSummary = NULL;
    class graphic *my_graphic;
    class sbutton *b = (this)->ButtonData();
    int style;
    int count=b->count;
    
    my_graphic = (class graphic *)(this)->GetDrawable();
    if(!b || my_graphic == NULL) {
	*desired_width=256;
	*desired_height=256;
	return (view::DSattributes)(view::HeightFlexible|view::WidthFlexible);
    }
    
    while(--count>=0) {
	long lwidth, lheight=1;
	struct sbutton_prefs *prefs=(b)->GetPrefs( count);
	int bdepth;
	style = DEFAULTSTYLE(sbutton::GetStyle(prefs));
	if (!(my_fontdesc = sbutton::GetFont(prefs))) {
	    my_fontdesc= fontdesc::Create(FONT, FONTTYPE, FONTSIZE);
	}
	if (my_fontdesc) {
	    const char *label=(b)->GetLabel( count) ? (b)->GetLabel( count) : NO_MSG;
	    struct fontdesc_charInfo ci;
	    if(strlen(label)==1) {
		(my_fontdesc)->CharSummary( my_graphic, *label, &ci);
		lwidth=ci.width+4;
		lheight=ci.height+4;
	    } else {

		(my_fontdesc)->StringSize( my_graphic, label, &lwidth, &lheight);
		my_FontSummary =  (my_fontdesc)->FontSummary( my_graphic);
		if(my_FontSummary) lheight=my_FontSummary->maxHeight;
		
	    }
	}
	switch (style) {
	    case sbutton_PLAIN:
	    case sbutton_PLAINBOX:
		lwidth += 2*TEXTPAD + 2*EXTRATEXTPAD;
		if(lwidth>maxwidth) maxwidth=lwidth;
		if (my_FontSummary && lheight + 2*TEXTPAD > maxheight) maxheight = lheight + 2*TEXTPAD;
		break;
	    case sbutton_MOTIF:
		bdepth = (prefs->bdepth > 0 ? prefs->bdepth : MOTIFBUTTONDEPTH);
		lwidth = lwidth + 2*TEXTPAD + 2*bdepth;
		if(lwidth>maxwidth) maxwidth=lwidth;
		if (my_FontSummary) {
		    lheight = lheight + 2*TEXTPAD + 2*bdepth;
		    if(lheight > maxheight) maxheight = lheight;
		}
		break;
	    case sbutton_BOXEDRECT:
	    case sbutton_THREEDEE:
		bdepth = (prefs->bdepth > 0 ? prefs->bdepth : BUTTONDEPTH);
		lwidth = lwidth + 2*TEXTPAD + 2*bdepth;
		if(lwidth>maxwidth) maxwidth=lwidth;
		if (my_FontSummary) {
		    lheight = lheight + 2*TEXTPAD + 2*bdepth;
		    if(lheight > maxheight) maxheight = lheight;
		}
		break;
	}
    }
   
    *desired_height=(maxheight+this->vspacing)*b->rows + 2*this->vborder - this->vspacing;
    *desired_width=(maxwidth+this->hspacing)*b->cols + 2*this->hborder - this->hspacing;
    return view::Fixed;
}

void sbuttonv::GetOrigin(long  width , long  height, long  *originX , long  *originY)
{
/*
  We want this object to sit in-line with text, not below the baseline.
  Simply, we could negate the height as the originX, but then our
  text would be too high.  So, instead, we use the height of
  our font above the baseline
*/

  struct FontSummary *my_FontSummary;
  class fontdesc *my_fontdesc;
  class graphic *my_graphic;
  class sbutton *b = (this)->ButtonData();
  int style, bdepth;
  long maxheight=0, maxbelow=0;
  struct sbutton_prefs *prefs=b->count?(b)->GetPrefs( b->count-1):NULL;

  /* This only really makes sense if there is only one button... should I check and do something different if there is more than one button?  this should at least still do the right thing for one button. */
  style = DEFAULTSTYLE(b->count ? sbutton::GetStyle(prefs) : sbutton_MOTIF);

  my_graphic = (class graphic *)(this)->GetDrawable();
  if (!(my_fontdesc = (prefs?sbutton::GetFont(prefs):NULL))) {
    my_fontdesc= fontdesc::Create(FONT, FONTTYPE, FONTSIZE);
  }
  if (my_fontdesc) {
      if(b->count && (b)->GetLabel( b->count-1) && strlen((b)->GetLabel( b->count-1))==1) {
	  struct fontdesc_charInfo ci;
	  (my_fontdesc)->CharSummary( my_graphic, (b)->GetLabel( b->count-1)[0], &ci);
	  maxheight=ci.height+4;
      } else {
	  my_FontSummary =  (my_fontdesc)->FontSummary( my_graphic);
	  maxheight=my_FontSummary->maxHeight;
	  maxbelow=my_FontSummary->maxBelow;
      }
  }

  *originX = 0;
  switch (style) {
    case sbutton_PLAIN:
    case sbutton_PLAINBOX:
      if (my_FontSummary)
	*originY = maxheight - maxbelow + 1 + (this)->GetVBorder();
      break;
    case sbutton_MOTIF:
      bdepth = (prefs->bdepth > 0 ? prefs->bdepth : MOTIFBUTTONDEPTH);
      if (my_FontSummary) 
	*originY = maxheight - maxbelow + 1 + TEXTPAD + bdepth + (this)->GetVBorder();
      break;
    case sbutton_BOXEDRECT:
    case sbutton_THREEDEE:
      bdepth = (prefs->bdepth > 0 ? prefs->bdepth : BUTTONDEPTH);
      if (my_FontSummary) 
	*originY = maxheight - maxbelow + 1 + TEXTPAD + bdepth + (this)->GetVBorder();
      break;
  }
  return;
}

static boolean definetriggers(class sbutton  *b, int  i, struct sbutton_info  *si, class sbuttonv  *self)
{
    if((b)->GetTrigger( i)) {
	observable::DefineTrigger(self->ATKregistry(), (b)->GetTrigger( i));
    }
    return FALSE;
}

void sbuttonv::SetDataObject(class dataobject  *db)
{
    class sbutton *b=(class sbutton *)db;
    if((b)->GetMaxCount()==0) (b)->EnsureSize( 0);
    (b)->Enumerate( (sbutton_efptr)definetriggers, (long)this);
    (this)->view::SetDataObject( b);	
}

void sbuttonv::WantUpdate(class view  *requestor)
{
    if ((class view *) this == requestor) {

	if(this->forceupdate && (this)->GetIM()) {
	    this->awaitingUpdate = TRUE;
	    im::ForceUpdate();
	    this->forceupdate=FALSE;
	} else {
	    if (this->awaitingUpdate) return;
	    this->awaitingUpdate = TRUE;
	}
    }
    (this)->view::WantUpdate( requestor);
    /* if this is an activation change make it happen NOW! */
} /* sbuttonv__WantUpdate */

class sbuttonv *sbuttonv::CreateFilledSButtonv(const char  *defview, struct sbutton_prefs  *prefs, const struct sbutton_list  *blist)
{
	ATKinit;

    class sbutton *data=sbutton::CreateFilledSButton(prefs, blist);
    class sbuttonv *self;
    
    if(!data) return NULL;

    if(defview) {
	if(!ATK::IsTypeByName(defview, "sbuttonv")) {
	    (data)->Destroy();
	    return NULL;
	}
    } else defview=(data)->ViewName();
    
    self=(class sbuttonv *)ATK::NewObject(defview);
    if(!self) {
	(data)->Destroy();
	return NULL;
    }
    (self)->SetDataObject( data);
    return self;
}


/* # # # # # # # # # # # # # # 
 *	PRINTING	
 *  # # # # # # # # # # # # #  */

static void OutputLabel(FILE  *f, const char  *l)
{
    while(*l) {
	if(l[0]=='\\' || l[0]=='\"') fprintf(f, "\\\\");
	else fprintf(f, "%c", l[0]);
	l++;
    }
}

void sbuttonv::Print(FILE   *file, const char    *processor, const char    *format, boolean    topLevel)
{
	int count;
	class sbutton *dobj = (class sbutton *)this->dataobject;

	if (strcmp(processor, "troff") == 0) {
		/* output to troff */
	    if (topLevel) {
		fprintf(stderr, "Warning... sbuttons cannot print as top level objects.\n");
			/* take care of initial troff stream 
			texttroff_BeginDoc(file); */
	    }

		fprintf(file, ".de bx\n\\(br\\|\\\\$1\\|\\(br\\l'|0\\(rn'\\l'|0\\(ul'\n..\n");
		fprintf(file, ".bx \"");
		for(count=(dobj)->GetCount()-1;count>=0;count--) {
		    const char *label=(dobj)->GetLabel( count);
		    if(label==NULL) label="Push Me";
		    fprintf(file, "[");
		    OutputLabel(file, label);
		    fprintf(file, "]%s", count>0?" ":"");
		}
		fprintf(file, "\"\\\n");
		    
	} else {
	    /* guess we're trying to write in postscript, no idea how to try this out though... */
	    fprintf(file, "(");
	    for(count=(dobj)->GetCount()-1;count>=0;count--) {
		const char *label=(dobj)->GetLabel( count);
		if(label==NULL) label="Push Me";
		fprintf(file, "[");
		OutputLabel(file, label);
		fprintf(file, "]%s", count>0?" ":"");
	    }
	    fprintf(file, ") show");
      }


	if (strcmp(processor, "troff") == 0){
	    fprintf(file, "\n");
		/* if (topLevel)
			texttroff_EndDoc(file); */
	}
}
