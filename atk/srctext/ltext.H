#ifndef _ltext_H_
#define _ltext_H_
/* File ltext.H
   (c) Copyright IBM Corp.  1988-1995.  All rights reserved.

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
