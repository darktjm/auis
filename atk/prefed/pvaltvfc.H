#ifndef _pvaltvfc_H_
#define _pvaltvfc_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

/** \addtogroup libprefs
 * @{ */
#include <pvaltvc.H>
class pvaltvfc : public  pvaltvc {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	static boolean InitializeClass();

    
	pvaltvfc();
    
	virtual ~pvaltvfc();


     void Select(int ind);


};
/** @} */
#endif /* _pvaltvfc_H_ */
