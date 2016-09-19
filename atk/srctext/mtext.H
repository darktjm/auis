#ifndef _mtext_H_
#define _mtext_H_
/* File mtext.H created by R L Quinn
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   mtext: a text subclass specialized for dealing with Modula-2 code. */

#include "modtext.H"
class mtext: public modtext {
  public:
    virtual ATKregistryEntry *ATKregistry();

    void SetupStyles();

    static boolean InitializeClass();
    mtext();
};

#endif /* _mtext_H_ */
