#ifndef _urlbuttonview_H_
#define _urlbuttonview_H_
/** \addtogroup libhtmltext
 * @{ */
/*
 * urlbtnv -- urlbuttonview
 *
 * A WWW hyperlink button.
 *
   (c) Copyright IBM Corp 1995.  All rights reserved. 
 */

#include <sbuttonv.H>

class menulist;
class keymap;
class urlbutton;

class urlbuttonview : public sbuttonv {
  public:
    urlbuttonview();
    virtual ~urlbuttonview();
    virtual ATKregistryEntry *ATKregistry();
    static boolean InitializeClass();

    // overrides
    boolean Touch(int ind, enum view_MouseAction action);
    void ReceiveInputFocus();
    void *GetPSPrintInterface(const char *printtype);

    // new function
    inline urlbutton *GetURLButton()	{ return (urlbutton *)ButtonData(); }
    static void SetURLProc(urlbuttonview *ubv, char *arg);	// urlbutton-set-url
    static void SetLabelProc(urlbuttonview *ubv, char *arg);	// urlbutton-set-label

  private:
    menulist *ml;
    keystate *ks;
    struct textview_insetdata *tidata;	// for printing

    void StartBrowser(const char *url);
    void OpenURL(const char *url);
    struct textview_insetdata *GetTextPrintData();
    void DestroyTextPrintData();

    static menulist *s_ml;
    static keymap *s_km;
};
/** @} */
#endif /* _urlbuttonview_H_ */
