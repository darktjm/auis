ATK_INTER
#ifndef _matte_H_
#define _matte_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/config/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/


 

#include <view.H>

class matte : public  view {
public:
	virtual ATKregistryEntry *ATKregistry();


     void WantNewSize(class view *requestor);
     void PostMenus(class menulist *menulist);
     void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);
     void Update();
     void Print(FILE *file, const char *processor, const char *finalFormat, boolean topLevel);
     class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
     view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long *dWidth, long *dheight)  ;
     void GetOrigin(long width, long height, long *originX, long *originY);
     void GetPrintOrigin(long width, long height, long *originX, long *originY);
     void WantInputFocus(class view *requestor);
     void ReceiveInputFocus();
     void LoseInputFocus();
     void SetDataObject(class dataobject *dataobject);
     void ObservedChanged (class observable *changed, long value);
     void LinkTree(class view *parent);
     void InitChildren();
     boolean RecSearch(struct SearchPattern *pat, boolean toplevel);
     boolean RecSrchResume(struct SearchPattern *pat);
     boolean RecSrchReplace(class dataobject *text, long pos, long len);
     void PrintPSDoc(FILE *outfile, long pagew, long pageh);
     void *GetPSPrintInterface(const char *printtype);
     void DesiredPrintSize(long width, long height, enum view_DSpass pass, long *desiredwidth, long *desiredheight);
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
    
	virtual ~matte();

    int desw,desh;
    class view *child;
    class viewref *ref;
    class cursor *widthcursor, *heightcursor;
    int Moving,resizing,WasMoving, WasResizing;
    class menulist *menus;
    int drawing, OldMode, sizepending;
    boolean childhasfocus;
};

#endif /* _matte_H_ */
