/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include <graphic.H>
#include <fontdesc.H>
#include <view.H>
#include <dataobject.H>
#include <rect.h>
#include <table.H>
#include <environ.H>
#include <print.H>
#define AUXMODULE
#include <spread.H>

#include <shared.h>

struct printlump {
    class spread *self;
    class table *tab;
    int bodyfont;
    int fontsize;
    struct font_afm *afm;
    short *encoding;

    boolean setfontyet;
};

static struct font_afm *symbolfont = NULL;

extern void spread_printVal(class table  * T, char *buf, extended_double  *value, char format, int prec);

static void EnsureBodyFont(struct printlump *lump)
{
    char namebuf[256];
    short *encoding;

    print::LookUpPSFont(namebuf, &encoding, lump->self->writingFont, NULL, 0, 0);

    lump->fontsize = lump->self->writingFont->GetFontSize();
    lump->encoding = encoding;
    lump->afm = print::GetPSFontAFM(namebuf);
    lump->bodyfont = print::PSRegisterFont(namebuf);
}

/* chi is an AGP-encoded value; result is in PS units */
static double ComputeCharWidth(int  chi, struct font_afm  *afm, int  size)
{
    double res;
    int chs, nump;

    if (chi < 0)
	return 0.0;

    nump = afm->ch[chi].numparts;
    if (nump==print_SymbolChar) {
	res = 0;
	/* pull width from symbol AFM */
	if (!symbolfont) {
	    symbolfont = print::GetPSFontAFM("Symbol");
	}
	chs = afm->ch[chi].u.symbolchar;
	res = symbolfont->ch[chs].x;
    }
    else {
	/* for normal or composite chars, the size is available here. */
	res = afm->ch[chi].x;
    }

    return (res * (double)(size) / 1000.0);
}


/* write contents of the given cell into the given rectangle (again, note that ypos is the top edge, and hpos is positive, so the bottom is ypos-hpos) */
static void WriteCell(struct printlump *lump, FILE *file, int cx, int cy, long xpos, long ypos, long wpos, long hpos)
{
    struct cell *c;
    struct viewlist *vl;
    const char *str;
    extended_double *celval;
    char buf[1030];

    c = lump->tab->GetCell(cy, cx);
    str = NULL;

    switch (c->celltype) {
	case table_ImbeddedObject:
	    for (vl = c->interior.ImbeddedObject.views; vl; vl=vl->next)
		if (vl->parent == lump->self)
		    break;
	    if (vl && vl->child && vl->child->GetPSPrintInterface("generic")) {
		struct rectangle visrect;

		fprintf(file, "gsave\n");
		fprintf(file, "%g %g translate\n", (double)xpos+1.5, (double)ypos-hpos+1.5);
		visrect.left = 0;
		visrect.top = hpos-spread_SPACING;
		visrect.width = wpos-spread_SPACING;
		visrect.height = hpos-spread_SPACING;
		fprintf(file, "0 0 moveto  %ld 0 rlineto  0 %ld rlineto  %ld 0 rlineto closepath clip newpath\n", visrect.width, visrect.height, -visrect.width);
		vl->child->PrintPSRect(file, visrect.width, visrect.height, &visrect);
		fprintf(file, "grestore\n");
	    }
	    break;
	case table_ValCell:
	    lump->tab->ReEval(cy, cx);
	    celval = (&(c->interior.ValCell.value));
	    if (IsStandard(celval)) {
		if (c->format == HORIZONTALBARFORMAT) {
		    double bleft, btop, bright, bbot;
		    btop = (double)ypos - 1.5;
		    bbot = (double)(ypos-hpos) + 1.5;
		    bleft = (double)xpos + 1.5;
		    bright = (double)(xpos+wpos) - 1.5;
		    if (StandardValue(celval) == 0.0) {
			break;
		    }
		    if (StandardValue(celval) >= 0.0
			&& StandardValue(celval) < 1.0) {
			bright = bleft + StandardValue(celval) * (double)(wpos-spread_SPACING);
		    }
		    else if (StandardValue(celval) < 0.0
			     && StandardValue(celval) > -1.0) {
			bleft = bright + StandardValue(celval) * (double)(wpos-spread_SPACING);
		    }
		    fprintf(file, "%g %g moveto  %g %g lineto  %g %g lineto  %g %g lineto closepath fill\n", bleft, btop,  bright, btop,  bright, bbot,  bleft, bbot);
		    break; /* no fall-through */
		}
		else if (c->format == VERTICALBARFORMAT) {
		    double bleft, btop, bright, bbot;
		    btop = (double)ypos - 1.5;
		    bbot = (double)(ypos-hpos) + 1.5;
		    bleft = (double)xpos + 1.5;
		    bright = (double)(xpos+wpos) - 1.5;
		    if (StandardValue(celval) == 0.0) {
			break;
		    }
		    if (StandardValue(celval) >= 0.0
			&& StandardValue(celval) < 1.0) {
			btop = bbot + StandardValue(celval) * (double)(hpos-spread_SPACING);
		    }
		    else if (StandardValue(celval) < 0.0
			     && StandardValue(celval) > -1.0) {
			bbot = btop + StandardValue(celval) * (double)(hpos-spread_SPACING);
		    }
		    fprintf(file, "%g %g moveto  %g %g lineto  %g %g lineto  %g %g lineto closepath fill\n", bleft, btop,  bright, btop,  bright, bbot,  bleft, bbot);
		    break; /* no fall-through */
		}
		else {
		    str = buf;
		    buf[0] = '"';
		    spread_printVal(lump->tab, buf+1, celval, c->format, c->precision);
		    /* fall though */
		}
	    }
	    else {
		str = ExtractBogus(celval);
		if (!str) {
		    break;
		}
		else {
		    /* fall though */
		}
	    }
	    /* no break */
	case table_TextCell:
	    if (!str)
		str = c->interior.TextCell.textstring;
	    if (!str || !str[0])
		break;
	    {
		int thech, just;
		double sofar, wix;
		const char *tmpstr;
		boolean clip = FALSE;

		just = str[0];
		if (just=='\'' || just=='"' || just=='^')
		    str++;
		else
		    just = '\'';

		EnsureBodyFont(lump);
		if (!lump->setfontyet) {
		    lump->setfontyet = TRUE;
		    fprintf(file, "%s%d %d scalefont setfont\n", print_PSFontNameID, lump->bodyfont, lump->fontsize);
		}
		sofar = 0.0;
		/* Grossosity. We assume that the string is AGP-encoded, and therefore skip the lump->encoding step, even though it's certainly 8859-encoded. This is so that we can put off writing print::DumpEncodedString and print::MeasureEncodedString procedures (right now, textview handles that magic itself, which is wrong.) */
		for (tmpstr = str; *tmpstr; tmpstr++) {
		    thech = (*tmpstr);
		    wix = ComputeCharWidth(thech, lump->afm, lump->fontsize);
		    sofar += wix;
		}

		if ((long)sofar > wpos-spread_SPACING
		    || lump->fontsize+2 > hpos-spread_SPACING) {
		    /* clip */
		    clip = TRUE;
		}

		if (clip) {
		    fprintf(file, "gsave\n%ld %ld moveto  %ld 0 rlineto  0 %ld rlineto  %ld 0 rlineto closepath clip newpath\n", xpos, ypos, wpos, -hpos, -wpos);
		}
		switch (just) {
		    case '\'':
		    default:
			wix = (double)xpos + 1.5;
			break;
		    case '"':
			wix = (double)(xpos+wpos) - sofar - 1.5;
			break;
		    case '^':
			wix = (double)xpos + ((double)wpos - sofar) / 2.0;
			break;
		}
		fprintf(file, "%g %ld moveto\n(", wix, ypos-lump->fontsize);
		print::OutputPSString(file, str, -1);
		fprintf(file, ") show\n");
		if (clip)
		    fprintf(file, "grestore\n");
	    }
	    break;
	case table_EmptyCell:
	    break;
    }
}

/* write PS for the rectangle of the table given by (xstart, ystart, wid, hgt). Borders of the rectangle, and cells that extend beyond the rectangle, should be included. To enjoin sanity, the table origin should go at the PS origin, so the table extends into the *negative* y-coordinates. */
static void WriteRect(FILE *file, class spread *self, int xstart, int ystart, int wid, int hgt)
{
    class table *tab = (class table *)self->GetDataObject();
    int cx, cy; /* the cell coordinates under consideration */
    long xpos, ypos, xposstart, yposstart;
    long rowh, colw;
    boolean joinleft, jointop;
    struct printlump lump;
    int tabsizew;
    int cx1, cy1;
    long cx1pos, cy1pos;

    xposstart = spread_Width(self, 0, xstart);
    yposstart = -spread_Height(self, 0, ystart);

    print::PSRegisterDef("tabVL", "{moveto 0 exch rlineto stroke} bind"); /* height startx starty tabVL */
    print::PSRegisterDef("tabHL", "{moveto 0 rlineto stroke} bind"); /* length startx starty tabHL */

    lump.self = self;
    lump.tab = tab;
    lump.bodyfont = (-1); /* so we don't register it unless necessary. */
    lump.afm = NULL;
    lump.encoding = NULL;
    lump.setfontyet = FALSE;

    tabsizew = tab->NumberOfColumns();

    cy = ystart;
    ypos = yposstart;
    while (cy<ystart+hgt) {
	rowh = spread_Height(self, cy, cy+1);

	cx = xstart;
	xpos = xposstart;
	while (cx<xstart+wid) {
	    colw = spread_Width(self, cx, cx+1);

	    /* strategy:
	     If the cell is joined left (but not on the left border) or joined top (but not on the top border), don't draw its contents. Otherwise, do.
	     If the cell is joined left, don't draw left edge. Ditto for top edge.
	     Note that if we're drawing the contents of a joined cell, we really have to chew up/left to the canonical cell, and then get the width/height right...
	     */
	    joinleft = tab->IsJoinedToLeft(cy, cx);
	    jointop = tab->IsJoinedAbove(cy, cx);

	    cx1 = (-1);
	    if (joinleft) {
		if (!jointop && cx==xstart) {
		    cy1 = cy;
		    cy1pos = ypos;

		    for (cx1 = cx-1;
			 cx1 > 0 && tab->IsJoinedToLeft(cy, cx1);
			 cx1--);
		    cx1pos = xpos - spread_Width(self, cx1, cx);
		}
	    }
	    else if (jointop) {
		if (!joinleft && cy==ystart) {
		    cx1 = cx;
		    cx1pos = xpos;

		    for (cy1 = cy-1;
			 cy1 > 0 && tab->IsJoinedAbove(cy1, cx);
			 cy1--);
		    cy1pos = ypos + spread_Height(self, cy1, cy);
		}
	    }
	    else {
		cx1 = cx;
		cy1 = cy;
		cx1pos = xpos;
		cy1pos = ypos;
	    }

	    if (cx1 >= 0) {
		int cx2, cy2;
		long cellw, cellh;
		for (cx2 = cx1+1;
		     cx2 <= tabsizew && tab->IsJoinedToLeft(cy1, cx2);
		     cx2++);
		for (cy2 = cy1+1;
		     cy2 <= tabsizew && tab->IsJoinedAbove(cy2, cx1);
		     cy2++);
		cellw = spread_Width(self, cx1, cx2);
		cellh = spread_Height(self, cy1, cy2);
		WriteCell(&lump, file, cx1, cy1, cx1pos, cy1pos, cellw, cellh);
	    }

	    if (!joinleft) {
		/* draw left edge */
		if (tab->ColorToLeft(cy, cx) == BLACK)
		    fprintf(file, "%ld %ld %ld tabVL\n", -rowh, xpos, ypos);
	    }
	    if (!jointop) {
		/* draw top edge */
		if (tab->ColorAbove(cy, cx) == BLACK)
		    fprintf(file, "%ld %ld %ld tabHL\n", colw, xpos, ypos);
	    }

	    /* end of this cell */
	    xpos += colw;
	    cx++;
	}
	/* check right edge of row */
	if (tab->ColorToLeft(cy, cx) == BLACK && !tab->IsJoinedToLeft(cy, cx))
	    fprintf(file, "%ld %ld %ld tabVL\n", -rowh, xpos, ypos);

	/* end of this row */
	ypos -= rowh;
	cy++;
    }

    /* check bottom edges of all cols */
    cx = xstart;
    xpos = xposstart;
    while (cx<xstart+wid) {
	colw = spread_Width(self, cx, cx+1);

	if (tab->ColorAbove(cy, cx) == BLACK && !tab->IsJoinedAbove(cy, cx))
	    fprintf(file, "%ld %ld %ld tabHL\n", colw, xpos, ypos);

	/* end of this col */
	xpos += colw;
	cx++;
    }
}

#define MARGIN (36)  /* leave a half-inch margin, ideally */

void spread::PrintPSDoc(FILE *file, long pagew, long pageh)
{
    class table *tab = (class table *)this->GetDataObject();
    long xpos, ypos;
    long areah, areaw;
    long xseg, yseg;
    long val;
    int cx, cy, cxend, cyend;
    int tabsizew, tabsizeh;

    if (!tab)
	return;

    if (this->grayPix == NULL)
	InitializeGraphic(this);
    ComputeRowSizes(this);

    tabsizew = tab->NumberOfColumns();
    tabsizeh = tab->NumberOfRows();

    areaw = pagew - 2*MARGIN;
    areah = pageh - 2*MARGIN;

    fprintf(file, "%% This document was generated by an AUIS table object.\n");

    cy = 0;
    ypos = 0;
    while (cy < tabsizeh) {
	yseg = 0;
	cyend = cy;
	while (cyend < tabsizeh &&
	       (yseg+(val=spread_Height(this, cyend, cyend+1)) <= areah
		|| cyend == cy)) {
	    cyend++;
	    yseg += val;
	}

	cx = 0;
	xpos = 0;

	while (cx < tabsizew) {
	    xseg = 0;
	    cxend = cx;
	    while (cxend < tabsizew &&
		   (xseg+(val=spread_Width(this, cxend, cxend+1)) <= areaw
		    || cxend == cx)) {
		cxend++;
		xseg += val;
	    }
	    
	    /* now xpos, ypos are the (positive) coords of cell (cx, cy) */
	    print::PSNewPage(print_UsualPageNumbering);
	    fprintf(file, "%d %d moveto  %ld 0 rlineto 0 %ld rlineto %ld 0 rlineto closepath clip newpath\n", MARGIN, MARGIN, areaw, areah, -areaw);
	    fprintf(file, "%ld %ld translate\n", MARGIN-xpos, MARGIN+areah+ypos);
	    fprintf(file, "%ld %ld moveto  %ld 0 rlineto 0 %ld rlineto %ld 0 rlineto closepath clip newpath\n", xpos, -ypos, xseg, -yseg, -xseg);
	    WriteRect(file, this, cx, cy, cxend-cx, cyend-cy);

	    cx = cxend;
	    xpos += xseg;
	}

	cy = cyend;
	ypos += yseg;
    }
}

void *spread::GetPSPrintInterface(const char *printtype)
{
    if (!strcmp(printtype, "generic"))
	return (void *)this;

    return NULL;
}

void spread::PrintPSRect(FILE *file, long logwidth, long logheight, struct rectangle *visrect)
{
    /* print as much as will fit */
    class table *tab = (class table *)this->GetDataObject();
    int cx, cy, cxend, cyend;
    int tabsizew, tabsizeh;
    long val, sofar;

    if (!tab)
	return;

    if (this->grayPix == NULL)
	InitializeGraphic(this);
    ComputeRowSizes(this);

    tabsizew = tab->NumberOfColumns();
    tabsizeh = tab->NumberOfRows();

    fprintf(file, "%% AUIS table inset.\n");
    fprintf(file, "%d %ld translate\n", 0, logheight);

    for (cx=0, sofar=0;
	 cx<tabsizew && sofar+(val=spread_Width(this, cx, cx+1)) <= visrect->left;
	 cx++, sofar += val);
    for (cxend=cx;
	 cxend<tabsizew && sofar < visrect->left+visrect->width;
	 cxend++, sofar += spread_Width(this, cxend, cxend+1));

    for (cy=0, sofar=0;
	 cy<tabsizeh && sofar+(val=spread_Height(this, cy, cy+1)) <= logheight-visrect->top;
	 cy++, sofar += val);
    for (cyend=cy;
	 cyend<tabsizeh && sofar < logheight-(visrect->top-visrect->height);
	 cyend++, sofar += spread_Height(this, cyend, cyend+1));

    if (cxend > cx && cyend > cy)
	WriteRect(file, this, cx, cy, cxend-cx, cyend-cy);
}
