#ifndef _atom_H_
#define _atom_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
#include <ATK.H>

/** The atom class exists to provide convenient handles for strings.
 *  If you try to create two atoms having the same name, the atom package
 *  will simply return the same atom for both calls.  Given this invariant,
 *  that calling atom_Intern with the same string twice will result in the
 *  same pointer being returned, one can build a very fast string
 *  comparison routine by storing and searching atoms instead of strings.
 *  In fact, just replacing string pointers with the result from Name()
 *  on an atom of the string, like so: s = atom::Intern(s)->Name();
 *  gives the same benefits.
 *
 * Note that this is only effective if you reference a string many times
 * for every instance of the string, since creating an atom takes somewhat
 * longer than a string comparison operation.  For example, one might use
 * atoms for font names, increasing the performance of font name searching
 * in the font package.
 *
 * Atoms' scope and lifetime are the entire program.
 */
class atom : public ATK  {
  public:
    virtual ATKregistryEntry *ATKregistry();
    static boolean InitializeClass(); /**< Class static intializer; do not call */

    static const class atom * Intern(const char *name)  ;
    /**< Create or return an atom for a string.
     *   \param name The string to turn into an atom.
     *   \return the atom corresponding to the string.
     */

    /** Only create an atom if you intend to use it. */
    atom();
    /** Never destroy an atom.  */
    virtual ~atom();

    inline const char * Name() const { return (this->name); }
    /**< Retrieve the string associated with this atom.
     *   \return the string.   For rock atoms, this will be the
     *           string generated from the rock.  */
  protected:
    const char *name; /**< The stored name; access using Name() */
};

/** Convenience helper for creating static atoms.
 *  This class acts as a pointer-to-const-atom on use.
 *  Simply create a static object of this type to have the
 *  atom available at program startup. */
class atom_def {
    const atom *a;
  public:
    inline atom_def() {
	a = NULL;
    }
    /**< Default constructor: a NULL atom. */
    inline atom_def(const char *def) {
	a = atom::Intern(def);
    }
    /**< Construct an atom with a string.
     *   \param def The string. */
    inline atom_def(const atom *def) {
	a = (const atom *)def;
    }
    /**< Copy another atom.
     *   \param def The atom to copy. */
    inline operator const atom *() const {
	return a;
    }
    /**< Returns a pointer to the atom, rather than this. */
    inline const atom *operator->() const {
	return a;
    }
    /**< Uses a pointer to the atom, rather than this. */
};
/** @} */
#endif /* _atom_H_ */
