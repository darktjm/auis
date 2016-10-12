#ifndef _atomlist_H_
#define _atomlist_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
#include <atom.H>

#include <ATK.H>

struct atoms;

/** A list of atoms.
 *  This is the list head of a list of atoms.
 *  It provides methods to traverse and manipulate the list.  It also
 *  provides a method to convert a string representation of dot-separated
 *  atom names into an equivalent list (but no inverse). */
class atomlist : public ATK  {
    // bloody stupid C++ won't let me expose private parts of struct atoms
    // to inline routines while retaining privacy of atoms implementation.
    struct _atoms {
	const class atom *atom;
	struct atoms *next;
    };
  public:
    virtual ATKregistryEntry *ATKregistry();

    virtual const class atom * Last()  ;
    /**< Get the last atom of the list.
     *   \return the last atom.  If the list is empty, this returns NULL. */
    virtual short Memberp(const class atom *list)  ;
    /**< Find out if an atom is in the list.
     *   \param The atom to look for
     *   \return TRUE iff the atom is in the list. */
    virtual void DropFirst();
    /**< Delete the first list entry.
     *   Makes the second entry the new head. */
    virtual void Prepend(const class atom *newitem);
    /**< Add a new list entry at the head.
     *   \param newitem The atom to add. */
    virtual void Append(const class atom *newitem);
    /**< Add a new list entry to the tail.
     *   \param newitem The atom to add.  */
    virtual void JoinToEnd(const class atomlist *otherlist);
    /**< Append a copy of list to the tail.
     *   \param otherlist The list to add a copy of. */
    virtual void JoinToBeginning(const class atomlist *otherlist);
    /**< Prepend a copy of list to the head.
     *   \param otherlist The list to add a copy of. */
    virtual void Cut(struct atoms *mark);
    /**< Delete entries from the list.
     *   Entries are deleted until all are gone or \p mark is the
     *   new head of the list.
     *   \param mark The potential new head of the list. */

    inline struct atoms * TraversalStart() { return (this->atoms); }
    /**< Return a traversal pointer to the first atom in the list.
     *   Find the atom associated with this pointer using TraversalAtom().
     *   \return A traversal pointer to an atom, or NULL if list is empty. */
    inline const struct atoms * TraversalStart() const { return (this->atoms); }
    /**< Return a traversal pointer to the first atom in the list.
     *   Find the atom associated with this pointer using TraversalAtom().
     *   \return A traversal pointer to an atom, or NULL if list is empty. */
    inline struct atoms * TraversalNext(struct atoms *loc)  
            { return ((loc) == NULL ? NULL : ((struct _atoms *)loc)->next); }
    /**< Return a traversal pointer to the next atom in the list.
     *   Find the atom associated with this pointer using TraversalAtom().
     *   \param A traversal pointer to an atom in a list.
     *   \return A traversal pointer to the next atom, or NULL if no more in list. */
    inline const struct atoms * TraversalNext(const struct atoms *loc) const
            { return ((loc) == NULL ? NULL : ((const struct _atoms *)loc)->next); }
    /**< Return a traversal pointer to the next atom in the list.
     *   Find the atom associated with this pointer using TraversalAtom().
     *   \param A traversal pointer to an atom in a list.
     *   \return A traversal pointer to the next atom, or NULL if no more in list. */
    inline const class atom * TraversalAtom(const struct atoms *loc) const
            { return ((loc) == NULL ? NULL : ((const struct _atoms *)loc)->atom); }
    /**< Return the atom associated with a traversal pointer.
     *   \param A traversal pointer to an atom in a list.
     *   \return A pointer to the atom, or NULL if the traversal pointer is NULL. */
    inline const class atom * First() { return (this->atoms != NULL ? ((const struct _atoms *)this->atoms)->atom : NULL); }
    /**< Return the first atom in the list.
     *   \return A pointer to the atom, or NULL if the list is empty. */
    inline struct atoms * Mark() { return (this->atoms); }
    /**< Return a traversal pointer to the first atom in the list.
     *   Find the atom associated with this pointer using TraversalAtom().
     *   \return A traversal pointer to an atom, or NULL if list is empty. */


    atomlist();
    virtual ~atomlist();

    static class atomlist * Copy(class atomlist *oldlist)  ;
    /**< Copy an atom list.
     *   There is no copy constructor in this class, so Copy() is the only
     *   means to copy a list.  Keep in mind that the lack of a copy
     *   constructor means that normal copies of lists are actually just copies
     *   of the head of the list.  If that head is destroyed in either
     *   copy, the other copy becomes invalid.  This function, however,
     *   makes fully independent lists using a deep copy.
     *   \param oldlist An list to copy.
     *   \return a newly allocated deep copy of the old list. */
    static class atomlist * StringToAtomlist(const char *string)  ;
    /**< Create an atom list from a string representation.
     *   The string is separated on dots (`.`), and the substrings are
     *   then added as atoms to the list, in order of appearance.
     *   \param string The string to parse.
     *   \return a newly allocated atom list based on the parsed string. */

  private:
    struct atoms *atoms;
};
/** @} */
#endif /* _atomlist_H_ */
