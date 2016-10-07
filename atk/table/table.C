/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h>
ATK_IMPL("table.H")

#include <ctype.h>
#include <dataobject.H>
#include <view.H>
#include <graphic.H>
#include <fontdesc.H>

#include <table.H>

#include <shared.h>

/* globals for entire package */

static class table * table_List;	/* root of list of all tables */

static char table_debug;			/* nonzero if debugging */
NO_DLL_EXPORT int table_DefaultPrecision;		/* precision when none provided */

/* initialize entire class */


boolean table::InitializeClass()
    {
    if (table_debug)
	printf("table_InitializeClass()\n");

    table_DefaultPrecision = 2;
    table_debug = 0;
    return TRUE;
}

ATKdefineRegistry(table, dataobject, table::InitializeClass);
void IgnoreObserved(class table  *T);
static char * myrealloc (char  * s, int  n);
static void CreateCell (class table  *T, struct cell  *newcell , struct cell  *oldcell);
void RemoveCellView(class table  *T,struct cell  *c,class view  *v);
void DestroyCell(class table  *T, struct cell  *oldcell);
void rangeLimit (class table  * T, Chunk  chunk);
void rcref (class table  * T, extended_double  *result, int      r , int      c , int      iftaped);
void MakeStandard(extended_double  *x, double  value);
void MakeBogus(extended_double  *x, const char  *message);


/* return corresponding view name */

const char * table::ViewName ()
{
    return "spread";
}

/* table data structure creation */

/* Initialize new data table */

table::table ()
{
	ATKinit;

    static int uniquifier = 0;
    char buff[20];

    if (table_debug)
	printf("table_InitializeObject()\n");

    this->tablename = NULL;
    this->NextTable = NULL;
    this->rows = this->cols = 0;
    this->row = this->col = NULL;
    this->cells = NULL;
    this->leftedge = NULL;
    this->aboveedge = NULL;
    this->edgeChanged = this->cellChanged = this->fileWritten = 0;
    this->everythingChanged = this->timeStamper = 1;
    this->inGetModified = FALSE;
    this->readonly = FALSE;
    /* the following is unnecessary for internal tables */

    (this)->ChangeSize ( 10, 5);
    sprintf(buff, "Table%d", ++uniquifier);
    (this)->SetName ( buff);
    THROWONFAILURE( TRUE);
}


void IgnoreObserved(class table  *T)
{
    long r,c;
    struct cell *cell;
    for (r = 0; r < (T)->NumberOfRows(); r++) {
	for (c = 0; c < (T)->NumberOfColumns(); c++) {
	    cell = (T)->GetCell( r, c);
	    if (cell->celltype == table_ImbeddedObject && cell->interior.ImbeddedObject.data!=NULL)
		(cell->interior.ImbeddedObject.data)->RemoveObserver( T);
	}
    }
}
    
/* tear down a table */

table::~table ()
{
	ATKinit;

    class table * S;
    
    if (table_debug)
	printf("table_FinalizeObject(%s)\n", (this)->Name());
    
   /* this is bad... there should be only one observable::OBJECTDESTROYED message sent, and it will be sent by the observable destructor. therefore here we use a magic value 42 which spread understands. -rr2b */
    (this)->NotifyObservers(42);
    IgnoreObserved(this);
    
    if (this->tablename) {
	if (this == table_List)
	    table_List = this->NextTable;
	else {
	    for (S = table_List; S->NextTable != this; S = S->NextTable) ;
	    S->NextTable = this->NextTable;
	}
	free (this->tablename);
	this->tablename = NULL;
    }
    (this)->ChangeSize ( 0, 0);
}

void table::SetReadOnly(boolean ro) {
    struct attributes attrs;
    long r,c;
    struct cell *cell;
    if(this->readonly==ro) return;
    this->readonly=ro;
    attrs.key="readonly";
    attrs.next=NULL;
    attrs.value.integer=ro;
    for (r = 0; r < this->NumberOfRows(); r++) {
	for (c = 0; c < this->NumberOfColumns(); c++) {
	    cell = this->GetCell( r, c);
	    if (cell->celltype == table_ImbeddedObject && cell->interior.ImbeddedObject.data!=NULL)
		(cell->interior.ImbeddedObject.data)->SetAttributes(&attrs);
	}
    }
    this->NotifyObservers(0);
}
    
void table::SetAttributes(struct attributes *attributes) {
    boolean oldreadonly=this->readonly;
    this->dataobject::SetAttributes(attributes);
    while (attributes) {
	if (strcmp(attributes->key, "readonly") == 0) {
	    this->SetReadOnly(attributes->value.integer);
	}
	attributes = attributes->next;
    }
    if(oldreadonly!=this->readonly) this->NotifyObservers(0);
}

void table::ObservedChanged(class observable  *changed, long  value)
{
    this->cellChanged = ++(this->timeStamper);
    (this)->NotifyObservers( 0);
    (this)->SetModified();
}

/* toggle table_debugging flag */

void table::ToggleDebug ()
{
    if (table_debug) {
	printf("Table debugging off\n");
	table_debug = 0;
    } else {
	table_debug = 1;
	printf("Table debugging on - ESC-ESC turns it off\n");
    }
}

/* find a table by name */

class table *table::FindName (const char  * name)
{
    class table *self=this;
    if (table_debug)  {
	printf("table_FindName(%s, %s)\n", (self)->Name(), name);
    }

    for (self = table_List; self; self = self->NextTable)
	if ((self->tablename != NULL) && strcmp (name, self->tablename) == 0)
	    break;
    return self;
}

/* rename a table (may return pointer to pre-existing table) */

class table * table::SetName (const char  * name)
{
    class table * S;

    if (table_debug)
	printf("table_SetName(%s, %s)\n", (this)->Name(), name);
    
    S = (this)->FindName ( name);
    if (S)
    	return S;

    if (this->tablename)
	free (this->tablename);
    else {
	this->NextTable = table_List;
	table_List = this;
    }
    this->tablename = strdup (name);
    if (this->tablename == NULL)
	printf("Out of memory for table name\n");
    return this;
}

/* reallocate including provisions for null pointers */

static char * myrealloc (char  * s, int  n)
{
    char *news;

    if (s == NULL && n <= 0)
	return NULL;
    else if (s == NULL)
	news = (char *)malloc (n);
    else if (n <= 0) {
	free (s);
	return NULL;
    }
    else
	news = (char *)realloc (s, n);

    if (news == NULL) {
	printf("Out of memory for new table\n");
	exit(4);
    }
    return news;
}

/* change dimensions of existing table */

void table::ChangeSize (int  nrows , int  ncols)
{
    int     r, c;
    struct cell *newcells, *p, *q;
    char *oldleft, *oldabove, *eleft, *eabove, *fleft, *fabove;
    int oldrows, oldcols;

    if (table_debug)
	printf("table_ChangeSize(%s, %d, %d)\n", (this)->Name(), nrows, ncols);

    /* the row and col arrays are one-dimensional, so realloc works */

    this->row = (struct slice   *) myrealloc ((char *)this->row, nrows * sizeof (struct slice));
    for (r = (this)->NumberOfRows(); r < nrows; r++) {
        this->row[r].thickness = TABLE_DEFAULT_ROW_THICKNESS;
        this->row[r].flags |= FLAG_DEFAULT_SIZE; // not explicitly set
    }

    this->col = (struct slice   *) myrealloc ((char *)this->col, ncols * sizeof (struct slice));
    for (c = ((this)->NumberOfColumns() >= 0 ? (this)->NumberOfColumns() : 0); c < ncols; c++)
    {
        this->col[c].thickness = TABLE_DEFAULT_COLUMN_THICKNESS;
        this->col[c].flags |= FLAG_DEFAULT_SIZE; // not explicitly set.
    }

    /* reallocate and copy cell array */

    for (r = nrows, p = (this)->GetCell( nrows, 0); r < (this)->NumberOfRows(); r++)
	for (c = 0; c < (this)->NumberOfColumns(); c++, p++)
	    DestroyCell (this, p);

    if (ncols == (this)->NumberOfColumns())
	p = q = newcells = (struct cell *) myrealloc ( (char *)this->cells,
	    nrows * ncols * sizeof (struct cell));
    else {
	p = this->cells;
	q = newcells = (struct cell *) myrealloc ( (char *)NULL,
	    nrows * ncols * sizeof (struct cell));
    }

    for (r = 0; r < nrows && r < (this)->NumberOfRows(); r++) {
	for (c = 0; c < ncols && c < (this)->NumberOfColumns(); c++, p++, q++)
	    if (p != q)
		memmove ((char *)q, (char *)p, sizeof(struct cell));
	for ( ; c < ncols; c++, q++) 
	    CreateCell (this, q, (struct cell *) NULL);
	for ( ; c < (this)->NumberOfColumns(); c++, p++)
	    DestroyCell (this, p);
    }
    for ( ; r < nrows; r++)
	for (c = 0; c < ncols; c++, q++)
	    CreateCell (this, q, (struct cell *) NULL);

    if (ncols != (this)->NumberOfColumns())
	this->cells = (struct cell *)myrealloc ((char *)this->cells, 0);
    this->cells = newcells;


    /* reallocate and copy edges */

    oldrows = (this)->NumberOfRows();
    oldcols = (this)->NumberOfColumns();
    this->rows = nrows;
    (this)->NumberOfColumns() = ncols;
    if (ncols == oldcols) {
	oldleft = oldabove = NULL;
	eleft = fleft = this->leftedge = myrealloc (this->leftedge,
	    adjustbyone(nrows) * adjustbyone(ncols) * sizeof (char));
	eabove = fabove = this->aboveedge = myrealloc (this->aboveedge,
	    adjustbyone(nrows) * adjustbyone(ncols) * sizeof (char));
    }
    else {
	eleft = oldleft = this->leftedge;
	eabove = oldabove = this->aboveedge;
	fleft = this->leftedge = myrealloc ( (char *) NULL,
	    adjustbyone(nrows) * adjustbyone(ncols) * sizeof (char));
	fabove = this->aboveedge = myrealloc ( (char *) NULL,
	    adjustbyone(nrows) * adjustbyone(ncols) * sizeof (char));
    }

    for (r = 0; r < adjustbyone(nrows) && r < adjustbyone(oldrows); r++) {
	for (c = 0; c < adjustbyone(ncols) && c < adjustbyone(oldcols); c++) {
	    *fleft++ = *eleft++;
	    *fabove++ = *eabove++;
	}
	for ( ; c < adjustbyone(ncols); c++) {
	    *fleft++ = *fabove++ = GHOST;
	}
	for ( ; c < adjustbyone(oldcols); c++, eleft++, eabove++)
	    /* do nothing */ ;
    }
    for ( ; r < adjustbyone(nrows); r++)
	for (c = 0; c < adjustbyone(ncols); c++) {
	    *fleft++ = *fabove++ = GHOST;
	}

    if (oldleft != NULL)
	oldleft = myrealloc (oldleft, 0);
    if (oldabove != NULL)
	oldabove = myrealloc (oldabove, 0);

    (this)->StampEverything();
    (this)->NotifyObservers( 0);
    (this)->SetModified();
}

/* extract chunk of table (destroys original) */

class table * table::ExtractData (Chunk  chunk)
{
    class table * S = new table;
    int r0, c0, nrows, ncols;
    int     r, c;

    if (table_debug)
	printf("table_ExtractData(%s, )\n", (this)->Name());

    r0 = max (chunk->TopRow, 0);
    c0 = max (chunk->LeftCol, 0);
    nrows = chunk->BotRow + 1 - r0;
    ncols = chunk->RightCol + 1 - c0;

    (S)->ChangeSize ( nrows, ncols);

    for (r = 0; r < (S)->NumberOfRows(); r++) {
        S->row[r].thickness = this->row[r + r0].thickness;
        S->row[r].flags = this->row[r + r0].flags;
    }

    for (c = 0; c < (S)->NumberOfColumns(); c++) {
        S->col[c].thickness = this->col[c + c0].thickness;
        S->col[c].flags = this->col[c + c0].flags;
    }

    for (r = 0; r < (S)->NumberOfRows(); r++) {
	for (c = 0; c < (S)->NumberOfColumns(); c++) {
	    struct cell *q = (S)->GetCell( r, c), *p = (this)->GetCell( r + r0, c + c0);
	    DestroyCell (S, q);
	    *q = *p;
	    CreateCell (this, p, (struct cell *) NULL);
	}
    }

    for (r = 0; r < adjustbyone((S)->NumberOfRows()); r++) {
	for (c = 0; c < adjustbyone((S)->NumberOfColumns()); c++) {
	    (S)->SetLeftColor( r, c, (this)->ColorToLeft( r+r0, c+c0));
	    (S)->SetAboveColor( r, c, (this)->ColorAbove( r+r0, c+c0));
	}
    }
    this->cellChanged = ++(this->timeStamper);
    (this)->NotifyObservers( 0);
    (this)->SetModified();
    return S;
}

/* insert chunk of table */

void table::InsertData (class table  * T, Chunk  chunk)
{
    int r0, c0;
    int r1, c1;
    int     r, c;
    int nrows, ncols;
    struct cell *cell;
    int iscombined;
    int newcolor;

    if (table_debug)
	printf("table_InsertData(%s, %s, )\n", (this)->Name(), (T)->Name());

    /* convenient variables for size and location of chunk */

    r0 = max (chunk->TopRow, 0);
    c0 = max (chunk->LeftCol, 0);
    nrows = chunk->BotRow + 1 - r0;
    ncols = chunk->RightCol + 1 - c0;
    r1 = (T)->NumberOfRows();
    c1 = (T)->NumberOfColumns();

    /* if pasting into a combined cell, paste only into main cell */

    iscombined = 1;
    for (r = 0; r < nrows && iscombined == 1; r++) {
	for (c = 0; c < ncols && iscombined == 1; c++) {
	    if ((r > 0 && !(this)->IsJoinedAbove( r+r0, c+c0)) || (c > 0 && !(this)->IsJoinedToLeft( r+r0, c+c0)))
		iscombined = 0;
	}
    }

    /* replace thickness data only if entire row/column involved */

    if (chunk->LeftCol < 0)
        for (r = 0; r < nrows; r++) {
            this->row[r + r0].thickness = T->row[r % r1].thickness;
            this->row[r + r0].flags = T->row[r % r1].flags;
        }
    if (chunk->TopRow < 0)
        for (c = 0; c < ncols; c++) {
            this->col[c + c0].thickness = T->col[c % c1].thickness;
            this->col[c + c0].flags = T->col[c % c1].flags;
        }

    for (r = 0; r < nrows; r++) {
	for (c = 0; c < ncols; c++) {

	    /* replace contents of cells */

	    cell = (this)->GetCell( r + r0, c + c0);
	    DestroyCell (this, cell);
	    CreateCell (this, cell, (T)->GetCell( r % r1, c % c1));
	    
	   if(cell->celltype==table_ImbeddedObject)
	       (cell->interior.ImbeddedObject.data)->RemoveObserver( T);
	   
	    if (iscombined != 0) break;

	    /* replace interior boundaries */

	    if (c > 0) {
		newcolor = (T)->ColorToLeft( r%r1, c%c1);
		if ((c%c1) == 0 && newcolor == GHOST)
		    newcolor = (T)->ColorToLeft( r%r1, c1);
		(this)->SetLeftColor( r+r0, c+c0, newcolor);
	    }
	    if (r > 0) {
		newcolor = (T)->ColorAbove( r%r1, c%c1);
		if ((r%r1) == 0 && newcolor == GHOST)
		    newcolor = (T)->ColorAbove( r1, c%c1);
		(this)->SetAboveColor( r+r0, c+c0, newcolor);
	    }
	}
	if (iscombined != 0) break;
    }

    /* replace exterior boundaries */

    for (c = 0; c < ncols; c++) {
	if ((this)->ColorAbove( r0, c+c0) == GHOST)
	    (this)->SetAboveColor( r0, c+c0, (T)->ColorAbove( 0, c % c1));
	if ((this)->ColorAbove( nrows+r0, c+c0) == GHOST)
	    (this)->SetAboveColor( nrows+r0, c+c0, (T)->ColorAbove( (nrows - 1) % r1 + 1, c % c1));
    }
    for (r = 0; r < nrows; r++) {
	if ((this)->ColorToLeft( r+r0, c0) == GHOST)
	    (this)->SetLeftColor( r+r0, c0, (T)->ColorToLeft( r % r1, 0));
	if ((this)->ColorToLeft( r+r0, ncols+c0) == GHOST)
	    (this)->SetLeftColor( r+r0, ncols+c0, (T)->ColorToLeft( r % r1, (ncols - 1) % c1 + 1));
    }

    /* tell the world */

    (this)->StampEverything();
    (this)->NotifyObservers( 0);
    (T)->SetModified();
}

/* write table to file */

long table::Write (FILE  * f, long  writeID, int  level)
{
    struct chunk chunk;

    if (table_debug)
	printf("table_Write(%s,, %ld, %d)\n", (this)->Name(), writeID, level);

    if (getDataObject(this).GetWriteID() != writeID) {
	getDataObject(this).SetWriteID(writeID);
	fprintf (f, "\\begindata{%s,%ld}\n", (this)->GetTypeName(), (this)->GetID());
	chunk.LeftCol = -1;
	chunk.RightCol = (this)->NumberOfColumns() - 1;
	chunk.TopRow = -1;
	chunk.BotRow = (this)->NumberOfRows() - 1;
	::WriteASCII (this, f, &chunk, level);
	fprintf (f, "\\enddata{%s,%ld}\n", (this)->GetTypeName(), (this)->GetID());
    }
    return (this)->GetID();
}

/* read table from file */

long table::Read (FILE  * f, long  id)
{
    if (table_debug)
	printf("table_Read(%s,, %ld)\n", (this)->Name(), id);

    (this)->SetModified();
    ::ReadASCII (this, f);

    this->fileWritten = this->timeStamper;
    (this)->NotifyObservers( observable::OBJECTCHANGED);
    return dataobject::NOREADERROR;
}

/* write subrectangle */

void table::WriteASCII (FILE  *f, Chunk  chunk)
{
    if (table_debug)
	printf("table_WriteASCII(%s)\n", (this)->Name());

    ::WriteASCII(this, f, chunk, 0);
}

/* read subrectangle */

class table * table::ReadASCII (FILE  *f)
{
    if (table_debug)
	printf("table_ReadASCII(%s)\n", (this)->Name());

    return ::ReadASCII(this, f);
}

/* format cell contents for external display */

void table::FormatCell (struct cell  * cell, char  **buff)
{
    if (table_debug)
	printf("table_FormatCell(%s, , )\n", (this)->Name());

    WriteCell (this, NULL, cell, buff, 0);
}

/* parse external cell contents */

void table::ParseCell(struct cell  * cell, char  *buff)
{
    char *cp = buff;

    ReadCell (this, NULL, buff, &cp, buff+1000, cell);
    this->cellChanged = ++(this->timeStamper);
    (this)->NotifyObservers( 0);
    (this)->SetModified();
}


/* create and copy a cell */


static void CreateCell (class table  *T, struct cell  *newcell , struct cell  *oldcell)
{
    if (oldcell) {
	newcell->format = oldcell->format;
	newcell->lock = oldcell->lock;
	newcell->precision = oldcell->precision;
	newcell->celltype = oldcell->celltype;
	switch (newcell->celltype) {
	    case table_TextCell:
		newcell->interior.TextCell.textstring = strdup (oldcell->interior.TextCell.textstring);
		if (newcell->interior.TextCell.textstring == NULL) {
		    newcell->celltype = table_EmptyCell;
		    break;
		}
		newcell->lastcalc = ++(T->timeStamper);
		break;
	    case table_ValCell:
		newcell->interior.ValCell.formula = strdup (oldcell->interior.ValCell.formula);
		if (newcell->interior.ValCell.formula == NULL) {
		    newcell->celltype = table_EmptyCell;
		    break;
		}
		newcell->interior.ValCell.value = oldcell->interior.ValCell.value;
		newcell->lastcalc = 0;
		break;
	    case table_ImbeddedObject:
		newcell->interior.ImbeddedObject.data = oldcell->interior.ImbeddedObject.data;
		newcell->interior.ImbeddedObject.views = oldcell->interior.ImbeddedObject.views;
		oldcell->celltype = table_EmptyCell;
		
		(newcell->interior.ImbeddedObject.data)->AddObserver( T);
		
	    default:
		newcell->lastcalc = ++(T->timeStamper);
	}
    }
    else {
	newcell->format = GENERALFORMAT;
	newcell->lock = 0;
	newcell->precision = table_DefaultPrecision;
	newcell->lastcalc = ++(T->timeStamper);
	newcell->celltype = table_EmptyCell;
    }
}

void RemoveCellView(class table  *T,struct cell  *c,class view  *v)
{
    struct viewlist *vl=c->interior.ImbeddedObject.views;
    if(vl==NULL) return;
    if(vl->child==v) {
	c->interior.ImbeddedObject.views=vl->next;
	if(vl->next==NULL) {
	    CreateCell(T,c,NULL);
	}
	free(vl);
    } else {
	struct viewlist *last=vl;
	while((vl=vl->next)) {
	    if(vl->child==v) {
		last->next=vl->next;
		free(vl);
		return;
	    } else last=vl;
	}
    }
}

void table::RemoveViewFromTable(class view  *v)
{
    long r,c;
    struct cell *cell;
    for (r = 0; r < (this)->NumberOfRows(); r++) {
	for (c = 0; c < (this)->NumberOfColumns(); c++) {
	    cell = (this)->GetCell( r, c);
	    if (cell->celltype == table_ImbeddedObject)
		RemoveCellView(this,cell,v);
	}
    }
    this->cellChanged = ++(this->timeStamper);
    (this)->NotifyObservers( 0);
    (this)->SetModified();
}

/* delete a cell */

void DestroyCell(class table  *T, struct cell  *oldcell)
{
    struct viewlist *vl;

    switch (oldcell->celltype) {
	case table_TextCell:
	    free(oldcell->interior.TextCell.textstring);
	    break;
	case table_ValCell:
	    free(oldcell->interior.ValCell.formula);
	    break;
	case table_ImbeddedObject:
	   
	    if (oldcell->interior.ImbeddedObject.data) {
		(oldcell->interior.ImbeddedObject.data)->Destroy();
		oldcell->interior.ImbeddedObject.data = NULL;
	    }
	    while ((vl = oldcell->interior.ImbeddedObject.views)) {
		if (table_debug)
		    printf("**MISUSE** removing imbedded view ref by %p\n", vl);
		    (vl->child)->UnlinkTree();
		    (vl->child)->Destroy();
		/* end of misuse */
		oldcell->interior.ImbeddedObject.views = vl->next;
		    free((char *) vl);
	    }
	    if (table_debug)
		printf("imbedded remove complete\n");
	    
	    break;
	case table_EmptyCell:
	    break;
    }
    oldcell->format = GENERALFORMAT;
    oldcell->lock = 0;
    oldcell->precision = table_DefaultPrecision;
    oldcell->celltype = table_EmptyCell;
    oldcell->lastcalc = ++(T->timeStamper);
}

void table::ChangeThickness (Dimension  dim, int      i , int      thickness)
{
    struct slice * slice;

    if (table_debug)
	printf("table_ChangeThickness(%s, %d, %d, %d)\n", (this)->Name(), (int)dim, i, thickness);

    if (dim == ROWS) {
	slice = this->row;
	if (thickness <= 0)
	    thickness = TABLE_DEFAULT_ROW_THICKNESS;
    }
    else {
	slice = this->col;
	if (thickness < 10)
	    thickness = 10;
    }

    slice[i].thickness = thickness;
    slice[i].flags&=~FLAG_DEFAULT_SIZE;

    (this)->StampEverything();
    (this)->NotifyObservers( 0);
    (this)->SetModified();
}

/* enlarge chunk so it doesn't include only part of a taped cell */

void table::FindBoundary (Chunk  chunk)
{
    int r, c;
    int ok;

    if (table_debug)
	printf("table_FindBoundary(%s, )\n", (this)->Name());

    for (; ;) {
	ok = 1;
	for (r = max (0, chunk->TopRow);
		chunk->LeftCol >= 0 && r <= chunk->BotRow; r++) {
	    while ((this)->IsJoinedToLeft( r, chunk->LeftCol) && chunk->LeftCol > 0) {
		chunk->LeftCol--;
		ok = 0;
	    }
	    while ((this)->IsJoinedToLeft( r, chunk->RightCol + 1)  && chunk->RightCol < (this)->NumberOfColumns() - 1) {
		chunk->RightCol++;
		ok = 0;
	    }
	}
	for (c = max (0, chunk->LeftCol);
		chunk->TopRow >= 0 && c <= chunk->RightCol; c++) {
	    while ((this)->IsJoinedAbove( chunk->TopRow, c) && chunk->TopRow > 0) {
		chunk->TopRow--;
		ok = 0;
	    }
	    while ((this)->IsJoinedAbove( chunk->BotRow + 1, c) && chunk->BotRow < (this)->NumberOfRows() - 1) {
		chunk->BotRow++;
		ok = 0;
	    }
	}
	if (ok) break;
    }
}

void table::SetInterior (Chunk  chunk, Color  color)
{
    int r, c;

    if (table_debug)
	printf("table_SetInterior(%s, , %d)\n", (this)->Name(), (int)color);

    (this)->FindBoundary( chunk);
    this->cellChanged = (this)->StampEverything();

    for (r = chunk->TopRow + 1; r <= chunk->BotRow; r++) {
   	for (c = max(chunk->LeftCol, 0); c <= chunk->RightCol; c++) {
	    if (c > chunk->LeftCol && (this)->ColorToLeft( r, c) != color) {
		struct cell * cell = (this)->GetCell( r, c);
		if (cell->celltype != table_ValCell)
		    cell->lastcalc = this->cellChanged;
		else
		    cell->lastcalc = 0;
	    }
	    (this)->SetAboveColor( r, c, color);
	}
    }
    for (r = max(chunk->TopRow, 0); r <= chunk->BotRow; r++) {
   	for (c = chunk->LeftCol + 1; c <= chunk->RightCol; c++) {
	    if (c > chunk->LeftCol && (this)->ColorToLeft( r, c) != color) {
		struct cell * cell = (this)->GetCell( r, c);
		if (cell->celltype != table_ValCell)
		    cell->lastcalc = this->cellChanged;
		else
		    cell->lastcalc = 0;
	    }
	    (this)->SetLeftColor( r, c, color);
	}
    }
    (this)->NotifyObservers( 0);
    (this)->SetModified();
}


void table::SetBoundary (Chunk  chunk, Color  color)
{
    int r, c, topr, leftc;

    if (table_debug)
	printf("table_SetBoundary(%s, , %d)\n", (this)->Name(), (int)color);

    (this)->FindBoundary ( chunk);

    this->edgeChanged = this->cellChanged = ++(this->timeStamper);
    topr= max (chunk->TopRow, 0);
    leftc= max (chunk->LeftCol, 0);
    for (r = topr; r <= chunk->BotRow; r++) {
	(this)->SetLeftColor( r, leftc, color);
	(this)->SetLeftColor( r, chunk->RightCol+1, color);
    }
    for (c = leftc; c <= chunk->RightCol; c++) {
	(this)->SetAboveColor( topr, c, color);
	(this)->SetAboveColor( chunk->BotRow+1, c, color);
    }
    this->edgeChanged = ++(this->timeStamper);
    (this)->NotifyObservers( 0);
    (this)->SetModified();
}

void rangeLimit (class table  * T, Chunk  chunk)
{
    if (chunk->TopRow < 1) chunk->TopRow = 1;
    if (chunk->BotRow > (T)->NumberOfRows()) chunk->BotRow = (T)->NumberOfRows();
    if (chunk->LeftCol < 1) chunk->LeftCol = 1;
    if (chunk->RightCol > (T)->NumberOfColumns()) chunk->RightCol = (T)->NumberOfColumns();
}

/* re-evaluate one cell */

static const char circ[] = "CIRC!";

void table::ReEval (int      r , int      c)
{
    struct cell * cell = (this)->GetCell( r, c);

    if (cell->lastcalc >= this->cellChanged)
	return;
    if (cell->celltype != table_ValCell)
	return;
    cell->lastcalc = this->cellChanged;
    MakeBogus(&(cell->interior.ValCell.value), circ);
    eval (this, &(cell->interior.ValCell.value), r, c, cell->interior.ValCell.formula);
    cell->lastcalc = this->cellChanged;
    return;
}

void rcref (class table  * T, extended_double  *result, int      r , int      c , int      iftaped)
{
    struct cell * cell;

    r--, c--;
    if (r >= 0 && r < (T)->NumberOfRows() && c >= 0 && c < (T)->NumberOfColumns()) {
	for (;;) {
	    if ((T)->IsJoinedToLeft( r, c))
		c--;
	    else if ((T)->IsJoinedAbove( r, c))
		r--;
	    else
		break;
	    if (!iftaped) {
		MakeBogus(result, "CELL!");
		return;
	    }
	}
	cell = (T)->GetCell( r, c);
	
	switch(cell->celltype) {
	    case table_ValCell:
		(T)->ReEval ( r, c);
		if (IsBogus(&(cell->interior.ValCell.value)) && ExtractBogus(&(cell->interior.ValCell.value)) != circ) {
		    MakeBogus(result, "REF!");
		}
		else {
		    *result = cell->interior.ValCell.value;
		}
		break;
	    case table_EmptyCell:
		MakeStandard(result, (double)0.0);
		break;
	    case table_ImbeddedObject:
		{
		const class atom *rock1=atom::Intern("rock1");
		const class atom *string=atom::Intern("string");
		const class atom *value=atom::Intern("value");
		long rock;
		char *rockp;
		if((cell->interior.ImbeddedObject.data)->Get(value, &string,(long *)&rockp)) MakeStandard(result,atof(rockp));
		else
		    if( !(cell->interior.ImbeddedObject.data)->Get(  value, &rock1, &rock)) MakeBogus(result,"NOVAL!");
		    else MakeStandard(result,(double)rock);
		break;
		}
	    default:	
		MakeBogus(result, "NOVAL!");
	}
    }
    else {
	MakeBogus(result, "CELL!");
    }
}
#define inside(r,c,ch) ( (r)>=((ch)->TopRow) && (r)<=((ch)->BotRow) && (c)>=((ch)->LeftCol) && (c) <= ((ch)->RightCol) )

struct movetrstate {
    int myrow, mycol;
    Chunk left, moved;
};
/* 
static movetr (r, c, absr, absc, ms)
int  *r, *c, absr, absc;
struct movetrstate *ms;
{
    int     refr = (ms->myrow) * (!absr) + *r, refc = (ms->mycol) * (!absc) + *c;
    if (inside ((ms->myrow), (ms->mycol), (ms->moved)))
	if (inside (refr, refc, (ms->moved))) {
	    if (absr)
		(*r) += ms->moved->TopRow - ms->left->TopRow;
	    if (absc)
		(*c) += ms->moved->LeftCol - ms->left->LeftCol;
	}
	else {
	    if (!absr)
		(*r) -= (ms->moved->TopRow - ms->left->TopRow);
	    if (!absc)
		(*c) -= (ms->moved->LeftCol - ms->left->LeftCol);
	    if (inside ((ms->myrow) * (!absr) + *r, (ms->mycol) * (!absc) + *c, (ms->moved)))
		return 0;
	}
    else
	if (inside(refr, refc, (ms->left)))
	    (*r) += (ms->moved->TopRow - ms->left->TopRow),
	    (*c) += (ms->moved->LeftCol - ms->left->LeftCol);
	else
	    if (inside (refr, refc, (ms->moved)))
		return 0;
    return 1;
}
 */

void table::SetFormat (char  ch, Chunk  chunk)
{
    int     r, c;
    struct cell *cell;

    if (table_debug)
	printf("table_SetFormat(%s, %c)\n", (this)->Name(), ch);

    for (r = max (chunk->TopRow, 0); r <= chunk->BotRow; r++) {
	for (c = max (chunk->LeftCol, 0); c <= chunk->RightCol; c++) {
	    if (!(this)->IsJoinedToAnother( r, c)) {
		cell = (this)->GetCell( r, c);
		if (!(cell->lock)) {
		    cell->format = ch;
		}
	    }
	}
    }
    this->cellChanged = ++(this->timeStamper);
    (this)->NotifyObservers( 0);
    (this)->SetModified();
}

void table::SetPrecision (int  prec, Chunk  chunk)
{
    int     r, c;
    struct cell *cell;

    if (table_debug)
	printf("table_SetPrecision(%s, %d)\n", (this)->Name(), prec);

    for (r = max (chunk->TopRow, 0); r <= chunk->BotRow; r++) {
	for (c = max (chunk->LeftCol, 0); c <= chunk->RightCol; c++) {
	    if (!(this)->IsJoinedToAnother( r, c)) {
		cell = (this)->GetCell( r, c);
		if (!(cell->lock)) {
		    cell->precision = prec;
		}
	    }
	}
    }
    this->cellChanged = ++(this->timeStamper);
    (this)->NotifyObservers( 0);
    (this)->SetModified();
}

void table::Imbed (char  *name, Chunk  chunk)
{
    class dataobject *newobject;
    int r, c;
    struct cell *cell;

    if (table_debug)
	printf("table_Imbed(%s, %s, )\n", (this)->Name(), name);

    (this)->StampEverything();
    for (r = max (chunk->TopRow, 0); r <= chunk->BotRow; r++) {
	for (c = max (chunk->LeftCol, 0); c <= chunk->RightCol; c++) {
	    cell = (this)->GetCell( r, c);
	    if (!(this)->IsJoinedToAnother( r, c) && !(cell->lock)) {
		DestroyCell(this, cell);
		newobject = (class dataobject *) ATK::NewObject(name);
		if (newobject) {
		    cell->celltype = table_ImbeddedObject;
		    cell->interior.ImbeddedObject.data = newobject;
		    cell->interior.ImbeddedObject.views = NULL;
		    cell->lastcalc = this->cellChanged;
		    
		    (newobject)->AddObserver(this);
		}
	    }
	}
    }
    (this)->NotifyObservers( 0);
    (this)->SetModified();
}

void table::Lock (char  ch, Chunk  chunk)
{
    int     r,  c;

    if (table_debug)
	printf("table_Lock(%s, %c, )\n", (this)->Name(), ch);

    for (r = max (chunk->TopRow, 0); r <= chunk->BotRow; r++)
	for (c = max (chunk->LeftCol, 0); c <= chunk->RightCol; c++)
	    if (!(this)->IsJoinedToAnother( r, c))
		(this)->GetCell( r, c)->lock = ch;
    this->cellChanged = ++(this->timeStamper);
    (this)->NotifyObservers( 0);
    (this)->SetModified();
}

/* format date */

static const char   * const monthname[] = {
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};

extern NO_DLL_EXPORT const int daysinmonth[];

const int     daysinmonth[] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

void table::FormatDate (double   fdate, char    *buf, char     format)
{
    int     date = (int) (fdate + 0.5);
    int     y, m, d;

    if (date <= 0 || date > 73050)
	strcpy (buf, "DATE!");
    date = date - 1 - 59 + 1461;
    if (date < 1461)
	date--;
    y = date / 1461;
    date -= y * 1461;
    y *= 4;
    d = date / 365;
    if (d >= 4) d = 3;
    date -= d * 365;
    y += d + 1900 - 4;
    for (m = 2; m != 1 && date >= daysinmonth[m]; ) {
	date -= (daysinmonth[m++]);
	if (m >= 12) {
	    m = 0;
	    y += 1;
	}
    }
    d = date + 1;
    if (format == DDMMMYYYYFORMAT)
	sprintf (buf, "%2d %3.3s %d", d, monthname[m], y);
    else if (format == MMMYYYYFORMAT)
	sprintf (buf, "%3.3s %d", monthname[m], y);
    else if (format == DDMMMFORMAT)
	sprintf (buf, "%2d %3.3s", d, monthname[m]);
}

/* construct an extended floating value */

void MakeStandard(extended_double  *x, double  value)
{
    ExtendedType(x) = extended_STANDARD;
    StandardValue(x) = value;
}

/* construct a extended floating "bogus" value */

void MakeBogus(extended_double  *x, const char  *message)
{
    ExtendedType(x) = extended_BOGUS;
    ExtractBogus(x) = message;
}

/* check to see if modified */

long table::GetModified()
{
    int r, c;
    struct cell *cell;
    long rc, cc;

    rc = (this)->dataobject::GetModified();
    if (!this->inGetModified) {
	this->inGetModified = TRUE;
	for (r = 0; r < this->rows; r++) {
	    for (c = 0; c < this->cols; c++) {
		cell = (this)->GetCell( r, c);
		if (cell->celltype == table_ImbeddedObject && cell->interior.ImbeddedObject.data != NULL) {
		    cc = (cell->interior.ImbeddedObject.data)->GetModified();
		    if (cc > rc) rc = cc;
		}
	    }
	}
	this->inGetModified = FALSE;
    }

    if (table_debug)
	printf("table_GetModified = %ld\n", rc);

    return rc;
}
