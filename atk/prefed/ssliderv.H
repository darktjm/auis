ATK_INTER
#ifndef _ssliderv_H_
#define _ssliderv_H_
/* Copyright 1992 by the Andrew Toolkit Consortium and Carnegie Mellon University All rights Reserved. */

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



#include <scroll.H>
class ssliderv : public  scroll {
public:
	virtual ATKregistryEntry *ATKregistry();


    
	ssliderv();
    
	virtual ~ssliderv();

    virtual long WhatIsAt(long num, long denom)  ;
    virtual void Endzone(int end, enum view_MouseAction act);
    virtual void SetFrame(long pos, long num, long denom);
    virtual void GetInfo(struct range *total, struct range *seen, struct range *dot);

     view_DSattributes DesiredSize(long width, long height, enum view_DSpass pass, long *dWidth, long *dheight)  ;
     void GetOrigin(long width, long height, long *originX, long *originY);

     const void * GetInterface(const char *name)  ;
     void FullUpdate(enum view_UpdateType type, long left, long top, long width, long right);



};

#endif /* _ssliderv_H_ */
