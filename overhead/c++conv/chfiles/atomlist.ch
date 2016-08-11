/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
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
*/


 


#include <atom.ih>

struct atoms 
{
    struct atom *atom;
    struct atoms *next;
};

class atomlist {
methods:
  Last() returns struct atom *;
    Memberp(struct atom *list) returns short;
  DropFirst();
    Prepend(struct atom *newitem);
    Append(struct atom *newitem);
    JoinToEnd(struct atomlist *otherlist);
    JoinToBeginning(struct atomlist *otherlist);
    Cut(struct atoms *mark);
macromethods:
    TraversalStart() (self->atoms)
    TraversalNext(struct atoms *loc) \
            ((loc) == NULL ? NULL : (loc)->next)
    TraversalAtom(struct atoms *loc) \
            ((loc) == NULL ? NULL : (loc)->atom)
    First() (self->atoms != NULL ? self->atoms->atom : NULL)
    Mark() (self->atoms)
classprocedures:
    InitializeObject(struct atomlist *self) returns boolean;
    FinalizeObject(struct atomlist *trashed);
    Copy(struct atomlist *oldlist) returns struct atomlist *;
    StringToAtomlist(char *string) returns struct atomlist *;
    /* (... assumes it can transiently smudge its input string) */
data:
    struct atoms *atoms;
};
