#ifndef _mbuttonv_H_
#define _mbuttonv_H_
/** \addtogroup libvalue
 * @{ */
#include <buttonV.H>
class mbuttonv: public buttonV {
public:
	TRACED_CLASS_NODEST(mbuttonv);


   void LookupParameters();


	mbuttonv();

	static boolean InitializeClass();

};
/** @} */
#endif /* _mbuttonv_H_ */
