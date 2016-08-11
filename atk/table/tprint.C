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
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/table/RCS/tprint.C,v 1.4 1996/09/03 19:16:14 robr Exp $";
#endif


 




#include <andrewos.h>
#include <table.H>
#include <texttroff.H>
#include <view.H>

#define AUXMODULE
#include <spread.H>

#include <shared.h>
#include <math.h>
/* get type of tab stop (Left, Right, Center, None) */

void spread_printVal(class table  * T, char *buf, extended_double  *value, char format, int prec);

static char TabType(class table  *T, int  r , int  c);
static int NeedNewTabs(class table  *T, int  r);
static int CellWidth(class table  *T, int  r , int  c, int  *nextc);
static void SetTabs(class table  *T, FILE  *f, int r);
static void PrintEdges(class table  *T, FILE  *f, int  r);
static void PrintChild(class table  *T, FILE  *f, int  r , int  c, class view  *child, char  *processor, char  *format, int  linemacro	/* numeric name of macro to restore line widths */);
static void PrintRow(class spread  *V, FILE  *f, int  r, char  *processor, char  *format, int  linemacro	/* numeric name of macro to restore line widths */);
void WriteTroff(class spread  * V, FILE  * f, char  *processor, char  *format, boolean toplevel);


static char TabType(class table  *T, int  r , int  c)
{
    struct cell *cell = (T)->GetCell( r, c);

    if ((T)->IsJoinedToAnother( r, c))
	return 'N';
    switch (cell->celltype) {
	case table_TextCell:
	    switch(*(cell->interior.TextCell.textstring)) {
		case '\"':
		    return 'R';
		case '^':
		    return 'C';
		case '\'':
		default:
		    return 'L';
	    }
	case table_ImbeddedObject:
	    return 'L';
	default:
	    return 'R';
    }
}

/* determine if a new set of tabs are needed */

static int NeedNewTabs(class table  *T, int  r)
{
    int c;

    if (r <= 0)
	return 1;
    for (c = 0; c < (T)->NumberOfColumns(); c++) {
	if (TabType(T, r, c) != TabType(T, r-1, c))
	    return 1;
    }
    return 0;
}

/* get end of (combined) cell */

static int CellWidth(class table  *T, int  r , int  c, int  *nextc)
{
    int k;
    int x;

    for (k = c, x = 0; k < (T)->NumberOfColumns(); ) {
	x += (T)->ColumnWidth( k++) + spread_SPACING;
	if (TabType(T, r, k) != 'N')
	    break;
    }
    *nextc = k;
    return x;
}

/* set tab stops */

static void SetTabs(class table  *T, FILE  *f, int r)
{
    int c;
    int x;
    int prevtab;
    int nexttab;
    char *prefix;
    int c1;
    int x1;

    fprintf(f, ".ta");
    prefix = "";
    prevtab = 0;
    for (c = 0, x = 0; c < (T)->NumberOfColumns(); c = c1, x = x1) {
	x1 = x + CellWidth(T, r, c, &c1);
	switch (TabType(T, r, c)) {
	    case 'L':
		nexttab = x + spread_SPACING + spread_CELLMARGIN;
		fprintf(f, " %s%dp", prefix, nexttab - prevtab);
		prevtab = nexttab;
		prefix = "+";
		break;
	    case 'R':
		nexttab = x1 - spread_CELLMARGIN;
		fprintf(f, " %s%dpR", prefix, nexttab - prevtab);
		prevtab = nexttab;
		prefix = "+";
		break;
	    case 'C':
		nexttab = (x + spread_SPACING + x1) / 2;
		fprintf(f, " %s%dpC", prefix, nexttab - prevtab);
		prevtab = nexttab;
		prefix = "+";
		break;
	}
    }
    fprintf(f, "\n");
}

/* print edges to left of and above cell */

/* expects top of cell (contents) in diversion mark */
/* scratches in number register 34 (height of current row) */

static void PrintEdges(class table  *T, FILE  *f, int  r)
{
    int c;
    int x;
    int currx, startx;
    int t = (spread_SPACING - spread_LINEWIDTH)/2;
    int k;
    int z;

    /* loop over cells in this row */

    for (c = 0, x = 0, currx = 0; c <= (T)->NumberOfColumns(); x += (T)->ColumnWidth( c++) + spread_SPACING) {

	/* draw vertical line to the left of above cell */

	if (r > 0) {
	    if ((T)->ColorToLeft( r-1, c) == BLACK) {
		startx = x + t;
		if (currx != startx) {
		    fprintf(f, "\\h'%dp'", startx - currx);
		    currx = startx;
		}
		fprintf(f, "\\D'l 0 -\\n(.hu-1v+\\n(32u-3p'\\v'\\n(.hu+1v-\\n(32u+3p'");
	    }
	}

	/* draw the max length horizontal line beginning above this cell */

	if (c < (T)->NumberOfColumns() && (T)->ColorAbove( r, c) == BLACK && !(T)->IsJoinedToLeft( r, c) && (c <= 0 || (T)->ColorAbove( r, c-1) != BLACK)) {
	    startx = x + t;
	    if (currx != startx) {
		fprintf(f, "\\h'%dp'", startx - currx);
		currx = startx;
	    }
	    for (k = c, z = 0; k < (T)->NumberOfColumns() && (T)->ColorAbove( r, k) == BLACK; z += (T)->ColumnWidth( k++) + spread_SPACING) /* continue horizontal */;
	    fprintf(f, "\\D'l %dp 0'", z);
	    currx += z;
	}
    }
    fprintf(f, "\n");
}

/* generate the value of a cell as a string */

void spread_printVal(class table  * T, char *buf, extended_double  *value, char format, int prec)
{
    extended_double newvalue;
    int decpt, sign;

    if (IsBogus(value)) {
	strcpy(buf, ExtractBogus(value));
	return;
    }
    if (isnan(StandardValue(value))) {
	strcpy(buf, "ARITH!");
	return;
    }

    switch (format) {
	case PERCENTFORMAT:
	    StandardValue(&newvalue) = StandardValue(value) * 100.0;
	    ExtendedType(&newvalue) = extended_STANDARD;
	    spread_printVal(T, buf, &newvalue, GENERALFORMAT, prec);
	    strcat(buf, "%");
	    break;
	case CURRENCYFORMAT:
	    buf[0] = '$';
	    spread_printVal(T, buf+1, value, FIXEDFORMAT, prec);
	    break;
	case DDMMMYYYYFORMAT: 
	case MMMYYYYFORMAT: 
	case DDMMMFORMAT: 
	    (T)->FormatDate( StandardValue(value), buf, format);
	    break;
	default:
	    {
		char *p = buf;

		sprintf(buf, "%.*f", prec, StandardValue(value));
		while (*p)
		    p++;
		p--;
		if (prec > 0 && format == 'G') {
		    while (p >= buf && *p == '0')
			*p-- = ' ';
		}
		if (p >= buf && *p == '.')
		    *p-- = ' ';
		break;
	    }
    }
}

/* print imbedded object */

/* uses macros 40... to stack current status */
/* assumes .rt will return to beginning of row */

static void PrintChild(class table  *T, FILE  *f, int  r , int  c, class view  *child, char  *processor, char  *format, int  linemacro	/* numeric name of macro to restore line widths */)
{
    int k;
    int x;
    int width;
    int dummy;

    for (k = 0, x = 0; k < c; )
	x += (T)->ColumnWidth( k++) + spread_SPACING;
    width = CellWidth(T, r, c, &dummy);

    fprintf(f, ".in \\n(.iu+%dp\n", x + spread_SPACING + spread_CELLMARGIN);
    fprintf(f, ".ll \\n(.iu+%dp\n", width - spread_SPACING - spread_CELLMARGIN * 2);
    fprintf(f, ".fi\n");
    if (child)
	(child)->Print( f, processor, format, 0);
    fprintf(f, ".nf\n");

    fprintf(f, ".%d\n",	linemacro); /* restore line bounds */
    SetTabs(T, f, r);		    /* restore tab stops */
    fprintf(f, ".rt\n");	    /* return to top of cell */
}

/* print one row */

static void PrintRow(class spread  *V, FILE  *f, int  r, char  *processor, char  *format, int  linemacro	/* numeric name of macro to restore line widths */)
{
    class table *T = MyTable(V);
    int c;
    struct cell *cell;
    int k;
    int hadchild;
    int ch,didprint;
    char buf[1030];

    hadchild = didprint = 0;
    fprintf(f, ".mk\n");
    for (c = 0; c < (T)->NumberOfColumns(); c++) {
	if (TabType(T, r, c) == 'N')
	    continue;
	didprint++;
	cell = (T)->GetCell( r, c);
	fprintf(f, "\t");
	switch (cell->celltype) {
	    case table_TextCell:
		ch = *cell->interior.TextCell.textstring;
		fputs (cell->interior.TextCell.textstring + (ch == '\'' || ch == '\"' || ch == '^'), f);
		break;
	    case table_ValCell:
		(T)->ReEval( r, c);
		spread_printVal(T, buf, &(cell->interior.ValCell.value), cell->format, cell->precision);
		fputs(buf, f);
		break;
	    case table_ImbeddedObject:
		fprintf(f, "\n.sp -1v\n");	/* get back to beginning of line */
		PrintChild(T, f, r, c, spread_FindSubview(V, cell), processor, format, linemacro);
		for (k = 0; k <= c; k++)	/* tab to column again */
		    if (TabType(T, r, c) != 'N')
			fprintf(f, "\t");
		hadchild = 1;
		break;
	}
    }
    if(didprint == 0) fprintf(f,"\\&");
    fprintf(f, "\n");
    if (hadchild)
	fprintf(f, ".sp |\\n(.hu\n");
}

/* write out printable representation of a table */

/* saves parent state in macros 30, 31, ... */
/* sets line bounds in macros 40, 41, ... */
/* diverts rows to macros 50, 51, ... */
/* number register 31 = trash */
/* number register 32 = top to baseline distance of digits */

void WriteTroff(class spread  * V, FILE  * f, char  *processor, char  *format, boolean toplevel)
{
    register class table *T = MyTable(V);
    int r;
    static int saveno = 30;

    /* set up  top-level stuff */

    if (ATK::LoadClass("texttroff") == NULL)
	printf("Can't load texttroff - document initializtion missing.\n");
    if (toplevel && ATK::IsLoaded("texttroff"))
	texttroff::BeginDoc(f);

    fprintf(f, "\\\"table begins\n");
    
    /* save state in macro 30, 31, ... */

    fprintf(f, ".de %d\n", saveno++);	/* macro to save & restore state */
    fprintf(f, ".if \\n(.u .fi\n");	/* filling */
    fprintf(f, ".if \\n(.j .ad\n");	/* adjusting */
    fprintf(f, ".if \\n(.j=0 .na\n");
    fprintf(f, "..\n");

    /* set up my state */

    fprintf(f, ".nf\n");		/* no filling */
    fprintf(f, ".br\n");
    fprintf(f, ".nr 31 \\w'0123456789.jJ'\n");
    fprintf(f, ".nr 32 \\n(st\n");	/* \n(32u = top to baseline */

    /* save line bounds in macro 40, 41, ... */

    fprintf(f, ".de %d\n", saveno+9);	/* stack line bounds in macro */
    fprintf(f, ".ft \\n(.f\n");		/* font */
    fprintf(f, ".ps \\n(.s\n");		/* point size */
    fprintf(f, ".ll \\n(.lu\n");	/* line length */
    fprintf(f, ".in \\n(.iu\n");	/* indentation */
    fprintf(f, "..\n");

    /* format the table a row at a time */

    fprintf(f, ".di %d\n", saveno+19);
    for (r = 0; ; r++) {
	PrintEdges(T, f, r);
	fprintf(f, ".di\n");
	fprintf(f, ".if \\n(dn>=\\n(.t .bp\n");
	fprintf(f, ".in 0\n");
	fprintf(f, ".%d\n", saveno+19);
	fprintf(f, ".%d\n", saveno+9);

	if (r >= (T)->NumberOfRows())
	    break;

	if (NeedNewTabs(T, r))
	    SetTabs(T, f, r);
	fprintf(f, ".di %d\n", saveno+19);
	PrintRow(V, f, r, processor, format, saveno+9);
	fprintf(f, ".sp -\\n(32u+3p\n");
   }
    fprintf(f, ".sp 1v\n");

    fprintf(f, ".%d\n",	--saveno);	/* restore state */
    fprintf(f, "\\\"table ends\n");

    if (toplevel && ATK::IsLoaded("texttroff"))
	texttroff::EndDoc(f);
}
