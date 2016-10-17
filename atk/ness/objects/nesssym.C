/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
	Copyright Carnegie Mellon University 1993 - All Rights Reserved
\* ********************************************************************** */

/*
 *    $Log: nesssym.C,v $
 * Revision 1.8  1996/05/17  20:53:12  robr
 * Fixed to include aaction.H and use Destroy method instead of delete.
 *
 * Revision 1.7  1995/12/07  16:41:27  robr
 * Support for exporting ness functions to the proctable.
 *
 * Revision 1.6  1995/02/27  02:31:32  rr2b
 * Ensure that function and event markers are removed from the ness text before
 * being deleted.
 * BUG
 *
 * Revision 1.5  1994/02/05  03:09:03  rr2b
 * Fixed bogus usage of Destroy on a non-traced object.
 * Fixed jump past initializer.
 * BUG
 *
 * Revision 1.4  1993/12/21  22:36:21  wjh
 * nope.  Don't destroy callnode pointed at from funcnode.
 *
 * Revision 1.3  1993/12/21  21:54:12  wjh
 * fixed destruction of funcnodes and eventnodes to destroy components
 *
 * Revision 1.2  1993/06/29  18:06:33  wjh
 * checkin to force update
 *
 * Revision 1.1  1993/06/21  20:43:31  wjh
 * Initial revision
 *
 * Revision 1.11  1993/02/17  06:10:54  wjh
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
 * Revision 1.10  1992/12/15  21:38:20  rr2b
 * more disclaimerization fixing
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
 * Revision 1.7  92/06/05  16:39:31  rr2b
 * added code to ensure proper deallocation of ness symbols
 * 
 * Revision 1.6  1991/09/12  16:26:37  bobg
 * Update copyright notice and rcsid
 *
 * Revision 1.5  1989/06/01  16:01:17  wjh
 * campus release version
 *
 * Revision 1.1  88/10/21  11:01:09  wjh
 * Initial revision
 *  
 * Revision 1.0  88/07/14  13:22:05WJHansen
 * Copied from sym.c and discarded EVERYTHING
 */

#include <andrewos.h>
#include <ness.H>
#include <aaction.H>
	
ATKdefineRegistryNoInit(nesssym, toksym);


nesssym::nesssym()  {
	this->next = NULL;
	this->parent.ness = NULL;
	this->flags = flag_undef;
	this->type = Tunk;
	THROWONFAILURE( TRUE);
}

nesssym::~nesssym() {
	switch (this->flags) {

	case flag_function | flag_ness:
		/* top level function in a script (funcnode) */
	case flag_function | flag_ness | flag_forward:
		/* function declared FORWARD  (funcnode) */
	case flag_function | flag_ness | flag_xfunc:
		/* function within an 'extend' (funcnode) */
	case flag_function | flag_ness | flag_forward | flag_xfunc: {
		/* function declared FORWARD in 'extend'  (funcnode) */
		struct funcnode *fnode = nesssym_NGetINode(this, funcnode);
		if (fnode->where) {
		    if(!fnode->where->ObjectFree() && fnode->where->GetObject()) {
			class ness *no=(class ness *)fnode->where->GetObject();
			no->RemoveMark(fnode->where);
		    }
		    delete (fnode->where);
		}
		if(fnode->pe) {
		    delete fnode->procname;
		    aaction *act=proctable::GetAction(fnode->pe);
		    if(act) act->Destroy();
		    if(fnode->oldcall && fnode->oldcall->variety==callPE && fnode->oldcall->where.pe) {
			*fnode->pe=*fnode->oldcall->where.pe;
			delete fnode->oldcall->where.pe;
		    } else proctable::DeleteProc(fnode->pe);
		    free(fnode->oldcall);
		}
		funcnode_Destroy(fnode);
		break;
	    }
	case flag_function | flag_proctable:
		/* function from proctable (callnode) */
	case flag_function | flag_classdefn:
		/* function from class (callnode) */
	case flag_function | flag_forward:
		/* possibly a forward func call (callnode) */
		/* callnode_Destroy(nesssym_NGetINode(self, callnode)); */
		free(nesssym_NGetINode(this, callnode));
		break;

	case flag_xobj:
		/* an extended object (objnode) */
		objnode_Destroy(nesssym_NGetINode(this, objnode));
		break;

	case flag_event: {
		/* an event (eventnode) */
		struct eventnode *n = nesssym_NGetINode(this, eventnode);
		if (n->TriggerHolder != NULL)
			(n->TriggerHolder)->DeleteRecipient(
				atom::Intern(n->spec), n->parentness);
		if (n->where) {
		    if(!n->where->ObjectFree()  && n->where->GetObject()) {
			class ness *no=(class ness *)n->where->GetObject();
			no->RemoveMark(n->where);
		    }
		    delete n->where;
		}
		if(n->spec!=NULL) {
			free(n->spec);
			n->spec=NULL;
		}
		if (n->rock!=NULL) {
			free(n->rock);
			n->rock=NULL;
		}
		if (n->meptr) *(n->meptr) = n->next;
		if (n->next) n->next->meptr = n->meptr;

		eventnode_Destroy(nesssym_NGetINode(this, eventnode));
	}	break;

	case flag_function | flag_ness | flag_builtin:
		/* a library function - do not destroy */
	case flag_function | flag_builtin:
		/* a builtin function (builtindef) */
	case flag_var | flag_builtin:
		/* predefined variable (builtindef) */
		/* DO NOT DESTROY the builtindef, it is permanent */
		break;

	case flag_var | flag_parmvar:
		/* parameter of a function (vardefnode) */
	case flag_var | flag_parmvar | flag_forward:
		/* parameter of a function (vardefnode) */
	case flag_var | flag_localvar:
		/* local within a function (vardefnode) */
	case flag_var | flag_globalvar: {
		/* global to a script (vardefnode) */
		struct vardefnode *v = nesssym_NGetINode(this, vardefnode);
		/* callnode_Destroy(v->sig); */
		free(v->sig);
		v->sig = NULL;
		vardefnode_Destroy(v);
	}	break;

	case flag_function | flag_undef:
		/* undefined function (NULL or 1) */
	case flag_var:
		/* as yet undifferentiated variable (none)*/
	case flag_undef | flag_var:
		/* (not used) as yet undifferentiated variable (none)*/
	case flag_const:
		/* a constant (TGlobRef) */
	case flag_undef:
		/* raw, newly initialized symbol (none) */
		break;

	default:
		fprintf(stderr, "Illegal flag value in Ness compilation: %ld\n",
			this->flags);
	}
	nesssym_NSetInfo(this, long, 0L);   /* (sets node to NULL) */
}
