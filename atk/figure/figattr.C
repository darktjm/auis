/* figattr.c - attributes for fig objects */
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
char *figattr_c_rcsid = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/figure/RCS/figattr.C,v 3.8 1996/01/21 03:34:54 wjh Stab74 $";
#endif 

#include <andrewos.h>
ATK_IMPL("figattr.H")
#include <math.h>
#include <figattr.H>
#include <figview.H>



static char attribute_names[figattr_NumAttributes][20] = {
    "shade",
    "color",
    "linewidth",
    "rrectcorner",
    "fontsize",
    "fontstyle",
    "fontfamily",
    "textpos",
    "arrowsize",
    "arrowpos",
    "arrow",
    "linestyle"
};




ATKdefineRegistry(figattr, dataobject, NULL);

static char *CopyString(const char  *str);


figattr::figattr()
{
    this->active = 0;

    this->shade = 0;
    this->linewidth = 0;
    this->rrectcorner = 10;
    this->color = CopyString("black");
    this->fontfamily = CopyString("andy");
    this->fontsize = 12;
    this->fontstyle = 0; /* fontdesc_Plain */
    this->textpos = figattr_PosCenter;
    this->arrowsize = 8;
    this->arrowpos = figattr_ArrowNone;
    this->arrow = figattr_IsoTriangle;
    this->linestyle = figattr_LineSolid;
    THROWONFAILURE( TRUE);
}

figattr::~figattr()
{
    if (this->color)
	free(this->color);
    if (this->fontfamily)
	free(this->fontfamily);
}

class figattr *figattr::CopySelf()
{
    class figattr *res = new figattr;

    if (!res) return NULL;
    res->active = this->active;

    /* ### should only copy active attributes, at least for high-cost ones like allocated string */
    res->shade = this->shade;
    res->linewidth = this->linewidth;
    res->rrectcorner = this->rrectcorner;
    res->color = CopyString(this->color);
    res->fontsize = this->fontsize;
    res->fontstyle = this->fontstyle;
    res->fontfamily = CopyString(this->fontfamily);
    res->textpos = this->textpos;
    res->arrowsize = this->arrowsize;
    res->arrowpos = this->arrowpos;
    res->arrow = this->arrow;
    res->linestyle = this->linestyle;

    return res;
}

void figattr::CopyData(class figattr  *src, unsigned long  mask)
{
    if (mask & (1<<figattr_Shade)) {
	if (!(src)->IsActive( figattr_Shade))
	    (this)->SetActive( figattr_Shade, FALSE);
	else 
	    (this)->SetShade( (src)->GetShadeVal());
    }

    if (mask & (1<<figattr_LineWidth)) {
	if (!(src)->IsActive( figattr_LineWidth))
	    (this)->SetActive( figattr_LineWidth, FALSE);
	else 
	    (this)->SetLineWidth( (src)->GetLineWidthVal());
    }

    if (mask & (1<<figattr_RRectCorner)) {
	if (!(src)->IsActive( figattr_RRectCorner))
	    (this)->SetActive( figattr_RRectCorner, FALSE);
	else 
	    (this)->SetRRectCorner( (src)->GetRRectCornerVal());
    }

    if (mask & (1<<figattr_Color)) {
	if (!(src)->IsActive( figattr_Color))
	    (this)->SetActive( figattr_Color, FALSE);
	else 
	    (this)->SetColor( (src)->GetColorVal());
    }

    if (mask & (1<<figattr_FontSize)) {
	if (!(src)->IsActive( figattr_FontSize))
	    (this)->SetActive( figattr_FontSize, FALSE);
	else 
	    (this)->SetFontSize( (src)->GetFontSizeVal());
    }

    if (mask & (1<<figattr_FontStyle)) {
	if (!(src)->IsActive( figattr_FontStyle))
	    (this)->SetActive( figattr_FontStyle, FALSE);
	else 
	    (this)->SetFontStyle( (src)->GetFontStyleVal());
    }

    if (mask & (1<<figattr_FontFamily)) {
	if (!(src)->IsActive( figattr_FontFamily))
	    (this)->SetActive( figattr_FontFamily, FALSE);
	else 
	    (this)->SetFontFamily( (src)->GetFontFamilyVal());
    }

    if (mask & (1<<figattr_TextPos)) {
	if (!(src)->IsActive( figattr_TextPos))
	    (this)->SetActive( figattr_TextPos, FALSE);
	else 
	    (this)->SetTextPos( (src)->GetTextPosVal());
    }

    if (mask & (1<<figattr_ArrowSize)) {
	if (!(src)->IsActive( figattr_ArrowSize))
	    (this)->SetActive( figattr_ArrowSize, FALSE);
	else 
	    (this)->SetArrowSize( (src)->GetArrowSizeVal());
    }

    if (mask & (1<<figattr_ArrowPos)) {
	if (!(src)->IsActive( figattr_ArrowPos))
	    (this)->SetActive( figattr_ArrowPos, FALSE);
	else 
	    (this)->SetArrowPos( (src)->GetArrowPosVal());
    }

    if (mask & (1<<figattr_Arrow)) {
	if (!(src)->IsActive( figattr_Arrow))
	    (this)->SetActive( figattr_Arrow, FALSE);
	else 
	    (this)->SetArrow( (src)->GetArrowVal());
    }

    if (mask & (1<<figattr_LineStyle)) {
	if (!(src)->IsActive( figattr_LineStyle))
	    (this)->SetActive( figattr_LineStyle, FALSE);
	else 
	    (this)->SetLineStyle( (src)->GetLineStyleVal());
    }
    /* ##new */
}

static char *CopyString(const char  *str)
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

/* does not use /begindata /enddata convention */
long figattr::Write(FILE  *fp, long  writeid, int  level)
{
    int ix;

    if ((this->active & figattr_MaskAll) == figattr_MaskAll) {
	/* write out brief attr form */
	
	fprintf(fp, "attrs:");
		fprintf(fp, " %d", this->shade);
		fprintf(fp, " %d", this->linewidth);
		fprintf(fp, " %d", this->rrectcorner);
		fprintf(fp, " %s", this->color);
		fprintf(fp, " %d", this->fontsize);
		fprintf(fp, " %d", this->fontstyle);
		fprintf(fp, " %s", this->fontfamily);
		fprintf(fp, " %d", this->textpos);
		fprintf(fp, " %d", this->arrowsize);
		fprintf(fp, " %d", this->arrowpos);
		fprintf(fp, " %d", this->arrow);
		fprintf(fp, " %d", this->linestyle);
	fprintf(fp, "\n");
	return 0;
    }

    fprintf(fp, "\\figattr{\n");

    for (ix=0; ix<figattr_NumAttributes; ix++) {
	if ((this)->IsActive( ix)) {
	    fprintf(fp, "%s:", attribute_names[ix]);
	    switch (ix) {
		case figattr_Shade:
		    fprintf(fp, "%d", this->shade);
		    break;
		case figattr_LineWidth:
		    fprintf(fp, "%d", this->linewidth);
		    break;
		case figattr_RRectCorner:
		    fprintf(fp, "%d", this->rrectcorner);
		    break;
		case figattr_Color:
		    fprintf(fp, "%s", this->color);
		    break;
		case figattr_FontSize:
		    fprintf(fp, "%d", this->fontsize);
		    break;
		case figattr_FontStyle:
		    fprintf(fp, "%d", this->fontstyle);
		    break;
		case figattr_FontFamily:
		    fprintf(fp, "%s", this->fontfamily);
		    break;
		case figattr_TextPos:
		    fprintf(fp, "%d", this->textpos);
		    break;
		case figattr_ArrowSize:
		    fprintf(fp, "%d", this->arrowsize);
		    break;
		case figattr_ArrowPos:
		    fprintf(fp, "%d", this->arrowpos);
		    break;
		case figattr_Arrow:
		    fprintf(fp, "%d", this->arrow);
		    break;
		case figattr_LineStyle:
		    fprintf(fp, "%d", this->linestyle);
		    break;
		    /* ##new */
		default:
		    break;
	    }
	    fprintf(fp, "\n");
	}
    }

    fprintf(fp, "}\n");
    return 0;
}

/* does not use /begindata /enddata convention */
long figattr::Read(FILE  *fp, long  id)
{
#define LINELENGTH (250)
    static char buf[LINELENGTH+1];
    char *pt, *tmp;
    int ix;
    long ival, jval;

    if (fgets(buf, LINELENGTH, fp) == NULL)
	return dataobject_PREMATUREEOF;

    if (*buf == 'a') {
	/* read brief attrs form */
	int n; 		
	long tshade, tlinewidth, trrectcorner, tfontsize, 
		tfontstyle, ttextpos, tarrowsize, tarrowpos, 
		tarrow, tlinestyle;
	char tcolor[256], tfontfamily[256];

	n = sscanf(buf, "attrs: %d %d %d %s %d %d %s %d %d %d %d %d",
		&tshade, &tlinewidth, &trrectcorner, &tcolor, &tfontsize, 
		&tfontstyle, &tfontfamily, &ttextpos, &tarrowsize, &tarrowpos, 
		&tarrow, &tlinestyle);
	if (n != figattr_NumAttributes) 
		return dataobject_PREMATUREEOF;
	SetShade(tshade);
	SetLineWidth(tlinewidth);
	SetRRectCorner(trrectcorner);
	SetColor(tcolor);
	SetFontSize(tfontsize);
	SetFontStyle(tfontstyle);
	SetFontFamily(tfontfamily);
	SetTextPos(ttextpos);
	SetArrowSize(tarrowsize);
	SetArrowPos(tarrowpos);
	SetArrow(tarrow);
	SetLineStyle(tlinestyle);
	return dataobject_NOREADERROR;
    }
   
    if (strncmp(buf, "\\figattr{", 9)) 
	return dataobject_BADFORMAT;
    if (fgets(buf, LINELENGTH, fp) == NULL)
	return dataobject_PREMATUREEOF;

    while (1) {
	
	if (!strncmp(buf, "}", 1)) return dataobject_NOREADERROR;
	pt = strchr(buf, ':');
	if (!pt) return dataobject_BADFORMAT;
	*pt = '\0';
	pt++;
	tmp = strchr(pt, '\n');
	if (tmp) 
	    *tmp = '\0';

	for (ix=0; ix<figattr_NumAttributes; ix++)
	    if (!strcmp(buf, attribute_names[ix])) break;

	switch (ix) {
	    case figattr_Shade:
		ival = atoi(pt);
		(this)->SetShade( ival);
		break;
	    case figattr_LineWidth:
		ival = atoi(pt);
		(this)->SetLineWidth( ival);
		break;
	    case figattr_RRectCorner:
		ival = atoi(pt);
		(this)->SetRRectCorner( ival);
		break;
	    case figattr_Color:
		(this)->SetColor( pt);
		break;
	    case figattr_FontFamily:
		(this)->SetFontFamily( pt);
		break;
	    case figattr_FontSize:
		ival = atoi(pt);
		(this)->SetFontSize( ival);
		break;
	    case figattr_FontStyle:
		ival = atoi(pt);
		(this)->SetFontStyle( ival);
		break;
	    case figattr_TextPos:
		ival = atoi(pt);
		(this)->SetTextPos( ival);
		break;
	    case figattr_ArrowSize:
		ival = atoi(pt);
		(this)->SetArrowSize( ival);
		break;
	    case figattr_ArrowPos:
		ival = atoi(pt);
		(this)->SetArrowPos( ival);
		break;
	    case figattr_Arrow:
		ival = atoi(pt);
		(this)->SetArrow( ival);
		break;
	    case figattr_LineStyle:
		ival = atoi(pt);
		(this)->SetLineStyle( ival);
		break;
		/* ##new */
	    case figattr_NumAttributes: /* unknown attribute -- ignore */
		break;
	    default:
		break;
	}

	if (fgets(buf, LINELENGTH, fp) == NULL)
	    return dataobject_PREMATUREEOF;
    }
}

/* returns a pointer to a (static) buffer containing the line style pattern for style val. The returned value may be overwritten by subsequent calls. */ 
char *figattr::LineStylePattern(long val, long lwidth)
{
    char *pt1, *pt2;
    static char retval[16];
    static char styletable[][16] = {  
	{0},				/* solid */	
	{1, 1, 0},			/* single */	
	{2, 2, 0},		    	/* double */	
	{4, 4, 0},		    	/* quad */	
	{4, 2, 1, 2, 0},		/* linedot */	
	{4, 2, 1, 1, 1, 2, 0}		/* line2dot */	
    };

    /* make sure result values are >= 0 and < 128 */
    if (lwidth < 1) lwidth = 1;
    else if (lwidth >= 31) lwidth = 31;

    for (pt1 = styletable[val], pt2 = retval;
	 *pt1;
	 pt1++, pt2++) {
	*pt2 = (*pt1) * lwidth;
    }
    *pt2 = 0;
    return retval;
}

/* on entry, vec should be any vector from back to tip (in fig coords). On exit, it will be a vector (from back to tip) of length equal to the arrowhead (in fig coords). */
void figattr::DrawArrowHead(class figview *v, long tipx, long tipy, long *vecx, long *vecy, long ashape, long asize)
{
    double dparx, dpary, mag; /* fig coords */
    long parx, pary, ortx, orty, crad; /* pix coords */
    static struct point arrowtemp[6];
    static struct rectangle rect;

    dparx = (double)(*vecx);
    dpary = (double)(*vecy);

    mag = sqrt(dparx*dparx + dpary*dpary);
    if (mag > 0.0) {
	dparx = dparx * ((double)asize) / mag;
	dpary = dpary * ((double)asize) / mag;
    }
    else {
	dparx = 0.0;
	dpary = (double)asize;
    }

    parx = (v)->ToPixW((long)dparx);
    pary = (v)->ToPixW((long)dpary);
    ortx = (pary);
    orty = (-parx);

    switch (ashape) {
	case figattr_IsoTriangle:
	default:
	    arrowtemp[0].x = tipx;
	    arrowtemp[0].y = tipy;
	    arrowtemp[1].x = tipx - ortx/2 - 2*parx;
	    arrowtemp[1].y = tipy - orty/2 - 2*pary;
	    arrowtemp[2].x = tipx + ortx/2 - 2*parx;
	    arrowtemp[2].y = tipy + orty/2 - 2*pary;
	    (v)->FillPolygon(arrowtemp, 3, (v)->BlackPattern());

	    *vecx = 2*(long)dparx;
	    *vecy = 2*(long)dpary;
	    break;
	case figattr_EquTriangle:
	    arrowtemp[0].x = tipx;
	    arrowtemp[0].y = tipy;
	    arrowtemp[1].x = tipx - ortx - 2*parx;
	    arrowtemp[1].y = tipy - orty - 2*pary;
	    arrowtemp[2].x = tipx + ortx - 2*parx;
	    arrowtemp[2].y = tipy + orty - 2*pary;
	    (v)->FillPolygon(arrowtemp, 3, (v)->BlackPattern());

	    *vecx = 2*(long)dparx;
	    *vecy = 2*(long)dpary;
	    break;
	case figattr_Square:
	    arrowtemp[0].x = tipx - ortx;
	    arrowtemp[0].y = tipy - orty;
	    arrowtemp[1].x = tipx - ortx - 2*parx;
	    arrowtemp[1].y = tipy - orty - 2*pary;
	    arrowtemp[2].x = tipx + ortx - 2*parx;
	    arrowtemp[2].y = tipy + orty - 2*pary;
	    arrowtemp[3].x = tipx + ortx;
	    arrowtemp[3].y = tipy + orty;
	    (v)->FillPolygon(arrowtemp, 4, (v)->BlackPattern());

	    *vecx = 2*(long)dparx;
	    *vecy = 2*(long)dpary;
	    break;
	case figattr_Circle:
	    crad = (v)->ToPixW(asize);
	    rect.left = (tipx-parx)-crad;
	    rect.width = 2*crad;
	    rect.top = (tipy-pary)-crad;
	    rect.height = 2*crad;
	    (v)->FillOval(&rect, (v)->BlackPattern());

	    *vecx = 2*(long)dparx;
	    *vecy = 2*(long)dpary;
	    break;
    }
}

/* on entry, vec should be any vector from back to tip (in fig coords). On exit, it will be a vector (from back to tip) of length equal to the arrowhead (in fig coords). */
void figattr::SetupPrintArrowHead(struct figattr_arrowhead *arrowhead, class figview *v, long tipx, long tipy, long *vecx, long *vecy, long ashape, long asize)
{
    struct point *arrowtemp = arrowhead->arrowtemp;
    double dparx, dpary, mag; /* fig coords */
    long parx, pary, ortx, orty, crad; /* pix coords */

    dparx = (double)(*vecx);
    dpary = (double)(*vecy);

    mag = sqrt(dparx*dparx + dpary*dpary);
    if (mag > 0.0) {
	dparx = dparx * ((double)asize) / mag;
	dpary = dpary * ((double)asize) / mag;
    }
    else {
	dparx = 0.0;
	dpary = (double)asize;
    }

    tipx = (v)->ToPrintPixW(tipx);
    tipy = (v)->ToPrintPixW(tipy);
    parx = (v)->ToPrintPixW((long)dparx);
    pary = (v)->ToPrintPixW((long)dpary);
    ortx = (pary);
    orty = (-parx);

    arrowhead->ashape = ashape;

    switch (ashape) {
	case figattr_IsoTriangle:
	default:
	    arrowtemp[0].x = tipx;
	    arrowtemp[0].y = tipy;
	    arrowtemp[1].x = tipx - ortx/2 - 2*parx;
	    arrowtemp[1].y = tipy - orty/2 - 2*pary;
	    arrowtemp[2].x = tipx + ortx/2 - 2*parx;
	    arrowtemp[2].y = tipy + orty/2 - 2*pary;
	    /*(v)->FillPolygon(arrowtemp, 3, (v)->BlackPattern());*/

	    *vecx = 2*(long)dparx;
	    *vecy = 2*(long)dpary;
	    break;
	case figattr_EquTriangle:
	    arrowtemp[0].x = tipx;
	    arrowtemp[0].y = tipy;
	    arrowtemp[1].x = tipx - ortx - 2*parx;
	    arrowtemp[1].y = tipy - orty - 2*pary;
	    arrowtemp[2].x = tipx + ortx - 2*parx;
	    arrowtemp[2].y = tipy + orty - 2*pary;

	    *vecx = 2*(long)dparx;
	    *vecy = 2*(long)dpary;
	    break;
	case figattr_Square:
	    arrowtemp[0].x = tipx - ortx;
	    arrowtemp[0].y = tipy - orty;
	    arrowtemp[1].x = tipx - ortx - 2*parx;
	    arrowtemp[1].y = tipy - orty - 2*pary;
	    arrowtemp[2].x = tipx + ortx - 2*parx;
	    arrowtemp[2].y = tipy + orty - 2*pary;
	    arrowtemp[3].x = tipx + ortx;
	    arrowtemp[3].y = tipy + orty;

	    *vecx = 2*(long)dparx;
	    *vecy = 2*(long)dpary;
	    break;
	case figattr_Circle:
	    crad = (v)->ToPixW(asize);
	    arrowtemp[0].x = (tipx-parx);
	    arrowtemp[0].y = (tipy-pary);
	    arrowtemp[1].x = crad;
	    arrowtemp[1].y = crad;

	    *vecx = 2*(long)dparx;
	    *vecy = 2*(long)dpary;
	    break;
    }
}


void figattr::PrintArrowHead(FILE *file, char *prefix, struct figattr_arrowhead *arrowhead)
{
    int ix;

    switch (arrowhead->ashape) {
	case figattr_IsoTriangle:
	default:
	    /*(v)->FillPolygon(arrowtemp, 3, (v)->BlackPattern());*/
	    for (ix=0; ix<3; ix++) {
		fprintf(file, "%s %d %d %s\n", prefix, arrowhead->arrowtemp[ix].x, arrowhead->arrowtemp[ix].y, (ix) ? "lineto" : "moveto");
	    }
	    fprintf(file, "%s closepath fill\n", prefix);
	    break;
	case figattr_EquTriangle:
	      /*(v)->FillPolygon(arrowtemp, 3, (v)->BlackPattern());*/
	    for (ix=0; ix<3; ix++) {
		fprintf(file, "%s %d %d %s\n", prefix, arrowhead->arrowtemp[ix].x, arrowhead->arrowtemp[ix].y, (ix) ? "lineto" : "moveto");
	    }
	    fprintf(file, "%s closepath fill\n", prefix);
	    break;
	case figattr_Square:
	    /*(v)->FillPolygon(arrowtemp, 4, (v)->BlackPattern());*/
	    for (ix=0; ix<4; ix++) {
		fprintf(file, "%s %d %d %s\n", prefix, arrowhead->arrowtemp[ix].x, arrowhead->arrowtemp[ix].y, (ix) ? "lineto" : "moveto");
	    }
	    fprintf(file, "%s closepath fill\n", prefix);
	    break;
	case figattr_Circle:
	    /*(v)->FillOval(&rect, (v)->BlackPattern());*/
	    fprintf(file, "%s %d %d %d 0 360 arc\n", prefix, arrowhead->arrowtemp[0].x, arrowhead->arrowtemp[0].y, arrowhead->arrowtemp[1].x);
	    fprintf(file, "%s closepath fill\n", prefix);
	    break;
    }
}
