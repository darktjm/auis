/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
	Copyright Carnegie Mellon University 1993 - All Rights Reserved
\* ********************************************************************** */

/*
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

/* nessmarktest.c
	test the nessmark object

	reads stdin, expands the tabs to spaces, and writes it out to stdout
	tab positions are assumed to be 9, 17, 25, 33, . . .

*/

/*
 * $Log: nessmrkt.C,v $
 * Revision 1.3  1994/08/12  20:43:22  rr2b
 * The great gcc-2.6.0 new fiasco, new class foo -> new foo
 *
 * Revision 1.2  1993/06/29  18:06:33  wjh
 * checkin to force update
 *
 * Revision 1.1  1993/06/21  20:43:31  wjh
 * Initial revision
 *
 * Revision 1.6  1992/12/15  21:38:20  rr2b
 * more disclaimerization fixing
 *
 * Revision 1.5  1992/12/15  00:50:53  rr2b
 * fixed disclaimerization
 *
 * Revision 1.4  1992/12/14  20:49:20  rr2b
 * disclaimerization
 *
 * Revision 1.3  1992/11/26  02:39:52  wjh
 * converted CorrectGetChar to GetUnsignedChar
 * moved ExtendShortSign to interp.h
 * remove xgetchar.h; use simpletext_GetUnsignedChar
 * nessrun timing messages go to stderr
 * replaced curNess with curComp
 * replaced genPush/genPop with struct compilation
 * created compile.c to handle compilation
 * moved scope routines to compile.c
 * converted from lex to tlex
 * convert to use lexan_ParseNumber
 * truncated logs to 1992 only
 * use bison and gentlex instead of yacc and lexdef/lex
 *
 * .
 *
 * Revision 1.2  91/09/12  16:26:21  bobg
 * Update copyright notice and rcsid
 * 
 * Revision 1.1  1989/06/01  15:39:58  wjh
 * Initial revision
 *
 * Creation 0.0  88/03/25 11:02:00  wjh
 * Initial creation by WJHansen
 * 
*/

#include <andrewos.h>

#include <simpletext.H>
#include <mark.H>

#if 0
/* include the ones utilized, but not by this .c file itself */
#define class_StaticEntriesOnly
#	include <observe.ih>
#	include <proctbl.ih>
#	include <dataobj.ih>
#undef class_StaticEntriesOnly
#endif

#include <nessmark.H>

static long writeid = 1;


int main(register int	   argc, register char   **argv);
void printdata(register class nessmark  *m);
void replacetabs(class nessmark  *file);


	int
main(register int	   argc, register char   **argv) {
	register class nessmark *m;
	register class simpletext *t;

	printf("Start\n"); fflush(stdout);
/*
	ATK_Init(".");
*/
	printf("Init done\n"); fflush(stdout);

/*
	observable_StaticEntry;
	proctable_StaticEntry;
	dataobject_StaticEntry;
	simpletext_StaticEntry;
	mark_StaticEntry;
*/
/* note that nessmark itself is dynamically loaded 
so nessmarktest need not be recompiled when nessmark is*/

	printf("About to New\n"); fflush(stdout);
	m = new nessmark;

	delete m;

	printf("\n Reading stdin\n");  fflush(stdout);
	m = new nessmark;
	t = new simpletext;
	(t)->SetAttributes( NULL);   /* XXX needed to set pendingReadOnly 
					to FALSE !!! */
	(t)->Read( stdin, 0);

	(m)->Set( t, 0, (t)->GetLength());
	printdata(m);
	replacetabs(m);
	printdata(m);

	(t)->Write( stdout, ++writeid, 0);
	return 0;
}

	void
printdata(register class nessmark  *m) {
	long loc, lend;
	lend = ((class mark *)m)->GetEndPos();
	if (lend > 31) lend = 31;
	printf("(%d,%d) ", ((class mark *)m)->GetPos(), 
			((class mark *)m)->GetLength());
	for (loc = ((class mark *)m)->GetPos(); loc < lend; loc++)
		putchar(((m)->GetText())->GetChar( loc));
	if (lend == 31 && ((class mark *)m)->GetLength() > 31)
		printf(" . . .");
	putchar('\n');
	fflush(stdout);
}

/*
 * function ReplaceTabs(m) == {
 *	marker f, tab, eight;
 *	eight := "        ";  -- 8 spaces
 *	tab := eight;		-- initial distance to tab
 *	while m /= "" do {
 *		f := first(m);
 *		m := rest(m);
 *		if f = "\t" then {
 *			-- replace tab with spaces 
 *			replace (f, tab);
 *			tab := eight;
 *		}
 *		else if  f = "\n"  or  tab = " "  then
 *			-- newline or single space for this tab,
 *			--	start next tab
 *			tab := eight;
 *		else
 *			-- non-tab: shorten distance to tab stop
 *			tab := rest(tab);
 *	}
 * }
 */

	void 
replacetabs(class nessmark  *file) {
	class nessmark *m = new nessmark, 
			*f = new nessmark,
			*tab = new nessmark, 
			*eight = new nessmark, 
			*temp = new nessmark,
			*tabchar = new nessmark,
			*spacechar = new nessmark,
			*nlchar = new nessmark;
	(m)->SetFrom( file);
	(eight)->MakeConst( "        ");
	(tabchar)->MakeConst( "\t");
	(spacechar)->MakeConst( " ");
	(nlchar)->MakeConst( "\n");
	(tab)->SetFrom( eight);
	while ( ! (m)->IsEmpty()) {
		/* f := first(m); */
		(f)->SetFrom( m);
		(f)->Start();
		(f)->Next();
		/* m := rest(m); */
		(temp)->SetFrom( m);
		(m)->Start();
		(m)->Next();
		(m)->Next();
		(m)->Extent( temp);

		if ((f)->Equal( tabchar)) {
			/* replace tab with spaces */
			(f)->Replace( tab);
			(tab)->SetFrom( eight);
		}
		else if ((f)->Equal( nlchar)  ||  (tab)->Equal( spacechar)) 
			/* non-tab at tab stop, set for next tab stop */
			(tab)->SetFrom( eight);
		else {
			/* non-tab: shorten distance to tab stop */
			/* tab := rest(tab); */
			(temp)->SetFrom( tab);
			(tab)->Start();
			(tab)->Next();
			(tab)->Next();
			(tab)->Extent( temp);
		}
	}
	delete m;
	delete f;
	delete tab;
	delete tabchar;
	delete spacechar;
	delete nlchar;
	delete eight;
	delete temp;
}
