#ifndef _sym_H_
#define _sym_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
\* ********************************************************************** */

/** \addtogroup libsym libsym.so
 *  A dynamically loadable ATK class file.  Contains sym, toksym.
 * @{ */
/* sym.c
 *
 * A sym object is an entry in a symbol table.  Each symbol has associated with it a uniqe
 * name and scope.  sym should be subclassed if any additional data is desired.
 *
 * History:
 *
 *	5-24-88 Fred Hansen:	original symbol table module created.
 *	6-29-88 George Baggott:	rewritten as an object.
 *	7-07-88 George Baggott:	no longer a subclass of node.
 *	7-13-88 George Baggott:	partitions axed.
 */
/*
 *    $Log: sym.H,v $
// Revision 1.5  1995/12/07  16:41:27  robr
// Support for exporting ness functions to the proctable.
//
// Revision 1.4  1994/11/30  20:42:06  rr2b
// Start of Imakefile cleanup and pragma implementation/interface hack for g++
//
// Revision 1.3  1993/06/10  20:43:11  rr2b
// Great atk_inited removal.
//
# Revision 1.2  1993/05/18  17:07:43  rr2b
# Added typedef for the function argument to FindAll.
# Removed unneeded used of unsigned char
#
# Revision 1.1  1993/05/06  16:57:06  rr2b
# Initial revision
#
*Revision 1.10  1992/12/14  20:57:48  rr2b
*disclaimerization
*
Revision 1.9  1992/11/26  02:02:58  wjh
updated header
.

Revision 1.8  92/06/05  16:50:26  rr2b
added support for proper destruction of symbols
. . .
Revision 1.1  88/10/21  10:42:15  wjh
Initial revision
 
 * Revision 1.0  88/06/23  12:15:00  gb17
 * Copied from /usr/andrew/lib/nullinset
 */


typedef long sym_ScopeType;

#define sym_GLOBAL ((sym_ScopeType) 0L)

#include <ATK.H>
class sym;
typedef long (*sym_findfptr)(class sym *s, long rock);
class sym : public ATK 
{
public:
	virtual ATKregistryEntry *ATKregistry();








	inline const char * GetName()		{ return ((this)->name); }

	inline long GetScope()		{ return ((this)->scope); }
	inline void SetScope(long newScope)	{ ((this)->scope = (newScope)); }



	
	static boolean InitializeClass();

	
	sym();
	
	virtual ~sym();

	static sym_ScopeType NewScope(sym_ScopeType enclosingScope)  ;
		/* creates a new scope in enclosingScope
		 */

	static void DestroyScope(sym_ScopeType scope);
		/* destroys all the symbols and scopes defined in the given scope
		 */

	static sym_ScopeType ParentScope(sym_ScopeType scope)  ;
		/* returns the enclosing scope for the given scope 
		 */

	static class sym* Define(const char *name, class sym *proto, sym_ScopeType scope)  ;
		/* defines the symbol (name, scope).  If it already exists, NULL is
		 * returned.
		 */

	static boolean Undefine(const char *name, sym_ScopeType scope)  ;
		/* undefines (name, scope).  Returns TRUE if successful,
		 * false otherwise.
		 */

	static class sym* Find(const char *name, sym_ScopeType scope)  ;
		/* attempts to find a match for (name, scope).  If one is found
		 * it is returned.  If not, then NULL is returned.
		 */

	static class sym* Locate(const char *name, class sym *proto, sym_ScopeType scope, boolean *new_c)
	           ;
		/* attempts to find a match for (name, scope).  If one is found, 
		 * it is returned.  If not, (name, scope) is defined.
		 */

	static long FindAll(const char *name, sym_ScopeType scope, sym_findfptr proc, long rock)  ;
		/* finds all of name's matches that are defined in scope or any of scope's
		 * descendent scopes.  For each match, proc is called with sym and rock.
		 * proc should be declared as:
		 *
		 * 	long proc(sym, rock);
		 *	class sym *sym;
		 *	long rock;
		 *
		 * if a call to proc returns a value other than 0, the search
		 * is terminated, and FindAll will return that value.  Otherwise,
		 * FindAll will return 0.
		 */
	static long Enumerate(sym_ScopeType scope, sym_findfptr proc, long rock)  ;
		/* finds all of the definitions in scope or any of scope's
		 * descendent scopes.  For each definition, proc is called with sym and rock.
		 * proc should be declared as:
		 *
		 * 	long proc(sym, rock);
		 *	class sym *sym;
		 *	long rock;
		 *
		 * if a call to proc returns a value other than 0, the search
		 * is terminated, and FindAll will return that value.  Otherwise,
		 * FindAll will return 0.
		 */
	static void printtable();


	char *name;		/* the name of the symbol */
	long scope;		/* the scope in which the symbol is defined */
	class sym *next;		/* used for handling collisions in hash table */
	boolean intable; /* whether this sym has been put in the table or not */
};
/** @} */
#endif /* _sym_H_ */
