#ifndef _ltextview_H_
#define _ltextview_H_
/* File ltextview.H
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

   ltextview: a textview subclass specialized for dealing with Lisp code. */

#include "srctextview.H"
class ltextview: public srctextview {
  public:
    virtual ATKregistryEntry *ATKregistry();

    class keystate *PrependKeyState();
    void PostMenus(class menulist *menulist);

    static boolean InitializeClass();
    ltextview();
    virtual ~ltextview();

    class keystate *l_state;
    class menulist *l_menus;    
};

#endif /* _ltextview_H_ */
