#ifndef _arbiter_H_
#define _arbiter_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libarbiter Object arbiter
 *  A dynamically loadable ATK class file.  Contains arbiter, arbiterview,
 *  arbcon, cel, celview, lset, lsetview, page, pagev, runadewapp,
 *  wincelview.
 * @{ */
#include <cel.H>
/** A data object to manage \ref cel names.
 *  This is meant to be viewed with the \ref arbiterview class.
 */
class arbiter : public  cel {
public:
	TRACED_CLASS_NODEST(arbiter);


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
