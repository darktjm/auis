/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
	Copyright Carnegie Mellon University 1992, 1993 - All Rights Reserved
\* ********************************************************************** */
/* error.c
	error processing for ness

	Entry points:

general:

codelocFind(loc)

compile time errors:

SetupErrorHandling(ness)
SaveError(msg, loc, len)
ReportError(msr, index)
ExprError(msg, expr)
errorfromparse(severity, msg)
errsynch()
isFuncStart(maybeend, ncheck)

run time errors:

LocateErrorFunc(loc, base, msg, ness)
MapRunError(ness)

*/

/*
 * $Log: error.C,v $
 * Revision 1.7  1994/04/16  21:48:41  rr2b
 * Updated for the demise of the parse class and the move of ParseNumber to the parser
 * class.
 * BUG
 *
 * Revision 1.6  1994/04/10  23:53:25  wjh
 * TRIV change errorfromparse parameters; switch to rules.H
 * TRIV change errorsynch to boolean; change from parse_ to parser_
 * TRIV NextToken->LexFunc
 *
 * Revision 1.5  1994/04/10  23:50:56  wjh
 * TRIV ness.gra->rules.gra; usr parser->Parse
 *
 * Revision 1.4  1993/12/21  21:54:12  wjh
 * fixed && to &.  Possible core dump.
 *
 * Revision 1.3  1993/12/16  20:38:01  wjh
 * curNess ==> curComp->ness
 *
 * Revision 1.2  1993/06/29  18:14:34  wjh
 * checkin to update revision to force checout to source tree
 *
 * Revision 1.1  1993/06/21  19:30:36  wjh
 * Initial revision
 *
 * Revision 1.24  1993/02/17  06:10:54  wjh
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
 * Revision 1.23  1992/12/16  04:10:48  wjh
 * addjust location reported with error message so it excludes any warning text
 * .
 *
 * Revision 1.22  1992/12/14  20:49:20  rr2b
 * disclaimerization
 *
 * Revision 1.21  1992/12/08  04:14:46  wjh
 * fixed so it will not crash when a runtime error occurs
 * 	moved the recompile to locate error operations to compile.c
 * .
 *
 * Revision 1.20  1992/11/26  02:39:52  wjh
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
 * Revision 1.19  91/12/03  22:36:59  gk5g
 * Patch submitted by Todd Inglett:
 * I found a nasty little bug in MapRunError() in atk/ness/objects/error.c. 
 * The errornode that was picked off the front of the list didn't have its 
 * ``next'' field NULLed, so it remained dangling into errornode's that are 
 * free'd later in the routine.  I also moved the check for NULL before the 
 * errornode is dereferenced (just in case).
 
   log purged Nov, 1992 -wjh
  
 * Creation 0.0  88/05/28 10:16:00  wjh
 * Initial creation by WJHansen
 * 
*/

/* the first byte of an error message indicates whether (:) it is in static memory
		(which means it never changes)
 	or (*) is the result of 'freeze' (which is used when the message is 
		newly generated for each occurrence)
*/

#include <andrewos.h>

#include <parser.H>
#include <tlex.H>

#include <ness.H>
#include <nessmark.H>
#include <rules.H>


struct errornode * errornode_New();
struct errornode * errornode_Create(class ness  *ness, long  loc , long  len , long  execloc, const char *msg, boolean  ownmsg, struct errornode  *next);
void  errornode_Destroy(struct errornode  *enode);
static boolean HasLoc(class nesssym  *s, long  loc);
class nesssym * codelocFind(long  loc);
void SetupErrorHandling(class ness  *ness);
void ReportError(const char *msg, long  index);
void ExprError(const char *msg, struct exprnode  *expr);
void SaveError(const char *msg, long  loc , long  len);
void errorfromparse(int  severity, const char *msg);
boolean errorsynch(long  index);
boolean isFuncStart(int nexttok, long ncheck);
void MapRunError(class ness  *ness);
struct errornode * LocateErrorFunc(const char *loc , unsigned const char *base, const char *msg, class ness  *ness);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
	errornode functions
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


	struct errornode *
errornode_New() {
	return (struct errornode *)malloc(sizeof(struct errornode));
}

	struct errornode *
errornode_Create(class ness  *ness, long  loc , long  len , long  execloc, 
		const char *msg, boolean  ownmsg, struct errornode  *next) {
	struct errornode *enode = (struct errornode *)malloc(sizeof(struct errornode));
	enode->where = (ness)->CreateMark( loc, len);
	enode->execloc = execloc;
	enode->msg = msg;
	enode->ownmsg = ownmsg;
	enode->next = next;
	return enode;
}


	void 
errornode_Destroy(struct errornode  *enode) {
	if (enode->ownmsg) free((char *)enode->msg);
	((class ness *)(enode->where)->GetObject())->RemoveMark( enode->where);
	delete enode->where;
	free(enode);
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
	OBJECT CODE LOCATION ROUTINES
	Map from object code location to the nesssym
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
	static boolean
HasLoc(class nesssym  *s, long  loc) {
	class nessmark *resmark = 
		ness_Globals[nesssym_NGetINode(s, funcnode)->
				SysGlobOffset].e.s.v;
	return (resmark &&  loc >= (resmark)->GetPos()  
			&&  loc < (resmark)->GetEndPos());
}

/* codelocFind(loc)
	find the nesssym corresponding to 'loc' in object code 

	scan NessList and scan globals for each Ness  
		(and scan children for each XObject)
*/
	class nesssym *
codelocFind(long  loc) {
	class ness *ness;
	class nesssym *g, *xg;

	for (ness = ness::GetList(); ness != NULL; ness = ness->next) 
	  if (ness->compiled)
	    for (g = ness->globals;  g != NULL;  g = g->next) 

		if (g->flags == (flag_function | flag_ness)) {
			if (HasLoc(g, loc)) return g;
		}
		else if (g->flags == flag_xobj)
			for (xg = nesssym_NGetINode(g, objnode)->attrs;
					xg != NULL;
					xg = xg->next) 
				if (xg->flags == (flag_function | flag_ness | flag_xfunc)
						||  xg->flags == flag_event)
					if (HasLoc(xg, loc)) return xg;

	return NULL;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
	COMPILE TIME ERRORS
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static long lastloc, lastlen;	/* loc/len of last ":. . . restart" */
static boolean Restarted;

	void
SetupErrorHandling(class ness  *ness) {
	lastloc = -1;
	Restarted = FALSE;
	while (ness->ErrorList != NULL) {
		struct errornode *t = ness->ErrorList;
		ness->ErrorList = t->next;
		errornode_Destroy(t);
	}
	ness->ErrorList = NULL;
}


/* ReportError(msg, index)
	calls SaveError after getting the loc and len
		from the token at 'index'
*/
	void
ReportError(const char *msg, long  index) {
	long loc, len;
	loc = (curComp->tlex)->RecentPosition( index, &len);
	SaveError(msg, loc, len);
}

/* ExprError(msg, expr)
	calls SaveError after getting the loc and len from 'expr'
*/
	void
ExprError(const char *msg, struct exprnode  *expr) {
	SaveError(msg, expr->loc, expr->len);
}

/* SaveError(msg, loc, len)
	adds the message to the list of errors
	list is maintained with earliest message first
*/
	void
SaveError(const char *msg, long  loc , long  len) {
	struct errornode *err;
	class ness *curNess = curComp->ness;

	err = errornode_Create(curNess, loc, len, 0, msg, (*msg == '*'), NULL);
	if (curNess->ErrorList == NULL)
		curNess->ErrorList = err;
	else {
		struct errornode *t;
		for (t = curNess->ErrorList; t->next != NULL; t = t->next) 
			{}
		t->next = err;
	}
}


	void
errorfromparse(int severity, const char *msg) {
	/* struct toksym *token; */
	long loc, len;
	static const char syntaxerror[] = "Syntax error";

	loc = (curComp->tlex)->RecentPosition( 0, &len);

	/* don't report error if we have Restarted from a former error 
			at the same loc/len */
	if (Restarted && loc == lastloc  &&  len ==lastlen)
		return;

	if (strcmp(msg, syntaxerror) == 0)
		SaveError(":syntax error", loc, len);
	else {
		char buf[300];
		buf[0] = '*';
		strncpy(buf+1, msg, 298);
		buf[299] = '\0';	/* truncate if needed */
		SaveError(strdup(buf), loc, len);
	}
	Restarted = FALSE;
	if (severity & parser_FREEMSG)
		free((char *)msg);
}

/* errorsynch(index)
	generate an error message that parser will restart after error 
	However, if it is the same restart token as the last, skip it
	returns value to return to parse
*/
	boolean
errorsynch(long  index) {
	long loc, len;

	loc = (curComp->tlex)->RecentPosition( index, &len);

	if ( ! Restarted || loc > lastloc || len != lastlen) { 
		/* this is the first restart at this token */
		Restarted = TRUE;
		lastloc = loc;
		lastlen = len;
		(curComp->tlex)->Repeat( index);
		SaveError(": . . . restart with token", loc, len); 
		return TRUE;
	}
	return FALSE;
}

/* isFuncStart(nexttok, ncheck)
	check to see if this token may be an error restart point
	if 'nexttok' is not 0, it must be the token type
		of the subsequent token
		moreover, the preceding token must not be END
	furthermore, in order to be a restart, one of the 'ncheck' tokens
			starting with this one must be unindented
	The position in the token stream is unaffected; the next lex_NextToken
	will get the token it would have returned had this function not been called
*/
	boolean
isFuncStart(int nexttok, long ncheck) {
	class nesssym *ttok;
	long n;
	int ttoknum;
	if (nexttok) {
		/* reject if predecessor is "end" or successor is not nexttok*/
		(curComp->tlex)->Repeat( -1);	 /* reread tokens */
		ttoknum = tlex::LexFunc(curComp->tlex, (void *)&ttok); // prev
		tlex::LexFunc(curComp->tlex, (void *)&ttok); 	// curr
		if (ttoknum == END)  return FALSE;
		ttoknum = tlex::LexFunc(curComp->tlex, (void *)&ttok); // next
		(curComp->tlex)->Repeat(0);	/* reset to current token */
		if (ttoknum == 0 /* EOF */  ||  ttoknum != nexttok)
			return FALSE;
	}
	/* at this point the input stream position is unchanged */
	for (n = 0; n < ncheck; n++) {
		if ((curComp->tlex)->RecentIndent( 0) == 0) {
			/* succeed */
			if (n>0) (curComp->tlex)->Repeat( 1-n);
					/*  ( n==1 ==> 0;  n==2 ==> -1 ) */
			return TRUE;
		}
		if (tlex::LexFunc(curComp->tlex, (void *)&ttok) == 0) // EOF 
			return FALSE;
	}
	/* found none unindented: fail */
	(curComp->tlex)->Repeat( 2-ncheck);	
			/*  ( ncheck==2 ==> 0;   ncheck==3 ==> -1 )  */
	return FALSE;
}


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
	RUNTIME ERRORS
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/* MapRunError (ness)
	Modify the errornode ness->ErrorList to have the source code location 
	corresponding to the object code location.
*/
	void
MapRunError(class ness  *ness) {
	struct funcnode *fnode;
	class nesssym *errsym;
	class nessmark *objcode;
	struct errornode *err;
	unsigned long loc, len;

	err = ness->ErrorList;
	if (err == NULL) 
		return;
	ness->ErrorList = err->next;	/* remove err from ErrorList
				before it is cleared by SetUpErrorHandling */
	err->next = NULL;
	errsym = codelocFind(err->execloc);
	if (errsym == NULL) 
		return;

	fnode = nesssym_NGetINode(errsym, funcnode);	/* (might be
			an event node.  KLUDGE leading fields are the same) */
	objcode = ness_Globals[fnode->SysGlobOffset].e.s.v;
	if (errsym->flags == (flag_function | flag_ness) ||
	    errsym->flags == (flag_function | flag_ness | flag_builtin)) {
		/* top level function */
		if (ness != errsym->parent.ness)
			fprintf(stderr, "MapRunErr - ness mismatch 1\n");
	}
	else {
		/* event or function defined in an 'extend' */
		if (ness != errsym->parent.nesssym->parent.ness)
			fprintf(stderr, "MapRunErr - ness mismatch 2\n");
	}

	(err->where)->SetPos( (fnode->where)->GetPos());
	(err->where)->SetLength( (fnode->where)->GetLength());

	if (compileForLocation(ness, 
			(fnode->where)->GetPos(), 
			(fnode->where)->GetLength(),
			err->execloc - (objcode)->GetPos(),
			errsym == ness->InitFunc, &loc, &len)) {
		/* found the line */
		(err->where)->SetPos( loc);
		(err->where)->SetLength( len);
	}

	/* delete compilation error messages */
	while (ness->ErrorList != NULL) {
		struct errornode *t = ness->ErrorList;
		ness->ErrorList = t->next;
		errornode_Destroy(t);
	}
	ness->ErrorList = err;
}


/* LocateErrorFunc(loc, base, msg, ness)
	Generates an errornode of 'msg' for the point of error.
	The error occurred at object code location 'loc', where
	the start of the object code is 'base'.
	If the top stack frame is in the 'ness', return the generated message.
	If the top stack frame is not in the 'ness', the actual failure
		is attached to its own ness, which is then exposed.
		Then the stack frames are traversed looking for one that 
		is in the 'ness' and the returned error indicates a 
		failure in the library routine called at that point.
*/
	struct errornode *
LocateErrorFunc(unsigned const char *loc , unsigned const char *base, const char *msg,
		class ness  *ness) {
	class nesssym *wheresym;
	class ness *whereness;
	struct frameelt *frame;

	wheresym = codelocFind(loc - base);
	if (wheresym == NULL) 
		return errornode_Create(ness, 0, 0, loc - base, msg, (*msg == '*'), NULL);
	whereness = (wheresym->flags == (flag_function | flag_ness) ||
		     wheresym->flags == (flag_function | flag_ness | flag_builtin))
				? wheresym->parent.ness
				: wheresym->parent.nesssym->parent.ness;
	if (whereness == ness) 
		/* the error is in the ness itself */
		/* the msg will be MapRunError'ed in the ness */
		return errornode_Create(whereness, 0, 0, loc - base, msg, (*msg == '*'), NULL);

	/* the message is not in the root ness.
		attach an error message to the ness with the error */
	whereness->ErrorList = errornode_Create(whereness, 0, 0, loc - base, msg, (*msg == '*'),
				whereness->ErrorList);
	MapRunError(whereness);
	(whereness)->Expose();

	/* go up stack looking for the root ness to give it a message */
	frame = FramePtr;
	while (TRUE) {
		if (frame->returnaddress == NULL) 
			/* this is the outermost frame, give up */
			return errornode_Create(ness, 0, 0, loc - base, 
				":error in subroutine called from unknown location", 
				FALSE, NULL);
		wheresym = codelocFind(frame->returnaddress - base - 2);
		whereness = (wheresym->flags == (flag_function | flag_ness) ||
			     wheresym->flags == (flag_function | flag_ness | flag_builtin))
				? wheresym->parent.ness
				: wheresym->parent.nesssym->parent.ness;
		if (whereness == ness) 
			return errornode_Create(whereness, 0, 0, frame->returnaddress - base - 2,
				":a library function had an error", 
				FALSE, NULL);
		frame = frame->prevframe;
	} 
}
