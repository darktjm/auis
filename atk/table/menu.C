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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/table/RCS/menu.C,v 1.5 1994/08/12 18:54:11 rr2b Stab74 $";
#endif


 

/* menu.c - menu operations for table */




#include <andrewos.h>
#include <im.H>
#include <view.H>
#include <dataobject.H>
#include <bind.H>
#include <table.H>

#define AUXMODULE
#include <spread.H>

#include <shared.h>

#ifndef NORCSID
#endif
void m_rename(register class spread  * V, char  ch);
static char *newext(char  *filename			/* "mumble.y" or something */, char  *extension		/* ".x" */);
void m_writeTroff(register class spread  * V, char  ch);
void m_write(register class spread  * V, char  ch);
void m_read(register class spread  * V, char  ch);
void m_cut(register class spread  * V, char  ch);
void m_copy(register class spread  * V, char  ch);
void m_paste(register class spread  * V, char  ch);
void m_combine(register class spread  * V, char  ch);
void m_separate(register class spread  * V, char  ch);
void m_drawedges(register class spread  * V, char  ch);
void m_eraseedges(register class spread  * V, char  ch);
void m_format (register class spread  * V, char     ch);
void m_precision (register class spread  * V, char     ch);
static objecttest(register class spread  *V, char  *name, char  *desiredname);
void m_imbed (register class spread  * V, char     ch);
void m_resetheights (register class spread  * V, char     ch);
void m_lock (register class spread  * V, char     ch);
void DefineMenus (class menulist  *mainmenus, class keymap  * mainmap, class menulist *readonlymenus, class keymap *readonlymap, struct ATKregistryEntry   *classinfo);


void m_rename(register class spread  * V, char  ch)
{
    char buff[257];

    if (!k_AskUser (V, "New name?:  ", (MyTable(V))->Name(), buff, sizeof buff))
	return;
    if (*buff && strcmp(buff, "(null)") != 0) {
	class table * S = (MyTable(V))->SetName ( buff);
	if (S != MyTable(V)) {
	    if (!k_WantToDiscard (V))
		return;
	    /* MyTable(V) = S; */
	    V->lastTime = -1;
	}
	/* im_SetTitle ((V)->GetIM(), safetn); */
    }
}

static char *newext(char  *filename			/* "mumble.y" or something */, char  *extension		/* ".x" */)	/* returns "mumble.x" */
{
    char *cp, *suffixp;
    static char newname[257];
    char *safefn;

    safefn = (filename == NULL ? "" : filename);
    strncpy(newname, safefn, (sizeof newname) - 1);
    newname[(sizeof newname) - 1] = '\0';
    for (cp = newname, suffixp = NULL; *cp; cp++)
	switch (*cp) {
	case '.':
	    suffixp = cp;
	    break;
	case '/':
	    suffixp = NULL;
	    break;
	}
    if (suffixp)
	cp = suffixp;
    strncpy(cp, extension, (sizeof newname) - 1 - (cp - newname));
    return newname;
}

void m_writeTroff(register class spread  * V, char  ch)
{
    char buf[300];
    char fname[300];
    FILE * f;

    k_AskUser(V, "Enter file name: ",  newext ((MyTable(V))->Name(), ".trf"), fname, 300);
    f = fopen (fname, "w");
    if (f) {
	WriteTroff(V, f, "troff", "PostScript", 1);
	fclose (f);
	sprintf(buf, "Wrote %s", fname);
	k_TellUser(V, buf);
    }
}

void m_write(register class spread  * V, char  ch)
{
    char buf[300];
    char fname[300];
    FILE * f;
    struct chunk chunk;

    k_AskUser(V, "Enter file name: ", newext ((MyTable(V))->Name(), ".table"), fname, 300);
    f = fopen (fname, "w");
    if (f) {
	chunk.LeftCol = -1;
	chunk.RightCol = (MyTable(V))->NumberOfColumns() - 1;
	chunk.TopRow = -1;
	chunk.BotRow = (MyTable(V))->NumberOfRows() - 1;
	(MyTable(V))->WriteASCII ( f, &chunk);
	fclose (f);
	sprintf(buf, "Wrote %s", fname);
	k_TellUser(V, buf);
    }
}

void m_read(register class spread  * V, char  ch)
{
    char buf[300];
    char fname[300];
    FILE * f;

    k_AskUser(V, "Enter file name: ", newext ((MyTable(V))->Name(), ".table"), fname, 300);
    f = fopen (fname, "r");
    if (f) {
	(MyTable(V))->ReadASCII ( f);
	fclose (f);
	sprintf(buf, "Read %s", fname);
	k_TellUser(V, buf);
    }
}

void m_cut(register class spread  * V, char  ch)
{
    if (V->selection.TopRow > V->selection.BotRow || V->selection.LeftCol > V->selection.RightCol)
	k_TellUser (V, "Please select region to cut");
    else {
	FILE * writeFile = ((V)->GetIM())->ToCutBuffer();
	
	class table *T;
	
	/* write to cutbuffer */

	(MyTable(V))->WriteASCII( writeFile, &(V->selection));
	((V)->GetIM())->CloseToCutBuffer( writeFile);

	T=(MyTable(V))->ExtractData( &(V->selection));
	(T)->AddObserver(V);
	(T)->Destroy();

	/* delete entire rows */

	if (V->selection.LeftCol < 0 && V->selection.RightCol == (MyTable(V))->NumberOfColumns() - 1) {
	    struct chunk remnant;
	    int n0 = V->selection.TopRow;
	    int n1 = V->selection.BotRow + 1;
	    int n2 = (MyTable(V))->NumberOfRows();
	    if (n0 < 0) n0 = 0;
	    if (n1 < 0) n1 = 0;
	    remnant.LeftCol = -1;
	    remnant.RightCol = (MyTable(V))->NumberOfColumns() - 1;
	    if (n0 < n1 && n1 < n2) {
		class table * Q;
		remnant.TopRow = n1;
		remnant.BotRow = n2 - 1;
		Q = (MyTable(V))->ExtractData ( &remnant);
		
		(Q)->AddObserver(V);
		
		remnant.TopRow -= (n1 - n0);
		remnant.BotRow -= (n1 - n0);
		(MyTable(V))->InsertData ( Q, &remnant);
		(Q)->Destroy();
	    }
	    (MyTable(V))->ChangeSize( (MyTable(V))->NumberOfRows() - (n1 - n0), (MyTable(V))->NumberOfColumns());
	    if ((MyTable(V))->NumberOfRows() <= 0)
		(MyTable(V))->ChangeSize( n0 = 10, (MyTable(V))->NumberOfColumns());
	    remnant.TopRow = V->selection.TopRow;
	    remnant.BotRow = n0 - 1;
	    SetCurrentCell (V, &remnant);
	}

	/* delete entire columns */

	if (V->selection.TopRow < 0 && V->selection.BotRow == (MyTable(V))->NumberOfRows() - 1) {
	    struct chunk remnant;
	    int n0 = V->selection.LeftCol;
	    int n1 = V->selection.RightCol + 1;
	    int n2 = (MyTable(V))->NumberOfColumns();
	    if (n0 < 0) n0 = 0;
	    if (n1 < 0) n1 = 0;
	    remnant.TopRow = -1;
	    remnant.BotRow = (MyTable(V))->NumberOfRows() - 1;
	    if (n0 < n1 && n1 < n2) {
		class table * Q;
		remnant.LeftCol = n1;
		remnant.RightCol = n2 - 1;
		Q = (MyTable(V))->ExtractData ( &remnant);
		
		(Q)->AddObserver(V);
		
		remnant.LeftCol -= (n1 - n0);
		remnant.RightCol -= (n1 - n0);
		(MyTable(V))->InsertData ( Q, &remnant);
		(Q)->Destroy();
	    }
	    (MyTable(V))->ChangeSize( (MyTable(V))->NumberOfRows(), (MyTable(V))->NumberOfColumns() - (n1 - n0));
	    if ((MyTable(V))->NumberOfColumns() <= 0)
		(MyTable(V))->ChangeSize( (MyTable(V))->NumberOfRows(), n0 = 5);
	    remnant.LeftCol = V->selection.LeftCol;
	    remnant.RightCol = n0 - 1;
	    SetCurrentCell (V, &remnant);
	}
    }
    (V->parent)->WantNewSize( V);
}

void m_copy(register class spread  * V, char  ch)
{
    if (V->selection.TopRow > V->selection.BotRow || V->selection.LeftCol > V->selection.RightCol)
	k_TellUser (V, "Please select region to copy");
    else {
	FILE * writeFile = ((V)->GetIM())->ToCutBuffer();

	(MyTable(V))->WriteASCII( writeFile, &(V->selection));
	((V)->GetIM())->CloseToCutBuffer( writeFile);
    }
}

void m_paste(register class spread  * V, char  ch)
{
    if ((V->selection.TopRow >= 0 || V->selection.BotRow < (MyTable(V))->NumberOfRows()-1) && (V->selection.LeftCol >= 0 || V->selection.RightCol < (MyTable(V))->NumberOfColumns()-1) && (V->selection.TopRow > V->selection.BotRow || V->selection.LeftCol > V->selection.RightCol))
	k_TellUser (V, "Please select region to paste into");
    else {
	class table * S;
	FILE * readFile = ((V)->GetIM())->FromCutBuffer();

	/* read from cutbuffer */

	S = new table;
	(S)->ReadASCII( readFile);
	((V)->GetIM())->CloseFromCutBuffer( readFile);

	/* insert entire rows */

	if (V->selection.LeftCol < 0 && V->selection.RightCol == (MyTable(V))->NumberOfColumns() - 1) {
	    struct chunk newselection;

	    AddRows(V, V->selection.TopRow, (S)->NumberOfRows());
	    newselection.LeftCol = -1;
	    newselection.RightCol = (MyTable(V))->NumberOfColumns() - 1;
	    newselection.TopRow = V->selection.TopRow;
	    newselection.BotRow = V->selection.TopRow + (S)->NumberOfRows() - 1;
	    SetCurrentCell (V, &newselection);
	    }

	/* insert entire columns */

	if (V->selection.TopRow < 0 && V->selection.BotRow == (MyTable(V))->NumberOfRows() - 1) {
	    struct chunk newselection;

	    AddCols(V, V->selection.LeftCol, (S)->NumberOfColumns());
	    newselection.TopRow = -1;
	    newselection.BotRow = (MyTable(V))->NumberOfRows() - 1;
	    newselection.LeftCol = V->selection.LeftCol;
	    newselection.RightCol = V->selection.LeftCol + (S)->NumberOfColumns() - 1;
	    SetCurrentCell (V, &newselection);
	}

	/* paste in data read */

	(MyTable(V))->InsertData ( S, &(V->selection));
	(S)->Destroy(); 
    }
    (V->parent)->WantNewSize( V);
}

void m_combine(register class spread  * V, char  ch)
{
    struct chunk chunk;

    if (max(0, V->selection.TopRow) > V->selection.BotRow || max(0, V->selection.LeftCol) > V->selection.RightCol)
	k_TellUser(V, "Please select a region to combine");
    else {
	(MyTable(V))->SetInterior ( &(V->selection), JOINED);
	CopyChunk (&chunk, &(V->selection));
	SetCurrentCell (V, &chunk);
    }
}

void m_separate(register class spread  * V, char  ch)
{
    struct chunk chunk;

    if (max(0, V->selection.TopRow) > V->selection.BotRow || max(0, V->selection.LeftCol) > V->selection.RightCol)
	k_TellUser(V, "Please select a region to separate");
    else {
	(MyTable(V))->SetInterior ( &(V->selection), GHOST);
	CopyChunk (&chunk, &(V->selection));
	SetCurrentCell (V, &chunk);
    }
}

void m_drawedges(register class spread  * V, char  ch)
{
    if (max(0, V->selection.TopRow) > V->selection.BotRow && max(0, V->selection.LeftCol) > V->selection.RightCol)
	k_TellUser(V, "Please select a region to draw edges");
    else
	(MyTable(V))->SetBoundary ( &(V->selection), BLACK);
}

void m_eraseedges(register class spread  * V, char  ch)
{
    if (max(0, V->selection.TopRow) > V->selection.BotRow && max(0, V->selection.LeftCol) > V->selection.RightCol)
	k_TellUser(V, "Please select a region to erase edges");
    else
	(MyTable(V))->SetBoundary ( &(V->selection), GHOST);
}

/* process formatting menu hit */

void m_format (register class spread  * V, char     ch)
{
    (MyTable(V))->SetFormat ( ch, &(V->selection));
}

/* process precision request */

void m_precision (register class spread  * V, char     ch)
{
    char parambuff[100];
    int param;
    
    if (!k_AskUser(V, "Enter precision: ", "", parambuff, sizeof parambuff))
	return;

    if (sscanf(parambuff, "%d", &param) == 1)
	(MyTable(V))->SetPrecision ( (param >= 0 ? param : 0), &(V->selection));
}

static objecttest(register class spread  *V, char  *name, char  *desiredname)
{
    if(ATK::LoadClass(name) == NULL){
        char foo[640];
        sprintf(foo,"Can't load %s",name);
         k_TellUser(V, foo);
        return(FALSE);
    }
    if(! ATK::IsTypeByName(name, desiredname)){
        char foo[640];
        sprintf(foo,"%s is not a %s",name,desiredname);
         k_TellUser(V, foo);
        return(FALSE);
    }
    return(TRUE);
}

/* process request for imbedded object */

void m_imbed (register class spread  * V, char     ch)
{
    char parambuff[100];
    struct cell * hitcell;
    class view * child;
    
    parambuff[0] = '\0';
    if (!k_AskUser(V, "Data object to enter here (text): ", "", parambuff, sizeof parambuff))
	return;
    if (!*parambuff)
	strcpy(parambuff, "text");
    if (objecttest(V, parambuff, "dataobject") == FALSE)
	return;

    (MyTable(V))->Imbed( parambuff, &(V->selection));

    if (V->anchor.TopRow >= 0 && V->anchor.LeftCol >= 0) {
	hitcell = (MyTable(V))->GetCell( V->anchor.TopRow, V->anchor.LeftCol);
	if (hitcell->celltype == table_ImbeddedObject) {
	    if ((child = spread_FindSubview(V, hitcell)))
		(child)->WantInputFocus( child);
	}
    }
}

/* compute row heights automatically */

void m_resetheights (register class spread  * V, char     ch)
{
    int r;
    class table *T = MyTable(V);
    
    for (r = max (V->selection.TopRow, 0); r <= V->selection.BotRow; r++) {
	(T)->ChangeThickness( ROWS, r, 0);
    }
}

/* lock or unlock cells */

void m_lock (register class spread  * V, char     ch)
{
    (MyTable(V))->Lock ( ch, &(V->selection));
}

static struct bind_Description menutable[] = {

    {"table-cut", "\027", 0, "Cut~10", 0, 0, (proctable_fptr)m_cut, "Save and erase cells"},
    {"table-copy", "\033w", 0, "Copy~11", 0, 0, (proctable_fptr)m_copy, "Copy cells to cutbuffer"},
    {"table-paste", "\031", 0, "Paste~12", 0, 0, (proctable_fptr)m_paste, "Copy cutbuffer to cells"},
    {"table-write", "/fw", 0, "Write table~80", 0, 0, (proctable_fptr)m_write, "Write table file"},
    {"table-read", "/fr", 0, "Read table~81", 0, 0, (proctable_fptr)m_read, "Read table file"},
    {"table-writeTroff", 0, 0, "Write Troff~82", 0, 0, (proctable_fptr)m_writeTroff, "Write .trf file"},
    {"table-rename", 0, 0, "Rename~84", 0, 0, (proctable_fptr)m_rename, "Rename table"},

    {"table-combine", "/rc", 0, "Cells~1,Combine~10", 0, 0, (proctable_fptr)m_combine, "Combine several cells into one",0},
    {"table-separate", "/rs", 0, "Cells~1,Separate~11", 0, 0, (proctable_fptr)m_separate, "Separate combined cells"},
    {"table-drawedges", "/rbd", 0, "Cells~1,Draw box~20", 0, 0, (proctable_fptr)m_drawedges, "Draw box around cells"},
    {"table-eraseedges", "/rbe", 0, "Cells~1,Erase box~21", 0, 0, (proctable_fptr)m_eraseedges, "Erase box around cells"},
    {"table-lock", "/rl", TRUE, "Cells~1,Lock~30", TRUE, 0, (proctable_fptr)m_lock, "Protect cells against modification"},
    {"table-unlock", "/ru", FALSE, "Cells~1,Unlock~31", FALSE, 0, (proctable_fptr)m_lock, "Allow cells to be modified"},
    {"table-imbed", "\033\t", 0, "Cells~1,Imbed~40", 0, 0, (proctable_fptr)m_imbed, "Place BE2 object in cell"},
    {"table-reset-height", 0, 0, "Cells~1,Reset Heights~41", 0, 0, (proctable_fptr)m_resetheights, "Comput row heights automatically"},

    {"table-general-format", "/rfg", GENERALFORMAT, "Number Format~2,general~10", GENERALFORMAT, 0, (proctable_fptr)m_format, "general number format"},
    {"table-currency-format", "/rfc", CURRENCYFORMAT, "Number Format~2,Dollar~11", CURRENCYFORMAT, 0, (proctable_fptr)m_format, "Dollar sign before number"},
    {"table-percent-format", "/rfp", PERCENTFORMAT, "Number Format~2,Percent~12", PERCENTFORMAT, 0, (proctable_fptr)m_format, "Multiply by 100 and display %"},
    {"table-exponential-format", "/rfe", EXPFORMAT, "Number Format~2,Exp~13", EXPFORMAT, 0, (proctable_fptr)m_format, "Exponential format (not implemented)"},
    {"table-fixed-format", "/rff", FIXEDFORMAT, "Number Format~2,Fixed~14", FIXEDFORMAT, 0, (proctable_fptr)m_format, "Always display decimal places"},
    {"table-hbar-format", "/rfh", HORIZONTALBARFORMAT, "Number Format~2,H-Bar~15", HORIZONTALBARFORMAT, 0, (proctable_fptr)m_format, "Display as horizontal bar"},
    {"table-vbar-format", "/rfv", VERTICALBARFORMAT, "Number Format~2,V-Bar~16", VERTICALBARFORMAT, 0, (proctable_fptr)m_format, "Display as vertical bar"},
    {"table-precision", "/rp", 0, "Number Format~2,Precision~20", 0, 0, (proctable_fptr)m_precision, "Set number of decimal places"},

    {"table-day-month-year", "/rfda", DDMMMYYYYFORMAT, "Date Format~3,19 Jun 1970~10", DDMMMYYYYFORMAT, 0, (proctable_fptr)m_format, "Display day,  month, and year"},
    {"table-month-year", "/rfdb", MMMYYYYFORMAT, "Date Format~3,Jun 1970~11", MMMYYYYFORMAT, 0, (proctable_fptr)m_format, "Display month and year"},
    {"table-day-month", "/rfdc", DDMMMFORMAT, "Date Format~3,19 Jun~12", DDMMMFORMAT, 0, (proctable_fptr)m_format, "Display day and month"},

    {0, 0, 0, 0, 0, 0, 0}
};
static struct bind_Description menutable_readonly[] = {

    {"table-copy", "\033w", 0, "Copy~11", 0, 0, (proctable_fptr)m_copy, "Copy cells to cutbuffer"},
    {"table-write", "/fw", 0, "Write table~80", 0, 0, (proctable_fptr)m_write, "Write table file"},
     {"table-writeTroff", NULL, 0, "Write Troff~82", 0, 0, (proctable_fptr)m_writeTroff, "Write .trf file"},
     {0, 0, 0, 0, 0, 0, 0}
};

void DefineMenus (class menulist  *mainmenus, class keymap  * mainmap, class menulist *readonlymenus, class keymap *readonlymap, struct ATKregistryEntry   *classinfo)
{
    bind::BindList(menutable, mainmap, mainmenus, classinfo);
    bind::BindList(menutable_readonly, readonlymap, readonlymenus, classinfo);
}