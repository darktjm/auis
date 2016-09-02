ATK_INTER
#ifndef _phlumphview_H_
#define _phlumphview_H_

/*
	Copyright Carnegie Mellon University 1994 - All rights reserved
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

#include <view.H>

class phlumphview : public view {
  public:
    virtual ATKregistryEntry *ATKregistry();

    static boolean InitializeClass();
    phlumphview(boolean iconmode = TRUE);
    virtual ~phlumphview();

    void FullUpdate(enum view_UpdateType type, long left, long top, long width, long height);
    void Update();
    void ObservedChanged(class observable  *observed, long  status);
    void SetDataObject(class dataobject *dobj);
    void LinkTree( class view  *parent );
    view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long *desiredWidth, long *desiredHeight);
    void WantInputFocus(class view *requestor);
    void ReceiveInputFocus();
    void LoseInputFocus();
    void PostMenus(class menulist *ml);
    void PostKeyState(class keystate *ks);
    class view *Hit(enum view_MouseAction action, long x, long y, long numberOfClicks);
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

#endif /* _phlumphview_H_ */
