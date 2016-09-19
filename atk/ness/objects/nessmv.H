ATK_INTER
/*
	Copyright Carnegie Mellon University 1992, 1993 - All Rights Reserved
*/

#ifndef _nessmv_H_
#define _nessmv_H_


#include <nessview.H>
class nessmv : public  nessview  {
public:
	virtual ATKregistryEntry *ATKregistry();

	void PostMenus (class menulist *menulist);

	static boolean InitializeClass();
	static boolean atk_inited;

	nessmv();
	virtual ~nessmv();

	class menulist *ml;
};


#endif /* _nessmv_H_ */
