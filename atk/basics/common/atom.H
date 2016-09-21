ATK_INTER
#ifndef _atom_H_
#define _atom_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libbasics
 * @{ */
#include <ATK.H>

class atom : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();


    static class atom * Intern(const char *name)  ;
    static class atom * Intern(char *name)  ;
    static class atom * InternRock(long rock)  ;
    
	static boolean InitializeClass();

    
	atom();
    
	virtual ~atom();

    inline char * Name() const { return (this->name); }
    inline char * Name() { return (this->name); }

    char *name;
};

class atom_def {
    atom *a;
  public:
    inline atom_def() {
	a = NULL;
    }
    inline atom_def(const char *def) {
	a = atom::Intern(def);
    }
    inline atom_def(const atom *def) {
	a = (atom *)def;
    }
    inline operator const atom *() const {
	return a;
    }
    inline const atom *operator->() const {
	return a;
    }
};
/** @} */
#endif /* _atom_H_ */
