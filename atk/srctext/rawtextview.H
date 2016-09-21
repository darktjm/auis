#ifndef _rawtextview_H_
#define _rawtextview_H_
/** \addtogroup libsrctext
 * @{ */
/* File rawtextview.H created by R S Kemmetmueller
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   rawtextview: a dataobject for plain ASCII files, with overstrike mode. */

#include <textview.H>
class rawtextview: public textview {
  public:
    virtual ATKregistryEntry *ATKregistry();

    void ReceiveInputFocus();

    static boolean InitializeClass();
    rawtextview();
    virtual ~rawtextview();

    class keystate *raw_state;
    class menulist *raw_menus;    
};
/** @} */
#endif /* _rawtextview_H_ */
