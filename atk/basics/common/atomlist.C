/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

#include <andrewos.h> /* strings.h */
ATK_IMPL("atomlist.H")
#include <atomlist.H>
#include <atom.H>

/*
 * Class Procedures
 */


ATKdefineRegistry(atomlist, ATK, NULL);

atomlist::atomlist()
{
  this->atoms = NULL;
  THROWONFAILURE( TRUE);
}

atomlist::~atomlist()
{
    struct atoms *next;

    while (this->atoms != NULL) {
	next = this->atoms->next;
	free(this->atoms);
	this->atoms = next;
    }
}


class atomlist *atomlist::Copy(class atomlist  *oldlist)
{
    class atomlist *newlist = new atomlist;
    struct atoms *atoms;

    for (atoms = oldlist->atoms; atoms != NULL; atoms = atoms->next)
        (newlist)->Append( atoms->atom);

    return newlist;
}

static int strsize=0;
static char *str=NULL;

class atomlist *atomlist::StringToAtomlist(const char  *string)
{
    char *atomstart;
    char *atomend;
    const class atom *atom;
    class atomlist *newlist = new atomlist;
    int len=strlen(string)+1;
    if(len>strsize) {
	strsize=len;
	if(str) str=(char *)realloc(str, strsize);
	else str=(char *)malloc(strsize);
    }
    strcpy(str, string);
    if (str != NULL)  {
	for (atomstart = atomend = str; atomend != NULL; atomstart =  1 + atomend)  {
	    atomend = strchr(atomstart,'.');
	    if (atomend != NULL) *atomend = '\0';
            atom = atom::Intern(atomstart);
	    if (atomend != NULL) *atomend = '.';
            (newlist)->Append( atom);
	}
    }
    return newlist;
}

/*
 * Methods
 */

const class atom *atomlist::Last()
{
    struct atoms *atoms;

    for (atoms = this->atoms; atoms != NULL && atoms->next != NULL; atoms = atoms->next)
        ;

    return atoms == NULL ? NULL : atoms->atom;
}


short atomlist::Memberp(const class atom  *key)
{
    struct atoms *atoms;

    for (atoms = this->atoms; atoms != NULL && atoms->atom != key; atoms = atoms->next)
        ;

    return (atoms != NULL);
}

void atomlist::DropFirst()
{
    struct atoms *oldfirst = this->atoms;

    if (oldfirst != NULL) {
	this->atoms = this->atoms->next;
	free(oldfirst);
    }
}

void atomlist::Prepend(const class atom  *atom)
{
    struct atoms *atoms;

    atoms = (struct atoms *) malloc(sizeof(struct atoms));

    atoms->next = this->atoms;
    atoms->atom = atom;
    this->atoms = atoms;
}

void atomlist::Append(const class atom  *atom)
{
    struct atoms *new_c;
    struct atoms **last;

    new_c = (struct atoms *) malloc(sizeof(struct atoms));
    new_c->atom = atom;
    new_c->next = NULL;

    for (last = &(this->atoms); *last != NULL; last = &((*last)->next))
        ;

    *last = new_c;
}


void atomlist::JoinToEnd(class atomlist  *otherlist)
{
    struct atoms *otherAtoms;
    struct atoms **last;

    for (last = &(this->atoms); *last != NULL; last = &((*last)->next))
        ;

    for (otherAtoms = (otherlist)->TraversalStart(); otherAtoms != NULL; otherAtoms = (otherlist)->TraversalNext( otherAtoms)) {
        *last = (struct atoms *) malloc(sizeof(struct atoms));
        (*last)->atom = (otherlist)->TraversalAtom( otherAtoms);
	(*last)->next = NULL;
	last = &((*last)->next);
    }
}

void atomlist::JoinToBeginning(class atomlist  *otherlist)
{
    struct atoms *otherAtoms;
    struct atoms **last;
    struct atoms *temp;

    last = &(this->atoms);

    for (otherAtoms = (otherlist)->TraversalStart(); otherAtoms != NULL; otherAtoms = (otherlist)->TraversalNext( otherAtoms)) {
        temp = (struct atoms *) malloc(sizeof(struct atoms));
        temp->atom = (otherlist)->TraversalAtom( otherAtoms);
	temp->next = *last;
	*last = temp;
	last = &(temp->next);
    }
}

void atomlist::Cut(struct atoms  *mark)
{
    struct atoms *next;

    while (this->atoms != mark && this->atoms != NULL)  {
	next = this->atoms->next;
	free(this->atoms);
	this->atoms = next;
    }
}
