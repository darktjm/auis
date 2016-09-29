#ifndef _arbiter_H_
#define _arbiter_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libarbiter libarbiter.so
 *  A dynamically loadable ATK class file.  Contains arbiter, arbiterview,
 *  arbcon, cel, celview, lset, lsetview, page, pagev, runadewapp,
 *  wincelview.
 * @{ */
#include <cel.H>
class arbiter : public  cel {
public:
	virtual ATKregistryEntry *ATKregistry();


	inline class cel * GetFirst() { return ((this)->first); }

 	 long Read (FILE *file, long id)  ;
	 long ReadFile(FILE *file)  ;

	virtual FILE * DeclareRead(class cel *cel)  ;
	virtual class cel * FindChildCelByName(char *name)  ;
	virtual class dataobject * FindChildObjectByName(char *name)  ;
	virtual void ReadObjects();

	static class arbiter * GetMaster()  ; 
	static class arbiter * SetMaster(class arbiter *newmaster)  ; 
	
	static boolean InitializeClass();

	
	arbiter();

	class cel *first;
};
/** @} */
#endif /* _arbiter_H_ */
