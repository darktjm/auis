/* ********************************************************************** *\
 *         Copyright Carnegie Mellon University 1992, 1993 - All Rights Reserved
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

/* compile.c
	Compile a ness 
*/
/*
 *    $Log: ncompile.C,v $
 * Revision 1.7  1995/02/27  02:30:48  rr2b
 * Initialize LocationAdvancing to a known value.
 * BUG
 *
 * Revision 1.6  1994/08/11  02:58:05  rr2b
 * The great gcc-2.6.0 new fiasco, new class foo -> new foo
 *
 * Revision 1.5  1994/04/10  23:50:56  wjh
 * TRIV ness.gra->rules.gra; usr parser->Parse
 *
 * Revision 1.4  1993/12/16  20:38:01  wjh
 * curNess ==> curComp->ness
 *
 * Revision 1.3  1993/11/08  18:48:48  gk5g
 * #ifdef'ed out some good code that g++ barfs on:
 * curNess = curComp ? curComp->ness : NULL
 * BUILD
 *
    elided log   9 April 1994   -wjh
 *
 * Revision 1.1  1992/12/03  03:01:34  wjh
 * Initial revision
 *
 */

#include <andrewos.h>
#include <ctype.h>

#include <im.H>
#include <environ.H>
#include <environment.H>
#include <text.H>
#include <toksym.H>
#include <tlex.H>

#include <ness.H>
#include <rules.H>


/* declarations needed for ness.tlc */

#if 0		/* XXXX ???? is this needed in C++ version ? */
#undef environment_GetParent
#define (env)->GetParent() \
		((class environment *)(env)->parent)
#endif

static toksym_scopeType GrammarScope = toksym_GLOBAL;
static class nesssym *SymProto;
/* struct ATKregistryEntry *textClass = NULL; from interp.c */
static long GenNum = 0;		/* generates a sequence of integer values */
static class toksym *EmptyStringToken;

#define SETPOS(sym) \
	((sym)->loc = (tlex)->GetTokPos(), \
	(sym)->len = (tlex)->GetTokEnd()-(tlex)->GetTokPos()+1)
#define CURCOMP ((struct compilation *)(tlex->rock))
static void parsepragmat(class ness  * self, char  *prag);

#include <ness.tlc>



NO_DLL_EXPORT struct compilation *curComp = NULL;	/* compilation in progress */

static struct compilation *Idle = NULL;
static class nesssym *proto;
static int idtok;


static boolean FoundIt = FALSE;	/* TRUE if search satisfied */
static long FoundLoc, FoundLen;	/* where we found it */


void compileLocate(long  bytes);
void compileStmtStart(long  tokeninx);
static void EnterReservedWord(void  *rock, const char  *w, int  i);
/* (above) static void parsepragmat(class ness  * self, char  *prag); */
static void compileInit();
static void doCompile(class ness  *ness, long  pos , long  len, boolean  locating);
struct errornode * compile(class ness  *ness, long  pos , long  len);
boolean compileForLocation(class ness  *ness, long  pos , long  len, unsigned long  objloc, boolean  LocateInInitFunc, unsigned long  *Ploc , unsigned long  *Plen);
nesssym_scopeType compNewScope();
void compPopScope();
void compPushScope(nesssym_scopeType  scope);


/* compileLocate(bytes)
	Test to see if we have found the offset we wanted. 
	Update the notion of what the current offset is.
*/
	void
compileLocate(long  bytes) {
	if ( ! curComp->LocationAdvancing)
		return;
	if (! FoundIt && curComp->CurrOffset >= curComp->Sought) {
		long loc, len;
		FoundIt = TRUE;
		loc = (curComp->tlex)->RecentPosition( 0, &len); 
		FoundLoc = (pssp->loc > curComp->StmtStart)
			? pssp->loc : curComp->StmtStart;
		FoundLen = loc+len - FoundLoc;
	}
	curComp->CurrOffset += bytes;
}

/* compileStmtStart(tokeninx)
	save location of statment start in source for errors 
*/
	void
compileStmtStart(long  tokeninx) {
	if (curComp->Locating) 
		curComp->StmtStart = (curComp->tlex)->RecentPosition( tokeninx, 0); 
}


	static void
EnterReservedWord(void  *rock, const char  *w, int  i) {
	class nesssym *s;
	boolean new_c;
	s = nesssym::NLocate(w, SymProto, GrammarScope, &new_c);
	s->toknum = i;
}

	static void
parsepragmat(class ness  * self, char  *prag) {
	const char enable[] = "enable class access";
	const char Ness[] = "Ness";
	
	if (strncmp(prag, enable, strlen(enable)) == 0)
		/* --$enable class access */
		self->ClassEnabled = TRUE;
	else if (strncmp(prag, Ness, strlen(Ness)) == 0) {
		/*  --$Ness <version-number>
			 check the version number */
		while (*prag && !isdigit(*prag)) prag++;
		if (*prag) 
			self->syntaxlevel = atoi(prag);
		else {
			/* Illegal $Ness pragmat */
			self->syntaxlevel = UNSPECIFIEDSYNTAXLEVEL;
		}
	}
}

	static void 
compileInit() {
	if (Idle == NULL) {
		/* create a new compilaton record */
	    Idle = (struct compilation *)malloc(sizeof(struct compilation));
		Idle->tlex = tlex::Create(&ness_tlex_tables, NULL, NULL, 0, 0);
		Idle->parser = new rules;
		(Idle->parser)->SetRock(Idle);
		Idle->next = NULL;
		Idle->LocationAdvancing=FALSE;	
	}
	if (GrammarScope == toksym_GLOBAL) {
		boolean new_c;
		class toksym *t;

		GrammarScope = toksym::TNewScope(toksym_GLOBAL);
		SymProto = new nesssym;
		initializeEnvt();	/* initialize Ness interpreter */

		(Idle->parser)->EnumerateReservedWords( 
				EnterReservedWord, NULL);
		idtok = (Idle->parser)->TokenNumberFromName( "setID");
		proto = new nesssym;
		callInit(GrammarScope, idtok, proto);

		t = toksym::TLocate("\"", proto, GrammarScope, &new_c);
		t->info.str = NULL;
		t->toknum = (Idle->parser)->TokenNumberFromName( 
			"setSTRINGCON");
		t->loc = 0;  t->len = 0;
		EmptyStringToken = t;
	}
}

	static void
doCompile(class ness  *ness, long  pos , long  len, boolean  locating) {
	curComp->ness = ness;
	(curComp->tlex)->SetRock(curComp);
	curComp->idtok = idtok;
	curComp->proto = proto;

	curComp->scopex = 0;
	curComp->scopes[0] = ness->outerScope;
	curComp->predcond = FALSE;	
	curComp->predtargetlist = NULL;
	curComp->preddropthrulist = NULL;
	curComp->varfixups = NULL;
	curComp->freefixups = NULL;
	curComp->Locating = locating;

	(curComp->tlex)->SetText( ness, pos, len);

	ness->compilationid = im::GetWriteID();
	ness->AttrDest = &ness->globals;  /* where to put defined symbols */
	SetupErrorHandling(ness);
	ness->CurrentObject = NULL;

	(curComp->parser)->Parse(tlex::LexFunc, curComp->tlex);
}


	struct errornode *
compile(class ness  *ness, long  pos , long  len) {
	struct compilation *saveComp = curComp;  /* save to restore below */
	compileInit();
	curComp = Idle;
	Idle = curComp->next;

	ness->outerScope = nesssym::NNewScope(GrammarScope);
	ness->constScope = nesssym::NNewScope(GrammarScope);

	doCompile(ness, pos, len, FALSE);

	curComp->next = Idle;
	Idle = curComp;
		/* XXX are there resources to release here ??? */
	curComp = saveComp;	/* restore prior compile */
	return ness->ErrorList;
}

/* compileForLocation(ness, pos, len, objloc, LocateInInitFunc, Ploc, Plen)
	run the compiler in a mode to find the source code that generated
	the code at objloc.  Set the position and length of the source 
	expression into *Ploc and *Plen.
	Return TRUE iff found the location.
	curComp->LocationAdvancing toggles back and forth in 
		genSaveFuncState and genRestoreFuncState.
		If it is true, the offset advances in compileLocate.  
		Otherwise not.  Initially curComp->LocationAdvancing 
		is True iff the location we want is within the InitFunc.
*/
	boolean
compileForLocation(class ness  *ness, long  pos , long  len, unsigned long  objloc, 
		boolean  LocateInInitFunc, unsigned long  *Ploc , unsigned long  *Plen) {
	struct compilation *saveComp = curComp;  /* save to restore below */

	compileInit();

	curComp = Idle;
	Idle = curComp->next;

	curComp->Sought = objloc;
	FoundIt = FALSE;
	curComp->CurrOffset = 0;
	curComp->StmtStart = 0;
	curComp->LocationAdvancing = LocateInInitFunc;
	pssp++;		/* allocate a dummy so pssp->loc is always defined */
	pssp->loc = 0;

	doCompile(ness, pos, len, TRUE);

	compileLocate(0);
	curComp->next = Idle;
	Idle = curComp;
		/* XXX are there resources to release here ??? */
	curComp = saveComp;	/* restore prior compile */

	*Ploc = FoundLoc;
	*Plen = FoundLen;
	return FoundIt;
}



/* compNewScope()
	pushes the current scope and makes a new one
	returns the new scope 
*/
	nesssym_scopeType
compNewScope() {
	if (curComp->scopex+1 >=
 			(int)(sizeof(curComp->scopes)/sizeof(curComp->scopes[0]))) {
		/* XXX compiler errror */
		ReportError(":disastrous compile error: scope stack overflow", 0);
		return curComp->scopes[curComp->scopex];
	}
	curComp->scopex++;
	curComp->scopes[curComp->scopex] 
		= nesssym::NNewScope(curComp->scopes[curComp->scopex-1]);
	return 	curComp->scopes[curComp->scopex];
}

/* compPopScope()
	revert to a prior scope
*/
	void
compPopScope() {
	if (curComp->scopex <= 0) return;
	curComp->scopex--;
}

/* compPushScope(scope)
	enter a scope 
	(used for the global initialization function)
*/
	void
compPushScope(nesssym_scopeType  scope) {
	if (curComp->scopex+1 >= 
			(int)(sizeof(curComp->scopes)/sizeof(curComp->scopes[0]))) {
		ReportError(":disastrous compile error: scope stack overflow", 0);
		return;
	}
	curComp->scopex++;
	curComp->scopes[curComp->scopex] = scope;
}
