ATK_INTER
#ifndef _atomlist_H_
#define _atomlist_H_
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


 


#include <atom.H>

struct atoms 
{
    class atom *atom;
    struct atoms *next;
};

#include <ATK.H>

class atomlist : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();


  virtual class atom * Last()  ;
    virtual short Memberp(class atom *list)  ;
  virtual void DropFirst();
    virtual void Prepend(class atom *newitem);
    virtual void Append(class atom *newitem);
    virtual void JoinToEnd(class atomlist *otherlist);
    virtual void JoinToBeginning(class atomlist *otherlist);
    virtual void Cut(struct atoms *mark);

    inline struct atoms * TraversalStart() { return (this->atoms); }
    inline struct atoms * TraversalNext(struct atoms *loc)  
            { return ((loc) == NULL ? NULL : (loc)->next); }
    inline class atom * TraversalAtom(struct atoms *loc)  
            { return ((loc) == NULL ? NULL : (loc)->atom); }
    inline class atom * First() { return (this->atoms != NULL ? this->atoms->atom : NULL); }
    inline struct atoms * Mark() { return (this->atoms); }

    
	atomlist();
    
	virtual ~atomlist();
    static class atomlist * Copy(class atomlist *oldlist)  ;
    static class atomlist * StringToAtomlist(const char *string)  ;
    

    struct atoms *atoms;
};

#endif /* _atomlist_H_ */
