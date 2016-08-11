ATK_INTER
#ifndef _pvaltv_H_
#define _pvaltv_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University. All rights Reserved. */

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



#include <textintv.H>
class pvaltv : public  textintv {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	static boolean InitializeClass();

    
	pvaltv();
    
	virtual ~pvaltv();

    virtual void UpdateText(long val);
    virtual void UpdateValue();
    virtual int Locate(long pos)  ;

     void ObservedChanged(class observable *changed, long value);
     class keystate * Keys()  ;
     class menulist * Menus()  ;
     void SetDataObject(class dataobject *d);


    class keystate *ks;
    class menulist *menulistp;
};

#endif /* _pvaltv_H_ */
