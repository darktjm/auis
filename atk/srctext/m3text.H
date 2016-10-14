#ifndef _m3text_H_
#define _m3text_H_
/** \addtogroup libsrctext
 * @{ */
/* File m3text.H created by R L Quinn
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   m3text: a text subclass specialized for dealing with Modula-3 code. */

#include "modtext.H"
class m3text: public modtext {
  public:
    TRACED_CLASS_NODEST(m3text);

    boolean Quoted(long pos);
    void RedoStyles();
    void SetupStyles();

    static boolean InitializeClass();
    m3text();
};
/** @} */
#endif /* _m3text_H_ */
