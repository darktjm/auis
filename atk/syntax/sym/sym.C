/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
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
\* ********************************************************************** */
#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
static char *rcsid = "$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/syntax/sym/RCS/sym.C,v 1.5 1995/12/07 16:41:27 robr Stab74 $";
#endif

/* sym.c		

	Code for the sym object
*/
/*
 *    $Log: sym.C,v $
// Revision 1.5  1995/12/07  16:41:27  robr
// Support for exporting ness functions to the proctable.
//
// Revision 1.4  1994/11/30  20:42:06  rr2b
// Start of Imakefile cleanup and pragma implementation/interface hack for g++
//
// Revision 1.3  1994/08/11  19:07:50  rr2b
// The great gcc-2.6.0 new fiasco, new class foo -> new foo
//
// Revision 1.2  1993/05/18  17:04:36  rr2b
// Moved #include of andrewos.h to the front.
// Fixed unneeded use of unsigned char to just char.
// Removed ::s from before local variables.
// Converted calls on *_NewFromObject.
// Added casts on return value from malloc.
// Added use of sym_findfptr typedef for sym::FindALL.
// Made NULLs 0 where appropriate.
//
// Revision 1.1  1993/05/06  16:57:06  rr2b
// Initial revision
//
 * Revision 1.11  1992/12/14  20:57:48  rr2b
 * disclaimerization
 *
 * Revision 1.10  1992/11/26  02:02:58  wjh
 * updated header
 * .
 *
 * Revision 1.9  92/06/05  16:50:26  rr2b
 * added support for proper destruction of symbols
. . .  
 * Revision 1.0  88/06/23  12:15:01  gb17
 * Copied from /usr/andrew/lib/nullinset
 */

/*****************************************************************************\
\*****************************************************************************/




#include <andrewos.h>
ATK_IMPL("sym.H")
#include <sym.H>


/*****************************************************************************\
 *
 * CLASS CONSTANTS
 *
\*****************************************************************************/

#define	initialScopes	    4		/* initial size of enclosing scope table   */

#define	SCOPE_NULL	    -1		/* indicates a NULL scope		   */
#define	SCOPE_FREE	    -2		/* indicates a free slot in a table          */


/*****************************************************************************\
 *
 * CLASS DATA
 *
\*****************************************************************************/

static long maxScopes, nextFreeScope, *enclosingScope;
/* this table gives the enclosing scope for each scope.  maxScopes is the size of the
 * table, and nextFreeScope is the index of the lowest numbered free slot in the
 * table.
 */

static long noOfEntries;
static class sym **table;
/* the hash table.  tableSize gives the size of the table, noOfEntries
 * gives the number of entries in the table, and table is the table itself.
 */

static long primes[] = {5, 11, 17, 37, 67, 131,
                        257, 521, 1031,2053, 4099,8209, 16411,  32771,
                        65537, 131101, 262147, 524309,1048583, 2097169,
                        4194319, 8388617, 16777259, 0 };
static long *tableSize;
/* this is the sequence of prime numbers used for determining the tableSize.
 */

/*****************************************************************************\
 *
 * PRIVATE FUNCTIONS
 *
\*****************************************************************************/


ATKdefineRegistry(sym, ATK, sym::InitializeClass);
#if !defined(lint) && !defined(LOCORE) && defined(RCS_HDRS)
#endif
static long hash(char *name);
static class sym** lookup(char  *name, long  scope, boolean  *found);
static class sym** lookupInScope(char  *name, long  scope, boolean  *found);
static void insert(class sym  *self);
static void resizeTable();
static void resizeArray(long  **a , long  from , long  to);
static void removeScopeFromScopes(sym_ScopeType  scope);
void printdata(class sym  *self);


static long
hash(char *name)
{
    register unsigned long val;
    register unsigned char *pos;

    for (val = 0, pos=(unsigned char *)name; *pos; ++pos)
	val = ((val<<5) | (val>>27)) ^ *pos;

    return val % *tableSize;
}

static class sym**
lookup(char  *name, long  scope, boolean  *found)
{
    register class sym **s, **start = table+hash(name);

    while (scope != SCOPE_NULL) {

	/* check all the other valid scopes for the symbol, working outward */

	for(s = start; *s != NULL; s = &((*s)->next)) {

	    /* look for name in the current scope, and return if found */

	    if (strcmp(name, (*s)->name) == 0 && scope == (*s)->scope) {
		*found = TRUE;
		return (s);
	    }
	}

	scope = enclosingScope[scope];

    };

    *found = FALSE;
    return s;
}

static class sym**
lookupInScope(char  *name, long  scope, boolean  *found)
{
    register class sym **s;

    if (name == NULL) {
	*found = FALSE;
	return NULL;
    }
    for(s = table+hash(name); *s != NULL; s = &((*s)->next)) {

	/* look for name in the current scope, and return if found */

	if (strcmp(name, (*s)->name) == 0 && scope == (*s)->scope) {
	    *found = TRUE;
	    return (s);
	}
    }

    *found = FALSE;
    return s;
}

static void
insert(class sym  *self)
{
    boolean found;
    class sym** loc = lookupInScope(self->name, self->scope, &found);

    ++noOfEntries;
    self->next = *loc;
    *loc = self;
}

static void
resizeTable()
{
    class sym **old = table;
    long oldSize = *tableSize;
    class sym *pos, *next;
    long i;

    ++tableSize;
    table = (class sym **) malloc(sizeof(class sym*) * *tableSize);
    noOfEntries = 0;

    for (i = 0; i < *tableSize; ++i)
	table[i] = NULL;

    for (i = 0; i < oldSize; ++i)
	for (pos = old[i]; pos != NULL;) {
	    next = pos->next;
	    insert(pos);
	    pos = next;
	}

    free(old);
}

static void
resizeArray(long  **a , long  from , long  to)
{
    long *newc = (long*) malloc(sizeof(long) * to);
    long i;

    for (i = 0; i < from; ++i)
	newc[i] = (*a)[i];

    for (i = from; i < to; ++i)
	newc[i] = SCOPE_FREE;
    free(*a);
    *a = newc;
}

static void
removeScopeFromScopes(sym_ScopeType  scope)
{
    register long i;
 
    for (i = 0; i < maxScopes; ++i)
	if (enclosingScope[i] == scope)
	    removeScopeFromScopes(i);

    enclosingScope[scope] = SCOPE_FREE;
    if (scope < nextFreeScope)
	nextFreeScope = scope;
}

/*****************************************************************************\
 *
 * CLASS METHODS
 *
\*****************************************************************************/

sym_ScopeType
sym::NewScope(sym_ScopeType  scope)
{
	ATKinit;

    long newc = nextFreeScope;

    if (nextFreeScope == maxScopes-1) {
	resizeArray(&enclosingScope, maxScopes, 2*maxScopes);
	maxScopes *= 2;
    }

    enclosingScope[newc] = scope;

    do
	++nextFreeScope;
    while (enclosingScope[nextFreeScope] != SCOPE_FREE);

    return newc;
}

void
sym::DestroyScope(sym_ScopeType  scope)
{
	ATKinit;

    register long i, s;
    register class sym **pos;

    for (i = 0; i < *tableSize; ++i)
	for (pos = &table[i]; *pos != NULL;) {
	    for(s = (*pos)->scope; s != scope && s != SCOPE_NULL;)
		s = enclosingScope[s];

	    if (s == scope) {
		class sym *trash = *pos;

		*pos = trash->next;
		free(trash->name);
		trash->name = NULL;
		delete trash;
	    } else
		pos = &((*pos)->next);
	}

    removeScopeFromScopes(scope);
}

sym_ScopeType
sym::ParentScope(sym_ScopeType  scope)
{
	ATKinit;

	return enclosingScope[scope];
}


class sym*
sym::Define(char  *name, class sym  *proto, sym_ScopeType  scope)
{
	ATKinit;

    boolean found;
    class sym *newSym, **loc;

    if (noOfEntries > *tableSize)
	resizeTable();

    loc = lookupInScope(name, scope, &found);

    if (found)
	return NULL;
    else {

	newSym = (proto == NULL) ? (new sym) : (class sym *)proto->NewFromObject();
	++noOfEntries;

	newSym->name = (char *)malloc(strlen(name)+1);
	strcpy(newSym->name, name);
	newSym->scope = scope;
	newSym->next = *loc;
	newSym->intable= TRUE;
	*loc = newSym;
	return newSym;
    }
}


boolean
sym::Undefine(char  *name, sym_ScopeType  scope)
{
	ATKinit;

    boolean found;
    class sym **loc = lookupInScope(name, scope, &found);

    if (found) {
	class sym *trash = *loc;

	*loc = trash->next;
	free(trash->name);
	trash->name = NULL;
	delete trash;
	return TRUE;
    }
    else
	return FALSE;
}


class sym*
sym::Find(char  *name, sym_ScopeType  scope)
{
	ATKinit;

    boolean found;
    class sym **loc = lookup(name, scope, &found);

    if (found)
	return *loc;
    else
	return NULL;
}

class sym*
sym::Locate(char  *name, class sym  *proto, sym_ScopeType  scope, boolean  *newc)
{
    ATKinit;

    boolean found;
    class sym *newSym, **loc;

    if (noOfEntries > *tableSize)
	resizeTable();

    loc = lookup(name, scope, &found);

    if (found) {
	*newc = FALSE;
	return *loc;
    } else {
	newSym = (proto == NULL) ? (new sym) : (class sym *)proto->NewFromObject();
	++noOfEntries;

	newSym->name = (char *)malloc(strlen(name)+1);
	strcpy(newSym->name, name);
	newSym->scope = scope;
	newSym->next = *loc;
	newSym->intable = TRUE;
	*loc = newSym;
	*newc = TRUE;
	return newSym;
    }
}

long
sym::FindAll(char  *name, sym_ScopeType  scope, sym_findfptr proc, long  rock)
{
	ATKinit;

    register long i, s;
    register class sym **pos;

    for (i = 0; i < *tableSize; ++i)
	for (pos = &table[i]; *pos != NULL;) {
	    for(s = (*pos)->scope; s != scope && s != SCOPE_NULL;)
		s = enclosingScope[s];

	    if (s == scope && strcmp(name, (*pos)->name) == 0) {
		long val = proc(*pos, rock);

		if (val != 0)
		    return val;
	    }
	    pos = &((*pos)->next);
	}
    return 0;
}

long
sym::Enumerate(sym_ScopeType  scope, sym_findfptr proc, long  rock)
{
	ATKinit;

    register long i, s;
    register class sym **pos;

    for (i = 0; i < *tableSize; ++i)
	for (pos = &table[i]; *pos != NULL;) {
	    for(s = (*pos)->scope; s != scope && s != SCOPE_NULL;)
		s = enclosingScope[s];
	    if (s == scope) {
		long val = proc(*pos, rock);

		if (val != 0)
		    return val;
	    }
	    pos = &((*pos)->next);
	}
    return 0;
}

boolean
sym::InitializeClass()
{
    long i;

    maxScopes = initialScopes;
    enclosingScope = (long *) malloc(sizeof(long) * maxScopes);
    for (i = 1; i < maxScopes; ++i)
	enclosingScope[i] = SCOPE_FREE;
    enclosingScope[sym_GLOBAL] = SCOPE_NULL;

    nextFreeScope = 1;

    tableSize = primes;
    table = (class sym **) malloc(sizeof(class sym*) * *tableSize);
    for(i = 0; i < *tableSize; ++i)
	table[i] = NULL;
    noOfEntries = 0;

    return TRUE;
}


sym::sym()
{
	ATKinit;

    this->name = NULL;
    this->next = NULL;
    this->scope = 0;
    this->intable = FALSE;
    THROWONFAILURE( TRUE);
}

sym::~sym()
{
	ATKinit;

    if(this->name) {
	free(this->name);
	this->name=NULL;
    }
    if(this->intable) {
	this->intable=FALSE;
	noOfEntries--;
    }
    return;
}





void
sym::printtable()
{
	ATKinit;

        long i;
	class sym *pos;

	printf("enclosingScope:");
	for (i = 0; i < maxScopes; ++i)
	    printf(" %ld", enclosingScope[i]);
	printf(" <%ld> \n", nextFreeScope);

	printf("table <%ld>:\n", noOfEntries);
	for (i = 0; i < *tableSize; ++i) {
	    printf("%ld:\n",i);
	    for(pos = table[i]; pos != NULL; pos = pos->next)
		printdata(pos);
	}

	printf("\n\n");

	fflush(stdout);
}


void printdata(class sym  *self)
{
    if (self == NULL)
	printf("NULL\n");
    else
	printf("%p:%s,%ld\n", self, self->name, self->scope);
}

/*****************************************************************************\
 *
 * INSTANCE METHODS
 *
\*****************************************************************************/

