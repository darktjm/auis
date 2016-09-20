/* Copyright 1995 Carnegie Mellon University  --All rights reserved */

#ifndef _wbuttonv_H_
#define _wbuttonv_H_
ATK_INTER
/** \addtogroup libhtmltext
 * @{ */
#include <buttonV.H>
class wbuttonv: public buttonV{
public:
	virtual ATKregistryEntry *ATKregistry();
	wbuttonv();
	static boolean InitializeClass();

	void FullUpdate(enum view_UpdateType type, 
		long left, long top, long width, long right);
	void Update();

	virtual void SetLabel(const char *label);

	char *wlabel;
	int needinit;
};
/** @} */
#endif /* _wbuttonv_H_ */
