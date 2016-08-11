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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/ness/objects/RCS/dump.C,v 1.3 1993/12/16 20:38:01 wjh Stab74 $";
#endif

/* dump.c
	debugging output for ness environment

	Entry points:

	dumpStack(file) - dumps the stack to the indicated FILE
	dumpFuncDef(file, f) - dump a function definition
	dumpAttrList(file, attrs) - dump each definition on attribute list

	ds() == dumpStack(stdout)   			(for gdb)
	da(ness) == dumpAttrList(stdout, ness)	(for gdb)
*/


/*
 * $Log: dump.C,v $
 * Revision 1.3  1993/12/16  20:38:01  wjh
 * curNess ==> curComp->ness
 *
 * Revision 1.2  1993/06/29  18:06:33  wjh
 * checkin to force update
 *
 * Revision 1.1  1993/06/21  19:30:36  wjh
 * Initial revision
 *
 * Revision 1.20  1993/02/17  06:10:54  wjh
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
log elided June 93   -wjh

 * Creation 0.0  88/05/09 19:57:00  wjh
 * Initial creation by WJHansen
 * 
*/

#include <andrewos.h>
#include <proctable.H>

#include <nessmark.H>
#include <ness.H>

static void printstaddr(FILE  *f, union stackelement  *a, union stackelement  *NSP);
void dumpStack(FILE  *f, union stackelement  *NSP);
static void  PrintFromTbl(FILE  *file, register class simpletext  *text, register long  i, struct srchtblelt  *Tbl);
static void PrintChar(class simpletext  *text, long  j, FILE  *f);
void dumpObjectCode(FILE  *file, long  offset);
void dumpFuncDef(FILE  *file, class nesssym  *f);
void dumpEventDef(FILE  *file, class nesssym  *f);
void dumpAttrList(FILE  *file, class nesssym  *symlist);
void  ds(union stackelement  *NSP);
void da(class ness *ness);
void dna(class nesssym  *n);


	static void
printstaddr(FILE  *f, union stackelement  *a, union stackelement  *NSP) {
	long v = (unsigned long)a-(unsigned long)NSP;
	if (a == NULL)
		fprintf(f, "NULL    ");
	else if (v < -100  || v > 1000)
		fprintf(f, "0x%lx", a);
	else 
		fprintf (f, "NSP%-+5ld", v);
}

	void
dumpStack(FILE  *f, union stackelement  *NSP)  {
	union stackelement *tsp, *newtsp;
	ATK *bo;
	long i, pos, len;
	class nessmark *m;
	class simpletext *t;
	long size;

	fprintf(f, "Stack bounds: 0x%lx ... 0x%lx    NSP: 0x%lx\n",
		NSLowEnd, NSHiEnd, NSP);
	fprintf(f, "FramePtr: ");
	printstaddr(f, (union stackelement *)FramePtr, NSP);
	fputc('\n', f);
	for (tsp = NSP-3; tsp < NSHiEnd; ) {
		printstaddr(f, tsp, NSP);  
		switch (tsp->l.hdr) {
		    case (longHdr):
			fprintf(f, ":  long   0x%lx  %d\n", tsp->l.v, tsp->l.v);
			size = sizeof(struct longstkelt);
			break;
		    case (funcHdr):
			fprintf(f, ":  func %s:  %s", 
				callvarietyname[(long)tsp->c.call->variety],
				(tsp->c.call->Sym)->NGetName());
			size = sizeof(struct funcstkelt);
			break;
		    case (boolHdr):
			fprintf(f, ":  bool   0x%lx  %d\n", tsp->b.v, tsp->b.v);
			size = sizeof(struct boolstkelt);
			break;
		    case (dblHdr):
			fprintf(f, ":  dbl    %10g\n", tsp->d.v);
			size = sizeof(struct dblstkelt);
			break;
		    case (ptrHdr):
			fprintf(f, ":  ptr    0x%lx  %d  (%s)\n", 
					tsp->p.v, (tsp->p.v)->GetTypeName());
			size = sizeof(struct ptrstkelt);
			break;
		    case (frameHdr):
			fprintf(f, ":  frame  ret %d   prev ", 
					tsp->f.returnaddress);
			printstaddr(f, (union stackelement *)tsp->f.prevframe, NSP);
			fputc('\n', f);
			size = sizeof(struct frameelt);
			break;
		    case (seqHdr):
			fprintf(f, ":  subseq  ");
			m = tsp->s.v;
			t = (m)->GetText();
			printstaddr(f, 
				(union stackelement *)(m)->GetNext(), 
				NSP);
			pos = (m)->GetPos();
			len = (m)->GetLength();
			fprintf(f, "  0x%lx[%d,%d]  ", t, pos, len);
			if (tsp >= NSP) {
				fprintf(f, "\"");
				for (i = 0; i < 20 && i < len; i++)
					fputc((t)->GetUnsignedChar( pos+i), f);
				fprintf(f, "%s", (i < len) ? "...\"\n" : "\"\n");
			}
			else fprintf(f, "(%d,%d)\n", pos, len);
			size = sizeof(struct seqstkelt);
			break;
		    default:
			/* UNKNOWN stack element */
			fprintf(f, ":  other  0x%lx  %d\n", 
					tsp->l.hdr, tsp->l.hdr);
			size = sizeof(long);
			break;
		}
		newtsp = (union stackelement *) (((unsigned long)tsp) + size);
		if (tsp < NSP && newtsp > NSP)
			tsp = NSP;
		else	tsp = newtsp;
	}
	if (tsp != NSHiEnd)
		fprintf(f, "Stack not sychronized at bottom.\n");
}


enum addrtype {none, enter, stack, global, branch, call, callnd, 
	srch, event, realunary, realbinary, current, cheat, cheatcall};

static struct {
	char *name;
	enum addrtype randcode;
}
opTbl[] = {		/* first operator is for char ' ' */
		{"ERR' '", none},		/* ' ' */
		{"ERR'!'", none},		/* '!' */
		{"ERR'\"'", none},		/* '\"' */
		{"ERR'#'", none},		/* '#' */
		{"ERR'$'", none},		/* '$' */
		{"%", none},		/* '\%' */
		{"ERR'&'", none},		/* '&' */
		{"ERR'\''", none},		/* '\'' */
		{"funcval", none},		/* '(' */
		{"ERR')'", none},		/* ')' */
		{"*", none},		/* '*' */
		{"+", none},		/* '+' */
		{"ERR','", none},		/* ',' */
		{"-", none},		/* '\-' */
		{"ERR'.'", none},		/* '.' */
		{"/", none},		/* '/' */
		{"ld0", none},		/* '0' */
		{"TRUE", none},		/* '1' */
		{"ERR'2'", none},		/* '2' */
		{"ERR'3'", none},		/* '3' */
		{"ERR'4'", none},		/* '4' */
		{"ERR'5'", none},		/* '5' */
		{"ERR'6'", none},		/* '6' */
		{"ERR'7'", none},		/* '7' */
		{"ERR'8'", none},		/* '8' */
		{"FALSE", none},		/* '9' */
		{"ERR':'", none},		/* ':' */
		{"ERR';'", none},		/* ';' */
		{"ERR'<'", none},		/* '<' */
		{"ERR'='", none},		/* '=' */
		{"ERR'>'", none},		/* '>' */
		{"ERR'?'", none},		/* '?' */
		{"ERR'@'", none},	/* '@' */
		{"append", none},		/* 'A' */
		{"bool", none},		/* 'B' */
		{"", callnd},		/* 'C' */
		{"", cheatcall},		/* 'D' */
		{"deftext", none},		/* 'E' */
		{"", srch},		/* 'F' */
		{"ERR'G'", none},		/* 'G' */
		{"", realunary},		/* 'H' */
		{"int", none},		/* 'I' */
		{"", cheat},		/* 'J' */
		{"ERR'K'", none},		/* 'K' */
		{"txtimg", none},		/* 'L' */
		{"", realbinary},		/* 'M' */
		{"\\n", none},		/* 'N' */
		{"call", call},		/* 'O' */
		{"enter", enter},		/* 'P' */
		{"exit", stack},		/* 'Q' */
		{"real", none},		/* 'R' */
		{"", current},		/* 'S' */
		{"cmpT", none},		/* 'T' */
		{"", event},		/* 'U' */
		{"ptr", none},		/* 'V' */
		{"wrtfile", none},		/* 'W' */
		{"system", none},		/* 'X' */
		{"setsel", none},		/* 'Y' */
		{"last", none},		/* 'Z' */
		{"ERR'['", none},		/* '[' */
		{"ERR'\\'", none},		/* '\\' */
		{"ERR']'", none},		/* ']' */
		{"NULL", none},		/* '^' */
		{"neg", none},		/* '_' */
		{"ERR'`'", none},		/* '\`' */
		{"lt", branch},		/* 'a' */
		{"gt", branch},		/* 'b' */
		{"le", branch},		/* 'c' */
		{"ge", branch},		/* 'd' */
		{"eq", branch},		/* 'e' */
		{"ne", branch},		/* 'f' */
		{"goto", branch},		/* 'g' */
	  	{"brerr", branch},		/* 'h' */
		{"readf", none},		/* 'i' */
		{"print", none},		/* 'j' */
		{"ldsys", global},	/* 'k' */
		{"ldstk", stack},		/* 'l' */
		{"dup", none},		/* 'm' */
		{"next", none},		/* 'n' */
		{"start", none},		/* 'o' */
		{"base", none},		/* 'p' */
		{"newbs", none},		/* 'q' */
		{"replace", none},		/* 'r' */
		{"ststk", stack},		/* 's' */
		{"strcmp", none},		/* 't' */
		{"eqempty", none},	/* 'u' */
		{"stsys", global},	/* 'v' */
		{"prev", none},		/* 'w' */
		{"extent", none},		/* 'x' */
		{"pop", none},		/* 'y' */
		{"swap", none},		/* 'z' */
		{"ERR'{'", none},		/* '{' */
		{"ERR'|'", none},		/* '|' */
		{"ERR'}'", none},		/* '}' */
		{"ERR'~'", none},		/* '~' */
		{"ERR'\\177'", none},	/* '\177' */
};
struct srchtblelt {
	char code;
	char *name;
};

static struct srchtblelt 
searchTbl[] = {
	{'a', "search"},
	{'b', "match"},
	{'c', "anyof"},
	{'d', "span"},
	{'e', "token"},
	{'j', "AddStyle"},
	{'k', "HasStyles"},
	{'s', "NextStyleGroup"},
	{'t', "EnclosingStyleGroup"},
	{'u', "ClearStyles"},
	{'p', "ParseInt"},
	{'q', "ParseReal"},
	{'r', "FirstObj"},
	{'w', "WhereItWas"},
	{'x', "ReplaceWithObj"},
	{'\0', "*Unknown Search Code*"}
};
static struct srchtblelt 
eventTbl[] = {
	{'a', "evLdn"},
	{'b', "evLup"},
	{'c', "evLmv"},
	{'d', "evRdn"},
	{'e', "evRup"},
	{'f', "evRmv"},
	{'k', "evK"},
	{'m', "evMenu"},
	{'n', "valgV"},
	{'o', "valgSz"},
	{'p', "valgStr"},
	{'q', "valgA"},
	{'r', "valsV"},
	{'s', "valsSz"},
	{'t', "valsStr"},
	{'u', "valsA"},
	{'v', "valsNt"},
	{'w', "evMAct"},
	{'x', "evMx"},
	{'y', "evMy"},
	{'F', "evIF"},
	{'H', "DoHit"},
	{'Q', "QueueAnswer"},
	{'I', "evCurInset"},
	{'R', "evIsR/O"},
	{'T', "evTell"},
	{'U', "evAsk"},
	{'\0', "*Unknown Event Code*"}
};
static struct srchtblelt 
realUnTbl[] = {
	{'a', "acos"},
	{'b', "acosh"},
	{'c', "asin"},
	{'d', "asinh"},
	{'e', "atan"},
	{'f', "atanh"},
	{'g', "cbrt"},
	{'i', "cos"},
	{'j', "cosh"},
	{'k', "erf"},
	{'l', "erfc"},
	{'m', "exp"},
	{'n', "expm1"},
	{'o', "fabs"},
	{'r', "j0"},
	{'s', "j1"},
	{'t', "lgamma"},
	{'u', "log"},
	{'v', "log10"},
	{'w', "log1p"},
	{'x', "pow"},
	{'y', "sin"},
	{'z', "sinh"},
	{'A', "sqrt"},
	{'B', "tan"},
	{'C', "tanh"},
	{'D', "y0"},
	{'E', "y1"},
	{'_', "-"},
	{'\0', "*Unknown real Unary Code*"}
};
static struct srchtblelt 
realBinTbl[] = {
	{'+', "R+"},
	{'-', "R-"},
	{'*', "R*"},
	{'/', "R/"},
	{'a', "atan2"},
	{'b', "hypot"},
	{'c', "round"},
	{'d', "floor"},
	{'e', "ceil"},
	{'f', "isnan"},
	{'g', "finite"},
	{'j', "float"},
	{'k', "jn"},
	{'l', "yn"},
	{'\0', "*Unknown real Binary Code*"}
};

static struct srchtblelt 
currentTbl[] = {
	{'m', "curmark"},
	{'s', "cursel"},
	{'\0', "*Unknown \"current\" Code*"}
};
static struct srchtblelt 
cheatcallTbl[] = {
	{'a', "CHcMeth "},
	{'b', "CHcProc "},
	{'\0', "*Unknown \"cheatcall\" Code*"}
};
static struct srchtblelt 
cheatTbl[] = {
	{'a', "CHgL"},
	{'b', "CHgD"},
	{'c', "CHgStr"},
	{'d', "CHgCh"},
	{'e', "CHgPtr"},
	{'f', "CHgB"},
	{'g', "CHsL"},
	{'h', "CHsD"},
	{'i', "CHsSP"},
	{'j', "CHsChA"},
	{'k', "CHsStrB"},
	{'l', "CHsCh"},
	{'m', "CHsPtr"},
	{'n', "CHsB"},
	{'q', "forceupdate"},
	{'r', "inset"},
	{'s', "new"},
	{'t', "class"},
	{'\0', "*Unknown \"cheat\" Code*"}
};

	static void 
PrintFromTbl(FILE  *file, register class simpletext  *text, register long  i, 
		struct srchtblelt  *Tbl) {
	register char c = (char)(text)->GetUnsignedChar( i);
	register struct srchtblelt *e;
	for (e = Tbl; e->code && e->code != c; e++) {}
	fprintf(file, "%s", e->name);
}

/* PrintChar(text, j, file)
	print the j'th character of 'text' to 'file'
	print non-ASCII characters with care
*/
	static void
PrintChar(class simpletext  *text, long  j, FILE  *f) {
	register long ic = (text)->GetUnsignedChar(j);
	if (ic < ' ') putc('^', f),  putc(ic+'@', f);
	else if (ic < '\177') putc (ic, f);
	else fprintf (f, "\\%03o", ic);
}


/* dumpObjectCode(file, offset)
	print to 'file' a representation of the function represented by the 
	nessmark at location 'offset' in ness_Globals
*/
	void
dumpObjectCode(FILE  *file, long  offset) {
	register long i, end;
	long start;
	register class simpletext *text;
	long sylcnt;	/* count syllables printed on each line */
	class nessmark *m = ness_Globals[offset].e.s.v;

	text = (m)->GetText();
	start = (m)->GetPos();
	end = start + (m)->GetLength();
	sylcnt = 9999;
	for (i = start;  i < end; ) {
		/* print syllable(s) for operator at i */
		long op;
		enum addrtype randcode;

		if (sylcnt > 6)	/* newline after 10 syllables */
			fprintf(file, "\n     %d:", i), sylcnt = 0;

		op = (text)->GetUnsignedChar( i);
		i++;
		if (op >= ' ' &&  op <= '\177') {
			fprintf(file, "   %s", opTbl[op-' '].name);
			randcode = opTbl[op - ' '].randcode;
		}
		else if (op == '\n') {
			fprintf(file, "   nop");
			randcode = none;
		}
		else {
			fprintf(file, "   ERR'\\%o'", op);
			randcode = none;
		}
		sylcnt++;

		switch (randcode) {
		case none:
			break;
		case enter:
			fprintf(file, " (#loc %d)", 
				(text)->GetUnsignedChar( i));
			i++;
			sylcnt += 3;
			break;
		case branch: {
			long offset;
			offset = ExtendShortSign(
				((text)->GetUnsignedChar( i) << 8)
				 + (text)->GetUnsignedChar( i+1)
			);
			fprintf(file, "->%d", (i-1) + offset);
			i += 2;
			sylcnt = 100;	/* put newline */
		}	break;
		case call:  {
			long index = ((text)->GetUnsignedChar( i)<<8)
					+(text)->GetUnsignedChar( i+1);
			fprintf(file, "->%d(@%d)", index,
				(ness_Globals[index].e.s.v)->GetPos());
			i += 2;
			sylcnt++;
		}	break;
		case stack:
			fprintf(file, " %d", (text)->GetUnsignedChar( i));
			i++;
			sylcnt++;
			break;
		case global: {
			TGlobRef t = ((text)->GetUnsignedChar( i)<<8)
					+(text)->GetUnsignedChar( i+1);
			class nessmark *m = ness_Globals[t].e.s.v;
			long j, end;
			class simpletext *text = (m)->GetText();
			fprintf(file, " (%d)\"", t);
			j = (m)->GetPos();
			end = j + (m)->GetLength();
			if(text!=NULL) {
			    if (end - j < 20)
				for ( ; j < end; j++)
				    PrintChar(text, j, file);
			    else {
				/* print 10 chars, three dots, and 7 more chars */
				long frontend = j + 10;
				for ( ; j < frontend; j++)
				    PrintChar(text, j, file);
				putc('.', file);  putc('.', file);  putc('.', file);
				for (j = end - 7 ; j < end; j++)
				    PrintChar(text, j, file);
			    }
			} else fprintf(file, "<no text object>");
			putc ('"', file);
			i += 2;
			sylcnt++;
		    }	break;
		case callnd: {
			unsigned char c0, c1, c2, c3;
			struct callnode *cnode;
			c0 = (text)->GetUnsignedChar( i);
			c1 = (text)->GetUnsignedChar( i+1);
			c2 = (text)->GetUnsignedChar( i+2);
			c3 = (text)->GetUnsignedChar( i+3);
			cnode = (struct callnode *)
				((c0<<24) | (c1 << 16) | (c2 << 8) | c3);
			if (cnode != NULL) {
				fprintf(file, "%s:%s", 
					callvarietyname[(long)cnode->variety],
					(cnode->Sym)->NGetName());
			}
			else 
				fprintf(file, "callnode(NULL)");
			i += 4;
			sylcnt += 3;
		}	break;
		case srch:
			PrintFromTbl(file, text, i++, searchTbl);  break;
		case current:
			PrintFromTbl(file, text, i++, currentTbl);  break;
		case cheat:
			PrintFromTbl(file, text, i++, cheatTbl);  break;
		case cheatcall:
			PrintFromTbl(file, text, i++, cheatcallTbl); 
			/* print number of args */
			fprintf(file, "%d", (text)->GetUnsignedChar( i));
			i ++;
			sylcnt++;
			break;
		case event:
			PrintFromTbl(file, text, i++, eventTbl);  break;
		case realunary:
			PrintFromTbl(file, text, i++, realUnTbl);  break;
		case realbinary:
			PrintFromTbl(file, text, i++, realBinTbl);  break;
		} /* end switch */
	} /* end for */
	fprintf(file, "\n");
}

	void
dumpFuncDef(FILE  *file, class nesssym  *f) {
	long offset = nesssym_NGetINode(f, funcnode)->SysGlobOffset;

	fprintf(file, "%d:  Function %s", offset, (f)->NGetName());
	dumpObjectCode(file, offset);
}

	void
dumpEventDef(FILE  *file, class nesssym  *f) {
	struct eventnode *enode = nesssym_NGetINode(f, eventnode);
	long offset = enode->SysGlobOffset;
	if (enode->enabled) {
		fprintf(file, "%d:  %s Event (%s)", offset, 
			(enode->varsym)->NGetName(), enode->spec);
		dumpObjectCode(file, offset);
	}
	else
		fprintf(file, "%d:  disabled Event (%s)", offset, enode->spec);
}


	void
dumpAttrList(FILE  *file, class nesssym  *symlist) {
	char buf[256];
	for ( ; symlist;  symlist = symlist->next)
		switch (symlist->flags) {
		case flag_function | flag_ness:
		case flag_function | flag_ness | flag_xfunc:
			dumpFuncDef(file, symlist);
			break;
		case flag_var | flag_globalvar:
			fprintf(file, "%d: %s %s\n", 
				nesssym_NGetINode(symlist, vardefnode)->addr,
				TypeName[symlist->type],
				(symlist)->NGetName());
			break;
		case flag_xobj:
			fprintf(file, "\nEXTENDED OBJECT %s\n",
				(symlist)->ToC(symlist->parent.ness,
						 buf, sizeof(buf)));
			dumpAttrList(file, (nesssym_NGetINode(symlist,objnode)->attrs));
			fprintf(file, "END OBJECT %s\n\n", buf);
			break;
		case flag_event:
			dumpEventDef(file, symlist);
			break;
		default:
			fprintf(file, "Symbol %s has unknown flags 0x%lx\n", 
					(symlist)->NGetName(), symlist->flags);
			break;
		}
}




/* functions for calling from the debugger */

	void 
ds(union stackelement  *NSP)  {
	dumpStack(stdout, NSP);
	fflush(stdout);
}

	void
da(class ness *ness) {
	dumpAttrList(stdout, *ness->AttrDest);
	fflush(stdout);
}

	void
dna(class nesssym  *n) {
	dumpAttrList(stdout, n);
	fflush(stdout);
}


