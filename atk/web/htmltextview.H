#ifndef _htmltextview_H_
#define _htmltextview_H_
/** \addtogroup libhtmltext
 * @{ */
/* File htmltextv.H created by Ward Nelson
       (c) Copyright IBM Corp 1995.  All rights reserved. 

    htmltextv, an object for viewing HTML+ formatted files. 
*/


#include <textview.H>
class htmltextview: public textview {
    long hitpos;
  public:
    TRACED_CLASS(htmltextview);

    void SetDataObject(class dataobject *dataObject);
    class view *Hit(enum view::MouseAction action, long x, long y, long numberOfClicks);
    void PostMenus(class menulist *menulist);
    void PostKeyState(class keystate *keystate);
    virtual boolean ConfirmReadOnly();

    htmltextview();
    static boolean InitializeClass();

    inline long HitPos() {
	return hitpos;
    }
    
    class keystate *html_state;
    class menulist *html_menus;
    boolean DisplayingHref;
};
/** @} */
#endif /* _htmltextview_H_ */
