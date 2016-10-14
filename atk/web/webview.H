/* Copyright 1995 Carnegie Mellon University  --All rights reserved */
#ifndef _webview_H_
#define _webview_H_
/** \addtogroup libhtmltext
 * @{ */
#include <htmltextview.H>
class webview: public htmltextview {
public:
	TRACED_CLASS(webview);
	static boolean InitializeClass();
	webview();

    static int VisitWeb(class view *self,const char *url, int flags=0, const char *postdata=NULL) ;
    static void movehist(class view *self,int res,class view *v);
    static void Clone(class view *self);
    static void OpenURL(class view *self);

    virtual void SetRemote(class im *im);

     class view* Hit(enum view::MouseAction action, 
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
/** @} */
#endif /* _webview_H_ */
