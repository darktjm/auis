#ifndef _AWidgetView_H_
#define _AWidgetView_H_
/** \addtogroup libawidget
 * @{ */
/*
awidgetview.H

	Class definitions for widget inset view


 */

class figview;
class figure;
class AShadow;
class AWidget;
#include <aformula.H>
#include <view.H>

class AWidgetView : public  view {
protected:

	menulist  *Menus;		// default Menus
	keystate *Keystate;		// default Keymap 
	boolean embedded;		// T until GetApplicationLayer()
	boolean updateRequested;	// T iff WantUpdate since Update
	double uxscale, uyscale;	// pixels*uscale ==> units
	boolean leftDown, rightDown;	// mouse button state
	boolean HasInputFocus;		// T iff have input focus 
	AShadow *border;		// borders for this widget
	fontdesc *defaultfont;		// font of last resort
        view *mouseFocus;
public:
	virtual ATKregistryEntry *ATKregistry();
	static boolean InitializeClass();
	AWidgetView();
	virtual ~AWidgetView();

	view *GetApplicationLayer();
	void LinkTree(view *parent);
	void InitChildren();
	void FullUpdate(enum view::UpdateType type, 
		long left, long top, long width, long height);
	void Update();
	void SetDataObject(class dataobject *d);
	view * Hit(enum view::MouseAction action, long x, long y, long n);
	view::DSattributes DesiredSize(long width, long height, 
		enum view::DSpass pass, long *desiredWidth, long *desiredHeight) ;
	void WantUpdate(view *requestor);
	void ReceiveInputFocus();
        void LoseInputFocus();
        void PostMenus(menulist *ml);
        void PostKeyState(keystate *ks);
	void ObservedChanged(observable *dobj, long status);
	void GetOrigin(long width, long height, 
			long *originX, long *originY);

	boolean RecSearch(struct SearchPattern *pat, boolean toplevel);
	boolean RecSrchResume(struct SearchPattern *pat);
	boolean RecSrchReplace(class dataobject *text, long pos, long len);
	void RecSrchExpose(const struct rectangle &logical, 
		struct rectangle &hit);
	void Print(FILE *file, const char *processor, const char *finalFormat, 
			boolean topLevel);
	void *GetPSPrintInterface(const char *printtype);
	void PrintPSDoc(FILE *outfile, long pagew, long pageh);
	void PrintPSRect(FILE *file, long logwidth, long logheight, 
			struct rectangle *visrect);

	inline AShadow *GetBorder() { return border; }
	inline boolean GetHasInputFocus() {return HasInputFocus; }
	virtual long StringWidth(const char *str);
		// width of the string in terms of data object's
		// designFont and units slots
	virtual long CharVertical(const char c,
			long *ascent, long *descent);
		// distance above and below baseline for character c
		// distance are computed in terms of the slots
		//    designFont and units in the view's data object
		// Returns total height
	inline fontdesc *DefaultFont() { return defaultfont; }

	// the following give incorrect code from gcc on RS_AIX
	//	when written as the more obvious
	//			{ return (long) (x*uscale); }  
	inline long ToPixX(long x) 
			{ long a = (long)(x/uxscale); return a; }
	inline long ToPixY(long y) 
			{ long a = (long)(y/uyscale); return a; }
	inline long ToUnitX(long x) 
			{ long a = (long)(x*uxscale); return a; }
	inline long ToUnitY(long y) 
			{ long a = (long)(y*uyscale); return a; }
};

class AFormulaViewAct : public AFormulaAct {
  protected:
    ATK *bv;
  public:
    virtual ATKregistryEntry *ATKregistry();
    
    AFormulaViewAct(view *v=NULL, aaction *a=NULL, aaction *w=NULL);
    virtual ~AFormulaViewAct();
    virtual void Evaluate();
    virtual void WantUpdate(ADependerKey key);
};
/*

 *    $Log: awidgetview.H,v $
 * Revision 1.16  1996/10/29  22:47:59  robr
 * Added support for the mouse hit functions to return the hit view.
 * If they do this the widgetview's mouse hit functions will *NOT* see
 * any further hits.  (Existing mouse functions will continue to work
 * as before since the new behavior is only triggered if the
 * mouse function returns something in its return value list.)
 * BUG
 *
 * Revision 1.15  1996/10/07  21:16:10  robr
 * Fixed to use PostMenus and PostKeystates.
 * The rule is that EVERY view which implements
 * any user callable proctable functions MUST implement
 * PostMenus and PostKeyState.
 * ReceiveInputFocus must ALWAYS call PostMenus & PostKeyState
 * with NULL arguments.  (The incoming argument is for derived
 * classes and children to use in passing in their keystates.)
 * BUG
 *
 * Revision 1.14  1996/09/28  17:39:23  robr
 * Sigh... workaround a hp C++ bug
 * BUG
 *
 * Revision 1.13  1996/09/25  19:32:44  robr
 * Fixed missing registry information for some of the formula classes.
 * Added AFormulaViewAct in awidgetview to support the widget convention of calling
 * aactions with the view as the first variable arg.
 * BUG/FEATURE
 *
 * Revision 1.12  1996/09/24  19:35:22  wjh
 * revise To{Pix,Unit}{X,Y} to avoid bug in C++
 *
 * Revision 1.11  1996/05/20  15:25:03  robr
 * /tmp/msg
 *
 * Revision 1.10  1996/05/14  14:37:44  robr
 * Added inline methods for converting to/from pixel/internal units
 * FEATURE
 *
 * Revision 1.9  1996/05/13  17:14:02  wjh
 *  reprivatize border and HasInputFocus; add defaultfont (per view) and DefaultFont(); added GetBorder and GetHasInputFocus; added StringWidth and CharVertical to do size computations for designImage
 *
 * Revision 1.8  1996/05/07  20:36:03  wjh
 * move log to end of file; \
 * make HasInputFocus and border public member values; \
 * 	(This is bogus and should change); \
 * removed the temporary baseFontSize function
 *
 * Revision 1.7  1996/04/26  17:12:15  robr
 * Removed figure support to afwidget.
 . . .
 * Revision 1.1  1995/09/23  15:18:47  wjh
 * Initial revision
 * 
 * Revision 1.0  95/09/05  16:23:22  wjh
 * Copied from /afs/cs/misc/atk/@sys/alpha/lib/null
 */
/** @} */
#endif /* _AWidgetView_H_ */
