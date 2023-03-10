/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
#include <graphic.H>
#include <view.H>
#include <message.H>
#include <table.H>

#include <util.h>

#define AUXMODULE
#include <spread.H>


#include <shared.h>

static boolean debug=FALSE;

static void movecoldown (class spread  * V, int      x , int      y, Chunk  chunk);
static void movecolmove (class spread  * V, int      x  , int      y);
static void movecolup (class spread  * V, int      x  , int      y);
static void movecolcancel (class spread  * V);
static void moverowdown (class spread  * V, int      x  , int      y, Chunk  chunk);
static void moverowmove (class spread  * V, int      x , int      y);
static void moverowup (class spread  * V, int      x  , int      y);
static void moverowcancel (class spread  * V);
static boolean IsNotSeparator (char  ch);
static void EnterCellName (class spread  * V, Chunk  chunk);

boolean CompareChunk(Chunk  to, Chunk  from)
{
    return (to->LeftCol != from->LeftCol ||
		to->RightCol != from->RightCol ||
		to->TopRow != from->TopRow ||
		to->BotRow != from->BotRow);
}

void CopyChunk(Chunk  to, Chunk  from)
{
    to->LeftCol = from->LeftCol;
    to->RightCol = from->RightCol;
    to->TopRow = from->TopRow;
    to->BotRow = from->BotRow;
}

#undef abs
#define abs(x) (((x)<0)?-(x):(x))

/* Get the formula of a cell */

void spread_ComputeAnchorOffsets(class spread *V, struct rectangle *rect) {
    if (MyTable(V)) {
	spread_ScrollLogically(V,V->anchor.TopRow,V->anchor.LeftCol,V->anchor.BotRow,V->anchor.RightCol, NULL);
	if(rect) {
	    rect->left=CtoX(V, V->anchor.LeftCol);
	    rect->top=RtoY(V, V->anchor.TopRow);
	    rect->width=CtoX(V, V->anchor.RightCol + 1) -rect->left+1;
	    rect->height=RtoY(V, V->anchor.BotRow + 1)-rect->top+1;
	}
    }
}

void SetCurrentCell (class spread  * V, Chunk  chunk, boolean expose)
{
    if (debug)
	printf("SetCurrentCell(%d..%d, %d..%d)\n", chunk->LeftCol, chunk->RightCol, chunk->TopRow, chunk->BotRow);

    if(!MyTable(V)) return;
    if (!V->hasInputFocus)
	(V)->WantInputFocus ( V);

    (MyTable(V))->FindBoundary ( chunk);
    if (CompareChunk(&(V->selection), chunk) || CompareChunk(&(V->anchor), chunk)) {
	spread_ClearSelectionBox (V);
	CopyChunk( &(V->anchor), chunk);
        CopyChunk (&(V->selection), chunk);
        TellFormula(V);
	if(expose) {
	    struct rectangle r;
	    spread_ComputeAnchorOffsets(V, &r);
	    V->WantExposure(V,&r);
	}
	(V)->WantUpdate ( V);
    }
    if (debug)
	printf("SetCurrentCell exited\n");
}


static void extendCurrentCell(class spread  * V, Chunk  chunk)
{
    int x0, x1, y0, y1, x2, y2, x3, y3;

    if (debug)
	printf("extendCurrentCell(%d..%d, %d..%d)\n", chunk->LeftCol, chunk->RightCol, chunk->TopRow, chunk->BotRow);

    if (!V->hasInputFocus)
	(V)->WantInputFocus ( V);

    if (chunk->TopRow > V->anchor.TopRow)
	chunk->TopRow = V->anchor.TopRow;
    if (chunk->LeftCol > V->anchor.LeftCol)
	chunk->LeftCol = V->anchor.LeftCol;
    if (chunk->BotRow < V->anchor.BotRow)
	chunk->BotRow = V->anchor.BotRow;
    if (chunk->RightCol < V->anchor.RightCol)
	chunk->RightCol = V->anchor.RightCol;

    (MyTable(V))->FindBoundary ( chunk);

    if (CompareChunk(&(V->selection), chunk)) {

	x0 = CtoX (V, V->selection.LeftCol);
	x1 = CtoX (V, V->selection.RightCol + 1) + spread_SPACING;
	y0 = RtoY (V, V->selection.TopRow);
	y1 = RtoY (V, V->selection.BotRow + 1) + spread_SPACING;
	if (V->selection.LeftCol < 0)
	    x0 = 0;
	else if (x0 < spread_BORDER(V))
	    x0 = spread_BORDER(V);
	if (V->selection.TopRow < 0)
	    y0 = 0;
	else if (y0 < spread_BORDER(V))
	    y0 = spread_BORDER(V);
	if (x0 > x1) x1 = x0;
	if (y0 > y1) y1 = y0;

	x2 = CtoX (V, chunk->LeftCol);
	x3 = CtoX (V, chunk->RightCol + 1) + spread_SPACING;
	y2 = RtoY (V, chunk->TopRow);
	y3 = RtoY (V, chunk->BotRow + 1) + spread_SPACING;
	if (chunk->LeftCol < 0)
	    x2 = 0;
	else if (x2 < spread_BORDER(V))
	    x2 = spread_BORDER(V);
	if (chunk->TopRow < 0)
	    y2 = 0;
	else if (y2 < spread_BORDER(V))
	    y2 = spread_BORDER(V);
	if (x2 > x3) x3 = x2;
	if (y2 > y3) y3 = y2;

	if (V->selectionvisible) {

	    if (x0 != x2)
		spread_InvertRectangle (V, x0, y0, x2 - x0, y1 - y0), x0 = x2;
	    if (x1 != x3)
		spread_InvertRectangle (V, x1, y0, x3 - x1, y1 - y0), x1 = x3;
	    if (y0 != y2)
		spread_InvertRectangle (V, x0, y0, x1 - x0, y2 - y0), y0 = y2;
	    if (y1 != y3)
		spread_InvertRectangle (V, x0, y1, x1 - x0, y3 - y1), y1 = y3;
	}
	CopyChunk (&(V->selection), chunk);
	(V)->WantUpdate ( V);
    }

}

/* Handle mouse hit */

class view * MouseHit (class spread  * V, enum view::MouseAction  action, long  x , long  y, long  numberOfClicks		/* how should i use this?? */)
{
    struct chunk chunk;
    struct cell * hitcell;
    struct chunk extendedchunk;
    int i;
    int xx, yy;
    class view * result;
    class view * child;

    /* Find cell containing (x, y) */
    /* Note that borders (index = -1) and lines (left=right+1) are possibilities */

    for (FirstX(V, i, xx); i < (MyTable(V))->NumberOfColumns() && xx < x - spread_SPACING; NextX(V, i, xx)) ;
    if (xx <= x) {
	chunk.LeftCol = i;
	chunk.RightCol = i - 1;
    }
    else if (i <= 0) {
	chunk.LeftCol = -1;
	chunk.RightCol = (MyTable(V))->NumberOfColumns() - 1;
    }
    else {
	chunk.LeftCol = i - 1;
	chunk.RightCol = i - 1;
	xx -= (V->colInfo[i].computedWidth - 1) + spread_SPACING;
    }
    
    for (FirstY(V, i, yy); i < V->rowInfoCount && yy < y - spread_SPACING; NextY(V, i, yy)) ;
    if (yy <= y) {
	chunk.TopRow = i;
	chunk.BotRow = i - 1;
    }
    else if (i <= 0) {
	chunk.TopRow = -1;
	chunk.BotRow = (MyTable(V))->NumberOfRows() - 1;
    }
    else {
	chunk.TopRow = i - 1;
	chunk.BotRow = i - 1;
	yy -= V->rowInfo[i - 1].computedHeight;
    }
    CopyChunk(&extendedchunk, &chunk);
    (MyTable(V))->FindBoundary( &extendedchunk);

    /* if this is a hit on an active subobject, it gets the hit */

    hitcell = (MyTable(V))->GetCell( extendedchunk.TopRow, extendedchunk.LeftCol);

    if (CompareChunk(&extendedchunk, &(V->anchor)) == 0 && extendedchunk.LeftCol >= 0 && extendedchunk.TopRow >= 0 && extendedchunk.LeftCol <= extendedchunk.RightCol && extendedchunk.TopRow <= extendedchunk.BotRow && hitcell->celltype == table_ImbeddedObject) {
	if ((child = spread_FindSubview(V, hitcell))) {
	    if (debug)
		printf("Passing hit at %ld %ld to child at %p\n", (child)->EnclosedXToLocalX( x), (child)->EnclosedYToLocalY( y), child);
	    result = (child)->Hit( action, (child)->EnclosedXToLocalX( x), (child)->EnclosedYToLocalY( y), numberOfClicks);
	    if (debug)
		printf("Child hit returned %p\n", result);
	    return result;
	}
    }

    /* apply action to chunk */
 
    switch (action) {

    case view::LeftDown:
	switch (V->movemode) {
	case 1:
	    movecolcancel (V);
	    break;
	case 2:
	    moverowcancel (V);
	    break;
	}

	SetCurrentCell (V, &chunk, FALSE);
	if (chunk.TopRow >= 0 && chunk.LeftCol >= 0 && chunk.TopRow <= chunk.BotRow && chunk.LeftCol <= chunk.RightCol && hitcell->celltype == table_ImbeddedObject) {
	    if ((child = spread_FindSubview(V, hitcell))) {
		if (debug)
		    printf("Passing hit at %ld %ld to child at %p\n", (child)->EnclosedXToLocalX( x), (child)->EnclosedYToLocalY( y), child);
		result = (child)->Hit( action, (child)->EnclosedXToLocalX( x), (child)->EnclosedYToLocalY( y), numberOfClicks);
		if (debug)
		    printf("Child hit returned %p\n", result);
		return result;
	    }
	}
	break;

    case view::RightDown:
	switch (V->movemode) {
	case 1:
	    movecolcancel (V);
	    break;
	case 2:
	    moverowcancel (V);
	    break;
	}

	/* right down in border - move cell boundaries */

	if (chunk.LeftCol < 0 && chunk.TopRow < 0)
	    /* do nothing */ ;
	else if (chunk.LeftCol < 0) 
	    moverowdown (V, x, y, &chunk);
	else if (chunk.TopRow < 0)
	    movecoldown (V, x, y, &chunk);
	    
	/* right down in body while entering - enter cell name */

	else if (V->bufferstatus == BUFFERHASINPUT)
	    EnterCellName (V, &chunk);
	
	/* right down in body while selection - extend selection */
	
	else
	    extendCurrentCell (V, &chunk);
	    
	break;

    case view::LeftUp:
	switch (V->movemode) {
	case 0:
	    extendCurrentCell(V, &chunk);
	    break;
	case 1:
	    movecolcancel (V);
	    break;
	case 2:
	    moverowcancel (V);
	    break;
	}
	break;

    case view::RightUp:
	switch (V->movemode) {
	case 1:
	    movecolup (V, x, y);
	    break;
	case 2:
	    moverowup (V, x, y);
	    break;
	}
	break;

    case view::LeftMovement:
	switch (V->movemode) {
	case 0:
	    extendCurrentCell(V, &chunk);
	    break;
	case 1:
	    movecolcancel (V);
	    break;
	case 2:
	    moverowcancel (V);
	    break;
	}
	break;

    case view::RightMovement:
	switch (V->movemode) {
	case 0:
	    if (V->bufferstatus != BUFFERHASINPUT)
		extendCurrentCell(V, &chunk);
	    break;
	case 1:
	    movecolmove (V, x, y);
	    break;
	case 2:
	    moverowmove (V, x, y);
	    break;
	}
	break;

    default: // filedrop, upmovement, nomouse
	break;
    }
    return V;
}


static void movecoldown (class spread  * V, int      x , int      y, Chunk  chunk)
{
    int index;

    if (debug)
	printf("movecoldown\n");
    if (abs(x - (CtoX(V, chunk->LeftCol) + spread_SPACING/2))
     < abs(x - (CtoX(V, chunk->RightCol + 1) + spread_SPACING/2)))
	index = chunk->LeftCol - 1;
    else
	index = chunk->RightCol;
    if (index >= 0 && index <= (MyTable(V))->NumberOfColumns()) {
	V->movemode = 1;
	V->currentslice = index;
	V->currentoffset = CtoX (V, index + 1) + spread_SPACING/2;
	V->icx = x;
	V->icy = y;
	spread_InvertRectangle (V, V->currentoffset, V->icy, V->icx - V->currentoffset, 2);
    }
}

static void movecolmove (class spread  * V, int      x  , int      y)
{
    if (debug)
	printf("movecolmove\n");
    (V)->SetTransferMode ( graphic::INVERT);
    spread_InvertRectangle (V, V->currentoffset, V->icy, V->icx - V->currentoffset, 2);
    V->icx = x;
    V->icy = y;
    spread_InvertRectangle (V, V->currentoffset, V->icy, V->icx - V->currentoffset, 2);
}

static void movecolup (class spread  * V, int      x  , int      y)
{
    movecolcancel (V);
    (MyTable(V))->ChangeThickness ( COLS, V->currentslice,
	(V->colInfo[V->currentslice].computedWidth - V->currentoffset + x));
    (V->GetParent())->WantNewSize( V);
}

static void movecolcancel (class spread  * V)
{
    if (debug)
	printf("movecolcancel\n");
    V->movemode = 0;
    (V)->SetTransferMode ( graphic::INVERT);
    spread_InvertRectangle (V, V->currentoffset, V->icy, V->icx - V->currentoffset, 2);
}

static void moverowdown (class spread  * V, int      x  , int      y, Chunk  chunk)
{
    int index;

    if (debug)
	printf("moverowdown\n");
    if (abs(y - (RtoY(V, chunk->TopRow) + spread_SPACING/2))
     < abs(y - (RtoY(V, chunk->BotRow + 1)) + spread_SPACING/2))
	index = chunk->TopRow - 1;
    else
	index = chunk->BotRow;
    if (index >= 0 && index <= (MyTable(V))->NumberOfRows()) {
	V->movemode = 2;
	V->currentslice = index;
	V->currentoffset = RtoY (V, index + 1) + spread_SPACING/2;
	V->icx = x;
	V->icy = y;
	(V)->SetTransferMode ( graphic::INVERT);
	spread_InvertRectangle (V, V->icx, V->currentoffset, 2, V->icy - V->currentoffset);
    }
}


static void moverowmove (class spread  * V, int      x , int      y)
{
    if (debug)
	printf("moverowmove\n");
    (V)->SetTransferMode ( graphic::INVERT);
    spread_InvertRectangle (V, V->icx, V->currentoffset, 2, V->icy - V->currentoffset);
    V->icx = x;
    V->icy = y;
    spread_InvertRectangle (V, V->icx, V->currentoffset, 2, V->icy - V->currentoffset);
}

static void moverowup (class spread  * V, int      x  , int      y)
{
    if (debug)
	printf("moverowup\n");
    moverowcancel (V);
    (MyTable(V))->ChangeThickness( ROWS, V->currentslice, (MyTable(V))->RowHeight( V->currentslice) - V->currentoffset + y);
    (V->GetParent())->WantNewSize( V);
}

static void moverowcancel (class spread  * V)
{
    if (debug)
	printf("moverowcancel\n");
    V->movemode = 0;
    (V)->SetTransferMode ( graphic::INVERT);
    spread_InvertRectangle (V, V->icx, V->currentoffset, 2, V->icy - V->currentoffset);
}

/* Get the formula of a cell */

void GetFormula (class spread  * V, Chunk  chunk, char  **keybuff)
{
    if (debug)
	printf("GetFormula entered\n");

    if (chunk->TopRow > chunk->BotRow || chunk->LeftCol > chunk->RightCol || chunk->TopRow < 0 || chunk->LeftCol < 0)
	 *keybuff = strdup("");

    else
	(MyTable(V))->FormatCell(
	    (MyTable(V))->GetCell ( chunk->TopRow, chunk->LeftCol), keybuff);

    if (debug)
	printf("GetFormula got %s\n", *keybuff);
}

/* true if character is not a separator */

static boolean IsNotSeparator (char  ch)
{
    return (ch != '=' && ch != '+' && ch != '-' && ch != '*' && ch != '^' && ch != '/' && ch != ':' && ch != ',' && ch != '(' && ch != '[');
}

/* Enter name of selected cell into input buffer

Note that this routine is useful only if bufferstatus = BUFFERHASINPUT
 */
static void EnterCellName (class spread  * V, Chunk  chunk)
{
    char buf[10], rbuf[5], cbuf[5];
    int i;
    char keybuff[257];

    if (debug)
	printf("EnterCellName entered, status = %d\n", V->bufferstatus);
    rbuf[0] = 0;
    cbuf[0] = 0;
    if (chunk->TopRow != V->selection.TopRow) {
	if (chunk->TopRow > V->selection.TopRow)
	    sprintf (rbuf, "+%d", chunk->TopRow - V->selection.TopRow);
	else
	    sprintf (rbuf, "-%d", V->selection.TopRow - chunk->TopRow);
    }
    if (chunk->LeftCol != V->selection.LeftCol) {
	if (chunk->LeftCol > V->selection.LeftCol)
	    sprintf (cbuf, "+%d", chunk->LeftCol - V->selection.LeftCol);
	else
	    sprintf (cbuf, "-%d", V->selection.LeftCol - chunk->LeftCol);
    }
    sprintf (buf, "[r%s,c%s]", rbuf, cbuf);
    i = message::GetCurrentString (V, keybuff, sizeof keybuff);
    if (debug)
	printf("GetCurrentString returned %d and '%s' length %ld", i, keybuff, strlen(keybuff));
    i += strlen(keybuff);
    if (i > 0 && IsNotSeparator(keybuff[i - 1]))
	message::InsertCharacters (V, i++, "+", 1);
    if (debug)
	printf(" inserted at %d\n", i);
    message::InsertCharacters (V, i, buf, strlen(buf));
    message::SetCursorPos(V, i + strlen(buf));

    if (debug)
	printf("EnterCellName exited, status = %d\n", V->bufferstatus);
}


void TellFormula (class spread  * V)
{
    char *keybuff;

    if (debug)
	printf("TellFormula entered\n");
    k_SetMessageState (V, BUFFERHASFORMULA);

    GetFormula (V, &(V->anchor), &keybuff);
    message::DisplayString(V, 0, keybuff);

    if (debug) printf("Cell contents:  \"%s\"\n", keybuff);
    free(keybuff);
}


/* reset current working cell */
void ResetCurrentCell (class spread  * V)
{
    struct chunk topleft;

    topleft.LeftCol = 0;
    topleft.TopRow = 0;
    topleft.RightCol = -1;
    topleft.BotRow = -1;
    if (debug)
	printf("ResetCurrentCell\n");
    SetCurrentCell (V, &topleft, FALSE);
}

/* set current working cell */

