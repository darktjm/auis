ATK_INTER
#ifndef _fontsample_H_
#define _fontsample_H_
/* fontsamp.ch - font sample view */
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

#include <view.H>

class fontsample  : public  view {
public:
	virtual ATKregistryEntry *ATKregistry();


    
      
	static boolean InitializeClass();

      
	fontsample();
      
	virtual ~fontsample();

    
       void FullUpdate(enum view_UpdateType type, long left, long top, long width, long height);
       void Update();
       class view * Hit(enum view_MouseAction action, long x, long y, long n)	 ;
       void ObservedChanged(class observable *fontsel, long status);

    
      virtual void SetString(char *val);
      virtual class fontdesc * GetFontDesc()  ;

    
      inline char  * GetString()  { return ((this)->teststring); }

    
      char *teststring;
      boolean dirty;
      class fontdesc *fdesc;
};

#endif /* _fontsample_H_ */
