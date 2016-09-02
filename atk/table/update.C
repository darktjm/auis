/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
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

#include <andrewos.h>
#include <graphic.H>
#include <fontdesc.H>
#include <im.H>
#include <view.H>
#include <dataobject.H>
#include <rect.h>
#include <table.H>
#include <environ.H>
#define AUXMODULE
#include <spread.H>

#include <shared.h>
#include <math.h>
static boolean debug=0;

/* loops over cells */

#define TooFarLeft(T, j, z, cr) ((j) < V->colInfoCount  && z + spread_SPACING + V->colInfo[j].computedWidth - spread_CELLMARGIN <= rectangle_Left(cr))
#define TooFarUp(T, j, z, cr) ((j) < V->rowInfoCount && z + V->rowInfo[j].computedHeight - spread_CELLMARGIN <= rectangle_Top(cr))
#define XinRange(T, j, z, cr) ((j) < (T)->NumberOfColumns() && (z) < rectangle_Right(cr))
#define YinRange(V, T, j, z, cr) ((j) < (T)->NumberOfRows() && (j) < V->rowInfoCount && (z) < rectangle_Bottom(cr))

/* return first pixel value for given cell index */

void spread_PartialUpdate(class spread  * V, enum view_UpdateType  how, struct rectangle  *updateClipRect);
void spread_InvertRectangle(class spread  * V, int  left , int  top , int  width , int  height);
static void SmashSelection (class spread  * V);
void spread_ClearSelectionBox (class spread  * V );
static void Flush(class spread  * V);
static void updateCells(class spread  * V, int  zapped, enum view_UpdateType  how, struct rectangle  *updateClipRect);
static void updateString (class spread  * V, char  justification, const char  *string, struct rectangle  *cellBounds);
static void  updateValue (class spread  * V, extended_double  *value, char  format, int  precision, struct rectangle  *cellBounds);
static void updateCell(class spread  * V, struct cell  * cell, int      zapped, enum view_UpdateType  how, struct rectangle  *bodyClipRect, struct rectangle  *cellBounds);
static void updateEdges(class spread  * V, struct rectangle  *updateClipRect);
static void updateBorder(class spread  * V, struct rectangle  *updateClipRect);


long spread_Width(class spread  * V, int  i, int  j)
{
    class table *T = MyTable(V);
    long p = 0;

    if (i < 0)
	i = 0;
    if (j < 0)
	j = 0;
    if(j>T->NumberOfColumns()) j=T->NumberOfColumns();
    if(i>T->NumberOfColumns()) i=T->NumberOfColumns();
    
    while (i < j)
	p += (V->colInfo[i++].computedWidth + spread_SPACING);
    while (i > j)
	p -= (V->colInfo[--i].computedWidth + spread_SPACING);
    return p;
}

long spread_Height(class spread  * V, int  i, int  j)
{
    long p = 0;

    if (j < 0)
	j = 0;
    if (i < 0)
	i = 0;
    if (j > V->rowInfoCount)
	j = V->rowInfoCount;
    if (i > V->rowInfoCount)
	i = V->rowInfoCount;
    while (i < j)
	p += (V->rowInfo[i++].computedHeight);
    while (i > j)
	p -= (V->rowInfo[--i].computedHeight);
    return p;
}

/* should I have highlighting? */

int spread_WantHighlight(class spread  *V)
{
    class table *T = MyTable(V);
    
    if(V->hidero && T->GetReadOnly()) return 0;
    
    if (spread::WantLimitedHighlighting()) {
	return (V->hasInputFocus);
    }

    if (V->hasInputFocus
	|| V->bufferstatus == BUFFERHASINPUT
	|| V->bufferstatus == BUFFERHASPARAM
	|| V->childhasfocus
	)
	return 1;
    else
	return 0;
}

/* fix cursors after update */

static void FixCursors(class spread  * V)
{
    struct rectangle vb;

    if (!(V->hasInputFocus))
	(V)->RetractCursor( V->tableCursor);
    else {
	(V)->GetVisualBounds( &vb);
	(V)->PostCursor( &vb, V->tableCursor);
    }
}

/* notify children of full update event */

static void NotifyKids(class spread  * V, enum view_UpdateType  how, struct rectangle  *updateClipRect)
{
    class table *T = MyTable(V);
    int r, c, x, y;
    int r0, c0, x0, y0;
    struct rectangle bodyClipRect;
    struct rectangle cellBounds;
    struct rectangle cellClipRect;
    class view *child;
    struct cell *cell;
    int     xth, yth;
    int     rr, cc;

    V->GetLogicalBounds(&bodyClipRect);
    
    if(how==view_Remove) {
	for(r0=0;r0<T->NumberOfRows();r0++) {
	    for(c0=0;c0<T->NumberOfColumns();c0++) {
		if (!(T)->IsJoinedToAnother( r0, c0)) {
		    cell = (T)->GetCell( r0, c0);
		    if (cell->celltype == table_ImbeddedObject) {
			child = spread_FindSubview(V, cell);
			if (child != 0) {
			    child->InsertViewSize(V, 0,0,0,0);
			    child->FullUpdate(view_Remove,0,0,0,0);
			}
		    }

		}
	    }
	}
	
    }
    rectangle_IntersectRect(&bodyClipRect, &bodyClipRect, updateClipRect);

    for (FirstY(V, r0, y0); TooFarUp(T, r0, y0, &bodyClipRect); NextY(V, r0, y0))
	/* skip rows not visible at top */ ;
    for (FirstX(V, c0, x0); TooFarLeft(T, c0, x0, &bodyClipRect); NextX(V, c0, x0))
	/* skip columns not visible to left */ ;
    for (r = r0, y = y0; YinRange(V, T, r, y + spread_SPACING + spread_CELLMARGIN, &bodyClipRect); NextY(V, r, y)) {
	for (c = c0, x = x0; XinRange(T, c, x + spread_SPACING + spread_CELLMARGIN, &bodyClipRect); NextX(V, c, x)) {
	    if (!(T)->IsJoinedToAnother( r, c)) {
		cell = (T)->GetCell( r, c);
		if (cell->celltype == table_ImbeddedObject) {
		    child = spread_FindSubview(V, cell);
		    if (child != 0) {
			for (rr = r + 1, yth = V->rowInfo[r].computedHeight - spread_SPACING - 2 * spread_CELLMARGIN;
			     (T)->IsJoinedAbove( rr, c);
			     NextY(V, rr, yth)) ;
			for (cc = c + 1, xth = V->colInfo[c].computedWidth - 2 * spread_CELLMARGIN;
			     (T)->IsJoinedToLeft( r, cc);
			     NextX(V, cc, xth)) ;
			if(how!=view_Remove) rectangle_SetRectSize(&cellBounds, x + spread_SPACING + spread_CELLMARGIN, y + spread_SPACING + spread_CELLMARGIN, xth, yth);
			else rectangle_EmptyRect(&cellBounds);
			(child)->InsertView( V, &cellBounds);
			rectangle_IntersectRect(&cellClipRect, &cellBounds, &bodyClipRect);
			(child)->FullUpdate( how,
			  rectangle_Left(&cellClipRect) - rectangle_Left(&cellBounds),
			  rectangle_Top(&cellClipRect) - rectangle_Top(&cellBounds),
			  rectangle_Width(&cellClipRect),
			  rectangle_Height(&cellClipRect));
		    }
		}
	    }
	}
    }
}

/*  redraw when exposed or size changed, etc */

void spread_update_FullUpdate(class spread  * V, enum view_UpdateType  how, struct rectangle  *updateClipRect)
{
    struct FontSummary *fs;

    if (V->grayPix == NULL)
	InitializeGraphic(V);

    V->lastTime = -1;
    switch(how) {

	case view_MoveNoRedraw:
	case view_Remove:
	    NotifyKids(V, how, updateClipRect);
	    FixCursors(V);
	    break;

	default:
	    spread_PartialUpdate(V, view_FullRedraw, updateClipRect);
    }
}


/*  redraw when contents changed */

void spread_PartialUpdate(class spread  * V, enum view_UpdateType  how, struct rectangle  *updateClipRect)
{
    class table *T = MyTable(V);
    int zapped;
    int wanthigh;

    (V)->SetClippingRect( updateClipRect);
    if (V->lastTime < (T)->EverythingTimestamp()) {
	zapped = 1;
	V->borderDrawn = 2;
	(V)->SetTransferMode( graphic_COPY);
	(V)->EraseRect( updateClipRect);
	V->selectionvisible = FALSE;
        V->lastTime = -1;
    }
    else
	zapped = 0;

    if (debug) printf("Update, cellChanged=%d, lastTime=%d, requests = %d\n", (T)->CellsTimestamp(), V->lastTime, V->updateRequests);

    wanthigh = spread_WantHighlight(V);

    if (wanthigh != V->borderDrawn || V->lastTime < (T)->EdgesTimestamp())
	updateEdges(V, updateClipRect);
    if(V->lastTime < (T)->CellsTimestamp())
	updateCells(V, zapped, how, updateClipRect);
    if (wanthigh != V->borderDrawn)
	updateBorder(V, updateClipRect);
    V->lastTime = (T)->CurrentTimestamp();
    if (wanthigh) {
	if (!(V->selectionvisible))
	    SmashSelection (V);
	V->selectionvisible = TRUE;
    }
    FixCursors(V);
    V->updateRequests = 0;
    if (debug)
	printf("*****\n");
}

void spread_InvertRectangle(class spread  * V, int  left , int  top , int  width , int  height)
{
    (V)->SetTransferMode( graphic_INVERT);
    if (width < 0) {
	left += width;
	width = -width;
    }
    if (height < 0) {
	top += height;
	height = -height;
    }
    (V)->FillRectSize( left, top, width, height, V->blackPix);
}

static void SmashSelection (class spread  * V)
{
    int x0, x1, y0, y1;
    int x2, x3, y2, y3;

    if (debug)
	printf("Selection %s\n", (V->selectionvisible) ? "cleared" : "drawn");

    x0 = CtoX (V, V->anchor.LeftCol) + spread_SPACING;
    x1 = CtoX (V, V->anchor.RightCol + 1);
    y0 = RtoY (V, V->anchor.TopRow) + spread_SPACING;
    y1 = RtoY (V, V->anchor.BotRow + 1);
    if (V->anchor.LeftCol < 0)
	x0 = spread_SPACING;
    else if (x0 < spread_BORDER(V) + spread_SPACING)
	x0 = spread_BORDER(V);
    if (V->anchor.TopRow < 0)
	y0 = spread_SPACING;
    else if (y0 < spread_BORDER(V) + spread_SPACING)
	y0 = spread_BORDER(V);
    if (x0 > x1) x1 = x0;
    if (y0 > y1) y1 = y0;

    x2 = CtoX (V, V->selection.LeftCol);
    x3 = CtoX (V, V->selection.RightCol + 1) + spread_SPACING;
    y2 = RtoY (V, V->selection.TopRow);
    y3 = RtoY (V, V->selection.BotRow + 1) + spread_SPACING;
    if (V->selection.LeftCol < 0)
	x2 = 0;
    else if (x2 < spread_BORDER(V))
	x2 = spread_BORDER(V);
    if (V->selection.TopRow < 0)
	y2 = 0;
    else if (y2 < spread_BORDER(V))
	y2 = spread_BORDER(V);
    if (x2 > x3) x3 = x2;
    if (y2 > y3) y3 = y2;

    if (x0 < x1 && y0 < y1) {
	spread_InvertRectangle (V, x2, y2, x3 - x2, y0 - y2);
	spread_InvertRectangle (V, x2, y0, x0 - x2, y1 - y0);
	spread_InvertRectangle (V, x1, y0, x3 - x1, y1 - y0);
	spread_InvertRectangle (V, x2, y1, x3 - x2, y3 - y1);
    } else
	spread_InvertRectangle (V, x2, y2, x3 - x2, y3 - y2);
}

void spread_ClearSelectionBox (class spread  * V )
{
    if(V->selectionvisible) {
	SmashSelection (V);
	V->selectionvisible = FALSE;
    }
}

static void Flush(class spread  * V)
{
    (V)->FlushGraphics();
}

static void updateCells(class spread  * V, int  zapped, enum view_UpdateType  how, struct rectangle  *updateClipRect)
{
    class table *T = MyTable(V);
    int r, c, x, y;
    int r0, c0, x0, y0;
    struct rectangle bodyClipRect;
    struct rectangle cellBounds;

    spread_ClearSelectionBox(V);
    if (debug)
	printf("Cells updated\n");

    rectangle_SetRectSize(&bodyClipRect, spread_BORDER(V), spread_BORDER(V), (V)->GetLogicalWidth() - spread_BORDER(V), (V)->GetLogicalHeight() - spread_BORDER(V));
    rectangle_IntersectRect(&bodyClipRect, &bodyClipRect, updateClipRect);
    for (FirstY(V, r0, y0); TooFarUp(T, r0, y0, &bodyClipRect); NextY(V, r0, y0))
	/* skip rows not visible at top */ ;
    for (FirstX(V, c0, x0); TooFarLeft(T, c0, x0, &bodyClipRect); NextX(V, c0, x0))
	/* skip columns not visible to left */ ;

    for (r = r0, y = y0; YinRange(V, T, r, y + spread_SPACING + spread_CELLMARGIN, &bodyClipRect); NextY(V, r, y)) {
	for (c = c0, x = x0; XinRange(T, c, x + spread_SPACING + spread_CELLMARGIN, &bodyClipRect); NextX(V, c, x)) {
	    if (!(T)->IsJoinedToAnother( r, c)) {
		struct cell * cell = (T)->GetCell( r, c);
		(T)->ReEval( r, c);
		if (V->lastTime < cell->lastcalc) {
		    int     xth, yth;
		    int     rr, cc;
		    for (rr = r + 1, yth = V->rowInfo[r].computedHeight - spread_SPACING - 2 * spread_CELLMARGIN;
			(T)->IsJoinedAbove( rr, c);
			NextY(V, rr, yth)) ;
		    for (cc = c + 1, xth = V->colInfo[c].computedWidth - 2 * spread_CELLMARGIN;
			(T)->IsJoinedToLeft( r, cc);
			NextX(V, cc, xth)) ;
		    rectangle_SetRectSize(&cellBounds,
			x + spread_SPACING + spread_CELLMARGIN, y + spread_SPACING + spread_CELLMARGIN, xth, yth);
		    updateCell(V, cell, zapped, how, &bodyClipRect, &cellBounds);
		}
	    }
	}
    }
    (V)->SetClippingRect( updateClipRect);
}

static void updateString (class spread  * V, char  justification, const char  *string, struct rectangle  *cellBounds)
{
    if (justification == '\"') { 		/* right */
	(V)->MoveTo( rectangle_Left(cellBounds) + rectangle_Width(cellBounds) - 1, rectangle_Top(cellBounds));
	(V)->DrawString( string, graphic_ATTOP | graphic_ATRIGHT);
    }
    else if (justification == '^') {		/* center */
	(V)->MoveTo( rectangle_Left(cellBounds) + (rectangle_Width(cellBounds) >> 1), rectangle_Top(cellBounds));
	(V)->DrawString( string, graphic_ATTOP | graphic_BETWEENLEFTANDRIGHT);
    }
    else {					/* left */
	(V)->MoveTo( rectangle_Left(cellBounds), rectangle_Top(cellBounds));
	(V)->DrawString( string, graphic_ATTOP | graphic_ATLEFT);
    }
}

static void updateValue (class spread  * V, extended_double  *value, char  format, int  precision, struct rectangle  *cellBounds)
{
    class table *T = MyTable(V);
    long     x, y;
    double val;
    int decpt, sign;
    int rightshim;
    char    buf[1030], *p;
    int adjustedprecision = precision + 1;

    if (IsBogus(value)) {
	updateString(V, '^', ExtractBogus(value), cellBounds);
	return;
    }
    if (isnan(StandardValue(value))) {
	updateString(V, '^', "ARITH!", cellBounds);
	return;
    }

    switch (format) {
	case PERCENTFORMAT:
	case CURRENCYFORMAT:
	case EXPFORMAT:
	case GENERALFORMAT:
	case FIXEDFORMAT:
	    {
		do {
		    adjustedprecision--;
		    p = buf;
		    if (format == CURRENCYFORMAT)
			*p++ = '$';
		    if (format == PERCENTFORMAT)
			sprintf (p, "%.*f", adjustedprecision, StandardValue(value) * 100);
		    else
			sprintf (p, "%.*f", adjustedprecision, StandardValue(value));
		    while (*p)
			p++;
		    if (format == PERCENTFORMAT) {
			*p++ = '%';
			*p = '\0';
		    }
		    p--;
		    (V->writingFont)->StringSize ( V->GetDrawable(), buf, &x, &y);
		} while (adjustedprecision > 0 && (x > rectangle_Width(cellBounds)));
		if (format == GENERALFORMAT && precision > 0) {
		    while (p > buf && *p == '0')
			*p-- = ' ';
		}
		if (p > buf && *p == '.')
		    *p-- = ' ';
		if (x > rectangle_Width(cellBounds)) {
		    strcpy (buf, "*");
		    (V->writingFont)->StringSize ( V->GetDrawable(), buf, &x, &y);
		    adjustedprecision = 0;
		}
		rightshim = ((adjustedprecision > 0 || precision <= 0) ? 0 : V->dotWidth) + (precision - adjustedprecision) * V->zeroWidth;
		if (rightshim > rectangle_Width(cellBounds) - x)
		    rightshim = rectangle_Width(cellBounds) - x;
		(V)->MoveTo( rectangle_Left(cellBounds) + rectangle_Width(cellBounds) - 1 - rightshim - x, rectangle_Top(cellBounds));
		(V)->DrawString( buf, graphic_ATTOP | graphic_ATLEFT);
		break;
	    }
	case HORIZONTALBARFORMAT:
	    val = StandardValue(value);
	    if(val > 1.0)
		val = 1.0;
	    if(val < -1.0)
		val = -1.0;
	    if(val < 0.0) {
		int     h = (int) (-val * rectangle_Width(cellBounds) + 0.5);
		(V)->FillRectSize( rectangle_Left(cellBounds) + rectangle_Width(cellBounds) - h, rectangle_Top(cellBounds), h, rectangle_Height(cellBounds), V->blackPix);
	    }
	    else {
		int     h = (int)(val * rectangle_Width(cellBounds) + 0.5);
		(V)->FillRectSize ( rectangle_Left(cellBounds), rectangle_Top(cellBounds), h, rectangle_Height(cellBounds), V->blackPix);
	    }
	    break;
	case VERTICALBARFORMAT:
	    val = StandardValue(value);
	    if(val > 1.0)
		val = 1.0;
	    if(val < -1.0)
		val = -1.0;
	    if(val < 0.0) {
		int     h = (int) (-val * rectangle_Height(cellBounds) + 0.5);
		(V)->FillRectSize ( rectangle_Left(cellBounds), rectangle_Top(cellBounds), rectangle_Width(cellBounds), h, V->blackPix);
	    }
	    else {
		int     h = (int)(val * rectangle_Height(cellBounds) + 0.5);
		(V)->FillRectSize ( rectangle_Left(cellBounds), rectangle_Top(cellBounds) + rectangle_Height(cellBounds) - h, rectangle_Width(cellBounds), rectangle_Height(cellBounds), V->blackPix);
	    }
	    break;
	case DDMMMYYYYFORMAT: 
	case MMMYYYYFORMAT: 
	case DDMMMFORMAT: 
	    {
		val = StandardValue(value);
		(T)->FormatDate( val, buf, format);
		updateString(V, '^', buf, cellBounds);
		break;
	    }
    }
}

static void updateCell(class spread  * V, struct cell  * cell, int      zapped, enum view_UpdateType  how, struct rectangle  *bodyClipRect, struct rectangle  *cellBounds)
{
    struct rectangle cellClipRect;
    class view *child;

    rectangle_IntersectRect(&cellClipRect, cellBounds, bodyClipRect);
    if (rectangle_Width(&cellClipRect) <= 0 || rectangle_Height(&cellClipRect) <= 0) {
	/* fprintf(stderr, "Unnecessary attempt to draw cell.\n"); */
	return;
    }
    (V)->SetClippingRect( &cellClipRect);

    (V)->SetTransferMode( graphic_COPY);
    if (!zapped)
	(V)->EraseRect( &cellClipRect);
    (V)->SetFont( V->writingFont);

    switch (cell->celltype) {

	case table_EmptyCell:
	    /* do nothing */
	    break;

	case table_TextCell:
	    {
	        char justification, *string;

		string = cell->interior.TextCell.textstring;
		justification = *string;
		if (justification == '\'' || justification == '^' || justification == '\"')
		    string++;
		updateString(V, justification, string, cellBounds);
	    }
	    break;

	case table_ValCell:
	    updateValue (V, &(cell->interior.ValCell.value), cell->format, cell->precision, cellBounds);
	    break;

	case table_ImbeddedObject:
	    if ((child = spread_FindSubview(V, cell))) {
		(child)->InsertView( V, cellBounds);
		(child)->FullUpdate( how, rectangle_Left(&cellClipRect) - rectangle_Left(cellBounds), rectangle_Top(&cellClipRect) - rectangle_Top(cellBounds), rectangle_Width(&cellClipRect), rectangle_Height(&cellClipRect));
	    }
	    break;
    }
}

static void updateEdges(class spread  * V, struct rectangle  *updateClipRect)
{
    class table *T = MyTable(V);
    int     x, y, r, c, k;
    int t = ((spread_SPACING - spread_LINEWIDTH)/2);
    struct rectangle edgeClipRect;

    spread_ClearSelectionBox(V);
    if (debug)
	printf("Edges updated\n");

    rectangle_SetRectSize(&edgeClipRect, spread_BORDER(V), spread_BORDER(V), (V)->GetLogicalWidth() - spread_BORDER(V), (V)->GetLogicalHeight() - spread_BORDER(V));
    rectangle_IntersectRect(&edgeClipRect, &edgeClipRect, updateClipRect);
    (V)->SetClippingRect( &edgeClipRect);
    (V)->SetTransferMode( graphic_COPY);

/* 
    Because vertical and horizontal lines intersect, we must first erase all
    old edges and then draw new edges.  To minimize window manager calls, we
    erase full-length lines and draw the longest possible line segments.
    These may not be the full grid length, even while erasing, for we can
    not erase across a joined cell.

 */    /* Erasing phase */

    for (FirstY(V, r, y); YinRange(V, T, (r-1), y, &edgeClipRect); NextY(V, r, y)) {
	for (FirstX(V, c, x); XinRange(T, (c-1), x, &edgeClipRect); NextX(V, c, x)) {
	    int     w;
	    
	    /* Erase horizontal line beginning above this cell. */

	    if (c < (T)->NumberOfColumns()) {
		if ((c <= 0 || (T)->IsJoinedAbove( r, c - 1)) && !(T)->IsJoinedAbove( r, c)) {
		    for (k = c, w = x;
		      XinRange(T, k, w, &edgeClipRect) && !(T)->IsJoinedAbove( r, k);
		      NextX(V, k, w)) ;
		    (V)->EraseRectSize( x + t, y + t, w - x + spread_LINEWIDTH, spread_LINEWIDTH);
		}
	    }
	    
	    /* Erase vertical line beginning to left of this cell. */

	    if (r < (T)->NumberOfRows()) {
		if ((r <= 0 || (T)->IsJoinedToLeft( r - 1, c)) && !(T)->IsJoinedToLeft( r, c)) { 
		    for (k = r, w = y;
		      YinRange(V, T, k, w, &edgeClipRect) && !(T)->IsJoinedToLeft( k, c);
		      NextY(V, k, w)) ;
		    (V)->EraseRectSize( x + t, y + t, spread_LINEWIDTH, w - y + spread_LINEWIDTH);
		}
	    }
	}
    }

    /* Drawing phase.  The line drawn may be black or dotted. */

    for (FirstY(V, r, y); YinRange(V, T, (r-1), y, &edgeClipRect); NextY(V, r, y)) {
	for (FirstX(V, c, x); XinRange(T, (c-1), x, &edgeClipRect); NextX(V, c, x)) {
	    int     w, color;
	    
	    /* Draw horizontal line beginning above this cell. */

	    if (c < (T)->NumberOfColumns()) {
		color = (T)->ColorAbove( r, c);
		if ((spread_WantHighlight(V) || color == BLACK)
		  && (c <= 0 || color != (T)->ColorAbove( r, c - 1))
		  && color >= GHOST) {
		    for (k = c, w =  x;
		      XinRange(T, k, w, &edgeClipRect) && color == (T)->ColorAbove( r, k);
		      NextX(V, k, w)) ;
		    (V)->FillRectSize( x + t, y + t, w - x + spread_LINEWIDTH, spread_LINEWIDTH, color == GHOST ? V->grayPix : V->blackPix);
		}
	    }
	    
	    /* Draw vertical line beginning to left of this cell. */

	    if (r < (T)->NumberOfRows()) {
		color = (T)->ColorToLeft( r, c);
		if ((spread_WantHighlight(V) || color == BLACK)
		  && (r <= 0 || color != (T)->ColorToLeft( r - 1, c)) && color >= GHOST) { 
		    for (k = r, w = y;
		      YinRange(V, T, k, w, &edgeClipRect) && color == (T)->ColorToLeft( k, c);
		      NextY(V, k, w)) ;
		    (V)->FillRectSize( x + t, y + t, spread_LINEWIDTH, w - y + spread_LINEWIDTH, color == GHOST ? V->grayPix : V->blackPix);
		}
	    }
	}
    }
    (V)->SetClippingRect( updateClipRect);
}

#define	spread_BORDERMARGIN 2	/* white space from table edge to bordering label */

static void updateBorder(class spread  * V, struct rectangle  *updateClipRect)
{
    class table *T = MyTable(V);
    int     r, c, x, y;
    char    buff[6];
    struct rectangle borderClipRect;

    spread_ClearSelectionBox(V);
    if (debug)
	printf("Border updated\n");

    V->borderDrawn = spread_WantHighlight(V);
    (V)->SetFont( V->writingFont);

    if(spread_BORDER(V)<=0) return;
    
    (V)->SetTransferMode( graphic_COPY);
    (V)->EraseRectSize( 0, spread_BORDER(V), spread_BORDER(V), (V)->GetLogicalHeight() - spread_BORDER(V));
    (V)->EraseRectSize( spread_BORDER(V), 0, (V)->GetLogicalWidth() - spread_BORDER(V), spread_BORDER(V));

    if (V->borderDrawn) {

	/* border labels down left side */

	rectangle_SetRectSize (&borderClipRect, 0, spread_BORDER(V), spread_BORDER(V), (V)->GetLogicalHeight() - spread_BORDER(V));
	rectangle_IntersectRect(&borderClipRect, &borderClipRect, updateClipRect);
	(V)->SetClippingRect( &borderClipRect);
	for (FirstY(V, r, y); TooFarUp(T, r, y, &borderClipRect); NextY(V, r, y))
	    /* skip rows */ ;
	for (; YinRange(V, T, r, y + spread_SPACING + spread_CELLMARGIN, &borderClipRect); NextY(V, r, y)) {
	    sprintf (buff, "%d", r + 1);
	    (V)->MoveTo( spread_BORDER(V) - spread_BORDERMARGIN, y + spread_SPACING + spread_CELLMARGIN);
	    (V)->DrawString( buff, graphic_ATTOP | graphic_ATRIGHT);
	}

	/* border labels across top */

	rectangle_SetRectSize (&borderClipRect, spread_BORDER(V), 0, (V)->GetLogicalWidth() - spread_BORDER(V), spread_BORDER(V));
	rectangle_IntersectRect(&borderClipRect, &borderClipRect, updateClipRect);
	(V)->SetClippingRect( &borderClipRect);
	for (FirstX(V, c, x); TooFarLeft(T, c, x, &borderClipRect); NextX(V, c, x))
	    /* skip columns */ ;
	for (; XinRange(T, c, x + spread_SPACING + spread_CELLMARGIN, &borderClipRect); NextX(V, c, x)) {
	    sprintf (buff, "%d", c + 1);
	    (V)->MoveTo( x +(V->colInfo[c].computedWidth / 2), (spread_BORDER(V) - spread_BORDERMARGIN));
	    (V)->DrawString( buff, graphic_ATBASELINE | graphic_BETWEENLEFTANDRIGHT);
	}

    }
    (V)->SetClippingRect( updateClipRect);
}
