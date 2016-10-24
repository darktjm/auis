#ifndef _htmlview_H_
#define _htmlview_H_
/*
 * Copyright 1993, City University
 * Copyright 1993, 1994, Nick Williams. 
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose is hereby granted without fee, 
 * provided that the above copyright notice appear in all copies and that 
 * both that copyright notice, this permission notice, and the following 
 * disclaimer appear in supporting documentation, and that the names of 
 * IBM, Carnegie Mellon University, and other copyright holders, not be 
 * used in advertising or publicity pertaining to distribution of the software 
 * without specific, written prior permission.
 *
 * IBM, CARNEGIE MELLON UNIVERSITY, AND THE OTHER COPYRIGHT HOLDERS 
 * DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING 
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO EVENT 
 * SHALL IBM, CARNEGIE MELLON UNIVERSITY, OR ANY OTHER COPYRIGHT HOLDER 
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY 
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, 
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS 
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */
/** \addtogroup libhtml
 * @{ */

#include <textview.H>
/** A viewer for \ref html objects.
 *  This view displays the HTML text as if it were an ATK document, and adds
 *  proc commands to support HTML editing.
 */
class htmlview : public  textview {
public:
	TRACED_CLASS(htmlview);

  
     void PostKeyState(class keystate* ks);
     void PostMenus(class menulist* menulist);
     class view* Hit(enum view::MouseAction action, long x, long y, long numberOfClicks)  ;

  
    
	static boolean InitializeClass();

    
	htmlview();
    

  
    class keystate* kstate;
    class menulist* menus;
    class environment* styleInQuestion; /* When editing styles, this is the selected env */
};
/** @} */
#endif /* _htmlview_H_ */
