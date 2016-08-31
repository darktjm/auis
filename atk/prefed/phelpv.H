ATK_INTER
#ifndef _phelpv_H_
#define _phelpv_H_
/* Copyright 1992 by Carnegie Mellon University All rights Reserved.
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
//  $ */

#include <textview.H>
class phelpv : public  textview  {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	static boolean InitializeClass();

    
	phelpv();
    
	virtual ~phelpv();

     void SetDotPosition(long pos);
     class view * Hit(enum view_MouseAction action, long x, long y, long clicks)  ;



    inline void SetPrefs(class prefs *p) { ((this)->prefs=(p)); }
    inline class prefs  * GetPrefs() { return ((this)->prefs); }

    class prefs *prefs;
};

#endif /* _phelpv_H_ */
