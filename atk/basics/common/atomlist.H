ATK_INTER
#ifndef _atomlist_H_
#define _atomlist_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

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
