ATK_INTER
#ifndef _panner_H_
#define _panner_H_
/* panner.ch - panner box view */

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

/*
	Copyright Carnegie Mellon University 1992 - All rights reserved
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


#include <rect.h>

#define scroll_DRAGGING (-4)
#define panner_NumberOfCursors (4)

#include <scroll.H>

class panner : public  scroll {
public:
	virtual ATKregistryEntry *ATKregistry();


    
      
	static boolean InitializeClass();

      static class panner * Create(class view *view)  ;
      
	panner();
      
	virtual ~panner();

    
       void SetLocation(int location);
       void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);
       void Update();
       class view * Hit(enum view_MouseAction action, long x, long y, long numberOfClicks)  ;
       void PostCursor(struct rectangle *rec, class cursor *cursor);

    
      boolean MotifStyle;
      int PannerShade;
      struct rectangle panrect;
      boolean visible;
      struct rectangle oldpanrect;
      boolean oldvisible;
      boolean panrectchanged;
      class region *pannerreg, *childclip;
      long idealwidth, idealheight;
      struct rectangle seenrect, dotrect, gseenrect;
      long rockx, rocky, isatx, isaty;
      class cursor *cursors[panner_NumberOfCursors];
};

#endif /* _panner_H_ */
