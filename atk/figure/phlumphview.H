#ifndef _phlumphview_H_
#define _phlumphview_H_

/*
	Copyright Carnegie Mellon University 1994 - All rights reserved
*/

/** \addtogroup libfigure
 * @{ */
#include <view.H>

class phlumphview : public view {
  public:
    virtual ATKregistryEntry *ATKregistry();

    static boolean InitializeClass();
    phlumphview(boolean iconmode = TRUE);
    virtual ~phlumphview();

    void FullUpdate(enum view::UpdateType type, long left, long top, long width, long height);
    void Update();
    void ObservedChanged(class observable  *observed, long  status);
    void SetDataObject(class dataobject *dobj);
    void LinkTree( class view  *parent );
    view::DSattributes DesiredSize(long width, long height, enum view::DSpass pass, long *desiredWidth, long *desiredHeight);
    void WantInputFocus(class view *requestor);
    void ReceiveInputFocus();
    void LoseInputFocus();
    void PostMenus(class menulist *ml);
    void PostKeyState(class keystate *ks);
    class view *Hit(enum view::MouseAction action, long x, long y, long numberOfClicks);
    void *GetPSPrintInterface(const char *printtype);

    class menulist *Menus;
    class keystate *Keystate;
    boolean IconMode; /* T if this is just a little icon that can open the real view. */
    boolean HasInputFocus;	/* T if received input focus */

    /* data relevant only to iconmode views */
    class phlumphview *toolset;

    /* data relevant only to real views */
    int curnum;
    class textflowview *curview;
    int lastwidth, lastheight;
};
/** @} */
#endif /* _phlumphview_H_ */
