ATK_INTER
#ifndef _consoleClass_H_
#define _consoleClass_H_
/* ********************************************************************** *\
 *         Copyright IBM Corporation 1988,1991 - All Rights Reserved      *
 *        For full copyright information see:'andrew/doc/COPYRITE'     *
\* ********************************************************************** */

/*
	$Disclaimer: 
// Permission to use, copy, modify, and distribute this software and its 
// documentation for any purpose and without fee is hereby granted, provided 
// that the above copyright notice appear in all copies and that both that 
// copyright notice and this permission notice appear in supporting 
// documentation, and that the name of IBM not be used in advertising or 
// publicity pertaining to distribution of the software without specific, 
// written prior permission. 
//                         
// THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD 
// TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL ANY COPYRIGHT 
// HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
// DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
// DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
// OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION 
// WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
// 
//  $
*/


 



#define consoleClass_VERSION 1
#define BOXINTERVAL 10

#include <view.H>
class consoleClass : public  view {
public:
	virtual ATKregistryEntry *ATKregistry();


     void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);
     class view * Hit(enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
     void ReceiveInputFocus();
     void LoseInputFocus();
     void Update();
     void WantUpdate(class view *requestor);
     void PostMenus(class menulist *menulist);

    consoleClass();    
    static boolean InitializeClass();


    class menulist *stdMenulist;
    class menulist *userMenulist;
    boolean haveInputFocus;
    boolean interactive;
    long menuMask;
    boolean is_visible;
};

#endif /* _consoleClass_H_ */
