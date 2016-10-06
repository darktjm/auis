#ifndef _atomlist_H_
#define _atomlist_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
#include <atom.H>

struct atoms 
{
    const class atom *atom;
    struct atoms *next;
};

#include <ATK.H>

class atomlist : public ATK  {
  public:
    virtual ATKregistryEntry *ATKregistry();

    virtual const class atom * Last()  ;
    virtual short Memberp(const class atom *list)  ;
    virtual void DropFirst();
    virtual void Prepend(const class atom *newitem);
    virtual void Append(const class atom *newitem);
    virtual void JoinToEnd(class atomlist *otherlist);
    virtual void JoinToBeginning(class atomlist *otherlist);
    virtual void Cut(struct atoms *mark);

    inline struct atoms * TraversalStart() { return (this->atoms); }
    inline struct atoms * TraversalNext(struct atoms *loc)  
            { return ((loc) == NULL ? NULL : (loc)->next); }
    inline const class atom * TraversalAtom(struct atoms *loc)  
            { return ((loc) == NULL ? NULL : (loc)->atom); }
    inline const class atom * First() { return (this->atoms != NULL ? this->atoms->atom : NULL); }
    inline struct atoms * Mark() { return (this->atoms); }


    atomlist();
    virtual ~atomlist();

    static class atomlist * Copy(class atomlist *oldlist)  ;
    static class atomlist * StringToAtomlist(const char *string)  ;

  private:
    struct atoms *atoms;
};
/** @} */
#endif /* _atomlist_H_ */
