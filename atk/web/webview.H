/* Copyright 1995 Carnegie Mellon University  --All rights reserved

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
#ifndef _webview_H_
#define _webview_H_
ATK_INTER

#include <htmltextview.H>
class webview: public htmltextview {
public:
	virtual ATKregistryEntry *ATKregistry();
	static boolean InitializeClass();
	webview();
	virtual ~webview();

    static int VisitWeb(class view *self,const char *url, int flags=0, const char *postdata=NULL) ;
    static void movehist(class view *self,int res,class view *v);
    static void Clone(class view *self);
    static void OpenURL(class view *self);

    virtual void SetRemote(class im *im);

     class view* Hit(enum view_MouseAction action, 
		long x, long y, long numberOfClicks)  ;
     class view * GetApplicationLayer()  ;
     void PostMenus(class menulist *menulist);
     void ObservedChanged (class observable *changed, 
		long value);
     void WantInputFocus(class view *requestor);
     void SetDataObject(class dataobject *obj);
     virtual void MoveToAnchor();
     
	class value *bt;
	class wbuttonv *bv;
	class view *subview;
	class lpair *lpMain;
      	class menulist *web_menus;
	class im *remoteim; /* used when view is being used as 
                             a remote controller (as with hotlists) */
        view *mouseFocus;
};

#endif /* _webview_H_ */
