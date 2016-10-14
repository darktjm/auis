#ifndef _matte_H_
#define _matte_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'        *
\* ********************************************************************** */

/** \addtogroup libsupviews
 * @{ */
#include <view.H>

class matte : public  view {
public:
	TRACED_CLASS(matte);


     void WantNewSize(class view *requestor);
     void PostMenus(class menulist *menulist);
     void FullUpdate(enum view::UpdateType type, long left, long top, long width, long right);
     void Update();
     void Print(FILE *file, const char *processor, const char *finalFormat, boolean topLevel);
     class view * Hit (enum view::MouseAction action, long x, long y, long numberOfClicks)  ;
     view::DSattributes DesiredSize(long width, long height, enum view::DSpass pass, long *dWidth, long *dheight)  ;
     void GetOrigin(long width, long height, long *originX, long *originY);
     void GetPrintOrigin(long width, long height, long *originX, long *originY);
     void WantInputFocus(class view *requestor);
     void ReceiveInputFocus();
     void LoseInputFocus();
     void SetDataObject(class dataobject *dataobject);
     void ObservedChanged (class observable *changed, long value);
     void LinkTree(class view *parent);
     void InitChildren();
     boolean RecSearch(class search *pat, boolean toplevel);
     boolean RecSrchResume(class search *pat);
     boolean RecSrchReplace(class dataobject *text, long pos, long len);
     void PrintPSDoc(FILE *outfile, long pagew, long pageh);
     void *GetPSPrintInterface(const char *printtype);
     void DesiredPrintSize(long width, long height, enum view::DSpass pass, long *desiredwidth, long *desiredheight);
     void PrintPSRect(FILE *outfile, long logwidth, long logheight, struct rectangle *visrect);
     void ChildLosingInputFocus();
     void ChildReceivingInputFocus();

    virtual void SetResizing(long key);

    virtual void RecSrchExpose(const struct rectangle &logical, struct rectangle &hit);

    virtual const char *WantInformation(const char *name);
    //  when name=="style":
    // compute a style representing the effects of all the styles
    // effective at this matte's location in the text.  the caller
    // is responsible for freeing the style object.
    // otherwise the call is passed to the view code and then
    // up the view tree.
   static class matte * Create(class viewref *vr,class view *parent)  ;
    
	static boolean InitializeClass();

    
	matte();
    

    int desw,desh;
    class view *child;
    class viewref *ref;
    class cursor *widthcursor, *heightcursor;
    int Moving,resizing,WasMoving, WasResizing;
    class menulist *menus;
    int drawing, OldMode, sizepending;
    boolean childhasfocus;
};
/** @} */
#endif /* _matte_H_ */
