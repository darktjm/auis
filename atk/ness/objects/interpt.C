/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988, 1991- All Rights Reserved      *
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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/ness/objects/RCS/interpt.C,v 1.4 1995/12/07 16:41:27 robr Stab74 $";
#endif

/* interptest.C
	test the interp object

	builds an object byte stream into ObjectCode
	and executes it
*/

/*
 * $Log: interpt.C,v $
 * Revision 1.4  1995/12/07  16:41:27  robr
 * Support for exporting ness functions to the proctable.
 *
 * Revision 1.3  1994/08/11  02:58:23  rr2b
 * The great gcc-2.6.0 new fiasco, new class foo -> new foo
 *
 * Revision 1.2  1993/06/29  18:06:33  wjh
 * checkin to force update
 *
 * Revision 1.1  1993/06/21  19:30:36  wjh
 * Initial revision
 *
 * Revision 1.12  1993/02/17  06:10:54  wjh
 * uservis: added FUNCVAL to grammar.  Now functions can be values.
 * uservis: added FORWARD decls of functions
 * uservis: added subseq as a synonym for marker
 * uservis: fixed to avoid coredumps that can happen after a syntax error
 * trivial(progvis): make Sym a part of all callnodes
 * 		so dumpstack can say what value is there
 * ignore: removed 'paren' field from varnode
 * ignore: fixed bug in which builtin function names would compile as values but would get runtime error.  Now a compile error.
 * ignore: removed use of ntFUNCTION, etc. in favor of FUNCTION, etc.
 * uservis: check the type of values in RETURN stmts.  This may produce compilation errors in programs which compiled successfully in the past.
 * uservis: change error from ParseReal to be -999.0.  length(WhereItWas()) will be zero.
 * progvis: changed nessruna's -d to -x as switch to dump symbol table
 * progvis: added nessruna switch -y to set debugging
 * trivial: removed disclaimer from ness warning text
 * trivial: removed the restriction in Ness which prevents storing a NaN value
 * trivial: Fixed so the proper function is reported for an error even if warning message is added or removed.  (used a mark in funcnode)
 * trivial: Rewrote the line number calculation (ness.c:LineMsg) so it works when there is a wrapped warning text.  (The warning text is excluded from the count.)
 * adminvis: Added file testness.n ($ANDREWDIR/lib/ness/demos) for testing many facets of Ness
 * uservis: In Ness, provide simple implementations on all platforms for acosh(), asinh(), atanh(), cbrt(), expm1(), and log1p().
 * uservis: Provide Ness predefined identifier 'realIEEE' which is True if isnan(), finite(), and lgamma() are implemented
 * uservis: In Ness, accept isnan(), finite(), and lgamma() during compilation, but give a runtime error if they are executed on a platform where they are not implemented.  (See predefined identifier realIEEE.)
 * ignore: changed eventnode to correspond to change in funcnode
 *
 * e
 *
 * Revision 1.11  1992/12/15  21:37:21  rr2b
 * more disclaimerization fixing
 *
 * Revision 1.10  1992/12/15  00:48:27  rr2b
 * fixed disclaimerization
 *
 * Revision 1.9  1992/12/14  20:49:20  rr2b
 * disclaimerization
 *
 * Revision 1.8  1992/11/26  02:38:01  wjh
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
 * Revision 1.7  91/09/12  16:25:50  bobg
 * Update copyright notice and rcsid
 * 
 * Revision 1.6  1989/09/03  22:48:05  wjh
 * newness
 *
 * Revision 1.5  89/06/01  16:00:15  wjh
 * campus release version
 * 
 * Revision 1.1  88/11/02  14:41:55  wjh
 * Initial revision
 * 
 * Creation 0.0  88/04/06 10:36:00  wjh
 * Initial creation by WJHansen
 * 
*/

#include <andrewos.h>

#include <nessmark.H>
#include <ness.H>

#if 0
/* include the ones utilized, but not by this .c file itself */
#define class_StaticEntriesOnly
#	include <observe.ih>
#	include <proctbl.ih>
#	include <dataobj.ih>
#undef class_StaticEntriesOnly
#endif

YYSTYPE yylval;

/* kludgery needed here because of kludgery in gen.c to do yyerrok and yyclearin */
int yychar = -1;
short yyerrflag = 0;


int main(register int   argc, register char   **argv);
void printdata(register class nessmark  *m);
long LoadReplaceTabs();


	int
main(register int   argc, register char   **argv) {
	register class nessmark *func, *printit;
	FILE *f;
	TGlobRef   f1, f2, hi, nl;
	TGlobRef   piloc, floc, reptabsloc;
	class nesssym *reptabsym, *pisym, *testsym, *proto;
	nesssym_scopeType scope = nesssym_GLOBAL;

	printf("Start\n"); fflush(stdout);
	ATK_Init(".");		/* use current directory for dataobject path (???) */
	printf("Init done\n"); fflush(stdout);

	observable_StaticEntry;
	proctable_StaticEntry;
	dataobject_StaticEntry;
	simpletext_StaticEntry;
	mark_StaticEntry;
	nessmark_StaticEntry;

	if ((f=fopen("/tmp/foo", "r")) == NULL) {
		printf("There needs to be a file /tmp/foo with tabs in it.\n");
		exit(1);
	}
	fclose(f);

	initializeEnvt(NULL);	/* initialize interpreter */
	dumpStack(stdout, NSPstore);

	reptabsloc = LoadReplaceTabs();

	f1 = makeConst("/tmp");
	f2 = makeConst("/foo");
	hi = makeConst("Goodbye");
	nl = makeConst("\n");

	printit = makeFunction(&piloc);
	refStack(printit, 'P', 0);	/* enter func, no locals */
	refStack(printit, 'l', 0);	/* load 1st arg */
	addOp(printit, 'm');		/* dup */		/* creates return value */
	addOp(printit, 'j');		/* print */
	addOp(printit, 'y');		/* pop discard function value */
	refSysMark(printit, 'k', nl);	/* "\n" */
	addOp(printit, 'j');		/* print */
	addOp(printit, 'y');		/* pop discard function value */
	refStack(printit, 'Q', sizeof(class nessmark));	/* leave function (1 arg)*/

  	func = makeFunction(&floc);
	refStack(func, 'P', 0);		/* enter function, with no locals  */
	addOp(func, 'q');		/* newbase */
	refSysMark(func, 'k', f1);	/* "/tmp" */
	refSysMark(func, 'O', piloc); /* call printit */
	addOp(func, 'm');		/* dup */
	addOp(func, 'j');		/* print */
	addOp(func, 'y');		/* pop discard function value */
	addOp(func, 'r');		/* replace */
	addOp(func, 'n');		/* next */
	refSysMark(func, 'k', f2);	/* "/foo" */
	refSysMark(func, 'O', piloc); /* call printit */
	addOp(func, 'r');		/* replace */
	addOp(func, 'p');		/* base */
	refSysMark(func, 'O', piloc); /* call printit */
	addOp(func, 'i');		/* readfile */
	refSysMark(func, 'O', piloc); /* call printit */
	addOp(func, 'm');		/* dup, save a copy for replacetabs call */

	addOp(func, 'o');		/* start */
	addOp(func, 'n');		/* next */
	refSysMark(func, 'O', piloc); /* call printit */
	addOp(func, 'n');		/* next */
	addOp(func, 'm');		/* dup */
	addOp(func, 'n');		/* next */
	addOp(func, 'n');		/* next */
	addOp(func, 'n');		/* next */
	refSysMark(func, 'O', piloc); /* call printit */
	addOp(func, 'x');		/* extent */
	refSysMark(func, 'O', piloc); /* call printit */
	refSysMark(func, 'k', hi);	/* "Goodbye" */
	addOp(func, 'r');		/* replace */
	addOp(func, 'm');		/* dup */
	addOp(func, 'j');		/* print */
	addOp(func, 'y');		/* pop discard function value */
	refSysMark(func, 'k', nl);	/* "\n" */
	addOp(func, 'j');		/* print */
	addOp(func, 'y');		/* pop discard function value */
	addOp(func, 'p');		/* base */
	addOp(func, 'j');		/* print */
	addOp(func, 'y');		/* pop discard function value */
	refSysMark(func, 'k', nl);	/* "\n" */
	addOp(func, 'j');		/* print */
	addOp(func, 'y');		/* pop discard function value */

	refSysMark(func, 'O', reptabsloc);	/* call replace tabs, using saved copy of file */
	addOp(func, 'p');		/* base */
	addOp(func, 'j');		/* print */
	addOp(func, 'y');		/* pop discard function value */
	refSysMark(func, 'k', nl);	/* "\n" */
	addOp(func, 'j');		/* print */
	addOp(func, 'y');		/* pop discard function value */
	refStack(func, 'Q', 0);	/* leave function */

	printdata(func);

	proto = new nesssym;
	reptabsym = nesssym::NDefine("ReplaceTabs", proto, scope);
	reptabsym->info.node = (struct node *)
		funcnode_Create(reptabsloc, 0,0, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL);
	reptabsym->flags = flag_function | flag_ness;

	testsym = nesssym::NDefine("Test", proto, scope);
	testsym->info.node = (struct node *)
		funcnode_Create(floc, 0,0, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL);
	testsym->flags = flag_function | flag_ness;

	pisym = nesssym::NDefine("PrintIt", proto, scope);
	pisym->info.node = (struct node *)
		funcnode_Create(piloc, 0,0, NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL);
	pisym->flags = flag_function | flag_ness;

	pisym->next = testsym;
	testsym->next = reptabsym;
	dumpAttrList(stdout, pisym);

	dumpStack(stdout, NSPstore);

	interpretNess(floc, new ness);	/* initiate interpretation */

	dumpStack(stdout, NSPstore);

	return 0;
}

	void 
printdata(register class nessmark  *m) {
	long loc, lend;
	lend = ((class mark *)m)->GetEndPos();
	if (lend > 31) lend = 31;
	printf("(%d,%d) ", ((class mark *)m)->GetPos(), 
			((class mark *)m)->GetLength());
	for (loc = ((class mark *)m)->GetPos(); loc < lend; loc++) {
		char c = ((m)->GetText())->GetChar( loc);
		if (' ' <= c && c <= '~')
			putchar(c);
		else 	printf("\\%3o", c);
	}
	if (lend == 31 && ((class mark *)m)->GetLength() > 31)
		printf(" . . .");
	putchar('\n');
	fflush(stdout);
}

/*
 * function ReplaceTabs(m) == {
 *	marker c, tab, eight;
 *	eight := "        ";  -- 8 spaces
 *	tab := eight;		-- initial distance to tab
 *	while m /= "" do {
 *		c := first(m);  -- same as front(m)
 *		m := rest(m);
 *		if c = "\t" then {
 *			-- replace tab with spaces 
 *			replace (c, tab);
 *			tab := eight;
 *		}
 *		else if  c = "\n"  or  tab = " "  then
 *			-- newline or single space for this tab,
 *			--	start next tab
 *			tab := eight;
 *		else
 *			-- non-tab: shorten distance to tab stop
 *			tab := rest(tab);
 *	}
 *	return (base(m));
 * }
 */

/* LoadReplaceTabs()
	builds a function for replacing tabs
	returns the index needed to call the function 
*/
	long
LoadReplaceTabs() {
	short floc;
	class nessmark *f;
	short sp8, tab, nl, sp;
	long mloc, cloc, eightloc, tabloc;  /* offsets of local vars on stack */
	long g1, g2, g4, g5, g6, g7;		/* goto targets */

	sp8 = makeConst("        ");
	tab = makeConst("\t");
	nl = makeConst("\n");
	sp = makeConst(" ");

	tabloc = 0;				/* local */
	eightloc = tabloc + sizeof(class nessmark);	/* local */
	cloc = eightloc + sizeof(class nessmark);	/* local */
	mloc = cloc + sizeof(class nessmark);	/* arg */

	f = makeFunction(&floc);
	refStack(f, 'P', 3 * sizeof(class nessmark));	/* enter func, 3 local vars */
	setcurfuncmark(f);				/* for generating branches */

	/* initialize locals */
	refSysMark(f, 'k', sp);	/* c := " "; */
	refStack(f, 's', cloc);

	refSysMark(f, 'k', sp8);	/* eight := "        "; */
	addOp(f, 'm');		/* dup */
	refStack(f, 's', eightloc);
	refStack(f, 's', tabloc);	/* tab := eight; */

	/* while m /= "" do {  */
/* g1:   repeat the while loop */
	g1 = genlabel();
	refStack(f, 'l', mloc);	/* load m */
	addOp(f, 'u');		/* cmp EMPTY */
	g2 = genbranch('e', 0);	/* brEQ to after loop */
	
		/* c := first(m);
		    m := rest(m);    -- extent (next(next(start(m))), m)
		*/
		refStack(f, 'l', mloc);	/* load m */
		addOp(f, 'o');		/* start */
		addOp(f, 'n');		/* next */
		refStack(f, 's', cloc);	/* store c */
		refStack(f, 'l', mloc);	/* load m */
		addOp(f, 'm');		/* dup */
		addOp(f, 'o');		/* start */
		addOp(f, 'n');		/* next */
		addOp(f, 'n');		/* next */
		addOp(f, 'z');		/* swap */
		addOp(f, 'x');		/* extent */
		refStack(f, 's', mloc);	/* store m */
		
		 /* if c = "\t" then {
			-- replace tab with spaces 
			chars(c) := tab;    -- replace(c,tab)
			tab := eight;
		  } */

		predpush(FALSE, 0, 'I');
		refStack(f, 'l', cloc);	/* load c */
		refSysMark(f, 'k', tab);	/* load "\t" */
		addOp(f, 't');		/* string compare */
		predtarget(predEQ);	/* brNE to after then clause */
		predfixdropthru();
		refStack(f, 'l', cloc);	/* load c */
		refStack(f, 'l', tabloc);	/* load tab */
		addOp(f, 'r');		/* replace */
		refStack(f, 'l', eightloc);	/* load eight */
		refStack(f, 's', tabloc);	/* store tab */
		g4 = genbranch('g', 0);	/* goto after else */
		predfixtarget();
		predpop();


		/*  else if  c = "\n"  or  tab = " "  then
			-- newline or single space for this tab,
			--	start next tab
			tab := eight;
		*/
		refStack(f, 'l', cloc);	/* load c */
		refSysMark(f, 'k', nl);	/* load "\n" */
		addOp(f, 't');		/* cmp */
		g7 = genbranch('e');	/* brEQ to then clause */
		refStack(f, 'l', tabloc);	/* load tab */
		refSysMark(f, 'k', sp);	/* load " " */
		addOp(f, 't');		/* cmp */
		g5 = genbranch('f');	/* brNE to after then clause */
/* g7: */
		fixbranch(g7);
		refStack(f, 'l', eightloc);	/* load eight */
		refStack(f, 's', tabloc);	/* store tab */
		g6 = genbranch('g', 0);	/* goto after else */
/* g5: */
		fixbranch(g5);

		/* else
			-- non-tab: shorten distance to tab stop
			tab := rest(tab);
		*/
		refStack(f, 'l', tabloc);	/* load tabloc */
		addOp(f, 'm');		/* dup */
		addOp(f, 'o');		/* start */
		addOp(f, 'n');		/* next */
		addOp(f, 'n');		/* next */
		addOp(f, 'z');		/* swap */
		addOp(f, 'x');		/* extent */
		refStack(f, 's', tabloc);	/* store tabloc */
/* g6: */
		fixbranch(g6);
/* g4: */
		fixbranch(g4);

	/* } -- end while */
	(void)genbranch('g', g1);	/* goto while test */
/* g2:  after the while loop */
	fixbranch(g2);

	/*  return (base(m));
	*/
	refStack(f, 'l', mloc);		/* load m */
	addOp(f, 'p');			/* base */
	refStack(f, 'Q', 4 * sizeof(class nessmark));	/* leave function, 4 args & locals */
	return floc;
}
