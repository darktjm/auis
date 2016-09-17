ATK_INTER
#ifndef _pagev_H_
#define _pagev_H_
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


 

struct pagev_switcheroo {
    struct page_switchee *switchee;
    class view *v;
    struct pagev_switcheroo *next;
};

#include <view.H>
class pagev: public view {
public:
	virtual ATKregistryEntry *ATKregistry();

    
      
	static boolean InitializeClass();

      
	pagev();
      
	virtual ~pagev();
    
       void FullUpdate(enum view_UpdateType type, long left,
		  long top, long width, long height);
       void Update();
       class view * Hit (enum view_MouseAction action, long x, long y,
	    long numberOfClicks)  ;
       void PostKeyState(class keystate *keystate);
       void PostMenus(class menulist *ml);
       void LinkTree(class view *parent);
       void WantInputFocus(class view *v);
       void InitChildren() ;
     view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long *dWidth, long *dheight)  ;
     void Print(FILE *file, const char *processor, const char *finalFormat, boolean topLevel);
     void ObservedChanged (class observable *changed, long value);

    
      class keystate *ks;
      class menulist *ml;
      struct pagev_switcheroo *Firstswitcheroo, *NowPlaying;
};

#endif /* _pagev_H_ */
