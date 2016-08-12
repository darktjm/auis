/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
	Copyright Carnegie Mellon University 1992, 1993 - All Rights Reserved
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
\* ********************************************************************** */
#ifndef NORCSID
static char *rcsid = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/ness/objects/RCS/gen.C,v 1.11 1995/12/07 16:41:27 robr Stab74 $";
#endif

/* 
	gen.c  -  generate interpreter code for ness

*/
/*
 * $Log: gen.C,v $
 * Revision 1.11  1995/12/07  16:41:27  robr
 * Support for exporting ness functions to the proctable.
 *
 * Revision 1.10  1994/12/19  19:32:19  rr2b
 * Fixed to avoid a g++ compiler bug with different type arguments to the , operator.
 * BUG
 *
 * Revision 1.9  1994/08/13  15:28:37  rr2b
 * new fiasco
 *
 * Revision 1.8  1994/08/11  02:58:11  rr2b
 * The great gcc-2.6.0 new fiasco, new class foo -> new foo
 *
 * Revision 1.7  1994/04/10  23:50:56  wjh
 * TRIV use rules.H for token names;  lexan->parser
 *
 * Revision 1.6  1994/03/21  17:00:38  rr2b
 * bzero->memset
 * bcopy->memmove
 * index->strchr
 * rindex->strrchr
 * some mktemp->tmpnam
 *
 * Revision 1.5  1993/12/16  20:38:01  wjh
 * curNess ==> curComp->ness
 *
 * Revision 1.4  1993/07/23  00:23:42  rr2b
 * Split off a version of CopyText which will copy surrounding
 * styles as well as embedded styles.
 *
 * Revision 1.3  1993/06/29  18:24:04  wjh
 * *** empty log message ***
 *
 * Revision 1.2  1993/06/21  20:43:31  wjh
 * fixed ungetMark problems
 *
 * Revision 1.1  1993/06/21  19:30:36  wjh
 * Initial revision
 *
 * Revision 1.31  1993/03/31  13:47:45  wjh
 * fixed so calling a function does not leave junk on stack
 *
 * Revision 1.30  1993/02/17  06:10:54  wjh
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
 * Revision 1.29  1992/12/14  20:49:20  rr2b
 * disclaimerization
 *
 * Revision 1.28  1992/12/09  04:25:22  wjh
 * fixed extraneous \...n around definitions in dict.n
 * by fixing BackSlashReduce to keep track of where it is in the source
 * .
 *
 * Revision 1.27  1992/12/08  04:14:46  wjh
 * fixed so it will not crash when a runtime error occurs
 * 	moved the recompile to locate error operations to compile.c
 * .
 *
 * Revision 1.26  1992/12/04  19:14:57  wjh
 * comment out the definition of parencheck, which is no longer called
 * .
 *
 * Revision 1.25  1992/11/26  02:42:25  wjh
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
 * Revision 1.24  92/06/05  16:39:31  rr2b
 * corrected sysmark allocation and made arrangements
 * for deallocation as well, sysmarks should no longer be
 * "leaked"
 * 
 * Revision 1.23  1991/09/24  18:34:39  wjh
 * make BackSlashReduce non-static
 * .c inside freeze
 * .
 * ...	log elided, Nov 92 wjh
 * 
 * Creation 0.0  88/03/29 10:25:00  wjh
 * Initial creation by WJHansen
 * 
*/
#include <andrewos.h>	/* for bzero() */
#include <ctype.h>

#include <text.H>
#include <tlex.H>

#include <nessmark.H>
#include <ness.H>
#include <rules.H>

/* functions declared FORWARD and not yet defined */
static class nesssym *ForwardFuncs = NULL;   

/* defining instance for declaration in compdefs.hn */
char *TypeName[/* Texpr */] = {
	"list-end", 
	"integer", 
	"boolean", 
	"boolean", 
	"real", 
	"subseq", 
	"object", 
	"function", 
	"void",
	"unknown",
	"Cstring",
	"short",
	"char"
};

/* the order of operators in Xoptbl is EQ, NE, LT, GE, GT, LE, BR, NOP
		BR is used for predT and predAND
		NOP is used for predF and predOR
	the FopTbl has the opposite operation from the TopTbl */
static char TopTbl[] = {'e', 'f', 'a', 'd', 'b', 'c', 'g', '\n'};
static char FopTbl[] = {'f', 'e', 'd', 'a', 'c', 'b', '\n', 'g'};


/* these two may have elements from more than one
	compilation if there is a compile during compile */
static struct predstatenode predstack[MAXEXPRDEPTH];
struct predstatenode *pssp = predstack - 1;   /* points to the in-use elt of predstate stack */


#define ENTERARGOFFSET 1	/* offset from start of object for function
				to the operand that must be fixed */


static TGlobRef allocSysGlob(long  erroff, class ness  *theNess);
static TGlobRef allocSysMark(long erroff, class ness *theNess);
void  deallocSysGlobs(TGlobRef  ind);
class nessmark * makeFunction(TGlobRef  *loc);
TGlobRef makeConst(char  *s);
long BackSlashReduce(register class text  *text);
TGlobRef makeStyledConst(class text  *text, long  pos , long  len, boolean  bsReduce);
TGlobRef makeGlobal();
void addOp(class nessmark  *m, char op);
TCodeRef refStack(class nessmark  *m, char op, TStackRef  rand);
void fixRefStack(class nessmark  *m, TCodeRef  loc, TStackRef  val);
TCodeRef refSysGlob(class nessmark  *m, char op, TGlobRef  rand);
void fixRefSysGlob(class nessmark  *m, TCodeRef  loc, TGlobRef  val);
void refAddress(class nessmark  *m, char op, struct callnode *address);
void genLinkGlobal(class nesssym  *sym);
static void RememberFixup(TCodeRef  refloc, class nesssym  *sym);
static void DoFixups(unsigned long  lpsize);
static unsigned long  allocate(register class nesssym  *syml, register unsigned long  prior);
TGlobRef genEnter();
void genreturn(struct exprnode  *exp);
boolean genExit(class nesssym  *parmlist , class nesssym  *locallist);
void startfunc(class nesssym  *fname, Texpr  functype);
void finishfunc(class nesssym  *fname, class nesssym  *parmlist, 
		class nesssym  *locallist, boolean Export);
void abortfunc();
void genchkforward();
class nesssym * appendlists(register class nesssym  *list1 , register class nesssym  *list2);
struct exprnode * genarith(char op, struct exprnode  *left , struct exprnode  *right);
void gencomp(struct exprnode  *left , struct exprnode  *right, long  rop);
Texpr genvarref(struct varnode  *var);
void genvarstore(struct varnode  *varnode);
struct varnode * genvarnode(class nesssym  *sym);
void genconstref(class nesssym  *constant);
void genop(char op);
void setcurfuncmark(class nessmark  *m);
class nessmark * getcurfuncmark();
long genlabel();
long genbranch(char op, long  dest);
void fixbranch(long  loc);
struct predstatenode * predpush(boolean  cond, long  loc, char  construct);
static void appendgotolists(struct gotonode  **list , struct gotonode  *addon);
void predpop();
void predvalue(Texpr  *Ptype);
void predbranch(struct exprnode  *expr);
void preddropthru(long  rop);
void predtarget(long  rop);
static void fixbranchlist(struct gotonode  * list);
void predfixdropthru();
void predfixtarget();
void predexit(char  construct);
void demandsymboltype(class nesssym  *sym, Texpr  type);
void demandnodetype(struct exprnode  *node, Texpr  type);
struct varnode * varIsStorable(struct varnode  *var);
struct varnode * varIsFunction(struct varnode  *var);
class nesssym * uniqueinscope(class nesssym * var, unsigned long  flags, long  tokoff);
void ProcessDeclList(Texpr  type, class nesssym  *idlist, long  flags);
class nesssym * genParmDecl(class nesssym  *sym  , Texpr  type);
void genCheckEndtag(class nesssym  *tag, long  desired);
void genSaveFuncState();
void genRestoreFuncState(class nesssym  *func);


/* allocSysGlob(erroff)
	allocate an element of the SysGlobals array
	if space exhausted, used 'erroff' to identify the error token 

	if we ever extend SysGlobals, we must not go over 65535 entries
		because there are only two bytes for addressing in operands
*/
	static TGlobRef
allocSysGlob(long  erroff, class ness  *theNess) {
	TGlobRef loc;

	if (ness_GlobFree != 0) {
		/* allocate from the free list */
		loc = ness_GlobFree;
		ness_GlobFree = ness_Globals[loc].next;
	}
	else if (ness_GlobSize < 65535) {
		/* realloc ness_Globals
		   link the new pieces onto ness_GlobFree by next field */
		long i;
		long oldsize = ness_GlobSize;
		ness_GlobSize += 1000;
		if (ness_GlobSize > 65535) 
			ness_GlobSize = 65535;
		ness_Globals = (struct globelt *)
			realloc(ness_Globals, 
				ness_GlobSize * sizeof(struct globelt));

		memset(ness_Globals + oldsize, 
		      0, (ness_GlobSize - oldsize) * sizeof(struct globelt));
		loc = oldsize;   /* allocate the first of the new elts */
		ness_GlobFree = oldsize+1;   /* link the rest */
		for (i = oldsize+1; i < ness_GlobSize-1; i++)
			ness_Globals[i].next = i+1;
		ness_Globals[ness_GlobSize-1].next = 0;
	}
	else {
		/* XXX need to realloc SysGlob, but would exceed 65535 */
		ReportError(":Global storage exceeded", erroff);
		/* KLUDGE to turn off compilation:   XXX */
			curComp->Locating = TRUE;  
			curComp->LocationAdvancing = TRUE;
			curComp->Sought = curComp->CurrOffset;
		return 0;
	}
	if(theNess) {
		ness_Globals[loc].next = theNess->marks;
		theNess->marks = loc;
	}
	return loc;
}

/* allocSysMark(erroff, theNess)
	allocate an element of the ness_Globals array
		and make it refer to a nessmark from ness_FreeSeqs
	if space exhausted, used 'erroff' to identify the error token 
*/
	static TGlobRef
allocSysMark(long  erroff, class ness  *theNess) {
	TGlobRef gr = allocSysGlob(erroff, theNess);
	if (gr == 0) return 0;

	/* now make ness_Globals[gr].e.s.v point to a nessmark */
	ness_Globals[gr].e.s.hdr = seqHdr;
	ness_Globals[gr].e.s.v = nessmark::getMark();
	return gr;
}


/* deallocate all ness_Globals elementson a given chain
	this is used to free the chain hanging off a ness when the
	ness calls releaseResources in preparation for another compile
	Also: unget any marks referenced from the freed globs.
*/
	void 
deallocSysGlobs(TGlobRef inx) {
	TGlobRef i = inx, succ;
	while(i > 0) {
		register struct globelt *g = &ness_Globals[i];
		succ = g->next;
		if (g->e.s.hdr == seqHdr) {
			/* free a nessmark.  but 1st check to free objcode */
			register class nessmark *n = g->e.s.v;

			if (n->GetText() == ObjectCode) {
				if (n->GetLength() > 0)
					(ObjectCode)->DeleteCharacters(
						n->GetPos(), n->GetLength()+1);
			}
			(n)->ungetMark();
		}
		g->e.i.hdr = idleHdr;
		g->next = ness_GlobFree;
		ness_GlobFree = i;
		i = succ;
	}
}
    

/* makeFunction(loc)
	returns a pointer to a mark for space to store a function in ObjectCode
	sets 'loc' to the index of the function for use in calls
	
	adds a dummy RETURN after the function to separate its marks 
	from that of the next function
*/
	class nessmark *
makeFunction(TGlobRef  *loc)  {
	long endpos;
	*loc = allocSysMark(-1, curComp->ness);
	if (*loc == 0) return NULL;
	if (curComp->Locating) {*loc = 0; return NULL;}

	class nessmark *v = ness_Globals[*loc].e.s.v;
	endpos = (ObjectCode)->GetLength();
	(ObjectCode)->InsertCharacters( endpos, "Q", 1);
	(v)->Set( ObjectCode, endpos, 0);
	return v;
}

/* makeConst(s)
	make a Ness constant for C string s
	returns an index into ness_Globals
*/
	TGlobRef
makeConst(char  *s) {
	TGlobRef loc = allocSysMark(0, NULL);
	if (loc == 0 || curComp->Locating) 
		return 0;

	class nessmark *v = ness_Globals[loc].e.s.v;

	(v)->MakeConst(s);
	return (loc);
}


/*BackSlashReduce(text)
	reduce any backslash constructs from the text
	if text ends with backslash, it will be deleted
	return length of resulting text
*/
	long
BackSlashReduce(register class text  *text) {
	register long pos, len;
	register char c;
	len = (text)->GetLength();

	for (pos = 0; pos < len; pos++) {
		c = (text)->GetChar( pos);
		if (c == '\\') {
			char buf[4];
			int elen;
			buf[0] = (text)->GetChar( pos+1);
			buf[1] = (text)->GetChar( pos+2);
			buf[2] = (text)->GetChar( pos+3);
			buf[3] = '\0';
			elen = len - pos;
			if (elen < 3) buf[elen] = '\0';
			c = parser::TransEscape(buf, &elen);
			(text)->DeleteCharacters( pos, elen+1);
			if (elen > 0) {
				char ch = c;
				(text)->InsertCharacters( pos, &ch, 1);
				len -= elen;
			}
			else len--;
		}
	}
	return len;
}


/* makeStlyedConst(text, pos, len)
	make a Ness constant from the string in 'text' of length 'len'
		starting at 'pos'
	returns an index into ness_Globals
*/
	TGlobRef
makeStyledConst(class text  *textp, long  pos , long  len, boolean  bsReduce) {
	if (len == 0)
		return makeConst("");
	TGlobRef loc = allocSysMark(0, NULL);
	if (loc == 0 || curComp->Locating) 
		return 0;

	class nessmark *v = ness_Globals[loc].e.s.v;
	class text *newt;

	newt = new text;
	(newt)->AlwaysCopyTextExactly( 0, textp, pos, len);
	if (bsReduce) 
		len = BackSlashReduce(newt);
	(newt)->SetReadOnly( TRUE);
	(v)->Set( newt, 0, len);
	return (loc);
}

/* makeGlobal()
	make a Ness global variable
	returns an index into ness_Globals
*/
	TGlobRef
makeGlobal() {
	TGlobRef loc = allocSysGlob(0, curComp->ness);
	if (loc == 0 || curComp->Locating) return 0;

	return (loc);
}

/* addOp(m, op);
	appends operator 'op' to 'm'
*/
	void
addOp(class nessmark  *m, char op) {
	long pos, len;
	if (curComp->Locating) {compileLocate(1); return;}
	pos = (m)->GetPos();
	len = (m)->GetLength();
	((m)->GetText())->InsertCharacters( pos+len, &op, 1);
	(m)->SetLength( len+1);
}

/* refStack(m, op, rand)
	append to m the opcode op with the one-byte operand rand 
	return location of the operand as an arg to RememberFixup

	the case of rand>0xFF is tested for in finishfunc 
*/
	TCodeRef
refStack(class nessmark  *m, char op, TStackRef  rand) {
	long pos, len;
	char s[2];
	if (curComp->Locating) {compileLocate(2); return 0;}
	pos = (m)->GetPos();
	len = (m)->GetLength();
	s[0] = op;
	s[1] = rand & 0xFF;
	((m)->GetText())->InsertCharacters( pos+len, s, 2);
	(m)->SetLength( len+2);
	return (len+1);
}

	void
fixRefStack(class nessmark  *m, TCodeRef  loc, TStackRef  val) {
	if (curComp->Locating)  {compileLocate(1); return;}
	((m)->GetText())->ReplaceCharacters( (m)->GetPos()+loc,
			sizeof(TStackRef), (char *)&val, sizeof(TStackRef));
}

/* refSysGlob(m, op, rand)
	append to m the opcode op with the two-byte operand rand
	return location in code of the operand field

	overflow of the address field is covered in the MakeXxx functions,
		which do not allocate more than 65535 ness_Global locations
*/
	TCodeRef
refSysGlob(class nessmark  *m, char op, TGlobRef  rand) {
	long pos, len;
	char s[3];
	if (curComp->Locating) {compileLocate(3); return 0;}
	pos = (m)->GetPos();
	len = (m)->GetLength();
	s[0] = op;
	s[1] = rand>>8;
	s[2] = rand & 0xFF;
	((m)->GetText())->InsertCharacters( pos+len, s, 3);
	(m)->SetLength( len+3);
	return len+1;
}

	void
fixRefSysGlob(class nessmark  *m, TCodeRef  loc, TGlobRef  val) {
	if (curComp->Locating)  {compileLocate(1); return;}
	((m)->GetText())->ReplaceCharacters( (m)->GetPos()+loc,
			sizeof(TGlobRef), (char *)&val, sizeof(TGlobRef));
}
/* refAddress(m, op, address)
	append to m the opcode op with the four-byte operand rand 
*/
	void
refAddress(class nessmark  *m, char op, struct callnode *address) {
	long pos, len;
	char s[5];
	if (curComp->Locating)  {compileLocate(5); return;}
	s[0] = op;
	s[1] = ((unsigned long)address)>>24;
	s[2] = ((unsigned long)address)>>16;
	s[3] = ((unsigned long)address)>>8;
	s[4] = ((unsigned long)address) & 0xFF;
	pos = (m)->GetPos();
	len = (m)->GetLength();
	((m)->GetText())->InsertCharacters( pos+len, s, 5);
	(m)->SetLength( len+5);
}

/* genLinkGlobal(sym)
	attach sym to the global list given by *curComp->ness->AttrDest
*/
	void
genLinkGlobal(class nesssym  *sym) {
	if (curComp->Locating) return;
	sym->next = *curComp->ness->AttrDest;
	*curComp->ness->AttrDest = sym;
}


	static void
RememberFixup(TCodeRef  refloc, class nesssym  *sym) {
	struct fixupstackrefnode *f;
	if (curComp->freefixups != NULL) {
		f = curComp->freefixups;
		curComp->freefixups= f->next;
		f->next = curComp->varfixups;
		f->refloc = refloc;
		f->sym = sym;
		curComp->varfixups = f;
	}
	else
		curComp->varfixups = fixupstackrefnode_Create(refloc, sym, 
			curComp->varfixups);
}
	static void
DoFixups(unsigned long  lpsize) {
	struct fixupstackrefnode *f, *t;
	long addr;
	struct vardefnode *vdnode;
	for (f = curComp->varfixups; f != NULL; f = t) {
		if (f->sym != NULL) {
			vdnode = nesssym_NGetINode(f->sym, vardefnode);
			if (vdnode) 
				fixRefStack(curComp->curfuncmark, f->refloc, 
					vdnode->addr);
		}
		else 
			fixRefStack(curComp->curfuncmark, f->refloc, lpsize);
		t = f->next;
		f->next = curComp->freefixups;
		curComp->freefixups = f;
	}
	curComp->varfixups = NULL;
}

/* allocate (syml, prior)
	set to the proper stack offset the node.vardefnode.addr
		of each s that is a symbol on the list at 'syml'
	begin with offset 'prior'
	Return total size allocated, including 'prior'

	Note that the list of parameters is in reverse order, but that this
	means they are in the order of increasing positive offset from the 
	frame header, which is at a higher location and lower address in the stack.
*/
	static unsigned long 
allocate(register class nesssym  *syml, register unsigned long  prior) {
	for ( ; syml != NULL; syml = syml->next) {
		nesssym_NGetINode(syml, vardefnode)->addr = prior;
		if (syml->type == Tbool)
			prior += sizeof(struct boolstkelt);
		else if (syml->type == Tlong)
			prior += sizeof(struct longstkelt);
		else if (syml->type == Tdbl)
			prior += sizeof(struct dblstkelt);
		else if (syml->type == Tptr)
			prior += sizeof(struct ptrstkelt);
		else if ((syml->type & 0xF) == Tfunc)
			prior += sizeof(struct funcstkelt);
		else
			prior += sizeof(struct seqstkelt);
	}
	return prior;
}

	
	TGlobRef
genEnter() {
	TGlobRef loc;
	curComp->varfixups = NULL;
	curComp->curfuncmark = makeFunction(&loc);
	/* generate: ENTER (sizeof locals)   (check return value in case the offset changes) */
	if (refStack(curComp->curfuncmark, 'P', 0) != ENTERARGOFFSET) 
		ReportError(":URP!", -1);
	return loc;
}

	void
genreturn(struct exprnode  *exp) {
	if (exp == (struct exprnode *)1)
		{} /* no type check */
	else if (exp == NULL) {
		if (curComp->curfunctype != Tvoid)
			ReportError(":This function must return a value", -1);
	}
	else if (exp->type != curComp->curfunctype)
		SaveError(":Value being returned is of wrong type", 
				exp->loc, exp->len);
	if (exp == NULL  ||  exp == (struct exprnode *)1) {
		/* provide zero as the default value */
		addOp(curComp->curfuncmark, '0');
	}
	RememberFixup(refStack(curComp->curfuncmark, 'Q', 0), NULL);
}

/* genExit(parmlist, locallist)
	generates code at end of function and does fixups
	returns TRUE if offsets are less than 255 and FALSE otherwise
*/
	boolean
genExit(class nesssym  *parmlist , class nesssym  *locallist) {
	long locsize, totsize;
	locsize = allocate(locallist, 0);
	fixRefStack(curComp->curfuncmark, ENTERARGOFFSET, locsize);
	genreturn((struct exprnode *)1);
	totsize=allocate(parmlist, locsize);
	DoFixups(totsize);
	return (totsize <= 255);
}

/* record function information needed to close out the function in finishfunc
	fname is the symbol for the function name
	functype is the type, or is Tend if the type is missing
*/
	void
startfunc(class nesssym  *fname, Texpr  functype) {
	nesssym_scopeType curscope = curComp->scopes[curComp->scopex];
	class nesssym *ts;
	int offset = -2;
	struct funcnode *func;

	if (functype == Tend) {
		functype = Tstr;
		offset = -1;
	}

	if ((fname->flags & (flag_forward | flag_ness)) 
			== (flag_forward | flag_ness)) {
		/* this is the real declaration of a function
			that was earlier declared FORWARD */
		/* check type versus forward decl */
		if (functype != 
				nesssym_NGetINode(fname, funcnode)->functype)
			ReportError(":Type differs from that in FORWARD declaration", offset);

		/* prune from ForwardFuncs list */
		if (ForwardFuncs == fname)
			ForwardFuncs = ForwardFuncs->next;
		else 
			for (ts = ForwardFuncs; ts != NULL; ts = ts->next) 
				if (ts->next == fname) {
					ts->next = ts->next->next;
					break;
				}
			/* (if ts == NULL, we have an internal error) */

		/* reenter the scope created earlier */
		func = nesssym_NGetINode(fname, funcnode);
		(func->where)->SetPos( 
			(curComp->tlex)->RecentPosition( offset, NULL));
		curComp->varfixups = NULL;
		curComp->curfuncmark = (class nessmark *)(func->locallist);
		compPushScope(func->ownscope);
	}
	else {
		nesssym_NSetINode(fname, funcnode, 
			funcnode_Create(genEnter(), curscope, compNewScope(), 
				(curComp->ness)->CreateMark( 
					(curComp->tlex)->RecentPosition( offset, NULL), 
					999), 
				NULL, NULL, functype, NULL, NULL, NULL, NULL));
	}
	curComp->curfunctype = functype;	/* for RETURN */
}

/* complete a function body
	fname is symbol for function name
	parmlist is list of symbols for parameters
	locallist is list of locally declared variables
		if local list is -1, this is a FORWARD declaration
 */
	extern void ness_ExportFunction(ness *self, nesssym *fname, funcnode *func);
	
	void
finishfunc(class nesssym  *fname, class nesssym  *parmlist, 
		class nesssym  *locallist, boolean Export) {
	long lastloc, lastlen, srclen, srcloc;
	int i;
	class nesssym *p;
	struct funcnode *func = nesssym_NGetINode(fname, funcnode);

	srcloc = (func->where)->GetPos();
	lastloc = (curComp->tlex)->RecentPosition( 0, &lastlen);
	srclen = lastloc + lastlen - srcloc;
	(func->where)->SetLength(srclen);

	if (fname->flags & flag_forward) {
		/* now finishing func that was declared FORWARD */
		boolean parmerr = FALSE;
		fname->flags = fname->flags & (~flag_forward);
		if (func->parmlist != parmlist) {
			parmerr = TRUE;
			func->parmlist = parmlist;  /* use more recent list */
		}
		for (p = func->parmlist; p != NULL; p = p->next) {
			if ((p->flags & flag_forward) == 0)
				parmerr = TRUE;
			p->flags = flag_var | flag_parmvar;
		}
		if (parmerr)
			SaveError(":parameters unlike forward declaration", 
				srcloc, srclen);
	}

	compPopScope();

	if (locallist == (class nesssym *)(-1)) {
		/* this is a FORWARD decl.  Mark parms as forward */
		for (p = func->parmlist; p != NULL; p = p->next)
			p->flags |= flag_forward;
		func->locallist = (class nesssym *)(curComp->curfuncmark);
		fname->flags |= flag_forward;
		fname->next = ForwardFuncs;
		ForwardFuncs = fname;
	}
	else {
		/* this is a normal function decl */
		func->locallist = locallist;
		for (i = 0, p = func->parmlist; p != NULL; i++, p = p->next) {}
	
		if ( ! genExit(func->parmlist, locallist) || i > MAXARGS)
			SaveError(":Too many arguments and locals", 
					srcloc, srclen);
		genLinkGlobal(fname);
	}
	if(Export) ness_ExportFunction(curComp->ness, fname, func);
}

	void
abortfunc() {
	/* XXX  gets wrong scope if inside EXTEND...END, but outside a function */
	compPopScope();
}


/* genchkforward
	check that all FORWARD declarations have been satisfied
*/
	void
genchkforward() {
	while (ForwardFuncs) {
		struct funcnode *func 
			= nesssym_NGetINode(ForwardFuncs, funcnode);
		SaveError(":There is no corresponding function declaration",
			(func->where)->GetPos(),
			(func->where)->GetLength());
		ForwardFuncs = ForwardFuncs->next;
		/* the symbol and funcnode will be discarded when the
			scope is cleared */
	}
}

/* appendlists(list1, list2)
	Concatenate the symbol lists 'list1' and 'list2'
	The lists are in _reverse_ order, so put 'list2' before 'list1'.
*/
	class nesssym *
appendlists(register class nesssym  *list1 , register class nesssym  *list2) {
	register class nesssym *lastElt, *t;
	if (list2 == NULL) return list1;
	if (list1 == NULL) return list2;

	lastElt = list2;
	while ((t=lastElt->next) != NULL)
		lastElt = t;
	lastElt->next = list1;	
	return list2;
}

	struct exprnode *
genarith(char op, struct exprnode  *left , struct exprnode  *right) {
	struct exprnode *r;
	if (left->type == Tdbl) {
	    demandnodetype(right, Tdbl);

	    r=left;
	    left->len=right->loc+right->len-left->loc;
	    left->IsCat=FALSE;
	    left->type=Tdbl;
	    exprnode_Destroy(right);
		if (op == '%') {
			ExprError(":% is not defined for real values", r);
			return r;
		}
		genop('M');
	}
	else {
		demandnodetype(left, Tlong);
		demandnodetype(right, Tlong);
		left->len = right->loc+right->len-left->loc;
		left->IsCat = FALSE;
		left->type = Tlong;
		exprnode_Destroy(right);
		r =left;
	}
	genop(op);
	return r;
}

	void
gencomp(struct exprnode  *left , struct exprnode  *right, long  rop) {
	long rloc = right->loc+right->len - left->loc;
	if (left->type != right->type) 
		SaveError(":comparison of different types", left->loc, rloc);
	else if (left->type == Tstr)
		{}
	else if (left->type == Tbool) 
		addOp(curComp->curfuncmark, 'B');
	else if (left->type == Tptr) {
		if (rop != predEQ && rop != predNE)
			SaveError(":illegal comparison of pointers", 
					left->loc, rloc);
		addOp(curComp->curfuncmark, 'V');
	}
	else if (left->type == Tlong) 
		addOp(curComp->curfuncmark, 'I');
	else if (left->type == Tdbl) 
		addOp(curComp->curfuncmark, 'R');
	else if ((left->type & 0xF) == Tfunc) {
		if (rop != predEQ && rop != predNE)
			SaveError(":illegal comparison of funcvals", 
					left->loc, rloc);
		addOp(curComp->curfuncmark, '(');
	}
	else 
		SaveError(":comparison is not implemented for this type",
				left->loc, right->loc+right->len - left->loc);
	addOp(curComp->curfuncmark, 't');
}

	Texpr
genvarref(struct varnode  *var) {
	class nesssym *sym = var->sym;
	struct callnode *cnode;
	/* if it is EITHER a function or undefined, try callLoadFuncval */
	if ((sym->flags & (flag_function | flag_undef) )
			 &&  (cnode = callLoadFuncval(var))) 
		return (cnode->rettype << 4) | Tfunc;
	if (sym->flags & flag_function)
		return Tunk;	/* callLoadFuncval failed */
	if ((sym->flags & flag_undef) != 0) {
		SaveError(":undefined variable", var->loc, var->len);
		return Tunk;
	}
	if (sym->flags == (flag_var | flag_builtin)) {
		/* pre-defined identifier */
		callPredefId(sym);
		return sym->type;
	}
	if (sym->type == Tbool)
		addOp(curComp->curfuncmark, 'B');
	else if (sym->type == Tptr)
		addOp(curComp->curfuncmark, 'V');
	else if (sym->type == Tlong)
		addOp(curComp->curfuncmark, 'I');
	else if (sym->type == Tdbl)
		addOp(curComp->curfuncmark, 'R');
	else if ((sym->type & 0xF) == Tfunc)
		addOp(curComp->curfuncmark, '(');
	else if (sym->type != Tstr)
		SaveError(":cannot load variables of this type", 
				var->loc, var->len);
	if (sym->flags & flag_globalvar)
		refSysGlob(curComp->curfuncmark, 'k', 
			nesssym_NGetINode(sym, vardefnode)->addr);
	else
		RememberFixup(refStack(curComp->curfuncmark, 'l', 0), sym);
	return sym->type;
}

	void
genvarstore(struct varnode  *varnode) {
	class nesssym *var = varnode->sym;
	if (var->type == Tbool)
		addOp(curComp->curfuncmark, 'B');
	else if (var->type == Tptr)
			addOp(curComp->curfuncmark, 'V');
	else if (var->type == Tlong)
			addOp(curComp->curfuncmark, 'I');
	else if (var->type == Tdbl)
			addOp(curComp->curfuncmark, 'R');
	else if ((var->type & 0xF) == Tfunc)
			addOp(curComp->curfuncmark, '(');
	else if (var->type != Tstr)
		SaveError(":cannot store into variables of this type", 
					varnode->loc, varnode->len);
	if (var->flags & flag_globalvar)
		refSysGlob(curComp->curfuncmark, 'v', 
				nesssym_NGetINode(var, vardefnode)->addr);
	else
		RememberFixup(refStack(curComp->curfuncmark, 's', 0), var);
	varnode_Destroy(varnode);
}

	struct varnode *
genvarnode(class nesssym  *sym) {
	long loc, len;
	if ((sym->flags & (flag_function | flag_var)) == 0)
		sym->flags |= flag_undef;
	loc = (curComp->tlex)->RecentPosition( 0, &len);
	return varnode_Create(loc, len, sym);
}

	void
genconstref(class nesssym  *constant) {
		if (constant->type == Tlong)
			addOp(curComp->curfuncmark, 'I');
		else if (constant->type == Tdbl)
			addOp(curComp->curfuncmark, 'R');
		else if (constant->type != Tstr)
			ReportError(":cannot load constants of this type", 0);
	refSysGlob(curComp->curfuncmark, 'k', 
			nesssym_NGetINode(constant, vardefnode)->addr);
}

	void
genop(char op) {
	addOp(curComp->curfuncmark, op);
}


/* = = = = = = = = = = = = = = = = = = =
 *    Branch Instructions
 * = = = = = = = = = = = = = = = = = = */
/* setcurfuncmark(m)
	set 'curComp->curfuncmark' to 'm'
	Use This ONLY for Testing 
	and nevent
*/
	void
setcurfuncmark(class nessmark  *m) {
	curComp->curfuncmark = m;
}

/* getcurfuncmark()
	return current value of curfuncmark
	used ONLY by call.c
*/
	class nessmark *
getcurfuncmark() {
	return curComp->curfuncmark;
}



/*
	The operand for a branch opcode is the offset from the opcode to the target.
*/

/* genlabel()
	returns the current location in the object code 
*/
	long
genlabel() {
	if (curComp->Locating) return 0;
	return (curComp->curfuncmark)->GetLength();
}

/* genbranch(op, dest)
	emits to the object code a branch with opcode 'op' to location 'dest'
	returns the location of the opcode of the branch
*/
	long
genbranch(char op, long  dest) {
	long branchloc = genlabel();
	char buf[3];
	short t = dest - branchloc;	/* compute offset */
	long len;

	if (curComp->Locating) {compileLocate(3); return 0;}
	len = (curComp->curfuncmark)->GetLength();

	buf[0] = op;
	buf[1] = ((unsigned short) t) >> 8;
	buf[2] = ((unsigned short) t) & 0xFF;
	((curComp->curfuncmark)->GetText())->InsertCharacters(
			(curComp->curfuncmark)->GetPos()+branchloc, buf, 3);
	(curComp->curfuncmark)->SetLength( len+3);	
	return branchloc;
}

/* fixbranch(loc)
	fixes up the branch op at 'loc' to branch to next generated opcode
*/
	void
fixbranch(long  loc) {
	long dest = genlabel();
	short t = dest - loc;		/* compute offset */
	char buf[2];
	if (curComp->Locating) return;
	buf[0] = ((unsigned short) t) >> 8;
	buf[1] = ((unsigned short) t) & 0xFF;
	((curComp->curfuncmark)->GetText())->ReplaceCharacters( 
			(curComp->curfuncmark)->GetPos()+loc+1, 2, buf, 2);
}



/* = = = = = = = = = = = = = = = = = = =
 *    Predicate Engine
 * = = = = = = = = = = = = = = = = = = */

/* state variables for predicate engine

	The predicate state is saved in a predstatenode having
		fixuploc - fixup loc (for ELIF)
		objloc - current location in object code (for WHILE)
		loc - the beginning of an expression (for compileLocate)
		targetlist - the prior curComp->predtargetlist
		dropthrulist - the prior curComp->preddropthrulist 
		cond - the prior curComp->predcond value 
		construct - a code saying what construct this is in  (see compdefs.hn)
*/

/* predpush(cond, loc, construct)
	Package the current predicate state in a predstatenode and 
	initialize for a new predicate, setting curComp->predcond from 'cond'
	save loc in predstack for compileLocate
*/
	struct predstatenode *
predpush(boolean  cond, long  loc, char  construct) {
	if (pssp - predstack >= MAXEXPRDEPTH - 1) {
		ReportError(":expression is too complicated", 0);
		return NULL;
	}
	pssp ++;
	pssp->fixuploc = 0;
	pssp->objloc = genlabel();
	pssp->loc = loc;
	pssp->targetlist = curComp->predtargetlist;
	pssp->dropthrulist = curComp->preddropthrulist;
	pssp->cond = curComp->predcond;
	pssp->construct = construct;

	/* establish new conditions */
	curComp->predtargetlist = NULL;
	curComp->preddropthrulist = NULL;
	curComp->predcond = cond;		/* store new cond */
	return pssp;
}

	static void
appendgotolists(struct gotonode  **list , struct gotonode  *addon) {
	if (*list == NULL) *list = addon;
	else if (addon == NULL) {}
	else {
		register struct gotonode *t = *list;
		while (t->next != NULL) t = t->next;
		t->next = addon;
	}
}

/* predpop()
	restore the state of the predicate engine from the fields of 'state'
*/
	void
predpop() {
	if (pssp < predstack) {
		ReportError(":COMPILER ERROR: expression stack underflow", 0);
		return;
	}
	curComp->predcond = pssp->cond;
	appendgotolists(&curComp->predtargetlist, pssp->targetlist);
	appendgotolists(&curComp->preddropthrulist, pssp->dropthrulist);
	pssp--;
}

/* predvalue(Ptype)
	Emit code to generate a value from a branching predicate
*/
	void
predvalue(Texpr  *Ptype) {
	long branchloc;

	if (*Ptype != Tbra) 
		return;		/* it is already a value */
	/* convert branching to a value */
	predfixdropthru();
	addOp(curComp->curfuncmark, '1');		/* load TRUE */
	branchloc = genbranch('g', 0);
	predfixtarget();
	addOp(curComp->curfuncmark, '9');		/* load FALSE */
	fixbranch(branchloc);
	*Ptype = Tbool;
}
/* predbranch(expr)
	Emit code to convert boolean value to branching
*/
	void
predbranch(struct exprnode  *expr) {
	if (expr->type == Tbra) 
		return;		/* it is already a branching predicate */
	else if (expr->type != Tbool) {
		ExprError(":expression should be of type Boolean", expr);
		return;
	}
	addOp(curComp->curfuncmark, 'T');	/* generate a comparison */
	predtarget(predEQ);	/* cmp to TRUE and branch */
}

/*
	An element of curComp->preddropthrulist or curComp->predtargetlist is a gotonode having:
		gotoloc - the location to be patched
		next - the next element on the list
*/
/* preddropthru(rop)
	Generates a branch and put a fixup for it on 'preddropthru' list
	opcode depends on curComp->predcond: 'topTbl[rop]' for TRUE, 'fopTbl[rop]' for FALSE
*/
	void
preddropthru(long  rop) {
	char opcode = (curComp->predcond) ? TopTbl[rop] : FopTbl[rop];
	if (opcode == '\n')  return;
	curComp->preddropthrulist = gotonode_Create(genbranch(opcode, 0), curComp->preddropthrulist);
}

/* predtarget(rop)
	Generates a branch and put a fixup for it on 'predtarget' list
	opcode depends on curComp->predcond: 'topTbl[rop]' for TRUE, 'fopTbl[rop]' for FALSE
*/
	void
predtarget(long  rop) {
	char opcode = (curComp->predcond) ? TopTbl[rop] : FopTbl[rop];
	if (opcode == '\n')  return;
	curComp->predtargetlist = gotonode_Create(genbranch(opcode, 0), curComp->predtargetlist);
}


/* fixbranchlist(list)
	generate fixups for all locations listed on 'list'
	cause them to branch to next opcode generated
	Discard list elements.
*/
	static void
fixbranchlist(struct gotonode  * list) {
	while (list != NULL) {
		struct gotonode *next = list->next;
		fixbranch(list->gotoloc);
		gotonode_Destroy(list);
		list = next;
	}
}

/* predfixdropthru()
	Fixes all branches on list 'curComp->preddropthrulist' to branch to 
	current position in object code.
*/
	void
predfixdropthru() {
	fixbranchlist(curComp->preddropthrulist);
	curComp->preddropthrulist = NULL;
}

/* predfixtarget()
	Fixes all branches on list 'curComp->predtargetlist' to branch to 
	current position in object code.
*/
	void
predfixtarget() {
	fixbranchlist(curComp->predtargetlist);
	curComp->predtargetlist = NULL;
}

/* predexit(construct)
	generate a branch to the 'target' location for the enclosing construct
	construct value must be 'I' for if or 'W' for while
	The curComp->predtargetlist we want may be the current one or the one on the stack
	in the frame one more recent than the most recent instance of 'construct'.
*/
	void
predexit(char  construct) {
	struct gotonode **targetlist;
	struct predstatenode *sp;

	if (pssp->construct == construct)
		targetlist = &curComp->predtargetlist;
	else {
		/* scan the stack for desired construct  
			when the targetlist is empty, we must be in the Elsepart already*/
		for (sp = pssp-1; sp >= predstack; sp--) 
			if (sp->construct == construct && (sp+1)->targetlist != NULL)
				break;
		if (sp >= predstack)
			targetlist = &((sp+1)->targetlist);
		else {
			ReportError(":Not within a suitable context", 0);
			return;
		}
	}
	*targetlist = gotonode_Create(genbranch('g', 0), *targetlist);
}


/*  = = = = = = = = = = = = = = = = = = =
 *    symbols and variables
 * = = = = = = = = = = = = = = = = = =*/

/*demandsymboltype(sym, type) 
	Check that the symbol is of the required type 
	and give an error message if not.
	Cannot generate coercions because the symbol is not on the stack.
*/
	void
demandsymboltype(class nesssym  *sym, Texpr  type) {
	if (sym->type != type)
		ReportError(":wrong type of variable", -1);
} 

/*demandnodetype(node, type) 
	Check that the exprnode 'node' specifies the required 'type'.
	and give an error message if not.
*/
	void
demandnodetype(struct exprnode  *node, Texpr  type) {
	if (node->type != type)
		ExprError(":expression is of the wrong type", node);
}

	struct varnode *
varIsStorable(struct varnode  *var) {
	class nesssym *sym = var->sym;
	if ((sym->flags & (flag_var | flag_undef | flag_builtin)) != flag_var) {
		if (sym->flags & flag_function)
			SaveError(":cannot assign to function name", 
					var->loc, var->len);
		else if (sym->flags & flag_builtin)
			SaveError(":cannot assign to predefined name", 
					var->loc, var->len);
		else
			SaveError(":refers to an undefined variable", 
					var->loc, var->len);
	}
	return var;
}

	struct varnode *
varIsFunction(struct varnode  *var) {
	class nesssym *sym = var->sym;
	if ((sym->flags & flag_var) != 0 && (sym->type & 0xF) == Tfunc) {
		/* will call a funcval */
		/* load variable value onto stack */
		genvarref(var);		
	}
	else if ((sym->flags & (flag_var | flag_const)) != 0) {
		ReportError(":not a proper function name", -1);
		var->sym = NULL;
	}
	else if (sym->flags == flag_undef) 
		/* unknown name:  assume it is a forward function call 
			or a call to an external function */
		callUnknown(sym);
	return var;
}

	class nesssym *
uniqueinscope(class nesssym * var, unsigned long  flags, long  tokoff) {
	static long junknum = 91;
	 /* if var->flags is not zero, we have some old symbol
		and need to redefine it.
	*/
	if (var->flags != flag_undef && (var)->NGetScope() 
					== curComp->scopes[curComp->scopex]) {
		/* redefining a symbol already defined in this function */
		char buf[12];
		ReportError(":previously defined", tokoff);
		sprintf(buf, "X:%d", junknum++);
		var = nesssym::NDefine(buf, var, 
				curComp->scopes[curComp->scopex]); 
	}
	else if (var->flags != flag_undef) {
		/* redefining something global, use curScope to avoid conflict  */
		var = nesssym::NDefine((var)->NGetName(), var,
				curComp->scopes[curComp->scopex]); 
		var->toknum = curComp->idtok;
	} 
	var->flags = flags; 
	return var;
}

/* process a list of id's being declared.  
	set flags and kind to given values. 
	setup the vardefnode and callnode for Tfunc variables
*/
	void
ProcessDeclList(Texpr  type, class nesssym  *idlist, long  flags) {
	class nesssym *id;
	struct vardefnode *vnode;
	for (id = idlist; id != NULL; id = id->next) {
		id-> flags = flags;
		id->type = type;
		vnode = vardefnode_Create(0, NULL);
		nesssym_NSetINode(id, vardefnode, vnode);
		if ((type & 0xF) == Tfunc) {
			struct callnode *c = (struct callnode *)malloc(
					sizeof(struct callnode));
			c->variety = callNone;
			c->Sym = id;  /* bogus */
			c->where.offset = 0;
			c->requiredclass = NULL;
			c->rettype = type >> 4;
			c->nargs = -99;		/* unknown as yet */
			vnode->sig = c;
		}
	}
}

	class nesssym *
genParmDecl(class nesssym  *sym  , Texpr  type) {
	if (sym->flags == (flag_var | flag_parmvar | flag_forward)) {
		/* we are now processing the real decl of a FORWARD func */
		if (type != sym->type)
			ReportError(":type does not match forward declaration",
					 -1);
	}
	else {
		sym = uniqueinscope(sym,  flag_var | flag_parmvar, 0);
		ProcessDeclList(type, sym, flag_var | flag_parmvar); 
	}
	return sym;
}


/* genCheckEndtag(tag, desired)
	checks that the tag is the one desired
	pop the predstate stack to get to the desired one
		or until get to function or extend
*/
	void
genCheckEndtag(class nesssym  *tag, long  desired) {
	long tagtok;
	char *desiredname;
	char buf[300];
	char constructForTag;

	tagtok = tag->toknum;
	if (tagtok == desired)
		return;
	if (desired == ON && (tagtok == EVENT  ||  tagtok == MENU  
				||  tagtok == MOUSE || tagtok == KEYS))
		return;	/* will be dealt with in nevent */

	if (desired == FUNCTION) {
		constructForTag = 'L';  desiredname = "function";
	}
	else if (desired == EXTEND) {
		constructForTag = 'X';  desiredname = "extend";
	}
	else if (desired == IF) {
		constructForTag = 'I';  desiredname = "if";
	}
	else if (desired == WHILE) {
		constructForTag = 'W';  desiredname = "while";
	}
	else if (desired == ON) {
		constructForTag = 'V';  desiredname = NULL;
	}
		/* error message for ON will come from neventFinishEvent */

	if (desiredname != NULL)  {
		sprintf(buf, "*Should be:   end %s.  Trying to fix by inventing ends.\n", 
				desiredname);
		ReportError(freeze(buf), -1);
	}
	while (pssp->construct != constructForTag
			&& pssp->construct != 'X'
			&& pssp->construct != 'L')
		predpop();
}


/* the next two functions are used by the grammar to save 
	the function state for the global init function 
	They provide only ONE level of save restore. 
*/
	void
genSaveFuncState() {
	curComp->savedCurfuncmark = curComp->curfuncmark;
	curComp->savedVarFixups = curComp->varfixups;
	curComp->LocationAdvancing = ! curComp->LocationAdvancing;
	compPopScope();	/* scope is saved in the funcnode */
}
	void
genRestoreFuncState(class nesssym  *func)
	 {
	curComp->curfuncmark = curComp->savedCurfuncmark;
	curComp->varfixups = curComp->savedVarFixups;
	curComp->LocationAdvancing = ! curComp->LocationAdvancing;
	compPushScope(nesssym_NGetINode(func, funcnode)->ownscope);
}
