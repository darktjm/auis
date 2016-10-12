#ifndef _AWgtFigView_H_
#define _AWgtFigView_H_
/* Copyright 1996 Carnegie Mellon University All rights reserved. */
/** \addtogroup libawidget
 * @{ */

#include <awidgetview.H>
#include <figview.H>

class AWgtFigView : public AWidgetView {
	figview *viewer;	// figview used to display screenImage
	figure *currfig;		// figure obj now viewed by viewer
	boolean currfigshared;  // T if using parent's currfig
	boolean needToPrepImage;  // TRUE which FullUpdate 
			//	or Update needs to call prepImage
   public:
	ATKregistryEntry *ATKregistry();

	AWgtFigView();
	virtual ~AWgtFigView();


	inline long ToFigX(long val) 
		{ return viewer->ToFigW(ToPixX(val)); }
	inline long ToFigY(long val) 
		{ return viewer->ToFigH(ToPixY(val)); }

	virtual void LinkTree(view *parent);
	virtual void InitChildren();
	void FullUpdate(enum view::UpdateType type, 
			long left, long top, long width, long height);
	void Update();
	view * Hit(enum view::MouseAction action, 
			long x, long y, long n);
	view::DSattributes DesiredSize(long width, long height, 
			enum view::DSpass pass, 
			long *desiredWidth, long *desiredHeight);
	virtual boolean RecSearch(class search *pat, 
			boolean toplevel);
	virtual boolean RecSrchResume(class search *pat);
	virtual boolean RecSrchReplace(class dataobject *text, 
			long pos, long len);
	virtual void RecSrchExpose(const struct rectangle &logical, 
			struct rectangle &hit);
	void Print(FILE *file, const char *processor, const char *finalFormat, 
			boolean topLevel);
	void PrintPSRect(FILE *file, long logwidth, long logheight, 
			struct rectangle *visrect);
	void ReceiveInputFocus();
	void LoseInputFocus();
};
/** @} */
#endif /* _AWgtFigView_H_ */
