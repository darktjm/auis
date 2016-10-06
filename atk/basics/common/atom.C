/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h> /* strings.h */
ATK_IMPL("atom.H")
#include <stdio.h>
#include <atom.H>


#define Log2HashTableSize   9
#define HashTableSize       (1 << Log2HashTableSize)

struct alist : public atom
{
    struct alist *next;
    static const class atom *CreateAtom(const char *name, int index);
};

static struct alist *hashTable[HashTableSize];

/*
 * Statics
 */


ATKdefineRegistry(atom, ATK, atom::InitializeClass);


static int Hash(unsigned const char *word)
{
    unsigned int total = 0;

    /* Pretty good hash function */

    while (*word)
        total = (total >> 1) + (total << 1) + *word++;

    return total & (HashTableSize - 1);
}

const class atom *alist::CreateAtom(const char  *name, int  index)
{
    struct alist *l;

    l = new alist;
    l->name = strdup(name);
    l->next = hashTable[index];

    hashTable[index] = l;
    return const_cast<const atom *>(static_cast<atom *>(l));
}

/*
 * Class procedures
 */

const class atom *atom::Intern(const char  *name)
{
	ATKinit;

    int index;
    const struct alist *a;

    index = Hash((unsigned char *)name);

    for (a = hashTable[index]; a != NULL; a = a->next)
        if (0 == strcmp(name, static_cast<const atom *>(a)->Name()))
            return static_cast<const atom *>(a);

    return alist::CreateAtom(name, index);
}

boolean atom::InitializeClass()
{
    struct alist **ap = hashTable;
    int i = HashTableSize;

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
    fprintf(stderr, "Illegal Destruction of an Atom\n");
}
