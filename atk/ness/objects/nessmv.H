/*
	Copyright Carnegie Mellon University 1992, 1993 - All Rights Reserved
*/

/** \addtogroup libness
 * @{ */
#ifndef _nessmv_H_
#define _nessmv_H_


#include <nessview.H>
class nessmv : public  nessview  {
public:
	TRACED_CLASS(nessmv);

	void PostMenus (class menulist *menulist);

	static boolean InitializeClass();
	static boolean atk_inited;

	nessmv();

	class menulist *ml;
};
/** @} */
#endif /* _nessmv_H_ */
