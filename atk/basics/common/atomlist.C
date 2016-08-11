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

#ifndef NORCSID
#define NORCSID
static char rcsid[]="$Header: /afs/cs.cmu.edu/project/atk-src-C++/atk/basics/common/RCS/atomlist.C,v 3.5 1994/12/13 20:35:03 rr2b Stab74 $";
#endif


 


#include <andrewos.h> /* strings.h */
ATK_IMPL("atomlist.H")
#include <atomlist.H>
#include <atom.H>

/*
 * Class Procedures
 */


ATKdefineRegistry(atomlist, ATK, NULL);
#ifndef NORCSID
#endif


atomlist::atomlist()
{
  this->atoms = NULL;
  THROWONFAILURE( TRUE);
}

atomlist::~atomlist()
{
    register struct atoms *next;

    while (this->atoms != NULL) {
	next = this->atoms->next;
	free(this->atoms);
	this->atoms = next;
    }
}


class atomlist *atomlist::Copy(class atomlist  *oldlist)
{
    class atomlist *newlist = new atomlist;
    register struct atoms *atoms;

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
    class atom *atom;
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

class atom *atomlist::Last()
{
    register struct atoms *atoms;

    for (atoms = this->atoms; atoms != NULL && atoms->next != NULL; atoms = atoms->next)
        ;

    return atoms == NULL ? NULL : atoms->atom;
}


short atomlist::Memberp(class atom  *key)
{
    register struct atoms *atoms;

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

void atomlist::Prepend(class atom  *atom)
{
    register struct atoms *atoms;

    atoms = (struct atoms *) malloc(sizeof(struct atoms));

    atoms->next = this->atoms;
    atoms->atom = atom;
    this->atoms = atoms;
}

void atomlist::Append(class atom  *atom)
{
    register struct atoms *new_c;
    register struct atoms **last;

    new_c = (struct atoms *) malloc(sizeof(struct atoms));
    new_c->atom = atom;
    new_c->next = NULL;

    for (last = &(this->atoms); *last != NULL; last = &((*last)->next))
        ;

    *last = new_c;
}


void atomlist::JoinToEnd(class atomlist  *otherlist)
{
    register struct atoms *otherAtoms;
    register struct atoms **last;

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
    register struct atoms *otherAtoms;
    register struct atoms **last;
    register struct atoms *temp;

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
    register struct atoms *next;

    while (this->atoms != mark && this->atoms != NULL)  {
	next = this->atoms->next;
	free(this->atoms);
	this->atoms = next;
    }
}
