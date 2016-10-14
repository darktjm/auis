/* Copyright 1995 Carnegie Mellon University  --All rights reserved */

#ifndef _wbuttonv_H_
#define _wbuttonv_H_
/** \addtogroup libhtmltext
 * @{ */
#include <buttonV.H>
class wbuttonv: public buttonV{
public:
	TRACED_CLASS_NODEST(wbuttonv);
	wbuttonv();
	static boolean InitializeClass();

	void FullUpdate(enum view::UpdateType type, 
		long left, long top, long width, long right);
	void Update();

	virtual void SetLabel(const char *label);

	char *wlabel;
	int needinit;
};
/** @} */
#endif /* _wbuttonv_H_ */
