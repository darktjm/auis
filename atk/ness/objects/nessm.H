ATK_INTER
/*
	Copyright Carnegie Mellon University 1992, 1993 - All Rights Reserved
*/

#ifndef _nessm_H_
#define _nessm_H_


#include <ATK.H>
class nessm : public ATK  {
public:
	virtual ATKregistryEntry *ATKregistry();

	static boolean InitializeClass();
	static boolean atk_inited;
};

#endif /* _nessm_H_ */
