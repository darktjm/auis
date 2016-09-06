/* Copyright 1996 Carnegie Mellon University All rights reserved.

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

/* ashadow.C		
	Draw borders around widgets
*/

#include <andrewos.h>
ATK_IMPL("ashadow.H")
#include <ashadow.H>

#include <awidget.H>
#include <awidgetview.H>
#include <util.h>

ASlot_NAME(sensitive);
ASlot_NAME(set);
ASlot_NAME(fillOnArm);
ASlot_NAME(highlightThickness);
ASlot_NAME(shadowThickness);
ASlot_NAME(defaultShadowThickness);
ASlot_NAME(showAsDefault);
static const ASlot_NameVar AShadowColors_Resources[AShadowColors_Colors]={
    "Background",
    "Foreground",
    "UpperShadow",
    "LowerShadow",
    "LabelFG",
    "LabelBG",
    "InsensitiveLabelFG",
    "InsensitiveLabelBG",
    "InsensitiveBackground",
    "InsensitiveForeground",
    "SelectedForeground",
    "SelectedBackground",
    "Transparent",
    "Custom",
    "DefaultButtonUS",
    "DefaultButtonLS",
    "Highlight"
};

static const char *abbrev[AShadowColors_Colors]={
    "B",
    "F",
    "U",
    "L",
    "LF",
    "LB",
    "ILF",
    "ILB",
    "IB",
    "IF",
    "SF",
    "SB",
    "T",
    "C",
    "DU",
    "DL",
    "H"
};

ASlot_NAME(borderShape);

static AShadowColors MapShade(const char *src) {
    int i;
    for(i=0;i<AShadowColors_Colors;i++) {
	if(strcmp(abbrev[i], src)==0) return (AShadowColors)i;
    }
    for(i=0;i<AShadowColors_Colors;i++) {
	if(FOLDEDEQ(AShadowColors_Resources[i]->Name(), src)) return (AShadowColors)i;
    }
    return AShadowColors_Background;
}
    

static color badcolor="Purple";

#define INSENSITIVELABELPCT 0.8
#define INSENSITIVETOPPCT 0.8
#define SELECTEDTOPPCT 0.9

AShadowColorBlock::AShadowColorBlock(AWidgetView *v, AVarSet *d) {
    resources=d;
    target=v;
}


color &AShadowColorBlock::operator[](AShadowColors type) {
    if(type>=AShadowColors_Colors || target==NULL) return badcolor;
    if(resources==NULL) resources=(AVarSet *)target->GetDataObject();
    const char *val=resources->Get(AShadowColors_Resources[type], (char *)0);
    if(val) {
	col[type]=val;
    } else {
        static const char whitename[]="white";
        static const char blackname[]="black";
	const char *sfgc=blackname;
	const char *sbgc=whitename;
        target->GetDrawable()->GetDefaultColors(&sfgc, &sbgc);
        if(sfgc==NULL) sfgc=blackname;
        if(sbgc==NULL) sbgc=whitename;
	unsigned short r, g, b;
	long rr, rg, rb;
	boolean mono=((target)->DisplayClass()&graphic_Monochrome)?TRUE:FALSE;
	if(type != AShadowColors_Background 
			&& type != AShadowColors_Foreground) 
		(*this)[AShadowColors_Background].HardwareRGB(		      *target->CurrentColormap(), r, g, b);
	switch(type) {
	    case AShadowColors_Background:
		col[AShadowColors_Background]=sbgc;
		break;
	    case AShadowColors_Foreground:
		col[AShadowColors_Foreground]=sfgc;
		break;
	    case AShadowColors_UpperShadow:
		if(mono && r==g && g==b &&(g==65535 || g==0)) {
		    col[type]=color(32768, 32768, 32768);
		} else {
		    graphic::ComputeShadow(r, g, b, &rr, &rg, &rb, shadows_TOPSHADOW);
		    col[type]=color((unsigned short)rr, (unsigned short)rg, (unsigned short)rb);
		}
		break;
	    case AShadowColors_LowerShadow:
		if(mono && r==g && g==b && (g==65535 || g==0)) {
		   col[type]=color(65535-r, 65535-g, 65535-b);
		} else {
		    graphic::ComputeShadow(r, g, b, &rr, &rg, &rb, shadows_BOTTOMSHADOW);
		    col[type]=color((unsigned short)rr, (unsigned short)rg, (unsigned short)rb);
		    break;
		}
	    case AShadowColors_DefaultButtonLS:
		if(mono && r==g && g==b &&(g==65535 || g==0)) {
		    col[type]=color(32768, 32768, 32768);
		} else {
		    graphic::ComputeShadow(r, g, b, &rr, &rg, &rb, shadows_TOPSHADOW);
		    col[type]=color((unsigned short)rr, (unsigned short)rg, (unsigned short)rb);
		}
		break;
	    case AShadowColors_DefaultButtonUS:
		if(mono && r==g && g==b && (g==65535 || g==0)) {
		   col[type]=color(65535-r, 65535-g, 65535-b);
		} else {
		    graphic::ComputeShadow(r, g, b, &rr, &rg, &rb, shadows_BOTTOMSHADOW);
		    col[type]=color((unsigned short)rr, (unsigned short)rg, (unsigned short)rb);
		    break;
		}
	    case AShadowColors_LabelFG:
		col[type]=(*this)[AShadowColors_Foreground];
		break;
	    case AShadowColors_LabelBG:
		col[type]=(*this)[AShadowColors_Background];
		break;
	    case AShadowColors_InsensitiveLabelFG:
		{
		color &src=(*this)[AShadowColors_LabelFG];
		unsigned short sr, sg, sb;
		src.HardwareRGB(*target->CurrentColormap(), sr, sg, sb);
		col[type]=color(
				(unsigned short)(sr *INSENSITIVELABELPCT),
				(unsigned short)(sg *INSENSITIVELABELPCT),
				(unsigned short)(sb *INSENSITIVELABELPCT));
		}
		break;
	    case AShadowColors_InsensitiveLabelBG:
		col[type]=(*this)[AShadowColors_Background];
		break;
	    case AShadowColors_InsensitiveBackground:
		{
		color &src=(*this)[AShadowColors_Background];
		unsigned short sr, sg, sb;
		src.HardwareRGB(*target->CurrentColormap(), sr, sg, sb);
		col[type]=color(
				(unsigned short)(sr *INSENSITIVETOPPCT),
				(unsigned short)(sg *INSENSITIVETOPPCT),
				(unsigned short)(sb *INSENSITIVETOPPCT));
		}
		break;
	    case AShadowColors_InsensitiveForeground:
		{
		color &src=(*this)[AShadowColors_Foreground];
		unsigned short sr, sg, sb;
		src.HardwareRGB(*target->CurrentColormap(), sr, sg, sb);
		col[type]=color(
				(unsigned short)(sr *INSENSITIVETOPPCT),
				(unsigned short)(sg *INSENSITIVETOPPCT),
				(unsigned short)(sb *INSENSITIVETOPPCT));
		}
		break;
	    case AShadowColors_SelectedBackground:
		{
		/*
		color &src=(*this)[AShadowColors_Background];
		unsigned short sr, sg, sb;
		src.HardwareRGB(*target->CurrentColormap(), sr, sg, sb); */
		col[type]=color(
				(unsigned short)(r *SELECTEDTOPPCT),
				(unsigned short)(g *SELECTEDTOPPCT),
				(unsigned short)(b *SELECTEDTOPPCT));
		}
		break;
	    case AShadowColors_SelectedForeground:
		{
		color &src=(*this)[AShadowColors_Foreground];
		unsigned short sr, sg, sb;
		src.HardwareRGB(*target->CurrentColormap(), sr, sg, sb);
		col[type]=color(
				(unsigned short)(sr *SELECTEDTOPPCT),
				(unsigned short)(sg *SELECTEDTOPPCT),
				(unsigned short)(sb *SELECTEDTOPPCT));
		}
		break;
	    case AShadowColors_Highlight:
		col[type]=(*this)[AShadowColors_Foreground];
		break;
	    case AShadowColors_Colors: /* tjm - unused */
	    case AShadowColors_Custom: /* tjm - unused */
	    case AShadowColors_Transparent: /* tjm - unused */
		break;
		
	}
    }
    return col[type];
}

    
AShadowColor::AShadowColor() {
    resource=NULL;
    shade=AShadowColors_Custom;
}

AShadowColor::AShadowColor(AShadowColors t, ASlot_NameParam name) {
    shade=t;
    resource=name;
}

AShadowColor::AShadowColor(AShadowColors t) {
    shade=t;
    if(t<AShadowColors_Custom) resource=AShadowColors_Resources[t];
    else resource=NULL;
}

AShadowColor::AShadowColor(const AShadowColor &src) {
    shade=src.shade;
    resource=src.resource;
}

   
AShadowColor &AShadowColor::operator=(const AShadowColor &src) {
    shade=src.shade;
    resource=src.resource;
    return *this;
}


	AShadow *
AShadow::Create(AWidgetView *v, AVarSet *d, unsigned char flags) {
	/* if (d && d->Get(slot_borderShape) == 0)
		return NULL;
	else */ {
		AShadow *res = new AShadow(v, d);
		res->UseBorder(flags);
		return res;
	}
}

AShadowColors AShadow::ShadeForState(AShadowColors s) {
	if(s>=AShadowColors_Colors || s==AShadowColors_Custom) 
		return s;
	if(s==AShadowColors_Transparent) return AShadowColors_Background;
	if(!highlighted && s==AShadowColors_Highlight) 
		return AShadowColors_Background;
	if(!Default() && (s==AShadowColors_DefaultButtonUS 
				|| s==AShadowColors_DefaultButtonLS)) 
		return AShadowColors_Background;
	if(!Sensitive()) {
	    switch(s) {
		case AShadowColors_Foreground:
		    s=AShadowColors_InsensitiveForeground;
		    break;
		case AShadowColors_Background:
		    s=AShadowColors_InsensitiveBackground;
		    break;
		case AShadowColors_LabelFG:
		    s=AShadowColors_InsensitiveLabelFG;
		    break;
		case AShadowColors_LabelBG:
		    s=AShadowColors_InsensitiveLabelBG;
		    break;
		default:
		        break;
	    }
	}
	if(indicator && (Selected()^(armed && FillOnArm()))) {
	    switch(s) {
		case AShadowColors_Foreground: 
			s=AShadowColors_SelectedForeground;
			break;
		case AShadowColors_Background: 
			s=AShadowColors_SelectedBackground;
			break;
		default:
		        break;
	    }
	}
	if(indicator &&(Selected()^armed)) {
	    switch(s) {
		case AShadowColors_UpperShadow:
		    s=AShadowColors_LowerShadow;
		    break;
		case AShadowColors_LowerShadow:
		    s=AShadowColors_UpperShadow;
		    break;
		default:
		    break;
	    }
	}
	return s;
}

void AShadow::AllocateShadowColor(AShadowColor &dest) {
    AShadowColors s=dest.Shade();
    if(s!=AShadowColors_Custom) {
	s=ShadeForState(s);
	dest=scb[s];
    } else if(dest.ResourceName()) {
	const char *c=resources->Get(dest.ResourceName(), (char *)0);
	if(c) dest=c;
    }
    /* else ;  // xxx error no color allocated */
}

void AShadow::SetArmed(boolean a) {
    if((!a) != (!armed)) {
	dirty = TRUE;
	armed=a;
    }
}

void AShadow::SetIndicator(boolean a) {
    if((!a) != (!indicator)) {
	dirty = TRUE;
	indicator=a;
    }
}
	
void AShadow::SetFillOnArm(boolean a) {
    if((!a) != (!FillOnArm())) {
	dirty = TRUE;
	fillOnArm=a;
	if(resources && standard) {
	    ASlot *s=resources->Get(slot_fillOnArm);
	    if(s && ((boolean)*s)!=a) *s=a;
	}
    }
}

boolean AShadow::FillOnArm() {
    if(standard) {
	ASlot *s=resources->Get(slot_fillOnArm);
	if(s) return *s;
    }
    return fillOnArm;
}

void AShadow::SetSelected(boolean a) {
    if((!a) != (!Selected())) {
	dirty = TRUE;
	selected=a;
	if(resources && standard) {
	    ASlot *s=resources->Get(slot_set);
	    if(s && ((boolean)*s)!=a) *s=a;
	}
    }
}
	
boolean AShadow::Selected() {
    if(standard) {
	ASlot *s=resources->Get(slot_set);
	if(s) return *s;
    }
    return selected;
}

void AShadow::SetHighlighted(boolean h) {
    if((!h) != (!highlighted)) {
	dirty = TRUE;
	highlighted=h;
    }
}

void AShadow::SetDefault(boolean a) {
    if((!a) != (!Default())) {
	dirty = TRUE;
	def=a;
	if(resources && standard) {
	    ASlot *s=resources->Get(slot_showAsDefault);
	    if(s && ((boolean)*s)!=a) *s=a;
	} 
    }
}
	
boolean AShadow::Default() {
    if(standard) {
	ASlot *s=resources->Get(slot_showAsDefault);
	if(s) return *s;
    }
    return def;
}

void AShadow::SetSensitive(boolean a) {
    if((!a) != (!Sensitive())) {
	dirty = TRUE;
	sensitive=a;
	if(resources && standard) {
	    ASlot *s=resources->Get(slot_sensitive);
	    if(s && ((boolean)*s)!=a) *s=a;
	} ;
    }
}

boolean AShadow::Sensitive() {
    if(standard) {
	ASlot *s=resources->Get(slot_sensitive);
	if(s) return *s;
    }
    return sensitive;
}

static const ASlot_NameVar borderCombo[] = {
  /* h d p */			// h = hilit,  d = default,  p = plain
  /* n n n */	"noBorder", 		// (not used)
  /* n n y */	"plainBorder", 		
  /* n y n */	"defaultBorder",   	
  /* n y y */	"defaultBorder",     	
  /* y n n */	"highlightBorder",  	
  /* y n y */	"highlightPlainBorder",	
  /* y y n */	"highlightDefaultBorder", 
  /* y y y */	"highlightDefaultBorder", 
};

const long defaultthickness=2;
const long defaulthighlightthickness=2;

const long rdt=192; // sigh... 2 pixels on a 75dpi screen in 1/100pts.
const long rdht=192;

ASlot_NAME(units);
void AShadow::UseBorder(unsigned char flags) {
    boolean highlight = (flags&AShadow_Highlight);
    boolean deflt =(flags&AShadow_Default);
    boolean plain = (flags&AShadow_Plain);
    double units=resources->Get(slot_units,0.0);
    unsigned char hthickness;
    unsigned char thickness;
    if(units==0.0) {
        hthickness=(unsigned char)resources->Get(slot_highlightThickness, defaulthighlightthickness);
        thickness=(unsigned char)resources->Get(slot_shadowThickness, defaultthickness);
    } else {
        hthickness=(unsigned char)resources->Get(slot_highlightThickness, (long)(rdht*units));
        thickness=(unsigned char)resources->Get(slot_shadowThickness, (long)rdt*units);
    }
    unsigned char bst=(unsigned char)resources->Get(slot_defaultShadowThickness, 0L);
    unsigned char sad;
    if(bst==0) sad=(unsigned char)resources->Get(slot_showAsDefault, 0L);
    else sad=bst;
    RemoveBorder();
    if(!highlight && !deflt && !plain) 
	return;
    int comboIndex = (highlight?4:0) + (deflt?2:0) + (plain?1:0);
    const atom *choice = borderCombo[ comboIndex ];
    const char *src=resources->Get(choice, (char *)0);
    if (src) 
	InterpretString(src);
    else  {
	if(highlight) {
	    addu(AShadowColors_Highlight,hthickness).
		      addl(AShadowColors_Highlight,hthickness);
	}
	if(highlight && deflt) {
	    addu(AShadowColors_Transparent, sad+thickness).
		      addl(AShadowColors_Transparent, sad+thickness);
	}
	if(deflt) {
	    addu(AShadowColors_DefaultButtonUS, sad).
		      addu(AShadowColors_Transparent, sad+thickness).
		      addl(AShadowColors_DefaultButtonLS, sad).
		      addl(AShadowColors_Transparent, sad+thickness);
	}
	if(plain) {
	    addu(AShadowColors_UpperShadow, thickness).
		      addl(AShadowColors_LowerShadow, thickness);
	}
    }
}

void AShadow::RemoveBorder() {
    if(upper.GetN()>0) upper.Remove((size_t)0, upper.GetN());
    if(lower.GetN()>0) lower.Remove((size_t)0, lower.GetN());
}
    
#if 0
unsigned int AShadow::TotalUpperBorder() {
    unsigned int r=0;
    for(size_t i=0;i<UpperLayers();i++) {
	r+=UpperThickness(i);
    }
    return r;
}

unsigned int AShadow::TotalLowerBorder() {
    unsigned int r=0;
    for(size_t i=0;i<LowerLayers();i++) {
	r+=LowerThickness(i);
    }
    return r;
}
#endif

#define UX(x) ((long)(((x)/uxscale)+0.5))
#define UY(y) ((long)(((y)/uyscale)+0.5))
void AShadow::TotalUpperBorderWidthHeight(long &w, long &h) {
    w=0;
    h=0;
    for(size_t i=0;i<UpperLayers();i++) {
        unsigned int r=UpperThickness(i);
        w+=UX(r);
        h+=UY(r);
    }
}

void AShadow::TotalLowerBorderWidthHeight(long &w, long &h) {
    w=0;
    h=0;
    for(size_t i=0;i<LowerLayers();i++) {
        unsigned int r=LowerThickness(i);
        w+=UX(r);
        h+=UY(r);
    }
}

void AShadow::DrawBorder(AWidgetView *v, struct rectangle *r, boolean ignoredirty) {
	if ( ! dirty && ! ignoredirty)
		return;
	switch(resources->Get(slot_borderShape, 1)) {
	case 0:		// no border
		break;
	case 1:		// rectangle
		DrawRectBorder(v, r);
		break;
	case 2:		// round rect
		DrawRectBorder(v, r);  // xxx interim
		break;
	case 3:		// diamond
		DrawDiamondBorder(v, r);
		break;
	case 4:		// circle
		DrawDiamondBorder(v, r);  // xxx interim
		break;
	}
	dirty = FALSE;
}
    
void AShadow::DrawRectBorder(AWidgetView *v, struct rectangle *r) {
    struct point pts[6];
    struct rectangle ir=*r;
    size_t i;

    CheckScale();
    for(i=0;i<UpperLayers();i++) {
	unsigned int ud=UpperThickness(i);
        {
	    v->SetForegroundColor(&UpperColor(i));
	    pts[0].x=ir.left;
	    pts[0].y=ir.top;
	    pts[1].x=ir.left+ir.width;
	    pts[1].y=ir.top;
	    pts[2].x=pts[1].x-UX(ud);
	    pts[2].y=pts[1].y+UY(ud);
	    pts[3].x=ir.left+UX(ud);
	    pts[3].y=pts[2].y;
	    pts[4].x=pts[3].x;
	    pts[4].y=ir.top+ir.height-UY(ud);
	    pts[5].x=ir.left;
	    pts[5].y=ir.top+ir.height;
	    v->FillPolygon(pts, 6, NULL);
	}
	ir.left+=UX(ud);
	ir.top+=UY(ud);
	ir.width-=UX(ud*2);
	ir.height-=UY(ud*2);
    }
    ir=*r;
    for(i=0;i<LowerLayers();i++) {
	unsigned int ld=LowerThickness(i);
	{
	    v->SetForegroundColor(&LowerColor(i));
	    pts[5].x=ir.left;
	    pts[5].y=ir.top+ir.height;
	    pts[4].x=ir.left+ir.width;
	    pts[4].y=pts[5].y;
	    pts[3].x=pts[4].x;
	    pts[3].y=ir.top;
	    pts[2].x=ir.left+ir.width-UX(ld);
	    pts[2].y=ir.top+UY(ld);
	    pts[1].x=pts[2].x;
	    pts[1].y=pts[4].y-UY(ld);
	    pts[0].x=ir.left+UX(ld);
	    pts[0].y=pts[1].y;
	    v->FillPolygon(pts, 6, NULL);
	}
	ir.top+=UY(ld);
        ir.left+=UX(ld);
	ir.width-=UX(ld*2);
	ir.height-=UY(ld*2);
    }
}

void AShadow::InteriorRect(struct rectangle *exterior, struct rectangle *interior) {
    long udx, udy;
    TotalUpperBorderWidthHeight(udx, udy);
    long ldx, ldy;
    TotalLowerBorderWidthHeight(ldx, ldy);
    CheckScale();
    interior->left=exterior->left+udx;
    interior->top=exterior->top+udy;
    interior->width=exterior->width-udx-ldx;
    interior->height=exterior->height-udy-ldy;
}

void AShadow::ExteriorRect(struct rectangle *interior, struct rectangle *exterior) {
    long udx, udy;
    TotalUpperBorderWidthHeight(udx, udy);
    long ldx, ldy;
    TotalLowerBorderWidthHeight(ldx, ldy);
    CheckScale();
    exterior->left=interior->left-udx;
    exterior->top=interior->top-udy;
    exterior->width=interior->width+udx+ldx;
    exterior->height=interior->height+udy+ldy;
}

void AShadow::FillRect(AWidgetView *v, struct rectangle *r) {
    struct rectangle interior;
    InteriorRect(r, &interior);
    v->SetForegroundColor(&Background());
    v->FillRect(&interior, NULL);
}

void AShadow::FillDiamond(AWidgetView *v, struct rectangle *r) {
    struct point pts[4];
    long udx=0, udy=0;
    long ldx=0, ldy=0;
    struct rectangle ir=*r;
    size_t i;
    CheckScale();

    for(i=0;i<UpperLayers();i++) {
        unsigned int ud=UpperThickness(i);
        udx+=UX(ud);
        udy+=UY(ud);
    }
    for(i=0;i<LowerLayers();i++) {
	unsigned int ld=LowerThickness(i);
        ldx+=UX(ld);
        ldy+=UY(ld);
    }

    pts[0].x=ir.left+ir.width/2;
    pts[0].y=ir.top+udy;
    pts[1].x=ir.left+udx;
    pts[1].y=ir.top+ir.height/2;
    pts[2].x=pts[0].x;
    pts[2].y=ir.top+ir.height-udy;
    pts[3].x=ir.left+ir.width-udx;
    pts[3].y=pts[1].y;
    v->SetForegroundColor(&Background());
    v->FillPolygon(pts, 4, NULL);
}

void AShadow::DrawDiamondBorder(AWidgetView *v, struct rectangle *r) {
    struct point pts[6];
    struct rectangle ir=*r;
    size_t i;
    
    CheckScale();
    
    for(i=0;i<UpperLayers();i++) {
	unsigned int ud=UpperThickness(i);
	{
	    pts[0].x=ir.left+ir.width/2;
	    pts[0].y=ir.top;
	    pts[1].x=ir.left+ir.width;
	    pts[1].y=ir.top+ir.height/2;
	    pts[2].x=pts[1].x-UX(ud);
	    pts[2].y=pts[1].y;
	    pts[3].x=pts[0].x;
	    pts[3].y=pts[0].y+UY(ud);
	    pts[4].x=ir.left+UX(ud);
	    pts[4].y=pts[1].y;
	    pts[5].x=ir.left;
	    pts[5].y=pts[1].y;
	    v->SetForegroundColor(&UpperColor(i));
	    v->FillPolygon(pts, 6, NULL);
	}
	ir.left+=UX(ud);
	ir.top+=UY(ud);
	ir.width-=UX(ud*2);
	ir.height-=UY(ud*2);
    }
    ir=*r;
    for(i=0;i<LowerLayers();i++) {
	unsigned int ld=LowerThickness(i);
	{
	    pts[0].x=ir.left+ir.width/2;
	    pts[0].y=ir.top+ir.height;
	    pts[1].x=ir.left+ir.width;
	    pts[1].y=ir.top+ir.height/2;
	    pts[2].x=pts[1].x-UX(ld);
	    pts[2].y=pts[1].y;
	    pts[3].x=pts[0].x;
	    pts[3].y=pts[0].y-UY(ld);
	    pts[4].x=ir.left+UX(ld);
	    pts[4].y=pts[1].y;
	    pts[5].x=ir.left;
	    pts[5].y=pts[1].y;
	    v->SetForegroundColor(&LowerColor(i));
	    v->FillPolygon(pts, 6, NULL);
	}
	ir.top+=UY(ld);
	ir.left+=UX(ld);
	ir.width-=UX(ld*2);
	ir.height-=UY(ld*2);
    }
}

#define SKIPPABLE(src) (isspace(*src) || *src=='(' || *src==')')

void AShadow::InterpretString(const char *src) {
    boolean upper=TRUE;
    unsigned char dthickness=(unsigned char)resources->Get(slot_shadowThickness, defaultthickness);
    while(*src) {
	unsigned char thickness=dthickness;
	
	while(SKIPPABLE(src)) src++;
	
	if(*src=='u' || *src=='l') {
	    upper=(*src)=='u';
	    while(isalpha(*src)) src++;
	}
	
	while(SKIPPABLE(src)) src++;
	
	if(isdigit(*src)) {
	    thickness=0;
	    while(isdigit(*src)) {
		thickness*=10;
		thickness+=*src-'0';
		src++;
	    }
	}
	
	while(SKIPPABLE(src)) src++;
	
	char shade[256];
	char *p=shade;
	while(p-shade<(int)sizeof(shade)-1 && isalpha(*src)) {
	    *p++=*src;
	    src++;
	}
	*p='\0';
	if(p-shade==sizeof(shade)-1 && isalpha(*src)) {
	    while(isalpha(*src)) src++;
	}
	AShadowColors t=MapShade(shade);
	if(t==AShadowColors_Custom) {
	    char name[256];
	    char *p=name;

	    while(SKIPPABLE(src)) src++;

	    while(p-name<(int)sizeof(name)-1 && isalpha(*src)) {
		*p++=*src;
		src++;
	    }
	    *p='\0';
	    if(p-name==sizeof(name)-1 && isalpha(*src)) {
		while(isalpha(*src)) src++;
	    }
	    if(upper) addu(t, name, thickness);
	    else addl(t, name, thickness);
	} else if(upper) addu(t, thickness);
	    else addl(t, thickness);    
    }
}
    
AShadowLayer::AShadowLayer() {
    thickness=0;
}

AShadowLayer::AShadowLayer(AShadowColors t, ASlot_NameParam name, unsigned char thickness) {
    color.SetShade(t);
    color.SetResourceName(name);
    SetThickness(thickness);
}

AShadowLayer::AShadowLayer(AShadowColors t, unsigned char thickness) {
    color.SetShade(t);
    SetThickness(thickness);
}

/* $Log: ashadow.C,v $
 * Revision 1.18  1996/11/12  21:51:10  robr
 * Fixed to correctly default the fg/bg colors.
 * BUG
 *
 * Revision 1.17  1996/10/15  00:30:39  robr
 * Even if borderShape is 0 an ashadow can be useful.
 * BUG
 *
 * Revision 1.16  1996/10/01  20:23:43  robr
 * Replaced Total{Upper,Lower}Border with versions which report the width and
 * height of the corresponding border in pixels.
 * (This accounts for scaling and the use of units other than pixels.)
 * BUG
 *
 * Revision 1.15  1996/09/25  18:21:33  robr
 * Fixed scaling to use macros UX, UY.
 * BUG
 *
 * Revision 1.14  1996/09/24  18:51:12  robr
 * Added support for scaling.
 * FEATURE
 *
 * Revision 1.13  1996/09/09  14:24:50  wjh
 * remove obsolete declarations of `s' in DrawXxxxBorder
 *
 * Revision 1.12  1996/06/27  14:09:43  robr
 * Fixed to use GetDefaultColors (the graphic method) instead of the
 * currently set foreground/background of the view.
 * The ashadow and AShadowColorBlock Init methods have been removed as unnecessary
 * now.
 * BUG
 *
 * Revision 1.11  1996/06/11  23:12:54  robr
 * Disabled caching for compatibility with formulas.
 * Sigh.
 * BUG
 *
 * Revision 1.10  1996/06/07  20:51:18  robr
 * Fixed to allow an ashadow to use several parameters directly
 * from a set of resources.
 * BUG
 *
 * Revision 1.9  1996/05/20  15:25:07  robr
 * /tmp/msg
 *
 * Revision 1.8  1996/05/13  17:14:05  wjh
 *  remove a 'static'
 *
 * Revision 1.7  1996/05/07  20:36:07  wjh
 * reformatted; \
 * add std hdr elements; \
 * new scheme for selecting which border combo to use; \
 * added dirty flag describing whether redraw is needed; \
 * define AShadow::Create and DrawBorder; \
 * use borderShape attribute; \
 * added $Log at end
 *
 */
