/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
 \* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("spread.H")
#include <view.H>
#include <im.H>
#include <graphic.H>
#include <cursor.H>
#include <environ.H>
#include <fontdesc.H>
#include <keymap.H>
#include <keystate.H>
#include <menulist.H>
#include <scroll.H>
#include <rect.h>
#include <dataobject.H>
#include <table.H>

#include <spread.H>

#include <shared.h>
/* initialize entire class */
static char debug=0;

static class keymap *mainmap =  NULL;
static class menulist *mainmenus =  NULL;
static class keymap *readonlymap = NULL;
static class menulist *readonlymenus = NULL;


static boolean LimitedHighlighting = FALSE;



ATKdefineRegistry(spread, view, spread::InitializeClass);

boolean spread::InitializeClass()
{
    LimitedHighlighting = environ::GetProfileSwitch("limitedspreadhighlight", FALSE);

    mainmap = new keymap;
    readonlymap=new keymap;
    k_DefineKeys (mainmap, readonlymap, &spread_ATKregistry_ );
    
    mainmenus = new menulist;
    readonlymenus=new menulist;
    DefineMenus (mainmenus, mainmap, readonlymenus, readonlymap,  &spread_ATKregistry_ );
    return TRUE;
}

boolean spread::WantLimitedHighlighting()
{
    ATKinit;

    return (LimitedHighlighting);
}

void spread::WantInputFocus(class view *requestor)
{
    if(blockro && MyTable(this)->GetReadOnly()) return;
    view::WantInputFocus(requestor);
}

/* initialize table view */

spread::spread()
{
    ATKinit;
    sizescached=FALSE;
    hidero = environ::GetProfileSwitch("TableHideGridAndLabelsWhenReadOnly", FALSE);
    blockro = environ::GetProfileSwitch("TableBlockInputWhenReadOnly", FALSE);
    if(environ::GetProfileSwitch("TableShowLabels", TRUE)) {
	sborder=20;
    } else sborder=0;
    
    rectangle_EmptyRect(&crect);
    readonlyposted=FALSE;
    finalizing = FALSE;
    hasInputFocus = FALSE;
    childhasfocus = FALSE;
    updateRequests = 0;
    borderDrawn = FALSE;
    rowInfoCount = 0;
    rowInfo = NULL;
    colInfoCount=0;
    colInfo=NULL;
    movemode = 0;
    currentslice = 0;
    currentoffset = 0;
    icx = icy = 0;
    lastTime = -1;
    vOffset = 0;
    hOffset = 0;
    recsearchvalid = FALSE;
    recsearchchild = NULL;
    selectionvisible = FALSE;
    bufferstatus = BUFFEREMPTY;
    keystatep = keystate::Create(this, mainmap);
    readonlykeystate = keystate::Create(this, readonlymap);
    menulistp = (mainmenus)->DuplicateML ( this);
    readonlymenulist = readonlymenus->DuplicateML(this);
    tableCursor = NULL;
    writingFont = NULL;
    standardHeight = 0;
    zeroWidth = 0;
    dotWidth = 0;
    grayPix = NULL;
    blackPix = NULL;
    ResetCurrentCell(this);
    lastTime = -1;
    vOffset = 0;
    hOffset = 0;
    anchor.LeftCol=anchor.RightCol=0;
    anchor.TopRow=anchor.BotRow=0;
    selection.LeftCol=0;
    selection.RightCol=(-1);
    selection.TopRow=0;
    selection.BotRow=(-1);
    THROWONFAILURE( TRUE);
}

/* initialize graphic-dependent data */

void InitializeGraphic(class spread  *V)
{
    class fontdesc *tempFont;
    const char *wfontname = NULL;
    struct FontSummary *fs;

    if (debug)
	printf("InitializeGraphic(%p)\n", V);

    if (!(V->GetDrawable())) {
	printf("InitializeGraphic called without drawable.\n");
	return;
    }
    V->grayPix = (V)->GrayPattern( 8, 16);
    V->blackPix = (V)->BlackPattern();
    V->tableCursor = cursor::Create(V);
    tempFont = fontdesc::Create("table", fontdesc_Plain, 12);
    (V->tableCursor)->SetGlyph( tempFont, 't');
    wfontname = environ::GetProfile("bodyfontfamily");
    if (!wfontname || !*wfontname) wfontname = "andysans";
    V->writingFont = fontdesc::Create (wfontname, fontdesc_Plain, environ::GetProfileInt("bodyfontsize", 12));
    if(V->writingFont) {
	fs = (V->writingFont)->FontSummary(V->GetDrawable());
	V->standardHeight = fs->maxHeight;
	(V->writingFont)->StringSize ( V->GetDrawable(), "0", &(V->zeroWidth), NULL);
	(V->writingFont)->StringSize ( V->GetDrawable(), ".", &(V->dotWidth), NULL);
    } else {
	V->zeroWidth=20;
	V->dotWidth=5;
	V->standardHeight=22;
    }
}

/* recompute thickness of rows */
class view * spread_FindSubview (class spread  * V, struct cell  * cell)
{
    struct viewlist *vl;
    const char *viewname;

    if (cell->celltype != table_ImbeddedObject) {
	printf("FindSubview called with non-subview cell! (internal bug)\n");
	return NULL;
    }
    for (vl = cell->interior.ImbeddedObject.views; vl && vl->parent != V; vl = vl->next) /* no body */ ;
    if (!vl) {
	vl = (struct viewlist *) malloc (sizeof *vl);
	if (!vl) {
	    printf("out of space for subview list\n");
	    return NULL;
	}
	else {
	    viewname = (cell->interior.ImbeddedObject.data)->ViewName();
	    vl->parent = V;
	    vl->child = (class view *) ATK::NewObject(viewname);
	    if (!vl->child) {
		printf("unable to create child view\n");
		return NULL;
	    }
	    (vl->child)->SetDataObject( cell->interior.ImbeddedObject.data); /*  */
	    (vl->child)->LinkTree( vl->parent);
	    (vl->child)->InsertViewSize(vl->parent, 0, 0, 0, 0);
	    if (debug)
		printf("FindSubview created %s at %p for %p\n", viewname, vl->child, vl->parent); 
	    vl->next = cell->interior.ImbeddedObject.views;
	    cell->interior.ImbeddedObject.views = vl;
	}
    }
    else if (debug)
	printf("FindSubview< found %p for %p\n", vl->child, vl->parent);
    return vl->child;
}

void spread::ComputeSizes() {
    table *t=MyTable(this);
    int r, c;
    struct cell *cell;
    long dWidth, dHeight;
    view *child;
    
    if(rowInfoCount==t->NumberOfRows() && rowInfo!=NULL && colInfoCount==t->NumberOfColumns() && colInfo!=NULL && sizescached) return;

    sizescached=TRUE;
    if (rowInfoCount != t->NumberOfRows() || rowInfo == NULL) {
        if (rowInfo != NULL)
            free(rowInfo);
        rowInfo = (struct rowInformation *) malloc(t->NumberOfRows() * sizeof(struct rowInformation));
        if (rowInfo == NULL) {
            fprintf(stderr, "Out of memory for row sizes\n");
            return;
        }
        rowInfoCount = t->NumberOfRows();
    }

    if (colInfoCount != t->NumberOfColumns() || colInfo == NULL) {
        if (colInfo != NULL)
            free(colInfo);
        colInfo = (struct colInformation *) malloc(t->NumberOfColumns() * sizeof(struct colInformation));
        if (colInfo == NULL) {
            fprintf(stderr, "Out of memory for col sizes\n");
            return;
        }
        colInfoCount = t->NumberOfColumns();
    }

    for (c = t->NumberOfColumns()-1; c >= 0; c--) {
        if(!(t->col[c].flags&FLAG_DEFAULT_SIZE)){
            // width explicitly set
            colInfo[c].computedWidth = t->ColumnWidth(c);
        } else colInfo[c].computedWidth=0;
    }
    
    for (r = t->NumberOfRows() - 1; r >= 0; r--) {
        long rh;
        if (t->RowHeight( r) > 0 ) {
            rh=rowInfo[r].computedHeight = t->RowHeight( r);
        } else {
            rowInfo[r].computedHeight = standardHeight + 2 * spread_CELLMARGIN + spread_SPACING;
            rh=0;
        }
        for (c = t->NumberOfColumns()-1; c >= 0; c--) {
            if (!(t->col[c].flags&FLAG_DEFAULT_SIZE)) continue; //  width explicitly set
            if(t->IsJoinedToAnother(r,c)) continue; // accounted for in the base cell.
            cell = t->GetCell( r, c);
            if (cell->celltype != table_ImbeddedObject) {
                colInfo[c].computedWidth=99;
                continue; // the standard default size is ok.
            }
            // XXX should add some intelligence about deciding the size of other kinds of cells too.
            child=spread_FindSubview(this, cell);
            /* 150 is chosen arbitrarily for now... how should we decide how much width we're willing to give an inset?
             future idea:
             take the suggested width of the table
             subtract out any explicitly set column widths or the default width for any column with no insets
             offer each column with insets an equal share of the remaining space.
             if any of the insets in a column report that their size is 'fixed' give the column the maximum of the
             'fixed' width.
             divide any remaining space among columns whose insets indicate that their width is flexible. */
            if(child) child->DesiredSize(150, rh?rh:16384, rh?view::HeightSet:view::NoSet, &dWidth, &dHeight);
            else dWidth=dHeight=0;
            dHeight += 2 * spread_CELLMARGIN + spread_SPACING;
            dWidth += 2 * spread_CELLMARGIN + spread_SPACING;

            // now account for any width & height provided by joined cells.
            int i;
            for (i = r + 1; i < t->NumberOfRows() && t->IsJoinedAbove( i, c); i++) {
                dHeight -= rowInfo[i].computedHeight;
            }
            for (i = c + 1; i < t->NumberOfColumns() && t->IsJoinedToLeft( i, c); i++) {
                dWidth -= colInfo[i].computedWidth;
            }

            // keep the maximum width &height for this row/column.
            if(t->RowHeight(r)<=0 && rowInfo[r].computedHeight<dHeight) {
                rowInfo[r].computedHeight=dHeight;
            }

            if(colInfo[c].computedWidth<dWidth) {
                colInfo[c].computedWidth=dWidth;
            }
        }
    }
    static char *tablesizedebug=getenv("ATKTABLESIZEDEBUG");
    if(tablesizedebug) {
        for(r=0;r<t->NumberOfRows();r++) {
            printf("row %d height:%d\n", r,  rowInfo[r].computedHeight);
            for(c=0;c<t->NumberOfColumns();c++) {
                printf("\t%d:%d", c,colInfo[c].computedWidth);
            }
            printf("\n");
        }
    }
}


void ComputeRowSizes(class spread  *V)
{
    class table *T = MyTable(V);
    int r, c;
    struct cell *cell;
    long dWidth, dHeight;
    class view *child;

    if (debug)
	printf("ComputeRowSizes(%p) with standardHeight = %ld\n", V, V->standardHeight);

    if (V->rowInfoCount != (T)->NumberOfRows() || V->rowInfo == NULL) {
	if (V->rowInfo != NULL)
	    free(V->rowInfo);
	V->rowInfo = (struct rowInformation *) malloc((T)->NumberOfRows() * sizeof(struct rowInformation));
	if (V->rowInfo == NULL) {
            fprintf(stderr, "Out of memory for row sizes\n");
            return;
            
	}
	V->rowInfoCount = (T)->NumberOfRows();
    }

    for (r = (T)->NumberOfRows() - 1; r >= 0; r--) {
	if ((MyTable(V))->RowHeight( r) > 0)
	    V->rowInfo[r].computedHeight = (MyTable(V))->RowHeight( r);
	else {
	    V->rowInfo[r].computedHeight = V->standardHeight + 2 * spread_CELLMARGIN + spread_SPACING;
	    for (c = 0; c < (T)->NumberOfColumns(); c++) {
		if (!(T)->IsJoinedToAnother( r, c)) {
		    cell = (T)->GetCell( r, c);
		    if (cell->celltype == table_ImbeddedObject) {
			int t;
			int width;

			dHeight = 0;
			width = (T)->ColumnWidth( c) - 2 * spread_CELLMARGIN;
			for (t = c + 1; t < (T)->NumberOfColumns() && (T)->IsJoinedToLeft( r, t); t++)  {
			    width += (T)->ColumnWidth( t);
			}
			child = spread_FindSubview(V, cell);
			if (child)
			    (child)->DesiredSize( width , V->rowInfo[r].computedHeight, view::WidthSet, &dWidth, &dHeight);
			else
			    dWidth = dHeight = 0;
			if (debug)
			    printf(" [%d,%d] wants %ld\n", r+1, c+1, dHeight);
			dHeight += 2 * spread_CELLMARGIN + spread_SPACING;
			for (t = r + 1; r < (T)->NumberOfRows() && (T)->IsJoinedAbove( t, c); t++) {
			    dHeight -= V->rowInfo[t].computedHeight;
			}
			if (V->rowInfo[r].computedHeight < dHeight) {
			    V->rowInfo[r].computedHeight = dHeight;
			}
		    }
		}
	    }
	}
    }
}
void spread_ScrollLogically(class spread *V, long tr, long lc, long br, long rc, struct rectangle *bounds) {
    long k;
    struct rectangle lb;
    if(bounds==NULL) {
	V->GetLogicalBounds(&lb);
	bounds=(&lb);
    }
    if ((k = CtoX(V, rc + 1) + spread_SPACING - rectangle_Width(bounds)) > 0) {
	    V->hOffset += k;
	    V->lastTime = -1;
	}
	if ((k = CtoX(V, lc) - spread_BORDER(V)) < 0 && lc >= 0) {
	    V->hOffset += k;
	    V->lastTime = -1;
	}
	if ((k = CtoX(V, (MyTable(V))->NumberOfColumns()+1) - rectangle_Width(bounds)) < 0 && V->hOffset) {
	    V->hOffset += k;
	    V->lastTime = -1;
	}
	/* a bogus left-right centric hack... there should be a preference to allow the same to be done for the right most edge. This code ensures that the left hand border is displayed if possible. x*/
	if ((k = CtoX(V, 0) - spread_BORDER(V)) < 0) {
	    long l = CtoX(V, rc + 1) + spread_SPACING;
	    if(rectangle_Width(bounds)-l>=-k) {
		V->hOffset += k;
		V->lastTime = -1;
	    }
	}
	if (V->hOffset < 0)
	    V->hOffset = 0;
	if ((k = RtoY(V, br + 1) + spread_SPACING - rectangle_Height(bounds)) > 0) {
	    V->vOffset += k;
	    V->lastTime = -1;
	}
	if ((k = RtoY(V, tr) - spread_BORDER(V)) < 0 && tr >= 0) {
	    V->vOffset += k;
	    V->lastTime = -1;
	}
	if ((k = RtoY(V, (MyTable(V))->NumberOfRows()+1) - rectangle_Height(bounds)) < 0 && V->vOffset) {
	    V->vOffset += k;
	    V->lastTime = -1;
	}
	if (V->vOffset < 0)
	    V->vOffset = 0;
}

void spread::WantExposure(class view *requestor, struct rectangle *childrect) {
    if(parent) {
	struct rectangle vb, cl;
	if(requestor!=this) {
	    int r,c;
	    struct cell *cell;
	    for (r = 0; r < MyTable(this)->NumberOfRows(); r++) {
		for (c = 0; c < MyTable(this)->NumberOfColumns(); c++) {
		    cell = MyTable(this)->GetCell( r, c);
		    if (cell->celltype == table_ImbeddedObject) {
			struct view *v=spread_FindSubview(this,cell);
			if(v!=NULL && (v==requestor || requestor->IsAncestor(v))) {
			    spread_ScrollLogically(this, r,c,r,c, NULL);
			    rectangle_SetRectSize(childrect, CtoX(this,c) + spread_SPACING + spread_CELLMARGIN + childrect->left, RtoY(this,r) + spread_SPACING + spread_CELLMARGIN + childrect->top, childrect->width, childrect->height);
			    goto found;
			}
		    }
		}
	    }
	}
	found:
	GetVisualBounds(&vb);
	rectangle_IntersectRect(&cl, &vb, childrect);
	if(rectangle_IsEqualRect(&cl, childrect)) {
	    return;
	}
	parent->WantExposure(this,childrect);
    }
}
/* filter update requests */

void spread::WantUpdate(class view  *requestor)
{

    if (debug)
	printf("spread__WantUpdate(%p,%p) requests = %d\n", this, requestor, updateRequests);

    if ((this != requestor || !(updateRequests++)) && parent != NULL)
	(parent)->WantUpdate( requestor);

    if (debug)
	printf("spread__WantUpdate exited\n");
}

/* negotiate size of view */

view::DSattributes spread::DesiredSize(long  width , long  height, enum view::DSpass  pass, long  *dWidth , long  *dHeight)
{
    long GrossWidth;
    long GrossHeight;


    if (debug)
        printf("spread_DesiredSize(%p, %ld, %ld, %d, .. )\n", this, width, height, (int)pass);

    if (grayPix == NULL)
        InitializeGraphic(this);
    ComputeSizes();

    GrossWidth = spread_Width(this, 0, (MyTable(this))->NumberOfColumns()) + spread_BORDER(this) + spread_SPACING;
    GrossHeight = spread_Height(this, 0, (MyTable(this))->NumberOfRows()) + spread_BORDER(this) + spread_SPACING;

    GrossWidth+=5; GrossHeight+=5; /* leave enough room to avoid having to chop off two of the borders -RSK*/
    *dWidth = (pass == view::WidthSet) ? width : GrossWidth;
    *dHeight = (pass == view::HeightSet) ? height : GrossHeight;

    return (view::DSattributes)
      ((int)((*dWidth > GrossWidth) ? view::WidthFlexible : view::WidthLarger)
       | (int)((*dHeight > GrossHeight) ? view::HeightFlexible : view::HeightLarger));
}

/* handle child's request for a new size */

void spread::WantNewSize(class view  *requestor)
{
    class table *T = MyTable(this);

    if (debug)
	printf("spread_WantNewSize(%p, %p)\n", this, requestor);

    (T)->StampEverything();
    sizescached=FALSE;
}

/* print as part of larger document */

void spread::Print(FILE  * f, const char  *proc /* processor */, const char  *format /* final format */, boolean  toplevel /* am I the top level view? */)
{
    if (debug)
	printf("spread_Print(%p, %p, %s, %s, %d)\n", this, f, proc, format, toplevel);
    putc('\n', f);
    WriteTroff(this, f, proc, format, toplevel);
}

/* full update when window changes */

void spread::FullUpdate(enum view::UpdateType  how, long  left , long  top , long  width , long  height)
{
    struct rectangle cliprect;
    long k;
    struct rectangle lb;
    GetLogicalBounds(&lb);
    if (grayPix == NULL)
	InitializeGraphic(this);

    if(how==view::FullRedraw && !rectangle_IsEqualRect(&lb,&crect)) {
        crect=lb;
        ComputeSizes();
	k=CtoX(this, MyTable(this)->cols);
	if(k<GetLogicalRight()) hOffset-=GetLogicalRight()-k;

	k=RtoY(this, MyTable(this)->rows);
	if(k<GetLogicalBottom()) vOffset-=GetLogicalBottom()-k;
	spread_ComputeAnchorOffsets(this, NULL);
    }
    if (debug)
	printf("spread_FullUpdate(%p, %d, %ld, %ld, %ld, %ld)\n", this, (int)how, left, top, width, height);
    if (how == view::PartialRedraw || how == view::LastPartialRedraw)
	rectangle_SetRectSize(&cliprect, left, top, width, height);
    else
	rectangle_SetRectSize(&cliprect, GetVisualLeft(), GetVisualTop(), GetVisualWidth(), GetVisualHeight());
    spread_update_FullUpdate(this, how, &cliprect);
}

/* partial update */

void spread::Update()
{
    struct rectangle cliprect;
    if (debug)
	printf("spread_Update(%p)\n", this);
    rectangle_SetRectSize(&cliprect, GetVisualLeft(), GetVisualTop(), GetVisualWidth(), GetVisualHeight());
    spread_PartialUpdate(this, view::FullRedraw, &cliprect);
}

/* process mouse hit */



class view * spread::Hit(enum view::MouseAction  action, long  x , long  y, long  numberOfClicks)
{
    if (debug)
	printf("spread_Hit(%p, %d, %ld, %ld, %ld)\n", this, (int) action, x, y, numberOfClicks);
    return MouseHit(this, action, x, y, numberOfClicks);
}

/* input focus lost; remove highlighting */

void spread::LoseInputFocus()
{
    if (debug)
	printf("spread_LoseInputFocus(%p)\n", this);

    if(hasInputFocus) {
	hasInputFocus = 0;
	if (!spread_WantHighlight(this))
	    WantUpdate( this);
    }
}

/* input focus obtained; highlight something */

void spread::ReceiveInputFocus()
{
    if (debug)
	printf("spread_ReceiveInputFocus(%p)\n", this);

    if (!(hasInputFocus)) {
	if (!spread_WantHighlight(this))
	    WantUpdate( this);
	hasInputFocus = 1;
	if(!MyTable(this)->GetReadOnly()) {
	    readonlyposted=FALSE;
	    keystatep->next = NULL;
	    PostKeyState( keystatep);
	    PostMenus( menulistp);
	} else {
	    readonlyposted=TRUE;
	    readonlykeystate->next=NULL;
	    PostKeyState(readonlykeystate);
	    PostMenus(readonlymenulist);
	}
    }
}

/* application layer for main program */

class view *spread::GetApplicationLayer()
{
    if (debug)
	printf("spread_GetApplicationLayer(%p)\n", this);

    return (class view *)scroll::Create(this, scroll_LEFT | scroll_TOP);
}

/* scroll vertically */

static void ySetFrame(class spread  * V, long  pos		/* pel within view to move */, long  coord		/* where to move it to (numerator) */, long  denom		/* where to move it to (denominator) */)
{
    long k = spread_Height(V, 0, (MyTable(V))->NumberOfRows()) + spread_BORDER(V) + spread_SPACING - (V)->GetLogicalHeight();
    V->vOffset = pos - (coord * (V)->GetLogicalHeight()) / denom;
    if (V->vOffset > k)
	V->vOffset = k;
    if (V->vOffset < 0)
	V->vOffset = 0;
    if (debug)
	printf ("ySetFrame(%p, %ld, %ld, %ld) = %ld\n", V, pos, coord, denom, V->vOffset);
    V->lastTime = -1;		/* for full update */
    (V)->WantUpdate( V);
}


/* scroll horizontally */

static void xSetFrame(class spread  * V, long  pos		/* pel within view to move */, long  coord		/* where to move it to (numerator) */, long  denom		/* where to move it to (denominator) */)
{
    long k = spread_Width(V, 0, (MyTable(V))->NumberOfColumns()) + spread_BORDER(V) + spread_SPACING - (V)->GetLogicalWidth();
    V->hOffset = pos - (coord * (V)->GetLogicalWidth()) / denom;
    if (V->hOffset > k)
	V->hOffset = k;
    if (V->hOffset < 0)
	V->hOffset = 0;
    if (debug)
	printf ("xSetFrame(%p, %ld, %ld, %ld) = %ld\n", V, pos, coord, denom, V->hOffset);
    V->lastTime = -1;
    (V)->WantUpdate( V);
}


/* get vertical scrolling information */

static void yGetInfo(class spread  * V, struct range  *total	/* overall inset bounds */, struct range  *seen	/* visible region */, struct range  *dot	/* selected region */)
{
    total->beg = 0;
    total->end = spread_Height(V, 0, (MyTable(V))->NumberOfRows()) + spread_BORDER(V);
    seen->beg = V->vOffset;
    seen->end = seen->beg + (V)->GetLogicalHeight();
    if (seen->end > total->end)
	seen->end = total->end;
    if(V->selection.BotRow >= 0) {
	dot->beg = spread_Height(V, 0, V->selection.TopRow) + spread_BORDER(V);
	dot->end = dot->beg + spread_Height(V, V->selection.TopRow, V->selection.BotRow + 1);
	if (V->selection.TopRow < 0)
	    dot->beg = 0;
    }
    else
	dot->beg = dot->end = spread_Height(V, 0, V->selection.TopRow);
    if (debug) {
	printf ("yGetInfo l=%ld visible=%ld..%ld dot=%ld..%ld\n",
		total->end, seen->beg, seen->end, dot->beg, dot->end);
	fflush (stdout);
    }
}


/* get horizontal scrolling information */

static void xGetInfo(class spread  * V, struct range  *total	/* overall inset bounds */, struct range  *seen	/* visible region */, struct range  *dot	/* selected region */)
{
    total->beg = 0;
    total->end = spread_Width(V, 0, (MyTable(V))->NumberOfColumns()) + spread_BORDER(V);
    seen->beg = V->hOffset;
    seen->end = seen->beg + (V)->GetLogicalWidth();
    if (seen->end > total->end)
	seen->end = total->end;
    if(V->selection.RightCol >= 0) {
	dot->beg = spread_Width(V, 0, V->selection.LeftCol) + spread_BORDER(V);
	dot->end = dot->beg + spread_Width(V, V->selection.LeftCol, V->selection.RightCol + 1);
	if (V->selection.LeftCol < 0)
	    dot->beg = 0;
    }
    else
	dot->beg = dot->end = spread_Width(V, 0, V->selection.LeftCol);
    if (debug) {
	printf ("xGetInfo l=%ld visible=%ld..%ld dot=%ld..%ld\n",
		total->end, seen->beg, seen->end, dot->beg, dot->end);
	fflush (stdout);
    }
}

/* convert vertical window position to view position */

static long yWhatIsAt(class spread  * V, long  coord, long  denom)
{
    long pos = (coord * (V)->GetLogicalHeight()) / denom + V->vOffset;

    if (debug)
	printf ("yWhatIsAt(%p, %ld, %ld) = %ld\n", V, coord, denom, pos);
    return pos;
}

/* convert horizontal window position to view position */

static long xWhatIsAt(class spread  * V, long  coord, long  denom)
{
    long pos = (coord * (V)->GetLogicalWidth()) / denom + V->hOffset;

    if (debug)
	printf ("xWhatIsAt(%p, %ld, %ld) = %ld\n", V, coord, denom, pos);
    return pos;
}

/* Define interface for use by scrollbar */

static const struct scrollfns verticalInterface = {
    (scroll_getinfofptr)yGetInfo,
    (scroll_setframefptr)ySetFrame,
    NULL,
    (scroll_whatfptr)yWhatIsAt,
};

static const struct scrollfns horizontalInterface = {
    (scroll_getinfofptr)xGetInfo,
    (scroll_setframefptr)xSetFrame,
    NULL,
    (scroll_whatfptr)xWhatIsAt,
};

const void *spread::GetInterface(const char  * type)
{
    if (debug)
	printf("spread_GetInterface(%p, %s)\n", this, type);

    if (strcmp(type, "scroll,vertical") == 0) {
	return &verticalInterface;
    } else if (strcmp(type, "scroll,horizontal") == 0) {
	return &horizontalInterface;
    } else
	return NULL;
}



static void DestroySubviews(class spread  * V,class table  *T)
{
    struct cell * cell;
    struct viewlist *vl;
    struct viewlist *prevvl, *nextvl;
    int r, c;

    for (r = 0; r < (T)->NumberOfRows(); r++) {
	for (c = 0; c < (T)->NumberOfColumns(); c++) {
	    cell = (T)->GetCell( r, c);
	    if (cell->celltype == table_ImbeddedObject) {
		for (vl = cell->interior.ImbeddedObject.views, prevvl = NULL; vl != NULL; vl = nextvl) {
		    if (vl->parent == V) {
			if (debug)
			    printf("destroying subview %p for %p\n", vl->child, cell->interior.ImbeddedObject.data);
			nextvl = vl->next;
			(vl->child)->UnlinkTree();
			(vl->child)->Destroy();
			if (prevvl)
			    prevvl->next = nextvl;
			else
			    cell->interior.ImbeddedObject.views = nextvl;
			free((char *) vl);
		    }
		    else {
			prevvl = vl;
			nextvl = vl->next;
		    }
		}
	    }
	}
    }
}

/* tear down a spreadsheet */

spread::~spread ()
{
    finalizing=TRUE;
    if (rowInfo)
	free(rowInfo);
    DestroySubviews(this,MyTable(this));
    delete menulistp;
}

void spread::LinkTree(class view  * parent)
{
    struct viewlist *vl;
    int r, c;
    struct cell *cell;

    if (debug)
	printf("spread_LinkTree(%p, %p)\n", this, parent);

    view::LinkTree( parent);
    if (MyTable(this) == NULL)
	return;
    for (r = 0; r < (MyTable(this))->NumberOfRows(); r++) {
	for (c = 0; c < (MyTable(this))->NumberOfColumns(); c++) {
	    cell = (MyTable(this))->GetCell( r, c);
	    if (cell->celltype == table_ImbeddedObject) {
		for (vl = cell->interior.ImbeddedObject.views; vl; vl = vl->next) {
		    if (vl->parent == this) {
			(vl->child)->LinkTree( vl->parent);
		    }
		}
	    }
	}
    }
}


void spread::UnlinkNotification(class view  *tree)
{
    if(!finalizing) (MyTable(this))->RemoveViewFromTable(tree);
    ((class view *)this)->view::UnlinkNotification(tree);
    lastTime=(-1);
    WantUpdate( this);
}

void spread::ObservedChanged(class observable  *changed, long  status)
{
    if (debug)
	printf("spread_ObservedChanged(%p, %p, %ld)\n", this, changed, status);

    if(status == 42 && !strcmp((changed)->GetTypeName(),"table")) {
	DestroySubviews(this,(class table *)changed);
    }
    view::ObservedChanged( changed, status);
    if(status==observable::OBJECTCHANGED && hasInputFocus && MyTable(this)->GetReadOnly()!=readonlyposted) {
	hasInputFocus=FALSE;
	ReceiveInputFocus();
    }
    if(status==observable::OBJECTCHANGED && changed==GetDataObject()) {
        sizescached=FALSE;
    }
}

void spread::ExposeChild(class view *v)
{
    class dataobject *dob = v->GetDataObject();
    short posx, posy;
    short width, height;
    struct cell *curc;
    struct chunk chunk;
    class table *d;
    
    d = MyTable(this);
    height = d->NumberOfRows();
    width = d->NumberOfColumns();

    for (posy=0; posy<height; posy++) {
	for (posx=0; posx<width; posx++) {
	    if (!d->IsJoinedToAnother(posy, posx)) {
		curc = d->GetCell(posy, posx);
		if (curc->celltype==table_ImbeddedObject && curc->interior.ImbeddedObject.data==dob) {
		    chunk.TopRow = posy;
		    chunk.LeftCol = posx;
		    chunk.BotRow = chunk.TopRow;
		    chunk.RightCol = chunk.LeftCol;
		    SetCurrentCell(this, &chunk);
		    return;
		}
	    }
	}
    }
}

void spread::ChildLosingInputFocus()
{
    childhasfocus = FALSE;

    if (!spread_WantHighlight(this))
	WantUpdate( this);
}

void spread::ChildReceivingInputFocus()
{
    childhasfocus = TRUE;

    if (!spread_WantHighlight(this))
	WantUpdate( this);
}

