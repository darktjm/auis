#ifndef _hiddenview_H_
#define _hiddenview_H_
/** \addtogroup libhtmltext
 * @{ */
/* File hiddenview.H created by Ward Nelson
      (c) Copyright IBM Corp 1995.  All rights reserved. 

   hiddenv, an inset for stashing non-displayed text inside of */

#define hiddenview_FullScreenMenus 32

#include <textview.H>
class hiddenview : public textview {
  public:
    virtual ATKregistryEntry *ATKregistry();

    void LoseInputFocus();
    void ReceiveInputFocus();
    void PostMenus(class menulist *menulist);

    view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long *dWidth, long *dheight);
    void Print(FILE *file, const char *processor, const char *finalFormat, boolean topLevel);
    void Update();
    void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);

    static boolean InitializeClass();
    hiddenview();
    virtual ~hiddenview();

    class keystate *hid_state;
    class menulist *hid_menus;    

    static keymap *hid_Map;
    static menulist *hid_Menus;
};
/** @} */
#endif /* _hiddenview_H_ */
