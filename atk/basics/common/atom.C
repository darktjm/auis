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

#include <andrewos.h> /* strings.h */

#ifndef NORCSID
#define NORCSID
static UNUSED const char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/common/RCS/atom.C,v 3.7 1996/04/01 15:29:17 wjh Stab74 $";
#endif


 


ATK_IMPL("atom.H")
#include <stdio.h>
#include <atom.H>


#define Log2HashTableSize   9
#define HashTableSize       (1 << Log2HashTableSize)

struct alist
{
    class atom *atom;
    struct alist *next;
};

static struct alist *hashTable[HashTableSize];

/*
 * Statics
 */


ATKdefineRegistry(atom, ATK, atom::InitializeClass);
#ifndef NORCSID
#endif
static int Hash(register unsigned char *word);
static class atom *CreateAtom(register const char  *name, register int  index);


static int Hash(register unsigned char *word)
{
    register unsigned int total = 0;

    /* Pretty good hash function */

    while (*word)
        total = (total >> 1) + (total << 1) + *word++;

    return total & (HashTableSize - 1);
}

static class atom *CreateAtom(register const char  *name, register int  index)
{
    register class atom *a;
    register struct alist *l;

    a = new atom;
    a->name = (char*) malloc(strlen(name) + 1);
    (void) strcpy(a->name, name);

    l = (struct alist *) malloc(sizeof (struct alist));
    l->atom = a;
    l->next = hashTable[index];

    hashTable[index] = l;
    return a;
}

/*
 * Class procedures
 */

class atom *atom::Intern(register const char  *name)
{
	ATKinit;

    register int index;
    register struct alist *a;

    index = Hash((unsigned char *)name);

    for (a = hashTable[index]; a != NULL; a = a->next)
        if (0 == strcmp(name, (a->atom)->Name()))
            return a->atom;

    return CreateAtom(name, index);
}

class atom *atom::Intern(register char  *name)
{
    return Intern((const char *)name);
}


class atom *atom::InternRock(long  rock)
{
	ATKinit;

    char temp[20];

    sprintf(temp, "g0x%lx", rock); 
    return atom::Intern(temp);
}

boolean atom::InitializeClass()
{
    register struct alist **ap = hashTable;
    register int i = HashTableSize;

    while (i--)
        *ap++ = NULL;

    return TRUE;
}

atom::atom()
{
	ATKinit;

    this->name = NULL;
    THROWONFAILURE( TRUE);
}

atom::~atom()
{
	ATKinit;

	fprintf(stderr, "Illegal Destruction of an Atom\n");
}
