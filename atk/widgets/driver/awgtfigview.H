#ifndef _AWgtFigView_H_
#define _AWgtFigView_H_
ATK_INTER
/* Copyright 1996 Carnegie Mellon University All rights reserved.
  $Disclaimer: 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, provided 
 * that the above copyright notice appear in all copies and that both that 
 * copyright notice and this permission notice appear in supporting 
 * documentation, and that the name of IBM not be used in advertising or 
 * publicity pertaining to distribution of the software without specific, 
 * written prior permission. 
 *                         
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
 * TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
 * HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 *  $
 */

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
	void FullUpdate(enum view_UpdateType type, 
			long left, long top, long width, long height);
	void Update();
	view * Hit(enum view_MouseAction action, 
			long x, long y, long n);
	view_DSattributes DesiredSize(long width, long height, 
			enum view_DSpass pass, 
			long *desiredWidth, long *desiredHeight);
	virtual boolean RecSearch(struct SearchPattern *pat, 
			boolean toplevel);
	virtual boolean RecSrchResume(struct SearchPattern *pat);
	virtual boolean RecSrchReplace(class dataobject *text, 
			long pos, long len);
	virtual void RecSrchExpose(const struct rectangle &logical, 
			struct rectangle &hit);
	void Print(FILE *file, char *processor, char *finalFormat, 
			boolean topLevel);
	void PrintPSRect(FILE *file, long logwidth, long logheight, 
			struct rectangle *visrect);
	void ReceiveInputFocus();
	void LoseInputFocus();
};
#endif /* _AWgtFigView_H_ */
