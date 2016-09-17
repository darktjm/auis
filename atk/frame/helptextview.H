ATK_INTER
#ifndef _helptextview_H_
#define _helptextview_H_
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


 

#include <textview.H>

class helptextview : public  textview {
public:
	virtual ATKregistryEntry *ATKregistry();

    
       void WantInputFocus( class view *requestor);
       void GetClickPosition(long position, long numberOfClicks, enum view_MouseAction action, long startLeft, long startRight, long *leftPos, long *rightPos);
       class view * Hit (enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
       void FullUpdate(enum view_UpdateType type, long left, long top, long width, long height);
    
      
	virtual ~helptextview();
      
	helptextview();    
    
      class cursor *myCursor;

};


#endif /* _helptextview_H_ */
