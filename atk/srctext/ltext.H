#ifndef _ltext_H_
#define _ltext_H_
/* File ltext.H
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

   ltext: a text subclass specialized for dealing with Lisp code. */

struct indenter {
    char *name;
    int args;
    struct indenter *next;
};

#include "srctext.H"
class ltext: public srctext {
  public:
    virtual ATKregistryEntry *ATKregistry();

    void SetAttributes(struct attributes *atts);
    int Indentation(long pos);
    void RedoStyles();
    boolean Quoted(long pos);

    virtual long ReverseBlance(long pos, int type);

    ltext();

    struct indenter *Indents;
};

#endif /* _ltext_H_ */
